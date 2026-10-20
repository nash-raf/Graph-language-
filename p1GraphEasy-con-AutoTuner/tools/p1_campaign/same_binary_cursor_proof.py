"""Same executable, equal-length environment values, address-only factor test.

All observations are diagnostic and instrumented, not acceptance measurements.
"""
from pathlib import Path
import argparse,ast,hashlib,html,json,math,os,random,shutil,statistics,subprocess,sys,time

TARGETS=[dict(algorithm=a,graph=g) for a,g in [('cc','dimacs10/inf-luxembourg_osm.edges'),('sssp','dimacs10/wing.edges'),('bfs','dimacs10/wing.edges')]]
TELEMETRY=r'''
static unsigned diag_cursor_offset,diag_record_offset;
static uint64_t diag_count_ns,diag_fill_ns,diag_rounds,diag_parallel_rounds;
static uint64_t diag_cursor_total,diag_cursor_shared,diag_cursor_shared_lines;
static uint64_t diag_record_shared_bytes,diag_record_shared_lines;
static uint64_t diag_pos_hist[4],diag_rec_hist[4],diag_ctx_hist[4];
__attribute__((constructor)) static void diag_offsets(void) {
 const char*p=getenv("SGPL_CURSOR_OFFSET"),*q=getenv("SGPL_RECORD_OFFSET");
 unsigned a=p?(unsigned)strtoul(p,NULL,10):0,b=q?(unsigned)strtoul(q,NULL,10):0;
 if(a>48||(a&15)||b>48||(b&15))abort();diag_cursor_offset=a;diag_record_offset=b;
}
__attribute__((noinline)) static void diag_cursor_rows(int64_t*p,int parts,int chunks,int width,int source,sgpl_exec_ctx*c) {
 ++diag_rounds;diag_ctx_hist[((uintptr_t)c%64)/16]++;
 if(width>1){++diag_parallel_rounds;diag_pos_hist[((uintptr_t)p%64)/16]++;}
 uintptr_t first=(uintptr_t)p&~(uintptr_t)63;
 size_t lines=(((uintptr_t)(p+(size_t)parts*chunks)-first)+63)/64;
 if(lines>128||width>63||source)return;
 uint64_t masks[128]={0},updates[128]={0};
 for(int k=0;k<chunks;++k)for(int v=0;v<parts;++v){
  int64_t n=p[(size_t)k*parts+v];if(n<=0)continue;
  size_t line=((uintptr_t)(p+(size_t)k*parts+v)-first)/64;
  masks[line]|=UINT64_C(1)<<(k%width);updates[line]+=(uint64_t)n;diag_cursor_total+=(uint64_t)n;
 }
 for(size_t l=0;l<lines;++l)if(masks[l]&(masks[l]-1)){++diag_cursor_shared_lines;diag_cursor_shared+=updates[l];}
}
__attribute__((noinline)) static void diag_record_rows(sgpl_exec_ctx*c,const int64_t*positions,int parts,int chunks,int width,size_t stride) {
 if(width>1)diag_rec_hist[((uintptr_t)c->gpu_records%64)/16]++;
 if((size_t)parts*chunks>128||width>63)return;
 uintptr_t addresses[256];uint64_t masks[256]={0},bytes[256]={0};int used=0;
 for(int p=0;p<parts;++p)for(int k=0;k<chunks;++k){
  int64_t lo=positions[(size_t)k*parts+p];
  int64_t hi=k+1<chunks?positions[(size_t)(k+1)*parts+p]:c->gpu_record_offsets[p+1];
  if(hi<=lo)continue;
  uintptr_t a=(uintptr_t)c->gpu_records+(size_t)lo*stride,b=(uintptr_t)c->gpu_records+(size_t)hi*stride;
  uintptr_t start=a&~(uintptr_t)63,end=(b-1)&~(uintptr_t)63;
  for(int endpoint=0;endpoint<2;++endpoint){
   uintptr_t line=endpoint?end:start;if(endpoint&&end==start)continue;
   int i=0;while(i<used&&addresses[i]!=line)++i;if(i==used){if(used>=256)return;addresses[used++]=line;}
   masks[i]|=UINT64_C(1)<<(k%width);
   uintptr_t x=a>line?a:line,y=b<line+64?b:line+64;bytes[i]+=y-x;
  }
 }
 for(int i=0;i<used;++i)if(masks[i]&(masks[i]-1)){++diag_record_shared_lines;diag_record_shared_bytes+=bytes[i];}
}
__attribute__((destructor)) static void diag_sharing_report(void) {
 const char*p=getenv("SGPL_SHARING_JSON");if(!p)return;FILE*f=fopen(p,"w");if(!f)return;
 fprintf(f,"{\"cursor_offset\":%u,\"record_offset\":%u,\"rounds\":%llu,\"parallel_rounds\":%llu,\"count_ms\":%.9f,\"fill_ms\":%.9f,\"cursor_updates\":%llu,\"cursor_updates_on_shared_lines\":%llu,\"shared_cursor_lines_over_rounds\":%llu,\"record_bytes_on_shared_lines\":%llu,\"shared_record_lines_over_rounds\":%llu,\"parallel_cursor_mod64\":[%llu,%llu,%llu,%llu],\"parallel_record_mod64\":[%llu,%llu,%llu,%llu]}\n",diag_cursor_offset,diag_record_offset,(unsigned long long)diag_rounds,(unsigned long long)diag_parallel_rounds,diag_count_ns/1e6,diag_fill_ns/1e6,(unsigned long long)diag_cursor_total,(unsigned long long)diag_cursor_shared,(unsigned long long)diag_cursor_shared_lines,(unsigned long long)diag_record_shared_bytes,(unsigned long long)diag_record_shared_lines,(unsigned long long)diag_pos_hist[0],(unsigned long long)diag_pos_hist[1],(unsigned long long)diag_pos_hist[2],(unsigned long long)diag_pos_hist[3],(unsigned long long)diag_rec_hist[0],(unsigned long long)diag_rec_hist[1],(unsigned long long)diag_rec_hist[2],(unsigned long long)diag_rec_hist[3]);fclose(f);
}
'''

