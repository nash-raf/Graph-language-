# Merge report: origin/effect_interim into two-axis-dag-scheduler

> Update, 2026-10-14: the budgeting integration is repaired and verified. The
> autotuner remains unchanged; the calibration claim below is corrected in
> [MERGE_INTEGRATION_FIX.md](MERGE_INTEGRATION_FIX.md). The rest of this file
> preserves the original merge investigation and its historical measurements.

> The subsequent [same-machine autotuner comparison](AUTOTUNER_BRANCH_COMPARISON.md)
> measures both parents and the merged revisions, resolving the accuracy
> attribution against the requested `effect_interim` baseline.

This file records the merge, the three failures that it caused, the fix for one of
them, the full verification we ran, and what is still open.

## 1. State

| Item | Value |
|---|---|
| Merge commit | `a019e0c` (parents `b145e74` ours, `59355db` effect_interim) |
| Commits from this work | `e50c3a2`, `8e574d1`, `d487ec6`, `b145e74`, `a019e0c`, `f2ac0fe` (all unpushed) |
| Compiler build | rc=0, 0 errors |
| `verify/run.sh parallel` | 68 passed / 1 failed (was 69/0 before the merge) |
| `verify/run.sh algo` | 7 passed / 0 failed |
| `verify/run.sh motif` | 3 passed / 0 failed |
| Fixtures | `two_reduce_slots` privatized, `priv_two_slots_big` privatized, `reduce_real_ops` sequential |
| Machine used | 4 CPUs, 15 GB, no reachable GPU driver (`user-B660M-Pro-RS`) |

The three failures below all belong to code that came from the effect_interim
branch. One is fixed. Two are open, and both have a measured cause.

## 2. What the merge brought, and what it kept

The merge touched 3151 files: 28 code files under `p1GraphEasy-con-AutoTuner/`
(outside `test/` and `proof/`), 12 test files, 3110 proof artifacts, and 1 file
outside the compiler tree.

### 2.1 Conflicts (12 files and 2 rename collisions)

| File | Conflict lines | Resolution |
|---|---|---|
| `graph_frontier_lowering.cpp` | 393 | ours; their `taskEffectObject` work dropped (0 references remain) |
| `parallel_runtime.c` | 185 | mixed: their prologue and machinery, our planner body restored |
| `main.cpp` | 97 | theirs plus our 25 lines (our helper and its call sites) |
| `pdg.cpp` | 61 | mostly theirs; 56 lines are ours (managed-dispatch guard, dag-owned-nest inline) |
| `proof/R2_REJECTION.md` | 697 | ours |
| `test/run_exec_r2_tests.sh` | 45 | theirs |
| `test/run_exec_engine_tests.sh` | 39 | theirs |
| `autotuner_runtime.c` | 26 | ours; their CleanCut partition-locking version dropped |
| `test/run_frontier_red_tests.sh` | 24 | theirs |
| `test/run_frontier_shadow_tests.sh` | 24 | theirs |
| `proof/algebra_golden_classify.txt` | 5 | ours (golden) |
| `gpu_runtime.c` | 5 | both registry locks kept (settled in `b145e74`) |
| `test/exec_engine_test.c`, `test/frontier_shadow_test.c` | rename collision | our newer copies kept; their moved copies already imported in `d487ec6` |

### 2.2 Ownership after the merge

From the effect_interim branch:

- `pdg.cpp`: their classifier and analysis, plus their effect work
  (`WholeTaskEffects` x10, `declaredTaskEdges` x4).
- `parallel_runtime.c`: their DOACROSS and DOALL predicates
  (`sgpl_should_parallelize_doacross`, `sgpl_should_parallelize_doall`), their
  budget and calibration functions (`sgpl_nlopt_budget_constraint`,
  `sgpl_calibrate_thread_launch_overhead_ns`,
  `sgpl_get_thread_launch_overhead_ns`, `sgpl_loop_parallel_model_time_ns`,
  `sgpl_loop_serial_model_time_ns`), their 5-argument
  `sgpl_evaluate_level_loop_plan`, and the include of `tdg_scheduler.inc` (44 KB).
