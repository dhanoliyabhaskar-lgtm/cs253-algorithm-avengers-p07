# Project Report: Lazy Propagation and What Composes

Team Algorithm Avengers — CS 253 / MA 253

## 1. Team and Contributions
- Bhaskar Dhanoliya (Architect): Common interface, simulator application, Derivation D3.
- Ajeet Kumar(Structure A Owner): Lazy Segment Tree implementation, Derivation D1.
- Faizan Beg Mirza(Structure B Owner): Sqrt Decomposition implementation, Derivation D2.
- Chandan Yadav(Oracle Owner): Array oracle, differential test harness, Derivation D4.
- Aryan Gothwal(Adversary): Adversarial input generators, Derivation D5.
- Arjun Kumar(Analyst): Benchmarking, profiling, Derivation D6.
## 2. Problem
Calibration table simulator handling 1,000,000 sensor readings subjected to range linear updates and aggregate queries (min, max, sum mod 998244353).

## 3. Interface and Invariants
Defined in `INTERFACE.md`.

## 4. Predicted Cost Model (Draft for Checkpoint 1)
- **Structure A (Segment Tree):**
  - Range update: $O(\log n)$ visits at most $4 \log n$ nodes.
  - Range query: $O(\log n)$ per query.
- **Structure B (Sqrt Decomposition with block size $b \approx \sqrt{n}$):**
  - Range update / query: At most $2b$ partial block element steps + $(n/b)$ full block touches, yielding $O(n/b + b) = O(\sqrt{n})$.
- **Cross-over Prediction:**
  - Structure A will consistently outperform Structure B for large $n = 10^6$, but Structure B may show competitive cache-locality for narrow range widths.# Project 07 Report: Lazy Propagation and What Composes

**Team Algorithm Avengers** — CS 253 / MA 253

---

## 1. Team and Contributions

