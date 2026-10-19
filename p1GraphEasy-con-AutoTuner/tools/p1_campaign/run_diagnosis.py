"""Collect causal controls for a rejected attempt, without changing its decision.

--disable is a JSON object of the candidate's feature-disable environment.
Profiling builds finish before the uninstrumented controls begin.
"""
from pathlib import Path
import argparse,json,statistics,sys,time
from types import SimpleNamespace
import runner as R
from profile_support import run_profiles,instrument
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);p.add_argument('--disable',required=True);p.add_argument('--cases');args=p.parse_args()
R.M=R.bootstrap()
selected=R.read(args.cases or R.OUT/(args.version+'_diagnostic_cases.json'))
assert selected,'Run the primary matrix first, or provide an explicit diagnostic case list'
disable=json.loads(args.disable);assert disable
with R.locked('diagnose'):
 for v in [args.version,args.baseline]:R.verify_fingerprint(v);instrument(R,v)
 for workers in [1,2,6]:
  for extra in ['{}',json.dumps(disable)]:
   options=SimpleNamespace(version=args.version,baseline=args.baseline,mode='phases',cases=None,workers=workers,extra=extra)
   selected_file=R.OUT/(args.version+'_diagnosis_selection.json');R.save(selected_file,selected);options.cases=str(selected_file);run_profiles(R,options)
 # PMU counters on all selected cases; expensive traces on one case/algorithm.
 for mode in ['perf','futex','sched']:
  subset=selected if mode=='perf' else [next(x for x in selected if x['algorithm']==a) for a in R.ALGS if any(x['algorithm']==a for x in selected)]
  f=R.OUT/(args.version+'_diagnosis_'+mode+'_selection.json');R.save(f,subset)
  options=SimpleNamespace(version=args.version,baseline=args.baseline,mode=mode,cases=str(f),workers=6,extra='{}');run_profiles(R,options)
 records=[]
 for a in R.ALGS:
  cases=[x for x in selected if x['algorithm']==a]
  if not cases:continue
  start=time.time()
  for x in cases:
   row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   for workers in [1,2,6]:
    configs=[('baseline',args.baseline,{}),('candidate',args.version,{}),('disabled',args.version,disable),('aa',args.baseline,{})]
    for label,v,extra in configs:R.invoke(args.version,'diagnostic_warmup',v,a,row,0,label=label,workers=workers,extra=extra)
    for rep in range(20):
     shift=rep%len(configs)
     for label,v,extra in configs[shift:]+configs[:shift]:records.append(R.invoke(args.version,'diagnostic_control',v,a,row,rep,label=label,workers=workers,extra=extra))
  R.calibrate(a,'diagnostic_controls',[cases,[1,2,6],20,disable],start)
 R.save(R.OUT/(args.version+'_diagnostic_controls.json'),records)
 summary=[]
 for x in selected:
  for workers in [1,2,6]:
   rows=[r for r in records if r['algorithm']==x['algorithm'] and r['graph']==x['graph'] and r['workers']==workers]
   values={label:{'mean_ms':statistics.mean(r['seconds']*1000 for r in rows if r['label']==label and not r.get('error')),'cpu_ms':statistics.mean((r['user_seconds']+r['system_seconds'])*1000 for r in rows if r['label']==label and not r.get('error')),'voluntary_switches':statistics.mean(r['nvcsw'] for r in rows if r['label']==label and not r.get('error'))} for label in ['baseline','candidate','disabled','aa']}
   summary.append(dict(x,workers=workers,values=values,errors=[r for r in rows if r.get('error')]))
 R.save(R.OUT/(args.version+'_diagnostic_summary.json'),summary)
 print(json.dumps({'version':args.version,'diagnostic_cases':len(selected),'summary':str(R.OUT/(args.version+'_diagnostic_summary.json'))}),flush=True)
