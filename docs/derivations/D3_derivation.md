# Derivation D3: Non-Composability of Range Modulo & Segment Tree Beats Complexity

## 1. Problem Statement
Consider an array $A[0 \dots N-1]$ of non-negative integers supporting two operations:
1. **Range Modulo Update:** Given $[l, r]$ and a positive integer $m$, update $A[i] \leftarrow A[i] \bmod m$ for all $i \in [l, r]$.
2. **Range Sum Query:** Given $[l, r]$, compute $\sum_{i=l}^{r} A[i]$.

We investigate why standard **Lazy Propagation** fails to handle Range Modulo in $O(\log N)$ time per operation, and how **Segment Tree Beats (Ji Driver Segment Tree)** resolves this using a potential-function argument.

---

## 2. Why Standard Lazy Propagation Fails (Non-Composability)

Standard lazy propagation relies on two algebraic properties of the update family $\mathcal{F}$ and aggregate function $\oplus$:
1. **Closure under Composition:** Applying $f_1 \in \mathcal{F}$ followed by $f_2 \in \mathcal{F}$ must be representable by a single compressed tag $f_3 = f_2 \circ f_1 \in \mathcal{F}$ in $O(1)$ space and time.
2. **Distributivity over Aggregates:** We must be able to update a node's aggregate $\text{sum}(S)$ using only the tag $f$ and summary metadata (e.g., $\text{len}(S)$), without inspecting individual elements.

### Proof of Non-Composability
Let $f_m(x) = x \bmod m$. Suppose a segment tree node already holds a pending tag $f_{m_1}(x) = x \bmod m_1$, and a new update $f_{m_2}(x) = x \bmod m_2$ arrives. The composed function is:
$$(f_{m_2} \circ f_{m_1})(x) = (x \bmod m_1) \bmod m_2$$

- **Case 1 ($m_1 \le m_2$):** Since $x \bmod m_1 < m_1 \le m_2$, the second modulo has no effect:
  $$(x \bmod m_1) \bmod m_2 = x \bmod m_1$$
  Here, the tag simplifies to $f_{m_1}$.
- **Case 2 ($m_1 > m_2$):** In general, $(x \bmod m_1) \bmod m_2$ **cannot** be reduced to a single $x \bmod m_3$ for any constant $m_3$.
  - *Counterexample:* Let $m_1 = 7$ and $m_2 = 4$.
    - For $x = 5$: $(5 \bmod 7) \bmod 4 = 5 \bmod 4 = 1$.
    - For $x = 8$: $(8 \bmod 7) \bmod 4 = 1 \bmod 4 = 1$.
    - For $x = 13$: $(13 \bmod 7) \bmod 4 = 6 \bmod 4 = 2$.
    - If a single $m_3$ existed such that $x \bmod m_3 = (x \bmod 7) \bmod 4$ for all $x$, then $x = 5 \implies 5 \bmod m_3 = 1 \implies m_3 \mid 4 \implies m_3 \in \{2, 4\}$. However, $8 \bmod 2 = 0 \ne 1$ and $8 \bmod 4 = 0 \ne 1$, a contradiction.

Furthermore, a sequence of $k$ updates with strictly decreasing moduli $m_1 > m_2 > \dots > m_k$ requires storing the entire history of $k$ moduli at a node ($\Omega(k)$ space per tag) because the modulo operation is non-linear and non-distributive over addition:
$$\sum_{i=l}^{r} (A[i] \bmod m) \neq \left(\sum_{i=l}^{r} A[i]\right) \bmod m$$
Thus, standard lazy propagation cannot compress tags or compute the updated range sum in $O(1)$ time.

---

## 3. Segment Tree Beats Resolution & Amortized Complexity

Instead of lazy tag composition, **Segment Tree Beats** augments each segment tree node $v$ with $\text{max}_v = \max_{i \in \text{range}(v)} A[i]$ alongside $\text{sum}_v$, and uses two pruning conditions during a Range Modulo update $[l, r]$ with modulus $m$:

1. **Break Condition (Skip Subtree):** If $\text{range}(v) \cap [l, r] = \emptyset$ **or** $\text{max}_v < m$, return immediately.
   - *Correctness:* If $\text{max}_v < m$, then for every $i \in \text{range}(v)$, $0 \le A[i] \le \text{max}_v < m$, so $A[i] \bmod m = A[i]$. No element changes.
2. **Base Case (Leaf Node):** If $v$ is a leaf representing index $i$, directly set $A[i] \leftarrow A[i] \bmod m$, update $\text{sum}_v = \text{max}_v = A[i]$, and return.
3. **Recursive Step:** Otherwise, recurse on both children of $v$ and pull up $\text{sum}_v = \text{sum}_{\text{left}} + \text{sum}_{\text{right}}$ and $\text{max}_v = \max(\text{max}_{\text{left}}, \text{max}_{\text{right}})$.

### Amortized Time Complexity Proof (Potential Function Method)
Let $A_{\max} = \max_{0 \le i < N} A[i]$ be the maximum initial value in the array.

**Lemma (Halving Property):** Whenever a leaf $A[i]$ is visited during a Range Modulo update with modulus $m$, the break condition guarantees $A[i] \ge m$. After the update $A'[i] = A[i] \bmod m$:
- **Subcase A ($m \le \frac{A[i]}{2}$):** Since $A'[i] < m$, we have $A'[i] < \frac{A[i]}{2}$.
- **Subcase B ($m > \frac{A[i]}{2}$):** Since $m \le A[i] < 2m$, we have $A'[i] = A[i] - m < A[i] - \frac{A[i]}{2} = \frac{A[i]}{2}$.

In **both** cases, every effective modulo update reduces $A[i]$ by at least a factor of $2$:
$$A'[i] < \frac{A[i]}{2}$$

**Potential Function:** Define the potential $\Phi$ of the array as:
$$\Phi = \sum_{i=0}^{N-1} \lfloor \log_2(A[i] + 1) \rfloor$$

- **Initial Potential:** $\Phi_0 \le N \log_2(A_{\max} + 1)$.
- **Final Potential:** Since $A[i] \ge 0$, $\Phi_{\text{final}} \ge 0$.
- **Drop per Leaf Update:** Because $A'[i] < A[i]/2$, $\lfloor \log_2(A[i] + 1) \rfloor$ decreases by at least $1$ every time leaf $i$ is updated. Therefore, across the entire sequence of $Q$ operations, the total number of effective leaf updates is bounded by:
  $$\text{Total Leaf Updates} \le \Phi_0 \le N \log_2(A_{\max} + 1)$$

**Total Running Time:**
- In a standard segment tree traversal for $[l, r]$, at most $O(\log N)$ nodes are visited on the boundary paths without reaching a leaf that gets updated.
- Every internal node visited beyond those $O(\log N)$ boundary nodes must lie on an ancestor path to at least one leaf whose value strictly decreases (guaranteed by the $\text{max}_v \ge m$ check). Since each leaf has depth $O(\log N)$, each effective leaf update contributes at most $O(\log N)$ internal node visits.
- Summing over $Q$ operations:
  $$T_{\text{total}}(N, Q) = O\Big((Q + N \log A_{\max}) \log N\Big)$$

Thus, the amortized time per operation is **$O(\log N \log A_{\max})$** (or **$O(\log^2 N)$** when $A_{\max} = \text{poly}(N)$).