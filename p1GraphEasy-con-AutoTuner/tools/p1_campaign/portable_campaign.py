"""Portable CPU campaign: rebuild both P1 versions and Graptor on this host.

Invoked inside an exported, hash-verified bundle. No benchmark runs on import.
All timings/validations/builds share the runner experiment lock. No auto-revert.
"""
from pathlib import Path
import argparse,hashlib,html,importlib.util,json,math,os,platform,re,shlex,shutil,statistics,subprocess,sys,time
BUNDLE=Path(__file__).resolve().parents[3]
WORK=Path(os.environ.get('P1_PORTABLE_WORK','/kaggle/working/p1-campaign'))
ALGS=['bfs','sssp','cc','pagerank']
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save(p,x):
 p=Path(p);p.parent.mkdir(parents=True,exist_ok=True);q=p.with_suffix(p.suffix+'.tmp');q.write_text(json.dumps(x,indent=2));q.replace(p)
def physical_cpus():
 allowed=sorted(os.sched_getaffinity(0));unique={};topology=[]
 for cpu in allowed:
  d=Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'topology'
  k=tuple((d/n).read_text().strip() for n in ['physical_package_id','core_id'])
  unique.setdefault(k,cpu);topology.append(dict(cpu=cpu,package=k[0],core=k[1]))
 quota=None
 for p in [Path('/sys/fs/cgroup/cpu.max')]:
  if p.exists():
   a,b=p.read_text().split()
   if a!='max':quota=max(1,int(a)//int(b))
 if quota is None:
  for base in [Path('/sys/fs/cgroup/cpu'),Path('/sys/fs/cgroup/cpu,cpuacct')]:
   q=base/'cpu.cfs_quota_us';period=base/'cpu.cfs_period_us'
   if q.exists() and period.exists() and int(q.read_text())>0:quota=max(1,int(q.read_text())//int(period.read_text()))
 selected=list(unique.values())[:min(6,quota or 6)]
 if not selected:raise RuntimeError('No distinct physical-core representatives are visible')
 return selected,dict(platform=platform.platform(),topology=topology,quota_cores=quota,
  boot_id=Path('/proc/sys/kernel/random/boot_id').read_text().strip(),
  cpu_model=next((x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),''),
  cpu_flags=next((x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('flags')),''),
  cpu_identity={k:next((line.split(':',1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if ':' in line and line.split(':',1)[0].strip()==k),None) for k in ['vendor_id','cpu family','model','stepping','microcode']},
  physical_cpus=selected,workers=len(selected),note='Guest-reported topology; one CPU per visible physical core, bounded by CPU quota. Host mapping may be virtualized.')
def setup():
 manifest=json.loads((BUNDLE/'bundle_manifest.json').read_text())
 for name,h in manifest.items():assert sha(BUNDLE/name)==h,'Bundle mismatch: '+name
 cpus,host=physical_cpus();WORK.mkdir(parents=True,exist_ok=True)
 identity={'bundle_manifest':sha(BUNDLE/'bundle_manifest.json'),'provenance':sha(BUNDLE/'provenance.json')}
 frozen=WORK/'campaign-input-identity.json'
 if frozen.exists():assert json.loads(frozen.read_text())==identity,'Campaign inputs/tooling changed; retain old evidence and start a separate attempt directory'
 else:save(frozen,identity)
 old=WORK/'host.json'
 if old.exists():assert json.loads(old.read_text())==host,'Host/affinity changed; start a new measurement campaign'
 else:save(old,host)
 os.environ.update(P1_CAMPAIGN_CPUS=','.join(map(str,cpus)),P1_CAMPAIGN_ROOT=str(WORK),P1_CAMPAIGN_OUTPUT=str(WORK/'results'))
 for name in ['references','frozen_reports','historical_harness']:
  target=WORK/name
  if not target.exists():target.symlink_to(BUNDLE/name,target_is_directory=True)
 paths=json.loads((BUNDLE/'input_paths.json').read_text());rows=json.loads((BUNDLE/'original_inputs.json').read_text())
 for r in rows:
  for k,v in list(r.items()):
   if isinstance(v,str) and v in paths:r[k]=str(BUNDLE/paths[v])
 save(WORK/'inputs.json',rows)
 common=(BUNDLE/'support/common_original.py').read_text()
 common=re.sub(r"^ROOT=.*$",'ROOT=Path('+repr(str(WORK))+')',common,flags=re.M)
 common=re.sub(r"^OUT=.*$",'OUT=ROOT/\'results\'',common,flags=re.M)
 common=re.sub(r'^CPUS=.*$','CPUS='+repr(cpus),common,flags=re.M)
 common=common.replace("def env(framework='p1',workers=6):","def env(framework='p1',workers=6):\n    workers=min(workers,len(CPUS))")
 common=common.replace('resource.setrlimit(resource.RLIMIT_STACK,(resource.RLIM_INFINITY,resource.RLIM_INFINITY))',
  'resource.setrlimit(resource.RLIMIT_STACK,(resource.getrlimit(resource.RLIMIT_STACK)[1],resource.getrlimit(resource.RLIMIT_STACK)[1]))')
 (WORK/'support').mkdir(exist_ok=True);(WORK/'support/common.py').write_text(common)
 sys.path.insert(0,str(Path(__file__).parent));import runner as R
 R.M=R.bootstrap();R.git=lambda *args:json.dumps({'exported_provenance':sha(BUNDLE/'provenance.json'),'query':args})
 original=R.binary_cmd
 def command(v,a,row,validation,workers,output):
  cmd,e,cwd=original(v,a,row,validation,min(workers,len(cpus)),output)
  if v.startswith('graptor'):cmd[cmd.index('-c')+1]='8'
  return cmd,e,cwd
 R.binary_cmd=command
 return R,cpus
def graptor(R):
 src=BUNDLE/'graptor_source';out=WORK/'graptor';out.mkdir(exist_ok=True)
 flags=['g++','-fopenmp','-O3','-DNDEBUG','-std=c++23','-DGRAPHTYPE=VEBOGraptorPullDataParCached',
  '-O4','-Wno-ignored-attributes','-ftemplate-backtrace-limit=0','-g','-DLONG','-UNDEBUG','-march=native',
  '-DMAX_VL=8','-I/usr/local/include','-L/usr/local/lib','-Wl,-rpath,/usr/local/lib',
  '-I'+str(src),'-I'+str(src/'include'),'-I'+str(src/'bench')]
 constants=out/'constants.o'
 if not constants.exists():R.checked([*flags,'-c',src/'src/constants.C','-o',constants],out,out/'constants.log')
 for name,file in [('BFSExport','BFSvExport.C'),('BFExport','BFvExport.C'),('CCv_VEBOGraptorPullDataParCached','CCv.C'),('PRsgpl','PRsgpl.C'),('PRdouble','PRdouble.C')]:
  R.progress(stage='build_graptor',item=name)
  if not (out/name).exists():R.checked([*flags,src/'bench'/file,constants,'-lconfig++','-o',out/name],out,out/(name+'.build.log'))
 (WORK/'timer.c').write_text('#include <time.h>\ndouble __wrap_timer_runtime(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}\n')
 R.checked(['gcc','-O2','-c',WORK/'timer.c','-o',WORK/'monotonic_timer.o'],WORK,WORK/'timer.build.log')
 (WORK/'gclock.c').write_text('#include <sys/time.h>\n#include <time.h>\nint gettimeofday(struct timeval *t,void *z){struct timespec s;int r=clock_gettime(CLOCK_MONOTONIC,&s);t->tv_sec=s.tv_sec;t->tv_usec=s.tv_nsec/1000;return r;}\n')
 R.checked(['gcc','-O2','-shared','-fPIC',WORK/'gclock.c','-o',WORK/'gclock.so'],WORK,WORK/'gclock.build.log')
 shutil.copy2(BUNDLE/'walltime.c',WORK/'walltime.c')
def build(R):
 graptor(R)
 versions=([('OriginB0','origin_b0')] if (BUNDLE/'origin_b0').exists() else [])+[('Accepted','accepted'),('StaticSchedule','candidate')]
 for v,source in versions:
  R.REPO=BUNDLE/source;R.build(v)
 inputs={str(BUNDLE/name):digest for name,digest in json.loads((BUNDLE/'bundle_manifest.json').read_text()).items()}
 save(R.OUT/'manifest.json',dict(inputs=inputs,cases=R.M.ROWS,dataset_hash=sha(BUNDLE/'input_paths.json'),
  graptor={p.name:sha(p) for p in (WORK/'graptor').iterdir() if p.is_file() and '.log' not in p.name},
  support_hashes={'support/common.py':sha(WORK/'support/common.py')},physical_cpus=json.loads((WORK/'host.json').read_text()),
  provenance=json.loads((BUNDLE/'provenance.json').read_text()),preserve_changes_on_regression=True))
def specs(a):
 s=[('Accepted','accepted',{}),('StaticSchedule','candidate',{}),('Accepted','AA',{}),
  ('StaticSchedule','disabled',{'SGPL_NO_STATIC_SCHEDULE':'1'}),
  ('graptor_double' if a=='pagerank' else 'graptor','graptor',{'LD_PRELOAD':str(WORK/'gclock.so')})]
 if a=='pagerank':s.append(('graptor','graptor_float',{'LD_PRELOAD':str(WORK/'gclock.so')}))
 return s
def validate(R,cpus):
 R.verify_inputs()
 env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,sorted(set([1,min(2,len(cpus)),len(cpus)]))))
 src=WORK/'versions/StaticSchedule/src'
 if not (R.OUT/'contracts_passed.json').exists():
  R.checked(['bash','test/run_static_schedule_tests.sh'],src,R.OUT/'runtime_compiler_contracts.log',env)
  save(R.OUT/'campaign_state.json',{'last_accepted_version':'Accepted'})
  R.checked([sys.executable,'test/check_fullgraph.py'],src,R.OUT/'frontend_contracts.log',env)
  save(R.OUT/'contracts_passed.json',dict(passed=True,bundle_manifest=sha(BUNDLE/'bundle_manifest.json')))
 exact=[]
 for w in sorted(set([1,min(2,len(cpus)),len(cpus)])):
  for a in ALGS:
   for i,row in enumerate(R.M.ROWS):
    for v,label,extra in specs(a):
     if label=='AA':continue
     x=R.invoke('static_schedule','validate',v,a,row,0,label=label,workers=w,extra=extra)
     if x.get('error'):raise RuntimeError('Correctness failure; retained: '+str(x['base']))
    if a=='pagerank':
     import numpy as np
     vectors=[]
     for v,label,extra in [x for x in specs(a) if x[1] in ['accepted','candidate','disabled']]:
      x=R.invoke('static_schedule','validate',v,a,row,0,label=label,workers=w,extra=extra)
      text=Path(x['base']+'.out').read_text().split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0]
      vectors.append(np.array(text.split(),dtype=np.float64).tobytes())
     assert all(v==vectors[0] for v in vectors),'Exact PageRank vector changed: '+row['graph']
     exact.append(dict(graph=row['graph'],workers=w,sha256=hashlib.sha256(vectors[0]).hexdigest(),passed=True))
    R.progress(stage='validate',workers=w,algorithm=a,completed=i+1,total=61)
 save(R.OUT/'exact_pagerank_vectors.json',exact)
 save(R.OUT/'validated.json',dict(fingerprints={v:R.verify_fingerprint(v) for v in ['Accepted','StaticSchedule']},workers=len(cpus)))
