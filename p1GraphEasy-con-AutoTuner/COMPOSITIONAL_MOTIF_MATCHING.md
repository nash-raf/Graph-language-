# Compositional Motif Matching for Frontier Graph Programs

This document describes the methodology behind SGPL’s compositional EdgeMap / VertexMap lowering: first as a theoretical design, then as it is realized in the compiler and runtime. The goal is to explain *what we are trying to recognize*, *why those pieces are the right building blocks*, *how composition is defined*, and *what happens once a composition explains a region of intermediate representation*.

---

# Part I — Theoretical Methodology

## 1. Motivation and problem statement

### 1.1 What a frontier graph program looks like

Many classical graph algorithms (breadth-first search, label-propagation connected components, fixed-\(k\) core peeling, Bellman–Ford-style single-source shortest paths, and others) share a recurring computational skeleton:

1. Maintain one or more **vertex properties** (integers indexed by vertex identity): claim/visited bits, component labels, residual degrees, distances, parent pointers, and so on.
2. Maintain an explicit **frontier**: a collection of vertices that are “active” for the current round.
3. In each round, for every active vertex \(u\), examine some set of **incident edges** \((u,v)\) (typically out-neighbors of \(u\)).
4. For each such edge, apply a **local update** to properties of \(v\) (and sometimes of \(u\)).
5. If the update “succeeds” in a sense defined by the algorithm, **activate** \(v\) into the next frontier.
6. Swap frontiers and repeat until the frontier is empty (or another stopping condition holds).

In a high-level graph DSL, this skeleton appears as nested control flow: an outer “while frontier nonempty” loop, an inner walk over frontier indices, a neighbor enumeration of the extracted vertex, and a nest of conditionals and property writes. Semantically this is correct; operationally it is expensive if compiled naïvely as generic parallel loops over array indices with fine-grained locking on every write.

### 1.2 Why “motif matching” exists at all

A *motif*, in this methodology, is a recurring structural pattern in the program’s abstract syntax / intermediate representation that:

- has a known, well-defined **parallel semantics**;
- admits a specialized **implementation strategy** (sparse push, dense owner-computes pull, atomic combine, activate-once deduplication, layout-native traversal); and
- can be **recognized statically** by structural matching rather than requiring the programmer to name an algorithm.

Historically, systems either (a) expose named operators (`edgemap`, `vertexmap`) that the programmer writes explicitly, or (b) hardcode detectors for whole algorithms (“this looks like BFS”, “this looks like CC”). SGPL’s compositional approach sits between those extremes:

- The programmer still writes ordinary loops and property updates in the DSL.
- The compiler recognizes that a region is an instance of a small algebra of frontier operators.
- The runtime executes that region through a single parametric engine parameterized by a **combine** (how destination properties are updated) and a small set of **side effects** (parent write, activation, thresholded activation, etc.).

The methodological claim is: **most frontier algorithms we care about are not distinct operators; they are different combine configurations of the same EdgeMap/VertexMap engine**.

### 1.3 What we deliberately do *not* claim

Compositional motif matching is *not*:

- a new DSL surface syntax (`edgemap { ... }` blocks);
- a rewrite of SGPL’s general parallelization paradigm (DOALL over ranges, persistent worker pool, existing cost models for non-frontier loops remain);
- a Graptor/VEBO/SIMD partitioner;
- a guarantee that every possible graph program will match;
- a license to silently change sequential semantics when matching fails—failed matches fall back to ordinary lowering.

Success, theoretically, is: **correctness-preserving recognition of a closed set of frontier effect patterns, and execution of those patterns through one shared engine**.

---

## 2. Abstract machine

To speak precisely about motifs, we fix an abstract machine.

### 2.1 Graph and properties

Let \(G = (V, E)\) be a directed graph (undirected inputs are treated as bidirectional edge sets). Vertices are identified with integers \(\{0,\ldots,n-1\}\).

A **vertex property** is a total map \(P: V \to \mathbb{Z}\) (or a finite integer domain). Typical properties:

- \(C(v)\): claim / visited / alive flag;
- \(L(v)\): label or distance;
- \(\pi(v)\): parent / predecessor;
- \(d(v)\): residual degree.

Edge weights, when present, are a map \(w: E \to \mathbb{Z}\).

### 2.2 Frontiers as ordered bags with activate-once discipline

