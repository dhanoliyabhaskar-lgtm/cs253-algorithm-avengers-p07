// app/simulator.cpp
// Use: simulator [file] [--verify]
// File format: har case = "n q" / n values / q ops (U l r g o | Q l r | S l r)
// '#' wali lines aur blank lines skip. "# EXPECTED a b c" = upar wali query ka sahi answer.
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <stdexcept>
#include "range_structure.hpp"

using namespace std;
using rs::QueryResult;

struct Op {
    char type; int l, r; 
    long long g, o;
    vector<long long> exp;               // EXPECTED answer (khali ho sakta hai)
};
struct Case { 
    vector<long long> init; 
    vector<Op> ops; 
};

// ---------- file padhna ----------
vector<Case> read_cases(istream& in) {
    vector<Case> cases;
    string line;
    int state = 0, n = 0, q = 0, cnt = 0, lineno = 0;   // 0: "n q" , 1: array , 2: ops
    while (getline(in, line)) {
        lineno++;
        if (lineno == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);  // UTF-8 BOM hatao
        if (!line.empty() && line.back() == '\r') line.pop_back();                      // Windows CRLF
        size_t p = line.find_first_not_of(" \t");
        if (p == string::npos) continue;                      // blank line
        if (line.compare(p, 10, "# EXPECTED") == 0) {         // upar wali query ka answer
            if (!cases.empty() && !cases.back().ops.empty()) {
                istringstream ss(line.substr(p + 10));
                long long x;
                while (ss >> x) cases.back().ops.back().exp.push_back(x);
            }
            continue;
        }
        if (line[p] == '#') continue;                         // comment
        istringstream ss(line);
        if (state == 0) {
            if (!(ss >> n >> q) || n <= 0 || q < 0)
                throw runtime_error("line " + to_string(lineno) + ": yahan 'n q' hona chahiye, mila: " + line);
            cases.push_back(Case()); cnt = 0;
            state = 1;
        } else if (state == 1) {
            cases.back().init.resize(n);
            for (auto& x : cases.back().init)
                if (!(ss >> x)) throw runtime_error("line " + to_string(lineno) + ": array me " + to_string(n) + " numbers hone chahiye");
            state = (q > 0) ? 2 : 0;
        } else {
            Op op;
            if (!(ss >> op.type >> op.l >> op.r) || (op.type != 'U' && op.type != 'Q' && op.type != 'S'))
                throw runtime_error("line " + to_string(lineno) + ": galat operation: " + line);
            if (op.type == 'U' && !(ss >> op.g >> op.o))
                throw runtime_error("line " + to_string(lineno) + ": U ke liye 'U l r g o' chahiye");
            if (op.l < 0 || op.r < op.l || op.r >= (int)cases.back().init.size())
                throw runtime_error("line " + to_string(lineno) + ": range bahar hai: " + line);
            cases.back().ops.push_back(op);
            if (++cnt == q) state = 0;
        }
    }
    return cases;
}

// ---------- structure chalana ----------
double run(rs::RangeStructure& s, const Case& c, vector<QueryResult>& out) {
    s.build(c.init);                                 // build ka time nahi ginte
    auto t0 = chrono::high_resolution_clock::now();
    for (const Op& op : c.ops) {
        if (op.type == 'U') s.range_update(op.l, op.r, op.g, op.o);
        else out.push_back(s.range_query(op.l, op.r));   // Q aur S dono me query
    }
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration<double>(t1 - t0).count();
}

// ---------- jawab check karna ----------
// 'S' query me sirf sum dekhte hain, 'Q' me teeno
bool same(char type, const QueryResult& a, const QueryResult& b) {
    return type == 'S' ? a.sum_mod == b.sum_mod : a == b;
}
int wrong_vs_expected(const Case& c, const vector<QueryResult>& res) {
    int bad = 0, k = 0;
    for (const Op& op : c.ops) {
        if (op.type == 'U') continue;
        const QueryResult& r = res[k++];
        if (op.exp.empty()) continue;
        QueryResult e = {0, 0, op.exp.back()};            // S: sirf sum
        if (op.type == 'Q') e = {op.exp[0], op.exp[1], op.exp[2]};
        if (!same(op.type, r, e)) bad++;
    }
    return bad;
}
int wrong_vs_oracle(const Case& c, const vector<QueryResult>& res, const vector<QueryResult>& orc) {
    int bad = 0, k = 0;
    for (const Op& op : c.ops) {
        if (op.type == 'U') continue;
        if (!same(op.type, res[k], orc[k])) bad++;
        k++;
    }
    return bad;
}
string status(int bad) { 
    return bad == 0 ? "OK" : to_string(bad) + " galat"; 
}

int run_main(int argc, char** argv) {
    bool verify = false;
    string path;
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--verify") verify = true;
        else path = argv[i];
    }

    ifstream file;
    istream* in = &cin;
    if (!path.empty()) {
        file.open(path);
        if (!file) { cout << "File nahi khuli: " << path << "\n"; return 1; }
        in = &file;
    }
    vector<Case> cases = read_cases(*in);
    cout << "[simulator v2] " << cases.size() << " cases padhe" << (path.empty() ? "" : " from " + path) << "\n\n";

    double totA = 0, totB = 0;
    long long totOps = 0;
    int failed = 0;
    for (size_t i = 0; i < cases.size(); i++) {
        const Case& c = cases[i];
        rs::A_Adapter A;
        rs::B_Adapter B;
        vector<QueryResult> ra, rb, ro;
        totA += run(A, c, ra);
        totB += run(B, c, rb);
        totOps += c.ops.size();

        int ba = wrong_vs_expected(c, ra), bb = wrong_vs_expected(c, rb);
        cout << "Case " << i + 1 << " | A: " << status(ba) << " | B: " << status(bb);
        if (verify) {
            rs::Oracle_Adapter O;
            run(O, c, ro);
            int bo = wrong_vs_expected(c, ro);
            ba += wrong_vs_oracle(c, ra, ro);
            bb += wrong_vs_oracle(c, rb, ro);
            cout << " | Oracle: " << status(bo);
            failed += bo;
        }
        failed += ba + bb;
        cout << "\n";
    }
    cout << "\nCases: " << cases.size() << ", operations: " << totOps << "\n";
    cout << "Structure A (Segment Tree) : " << totA << " s, " << totOps / totA << " ops/sec\n";
    cout << "Structure B (Sqrt Decomp)  : " << totB << " s, " << totOps / totB << " ops/sec\n";
    cout << (failed == 0 ? "ALL OK\n" : "KUCH CASES FAIL HUE\n");
    return failed == 0 ? 0 : 2;
}

int main(int argc, char** argv) {
    try { return run_main(argc, argv); }
    catch (const exception& e) { cout << "ERROR: " << e.what() << "\n"; return 1; }
}
