# Autotuner branch comparison — 2026-10-14

Execution follow-up: [EFFECT_EXECUTION_OPTIMIZATION.md](EFFECT_EXECUTION_OPTIMIZATION.md)
records correctness-preserving runtime optimizations and a fresh three-version
comparison. The cost model remains unchanged; the baseline gap is reduced but
not eliminated.

## Conclusion

**The merged version is substantially worse than `effect_interim` on CC and
SSSP's absolute prediction-accuracy tests, although the cost-model source is
exactly identical.** SSSP passes the existing 5× criterion in all 11 baseline
measurements and fails it in all 11 exact-merge measurements. The budgeting
repair does not remove these accuracy regressions.

There is an important attribution distinction: `effect_interim` is the incoming
parent of the merge. The other parent already contains most of the degradation.
The merge retained that parent's runtime and frontier lowering. The large
difference from `effect_interim` is therefore chiefly inherited implementation
behavior, rather than newly changed equations or a new failure created from two
previously accurate parents.

No autotuner implementation or equation was changed for this experiment.

## Revisions and exact source comparison

| Label | Revision | Relationship |
|---|---|---|
| Requested baseline | `59355db` (`effect_interim`) | Incoming parent |
| Other parent | `b145e74` | First parent, immediately before the merge |
| Exact merge | `a019e0c` | Merge of the two parents above |
| Repaired merge | `772379d` | Current branch, including budgeting repair |

`AutoTunerPass.cpp`, `AutoTunerPass.h`, and `cost_model.py` are **byte-for-byte
identical across all four Git revisions**. This comparison covers the whole
files, including equations, constants, calibration loading, counting, and
layout selection. SHA-256 values and post-build working-file checks are saved.

The surrounding `autotuner_runtime.c` and `.h` differ between `effect_interim`
and the merged branch. Those two files, and `graph_frontier_lowering.cpp`, are
unchanged between the **other parent and the exact merge**. Identical cost-model
files do not imply identical execution or profiling behavior.

## Controlled experiment

- Separate managed checkouts; no use of stale compiler or program binaries.
- Same WSL machine, LLVM 20.1.2, ANTLR 4.10.1, compiler flags, and runtime build
  flags. `FORCE_CPU=1` selects CPU compilation on this GPU-capable host.
- Four workers, pinned to the same four distinct physical cores: `0,2,4,6`.
- One frozen calibration file, explicitly selected with `AUTOTUNER_HW_CALIB`.
- Identical fixture bytes and algorithm bodies. The baseline uses the equivalent
  classic loop spelling because its parser predates member loop syntax. A
  reverse-translation and normalized source hash verify that only headers change.
- Builds finish before measurements. Warm filesystem caches, fresh processes,
  randomized version order, 11 measurements per case and version.
- All 165 measurements in the main comparison produce identical case answers
  across versions and match the independent expected results. Another 44 paired
  measurements check the other parent on CC and SSSP, also with identical answers.

The existing autotuner test group's five cases and its unchanged assertions were
also replayed against the compiled binaries. That single replay reported **7
passed / 3 failed** on `effect_interim`, and **5/5** on both merged versions.
The replay uses precompiled case binaries; its assertion block is copied verbatim
from `verify/run.sh`. Repeated measurements below provide stronger evidence than
that single replay, particularly for borderline CC baseline runs.

### Baseline build qualification

The committed `effect_interim` grammar does not build with its committed AST
builder. An unmodified parser generation and compile attempt records the missing
`WeightedGraphDefContext` / `UnweightedGraphDefContext` errors.

The isolated baseline needs three grammar compatibility repairs: restore the two
graph-definition labels, the directed-property Boolean accessor, and the direct
function-call ID accessor expected by its existing visitor. No AST builder,
lowering, runtime, cost model, profiler, or algorithm body was edited. The grammar
patch and original failure log are saved. Thus this is a controlled comparison of
the branch's implementations after a documented frontend build repair, not a
claim that the untouched committed branch builds successfully.

All fresh checkouts also expose a cache-helper link defect: `03_run.sh` omits
`gpu_runtime.o` from the helper link. The helper was linked externally with each
branch's own GPU runtime object. No autotuner or GPU source was edited; GPU
execution was not used.

## Main results

Values are **median measured traversal-region time / predicted traversal-region
time**. Closer to 1 is better; the existing criterion is 0.2–5.

| Algorithm | `effect_interim` | Exact merge | Repaired merge |
|---|---:|---:|---:|
| BFS | 8.25× | 11.72× | 12.60× |
| CC | 5.21× | 35.72× | 36.70× |
| k-core | 12.75× | 9.05× | 9.51× |
| PageRank | 18.63× | 13.53× | 13.87× |
| SSSP | 2.71× | 25.46× | 25.64× |

BFS, CC, and SSSP worsen; k-core and PageRank move closer to their predictions.
This is not a uniform degradation of every test.

