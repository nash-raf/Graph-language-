"""Isolate construction/cache work from descriptor traversal in rejected R3."""
import argparse,shutil,time,statistics,re
from pathlib import Path
import numpy as np
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);p.add_argument('--cases');p.add_argument('--descriptor-wide',action='store_true',help='Restore only the former 24-byte descriptor as a component control');args=p.parse_args();R.M=R.bootstrap()
with R.locked('direct_index_controls'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.version,args.baseline]}
 selected=R.read(args.cases or R.OUT/(args.version+'_diagnostic_cases.json'));d=R.ROOT/'versions'/args.version
 text=(d/'src/array_direct_runtime.inc').read_text()
 component='wide_descriptor' if args.descriptor_wide else 'index_only';stem='descriptor_controls' if args.descriptor_wide else 'index_controls'
 if args.descriptor_wide:
  needle='typedef struct {int32_t occurrence,work,source,group;} SgplDirectDescriptor;';assert text.count(needle)==1
  text=text.replace(needle,'typedef struct {int64_t occurrence,work;int32_t source,group;} SgplDirectDescriptor;')
  text=re.sub(r'_Static_assert\(sizeof\(SgplDirectDescriptor\)==16,[^;]+;','',text)
 else:
  needle='SgplDirectIndex *s=sgpl_direct_index_acquire(c,m);if(!s)return 1;';assert text.count(needle)==1
  text=text.replace(needle,needle+'\n  if(getenv("SGPL_DIRECT_INDEX_ONLY")){sgpl_direct_index_release(s);return 1;}')
 identity=R.key([fingerprints,text,selected,R.sha(Path(__file__))])[:16];work=d/(stem+'_'+identity);src=work/'src';src.mkdir(parents=True,exist_ok=True)
 for item in (d/'src').iterdir():
  if item.suffix in ['.c','.h','.inc'] and not (src/item.name).exists():shutil.copy2(item,src/item.name)
 (src/'array_direct_runtime.inc').write_text(text)
 obj=work/'autotuner_runtime.o'
 if not obj.exists():R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',obj],src,work/'build.log')
 builds={};hashes={}
 for x in selected:
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=work/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for mode in ['timed','validate']:
   exe=folder/mode
   if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(args.version,a,row)/(mode+'.o'),*[obj if n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS],R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(mode+'.link.log'))
   builds[a,row['graph'],mode]=exe;hashes[str(exe)]=R.sha(exe)
 protocol=dict(identity=identity,selected=selected,fingerprints=fingerprints,executables=hashes,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'],change='Restore only the former 24-byte descriptor; retain all preparation and execution bodies.' if args.descriptor_wide else 'After native index acquisition, release round reference and fall back to the unchanged lean materializer. Index cache and lookup run; descriptors and direct execution do not.')
 R.save(R.OUT/(args.version+'_'+stem+'_'+identity+'_protocol.json'),protocol);records=[]
 for a in R.ALGS:
  cases=[x for x in selected if x['algorithm']==a];start=time.time()
  for completed,x in enumerate(cases,1):
   row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   for workers in [1,2,6]:
    configs=[('baseline',args.baseline,{},None),('candidate',args.version,{},None),('disabled',args.version,{'SGPL_NO_DIRECT_TRAVERSAL':'1'},None),(component,args.version,{} if args.descriptor_wide else {'SGPL_DIRECT_INDEX_ONLY':'1'},True),('index_control_off',args.version,{'SGPL_NO_DIRECT_TRAVERSAL':'1'},True),('aa',args.baseline,{},None)]
    for label,v,extra,control in configs:
     exe=builds[a,row['graph'],'validate'] if control else None
     rec=R.invoke(args.version,'validate_'+stem+'_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=exe);assert rec.get('validation') and not rec.get('error'),rec
     R.invoke(args.version,stem+'_warmup_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'timed'] if control else None)
    for rep in range(20):
     shift=rep%len(configs)
     for label,v,extra,control in configs[shift:]+configs[:shift]:records.append(R.invoke(args.version,stem+'_'+identity,v,a,row,rep,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'timed'] if control else None))
   R.progress(stage=stem,algorithm=a,completed=completed,total=len(cases),elapsed_seconds=time.time()-start)
  R.calibrate(a,stem,[cases,20,[1,2,6],list(hashes)],start)
 R.save(R.OUT/(args.version+'_'+stem+'_'+identity+'_records.json'),records);summary=[];rng=np.random.default_rng(2358)
 for x in selected:
  for workers in [1,2,6]:
   rows=[q for q in records if q['algorithm']==x['algorithm'] and q['graph']==x['graph'] and q['workers']==workers];values={label:np.array([q['seconds'] for q in rows if q['label']==label and not q.get('error')]) for label in {q['label'] for q in rows}}
   entry=dict(x,workers=workers,mean_ms={label:float(v.mean()*1000) for label,v in values.items()},comparisons={})
   for n,de in [(component,'index_control_off'),('candidate',component),('candidate','disabled'),('aa','baseline')]:
    b=values[de];c=values[n];assert len(b)==len(c)==20;ix=rng.integers(0,20,(10000,20));entry['comparisons'][n+'_over_'+de]=dict(ratio=float(c.mean()/b.mean()),paired_bootstrap95=list(np.quantile(c[ix].mean(axis=1)/b[ix].mean(axis=1),[.025,.975])))
   summary.append(entry)
 R.save(R.OUT/(args.version+'_'+stem+'_'+identity+'_summary.json'),summary);print(stem+' complete '+identity,flush=True)
