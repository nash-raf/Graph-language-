SGPL allocation and scheduling validation

Dynamic default integration (October 9, 2026)
---------------------------------------------
The normal runtime entry sgpl_run_tdg_level now uses the dynamic allocation and
ordering model. No build flag or compiler option is needed. Ordinary callbacks
use one worker; only declared loop sites receive extra workers. Planning covers
one level, using its actual available budget. Cold, unsupported and nested
levels use a bounded FIFO queue. Graph/GPU levels also use FIFO pending Phase B.
The ordinary-first production implementation and compiler option are removed.
The test-only ordinary_first_reference.inc and frozen proof snapshots retain
that policy for reproducible comparisons; production has no dependency on them.

The ongoing proof/tdg_allocation_sweep CSV records the PRE-INTEGRATION runtime.
Its verifier accepts only exact source hashes, either current or frozen before
promotion in proof/tdg_dynamic_default_2026_10_09/before. Keep new measurements
separate because the dynamic baseline/fallback is now FIFO, not ordinary-first:
  python3 test/tdg_allocation_sweep/mixed.py --out proof/tdg_dynamic_default_2026_10_09/mixed --no-append
New manifests use oracle_version=2 and include backend 2 (FIFO) alongside
backend 0 (test-only ordinary-first) and backend 1 (dynamic admission executor).
Old manifests retain their original enumeration. Never combine versions or
source revisions into one comparison without identifying them separately.

Default integration correctness checks (Linux/WSL):
  export SGPL_JOINT_RESULTS_DIR=proof/tdg_dynamic_default_2026_10_09
  python3 tools/tdg_joint_build.py
  python3 tools/tdg_joint_validate.py
  python3 tools/tdg_joint_validate.py --generated
  python3 tools/tdg_joint_validate.py --sanitizers

Mixed ordinary-task extension (October 2026)
--------------------------------------------
Historical mixed-suite command (use a separate output after source changes):
  python3 test/tdg_allocation_sweep/mixed.py
Smoke without appending:
  python3 test/tdg_allocation_sweep/mixed.py --cases mixed_short_ordinary --budgets 4 --samples 3 --out proof/tdg_allocation_sweep_mixed_smoke --no-append
Verify saved mixed coverage and actual phase ordering:
  python3 test/tdg_allocation_sweep/mixed.py --check-only --out proof/tdg_allocation_sweep
Independently recompute mixed statistics and the comparison chart:
  python3 test/tdg_allocation_sweep/mixed_report.py proof/tdg_allocation_sweep
Fresh-process confirmation of fixed winners (no reselection):
  python3 test/tdg_allocation_sweep/mixed_holdout.py proof/tdg_allocation_sweep
ASan/UBSan of both native policies and every B4 short-ordinary schedule:
  python3 test/tdg_allocation_sweep/mixed_sanitizers.py proof/tdg_allocation_sweep
Rebuild combined CSVs/tables/Matplotlib plots from saved raw timings:
  python3 test/tdg_allocation_sweep/run.py --analyze-only

Eight mixed SGPL fixtures add one, two or three ordinary siblings to two DOALL
loops. Test overloaded levels as well as levels that exactly fit or have spare
parent capacity, short/medium/long serial work, balanced loops and loop skew.
Each ordinary callback invokes the SGPL function sgpl_ordinary_work(rounds,1).
Its scalar recurrence is deliberately serial, and its IR is checked to contain
no parallel loop dispatch. These are ordinary tasks despite the serial while
inside their bodies: no declared parallel loop site and no additional workers.
The test adapter instantiates the ordinary callbacks in the same synthetic
runtime level as the two SGPL-generated loop callbacks. It does not change
compiler grouping, production scheduling, cost equations, or autotuning.

Enumerate every positive loop width 1..B-1, every priority order, every parent
count 1..min(task_count,B), and both backfill settings of the existing joint
executor. Ordinary callbacks remain width 1. Permutations of identical ordinary
tasks (same code, rounds and seed) are deduplicated; distinct duration classes
retain all relative orders. Forced ordinary-first configurations reserve parents
according to its actual two-phase rule when total tasks exceed B, otherwise
according to its single task queue. order in raw/schedules is an integer code:
for backend=1, code//2 indexes mixed_manifest.json case.orders and code%2 is
backfill; backend=0 uses the integrated current policy. allocations.csv and
tables.html decode the best schedule into readable task names.

