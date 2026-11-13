# Project 07: Lazy propagation and what composes
Team Algorithm Avengers — CS 253 / MA 253

## Team and roles
| Name | Roll no. | GitHub | Role | Module folder | Derivation |
|------|----------|--------|------|---------------|------------|
| Bhaskar Dhanoliya | 250001013 | @dhanoliyabhaskar-lgtm | Architect | app/ | D3 |
| Ajeet Kumar | 250041002 | @ajeetK-dot | Structure A owner | src/structA/ | D1 |
| Arjun Kumar | 250001007 | @cse250001007 | Analyst | bench/ | D6 |
| Chandan Yadav | 250041010 | @chandan41010 | Oracle owner | oracle/ | D4 |
| Aryann Gothwal | 250001010 | @aryann-git | Adversary | adversary/ | D5 |
| Faizan Beg Mirza | 250001029 | @FaizanBegMirza-cse250001029 | Structure B owner | src/structB/ | D2 |

## Summary
A calibration table simulator of 1,000,000 sensor readings with bulk affine corrections (gain * x + offset) and queries for range min, max, and sum mod 998244353. We compare a Lazy Segment Tree against Square-Root Decomposition and study operation composition limits.

## Requirements
- OS: Linux / Windows
- Compiler: g++ with C++17 support, -O2

## Build
    make

## Quick start
    ./bin/app --input examples/small.txt

## Tests
    make test

## Evaluation interface (hidden tests)
    ./bin/run_ops --structure A < ops.txt > out.txt
    ./bin/run_ops --structure B < ops.txt > out.txt

## Benchmarks and figures
    ./scripts/run_all.sh

## Repository map
- `include/`: Shared interfaces and common headers
- `src/structA/`: Structure A (Lazy Segment Tree)
- `src/structB/`: Structure B (Square-Root Decomposition)
- `app/`: Calibration simulator application
- `oracle/`: Array oracle and differential test harness
- `adversary/`: Adversarial test input generators
- `bench/`: Benchmarking harness and profiling scripts
- `tests/`: Unit tests and invariant checks
- `results/`: Raw CSV data and generated figures
- `docs/`: Derivation documents (D1-D6) and slides

## Results at a glance
TBD (Benchmarking in progress)

## Known limitations
None currently.

## Tools and references
See TOOLS.md and REPORT.md.