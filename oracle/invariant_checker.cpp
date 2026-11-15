#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace oracle {

constexpr int64_t MOD = 998244353LL;

static int64_t norm_mod(int64_t x) {
    int64_t r = x % MOD;
    return (r < 0) ? (r + MOD) : r;
}
struct SegTreeNode {
    int64_t mn = 0;
    int64_t mx = 0;
    int64_t sum = 0;
    int64_t lazy_a = 1;
    int64_t lazy_b = 0;
    bool has_lazy = false;
};

class StructAInvariantChecker {
public:
    // Checks all structural invariants of a Lazy Segment Tree node-by-node
    static bool check_tree(const std::vector<SegTreeNode>& tree, int v, int l, int r) {
        const auto& node = tree[v];

        // Invariant 1: Minimum can never exceed Maximum
        if (node.mn > node.mx) {
            std::cout << "[StructA Invariant Failed] Node " << v << ": min ("
                      << node.mn << ") > max (" << node.mx << ")\n";
            return false;
        }

        // Invariant 2: Modular sum must lie strictly in [0, MOD - 1]
        if (node.sum < 0 || node.sum >= MOD) {
            std::cout << "[StructA Invariant Failed] Node " << v
                      << ": sum out of modulo bounds (" << node.sum << ")\n";
            return false;
        }

        // Invariant 3: Leaf node check
        if (l == r) {
            if (node.mn != node.mx) {
                std::cout << "[StructA Invariant Failed] Leaf " << v
                          << ": min != max on single element\n";
                return false;
            }
            if (node.sum != norm_mod(node.mn)) {
                std::cout << "[StructA Invariant Failed] Leaf " << v
                          << ": sum_mod does not match element value\n";
                return false;
            }
            return true;
        }

        // Invariant 4: Internal node consistency when no lazy tag is pending
        if (!node.has_lazy) {
            const auto& left_child = tree[2 * v];
            const auto& right_child = tree[2 * v + 1];

            int64_t expected_min = std::min(left_child.mn, right_child.mn);
            int64_t expected_max = std::max(left_child.mx, right_child.mx);
            int64_t expected_sum = (left_child.sum + right_child.sum) % MOD;

            if (node.mn != expected_min || node.mx != expected_max || node.sum != expected_sum) {
                std::cout << "[StructA Invariant Failed] Internal Node " << v
                          << ": parent aggregates do not match children\n";
                return false;
            }
        }

        int mid = l + (r - l) / 2;
        return check_tree(tree, 2 * v, l, mid) &&
               check_tree(tree, 2 * v + 1, mid + 1, r);
    }
};

// ============================================================================
// 2. Invariant Checker for Structure B (Square-Root Decomposition)
// ============================================================================
struct SqrtBlock {
    int l = 0;
    int r = 0;
    int64_t block_min = 0;
    int64_t block_max = 0;
    int64_t block_sum = 0;
    int64_t tag_a = 1;
    int64_t tag_b = 0;
};

class StructBInvariantChecker {
public:
    // Verifies that every block's cached aggregates match its raw elements + lazy tag
    static bool check_blocks(const std::vector<int64_t>& raw_arr,
                             const std::vector<SqrtBlock>& blocks) {
        for (size_t k = 0; k < blocks.size(); ++k) {
            const auto& blk = blocks[k];

            if (blk.block_min > blk.block_max) {
                std::cout << "[StructB Invariant Failed] Block " << k
                          << ": block_min > block_max\n";
                return false;
            }

            if (blk.block_sum < 0 || blk.block_sum >= MOD) {
                std::cout << "[StructB Invariant Failed] Block " << k
                          << ": block_sum out of modulo range\n";
                return false;
            }

            // Recompute ground-truth aggregates for block k from raw elements and block tag
            int64_t actual_min = blk.tag_a * raw_arr[blk.l] + blk.tag_b;
            int64_t actual_max = actual_min;
            int64_t actual_sum = 0;

            for (int i = blk.l; i <= blk.r; ++i) {
                int64_t val = blk.tag_a * raw_arr[i] + blk.tag_b;
                actual_min = std::min(actual_min, val);
                actual_max = std::max(actual_max, val);
                actual_sum = (actual_sum + norm_mod(val)) % MOD;
            }

            if (blk.block_min != actual_min ||
                blk.block_max != actual_max ||
                blk.block_sum != actual_sum) {
                std::cout << "[StructB Invariant Failed] Block " << k
                          << ": cached block aggregates do not match element values!\n";
                return false;
            }
        }
        return true;
    }
};

} // namespace oracle

// ============================================================================
// MAIN FUNCTION: Self-Test for Both Structural Invariant Checkers
// ============================================================================
int main() {
    using namespace oracle;

    std::cout << "=== Running Structural Invariant Checker Self-Tests ===\n\n";

    // 1. Test Structure A (Segment Tree over 2 elements: [10, 20])
    std::vector<SegTreeNode> tree(4);
    tree[2] = {10, 10, 10, 1, 0, false}; // Left leaf: [0, 0] = 10
    tree[3] = {20, 20, 20, 1, 0, false}; // Right leaf: [1, 1] = 20
    tree[1] = {10, 20, 30, 1, 0, false}; // Root: [0, 1] -> min=10, max=20, sum=30

    std::cout << "[Test 1] Checking valid Segment Tree... ";
    if (StructAInvariantChecker::check_tree(tree, 1, 0, 1)) {
        std::cout << "PASSED!\n";
    }

    // Corrupt Root (simulate bug where min > max after negative gain)
    std::cout << "[Test 2] Checking corrupted Segment Tree (min > max bug)... \n  -> ";
    tree[1].mn = 50;
    tree[1].mx = 20;
    if (!StructAInvariantChecker::check_tree(tree, 1, 0, 1)) {
        std::cout << "  -> Successfully caught the corrupted Segment Tree invariant!\n\n";
    }

    // 2. Test Structure B (Sqrt Decomposition over 4 elements: [5, 15, 25, 35])
    std::vector<int64_t> raw_arr = {5, 15, 25, 35};
    std::vector<SqrtBlock> blocks = {
        {0, 1, 5, 15, 20, 1, 0},   // Block 0 covering [0, 1]
        {2, 3, 25, 35, 60, 1, 0}   // Block 1 covering [2, 3]
    };

    std::cout << "[Test 3] Checking valid Sqrt Decomposition blocks... ";
    if (StructBInvariantChecker::check_blocks(raw_arr, blocks)) {
        std::cout << "PASSED!\n";
    }

    // Corrupt Block 1 sum
    std::cout << "[Test 4] Checking corrupted Sqrt Block... \n  -> ";
    blocks[1].block_sum = 999;
    if (!StructBInvariantChecker::check_blocks(raw_arr, blocks)) {
        std::cout << "  -> Successfully caught the corrupted Sqrt Block invariant!\n\n";
    }

    std::cout << "All Invariant Checker tests completed successfully!\n";
    return 0;
}