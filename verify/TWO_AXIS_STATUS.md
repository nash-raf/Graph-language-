# Two-axis DAG scheduler — status

Branch: `two-axis-dag-scheduler` — commits `7c50bd6` + `d272c4a` + `81c61a5` on
`92e34c1`, then `104beb9` (latest, == `origin/ARS-1`), plus the proof-layer
commits `53473fc` (sections 3-10) and `45dc1ec` (section 11).  Pod:
`root@213.173.110.198 -p 10574` (quota-capped container: 5.1 CPUs reported as
48, 31 GB reported as 251 GB; LLVM 20.1.8 from apt.llvm.org with NVPTX, antlr4
4.13.1 installed under /usr/local, libpolly-20-dev + libisl-dev, RTX 2000 Ada).

## The contract, as implemented

Pipeline (plan sections 3-10):
`effects → footprints → complete RAW/WAR/WAW → per-relation temporal/owner
projections → transformations → phase + round + spatial graphs → AxisProof →
schedule → runtime descriptors`.

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
- **Proof-derived verdict (new)**: `selectSchedule` consumes `Info.Proof`
  (section 9).  R8 keeps its guard/reason; an inconclusive (overflow, invalid,
  cyclic) or unrealizable proof fails closed to serial+impl; otherwise the
  effective axes are the certificate **or** the graph — the graph may only be
  stricter.  A surviving `CrossOwner` proof edge adds the tagged
  `RealizationOrder` edge the staged dual needs even with no witness.
- **Per-step ABI**: `autograph_exec_ctx_set_dag_axes` + `sgpl_ctx_dag_*`; the
  CPU dispatch sites read the declaration; `SGPL_DAG_SPATIAL=0` is the kill
  switch, `=1` forces the DAG dispatch for undeclared stages.
- **§15 allocation policy**: `share = min(max(1, W/(active+ready)),
  W·w/Σw_remaining)` — ready-set reservation, redistribution at every pop and
  completion, the measured per-partition pair counts as a *ceiling-only* work
  estimate, nested dispatches clamped by `sgpl_current_thread_budget`.
- **§19 inventory gate**: `verify/sweep_certificates.sh` → the census;
  `verify/check_census.sh` asserts every witness/discharge/schedule kind stays
  exercised and R5 appears zero times.
- **Device boundary (GPU)**: `sgpl_gpu_step_schedule_ok(spatial, temporal)`
  consulted at both device decision sites before the cost model; the spatial
  axis gates, the temporal axis never does; refusal falls back to CPU with
  identical answers.
- **Proof layer (§3-§11)**: footprints with read+write modes and resource
  footprints; complete RAW/WAR/WAW with an overflow gate; temporal projection
  at both scopes; owner resolution (region is provenance, not ownership);
  transformation records with their conditions; three proof graphs with
  unary obligations, components, cycle checks; `AxisProof` three-valued
  verdict; runtime descriptors with fail-closed validation plus
  `sgpl_exec_dag_spatial_graph`, `sgpl_exec_dag_temporal_units` and
  `sgpl_exec_dag_product`.

## Evidence (this round)

| check | result |
|---|---|
| `verify/run.sh parallel` (local) | **69 PASS / 0 FAIL** (includes the §14.11 overflow check and the §13 adversarial completeness check) |
| `test/run_exec_engine_tests.sh` (local + pod) | ALL PASS — exec engine 0 failures at 1/3/8 + dag scheduler ALL PASS incl. 17 new §11 checks |
| TSan (`setarch -R`, 8 threads) | dag scheduler **0 races**, exec engine **0 races** |
| `validate_algebra.sh` | **21 fixtures, 0 failures**; `validate_tdg_budget.sh` ALL PASS; `validate_composition.sh` DONE |
| `verify/check_census.sh` | **PASS** over 161 fixtures (nested 60, spatial-dag 5, temporal-dag 3, serial 1, serial+impl 5; splits R_S=1 R_T=0 ×6, R_S=0 R_T=1 ×1) |
| `verify/check_graphs.sh` | case corpus: 52 candidates, **cover_S 52/52, cover_T 52/52**; **3 strictness deltas** (`maycross_same_region` with no witness at all, plus the two R8-guarded `reduce_real_ops` min/max candidates) |
| adversarial completeness | `maycross_same_region`: certificate `R_S=0 R_T=0`, graph connected (3 surviving `MayCrossOwner` edges) → **serial + impl_failure=1** ("surviving edge without a realization"), answers == unrewritten at 1/4 threads (local and pod) |
| overflow fail-closed | `SGPL_REL_CAP=2` on `mixed_regions` → serial+impl with the overflow reason, answers == unrewritten (local and pod) |
| pod suite | **64 PASS / 0 FAIL / 4 SKIP** (quota scaling 6 CPUs < nproc 48; three `SIGKILL(rc=137 — container memory limit)` cases: nested_step, marker_derived, derived_default) |
| pod device diffs | `carried_read_state` / `shadow_snapshot`: P=1/4/8 identical (16 dispatches, 1 fallback each); `dual_same_base`: identical, 0 dispatches (staged path, by design); `maycross_same_region`: identical, **0 dispatches** (the proof-refused spatial axis denies the device step) |
| pod corpus device diff | **85 rows, 0 DIFF** (79 identical), 6 attributed `SIGKILL(rc=137)` rows, **0 BUILD-FAILED** |
| proof-driven schedule | suite unchanged where the graph agrees with the certificate; on the strictness deltas the realization becomes more conservative (serial+impl), never more parallel |

## Environment (measured; do not re-litigate)

- Pod: cpu quota 5.1 CPUs (`nproc` 48), memory 31 GB, `/usr/bin/time` absent,
  RTX 2000 Ada 16 GB, LLVM 20.1.8 (apt.llvm.org) with NVPTX, antlr4 4.13.1
  under `/usr/local` (the distro's 4.10 is too old for the generated parser),
  `libpolly-20-dev` + `libisl-dev` for the link line.  The pod's build uses
  `POLLY_LIBS="" LLVM_CONFIG=/root/llvm-cfg` (static LLVM): its
  `libLLVM-20.so` registers Polly's options internally, so linking the static
  Polly archives as well aborts with "Option 'polly-dependences-computeout'
  registered more than once".
- Local: 4 cores, no quota, `/usr/local/llvm-20-polly-rtti`, antlr4 4.13.1.
- Harness: run `03_run.sh` via `bash`; `validate_composition.sh` from the
  compiler dir; standalone tests link `autotuner_runtime.c + parallel_runtime.c
  -lnlopt` and *stub* the GPU API; pass absolute fixture paths to
  `gpu_device_diff.sh`; `run.sh` rebuilds when the binary is missing.

## Open (explicit)

1. §15's ready-set reservation uses the measured pair-count estimate; dynamic
   per-node re-estimation remains future work.
2. R5 stays unreachable from the DSL by construction (enforced by
   `check_census.sh`).
3. §8 refinement: the R6 constraint is realized as a single region-level
   `RealizationOrder` edge; per-partition-pair constraints would expose more
   concurrency (sound superset today).
4. The emitted programs declare their schedule through
   `autograph_exec_ctx_set_dag_axes`; passing the compiler's *descriptor array*
   to the new `sgpl_exec_dag_*_graph`/`_units`/`product` entry points is the
   remaining section 11/12 wiring.
5. `Tail`: the strictness deltas found by `check_graphs.sh` are the empirical
   counterexamples to "no witness ⇒ parallelize"; each one either gains a
   transformation or stays serial, and the next sweep re-checks them.

