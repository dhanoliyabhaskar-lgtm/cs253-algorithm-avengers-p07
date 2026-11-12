#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <chrono>

namespace oracle {

constexpr int64_t MOD = 998244353LL;

struct QueryResult {
    int64_t min_val;
    int64_t max_val;
    int64_t sum_mod;

    bool operator==(const QueryResult& other) const {
        return min_val == other.min_val &&
               max_val == other.max_val &&
               sum_mod == other.sum_mod;
    }

    bool operator!=(const QueryResult& other) const {
        return !(*this == other);
    }
};

struct Operation {
    char type; // 'U' for Update, 'Q' for Query
    int l;
    int r;
    int64_t a;
    int64_t b;
};

// 1. Ground-Truth Plain Array Oracle
class ArrayOracle {
private:
    std::vector<int64_t> exact_data_;
    std::vector<int64_t> mod_data_;

    static int64_t normalize_mod(int64_t x) {
        int64_t rem = x % MOD;
        return (rem < 0) ? (rem + MOD) : rem;
    }

public:
    explicit ArrayOracle(const std::vector<int64_t>& init_values)
        : exact_data_(init_values), mod_data_(init_values.size(), 0) {
        for (size_t i = 0; i < init_values.size(); ++i) {
            mod_data_[i] = normalize_mod(init_values[i]);
        }
    }

    void range_update(int l, int r, int64_t a, int64_t b) {
        int64_t a_mod = normalize_mod(a);
        int64_t b_mod = normalize_mod(b);

        for (int i = l; i <= r; ++i) {
            exact_data_[i] = a * exact_data_[i] + b;
            __int128 prod = static_cast<__int128>(a_mod) * mod_data_[i] + b_mod;
            mod_data_[i] = static_cast<int64_t>(prod % MOD);
        }
    }

    QueryResult range_query(int l, int r) const {
        int64_t current_min = exact_data_[l];
        int64_t current_max = exact_data_[l];
        int64_t current_sum = 0;

        for (int i = l; i <= r; ++i) {
            if (exact_data_[i] < current_min) current_min = exact_data_[i];
            if (exact_data_[i] > current_max) current_max = exact_data_[i];
            current_sum += mod_data_[i];
            if (current_sum >= MOD) current_sum -= MOD;
        }
        return QueryResult{current_min, current_max, current_sum};
    }
};

// 2. Reference Lazy Segment Tree + Structural Invariant Checker
class ReferenceLazySegTree {
public:
    struct Node {
        int64_t mn = 0;
        int64_t mx = 0;
        int64_t sum = 0;
        int64_t lazy_a = 1;
        int64_t lazy_b = 0;
        bool has_lazy = false;
    };

private:
    int n_;
    std::vector<Node> tree_;
    bool bug_injected_ = false;

    static int64_t norm_mod(int64_t x) {
        int64_t r = x % MOD;
        return (r < 0) ? (r + MOD) : r;
    }

    void apply_tag(int v, int l, int r, int64_t a, int64_t b) {
        int64_t len = (r - l + 1);
        int64_t a_mod = norm_mod(a);
        int64_t b_mod = norm_mod(b);

        __int128 new_sum = static_cast<__int128>(a_mod) * tree_[v].sum +
                           static_cast<__int128>(b_mod) * len;
        tree_[v].sum = static_cast<int64_t>(new_sum % MOD);

        int64_t old_mn = tree_[v].mn;
        int64_t old_mx = tree_[v].mx;
        if (a == 1) {
            tree_[v].mn = old_mn + b;
            tree_[v].mx = old_mx + b;
        } else if (a == 0) {
            tree_[v].mn = b;
            tree_[v].mx = b;
        } else if (a == -1) {
            if (bug_injected_) {
                // Intentional bug for shrinker demo: fails to swap min and max
                tree_[v].mn = -old_mn + b;
                tree_[v].mx = -old_mx + b;
            } else {
                tree_[v].mn = -old_mx + b;
                tree_[v].mx = -old_mn + b;
            }
        }

        tree_[v].lazy_a = a * tree_[v].lazy_a;
        tree_[v].lazy_b = a * tree_[v].lazy_b + b;
        tree_[v].has_lazy = true;
    }

    void push_down(int v, int l, int r) {
        if (!tree_[v].has_lazy || l == r) return;
        int mid = l + (r - l) / 2;
        apply_tag(2 * v, l, mid, tree_[v].lazy_a, tree_[v].lazy_b);
        apply_tag(2 * v + 1, mid + 1, r, tree_[v].lazy_a, tree_[v].lazy_b);
        tree_[v].lazy_a = 1;
        tree_[v].lazy_b = 0;
        tree_[v].has_lazy = false;
    }

