"""Graptor timer correction, work descriptors and CC algorithm ablation."""
import kernel_gap as K
R=K.R
from pathlib import Path
import subprocess,os,time,sys,re,statistics,shlex
W=Path('/home/kazi/p1_kernel_20261020');W.mkdir(exist_ok=True)
def clock():
 src=W/'monotonic_gettimeofday.c';lib=W/'monotonic_gettimeofday.so'
 text='#include <sys/time.h>\n#include <time.h>\nint gettimeofday(struct timeval *t,void *z){struct timespec s;int r=clock_gettime(CLOCK_MONOTONIC,&s);if(t){t->tv_sec=s.tv_sec;t->tv_usec=s.tv_nsec/1000;}return r;}\n'
 if src.exists():assert src.read_text()==text
 else:src.write_text(text)
 if not lib.exists():R.checked(['gcc','-O2','-shared','-fPIC',src,'-o',lib],W,W/'monotonic_gettimeofday.build.log')
 R.save(R.OUT/'graptor_monotonic_timer.json',dict(source_sha256=R.sha(src),library_sha256=R.sha(lib),method='LD_PRELOAD gettimeofday -> CLOCK_MONOTONIC, same pinned executable, diagnostic measurements only'))
 return lib
def work():
 lib=clock();records=[];original=R.binary_cmd
 def cmd(*args):
  c,e,d=original(*args);c.insert(-1,'-itimes');return c,e,d
 R.binary_cmd=cmd
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);v='graptor_double' if a=='pagerank' else 'graptor'
  variants=[('pinned',None)]
  if a=='cc':variants.append(('no_zero_no_fusion',R.OLD/'graptor_controls/cc'))
  for name,exe in variants:
   z=R.invoke(K.ATTEMPT,'validate_graptor_work_v2',v,a,row,0,label=name,extra={'LD_PRELOAD':str(lib)},executable_override=exe)
   assert not z.get('error'),z
   text=Path(z['base']+'.err').read_text()
   lines=[s for s in text.splitlines() if s.startswith('Iteration ')]
   edges=[];vertices=[]
   for line in lines:
    e=re.search(r'(?:nacte: |E:)(\d+)',line);n=re.search(r'(?:nactv: |V:)(\d+)',line)
    if e:edges.append(int(e[1]))
    if n:vertices.append(int(n[1]))
   # Active-edge counts describe frontier volumes, not actual SIMD visits:
   # pull traversal can scan inactive inputs and apply weak filters.
   records.append(dict(**x,variant=name,iterations=len(lines),frontier_arc_volume=sum(edges),frontier_vertex_volume=sum(vertices),iteration_lines=lines,base=z['base'],validation=z['validation']))
  R.save(R.OUT/'graptor_work.json',records);R.progress(stage='graptor_work',algorithm=a,graph=row['graph'],completed=len(records),total=16)
def cc():
 lib=clock();records=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  if x['algorithm']!='cc':continue
  row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
  for rep in range(20):
   specs=[('pinned',None),('no_zero_no_fusion',R.OLD/'graptor_controls/cc')]
   if rep%2:specs.reverse()
   for n,p in specs:
    z=R.invoke(K.ATTEMPT,'cc_ablation','graptor','cc',row,rep,label=n,extra={'LD_PRELOAD':str(lib)},executable_override=p);assert not z.get('error'),z;records.append(z)
  R.save(R.OUT/'cc_ablation.json',records);R.progress(stage='cc_ablation',algorithm='cc',graph=row['graph'])
if __name__=='__main__':
 with R.locked('kernel_graptor_'+sys.argv[1]):
  R.verify_inputs();{'clock':clock,'work':work,'cc':cc}[sys.argv[1]]()
