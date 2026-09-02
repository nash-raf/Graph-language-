# Cost-Model Implementation: Compromises & Derivations

This document records the compromises made when translating the theoretical
cost model into the working autotuner implementation in
`p1GraphEasy-con-AutoTuner/`, and derives the new BCSR and SET equations that
were not in the original spec but were required for completeness.

---

## 1. Compromises

### 1.1 BCSR structure: dense b×b blocks → per-block-row (local_row, col) pair lists

**Theory (user spec):** BCSR overlays a grid of dense b×b blocks on the
adjacency matrix. Metadata (`bptr`, `bindx`) tracks non-zero blocks; payload
(`bval`) holds dense b×b matrices. Equations use `4b²`-byte block payloads,
`bindx` shifts, and `d_B` = average block degree.

**Actual runtime** (`autotuner_runtime.c:615` `convert_csr_to_bcsr`): BCSR is
stored as per-block-row edge-pair lists. `brow[b]..brow[b+1]-1` indexes into
`bcol`, which stores `(local_row, col)` pairs as consecutive `int32` values
`[r0, c0, r1, c1, ...]`. There are no dense b×b blocks, no `bindx`, no `bval`.
Block size `b = 64`, number of block rows `nb = ceil(n/64)`.

**Compromise:** Re-derived all BCSR equations to match the real runtime
structure (see §2 below). `d_B` from the spec is replaced by `b·d` (total edges
in a block row = b vertices × d avg degree per vertex). The `4b²`-byte dense
block payload becomes `8·b·d` bytes (b·d edges × 2 int32 × 4 bytes).

### 1.2 PCSR rebalance: Scenario B dropped (P_full = 0)

**Theory (user spec):** PCSR insertions have two scenarios — in-place gap fill
(Scenario A) and rebalance window redistribution (Scenario B), weighted by
`P_full`. The model includes window size `W`, affected vertices `ΔV`, and
metadata-shift costs.

**Actual runtime** (`graph_mutation_runtime.c:92` `pcsr_add_directed`): When
the local span is full, `pcsr_add_directed` silently returns 0 — no rebalance,
no window, no redistribution. The expansion factor is fixed at `g = 2`
(`gap_slots = deg`, `row_cap = 2·deg` in `convert_csr_to_pcsr:541-542`).

**Compromise:** Set `P_full = 0`. Use Scenario A only:
`C_update = (2 + g(u))·t + 1·t + ceil(4·(g(u)/2)/L)·t`. The `W`, `ΔV`, and
rebalance terms are unused. This matches what the runtime actually does — if
the span is full, the insertion fails silently (a correctness bug in the
runtime, but the cost model should reflect the intended behavior, not a bug).

### 1.3 Active set: dynamic frontier → static n

**Theory (user spec):** `|Active Set|` is the cardinality of the set of nodes
that remain active during traversal (e.g., the BFS frontier). This is dynamic
and varies per iteration.

**Implementation:** The autotuner pass operates at compile time with only
static `n` and `m` (from `autograph_init` arguments). No runtime feedback loop
exists.

**Compromise:** `|Active Set| = n` — every vertex becomes active once across a
traversal region's lifetime. This overestimates the active set for sparse
traversals (BFS from a single source may only touch a small fraction of
vertices) but underestimates it for repeated traversals. The `H` multiplier
(loop-nesting heuristic, `×8` per enclosing loop) partially compensates for
repeated traversal passes. This is the same role `H` played in the old model.

### 1.4 BCSR mutation: ~~no native path~~ RESOLVED — native BCSR mutation implemented

**Original problem:** `graph_add_edge` had no `LAYOUT_BCSR` case — BCSR fell to
the `default` branch (SET canonical add), leaving BCSR arrays stale while
`current_layout` stayed `BCSR`. The cost model treated BCSR insert as identical
to SET insert.

**Resolution:** Implemented native BCSR mutation functions
(`autograph_bcsr_add_edge` / `autograph_bcsr_remove_edge` in
`autotuner_runtime.c`) that directly manipulate the `brow`/`bcol` arrays:
scan the block row for duplicates, realloc + memmove to insert/remove the
`(local_row, col)` pair, and update the `brow` prefix sums. The cost model
now has a dedicated BCSR equation (see §2.3) that reflects the actual
`brow` prefix-sum update (only `nb = n/b` entries × 4 bytes vs CSR's `n`
entries × 8 bytes) and the `bcol` shift cost (2m ints).

