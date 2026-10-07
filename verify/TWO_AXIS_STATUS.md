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

FINDING (step 7 wiring, this turn): SGPL_DAG_SPATIAL=1 differs from the
default path on bfs_level (diff non-empty). Root cause: the DAG scheduler's own
pthreads do not carry the runtime's worker-index TLS, so partition code that
reads sgpl_current_worker_index() (lane assignment in frontier append) behaves
differently. Fix next: route DAG node execution through the runtime worker pool
(or set the worker-index TLS in dag_worker) before enabling SGPL_DAG_SPATIAL;
flag is OFF by default so default behavior is unchanged.

NEXT (in order, commit each verified):
1. step 7 wiring: env-gated SGPL_DAG_SPATIAL dispatch in autotuner_runtime.c
   - line ~3587 fallback dispatch, planned dispatch past ~3617; DO NOT touch ~3599 (device) or ~3610 (calibration, desired_threads(1,1))
   - then BFS differential: flag off vs on, identical answers
2. step 8 temporal wrappers (round lifecycle stays in autograph_frontier_execute)
3. step 9 nested scheduling + budget threading
4. step 10 metadata sgpl.frontier.dag.* + markSequential replacement + pdg/outliner exclusions
5. step 11 enable + docs (EFFECT_SYSTEM.md, proof/EFFECT_ALGEBRA_DESIGN.md) + compat switch
6. VERIFICATION: corpus mismatch scan, validate_algebra.sh (52/53, class= diff on ultimate_pagerank is pre-existing),
   dag_scheduler_test, tdg budget sweep, RACING: TSan build of dag test, worker-count sweeps, repeat-run determinism,
   helgrind/TSan on BFS with SGPL_DAG_SPATIAL=1

Recipes: TU check /tmp/cc_gfl.sh; dag test link g++ -Wl,--gc-sections; verify script /tmp/verify4.sh
