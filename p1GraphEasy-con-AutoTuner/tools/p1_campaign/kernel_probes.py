"""Private scoped timers and PMU sampling for accepted P1; never acceptance data."""
import kernel_gap as K
R=K.R
from pathlib import Path
import re,sys,os,subprocess,json,time,statistics
W=Path('/home/kazi/p1_kernel_20261020');W.mkdir(exist_ok=True)
P='/usr/lib/linux-tools/6.8.0-147-generic/perf'
FUNCTIONS={
'autotuner_runtime.c':['autograph_build_clean_cut','autograph_build_clean_cut_inner','autograph_exec_ctx_create','autograph_exec_ctx_destroy','autograph_exec_ctx_bind_fullgraph_v1','autograph_exec_ctx_bind_array_frontier','autograph_frontier_execute','sgpl_exec_spatial_begin','sgpl_exec_spatial_finish','sgpl_exec_source_coverage','sgpl_exec_step_dispatch','sgpl_exec_step_dispatch_body','sgpl_dag_spatial_dispatch','autograph_execute_dag','dag_prepare_run'],
'array_frontier_runtime.inc':['sgpl_array_records','sgpl_array_weight_indices','sgpl_array_compact'],
'array_preparation_runtime.inc':['sgpl_array_records_csr'],
'fullgraph_runtime.inc':['sgpl_incoming_build','sgpl_fullgraph_prepare_map','sgpl_fullgraph_release','autograph_exec_ctx_bind_fullgraph_v1'],
'execution_scratch_runtime.inc':['sgpl_execution_buffer'],
'parallel_runtime.c':['sgpl_run_on_worker_pool','parallel_for_runtime','sgpl_should_parallelize_doall','sgpl_run_tdg_level','sgpl_calibrate_thread_launch_overhead_ns','sgpl_calibrate_total_launch_overhead_ns','sgpl_get_path_launch_overhead_ns','sgpl_get_thread_launch_overhead_ns']}
HEADER=r'''
#include <stdint.h>
#include <time.h>
#include <pthread.h>
extern pthread_t gap_main;
extern int gap_active;
extern unsigned long long gap_ns[96],gap_excl[96],gap_calls[96];
extern _Thread_local int gap_depth;
extern _Thread_local unsigned long long gap_children[128];
static inline unsigned long long gap_now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned long long)t.tv_sec*1000000000ULL+t.tv_nsec;}
typedef struct{unsigned long long start;int id,level;} GapScope;
static inline GapScope gap_enter(int id){GapScope s={0,id,-1};if(pthread_equal(pthread_self(),gap_main)&&gap_active&&gap_depth<127){s.level=gap_depth++;gap_children[s.level]=0;s.start=gap_now();}return s;}
static inline void gap_leave(GapScope *s){if(s->level<0)return;unsigned long long d=gap_now()-s->start;gap_ns[s->id]+=d;gap_excl[s->id]+=d-gap_children[s->level];gap_calls[s->id]++;gap_depth--;if(gap_depth>0)gap_children[gap_depth-1]+=d;}
#define GAP(ID) GapScope gap_scope __attribute__((cleanup(gap_leave)))=gap_enter(ID)
'''
TIMER=r'''
#define _GNU_SOURCE
#include "gap.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
pthread_t gap_main;
int gap_active;
unsigned long long gap_ns[96],gap_excl[96],gap_calls[96];
_Thread_local int gap_depth;
_Thread_local unsigned long long gap_children[128];
static int calls;
static void control(const char *cmd){const char *ctl=getenv("GAP_PERF_CTL"),*ack=getenv("GAP_PERF_ACK");if(!ctl)return;int a=open(ack,O_RDONLY|O_NONBLOCK),c=open(ctl,O_WRONLY|O_NONBLOCK);if(a<0||c<0)_exit(96);if(write(c,cmd,strlen(cmd))!=(ssize_t)strlen(cmd))_exit(97);struct pollfd f={a,POLLIN,0};if(poll(&f,1,5000)!=1)_exit(98);char b[32];if(read(a,b,sizeof b)<4)_exit(99);close(c);close(a);}
double __wrap_timer_runtime(void){
 if(calls++==0){gap_main=pthread_self();control("enable\n");gap_active=1;}
 else {gap_active=0;control("disable\n");}
 return gap_now()*1e-9;
}
__attribute__((destructor)) static void report(void){const char *p=getenv("GAP_TIMER_JSON");if(!p)return;FILE *f=fopen(p,"w");if(!f)return;fprintf(f,"[");for(int i=0;i<96;i++)fprintf(f,"%s{\"id\":%d,\"inclusive_ns\":%llu,\"exclusive_ns\":%llu,\"calls\":%llu}",i?",":"",i,gap_ns[i],gap_excl[i],gap_calls[i]);fprintf(f,"]\n");fclose(f);}
'''
def build():
 src=R.ROOT/'versions'/K.V/'src';out=W/'scoped_src_v2';out.mkdir(exist_ok=True)
 for p in src.iterdir():
  if p.suffix in ['.c','.h','.inc']:(out/p.name).write_text(p.read_text())
 (out/'gap.h').write_text(HEADER);(out/'timer.c').write_text(TIMER)
 names=[];missing=[]
 for name,functions in FUNCTIONS.items():
  p=out/name
  if not p.exists():missing+=functions;continue
  text=p.read_text()
  for fn in functions:
   ident=len(names)
   text,n=re.subn(r'(^[^\n;{}]*\b'+fn+r'\([^;{}]*?\)\s*\{)',lambda m:m[1]+f' GAP({ident});',text,count=1,flags=re.M)
   if n:names.append(fn)
   else:missing.append(fn)
  p.write_text(text)
 R.save(W/'scope_names.json',names);R.save(W/'missing_scope_names.json',missing)
 for name in ['autotuner_runtime','parallel_runtime']:
  R.checked(['gcc','-D_GNU_SOURCE','-O3','-g','-fopenmp','-pthread','-include','gap.h','-c',name+'.c','-o',name+'.o'],out,out/(name+'.build.log'))
 R.checked(['gcc','-O3','-g','-pthread','-c','timer.c','-o','timer.o'],out,out/'timer.build.log')
 print('SCOPES',len(names),'MISSING',missing,flush=True)
 return out
