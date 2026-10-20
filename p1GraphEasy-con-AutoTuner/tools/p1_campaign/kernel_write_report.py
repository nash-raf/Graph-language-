"""Render the audited kernel investigation as a self-contained text report."""
import kernel_gap as K
R=K.R
from pathlib import Path
import statistics,sys,time
def avg(xs):return statistics.mean(xs)
def table(headers,rows):return '\n'.join([' | '.join(headers),' | '.join('---' for _ in headers)]+[' | '.join(str(x) for x in row) for row in rows])+'\n'
def measurements(file,phase=None):return [x for x in R.read(R.OUT/file) if not phase or x['phase']==phase]
def value(records,a,g,label,key='internal_seconds'):return avg(x[key] for x in records if x['algorithm']==a and x['graph']==g and x['label']==label)*1000
def run():
 aggregate=R.read(R.OUT/'monotonic_aggregate.json');effects=R.read(R.OUT/'diagnostic_effects.json');pub=measurements('publication.json');algo=measurements('algorithm_controls.json','algorithm_controls');work=R.read(R.OUT/'algorithm_control_work.json');window=R.read(R.OUT/'window_instruction_comparison.json');phases=R.read(R.OUT/'phase_decomposition.json');waiting=measurements('graptor_waiting.json')
 assert len(window)==13 and len(R.read(R.OUT/'publication_exact_bits.json'))==9
 report='''P1 kernel-gap investigation — 2026-10-20

The kernel gap is real, but it combines runtime startup and orchestration,
materialization of ordered edge occurrences, different algorithm work, and
scalar generic execution. It is not one unexplained microarchitectural effect.
The experiments below separate demonstrated causes from unresolved attribution.

Scope and provenance

Accepted P1 is the frozen R4a3 compiler/runtime. This investigation changes no
accepted native source. Runs use physical CPUs 0,2,4,6,8,10, with physical subsets
at 1 and 2 workers; lscpu confirms each selected CPU has a different core ID.
Graptor uses six OpenMP workers and eight graph partitions. Eight partitions
are a layout parameter, not eight workers. Its layout stays fixed at 1/2/6
workers. All data, weights, sources, binaries and settings remain campaign-pinned.
The pinned environments differ: Graptor binds OpenMP workers to physical places
and uses libgomp's default waiting behavior. P1's loading OpenMP team is passive;
its execution team is a separate pthread pool with a fixed 50-us spin budget.

New measurements and private controls are under C:/p1kernel-investigation-20261020
and /home/kazi/p1_kernel_20261020. Append-only process observations are in the
new kernel_gap_c8_20261020 directory under the campaign's runs directory. The
old sealed campaign at C:/p1campaign-results remains unchanged.

1. Reproduction on all 244 cases

One warmup and five fresh processes per implementation per case; algorithm
blocks run sequentially under the shared experiment lock. Both implementations
pass independent full-result validation for every graph/algorithm before timing.
All PageRank figures in the new comparison use double precision and 20 rounds.

Graptor's original gettimeofday timer produced one -1.0219-second observation
in the first diagnostic run, whose monotonic process time was 79.10 ms. A small
preload changes only its timer to CLOCK_MONOTONIC for the following rerun. No
original final-campaign internal record was nonpositive; that does not rule out
positive realtime-clock bias. The campaign's acceptance/end-to-end clock was
already CLOCK_MONOTONIC, so this defect does not invalidate those promotion gates.

Ratios below are P1/Graptor: larger means P1 is slower. These are ratios of
reported compute regions, which have different preprocessing boundaries.

'''
 report+=table(['Algorithm','Cases','Mean-based geometric ratio','Median-based geometric ratio'],[(a,61,f"{v['kernel_p1_over_graptor_mean_geomean']:.2f}x",f"{v['kernel_p1_over_graptor_median_geomean']:.2f}x") for a,v in aggregate.items()])
 report+='''
2. Cold worker setup inflates small P1 compute regions

Graptor's graph construction/reordering exercises its OpenMP team before the
driver kernel timer. P1 BFS can create its persistent team inside its timer.
A same-binary control issues one no-op pool job immediately before P1's first
timer, keeping the actual graph algorithm unchanged:

'''
 rows=[]
 for g in ['misc/G12.edges','socfb/socfb-Simmons81.edges','dimacs/hamming8-2.edges']:
  rows.append((g,f"{value(pub,'bfs',g,'control'):.3f}",f"{value(pub,'bfs',g,'prewarm'):.3f}",f"{value(pub,'bfs',g,'control','seconds'):.3f}",f"{value(pub,'bfs',g,'prewarm','seconds'):.3f}"))
 report+=table(['BFS graph','Control kernel ms','Prewarmed kernel ms','Control process ms','Prewarmed process ms'],rows)
 report+='''
This is attribution, not an optimization gain: moving setup outside a timer
still pays for it in the process. It explains a substantial fixed cost on tiny
graphs. Native 1/2/6-worker tests also reproduce G12 BFS at roughly 0.19/0.62/0.98
ms in the first block: the six-worker configuration adds more runtime work than
this 3,200-occurrence traversal needs. Both frameworks have 34 BFS frontier rounds
and the same frontier degree volume on this case.

G12 BFS continuous sampling assigns about 64% of P1's kernel-window retired
instruction samples to sgpl_pool_wait. This is an instruction fraction, not a
wall-time fraction. Turning spinning off alone worsens G12 BFS from 1.14 to 3.93
ms and road BFS from 14.91 to 57.80 ms in paired controls. A long spin or disabling
warmup calibration does not generally close the gap. The actionable problem is
submission/participation/join overhead at small work sizes; the fixed 50-us
policy should remain while those unnecessary operations are removed.

Graptor's hot-team waiting policy is itself a large contributor. A separate
same-executable control changes only OMP_WAIT_POLICY=PASSIVE and GOMP_SPINCOUNT=0:

'''
 rows=[]
 for a,g in [('bfs','socfb/socfb-Simmons81.edges'),('cc','road/road-usroads-48.edges'),('pagerank','misc/G12.edges'),('pagerank','labeled/Tox21-MMP.edges')]:
  rows.append((a,g,f"{value(waiting,a,g,'default'):.3f}",f"{value(waiting,a,g,'passive'):.3f}"))
 report+=table(['Algorithm','Graph','Default Graptor ms','Passive Graptor ms'],rows)
 report+='''
This is a diagnostic policy change, not the pinned comparison and not an exact
match to P1's 50-us pthread policy. It demonstrates that very short reported
kernels depend on an already-hot, spinning team. Waiting, publication overhead
and algorithm work interact; their factors must not be multiplied independently.
Full distributions and A/A records are retained, including large SSSP outliers.

3. PageRank's publication DAG is an independently measured bottleneck

sgpl_dag_spatial_dispatch represents the source-publication barrier as one extra
DAG node plus a dependency to every owner partition, once per contribution
round. That prevents its independent-owner fast path. A diagnostic control runs
the existing ordered source callbacks and source map on the submitting thread,
then invokes the existing independent-owner dispatch. It changes neither rank
arithmetic nor contribution order:

'''
 rows=[]
 for g in ['misc/G12.edges','labeled/Tox21-MMP.edges','dimacs10/inf-luxembourg_osm.edges']:
  b=value(pub,'pagerank',g,'control');c=value(pub,'pagerank',g,'inline_publication')
  t=next(x for x in effects['tests'] if x['experiment']=='publication' and x['graph']==g and x['variant']=='inline_publication');lo,hi=t['paired_log_bootstrap95']
  rows.append((g,f'{b:.3f}',f'{c:.3f}',f'{(1-c/b)*100:.1f}%',f'{lo:.3f}–{hi:.3f}',f"{t['holm_p']:.5f}"))
 report+=table(['PR graph','Control ms','Publication control ms','Kernel reduction','Paired ratio 95% CI','Holm p'],rows)
 report+='''
The accepted binary, private control, publication control and publication-plus-
prewarm control match bit-for-bit at 1/2/6 workers on all three diagnostic graphs
(nine graph/worker groups). These are private causal controls, not promoted optimizations. A general
production change still needs a proof that moving source callbacks does not
alter observable worker/TLS state, budgets, errors or effects, plus the complete
244-case correctness and promotion gates.

4. Frontier-record preparation consumes a large part of BFS/SSSP/CC

P1 counts active occurrences by owner, fills a record for each occurrence,
maintains epoch-tagged ordered append storage, calls the operation, then compacts append
slots in original order. Lean records reduce bytes but retain this pipeline.
The instrumented all-61-graph phase data places approximately 31% of BFS, 52%
of SSSP and 44% of CC time in record preparation alone. CC compaction adds about
12%. These fractions belong to instrumented runs; profiling changes code layout
and cost. They must not be added to uninstrumented acceptance timings. Weight
index construction is nested inside preparation; compaction is nested in finish.

On DSJC1000-5 SSSP, the instrumented run totals 31.73 ms: preparation 20.95 ms
(including 5.67 ms first-match weight indexing), dispatch/execution 8.44 ms and
finish 2.08 ms. The accepted format materializes 70.92 MB of logical record payload across
4,432,345 edge occurrences. Count and fill walk those occurrences before the
execution walk. The existing scratch lease and append epochs already avoid
routine full append-log clearing; repeated clearing is not assigned as a cause.
Ownership, ordinal and append processing add work beyond the relaxation add.

Native incoming PageRank has no frontier records. Record removal therefore
cannot explain or fix its remaining gap.

5. SSSP does more logical work, for two separate reasons

Graptor's pinned BF uses LEVEL_ASYNC=1, allowing new distances to propagate
within a round. P1's owner execution reads its round snapshot. Graptor also
records a vertex frontier as a set; P1 preserves every appended occurrence.
Switching only Graptor's source-read mode to snapshots gives this exact match:

'''
 rows=[]
 for g in ['socfb/socfb-USFCA72.edges','dimacs/DSJC1000-5.edges','road/road-usroads-48.edges']:
  p=next(x for x in phases if x['algorithm']=='sssp' and x['graph']==g)['work'];s=next(x for x in work if x['algorithm']=='sssp' and x['graph']==g and x['variant']=='sssp_snapshot')
  assert s['frontier_arc_volume']==p['unique_source_pairs_within_each_round'] and s['frontier_vertex_volume']==p['unique_sources_within_each_round'] and s['iterations']-1==p['rounds']
  rows.append((g,f"{p['pair_occurrences']:,}",f"{p['unique_source_pairs_within_each_round']:,}",f"{s['frontier_arc_volume']:,}",p['rounds']))
 report+=table(['SSSP graph','P1 occurrence degree volume','P1 per-round unique degree volume','Graptor snapshot frontier degree volume','Relaxation rounds'],rows)
 report+='''
The reported Graptor iteration count has one extra initialization entry, removed
in this table. These are frontier degree volumes, not hardware SIMD edge-visit
counts: dense scans and weak filters can execute a different number of actual
visits. Nevertheless, the exact volume/round match isolates the source snapshot
and occurrence/set differences. On DSJC, duplicate occurrences alone expand
1,678,552 logical pairs to 4,432,345 (2.64x). Original async Graptor has a still
smaller frontier degree volume, around 1.22 million. Deduplicating P1 to remove
that cost would violate the campaign's occurrence/order requirements.

6. Much of CC's large road gap is an algorithm/label trajectory difference

Pinned Graptor CC enables PRESET_ZERO, PUSH_ZERO and FUSION. It initializes labels
using VEBO-renumbered IDs (INITID=0). P1 starts with original IDs and preserves its
round/occurrence program. The benchmark validates component partitions, so
different component-label numbers are accepted. Those choices materially change
the propagation work; they are not interchangeable language optimizations.

Paired Graptor-only controls use the same input, thread count and monotonic timer:

'''
 rows=[]
 for g in ['road/road-usroads-48.edges','dimacs10/inf-luxembourg_osm.edges']:
  rows.append((g,f"{value(algo,'cc',g,'pinned'):.2f}",f"{value(algo,'cc',g,'no_zero_no_fusion'):.2f}",f"{value(algo,'cc',g,'cc_original_labels'):.2f}"))
 report+=table(['CC graph','Pinned kernel ms','Zero/fusion disabled ms','Also original-ID labels ms'],rows)
 report+='''
Original-ID controls also pass an exact minimum-original-ID label check, not
just the partition validator. The original-label change is isolated on top of
the no-zero/no-fusion control. The disabled-zero/fusion flags form one combined
intervention; this experiment does not assign separate speedup factors to
PRESET_ZERO, PUSH_ZERO and FUSION individually.

The additional synchronous CC control fails Graptor's own neighbour validation
on both road inputs and is excluded from timings and conclusions. It is not a
valid matched-algorithm baseline. The failure and logs remain recorded. P1 still
has a real runtime/preparation cost after accounting for the successful controls;
an exact additive decomposition of the remaining CC gap is not established.

7. The remaining generated loops execute many more instructions

Continuous instructions:u sampling records the entire process, then retains only
samples between each PID's two monotonic timer boundaries. There are no profiler
enable/ack handshakes or counter-control ioctls in the timed interval. Counts are
period-sampling estimates with finite statistical error, not exact totals.

'''
 rows=[]
 for a,g in [('sssp','dimacs/DSJC1000-5.edges'),('sssp','road/road-usroads-48.edges'),('cc','road/road-usroads-48.edges'),('pagerank','labeled/Tox21-MMP.edges'),('pagerank','dimacs10/inf-luxembourg_osm.edges')]:
  z=next(x for x in window if x['algorithm']==a and x['graph']==g)
  rows.append((a,g,f"{z['ratio']:.1f}x",f"±{z['approx_ratio95_relative']*100:.1f}%",f"{z['p1_samples']:,}/{z['graptor_samples']:,}"))
 report+=table(['Algorithm','Graph','P1/Graptor instruction estimate','Approx. sampling 95% relative margin','P1/Graptor retained samples'],rows)
 report+='''
These ratios include both extra work and runtime instructions; they are not
per-equal-edge efficiency ratios. The confidence margin is a Poisson sampling
approximation, not a complete uncertainty model for repeated processes or skid.

On both large PR examples, about 60% of P1's kernel-window instruction samples
belong to sgpl_full_incoming. Disassembly shows a scalar destination load/add/store
for each incoming edge, plus repeated error/gate/group checks and pointer loads.
Source coverage and its gate setter contribute another roughly 17–18%, and finish
group accounting roughly 9–10%. These are instruction shares, not wall-time shares.
Graptor uses compiled SIMD graph traversal and fused maps on its reordered graph.
The new comparison already uses double precision: float versus double does not
explain this remaining gap.

The specialized P1 functions are created after the main O3 pipeline. Re-running
a scalar late pass pipeline on only those functions, with and without additional
diagnostic alias metadata, gives little benefit on the two large PR graphs. All
result vectors remain bit-identical at 1/2/6 workers; all executable sections
outside the two fixed-address kernel sections are byte-identical across controls.
Thus 'run O3 again' is not a measured solution. The loop's representation and
contracts need to permit fewer checks and a different legal lowering.

The pinned Graptor layout rejects -l1/-l2/-l4 with its hard-coded degree-bit
encoding assertion. Those rejected trials are retained; they provide no valid
scalar-versus-SIMD ablation. The separate shares of SIMD, graph layout, accumulator
load/store traffic and individual guards remain unquantified. None is presented
as an exact independent speedup factor.

8. What to implement next, while keeping the requested semantics

First: remove the full-domain publication DAG when a compiler certificate proves
ordered source publication can run before a single owner-team launch without
observable worker/TLS changes. Keep the original callbacks, failure publication,
budget, join and fallback. The diagnostic above has the largest demonstrated
small-graph gain and a consistent 14–15% gain on larger PR examples.

Second: make ready work and measured launch/join cost determine whether an owner
batch needs a parallel submission at all. Keep 50 us as the waiting policy.
Optimize cold initialization only as a whole-process change; prewarming outside
the kernel clock must never be counted as speedup. Preserve all required lifecycle
calls even when a batch executes serially.

Third: generate a certified native owner loop for ordered frontier occurrences,
with original degree-prefix work ordinals and native first-match weights. Avoid
count/fill/copy where the effect and order proof permits it. R3's previously
rejected generic descriptor/gather callback must not be reinstated unchanged.
Specialize weighted access and error checks at a batch boundary only when the
compiler/runtime descriptor proves the same topology, row bounds and failure
semantics. Retain duplicate occurrences and append order. Prove individual dead
relaxations before omitting them; never substitute frontier sets or a CC algorithm.

Fourth: improve the actual full-domain loop representation: expose certified
alias separation, keep an ordered destination accumulator in a register when
observable stores/errors permit, hoist invariant gate/group work under a complete
effect proof, and vectorize independent destinations or maps. Preserve each
destination's contribution order, duplicate edges, double precision, signed-zero/
NaN behavior and existing FP rounding; no reassociation or fast-math.

Every proposed production change still needs the original all-244-case validation,
interleaved whole-process measurements, 20-pair confirmation, A/A checks and
zero-confirmed-regression gate. No private diagnostic variant in this report is
accepted or included in the cumulative optimization branch.

9. Evidence quality, limits and reproducibility

The publication and late-optimization experiments include 20 alternating-order
paired observations and identical-binary A/A controls. Effects use a fixed seed,
100,000 paired sign permutations and Holm correction within the diagnostic family;
A/A tests have their own corrected family. There are no corrected A/A failures.
These diagnostic tests are two-sided and are not substituted for promotion gates.

CPU wait samples must not be summed across threads as wall time. Instrumented
phase budgets must not be added to uninstrumented clocks. Kernel ratios are not
end-to-end ratios: Graptor's graph transformation and property setup occur before
its timer, while P1 pays some index/context setup inside its timer. All construction
costs remain inside whole-process acceptance timings.

Two attempted cross-framework counter methods are rejected and explicitly marked:
external perf control injects enable/ack idle spinning, and inherited event-group
ioctls take milliseconds on this WSL host. Their raw records and the old provisional
pmu_analysis.json remain for provenance, but its cross-framework ratios are invalid.
The authoritative replacement is window_instruction_comparison.json. Low-sample
cases and unknown symbol shares are visible in window_instruction_analysis.json;
for example, brock400-1 CC has only 39 Graptor samples and is not used for a precise
instruction-ratio conclusion.

This investigation identifies concrete dominant costs and causal interventions.
It does not claim a complete additive cycle budget for every one of the 244 cases,
nor that all of Graptor's algorithmic advantages are legal under P1's contract.

Key files in C:/p1kernel-investigation-20261020:
  manifest.json; investigation_manifest.json; evidence_fingerprints.json
  full_mono.json; full_mono_summary.json; monotonic_aggregate.json
  measure_mono.json; controls.json; diagnostic_effects.json; graptor_waiting.json
  phase_decomposition.json; deep.json; publication.json
  publication_exact_bits.json; exact_control_audit.json
  late_opt.json; late_opt_bits.json; late_opt_layout_audit.json
  algorithm_controls.json; algorithm_control_work.json; algorithm_control_builds.json
  window_samples.json; window_instruction_analysis.json
  window_instruction_comparison.json; counter_method_rejections.txt

The scripts are tools/p1_campaign/kernel_*.py on the diagnostic branch. Recommended
WSL interpreter: /home/kazi/p1_local_regressions_20261019/venv/bin/python.
Run kernel_sequence.py for the validated sequence, or specify a stage name to resume.
The original campaign runner's status command reads the new output directory when
P1_CAMPAIGN_OUTPUT=/mnt/c/p1kernel-investigation-20261020 is set. Detailed raw output
stays on disk. All benchmarking, compilation and profiling use one shared lock.
'''
 target=R.OUT/'kernel_gap_report.txt';target.write_text(report,encoding='utf-8')
 copy=R.REPO/'reports/p1_kernel_gap_investigation_20261020.txt';copy.parent.mkdir(exist_ok=True);copy.write_text(report,encoding='utf-8')
 R.save(R.OUT/'kernel_gap_report_identity.json',dict(path=str(target),sha256=R.sha(target),repo_copy=str(copy),aggregate_sha256=R.sha(R.OUT/'monotonic_aggregate.json'),effects_sha256=R.sha(R.OUT/'diagnostic_effects.json'),window_sha256=R.sha(R.OUT/'window_instruction_comparison.json')))
 print(str(target),len(report),'characters',flush=True)
if __name__=='__main__':
 with R.locked('kernel_write_report'):run()
