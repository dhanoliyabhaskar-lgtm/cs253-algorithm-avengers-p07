```markdown
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
  - Structure A will consistently outperform Structure B for large $n = 10^6$, but Structure B may show competitive cache-locality for narrow range widths.