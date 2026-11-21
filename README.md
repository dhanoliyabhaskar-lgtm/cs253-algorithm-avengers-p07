# Project 07: Lazy Propagation and What Composes

**Team Algorithm Avengers** — CS 253 / MA 253

## Team and Roles

| Name | Roll No. | GitHub | Role | Module Folder | Derivation |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Bhaskar Dhanoliya | 250001013 | [@dhanoliyabhaskar-lgtm](https://github.com/dhanoliyabhaskar-lgtm) | System Architect | `include/`, `app/` | D3 |
| Ajeet Kumar | 250041002 | [@ajeetK-dot](https://github.com/ajeetK-dot) | Structure A Owner | `src/structA/` | D1 |
| Faizan Beg Mirza | 250001029 | [@FaizanBegMirza-cse250001029](https://github.com/FaizanBegMirza-cse250001029) | Structure B Owner | `src/structB/` | D2 |
| Chandan Yadav | 250041010 | [@chandan41010](https://github.com/chandan41010) | Oracle Owner | `oracle/` | D4 |
| Aryan Gothwal | 250001010 | [@aryann-git](https://github.com/aryann-git) | Adversary | `adversary/` | D5 |
| Arjun Kumar | 250001007 | [@cse250001007](https://github.com/cse250001007) | Analyst | `bench/` | D6 |

---

## Summary

A high-throughput calibration table simulator supporting up to $N = 10^6$ sensor readings with bulk range affine transformations ($x' = a \cdot x + b$), range assignments ($x' = v$), range additions ($x' = x + d$), and range queries for exact `min`, `max`, and positive `sum` ($\bmod\ 998244353$).

We implement all models behind a unified polymorphic C++17 interface (`rs::RangeStructure`), verify 100% output correctness against a brute-force reference oracle, benchmark Lazy Segment Tree ($O(\log N)$) against Square-Root Decomposition ($O(\sqrt{N})$), and analyze the algebraic limits of lazy tag composition (such as non-composability of Range Modulo and Segment Tree Beats resolution).

---

## Requirements

- **OS:** Linux / macOS / Windows (WSL or MinGW)
- **Compiler:** `g++` (GCC 9+) or `clang++` with `-std=c++17 -O2`
- **Build Tool:** GNU `make`
- **Optional (for plots):** Python 3 with `pandas` and `matplotlib`

---

## Build

Compile all binaries (`app`, `run_ops`, and test suites):

```bash
make
```

To clean build artifacts:

```bash
make clean
```

---

## Quick Start & Differential Verification

Run the calibration simulator on a sample input file:

```bash
./bin/app --input examples/small.txt
```

Run with `--verify` to cross-check every query output of Structure A and Structure B against the ground-truth `ArrayOracle`:

```bash
./bin/app --input examples/small.txt --verify
```

---

## Tests

Execute unit tests, adversarial edge cases, and differential invariant checks:

```bash
make test
```

---

## Evaluation Interface (Hidden Tests)

For automated grading on hidden workloads via standard I/O:

```bash
./bin/run_ops --structure A < ops.txt > out.txt
./bin/run_ops --structure B < ops.txt > out.txt
```

---

## Benchmarks and Figures

Run the full profiling suite across varying array sizes ($N$) and update/query ratios, and generate CSV logs and plots in `results/`:

```bash
./scripts/run_all.sh
```

---

## Repository Map

- `include/`: Unified polymorphic interface (`range_structure.hpp`) and overflow-safe modulo helpers
- `src/structA/`: Structure A — Lazy Segment Tree implementation ($O(\log N)$ per operation)
- `src/structB/`: Structure B — Square-Root Decomposition implementation ($O(\sqrt{N})$ per operation)
- `app/`: Calibration simulator application (`simulator.cpp`)
- `oracle/`: Brute-force `ArrayOracle` and differential testing harness
- `adversary/`: Adversarial edge-case and stress-test input generators
- `bench/`: Benchmarking harness and timing scripts
- `tests/`: Unit tests and invariant checks
- `results/`: Raw benchmark CSV outputs and performance comparison figures
- `docs/`: Theoretical derivations (`D1`–`D6`), `INTERFACE.md`, and presentation slides

---

## Results at a Glance

- **Correctness:** 100% agreement across Structure A, Structure B, and `ArrayOracle` on randomized, adversarial, and boundary workloads.
- **Asymptotic Scaling:** Structure A (Lazy Segment Tree) achieves $O(\log N)$ updates and queries, outperforming Structure B ($O(\sqrt{N})$) significantly for large $N \ge 10^4$ and mixed/update-heavy workloads, while Structure B exhibits competitive constant-factor overhead on small arrays ($N \le 10^3$).
- **Theoretical Findings (D1–D6):** Affine tags form a closed monoid under composition, whereas non-linear updates like Range Modulo fail tag closure and require potential-based pruning (Segment Tree Beats) in $O((Q + N \log A_{\max}) \log N)$ amortized time.

---

## Known Limitations

- **Exact `min`/`max` vs. Modular `sum`:** `min` and `max` queries track exact 64-bit signed values, whereas `sum` is maintained modulo $998244353$; extreme chained multiplications without assignments can cause signed 64-bit wrap-around on exact `min`/`max` if inputs exceed $2^{63}-1$.
- **Non-Composable Updates:** Operations lacking $O(1)$ tag closure (e.g., Range Modulo) are analyzed theoretically in `docs/` (Derivation D3) and are not part of the standard affine lazy tag pipeline.

---

## Tools and References

See `INTERFACE.md`, `TOOLS.md`, and `REPORT.md` (along with derivations in `docs/`) for full API specifications, proof details, and benchmark analysis.