# CleanCut Loop Parallelisation Spec — GraphEasy AutoTuner

## 1. Which loops we tackle

**Graph loop (the only unit considered).** Any loop, anywhere in the IR, whose
body uses the graph iterator and is structurally the canonical iterator shape:

*inner loop*: exit condition is `autograph_neighbor_iter_next(&iter, &v) != 0`
(an `ICmp` whose operand is the iterator-next call), nested inside a *driver
loop* (its parent loop), with a matching
`autograph_neighbor_iter_init(graph, u, &iter)` somewhere in the same function
that feeds the same iterator pointer.

Naming in analysis:
- **driver axis** `u` — the value passed to `init(..., u, ...)`; its induction
  variable is the driver header phi.
- **walked neighbor** `v` — the loop variable slot the iterator writes.
- **per-pair body** — inner-loop blocks minus its header; every invocation
  corresponds to exactly one walked edge `(u, v)`.

**Graph-construct write (iterator-less loops).** A loop is *also* a graph loop —
even with no iterator call — if its body contains a **graph-data write**: a store
whose subscript is load-derived from a graph data array element (e.g.
`next_rank[nodes[i]]`, `perm[v]`, `deg[v]`). Structurally: some `load` in the
index def-chain has a *GetElementPtr* pointer (an array element), which excludes
the scalar iterator v-slot (`load i32, ptr %v_alloca`, no element GEP). All
`n`-sized data arrays qualify (vertex, edge, and property arrays alike). Such
loops are conservatively marked **sequential** (never handed to the racy DOALL
path); see 2 (WriteData) and 3.5.

Non-graph loops (neither an iterator shape nor a graph-data write) are **not**
touched by this machinery; the existing PDG/outliner paths apply as before.

## 2. Criteria for loop parallelisation

Classification is structural — the write pattern against the walked edge's
endpoints. For every store in the per-pair body, decide the index origin:

| write target | origin | judgement |
|---|---|---|
| array indexed by `v` (neighbor) | `derivedFrom(idx, v-slot-load)` | **dest-indexed** |
| array indexed by `u` (axis) | `derivedFrom(idx, driver indvar)` | **src/axis-indexed** |
| scalar / array indexed by neither | — | **reduction candidate** |
| both dest- and src-indexed writes | both | **mixed → sequential** |
| array indexed by graph data | `isGraphDataIndex(idx)` (load of an array element in the def-chain) | **WriteData → sequential** |
| unclassifiable index | — | **unknown → sequential** |

**Read/write-overlap rule (round-separation criterion).** If a vertex array
(base loaded in the body) is *both read and written* in the same round (in-place
update, e.g. `dist[v] = min(dist[v], ...)`), ownership alone is not enough:
reads must resolve against the previous round's snapshot.

**Decision table.**

| body pattern | decision | technique |
|---|---|---|
| dest-indexed write only | parallel | owner-computes **push** (partition by destination) |
| src/axis-indexed write only | parallel | **source-owned** traversal (partition by source range) |
| reduction (scalar / third array) | parallel | **per-partition partials + ordered combine** |
| read∩write overlap (in-place) | parallel if we emit round separation; otherwise conservative **sequential** | round-sep double-buffer (emission in progress); seq fallback today |
| graph-data-indexed write (`a[nodes[i]]`, iterator or hand-rolled) | **sequential, never DOALL** | conservative sequential mark |
| mixed (dest+src/unknown) | **sequential, never DOALL** | conservative sequential mark |

## 3. Parallelisation techniques

### 3.1 Owner-computes CleanCut push (dest-owned) — `autograph_frontier_step_owner_push`
- `autograph_build_clean_cut(graph, partitions)`: O(E), idempotent. Each
  partition owns a contiguous **destination** range; all edges landing in it are
  grouped by source in `push_rp/ci/indir` (with per-partition source
  deduplication).
- **Layout-native**: the partition build enumerates the arcs from whatever
  layout the AutoTuner picked — CSR rows zero-copy; PCSR rows with GAP
  sentinel slots skipped; BCSR block-decoded; SET walks the static base
  pairs (raw order). **No conversion to CSR anywhere in the CleanCut path**
  (the `convert_*` functions exist only in the runtime's own layout-conversion
  machinery). CSR/PCSR/BCSR are u-major → bit-identical partitions; SET is
  deterministic (fixed pair order).
- One worker per partition, scanning its own rows → every write targets a
  home-owned destination → **no atomics, race-free for arbitrary per-pair
  work**. Source order within partition = serial order → bit-exact.
- Used for: `next_rank[v] += f(u,v)`-style accumulations.

### 3.2 Source-owned traversal — `autograph_frontier_step_owner_source`
- Partition p owns source range `[p·n/P, (p+1)·n/P)`; the build stores
  per-partition **flat pair lists** (`src_pairs[p] = [(u,v),...]`,
  `src_pair_count[p]`) taken from the same layout-native enumeration as the
  push (3.1), so the step never touches the transient layout directly —
  correct under any picked layout.
- Makes **source-indexed** writes (`out_degree[u]++`, u-owned state)
  race-free; exact serial order (u-major for CSR/PCSR/BCSR, raw for SET) —
  used by the degree/count loops.

### 3.3 Per-partition partials + combine — `autograph_frontier_step_owner_red`
- Each partition accumulates into its own partial slot (`red_partials[P]`),
  work fn writes to its env partial; after the parallel loop the runtime
  combines in partition order into the original target.