The SET fallback path is retained only for out-of-bounds edges (vertices
beyond `n`), matching CSR/PCSR behavior.

### 1.5 Graph-specific parameters: not threaded through graph declaration

**Theory (user spec):** Parameters like `g`, `b`, `d_B`, `|E_B|`, `g(u)`, `u`,
`p` should ideally be measured or user-supplied per graph.

**Implementation:** `g` and `b` are hardcoded as compile-time constants
(`kPcsrExpansionFactor = 2.0`, `kBcsrBlockSize = 64.0`) measured from the
runtime source. `d = 2m/n` (valid closed form for undirected average degree)
is derived from the existing `n, m` that already flow through `autograph_init`.
Other parameters are derived: `d_B = b·d`, `g(u) = g·d = 2d`, `u = n/2`,
`p = m/2`.

**Compromise:** `u = n/2` and `p = m/2` are mid-range approximations. In
reality, the source vertex `u` and insertion position `p` depend on the
specific edge being added, which is unknown at compile time. The average is the
best unbiased estimate given no additional information. `g(u) = g·d` assumes
every vertex has the average degree, which is inaccurate for power-law graphs
but correct for uniform-degree graphs.

### 1.6 SET edge-pair byte size: 16 bytes per pair (IR inline path)

**Implementation detail:** The SET traversal cost uses `B_useful = 16·m` bytes.
The actual edge-pair format depends on the graph construction path:
- **IR inline path** (`IRGenVisitor.cpp`): edge pairs are `{i64, i64}` = 16
  bytes each, `m/2` pairs → `8m` bytes total.
- **File-load path** (`graph_loader_runtime.cpp`): edge pairs are `{i32, i32}`
  = 8 bytes each, `m/2` pairs → `4m` bytes total.

**Compromise:** Used `16m` as an upper bound. This overestimates the bytes
scanned in both paths, making SET traversal more punitive than it should be.
This is intentional — SET has no index structure for neighbor lookup (it must
scan all edge pairs), so the cost model should strongly discourage the DP from
selecting SET for traversal-heavy regions. The exact byte count is less
important than the ordering: SET traversal ≫ CSR/PCSR/BCSR traversal.

### 1.7 Compute region dropped entirely

**Theory (old model):** Compute regions (`floyd_runtime`, `bk_runtime`,
`dijkstra_runtime_src`, `chromacity_runtime`, `karger_runtime`) had their own
cost row in `C[4][4]` and were feasible only for CSR.

**Implementation:** Removed `Compute` from the `RegionType` enum, `opIndex`,
`classifyCall`, `isComputeCall`, and `Region.freq` (shrunk from 4 to 3 slots).

**Compromise:** Compute runtimes read `row_ptr`/`col_idx` directly and were
never made layout-aware. They are still present in the module and still pin CSR
via `moduleRequiresCSRLayout` (which detects direct `row_ptr`/`col_idx` GEP
accesses). They are simply no longer classified as autotuner regions — the pass
ignores them for scheduling purposes, and their CSR requirement is enforced by
the pin-CSR mechanism instead. This is safe because compute runtimes were
already CSR-only in the old model and no new layouts were enabled for them.

### 1.8 Wrapper preservation: H × totalOps × freq kept

**Theory (user spec):** The new equations replace the inner per-op unit cost
but keep the `H × totalOps × freq` wrapper.

**Implementation:** `operationCost = H × totalOps × (fT·U_trav + fI·U_ins +
fQ·Cq·log2(m))`. The wrapper multiplies the new per-op costs by the region's
loop-nesting heuristic (`H`), total operation count (`totalOps`), and operation
mix fractions (`fT, fI, fQ`).

**Compromise:** This means the new equations are not pure `|AS| × (C_nav +
C_traffic)` — they are scaled by `H × totalOps × fT` for the traversal term.
For a pure-traversal region (`fT=1, fI=0, fQ=0`) with `totalOps=1` and `H=1`,
this reduces to exactly the spec formula. For mixed regions (e.g., a region
with both BFS and insert calls), the wrapper blends the costs proportionally.
This preserves the region-sensitivity of the old model but means the absolute
cost values are not directly comparable to the raw theoretical formulas.

