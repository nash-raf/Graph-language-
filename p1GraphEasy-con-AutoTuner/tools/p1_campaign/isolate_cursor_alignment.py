"""Causal cursor-alignment controls, using the frozen diagnostic build outputs."""
from pathlib import Path
import argparse, collections, hashlib, html, importlib.util, json, math, os, random, re, shutil, statistics, struct, subprocess, sys, tarfile, time

HERE=Path(__file__).resolve().parent
SELECTED=[dict(algorithm=a,graph=g) for a,g in [
 ('cc','dimacs10/inf-luxembourg_osm.edges'),('cc','dimacs10/wing_nodal.edges'),
 ('cc','socfb/socfb-Williams40.edges'),('sssp','dimacs10/wing.edges'),
 ('sssp','socfb/socfb-Brandeis99.edges'),('bfs','dimacs10/wing.edges')]]

PROBE=r'''
static unsigned long long diag_cursor_hist[4],diag_cursor_rounds,diag_cursor_parallel;
static int diag_cursor_parts,diag_cursor_chunks;
static void diag_cursor_record(const void *p,int parts,int chunks,int width) {
 ++diag_cursor_rounds;if(width>1){++diag_cursor_parallel;++diag_cursor_hist[((uintptr_t)p%64)/16];}
 diag_cursor_parts=parts;diag_cursor_chunks=chunks;
}
__attribute__((destructor)) static void diag_cursor_report(void) {
 const char *path=getenv("SGPL_CURSOR_LAYOUT_JSON");if(!path)return;
 FILE*f=fopen(path,"w");if(!f)return;
 fprintf(f,"{\"rounds\":%llu,\"parallel_rounds\":%llu,\"parts\":%d,\"last_chunks\":%d,\"parallel_address_mod64_counts\":[%llu,%llu,%llu,%llu]}\n",diag_cursor_rounds,diag_cursor_parallel,diag_cursor_parts,diag_cursor_chunks,diag_cursor_hist[0],diag_cursor_hist[1],diag_cursor_hist[2],diag_cursor_hist[3]);fclose(f);
}
'''

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save(p,x):
 p=Path(p);p.parent.mkdir(parents=True,exist_ok=True);q=p.with_suffix('.tmp');q.write_text(json.dumps(x,indent=2));q.replace(p)

def symbols(p):
 out={}
 for line in subprocess.check_output(['nm','-S',str(p)],text=True).splitlines():
  z=line.split()
  if len(z)==4 and z[2].lower()=='t':out[z[3]]=dict(address=int(z[0],16),bytes=int(z[1],16))
 data=Path(p).read_bytes();offset=struct.unpack_from('<Q',data,40)[0];stride,count=struct.unpack_from('<HH',data,58)
 sections=[struct.unpack_from('<IIQQQQIIQQ',data,offset+i*stride) for i in range(count)]
 for name,x in out.items():
  if name not in ['sgpl_array_count_chunks','sgpl_array_fill_chunks_lean','sgpl_array_fill_chunks_lean_weighted','sgpl_array_records_csr']:continue
  section=next(s for s in sections if s[1]!=8 and s[3]<=x['address'] and x['address']+x['bytes']<=s[3]+s[5])
  pos=section[4]+x['address']-section[3];x['instruction_sha256']=hashlib.sha256(data[pos:pos+x['bytes']]).hexdigest()
 return out

def prep_source(text,offset):
 needle='''  int64_t *positions=sgpl_execution_buffer(c,SGPL_BUF_PREP_POSITIONS,
      (size_t)chunks*(parts?parts:1)*8,1);'''
 assert text.count(needle)==1
 replacement='''  size_t cursor_bytes=(size_t)chunks*(parts?parts:1)*8;
  if(cursor_bytes>SIZE_MAX-128)return SGPL_DAG_ERR_NOMEM;
  void *positions_storage=sgpl_execution_buffer(c,SGPL_BUF_PREP_POSITIONS,cursor_bytes+128,1);
  int64_t *positions=positions_storage?(int64_t*)((((uintptr_t)positions_storage+63)&~(uintptr_t)63)+'''+str(offset)+'''):NULL;'''
 text=text.replace(needle,replacement,1)
 old='if(!c->execution_scratch){free(positions);free(work);}'
 assert text.count(old)==1
 return text.replace(old,'if(!c->execution_scratch){free(positions_storage);free(work);}',1)