def main():
 bundle=Path(os.environ['P1_DIAGNOSTIC_BUNDLE']);sys.path.insert(0,str(bundle/'candidate/tools/p1_campaign'))
 import portable_campaign as P
 import isolate_cursor_alignment as I
 discover=P.physical_cpus
 def two():
  cpus,host=discover();cpus=cpus[:2];host.update(physical_cpus=cpus,workers=len(cpus));return cpus,host
 P.physical_cpus=two;R,cpus=P.setup();assert len(cpus)==2
 with R.locked('same_binary_sharing_proof'):
  shutil.copy2(bundle/'walltime.c',R.ROOT/'walltime.c');timer=R.ROOT/'timer.c'
  timer.write_text('#include <time.h>\ndouble __wrap_timer_runtime(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}\n')
  R.checked(['gcc','-O2','-c',timer,'-o',R.ROOT/'monotonic_timer.o'],R.ROOT,R.OUT/'timer.log')
  R.checked(['gcc','-O2',R.ROOT/'walltime.c','-o',R.ROOT/'walltime'],R.ROOT,R.OUT/'walltime.log')
  version='Cursor_Proof';d=R.ROOT/'versions'/version;src=d/'src';src.mkdir(parents=True)
  for p in (R.ROOT/'versions/Candidate/src').iterdir():
   if p.is_file() and p.suffix in ['.c','.cpp','.h','.inc','.ll']:shutil.copy2(p,src/p.name)
  f=src/'array_preparation_runtime.inc';text=I.prep_source(f.read_text(),0)
  assert text.count(')+0):NULL;')==1;text=text.replace(')+0):NULL;',')+diag_cursor_offset):NULL;',1)
  needle='  sgpl_run_preparation(sgpl_array_count_chunks,&job,width);';assert text.count(needle)==1
  text=text.replace(needle,'  {uint64_t t=now_monotonic_ns();'+needle.strip()+'diag_count_ns+=now_monotonic_ns()-t;}\n  diag_cursor_rows(positions,parts,chunks,width,job.source,c);',1)
  needle='c->gpu_records=sgpl_execution_buffer(c,SGPL_BUF_RECORDS,(size_t)(records?records:1)*record_bytes,0);';assert text.count(needle)==1
  text=text.replace(needle,'''size_t data_bytes=(size_t)(records?records:1)*record_bytes;
  if(data_bytes>SIZE_MAX-128)goto done;
  void *record_storage=sgpl_execution_buffer(c,SGPL_BUF_RECORDS,data_bytes+(c->execution_scratch?128:0),0);
  c->gpu_records=record_storage&&c->execution_scratch?(int32_t*)((((uintptr_t)record_storage+63)&~(uintptr_t)63)+diag_record_offset):record_storage;''',1)
  start=text.index('  sgpl_run_preparation(c->array_compact_records==4?');end=text.index(';',start)+1
  call=text[start:end]
  text=text[:start]+'  diag_record_rows(c,positions,parts,chunks,width,record_bytes);\n  {uint64_t t=now_monotonic_ns();'+call.strip()+'diag_fill_ns+=now_monotonic_ns()-t;}'+text[end:]
  f.write_text(TELEMETRY+'\n'+text)
  R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',d/'autotuner_runtime.o'],src,d/'build.log')
  I.link(R,version,'Candidate',TARGETS)
  aliases=R.ROOT/'canonical';aliases.mkdir();exes={};hashes={}
  for item in TARGETS:
   row=next(r for r in R.M.ROWS if r['graph']==item['graph'])
   for mode in ['timed','validate']:
    exe=R.case(version,item['algorithm'],row)/mode;h=R.sha(exe);alias=aliases/h
    if not alias.exists():os.link(exe,alias)
    exes[(item['algorithm'],item['graph'],mode)]=alias;hashes[str(exe)]=h
  configs={'00':('00','00'),'16':('16','00'),'R1':('00','16'),'B1':('16','16'),'AA':('00','00')}
  save=R.save
  save(R.OUT/'proof_protocol.json',dict(executables=hashes,configs=configs,cases=TARGETS,physical_cpus=cpus,
   instrumented=True,acceptance_eligible=False,method='Same binary and same code addresses per graph; equal-length environment values and output paths; only cursor/record pointers change. Exact counterline writer masks are collected after the count join. All correctness first.',
   caveat='Shared-line updates count logical writes to lines used by multiple worker lanes, not hardware coherence transactions.'))
  def invoke(phase,item,label,rep,workers):
   row=next(r for r in R.M.ROWS if r['graph']==item['graph']);a,b=configs[label]
   case_id=R.key(item)[:12];phase_id=hashlib.sha256(phase.encode()).hexdigest()[:8]
   path=R.OUT/f'proof_{case_id}_{label}_{phase_id}_{rep:02d}_{workers}.json'
   extra={'SGPL_CURSOR_OFFSET':a,'SGPL_RECORD_OFFSET':b,'SGPL_SHARING_JSON':str(path)}
   validation=phase.startswith('validate')
   x=R.invoke('sharing_proof',phase,version,item['algorithm'],row,rep,label=label,workers=workers,extra=extra,
       executable_override=exes[(item['algorithm'],item['graph'],'validate' if validation else 'timed')])
   assert not x.get('error'),x
   if validation:assert x.get('validation'),x
   x=dict(x,sharing=R.read(path));assert x['sharing'],x
   return x
  for workers in [1,2]:
   for item in TARGETS:
    for label in configs:invoke('validate_proof',item,label,0,workers)
  records=[];rng=random.Random(20261022)
  for workers in [2,1]:
   for block in range(2 if workers==2 else 1):
    for item in TARGETS:
     for label in configs:invoke(f'warmup_proof_{block}',item,label,0,workers)
     for rep in range(20 if workers==2 else 5):
      order=list(configs);rng.shuffle(order)
      for label in order:records.append(dict(invoke(f'proof_{block}',item,label,rep,workers),block=block))
     save(R.OUT/'proof_observations.json',records)
     xs=[x for x in records if x['algorithm']==item['algorithm'] and x['graph']==item['graph'] and x['workers']==workers and x['block']==block]
     means={n:dict(whole_ms=statistics.mean(x['seconds'] for x in xs if x['label']==n)*1000,
       count_ms=statistics.mean(x['sharing']['count_ms'] for x in xs if x['label']==n),fill_ms=statistics.mean(x['sharing']['fill_ms'] for x in xs if x['label']==n),
       shared_updates=statistics.mean(x['sharing']['cursor_updates_on_shared_lines'] for x in xs if x['label']==n)) for n in configs}
     R.progress(stage='same_binary_proof',workers=workers,block=block,**item,means=means)
  report(R,records,configs);assert all(R.sha(p)==h for p,h in hashes.items())
  R.progress(stage='proof_complete',completed=len(records),failures=0)

