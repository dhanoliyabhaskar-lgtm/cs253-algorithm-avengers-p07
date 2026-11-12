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
