"""Sample continuously, then select kernel timestamps offline.

No perf enable/disable commands or ioctls occur in the timed interval. Timestamp
markers use CLOCK_MONOTONIC, as does perf (-k mono). Counts are statistical
instruction-period estimates, not exact counter reads. Samples outside each
PID's first/second timer are discarded, including graph construction/validation.
"""
import kernel_gap as K
R=K.R
from pathlib import Path
import os,sys,re,subprocess,time
W=Path('/home/kazi/p1_kernel_20261020/window_samples');W.mkdir(parents=True,exist_ok=True)
P='/usr/lib/linux-tools/6.8.0-147-generic/perf'
C=r'''
#define _GNU_SOURCE
#include <sys/time.h>
#include <time.h>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
static uintptr_t first,last,base;
static uint64_t start,stop;
static int measuring,calls;
static int image(struct dl_phdr_info *i,size_t n,void *p){if(!i->dlpi_name[0]){base=i->dlpi_addr;first=base+strtoull(getenv("GAP_GATE_START"),0,0);last=base+strtoull(getenv("GAP_GATE_STOP"),0,0);return 1;}return 0;}
static uint64_t stamp(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000000000ULL+t.tv_nsec;}
__attribute__((constructor)) static void init(void){const char *expected=getenv("GAP_EXPECT_EXE");char actual[4096];ssize_t n=readlink("/proc/self/exe",actual,sizeof(actual)-1);if(n<0)return;actual[n]=0;if(expected&&strcmp(expected,actual))return;measuring=1;if(getenv("GAP_GATE_START"))dl_iterate_phdr(image,0);}
#ifdef GAP_P1
double __wrap_timer_runtime(void){uint64_t t=stamp();if(!calls++)start=t;else stop=t;return t*1e-9;}
#else
int gettimeofday(struct timeval *t,void *z){uint64_t n=stamp();uintptr_t ra=(uintptr_t)__builtin_return_address(0);if(ra==first)start=n;if(ra==last)stop=n;if(t){t->tv_sec=n/1000000000ULL;t->tv_usec=(n%1000000000ULL)/1000;}return 0;}
#endif
__attribute__((destructor)) static void audit(void){if(!measuring)return;if(!start||stop<=start)_exit(98);char b[320];int n=snprintf(b,sizeof b,"{\"pid\":%ld,\"start_ns\":%llu,\"stop_ns\":%llu,\"image_base\":%llu}\n",(long)getpid(),(unsigned long long)start,(unsigned long long)stop,(unsigned long long)base);int f=open(getenv("GAP_MARKERS"),O_WRONLY|O_APPEND|O_CREAT,0600);if(f<0)_exit(99);if(write(f,b,n)!=n)_exit(97);close(f);}
'''
def run():
 src=W/'marker.c';src.write_text(C);lib=W/'graptor.so';obj=W/'p1.o';d=R.ROOT/'versions'/K.V
 if not lib.exists():R.checked(['gcc','-O2','-shared','-fPIC',src,'-o',lib],W,W/'graptor.build.log')
 if not obj.exists():R.checked(['gcc','-O2','-DGAP_P1','-c',src,'-o',obj],W,W/'p1.build.log')
 results=[];limit=int(os.environ.get('GAP_SAMPLE_LIMIT','999'))
 for x in R.read(R.OUT/'selected_cases.json')[:limit]:
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=W/'cases'/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  exe=folder/'p1'
  if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(K.V,a,row)/'timed.o',*[d/(n+'.o') for n in R.OBJECTS],obj,'-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/'link.log')
  for framework in ['p1','graptor']:
   done=folder/(framework+'.json')
   if done.exists():
    cached=R.read(done);assert R.sha(Path(cached['executable']))==cached['executable_sha256'],'Profile executable changed; use a new method directory'
    results.append(cached);continue
   v=K.V if framework=='p1' else ('graptor_double' if a=='pagerank' else 'graptor');cmd,env,_=R.binary_cmd(v,a,row,False,6,folder/'unused')
   markers=folder/(framework+'.markers.jsonl');extra={'GAP_MARKERS':str(markers)}
   if framework=='p1':cmd[0]=str(exe)
   else:
    dis=subprocess.check_output(['objdump','-d','--disassemble=main',cmd[0]],text=True);sites=[int(re.match(r'\s*([0-9a-f]+):',s)[1],16)+5 for s in dis.splitlines() if re.search(r'call\s+.*<gettimeofday@plt>',s)];assert len(sites)==2
    extra.update(LD_PRELOAD=str(lib),GAP_GATE_START=str(sites[0]),GAP_GATE_STOP=str(sites[1]),GAP_EXPECT_EXE=cmd[0])
   small=x['graph']=='misc/G12.edges' or x['algorithm']=='bfs' and x['role']!='largest_p1_time'
   repeats=150 if small and framework=='graptor' else (50 if small else 15)
   period=10003 if small else 50003
   data=folder/(framework+'.perf.data')
   # Keep interrupted observations rather than append a resumed run's markers
   # to an incomplete process set. A committed done record is the only cache.
   leftovers=[markers,data,folder/(framework+'.samples.txt'),folder/(framework+'.record.log')]
   suffix='.incomplete_'+str(time.time_ns())
   for old in leftovers:
    if old.exists():old.rename(old.with_name(old.name+suffix))
   code='import subprocess,sys,json,os;e=os.environ.copy();e.update(json.loads(sys.argv[3]));[subprocess.run(json.loads(sys.argv[1]),env=e,check=True) for _ in range(int(sys.argv[2]))]'
   child=['python3','-c',code,R.json.dumps(cmd),str(repeats),R.json.dumps(extra)]
   R.checked([P,'record','-k','mono','-e','instructions:u','-c',str(period),'-m','1024','-o',data,'--',*child],folder,folder/(framework+'.record.log'),env,timeout=180)
   samples=folder/(framework+'.samples.txt')
   R.checked([P,'script','--ns','-F','pid,time,period,ip,sym,dso','-i',data],folder,samples,timeout=180)
   gates=[R.json.loads(s) for s in markers.read_text().splitlines()];assert len(gates)==repeats and all(s['stop_ns']>s['start_ns'] for s in gates)
   z=dict(**x,framework=framework,repeats=repeats,period=period,executable=cmd[0],executable_sha256=R.sha(cmd[0]),folder=str(folder),samples=str(samples),markers=str(markers),record_log=str(folder/(framework+'.record.log')));R.save(done,z);results.append(z)
  R.save(R.OUT/'window_samples.json',results);R.progress(stage='window_samples',algorithm=a,graph=row['graph'],completed=len(results),total=26)
if __name__=='__main__':
 with R.locked('kernel_window_samples'):
  R.verify_inputs();R.verify_fingerprint(K.V);run()
