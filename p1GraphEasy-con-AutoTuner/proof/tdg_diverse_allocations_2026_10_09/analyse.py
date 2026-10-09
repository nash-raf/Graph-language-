"""Summarize the saved paired measurements without rerunning workloads."""
from pathlib import Path
import csv
import json
import statistics as st

OUT=Path(__file__).resolve().parent
def read(path):
    with path.open() as f: return list(csv.DictReader(f))

primary=read(OUT/'paired_summary.csv')
confirmation=read(OUT/'confirmation/paired_summary.csv')
mixed=[r for r in primary if r['case'].startswith('mixed_')]
def metrics(rows):
    return dict(settings=len(rows),
        summed_time_reduction_pct=100*(1-sum(float(r['after_native_ms']) for r in rows)/
                                          sum(float(r['before_native_ms']) for r in rows)),
        median_extra_first_policy_call_ms=st.median((float(r['after_cold_native_ns'])-
                                                    float(r['before_cold_native_ns']))/1e6 for r in rows),
        median_extra_learning_ms=st.median((float(r['after_total_learning_ns'])-
                                           float(r['before_total_learning_ns']))/1e6 for r in rows),
        median_after_steady_learning_us_per_call=st.median(float(r['after_steady_learning_ns'])/15/1000
                                                          for r in rows),
        settings_faster_by_more_than_5pct=sum(float(r['after_over_before'])<.95 for r in rows),
        settings_slower_by_more_than_5pct=sum(float(r['after_over_before'])>1.05 for r in rows))

analysis=dict(primary=metrics(primary),mixed=metrics(mixed),
    confirmation=[dict(case=r['case'],budget=int(r['budget']),
        before_ms=float(r['before_native_ms']),after_ms=float(r['after_native_ms']),
        time_change_pct=100*(float(r['after_over_before'])-1),allocation=r['after_allocations'])
        for r in confirmation])
(OUT/'analysis.json').write_text(json.dumps(analysis,indent=2)+'\n')
scope=json.loads((OUT/'scope_validation.json').read_text(encoding='utf-8-sig'))
assert [r['file'] for r in scope if r['changed']]==['tdg_scheduler.inc']
lines=[
    'Early diverse allocation candidates -- 2026-10-09',
    '',
    'Scope: only the optimizer seed construction and enumeration order changed.',
    'For each admission setting, try reference, directly wide, serial, and curve-selected',
    'widths before moving to the next admission setting. Wide uses budget-1, respects',
    'trip/max-thread caps and repeated-site reuse, and wins only under the existing objective.',
    'The original 2048-evaluation maximum and soft 5%-of-serial-work deadline remain.',
    'The executor, cache policy, equations, loop engines, compiler and autotuner are unchanged.',
    'The old seed order could spend its first 16 evaluations on similar narrow plans;',
    'the new regression reproduces this miss with fixed costs, and fails against the old search.',
    '',
    'Method: 53 workload/budget settings, 5 randomized paired fresh processes,',
    '15 steady native timings and 15 frozen-winner replays per variant/process: 15,900 timings.',
    'Confirmation: 5 selected settings, 9 fresh pairs, 25 timings per policy: 4,500 timings.',
    'Same saved SGPL objects, same adapters, two frozen runtime builds.',
    'Each process trains 8 serial levels, then warms 12 native levels before measurement.',
    'Reported steady time is median of process medians and includes policy execution overhead.',
    'The first-policy-call metric is after serial training; it is not whole-program startup.',
    'Learning counters include search and calibration/revalidation, exclude cache-hit lookup.',
    'The fixed winner is the previously exhaustive winner replayed in this run; this is',
    'not a new exhaustive search, so it is not proof of the current absolute optimum.',
    'Mixed cases exercise real compiled SGPL callbacks in a synthetic runtime level.',
    '',
    f"Primary: summed steady times fell {analysis['primary']['summed_time_reduction_pct']:.2f}%.",
    f"Settings >5% faster: {analysis['primary']['settings_faster_by_more_than_5pct']}; "
    f">5% slower: {analysis['primary']['settings_slower_by_more_than_5pct']}.",
    f"Mixed settings: median additional first-policy-call time {analysis['mixed']['median_extra_first_policy_call_ms']:.3f} ms; "
    f"additional learning {analysis['mixed']['median_extra_learning_ms']:.3f} ms.",
    'Existing launch calibration is atomic and can overrun the soft search deadline.',
    'Evaluating wide plans sooner can trigger previously deferred width/parent calibration.',
    '',
    'Independent confirmation (negative change means faster):',
    f"{'Case':24} {'B':>2} {'Before ms':>10} {'After ms':>10} {'Change':>9} {'Widths':>8}",
]
for r in analysis['confirmation']:
    lines.append(f"{r['case']:24} {r['budget']:2} {r['before_ms']:10.6f} {r['after_ms']:10.6f} "
                 f"{r['time_change_pct']:+8.2f}% {r['allocation']:>8}")
lines += [
    '',
    'The large_arrays/B8 regression persisted. Earlier diversity fixes a search miss,',
    'but also exposes wider allocations that the unchanged cost curve overvalues.',
    'The initial mixed_short_ordinary/B6 slowdown did not reproduce (confirmation +0.05%).',
    'No workload-specific correction or forced-width rule was added.',
    '',
    'Validation: structural/resource tests at 1/2/4/8 workers; independent simulator;',
    'equation and dispatch gates; ASan/UBSan; generated sibling-graph fixtures, formal',
    'tests and default/alias numeric compilation. All requested checks passed.',
    'The existing append-order assertion fails identically against the frozen reference.',
    'scope_validation.json verifies all snapshotted production sources except the optimizer',
    'are byte-for-byte unchanged, including both autotuner sources.',
    '',
    'Reproduce from the repository in WSL Ubuntu:',
    'python3 tools/tdg_diverse_allocation_benchmark.py',
    'python3 tools/tdg_diverse_allocation_benchmark.py --measure-only --cases large_arrays mixed_short_ordinary mixed_compute mixed_loop_skew --budgets 6 8 --repeats 9 --samples 25 --tag confirmation',
    'python3 proof/tdg_diverse_allocations_2026_10_09/analyse.py',
    'python3 proof/tdg_diverse_allocations_2026_10_09/negative_control.py',
    'SGPL_JOINT_RESULTS_DIR=proof/tdg_diverse_allocations_2026_10_09 python3 tools/tdg_joint_validate.py',
    'SGPL_JOINT_RESULTS_DIR=proof/tdg_diverse_allocations_2026_10_09 python3 tools/tdg_joint_validate.py --sanitizers',
    'SGPL_JOINT_RESULTS_DIR=proof/tdg_diverse_allocations_2026_10_09 python3 tools/tdg_joint_validate.py --generated',
    '',
    'The benchmark requires the saved SGPL/SDK objects referenced by the previous manifests.',
    'Before/after source hashes and the saved before implementation remain in this folder.',
]
(OUT/'report.txt').write_text('\n'.join(lines)+'\n')
print(json.dumps(analysis,indent=2))