- `main.cpp`: their two helper functions.
- `cost_model.py` (40 KB) and `AutoTunerPass.cpp` (80 KB).
- Support files: `tdg_scheduler.inc`, `tdg_trace.inc`,
  `experimental/tdg_joint_{schedule,trace}.inc`, `tdg_budget_test.c`,
  the `tools/tdg_*` and `tools/cost_model_*` scripts, `CLEANCUT_LOOP_SPEC.md`,
  `EFFECT_SYSTEM.md`.
- Tests: the `test/pdg_tdg_verification/` tree (45 files),
  `test/doacross_cost_validation/`, three top-level audit scripts, four runner
  scripts.
- 3110 proof artifacts.

Kept from our branch:

- `autotuner_runtime.c` (unchanged), `graph_frontier_lowering.cpp`,
  `gpu_runtime.c`, `proof/R2_REJECTION.md`,
  `proof/algebra_golden_classify.txt`, `test/exec_engine_test.c`,
  `test/frontier_shadow_test.c`.
- `verify/`: effect_interim changed nothing here, so the whole harness merged
  with no conflict.
- Inside `sgpl_choose_tdg_threads_with_loop_budget`: our planner, plus four
  restored helpers (`sgpl_plan_single_site`, `sgpl_tdg_model_time_ns`,
  `sgpl_tdg_launch_overhead`, `sgpl_tdg_effective_threads`) and two constants
  (`SGPL_TDG_BASE_LAUNCH_NS`, `SGPL_TDG_LAUNCH_PER_THREAD_NS`). Their branch had
  deleted all six, and the merged callers need them.

## 3. Problem 1: the scan loop classification (fixed)

- Check: `race/doacross_scan` in `verify/run.sh`. It compiles
  `verify/cases/parallel/doacross_scan.graph` with `SGPL_LOOP_CLASSIFY_DEBUG=1`
  and looks for `classification=DOACROSS` in the log.
- Error: `FAIL race/doacross_scan scan not classified DOACROSS (no DOACROSS coverage)`
- Compiler trace:
  ```
  [loop-classify] fn=main hdr=loopcond depth=1 carrier=1 non-positive constant distance=-1
  [loop-classify] fn=main hdr=loopcond depth=1 classification=SEQUENTIAL hasProvenCarriedDep=1 hasUnschedulableCarriedDep=1
  ```
- The case is a prefix scan: `pref[i] = pref[i - 1] + 1.0;`
- Cause: the merged `pdg.cpp` attributes the -1 direction of the carried
  dependence at carrier level 1. The rule `isSchedulableAtCarrier()` accepts only
  `ProvenPositiveConstant`, so the -1 direction set
  `hasUnschedulableCarriedDep`, and the loop became SEQUENTIAL. A DOACROSS
  schedule paces the iterations and inserts the syncs at the exact distance, so a
  proven constant is orderable whatever its sign.
- Evidence that effect_interim causes it: our branch passed this check before the
  merge (69/0). The check failed when the merge brought `pdg.cpp` only, while
  `parallel_runtime.c` was still ours. The rejection rule text is identical in
  both versions, so the carrier attribution is the difference.
- Fix, commit `f2ac0fe` (11 insertions, 1 deletion in `pdg.cpp`):
  ```cpp
  // A proven constant distance is orderable whatever its sign: a DOACROSS
  // schedule paces the iterations and inserts the syncs at the exact distance.
  // Unknown and sign-unknown proofs still fail closed through
  // isSchedulableAtCarrier below.
  static bool isOrderableCarriedDistance(const DistanceProof &Proof)
  {
      return Proof.kind == DistanceProofKind::ProvenPositiveConstant ||
             Proof.kind == DistanceProofKind::ProvenNonPositiveConstant;
  }
  ```
  The single call site in the carrier attribution switched from
  `isSchedulableAtCarrier` to `isOrderableCarriedDistance`. Every other proof
  kind (`Unknown`, `ProvenPositiveUnknown`, `ProvenNonZeroUnknownSign`,
  `ProvenZero`) still fails closed, so the fail-closed guards that came with the
  merge are untouched.
- Result: the trace reports `classification=DOACROSS hasUnschedulableCarriedDep=0`.
  No race row changed its verdict. `parallel` moved from 67/2 to 68/1.
- Note: this diverges from the effect_interim classifier, so the trace is worth
  sending back to its author.

## 4. Problem 2: the vertex loop speed (open)