def benchmark(R,cpus,phase,reps):
 valid=R.read(R.OUT/'validated.json');assert valid,'Validate all cases first'
 for v,h in valid['fingerprints'].items():assert R.verify_fingerprint(v)==h
 R.verify_inputs();w=len(cpus);observations=[]
 for a in ALGS:
  start=time.monotonic()
  for i,row in enumerate(R.M.ROWS):
   for v,label,extra in specs(a):
    x=R.invoke('static_schedule','warmup_'+phase,v,a,row,0,label=label,workers=w,extra=extra)
    if x.get('error'):raise RuntimeError('Warmup failed: '+x['base'])
   for rep in range(reps):
    choices=specs(a)
    for v,label,extra in choices if rep%2==0 else choices[::-1]:
     x=R.invoke('static_schedule',phase,v,a,row,rep,label=label,workers=w,extra=extra)
     observations.append(x)
     if x.get('error') or x.get('seconds',0)<=0:raise RuntimeError('Measurement failed: '+x['base'])
   save(R.OUT/(phase+'.json'),observations)
   R.progress(stage=phase,algorithm=a,completed=i+1,total=61,failures=0)
  save(R.OUT/(phase+'_'+a+'_duration.json'),dict(seconds=time.monotonic()-start,repetitions=reps,workers=w,cases=61))
