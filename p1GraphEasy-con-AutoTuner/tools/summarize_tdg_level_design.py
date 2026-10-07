#!/usr/bin/env python3
"""Summarize measurements and their limits; no fitting or production edits."""
import csv
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'proof/tdg_level_design'

def main():
    rows = list(csv.DictReader((OUT/'raw.csv').open()))
    current = [r for r in rows if r['policy']=='new']
    def errors(group,column):
        return [abs(float(r[column])/float(r['loop_ns'])-1)*100 for r in group]
    with (OUT/'report.txt').open('w') as report:
        report.write('TDG LEVEL DESIGN AND THREAD-BUDGET AUDIT — 2026-10-07\n\n')
        report.write('The runtime now reserves capacity once per TDG level, fixes the parent-worker count '
                     'to h=min(number of tasks, actual granted budget), and optimizes only loop teams. '
                     'An overloaded mixed level finishes ordinary tasks first, then replans the loop tasks '
                     'within the same reservation. Different TDG levels are not jointly optimized.\n\n')
        report.write('An ordinary callback was never intrinsically divided among several workers: the old '
                     'model changed the number of workers serving the whole level. The error relative to '
                     'the stated design was reducing h using task work/span estimates and exchanging parent '
                     'workers for loop capacity. This also combined full task durations (which include their '
                     'loops) with a separate sum of loop costs. Those durations no longer select h. '
                     'Profiles remain available for diagnostics.\n\n')
        report.write('For loop site i, keep the existing per-loop cost T_i(p_i). Serial execution p_i=1 '
                     'uses its parent and consumes zero extra workers; a parallel team p_i>=2 consumes '
                     'p_i extra workers because the existing dispatcher blocks the parent. The parent '
                     'does not participate in that loop team. The runtime implementation is preserved.\n\n')
        report.write('The capacity constraint is:\n'
                     '  sum over tasks j of max over sites i in j {0 if p_i=1, else p_i} <= B-h.\n'
                     'Sequential sites in one task reuse their team reservation. A plan records serial '
                     'sites explicitly, so a serial choice cannot silently acquire spare capacity. '
                     'Matched and unprofiled sites use the same atomic reservation protocol. '
                     'Forced-width probes cannot enlarge B-h.\n\n')
        report.write('The loop allocation objective is:\n'
                     '  A_j = sum over sites i in task j of T_i(p_i)\n'
                     '  Q   = sum of T_i(p_i) for parallel DOALL sites using the exclusive pool\n'
                     '  F   = max(Q, max_j A_j, sum_j A_j / h).\n'
                     'Parallel DOALL sibling launches serialize on the existing pool mutex; serial '
                     'loops on distinct parents overlap; DOACROSS teams use the ephemeral path and '
                     'are excluded from Q. F is a resource lower bound for profiled loop work, not '
                     'an absolute prediction of full-level wall time. Ordinary work and parent launches '
                     'still contribute to measured full-level time. Their duration does not change '
                     'task worker counts or loop allocation.\n\n')
        report.write('The existing NLOpt solver and measured loop cost tables are retained. Its constraint '
                     'and rounding repair use the per-task maximum reservation and the level objective. '
                     'Compare the repaired allocation with all-serial; enumerate feasible integer widths '
                     'for one/two-site plans when the maximum grid has at most 65536 trials. This bound '
                     'controls optimizer work, and is not a hardware or workload fit coefficient. '
                     'Larger plans retain the heuristic solver, followed by at most two repair passes '
                     'and 65536 trials that reuse each task\'s already reserved peak team for other '
                     'profitable sequential sites. Task grouping is cached so each optimizer evaluation '
                     'does linear work in site count. No per-machine coefficients were fitted.\n\n')
        report.write('Four policies use identical arithmetic and output bodies, with no shared atomics '
                     'inside timed loop bodies:\n'
                     '  old: pre-design-change snapshot, including the earlier loop-equation fixes\n'
                     '  fixed: fixed task workers, original ordering, sum of loop costs\n'
                     '  phased_sum: ordinary-first phases, sum of loop costs\n'
                     '  new: implemented policy with the resource objective.\n'
                     'Each of 24 workload/thread-budget combinations has three processes per policy. '
                     'Each process warms three levels and reports the median of eleven timed levels. '
                     'Jobs are deterministically shuffled to reduce ordering bias. Timing is inside '
                     'the process and includes planning, launches, joins, and both phases.\n\n')
        report.write((OUT/'summary.txt').read_text()+'\n')
        report.write('These results support ordinary-first phasing for short ordinary work plus heavy '
                     'loops when it creates spare capacity. It is not a universal speedup: long ordinary '
                     'work loses overlap, tiny work pays another phase launch, and a loop phase with '
                     'at least B tasks has h=B and no extra team workers. Simply sorting the input '
                     'ordinary-first while retaining one mixed phase does not free the parent slots '
                     'reserved for that phase; the ordinary-first input-order case tests this.\n\n')
        report.write('Fixed workers can also lose to the old policy on levels that already fit: e.g. '
                     'a very short ordinary callback reserves a parent slot for its phase, while the old '
                     'model could use one parent and leave a larger team for the heavy loop. This is '
                     'a tradeoff of the requested fixed-worker design, not evidence of an ordinary '
                     'callback speeding up by receiving several workers.\n\n')
        report.write('Prediction comparison at the SAME implemented allocations, using the loop callback '
                     'span (first loop callback start to last loop callback finish), not total level wall:\n')
        for label,group in (('All process medians',current),
                            ('Loop spans >= 1 ms',[r for r in current if float(r['loop_ns'])>=1e6])):
            a,b = errors(group,'sum_prediction_ns'),errors(group,'resource_prediction_ns')
            report.write(f'  {label}, n={len(group)}: median APE sum={statistics.median(a):.2f}%, '
                         f'resource={statistics.median(b):.2f}%; mean APE sum={statistics.mean(a):.2f}%, '
                         f'resource={statistics.mean(b):.2f}%; resource within 30%={sum(e<=30 for e in b)}/{len(b)}.\n')
        report.write('The sum/resource comparison above isolates composition at identical chosen widths. '
                     'The phased_sum/new policy comparison separately includes changes in chosen widths. '
                     'The individual DOALL/DOACROSS equations were not changed during this design step.\n\n')
        report.write('Legal allocation sweep, fixed two task parents, equal/unequal arithmetic plus '
                     'streaming stores, B=4 and B=8, two processes per choice and eleven measured '
                     'levels per process. Every choice, including the model choice, enables identical '
                     'plan logging. Serial widths consume zero extra workers, and unused capacity is '
                     'allowed. Compare process-median aggregates; negative apparent regret is clipped '
                     'to zero because these are noisy measurements, not exact optima. Width-selection '
                     'regret compares the forced measurement of the selected width with the best forced '
                     'measurement, under identical controls. The default-run gap also includes differences '
                     'in model planning, calibration history, and run-to-run performance; it is reported '
                     'separately and is not silently attributed to the allocation choice.\n')
        report.write((OUT/'sweep_summary.txt').read_text()+'\n')
        if (OUT/'multisite_summary.txt').exists():
            report.write('Three sequential profiled sites in one task, B=4, N=100000, arithmetic '
                         'chains 12/36/12, three process medians each. Compare the same model before '
                         'and after the bounded integer reuse repair:\n')
            report.write((OUT/'multisite_summary.txt').read_text()+'\n')
        report.write('Correctness: TDG/budget integration and structural checks pass with B=1,2,4,8; '
                     'unregistered sharing disabled passes at B=4; six equation reference tests pass. '
                     'Checks cover distinct workers for fitting tasks, a highly skewed task profile, '
                     'ordinary-before-loop ordering in overloaded levels, declared DOALL and independent '
                     'DOACROSS work, sequential loop-site reuse, forced capacity bounds, 1000 concurrent '
                     'planned/unprofiled reservation races per budget, pre-reserved capacity, and '
                     'balanced global reservations. See validation.txt.\n'
                     'The existing append-order test fails on both the captured baseline and current '
                     'runtime with the identical message: order mismatch at 1: got 1, expected 4. '
                     'The append scheduler is unchanged in this work.\n\n')
        report.write('Limits: this is evidence from one host, across several budgets and workload shapes; '
                     'it is not validation on every machine. Shared memory bandwidth, SMT/core contention, '
                     'staggered releases, and sequences involving both the pool and ephemeral teams can '
                     'make F underestimate time. Missing loop metadata cannot be inferred from an '
                     'opaque callback; classification uses the existing declared loop-site list. '
                     'Unprofiled sites retain the bounded fallback until calibration is ready. '
                     'The earlier DOACROSS synchronization/dependency-distance limitations remain; '
                     'these budget tests do not establish full DOACROSS time-model accuracy. '
                     'No redesign of loop execution, pooling, dependencies, or graph layout is included.\n\n')
        report.write('Reproduce in Linux/WSL with GCC/G++, pthread/OpenMP support, Python 3 and libnlopt:\n'
                     '  python3 tools/tdg_level_design_audit.py --runs 3\n'
                     '  python3 tools/tdg_level_design_sweep.py\n'
                     '  python3 tools/tdg_level_design_validation.py\n'
                     '  python3 tools/tdg_level_design_multisite.py\n'
                     '  python3 tools/summarize_tdg_level_design.py\n'
                     'metadata.json records source hashes, HEAD and hardware. The baseline and intermediate '
                     'snapshots are local ignored build inputs; saved patches reconstruct the baseline '
                     'and phased snapshot from the recorded HEAD, and the pre-repair snapshot from the '
                     'recorded final runtime. All probes omit the graph-layout '
                     'autotuner. Only parallel_runtime.c, its header documentation, the budget-test '
                     'expectations, and diagnostic tools/artifacts changed during this design step.\n')
    print((OUT/'report.txt').read_text().split('Prediction comparison',1)[1].split('Correctness:',1)[0])

if __name__ == '__main__':
    main()
