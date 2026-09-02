# SGPL BFS Performance Journey

How the `p1GraphEasy-con-AutoTuner` graph compiler went from **3x slower than Graptor**
to **1.66x faster than Graptor** end-to-end on a 2M-vertex / 8M-edge BFS, and what is
left to fix. Each section states the concrete problem, the approach taken, and the
measured result.

All numbers below are medians of paired, warm-up-preceded launches on the same host,
running BFS-parent from source 0 on a connected graph with 2,000,000 vertices and
8,000,000 undirected input edges (16,000,000 directed CSR arcs). "End-to-end" means
process launch-to-exit wall time, including graph loading.

---

## TL;DR

| Milestone | SGPL 8-thread end-to-end | Graptor 8-thread end-to-end | Verdict |
|---|---|---|---|
| Before this work | 960.9 ms | 320.4 ms | SGPL ~3.0x slower |
| After binary cache + parallel CSR | **189.1 ms** | 313.5 ms | **SGPL 1.66x faster** |

The single biggest win was recognizing that the end-to-end gap was **not** in the BFS
kernel at all: it was in graph loading. Fixing loading (binary cache + parallel CSR
construction) turned a 3x loss into a 1.66x win without touching the traversal.

---

## Benchmark methodology

- Identical edge set fed to both systems; both traverse from vertex 0 and must reach all
  2,000,000 vertices, validated every run.
- Thread-count sweeps are **randomized/interleaved** per round, so sustained-load or
  thermal drift on the host cannot masquerade as a scaling trend.
- Graptor is run from its prebuilt binary CSR format (`.graptor`); SGPL originally parsed
  a text edge list every run. That asymmetry turned out to be the whole story (Problem 2).

---

## Problem 1: BFS thread scalability was flat and noisy

### The problem
Compared against Graptor, SGPL's per-thread scaling was poor: adding threads barely
reduced time, and results fluctuated (2 threads was sometimes *slower* than 1). Root
causes identified:

- **Per-loop thread creation.** Every parallel loop spun up and joined threads, so launch
  overhead dominated the many short BFS levels.
- **Contention on the shared next-frontier.** All workers appended discovered vertices to
  one array behind a shared counter, serializing the hottest path.
- **Static cyclic work distribution** with no sparse/dense adaptivity.

### The approach
Implemented within SGPL's existing parallelization paradigm (no rewrite):

1. **Persistent thread pool** (`parallel_runtime.c`). A `pthread_once`-initialized pool
   with a barrier replaces per-loop `pthread_create`/`join`, so repeated BFS levels reuse
   resident workers. The launch cost model was retuned to reflect the lower pooled cost.
2. **Next-frontier privatization / first-wins CAS.** The `if (visited[u]==0) { visited[u]=1; ... }`
   pattern is lowered to an atomic `cmpxchg` (`IRGenVisitor.cpp`), so exactly one worker
   claims each vertex. Each worker appends to a **private lane**, eliminating the shared
   counter.
3. **Parallel frontier merge.** Worker-local lanes (`AutoFrontierLane`) are combined into
   the global next frontier with a prefix-sum + parallel copy (`autograph_frontier_step`
   in `autotuner_runtime.c`), instead of a serialized append.
4. **Adaptive push/pull.** Per level, the runtime estimates frontier edge work and chooses
   sparse push vs. dense pull (`push_edge_work > csr_n`), keeping the decision
   representation-agnostic across CSR/PCSR/BCSR/SET.

### The result
Traversal-side contention and launch overhead dropped, but end-to-end time on the large
graph was still ~3x behind Graptor. That pointed the investigation elsewhere.

---

## Problem 2: End-to-end time was dominated by graph loading (the real bottleneck)

### The problem
Phase profiling of the 960.9 ms 8-thread run showed the BFS kernel was **not** the
problem:

- SGPL **load-only** median: ~844 ms — about **88%** of the entire 8-thread run.
- Estimated work after loading: only ~117 ms.

Graptor started from a prebuilt binary CSR (read in ~44 ms). SGPL re-parsed a 119 MB text
edge list and built CSR **serially** on every single execution. We were comparing a text
parser against a binary loader and blaming the BFS kernel.

Two concrete deficiencies in `graph_loader_runtime.cpp`:
- After a parallel text parse, results were merged and CSR was built in a **fully serial**
  block (degree count, prefix sum, fill).
- There was **no persistence**: identical work was redone from scratch every run.

### The approach

#### A. Parallel CSR construction
Replaced the serial merge + serial CSR block with a bounded-memory parallel builder:

- Per-thread parsed-edge vectors are prefix-summed and flattened with parallel copies (no
  serial concatenation).
- **Hybrid CSR assembly:**
  - When `threads x vertices` fits a memory budget (`SGPL_CSR_HISTOGRAM_MB`, default 64 MB):
    per-thread degree histograms -> parallel per-vertex reduction -> parallel prefix scan
    -> disjoint per-thread row offsets -> atomic-free parallel fill.
  - Otherwise: parallel atomic degree/cursor updates, so very large graphs never fail from
    histogram memory blowup.
- Dense node/edge metadata bitmaps are built once (the multi-bitmap union variant was
  removed after it produced a thread-count-dependent cache mismatch; correctness beat the
  micro-optimization).

#### B. Transparent binary sidecar cache
Added a `<edge-list>.sgplbin` sidecar (weighted graphs use `.weighted.sgplbin`) that
persists the fully-built graph. Key design points:

- **Versioned fixed-width header**: magic, version, endian marker, weighted flag, source
  size + nanosecond mtime, `n`, directed `m`, logical edge count, per-section
  offsets/sizes, and **per-section checksums**.
