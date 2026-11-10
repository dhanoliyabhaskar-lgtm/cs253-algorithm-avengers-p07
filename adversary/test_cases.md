# Adversary test cases

## 1. Overlapping affine updates with a negative multiplier

Ranges are 0-based and inclusive.

1. Start with `[1, 2, 3, 4]`.
2. Apply `x = 2*x + 1` to `[0, 3]` → `[3, 5, 7, 9]`.
3. Apply `x = -x + 4` to `[1, 2]` → `[3, -1, -3, 9]`.
4. Query `[1, 2]`.

Expected `(min, max, sum_mod)` = `(-3, -1, 998244349)`.
Verified against the oracle.