def link(R,version,base,selected,profile=False):
 d=R.ROOT/'versions'/version;parent=R.ROOT/'versions'/base
 obj=d/('profile.o' if profile else 'autotuner_runtime.o')
 for item in selected:
  row=next(r for r in R.M.ROWS if r['graph']==item['graph']);folder=R.case(version,item['algorithm'],row);folder.mkdir(parents=True,exist_ok=True)
  for mode in ['timed','validate']:
   target=folder/(('layout_profile.' if profile else '')+mode)
   driver=R.case(base,item['algorithm'],row)/(mode+'.o')
   libs=[obj if n=='autotuner_runtime' else parent/(n+'.o') for n in R.OBJECTS]
   R.checked(['g++','-O3','-fopenmp','-no-pie',driver,*libs,R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',target],folder,folder/(target.name+'.link.log'))

def run():
 bundle=Path(os.environ['P1_DIAGNOSTIC_BUNDLE']);sys.path.insert(0,str(bundle/'candidate/tools/p1_campaign'))
 import portable_campaign as P
 discover=P.physical_cpus
 def two():
  cpus,host=discover();cpus=cpus[:2];host.update(physical_cpus=cpus,workers=len(cpus));return cpus,host
 P.physical_cpus=two
 R,cpus=P.setup();assert len(cpus)==2
 assert Path(f'/sys/devices/system/cpu/cpu{cpus[0]}/cache/index0/coherency_line_size').read_text().strip()=='64','This controlled offset experiment requires 64-byte cache lines'
 with R.locked('isolate_cursor_alignment'):
  shutil.copy2(bundle/'walltime.c',R.ROOT/'walltime.c')
  timer=R.ROOT/'timer.c';timer.write_text('#include <time.h>\ndouble __wrap_timer_runtime(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}\n')
  R.checked(['gcc','-O2','-c',timer,'-o',R.ROOT/'monotonic_timer.o'],R.ROOT,R.OUT/'timer.log')
  if not (R.ROOT/'walltime').exists():R.checked(['gcc','-O2',R.ROOT/'walltime.c','-o',R.ROOT/'walltime'],R.ROOT,R.OUT/'walltime.log')
  bases={'A':'Accepted','C':'Candidate'};configs={};builds={}
  for family,base in bases.items():
   configs[family+'_original']=(base,False)
   for mutation in ['offset0','offset16','offset32','offset48','ctx640']:
    version='Cursor_'+family+'_'+mutation;d=R.ROOT/'versions'/version;src=d/'src';src.mkdir(parents=True)
    for p in (R.ROOT/'versions'/base/'src').iterdir():
     if p.is_file() and p.suffix in ['.c','.cpp','.h','.inc','.ll']:shutil.copy2(p,src/p.name)
    if mutation.startswith('offset'):
     f=src/'array_preparation_runtime.inc';f.write_text(prep_source(f.read_text(),int(mutation[6:])))
    else:
     f=src/'autotuner_runtime.c';s=f.read_text();needle='calloc(1, sizeof(*ctx))';assert s.count(needle)==1;f.write_text(s.replace(needle,'calloc(1, 640)',1))
    R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',d/'autotuner_runtime.o'],src,d/'build.log')
    link(R,version,base,SELECTED)
    configs[family+'_'+mutation]=(version,False);builds[version]=base
    R.progress(stage='cursor_build',version=version)
  configs['AA']=('Accepted',False)
  # Equal-length executable names control argv/AT_EXECFN layout. Original-path
  # copies below explicitly test whether that choice changes the source effect.
  for family,base in bases.items():configs[family+'_original_path']=(base,True)
  aliases=R.ROOT/'canonical';aliases.mkdir();exe_map={};fingerprints={}
  for label,(version,original_path) in configs.items():
   for item in SELECTED:
    row=next(r for r in R.M.ROWS if r['graph']==item['graph'])
    for mode in ['timed','validate']:
     exe=R.case(version,item['algorithm'],row)/mode;digest=R.sha(exe)
     alias=aliases/digest
     if not alias.exists():os.link(exe,alias)
     exe_map[(label,item['algorithm'],item['graph'],mode)]=exe if original_path else alias
     fingerprints[str(exe)]=digest
  metadata={}
  for label,(version,_) in configs.items():
   exe=exe_map[(label,SELECTED[0]['algorithm'],SELECTED[0]['graph'],'timed')]
   metadata[label]=symbols(exe)
  save(R.OUT/'cursor_protocol.json',dict(cases=SELECTED,workers=2,physical_cpus=cpus,blocks=2,repetitions=20,
    configs={k:dict(version=v[0],original_path=v[1]) for k,v in configs.items()},fingerprints=fingerprints,
    method='Exact original inputs/binaries; cursor variants allocate equal extra capacity and force only cursor offset modulo64. No callback, order, worker, work-volume or algorithm change.',symbols=metadata,
    cpuinfo=Path('/proc/cpuinfo').read_text(),cache_line_bytes=Path(f'/sys/devices/system/cpu/cpu{cpus[0]}/cache/index0/coherency_line_size').read_text().strip()))
  for workers in [1,2]:
   for item in SELECTED:
    row=next(r for r in R.M.ROWS if r['graph']==item['graph'])
    for label,(version,_) in configs.items():
     x=R.invoke('cursor_mechanism','validate_cursor',version,item['algorithm'],row,0,label=label,workers=workers,
                executable_override=exe_map[(label,item['algorithm'],item['graph'],'validate')])
     assert x.get('validation') and not x.get('error'),x
   R.progress(stage='cursor_validated',workers=workers,completed=len(SELECTED)*len(configs))
  records=[];rng=random.Random(20261021)
  for block in range(2):
   for a in ['bfs','sssp','cc']:
    selected=[x for x in SELECTED if x['algorithm']==a];started=time.monotonic()
    for i,item in enumerate(selected):
     row=next(r for r in R.M.ROWS if r['graph']==item['graph'])
     for label,(version,_) in configs.items():
      x=R.invoke('cursor_mechanism',f'cursor_warmup_{block}',version,a,row,0,label=label,workers=2,executable_override=exe_map[(label,a,row['graph'],'timed')]);assert not x.get('error'),x
     for rep in range(20):
      order=list(configs);rng.shuffle(order)
      for label in order:
       version=configs[label][0]
       x=R.invoke('cursor_mechanism',f'cursor_{block}',version,a,row,rep,label=label,workers=2,executable_override=exe_map[(label,a,row['graph'],'timed')]);assert not x.get('error'),x
       records.append(dict(x,independent_block=block))
     save(R.OUT/'cursor_measurements.json',records)
     subset=[x for x in records if x['algorithm']==a and x['graph']==row['graph'] and x['independent_block']==block]
     means={label:statistics.mean(x['seconds'] for x in subset if x['label']==label)*1000 for label in configs}
     R.progress(stage='cursor_timing',algorithm=a,graph=row['graph'],block=block,completed=i+1,total=len(selected),elapsed_seconds=time.monotonic()-started,mean_ms=means)
  analyse(R,records,configs)
  # Minimal address instrumentation: no allocation or logging inside the kernel.
  # This is separate from all uninstrumented timing and checked independently.
  targets=[SELECTED[0],SELECTED[3]];probes=[]
  for label in ['A_original','C_original','A_offset0','C_offset0','A_offset32','C_offset32','A_ctx640','C_ctx640']:
   version=configs[label][0];base=builds.get(version,version);d=R.ROOT/'versions'/version
   src=d/('probe_src_'+label);src.mkdir()
   for p in (d/'src').iterdir():
    if p.is_file() and p.suffix in ['.c','.h','.inc']:shutil.copy2(p,src/p.name)
   f=src/'array_preparation_runtime.inc';text=f.read_text();needle='  if(!positions||!work)goto done;';assert text.count(needle)==1
   text=PROBE+'\n'+text.replace(needle,needle+'\n  diag_cursor_record(positions,parts,chunks,width);',1);f.write_text(text)
   R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',d/'profile.o'],src,d/('probe_'+label+'.log'))
   link(R,version,base,targets,profile=True)
   for item in targets:
    row=next(r for r in R.M.ROWS if r['graph']==item['graph']);folder=R.case(version,item['algorithm'],row)
    path=R.OUT/(label+'_'+item['algorithm']+'_cursor_addresses.json')
    x=R.invoke('cursor_mechanism','validate_cursor_probe',version,item['algorithm'],row,0,label=label,workers=2,
       extra={'SGPL_CURSOR_LAYOUT_JSON':str(path)},executable_override=folder/'layout_profile.validate')
    assert x.get('validation') and not x.get('error'),x
    probes.append(dict(label=label,**item,address_layout=R.read(path),measurement=x))
   R.progress(stage='cursor_address_profile',variant=label,completed=len(probes))
  save(R.OUT/'cursor_address_profiles.json',probes)
  # Bypass the distro perf wrapper, which did not recognize Kaggle's kernel.
  perf=next((p for p in Path('/usr/lib/linux-tools').glob('*/perf') if p.is_file() and os.access(p,os.X_OK)),None)
  if perf:
   p=subprocess.run([str(perf),'stat','-e','cycles:u,instructions:u,cache-misses:u','--','true'],capture_output=True,text=True,timeout=15)
   save(R.OUT/'direct_perf_availability.json',dict(path=str(perf),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr))
  assert all(R.sha(p)==h for p,h in fingerprints.items())
  R.progress(stage='cursor_complete',completed=len(records),failures=0)

def analyse(R,records,configs):
 import numpy as np
 from scipy.stats import permutation_test
 rows=[];tests=[];aa=[]
 contrasts=[('A_original','C_original'),('A_original','A_ctx640'),('C_original','C_ctx640'),('A_ctx640','C_ctx640'),
  ('A_offset0','C_offset0'),('A_offset0','A_offset16'),('A_offset0','A_offset32'),('A_offset0','A_offset48'),
  ('C_offset0','C_offset16'),('C_offset0','C_offset32'),('C_offset0','C_offset48'),('C_original','C_offset0'),
  ('A_original','A_original_path'),('C_original','C_original_path')]
 for block in range(2):
  for item in SELECTED:
   selected=[x for x in records if x['algorithm']==item['algorithm'] and x['graph']==item['graph'] and x['independent_block']==block]
   groups={n:sorted([x for x in selected if x['label']==n],key=lambda x:x['repeat']) for n in configs}
   assert all(len(x)==20 for x in groups.values())
   rows.append(dict(item,block=block,means={n:dict(whole_ms=statistics.mean(x['seconds'] for x in xs)*1000,
      kernel_ms=statistics.mean(x['internal_seconds'] for x in xs)*1000,cpu_ms=statistics.mean(x['user_seconds']+x['system_seconds'] for x in xs)*1000,
      minor_faults=statistics.mean(x['minflt'] for x in xs)) for n,xs in groups.items()}))
   for left,right in [*contrasts,('A_original','AA')]:
    d=np.log([x['seconds'] for x in groups[right]])-np.log([x['seconds'] for x in groups[left]])
    p=1.0 if np.all(d==0) else float(permutation_test((d,),np.mean,permutation_type='samples',alternative='two-sided',n_resamples=100000,random_state=np.random.default_rng(2358)).pvalue)
    t=dict(item,block=block,left=left,right=right,p=p,geometric_percent=(math.exp(float(np.mean(d)))-1)*100)
    (aa if right=='AA' else tests).append(t)
 for family in [tests,aa]:
  running=0
  for i,t in enumerate(sorted(family,key=lambda x:x['p'])):
   running=max(running,min(1,t['p']*(len(family)-i)));t['holm_p']=running
 save(R.OUT/'cursor_summary.json',dict(cases=rows,tests=tests,aa_tests=aa))
 doc=['<!doctype html><meta charset="utf-8"><title>Cursor alignment mechanism controls</title><style>body{font:14px system-ui;margin:24px}table{border-collapse:collapse}td,th{border:1px solid #aaa;padding:5px;text-align:right}td:first-child{text-align:left}</style><h1>Controlled cursor alignment</h1><p>Mean whole-process milliseconds; two independent 20-pair blocks; two physical workers; forced offsets differ only in the cursor address within equally sized backing storage. Primary names have equal length. Raw observations and corrected tests are retained.</p><div style="overflow:auto"><table><tr><th>Case / block</th>'+''.join('<th>'+html.escape(n)+'</th>' for n in configs)+'</tr>']
 for x in rows:doc.append('<tr><td>'+html.escape(x['algorithm']+' '+x['graph']+' / '+str(x['block']+1))+'</td>'+''.join(f'<td>{x["means"][n]["whole_ms"]:.5f}</td>' for n in configs)+'</tr>')
 doc.append('</table></div>');(R.OUT/'cursor_report.html').write_text('\n'.join(doc))

def prepare(args):
 out=Path(args.prepare);out.mkdir(parents=True,exist_ok=False)
 archive=Path(args.builds);members={};wanted=set()
 for family in ['Accepted','Candidate']:
  prefix='p1-campaign/versions/'+family+'/'
  wanted.update(prefix+n+'.o' for n in ['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime','roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime'])
  for item in SELECTED:
   for mode in ['timed','validate']:
    path=prefix+'cases/'+item['algorithm']+'/'+item['graph'].removesuffix('.edges')+'/'+mode
    wanted.update([path,path+'.o'])
 with tarfile.open(archive) as t:
  for m in t:
   src=any(m.name.startswith('p1-campaign/versions/'+v+'/src/') for v in ['Accepted','Candidate'])
   if m.isfile() and (m.name in wanted or (src and Path(m.name).suffix in ['.c','.cpp','.h','.inc','.ll'])):
    members[m.name]=hashlib.sha256(t.extractfile(m).read()).hexdigest()
 assert wanted<=members.keys()
 payload=Path(__file__).read_text()
 manifest='18b58141f3c2d62de2af4cdfef8042fcba13eeebe0aae15db802d359c602fb21'
 code='''from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,tarfile,traceback
S=Path('/kaggle/temp/p1-static-schedules');W=Path('/kaggle/working');S.mkdir(parents=True,exist_ok=True)
try:
 archive=next(Path('/kaggle/input').rglob('p1-campaign.blob'))
 with tarfile.open(archive) as t:t.extractall(S,filter='data')
 bundle=S/'p1-bundle'
 assert hashlib.sha256((bundle/'bundle_manifest.json').read_bytes()).hexdigest()==MANIFEST
 for n,h in json.loads((bundle/'bundle_manifest.json').read_text()).items():assert hashlib.sha256((bundle/n).read_bytes()).hexdigest()==h,n
 archives=list(Path('/kaggle/input').rglob('diagnostic-builds.tar.gz'))
 if archives:
  assert hashlib.sha256(archives[0].read_bytes()).hexdigest()==ARCHIVE_HASH
  with tarfile.open(archives[0]) as t:
   for m in t:
    if m.name not in FILES:continue
    assert m.isfile() and '..' not in Path(m.name).parts
    p=S/m.name;p.parent.mkdir(parents=True,exist_ok=True)
    with p.open('wb') as f:shutil.copyfileobj(t.extractfile(m),f)
    os.chmod(p,m.mode)
 else:
  # Some input providers expose archives as an expanded tree.
  roots=list(Path('/kaggle/input').rglob('Accepted/src/autotuner_runtime.c'))
  assert len(roots)==1
  expanded=roots[0].parents[3]
  for n in FILES:
   p=S/n;p.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(expanded/Path(n).relative_to('p1-campaign'),p);os.chmod(p,0o755 if p.name in ['timed','validate'] else 0o644)
 for n,h in FILES.items():assert hashlib.sha256((S/n).read_bytes()).hexdigest()==h,n
 sys.path.insert(0,str(bundle/'candidate/tools/p1_campaign'));import portable_campaign as P
 cpus,host=P.physical_cpus();cpus=cpus[:2];assert len(cpus)==2;os.sched_setaffinity(0,cpus)
 (W/'mechanism_host.json').write_text(json.dumps(dict(host=host,cpuinfo=Path('/proc/cpuinfo').read_text()),indent=2))
 subprocess.run(['apt-get','update','-qq'],check=True)
 subprocess.run(['apt-get','install','-y','-qq','libnlopt-dev','strace','linux-tools-generic'],check=True)
 os.environ['P1_DIAGNOSTIC_BUNDLE']=str(bundle);os.environ['P1_PORTABLE_WORK']=str(S/'p1-campaign')
 script=S/'isolate_cursor_alignment.py';script.write_text(PAYLOAD)
 with (W/'cursor_run.log').open('w') as f:
  p=subprocess.Popen([sys.executable,script,'--run'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,bufsize=1)
  for line in p.stdout:f.write(line);f.flush();print(line.rstrip(),flush=True)
  p.wait();assert p.returncode==0,'Diagnostic process failed'
except BaseException:
 (W/'failure.txt').write_text(traceback.format_exc());raise
finally:
 root=S/'p1-campaign'
 with tarfile.open(W/'cursor-evidence.tar.gz','w:gz',compresslevel=3,dereference=True) as t:
  for p in [root/'results',root/'runs']:
   if p.exists():t.add(p,arcname=str(p.relative_to(S)))
  for p in (root/'versions').glob('Cursor_*'):
   t.add(p,arcname=str(p.relative_to(S)))
 for n in ['cursor_summary.json','cursor_report.html','cursor_address_profiles.json','direct_perf_availability.json']:
  p=root/'results'/n
  if p.exists():shutil.copy2(p,W/n)
'''
 code='MANIFEST='+repr(manifest)+'\nARCHIVE_HASH='+repr(sha(archive))+'\nFILES='+repr(members)+'\nPAYLOAD='+repr(payload)+'\n'+code
 compile(code,str(out/'mechanism.py'),'exec');(out/'mechanism.py').write_text(code)
 metadata=dict(id='kazisahib/p1-cursor-alignment-mechanism',title='P1 cursor alignment mechanism',code_file='mechanism.py',language='python',kernel_type='script',is_private=True,enable_gpu=False,enable_internet=True,competition_sources=[],dataset_sources=['kazisahib/p1-static-dag-cpu-campaign-20261020'],kernel_sources=['kazisahib/p1-static-schedule-regression-diagnosis'])
 save(out/'kernel-metadata.json',metadata)
 save(out/'identity.json',dict(notebook_sha256=sha(out/'mechanism.py'),artifact_sha256=sha(archive),selected_files=len(members),cases=SELECTED))
 print(json.dumps(dict(notebook=str(out),sha256=sha(out/'mechanism.py'),files=len(members))))

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prepare');p.add_argument('--builds');p.add_argument('--run',action='store_true');args=p.parse_args()
 if args.prepare:prepare(args)
 elif args.run:run()
 else:p.error('Specify --prepare or --run')