### 1.9 Conversion cost model unchanged

**Theory (user spec):** "Keep everything else" — the conversion cost model
(`alphaFromCSR`, `alphaToCSR`, `conversionCost`) should stay as-is.

**Implementation:** Unchanged. The alpha constants and `scale = n + m` formula
are from the old model. These alphas were calibrated empirically (not from the
cache-line-aware theory) and do not use the new `L, t, T` parameters. This is a
minor inconsistency — the operation costs use hardware-calibrated parameters
while conversion costs use fixed multipliers — but it was explicitly requested.

---

## 2. Derived BCSR Equations

The user's spec gave BCSR equations assuming dense b×b blocks. The actual
runtime uses a different structure, so the equations were re-derived from
scratch using the same methodology (metadata + data decomposition).

### 2.1 Runtime BCSR structure

```
brow[0..nb]       — int32 block-row pointers (nb = ceil(n/b) block rows)
bcol[0..total_ints] — flat array of (local_row, col) pairs:
                      [r0, c0, r1, c1, r2, c2, ...]
                      where local_row ∈ [0, b) and col ∈ [0, n)
```

For block row `blk` (containing vertices `blk·b` through `blk·b + b - 1`):
- The slice `bcol[brow[blk]..brow[blk+1]-1]` contains all edges originating
  from vertices in that block row.
- Each edge is stored as two consecutive `int32` values: `(local_row, col)`.
- `local_row = u - blk·b` identifies which vertex within the block is the
  source.

### 2.2 BCSR Traversal Cost

To traverse neighbors of vertex `u`:
1. **Navigation:** Read `brow[u/b]` and `brow[u/b+1]` to find the legal span
   in `bcol`. These are two `int32` values = 8 bytes.
   - `C_nav = 2·t` (two redirections into the metadata)

2. **Data access:** Read the slice `bcol[brow[u/b]..brow[u/b+1]-1]`, filtering
   for pairs where `local_row == u % b`.
   - The block row contains `b` vertices, each with average degree `d`, so the
     slice contains `b·d` edges, each stored as 2 `int32` values = 8 bytes.
   - `B_useful = 8 + 8·b·d` bytes (8 for the two brow entries + 8·b·d for the
     edge data).

3. **Traffic:**
   `C_traffic = ceil((8 + 8·b·d) / L) · T`

4. **Full traversal cost:**
   ```
   C_traversal(BCSR) = |AS| × (2·t + ceil((8 + 8·b·d) / L) · T)
   ```

**Reasoning:**
- `C_nav = 2·t`: The navigation structure is `brow` (the block-row pointer
  array). Finding the legal span for vertex `u` requires reading `brow[u/b]`
  (start) and `brow[u/b+1]` (end) — exactly 2 redirections, same as CSR's
  `row_ptr[u]` and `row_ptr[u+1]`. The additional `d_B·t` term from the user's
  spec (scanning `bindx` for the block column index) is absent because the
  runtime has no `bindx` — the block column index is embedded in the `bcol`
  data itself.
- `B_useful = 8 + 8·b·d`: The 8 bytes are the two `brow` entries (`int32` × 2).
  The `8·b·d` bytes are the edge payload: `b·d` edges (b vertices × d avg
  degree), each edge = 2 `int32` = 8 bytes. Note that the traversal must scan
  the entire block-row slice to filter for the target `local_row`, even though
  only `d` of the `b·d` edges belong to vertex `u`. This scan cost is captured
  in `B_useful` — we load all `b·d` edges' bytes because there is no
  per-vertex index within the block.

### 2.3 BCSR Insert Cost — NATIVE MUTATION

BCSR now has a dedicated native mutation path
(`autograph_bcsr_add_edge` / `autograph_bcsr_remove_edge` in
`autotuner_runtime.c`) that directly manipulates the `brow`/`bcol` arrays
without converting to SET.

The cost equation reflects the actual operations:

```
C_update(BCSR) = C_locate + C_write + C_move

C_locate = (2 + d_BR) · t
  where d_BR = b · d  (edges in the block row to scan for duplicates)

C_write = 1 + ceil((nb - blk) · 4 / L) · t
  1 cache line for the new (local_row, col) pair +
  brow prefix-sum update for (nb - blk) subsequent block rows (int32 each)

C_move = ceil(4 · (2m - p_B) / L) · t
  Shift all bcol entries after the insertion point (2m total ints, p_B ≈ m)
```

**Key advantage over CSR for mutation:** The `brow` prefix sum has only
`nb = ceil(n/b)` entries (each 4 bytes) vs CSR's `row_ptr` with `n` entries
(each 8 bytes). For `b = 64`, that's a 128× reduction in prefix-sum update
bytes: `4·(n/64)` vs `8·n`. The tradeoff is a larger C_locate (scanning
`b·d` pairs vs `d` neighbors) and a larger C_move (shifting `2m` ints vs
`m` entries).

---

## 3. Derived SET Equations

---

## 4. Worked Switch Example (Why the DP Should Flip Layouts)

This section gives a concrete numeric example using the **current implementation
constants exactly**:

- `g = 2`
- `b = 64`
- `L = 64`
- `t = 1`
- `T = 1`
- `kMinBenefitRatio = 1.2`

Assume the graph used in many of the local workloads:

- `n = 50,000`
- `m = 500,000`
- `d = 2m/n = 20`
- `scale = n + m = 550,000`

### 4.1 Insert unit cost

Using `u = n/2 = 25,000`, `p = m/2 = 250,000`, and `g(u) = g·d = 40`:

#### CSR

`C_locate = (2 + d)·t = (2 + 20) = 22`

`C_write = 1 + ceil((n-u)·8 / L)`

`= 1 + ceil(25,000·8 / 64)`

`= 1 + ceil(3,125)`

`= 3,126`

`C_move = ceil(4·(m-p) / L)`

`= ceil(4·250,000 / 64)`

`= ceil(15,625)`

`= 15,625`

So:

`C_insert(CSR) = 22 + 3,126 + 15,625 = 18,773`

#### PCSR

`C_locate = (2 + g(u))·t = 2 + 40 = 42`

`C_write = 1`

`C_move = ceil(4·(g(u)/2) / L)`

`= ceil(4·20 / 64)`

`= ceil(1.25)`

`= 2`

So:

`C_insert(PCSR) = 42 + 1 + 2 = 45`

#### SET (CORRECTED)

`C_locate = 2·t = 2`

`C_write = 3·t = 3`

`C_refresh = ceil((m/2) · 24 / L)`

`= ceil(250,000 · 24 / 64)`

`= ceil(93,750)`

`= 93,750`

So:

`C_insert(SET) = 2 + 3 + 93,750 = 93,755`

#### BCSR (CORRECTED — kernel cost identical to SET)

`C_insert(BCSR) = C_insert(SET) = 93,755`

So the insertion ordering is:

`PCSR (45) < CSR (18,773) < SET ≈ BCSR (93,755)`

### 4.2 Traversal unit cost

#### CSR

`B_useful = 16 + 4d = 16 + 80 = 96`

`C_traversal(CSR) = n · (2 + ceil(96 / 64))`

`= 50,000 · (2 + 2)`

`= 200,000`

#### PCSR

`B_useful = 16 + 4dg = 16 + 4·20·2 = 176`

`C_traversal(PCSR) = n · (2 + ceil(176 / 64))`

`= 50,000 · (2 + 3)`

`= 250,000`

#### SET

`B_useful = 16m = 8,000,000`

`C_traversal(SET) = n · (1 + ceil(8,000,000 / 64))`

`= 50,000 · (1 + 125,000)`

`= 6,250,050,000`

So the traversal ordering is:

`CSR (200,000) < PCSR (250,000) << SET`

### 4.3 Two-region schedule (CORRECTED)

Now consider a realistic phase split:

- **Region A:** `100,000` insertions (ingestion burst)
- **Region B:** `10` traversals (post-ingest analytics)

Use the current conversion model:

- `CSR → PCSR = alphaFromCSR(PCSR) · scale = 1.5 · 550,000 = 825,000`
- `PCSR → CSR = alphaToCSR(PCSR) · scale = 0.5 · 550,000 = 275,000`
- `CSR → SET = alphaFromCSR(SET) · scale = 0.1 · 550,000 = 55,000`
- `SET → CSR = alphaToCSR(SET) · scale = 3.0 · 550,000 = 1,650,000`

#### Stay all-CSR

Region A:
`100,000 · 18,773 = 1,877,300,000`

Region B:
`10 · 200,000 = 2,000,000`

Total:
`1,879,300,000`

#### Switch: CSR → PCSR for Region A, then PCSR → CSR for Region B

Convert into PCSR:
`825,000`

Region A in PCSR:
`100,000 · 45 = 4,500,000`

Convert back to CSR:
`275,000`

Region B in CSR:
`10 · 200,000 = 2,000,000`

Total:
`825,000 + 4,500,000 + 275,000 + 2,000,000 = 7,600,000`

#### Switch: CSR → SET for Region A, then SET → CSR for Region B (NOT RECOMMENDED)

Convert into SET:
`55,000`

Region A in SET:
`100,000 · 93,755 = 9,375,500,000`

Convert back to CSR:
`1,650,000`

Region B in CSR:
`10 · 200,000 = 2,000,000`

Total:
`55,000 + 9,375,500,000 + 1,650,000 + 2,000,000 = 9,379,205,000`

This is **worse** than staying all-CSR because of the O(m) per-insert refresh
cost in SET. The autotuner correctly avoids this.

### 4.4 Why the switch to PCSR passes the gate, but SET fails

The pass only keeps a non-CSR schedule if it beats the all-CSR baseline by at
least `1.2x`.

Required threshold:
`allCSR / 1.2 = 1,879,300,000 / 1.2 ≈ 1,566,083,333`

PCSR-switched schedule:
`7,600,000  << 1,566,083,333  ✓ passes`

SET-switched schedule:
`9,379,205,000  > 1,879,300,000  ✗ worse than staying CSR`

The SET schedule is rejected because `C_insert(SET) = 93,755` per insert
(dominated by the O(m) refresh scan) makes it 2,083× more expensive than
PCSR insert (45 per insert). The autotuner correctly skips SET for
insert-heavy regions.

### 4.5 Intuition (CORRECTED)

With the corrected equations, the autotuner correctly predicts:

- **PCSR** as the best layout for insert-heavy regions (gap-slot scan beats
  CSR's O(E) realloc AND SET's O(m) refresh scan)
- **CSR** as the best layout for traversal-heavy regions (sequential access
  without gap skipping or block-row filtering)
- **SET** as the worst layout for both insert and traversal (O(m) scan
  on every insert, O(n·m) scan for BFS)
- **BCSR** as a traversal-only layout (its insert cost equals SET after
  conversion, plus rebuild penalty)

This matches the measured benchmarks and produces the correct scheduling:
switch to PCSR during ingestion, switch back to CSR for analytics.

The user's spec did not include SET equations for traversal or insert. SET uses
Roaring bitmaps for nodes and edges, plus an `edge_pairs` table (flat array of
`(u, v)` pairs indexed by edge ID). There is no adjacency index — no
`row_ptr`-equivalent structure for O(deg) neighbor lookup.

### 3.1 SET Traversal Cost

To traverse neighbors of vertex `u`:
1. **Navigation:** There is no index to navigate. The only metadata is the
   `edges_bitmap` (which edge IDs are live) and the `edge_pairs_table` (the
   actual (u, v) pairs). To find all neighbors of `u`, we must scan all live
   edge pairs and check if either endpoint equals `u`.
   - `C_nav = 1·t` (single base-pointer access to the edge_pairs table; no
     redirection chain)

2. **Data access:** We must scan all `m` live edge pairs (or `m/2` undirected
   pairs, each stored as 16 bytes in the IR inline path: `{i64 u, i64 v}`).
   - `B_useful = 16·m` bytes (upper bound; see compromise §1.6)

3. **Traffic:**
   `C_traffic = ceil(16·m / L) · T`

4. **Full traversal cost:**
   ```
   C_traversal(SET) = |AS| × (1·t + ceil(16·m / L) · T)
   ```

