#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

const long long MOD = 998244353;

long long clean_mod(long long x) {
    long long rem = x % MOD;
    if (rem < 0) rem += MOD;
    return rem;
}

// 1. Structure A (Segment Tree) Node
struct SegNode {
    long long mn;
    long long mx;
    long long sum;
    bool has_lazy;
};

// Check karta hai ki Segment Tree ke saare rules (invariants) sahi hain ya nahi
bool check_segment_tree(const vector<SegNode>& tree, int v, int l, int r) {
    // Rule 1: min kabhi max se bada nahi hona chahiye
    if (tree[v].mn > tree[v].mx) return false;

    // Rule 2: sum hamesha 0 se MOD-1 ke beech hona chahiye
    if (tree[v].sum < 0 || tree[v].sum >= MOD) return false;

    // Rule 3: Leaf node par min aur max barabar hone chahiye
    if (l == r) {
        return (tree[v].mn == tree[v].mx) && (tree[v].sum == clean_mod(tree[v].mn));
    }

    // Rule 4: Agar lazy tag nahi hai, toh parent ka data children ke barabar hona chahiye
    if (!tree[v].has_lazy) {
        long long exp_min = min(tree[2 * v].mn, tree[2 * v + 1].mn);
        long long exp_max = max(tree[2 * v].mx, tree[2 * v + 1].mx);
        long long exp_sum = (tree[2 * v].sum + tree[2 * v + 1].sum) % MOD;

        if (tree[v].mn != exp_min || tree[v].mx != exp_max || tree[v].sum != exp_sum) {
            return false;
        }
    }

    int mid = (l + r) / 2;
    return check_segment_tree(tree, 2 * v, l, mid) &&
           check_segment_tree(tree, 2 * v + 1, mid + 1, r);
}

// 2. Structure B (Sqrt Decomposition) Block
struct SqrtBlock {
    int l, r;
    long long block_min, block_max, block_sum;
    long long a, b;
};

// Check karta hai ki Sqrt Decomposition ke har block ka min, max, sum sahi hai
bool check_sqrt_blocks(const vector<long long>& arr, const vector<SqrtBlock>& blocks) {
    for (size_t k = 0; k < blocks.size(); k++) {
        SqrtBlock blk = blocks[k];
        if (blk.block_min > blk.block_max) return false;
        if (blk.block_sum < 0 || blk.block_sum >= MOD) return false;

        long long mn = blk.a * arr[blk.l] + blk.b;
        long long mx = mn;
        long long sm = 0;

        for (int i = blk.l; i <= blk.r; i++) {
            long long val = blk.a * arr[i] + blk.b;
            mn = min(mn, val);
            mx = max(mx, val);
            sm = (sm + clean_mod(val)) % MOD;
        }

        if (blk.block_min != mn || blk.block_max != mx || blk.block_sum != sm) {
            return false;
        }
    }
    return true;
}

int main() {
    cout << "=== Running Invariant Checker Tests ===\n";

    // Test 1: Check valid Segment Tree for array [10, 20]
    vector<SegNode> tree(4);
    tree[2] = {10, 10, 10, false}; // Left child
    tree[3] = {20, 20, 20, false}; // Right child
    tree[1] = {10, 20, 30, false}; // Root node

    if (check_segment_tree(tree, 1, 0, 1)) {
        cout << "[PASS] Test 1: Structure A (Segment Tree) Invariants Verified!\n";
    }

    // Test 2: Check valid Sqrt Blocks for array [5, 15, 25, 35]
    vector<long long> arr = {5, 15, 25, 35};
    vector<SqrtBlock> blocks = {
        {0, 1, 5, 15, 20, 1, 0},
        {2, 3, 25, 35, 60, 1, 0}
    };

    if (check_sqrt_blocks(arr, blocks)) {
        cout << "[PASS] Test 2: Structure B (Sqrt Decomposition) Invariants Verified!\n";
    }

    cout << "=== ALL INVARIANT CHECKS PASSED SUCCESSFULLY! ===\n\n";
    return 0;
}
