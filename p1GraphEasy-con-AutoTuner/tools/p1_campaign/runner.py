#!/usr/bin/env python3
"""Resumable, serialized six-physical-core optimization experiments (WSL).

Run with the investigation venv Python. No command overwrites old observations.
Build/validate before benchmark; status is read-only and does not take the lock.
"""
from pathlib import Path
import argparse,concurrent.futures,contextlib,fcntl,hashlib,importlib.util,json,math,os,re,resource,shlex,shutil,subprocess,sys,time,traceback,statistics,threading
import numpy as np
from scipy.stats import permutation_test

HERE=Path(__file__).resolve().parent
REPO=HERE.parent.parent
ROOT=Path(os.environ.get('P1_CAMPAIGN_ROOT',str(Path.home()/'p1_campaign_20261019')))
OUT=Path(os.environ.get('P1_CAMPAIGN_OUTPUT','/mnt/c/p1campaign-results'))
OLD=Path.home()/'p1_local_regressions_20261019'
OLDOUT=Path('/mnt/d/sgpl/p1_local_regressions_20261019')
ALGS=['bfs','sssp','cc','pagerank'];CPUS=[0,2,4,6,8,10]
OBJECTS=['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime','roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']
ROOT.mkdir(exist_ok=True);OUT.mkdir(exist_ok=True)
os.sched_setaffinity(0,CPUS)
resource.setrlimit(resource.RLIMIT_STACK,(resource.RLIM_INFINITY,resource.RLIM_INFINITY))
resource.setrlimit(resource.RLIMIT_CORE,(0,0))

def save(p,x):
 p=Path(p);p.parent.mkdir(parents=True,exist_ok=True);t=p.with_suffix(p.suffix+'.'+str(os.getpid())+'.'+str(threading.get_ident())+'.tmp');t.write_text(json.dumps(x,indent=2));t.replace(p)
def read(p,default=None):return json.loads(Path(p).read_text()) if Path(p).exists() else default
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def key(x):return hashlib.sha256(json.dumps(x,sort_keys=True).encode()).hexdigest()
def journal(p,x):
 with Path(p).open('a') as f:f.write(json.dumps(x)+'\n')
def git(*args):
 # The managed repository metadata uses Windows absolute worktree paths.
 win=subprocess.check_output(['wslpath','-w',str(REPO)],text=True).strip()
 return subprocess.check_output(['/mnt/c/Program Files/Git/cmd/git.exe','-C',win,*args],text=True).strip()
def state(**kw):
 x=read(OUT/'status.json',{});x.update(kw,updated=time.time());save(OUT/'status.json',x)
def progress(**kw):
 state(**kw);print(json.dumps(kw),flush=True)
def cfg(*args):return shlex.split(subprocess.check_output(['llvm-config-20',*args],text=True))

def bootstrap():
 """Pin helper code, templates, reference vectors and input identities once."""
 if not (ROOT/'support/common.py').exists():
  d=ROOT/'support';d.mkdir(exist_ok=True)
  t=(OLDOUT/'common.py').read_text().replace("Path.home()/'p1_local_regressions_20261019'",repr(str(ROOT)))
  t=t.replace('ROOT='+repr(str(ROOT)),'ROOT=Path('+repr(str(ROOT))+')')
  t=t.replace("Path('/mnt/d/sgpl/p1_local_regressions_20261019')",'Path('+repr(str(OUT))+')')
  start=t.index('REPORTS=');end=t.index('\nCPUS=',start)
  t=t[:start]+"REPORTS=ROOT/'frozen_reports'"+t[end:]
  (d/'common.py').write_text(t)
  shutil.copy2(OLD/'inputs.json',ROOT/'inputs.json')
  shutil.copytree(OLD/'references',ROOT/'references',dirs_exist_ok=True)
  shutil.copytree(OLD/'historical_harness',ROOT/'historical_harness',dirs_exist_ok=True)
  reports=Path('/mnt/c/Users/kazis/.codex/visualizations/2026/10/15/01a13e48-4154-7852-acb7-9850e234ec1b')
  shutil.copytree(reports/'profile50_five_algorithms_20261017/algorithms',ROOT/'frozen_reports/profile50_five_algorithms_20261017/algorithms',dirs_exist_ok=True)
  shutil.copy2(OLDOUT/'walltime.c',ROOT/'walltime.c')
  shutil.copy2(OLD/'monotonic_timer.o',ROOT/'monotonic_timer.o')
  shutil.copytree(OLD/'graptor',ROOT/'graptor',dirs_exist_ok=True)
  shutil.copy2(OLD/'graptor_controls/pagerank',ROOT/'graptor/PRdouble')
 spec=importlib.util.spec_from_file_location('campaign_common',ROOT/'support/common.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 return m

@contextlib.contextmanager
def locked(command):
 p=ROOT/'experiment.lock'
 with p.open('a+') as f:
  try:fcntl.flock(f,fcntl.LOCK_EX|fcntl.LOCK_NB)
  except BlockingIOError:raise RuntimeError('Another build, validation, benchmark or profile holds the experiment lock')
  state(command=command,pid=os.getpid(),started=time.time(),active=True,failures=0,error=None)
  try:yield
  except BaseException as e:
   state(active=False,error=(type(e).__name__+': '+str(e))[:3000]);raise
  finally:state(active=False)

def checked(cmd,cwd,log,env=None,timeout=1800):
 Path(log).parent.mkdir(parents=True,exist_ok=True)
 with open(log,'w') as f:p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=timeout)
 if p.returncode:raise RuntimeError(str(log)+': '+Path(log).read_text()[-2500:])