Each native policy is measured before any forced timings in its own fresh
process, with 8 serial training levels and 12 native warmups. Randomize policy
process order between repeats. The common forced oracle runs once per repeat
after the dynamic native measurement. Its rows are shared under both policy
labels for compatible exports; they are references to the SAME physical timing,
not independent reruns. mixed_manifest.json reports physical_timed_samples.
This gives both policies exactly the same comparison baseline. Default mixed
settings: 3 fresh-process repeats, 5 timings/config, 2 warmups/config, shuffled
configuration order. Instrumented callback boundaries check actual ordinary-first
phase completion on every overloaded ordinary execution, callback counts,
results, loop grants and budget balance. Checks occur outside the level timer.
Callback boundary timestamps have a small common cost included in both policies.

Raw mixed data, builds, traces and metadata live under proof/tdg_allocation_sweep/
mixed/. Completed data append to the existing root raw.csv; schedules.csv,
allocations.csv, allocation_columns.csv, summary.csv and policy_comparison.csv
are regenerated from all rows. Original raw rows are compared before/after
append and retained exactly. The original exports are backed up in
two_loop_snapshot/ and the original two-loop manifest/builds are retained.
Rerunning the same mixed cases replaces their rows instead of duplicating them.
The empirical best is the minimum over this executor's tested policies and
settings, not over arbitrary OS interleavings, preemption or start delays.
The many-config minimum is optimistic; min/max process medians and native
bootstrap intervals do not eliminate selection bias.

Original two-loop suite
----------------------

Run from the repository in Linux/Ubuntu WSL:
  python3 test/tdg_allocation_sweep/run.py
On Windows:
  wsl -d Ubuntu bash -lc 'cd /mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner && python3 test/tdg_allocation_sweep/run.py'
Smoke run (three independent processes, both policies):
  python3 test/tdg_allocation_sweep/run.py --cases short --budgets 4 --samples 3 --out proof/tdg_allocation_sweep_smoke
Recreate plots/tables from saved data:
  python3 test/tdg_allocation_sweep/run.py --analyze-only
Repeat representative cases and diagnose bounded search vs equations:
  python3 test/tdg_allocation_sweep/run.py --cases tiny uneven_chunks balanced_compute trip_skew work_skew --budgets 6 --repeats 5 --samples 15 --diagnose --out proof/tdg_allocation_sweep/confirmation
Check the test adapter and runtime with ASan/UBSan after a build:
  python3 test/tdg_allocation_sweep/run.py --sanitizers-only
Independently verify saved coverage, exported statistics and source hashes:
  python3 test/tdg_allocation_sweep/check_exports.py proof/tdg_allocation_sweep

Requires the existing LLVM20/ANTLR/compiler toolchain, GCC/G++, NLOPT,
Python3, NumPy and Matplotlib. No GPU or graph-layout autotuning is exercised.
Sources are regenerated deterministically from the case specifications in run.py.
The harness raises its subprocess stack limit to 64 MiB for SGPL stack arrays.
All generated SGPL fixtures remain in cases/. Builds are incremental for the
compiler, but fresh for each workload and adapter. Results use a fixed shuffle
seed; settings, CPU, affinity and source/object hashes are saved in manifest.json.

Scope and legality
------------------
This is a runtime-model unit validation using real SGPL-generated DOALL loops.
The numeric compiler currently emits these independent source loops in separate
TDG levels. A linker adapter combines their callbacks only in the test executable.
Compiler grouping is therefore NOT validated by these allocation sweeps.
Each callback has its own separately allocated array; its only argument is that
array. The harness checks the emitted environment shape and fails if it changes.
It invokes both callbacks before SGPL consumes either output. No generated loop
body, production model, runtime scheduling algorithm, or autotuner is modified.
A temporary runtime copy adds two boundary grant-observation calls, shared by
both policies. Production sources are hashed before/after the complete run.

