# Verification inventory

Everything that verifies this compiler, where it lives, and how to run it.
State measured on `dedbc33` + the `effect_interim` free imports (2026-10-13).

## 1. Corpus harness — `verify/run.sh`

- **141 assertion rows** (`ok`/`no` pairs), **20 of them `expect_class`** verdict assertions
  (parallelization/proof classes). Groups: `all | lang | algo | parallel | autotuner | motif`.
- Inputs: **85 `.graph` programs** under `verify/cases/` (`algo`, `lang`, `parallel`, `motif`)
  and **26 graph data files** in `verify/fixtures/` (`tiny.txt`, `g20k.txt`, …).
- Run: `cd verify && bash ./run.sh parallel` (or `all`).
- Last measured: **parallel 69 passed / 0 failed**; **lang 19 passed / 1 failed**
  (`lang/keyword_INF`: `INF` is a parser literal, so `int INF` lexes as one ID — open decision).

## 2. Compiler unit tests — `p1GraphEasy-con-AutoTuner/test/`

- C/C++ tests: `exec_engine_test.c`, `exec_forkjoin_test.c`, `exec_snapshot_test.c`,
  `exec_source_cover_test.c`, `dag_scheduler_test.c`, `frontier_motif_test.c`,
  `worker_pool_test.c`, `gpu_invocation_test.c`, `round_spatial_exec_test.c`, `tdg_budget_test.c`,
  `array_frontier_exec_test.c`, `budget_alloc_test.c`, `spatial_static_test.cpp`, … (21+ sources).
- Runner scripts: `run_worker_pool_tests.sh`, `run_temporal_waw_tests.sh`, `run_exec_engine_tests.sh`.

### 2a. PDG/TDG verification tree (imported from `effect_interim`, commit *this one*)

`test/pdg_tdg_verification/` — 45 files, previously absent from this branch:

- **Formal (TLA+):** `TDGTaskSafety.tla`, `TDGTaskSafetyProof.tla`, `TDGTaskSafety.cfg`,
  `TDGTaskSafety_unsafe.cfg`, `PDGEdgeCoverage.tla` + `PDG_SOUNDNESS.md`, `TDG_TASK_SAFETY.md`.
- **Execution tests:** `exec_engine_test.c`, `exec_forkjoin_test.c`, `exec_snapshot_test.c`,
  `exec_source_cover_test.c`, `frontier_motif_test.c`, `frontier_owner_pull_test.c`,
  `frontier_red_test.c`, `frontier_shadow_test.c`, `autotuner_frequency_driver.cpp`.
- **Runners:** `run_all.sh`, `run_exec_engine_tests.sh`, `run_exec_r2_tests.sh`,
  `run_frontier_{motif,owner_pull,red,shadow}_tests.sh`, `run_pdg_argmem_expansion.sh`,
  `run_tdg_formal.sh`, `build_tdg_validation.sh`.
- **Checkers:** `audit_pdg_argmem_calls.py`, `check_tdg_level_certificates.py`.
- **Duplicates to reconcile:** this folder's `exec_engine_test.c` / `exec_forkjoin_test.c` /
  `exec_snapshot_test.c` / `exec_source_cover_test.c` / `frontier_motif_test.c` differ from the
  top-level copies of the same names; the top-level ones are the newer generation
  (`exec_engine_test.c` 695 vs 546 lines) and were left untouched.
- Also imported alongside: `test/doacross_cost_validation/` (probe + `model_test.c` + python
  diagnostics), `test/audit_pdg_argmem_calls.py`, `test/build_tdg_validation.sh`,
  `test/check_tdg_level_certificates.py`.

## 3. Proof-layer smokes — `p1GraphEasy-con-AutoTuner/proof/*/`

Seven smoke directories (e.g. `tdg_allocation_sweep`, `tdg_joint_schedule`, `tdg_level_design`)
with their own build/compare scripts.

## 4. Grammar limits suite — `test/grammar_limits_suite.py` (38 cases)

Char-by-char surface checks (declarations, loops, arrays incl. 2-D, functions, sets, membership,
mutations, all comprehension/motif spellings, error cases). **Not in the tree**: it is part of the
stashed work (`git stash@{0}`: `removeNodes` membership fix + this suite) — `git stash pop` to restore.

## 5. Analysis documents (not tests, but written against measured runs)

`verify/R3_REGRESSION_ANALYSIS.md`, `verify/PARALLEL_FAILURES_ANALYSIS.md` (both carry status notes
on what has since been fixed upstream), `verify/PLAN_CONFORMANCE.md`, `verify/PLAN_PROGRESS.md`.

## Known-red / open

- `lang/keyword_INF` (grammar keyword vs identifier — open decision).
- The five `effect_interim` code files are **not** merged yet: `parallel_runtime.c`, `pdg.cpp`,
  `autotuner_runtime.c`, `main.cpp`, `gpu_runtime.c` — until then some of
  `test/pdg_tdg_verification/*` runners may fail because they encode that branch's behavior.
