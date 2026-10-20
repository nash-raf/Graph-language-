"""Recompute investigation tables, paired diagnostics, PMU attribution and audit."""
import kernel_gap as K
R=K.R
from pathlib import Path
import math,re,statistics,subprocess,bisect,sys,struct,hashlib
import numpy as np
W=Path('/home/kazi/p1_kernel_20261020')
def mean(xs):return statistics.mean(xs)
def gm(xs):return math.exp(mean(math.log(x) for x in xs))
def paired(records,phase,base,other):
 result=[]
 for a,g in sorted(set((x['algorithm'],x['graph']) for x in records if x['phase']==phase)):
  groups={n:{x['repeat']:x['internal_seconds'] for x in records if x['phase']==phase and x['algorithm']==a and x['graph']==g and x['label']==n} for n in [base,other]}
  reps=sorted(groups[base].keys()&groups[other].keys())
  if not reps:continue
  b=np.array([groups[base][i] for i in reps]);c=np.array([groups[other][i] for i in reps]);assert (b>0).all() and (c>0).all()
  logs=np.log(c/b);rng=np.random.default_rng(20261020)
  null=np.mean(rng.choice([-1,1],size=(100000,len(reps)))*logs,axis=1)
  boot=rng.integers(len(reps),size=(20000,len(reps)));ci=np.exp(np.quantile(logs[boot].mean(axis=1),[.025,.975]))
  p=(np.sum(np.abs(null)>=abs(logs.mean()))+1)/100001
  result.append(dict(algorithm=a,graph=g,base=base,variant=other,pairs=len(reps),base_ms=float(b.mean()*1000),variant_ms=float(c.mean()*1000),ratio_of_means=float(c.mean()/b.mean()),paired_geomean_ratio=float(np.exp(logs.mean())),paired_log_bootstrap95=ci.tolist(),permutation_two_sided_p=float(p)))
 return result
def holm(rows):
 order=sorted(rows,key=lambda r:r['permutation_two_sided_p']);last=0
 for i,x in enumerate(order):
  last=max(last,min(1,x['permutation_two_sided_p']*(len(rows)-i)));x['holm_p']=last
 return rows
def stat(p,repeats):
 values={}
 for line in p.read_text().splitlines():
  f=line.split(',')
  if len(f)>2 and f[0].isdigit():values[f[2]]=int(f[0])/repeats
 return values
def symbols(folder,exe):
 text=(folder/'report.txt').read_text();lost=re.search(r'Total Lost Samples:\s*(\d+)',text)
 # Re-render without a cutoff so all sampled percentages can be accounted for.
 raw=subprocess.check_output(['/usr/lib/linux-tools/6.8.0-147-generic/perf','report','--stdio','--no-children','--call-graph','none','--sort','dso,symbol','--percent-limit','0','-i',str(folder/'perf.data')],text=True,stderr=subprocess.DEVNULL)
 spans=[]
 for line in subprocess.check_output(['nm','-n','-S','--defined-only',str(exe)],text=True).splitlines():
  m=re.match(r'([0-9a-f]+)\s+([0-9a-f]+)\s+[tTwW]\s+(.+)',line)
  if m:spans.append((int(m[1],16),int(m[2],16),m[3]))
 starts=[x[0] for x in spans];out={};unknown=0
 for line in raw.splitlines():
  m=re.match(r'\s*([\d.]+)%\s+(.*?)\s+\[([.k])\]\s+(.+?)\s*$',line)
  if not m:continue
  val=float(m[1]);symbol=re.sub(r'\s+-\s+-.*$','',m[4]).strip()
  if symbol.startswith('0x') and m[3]=='.':
   ip=int(symbol.split()[0],16);i=bisect.bisect_right(starts,ip)-1
   if i>=0 and ip<spans[i][0]+spans[i][1]:symbol=spans[i][2]
   else:unknown+=val
  out[symbol]=out.get(symbol,0)+val
 return dict(lost_samples=int(lost[1]) if lost else None,unmapped_user_percent=unknown,functions_percent=dict(sorted(out.items(),key=lambda x:-x[1])))
