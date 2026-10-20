"""Read-only accepted-binary investigation; results are separate from campaign gates.

WSL venv Python kernel_gap.py audit|measure|controls|phases
Native variants are diagnostics, never promotion measurements.
"""
import os
os.environ['P1_CAMPAIGN_OUTPUT']='/mnt/c/p1kernel-investigation-20261020'
import runner as R
from pathlib import Path
import json,statistics,sys,time,math,shutil

OLD=Path('/mnt/c/p1campaign-results')
ATTEMPT='kernel_gap_c8_20261020'
V='R4a3'
R.M=R.bootstrap()
_binary_cmd=R.binary_cmd
def pinned_command(v,a,r,validation,workers,output):
 cmd,e,cwd=_binary_cmd(v,a,r,validation,workers,output)
 # Partitions are a layout parameter, independent of OpenMP worker count.
 # Keep the campaign's eight partitions at 1/2/6 workers. P=1 asserts in VEBO.
 if v.startswith('graptor'):cmd[cmd.index('-c')+1]='8'
 return cmd,e,cwd
R.binary_cmd=pinned_command
def mean(xs):return statistics.mean(xs)
def audit():
 if not (R.OUT/'manifest.json').exists():shutil.copy2(OLD/'manifest.json',R.OUT/'manifest.json')
 R.verify_inputs()
 final=R.read(OLD/'FinalComparison_summary.json')['per_case']
 profiles=R.read(OLD/'R4a3_profile_phases_w6_c8718c4e.json')
 cases=[];selection=[]
 for x in final:
  a,g=x['algorithm'],x['graph'];p=[p for p in profiles if p['version']==V and p['algorithm']==a and p['graph']==g]
  assert len(p)==5
  phases={k:mean(y['phases'][k]['ms'] for y in p) for k in p[0]['phases']}
  internal=mean(y['internal_seconds'] for y in p)*1000
  # Disjoint main-thread regions. Source-map is nested in dispatch/begin;
  # compaction is nested in finish. Never add either again.
  accounted=sum(phases[k] for k in ['records','dispatch_including_execution','begin','finish'])
  cases.append(dict(algorithm=a,graph=g,native_ms=x['internal_ms']['candidate'],graptor_ms=x['internal_ms']['graptor_double' if a=='pagerank' else 'graptor'],profile_ms=internal,phases_ms=phases,unaccounted_profile_ms=internal-accounted,work=p[0]['work'],full_domain_work=p[0]['full_domain_work'],record_bytes=p[0]['record_bytes'],pool_main_wait_ms=mean(sum(z['wait_ms'] for z in y['pool'] if z['lane']==0) for y in p)))
 for a in R.ALGS:
  c=[x for x in cases if x['algorithm']==a]
  c.sort(key=lambda x:x['native_ms']/x['graptor_ms'])
  for role,x in [('median_gap',c[len(c)//2]),('largest_gap',c[-1]),('largest_p1_time',max(c,key=lambda x:x['native_ms']))]:
   if not any(y['algorithm']==a and y['graph']==x['graph'] for y in selection):selection.append(dict(algorithm=a,graph=x['graph'],role=role))
 # Road CC and representative PR/BFS isolate known mechanism families.
 for a,g in [('cc','dimacs10/inf-luxembourg_osm.edges'),('pagerank','dimacs10/inf-luxembourg_osm.edges'),('bfs','dimacs/hamming8-2.edges'),('sssp','dimacs/DSJC1000-5.edges')]:
  assert any(x['algorithm']==a and x['graph']==g for x in cases),(a,g)
  if not any(y['algorithm']==a and y['graph']==g for y in selection):selection.append(dict(algorithm=a,graph=g,role='mechanism_control'))
 R.save(R.OUT/'phase_decomposition.json',cases);R.save(R.OUT/'selected_cases.json',selection)
 identity=R.read(R.OUT/'investigation_manifest.json',dict(accepted_commit=R.git('rev-parse','HEAD'),accepted_version=V,accepted_fingerprint=R.verify_fingerprint(V),physical_cpus=R.CPUS,campaign_manifest_sha256=R.sha(OLD/'manifest.json'),selection=selection,created=time.time(),scope='diagnostic; does not modify sealed campaign or promote changes'))
 assert identity['selection']==selection and identity['accepted_fingerprint']==R.verify_fingerprint(V)
 identity['accepted_native_source_commit']=R.read(R.ROOT/'versions'/V/'identity.json')['commit'];R.save(R.OUT/'investigation_manifest.json',identity)
 for a in R.ALGS:
  c=[x for x in cases if x['algorithm']==a]
  print(a,json.dumps(dict(native_sum_ms=sum(x['native_ms'] for x in c),graptor_sum_ms=sum(x['graptor_ms'] for x in c),profile_sum_ms=sum(x['profile_ms'] for x in c),phase_sums_ms={k:sum(x['phases_ms'][k] for x in c) for k in c[0]['phases_ms']},unaccounted_sum_ms=sum(x['unaccounted_profile_ms'] for x in c))))
 print('SELECTION',json.dumps(selection),flush=True)

def run(mode):
 R.verify_inputs();R.verify_fingerprint(V)
 selected=([dict(algorithm=a,graph=r['graph'],role='full_dataset') for a in R.ALGS for r in R.M.ROWS] if mode=='full_mono' else R.read(R.OUT/'selected_cases.json'));records=[]
 if mode=='phases':R.PROFILE_IDS[V]='e731003a4d9b393f'
 for a in R.ALGS:
  start=time.time();block=[x for x in selected if x['algorithm']==a]
  for i,x in enumerate(block):
   row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   if mode.startswith('measure') or mode=='full_mono':
    if mode in ['measure_mono','full_mono']:
     lib=Path('/home/kazi/p1_kernel_20261020/monotonic_gettimeofday.so');assert lib.exists()
    specs=[(v,{'LD_PRELOAD':str(lib)} if mode in ['measure_mono','full_mono'] and v.startswith('graptor') else {},v,w) for w in ([6] if mode=='full_mono' else [1,2,6]) for v in [V,'graptor_double' if a=='pagerank' else 'graptor']]
   elif mode=='controls':
    specs=[(V,e,n,6) for n,e in [('default',{}),('no_warmup_calibration',{'SGPL_NO_WARMUP_CALIBRATION':'1'}),('no_spin',{'SGPL_POOL_SPIN_US':'0'}),('long_spin',{'SGPL_POOL_SPIN_US':'1000'}),('no_lean',{'SGPL_NO_LEAN_RECORDS':'1'}),('no_fullgraph',{'SGPL_NO_FULLGRAPH':'1'})] if (n!='no_fullgraph' or a=='pagerank') and (n!='no_lean' or a!='pagerank')]
   else:specs=[(V,{},'profile',w) for w in [1,2,6]]
   for v,e,n,w in specs:
    valid=R.invoke(ATTEMPT,'validate_'+mode,v,a,row,0,label=n,workers=w,extra=e,profile='phases' if mode=='phases' else None)
    assert valid.get('validation') and not valid.get('error'),valid
    if mode!='phases':R.invoke(ATTEMPT,'warmup_'+mode,v,a,row,0,label=n,workers=w,extra=e)
   for rep in range(5 if mode in ['phases','full_mono'] else 20):
    for v,e,n,w in (specs if rep%2==0 else list(reversed(specs))):
     z=R.invoke(ATTEMPT,mode,v,a,row,rep,label=n,workers=w,extra=e,profile='phases' if mode=='phases' else None);records.append(z)
     assert not z.get('error'),z
   R.save(R.OUT/(mode+'.json'),records)
   R.progress(stage=mode,algorithm=a,graph=row['graph'],completed=i+1,total=len(block))
  R.calibrate(a,mode,[selected,20 if mode!='phases' else 5],start)
 summary=[]
 for x in selected:
  p=[z for z in records if z['algorithm']==x['algorithm'] and z['graph']==x['graph']]
  for n,w in sorted(set((z['label'],z['workers']) for z in p)):
   q=[z for z in p if z['label']==n and z['workers']==w]
   summary.append(dict(**x,label=n,workers=w,samples=len(q),internal_ms=mean(z['internal_seconds'] for z in q)*1000,internal_median_ms=statistics.median(z['internal_seconds'] for z in q)*1000,whole_ms=mean(z['seconds'] for z in q)*1000,cpu_ms=mean(z['user_seconds']+z['system_seconds'] for z in q)*1000,nvcsw=mean(z['nvcsw'] for z in q)))
 R.save(R.OUT/(mode+'_summary.json'),summary)

if __name__=='__main__':
 with R.locked('kernel_gap_'+sys.argv[1]):
  if sys.argv[1]=='audit':audit()
  else:run(sys.argv[1])
