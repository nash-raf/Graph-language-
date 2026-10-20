"""Same-binary controls: cold pool and full-domain publication DAG costs.

Publication control executes the existing source callbacks and map in native
order before the existing independent-owner dispatch. Diagnostic only.
"""
import kernel_gap as K
R=K.R
from pathlib import Path
import re,sys,statistics,time
W=Path('/home/kazi/p1_kernel_20261020/publication');W.mkdir(parents=True,exist_ok=True)
TIMER=r'''
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
extern int32_t sgpl_run_on_worker_pool(void (*)(void *,int32_t),void *,int32_t);
static void noop(void *p,int32_t w){}
static int calls;
double __wrap_timer_runtime(void){if(calls++==0&&getenv("GAP_PREWARM_POOL")){int n=atoi(getenv("SGPL_NUM_THREADS"));if(sgpl_run_on_worker_pool(noop,0,n?n:1))abort();}struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
'''
def build():
 src=R.ROOT/'versions'/K.V/'src';out=W/'src';out.mkdir(exist_ok=True)
 for p in src.iterdir():
  if p.suffix in ['.c','.h','.inc']:(out/p.name).write_text(p.read_text())
 p=out/'autotuner_runtime.c';text=p.read_text()
 pattern=r'(static int32_t sgpl_dag_spatial_dispatch\([^;]*?\)\s*\{)'
 text,n=re.subn(pattern,lambda m:m[1]+'''\n
  if(ctx && ctx->array_compact_records==6 && !relation_count &&
      ctx->spatial_coverage_pending && getenv("GAP_INLINE_PUBLICATION")) {
    sgpl_exec_source_coverage(ctx,1);
    if(!sgpl_exec_failed(ctx))sgpl_fullgraph_prepare_map(ctx);
    ctx->spatial_coverage_pending=0;
  }
''',text,count=1);assert n==1
 p.write_text(text);(out/'timer.c').write_text(TIMER)
 R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-c','autotuner_runtime.c','-o','autotuner_runtime.o'],out,out/'build.log')
 R.checked(['gcc','-O3','-g','-c','timer.c','-o','timer.o'],out,out/'timer.build.log')
 R.save(R.OUT/'publication_control_protocol.json',dict(runtime_sha256=R.sha(out/'autotuner_runtime.o'),timer_sha256=R.sha(out/'timer.o'),diagnostic_only=True,change='same source publication order and barriers, publication on submitting thread before independent-owner pool dispatch; optional one no-op pool job before first timer'))
 return out
def run():
 out=build();d=R.ROOT/'versions'/K.V;records=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=W/'cases'/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for mode in ['timed','validate']:
   exe=folder/mode
   if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(K.V,a,row)/(mode+'.o'),*[out/(n+'.o') if n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS],out/'timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(mode+'.link.log'))
  specs=[('control',{}),('prewarm',{'GAP_PREWARM_POOL':'1'})]
  if a=='pagerank':specs += [('inline_publication',{'GAP_INLINE_PUBLICATION':'1'}),('both',{'GAP_INLINE_PUBLICATION':'1','GAP_PREWARM_POOL':'1'})]
  specs.append(('AA',{}))
  for n,e in specs:
   z=R.invoke(K.ATTEMPT,'validate_publication',K.V,a,row,0,label=n,extra=e,executable_override=folder/'validate');assert not z.get('error'),z
  for rep in range(20):
   for n,e in specs if rep%2==0 else list(reversed(specs)):
    z=R.invoke(K.ATTEMPT,'publication',K.V,a,row,rep,label=n,extra=e,executable_override=folder/'timed');assert not z.get('error'),z;records.append(z)
  R.save(R.OUT/'publication.json',records);R.progress(stage='publication',algorithm=a,graph=row['graph'])
def exact_bits():
 out=W/'src';d=R.ROOT/'versions'/K.V;helper=W/'exact_bits.o';results=[]
 if not helper.exists():R.checked(['gcc','-O2','-c',R.HERE/'exact_bits.c','-o',helper],W,W/'exact_bits.build.log')
 for x in R.read(R.OUT/'selected_cases.json'):
  if x['algorithm']!='pagerank':continue
  row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=W/'cases'/'pagerank'/row['graph'].removesuffix('.edges');exe=folder/'exact_validate'
  if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(K.V,'pagerank',row)/'validate.o',*[out/(n+'.o') if n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS],out/'timer.o',helper,'-Wl,--wrap=timer_runtime','-Wl,--wrap=printf','-Wl,--wrap=__printf_chk','-Wl,--wrap=puts','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/'exact.link.log')
  accepted=next(R.case(K.V,'pagerank',row).glob('exact_validate.*'))
  for w in [1,2,6]:
   raw=[]
   for n,e,target in [('accepted',{},accepted),('control',{},exe),('inline_publication',{'GAP_INLINE_PUBLICATION':'1'},exe),('both',{'GAP_INLINE_PUBLICATION':'1','GAP_PREWARM_POOL':'1'},exe)]:
    path=folder/(n+f'.w{w}.bits');z=R.invoke(K.ATTEMPT,'validate_publication_bits',K.V,'pagerank',row,0,label=n,workers=w,extra=dict(e,SGPL_BITS_OUTPUT=str(path)),executable_override=target);assert not z.get('error'),z;raw.append(path.read_bytes())
   assert len(raw[0])==row['vertices']*8 and all(b==raw[0] for b in raw),'Publication control changed a rank bit'
   results.append(dict(graph=row['graph'],workers=w,doubles=row['vertices'],variants=4,passed=True,sha256=R.hashlib.sha256(raw[0]).hexdigest()))
 R.save(R.OUT/'publication_exact_bits.json',results)
if __name__=='__main__':
 with R.locked('kernel_publication'):
  R.verify_inputs();R.verify_fingerprint(K.V);exact_bits() if len(sys.argv)>1 and sys.argv[1]=='bits' else run()
