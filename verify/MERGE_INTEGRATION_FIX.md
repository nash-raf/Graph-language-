# Merge integration repair — 2026-10-14

This follows `MERGE_THREE_FAILURES.md`. The budgeting integration is repaired.
The layout autotuner is diagnosed below and was not modified.

## Context recovered from the project chats

- **Improve cost model equations**: retain the implementation-based DOALL and
  DOACROSS equations, account for real launch paths and per-worker synchronization,
  and share a bounded budget between task callbacks and their loops. The layout
  chooser is a separate model and is protected from these changes.
- **Ideate Doall and Doacross Cost**: removing profiling through a static model was
  a design discussion, with no authorization to implement that redesign. This
  repair does not implement it or add profiling.
- **Use ars_merge cost model**: import layout equations without reverting newer
  effect-system and runtime fixes.

## What broke and what changed

The merge retained the new joint scheduler but restored an older work/span-based
planner as its FIFO reference and cold fallback. These policies are incompatible.
The imported reference expects one callback worker per runnable task, limited by
the actual grant, with the residual reservation available to loop teams.

The old planner could return early with an empty loop plan. The executor still
bound an **active pool with zero loop threads**. Worse, the loop decision helper
ignored `remaining_threads == 0`, so it could predict parallel execution at width
three while the allocator granted one. The parallel clone then discarded its
range entry point and executed its per-index wrapper serially. This explains the
large slowdown even though the classifier still reported DOALL.

`parallel_runtime.c` now restores the imported FIFO reference contract, removes
the obsolete work/span helpers, and makes zero remaining capacity visible to the
loop predicate. Single-site debug diagnostics remain available. The joint
scheduler, loop equations, profitability predicates, and loop allocation solver
are unchanged. Parent reservations and the sharing-disable switch remain enforced.

The existing signed-distance DOACROSS classification fix is retained and its
wait/post execution test passes.

`build_lowmem.sh` now discovers system LLVM when the custom prefix is absent,
avoids duplicate Polly registration with monolithic LLVM, honors an explicitly
empty Polly override, and invalidates objects when generated parser headers change.
The stale, ignored parser was regenerated from the current grammar using ANTLR
4.10.1. No grammar change was needed.

## Evidence

CPU compilation was explicitly selected with `FORCE_CPU=1`. This host detects a
GPU automatically; GPU timings do not establish CPU thread scaling.

The A/B binaries use identical generated code and differ only in the linked
parallel runtime. Best of five fresh processes:

| Runtime | One worker | Four workers | One/four speedup |
|---|---:|---:|---:|
| Original merged runtime | 0.288 s | 1.027 s | 0.28× |
| Repaired runtime | 0.280 s | 0.186 s | 1.51× |

Every run prints `checksum 674218.0289763`. Traces change from
`requested=3 granted=1` to a bounded `requested=2 granted=2` for the compute loop.

- Compiler rebuild and final relink: passed.
- Parallel corpus: **68 passed / 1 failed** during the repair. Its only failure
  was the single-site debug-label assertion. The label was restored, then the
  complete failing assertion was replayed successfully: stateful-call refusal,
  TDG decision, and `checksum 200` at one and four workers. Thus all 69 assertions
  have passing evidence; this is not a claim of a second complete 69/0 run.
- Algorithm corpus: **7/0**. Motif corpus: **3/0**.
- Budget harness: passed with integration enabled, disabled, and one worker.
  New coverage checks cold registered sites and a parent budget of two.
- Joint scheduler tests: passed at **1, 2, 4, 8 workers**; ASan/UBSan passed at four.
  Coverage includes exhausted pools, actual grants, concurrent reservations,
  callback completion, nesting, and allocation/thread-creation fallbacks.
- DOACROSS profile/model regressions and six resource-equation tests: passed.
- Source comparisons confirm that the loop equations, profitability predicates,
  allocation solver, and all protected autotuner source files are unchanged.

Logs, A/B measurements, emitted IR, calibration inputs, and source-scope checks
are saved under `verify/bin/merge_integration_2026_10_14/`.

## Autotuner diagnosis — no changes made

The historical report's statement that calibration is not connected is incorrect.
`03_run.sh` creates/uses `~/.config/sgpl/hw_calib.json`; both the C++ and Python
models load it. This run used `L=64`, `t=9.232`, `T=2.4878`, `LLC=16777216`.
Missing class-rate calibration is a separate limitation, particularly for other
layout operations, but is not an explanation for disconnected CSR calibration.

There are two established mismatches:

1. **Execution counts.** `ExecutionCounts::multiplier` keeps a one-visit estimate
   for data-dependent loops. CC actually executes its step region six times.
   `autograph_profile_region_enter` stores the static prediction only once,
   whereas measured time accumulates across visits. SSSP's step has 22 visits in
   the captured run. The corpus compares these totals without reconciling the
   unknown dynamic count. Visits are not universally a correct multiplier:
   statically known counts are already included, and iterator visits differ
   from whole-graph passes.
2. **Work priced versus work timed.** CSR traversal pricing accounts for row
   navigation and neighbor traffic. The step timer brackets the executor,
   including callback work, state updates, frontiers, synchronization, and runtime
   overhead. It does not predict arbitrary algorithm-body work or active-frontier
   size per round. Hardware transfer calibration cannot supply these missing terms.

In the captured four-worker CPU probes, CC predicts **0.469 ms** versus **27.947 ms**
measured; SSSP predicts **2.344 ms** versus **79.675 ms**. These are diagnostic
single-run values collected alongside verification, not precision benchmarks.
Both fail the 5× criterion with the original and repaired budgeting runtimes;
predictions are identical between those runtimes and answers are identical.

The `pure_kernel_ns` counter is also not a region-only measurement: iterator
scans and executor calls contribute by operation kind without checking an active
region. It can include traversal outside the region boundaries. Do not substitute
that counter for a region total without matching its scope.

Finally, `AutoTunerPass.cpp` and `cost_model.py` are byte-identical between the
immediate pre-merge parent `b145e74` and merge `a019e0c`: they had already been
imported earlier. The autotuner group was not run before the merge. The existing
evidence therefore does **not** establish an equation regression caused by that
merge. It exposes counting and scope limitations in absolute-time validation.
These can affect conversion amortization and deserve a separate autotuner repair;
loosening the test thresholds would not correct them.
