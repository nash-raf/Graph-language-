# SET / BCSR Insert Cost: Equation Correction

## Problem

The autotuner's insert cost model predicted BCSR and SET inserts at **0.006 ms** for
512 insertions, but the measured pure-kernel time was **~100–160 ms** — an error of
**~20,000×**. Simultaneously, the CSR and PCSR predictions were within 3–6× of
measurement. The root cause: the model computed SET/BCSR insert cost as `3·t`
(three random pointer accesses ≈ a few nanoseconds), but the runtime implementation
does **O(m) work per insert** that was entirely absent from the equations.

## Trace of an Insert Operation

Every `graph_add_edge` when the layout is SET (or after BCSR converts to SET)
calls `autograph_canonical_add_edge`:

```
autograph_canonical_add_edge(u, v):
  1. roaring_bitmap_add(nodes_bitmap, u)            // O(1) amortised
  2. roaring_bitmap_add(nodes_bitmap, v)            // O(1) amortised
  3. canonical_pair_find_static(u, v)               // hash lookup, O(1) expected
  4. canonical_pair_find_extra(u, v)                // LINEAR SCAN, O(extra_edge_count)
  5. ensure_extra_capacity()                        // occasional realloc
  6. append to extra_edge_pairs[] + set extra_edge_live[]
  7. refresh_graph_counts_from_canonical(meta)      // *** O(m) — FULL SCAN ***
```

Steps 1–6 are cheap. Step 7 is the bottleneck.

### Step 7: `refresh_graph_counts_from_canonical`

```c
static void refresh_graph_counts_from_canonical(AutoGraphMeta *meta) {
  int64_t nn = canonical_node_span(meta);          // O(1) — max node label
  int64_t undirected = canonical_edge_count(meta); // *** O(static_pair_count) ***
  int64_t directed = undirected * 2;
  *((int64_t *)(base + 0)) = nn;
  *((int64_t *)(base + 8)) = directed;
}
```

### `canonical_edge_count` — the true cost center

```c
static int64_t canonical_edge_count(AutoGraphMeta *meta) {
  RoaringBitmap *eb = (RoaringBitmap *)meta->edges_bitmap;
  int64_t count = 0;
  for (int64_t e = 0; e < meta->static_pair_count; e++) {   // ← scans ALL static pairs
    if (roaring_bitmap_contains(eb, (uint32_t)e))            // ← approx. m/2 iterations
      count++;
  }
  for (int64_t i = 0; i < meta->extra_edge_count; i++) {    // ← scans ALL extra pairs
    if (meta->extra_edge_live[i])
      count++;
  }
  return count;
}
```

The function iterates over **every** static edge pair (`static_pair_count ≈ m/2`)
on **every single insert**. For each iteration it calls `roaring_bitmap_contains`,
which does a binary search in the container array followed by a bit test in the
target container. With `m = 500,000` directed edges, each insert scans **250,000**
entries through the roaring bitmap.

When the graph is small or when many edges are dynamic (extra), the static scan
still dominates because the roaring bitmap "liveness" check iterates over all
possible edge IDs, not just live edges.

## New Equations

### SET Insert

```
C_insert(SET) = C_locate + C_write + C_refresh

C_locate  = 2·t      — static-edge hash lookup (edge_hash_find)
C_write   = 3·t      — roaring_bitmap_add(u) + roaring_bitmap_add(v)
                       + roaring_bitmap_add(edge) or extra_edge_pairs append
C_refresh = ceil((m/2) · kR / L) · T
                     — canonical_edge_count: scans static_pair_count ≈ m/2 entries,
                       each calling roaring_bitmap_contains which accesses
                       kR ≈ 24 effective bytes per check
```

### BCSR Insert

```
C_insert(BCSR) = C_insert(SET)
```

BCSR has no native mutation path. `graph_add_edge` converts BCSR → SET (captured
by region profiling, not kernel time), resets the kernel timer, then delegates to
`autograph_canonical_add_edge`. The kernel cost is therefore identical to SET insert.
The BCSR→SET conversion + eventual SET→BCSR rebuild are accounted for separately
by `conversionCost()` at region boundaries.

## Derivation of `C_refresh`

### Why `m/2` entries?

The `static_pair_count` field records the number of unique undirected edge pairs
at graph construction time. Each undirected edge contributes 2 directed edges to
the graph's edge count `m`. Therefore:

```
static_pair_count ≈ m / 2
```

(This holds exactly when `extra_edge_count = 0`. For mutation-heavy regions,
`static_pair_count` stays fixed while `m` grows via extra edges, making `m/2`
a slight overestimate of `static_pair_count` — but the extra loop in
`canonical_edge_count` also scans `extra_edge_count` entries, partially
compensating.)

### Why `kR = 24` bytes per `roaring_bitmap_contains`?