Width 1 means serial loop execution on its task's parent. A parallel width p>=2
requires p extra workers, because its parent waits. These are NOT p-1 workers.
For ordinary-first with two parent callbacks and budget B, enumerate ALL positive
integer pairs satisfying e(p1)+e(p2)<=B-2, e(1)=0 and e(p)=p for p>=2.
Unused capacity is allowed; an optimal allocation may leave workers idle.
For dynamic scheduling, enumerate ALL p1,p2<=B-1 (also capped by trip counts),
both admission orders, and one-parent execution. Enumerate two-parent execution
when 2+e(p1)+e(p2)<=B. Otherwise the plan necessarily serializes admission and
uses one parent. Backfill makes no additional choices for just two callbacks.
The dynamic oracle also includes the ordinary executor, matching its fallback.
The single DOALL pool serializes parallel teams; parallel loops are not assumed
to overlap merely because their thread reservations fit.
These are all legal widths under the tested executors, not all possible operating
system interleavings or arbitrary timing delays. There are no ordinary sibling
tasks in the original nine fixtures: use the mixed extension above for the
ordinary-first phase rule itself.
DOACROSS dependencies and Phase B graph sibling scaling need separate fixtures;
these results must not be generalized to those paths.

Measurement and interpretation
------------------------------
Each case/budget/policy is run in three fresh processes by default. Each process
first trains with eight useful serial SGPL level executions and twelve policy
warmup executions, then records nine model executions. It next evaluates every
legal forced allocation/schedule in a randomized order, with two untimed warmups
and nine timed executions per configuration. Models never see sweep results
before making the recorded choices. Model measurement comes before the sweep;
repeat-to-repeat variation and process medians expose some, not all, clock drift.
Timers enclose the complete two-callback level: planning, dispatch, synchronization
and useful work. Source allocation/zeroing, checks and printing are outside it.
Ordinary forced allocations keep the ordinary executor's planning overhead.
Joint forced schedules replay fixed plans without joint search overhead. The
model/oracle ratio therefore measures attainable runtime including the model's
planning cost; it is not solely a test of the mathematical width prediction.
summary.csv separates allocation_only_ratio (best schedule at the model's actual
width pair), selected_schedule_replay_ratio (replay its actual width/order/parent
choice), and model_over_best_ratio (native runtime including planning). A model
can choose the fastest allocation while its measured runtime ratio exceeds 1
because of timing noise or planning overhead. These must not be conflated.

Forced runs override the profitability gate at the loop boundary and verify the
actual grant exactly matches the requested width. They do not use force-borrow
environment flags. Native model runs preserve the profitability gate; both the
requested widths and actual granted widths are recorded. Orange bars mark every
actual allocation used by the model, rather than hiding unstable choices.
Purple diamonds mark the planner's assigned widths; these can differ from the
executed widths because the per-loop profitability gate can choose serial work.
Each bar is the median across process medians of the best tested schedule for
that allocation. Error bars show the min/max process medians. The red dashed
line is native model time, including its planning. The green outline marks the
fastest measured configuration. The complete schedule table is schedules.csv;
allocations.csv collapses schedules only for plotting. raw.csv retains every
sample, allocation_columns.csv supplies allocations as columns, and tables.html
contains the complete readable tables and plots.

Summary bootstrap intervals resample paired process medians (4000 draws); with
only three processes, these intervals are coarse. Selecting the minimum among
many noisy configurations is optimistic. The fastest observed allocation is an
empirical optimum for this workload, machine and run, not a universal proof.
Use more repeats/samples and reproduce on other machines before changing models.
--diagnose exhaustively scores frozen learned profiles after the native model
measurements and before the oracle. DIAG_SEARCH and DIAG_SCORE lines in traces/
record the bounded search's evaluations, planning time, chosen widths, and each
alternative's predicted cost. No alternate plan is fed back to the native model.
The 10% and 25% classifications are reporting tolerances, not correctness claims.
