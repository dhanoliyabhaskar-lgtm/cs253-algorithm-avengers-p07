
#include <bits/stdc++.h>
using namespace std;

const long long MOD = 998244353;

// Stores a transformation: x -> a*x + b
struct AffineTag {
    long long a, b;
};

// Combines two tags: first tag1, then tag2
AffineTag compose_tags(const AffineTag& tag2, const AffineTag& tag1) {
    long long new_a = (tag2.a * tag1.a) % MOD;
    long long new_b = ((tag2.a * tag1.b) % MOD + tag2.b) % MOD;

    return {new_a, new_b};
}

// Updates minimum and maximum
void apply_min_max_tag(long long& min_val, long long& max_val,
                       long long a, long long b) {

    if (a == 1) {
        min_val += b;
        max_val += b;
    }
    else if (a == -1) {
        long long old_min = min_val;
        long long old_max = max_val;

        min_val = -old_max + b;
        max_val = -old_min + b;
    }
    else if (a == 0) {
        min_val = b;
        max_val = b;
    }
}

// Updates sum
long long apply_sum_tag(long long sum_val, int len,
                        long long a, long long b) {

    return ((a * sum_val) % MOD +
            (b * len) % MOD) % MOD;
}

// Converts negative values into positive modulo values
long long normalise(long long x) {
    return (x % MOD + MOD) % MOD;
}


// Common interface
class RangeStructure {
public:
    virtual ~RangeStructure() = default;

    virtual void build(const vector<long long>& data) = 0;

    virtual void update_range(int l, int r,
                              long long a, long long b) = 0;

    virtual long long query_min(int l, int r) = 0;
    virtual long long query_max(int l, int r) = 0;
    virtual long long query_sum(int l, int r) = 0;

    virtual void reset_counters() = 0;
    virtual long long get_operation_cost() const = 0;
};


// Structure A: Segment Tree
class SegmentTree : public RangeStructure {

private:

    struct Node {
        long long sum_val = 0;
        long long min_val = 0;
        long long max_val = 0;

        // Tags for sum (modulo MOD)
        long long sum_a = 1;
        long long sum_b = 0;

        // Tags for exact min and max
        long long minmax_a = 1;
        long long minmax_b = 0;
    };

    vector<Node> tree;

    int n = 0;

    long long operation_cost = 0;


    // Builds the tree
    void build_tree(int node, int l, int r,
                    const vector<long long>& data) {

        if (l == r) {
            tree[node].sum_val = normalise(data[l]);
            tree[node].min_val = data[l];
            tree[node].max_val = data[l];
            return;
        }

        int mid = (l + r) / 2;

        build_tree(2 * node, l, mid, data);
        build_tree(2 * node + 1, mid + 1, r, data);

        pull(node);
    }


    // Combines the information of two children
    void pull(int node) {

        Node& left = tree[2 * node];
        Node& right = tree[2 * node + 1];

        tree[node].sum_val =
            (left.sum_val + right.sum_val) % MOD;

        tree[node].min_val =
            min(left.min_val, right.min_val);

        tree[node].max_val =
            max(left.max_val, right.max_val);
    }


    // Applies an update to one node
    void apply_node(int node, int l, int r,
                    long long a_sum, long long b_sum,
                    long long a_mm, long long b_mm) {

        int len = r - l + 1;

        // Update sum
        tree[node].sum_val =
            apply_sum_tag(tree[node].sum_val,
                          len, a_sum, b_sum);

        // Update minimum and maximum
        apply_min_max_tag(tree[node].min_val,
                          tree[node].max_val,
                          a_mm, b_mm);

        // Combine new sum tag with old sum tag
        AffineTag new_tag = {a_sum, b_sum};
        AffineTag old_tag = {
            tree[node].sum_a,
            tree[node].sum_b
        };

        AffineTag combined = compose_tags(new_tag, old_tag);

        tree[node].sum_a = combined.a;
        tree[node].sum_b = combined.b;

        // Combine min/max tags using signed values
        long long old_a = tree[node].minmax_a;
        long long old_b = tree[node].minmax_b;

        tree[node].minmax_a = a_mm * old_a;
        tree[node].minmax_b = a_mm * old_b + b_mm;
    }


