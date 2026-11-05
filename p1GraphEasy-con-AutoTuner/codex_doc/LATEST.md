# Completed task / resume state

CPU-first replacement implemented, validated, and benchmarked on branch
`codex/layout-native-execution`. GPU backend preserved. No record-level DAG.
Baseline local checkpoint: `509f7602daa42d4c114c0b80d4b1d0e8a6959f77`.
Implementation commit subject: `Execute CPU owner batches over autotuned graph layouts`.
No push. Existing user identity, no contribution trailers.

Final production freeze: **native07**. Identical compiler to native06; the only
runtime difference is a corrected diagnostic count. ABI is 5: never mix old and
new compiler/runtime objects. final_audit.py checks production against the freeze.

Completed evidence:
- Final native07 and baseline each pass 732 full-vector checks at 1/2/6 workers.
- Native06 core passes 36 forced-layout comparisons, 12 temporal and 13 claims
  differential fixtures at 1/2/6, and three autotuner suites.
- Current runtime ASan/UBSan (native15_asan), TSan (native14_tsan) pass 1/2/6.
- 29/31 existing suites pass, including all seven GPU suites. Claims and temporal
  suite failures reproduce on baseline; details and test-only corrections in
  RESULTS.md. They are not marked passed.
- All 2,440 CPU whole-process timing samples collected over 244 cases. Overall
  geometric-mean speedup 0.976x; sum of case medians rises 10.49%. PageRank improves
  (1.165x), BFS regresses (0.801x). This is NOT an overall performance improvement.

No build/test/benchmark processes remain from this task. No further task steps
are required. Before extending it, read RESULTS.md and DESIGN.md. The known next
performance issue is repeated destination filtering (P scans of active native
rows); the old cost model does not price it explicitly. Keep the existing proof,
occurrence ordering, source lifecycle and join obligations. Do not claim universal
race freedom from finite tests.

All scripts and compact evidence are in this folder. README.md gives commands.
Full frozen source/binaries/logs: `/home/kazi/p1_layout_native_20261105` (Ubuntu WSL).
Python: `/home/kazi/p1_local_regressions_20261019/venv/bin/python`.
Repo scripts: `/mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner/codex_doc`.
Never overwrite a frozen version after changing production. Use a new version
and new test attempt. Do not run concurrent regression runners in one version;
they share program.o. Do not time while other build/tests are active.
