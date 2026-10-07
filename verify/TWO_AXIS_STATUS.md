# Two-axis DAG scheduler — status / resume pointer (keep updated every turn)

Branch: two-axis-dag-scheduler   Tip: 739226c   (tdg-engine-budget = 3779928 checkpoint)
Pod: root@213.173.108.40 -p 22295 -i ~/.ssh/id_ed25519  (48 cores, 251 GB)
Pod tree: /workspace/IMTalker/p1/Graph-language-/p1GraphEasy-con-AutoTuner (branch checked out, GraphProgram built)
Pod toolchain: /usr/local/llvm-20-polly-rtti (synced); build: PATH=/usr/local/llvm-20-polly-rtti/bin:$PATH bash build_lowmem.sh  (~2-4 min)
Note: fresh pods need apt: libisl-dev libxml2-dev libzstd-dev libedit-dev libffi-dev zlib1g-dev libtinfo-dev

DONE (verified):
- step 1 witnesses/certificate/diagnostics (6c76718, 7728b8d)
- step 2 gate is single certificate producer; verdict = !(RS|RT|R8); MISMATCH self-checks (ac623e5)
- step 3 RAW/WAR/WAW relation layer (c849d87)
- step 4 spatial/temporal templates + validation (aee97a8)
- steps 5+6 runtime ready-work scheduler + test ALL PASS (5772cfc)
- step 7 runtime half: sgpl_exec_dag_spatial (739226c)

DONE: step 7 complete + verified.  Wiring is env-gated (SGPL_DAG_SPATIAL,
default OFF) at both CPU partition dispatch sites (fallback and
sgpl_exec_step_task); device path and calibration pass untouched.
The differential first FAILED, root cause was the DAG threads not carrying
the worker-index TLS; fixed by sgpl_set_current_worker_index() (new in
parallel_runtime.[ch]) called in dag_worker with a per-thread ordinal.
Strict differential (profile lines excluded): bfs_level answers IDENTICAL at
P=1,4,8,16, flag off vs on ("reached 20000 level_checksum 75722").
DAG path is slower on this small graph (8.46 vs 5.53 ms kernel) -- scheduler
overhead, a cost-model input later.

RACING detail (final): TSan on the DAG test + engine test = 0 warnings (after
fixing the pool's env caches to relaxed atomics).  TSan on the BFS program with
SGPL_DAG_SPATIAL=1 found one more real race in the step-9 code (dag_dispatch_budget
read R->active outside the run lock) -- fixed by computing the share under the
lock; the TSan BFS run is now 0 warnings.  The loader's OMP-hash loop reports a
libgomp barrier false positive (this libgomp has 0 TSan annotations), only when
OMP threads > 1.

NEXT:
- Optional: enable SGPL_DAG_SPATIAL by default after a perf review (small-graph
  overhead 8.5 vs 5.5 ms), retire compat switches after more regression
  coverage.  Nothing pending from the spec steps.

VERIFICATION (this batch, all on the pod + locally):
- corpus mismatch scan: 0 mismatches, no crashes (all verify/cases + test graphs)
- validate_algebra.sh: 52/53 -- the 1 FAIL (ultimate_pagerank) is the
  pre-existing class= golden diff, not from this work
- dag_scheduler_test: ALL PASS (14 checks incl. 5 temporal-wrapper checks)
- exec_engine_test: 0 failures (partitions 1,3,8; temporal chain, nested budget,
  TLS-budget-balance invariants)
- BFS differential SGPL_DAG_SPATIAL=0 vs 1: IDENTICAL at P=1,4,8,16
  ("reached 20000 level_checksum 75722"); repeat-run determinism: 1 distinct
  output hash over 3 runs
- RACING: TSan 0 warnings on dag_scheduler_test and exec_engine_test after
  fixing a real race (worker_main hit plain-int env caches; now relaxed atomics)
- budget sweep re-run: reproduces the documented model mismatch
  (model light=7 heavy=7 vs measured light=1 heavy=13)
- GPU demo (separate ask): RTX 2000 Ada, ~55% util / 24% memory / 3968 MiB,
  see verify/GPU_MATMUL_DEMO.md