def run():
 full=R.read(R.OUT/'full_mono.json');assert len(full)==2440
 assert not any(x.get('error') or x.get('internal_seconds',0)<=0 for x in full)
 aggregate={}
 for a in R.ALGS:
  ratios=[];medians=[];p1=[];gr=[]
  for row in R.M.ROWS:
   x=[z for z in full if z['algorithm']==a and z['graph']==row['graph']]
   b=[z['internal_seconds'] for z in x if z['label']==K.V];g=[z['internal_seconds'] for z in x if z['label'].startswith('graptor')];assert len(b)==len(g)==5
   ratios.append(mean(b)/mean(g));medians.append(statistics.median(b)/statistics.median(g));p1.append(mean(b));gr.append(mean(g))
  aggregate[a]=dict(cases=61,kernel_p1_over_graptor_mean_geomean=gm(ratios),kernel_p1_over_graptor_median_geomean=gm(medians),kernel_p1_total_ms=sum(p1)*1000,kernel_graptor_total_ms=sum(gr)*1000)
 R.save(R.OUT/'monotonic_aggregate.json',aggregate)
 def code_sections(path):
  data=path.read_bytes();h=struct.unpack_from('<16sHHIQQQIHHHHHH',data);assert h[0][:5]==b'\x7fELF\x02'
  sections=[struct.unpack_from('<IIQQQQIIQQ',data,h[6]+i*h[11]) for i in range(h[12])];s=sections[h[13]];names=data[s[4]:s[4]+s[5]];out={}
  for s in sections:
   name=names[s[0]:].split(b'\0',1)[0].decode()
   if s[2]&4 and not name.startswith('.gap_'):out[name]=dict(address=s[3],size=s[5],sha256=hashlib.sha256(data[s[4]:s[4]+s[5]]).hexdigest())
  return out
 layouts=[]
 for p in (W/'late_opt/cases').rglob('timed.control'):
  code=[code_sections(p.with_name('timed.'+v)) for v in ['control','opt','alias']];assert all(x==code[0] for x in code),'Late optimization relocated or changed non-kernel code'
  layouts.append(dict(case=str(p.parent),non_kernel_executable_sections_identical=True,sections=code[0]))
 R.save(R.OUT/'late_opt_layout_audit.json',layouts)
 late={(x['graph'],x['workers']):x['sha256'] for x in R.read(R.OUT/'late_opt_bits.json')};pub=R.read(R.OUT/'publication_exact_bits.json');assert len(pub)==9 and all(late[x['graph'],x['workers']]==x['sha256'] for x in pub)
 R.save(R.OUT/'exact_control_audit.json',dict(graph_worker_groups=9,publication_variants=4,late_optimization_variants=3,all_match_accepted_native_full_vector_bits=True))
 tests=[];aa=[]
 for file,phase,base,variants in [('publication.json','publication','control',['prewarm','inline_publication','both','AA']),('late_opt.json','late_opt','control',['opt','alias','AA']),('cc_ablation.json','cc_ablation','pinned',['no_zero_no_fusion']),('controls.json','controls','default',['no_spin','long_spin','no_warmup_calibration','no_lean','no_fullgraph'])]:
  records=R.read(R.OUT/file)
  for other in variants:
   t=paired(records,phase,base,other)
   for x in t:x['experiment']=phase
   (aa if other=='AA' else tests).extend(t)
 records=R.read(R.OUT/'algorithm_controls.json')
 for base,other in [('pinned','sssp_snapshot'),('no_zero_no_fusion','cc_original_labels'),('cc_original_labels','cc_snapshot')]:
  t=paired(records,'algorithm_controls',base,other)
  for x in t:x['experiment']='algorithm_controls'
  tests.extend(t)
 records=R.read(R.OUT/'graptor_waiting.json')
 for other in ['passive','AA']:
  t=paired(records,'graptor_waiting','default',other)
  for x in t:x['experiment']='graptor_waiting'
  (aa if other=='AA' else tests).extend(t)
 holm(aa);holm(tests);R.save(R.OUT/'diagnostic_effects.json',dict(tests=tests,AA=aa,method='20 paired observations where available; fixed-seed 100000 sign permutations, two-sided, Holm within diagnostic-effects and AA families separately; paired log bootstrap CI; these are diagnostic tests, not promotion gates'))
 pmu=[]
 for x in R.read(R.OUT/'samples.json',[]):
  if x['kind']!='stat':continue
  a,g=x['algorithm'],x['graph'];p=Path(x['folder'])
  q=stat(p/'stat.csv',x['repeats'])
  ip=next(z for z in R.read(R.OUT/'samples_ip.json') if z['algorithm']==a and z['graph']==g);exe=W/'cases_v2'/a/g.removesuffix('.edges')/'scoped'
  native=symbols(Path(ip['folder']),exe)
  pmu.append(dict(algorithm=a,graph=g,p1_external_gated_counters=q,p1_sampled_functions=native,cross_framework_counter_comparison_valid=False,reason='External perf enable/ack and disable/ack insert idle spinning, especially on Graptor. Use continuous window instruction samples for cross-framework attribution.'))
 R.save(R.OUT/'pmu_native_analysis.json',pmu)
 historical=[]
 for line in (K.OLD/'FinalComparison_measurements.jsonl').read_text().splitlines():
  z=R.json.loads(line)
  if z.get('internal_seconds',1)<=0:historical.append({k:z.get(k) for k in ['version','algorithm','graph','repeat','internal_seconds','base']})
 R.save(R.OUT/'historical_internal_timer_audit.json',dict(nonpositive_final_campaign_records=historical,scope='Only nonpositive readings are detected; positive readings can still be biased by realtime clock adjustment. Original whole-process acceptance uses CLOCK_MONOTONIC and is unaffected.'))
 # Native code and inputs remain pinned; hash every private executable actually
 # referenced by a retained successful measurement, plus diagnostic source.
 hashes={};private=[]
 for file in ['measure_mono.json','controls.json','deep.json','publication.json','late_opt.json','full_mono.json','cc_ablation.json','vector_control.json','direct_pmu.json']:
  for z in R.read(R.OUT/file,[]):
   if z.get('executable_override'):private.append(z['executable_override'])
 for p in sorted(set(private)):
  if Path(p).is_file():hashes[p]=R.sha(p)
 helpers={str(p):R.sha(p) for p in [W/'window_samples/graptor.so',W/'window_samples/p1.o',W/'window_samples/marker.c',W/'monotonic_gettimeofday.so',W/'monotonic_gettimeofday.c']}
 R.save(R.OUT/'evidence_fingerprints.json',dict(accepted_fingerprint=R.verify_fingerprint(K.V),private_executables=hashes,helpers=helpers,scripts={p.name:R.sha(p) for p in R.HERE.glob('kernel_*.py')},manifest_sha256=R.sha(R.OUT/'manifest.json'),created=R.time.time()))
 for a,x in aggregate.items():print(a,round(x['kernel_p1_over_graptor_mean_geomean'],3))
 print('AA corrected failures',[(x['experiment'],x['graph'],x['holm_p']) for x in aa if x['holm_p']<.05])
 print('Native IP profiles:',len(pmu),'; cross-framework gated-counter ratios are rejected; use window_instruction_comparison.json')
if __name__=='__main__':
 with R.locked('kernel_report_analysis'):
  R.verify_inputs();run()
