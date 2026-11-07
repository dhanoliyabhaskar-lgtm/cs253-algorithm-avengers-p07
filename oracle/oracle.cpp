#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <iostream>

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
};

class ArrayOracle {
private:
    std::vector<int64_t> exact_data_;
    std::vector<int64_t> mod_data_;

    static int64_t normalize_mod(int64_t x) {
        int64_t rem = x % MOD;
        return (rem < 0) ? (rem + MOD) : rem;
    }

    void validate_range(int l, int r) const {
        int n = static_cast<int>(exact_data_.size());
        if (l < 0 || r < l || r >= n) {
            throw std::out_of_range("ArrayOracle: invalid range [l, r]");
        }
    }

public:
    explicit ArrayOracle(int n)
        : exact_data_(n, 0), mod_data_(n, 0) {}

    explicit ArrayOracle(const std::vector<int64_t>& init_values)
        : exact_data_(init_values), mod_data_(init_values.size(), 0) {
        for (size_t i = 0; i < init_values.size(); ++i) {
            mod_data_[i] = normalize_mod(init_values[i]);
        }
    }

    void range_update(int l, int r, int64_t a, int64_t b) {
        validate_range(l, r);
        int64_t a_mod = normalize_mod(a);
        int64_t b_mod = normalize_mod(b);

        for (int i = l; i <= r; ++i) {
            exact_data_[i] = a * exact_data_[i] + b;
            __int128 prod = static_cast<__int128>(a_mod) * mod_data_[i] + b_mod;
            mod_data_[i] = static_cast<int64_t>(prod % MOD);
        }
    }

    QueryResult range_query(int l, int r) const {
        validate_range(l, r);
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

} // namespace oracle

int main() {
    std::vector<int64_t> initial = {10, 20, 30, 40, 50};
    oracle::ArrayOracle orc(initial);

    auto res1 = orc.range_query(1, 3);
    std::cout << "Query [1, 3] -> Min: " << res1.min_val
              << ", Max: " << res1.max_val
              << ", Sum: " << res1.sum_mod << "\n";

    orc.range_update(1, 3, -1, 5);
    auto res2 = orc.range_query(1, 3);
    std::cout << "After Update (a=-1, b=5) on [1, 3] -> Min: " << res2.min_val
              << ", Max: " << res2.max_val
              << ", SumMod: " << res2.sum_mod << "\n";

    std::cout << "Oracle self-test passed successfully!\n";
    return 0;
}