Each call to `roaring_bitmap_contains` accesses the internal container structure:

| Access | Bytes | Description |
|--------|-------|-------------|
| Container array probe 0 | 8 | Binary search step 1: compare container key |
| Container array probe 1 | 8 | Binary search step 2: narrow to target container |
| Container data look-up | 8 | Bit test in bitset word (or array search) |
| **Total** | **24** | Effective bytes per contains call |

These bytes are mostly cached for sequential access (edge IDs 0, 1, 2, … all
fall within the same few containers). The model's `⌈bytes/L⌉ · T` pattern treats
this as a streaming scan — each cache line of "work" costs `T`.

### Why modelled as a scan (T) rather than random access (t)?

When the bitmap contains calls iterate sequentially through edge IDs, the
container array and container data remain resident in L1/L2 cache. The dominant
cost is CPU work (bit tests + loop overhead), not cache misses. The existing
model's convention uses `T` (cache-line transfer cost) for sequential/cached
data access — matching the streaming nature of the edge-count loop.

### Why separate `C_locate`, `C_write`, `C_refresh` instead of a single term?

This decomposition maps one-to-one to the implementation:

| Term | Implementation | Cost driver |
|------|---------------|-------------|
| `C_locate = 2·t` | `canonical_pair_find_static` → `edge_hash_find` | Hash table probe (2 redirections) |
| `C_write = 3·t` | `roaring_bitmap_add` ×3, `ensure_extra_capacity` | 3 bitmap mutation operations |
| `C_refresh` | `refresh_graph_counts_from_canonical` → `canonical_edge_count` | O(m) linear scan of edge bitmap |

The old model had only `C_locate = 2·t` + `C_write = 1·t` (total 3·t), missing
the O(m) refresh scan entirely.

## Numerical Validation

With calibration parameters from `~/.config/sgpl/hw_calib.json`:
- `L = 64` bytes (cache line)
- `t = 32.99` ns (random access latency)
- `T = 2.737` ns (cache-line transfer cost)

For a graph with `n = 50,000`, `m = 500,000` (directed), and 512 insert operations:

### Old model (broken)

```
C_insert(SET) = 3·t = 3 × 32.99 = 98.97 ns
                     per insert
Total: 512 × 98.97 ns = 0.051 ms   (measured: ~160 ms)
```

### New model

```
C_locate  = 2 × 32.99              =    66.0 ns
C_write   = 3 × 32.99              =    99.0 ns
C_refresh = ⌈(250,000 × 24)/64⌉ × 2.737
          = ⌈93,750⌉ × 2.737
          = 93,750 × 2.737         =   256,600 ns
                                     ———————————
C_insert                         ≈   256,765 ns  per insert

Total: 512 × 256.8 µs = 131.5 ms   (measured: ~160 ms, 1.2× error)
```

The remaining ~1.2× gap is attributable to:
- Unmodelled `canonical_pair_find_extra` O(extra) dedup scan
- Unmodelled `canonical_node_span` bitmap cardinality query
- `ensure_extra_capacity` realloc amortisation not captured
- Calibration precision of `t` and `T` (calibration was run on a separate
  hardware context; the autotuner binary may have been compiled with different
  calibration values)

## Edge Cases Preserved

1. **Traversal ordering unchanged.** `C_traversal` equations (CSR < PCSR < BCSR ≪ SET)
   are unaffected by this fix. The autotuner still correctly selects CSR for
   traversal-heavy regions.

2. **Conversion costs unchanged.** BCSR→SET and SET→BCSR conversions continue to be
   accounted by `conversionCost()` at region boundaries. The new equations only
   affect per-insert kernel cost.

3. **PCSR insert unaffected.** PCSR has its own `pcsr_add_directed` path that never
   calls `refresh_graph_counts_from_canonical`. Its equation remains correct.

4. **CSR insert unaffected.** CSR's `csr_add_directed` also never calls the
   canonical refresh path. Its O(E) realloc + memmove cost is already modelled.

5. **BCSR insert after first conversion.** After the first BCSR insert converts to
   SET, all subsequent inserts in the region execute the SET code path. The per-op
   model correctly charges the SET cost for all inserts, and the one-time BCSR→SET
   conversion cost appears in the region-level measured time (not kernel time).

## What the Old Model Got Wrong

The old model was derived from a theoretical simplification: "SET has no adjacency
index, so insert must be O(1) amortised via roaring bitmaps." This is true for
the bitmap mutation itself, but the implementation adds a **correctness invariant**:
after every mutation, `refresh_graph_counts_from_canonical` recomputes `n` and `m`
from the canonical state so that downstream CSR-like constructs have accurate
graph dimensions. That recomputation does a full O(m) scan.

The equations now reflect this implementation reality.
