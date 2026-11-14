# Adversary test cases

## 1. Overlapping affine updates with a negative multiplier

Ranges are 0-based and inclusive.

1. Start with `[1, 2, 3, 4]`.
2. Apply `x = 2*x + 1` to `[0, 3]` → `[3, 5, 7, 9]`.
3. Apply `x = -x + 4` to `[1, 2]` → `[3, -1, -3, 9]`.
4. Query `[1, 2]`.

Expected `(min, max, sum_mod)` = `(-3, -1, 998244349)`.
Verified against the oracle.


## Test: Assignment followed by a partial update

All ranges use 0-based inclusive indexing.

Initial array: [1, 2, 3, 4]

1. Update [0, 3] with x = 0*x + 5.
   Expected array: [5, 5, 5, 5]

2. Update [1, 2] with x = -x + 2.
   Expected array: [5, -3, -3, 5]

3. Query [0, 3].
   Expected minimum: -3
   Expected maximum: 5
   Expected sum modulo 998244353: 4

4. Query [1, 2].
   Expected minimum: -3
   Expected maximum: -3
   Expected sum modulo 998244353: 998244347

Purpose:
Check whether a zero-multiplier update correctly replaces old
values, and whether a later partial update is applied correctly.


## Test: Affine update composition order

Purpose:
Check that consecutive updates are applied in the correct order,
including when a later query needs pending updates pushed down.

Initial array:
[1, 2, 3, 4]

Use 0-based inclusive indices.

Update 1:
Apply x = 2*x + 1 to [0, 3].
Expected array: [3, 5, 7, 9]

Update 2:
Apply x = 3*x + 4 to [0, 3].
Expected array: [13, 19, 25, 31]

Expected queries (min, max, sum_mod):
Query [0, 3]: (13, 31, 88)
Query [1, 2]: (19, 25, 44)
Query [0, 0]: (13, 13, 13)
Query [3, 3]: (31, 31, 31)

Why this catches bugs:
The combined update must be:
3*(2*x + 1) + 4 = 6*x + 7.

Reversing the order incorrectly gives:
2*(3*x + 4) + 1 = 6*x + 9.

Partial-range queries check whether pending updates are
correctly passed to child nodes.

Status:
Expected results calculated manually; implementation not yet tested.
