# Regression campaign

Core regression version: native06; final native07 changes only diagnostics. Runner stores cumulative results and skips
previous passes; retain failed attempts. Do not run two runners against the same
frozen source copy concurrently (scripts share program.o). Run from PowerShell:

```powershell
wsl -d Ubuntu -- /home/kazi/p1_local_regressions_20261019/venv/bin/python /mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner/codex_doc/run_regressions.py native06 run_spatial_exec_tests run_array_frontier_exec_tests run_round_spatial_exec_tests run_spatial_static_tests run_spatial_tests run_array_frontier_lowering_tests run_round_spatial_lowering_tests run_context_copy_tests run_execution_simplify_tests run_fullgraph_tests run_relax_simplify_tests run_frontier_simplify_tests run_array_preparation_exec_tests run_array_frontier_lease_tests run_graph_loader_tests run_parallel_claims_tests run_round_spatial_tests run_temporal_tests run_temporal_waw_tests run_static_schedule_tests run_empty_partition_effects_tests run_effect_fusion_tests run_general_frontier_optimization_tests run_program_array_tests run_gpu_four_tests run_gpu_batch_tests run_gpu_sync_tests run_gpu_storage_tests run_gpu_invocation_tests run_gpu_ir_tests run_gpu_schedule_tests
```

The runner supplies explicit local-test opt-in, baseline/campaign paths and GPU
build arguments; it removes CPU force variables for GPU suites. The scripts'
assertions, reference comparisons and failures remain unchanged. Evidence:
`/home/kazi/p1_layout_native_20261105/versions/native06/regressions/`.

Focused native fixture: `test/layout_execution_test.c`. Use a new attempt name
whenever source changes. `run_native_tests.py ATTEMPT` supports
`LAYOUT_TEST_SANITIZER=address,undefined` or `thread`. For recognized pre-main
TSan address mapping failures only, run `run_race_checks.py ATTEMPT`; all failed
launches remain, and an actual race/correctness report stops immediately.
