// include/range_structure.hpp
// Teammates ki files ko BINA COPY/EDIT kiye #include karta hai.
// Har file apne namespace me aati hai, taaki MOD / RangeStructure / main() ke naam na takrayein.
#pragma once
#include <bits/stdc++.h>   // pehle std headers (neeche wali files ke #include ab ignore ho jaayenge)

#define main main_A
namespace teamA {
#include "../src/structA/1.cpp"           // Structure A: SegmentTree
}
#undef main

#define main main_B
namespace teamB {
#include "../src/structB/sqrt_decomp.cpp" // Structure B: structB
}
#undef main

#define main main_O
namespace teamO {
#include "../oracle/oracle.cpp"           // Oracle: oracle::ArrayOracle
}
#undef main

// ================= Yahan se tumhara kaam =================
namespace rs {

struct QueryResult {
    long long min_val, max_val, sum_mod;
    bool operator==(const QueryResult& o) const {
        return min_val == o.min_val && max_val == o.max_val && sum_mod == o.sum_mod;
    }
};

// Common interface
class RangeStructure {
public:
    virtual ~RangeStructure() {}
    virtual void build(const std::vector<long long>& d) = 0;
    virtual void range_update(int l, int r, long long g, long long o) = 0;
    virtual QueryResult range_query(int l, int r) = 0;
};

// Adapter: Structure A
class A_Adapter : public RangeStructure {
    teamA::SegmentTree t;
public:
    void build(const std::vector<long long>& d) { t.build(d); }
    void range_update(int l, int r, long long g, long long o) { t.update_range(l, r, g, o); }
    QueryResult range_query(int l, int r) {
        return {t.query_min(l, r), t.query_max(l, r), t.query_sum(l, r)};
    }
};

// Adapter: Structure B
class B_Adapter : public RangeStructure {
    std::unique_ptr<teamB::structB> b;
public:
    void build(const std::vector<long long>& d) {
        std::vector<long long> tmp = d;   // structB ka constructor non-const reference leta hai
        b.reset(new teamB::structB((int)tmp.size(), tmp));
    }
    void range_update(int l, int r, long long g, long long o) { b->update(l, r, g, o); }
    QueryResult range_query(int l, int r) {
        return {b->find_min(l, r), b->find_max(l, r), b->find_sum(l, r)};
    }
};

// Adapter: Oracle
class Oracle_Adapter : public RangeStructure {
    std::unique_ptr<teamO::oracle::ArrayOracle> o;
public:
    void build(const std::vector<long long>& d) { o.reset(new teamO::oracle::ArrayOracle(std::vector<int64_t>(d.begin(), d.end()))); }
    void range_update(int l, int r, long long g, long long v) { o->range_update(l, r, g, v); }
    QueryResult range_query(int l, int r) {
        auto q = o->range_query(l, r);
        return {q.min_val, q.max_val, q.sum_mod};
    }
};

}  // namespace rs