- **Cache validation**: a cache is used only if magic/version/endian/mode match, the source
  size and mtime are identical, section geometry is internally consistent, and every
  section checksum verifies. Any mismatch -> silently rebuild from text. This rejects
  stale, truncated, wrong-mode, overflowed, and corrupt caches.
- **Atomic publication**: written to a same-directory temp file, `fsync`ed, then
  `rename`d into place, with a re-`stat` guard so a source changed mid-build is not cached.
  If the directory is not writable, the run proceeds normally without caching.
- **Zero-copy warm loads**: a cache hit `mmap`s the sidecar (private/COW) and points
  `Graph`/`GraphExtra` directly at the mapped CSR, logical-edge, and serialized-bitmap
  sections instead of allocating and copying. Ownership flags prevent the borrowed regions
  from being freed. Graphs have process lifetime, so the mapping safely stays alive.
- **Parallel validation**: section checksums and the CSR bounds check run across the thread
  pool, so validating a hit stays cheap.
- Controls: `SGPL_GRAPH_CACHE=off` (never read/write) and `SGPL_GRAPH_CACHE=refresh`
  (ignore existing, rebuild and republish).

The public ABI (`load_graph_from_file` / `load_weighted_graph_from_file`) and the GraphEasy
`edges: file "..."` syntax are unchanged, so nothing in the compiler front-end or IR
lowering had to change.

#### C. Hardening the parser
Added rejection of negative and overflowing vertex IDs before they can index CSR arrays,
preserving previous behavior for valid inputs.

### The result

| Loader phase (8 threads) | Median |
|---|---|
| Cold text parse + parallel CSR | 1257.5 ms |
| First run (build + checksum + publish cache) | 1425.9 ms |
| **Warm binary cache hit** | **41.0 ms** |

The warm cache is roughly **31x faster** than cold text construction. Building the cache
costs slightly more than a plain cold run because it additionally checksums and atomically
publishes the sidecar — a one-time cost amortized across all later runs.

Feeding this into the full BFS end-to-end benchmark:

| Metric (8 threads, 2M/8M) | SGPL | Graptor | Ratio |
|---|---|---|---|
| End-to-end, before | 960.9 ms | 320.4 ms | 3.0x slower |
| **End-to-end, warm cache** | **189.1 ms** | 313.5 ms | **1.66x faster** |

SGPL's own end-to-end median dropped **80.3%** (960.9 -> 189.1 ms).

---

## Correctness and resilience

A dedicated loader test harness (`test/graph_loader_runtime_test.cpp`,
`test/run_graph_loader_tests.sh`) compares canonical graph semantics — `n`/`m`, sorted
adjacency rows, weights, logical edge metadata, and bitmap cardinalities — and covers:

- Text build vs. warm cache hit at 1, 2, 4, and 8 threads (semantics must match).
- Histogram builder vs. bounded-memory atomic-fallback builder (must match).
- Weighted vs. unweighted cache separation.
- Source modification invalidation; `off` and `refresh` modes.
- Truncated and checksum-corrupt caches (must be ignored and rebuilt).
- Empty / commented / malformed inputs, duplicates, and self-loops.
- Negative and overflowing vertex IDs (must be rejected).
- Unwritable cache directory (must degrade gracefully, not crash).
- Concurrent writers (both correct; final cache valid).

The changed runtimes also compile cleanly under `-Wall -Wextra -Werror`, and cold and
cached BFS both reach all 2,000,000 vertices.

---

## Scaling ceiling that remains

End-to-end SGPL now wins on absolute time, but normalized BFS thread scaling is still
roughly flat (1-thread ~360 ms, 10-thread ~416 ms in the sustained sweep). Now that
loading is cheap (~41 ms), the remaining ceiling comes from:

1. **Serial O(n) phases** in the BFS programs — the `visited`/`parent` init loop and the
   final reached-count loop run single-threaded over all vertices and were previously
   hidden by the 844 ms load. Making the compiler treat these simple init/reduction loops
   as DOALL would lift the Amdahl ceiling.
2. **Per-level dispatch and allocation churn** — `autograph_frontier_step` allocates lanes,
   membership, and offsets and launches multiple parallel regions per BFS level. Persistent
   scratch and a single resident region across levels would amortize this.
3. **Atomic CAS in push mode** — an owner-computes (destination-partitioned) pull would let
   the CAS on `claim[destination]` become a plain load/store in dense levels.
4. **Cyclic scheduling / false sharing** — contiguous, edge-balanced chunks plus
   cache-line-padded lanes would improve locality.

These are the next highest-value changes; the loading bottleneck that dominated end-to-end
time is now solved.

---

## Where the work lives

- `graph_loader_runtime.cpp` — binary sidecar cache (read/validate/publish) and parallel
  CSR construction.
- `roaring_bitmap.cpp` / `roaring_bitmap.h` — portable bitmap deserialization used by cache
  loads.
- `autotuner_runtime.c` — `autograph_frontier_step`: private lanes, prefix-sum merge,
  adaptive push/pull.
- `parallel_runtime.c` — persistent thread pool, worker indexing.
- `IRGenVisitor.cpp` / `pdg.cpp` — first-wins CAS lowering and DOALL classification of the
  frontier loop.
- `test/graph_loader_runtime_test.cpp`, `test/run_graph_loader_tests.sh` — loader
  correctness/resilience suite.
- `benchmark_graph_loader.py`, `benchmark_bfs_compare.py`, `benchmark_bfs_*.sh` — benchmarks.
- `benchmark_results/` — CSV + SVG artifacts referenced above.
