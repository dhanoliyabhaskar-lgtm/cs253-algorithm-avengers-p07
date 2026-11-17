// simulator.cpp  (chhota version)
// Use: simulator [file] [--verify]
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include "range_structure.hpp"

struct Op { char type; int l, r; ll g, o; };

// structure par saare ops chalao, queries ke answers out me daalo, time return karo
double run(RangeStructure& s, const vector<ll>& init, const vector<Op>& ops, vector<QueryResult>& out) {
    s.build(init);
    auto t0 = chrono::high_resolution_clock::now();
    for (const Op& op : ops) {
        if (op.type == 'U') s.range_update(op.l, op.r, op.g, op.o);
        else out.push_back(s.range_query(op.l, op.r));
    }
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration<double>(t1 - t0).count();
}

int main(int argc, char** argv) {
    bool verify = false;
    string path;
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--verify") verify = true;
        else path = argv[i];
    }

    ifstream file;
    istream* in = &cin;                       // file na mile to cin
    if (!path.empty()) {
        file.open(path);
        if (!file) { cout << "File nahi khuli: " << path << "\n"; return 1; }
        in = &file;
    }

    int n, q;
    *in >> n;
    vector<ll> init(n);
    for (ll& x : init) *in >> x;
    *in >> q;
    vector<Op> ops(q);
    for (Op& op : ops) {
        *in >> op.type >> op.l >> op.r;
        if (op.type == 'U') *in >> op.g >> op.o;
    }

    SegTree A;
    Sqrt B;
    vector<QueryResult> ra, rb;
    double ta = run(A, init, ops, ra);
    double tb = run(B, init, ops, rb);
    cout << "Segment Tree : " << ta << " s, " << q / ta << " ops/sec\n";
    cout << "Sqrt Decomp  : " << tb << " s, " << q / tb << " ops/sec\n";

    if (verify) {
        Oracle O;
        vector<QueryResult> ro;
        run(O, init, ops, ro);
        cout << "A vs Oracle  : " << (ra == ro ? "OK" : "MISMATCH") << "\n";
        cout << "B vs Oracle  : " << (rb == ro ? "OK" : "MISMATCH") << "\n";
    }
    return 0;
}