- Check: `scaling/doall_scaling`. The harness runs the compiled program with 1
  thread and with 4 threads, keeps the best of five wall-clock values, and
  requires a speedup of at least 1.2x. The row already has a load gate that
  skips the assertion when the 1-minute load average is above the core count.
- Error: `FAIL scaling/doall_scaling 4thr only 0.25x of 1thr (0.31s -> 1.24s); expected >=1.2x`
- The case runs 200 rounds of a `for v in G.nodes` loop over `g20k`, which is
  5.02M iterations.
- Pre-merge measurement of the same case on the same machine: 0.31 s at 1 thread,
  0.14 s at 4 threads (2.2x).
- Cause: at 4 threads the merged runtime executes the loop through the scope
  route. The runtime log shows:
  ```
  [tdg.univ] input work=42 span=42 tasks=1 level_budget=4 loop_sites=0
  [tdg.univ] decision=serial final_threads=1 reason=serial-single-task
  [tdg.plan] level entries=0 budget=4 loop_budget=0 idle=0
  [tdg.profile] path=serial-budget-scope elapsed_ns=1254878608 refined_units=5019514
  ```
  That path costs 250 ns per iteration, against 62 ns per iteration on the
  1-thread route. The scope has no level-loop site registered for this loop
  (`loop_sites=0`), so the planner grants `loop_budget=0`, decides serial, and
  the loop runs through the slower scoped execution.
- Ruled out: idle pool workers that steal cores (the pool blocks on
  `pthread_barrier_wait`), machine load (the 4x appears as single-threaded scoped
  work, and the A/B reproduces), and lost metadata (the collector
  `sgpl_collect_level_loop_candidates` is byte-identical between the two trees,
  72 lines, empty diff, and the loop is still classified DOALL).
- Fix, open: when a scope grants no loop budget, or the decision is serial,
  execute the DOALL loop through the ordinary parallel loop runtime. That is what
  the pre-merge code did. A deeper option is to register the loop as a level-site
  candidate, so the budget split hands it threads.

## 5. Problem 3: the cost-model accuracy (open)

- Checks: `autotuner/cc prediction accuracy` and `autotuner/sssp prediction accuracy`.
- Errors:
  ```
  FAIL autotuner/cc prediction accuracy measured/predicted = 6.3x (>5x off)
  FAIL autotuner/sssp prediction accuracy measured/predicted = 5.6x (>5x off)
  ```
- Cause: the merge brought `cost_model.py` and `AutoTunerPass.cpp` from the
  effect_interim branch. They supply the predictions now, and the calibration is
  not connected.
- Limit of the evidence: the autotuner group did not run before the merge, and
  the effect_interim harness has no autotuner rows. The branch probably causes
  this failure. We did not compare the two states.
- Fix, open: connect the calibration (`hw_calib_bench.c`, `class_calib.c`,
  `shift_state_bench.c` are imported), or change the two limits together with the
  model's author.

## 6. Failures that the merge does not cause

- `lang/keyword_INF`: `COMPILE: line 2:4 no viable alternative at input 'intINF'`.
  The token `INF` is a parser literal at `Base.g4:330`, so `int INF` lexes as one
  identifier. This predates the merge. Decision open: value keyword, or normal
  identifier.
- Four test scripts return code 127 and one more fails in the same area, because
  the command `llvm-config-20` is absent. Correct link target:
  `/usr/local/llvm-20-polly-rtti/bin/llvm-config`.
- `run_graph_loader_tests.sh`: `undefined reference to gpup_step_v_try`. The link
  line omits `gpu_runtime.c`. Before the merge the same script failed with
  `autograph_gpu_step_name_for`, and the effect_interim branch has the same
  defect, so this is a defect of the test.
- `test/pdg_tdg_verification/run_all.sh`: cannot find `llvm/ADT/DenseMap.h`. The
  compile line needs the LLVM include path.
- `run_gpu_invocation_tests.sh` aborts on `gpu_invocation_test.c:245`
  (`sgpl_gpu_execute_plan(&p)==0`) before and after the merge.
  `run_worker_pool_tests.sh` fails the same three checks before and after.
- The GPU rows cannot pass on this PC, because the driver is not reachable.

## 7. Verification we ran, in order

1. Compiler build after each step: `bash p1GraphEasy-con-AutoTuner/build_lowmem.sh`
   -> rc=0, 0 errors.