def linked(a,row,deep):
 out=W/'scoped_src_v2';d=R.ROOT/'versions'/K.V
 folder=W/'cases_v2'/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
 exe=folder/('deep' if deep else 'scoped')
 for mode in ['timed','validate']:
  dest=Path(str(exe)+('.validate' if mode=='validate' else ''))
  if dest.exists():continue
  libs=[out/(n+'.o') if deep and n in ['autotuner_runtime','parallel_runtime'] else d/(n+'.o') for n in R.OBJECTS]
  R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(K.V,a,row)/(mode+'.o'),*libs,out/'timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',dest],folder,folder/(dest.name+'.link.log'))
 return exe
def deep():
 build();records=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);exe=linked(a,row,True)
  for w in [1,6]:
   valid=R.invoke(K.ATTEMPT,'validate_deep',K.V,a,row,0,label='deep',workers=w,executable_override=str(exe)+'.validate');assert not valid.get('error'),valid
   for rep in range(10):
    path=W/'timers'/(a+'_'+row['graph'].replace('/','_')+f'_w{w}_{rep}.json');path.parent.mkdir(exist_ok=True)
    z=R.invoke(K.ATTEMPT,'deep',K.V,a,row,rep,label='deep',workers=w,extra={'GAP_TIMER_JSON':str(path)},executable_override=exe)
    assert not z.get('error'),z
    z=dict(z,timers=R.read(path));records.append(z)
  R.save(R.OUT/'deep.json',records);R.progress(stage='deep',algorithm=a,graph=row['graph'],completed=len(records),total=260)
def sample(thin=False):
 assert (W/'scoped_src_v2/timer.o').exists()
 records=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);exe=linked(a,row,False)
  folder=W/('samples_ip' if thin else 'samples')/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for typ in (['record'] if thin else ['stat','record']):
   target=folder/(typ+'.json')
   if target.exists():records.append(R.read(target));continue
   ctl=folder/(typ+'.ctl');ack=folder/(typ+'.ack')
   for p in [ctl,ack]:
    if not p.exists():os.mkfifo(p)
   cmd=[P,typ,'--delay=-1','--control=fifo:'+str(ctl)+','+str(ack)]
   if typ=='record':cmd+=['-e','cycles:u','-c','500003' if thin else '100003',*(['-m','1024'] if thin else ['--call-graph','dwarf,4096']),'-o',str(folder/'perf.data')]
   else:cmd+=['-x,','-e','cycles:u,instructions:u,branches:u,branch-misses:u,cache-references:u,cache-misses:u','-o',str(folder/'stat.csv')]
   # Repeated fresh processes; only the first-to-second language timer window
   # is enabled. Loading, reporting, teardown and repeat controller are disabled.
   repeats=(40 if x['role']!='largest_p1_time' and a!='cc' else 10) if thin else (100 if x['role']!='largest_p1_time' and a!='cc' else 20)
   child=['python3','-c','import subprocess,sys; [subprocess.run([sys.argv[1]],check=True) for _ in range(int(sys.argv[2]))]',str(exe),str(repeats)]
   env=R.M.env(workers=6);env.update(GAP_PERF_CTL=str(ctl),GAP_PERF_ACK=str(ack))
   start=time.time();R.checked(cmd+['--',*child],folder,folder/(typ+'.log'),env=env,timeout=180)
   if typ=='record':
    R.checked([P,'report','--stdio','--no-children','--sort','dso,symbol','--percent-limit','0.5','-i',folder/'perf.data'],folder,folder/'report.txt')
   z=dict(**x,kind=typ,repeats=repeats,seconds=time.time()-start,executable_sha256=R.sha(exe),folder=str(folder));R.save(target,z);records.append(z)
  R.save(R.OUT/('samples_ip.json' if thin else 'samples.json'),records);R.progress(stage='samples_ip' if thin else 'samples',algorithm=a,graph=row['graph'],completed=len(records),total=13 if thin else 26)
if __name__=='__main__':
 with R.locked('kernel_probes_'+sys.argv[1]):
  R.verify_inputs();R.verify_fingerprint(K.V)
  {'deep':deep,'sample':sample,'sample_ip':lambda:sample(True)}[sys.argv[1]]()
