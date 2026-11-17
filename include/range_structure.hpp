// range_structure.hpp  (chhota version)
#pragma once
#include <vector>
#include <algorithm>
#include <climits>
#include <cmath>
using namespace std;
typedef long long ll;
const ll MOD = 998244353;

ll norm(ll x) { return ((x % MOD) + MOD) % MOD; }   // negative ko [0, MOD-1] me lao

struct QueryResult {
    ll min_val, max_val, sum_mod;
    bool operator==(const QueryResult& o) const {
        return min_val == o.min_val && max_val == o.max_val && sum_mod == o.sum_mod;
    }
};

// ---------- common interface ----------
class RangeStructure {
public:
    virtual ~RangeStructure() {}
    virtual void build(const vector<ll>& a) = 0;
    virtual void range_update(int l, int r, ll g, ll o) = 0;   // a[i] = g*a[i] + o
    virtual QueryResult range_query(int l, int r) = 0;
};

// ---------- Oracle: seedha brute force ----------
class Oracle : public RangeStructure {
    vector<ll> a;
public:
    void build(const vector<ll>& v) { a = v; }
    void range_update(int l, int r, ll g, ll o) {
        for (int i = l; i <= r; i++){
            a[i] = g * a[i] + o;
        }
    }
    QueryResult range_query(int l, int r) {
        QueryResult res = {LLONG_MAX, LLONG_MIN, 0};
        for (int i = l; i <= r; i++) {
            res.min_val = min(res.min_val, a[i]);
            res.max_val = max(res.max_val, a[i]);
            res.sum_mod = (res.sum_mod + norm(a[i])) % MOD;
        }
        return res;
    }
};

// ---------- Structure A: Lazy Segment Tree ----------
class SegTree : public RangeStructure {
    struct Node { ll mn = 0, mx = 0, sum = 0, g = 1, o = 0; };   // g,o = pending lazy tag
    vector<Node> t;
    int n;

    // node par (g,o) lagao
    void apply(int x, int len, ll g, ll o) {
        ll p = g * t[x].mn + o, q = g * t[x].mx + o;
        t[x].mn = min(p, q);                 // g negative ho to min/max khud swap ho jaate hain
        t[x].mx = max(p, q);
        t[x].sum = (norm(g) * t[x].sum + norm(o) * len) % MOD;
        t[x].g = g * t[x].g;                 // tag compose: pehle purana, phir naya
        t[x].o = g * t[x].o + o;
    }
    void push(int x, int l, int r) {         // lazy tag bachchon ko do
        if (t[x].g == 1 && t[x].o == 0) return;
        int m = (l + r) / 2;
        apply(2 * x, m - l + 1, t[x].g, t[x].o);
        apply(2 * x + 1, r - m, t[x].g, t[x].o);
        t[x].g = 1; t[x].o = 0;
    }
    void pull(int x) {                       // bachchon se parent banao
        t[x].mn = min(t[2 * x].mn, t[2 * x + 1].mn);
        t[x].mx = max(t[2 * x].mx, t[2 * x + 1].mx);
        t[x].sum = (t[2 * x].sum + t[2 * x + 1].sum) % MOD;
    }
    void build(int x, int l, int r, const vector<ll>& a) {
        if (l == r) { t[x].mn = t[x].mx = a[l]; t[x].sum = norm(a[l]); return; }
        int m = (l + r) / 2;
        build(2 * x, l, m, a); build(2 * x + 1, m + 1, r, a);
        pull(x);
    }
    void update(int x, int l, int r, int ql, int qr, ll g, ll o) {
        if (qr < l || r < ql) return;
        if (ql <= l && r <= qr) { apply(x, r - l + 1, g, o); return; }
        push(x, l, r);
        int m = (l + r) / 2;
        update(2 * x, l, m, ql, qr, g, o);
        update(2 * x + 1, m + 1, r, ql, qr, g, o);
        pull(x);
    }
    QueryResult query(int x, int l, int r, int ql, int qr) {
        if (qr < l || r < ql) return {LLONG_MAX, LLONG_MIN, 0};
        if (ql <= l && r <= qr) return {t[x].mn, t[x].mx, t[x].sum};
        push(x, l, r);
        int m = (l + r) / 2;
        QueryResult L = query(2 * x, l, m, ql, qr), R = query(2 * x + 1, m + 1, r, ql, qr);
        return {min(L.min_val, R.min_val), max(L.max_val, R.max_val), (L.sum_mod + R.sum_mod) % MOD};
    }
public:
    void build(const vector<ll>& a) {
        n = a.size();
        t.assign(4 * n, Node());
        build(1, 0, n - 1, a);
    }
    void range_update(int l, int r, ll g, ll o) { update(1, 0, n - 1, l, r, g, o); }
    QueryResult range_query(int l, int r) { return query(1, 0, n - 1, l, r); }
};