- Deterministic and bit-exact (partition order == serial source order).
- Used for scalar/third-array reductions (`sum += f(u,v)` etc.).

### 3.4 Round separation (double-buffer) — in progress
- Read/write-overlap arrays: reads from the previous round snapshot, writes
  into the round target, swap at round end (kernel-internal swap-copy loop is
  reused). Detection (`NeedsRoundSep`) is live; emission is the next step
  (required for sssp in-place `dist`).

### 3.5 Conservative sequential (never DOALL) — `sgpl.frontier.nested.sequential`
- `markSequential` applies the metadata to the loop, **all ancestors and every
  subloop** of the round nest.
- PDG classifier (`pdg.cpp`) reads it → hard `SEQUENTIAL`; the outliner's
  `parseParallelMode` never outlines it; `reconstructParallelIR` **leaves the
  nest inline** (no `task_N()` split — the dedicated skip prevents both the
  value loss and the dangling-`Loop*` deref).
- **Motif engine is untouched** — it already had its chance; it may still
  parallelise such loops safely.
- **Graph-data writes** (`WriteData`) route here too: `isGraphDataIndex` flags any
  store whose subscript is load-derived from a graph data array; iterator loops
  with such a write are forced sequential (not rewritten), and iterator-less
  loops carrying one (`loopHasGraphDataWrite`) are detected and marked sequential
  directly — closing the racy-DOALL hole for hand-rolled `a[nodes[i]]` shapes.

### 3.6 Emission (pass `graph_frontier_lowering.cpp`)
- Runs **before** the AutoTuner region pass and the PDG/outliner.
- Per-pair body: **manual clone** into `sgpl_frontier_pair_fn(src, dst, idx,
  env)` — no CodeExtractor (single-block bodies and reshape issues).
  Rebindings: `v`-slot ← `dst`; `u`-derived chains (driver indvar) ← `src`;
  globals direct; cross-region phis via slot fallback.
- Build+step calls at the driver preheader; **fresh `load` of the graph
  global** at the call site (the init's graph load lives in the soon-dead
  driver body and dead-value replacement turns it into poison).
- Old driver nest deactivated (header true-edge → exit) → dead, DCE removes it.
- **Enabled by default** (no env gate): every graph loop is lowered or marked
  sequential. `GRAPH_FRONTIER_REWRITE_OFF=1` restores the old pipeline
  (graph loops then still get the unconditional sequential marking).
- **AutoTuner dual annotation**: the AutoTuner pass (`AutoTunerPass.cpp`)
  annotates both IR versions — the (dead) loop regions keep their events, and
  each CleanCut step call gets a paired Traverse region (same graph, same
  `totalOps` prediction) with `profile_region_enter` before and
  `profile_region_exit` immediately after the call, so predicted-vs-measured
  and layout decisions cover the executed kernel. Layout conversions are safe
  for CleanCut graphs because the partitions are layout-agnostic (3.1/3.2).

### 3.7 Envelope (frontier semantics) — active
Frontier-set loops (the motif shapes) flow through the effect system:
- `foreach element u in frontier`-style round loops with `setSize`/`next` swaps
  are recognized structurally (`MembershipGated`, `FrontierSetPtr/NextSetPtr`,
  `Activate` frontier appends, `envelopeWired`).
- **sssp/cc (relax-min/min-copy)**: `DestOwner` + `PreviousRoundRead` — the
  frontier round boundary makes the cross-endpoint read safe (monotone min
  semantics are additionally self-healing for same-round re-relax races).
- **k-core/peel**: `DualOwner` — per round: source phase (alive claims /
  U-preamble incl. first-wins) then dest phase (degree updates), disjoint
  bases, staged by the round boundary; peel rounds are round-separated by
  construction (the compiler inserts the explicit peel skeleton when a flat
  kernel is encountered).
- **BFS/claims**: `Claim`/first-wins effects; under the effect system claims
  lower to plain stores (CleanCut ownership serializes each source), no CAS
  needed.
- **Compiler motif engine gated off by default**: the front-end edge-map
  dispatch (`analyzeFrontierEdgeMap`: CAS_FIRST/MIN_COPY/PEEL_K/MIN_WEIGHTED
  -> `autograph_edgemap`) is disabled (`GRAPH_FRONTIER_REWRITE_OFF=1`
  restores it) so these loops emit ordinary IR and are lowered by the effect
  system. Front-end motif syntax (`motifs X = [G where ...]`) is untouched.
- The four acceptance kernels (`small_sssp/cc/kcore/bfs.graph`) classify
  parallel and produce bit-identical results vs the motif path (12T = 1T).

## 4. Coverage matrix

| program | loop shape | class | technique |
|---|---|---|---|
| pagerank | degree count | src-indexed (u) | source-owned (3.2) |
| pagerank | rank accumulation | dest-indexed (v) | dest push (3.1) |
| sssp (in-place) | relax | read∩write | round-sep (3.4) / seq fallback (3.5) |
| cc / kcore / bfs | detect + classify | per-case | per decision table |

## 5. Enforcement summary

1. Detect graph loop (structural). 2. Classify write pattern (u/y/other/overlap/
   mixed). 3. Rewrite to matching CleanCut step (3.1/3.2/3.3/3.4) or mark
   sequential (3.5). 4. A graph loop is **never** handed blind to the racy
   DOALL/outliner path.