| Name | Roll No. | GitHub | Role | Module Folder | Derivation |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Bhaskar Dhanoliya | 250001013 | [@dhanoliyabhaskar-lgtm](https://github.com/dhanoliyabhaskar-lgtm) | System Architect | `include/`, `app/` | D3 |
| Ajeet Kumar | 250041002 | [@ajeetK-dot](https://github.com/ajeetK-dot) | Structure A Owner | `src/structA/` | D1 |
| Faizan Beg Mirza | 250001029 | [@FaizanBegMirza-cse250001029](https://github.com/FaizanBegMirza-cse250001029) | Structure B Owner | `src/structB/` | D2 |
| Chandan Yadav | 250041010 | [@chandan41010](https://github.com/chandan41010) | Oracle Owner | `oracle/` | D4 |
| Aryan Gothwal | 250001010 | [@aryann-git](https://github.com/aryann-git) | Adversary | `adversary/` | D5 |
| Arjun Kumar | 250001007 | [@cse250001007](https://github.com/cse250001007) | Analyst | `bench/` | D6 |

### Individual Module Contributions
* **Bhaskar Dhanoliya (System Architect):** Designed the unified polymorphic interface (`include/range_structure.hpp`), wrote API invariants (`INTERFACE.md`), built the calibration simulator (`app/simulator.cpp`), and authored Derivation D3 (Non-composability of Range Modulo & Segment Tree Beats complexity). *(Add main PR/commit links here)*
* **Ajeet Kumar (Structure A Owner):** Implemented the Lazy Segment Tree (`src/structA/`) supporting $O(\log N)$ affine, assign, and add updates alongside min/max/sum queries, and authored Derivation D1. *(Add main PR/commit links here)*
* **Faizan Beg Mirza (Structure B Owner):** Implemented Square-Root Decomposition (`src/structB/`) with block-level lazy tags and partial-block rebuilds in $O(\sqrt{N})$, and authored Derivation D2. *(Add main PR/commit links here)*
* **Chandan Yadav (Oracle Owner):** Built the brute-force `ArrayOracle` (`oracle/`) and automated differential test suite verifying 100% agreement across $10^6$ operations, and authored Derivation D4. *(Add main PR/commit links here)*
* **Aryan Gothwal (Adversary):** Created edge-case and stress-test workload generators (`adversary/`) targeting overflow boundaries, block splits, and deeply nested affine compositions, and authored Derivation D5. *(Add main PR/commit links here)*
* **Arjun Kumar (Analyst):** Developed the benchmarking harness (`bench/`), collected raw timing CSVs across varying $N$ and workload mixes, generated plots, and authored Derivation D6. *(Add main PR/commit links here)*

---

## 2. Problem

Modern high-frequency sensor calibration pipelines maintain large arrays of sensor readings ($N$ up to $10^6$) that undergo frequent batch recalibrations and diagnostic range queries. Naively scanning every element in $[l, r]$ takes $O(N)$ time per operation, which becomes a severe bottleneck under $Q = 10^6$ mixed updates and queries.

In this project, we study how algebraic operation composition enables sub-linear range processing. Specifically, our application simulates a calibration table supporting:
1. **Range Affine Update ($l, r, a, b$):** $A[i] \leftarrow a \cdot A[i] + b$ for all $i \in [l, r]$.
2. **Range Assignment ($l, r, v$):** $A[i] \leftarrow v$ for all $i \in [l, r]$.
3. **Range Addition ($l, r, d$):** $A[i] \leftarrow A[i] + d$ for all $i \in [l, r]$.
4. **Range Aggregate Queries ($l, r$):** Exact minimum $\min_{i \in [l, r]} A[i]$, exact maximum $\max_{i \in [l, r]} A[i]$, and modular sum $\left(\sum_{i=l}^{r} A[i]\right) \bmod 998244353$.

We implement and compare two sub-linear data structures—a **Lazy Segment Tree** ($O(\log N)$) and **Square-Root Decomposition** ($O(\sqrt{N})$)—against a brute-force **Oracle** ($O(N)$), and analyze where lazy tag composition breaks down (such as Range Modulo).

---

## 3. Interface and Invariants

All data structures inherit from the abstract base class `rs::RangeStructure` defined in `include/range_structure.hpp` (detailed in `INTERFACE.md`).

### Unified API Contract
```cpp
struct QueryResult {
    int64_t min_val;
    int64_t max_val;
    int64_t sum_mod; // Maintained modulo 998244353
};

class RangeStructure {
public:
    virtual ~RangeStructure() = default;
    virtual void range_affine(int l, int r, int64_t a, int64_t b) = 0;
    virtual void range_assign(int l, int r, int64_t v) = 0;
    virtual void range_add(int l, int r, int64_t d) = 0;
    virtual QueryResult query(int l, int r) = 0;
};
```

### Structural and Algebraic Invariants
1. **Indexing & Bounds Invariant:** All ranges $[l, r]$ are $0$-indexed and inclusive ($0 \le l \le r < N$).
2. **Modular Sum Invariant:** `sum_mod` is always normalized to $[0, M - 1]$ where $M = 998244353$, using overflow-safe wrap-around arithmetic (`wmul`, `wadd`).
3. **Exact Extrema Invariant:** `min_val` and `max_val` track exact signed 64-bit values (not reduced modulo $M$). Note that for affine updates with $a < 0$, the minimum and maximum of a segment swap:
   $$\min'(S) = a \cdot \max(S) + b, \qquad \max'(S) = a \cdot \min(S) + b$$
4. **Lazy Tag Closure Invariant:** Applying $f_1(x) = a_1 x + b_1$ followed by $f_2(x) = a_2 x + b_2$ composes into a single affine tag:
   $$(f_2 \circ f_1)(x) = (a_2 a_1)x + (a_2 b_1 + b_2)$$
   Range assignment ($x \mapsto v$) corresponds to $(0, v)$ and range addition ($x \mapsto x + d$) corresponds to $(1, d)$.

---

## 4. Structure A: Lazy Segment Tree (`src/structA/`)
*(Owner: Ajeet Kumar — 2–3 pages)*

### 4.1 Node Representation & Memory Layout
* Describe array-based $4N$ tree storage (`sum_mod`, `min_val`, `max_val`, and lazy tag pair `(lazy_a, lazy_b)` with identity `(1, 0)`).

### 4.2 Operations & Complexity
* **`build` — $O(N)$:** Bottom-up aggregate initialization.
* **`push_down` & `apply_tag` — $O(1)$:** Pushing pending affine tags to left and right children; handling negative multipliers ($a < 0$) when updating `min_val` and `max_val`.
* **`range_affine` / `range_assign` / `range_add` — $O(\log N)$:** Standard recursive decomposition visiting at most $4 \log_2 N + O(1)$ nodes (Derivation D4).
* **`query` — $O(\log N)$:** Merging `QueryResult` across canonical nodes.

### 4.3 Design Decisions & Rejected Alternatives
* Explain why a single unified affine tag `(a, b)` was chosen over separate boolean flags for `assign` and `add` (eliminates tag precedence bugs and simplifies `push_down`).
* Explain why `std::vector` flat indexing (`2*v`, `2*v+1`) was chosen over pointer-based nodes (better cache locality, zero heap fragmentation).

---

## 5. Structure B: Square-Root Decomposition (`src/structB/`)
*(Owner: Faizan Beg Mirza — 2–3 pages)*

### 5.1 Block Representation & Partitioning
* Describe partitioning $A[0 \dots N-1]$ into $\lceil N / B \rceil$ blocks of size $B \approx \lfloor \sqrt{N} \rfloor$, storing raw elements alongside block-level aggregates (`block_sum`, `block_min`, `block_max`) and block-level lazy tags `(block_a, block_b)`.

### 5.2 Operations & Complexity
* **`push_block` & `rebuild_block` — $O(\sqrt{N})$:** Materializing pending block tags onto individual elements before partial-block updates/queries.
* **`range_affine` / `range_assign` / `range_add` — $O(\sqrt{N})$:** Updating at most 2 partial end-blocks element-by-element ($O(B)$) and up to $N/B$ full interior blocks in $O(1)$ each.
* **`query` — $O(\sqrt{N})$:** Scanning partial blocks and combining full block aggregates.

### 5.3 Design Decisions & Rejected Alternatives
* Discuss choosing fixed power-of-two or $\lfloor \sqrt{N} \rfloor$ block size vs. adaptive tuning for L1 cache fit, and why maintaining block min/max under negative `a` requires tracking both extrema per block.

---

## 6. Requirement Beyond the Lecture: Non-Composable Updates & Segment Tree Beats
*(Author: Bhaskar Dhanoliya — 1–2 pages)*

While affine transformations form a closed monoid under composition, real-world range pipelines often encounter non-linear updates such as **Range Modulo** ($A[i] \leftarrow A[i] \bmod m$).

### 6.1 Why Standard Lazy Propagation Fails (Derivation D3)
1. **Non-Composability of Tags:** Two modulo operations $(x \bmod m_1) \bmod m_2$ with $m_1 > m_2$ cannot be compressed into a single $x \bmod m_3$ tag (e.g., $(x \bmod 7) \bmod 4$ yields $1, 1, 2$ for $x = 5, 8, 13$, which no single modulus $m_3$ can reproduce). A strictly decreasing sequence of $k$ moduli requires $\Omega(k)$ tag storage.
2. **Non-Distributivity over Sum:** $\sum_{i=l}^r (A[i] \bmod m) \neq \left(\sum_{i=l}^r A[i]\right) \bmod m$, preventing $O(1)$ aggregate updates without inspecting children.

### 6.2 Segment Tree Beats Resolution
Following Ji Driver's Segment Tree Beats framework, each node stores $\max_v$ alongside $\text{sum}_v$ and prunes any subtree where $\max_v < m$ (since $x \bmod m = x$ for all $x < m$). Whenever an element is effectively updated ($A[i] \ge m$), its value strictly halves ($A'[i] < A[i]/2$). Using the potential function $\Phi = \sum_{i=0}^{N-1} \lfloor \log_2(A[i] + 1) \rfloor$, total effective leaf updates are bounded by $O((N + M)\log(C + 1))$, giving an amortized running time of:
$$T = O\big((N + Q) \log N \log(C + 1)\big)$$
*(Full step-by-step proof is provided in `docs/derivations/D3_250001013.pdf`.)*

---

## 7. Correctness, Differential Testing, and Bug Log
*(Owner: Chandan Yadav — 1–2 pages)*

### 7.1 Ground-Truth Oracle (`oracle/`)
* `ArrayOracle` maintains a plain `std::vector<int64_t>` and executes every update and query via direct $O(N)$ loops using exact 64-bit arithmetic and modular reduction. Because there is no lazy state or block caching, its correctness is transparent by inspection.

### 7.2 Differential Testing & Invariant Checks (`tests/`)
* Describe the automated differential harness (`make test`) running $10^6$ randomized operations (seed `20260928`) across Structure A, Structure B, and `ArrayOracle`, asserting exact equality of `min_val`, `max_val`, and `sum_mod` after every query.

### 7.3 Bug Log

| Date | Module | Symptom | How Found |
| :--- | :--- | :--- | :--- | :--- |
| 2026-11-17 | `src/structA/` | `min_val` and `max_val` inverted after negative affine multiplier ($a < 0$) | Differential test against `ArrayOracle` | 
| 2026-11-18 | `src/structB/` | Stale block aggregate when partial update hit a block with pending lazy tag | Adversarial single-element boundary test | 
| 2026-11-20 | `include/` | Signed overflow on `a * x + b` before modulo reduction | Adversarial max-value input (`10^9`) |

---

## 8. Adversarial Inputs
*(Owner: Aryan Gothwal — 1–2 pages)*

* **Target 1 — Block Boundary Thrashing (Structure B):** Queries and updates spanning $[kB + 1, (k+1)B - 1]$ to force maximum partial-block reconstructions per operation.
* **Target 2 — Deep Tree Path & Tag Cascading (Structure A):** Alternating single-element and full-range updates forcing repeated `push_down` cascades down all $\log_2 N$ levels.
* **Target 3 — Arithmetic Overflow & Zero/Negative Multipliers:** Sequences mixing $a = 0$ (assignment collapse), $a = -1$ (extrema inversion), and values near $10^9$ to stress 64-bit bounds.
* **Measured Effect:** Summarize slowdown/stress observed on Structure A vs. Structure B under adversarial workloads compared to uniform random inputs.

---

## 9. Predicted Cost Model

### 9.1 Asymptotic & Constant-Factor Predictions
Let $N$ be the array size and $Q$ be the number of operations:
* **Build Time:** Both Structure A ($O(N)$) and Structure B ($O(N)$) scale linearly; Structure B has lower initialization overhead due to a single flat pass without tree node construction.
* **Per-Operation Latency:**
  * Structure A visits at most $4 \log_2 N$ nodes per operation ($T_A(N) \approx c_1 \log_2 N + c_2$).
  * Structure B processes up to $2\sqrt{N}$ boundary elements plus $\sqrt{N}$ full blocks ($T_B(N) \approx c_3 \sqrt{N} + c_4$).
* **Predicted Crossover Point:** For very small arrays ($N \le 512$), Structure B's contiguous sequential memory access and low constant factor ($c_3 < c_1$) are predicted to match or slightly beat Structure A's recursive tree traversal. Beyond $N \approx 2{,}000$, Structure A's logarithmic scaling will dominate, achieving $> 10\times$ speedup over Structure B at $N = 10^5$ and $> 30\times$ at $N = 10^6$.
* **Workload Sensitivity:**
  * *Query-heavy (80% queries):* Structure A requires zero `push_down` writes on unmodified segments, widening its lead.
  * *Update-heavy (80% affine updates):* Structure B avoids tree recursion overhead on full blocks, narrowing the constant-factor gap at medium $N$, though $O(\sqrt{N})$ still dominates asymptotically.

---

## 10. Experimental Setup
*(Owner: Arjun Kumar — ½ page)*

* **Hardware:** *(e.g., CPU model, clock speed, L1/L2/L3 cache sizes, RAM)*
* **Software:** *(e.g., Ubuntu 22.04 LTS, `g++ 11.4.0` with `-std=c++17 -O2`)*
* **Benchmark Parameters:**
  * Array sizes: $N \in \{10^2, 10^3, 10^4, 10^5, 10^6\}$
  * Operations per run: $Q = 10^5$ (and $10^6$ for scaling check)
  * Workload mixes: Balanced (50% update / 50% query), Update-heavy (80% / 20%), Query-heavy (20% / 80%), and Adversarial
  * Repetitions & Seeds: Mean and standard deviation across $\ge 5$ independent runs per data point with recorded deterministic seeds (`results/raw/*.csv`).

---

## 11. Results
*(Owner: Arjun Kumar — 3–5 pages)*


* **Figure 1 (`bench`):** Total execution time (ms) vs. Array size $N$ (log-log scale) comparing Predicted vs. Measured curves for Structure A, Structure B, and Oracle.
* **Figure 2 (`bench`):** Per-operation latency ($\mu$s) across Update-Heavy, Balanced, and Query-Heavy workloads at $N = 10^5$ and $N = 10^6$.
* **Figure 3 (`bench`):** Uniform random vs. Adversarial worst-case runtime comparison.
* **Gap Analysis:** Explain any divergence between the predicted cost model (§9) and empirical curves (e.g., L2/L3 cache misses once $4N$ segment tree nodes exceed cache capacity at $N = 10^6$, or hardware branch prediction effects).

---

## 12. Limitations

1. **Exact `min`/`max` Overflow under Unbounded Chained Multiplications:** While `sum_mod` is safely wrapped modulo $998244353$, `min_val` and `max_val` store exact 64-bit signed integers. Repeated affine multiplications with $|a| > 1$ and without intermediate `range_assign` resets can exceed `INT64_MAX` ($2^{63}-1$).
2. **Non-Composable Updates Excluded from Standard Pipeline:** Non-linear updates such as Range Modulo are proved non-composable in §6 (Derivation D3) and require specialized pruning (Segment Tree Beats) rather than the standard $(a, b)$ affine lazy tag interface.
3. **Future Work:** Extending the unified interface to support hybrid Segment Tree Beats nodes and measuring hardware cache-miss counters (`perf stat`) across block sizes in Structure B.

---

## 13. References

1. **Ji Driver Segment Tree (Segment Tree Beats):** J. Ji, *"Segment Tree Beats: A Data Structure for Range Chmin/Chmax and Range Modulo Operations,"* IOI / Competitive Programming Monograph.
2. **Lazy Propagation & Monoid Composition:** M. de Berg et al., *Computational Geometry: Algorithms and Applications* (Segment Trees chapter), Springer.
3. **Square-Root Decomposition:** A. Laaksonen, *Competitive Programmer's Handbook*, Chapter 27 (Square Root Algorithms).
4. **Project Interface & Derivations:** `INTERFACE.md`, `TOOLS.md`, and `docs/derivations/D1`–`D6` in this repository.