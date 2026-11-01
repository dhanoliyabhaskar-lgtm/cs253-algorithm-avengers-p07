# Common Interface and Invariants

**Author:** Architect (Bhaskar Dhanoliya)  
**Project:** 07 — Lazy propagation and what composes  

---

## 1. Core Data Types and Constants

* **Modulo Constant:** `MOD = 998244353`
* **Affine Tag:** `(a, b)` representing transformation $x \to a \cdot x + b$. Identity tag is `(1, 0)`.
* **Aggregates:**
  * `min_val`: Exact for $a \in \{-1, 0, 1\}$.
  * `max_val`: Exact for $a \in \{-1, 0, 1\}$.
  * `sum_val`: Maintained modulo $998244353$ for arbitrary $a, b$.

---

## 2. Common Interface Specification

Structure A (`SegmentTree`), Structure B (`SqrtDecomposition`), and `OracleArray` implement the following operations:

```cpp
class RangeStructure {
public:
    virtual ~RangeStructure() = default;

    virtual void build(const std::vector<long long>& data) = 0;
    virtual void update_range(int l, int r, long long a, long long b) = 0;

    virtual long long query_min(int l, int r) = 0;
    virtual long long query_max(int l, int r) = 0;
    virtual long long query_sum(int l, int r) = 0;

    virtual void reset_counters() = 0;
    virtual long long get_operation_cost() const = 0;
};

// 1. Tag Composition Order (Non-Commutative)
// Applying (a1, b1) followed by (a2, b2) gives:
// a_new = (a2 * a1) % MOD
// b_new = (a2 * b1 + b2) % MOD
inline AffineTag compose_tags(const AffineTag& tag2, const AffineTag& tag1) {
    long long new_a = (tag2.a * tag1.a) % MOD;
    long long new_b = ((tag2.a * tag1.b) % MOD + tag2.b) % MOD;
    return {new_a, new_b};
}

// 2. Min/Max Reversal Under Gain
// For a ==  1: min' = min + b,         max' = max + b
// For a == -1: min' = -1 * max + b,    max' = -1 * min + b
// For a ==  0: min' = b,               max' = b
inline void apply_min_max_tag(long long& min_val, long long& max_val, long long a, long long b) {
    if (a == 1) {
        min_val += b;
        max_val += b;
    } else if (a == -1) {
        long long old_min = min_val;
        long long old_max = max_val;
        min_val = -old_max + b;
        max_val = -old_min + b;
    } else if (a == 0) {
        min_val = b;
        max_val = b;
    }
}

// 3. Sum Maintenance
// sum' = (a * sum + b * length) % MOD
inline long long apply_sum_tag(long long sum_val, int len, long long a, long long b) {
    return ((a * sum_val) % MOD + (b * len) % MOD) % MOD;
}

// 4. Segment Tree Push-down Invariant
// Node aggregates reflect its pending tag; tag clears to identity (1, 0) after push to children.
virtual void push_down(int node, int l, int r) = 0;

// 5. Sqrt Block Consistency Invariant
// Full block updates take O(1) tag composition; partial blocks push tags before recomputing aggregates.
virtual void update_block(int block_idx, long long a, long long b) = 0;