"""Validate and measure inactive-algorithm compiler/runtime and data-section controls.

Private controls never enter promotion. Cross-links require identical public
struct declarations and no reference to the new full-graph binder. The wrapped
ABI check accepts only the two audited version numbers and exact struct sizes.
"""
import argparse,re,shutil,subprocess,time
from pathlib import Path
import numpy as np
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);p.add_argument('--cases');args=p.parse_args();R.M=R.bootstrap()
def structure(text,name):
 m=re.search(r'typedef struct '+name+r'\s*\{.*?\}\s*'+name+r';',text,re.S);assert m,name
 return re.sub(r'\s+','',re.sub(r'/\*.*?\*/','',m[0],flags=re.S))
def symbols(path):
 return subprocess.check_output(['nm','-S','-n',str(path)],text=True)
with R.locked('fullgraph_cross_controls'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.baseline,args.version]}
 bd=R.ROOT/'versions'/args.baseline;cd=R.ROOT/'versions'/args.version
 bh=(bd/'src/autotuner_runtime.h').read_text();ch=(cd/'src/autotuner_runtime.h').read_text()
 for name in ['sgpl_exec_ctx','sgpl_runtime_op']:assert structure(bh,name)==structure(ch,name),name
 versions=[int(re.search(r'SGPL_EXECUTION_ABI_VERSION\s*=\s*(\d+)',h)[1]) for h in [bh,ch]]
 selected=[x for x in R.read(args.cases or R.OUT/(args.version+'_diagnostic_cases.json')) if x['algorithm']=='cc'];assert selected
 text=(cd/'src/fullgraph_runtime.inc').read_text()
 old='static pthread_mutex_t sgpl_incoming_lock=PTHREAD_MUTEX_INITIALIZER;\nstatic SgplIncomingIndex *sgpl_incoming_cache;'
 new='static pthread_mutex_t sgpl_incoming_lock __attribute__((section(".sgpl_full_cache")))=PTHREAD_MUTEX_INITIALIZER;\nstatic SgplIncomingIndex *sgpl_incoming_cache __attribute__((section(".sgpl_full_cache")));'
 assert text.count(old)==1;text=text.replace(old,new)
 identity=R.key([fingerprints,text,selected,R.sha(Path(__file__))])[:16];work=cd/('full_cross_controls_'+identity);src=work/'src';src.mkdir(parents=True,exist_ok=True)
 for item in (cd/'src').iterdir():
  if item.suffix in ['.c','.h','.inc'] and not (src/item.name).exists():shutil.copy2(item,src/item.name)
 (src/'fullgraph_runtime.inc').write_text(text);obj=work/'data_section.o'
 if not obj.exists():R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',obj],src,work/'build.log')
 shim=work/'abi_shim.c';shim.write_text('#include "autotuner_runtime.h"\n#include <stdlib.h>\nvoid __wrap_autograph_execution_abi_check_v1(int32_t v,uint64_t c,uint64_t o){if((v!='+str(versions[0])+'&&v!='+str(versions[1])+')||c!=sizeof(sgpl_exec_ctx)||o!=sizeof(sgpl_runtime_op))abort();}\n')
 if not (work/'shim.o').exists():R.checked(['gcc','-O3','-I',src,'-c',shim,'-o',work/'shim.o'],src,work/'shim.log')
 builds={};hashes={}
 definitions=[('base_program_candidate_runtime',args.baseline,args.version,False,True),('candidate_program_base_runtime',args.version,args.baseline,False,True),('candidate_program_data_runtime',args.version,args.version,True,False),('candidate_program_shim_runtime',args.version,args.version,False,True)]
 for x in selected:
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=work/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for label,program,runtime,data,shimmed in definitions:
   for mode in ['timed','validate']:
    po=R.case(program,a,row)/(mode+'.o');undefined=subprocess.check_output(['nm','-u',str(po)],text=True);assert 'bind_fullgraph' not in undefined,undefined
    exe=folder/(label+'.'+mode);d=R.ROOT/'versions'/runtime
    if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',po,*[obj if data and n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS],*([work/'shim.o'] if shimmed else []),R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime',*(['-Wl,--wrap=autograph_execution_abi_check_v1'] if shimmed else []),'-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(label+'.'+mode+'.link.log'))
    builds[a,row['graph'],label,mode]=exe;hashes[str(exe)]=R.sha(exe)
 for name,path in [('baseline',bd/'autotuner_runtime.o'),('candidate',cd/'autotuner_runtime.o'),('data',obj)]: (work/(name+'.symbols.txt')).write_text(symbols(path))
 R.save(R.OUT/(args.version+'_full_cross_controls_'+identity+'_protocol.json'),dict(identity=identity,selected=selected,fingerprints=fingerprints,executables=hashes,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'],versions=versions,change='Data-only executable has no ABI shim and changes only two cache globals. Shim-only executable separately isolates the compatibility wrapper. Cross-links exchange generated program/runtime after auditing structs and absence of binder reference.'))
 records=[];start=time.time()
 for completed,x in enumerate(selected,1):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
  for workers in [1,2,6]:
   configs=[('baseline',args.baseline,{},None),('candidate',args.version,{},None),('disabled',args.version,{'SGPL_NO_FULLGRAPH':'1'},None),('aa',args.baseline,{},None)]+[(label,args.version,{},label) for label,*_ in definitions]
   for label,v,extra,control in configs:
    rec=R.invoke(args.version,'validate_full_cross_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],control,'validate'] if control else None);assert rec.get('validation') and not rec.get('error'),rec
    R.invoke(args.version,'full_cross_warmup_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],control,'timed'] if control else None)
   for rep in range(20):
    shift=rep%len(configs)
    for label,v,extra,control in configs[shift:]+configs[:shift]:records.append(R.invoke(args.version,'full_cross_control_'+identity,v,a,row,rep,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],control,'timed'] if control else None))
  R.progress(stage='full_cross_controls',algorithm=a,completed=completed,total=len(selected),elapsed_seconds=time.time()-start)
 R.calibrate('cc','full_cross_controls',[selected,20,[1,2,6],hashes],start);R.save(R.OUT/(args.version+'_full_cross_controls_'+identity+'_records.json'),records)
 summary=[];rng=np.random.default_rng(2358)
 for x in selected:
  for workers in [1,2,6]:
   rows=[q for q in records if q['graph']==x['graph'] and q['workers']==workers];values={label:np.array([q['seconds'] for q in rows if q['label']==label and not q.get('error')]) for label in {q['label'] for q in rows}}
   entry=dict(x,workers=workers,mean_ms={label:float(v.mean()*1000) for label,v in values.items()},comparisons={})
   for n,de in [('candidate','baseline'),('disabled','candidate'),('base_program_candidate_runtime','baseline'),('candidate_program_base_runtime','baseline'),('candidate_program_data_runtime','candidate'),('candidate_program_shim_runtime','candidate'),('candidate_program_data_runtime','candidate_program_shim_runtime'),('aa','baseline')]:
    b=values[de];c=values[n];assert len(b)==len(c)==20;ix=rng.integers(0,20,(10000,20));entry['comparisons'][n+'_over_'+de]=dict(ratio=float(c.mean()/b.mean()),paired_bootstrap95=list(np.quantile(c[ix].mean(axis=1)/b[ix].mean(axis=1),[.025,.975])))
   summary.append(entry)
 R.save(R.OUT/(args.version+'_full_cross_controls_'+identity+'_summary.json'),summary);print('Full cross controls complete '+identity,flush=True)
