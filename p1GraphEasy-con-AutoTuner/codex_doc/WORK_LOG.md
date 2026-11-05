# Work log

## Completed validation and benchmark

- Final native07 passes all 732 full-vector checks; final source audit passes.
- Corrected overlap probe and GPU batch test pass. 29/31 original suites pass;
  claims and temporal retain baseline failures. Claims remaining overlay gets
  through seven fixtures before another baseline classifier assertion. All 13
  claims and all 12 temporal fixtures pass independent differential comparisons.
- Recorded final >64 GPU gate behavior on baseline/native07, both batch modes,
  with durable logs. Original temporary batching failure directories disappeared;
  the focused reproduction avoids depending on those temporary paths.
- Completed all 2,440 timing samples with no build/test job concurrently active.
  Geomean speedup 0.976x; sum-of-medians 2562.03 -> 2830.82 ms. PageRank 1.165x,
  BFS 0.801x, SSSP 1.001x, CC 0.973x. No outliers removed. Median relative sample
  MAD 4.56%, so small differences are not established significance.
- BFS worst case uses 24 destination owners; repeated active-row scanning is an
  added cost. No phase profile proves its exact share. Cost model remains unchanged
  and does not price the new P-fold filtering explicitly. No performance claim.
- Replaced stale README/LATEST with completed state. Full results, failure scope,
  raw timing data, audit/hashes and resume commands retained under codex_doc.
- Local implementation commit prepared under user identity; no push/credits.


## Final production freeze native07

- native06 compiler/runtime logic passed all 732 full-vector checks and all 12
  temporal differential fixtures. native07 changes only debug reporting of the
  actual source/record counts; compiler SHA is identical. Reuse generated case
  objects only after exact compiler/source checks, relink all final runtimes.
- Latest runtime native15_asan passes ASan/UBSan at 1/2/6 workers.
- GPU batch suite's large >64-partition test expected a launch log after the
  existing early capability gate. Baseline fails identically; outputs match.
  Test-only update verifies early rejection, absence of device effects and output
  equality to supported width. Original and updated hashes/logs are retained.
- No timing samples yet. See LATEST.md for pending work.


## native06 validation milestone

- Frozen CPU-only native06 built and passed all 732 independent full-vector
  algorithm/graph/worker checks. Baseline also passed all 732. SSSP/CC may execute
  existing whole-stage certified specializations, so absence of a generic native
  preparation log is expected and is not evidence of a fallback to CleanCut.
- 36 forced-layout compiled comparisons passed, with activation evidence. Native
  staged and round/spatial paths now participate in layout selection; earlier
  native05 returned correct results but kept those paths on CSR.
- Current runtime sanitizer evidence: native13_asan and native14_tsan, 1/2/6.
  Only compiler round lowering changed since ASan source capture, not runtime.
- Existing autotuner frequency/iterator/cost-model suites pass. Updated the
  frequency fixture's old CleanCut-only API assertion, preserving all counts.
- Broad suites exposed an existing membership_call_aliases compiler rejection:
  raw CSR vs bitmap-query layout conflict. Reproduced identically on baseline;
  logs/fixture hashes in native06/known_failures. Remaining claims overlay skips
  only that separately recorded failure and keeps all remaining assertions.
- Existing round/spatial overlap probe wrapped only build_clean_cut to force two
  partitions. Updated probe/linker to wrap build_layout_batches too, retaining
  its original peak=4 and ordering assertions. Test-only overlay retained; rerun
  pending after the current regression runner completes.
- Existing temporal suite fails its first expected classifier-log substring on
  baseline too (reference numeric output 414 matches). Baseline xtrace retained.
  A separate full-stdout differential of all 12 temporal fixtures is running.
- No timing samples collected yet; finish regression review first.

## CPU-only clarification and forced-layout ordering fix

- Received explicit answer: CPU first; preserve GPU backend. Restored GPU builder,
  context preparation and record reuse/construction paths. Removed exploratory
  native-device code/tests. CPU single/dual composition stays layout-native.
- native04 froze that CPU-only scope. No CPU benchmarks were run on exploratory
  GPU-modified versions. Stopped native03 regression process tree after ten
  passing suites; native03 case compilation had completed all 244 cases.
- New compiled forced-layout harness found native03 PCSR failed before effects:
  autotuner emitted ensure_layout after prepare_layout. CSR-only fixtures missed
  this integration bug. Full logs and IR retained in native03/layout_checks.
- Added AutoTunerPass preparation-boundary anchoring, preserving the original
  executor as profile end. Native05 freeze/build in progress. See LATEST.md.

## Latest implementation checkpoint: native02/native03 (2026-11-05)

- native01 completed all 732 full-vector checks and all 15 selected regression
  suites. Each algorithm/graph case logged layout-native preparation.
- Extended compiler routing to all single-stage and dual-owner composition
  builders, both backends. Legacy composition gets a representation-only bind;
  existing legality/schedule remains unchanged, no invented spatial proof.
- Added native device record construction: two stable topology passes directly
  from selected layout, followed by existing GPU grouping. Ordered arrays reuse
  existing native iterator record builder. CPU fallback retains native context.
- Added byte-exact device records/offsets/groups differential tests for 4 layouts
  and both owners. native09_asan passed these plus prior tests at 1/2/6 workers.
- Added new prep/build names to existing optimization whitelists after inspecting
  their preparation-only contracts. Relax proof worklists remain available on
  CSR through their existing builder; these are not CleanCut or a record DAG.
- native02 frozen build is running. Subsequent working changes (early GPU cost
  estimate, relax worklist integration, scratch ownership cleanup) require a new
  native03 freeze; do not call native02 the final working implementation.