    void pull_up(int v) {
        tree_[v].mn = std::min(tree_[2 * v].mn, tree_[2 * v + 1].mn);
        tree_[v].mx = std::max(tree_[2 * v].mx, tree_[2 * v + 1].mx);
        tree_[v].sum = (tree_[2 * v].sum + tree_[2 * v + 1].sum) % MOD;
    }

    void build(int v, int l, int r, const std::vector<int64_t>& init) {
        if (l == r) {
            tree_[v].mn = init[l];
            tree_[v].mx = init[l];
            tree_[v].sum = norm_mod(init[l]);
            return;
        }
        int mid = l + (r - l) / 2;
        build(2 * v, l, mid, init);
        build(2 * v + 1, mid + 1, r, init);
        pull_up(v);
    }

    void update(int v, int l, int r, int ql, int qr, int64_t a, int64_t b) {
        if (ql <= l && r <= qr) {
            apply_tag(v, l, r, a, b);
            return;
        }
        push_down(v, l, r);
        int mid = l + (r - l) / 2;
        if (ql <= mid) update(2 * v, l, mid, ql, qr, a, b);
        if (qr > mid)  update(2 * v + 1, mid + 1, r, ql, qr, a, b);
        pull_up(v);
    }

    QueryResult query_internal(int v, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) {
            return {tree_[v].mn, tree_[v].mx, tree_[v].sum};
        }
        push_down(v, l, r);
        int mid = l + (r - l) / 2;
        if (qr <= mid) return query_internal(2 * v, l, mid, ql, qr);
        if (ql > mid)  return query_internal(2 * v + 1, mid + 1, r, ql, qr);

        QueryResult L = query_internal(2 * v, l, mid, ql, qr);
        QueryResult R = query_internal(2 * v + 1, mid + 1, r, ql, qr);
        return {
            std::min(L.min_val, R.min_val),
            std::max(L.max_val, R.max_val),
            (L.sum_mod + R.sum_mod) % MOD
        };
    }

    bool check_invariants(int v, int l, int r) const {
        if (tree_[v].mn > tree_[v].mx) return false;
        if (tree_[v].sum < 0 || tree_[v].sum >= MOD) return false;
        if (l == r) return true;
        int mid = l + (r - l) / 2;
        if (!tree_[v].has_lazy) {
            if (tree_[v].mn != std::min(tree_[2 * v].mn, tree_[2 * v + 1].mn)) return false;
            if (tree_[v].mx != std::max(tree_[2 * v].mx, tree_[2 * v + 1].mx)) return false;
            if (tree_[v].sum != (tree_[2 * v].sum + tree_[2 * v + 1].sum) % MOD) return false;
        }
        return check_invariants(2 * v, l, mid) && check_invariants(2 * v + 1, mid + 1, r);
    }

public:
    explicit ReferenceLazySegTree(const std::vector<int64_t>& init, bool inject_bug = false)
        : n_(static_cast<int>(init.size())), tree_(4 * n_), bug_injected_(inject_bug) {
        build(1, 0, n_ - 1, init);
    }

    void range_update(int l, int r, int64_t a, int64_t b) {
        update(1, 0, n_ - 1, l, r, a, b);
    }

    QueryResult range_query(int l, int r) {
        return query_internal(1, 0, n_ - 1, l, r);
    }

    bool check_all_invariants() const {
        return check_invariants(1, 0, n_ - 1);
    }
};

// 3. Delta-Debugging Test Case Shrinker
bool reproduces_failure(const std::vector<int64_t>& init, const std::vector<Operation>& ops, bool inject_bug) {
    ArrayOracle orc(init);
    ReferenceLazySegTree seg(init, inject_bug);

    for (const auto& op : ops) {
        if (op.type == 'U') {
            orc.range_update(op.l, op.r, op.a, op.b);
            seg.range_update(op.l, op.r, op.a, op.b);
        } else {
            QueryResult expected = orc.range_query(op.l, op.r);
            QueryResult actual = seg.range_query(op.l, op.r);
            if (expected != actual || !seg.check_all_invariants()) {
                return true;
            }
        }
    }
    return false;
}

