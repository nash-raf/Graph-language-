#!/usr/bin/env python3
"""Exhaustive list schedules for small exported one-loop task instances."""
import heapq
import itertools
import json
from pathlib import Path
import statistics
import subprocess
from tdg_joint_build import BUILD, OUT, run

def simulate(case, order, widths, backfill, parents=0):
    # Whole-callback reservation matches the conservative executor. These
    # fixtures have one immediate loop or one ordinary interval per callback.
    jobs=case['jobs']; budget=case['budget']
    pending=list(order); running=[]; free=budget; now=pool_end=0.0
    while pending or running:
        for j in list(pending):
            if parents and len(running)>=parents: break
            p=widths[j]; demand=1+(p if p>1 else 0)
            if demand>free:
                if not backfill: break
                continue
            free-=demand; pending.remove(j)
            start=now
            uses_pool=jobs[j]['loop'] and jobs[j]['mode']==0 and p>1
            if uses_pool: start=max(start,pool_end)
            end=start+jobs[j]['costs'][p-1]
            if uses_pool: pool_end=end
            heapq.heappush(running,(end,j,demand))
        if not running: return float('inf')
        now,j,demand=heapq.heappop(running); free+=demand
        # At simultaneous completion all released capacity is available to
        # the next admission event. Fixtures include no intermediate stages.
        while running and running[0][0]==now:
            _,j,demand=heapq.heappop(running); free+=demand
    return now

def main():
    obj=BUILD/'oracle.o'; exe=BUILD/'oracle'
    run(['gcc','-O2','-fopenmp','-pthread','-I.','-c','tools/tdg_joint_oracle_probe.c','-o',obj])
    run(['g++','-pthread','-fopenmp',obj,BUILD/'bitmap.o','-lnlopt','-lm','-o',exe])
    cases=[json.loads(row) for row in subprocess.check_output([str(exe)],text=True).splitlines()]
    results=[]
    for case in cases:
        best=float('inf')
        for p,q in itertools.product(range(1,case['budget']),repeat=2):
            widths=(p,q,1,1)
            for order in itertools.permutations(range(4)):
                for backfill in (0,1):
                    best=min(best,simulate(case,order,widths,backfill))
        selected=simulate(case,case['order'],case['widths'],case['backfill'],case['parents'])
        assert abs(selected-case['score'])<=max(1,selected*1e-7),(case,selected)
        assert case['evaluations']<=2048
        assert selected>=best-1
        results.append(dict(case=case['case'],selected_ns=selected,oracle_ns=best,
                            regret=max(0,selected/best-1),evaluations=case['evaluations']))
    (OUT/'oracle.json').write_text(json.dumps(results,indent=2)+'\n')
    regrets=[r['regret'] for r in results]
    text=(f'PASS: independent simulator agrees on all {len(results)} selected schedules; '
          f'evaluation bound respected.\nExhaustive small-instance allocation/order regret: '
          f'median={statistics.median(regrets)*100:.2f}%, max={max(regrets)*100:.2f}%.\n')
    (OUT/'oracle_summary.txt').write_text(text)
    print(text)

if __name__=='__main__': main()