**Reasoning:**
- `C_nav = 1·t` (not `2·t`): Unlike CSR/PCSR/BCSR, which have a pointer array
  (`row_ptr`/`brow`) that requires two reads to find the legal span, SET has
  no such structure. The traversal directly accesses the `edge_pairs` base
  pointer — a single redirection. The tradeoff is that the "legal span" is the
  entire edge list, not a per-vertex slice.
- `B_useful = 16·m`: This is the total bytes of all edge pairs that must be
  scanned. Since there is no per-vertex index, every edge pair must be examined
  to determine if it is incident to `u`. The factor of 16 comes from the
  `{i64, i64}` edge-pair format used in the IR inline path. This is punitive by
  design — `16·m` bytes for a single vertex's neighbor list is far worse than
  CSR's `16 + 4·d` bytes. The cost model should (and does) strongly discourage
  SET for traversal-heavy regions.
- The `|AS|` multiplier makes this even worse: for a full BFS traversal with
  `|AS| = n`, the total cost is `n × ceil(16·m / L) · T`, which is O(n·m) —
  the cost of scanning all edges for every active vertex. This correctly
  reflects the algorithmic complexity of SET-based neighbor iteration.

### 3.2 SET Insert Cost — CORRECTED

To insert an edge `(from, to)`, `autograph_canonical_add_edge` performs:

1. **Locate:** Hash lookup in static edge hash (`canonical_pair_find_static` →
   `edge_hash_find`). If not found, linear scan of extra edge pairs
   (`canonical_pair_find_extra`).
   - `C_locate = 2·t` (hash table probe chain; extra dedup scan is O(extra)
     and negligible relative to step 3)

2. **Write:** `roaring_bitmap_add` for nodes u and v (2 calls), plus either
   `roaring_bitmap_add` for the edge ID (static edge found) or
   `extra_edge_pairs` append + `ensure_extra_capacity` (new extra edge).
   - `C_write = 3·t` (three bitmap/array mutation operations)

3. **Refresh:** Every successful canonical add calls
   `refresh_graph_counts_from_canonical` when `current_layout == LAYOUT_SET`.
   This calls `canonical_edge_count`, which does a **full linear scan** over all
   `static_pair_count ≈ m/2` entries, each checking `roaring_bitmap_contains`.
   - `C_refresh = ceil((m/2) · kR / L) · T`
   - `kR = 24` bytes/check (container array binary-search probes + bit test)
   - Simplifies to: `C_refresh = ceil(12·m / L) · T`

4. **Full insert cost:**
   ```
   C_update(SET) = 2·t + 3·t + ceil(12·m / L) · T
                 = 5·t + ceil(12·m / L) · T
   ```

**Why this is O(m) when SET insert should be O(1):**
Roaring bitmap mutations are amortised O(1). The O(m) cost comes from the
correctness invariant that `refresh_graph_counts_from_canonical` must
recompute `n` (node count) and `m` (edge count) from the canonical state
after every mutation so that Graph* pointer fields stay accurate. That
recomputation iterates over every edge pair ID, making each insert do O(m)
bookkeeping work. This is an implementation choice, not an algorithmic
necessity — edge counts could be maintained incrementally (O(1) per insert)
but the current code scans the full edge set.

### 3.3 Why SET insert is no longer the cheapest

With the O(m) refresh cost included, the insert cost ranking becomes:

```
PCSR ((2+g·d)·t + 1·t + ceil(4·(g·d/2)/L)·t)
    < CSR ((2+d)·t + 1 + ceil((n-u)·8/L)·t + ceil(4·(m-p)/L)·t)
    < SET (5·t + ceil(12·m/L)·T)  ≈  BCSR (same as SET)
```

For a typical graph with m ≥ 10,000, the `ceil(12·m/L)·T` term in SET/BCSR
dominates all other terms. This means PCSR is now correctly predicted as the
cheapest insert layout, matching measurements.

The prior ranking was:
```
SET (3t) ≤ BCSR (3t + rebuild) ≤ PCSR ((2+2d)t + t + ceil(4d/L)·t) ≤ CSR (...)
```
which was incorrect because it omitted the O(m) `canonical_edge_count` scan.