| Case/version | Predicted ms | Median measured ms | Accuracy passes / 11 |
|---|---:|---:|---:|
| CC baseline | 0.468792 | 2.441166 | 4 |
| CC exact merge | 0.468792 | 16.743859 | 0 |
| CC repaired merge | 0.468792 | 17.206221 | 0 |
| SSSP baseline | 2.343960 | 6.363427 | 11 |
| SSSP exact merge | 2.343960 | 59.669980 | 0 |
| SSSP repaired merge | 2.343960 | 60.094856 | 0 |

CC is already borderline or inaccurate on the baseline. The merge greatly
worsens it. SSSP is a consistent newly failing accuracy check relative to the
requested baseline. Their predictions remain exactly constant across versions.

The exact merge increases median CC region time by **6.86×** and SSSP region time
by **9.38×** versus `effect_interim`. Paired bootstrap 95% intervals for the
measured/predicted ratio change are **6.57–10.27×** and **9.06–9.68×**, respectively.
These differences are much larger than the observed timing variation.

Whole-process medians also increase: CC **32.35 → 236.15 ms**, SSSP **72.25 →
341.05 ms**. The discrepancy is therefore accompanied by real program slowdown,
not merely a different numerical prediction.

## Other-parent control: what the merge inherited

This separate randomized paired experiment uses 11 measurements per version on
the same cores and calibration, and the same CC and SSSP binaries/cases.

| Case | Other parent ratio | Exact merge ratio | Other parent measured ms | Merge measured ms |
|---|---:|---:|---:|---:|
| CC | 32.83× | 34.78× | 15.389175 | 16.302791 |
| SSSP | 24.58× | 25.14× | 57.608844 | 58.935750 |

Both checks already fail badly in the first parent. CC has a further small region
time increase (paired median 1.059×, bootstrap interval 1.040–1.101×). SSSP's small
change is not clearly distinguishable from variation (paired median 1.062×,
interval 0.999–1.070×). Neither resembles the large degradation relative to
`effect_interim`.

## Why unchanged equations give different accuracy

The compiler/runtime changes alter the amount and cost of executed work and the
regions seen by the unchanged model. In the measurements:

- CC's step executes **2 or 3 times** on `effect_interim`, versus **6 times** on
  both merged versions. Its static prediction remains one 0.468792 ms charge.
- SSSP's step has **21 versus 22** visits. That small count change cannot explain
  its approximately ninefold region-time increase; the implementation and
  region costs differ substantially as well.
- k-core and PageRank have different region structures and total predictions
  despite identical model source. Their accuracy improves partly because more
  operations are charged, not because new equations were imported.

This establishes a mismatch between an unchanged layout cost model and the
retained implementation. The follow-up below identifies concrete execution
changes. It does not assign each millisecond to an individual routine or validate
relative layout-selection quality: all captured traversal regions use CSR, and
the requested accuracy tests compare absolute region times.

## Follow-up: concrete reasons for the slowdown

The decisive default-path change predates the merge. In `IRGenVisitor.cpp`,
`visitWhile` changes `if (!getenv("GRAPH_FRONTIER_REWRITE_OFF"))` on
`effect_interim` to `if (getenv("GRAPH_FRONTIER_REWRITE_OFF"))` on the other
parent. Git blame attributes that change to **1c383ed8**, dated October 13,
before merge a019e0c. The associated comment explicitly retires the AST motif
dispatch in favor of the effect-algebra path.

The saved final LLVM modules confirm what executes under the default controls:

- Baseline CC calls `autograph_edgemap(..., combine=1, ...)`; baseline SSSP calls
  `autograph_edgemap(..., combine=3, ...)`. These select specialized min-label
  and weighted-relaxation kernels.
- Merged CC and SSSP instead create an operation/context, bind an ordered array
  frontier, prepare spatial execution, bind a property snapshot, and call
  `autograph_frontier_execute`. Both use destination groups and a spatial DAG.

Several concrete differences make that replacement more expensive:

1. **Propagation changes.** The baseline specialized kernels read live source
   labels/distances, so an improvement can propagate again during the same
   pass. The new callbacks read source properties through
   `autograph_exec_ctx_snapshot`, while destination updates remain live. This
   freezes source values at round start and delays propagation to later rounds.
   That explains the direction of CC's increase from 2–3 to 6 rounds.
2. **Array occurrences replace a deduplicated worklist.** Baseline
   `autograph_motif_activate_once` permits one append per destination per round.
   The new `autograph_frontier_activate` array route logs every successful append
   into its original pair occurrence's slot; `sgpl_array_compact` preserves
   duplicates and order. This is intentional array-sequence behavior, documented
   in `FRONTIER_STORAGE_AND_PROOFS.md`, not evidence of an accidental merge
   conflict. It increases later frontier work, particularly for SSSP.