// ---------- Structure B: Sqrt Decomposition ----------
class Sqrt : public RangeStructure {
    int n, S;                                // S = block size
    vector<ll> a, mn, mx, sum, G, O;         // har block ka min, max, sum, lazy gain, lazy offset
    int lo(int b) { return b * S; }
    int hi(int b) { return min(n, (b + 1) * S) - 1; }   // aakhri block chhota ho sakta hai

    void push(int b) {                       // lazy ko asli array me daalo
        if (G[b] == 1 && O[b] == 0) return;
        for (int i = lo(b); i <= hi(b); i++) a[i] = G[b] * a[i] + O[b];
        G[b] = 1; O[b] = 0;
    }
    void rebuild(int b) {                    // block ke stats dubara nikalo
        mn[b] = LLONG_MAX; mx[b] = LLONG_MIN; sum[b] = 0;
        for (int i = lo(b); i <= hi(b); i++) {
            mn[b] = min(mn[b], a[i]);
            mx[b] = max(mx[b], a[i]);
            sum[b] = (sum[b] + norm(a[i])) % MOD;
        }
    }
public:
    void build(const vector<ll>& v) {
        a = v; n = a.size();
        S = max(1, (int)sqrt(n));
        int B = (n + S - 1) / S;             // blocks ki sahi ginti
        mn.assign(B, 0); mx.assign(B, 0); sum.assign(B, 0); G.assign(B, 1); O.assign(B, 0);
        for (int b = 0; b < B; b++) rebuild(b);
    }
    void range_update(int l, int r, ll g, ll o) {
        for (int b = l / S; b <= r / S; b++) {
            if (l <= lo(b) && hi(b) <= r) {              // poora block cover -> lazy
                int len = hi(b) - lo(b) + 1;             // asli length (aakhri block ke liye zaruri)
                ll p = g * mn[b] + o, q = g * mx[b] + o;
                mn[b] = min(p, q); mx[b] = max(p, q);
                sum[b] = (norm(g) * sum[b] + norm(o) * len) % MOD;
                G[b] = g * G[b];
                O[b] = g * O[b] + o;
            } else {                                      // aadha block -> element by element
                push(b);
                for (int i = max(l, lo(b)); i <= min(r, hi(b)); i++) a[i] = g * a[i] + o;
                rebuild(b);
            }
        }
    }
    QueryResult range_query(int l, int r) {
        QueryResult res = {LLONG_MAX, LLONG_MIN, 0};
        for (int b = l / S; b <= r / S; b++) {
            if (l <= lo(b) && hi(b) <= r) {              // poora block -> stored stats
                res.min_val = min(res.min_val, mn[b]);
                res.max_val = max(res.max_val, mx[b]);
                res.sum_mod = (res.sum_mod + sum[b]) % MOD;
            } else {                                      // aadha block -> elements dekho
                push(b);
                for (int i = max(l, lo(b)); i <= min(r, hi(b)); i++) {
                    res.min_val = min(res.min_val, a[i]);
                    res.max_val = max(res.max_val, a[i]);
                    res.sum_mod = (res.sum_mod + norm(a[i])) % MOD;
                }
            }
        }
        return res;
    }
};