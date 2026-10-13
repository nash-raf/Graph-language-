#!/usr/bin/env python3
"""Paired randomized Phase A timings; startup and all-level totals retained."""
import argparse
import csv
import json
import os
import random
import statistics
import subprocess
from tdg_joint_build import BUILD, OUT, build

def cases():
    for p in (2,4,8):
        for name, args in (
            ('fits-mixed',(1,1,100000,1000,1,0)),
            ('fits-unequal-loops',(0,2,100000,1000,3,0)),
            ('overflow-short',(p+2,1,100000,1000,1,0)),
            ('overflow-unequal-loops',(p+2,2,100000,1000,3,0)),
            ('overflow-one-long',(p+2,1,100000,1000,1,0,12,0,200000)),
            ('overflow-all-long',(p+2,1,100000,150000,1,0)),
            ('many-loops',(2,p,65536,1000,1,0)),
            ('tiny',(p+2,1,1025,16,1,0)),
            ('streaming',(p+2,2,500000,1000,1,0,0)),
            ('ordinary-only',(p+2,0,1000,1000,1,0,12,0,150000)),
            ('sequential-sites',(0,3,65536,1000,3,0,12,1)),
            ('independent-doacross',(p+2,2,65536,1000,1,0,12,0,150000,1)),
        ):
            yield p,name,args

def environment(p, variant, cold):
    env=dict(os.environ, SGPL_NUM_THREADS=str(p), OMP_NUM_THREADS=str(p))
    for key in ('SGPL_FORCE_WIDTHS','SGPL_TDG_DEBUG','SGPL_BUDGET_DEBUG','GRAPH_PARALLEL_DEBUG',
                'SGPL_TDG_PLAN_DUMP','SGPL_FORCE_DOALL_PARALLEL','SGPL_FORCE_DOACROSS_PARALLEL',
                'SGPL_NO_UNREGISTERED_POOL_SHARE','SGPL_TDG_TEST_ALLOC_FAIL','SGPL_TDG_TEST_THREAD_FAIL',
                'SGPL_JOINT_ORDERING_ONLY','SGPL_JOINT_OVERLOAD_ONLY','SGPL_JOINT_PROBE_COLD'):
        env.pop(key,None)
    if variant=='ordering': env['SGPL_JOINT_ORDERING_ONLY']='1'
    if variant=='overload': env['SGPL_JOINT_OVERLOAD_ONLY']='1'
    if cold: env['SGPL_JOINT_PROBE_COLD']='1'
    return env

def summarize(path):
    rows=list(csv.DictReader(path.open()))
    lines=[]
    ratios=[]
    for p,name,args in cases():
        selected=[r for r in rows if int(r['p'])==p and r['case']==name]
        if not selected: continue
        policies=list(dict.fromkeys(r['policy'] for r in selected))
        med={policy:statistics.median(float(r['steady_ns']) for r in selected if r['policy']==policy)
             for policy in policies}
        ratios.append(med['joint']/med['reference'])
        lines.append(f'B={p} {name}: '+', '.join(f'{key}={value/1e6:.3f}ms' for key,value in med.items())+
                     f"; joint change={(med['joint']/med['reference']-1)*100:+.1f}%")
    import math
    lines.append(f'Geometric mean joint/reference={math.exp(statistics.mean(math.log(r) for r in ratios)):.4f}')
    lines.append(f'Cases improved={sum(r<1 for r in ratios)}/{len(ratios)}; worst ratio={max(ratios):.3f}')
    path.with_suffix('.summary.txt').write_text('\n'.join(lines)+'\n')
    print('\n'.join(lines))

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--runs',type=int,default=3)
    parser.add_argument('--pilot',action='store_true')
    parser.add_argument('--cold',action='store_true')
    parser.add_argument('--no-build',action='store_true')
    parser.add_argument('--summarize-only',action='store_true')
    args=parser.parse_args()
    path=OUT/('pilot.csv' if args.pilot else 'cold.csv' if args.cold else 'benchmark.csv')
    if args.summarize_only:
        summarize(path); return
    if not args.no_build: build()
    chosen=list(cases())
    policies=['reference','joint','ordering','overload']
    if os.environ.get('SGPL_JOINT_COMPARE_PREVIOUS') == '1':
        policies.append('previous')
    if args.pilot:
        chosen=[c for c in chosen if c[0]==4 and c[1] in
                ('fits-mixed','overflow-short','overflow-one-long','overflow-all-long','tiny','streaming')]
        policies=['reference','current','joint']; args.runs=1
    if args.cold:
        chosen=[c for c in chosen if c[1] in ('overflow-short','overflow-one-long','tiny')]
        policies=['reference','joint']
        if os.environ.get('SGPL_JOINT_COMPARE_PREVIOUS') == '1':
            policies.append('previous')
    # Shuffle cases, then randomize each paired policy order. Do not overlap jobs.
    rng=random.Random(20261008)
    pairs=[(*case,run) for case in chosen for run in range(args.runs)]
    rng.shuffle(pairs)
    with path.open('w',newline='') as output:
        writer=csv.writer(output)
        writer.writerow(['p','case','run','policy','steady_ns','first_ns','all14_ns',
                         'early_loops','widths','cache_hits','plans','reference_calls',
                         'level_prediction_ns','predicted_samples','calibration_runs',
                         'revalidations','learning_ns','evaluations','args'])
        for number,(p,name,arguments,run) in enumerate(pairs):
            order=list(policies); rng.shuffle(order)
            for policy in order:
                exe=policy if policy in ('reference','current','previous') else 'joint'
                result=subprocess.run([str(BUILD/f'probe_{exe}'),*map(str,arguments)],
                    env=environment(p,policy,args.cold),capture_output=True,text=True,check=True,timeout=60)
                row=result.stdout.strip().split(',')
                loops=int(row[2]); tail=12+3*loops
                writer.writerow([p,name,run,policy,row[7],row[tail],row[tail+1],row[11],
                                 ';'.join(row[12:tail:3]),*row[tail+2:tail+11],json.dumps(arguments)])
                output.flush()
            print(f'{number+1}/{len(pairs)} B={p} {name}',flush=True)
    summarize(path)

if __name__=='__main__': main()
