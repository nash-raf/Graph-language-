"""Immutable same-binary component comparisons on explicit diagnostic cases."""
import argparse,json,time
from pathlib import Path
import numpy as np
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);p.add_argument('--cases',required=True);p.add_argument('--controls',required=True,help='JSON label-to-environment map');p.add_argument('--workers',type=int,nargs='+',choices=[1,2,6],default=[1,2,6]);args=p.parse_args();R.M=R.bootstrap()
with R.locked('component_controls'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.baseline,args.version]};selected=R.read(args.cases);controls=json.loads(args.controls)
 assert selected and controls and not set(controls)&{'baseline','candidate','aa'}
 identity=R.key([fingerprints,selected,controls,args.workers,R.sha(Path(__file__))])[:16]
 protocol=dict(identity=identity,fingerprints=fingerprints,selected=selected,controls=controls,workers=args.workers,pairs=20,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'])
 path=R.OUT/(args.version+'_components_'+identity+'_protocol.json')
 if path.exists():assert R.read(path)==protocol
 else:R.save(path,protocol)
 records=[];start=time.time()
 for a in R.ALGS:
  cases=[x for x in selected if x['algorithm']==a]
  if not cases:continue
  block=time.time()
  for completed,x in enumerate(cases,1):
   row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   for workers in args.workers:
    configs=[('baseline',args.baseline,{}),('candidate',args.version,{}),('aa',args.baseline,{})]+[(label,args.version,env) for label,env in controls.items()]
    for label,v,extra in configs:
     rec=R.invoke(args.version,'validate_components_'+identity,v,a,row,0,label=label,workers=workers,extra=extra);assert rec.get('validation') and not rec.get('error'),rec
     R.invoke(args.version,'components_warmup_'+identity,v,a,row,0,label=label,workers=workers,extra=extra)
    for rep in range(20):
     shift=rep%len(configs)
     for label,v,extra in configs[shift:]+configs[:shift]:records.append(R.invoke(args.version,'components_'+identity,v,a,row,rep,label=label,workers=workers,extra=extra))
   R.progress(stage='component_controls',algorithm=a,completed=completed,total=len(cases),elapsed_seconds=time.time()-block)
  R.calibrate(a,'component_controls',[cases,20,args.workers,controls],block)
 R.save(R.OUT/(args.version+'_components_'+identity+'_records.json'),records)
 summary=[];rng=np.random.default_rng(2358)
 for x in selected:
  for workers in args.workers:
   rows=[q for q in records if q['algorithm']==x['algorithm'] and q['graph']==x['graph'] and q['workers']==workers];values={label:np.array([q['seconds'] for q in rows if q['label']==label and not q.get('error')]) for label in {q['label'] for q in rows}}
   entry=dict(x,workers=workers,mean_ms={label:float(v.mean()*1000) for label,v in values.items()},comparisons={})
   for n,de in [('candidate','baseline'),('aa','baseline')]+[('candidate',label) for label in controls]:
    b=values[de];c=values[n];assert len(b)==len(c)==20;ix=rng.integers(0,20,(10000,20));entry['comparisons'][n+'_over_'+de]=dict(ratio=float(c.mean()/b.mean()),paired_bootstrap95=list(np.quantile(c[ix].mean(axis=1)/b[ix].mean(axis=1),[.025,.975])))
   summary.append(entry)
 R.save(R.OUT/(args.version+'_components_'+identity+'_summary.json'),summary);print('Component controls complete '+identity,flush=True)
