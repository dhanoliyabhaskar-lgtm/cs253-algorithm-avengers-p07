// include/range_structure.hpp
// CS253 Lab Project 7 - "Lazy propagation and what composes"
// Unified interface + adapters for: Structure A (lazy segment tree),
// Structure B (sqrt decomposition) and the brute-force ArrayOracle.
#pragma once

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rs {

constexpr int64_t MOD = 998244353LL;

struct QueryResult {
    int64_t min_val;
    int64_t max_val;
    int64_t sum_mod;  // always in [0, MOD-1]

    bool operator==(const QueryResult& o) const {
        return min_val == o.min_val && max_val == o.max_val && sum_mod == o.sum_mod;
    }
    bool operator!=(const QueryResult& o) const { return !(*this == o); }
};

// ---------- helpers ----------
inline int64_t normalise(int64_t x) {
    int64_t r = x % MOD;
    return r < 0 ? r + MOD : r;
}
// Wrap-around (two's complement) arithmetic: avoids signed-overflow UB so that
// all three implementations behave identically even for huge gains.
inline int64_t wmul(int64_t a, int64_t b) {
    return static_cast<int64_t>(static_cast<uint64_t>(a) * static_cast<uint64_t>(b));
}
inline int64_t wadd(int64_t a, int64_t b) {
    return static_cast<int64_t>(static_cast<uint64_t>(a) + static_cast<uint64_t>(b));
}

// ---------- abstract interface ----------
class RangeStructure {
public:
    virtual ~RangeStructure() = default;
    virtual std::string name() const = 0;
    virtual void build(const std::vector<int64_t>& data) = 0;
    // a[i] = a*a[i] + b for every i in [l, r] (0-based, inclusive)
    virtual void range_update(int l, int r, int64_t a, int64_t b) = 0;
    virtual QueryResult range_query(int l, int r) = 0;  // non-const: lazy structures push down
    virtual void reset_counters() {}
    virtual int64_t operation_cost() const { return 0; }
};

// =====================================================================
// Structure A core: lazy segment tree
// =====================================================================
namespace detail {

struct AffineTag { int64_t a, b; };  // x -> a*x + b  (mod MOD)

// first tag1, then tag2
inline AffineTag compose_tags(const AffineTag& tag2, const AffineTag& tag1) {
    return { (tag2.a * tag1.a) % MOD, ((tag2.a * tag1.b) % MOD + tag2.b) % MOD };
}

// FIX (A): original only handled a in {-1,0,1} and silently ignored any other
// gain. Now handles every gain: a>=0 keeps order, a<0 swaps min and max.
inline void apply_min_max_tag(int64_t& mn, int64_t& mx, int64_t a, int64_t b) {
    if (a >= 0) {
        mn = wadd(wmul(mn, a), b);
        mx = wadd(wmul(mx, a), b);
    } else {
        int64_t nmn = wadd(wmul(mx, a), b);
        int64_t nmx = wadd(wmul(mn, a), b);
        mn = nmn;
        mx = nmx;
    }
}

inline int64_t apply_sum_tag(int64_t sum_val, int64_t len, int64_t a, int64_t b) {
    return ((a * sum_val) % MOD + (b * (len % MOD)) % MOD) % MOD;
}

class SegmentTreeCore {
    struct Node {
        int64_t sum_val = 0, min_val = 0, max_val = 0;
        int64_t sum_a = 1, sum_b = 0;        // lazy tag for the modular sum
        int64_t minmax_a = 1, minmax_b = 0;  // lazy tag for exact min/max
    };
    std::vector<Node> tree;
    int n = 0;
    int64_t operation_cost = 0;

    void build_tree(int node, int l, int r, const std::vector<int64_t>& d) {
        if (l == r) {
            tree[node].sum_val = normalise(d[l]);
            tree[node].min_val = tree[node].max_val = d[l];
            return;
        }
        int mid = (l + r) / 2;
        build_tree(2 * node, l, mid, d);
        build_tree(2 * node + 1, mid + 1, r, d);
        pull(node);
    }
    void pull(int node) {
        Node& L = tree[2 * node];
        Node& R = tree[2 * node + 1];
        tree[node].sum_val = (L.sum_val + R.sum_val) % MOD;
        tree[node].min_val = std::min(L.min_val, R.min_val);
        tree[node].max_val = std::max(L.max_val, R.max_val);
    }
    void apply_node(int node, int l, int r, int64_t a_sum, int64_t b_sum,
                    int64_t a_mm, int64_t b_mm) {
        Node& nd = tree[node];
        nd.sum_val = apply_sum_tag(nd.sum_val, r - l + 1, a_sum, b_sum);
        apply_min_max_tag(nd.min_val, nd.max_val, a_mm, b_mm);

        AffineTag c = compose_tags({a_sum, b_sum}, {nd.sum_a, nd.sum_b});
        nd.sum_a = c.a;
        nd.sum_b = c.b;

        // FIX (A): wrap-safe composition (no signed-overflow UB)
        int64_t old_a = nd.minmax_a, old_b = nd.minmax_b;
        nd.minmax_a = wmul(a_mm, old_a);
        nd.minmax_b = wadd(wmul(a_mm, old_b), b_mm);
    }
    void push_down(int node, int l, int r) {
        if (l == r) return;
        Node& cur = tree[node];
        if (cur.sum_a == 1 && cur.sum_b == 0 && cur.minmax_a == 1 && cur.minmax_b == 0) return;
        int mid = (l + r) / 2;
        apply_node(2 * node, l, mid, cur.sum_a, cur.sum_b, cur.minmax_a, cur.minmax_b);
        apply_node(2 * node + 1, mid + 1, r, cur.sum_a, cur.sum_b, cur.minmax_a, cur.minmax_b);
        cur.sum_a = 1; cur.sum_b = 0;
        cur.minmax_a = 1; cur.minmax_b = 0;
    }
    void update_tree(int node, int l, int r, int ql, int qr, int64_t a, int64_t b) {
        if (r < ql || l > qr) return;
        ++operation_cost;
        if (ql <= l && r <= qr) {
            apply_node(node, l, r, normalise(a), normalise(b), a, b);
            return;
        }
        push_down(node, l, r);
        int mid = (l + r) / 2;
        update_tree(2 * node, l, mid, ql, qr, a, b);
        update_tree(2 * node + 1, mid + 1, r, ql, qr, a, b);
        pull(node);
    }
    int64_t min_tree(int node, int l, int r, int ql, int qr) {
        if (r < ql || l > qr) return LLONG_MAX;
        ++operation_cost;
        if (ql <= l && r <= qr) return tree[node].min_val;
        push_down(node, l, r);
        int mid = (l + r) / 2;
        return std::min(min_tree(2 * node, l, mid, ql, qr),
                        min_tree(2 * node + 1, mid + 1, r, ql, qr));
    }
    int64_t max_tree(int node, int l, int r, int ql, int qr) {
        if (r < ql || l > qr) return LLONG_MIN;
        ++operation_cost;
        if (ql <= l && r <= qr) return tree[node].max_val;
        push_down(node, l, r);
        int mid = (l + r) / 2;
        return std::max(max_tree(2 * node, l, mid, ql, qr),
                        max_tree(2 * node + 1, mid + 1, r, ql, qr));
    }
    int64_t sum_tree(int node, int l, int r, int ql, int qr) {
        if (r < ql || l > qr) return 0;
        ++operation_cost;
        if (ql <= l && r <= qr) return tree[node].sum_val;
        push_down(node, l, r);
        int mid = (l + r) / 2;
        return (sum_tree(2 * node, l, mid, ql, qr) +
                sum_tree(2 * node + 1, mid + 1, r, ql, qr)) % MOD;
    }

public:
    void build(const std::vector<int64_t>& data) {
        n = static_cast<int>(data.size());
        tree.assign(4 * static_cast<size_t>(n) + 1, Node());
        operation_cost = 0;
        if (n > 0) build_tree(1, 0, n - 1, data);
    }
    void update_range(int l, int r, int64_t a, int64_t b) { update_tree(1, 0, n - 1, l, r, a, b); }
    int64_t query_min(int l, int r) { return min_tree(1, 0, n - 1, l, r); }
    int64_t query_max(int l, int r) { return max_tree(1, 0, n - 1, l, r); }
    int64_t query_sum(int l, int r) { return sum_tree(1, 0, n - 1, l, r); }
    void reset_counters() { operation_cost = 0; }
    int64_t get_operation_cost() const { return operation_cost; }
};

// =====================================================================
// Structure B core: sqrt decomposition (bugs fixed)
// =====================================================================
struct BlockState {
    int64_t mn = 0, mx = 0, sum = 0;
    int64_t gain = 1, offset = 0;
};

class SqrtCore {
public:
    int n = 0;
    int size = 1;
    int64_t ops = 0;
    std::vector<int64_t> sensors;
    std::vector<BlockState> blocks;

    explicit SqrtCore(const std::vector<int64_t>& initial) {
        sensors = initial;
        n = static_cast<int>(initial.size());
        size = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n)))));
        // FIX (B): number of blocks is ceil(n/size), not `size`
        blocks.assign(static_cast<size_t>((n + size - 1) / size), BlockState());
        for (int b = 0; b < static_cast<int>(blocks.size()); ++b) rebuild(b);
        ops = 0;
    }

    void push(int b) {
        if (blocks[b].gain == 1 && blocks[b].offset == 0) return;
        ops += size;
        int start = b * size, end = std::min((b + 1) * size, n);
        for (int i = start; i < end; ++i)
            sensors[i] = wadd(wmul(blocks[b].gain, sensors[i]), blocks[b].offset);
        blocks[b].gain = 1;
        blocks[b].offset = 0;
    }

    void rebuild(int b) {
        int start = b * size, end = std::min((b + 1) * size, n);
        ops += 3 * size;
        BlockState& w = blocks[b];
        w.mn = w.mx = sensors[start];
        w.sum = normalise(sensors[start]);  // FIX (B): negative-modulo normalisation
        for (int i = start + 1; i < end; ++i) {
            w.mn = std::min(w.mn, sensors[i]);
            w.mx = std::max(w.mx, sensors[i]);
            w.sum = (w.sum + normalise(sensors[i])) % MOD;
        }
    }

    void comp_upd(int b, int64_t g, int64_t o) {
        BlockState& w = blocks[b];
        ops += 3;
        int64_t len_b = std::min((b + 1) * size, n) - b * size;  // FIX (B): real block length
        int64_t nmn = wadd(wmul(w.mn, g), o);
        int64_t nmx = wadd(wmul(w.mx, g), o);
        w.mn = nmn; w.mx = nmx;
        if (g < 0) std::swap(w.mn, w.mx);
        // FIX (B): both terms are normalised so the sum never goes negative
        w.sum = ((w.sum * normalise(g)) % MOD + (len_b % MOD) * normalise(o) % MOD) % MOD;
        w.gain = wmul(g, w.gain);
        w.offset = wadd(wmul(g, w.offset), o);
    }

    void update(int start, int end, int64_t g, int64_t o) {
        int b1 = start / size, b2 = end / size;
        for (int b = b1; b <= b2; ++b) {
            int l = b * size, r = std::min((b + 1) * size - 1, n - 1);
            if (l >= start && r <= end) {
                comp_upd(b, g, o);
            } else {
                push(b);
                int s = std::max(start, l), e = std::min(end, r);
                for (int i = s; i <= e; ++i) {
                    ++ops;
                    sensors[i] = wadd(wmul(sensors[i], g), o);
                }
                rebuild(b);
            }
        }
    }

    int64_t find_min(int start, int end) {
        int b1 = start / size, b2 = end / size;
        int64_t res = LLONG_MAX;
        for (int b = b1; b <= b2; ++b) {
            int l = b * size, r = std::min((b + 1) * size - 1, n - 1);
            if (l >= start && r <= end) { ++ops; res = std::min(res, blocks[b].mn); }
            else {
                push(b);
                for (int i = std::max(start, l); i <= std::min(end, r); ++i) { ++ops; res = std::min(res, sensors[i]); }
            }
        }
        return res;
    }
    int64_t find_max(int start, int end) {
        int b1 = start / size, b2 = end / size;
        int64_t res = LLONG_MIN;
        for (int b = b1; b <= b2; ++b) {
            int l = b * size, r = std::min((b + 1) * size - 1, n - 1);
            if (l >= start && r <= end) { ++ops; res = std::max(res, blocks[b].mx); }
            else {
                push(b);
                for (int i = std::max(start, l); i <= std::min(end, r); ++i) { ++ops; res = std::max(res, sensors[i]); }
            }
        }
        return res;
    }
    int64_t find_sum(int start, int end) {
        int b1 = start / size, b2 = end / size;
        int64_t total = 0;
        for (int b = b1; b <= b2; ++b) {
            int l = b * size, r = std::min((b + 1) * size - 1, n - 1);
            if (l >= start && r <= end) { ++ops; total = (total + blocks[b].sum) % MOD; }
            else {
                push(b);
                for (int i = std::max(start, l); i <= std::min(end, r); ++i) { ++ops; total = (total + normalise(sensors[i])) % MOD; }
            }
        }
        return total;
    }
    int64_t get_cost() const { return ops; }
};

}  // namespace detail

