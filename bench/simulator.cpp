
#include <bits/stdc++.h>
using namespace std;

#include "structA/segment_tree.cpp"
#include "structB/sqrt_decomp.cpp"

struct Op { bool update; int l, r; long long a, b; };

static vector<Op> make_ops(int n, int q, int update_percent, uint64_t seed) {
    mt19937_64 rng(seed);
    uniform_int_distribution<int> pos(0, n - 1), coeff(-10, 10), pick(0, 99);
    const int gains[] = {-1, 0, 1};
    vector<Op> ops;
    ops.reserve(q);
    for (int i = 0; i < q; ++i) {
        int l = pos(rng), r = pos(rng);
        if (l > r) swap(l, r);
        if (pick(rng) < update_percent) {
            long long a = gains[rng() % 3];
            long long b = coeff(rng);
            ops.push_back({true, l, r, a, b});
        } else {
            ops.push_back({false, l, r, 1, 0});
        }
    }
    return ops;
}

static vector<long long> make_initial(int n, uint64_t seed) {
    mt19937_64 rng(seed);
    uniform_int_distribution<int> val(-1000, 1000);
    vector<long long> a(n);
    for (auto &x : a) x = val(rng);
    return a;
}

template<class Runner>
static pair<double,long long> timed_run(Runner run) {
    auto start = chrono::steady_clock::now();
    long long checksum = run();
    auto stop = chrono::steady_clock::now();
    double ms = chrono::duration<double, milli>(stop - start).count();
    return {ms, checksum};
}

int main(int argc, char** argv) {
    vector<int> sizes = {1000, 10000, 100000};
    vector<int> query_counts = {1000, 5000, 10000};
    struct Workload { string name; int update_percent; };
    vector<Workload> workloads = {
        {"update_heavy", 80}, {"query_heavy", 20}, {"mixed", 50}
    };

    cout << "structure,N,Q,workload,update_percent,build_ms,execution_ms,operation_cost,checksum\n";
    for (int n : sizes) {
        auto initial = make_initial(n, 20261118ULL + n);
        for (int q : query_counts) {
            for (const auto& w : workloads) {
                auto ops = make_ops(n, q, w.update_percent,
                                    987654321ULL + n * 17ULL + q * 3ULL + w.update_percent);

                // Structure A: Lazy Segment Tree
                SegmentTree st;
                auto build_a_start = chrono::steady_clock::now();
                st.build(initial);
                auto build_a_end = chrono::steady_clock::now();
                double build_a = chrono::duration<double, milli>(build_a_end-build_a_start).count();
                st.reset_counters();
                auto result_a = timed_run([&]() {
                    long long checksum = 0;
                    for (const auto& op : ops) {
                        if (op.update) st.update_range(op.l, op.r, op.a, op.b);
                        else {
                            checksum ^= st.query_min(op.l, op.r);
                            checksum += st.query_max(op.l, op.r);
                            checksum += st.query_sum(op.l, op.r);
                        }
                    }
                    return checksum;
                });
                cout << "SegmentTree," << n << "," << q << "," << w.name << ","
                     << w.update_percent << "," << fixed << setprecision(6) << build_a
                     << "," << result_a.first << "," << st.get_operation_cost()
                     << "," << result_a.second << "\n";

                // Structure B: Square Root Decomposition
                auto build_b_start = chrono::steady_clock::now();
                structB sq(n, initial);
                auto build_b_end = chrono::steady_clock::now();
                double build_b = chrono::duration<double, milli>(build_b_end-build_b_start).count();
                sq.ops = 0;
                auto result_b = timed_run([&]() {
                    long long checksum = 0;
                    for (const auto& op : ops) {
                        if (op.update) sq.update(op.l, op.r, op.a, op.b);
                        else {
                            checksum ^= sq.find_min(op.l, op.r);
                            checksum += sq.find_max(op.l, op.r);
                            checksum += sq.find_sum(op.l, op.r);
                        }
                    }
                    return checksum;
                });
                cout << "SqrtDecomposition," << n << "," << q << "," << w.name << ","
                     << w.update_percent << "," << fixed << setprecision(6) << build_b
                     << "," << result_b.first << "," << sq.get_cost()
                     << "," << result_b.second << "\n";
            }
        }
    }
    return 0;
}