def permutation_pvalue(d,two=False):
 import numpy as np
 from scipy.stats import permutation_test
 if np.all(d==0):return 1.0
 # Vectorize across resamples. The seed, sign permutations, arithmetic mean,
 # tail and sample count are unchanged; avoid 100,000 Python calls per test.
 return float(permutation_test((d,),lambda z,axis=-1:np.mean(z,axis=axis),
  permutation_type='samples',alternative='two-sided' if two else 'greater',
  n_resamples=100000,random_state=np.random.default_rng(2358)).pvalue)

def report(R):
 import numpy as np
 def holm(tests):
  running=0
  for rank,t in enumerate(sorted(tests,key=lambda x:x['p'])):
   running=max(running,min(1,t['p']*(len(tests)-rank)));t['holm_p']=running
 def test(x,y,two=False):
  d=np.log(np.asarray(y)/np.asarray(x))
  return permutation_pvalue(d,two)
 results=[];summary={};all_tests=[]
 for phase in ['primary','confirmation','primary_retry1','confirmation_retry1']:
  records=R.read(R.OUT/(phase+'.json'),[])
  if not records:continue
  phase_rows=[];tests=[];aa=[];alg_diffs={a:[] for a in ALGS};alg_aa={a:[] for a in ALGS}
  for a in ALGS:
   for row in R.M.ROWS:
    items=[x for x in records if x['algorithm']==a and x['graph']==row['graph']]
    labels=['accepted','candidate','AA','disabled','graptor']+(['graptor_float'] if a=='pagerank' else [])
    groups={label:sorted([x for x in items if x['label']==label],key=lambda x:x['repeat']) for label in labels}
    expected=5 if phase.startswith('primary') else 20
    if any(len(xs)!=expected for xs in groups.values()):raise RuntimeError('Incomplete measurement table: '+a+' '+row['graph'])
    x=[z['seconds'] for z in groups['accepted']];y=[z['seconds'] for z in groups['candidate']];z=[q['seconds'] for q in groups['AA']]
    t=dict(phase=phase,algorithm=a,graph=row['graph'],p=test(x,y),kind='case');tests.append(t)
    aa.append(dict(phase=phase,algorithm=a,graph=row['graph'],p=test(x,z,True),kind='case'))
    alg_diffs[a].append(np.log(np.asarray(y)/x));alg_aa[a].append(np.log(np.asarray(z)/x))
    result=dict(phase=phase,algorithm=a,graph=row['graph'],cohort=row['cohort'],test=t,
     mean_ms={k:statistics.mean(z['seconds'] for z in xs)*1000 for k,xs in groups.items()},
     median_ms={k:statistics.median(z['seconds'] for z in xs)*1000 for k,xs in groups.items()},
     kernel_mean_ms={k:statistics.mean(z['internal_seconds'] for z in xs)*1000 if all('internal_seconds' in z for z in xs) else None for k,xs in groups.items()})
    result['change_percent']=(result['mean_ms']['candidate']/result['mean_ms']['accepted']-1)*100
    phase_rows.append(result)
  for a in ALGS:
   for diffs,target,two in [(alg_diffs,tests,False),(alg_aa,aa,True)]:
    d=np.mean(diffs[a],axis=0)
    p=permutation_pvalue(d,two)
    target.append(dict(phase=phase,algorithm=a,kind='algorithm',p=p))
  holm(tests);holm(aa)
  stable=not any(x['holm_p']<.05 for x in aa)
  for x in phase_rows:x['confirmed_regression']=stable and x['test']['holm_p']<.05
  summary[phase]=dict(aa_failures=[x for x in aa if x['holm_p']<.05],tests=tests,aa_tests=aa,
   valid_for_inference=stable,
   mean_geomean_speedup=math.exp(statistics.mean(math.log(x['mean_ms']['accepted']/x['mean_ms']['candidate']) for x in phase_rows)),
   median_geomean_speedup=math.exp(statistics.mean(math.log(x['median_ms']['accepted']/x['median_ms']['candidate']) for x in phase_rows)),
   confirmed_case_regressions=sum(x['confirmed_regression'] for x in phase_rows),candidate_retained=True,
   by_algorithm={a:dict(mean_geomean_speedup=math.exp(statistics.mean(math.log(x['mean_ms']['accepted']/x['mean_ms']['candidate']) for x in phase_rows if x['algorithm']==a)),
    accepted_total_ms=sum(x['mean_ms']['accepted'] for x in phase_rows if x['algorithm']==a),
    candidate_total_ms=sum(x['mean_ms']['candidate'] for x in phase_rows if x['algorithm']==a),
    graptor_total_ms=sum(x['mean_ms']['graptor'] for x in phase_rows if x['algorithm']==a),
    raw_slower_cases=sum(x['change_percent']>0 for x in phase_rows if x['algorithm']==a),
    confirmed_slower_cases=sum(x['confirmed_regression'] for x in phase_rows if x['algorithm']==a)) for a in ALGS})
  main50=[x for x in phase_rows if x['cohort']=='main50']
  summary[phase]['original50_mean_geomean_speedup']=math.exp(statistics.mean(math.log(x['mean_ms']['accepted']/x['mean_ms']['candidate']) for x in main50))
  unique={}
  for x in phase_rows:
   row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   identity=(x['algorithm'],Path(row['bin']).name,row['p1_directed'],row['source'] if x['algorithm'] in ['bfs','sssp'] else None,
    Path(row['weights']).name if x['algorithm']=='sssp' else None)
   unique.setdefault(identity,[]).append(math.log(x['mean_ms']['accepted']/x['mean_ms']['candidate']))
  summary[phase]['duplicate_content_sensitivity']=dict(unique_cases=len(unique),mean_geomean_speedup=math.exp(statistics.mean(statistics.mean(v) for v in unique.values())))
  results+=phase_rows
 save(R.OUT/'comparison.json',dict(summary=summary,cases=results,host=R.read(WORK/'host.json')))
 body=['<!doctype html><meta charset="utf-8"><title>P1 static schedules benchmark</title><style>body{font:15px system-ui;margin:24px}table{border-collapse:collapse}td,th{padding:6px;border:1px solid #bbb;text-align:right}td:first-child{text-align:left}.regress{background:#ffcccc}</style>',
  '<h1>P1 effect-derived static schedules</h1><p>Whole-process monotonic times; milliseconds. Positive change means slower. Red rows have Holm-corrected confirmed regressions. Kernel time is reported separately. All changes are retained.</p>',
  '<pre>'+html.escape(json.dumps({k:{a:b for a,b in v.items() if a not in ['tests','aa_tests']} for k,v in summary.items()},indent=2))+'</pre>']
 for phase in summary:
  for a in ALGS:
   body+=['<h2>'+html.escape(phase+' / '+a)+'</h2><table><tr><th>Graph</th><th>Accepted ms</th><th>Candidate ms</th><th>Change %</th><th>Disabled ms</th><th>Graptor ms (PR double)</th><th>Graptor PR float ms</th><th>Accepted kernel ms</th><th>Candidate kernel ms</th><th>Graptor kernel ms</th></tr>']
   for x in results:
    if x['phase']!=phase or x['algorithm']!=a:continue
    cells=[html.escape(x['graph'])]+[f'{x["mean_ms"][k]:.6f}' for k in ['accepted','candidate']]+[f'{x["change_percent"]:+.2f}']+[f'{x["mean_ms"][k]:.6f}' for k in ['disabled','graptor']]+([f'{x["mean_ms"]["graptor_float"]:.6f}'] if a=='pagerank' else ['n/a'])+[f'{x["kernel_mean_ms"][k]:.6f}' if x['kernel_mean_ms'][k] is not None else 'missing' for k in ['accepted','candidate','graptor']]
    body.append('<tr'+(' class="regress"' if x['confirmed_regression'] else '')+'>'+''.join('<td>'+c+'</td>' for c in cells)+'</tr>')
   body.append('</table>')
 (R.OUT/'comparison.html').write_text('\n'.join(body))
 print(json.dumps(summary,indent=2),flush=True)
 return summary
def main():
 p=argparse.ArgumentParser();p.add_argument('stage',choices=['preflight','build','validate','primary','confirmation','report','all']);a=p.parse_args()
 R,cpus=setup()
 with R.locked('portable_'+a.stage):
  if a.stage=='preflight':print(json.dumps(R.read(WORK/'host.json'),indent=2));return
  for stage in (['build','validate','primary','confirmation','report'] if a.stage=='all' else [a.stage]):
   if stage=='build':build(R)
   elif stage=='validate':validate(R,cpus)
   elif stage in ['primary','confirmation']:benchmark(R,cpus,stage,5 if stage=='primary' else 20)
   else:
    summary=report(R)
    for phase in ['primary','confirmation']:
     if phase in summary and not summary[phase]['valid_for_inference'] and phase+'_retry1' not in summary:
      benchmark(R,cpus,phase+'_retry1',5 if phase=='primary' else 20)
    if any(not x['valid_for_inference'] for x in summary.values()):report(R)
if __name__=='__main__':main()