3. **Per-round materialization is added.** `sgpl_array_records` traverses each
   frontier occurrence's neighbors three times: once to count, then twice to
   count/fill stable partition batches. It allocates pair records, source/work
   ordinals, and append slots. `sgpl_gpu_prepare_spatial_records` then groups and
   reorders those records; despite its name, spatial preparation calls it for
   these CPU array contexts too. Execution clears append slots, invokes the
   outlined pair callback, and scans the slots twice to compact the result.
   Snapshot/context creation and destruction also occur every round. The old
   specialized path traverses CSR directly, reuses scratch storage, and chooses
   push/pull or a small-frontier serial path.
4. **SSSP loses direct aligned weight access.** The baseline CSR weighted kernel
   reads `csr_weights[j]` while walking `csr_col_idx[j]`. The merged emitted pair
   callback calls `graph_get_edge_weight(G,u,v)`, which linearly searches u's
   neighbor row for v. It therefore re-scans adjacency for each relaxed pair.
   This and the general callback path add work even when round counts differ
   by only one.
5. **The prediction does not follow these changes.** `operationCost` still
   charges `H * totalOps * traversalCost(...)` (plus insert terms where applicable).
   An unknown dynamic round count falls back to H=1. The profiler records the
   prediction once per static region and accumulates measured time across all
   visits. The unchanged equations do not explicitly charge the new array-record
   passes, append logs, snapshot/context work, or per-pair weight searches.

A read-only GDB trace of the preserved binaries verifies frontier sizes and
duplicates. It uses the same fixtures, calibration, four-core affinity and CPU
controls. Breakpoints read function argument registers and frontier memory;
no source or binary was modified. These traces are structural observations,
not benchmark timing samples:

| Case/version | Rounds | Sum of frontier entries | Sum of degrees of frontier entries |
|---|---:|---:|---:|
| CC baseline | 3 | 40,589 | 648,521 |
| CC exact merge | 6 | 95,722 | 1,525,831 |
| SSSP baseline | 21 | 68,803 | 1,103,236 |
| SSSP exact merge | 22 | 123,300 | 1,981,056 |

The last column is frontier-driven push work, not an exact count of baseline
edge executions: the specialized baseline can choose pull instead. In SSSP's
fifth round the new frontier has **34,351 entries but only 16,898 distinct
vertices**; baseline frontiers in this trace have no duplicates. CC's traced
frontiers have no duplicates, so its observed extra work mainly reflects
slower propagation rather than duplicate entries. A baseline trace's exact
round/work counts can vary with parallel live updates, as seen in the original
11-run measurements.

The original uninstrumented SSSP samples also expose a timing-boundary issue:
median region 0 time increases **0.142 to 16.045 ms**, while median region 1
time increases **6.221 to 43.676 ms**. The generated region-0 timer begins in
distance initialization and is not closed until immediately before the first
executor call. Thus it also includes intervening initialization and first-round
setup, including the new preparation. This means the reported ninefold total
contains both real executor slowdown and setup charged to the preceding region.
The whole-process slowdown and region-1 increase still establish a real runtime
regression.

`SGPL_DAG_DEBUG` in the trace confirms spatial dispatch, including width four
and later width-three grants for CC; it does not support attributing this large
regression solely to complete loss of parallelism. Individual overhead shares
would require a separate profiling/ablation experiment. The path change,
snapshot behavior, duplicate preservation, repeated materialization and weight
lookup are established directly by emitted IR, implementation, and traces.

Additional artifacts: `{cc,sssp}_final.ll`, per-version
`{cc,sssp}_frontier_trace.log`, `frontier_trace_summary.json`, and
`trace_frontiers.py`. No autotuner implementation was changed.

## Evidence and reproducibility

Artifacts are under `verify/bin/autotuner_branch_comparison_2026_10_14/`:

- `source_comparison.json`, `protected_source_verification.json`: source identity.
- `environment.json`, `calibration.json`, `input_equivalence.json`: controls.
- `raw.json`, `summary.json`, `summary.csv`, `comparisons.json`: 165-run results.
- `other_parent_raw.json`, `other_parent_summary.json`: 44-run attribution control.
- Per-version compiler logs, emitted bitcode, binaries, raw profile logs,
  stock assertion logs, and the baseline grammar patch/failure.
- `build_compare.py`, `run_compare.py`, `run_stock.py`, `check_other_parent.py`:
  the experiment drivers. Worktree paths are recorded explicitly.

Compiler/program binaries and fixture copies are preserved with the artifacts.
The temporary managed worktrees were archived through the app's recoverable
archive mechanism. The recorded checkouts can be restored when
re-running the drivers; saved program binaries can also use the artifact folder's
`verify/fixtures` copies when launched from their per-version directories.

The main checkout's autotuner files and all five protected implementation files
in every test checkout remain unchanged. The isolated baseline grammar repair
and generated build artifacts are the only compatibility changes needed here.
