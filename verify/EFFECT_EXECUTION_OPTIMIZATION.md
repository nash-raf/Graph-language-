# Effect-system execution optimization — 2026-10-14

## Outcome

Avoidable execution overhead is reduced without changing the layout autotuner
or the DOALL, DOACROSS, and thread-budget equations/policies. The new snapshots,
ordered array entries, duplicate occurrences, proof decisions, and DAG lifecycle
remain in place. **The gap to effect_interim is reduced, not eliminated.**

The final rerun uses 11 randomized paired measurements per case and version,
165 fresh processes in total. All outputs match across the three versions and
match the independent goldens. The original prediction-accuracy assertions
remain unchanged; they still fail on the optimized version.

## Final controlled comparison

`effect_interim` is 59355db. Before optimization is repaired merge HEAD 772379d.
Optimized is that HEAD plus the current uncommitted execution changes. Both
reference versions use the immutable binaries built in the previous controlled
experiment; the optimized compiler and programs are rebuilt from source.

This rerun uses the same WSL host, LLVM 20.1.2, pinned ANTLR 4.10.1, compiler and
runtime flags, frozen calibration, fixture bytes, algorithm bodies, and CPU
affinity 0,2,4,6. Four CPU workers and four Roaring/OpenMP workers are fixed.
`FORCE_CPU=1` and `SGPL_FRONTIER_STRICT=1` are retained. Builds and correctness
tests finish before timing; each executable is warmed before its measurements.

The baseline grammar-only compatibility repair remains documented in
[AUTOTUNER_BRANCH_COMPARISON.md](AUTOTUNER_BRANCH_COMPARISON.md). No baseline
lowering/runtime/model source or algorithm body was changed.

Median **profiled traversal-region time, milliseconds**:

| Algorithm | effect_interim | Before optimization | Optimized |
|---|---:|---:|---:|
| bfs_level | 4.148 | 6.284 | 6.043 |
| cc | 1.763 | 18.788 | 12.339 |
| kcore | 18.105 | 18.890 | 18.083 |
| pagerank | 43.085 | 45.406 | 46.396 |
| sssp | 6.488 | 62.744 | 46.969 |

Median **whole-process wall time, milliseconds**:

| Algorithm | effect_interim | Before optimization | Optimized |
|---|---:|---:|---:|
| bfs_level | 53.169 | 99.727 | 66.954 |
| cc | 33.575 | 264.188 | 124.838 |
| kcore | 85.989 | 80.465 | 82.504 |
| pagerank | 74.375 | 78.242 | 77.583 |
| sssp | 73.842 | 351.391 | 191.281 |

- CC profiled-region: median reduction **34.3%**; paired optimized/before ratio 0.652, bootstrap 95% interval 0.644–0.691.
- CC whole-process: median reduction **52.7%**; paired optimized/before ratio 0.461, bootstrap 95% interval 0.451–0.486.
- SSSP profiled-region: median reduction **25.1%**; paired optimized/before ratio 0.767, bootstrap 95% interval 0.734–0.792.
- SSSP whole-process: median reduction **45.6%**; paired optimized/before ratio 0.546, bootstrap 95% interval 0.535–0.560.

BFS's whole-process reduction is significant in the paired bootstrap result;
its small region-time change is not clearly distinguishable from variation.
K-core and PageRank have no clearly established change in this experiment:
their paired intervals include 1. The full data and intervals are saved.

All optimized predictions equal the before-optimization predictions exactly.
The earlier baseline region-structure differences in k-core/PageRank remain;
identical model source does not imply identical region structure.

## What changed

- Array record construction combines its work count with the partition count
  pass, reducing three topology traversals to two. Native CSR uses direct rows;
  other layouts retain their existing iterator route. A vertex-to-owner table
  replaces repeated partition-boundary searches.
- CPU preparation constructs the occurrence records it needs. Device-only
  grouping/reordering is deferred to actual GPU preparation. The CPU continues
  through the declared spatial DAG and the existing budgeting/planning policy.
- CSR frontier-capacity reservation reads row lengths directly rather than
  enumerating every neighbor. Normal compaction emits/counts in one pass when
  the reserved capacity covers every slot. Under-reservation retains the checked
  count/grow route, preserving output prefixes and never replaying callbacks.
- Weighted CSR execution caches each column's **first matching index in its
  row**, including unsorted rows and duplicate edges. A context owns its copied
  hints. The preparation-thread cache is invalidated by topology publication,
  mutation, layout epoch, pointer, and size changes; thread exit releases it.
  Queries on another graph/edge, other representations, and GPU-reordered CPU
  fallback retain the original query. Weight values are read live, not cached.
- After proof production and scheduling/outlining, small extracted task bodies
  and checked context getters are inlined into array-frontier callbacks. The
  host weight-query path consumes its validated index hint. GPU callbacks were
  captured before this host cleanup. Ordinary graph callbacks retain their
  established outlining/calibration shape.

