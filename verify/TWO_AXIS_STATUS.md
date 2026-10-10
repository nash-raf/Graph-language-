# Two-axis DAG scheduler — status

Branch: `two-axis-dag-scheduler` — commits 7c50bd6 + d272c4a + 81c61a5 on
92e34c1, plus the working-tree changes (not committed): the staged dual
realization, the per-step dispatch ABI, the §15 allocation policy, the §19
fixture set + inventory gate, the GPU device-boundary contract, the harness
repairs and the environment guards.  Pod: `root@213.173.110.201 -p 18657`
(quota-capped container: 5.1 CPUs reported as 48, 31 GB reported as 251 GB) —
same content; pod HEAD is behind (739226c) with newer content as working-tree
changes.

## The contract, as implemented

Pipeline: `R1..R7 → (R_S,R_T) → (NI_S,NI_T) → (G_S,G_T) → schedule`
(`selectSchedule`, `graph_frontier_lowering.cpp`).

- **Complete certificate**: `supportedByAlgebra` evaluates every R1-R7 check
  without short-circuiting; both axes populated; the first refusal keeps the
  historical reason string; `Rewritable = Modelable && schedule != serial &&
  emitted` is the gate split.
- **All four realization cases emitted**: both clean → `nested`;
  `!R_S ∧ R_T` → `spatial-dag` (round sequence realizes R4's order);
  `R_S ∧ !R_T` → `temporal-dag` = the staged dual owner
  (`autograph_frontier_staged`: all U-owned work before all V-owned work as a
  tagged `RealizationOrder` edge, children internally partition-parallel);
  `R_S ∧ R_T`/R8/unrepresentable/failed emit → `serial` with the semantic vs
  implementation distinction carried by `impl_failure`.
- **Per-step ABI**: `autograph_exec_ctx_set_dag_axes` + `sgpl_ctx_dag_*`; the
  CPU dispatch sites read the declaration; `SGPL_DAG_SPATIAL=0` is the kill
  switch, `=1` forces the DAG dispatch for undeclared stages.
- **§15 allocation policy**: `share = min(max(1, W/(active+ready)),
  W·w/Σw_remaining)` — ready-set reservation, redistribution at every pop and
  completion, the measured per-partition pair counts as a *ceiling-only* work
  estimate (an estimate error can only under-grant; the TDG cost model's
  mismatch is bypassed here), nested dispatches clamped by
  `sgpl_current_thread_budget`, denial degrades to serial and never blocks.
  `SGPL_DAG_DEBUG=1` prints
  `nodes/rels/budget/peak/max_share/grants=multi/serial/weighted/completed/rc`.
- **§19 inventory gate**: `verify/sweep_certificates.sh` → the full certificate
  census (`verify/bin/certificate_census.txt`, 159 fixtures);
  `verify/check_census.sh` asserts every witness/discharge/schedule kind stays
  exercised, both one-axis splits appear, and R5 appears **zero** times (its
  predicate is unsatisfiable from the DSL under the RS-eligibility rule; the
  shadow case discharges R4 through the snapshot instead — see
  `shadow_snapshot.graph`; any future R5 fails the gate and needs its fixture).
- **Device boundary (GPU)**: `sgpl_gpu_step_schedule_ok(spatial, temporal)` is
  consulted at both device decision sites *before* the cost model; a stage
  whose spatial axis is held in order is refused the device (CPU fallback,
  identical answers, reason traced); the temporal axis never gates.  Staged
  dual children declare `spatial=1`, so the witness never removes device work
  and the device never violates the witness.

## Evidence (this round)

| check | result |
|---|---|
| `verify/run.sh parallel` (local) | **66 PASS / 0 FAIL** (adds `race/r3_unknown_provenance`, `race/r7_append_unwired`, `race/shadow_snapshot`; R1/R2/R4/R6 discharge assertions added to `data_index_write`, `claim_driver`, `mixed_regions`) |
| `test/run_exec_engine_tests.sh` | PASS — exec engine 0 failures at 1/3/8 + **dag scheduler 21/21** (partial-DAG overlap for temporal U1→U3 and spatial P1→P3 with the middle node independent, `[1,W]` grant bounds, weight ceiling capping the light node to a serial grant, nested clamp) |
| `test/run_exec_r2_tests.sh` / `run_frontier_shadow_tests.sh` | PASS / PASS |
| `validate_tdg_budget.sh` | 63 PASS / 0 FAIL in all three configs, incl. the five T11 device-boundary checks |
| `verify/check_census.sh` | CERTIFICATE INVENTORY: PASS — R1×2 (privatized), R2×9 (claim-staged), R3×1 (none → serial+impl), R4×2 (none→spatial-dag + privatized), R6×3 (none→staged + privatized), R7×2 (none → serial+impl), R5×0, nested 64 / spatial-dag 1 / temporal-dag 2 / serial+impl 5, splits 5 and 1 |
| device diff `carried_read_state` (pod) | P=1/4/8 identical, 16 device dispatches / 1 fallback, kernels.ptx 1918 B |
| device diff `dual_same_base` (pod) | P=1/4/8 identical, 0 device dispatches (the staged path emits no kernel — by design) |
| device diff `shadow_snapshot` (pod) | P=1/4/8 identical, 16 dispatches / 1 fallback |
| `gpu_corpus_diff.sh` (pod, 83 rows) | **0 DIFF**; 76 rows device-on == device-off answers; 6 rows `SIGKILL(rc=137: the program run was killed -- container memory limit)` (pagerank, dg_nbr_keyed, edge_write_v, edge_write_v5, edge_write_v_rev, nested_step — each reproduced and attributed); **0 BUILD-FAILED**.  The script now labels the SIGKILL rows honestly instead of `BUILD-FAILED`. |
| **One-axis-ordered property, measured** | direction A (`carried_read_state`, `R_S=0 R_T=1`): the declared schedule now *runs* — 16 partitions through the ready-work scheduler at **peak = 4** (was: the only pass fell into the serial profiling ramp, so the parallelism was nominal); `SGPL_DAG_SPATIAL=0` keeps the answer.  Direction B (`dual_same_base_big`, `R_S=1 R_T=0`): **two ordered unit runs** (`peak = 3`/`4`, U's run before V's — the staged call sequence), each unit internally parallel.  Non-commuting probe (`r6_order_sensitive`, multiply in U + add in V on the same base): `R_S=1 R_T=1` → **semantic serial** (`impl_failure=0`), answer == unrewritten at 1/4/8 — the staged realization is never applied to an order-sensitive shape. |
| `validate_algebra.sh` | **21/21 PASS** after refreshing the stale golden: 20 entries differed *only* by the `class=` field (added to the candidate print after the golden was written — semantic fields verified identical by stripping `class=` and diffing), and `mixed_same_array` additionally lost its former `same-base or cross-phase U+V` refusal text because the shape is now realized (staged dual; `dual_same_base.graph` + run.sh assertions).  The golden header records both deltas and `validate_algebra.sh` no longer aborts after the first diff (it prints every failure). |
| `validate_roundsep` / `validate_reduction` / `validate_rt_expr` | PASS / PASS / PASS |
| `validate_composition.sh` | 5 MATCH, no MISMATCH/LINK FAIL, DONE |

## Environment (measured; do not re-litigate)

- Pod: cpu quota 5.1 CPUs (`nproc` 48), memory 31 GB (host 251 GB),
  `/usr/bin/time` absent, NVPTX LLVM without Polly.  Heavy fixtures get
  SIGKILLed (rc=137) and 4-thread scaling is not assertable; `run.sh` folds
  both into SKIPs with the measured reason (`eff_cores` reads the cgroup quota).
- Local: 4 cores, no quota, `/usr/local/llvm-20-polly-rtti`.
- Harness: run `03_run.sh` via `bash`; `validate_composition.sh` from the
  compiler dir; standalone tests link `autotuner_runtime.c + parallel_runtime.c
  -lnlopt` and *stub* the GPU API (adding a GPU symbol to the runtime requires
  a stub in `exec_engine_test.c` and `dag_scheduler_test.c`); pass absolute
  fixture paths to `gpu_device_diff.sh`; `run.sh` rebuilds when `GraphProgram`
  is *missing* or stale, and the pod's rebuild needs
  `LLVM_CONFIG=/root/llvm-20.1.8-nvptx/bin/llvm-config` exported in the shell
  (the default in `build_lowmem.sh` is the local box's polly path).

## Open (explicit)

1. §15's ready-set reservation is implemented with the *measured* pair-count
   estimate; a future revision may add per-node *dynamic* re-estimation
   (the current estimate is computed once per dispatch).
2. R5 stays unreachable from the DSL by construction (documented above and
   enforced by `check_census.sh`); the upstream C-frontend's array-append
   `Activate` could reach it, but no `.graph` fixture can.
3. §8 refinement: the R6 constraint is realized as a single region-level
   `RealizationOrder` edge (all U-owned work before all V-owned work), which is
   a sound superset of the per-element order the witness needs; per-partition-
   pair constraints would expose more concurrency and are future work, not a
   contract gap (the §17 table's behavior is what the contract requires).
4. Pod refresh: **done**.  The pod's `GraphProgram` had been removed by a prior
   failed rebuild (`build_lowmem.sh` defaults to the local box's LLVM path);
   rebuilt with `LLVM_CONFIG=/root/llvm-20.1.8-nvptx/bin/llvm-config`, then
   pod suite **63 PASS / 0 FAIL / 4 environment SKIPs** (3× rc=137 memory
   kills, 1× 5.1-CPU quota scaling), pod engine tests 0 failures + dag
   ALL PASS, pod `validate_algebra` **21/21 PASS**, and the order-sensitivity
   probe identical on the pod (`ord_sum 1483477328`, serial == rewritten).
   `run.sh` now rebuilds when the binary is missing, so the harness is
   self-healing on both machines.  TSan (with ASLR disabled via `setarch -R`):
   **0 races** in both the DAG-scheduler and exec-engine suites.
