#ifndef ORACLE_HPP
#define ORACLE_HPP

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

const long long MOD = 998244353;

struct QueryResult {
    long long min_val;
    long long max_val;
    long long sum_mod;

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
    char type; // 'U' for update, 'Q' for query
    int l, r;
    long long a, b;
};

class ArrayOracle {
public:
    int n;
    vector<long long> arr;

    ArrayOracle(const vector<long long>& initial) {
        n = initial.size();
        arr = initial;
    }

    long long clean_mod(long long x) const {
        long long rem = x % MOD;
        if (rem < 0) rem += MOD;
        return rem;
    }

    void range_update(int l, int r, long long a, long long b) {
        for (int i = l; i <= r; i++) {
            arr[i] = a * arr[i] + b;
        }
    }

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

#endif