std::vector<Operation> shrink_failing_sequence(
    const std::vector<int64_t>& init,
    const std::vector<Operation>& failing_ops,
    bool inject_bug) {

    if (failing_ops.empty()) return {};
    Operation final_query = failing_ops.back();
    std::vector<Operation> prefix(failing_ops.begin(), failing_ops.end() - 1);

    size_t granularity = 2;
    while (!prefix.empty()) {
        size_t chunk_size = (prefix.size() + granularity - 1) / granularity;
        bool shrunk = false;

        for (size_t i = 0; i < prefix.size(); i += chunk_size) {
            std::vector<Operation> candidate;
            candidate.insert(candidate.end(), prefix.begin(), prefix.begin() + i);
            size_t end_idx = std::min(prefix.size(), i + chunk_size);
            candidate.insert(candidate.end(), prefix.begin() + end_idx, prefix.end());
            candidate.push_back(final_query);

            if (reproduces_failure(init, candidate, inject_bug)) {
                candidate.pop_back();
                prefix = std::move(candidate);
                granularity = std::max<size_t>(2, granularity - 1);
                shrunk = true;
                break;
            }
        }
        if (!shrunk) {
            if (granularity >= prefix.size()) break;
            granularity = std::min(prefix.size(), granularity * 2);
        }
    }

    prefix.push_back(final_query);
    return prefix;
}

} // namespace oracle

int main() {
    using namespace oracle;

    const uint64_t SEED = 20261117ULL;
    const int N = 100;
    const int TOTAL_OPS = 1000000;

    std::mt19937_64 rng(SEED);
    std::uniform_int_distribution<int> idx_dist(0, N - 1);
    std::uniform_int_distribution<int> type_dist(0, 1);
    std::uniform_int_distribution<int> gain_dist(-1, 1);
    std::uniform_int_distribution<int64_t> offset_dist(-1000, 1000);

    std::vector<int64_t> init_arr(N);
    for (int i = 0; i < N; ++i) init_arr[i] = offset_dist(rng);

    std::cout << "=== [1/2] Running Differential Test (1,000,000 Seeded Operations) ===\n";
    ArrayOracle orc(init_arr);
    ReferenceLazySegTree seg(init_arr, false);

    auto start_time = std::chrono::steady_clock::now();
    int queries_verified = 0;
    int invariant_audits = 0;

    for (int step = 1; step <= TOTAL_OPS; ++step) {
        int l = idx_dist(rng);
        int r = idx_dist(rng);
        if (l > r) std::swap(l, r);

        if (type_dist(rng) == 0) {
            int64_t a = gain_dist(rng);
            int64_t b = offset_dist(rng);
            orc.range_update(l, r, a, b);
            seg.range_update(l, r, a, b);
        } else {
            if (orc.range_query(l, r) != seg.range_query(l, r)) {
                std::cerr << "MISMATCH at step " << step << "!\n";
                return 1;
            }
            ++queries_verified;
        }

        if (step % 10000 == 0) {
            if (!seg.check_all_invariants()) {
                std::cerr << "INVARIANT FAILURE at step " << step << "!\n";
                return 1;
            }
            ++invariant_audits;
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(end_time - start_time).count();
    std::cout << "PASS: All " << TOTAL_OPS << " operations matched the Oracle!\n";
    std::cout << "Queries verified: " << queries_verified
              << " | Invariant Audits: " << invariant_audits
              << " | Time: " << elapsed_sec << " s\n\n";

    std::cout << "=== [2/2] Testing Failing-Case Shrinker (Delta Debugging) ===\n";
    std::vector<Operation> history;
    ArrayOracle bug_orc(init_arr);
    ReferenceLazySegTree buggy_seg(init_arr, true);

    for (int step = 1; step <= 500; ++step) {
        int l = idx_dist(rng);
        int r = idx_dist(rng);
        if (l > r) std::swap(l, r);

        int64_t a = (step < 50) ? 1 : gain_dist(rng);
        int64_t b = offset_dist(rng);

        if (step % 3 != 0) {
            Operation op{'U', l, r, a, b};
            history.push_back(op);
            bug_orc.range_update(l, r, a, b);
            buggy_seg.range_update(l, r, a, b);
        } else {
            Operation op{'Q', l, r, 0, 0};
            history.push_back(op);
            if (bug_orc.range_query(l, r) != buggy_seg.range_query(l, r)) {
                std::cout << "Bug detected at operation #" << history.size()
                          << ". Shrinking failing trace...\n";
                auto minimal = shrink_failing_sequence(init_arr, history, true);
                std::cout << "Shrunk from " << history.size()
                          << " operations down to minimal sequence of "
                          << minimal.size() << " operations:\n";
                for (size_t i = 0; i < minimal.size(); ++i) {
                    const auto& mop = minimal[i];
                    if (mop.type == 'U') {
                        std::cout << "  [" << i + 1 << "] UPDATE L=" << mop.l
                                  << " R=" << mop.r << " a=" << mop.a << " b=" << mop.b << "\n";
                    } else {
                        std::cout << "  [" << i + 1 << "] QUERY  L=" << mop.l
                                  << " R=" << mop.r << "\n";
                    }
                }
                break;
            }
        }
    }
    return 0;
}