def local_deps(src,base,seen=None):
 seen=set() if seen is None else seen
 p=Path(src)
 if p in seen or not p.exists():return {}
 seen.add(p);out={str(p.relative_to(base)):sha(p)}
 for inc in re.findall(r'^\s*#\s*include\s*["<]([^">]+)',p.read_text(errors='replace'),re.M):
  q=next((x for x in [p.parent/inc,base/inc,base/'generated'/inc] if x.exists() and x.is_file()),None)
  if q:out.update(local_deps(q,base,seen))
 return out

def freeze(v):
 d=ROOT/'versions'/v;src=d/'src';d.mkdir(parents=True,exist_ok=True)
 identity={'commit':git('rev-parse','HEAD'),'diff':git('diff','HEAD','--','.')}
 files=[p for p in REPO.iterdir() if p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
 files += [p for p in (REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
 contents={str(p.relative_to(REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files if p.is_file()}
 identity['source_hash']=key(contents)
 old=read(d/'identity.json')
 if old:
  assert old==identity,'Version name already belongs to different source; choose a new attempt name'
 else:
  src.mkdir(exist_ok=True)
  for p in files:
   if p.is_file():q=src/p.relative_to(REPO);q.parent.mkdir(parents=True,exist_ok=True);q.write_text(p.read_text())
  shutil.copytree(REPO/'test',src/'test',dirs_exist_ok=True)
  save(d/'identity.json',identity);(d/'candidate.patch').write_text(identity['diff'])
  shutil.copy2(d/'identity.json',OUT/(v+'_identity.json'))
 return d,src,identity

def compile_cache(src,base,cmd,log,seed=None):
 deps=local_deps(src,base);k=key({'deps':deps,'flags':cmd,'compiler':subprocess.check_output([cmd[0],'--version'],text=True).splitlines()[0]})
 cache=ROOT/'object_cache';cache.mkdir(exist_ok=True);obj=cache/(k+'.o')
 if not obj.exists():
  if seed and seed.exists():shutil.copy2(seed,obj)
  else:checked([*cmd,'-c',str(src.relative_to(base)),'-o',str(obj)],base,log)
 return obj

def build(v):
 d,src,identity=freeze(v);logs=d/'build_logs';logs.mkdir(exist_ok=True)
 flags=['g++','-O2','-mavx2','-march=native','-std=c++17','-fexceptions','-pthread','-I/usr/local/include/antlr4-runtime','-Igenerated','-iquote','.', '-I'+cfg('--includedir')[0]+'/polly',*[x for x in cfg('--cxxflags') if x!='-fno-exceptions']]
 sources=['main.cpp','IRGenVisitor.cpp','MotifPattern.cpp','MotifIRBuilder.cpp','ASTBuilder.cpp','pdg.cpp','parallel_loop_outline.cpp','graph_frontier_lowering.cpp','SemanticAnalyzer.cpp','roaring_bitmap.cpp','AutoTunerPass.cpp']+[str(p.relative_to(src)) for p in (src/'generated').glob('*.cpp')]
 def cc(s):
  progress(stage='build',version=v,item=s)
  return compile_cache(src/s,src,flags,logs/(s.replace('/','_')+'.log'))
 if not (d/'GraphProgram').exists():
  with concurrent.futures.ThreadPoolExecutor(2) as pool:objects=list(pool.map(cc,sources))
  checked(['g++',*objects,*cfg('--ldflags'),'-Wl,--no-keep-memory','-pthread','-lantlr4-runtime',*cfg('--libs','all'),*cfg('--system-libs'),'-o',d/'GraphProgram'],src,logs/'compiler_link.log')
 for name in OBJECTS:
  ext='.cpp' if name in ['roaring_bitmap','graph_loader_runtime','graph_runtime'] else '.c'
  f=['g++','-std=c++17','-mavx2','-march=native'] if ext=='.cpp' else ['gcc']
  obj=compile_cache(src/(name+ext),src,[*f,'-O3','-g','-fopenmp','-pthread','-iquote','.'],logs/(name+'.log'))
  target=d/(name+'.o')
  if not target.exists():target.symlink_to(obj)
 if not (ROOT/'walltime').exists():checked(['gcc','-O2',ROOT/'walltime.c','-o',ROOT/'walltime'],ROOT,logs/'walltime.log')
 rows=M.ROWS
 failures=[];completed=set(read(d/'program_builds.json',[]))
 compiler_id=sha(d/'GraphProgram')
 program_cache=ROOT/'program_cache';program_cache.mkdir(exist_ok=True)
 for alg in ALGS:
  for r in rows:
   ck=alg+'/'+r['graph']
   if ck in completed:continue
   p=case(v,alg,r);p.mkdir(parents=True,exist_ok=True)
   for ll in src.glob('*.ll'):
    if not (p/ll.name).exists():(p/ll.name).symlink_to(ll)
   try:
    for mode in ['timed','validate']:
     text=M.make_source(alg,r,mode=='validate');source=p/(mode+'.graph');source.write_text(text)
     k=key([compiler_id,text]);obj=program_cache/(k+'.o')
     if not obj.exists():
      e=M.env();e.update(FORCE_CPU='1',GRAPH_FRONTIER_STATS='1',SGPL_RECORD_LIVENESS_STATS='1')
      checked([d/'GraphProgram','--ir-backend=cpu',source],p,p/(mode+'.compile.log'),e,1200)
      shutil.move(p/'program.o',obj)
     target=p/(mode+'.o')
     if not target.exists():target.symlink_to(obj)
     if not (p/mode).exists():checked(['g++','-O3','-fopenmp','-no-pie',obj,*[d/(n+'.o') for n in OBJECTS],ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',p/mode],p,p/(mode+'.link.log'))
    completed.add(ck);save(d/'program_builds.json',sorted(completed));progress(stage='programs',version=v,algorithm=alg,completed=len(completed),total=244)
   except Exception as ex:
    failures.append({'case':ck,'error':repr(ex)});save(d/'build_failures.json',failures);raise
 fingerprints={str(p.relative_to(d)):sha(p) for r in rows for a in ALGS for p in [case(v,a,r)/'timed',case(v,a,r)/'validate']}
 save(d/'build.json',{'identity':identity,'compiler':compiler_id,'executables':fingerprints,'fingerprint':key(fingerprints)})
 progress(stage='build_complete',version=v,completed=244)

def case(v,a,r):return ROOT/'versions'/v/'cases'/a/r['graph'].removesuffix('.edges')
def binary_cmd(v,a,r,validation,workers,output):
 if v.startswith('graptor'):
  cmd,e=M.command(a,r,'graptor',validation,workers,output)
  if v=='graptor_double' and a=='pagerank':cmd[0]=str(ROOT/'graptor/PRdouble')
  return cmd,e,ROOT
 return [str(case(v,a,r)/('validate' if validation else 'timed'))],M.env(workers=workers),case(v,a,r)

def invoke(attempt,phase,v,a,r,rep,label=None,workers=6,extra=None,profile=None,repeat_block=0):
 name=label or v;tag=key([phase,name,a,r['graph'],rep,workers,extra,profile,repeat_block])[:20]
 d=ROOT/'runs'/attempt/phase/a/r['graph'].removesuffix('.edges');d.mkdir(parents=True,exist_ok=True)
 base=d/tag;stamp=base.with_suffix('.stamp.json');record=base.with_suffix('.record.json')
 if record.exists():return read(record)
 validate=phase.startswith('validate')
 cmd,e,cwd=binary_cmd(v,a,r,validate,workers,base.with_suffix('.values'))
 if extra:e.update(extra)
 if profile=='phases':
  cmd[0]=str(case(v,a,r)/('profile_validate' if validate else 'profile'));e.update(SGPL_PROFILE_JSON=str(base)+'.phases.json',SGPL_POOL_PROFILE_JSON=str(base)+'.pool.json',SGPL_WORK_JSON=str(base)+'.work.json')
 if profile=='perf':cmd=['/usr/lib/linux-tools/6.8.0-147-generic/perf','stat','-x,','-e','cycles:u,instructions:u,branches:u,branch-misses:u,cache-references:u,cache-misses:u','-o',str(base)+'.perf.csv','--',*cmd]
 if profile=='futex':cmd=['strace','-f','-c','-e','futex','-o',str(base)+'.futex.txt',*cmd]
 if profile=='sched':cmd=['sudo','-n','-E','/usr/lib/linux-tools/6.8.0-147-generic/perf','record','-a','-C',','.join(map(str,CPUS[:workers])),'-e','sched:sched_switch','-e','sched:sched_wakeup','-o',str(base)+'.sched.data','--',*cmd]
 os.sched_setaffinity(0,CPUS[:workers]);start=time.time()
 try:
  with base.with_suffix('.out').open('w') as f,base.with_suffix('.err').open('w') as g:
   p=subprocess.run([str(ROOT/'walltime'),str(stamp),*cmd],cwd=cwd,env=e,stdout=f,stderr=g,timeout=190)
  x=read(stamp,{'exit_code':p.returncode,'seconds':time.time()-start})
  x.update(version=v,label=name,algorithm=a,graph=r['graph'],cohort=r['cohort'],phase=phase,repeat=rep,workers=workers,extra=extra or {},profile=profile,base=str(base),command=cmd,block=repeat_block)
  if p.returncode:x['error']=base.with_suffix('.err').read_text()[-3000:]
  elif validate:
   try:
    arr=np.loadtxt(base.with_suffix('.values'),ndmin=1) if v.startswith('graptor') else np.array(base.with_suffix('.out').read_text().split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0].split(),dtype=float)
    x['validation']=M.validate(a,r,arr,'graptor' if v.startswith('graptor') else 'p1')
   except Exception as ex:x['error']=repr(ex)
  if profile=='phases':
   for n in ['phases','pool','work']:
    path=Path(str(base)+'.'+n+'.json')
    if path.exists():x[n]=read(path)
  if profile in ['perf','futex']:
   path=Path(str(base)+('.perf.csv' if profile=='perf' else '.futex.txt'));x['diagnostic_output']=path.read_text() if path.exists() else ''
  save(record,x);journal(OUT/(attempt+'_measurements.jsonl'),x)
  return x
 finally:os.sched_setaffinity(0,CPUS)

def verify_fingerprint(v):
 b=read(ROOT/'versions'/v/'build.json');assert b,'Version is not fully built'
 for n,h in b['executables'].items():assert sha(ROOT/'versions'/v/n)==h,'Executable fingerprint changed: '+n
 return b['fingerprint']

def validate_version(v):
 verify_fingerprint(v);records=[]
 for a in ALGS:
  for i,r in enumerate(M.ROWS):
   x=invoke(v,'validate',v,a,r,0);records.append(x)
   progress(stage='validate',version=v,algorithm=a,completed=i+1,total=61,failures=sum('error' in t for t in records))
 save(OUT/(v+'_validation.json'),records)
 assert all('validation' in x and not x.get('error') for x in records),'Correctness failure; no acceptance timings permitted'

def calibrate(a,mode,signature,started):
 elapsed=time.time()-started;path=OUT/'calibration.json';c=read(path,{})
 k=key([a,mode,signature]);first=k not in c
 if first:c[k]={'algorithm':a,'mode':mode,'signature':signature,'seconds':elapsed,'measured':time.time()};save(path,c)
 progress(stage=mode+'_block_complete',algorithm=a,block_seconds=elapsed,monitor_interval_seconds=c[k]['seconds'],first_calibration=first)

def samples(records,phase,label,a,g):return [x['seconds'] for x in records if x['phase']==phase and x['label']==label and x['algorithm']==a and x['graph']==g and not x.get('error')]
def gm(v):return math.exp(statistics.mean(math.log(t) for t in v))

def aggregate(records,phase,base='baseline',candidate='candidate'):
 out={}
 for cohort in ['all','main50','deduplicated']:
  t={}
  for a in ALGS+['overall']:
   pairs=[];seen=set()
   for alg in ALGS:
    if a!='overall' and a!=alg:continue
    for r in M.ROWS:
     if cohort=='main50' and r['cohort']!='main50':continue
     semantic=(alg,r['original_sha256'],r['source'],r['p1_directed'],sha(r['weights_i32']) if alg=='sssp' else '')
     if cohort=='deduplicated' and semantic in seen:continue
     seen.add(semantic)
     b=samples(records,phase,base,alg,r['graph']);c=samples(records,phase,candidate,alg,r['graph'])
     if b and len(b)==len(c):pairs.append((b,c))
   t[a]={'cases':len(pairs),'speedup_mean':gm([statistics.mean(b)/statistics.mean(c) for b,c in pairs]) if pairs else None,'speedup_median':gm([statistics.median(b)/statistics.median(c) for b,c in pairs]) if pairs else None}
  out[cohort]=t
 return out

def holm(p):
 order=np.argsort(p);res=np.ones(len(p));prior=0
 for rank,i in enumerate(order):prior=max(prior,min(1.,p[i]*(len(p)-rank)));res[i]=prior
 return list(map(float,res))

def regression_tests(records,phase,candidate='candidate',baseline='baseline'):
 rng=np.random.default_rng(2358);tests=[];blocks={}
 for a in ALGS:
  rows=[]
  for r in M.ROWS:
   b=np.array(samples(records,phase,baseline,a,r['graph']));c=np.array(samples(records,phase,candidate,a,r['graph']))
   if len(b)!=20 or len(c)!=20:raise RuntimeError('Confirmation needs 20 complete paired observations per case')
   diff=c-b
   p=float(permutation_test((diff,),lambda x,axis=-1:np.mean(x,axis=axis),permutation_type='samples',alternative='greater',n_resamples=100000,batch=512,rng=rng).pvalue) if np.any(diff) else 1.
   tests.append({'algorithm':a,'graph':r['graph'],'ratio_candidate_over_baseline':float(c.mean()/b.mean()),'p':p,'kind':'case'})
   rows.append((b,c))
  blocks[a]=rows
 for a,rows in blocks.items():
  b=np.array([x[0] for x in rows]);c=np.array([x[1] for x in rows]);obs=float(np.mean(np.log(c.mean(axis=1)/b.mean(axis=1))))
  count=0;done=0
  while done<100000:
   n=min(256,100000-done);swap=rng.integers(0,2,(n,*b.shape),dtype=np.int8)
   lhs=np.where(swap,c,b).mean(axis=2);rhs=np.where(swap,b,c).mean(axis=2)
   statistic=np.mean(np.log(rhs/lhs),axis=1);count+=int(np.count_nonzero(statistic>=obs));done+=n
  tests.append({'algorithm':a,'kind':'algorithm','ratio_candidate_over_baseline':math.exp(obs),'p':(count+1)/100001})
 adj=holm([x['p'] for x in tests])
 for x,p in zip(tests,adj):x.update(p_holm=p,regression=p<.05 and x['ratio_candidate_over_baseline']>1)
 return tests

def benchmark(v,baseline=None,confirm=False,block=0):
 verify_fingerprint(v)
 val=read(OUT/(v+'_validation.json'));assert val and len(val)==244 and all(not x.get('error') for x in val)
 if baseline:verify_fingerprint(baseline)
 records=[];phase='confirmation' if confirm else 'primary';reps=20 if confirm else 5
 versions=[('baseline',baseline),('candidate',v),('aa',baseline)] if baseline else [('candidate',v)]
 if not confirm:versions+=[('graptor','graptor')]
 fingerprints={x:verify_fingerprint(x) for _,x in versions if x and not x.startswith('graptor')}
 protocol={'candidate':v,'baseline':baseline,'fingerprints':fingerprints,'dataset':read(OUT/'manifest.json')['dataset_hash'],'repetitions':reps,'phase':phase,'block':block}
 path=OUT/(v+'_'+phase+('_repeat' if block else '')+'_protocol.json')
 if path.exists():assert read(path)==protocol,'Cannot resume modified binaries or inputs'
 else:save(path,protocol)
 for a in ALGS:
  start=time.time()
  for i,r in enumerate(M.ROWS):
   for label,w in versions:invoke(v,phase+'_warmup',w,a,r,0,label=label,repeat_block=block)
   for rep in range(reps):
    shift=(rep+i)%len(versions);ordered=versions[shift:]+versions[:shift]
    for label,w in ordered:
     x=invoke(v,phase,w,a,r,rep,label=label,repeat_block=block);records.append(x)
   progress(stage=phase,version=v,baseline=baseline,algorithm=a,completed=i+1,total=61,failures=sum(bool(x.get('error')) for x in records),elapsed_seconds=time.time()-start)
  calibrate(a,phase,[61,reps,[n for n,_ in versions],6],start)
 save(OUT/(v+'_'+phase+('_repeat' if block else '')+'.json'),records)
 if not baseline:return
 result={'candidate':v,'baseline':baseline,'phase':phase,'block':block,'aggregate':aggregate(records,phase),'graptor':aggregate(records,phase,'graptor','candidate') if not confirm else None,'failures':sum(bool(x.get('error')) for x in records)}
 save(OUT/(v+'_'+phase+'_summary.json'),result)
 if confirm:
  if result['failures']:
   result.update(decision='reject',reason='incomplete_or_failed_confirmation',promotable=False)
   save(OUT/(v+'_decision.json'),result);return
  result['tests']=regression_tests(records,phase);result['aa_tests']=regression_tests(records,phase,'aa')
  result['aa_invalid']=any(x['regression'] for x in result['aa_tests'])
  result['confirmed_regressions']=[x for x in result['tests'] if x['regression']]
  g=result['aggregate']['all']['overall'];primary=read(OUT/(v+'_primary_summary.json'))['aggregate']['all']['overall']
  result['promotable']=not result['failures'] and not result['aa_invalid'] and not result['confirmed_regressions'] and all(x[k]>1.01 for x in [g,primary] for k in ['speedup_mean','speedup_median'])
  result['decision']='invalid_aa' if result['aa_invalid'] else 'accept' if result['promotable'] else 'reject'
  save(OUT/(v+'_decision.json'),result)

def contracts(v):
 d=ROOT/'versions'/v;src=d/'src';td=src/'test';records=[]
 tests=['run_worker_pool_tests.sh','run_array_frontier_exec_tests.sh','run_array_append_epoch_tests.sh','run_general_frontier_optimization_tests.sh']
 for name in tests:
  p=td/name
  if not p.exists():continue
  t=p.read_text();t=re.sub(r'for (workers|threads|width) in [0-9 ]+;\s*do',lambda m:'for '+m[1]+' in 1 2 6; do',t)
  t=t.replace('SGPL_ALLOW_HT=1','SGPL_ALLOW_HT=0').replace('widths=(1 4 32)','widths=(1 2 6)').replace('widths=(8)','widths=(2 6)')
  if name=='run_array_frontier_exec_tests.sh':t=t.replace('for workers in 1 2 6; do\n  for mode','for workers in 2 6; do\n  for mode')
  p.write_text(t)
  assert not re.search(r'SGPL_NUM_THREADS=(?:8|32)\b|SGPL_ALLOW_HT=1|in [^;\n]*\b32\b',t)
  resource.setrlimit(resource.RLIMIT_STACK,(8*1024*1024,resource.RLIM_INFINITY))
  try:checked(['bash',str(p)],src,d/'contracts'/(name+'.log'),M.env(),1200);passed=True;error=None
  except Exception as e:passed=False;error=repr(e)
  finally:resource.setrlimit(resource.RLIMIT_STACK,(resource.RLIM_INFINITY,resource.RLIM_INFINITY))
  x={'test':name,'passed':passed,'error':error};records.append(x);progress(stage='contracts',version=v,**x)
 save(OUT/(v+'_contracts.json'),records);assert all(x['passed'] for x in records)
 # The same contracts are exercised with the version's frozen runtime objects.
 for name in ['graph_storage_layout_test','pool_participation_test']:
  ext='.cpp' if name.startswith('graph_') else '.c';testsrc=REPO/'test'/(name+ext)
  obj=d/'contracts'/(name+'.o');exe=d/'contracts'/name
  compiler='g++' if ext=='.cpp' else 'gcc'
  checked([compiler,'-O2','-I',src,'-c',testsrc,'-o',obj],src,d/'contracts'/(name+'.compile.log'))
  checked(['g++','-fopenmp',obj,*[d/(n+'.o') for n in OBJECTS],'-pthread','-ldl','-lnlopt','-o',exe],src,d/'contracts'/(name+'.link.log'))
  if ext=='.cpp':
   fixture=d/'contracts/input.edges';fixture.write_text('0 1 7\n0 1 9\n1 2 3\n2 2 4\n')
   for mode in ['off','refresh','on']:
    e=M.env();e['SGPL_GRAPH_CACHE']=mode
    checked([exe,fixture],src,d/'contracts'/(name+'.'+mode+'.log'),e)
  else:
   for workers in [1,2,6]:
    for legacy in ['0','1']:
     e=M.env(workers=workers);e['SGPL_POOL_LEGACY']=legacy
     checked([exe],src,d/'contracts'/(name+'.w'+str(workers)+'.legacy'+legacy+'.log'),e,120)
  records.append({'test':name,'passed':True,'source_sha256':sha(testsrc)})
 save(OUT/(v+'_contracts.json'),records)

def init():
 topo=[]
 for c in CPUS:
  p=Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology';topo.append((c,(p/'physical_package_id').read_text().strip(),(p/'core_id').read_text().strip()))
 assert len(set(x[1:] for x in topo))==6
 oldverify=read(OLDOUT/'input_hash_verification.json')
 input_hashes={}
 for r in M.ROWS:
  for k in ['original_file','p1_weighted_file','bin','weights','weights_i32']:input_hashes[r[k]]=sha(r[k])
 baseline_old=read(OLDOUT/'inputs.json')
 assert all(r['original_sha256']==sha(r['original_file']) for r in M.ROWS)
 manifest={'dataset_hash':key(input_hashes),'inputs':input_hashes,'cases':M.ROWS,'physical_cpus':topo,'workers':6,'graptor_partitions':8,'starting_commit':git('rev-parse','HEAD'),'graptor':{p.name:sha(p) for p in (ROOT/'graptor').iterdir() if p.is_file()},'llvm':cfg('--version'),'g++':subprocess.check_output(['g++','--version'],text=True),'old_hash_audit':key(oldverify),'support_hashes':{str(p.relative_to(ROOT)):sha(p) for p in (ROOT/'support').glob('*.py')}}
 p=OUT/'manifest.json'
 if p.exists():assert read(p)['dataset_hash']==manifest['dataset_hash'],'Dataset changed'
 else:save(p,manifest)
 progress(stage='initialized',graphs=len(M.ROWS),cases=244,root=str(ROOT),output=str(OUT))

def report():
 results={p.stem:read(p) for p in OUT.glob('*_decision.json')};save(OUT/'campaign_summary.json',results)
 print(json.dumps({k:{'decision':v['decision'],'aggregate':v['aggregate']['all']['overall'],'confirmed_regressions':len(v.get('confirmed_regressions',[]))} for k,v in results.items()},indent=2))

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('command',choices=['init','build','validate','benchmark','contracts','profile','diagnose','status','resume','report']);p.add_argument('version',nargs='?');p.add_argument('--baseline');p.add_argument('--confirmation',action='store_true');p.add_argument('--repeat-block',type=int,default=0);p.add_argument('--mode',choices=['phases','perf','futex','sched'],default='phases');p.add_argument('--cases',help='JSON list of {algorithm,graph}; default full 244-case matrix');p.add_argument('--workers',type=int,choices=[1,2,6],default=6);p.add_argument('--extra',default='{}');args=p.parse_args()
 if args.command=='status':
  raw=read(OUT/'status.json',{});allowed=['command','pid','started','active','failures','updated','stage','version','algorithm','completed','total','monitor_interval_seconds','elapsed_seconds','block_seconds']
  x={k:raw[k] for k in allowed if k in raw};x['wall_elapsed']=time.time()-raw.get('started',time.time());x['root']=str(ROOT);x['output']=str(OUT)
  if raw.get('error'):x['error']=str(raw['error'])[:1000]
  print(json.dumps(x,indent=2));return
 if args.command=='report':report();return
 global M;M=bootstrap()
 with locked(args.command):
  if args.command=='init':init()
  elif args.command=='build':build(args.version)
  elif args.command=='contracts':contracts(args.version)
  elif args.command=='validate':validate_version(args.version)
  elif args.command in ['benchmark','resume']:benchmark(args.version,args.baseline,args.confirmation,args.repeat_block)
  elif args.command in ['profile','diagnose']:
   from profile_support import run_profiles
   run_profiles(sys.modules[__name__],args)
if __name__=='__main__':main()
