Dynamic TDG scheduler promotion, October 9, 2026

The normal sgpl_run_tdg_level entry now uses allocation plus ordering within
one level. Each callback uses one parent worker; only declared loops receive
extra workers. No build flag is required. The compiler defaults to dynamic;
current and joint-experimental remain compatibility aliases. The ordinary-first
production policy and compiler option have been removed. Its backup is test-only.

Production scheduling and profiling are in tdg_scheduler.inc and tdg_trace.inc.
The experimental include paths are compatibility wrappers. The baseline and
fallback now use one FIFO queue without an ordinary-first barrier. Cold,
unsupported and nested levels use FIFO. Graph/GPU levels use FIFO pending Phase B.
Existing thread accounting, loop engines and loop cost equations are retained.
Autotuner source hashes are unchanged: see integration_validation.json.

Validation passed:
- Default runtime resource, model, cache and adaptation tests at 1/2/4/8 workers.
- Thread-budget gates, including disabled optional sharing; six equation tests.
- Generated numeric SGPL with default, dynamic and compatibility aliases.
- Eleven generated sibling-graph fixtures: serial equivalence, warmed execution,
  allocation/extraction failures, and level certificates.
- Formal proofs, safe-model checks and expected unsafe-model counterexample.
- ASan/UBSan on runtime tests and both mixed native policies plus all 1,444
  forced B4 short-ordinary configurations.
- Legacy design-probe structural checks at 1/2/4/8 workers.
- Historical ongoing CSV: exact source hashes and original byte prefix verified.

Known pre-existing failure: parallel_append_runtime_test reports an append-order
mismatch. The promoted runtime and frozen reference fail identically. This
integration does not claim to fix that unrelated test.

Fresh promotion sweep: four workloads, seven case/budget settings, three fresh
processes and three samples per configuration. All 23,662 legal configurations
of the test-only ordinary-first, dynamic admission and FIFO executors checked.
213,084 physically timed executions; 355,896 checked levels including warmup.
The forced oracle is measured once and referenced under both policy labels;
those duplicate CSV references are not independent measurements.
Mixed levels are constructed by the runtime adapter from actual SGPL-generated
loop callbacks and serial SGPL function invocations. Compiler grouping is not
tested by these mixed fixtures; generated compiler tests are separate.

Main sweep: dynamic wins 6/7 settings; summed execution time is 17.5% lower.
Fresh-process holdout: dynamic wins 6/7; summed time is 19.0% lower, from 1,260
additional timings. Compute B6 remains an exception: dynamic 1.508838 ms versus
ordinary-first 1.434600 ms (5.17% slower). Neither policy reliably selects the
observed optimum on heavy compute/skew. This is a default preference, not a
claim of universal superiority.

The ongoing CSV was not merged with this different source revision. before/
freezes pre-promotion sources; after/ freezes the promoted sources and tools.
See their manifests. Promotion CSVs, tables and Matplotlib plots are in mixed/.

Reproduce from repository root in Linux/WSL:
  export SGPL_JOINT_RESULTS_DIR=proof/tdg_dynamic_default_2026_10_09
  python3 tools/tdg_joint_build.py
  python3 tools/tdg_joint_validate.py
  python3 tools/tdg_joint_validate.py --generated
  python3 tools/tdg_joint_validate.py --sanitizers
  python3 test/tdg_allocation_sweep/mixed.py --cases mixed_short_ordinary mixed_one_long_ordinary mixed_compute mixed_loop_skew --budgets 4 6 --samples 3 --out proof/tdg_dynamic_default_2026_10_09/mixed --no-append
  python3 test/tdg_allocation_sweep/mixed_holdout.py proof/tdg_dynamic_default_2026_10_09/mixed
  python3 test/tdg_allocation_sweep/mixed_sanitizers.py proof/tdg_dynamic_default_2026_10_09/mixed
  python3 test/tdg_allocation_sweep/check_exports.py proof/tdg_allocation_sweep
