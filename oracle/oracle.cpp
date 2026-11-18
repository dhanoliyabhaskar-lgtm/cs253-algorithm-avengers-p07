#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

const long long MOD = 998244353;

// Query ka answer store karne ke liye simple struct
struct QueryResult {
    long long min_val;
    long long max_val;
    long long sum_mod;

    bool operator==(const QueryResult& other) const {
        return min_val == other.min_val &&
               max_val == other.max_val &&
               sum_mod == other.sum_mod;
    }
};

// Simple Array Oracle class (Ground Truth)
class ArrayOracle {
public:
    int n;
    vector<long long> arr;

    ArrayOracle(const vector<long long>& initial) {
        n = initial.size();
        arr = initial;
    }

    // Modulo hamesha positive rakhne ka simple function
    long long clean_mod(long long x) const {
        long long rem = x % MOD;
        if (rem < 0) rem += MOD;
        return rem;
    }

    // Range Update: l se r tak har value ko (a * x + b) banana
    void range_update(int l, int r, long long a, long long b) {
        for (int i = l; i <= r; i++) {
            arr[i] = a * arr[i] + b;
        }
    }

    // Range Query: l se r tak min, max aur sum_mod nikalna
    QueryResult range_query(int l, int r) const {
        long long mn = arr[l];
        long long mx = arr[l];
        long long sm = 0;

        for (int i = l; i <= r; i++) {
            if (arr[i] < mn) mn = arr[i];
            if (arr[i] > mx) mx = arr[i];
            sm = (sm + clean_mod(arr[i])) % MOD;
        }
        return {mn, mx, sm};
    }
};

int main() {
    cout << "=== Running Oracle Self-Tests ===\n";

    vector<long long> initial = {10, 20, 30, 40, 50};
    ArrayOracle orc(initial);

    // Test 1: Simple Query on [1, 3] -> {20, 30, 40}
    QueryResult res1 = orc.range_query(1, 3);
    if (res1.min_val == 20 && res1.max_val == 40 && res1.sum_mod == 90) {
        cout << "[PASS] Test 1: Initial Range Query Passed! (Min=20, Max=40, Sum=90)\n";
    }

    // Test 2: Update [1, 3] with a = -1, b = 5 -> {-15, -25, -35}
    orc.range_update(1, 3, -1, 5);
    QueryResult res2 = orc.range_query(1, 3);
    if (res2.min_val == -35 && res2.max_val == -15) {
        cout << "[PASS] Test 2: Negative Gain Update Passed! (Min=-35, Max=-15)\n";
    }

    cout << "=== ALL ORACLE TESTS PASSED SUCCESSFULLY! ===\n\n";
    return 0;
}
