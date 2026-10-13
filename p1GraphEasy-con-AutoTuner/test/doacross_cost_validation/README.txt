DOACROSS cost-model accuracy validation

Run from the repository in WSL Ubuntu:
  python3 test/doacross_cost_validation/run.py
  python3 test/doacross_cost_validation/run.py --render-only
  python3 test/doacross_cost_validation/run.py --sanitizers --out proof/doacross_cost_worker_work_2026_10_09/sanitizers
  python3 test/doacross_cost_validation/verify.py
  python3 test/doacross_cost_validation/regressions.py

Requires GCC/G++, AVX2 support (existing bitmap implementation), NLOPT and
Matplotlib. --threads, --repeats, --samples and --out customize the sweep.

This validates the production DOACROSS parallel cost equation directly.
It does not test the universal thread allocator or invoke the graph-layout
autotuner. The source_manifest.json snapshot and scope_validation.json verify
that the sources frozen for each measurement run stayed unchanged during that run.
The original run stays in proof/doacross_cost_validation_2026_10_09. The earlier
synchronization-only correction stays in proof/doacross_cost_updated_2026_10_09.
The current per-worker work model defaults to proof/doacross_cost_worker_work_2026_10_09.

Prediction uses the exact sgpl_loop_parallel_model_time_ns helper, whose
DOACROSS expression matches the per-loop chooser's parallel_rhs:
  launch + ceil(N/P)*(c_dep + c_ind
         + waits_per_iteration*sigma_wait(P) + posts_per_iteration*sigma_post(P)).
Dependent computation is useful worker work. There is no additional global
N*c_dep charge; measured blocking waits already carry dependency delays.
The serial original still costs N*(c_dep+c_ind). Unobserved widths rely on
seeds, which cannot reliably describe the dependency chain's blocking time.
Synchronization observations are specific to actual execution width, trip-count
bucket and descriptor signature. Unseen/evicted entries use seeds. The bounded
16-entry cache per loop does not borrow another width's blocking costs. Samples
accumulate in worker-local rows and merge after join, then update an EWMA once
per completed loop. The CSV sync_samples field counts width-matched batches
(0 seeded, 4 learned here), whereas the original run counted iteration updates.

Each configuration starts a fresh process. Eight serial executions feed real
block timing into sgpl_record_doacross_serial_sample. Clock calls and accumulation
outside the sampled interval remain outside c_dep/c_ind, as with compiler block
profiling. This is the existing model's input convention, not fitted useful-work
coefficients. Dependent and independent work occupy separately timed blocks.

A seeded forecast uses the existing synchronization seeds, with calibrated
launch overhead. Four separately checked parallel warmup runs supply normal
wait/post profile observations. A learned forecast freezes that runtime state
before seven held-out timings. Neither forecast incorporates the holdout timings.
The same held-out actual time is compared with both forecasts. Duplicated CSV
rows are two forecast comparisons against one physical timing, not two samples.

Parallel execution uses the public parallel_for_runtime call and its existing
per-iteration profile_enter/profile_exit wrapper around real wait/post operations.
Setup, resetting arrays and full-output checking are outside measured intervals.
There are no per-iteration validation counters in the timed body. Every output
is checked against serial execution. Index-varying recurrence inputs prevent
constant fixed-point outputs from hiding missing dependency execution.

The harness deliberately measures the parallel path even when the chooser
would select serial. This tests the predicted parallel service time and does
not claim a dispatch-policy speedup. No forced model coefficients are used.

The 13 kernels cover 257 to 1,000,000 trips, dependency distances 1/3/4/64,
dependent compute, independent compute before wait or after post, and two
independent synchronization streams. Updated default widths are 2 through 8.
Every kernel has actual data dependences. These are direct C runtime probes
with compiler-like profiling boundaries, not SGPL compiler integration tests.

Summary values are medians of per-process medians. Signed prediction error is
100*(predicted/measured-1). Negative means underestimated. Absolute error is
its magnitude. Aggregate mean/median absolute errors give equal weight to each
completed kernel/width configuration. Startup calibration is excluded from
the warm parallel prediction and held-out service time.

An output assertion or 30-second timeout invalidates the entire kernel/width
configuration for accuracy statistics, even if other processes passed. Such
rows appear as FAILED/n.a. in plotted tables. Partial successful timings remain
in raw.csv. failures.csv and per-process traces preserve the failed checks.
The harness does not repair runtime failures or turn them into timing results.

Outputs: raw.csv, summary.csv, accuracy.json, accuracy_by_threads.csv,
manifest.json, environment.txt, source hashes/snapshots, per-process traces,
and PNG/SVG/PDF tables. --render-only regenerates tables from saved data.

verify.py compiles model regression tests against the frozen source, checks them
normally and under ASan/UBSan, reconstructs every exported prediction, and writes
before_after.csv plus a Matplotlib accuracy comparison table. Comparisons use
only configurations completed in both the previous correction and new run; each uses
its own calibration and actual timings. They are accuracy comparisons, not a
controlled paired speedup benchmark. regressions.py checks scheduler resources,
budgets and numerical oracles without linking the autotuner. The compiler and
completion-ring wait/post protocol are unchanged. This is not race-detector
validation or an end-to-end SGPL-generated DOACROSS accuracy suite.

Read-only equation ablations and timeout diagnosis:
  python3 test/doacross_cost_validation/diagnose.py

This requires the original validation's frozen source/build and CSV files.
It writes proof/doacross_cost_diagnosis_2026_10_09. All equation candidates
reuse the original pre-holdout inputs and actual held-out timings, with no
fitted constants. They do not measure a changed dispatch policy or validate
prediction at widths that have never been sampled.

The diagnostic probe adds worker progress observations and a watchdog to an
isolated test copy. It includes unchanged production synchronization from
the frozen source. These instrumented timings are not performance evidence.
Exit 90 means the watchdog captured a stall; it is an expected diagnostic
outcome, not a successful kernel execution. Runs stop on the first failure.
The separate --slot-replay mode deterministically demonstrates completion
slot regression through unchanged production post helpers. Production
source hashes are checked against the validation snapshot at completion.
