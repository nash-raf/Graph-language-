"""Causal compiler/runtime and helper-section controls for a record candidate.

These controls never participate in acceptance. Cross-linking requires identical
execution struct declarations; only the version/size check is supplied as a shim.
"""
from pathlib import Path
import argparse,json,re,shutil,statistics,subprocess,time
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);p.add_argument('--cases');p.add_argument('--console-log');args=p.parse_args()
if args.console_log:
 import sys
 sys.stdout=open(args.console_log,'a',buffering=1);sys.stderr=sys.stdout
R.M=R.bootstrap()
def structure(text,name):
 match=re.search(r'typedef struct '+name+r'\s*\{.*?\}\s*'+name+r';',text,re.S);assert match,name
 return re.sub(r'\s+','',re.sub(r'/\*.*?\*/','',match[0],flags=re.S))
def symbol(path,name='sgpl_exec_partition_body'):
 for line in subprocess.check_output(['nm','-S',str(path)],text=True).splitlines():
  fields=line.split()
  if len(fields)==4 and fields[-1]==name:return dict(address=int(fields[0],16),bytes=int(fields[1],16))
 return None
with R.locked('layout_controls'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.baseline,args.version]}
 bd=R.ROOT/'versions'/args.baseline;cd=R.ROOT/'versions'/args.version
 baseline=(bd/'src/autotuner_runtime.h').read_text();candidate=(cd/'src/autotuner_runtime.h').read_text()
 for name in ['sgpl_exec_ctx','sgpl_runtime_op']:assert structure(baseline,name)==structure(candidate,name),'Cross-link ABI changed: '+name
 selected=[x for x in R.read(args.cases or R.OUT/(args.version+'_diagnostic_cases.json')) if x['algorithm']=='pagerank']
 assert selected
 source=(cd/'src/autotuner_runtime.c').read_text()
 names=[('int32_t','sgpl_dag_partition_node_lean'),('int32_t','sgpl_dag_partition_node_lean_weighted'),('void','sgpl_independent_spatial_worker_lean'),('void','sgpl_independent_spatial_worker_lean_weighted')]
 for typ,name in names:
  old='static '+typ+' '+name+'(';assert source.count(old)==1,name
  source=source.replace(old,'static __attribute__((section(".sgpl_lean_dispatch"))) '+typ+' '+name+'(')
 identity=R.key([fingerprints,source,selected,R.sha(Path(__file__))])[:16]
 work=cd/('layout_controls_'+identity);work.mkdir(exist_ok=True);src=work/'src';src.mkdir(exist_ok=True)
 for item in (cd/'src').iterdir():
  if item.suffix in ['.c','.h','.inc'] and not (src/item.name).exists():shutil.copy2(item,src/item.name)
 (src/'autotuner_runtime.c').write_text(source)
 if not (work/'layout.o').exists():R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',work/'layout.o'],src,work/'layout.compile.log')
 shim=work/'abi_shim.c';shim.write_text('#include "autotuner_runtime.h"\n#include <stdlib.h>\nvoid autograph_execution_abi_check_v1(int32_t v,uint64_t c,uint64_t o){if(v!=SGPL_EXECUTION_ABI_VERSION||c!=sizeof(sgpl_exec_ctx)||o!=sizeof(sgpl_runtime_op))abort();}\n')
 if not (work/'abi_shim.o').exists():R.checked(['gcc','-O3','-I',src,'-c',shim,'-o',work/'abi_shim.o'],src,work/'shim.compile.log')
 object_symbols={v:symbol(d/'autotuner_runtime.o') for v,d in [(args.baseline,bd),(args.version,cd)]};object_symbols['layout']=symbol(work/'layout.o')
 builds={};exe_hashes={}
 for item in selected:
  row=next(r for r in R.M.ROWS if r['graph']==item['graph']);a=item['algorithm']
  folder=work/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for label,program,runtime,layout in [('base_program_candidate_runtime',args.baseline,args.version,False),('candidate_program_base_runtime',args.version,args.baseline,False),('base_program_layout_runtime',args.baseline,args.version,True),('candidate_program_layout_runtime',args.version,args.version,True)]:
   for mode in ['timed','validate']:
    exe=folder/(label+'.'+mode)
    d=R.ROOT/'versions'/runtime;libs=[work/'layout.o' if layout and n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS]
    if runtime==args.baseline:libs.append(work/'abi_shim.o')
    if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(program,a,row)/(mode+'.o'),*libs,R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(label+'.'+mode+'.link.log'))
    builds[(a,row['graph'],label,mode)]=exe;exe_hashes[str(exe)]=dict(sha256=R.sha(exe),generic_executor=symbol(exe))
 protocol=dict(fingerprints=fingerprints,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'],identity=identity,selected=selected,objects=object_symbols,executables=exe_hashes,change='Only move four new helpers into a distinct ELF section; no instruction/body edit',crosslink='Identical C struct declarations; old runtime ignores unobservable new capability; ABI-check shim only')
 path=R.OUT/(args.version+'_layout_controls_'+identity+'_protocol.json')
 if path.exists():assert R.read(path)==protocol
 else:R.save(path,protocol)
 records=[]
 for item in selected:
  a=item['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==item['graph'])
  for workers in [1,2,6]:
   configs=[('baseline',args.baseline,{},None),('candidate',args.version,{},None),('disabled',args.version,{'SGPL_NO_LEAN_RECORDS':'1'},None),('aa',args.baseline,{},None)]+[(name,args.version,{},name) for _,name in enumerate(['base_program_candidate_runtime','candidate_program_base_runtime','base_program_layout_runtime','candidate_program_layout_runtime'])]
   for label,v,extra,override in configs:
    exe=builds[(a,row['graph'],override,'validate')] if override else None
    x=R.invoke(args.version,'validate_layout_controls_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=exe);assert x.get('validation') and not x.get('error'),x
    exe=builds[(a,row['graph'],override,'timed')] if override else None
    R.invoke(args.version,'layout_warmup_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=exe)
   start=time.time()
   for rep in range(20):
    shift=rep%len(configs)
    for label,v,extra,override in configs[shift:]+configs[:shift]:
     exe=builds[(a,row['graph'],override,'timed')] if override else None
     records.append(R.invoke(args.version,'layout_control_'+identity,v,a,row,rep,label=label,workers=workers,extra=extra,executable_override=exe))
   R.progress(stage='layout_controls',algorithm=a,graph=row['graph'],workers=workers,completed=20,total=20,elapsed_seconds=time.time()-start)
 for path,metadata in exe_hashes.items():assert R.sha(path)==metadata['sha256']
 R.save(R.OUT/(args.version+'_layout_controls_'+identity+'.json'),records)
 summary=[]
 for item in selected:
  for workers in [1,2,6]:
   rows=[r for r in records if r['graph']==item['graph'] and r['workers']==workers]
   values={label:dict(mean_ms=statistics.mean(x['seconds']*1000 for x in rows if x['label']==label and not x.get('error')),cpu_ms=statistics.mean((x['user_seconds']+x['system_seconds'])*1000 for x in rows if x['label']==label and not x.get('error'))) for label in set(x['label'] for x in rows)}
   summary.append(dict(item,workers=workers,values=values,errors=[x for x in rows if x.get('error')]))
 R.save(R.OUT/(args.version+'_layout_controls_'+identity+'_summary.json'),summary)
 print(json.dumps({'identity':identity,'selected':len(selected),'object_symbols':object_symbols,'summary':str(R.OUT/(args.version+'_layout_controls_'+identity+'_summary.json'))}),flush=True)
