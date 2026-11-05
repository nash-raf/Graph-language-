# CPU layout-native execution

The CPU replacement is implemented on `codex/layout-native-execution`.
GPU execution retains the existing CleanCut backend, per the user's clarification.
No record-level execution DAG or new proof/transformation was added.

Read [RESULTS.md](RESULTS.md) for results and limitations, [LATEST.md](LATEST.md)
for resumable state, and [DESIGN.md](DESIGN.md) for execution/race obligations.
[WORK_LOG.md](WORK_LOG.md) retains failed attempts and intermediate discoveries.

The initial local checkpoint is `509f7602daa42d4c114c0b80d4b1d0e8a6959f77`.
No changes were pushed. Commits use the user's existing identity without
contribution credits. Final production is frozen as native07; native06 has the
same compiler and runtime behavior except one diagnostic message.

CPU timings are mixed: PageRank improved, BFS regressed. This implementation is
not an overall speed improvement. The full 244-case end-to-end campaign is in
[evidence/benchmarks/native07](evidence/benchmarks/native07).

Validation: 732 final full-vector checks pass; 29 of 31 existing suites pass.
Two suites retain checkpoint failures. Supplementary fixture comparisons,
forced-layout checks, ASan/UBSan and TSan checks passed. See RESULTS.md for exact
scope, test harness corrections and limitations; do not report all tests green.

## Reproduce or resume

Workspace: `D:\sgpl\Graph-language-\p1GraphEasy-con-AutoTuner`.
Native artifacts: `/home/kazi/p1_layout_native_20261105` under Ubuntu WSL.
Python: `/home/kazi/p1_local_regressions_20261019/venv/bin/python`.
Scripts: `/mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner/codex_doc`.

Run scripts from PowerShell with `wsl -d Ubuntu -- PYTHON SCRIPT ARGUMENTS`.
Use the paths above in place of PYTHON and SCRIPT. All benchmark and validation
scripts retain progress and hashes. Do not reuse a version name after changing
production source. Do not run two regression runners against the same version.

- `build_version.py NEW_VERSION`: freeze/build compiler and runtime.
- `run_native_tests.py NEW_ATTEMPT`: focused runtime checks; set
  `LAYOUT_TEST_SANITIZER=address,undefined` or `thread` with `env` before Python.
- `run_race_checks.py ATTEMPT`: retain/retry only recognized TSan startup failures.
- `run_regressions.py VERSION SUITES...`: existing suites; list in REGRESSION_PLAN.md.
- `check_layout_selection.py VERSION`: forced CSR/PCSR/BCSR/SET comparisons.
- `check_layout_selection.py VERSION temporal` or `claims`: differential fixtures.
- `run_autotuner_tests.py VERSION`: existing frequency/iterator/cost-model checks.
- `cpu_campaign.py build VERSION`, then `validate VERSION`, then `benchmark VERSION`.
  Run timing only after checks and with no competing build/test job.
- `publish_evidence.py VERSION`: copy compact evidence into this directory.

Frozen baseline/native07 objects have different context ABI versions. Never mix
their compiler-generated code and runtime objects. Full native logs and binaries
are retained in WSL; portable summaries, raw timings, input hashes and relevant
logs are committed under evidence. Large logs have excerpts and full-log hashes.