    // Sends pending updates to children
    void push_down(int node, int l, int r) {

        if (l == r)
            return;

        Node& current = tree[node];

        if (current.sum_a == 1 && current.sum_b == 0 &&
            current.minmax_a == 1 && current.minmax_b == 0) {
            return;
        }

        int mid = (l + r) / 2;

        // Update left child
        apply_node(2 * node, l, mid,
                   current.sum_a, current.sum_b,
                   current.minmax_a, current.minmax_b);

        // Update right child
        apply_node(2 * node + 1, mid + 1, r,
                   current.sum_a, current.sum_b,
                   current.minmax_a, current.minmax_b);

        // Clear parent's pending update
        current.sum_a = 1;
        current.sum_b = 0;

        current.minmax_a = 1;
        current.minmax_b = 0;
    }


    // Recursive range update
    void update_tree(int node, int l, int r,
                     int ql, int qr,
                     long long a, long long b) {

        if (r < ql || l > qr)
            return;

        operation_cost++;

        if (ql <= l && r <= qr) {

            apply_node(node, l, r,
                       normalise(a), normalise(b),
                       a, b);

            return;
        }

        push_down(node, l, r);

        int mid = (l + r) / 2;

        update_tree(2 * node, l, mid, ql, qr, a, b);

        update_tree(2 * node + 1, mid + 1, r, ql, qr, a, b);

        pull(node);
    }


    // Recursive minimum query
    long long min_tree(int node, int l, int r,
                       int ql, int qr) {

        if (r < ql || l > qr)
            return LLONG_MAX;

        operation_cost++;

        if (ql <= l && r <= qr)
            return tree[node].min_val;

        push_down(node, l, r);

        int mid = (l + r) / 2;

        long long left_min =
            min_tree(2 * node, l, mid, ql, qr);

        long long right_min =
            min_tree(2 * node + 1, mid + 1, r, ql, qr);

        return min(left_min, right_min);
    }


    // Recursive maximum query
    long long max_tree(int node, int l, int r,
                       int ql, int qr) {

        if (r < ql || l > qr)
            return LLONG_MIN;

        operation_cost++;

        if (ql <= l && r <= qr)
            return tree[node].max_val;

        push_down(node, l, r);

        int mid = (l + r) / 2;

        long long left_max =
            max_tree(2 * node, l, mid, ql, qr);

        long long right_max =
            max_tree(2 * node + 1, mid + 1, r, ql, qr);

        return max(left_max, right_max);
    }


    // Recursive sum query
    long long sum_tree(int node, int l, int r,
                       int ql, int qr) {

        if (r < ql || l > qr)
            return 0;

        operation_cost++;

        if (ql <= l && r <= qr)
            return tree[node].sum_val;

        push_down(node, l, r);

        int mid = (l + r) / 2;

        long long left_sum =
            sum_tree(2 * node, l, mid, ql, qr);

        long long right_sum =
            sum_tree(2 * node + 1, mid + 1, r, ql, qr);

        return (left_sum + right_sum) % MOD;
    }


public:

    // Builds the segment tree
    void build(const vector<long long>& data) override {

        n = data.size();

        tree.assign(4 * n + 1, Node());

        operation_cost = 0;

        if (n > 0)
            build_tree(1, 0, n - 1, data);
    }


    // Updates range [l, r]
    void update_range(int l, int r,
                      long long a, long long b) override {

        if (n == 0 || l > r)
            return;

        update_tree(1, 0, n - 1, l, r, a, b);
    }


    // Returns minimum in range [l, r]
    long long query_min(int l, int r) override {

        if (n == 0 || l > r)
            return LLONG_MAX;

        return min_tree(1, 0, n - 1, l, r);
    }


    // Returns maximum in range [l, r]
    long long query_max(int l, int r) override {

        if (n == 0 || l > r)
            return LLONG_MIN;

        return max_tree(1, 0, n - 1, l, r);
    }


    // Returns sum in range [l, r]
    long long query_sum(int l, int r) override {

        if (n == 0 || l > r)
            return 0;

        return sum_tree(1, 0, n - 1, l, r);
    }


    // Resets operation counter
    void reset_counters() override {
        operation_cost = 0;
    }


    // Returns total operation cost
    long long get_operation_cost() const override {
        return operation_cost;
    }
};
