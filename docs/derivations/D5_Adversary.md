# D5 — How many push-downs can one range query force?

Imagine an array of eight zeros, at positions 0 through 7.

## Step 1. Add 1 to every element

Update `[0,7]` with `x = x + 1`. The root represents the whole array, so it can update its own summary right away. It also remembers a pending `+1` for its children. That reminder is called a **lazy tag**.

## Step 2. Query positions 1 through 6

We want only the middle six elements. The root also covers positions 0 and 7, so we need to look at its children. Before looking at them, the root passes its `+1` to both children. That is **one push-down**.

- `[0,7]` pushes to `[0,3]` and `[4,7]`. **1 push**
- `[0,3]` and `[4,7]` each pass `+1` to their children. **2 pushes**
- `[0,1]` and `[6,7]` each pass `+1` to their leaves. **2 pushes**

Why split `[0,1]`? We want index 1 and skip index 0. Similarly, we split `[6,7]` to take index 6 and skip index 7. For `[2,3]` and `[4,5]`, we want every element in the segment, so we can use its summary right away.

**Total for this query = 1 + 2 + 2 = 5 push-downs**

## Why can no other range force more?

Picture two cuts, one just before the first index you want and one just after the last. A node needs to split only when a cut falls inside its segment. At any level, the segments sit side by side. Each cut can fall inside at most one segment, so at most two nodes can push at that level.

If `n = 2^h`, there are `h` levels with children. The root counts at most once, and the other `h - 1` levels count at most twice each. This gives:

`1 + 2(h - 1) = 2h - 1`

For `n = 4, 8, 16, ...`, adding 1 to the whole array and querying `[1,n-2]` reaches that number. Both cuts force a split all the way down on opposite sides of the root. For general `n`, the worst-case count grows as `Theta(log n)`.