// =====================================================================
// Oracle (brute force reference)
// =====================================================================
class ArrayOracle {
    std::vector<int64_t> exact_data_;
    std::vector<int64_t> mod_data_;

    void validate_range(int l, int r) const {
        int n = static_cast<int>(exact_data_.size());
        if (l < 0 || r < l || r >= n) throw std::out_of_range("ArrayOracle: invalid range [l, r]");
    }

public:
    ArrayOracle() = default;
    explicit ArrayOracle(const std::vector<int64_t>& init) : exact_data_(init), mod_data_(init.size(), 0) {
        for (size_t i = 0; i < init.size(); ++i) mod_data_[i] = normalise(init[i]);
    }
    void range_update(int l, int r, int64_t a, int64_t b) {
        validate_range(l, r);
        int64_t a_mod = normalise(a), b_mod = normalise(b);
        for (int i = l; i <= r; ++i) {
            exact_data_[i] = wadd(wmul(a, exact_data_[i]), b);  // wrap-safe
            // __int128 prod = static_cast<__int128>(a_mod) * mod_data_[i] + b_mod;
            // mod_data_[i] = static_cast<int64_t>(prod % MOD);
            mod_data_[i] = (a_mod * mod_data_[i] + b_mod) % MOD;
        }
    }
    QueryResult range_query(int l, int r) const {
        validate_range(l, r);
        int64_t mn = exact_data_[l], mx = exact_data_[l], sum = 0;
        for (int i = l; i <= r; ++i) {
            mn = std::min(mn, exact_data_[i]);
            mx = std::max(mx, exact_data_[i]);
            sum += mod_data_[i];
            if (sum >= MOD) sum -= MOD;
        }
        return QueryResult{mn, mx, sum};
    }
};