def report(R,records,configs):
 import numpy as np
 from scipy.stats import permutation_test
 rows=[];tests=[];aa=[]
 for item in TARGETS:
  for workers in [1,2]:
   for block in range(2 if workers==2 else 1):
    xs=[x for x in records if all(x[k]==v for k,v in item.items()) and x['workers']==workers and x['block']==block]
    groups={n:sorted([x for x in xs if x['label']==n],key=lambda x:x['repeat']) for n in configs}
    values={n:{key:statistics.mean(x['sharing'][key] for x in q) for key in q[0]['sharing'] if isinstance(q[0]['sharing'][key],(int,float))} for n,q in groups.items()}
    for n,q in groups.items():values[n]['whole_ms']=statistics.mean(x['seconds'] for x in q)*1000
    rows.append(dict(item,workers=workers,block=block,values=values))
    if workers==1:continue
    for left,right in [('00','16'),('00','R1'),('00','B1'),('00','AA')]:
     delta=np.log([x['seconds'] for x in groups[right]])-np.log([x['seconds'] for x in groups[left]])
     p=1.0 if np.all(delta==0) else float(permutation_test((delta,),np.mean,permutation_type='samples',alternative='two-sided',n_resamples=100000,random_state=np.random.default_rng(2358)).pvalue)
     t=dict(item,workers=workers,block=block,left=left,right=right,p=p,geometric_percent=(math.exp(float(np.mean(delta)))-1)*100)
     (aa if right=='AA' else tests).append(t)
 for family in [tests,aa]:
  running=0
  for i,t in enumerate(sorted(family,key=lambda x:x['p'])):running=max(running,min(1,t['p']*(len(family)-i)));t['holm_p']=running
 R.save(R.OUT/'proof_summary.json',dict(cases=rows,tests=tests,aa_tests=aa,instrumented=True,not_acceptance=True))
 doc=['<!doctype html><meta charset="utf-8"><title>Same-binary cacheline proof</title><style>body{font:14px system-ui;margin:24px}td,th{border:1px solid #bbb;padding:6px;text-align:right}table{border-collapse:collapse}</style><h1>Same-binary address-only experiment</h1><p>Instrumented diagnostic only. 00: aligned cursors and records; 16: cursor offset16; R1: record offset16; B1: both offsets16; AA: identical to00. No executable, code-address, worker or algorithm difference within a graph. Two independent20-pair blocks at2workers;1worker is a5-sample negative control.</p><table><tr><th>Case / workers / block</th>'+''.join('<th>'+n+'</th>' for n in configs)+'</tr>']
 for x in rows:doc.append('<tr><td>'+html.escape(x['algorithm']+' '+x['graph']+' / '+str(x['workers'])+' / '+str(x['block']+1))+'</td>'+''.join(f'<td>{x["values"][n]["whole_ms"]:.4f}</td>' for n in configs)+'</tr>')
 (R.OUT/'proof_report.html').write_text('\n'.join(doc)+ '</table>')

