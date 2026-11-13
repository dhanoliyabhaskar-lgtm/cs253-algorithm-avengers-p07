// app/simulator.cpp - Calibration simulator for CS253 Lab Project 7
// Usage: simulator [input_file] [--verify] [--print]
// Input: N / N sensor values / Q / then Q lines: "U l r g o"  or  "Q l r"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "range_structure.hpp"

using namespace rs;

struct Op {
    bool is_update;
    int l, r;
    int64_t g, o;
};

struct RunResult {
    double seconds = 0.0;
    std::vector<QueryResult> answers;
};

static RunResult run(RangeStructure& s, const std::vector<int64_t>& init, const std::vector<Op>& ops) {
    RunResult res;
    s.build(init);
    s.reset_counters();
    res.answers.reserve(ops.size());
    auto t0 = std::chrono::high_resolution_clock::now();
    for (const Op& op : ops) {
        if (op.is_update) s.range_update(op.l, op.r, op.g, op.o);
        else res.answers.push_back(s.range_query(op.l, op.r));
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    res.seconds = std::chrono::duration<double>(t1 - t0).count();
    return res;
}

static bool read_input(std::istream& in, std::vector<int64_t>& init, std::vector<Op>& ops, std::string& err) {
    int n = 0, q = 0;
    if (!(in >> n) || n <= 0) { err = "invalid or missing N"; return false; }
    init.resize(n);
    for (int i = 0; i < n; ++i)
        if (!(in >> init[i])) { err = "not enough initial values"; return false; }
    if (!(in >> q) || q < 0) { err = "invalid or missing Q"; return false; }
    ops.reserve(q);
    for (int i = 0; i < q; ++i) {
        std::string t;
        Op op{};
        if (!(in >> t)) { err = "missing operation #" + std::to_string(i + 1); return false; }
        if (t == "U" || t == "u") {
            op.is_update = true;
            if (!(in >> op.l >> op.r >> op.g >> op.o)) { err = "bad update #" + std::to_string(i + 1); return false; }
        } else if (t == "Q" || t == "q") {
            op.is_update = false;
            if (!(in >> op.l >> op.r)) { err = "bad query #" + std::to_string(i + 1); return false; }
        } else { err = "unknown op '" + t + "' at #" + std::to_string(i + 1); return false; }
        if (op.l < 0 || op.r < op.l || op.r >= n) { err = "range out of bounds at op #" + std::to_string(i + 1); return false; }
        ops.push_back(op);
    }
    return true;
}

static void report(const RangeStructure& s, const RunResult& r, size_t q) {
    double tput = r.seconds > 0 ? static_cast<double>(q) / r.seconds : 0.0;
    std::cout << std::left << std::setw(34) << s.name()
              << " time: " << std::fixed << std::setprecision(6) << r.seconds << " s"
              << " | throughput: " << std::setprecision(0) << tput << " ops/sec"
              << " | cost: " << s.operation_cost() << "\n";
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string path;
    bool verify = false, print = false;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--verify")) verify = true;
        else if (!std::strcmp(argv[i], "--print")) print = true;
        else path = argv[i];
    }

    std::vector<int64_t> init;
    std::vector<Op> ops;
    std::string err;
    bool ok;
    if (!path.empty()) {
        std::ifstream f(path);
        if (!f) { std::cerr << "Error: cannot open '" << path << "'\n"; return 1; }
        ok = read_input(f, init, ops, err);
    } else {
        ok = read_input(std::cin, init, ops, err);
    }
    if (!ok) { std::cerr << "Input error: " << err << "\n"; return 1; }

    size_t num_q = 0;
    for (const Op& o : ops) num_q += !o.is_update;
    std::cout << "N = " << init.size() << ", operations = " << ops.size()
              << " (" << (ops.size() - num_q) << " updates, " << num_q << " queries)\n";

    SegmentTreeStructure A;
    SqrtDecompStructure B;
    RunResult ra = run(A, init, ops);
    RunResult rb = run(B, init, ops);
    report(A, ra, ops.size());
    report(B, rb, ops.size());

    if (print) {
        for (size_t i = 0; i < ra.answers.size(); ++i)
            std::cout << "Q" << i << ": " << ra.answers[i].min_val << " " << ra.answers[i].max_val
                      << " " << ra.answers[i].sum_mod << "\n";
    }

    int rc = 0;
    if (verify) {
        OracleStructure O;
        RunResult ro = run(O, init, ops);
        size_t bad_a = 0, bad_b = 0, first_a = 0, first_b = 0;
        for (size_t i = 0; i < ro.answers.size(); ++i) {
            if (ra.answers[i] != ro.answers[i] && bad_a++ == 0) first_a = i;
            if (rb.answers[i] != ro.answers[i] && bad_b++ == 0) first_b = i;
        }
        std::cout << "Oracle time: " << std::fixed << std::setprecision(6) << ro.seconds << " s\n";
        std::cout << "Verify A vs Oracle: " << (bad_a ? "MISMATCH" : "OK") << " (" << bad_a << " / " << ro.answers.size() << " bad)\n";
        std::cout << "Verify B vs Oracle: " << (bad_b ? "MISMATCH" : "OK") << " (" << bad_b << " / " << ro.answers.size() << " bad)\n";
        auto show = [&](const char* nm, const QueryResult& x, const QueryResult& y, size_t idx) {
            std::cout << "  first " << nm << " mismatch at query #" << idx << ": got {" << x.min_val << "," << x.max_val
                      << "," << x.sum_mod << "} expected {" << y.min_val << "," << y.max_val << "," << y.sum_mod << "}\n";
        };
        if (bad_a) { show("A", ra.answers[first_a], ro.answers[first_a], first_a); rc = 2; }
        if (bad_b) { show("B", rb.answers[first_b], ro.answers[first_b], first_b); rc = 2; }
        if (!rc) std::cout << "100% agreement.\n";
    }
    return rc;
}