The optimization does not re-enable AST motif dispatch, deduplicate frontiers,
replace frozen source reads with live reads, change effect witnesses or their
decisions, bypass the ledger, or adjust prediction thresholds/calibration.

A destination-grouped CPU experiment passed correctness but increased total
preparation/runtime in its paired measurements. It was discarded. The final
implementation retains the faster partition-batched route; its experiment
data is retained separately as `endpoint_grouped_*`.

## Preservation and validation

The complete AutoTunerPass.cpp, AutoTunerPass.h, and cost_model.py sources remain
identical to HEAD and effect_interim (Git content, accounting for checkout line
endings). The complete parallel_runtime.c/.h and tdg_scheduler.inc sources are
unchanged. pdg.cpp, graph_frontier_lowering.cpp, and parallel_loop_outline.cpp
are also unmodified. All six profiling functions are unchanged. The execution
ABI and helpers live in autotuner_runtime.c/.h; edits there concern execution
and invalidation of the new execution hint cache, not cost/layout equations.

Validation passed:

- Array executor tests at 1/2/4/8 workers over all four layouts: repeated
  sources/edges, multiple append sites, empty-source hooks, reused contexts,
  lazy device grouping followed by CPU execution, checked growth/prefix
  preservation, duplicate-edge first-match weights, and same-pointer topology
  publication invalidating the weight index.
- Generated array tests with exact ordering/duplicates and growth checks on
  sequential, CPU, and real GPU execution at 1/2/4/8 workers. An added weighted
  fixture checks unsorted rows, unequal duplicate-edge weights, repeated
  sources, and a sorted row on sequential/CPU execution.
- Full spatial and temporal compiler/runtime suites, including positive and
  negative proof decisions, snapshots, source claims, private folds, carried
  round dependencies, and scalar observers.
- Round/spatial tests at 1/2/4/8 workers, with generated overlap peaks 1/2/4/4
  for budgets 1/2/4/8, retaining the budget bound and both-axis execution.
- Budget, joint scheduling, and DAG tests at 1/2/4/8 workers; DOACROSS model/
  profile regressions at four workers. These cover cold FIFO, exhausted-pool
  serial fallback, balanced reservations, width/regime isolation, cache
  adaptation, dependencies, cancellation, and cycle refusal.
- Array executor under ASan/UBSan at four workers: no detected memory/undefined
  behavior errors. Leak detection is disabled for the process-lifetime fixture
  metadata; this is not a leak-clean claim.
- LLVM verifier for every final benchmark module; git diff --check.

One existing test expected `round-dag` for round_spatial_twofold. The immutable
pre-optimization compiler already selects `round-spatial-dag` for that fixture,
as recorded in `round_twofold_original_compiler.log`. Its assertion was updated
to the actual both-axis schedule; all numeric/overlap/budget checks were kept.

## Why a gap remains, and whether this is a fundamental clash

The effect system constrains which transformations are valid. It does not
inherently require repeated topology scans, per-pair weight searches, eager
device preparation, or unnecessary helper calls; removing those costs improves
performance while keeping the proof and resource models.

The old specialized path also performs different work: live source updates can
propagate again within a round, and destinations are appended once per round.
The current path retains frozen source snapshots and every successful ordered
array append. CC still has six visits; optimized SSSP still has 22. Its worklist
can contain duplicates. Those semantics cannot simply be disabled to recover
the baseline timings while claiming the same guarantees.

Generic record materialization, callback dispatch, append-log processing, and
round setup still cost time. The existing SSSP region-0 timing boundary also
includes intervening initialization and first-step setup. Keeping the autotuner
unchanged means that boundary and its dynamic H=1 fallback remain unchanged.
The whole-process measurements independently establish real speedups and a
remaining real gap. This experiment does not prove that every remaining
millisecond is unavoidable; additional execution optimization is possible.

Removing algorithm-specific work such as duplicate relaxations or snapshot
rounds would require an additional equivalence/proof rule covering observable
array order, multiplicity, intermediate values and convergence. Re-enabling
the old shortcut without that proof would weaken the requested contract.

The stock autotuner group's exact assertion block was replayed against the
preserved binaries. It reports **7 passed / 3 failed** for effect_interim and
**5 passed / 5 failed** for both before and optimized versions. Thus this work
improves execution but does not claim to fix absolute autotuner accuracy.

## Artifacts and reproduction

`verify/bin/effect_execution_optimization_2026_10_14/` contains compiler/runtime
build logs, the final compiler, program objects/bitcode, correctness logs,
raw.json, summary.json, comparisons.json, stock test replay logs,
protected_implementation_verification.json, and final_source_sha256.json.
The optimized executables are under the prior comparison artifact directory's
`optimized_merge/`, beside its baseline binaries and common fixture copies.

Reproduction drivers: build_and_compare.py (build, compile, measure phases),
run_correctness.py, check_models.py, and postprocess.py. The source comparison
report records the original reference build and grammar qualification.