def prepare(args):
 out=Path(args.prepare);out.mkdir(parents=True,exist_ok=False)
 text=Path(args.bootstrap).read_text();tree=ast.parse(text)
 prefix={n.targets[0].id:ast.literal_eval(n.value) for n in tree.body[:4] if isinstance(n,ast.Assign)}
 old=prefix['PAYLOAD'];payload=Path(__file__).read_text()
 header='\n'.join(k+'='+repr(prefix[k]) for k in ['MANIFEST','ARCHIVE_HASH','FILES'])+'\nHELPER='+repr(old)+'\nPAYLOAD='+repr(payload)+'\n'
 body=text[text.index('from pathlib import Path\nimport hashlib'):]
 needle=" script=S/'isolate_cursor_alignment.py';script.write_text(PAYLOAD)"
 assert body.count(needle)==1
 body=body.replace(needle," (S/'isolate_cursor_alignment.py').write_text(HELPER)\n script=S/'same_binary_cursor_proof.py';script.write_text(PAYLOAD)",1)
 body=body.replace("'cursor-evidence.tar.gz'","'proof-evidence.tar.gz'")
 body=body.replace("['cursor_summary.json','cursor_report.html','cursor_address_profiles.json','direct_perf_availability.json']","['proof_summary.json','proof_report.html','proof_protocol.json']")
 code=header+body;compile(code,str(out/'proof.py'),'exec');(out/'proof.py').write_text(code)
 meta=dict(id='kazisahib/p1-cursor-sharing-proof',title='P1 same binary cursor sharing proof',code_file='proof.py',language='python',kernel_type='script',is_private=True,enable_gpu=False,enable_internet=True,competition_sources=[],dataset_sources=['kazisahib/p1-static-dag-cpu-campaign-20261020'],kernel_sources=['kazisahib/p1-static-schedule-regression-diagnosis'])
 (out/'kernel-metadata.json').write_text(json.dumps(meta,indent=2));print(hashlib.sha256(code.encode()).hexdigest())

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare');p.add_argument('--bootstrap');p.add_argument('--run',action='store_true');a=p.parse_args()
 if a.prepare:prepare(a)
 elif a.run:main()
 else:p.error('Specify --prepare or --run')