A **frontier** \(F\) is an ordered sequence of vertex ids of length \(f = |F|\). Conceptually it is a bag; the compiler’s array representation is an `int` array plus a length. The **next frontier** \(F'\) starts empty each round (length zero) and grows by append.

**Activation** means appending a vertex \(v\) to \(F'\). Naïve activation under parallelism can append the same \(v\) many times if multiple sources successfully update \(v\). The methodology therefore distinguishes:

- **Logical activation**: “\(v\) became interesting this round.”
- **Physical activation**: a single append of \(v\) into \(F'\) for this round (**activate-once**).

Activate-once is part of the *operator semantics* of EdgeMap, not an optional optimization. Programs written as “append on every successful update” are interpreted as “activate once if any successful update occurs,” which is the intended concurrent meaning for the algorithms we target.

### 2.3 Rounds and the frontier region

A **frontier region** is the inner loop that consumes \(F\) and produces \(F'\):

\[
\begin{align*}
&\textbf{for } i = 0 \ldots |F|-1:\\
&\quad u \leftarrow F[i]\\
&\quad \textbf{body}(u)
\end{align*}
\]

The outer “while \(|F| > 0\)” loop, the swap of \(F\) and \(F'\), and initialization of properties are *outside* the motif. Motif matching targets the **body that turns one frontier into the next**, because that is where the expensive edge work lives and where push/pull duality applies.

### 2.4 EdgeMap and VertexMap as operators

We adopt the Ligra/Graptor *idea* of operators, not their implementation stack.

**VertexMap.** Given a frontier \(F\) and a function \(f\) on vertices,

\[
\texttt{VertexMap}(F, f) = \{ u \in F \mid f(u)\ \text{succeeds} \}
\]

or, alternatively, \(f\) applied for side effect to every \(u \in F\) (filtering is optional). In peeling, the important VertexMap is “try to remove \(u\) from the alive set.”

**EdgeMap.** Given a frontier \(F\), a neighbor relation \(N(u)\), a **combine** \(\oplus\) on destination properties, and an **activation predicate** \(A\),

\[
\begin{align*}
\texttt{EdgeMap}(F, \oplus, A):\quad
&\textbf{for } u \in F:\\
&\quad \textbf{for } v \in N(u):\\
&\quad\quad \textbf{if }\ \texttt{Combine}(u,v,\oplus)\ \textbf{then}\\
&\quad\quad\quad \textbf{if }\ A(u,v)\ \textbf{then activate-once}(v)
\end{align*}
\]

The combine is a function from \((u,v)\) and current property state to a boolean “did the destination property change (or get claimed)?”. Activation may be unconditional on success (BFS, CC, SSSP) or gated by a further predicate (peel: activate if residual degree falls below \(k\)).

EdgeMap is directional: sources are frontier vertices; destinations are neighbors. Pull and push are dual *schedules* of the same logical EdgeMap, not different operators.

### 2.5 Push versus pull (dual schedules of one EdgeMap)

**Sparse push.** Iterate sources in \(F\). For each source \(u\), walk \(N(u)\) and attempt combines on destinations. This is efficient when \(|F|\) is small relative to \(n\), or when the total outgoing edge work from \(F\) is modest.

**Dense owner-computes pull.** Partition the *destination* vertex space among workers. Each worker owns a contiguous span of destinations. For each owned destination \(v\), examine candidate sources (typically: neighbors of \(v\) that lie in the current frontier membership set) and apply the combine under exclusive ownership of \(v\)’s property cells.

Ownership implies that, within a single EdgeMap step, a destination’s property can be updated with ordinary stores rather than atomics, because no other worker writes that destination. Push still requires atomics (or equivalent) because many sources may race on the same destination.

**Cost heuristic (abstract).** Let \(W_{\text{push}}\) be the sum of out-degrees (or layout-equivalent row work) of vertices in \(F\), and let \(n = |V|\). Prefer pull when \(W_{\text{push}} > n\); otherwise prefer push. This is a *schedule choice*, not a change in mathematical EdgeMap semantics. The methodology treats schedule selection as part of the engine, not part of the motif.

### 2.6 Parallelism model assumed by motifs

Motifs assume:

- a fixed worker pool;
- DOALL over either frontier indices (push) or destination partitions (pull);
- per-worker **scratch lanes** for collecting activations, merged after the parallel region;
- a round-local **membership** structure for activate-once and for pull’s frontier filter;
- memory ordering sufficient for the chosen atomic primitives (relaxed CAS / fetch-add / atomic min are admissible for the combines we define).

Anything outside that model (arbitrary nested mutation, set-theoretic frontiers without array form, non-registered combines) is rejected from motif lowering.

---

## 3. Building-block motifs (the algebra)

Compositional matching is built from a small set of **primitive effects**. These are chosen because they (a) appear repeatedly across algorithms, (b) have clear concurrent implementations, and (c) can be composed without inventing per-algorithm engines.

### 3.1 Shared frontier header (not a combine; a shape)

Every matched EdgeMap region begins with the same **array-frontier header**:

1. Induction variable \(i\) and bound \(b = |F|\).
2. Loop condition \(i < b\).
3. Body of exactly three conceptual steps:
   - bind \(u \leftarrow F[i]\);
   - perform the **active body** (neighbor walk, or VertexMap-then-neighbor);
   - increment \(i\) by one.

This header is the *syntactic container* that makes a region an EdgeMap candidate. It does not itself update graph properties. Matching fails if the header is missing, if the induction is not a unit increment, if \(u\) is not read from the frontier at \(i\), or if extra statements pollute the three-step shape.

### 3.2 Primitive: FirstWins (CAS-first claim)

**Intent.** At most one concurrent updater may “claim” a destination vertex by changing a claim property from an expected value \(e\) to a desired value \(d \neq e\).

**Abstract rule.** On edge \((u,v)\):

\[
\texttt{FirstWins}(C, e, d):\quad
\textbf{if } C(v) = e\ \textbf{then}\ C(v) \leftarrow d\ \textbf{and return success}
\]

Concurrently this is a compare-and-swap on \(C(v)\). Success is exclusive: exactly one claimer wins.

**Typical side effect.** On success, write a secondary property such as parent \(\pi(v) \leftarrow u\), then activate \(v\).

**Why it is a building block.** BFS “visit once,” peel “remove once from alive,” and other exclusive claims are the same concurrent object. Separating FirstWins from “what else happens after the claim” is what allows peel to be VertexMap+EdgeMap rather than a BFS-shaped EdgeMap.

### 3.3 Primitive: RelaxMin (monotone min combine)

**Intent.** Destination property \(L(v)\) may only decrease; a candidate value \(c(u,v)\) is written if it is strictly smaller than the current \(L(v)\).

**Abstract rule.**

\[
\texttt{RelaxMin}(L, c):\quad
\textbf{if } c(u,v) < L(v)\ \textbf{then}\ L(v) \leftarrow c(u,v)\ \textbf{and return success}
\]

Two registered candidate families:

1. **Min-copy:** \(c(u,v) = L(u)\).  
   This is label propagation / hooking CC: “copy a better (smaller) label from \(u\) to \(v\).”

2. **Min-weighted:** \(c(u,v) = L(u) + w(u,v)\).  
   This is Bellman–Ford / Δ-less SSSP relaxation on integer weights.

**Why one primitive, two candidates.** The concurrent object is identical: atomic (or owner) minimum into \(L(v)\), then activate on success. The only difference is how the candidate is computed. Treating CC and SSSP as different *motifs* would duplicate push/pull, lanes, and activate-once; treating them as different *candidates under RelaxMin* is the compositional move.

### 3.4 Primitive: FetchAdd / decrement-with-threshold (degree peel)

**Intent.** On an edge to a still-alive neighbor, decrement residual degree; if the degree crosses below threshold \(k\), activate.

**Abstract rule** (destination-side):

\[
\begin{align*}
&d(v) \leftarrow d(v) - 1\\
&\textbf{if } d(v) < k\ \textbf{then activate-once}(v)
\end{align*}
\]

Concurrently the decrement is a fetch-and-add (or owned store in pull). The threshold test uses the post-decrement value.

**Why it is a building block.** It is not a min and not a claim; it is a commutative reduction on a counter plus a derived activation. Peel needs it; BFS/CC/SSSP do not.

### 3.5 Primitive: Activate-append

**Intent.** Record \(v\) in the next frontier exactly once this round.

**Abstract rule.**

\[
\texttt{Activate}(v):\quad
\textbf{if } v \notin F'_{\text{set}}\ \textbf{then}\ F'.\texttt{append}(v);\ F'_{\text{set}}.\texttt{insert}(v)
\]

In source programs this often appears as unconditional append on every successful update. The motif interprets that as activate-once.

### 3.6 Composite shape: Peel = VertexMap(FirstWins) then EdgeMap(FetchAdd)

Fixed-\(k\) peeling is intentionally **not** a single flat EdgeMap over the frontier. Its abstract structure is:

1. **VertexMap on frontier sources:** for each \(u \in F\), FirstWins on \(\texttt{alive}(u)\) from \(1\) to \(0\). Only the worker that successfully removes \(u\) proceeds.
2. **EdgeMap from removed \(u\):** for each neighbor \(v\) that is still alive, FetchAdd on \(d(v)\), and activate if \(d(v) < k\).

This is the clearest example of *composition* in the methodology: two operators sequenced, sharing the same frontier machinery and the same round’s next-frontier lanes. Flattening peel into “just another EdgeMap combine” is an implementation packaging choice; semantically it remains VertexMap; EdgeMap.

---

## 4. Effect summaries (what “matching” actually produces)

A successful match does not produce an algorithm name. It produces an **effect summary**: a finite record describing how the frontier region behaves.

### 4.1 Contents of an effect summary

At minimum:

| Field | Meaning |
|---|---|
| Combine class | FirstWins / Min-copy / Min-weighted / Peel(FetchAdd composite) |
| Graph identity | Which graph’s adjacency is traversed |
| Frontier / next frontier | Which arrays and length variables participate |
| Primary property | Claim array, label/distance array, or alive array |
| Secondary property | Parent array, degree array, or absent |
| Scalars | Expected/desired for FirstWins; threshold \(k\) for peel |
| Activation | Always-on-success vs threshold-gated |
| Candidate expression | For RelaxMin: copy vs \(L(u)+w(u,v)\) |

Optional / derived:

- whether weights are required;
- whether pull is admissible for this combine under the current layout model;
- metadata tags for debugging (combine name, graph name).

### 4.2 What an effect summary is *not*

It is not a residual program. After matching, the matched AST region is **replaced wholesale** by a call to the parametric engine configured by the summary. The original nested loops are not emitted “around” the engine.

### 4.3 Rejection

Matching is conservative. If any of the following hold, the region is **not** an EdgeMap effect and ordinary while/DOALL lowering applies:

- frontier header shape fails;
- neighbor enumeration is missing or not of the bound source vertex;
- extra statements appear in the then-block beyond the registered template;
- combine is not one of the registered classes;
- properties alias in illegal ways (e.g., frontier array used as the claim array);
- weighted relaxation appears without a recognizable weight expression;
- peel’s alive/degree/threshold structure is incomplete.

Rejection is part of the methodology: **unrecognized effects must not be forced into the engine**.

---

## 5. How composition is defined

“Composition” has three layers in this methodology. Confusing them is the main source of ambiguity, so they are separated explicitly.

### 5.1 Layer A — Algebraic composition of operators

At the semantic level, programs are compositions of VertexMap and EdgeMap:

- BFS level: \(\texttt{EdgeMap}(\texttt{FirstWins}+\texttt{parent}+\texttt{activate})\)
- CC round: \(\texttt{EdgeMap}(\texttt{RelaxMin}_{\text{copy}}+\texttt{activate})\)
- SSSP round: \(\texttt{EdgeMap}(\texttt{RelaxMin}_{\text{weighted}}+\texttt{activate})\)
- Peel round: \(\texttt{VertexMap}(\texttt{FirstWins}_{\text{alive}})\,;\,\texttt{EdgeMap}(\texttt{FetchAdd}_{\text{deg}}+\texttt{threshold activate})\)

This is the *meaning* of composition: sequencing and parameterization of a tiny operator algebra.

### 5.2 Layer B — Structural composition of AST shapes

At recognition time, composition means: **decompose a frontier-loop AST into the shared header plus a classified body**, then **summarize the body as an effect record**.

The body classification tree is:

```
frontier header?
  ├─ no → reject
  └─ yes → body shape?
        ├─ neighbor-foreach of u
        │     ├─ FirstWins then parent+activate     → CasFirst EdgeMap
        │     ├─ RelaxMin copy + activate           → MinCopy EdgeMap
        │     └─ nd = L[u]+w; RelaxMin nd + activate→ MinWeighted EdgeMap
        └─ FirstWins on u, then neighbor-foreach
              └─ alive-guarded FetchAdd + threshold → Peel (VertexMap; EdgeMap)
```

Order matters. FirstWins-shaped EdgeMap is tried before peel-shaped VertexMap+EdgeMap before RelaxMin, because the shapes are mutually exclusive in well-formed programs but overlapping prefixes exist (all start with the same header; peel starts with FirstWins on the source rather than on the neighbor).

### 5.3 Layer C — Implementation packaging into one engine entry

At execution time, composition means: **all of the above effect summaries are configurations of one runtime EdgeMap engine** (with peel packaged as a combine mode that internally performs the VertexMap phase then the EdgeMap phase). Push/pull, lanes, membership, CSR/PCSR/BCSR walks, and lane merge are *shared*; only the per-edge combine differs.

This is the operational payoff of the methodology: four algorithms, one engine, four combine IDs.

### 5.4 What composition is *not* (yet)

This milestone does **not** claim a free-form synthesizer that invents new combines from arbitrary statement lists. The registered combines are a closed set. “Effect-based” means: the *interface* to the engine is an effect/combine summary rather than an algorithm name; recognition may still use structural oracles that are known to be sound for those effects. Expanding the algebra (e.g., float sum for PageRank) is a future extension of the same methodology, not a different methodology.

---

## 6. Semantics of each registered composition

### 6.1 CasFirst EdgeMap (BFS-shaped)

**Logical step.** For each \(u \in F\), for each neighbor \(v\):

1. FirstWins on claim \(C(v): e \to d\).
2. On success: \(\pi(v) \leftarrow u\); activate-once \(v\).

**Concurrent meaning.** The claim array is the exclusive visit token. Parent is a race-free write under the claim (only the winner writes). Activation is once per destination per round.

**Schedule.** Push with CAS on claim; pull with owner store on claim when destination partitions are exclusive.

### 6.2 MinCopy EdgeMap (CC-shaped)

**Logical step.** For each \(u \in F\), for each neighbor \(v\):

1. If \(L(u) < L(v)\), set \(L(v) \leftarrow L(u)\).
2. On success: activate-once \(v\).

**Concurrent meaning.** Atomic/owner minimum with candidate \(L(u)\). Multiple decreases of \(L(v)\) across rounds are expected; within a round, activate-once collapses redundant appends.

### 6.3 MinWeighted EdgeMap (SSSP-shaped)

**Logical step.** For each \(u \in F\), for each neighbor \(v\):

1. Let \(c \leftarrow L(u) + w(u,v)\).
2. If \(c < L(v)\), set \(L(v) \leftarrow c\).
3. On success: activate-once \(v\).

**Concurrent meaning.** Same as MinCopy with a different candidate. Requires a weight oracle on edges. Pull may be disabled if weights are unavailable for the active layout.

### 6.4 Peel composition

**Logical step.** For each \(u \in F\):

1. VertexMap: FirstWins \(\texttt{alive}(u): 1 \to 0\). If fail, skip \(u\).
2. EdgeMap: for each neighbor \(v\) with \(\texttt{alive}(v)=1\):
   - \(d(v) \leftarrow d(v)-1\);
   - if \(d(v) < k\), activate-once \(v\).

**Concurrent meaning.** A vertex is removed at most once. Degree decrements from distinct removals commute. Threshold activation may fire from any decrement that crosses \(k\); activate-once prevents duplicate next-frontier entries.

**Pull packaging.** One admissible dense schedule marks removals into a membership structure, then lets destination owners apply degree updates for edges whose source was removed—still the same VertexMap; EdgeMap semantics.

---

## 7. After a composition explains a region

Once an effect summary successfully explains a frontier region, the compilation pipeline does the following, in order.

### 7.1 Do not emit the nested loops

The matched while-region is elided from ordinary IR generation. Emitting both the engine call and the original loops would duplicate work and break semantics.

### 7.2 Emit a single parametric operator call

The compiler emits one call to the EdgeMap engine, passing:

- the graph;
- the combine class (as an enumeration);
- frontier data pointer and length;
- next-frontier data pointer and initial length (usually zero at the start of a round);
- property pointers required by the combine;
- scalar parameters (expected/desired or \(k\)).

The return value is the new next-frontier length, which is stored back into the program’s length variable.

### 7.3 Preserve surrounding program structure

Everything outside the matched region remains ordinary IR:

- property initialization loops (possibly themselves DOALL’d by other passes);
- the outer `while (|F| > 0)` control;
- `swap(F, F')` and `|F| ← |F'|`;
- post-processing (counting reached vertices, printing distances, etc.).

Motif matching accelerates the *edge step*, not the entire algorithm as a named builtin.

### 7.4 Runtime interprets the combine

At run time, the engine:

1. validates arguments and graph metadata;
2. ensures per-worker scratch lanes and round membership storage;
3. chooses push or pull by the shared cost heuristic (with combine-specific restrictions);
4. executes the combine on every relevant edge under the chosen schedule;
5. merges worker lanes into the next frontier array;
6. returns the merged length.

### 7.5 Fallback path remains available

If matching fails, the same AST lowers to generic loops (and whatever DOALL outlining the rest of the compiler applies). Correctness of unmatched programs does not depend on the motif engine.

### 7.6 Observability

Successful matches may be tagged in the IR (combine name, graph name) so that binary inspection can confirm which combine was emitted. This is methodological hygiene: recognition claims must be auditable.

---

## 8. Correctness obligations

A composition is admissible only if:

1. **Sequential soundness:** on a single thread, the engine’s updates equal the AST’s updates up to activate-once collapsing of duplicate appends (which does not change reachability of subsequent rounds for the registered algorithms).
2. **Concurrent soundness:** races are confined to the registered atomic objects (CAS claim, atomic min, fetch-add degree); parent writes occur only under claim ownership; pull stores occur only under destination ownership.
3. **Schedule independence:** push and pull compute the same mathematical EdgeMap result for a given round’s inputs (same final properties and same activate-once set), modulo nondeterministic choice among equal-priority concurrent winners where the AST already admitted that nondeterminism (e.g., which parent wins a BFS race—FirstWins already fixes a winner; parent is tied to that winner).
4. **Rejection safety:** unmatched shapes never enter the engine.

---

## 9. Design rationale: why these building blocks and not others

- **FirstWins** is the exclusive “token” object behind visit-once and remove-once.
- **RelaxMin** is the monotone lattice update behind CC and SSSP; splitting copy vs weighted at the candidate level avoids duplicate engines.
- **FetchAdd+threshold** is the commutative counter update behind peeling.
- **Activate-once** is mandatory under parallelism for all of the above.
- **VertexMap before EdgeMap** is required for peel’s remove-then-decrement sequencing.
- **Closed combine set** keeps recognition decidable and runtime simple.
- **Shared push/pull engine** is where performance engineering concentrates, so algorithmic variety should appear as parameters, not as forked implementations.

Out of scope for this milestone, but compatible with the same theory: float summations (PageRank-style), explicit DSL operator syntax, and richer effect synthesis beyond the closed set.

---

# Part II — Realization in the SGPL Compiler and Runtime

This part maps every theoretical object above onto the concrete artifacts in `p1GraphEasy-con-AutoTuner`, without re-deriving the theory.

## 10. Where recognition lives

Recognition runs during IR generation of while-loops (`IRGenVisitor::visitWhile`). Before ordinary loop CFG is built, the compiler asks whether the while-body is a frontier EdgeMap effect. The analyzer returns either:

- a `FrontierEdgeMapEffect` record (combine enum + names of graph/frontier/properties/scalars), or
- empty, in which case the ordinary while lowering proceeds.

The effect record is the Part I “effect summary.” The combine enum is the Part I combine class.

## 11. Building blocks as detectors (oracles for effects)

The closed algebra is currently recognized by structural detectors that act as **sound oracles** for the registered effects. They are not the public API of the lowering—the public API is the effect record and the `autograph_edgemap` call—but they are how Layer B (AST composition) is implemented today.

### 11.1 Shared header

`parseArrayFrontierHeader` implements §3.1:

- condition `i < frontier_size`;
- body length 3;
- `u = frontier[i]`;
- `i = i + 1`.

BFS’s FirstWins detector inlines an equivalent header check; RelaxMin and Peel call the shared helper.

### 11.2 FirstWins primitive

`detectFirstWinsPattern` matches `if (claim[v] == expected) { claim[v] = desired; ... }` with `expected != desired`. This is the CAS-first claim of §3.2. It is reused in:

- BFS neighbor claim (`detectFirstWinsFrontierLoop`);
- peel source removal (`detectPeelKFrontierLoop`);
- and also in standalone conditionals elsewhere (local CAS emission), which is the same primitive outside EdgeMap.

### 11.3 CasFirst EdgeMap (BFS)

`detectFirstWinsFrontierLoop` matches:

```
while (i < frontier_size) {
  u = frontier[i];
  for each neighbor v of u in G {
    if (claim[v] == expected) {
      claim[v] = desired;
      parent[v] = u;
      next_frontier[next_size] = v;
      next_size = next_size + 1;
    }
  }
  i = i + 1;
}
```

This is §6.1. Concrete programs: `bfs_parent_graptor_compare.graph` (and variants), with `visited` as claim and `parent` as the secondary property.

### 11.4 RelaxMin EdgeMap (CC and SSSP)

`detectRelaxMinFrontierLoop` matches both candidates of §3.3:

**Min-copy (CC):**

```
for each neighbor v of u in G {
  if (id[u] < id[v]) {
    id[v] = id[u];
    next_frontier[next_size] = v;
    next_size = next_size + 1;
  }
}
```

**Min-weighted (SSSP):**

```
for each neighbor v of u in G {
  int nd = dist[u] + weight(G, u, v);
  if (nd < dist[v]) {
    dist[v] = nd;
    next_frontier[next_size] = v;
    next_size = next_size + 1;
  }
}
```

Helpers `matchesWeightCall` / `matchesPropPlusWeight` enforce the candidate expression. Concrete programs: `cc.graph`, `sssp.graph`.

### 11.5 Peel = VertexMap then EdgeMap

`detectPeelKFrontierLoop` matches §6.4:

```
u = frontier[i];
if (alive[u] == 1) {
  alive[u] = 0;                    // VertexMap FirstWins
  for each neighbor v of u in G {
    if (alive[v] == 1) {
      deg[v] = deg[v] - 1;         // FetchAdd
      if (deg[v] < k) {            // threshold activate
        next_frontier[next_size] = v;
        next_size = next_size + 1;
      }
    }
  }
}
```

Concrete program: `kcore_belgium.graph`.

### 11.6 Activate-append helper

`matchesActivateAppend` recognizes the two-statement append+increment pattern and binds next-frontier / next-size names. Activate-once itself is enforced in the runtime (§13), not by rewriting the AST to insert an explicit set.

## 12. How composition happens in the analyzer

`analyzeFrontierEdgeMap` is Layer B’s dispatcher:

1. Try CasFirst EdgeMap (`detectFirstWinsFrontierLoop`) → effect with `EdgeMapCombine::CasFirst`.
2. Else try Peel (`detectPeelKFrontierLoop`) → effect with `EdgeMapCombine::PeelK`.
3. Else try RelaxMin (`detectRelaxMinFrontierLoop`) → effect with `MinCopy` or `MinWeighted` according to the candidate.
4. Else return empty (reject).

Each successful branch **copies structural names into the effect record** (graph, frontiers, properties, scalars). Algorithm names never appear in the effect record—only combine classes and operands.

This is the concrete meaning of “composition” in the current milestone:

- Peel is recognized as a VertexMap-shaped prefix plus an EdgeMap-shaped suffix, then *packaged* as one peel combine for a single engine call.
- CC and SSSP are recognized as one RelaxMin shape with different candidates, then packaged as two combine IDs sharing one motif engine path.
- BFS is recognized as FirstWins+parent+activate, packaged as CasFirst.

The old mode-named motif API (`WRITE_MIN` / `PEEL_K` / `RELAX_MIN_WEIGHTED`) remains as a thin ABI wrapper over the same engine; IR emission prefers the compositional `autograph_edgemap` entry with combine enums `0/1/2/3`.

## 13. What we do after a composition matches

### 13.1 IR emission (`emitEdgeMap` inside `visitWhile`)

On a non-empty effect:

1. Load the graph pointer and the frontier / next-frontier / property array data pointers.
2. Load frontier length and initial next size from the program’s length variables.
3. For peel, load \(k\) from the named scalar; for CasFirst, pass expected/desired literals.
4. Emit a call to `autograph_edgemap(graph, combine, frontier, |F|, next, |F'|_0, prop0, prop1, scalar0, scalar1)`.
5. Store the returned length into the next-size variable.
6. Attach metadata `sgpl.edgemap.step` with graph name and combine name (`cas_first`, `min_copy`, `min_weighted`, `peel_k`) for auditability.
7. **Return** from `visitWhile` without building the ordinary loop CFG for that region.

Surrounding code (outer while, swap, init) is emitted as usual by other visits.

### 13.2 Runtime engine (`autograph_edgemap`)

The C runtime implements Layer C:

| Combine ID | Name | Internal path |
|---|---|---|
| 0 | `SGPL_COMBINE_CAS_FIRST` | CasFirst push/pull engine (claim+parent) |
| 1 | `SGPL_COMBINE_MIN_COPY` | Motif engine in write-min mode |
| 2 | `SGPL_COMBINE_PEEL_K` | Motif engine in peel mode (VertexMap then EdgeMap) |
| 3 | `SGPL_COMBINE_MIN_WEIGHTED` | Motif engine in weighted relax mode |

Shared across paths:

- scratch lane allocation and reset;
- round-member activate-once bits;
- `autograph_should_use_pull`: prefer pull when push edge work \(> n\) (overridable by `SGPL_FRONTIER_MODE`);
- CSR / PCSR / BCSR neighbor walks without materializing neighbor arrays;
- lane merge into the next frontier.

CasFirst path: CAS publish on push; owner store on pull partitions.  
RelaxMin path: atomic/owner min with copy or \(L[u]+w\) candidate; activate-once on success.  
Peel path: CAS alive \(1\to0\) per source; fetch-add / owned decrements on degrees; threshold activate; dense peel uses a mark-removed then owner-partition sequence.

Thin wrappers:

- `autograph_frontier_step` → EdgeMap CasFirst;
- `autograph_motif_frontier_step(mode)` → EdgeMap with mapped combine.

These preserve ABI during the transition; new IR does not need them.

### 13.3 If composition does not match

`analyzeFrontierEdgeMap` returns empty. `visitWhile` builds the standard loop basic blocks and may later be subject to generic DOALL outlining. No EdgeMap call is emitted. This is the Part I rejection path.

## 14. End-to-end example (theory ↔ code)

### 14.1 SSSP round

**Theory.** Frontier header + EdgeMap(RelaxMin with candidate \(L(u)+w(u,v)\)) + activate-once.

**Code recognition.** `detectRelaxMinFrontierLoop` sees the `nd` declaration, weight call, `nd < dist[v]`, store, append. `analyzeFrontierEdgeMap` sets `MinWeighted`. `emitEdgeMap` calls `autograph_edgemap(..., combine=3, prop0=dist, ...)`.

**Runtime.** Motif engine weighted mode; push (or pull if weights and heuristic allow); lane merge; return new frontier size.

### 14.2 Peel round

**Theory.** VertexMap(FirstWins alive) sequenced with EdgeMap(FetchAdd deg + threshold activate).

**Code recognition.** `detectPeelKFrontierLoop` requires FirstWins on the source *before* the neighbor loop—the compositional tell. Effect combine `PeelK` with `prop0=alive`, `prop1=deg`, `scalar0=k`.

**Runtime.** One `autograph_edgemap` call performs both phases internally; IR does not emit two calls. That packaging is an engineering choice; the semantic composition remains two operators.

### 14.3 BFS round

**Theory.** EdgeMap(FirstWins + parent + activate).

**Code recognition.** CasFirst effect with claim/parent/expected/desired. Runtime CasFirst path with push/pull duality identical in spirit to the motif path but specialized to the claim object.

## 15. Testing and audit obligations (methodological)

Methodology requires evidence, not only design:

- Unit tests that exercise EdgeMap directly with MinCopy and CasFirst, and the frontier motif / owner-pull matrices at multiple thread counts.
- Smoke correctness of rebuilt BFS / CC / k-core / SSSP programs after relinking against the unified engine.
- Binary audit (`nm` / `objdump`) confirming `autograph_edgemap` and the expected combine immediates (0/1/2/3).
- Benchmarks reported as measured against prior SGPL motif baselines and Graptor references, without Graptor-targeted tuning as a success criterion.

## 16. Summary

**Theoretically:** frontier algorithms are compositions of a tiny operator algebra (VertexMap, EdgeMap) parameterized by a closed set of combines (FirstWins, RelaxMin with two candidates, FetchAdd+threshold) plus activate-once, executed under dual push/pull schedules of one EdgeMap semantics.

**In code:** structural oracles recognize those compositions in while-loop ASTs, summarize them as `FrontierEdgeMapEffect` records, replace the region with a single `autograph_edgemap` call, and execute through one parametric runtime engine. Unrecognized regions fall back to ordinary lowering. Algorithm names are not the IR interface—combine effects are.

That is the methodology of compositional motif matching in SGPL for this milestone.