// =====================================================================
// Adapters implementing RangeStructure
// =====================================================================
class SegmentTreeStructure : public RangeStructure {
    detail::SegmentTreeCore core_;
public:
    std::string name() const override { return "Structure A (Lazy Segment Tree)"; }
    void build(const std::vector<int64_t>& d) override { core_.build(d); }
    void range_update(int l, int r, int64_t a, int64_t b) override { core_.update_range(l, r, a, b); }
    QueryResult range_query(int l, int r) override {
        return QueryResult{core_.query_min(l, r), core_.query_max(l, r), core_.query_sum(l, r)};
    }
    void reset_counters() override { core_.reset_counters(); }
    int64_t operation_cost() const override { return core_.get_operation_cost(); }
};

class SqrtDecompStructure : public RangeStructure {
    std::unique_ptr<detail::SqrtCore> core_;
public:
    std::string name() const override { return "Structure B (Sqrt Decomposition)"; }
    void build(const std::vector<int64_t>& d) override { core_ = std::make_unique<detail::SqrtCore>(d); }
    void range_update(int l, int r, int64_t a, int64_t b) override { core_->update(l, r, a, b); }
    QueryResult range_query(int l, int r) override {
        return QueryResult{core_->find_min(l, r), core_->find_max(l, r), core_->find_sum(l, r)};
    }
    void reset_counters() override { if (core_) core_->ops = 0; }
    int64_t operation_cost() const override { return core_ ? core_->get_cost() : 0; }
};

class OracleStructure : public RangeStructure {
    ArrayOracle oracle_;
public:
    std::string name() const override { return "Oracle (brute force)"; }
    void build(const std::vector<int64_t>& d) override { oracle_ = ArrayOracle(d); }
    void range_update(int l, int r, int64_t a, int64_t b) override { oracle_.range_update(l, r, a, b); }
    QueryResult range_query(int l, int r) override { return oracle_.range_query(l, r); }
};

}  // namespace rs