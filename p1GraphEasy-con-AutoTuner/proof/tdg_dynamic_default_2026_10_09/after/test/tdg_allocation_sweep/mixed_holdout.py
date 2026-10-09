#!/usr/bin/env python3
"""Fresh-process check of fixed exhaustive winners, without rerunning selection."""
import argparse
from collections import defaultdict
import csv
import json
import os
from pathlib import Path
import random
import resource
import statistics as st
import mixed
import run as sweep

def holdout(out,repeats,samples):
    meta=json.loads((out/'mixed_manifest.json').read_text())
    with (out/'summary.csv').open() as f: summaries=list(csv.DictReader(f))
    folder=out/'holdout'; folder.mkdir(exist_ok=True)
    rows=[]; groups=defaultdict(list)
    for case in meta['cases']:
        for budget in case['budgets']:
            winner=next(s['best_config'] for s in summaries if (s['case'],int(s['budget']),s['policy'])==(case['name'],budget,'dynamic'))
            chosen=tuple(map(int,winner.split(':')))
            for rep in range(repeats):
                policies=['ordinary-first','dynamic']; random.Random(f'holdout-{case["name"]}-{budget}-{rep}').shuffle(policies)
                for policy in policies:
                    env={k:v for k,v in os.environ.items() if not k.startswith(
                        ('SGPL_FORCE','SGPL_JOINT','SGPL_TDG','SGPL_BUDGET','GRAPH_PARALLEL'))}
                    env.update(SGPL_NUM_THREADS=str(budget),OMP_NUM_THREADS=str(budget),SWEEP_SAMPLES=str(samples))
                    result=sweep.run([Path(case.get('build_directory',meta['build_directory']))/case['name'],int(policy=='dynamic'),-2,1,1,0,1,
                                      *case['n'],case['depth'][0]*100+case['depth'][1]],
                                     env=env,input=mixed.header(case)+' '.join(map(str,chosen))+'\n',
                                     capture_output=True,text=True,timeout=60)
                    (folder/f'{case["name"]}.B{budget}.{policy}.r{rep}.txt').write_text(result.stderr)
                    parsed=list(csv.reader(result.stdout.splitlines())); assert len(parsed)==2*samples
                    for row in parsed:
                        phase,it,rl,rr,al,ar,backend,order,h,ns,pred=map(int,row)
                        if phase!=1: assert (backend,al,ar,order,h)==chosen
                        item=dict(case=case['name'],budget=budget,policy=policy,repeat=rep,
                            kind='model' if phase==1 else 'fixed_winner',config='' if phase==1 else winner,
                            backend=backend,order=order,parents=h,requested_left=rl,requested_right=rr,
                            actual_left=al,actual_right=ar,time_ns=ns,prediction_ns=pred)
                        rows.append(item); groups[case['name'],budget,policy].append(item)
            print('Holdout',case['name'],'B'+str(budget),flush=True)
    sweep.write_csv(out/'mixed_holdout_raw.csv',rows)
    results=[]
    for (case,budget,policy),block in sorted(groups.items()):
        native=[st.median(r['time_ns'] for r in block if r['kind']=='model' and r['repeat']==rep) for rep in range(repeats)]
        fixed=[st.median(r['time_ns'] for r in block if r['kind']=='fixed_winner' and r['repeat']==rep) for rep in range(repeats)]
        results.append(dict(case=case,budget=budget,policy=policy,model_ms=st.median(native)/1e6,
            fixed_winner_ms=st.median(fixed)/1e6,model_over_fixed_winner=st.median(native)/st.median(fixed),
            fixed_winner_config=next(r['config'] for r in block if r['kind']=='fixed_winner'),
            native_actual_pairs=';'.join(sorted({f'{r["actual_left"]},{r["actual_right"]}' for r in block if r['kind']=='model'}))))
    sweep.write_csv(out/'mixed_holdout_summary.csv',results)
    comparisons=[]
    for case,budget in sorted({(r['case'],r['budget']) for r in results}):
        a=next(r for r in results if (r['case'],r['budget'],r['policy'])==(case,budget,'ordinary-first'))
        b=next(r for r in results if (r['case'],r['budget'],r['policy'])==(case,budget,'dynamic'))
        comparisons.append(dict(case=case,budget=budget,ordinary_first_ms=a['model_ms'],dynamic_ms=b['model_ms'],
                                dynamic_over_ordinary=b['model_ms']/a['model_ms']))
    sweep.write_csv(out/'mixed_holdout_comparison.csv',comparisons)
    (out/'mixed_holdout_manifest.json').write_text(json.dumps(dict(repeats=repeats,samples=samples,
        settings=len(comparisons),physical_timed_samples=len(rows),method='Freeze exhaustive winner, measure it and native policies on fresh independent processes. No reselection; not another exhaustive sweep.',
        dynamic_wins=sum(r['dynamic_ms']<r['ordinary_first_ms'] for r in comparisons),
        ordinary_first_wins=sum(r['ordinary_first_ms']<r['dynamic_ms'] for r in comparisons),
        total_time_dynamic_over_ordinary=sum(r['dynamic_ms'] for r in comparisons)/sum(r['ordinary_first_ms'] for r in comparisons)),indent=2)+'\n')
    for name,digest in meta['protected_sha256'].items(): sweep.verify_recorded_source(name,digest)
    (out/'mixed_holdout_validation.txt').write_text('PASS: expected fresh-process samples, serial task/loop outputs, grants, budget, ordinary-first phase checks and protected sources.\n')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__); p.add_argument('out',type=Path)
    p.add_argument('--repeats',type=int,default=3); p.add_argument('--samples',type=int,default=15)
    args=p.parse_args(); assert args.repeats>=3 and args.samples>=3
    soft,hard=resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK,(64*1024*1024 if hard<0 else min(64*1024*1024,hard),hard))
    holdout(args.out.resolve(),args.repeats,args.samples)
