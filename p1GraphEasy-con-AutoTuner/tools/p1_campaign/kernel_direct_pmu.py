"""REJECTED profiling method, retained to reproduce its measurement bias.

Count only user events from first to second language/driver timer. Each process
opens its group before helper creation and enables/disables inherited events
with ioctl. On this WSL host the group ioctls take milliseconds and inject idle
spinning into measurements. Do not use its ratios. Use kernel_window_samples.py.
"""
import kernel_gap as K
R=K.R
from pathlib import Path
import os,sys,re,subprocess,statistics
W=Path('/home/kazi/p1_kernel_20261020/direct_pmu_v2');W.mkdir(parents=True,exist_ok=True)
C=r'''
#define _GNU_SOURCE
#include <sys/time.h>
#include <time.h>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
static int fds[6],starts,stops,graptor,measuring;
static uintptr_t first,last;
static const char *names[]={"cycles:u","instructions:u","branches:u","branch-misses:u","cache-references:u","cache-misses:u"};
static uint64_t counts[6],enabled[6],running[6];
static int image(struct dl_phdr_info *i,size_t n,void *p){if(!i->dlpi_name[0]){first=i->dlpi_addr+strtoull(getenv("GAP_GATE_START"),0,0);last=i->dlpi_addr+strtoull(getenv("GAP_GATE_STOP"),0,0);return 1;}return 0;}
__attribute__((constructor)) static void init(void){
 const char *expected=getenv("GAP_EXPECT_EXE");char actual[4096];ssize_t n=readlink("/proc/self/exe",actual,sizeof(actual)-1);if(n<0)return;actual[n]=0;if(expected&&strcmp(expected,actual))return;measuring=1;
 graptor=getenv("GAP_GATE_START")!=NULL;if(graptor)dl_iterate_phdr(image,0);
 uint64_t types[]={PERF_COUNT_HW_CPU_CYCLES,PERF_COUNT_HW_INSTRUCTIONS,PERF_COUNT_HW_BRANCH_INSTRUCTIONS,PERF_COUNT_HW_BRANCH_MISSES,PERF_COUNT_HW_CACHE_REFERENCES,PERF_COUNT_HW_CACHE_MISSES};
 for(int i=0;i<6;i++){struct perf_event_attr a={0};a.type=PERF_TYPE_HARDWARE;a.size=sizeof a;a.config=types[i];a.disabled=1;a.inherit=1;a.exclude_kernel=1;a.exclude_hv=1;a.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;
  fds[i]=syscall(SYS_perf_event_open,&a,0,-1,i?fds[0]:-1,PERF_FLAG_FD_CLOEXEC);if(fds[i]<0){perror("direct perf_event_open");_exit(94);}
 }
}
static void begin(void){starts++;if(ioctl(fds[0],PERF_EVENT_IOC_RESET,PERF_IOC_FLAG_GROUP)||ioctl(fds[0],PERF_EVENT_IOC_ENABLE,PERF_IOC_FLAG_GROUP))_exit(95);}
static void end(void){stops++;if(ioctl(fds[0],PERF_EVENT_IOC_DISABLE,PERF_IOC_FLAG_GROUP))_exit(96);for(int i=0;i<6;i++){uint64_t v[3];if(read(fds[i],v,sizeof v)!=sizeof v)_exit(97);counts[i]=v[0];enabled[i]=v[1];running[i]=v[2];}}
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
#ifdef GAP_P1
double __wrap_timer_runtime(void){if(!starts)begin();else end();return now();}
#else
int gettimeofday(struct timeval *t,void *z){uintptr_t ra=(uintptr_t)__builtin_return_address(0);if(ra==first)begin();struct timespec s;int r=clock_gettime(CLOCK_MONOTONIC,&s);if(t){t->tv_sec=s.tv_sec;t->tv_usec=s.tv_nsec/1000;}if(ra==last)end();return r;}
#endif
__attribute__((destructor)) static void audit(void){if(!measuring)return;const char *p=getenv("GAP_DIRECT_JSON");if(!p||starts!=1||stops!=1)_exit(98);FILE *f=fopen(p,"w");if(!f)_exit(99);fprintf(f,"{\"starts\":%d,\"stops\":%d,\"events\":{",starts,stops);for(int i=0;i<6;i++)fprintf(f,"%s\"%s\":{\"count\":%llu,\"enabled_ns\":%llu,\"running_ns\":%llu}",i?",":"",names[i],(unsigned long long)counts[i],(unsigned long long)enabled[i],(unsigned long long)running[i]);fprintf(f,"}}\n");fclose(f);}
'''
def run():
 src=W/'counter.c';src.write_text(C);lib=W/'graptor.so';obj=W/'p1.o';d=R.ROOT/'versions'/K.V
 if not lib.exists():R.checked(['gcc','-O2','-shared','-fPIC',src,'-ldl','-o',lib],W,W/'graptor.build.log')
 if not obj.exists():R.checked(['gcc','-O2','-DGAP_P1','-c',src,'-o',obj],W,W/'p1.build.log')
 results=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=W/'cases'/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  exe=folder/'p1'
  if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(K.V,a,row)/'timed.o',*[d/(n+'.o') for n in R.OBJECTS],obj,'-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/'link.log')
  gv='graptor_double' if a=='pagerank' else 'graptor';cmd,_,_=R.binary_cmd(gv,a,row,False,6,folder/'unused')
  dis=subprocess.check_output(['objdump','-d','--disassemble=main',cmd[0]],text=True);sites=[int(re.match(r'\s*([0-9a-f]+):',s)[1],16)+5 for s in dis.splitlines() if re.search(r'call\s+.*<gettimeofday@plt>',s)];assert len(sites)==2
  for rep in range(20):
   for label in ['p1','graptor'] if rep%2==0 else ['graptor','p1']:
    output=folder/(label+f'.{rep}.json');e={'GAP_DIRECT_JSON':str(output)}
    if label=='graptor':e.update(LD_PRELOAD=str(lib),GAP_GATE_START=str(sites[0]),GAP_GATE_STOP=str(sites[1]),GAP_EXPECT_EXE=cmd[0])
    z=R.invoke(K.ATTEMPT,'direct_pmu_v2',K.V if label=='p1' else gv,a,row,rep,label=label,extra=e,executable_override=exe if label=='p1' else None);assert not z.get('error'),z
    counts=R.read(output);assert counts['starts']==counts['stops']==1
    assert all(t['count']>0 and t['running_ns']>0 and t['running_ns']/t['enabled_ns']>.95 for t in counts['events'].values()),counts
    results.append(dict(z,counters=counts))
  R.save(R.OUT/'direct_pmu.json',results);R.progress(stage='direct_pmu',algorithm=a,graph=row['graph'])
if __name__=='__main__':
 with R.locked('kernel_direct_pmu'):
  R.verify_inputs();R.verify_fingerprint(K.V);run()