- Pending: final freeze/build, comprehensive suite rerun, expanded TSan, CPU
  validation and timings, GPU correctness/fallback checks, path/contract audit.
- All pre-existing CleanCut ABI functions remain for older callers/tests. New
  compiler lowering no longer emits build_clean_cut/build_array_cut. Remaining
  legacy GPU step hook is disabled on native contexts (dedicated spatial GPU
  submission remains supported). Document this compatibility boundary honestly.

## Latest checkpoint: native01 validation (2026-11-05)

- Baseline and native01 compiler/runtime and 244 case pairs built successfully.
- Baseline independent complete-vector validation passed all 732 runs.
- Native01 validation is running. Initial harness combined stdout/stderr, causing
  autotuner diagnostic text to interrupt printed vectors; separated streams and
  retained old log. This was a parsing failure, not a numerical mismatch.
- native06_asan and expanded native07_asan passed at 1/2/6 workers. The latter
  checks no source/push edge payload, allocation failure, and stale publication.
- native05_tsan passed 1/2/6. Two-worker and six-worker runs each required two
  retries for recognized pre-main mapping failures; all logs are retained in
  tests/native05_tsan/race_results.json. No reported race in successful runs.
- Existing suites additionally passed array_preparation_exec,
  array_frontier_lease, graph_loader, and execution_simplify (12 total so far).
- Four initial regression failures were harness environment omissions, not
  accepted passes: local-contract opt-in and campaign baseline variables added;
  corrected reruns in progress. No nonexistent graph_storage_layout shell suite
  should be invoked (there is a C++ fixture).
- Audit found new preparation API absent from several optimization whitelists.
  Do not time as final candidate until these are reviewed and integrated.
- No benchmarks have run. No push. Wholesale CPU/GPU removal remains pending.

## 2026-11-05: checkpoint

- Read repository status and checked ancestor AGENTS.md locations; none found.
- Reviewed changed and untracked file inventory.
- Committed all existing changes as 509f7602daa42d4c114c0b80d4b1d0e8a6959f77.
- Created codex/layout-native-execution; no push.
- Began durable documentation before implementation.
- Tests and benchmarks for the replacement have not run.

## Baseline and architecture audit

- Added build_version.py: immutable source freeze, content-addressed compilation,
  native Linux artifacts, compiler and object SHA-256 identities.
- System python3 import failed because scipy is absent. Retried with existing
  /home/kazi/p1_local_regressions_20261019/venv/bin/python; baseline build passed.
- Frontend main.cpp and graph_frontier_lowering.cpp and changed runtime units
  were compiled. Exact matching cached dependency/flag objects were reused.
- Confirmed generic CPU traversal uses CleanCut source pairs/destination rows;
  ordered array frontiers use separate occurrence records. Existing optimized
  fullgraph mode 6 already uses a stable incoming index for selected CSR cases.
- Confirmed selected native layout has a common neighbor iterator for CSR,
  PCSR, BCSR, SET. The iterator preserves layout-specific occurrence semantics.
- Confirmed destination grouping cannot simply be changed to source grouping
  at runtime: selected snapshots, private folds, and source coverage are emitted
  by the compiler and remain proof obligations.
- No benchmark or replacement correctness result yet.

## First native execution prototype (not enabled by compiler yet)

- Added layout_execution_runtime.inc and explicit range/preparation APIs.
- Native source tasks walk owned source rows. Destination tasks filter the
  selected native traversal without making edge records. This repeats topology
  scans per destination batch; performance is unmeasured and may regress.
- Array frontiers retain source copies, degree prefixes and ordered append slots,
  not per-edge work records. Existing lifecycle/snapshot/fold execution remains.
- Extended execution context with private state and bumped ABI 4 -> 5; baseline
  and candidate objects must never be mixed.
- native01/native02_asan failed: initial dispatch hook was in the retained legacy
  body, while execution_partition_bodies.inc contains the active extracted body.
  Added hook to the active body. Logs preserved under native Linux tests/.
- Corrected destination local-context source_state preservation and use of the
  existing atomic-aware context-copy helper. Preserve full-domain destination
  partition boundaries separately from source/array boundaries.
- native03_asan is the next run (started before the context-copy correction);
  rerun under a new attempt name after any source change.
- Compiler routing, optimized pass integration, comprehensive regression checks,
  race detector execution, GPU replacement, and benchmarks remain outstanding.

## native01 build and initial regressions

- native03_asan completed successfully at 1, 2, 6 workers. native06_asan reruns
  current code (atomic-aware context copy) and retains separate evidence.
- native01 compiler/runtime built from frozen source. Accepted CPU spatial and
  round/spatial lowering now selects range-only batches and native preparation.
  GPU and legacy lowering remain unchanged; this is not wholesale completion.
- Existing suites passed: spatial_exec, array_frontier_exec, round_spatial_exec,
  spatial_static, spatial, array_frontier_lowering, round_spatial_lowering,
  context_copy. Full logs/results are in versions/native01/regressions.
- TSan startup initially failed with unexpected memory mapping (PIE) and an
  empty pre-main SIGSEGV (non-PIE). A subsequent non-PIE launch reached main and
  finished all one-worker checks. run_race_checks.py retains bounded startup
  retries and never retries a race report; 2/6-worker race checks remain pending.
- cpu_campaign.py added. Building baseline and native01 executables for 244
  algorithm/graph cases (BFS, SSSP, CC, PageRank; 61 graphs). No timing run yet.
  Uses existing venv (system Python lacks scipy), native Linux output, fixed
  physical CPUs, independent reference validation at workers 1/2/6, and alternating
  whole-process samples only after validation. Full campaign remains pending.
