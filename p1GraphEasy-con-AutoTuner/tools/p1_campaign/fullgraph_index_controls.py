"""Separate incoming-index construction from full-graph execution; no promotion."""
import argparse,shutil,time
from pathlib import Path
import numpy as np
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);args=p.parse_args();R.M=R.bootstrap()
with R.locked('fullgraph_index_controls'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.version,args.baseline]}
 selected=[x for x in R.read(R.OUT/(args.version+'_diagnostic_cases.json')) if x['algorithm']=='pagerank'];d=R.ROOT/'versions'/args.version
 text=(d/'src/fullgraph_runtime.inc').read_text();needle='s->index=sgpl_incoming_acquire(m);if(!s->index){free(s);return;}'
 assert text.count(needle)==1;text=text.replace(needle,needle+'\n  if(getenv("SGPL_FULL_INDEX_ONLY")){sgpl_incoming_release(s->index);free(s);return;}')
 identity=R.key([fingerprints,text,selected,R.sha(Path(__file__))])[:16];work=d/('full_index_controls_'+identity);src=work/'src';src.mkdir(parents=True,exist_ok=True)
 for item in (d/'src').iterdir():
  if item.suffix in ['.c','.h','.inc'] and not (src/item.name).exists():shutil.copy2(item,src/item.name)
 (src/'fullgraph_runtime.inc').write_text(text);obj=work/'autotuner_runtime.o'
 if not obj.exists():R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',obj],src,work/'build.log')
 builds={};hashes={}
 for x in selected:
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=work/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for mode in ['timed','validate']:
   exe=folder/mode
   if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(args.version,a,row)/(mode+'.o'),*[obj if n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS],R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(mode+'.link.log'))
   builds[a,row['graph'],mode]=exe;hashes[str(exe)]=R.sha(exe)
 R.save(R.OUT/(args.version+'_full_index_controls_'+identity+'_protocol.json'),dict(identity=identity,selected=selected,fingerprints=fingerprints,executables=hashes,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'],change='Acquire/cache the incoming index, immediately release its round reference and retain generic callbacks; omit map allocation and specialized execution.'))
 records=[];start=time.time()
 for completed,x in enumerate(selected,1):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
  for workers in [1,2,6]:
   configs=[('baseline',args.baseline,{},False),('candidate',args.version,{},False),('disabled',args.version,{'SGPL_NO_FULLGRAPH':'1'},False),('index_only',args.version,{'SGPL_FULL_INDEX_ONLY':'1'},True),('index_control_off',args.version,{'SGPL_NO_FULLGRAPH':'1'},True),('aa',args.baseline,{},False)]
   for label,v,extra,control in configs:
    rec=R.invoke(args.version,'validate_full_index_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'validate'] if control else None);assert rec.get('validation') and not rec.get('error'),rec
    R.invoke(args.version,'full_index_warmup_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'timed'] if control else None)
   for rep in range(20):
    shift=rep%len(configs)
    for label,v,extra,control in configs[shift:]+configs[:shift]:records.append(R.invoke(args.version,'full_index_control_'+identity,v,a,row,rep,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'timed'] if control else None))
  R.progress(stage='full_index_controls',algorithm=a,completed=completed,total=len(selected),elapsed_seconds=time.time()-start)
 R.calibrate('pagerank','full_index_controls',[selected,20,[1,2,6],hashes],start);R.save(R.OUT/(args.version+'_full_index_controls_'+identity+'_records.json'),records)
 summary=[];rng=np.random.default_rng(2358)
 for x in selected:
  for workers in [1,2,6]:
   rows=[q for q in records if q['graph']==x['graph'] and q['workers']==workers];values={label:np.array([q['seconds'] for q in rows if q['label']==label and not q.get('error')]) for label in {q['label'] for q in rows}}
   entry=dict(x,workers=workers,mean_ms={label:float(v.mean()*1000) for label,v in values.items()},comparisons={})
   for n,de in [('index_only','index_control_off'),('candidate','index_only'),('candidate','disabled'),('aa','baseline')]:
    b=values[de];c=values[n];assert len(b)==len(c)==20;ix=rng.integers(0,20,(10000,20));entry['comparisons'][n+'_over_'+de]=dict(ratio=float(c.mean()/b.mean()),paired_bootstrap95=list(np.quantile(c[ix].mean(axis=1)/b[ix].mean(axis=1),[.025,.975])))
   summary.append(entry)
 R.save(R.OUT/(args.version+'_full_index_controls_'+identity+'_summary.json'),summary);print('Full index controls complete '+identity,flush=True)