2. Fixture classes after each step: privatized / privatized / sequential.
3. Step gates while resolving the merge:
   - compiler files merged, runtime still ours: `parallel` 68 passed / 1 failed.
   - runtime merged with their policy tail: 67 / 2.
   - our planner restored: 67 / 2.
   - all three files resolved: 67 / 2.
   - after the classification fix: 68 / 1.
4. `bash verify/run.sh all` before the fix: 98 passed / 5 failed
   (`keyword_INF`, `doall_scaling`, `doacross_scan`, cc accuracy, sssp accuracy).
5. After the fix: `algo` 7/0 and `motif` 3/0, so no computed result changed.
6. A/B experiment on `doall_scaling`, same machine:
   - our pre-merge tree: 0.31 s at 1 thread, 0.14 s at 4 threads.
   - merged tree: 0.31 s at 1 thread, 1.24 s at 4 threads.
7. 23 unit scripts under `p1GraphEasy-con-AutoTuner/test/`. Results and
   attribution are in section 6.
8. Standalone baseline on effect_interim: not possible. Their committed
   `Base.g4` is byte-identical to the merge-base grammar, while their
   `ASTBuilder` needs rule names that no committed revision defines, so their
   branch does not build on its own. The per-file isolation above is the
   substitute.

## 8. Verification files

- `verify/run.sh`: the corpus harness. 141 assertion rows, 20 of them class
  assertions, 85 `.graph` cases, 26 fixture data files. Groups: `all`, `lang`,
  `algo`, `parallel`, `autotuner`, `motif`. Run with bash:
  `cd verify && bash ./run.sh parallel`.
- `verify/VERIFICATION_INVENTORY.md`: every verification surface with its state.
- `verify/PARALLEL_FAILURES_ANALYSIS.md` and `verify/R3_REGRESSION_ANALYSIS.md`:
  the earlier analyses, with status notes.
- `verify/MERGE_THREE_FAILURES.md`: this file.
- `p1GraphEasy-con-AutoTuner/test/pdg_tdg_verification/`: the imported PDG and TDG
  verification tree (TLA+ task-safety spec, exec and frontier tests, runners,
  certificate checkers).
- `p1GraphEasy-con-AutoTuner/test/run_*_tests.sh`: the per-feature unit scripts.

## 9. Reproduction commands

```bash
cd verify && bash ./run.sh parallel

cd p1GraphEasy-con-AutoTuner
SGPL_LOOP_CLASSIFY_DEBUG=1 GRAPH_FILE=../verify/cases/parallel/doacross_scan.graph bash ./03_run.sh

GRAPH_FILE=../verify/cases/parallel/doall_scaling.graph bash ./03_run.sh
SGPL_TDG_DEBUG=1 SGPL_NUM_THREADS=1 /usr/bin/time -f 'T=%e' ./final_program
SGPL_TDG_DEBUG=1 SGPL_NUM_THREADS=4 /usr/bin/time -f 'T=%e' ./final_program
```

## 10. Environment, and why the runs are slow

- The work ran on the PC (`user-B660M-Pro-RS`): 4 CPUs, 15 GB, and the NVIDIA
  driver is not reachable, so GPU rows cannot run here.
- One compiler build takes about 5 to 6 minutes. One corpus group takes about 12
  to 13 minutes. Each merge step needed both.
- The 4-core box loads itself during a corpus run, so the scaling row had to be
  measured alone.
- A pod with more cores and a working GPU would remove most of this cost. The
  toolchain for the merged build script lives at
  `/usr/local/llvm-20-polly-rtti`; the runtime needs `libnlopt`; the parser must
  be regenerated with ANTLR after a fresh clone, because `generated/` is
  gitignored.

## 11. Open items

1. Problem 2: route a budget-less DOALL loop through the ordinary parallel loop
   runtime, then re-run `parallel` (expect 69/0) and `algo` and `motif`.
2. Problem 3: connect the cost-model calibration, or change the two limits.
3. `lang/keyword_INF`: decide value keyword or normal identifier.
4. Environment: link `llvm-config-20` to
   `/usr/local/llvm-20-polly-rtti/bin/llvm-config`, and add `gpu_runtime.c` to
   the `run_graph_loader_tests.sh` link line.
5. Optional: pull `test/pdg_tdg_verification/` runner expectations once the
   budget handoff is fixed.
