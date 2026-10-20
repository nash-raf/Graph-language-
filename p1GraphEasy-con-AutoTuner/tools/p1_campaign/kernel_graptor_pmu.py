"""REJECTED externally gated counter method, kept for reproducibility.

Locate the two gettimeofday calls in driver main, verify there are exactly two,
and gate perf at those return sites. Graph construction is excluded. ASLR base
is resolved with dl_iterate_phdr. Each process audits one start and one stop.
The enable/ack wait injects OpenMP spinning before the language timer starts;
cross-framework ratios are invalid. Use kernel_window_samples.py instead.
"""
import kernel_gap as K
R=K.R
from pathlib import Path
import os,re,sys,subprocess,time
W=Path('/home/kazi/p1_kernel_20261020/graptor_pmu');W.mkdir(parents=True,exist_ok=True)
P='/usr/lib/linux-tools/6.8.0-147-generic/perf'
SRC=r'''
#define _GNU_SOURCE
#include <sys/time.h>
#include <time.h>
#include <dlfcn.h>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
static int starts,stops;
static uintptr_t first,last;
static int image(struct dl_phdr_info *i,size_t n,void *p){if(!i->dlpi_name[0]){first=i->dlpi_addr+strtoull(getenv("GAP_GATE_START"),0,0);last=i->dlpi_addr+strtoull(getenv("GAP_GATE_STOP"),0,0);return 1;}return 0;}
__attribute__((constructor)) static void init(void){dl_iterate_phdr(image,0);}
static void control(const char *cmd){int a=open(getenv("GAP_PERF_ACK"),O_RDONLY|O_NONBLOCK),c=open(getenv("GAP_PERF_CTL"),O_WRONLY|O_NONBLOCK);if(a<0||c<0)_exit(96);if(write(c,cmd,strlen(cmd))!=(ssize_t)strlen(cmd))_exit(97);struct pollfd f={a,POLLIN,0};if(poll(&f,1,5000)!=1)_exit(98);char b[32];if(read(a,b,sizeof b)<4)_exit(99);close(c);close(a);}
int gettimeofday(struct timeval *t,void *z){uintptr_t ra=(uintptr_t)__builtin_return_address(0);
 if(ra==first){starts++;control("enable\n");}
 struct timespec s;int r=clock_gettime(CLOCK_MONOTONIC,&s);if(t){t->tv_sec=s.tv_sec;t->tv_usec=s.tv_nsec/1000;}
 if(ra==last){stops++;control("disable\n");}return r;}
__attribute__((destructor)) static void audit(void){char b[256];int n=snprintf(b,sizeof b,"{\"pid\":%ld,\"starts\":%d,\"stops\":%d}\n",(long)getpid(),starts,stops);int f=open(getenv("GAP_GATE_AUDIT"),O_WRONLY|O_APPEND|O_CREAT,0600);if(f>=0){write(f,b,n);close(f);}if(starts!=1||stops!=1)_exit(95);}
'''
def run():
 src=W/'scoped.c';lib=W/'scoped.so';src.write_text(SRC)
 if not lib.exists():R.checked(['gcc','-O2','-shared','-fPIC',src,'-ldl','-o',lib],W,W/'build.log')
 results=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);v='graptor_double' if a=='pagerank' else 'graptor'
  folder=W/'cases'/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True);done=folder/'result.json'
  if done.exists():results.append(R.read(done));continue
  cmd,e,_=R.binary_cmd(v,a,row,False,6,folder/'unused.values');exe=cmd[0]
  dis=subprocess.check_output(['objdump','-d','--disassemble=main',exe],text=True)
  calls=[int(re.match(r'\s*([0-9a-f]+):',s)[1],16)+5 for s in dis.splitlines() if re.search(r'call\s+.*<gettimeofday@plt>',s)];assert len(calls)==2,(exe,calls)
  (folder/'main.disassembly.txt').write_text(dis)
  for typ in ['stat','record']:
   ctl=folder/(typ+'.ctl');ack=folder/(typ+'.ack');audit=folder/(typ+'.audit.jsonl')
   for p in [ctl,ack]:
    if not p.exists():os.mkfifo(p)
   env=dict(e,LD_PRELOAD=str(lib),GAP_PERF_CTL=str(ctl),GAP_PERF_ACK=str(ack),GAP_GATE_EXE=exe,GAP_GATE_START=str(calls[0]),GAP_GATE_STOP=str(calls[1]),GAP_GATE_AUDIT=str(audit))
   args=[P,typ,'--delay=-1','--control=fifo:'+str(ctl)+','+str(ack)]
   if typ=='stat':args+=['-x,','-e','cycles:u,instructions:u,branches:u,branch-misses:u,cache-references:u,cache-misses:u','-o',str(folder/'stat.csv')]
   else:args+=['-e','cycles:u','-c','500003','-m','1024','-o',str(folder/'perf.data')]
   repeats=30;child=['python3','-c','import subprocess,sys,json; [subprocess.run(json.loads(sys.argv[1]),check=True) for _ in range(int(sys.argv[2]))]',R.json.dumps(cmd),str(repeats)]
   # Preload belongs only to the measured child, not perf or Python controller.
   code='import subprocess,sys,json,os; e=os.environ.copy();e.update(json.loads(sys.argv[3]));[subprocess.run(json.loads(sys.argv[1]),env=e,check=True) for _ in range(int(sys.argv[2]))]'
   scoped={k:val for k,val in env.items() if k.startswith('GAP_') or k=='LD_PRELOAD'}
   child=['python3','-c',code,R.json.dumps(cmd),str(repeats),R.json.dumps(scoped)]
   R.checked(args+['--',*child],folder,folder/(typ+'.log'),e,timeout=180)
   gates=[R.json.loads(s) for s in audit.read_text().splitlines()];assert len(gates)==repeats and all(s['starts']==s['stops']==1 for s in gates),gates
   if typ=='record':R.checked([P,'report','--stdio','--no-children','--call-graph','none','--sort','dso,symbol','--percent-limit','0.5','-i',folder/'perf.data'],folder,folder/'report.txt')
  z=dict(**x,repeats=repeats,folder=str(folder),executable_sha256=R.sha(exe),preload_sha256=R.sha(lib),timer_return_offsets=calls,gate_audit_passed=True);R.save(done,z);results.append(z);R.save(R.OUT/'graptor_pmu.json',results);R.progress(stage='graptor_pmu',algorithm=a,graph=row['graph'],completed=len(results),total=13)
if __name__=='__main__':
 with R.locked('kernel_graptor_pmu'):
  R.verify_inputs();run()
