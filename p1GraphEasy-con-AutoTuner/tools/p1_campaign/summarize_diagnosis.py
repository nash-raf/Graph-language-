"""Summarize independent diagnostic controls; never changes acceptance decisions."""
import argparse,collections,statistics
import numpy as np
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');args=p.parse_args()
rows=R.read(R.OUT/(args.version+'_diagnostic_controls.json'));rng=np.random.default_rng(2358);summaries=[]
groups=collections.defaultdict(list)
for x in rows:groups[(x['algorithm'],x['graph'],x['workers'])].append(x)
for (a,g,w),values in groups.items():
 arrays={label:np.array([x['seconds'] for x in sorted(values,key=lambda x:x['repeat']) if x['label']==label and not x.get('error')]) for label in ['baseline','candidate','disabled','aa']}
 entry=dict(algorithm=a,graph=g,workers=w,comparisons={})
 for numerator,denominator in [('candidate','baseline'),('disabled','baseline'),('aa','baseline'),('candidate','disabled')]:
  x=arrays[numerator];y=arrays[denominator]
  assert len(x)==len(y)==20
  index=rng.integers(0,20,(10000,20));ratios=x[index].mean(axis=1)/y[index].mean(axis=1)
  entry['comparisons'][numerator+'_over_'+denominator]={'ratio':float(x.mean()/y.mean()),'paired_bootstrap95':list(np.quantile(ratios,[.025,.975]))}
 summaries.append(entry)
R.save(R.OUT/(args.version+'_diagnostic_intervals.json'),summaries)
profiles=[]
for path in R.OUT.glob(args.version+'_profile_phases_w6_*.json'):
 for x in R.read(path):
  if x.get('extra'):continue
  profiles.append(x)
groups=collections.defaultdict(list)
for x in profiles:groups[(x['algorithm'],x['graph'],x['version'])].append(x)
phase=[]
for (a,g,v),values in groups.items():
 names=values[0].get('phases',{})
 item=dict(algorithm=a,graph=g,version=v,phases={n:{k:statistics.mean(x['phases'][n][k] for x in values) for k in ['ms','calls']} for n in names})
 item['work']={k:statistics.mean(x['work'][k] for x in values) for k in values[0].get('work',{})}
 item['pool']={k:statistics.mean(sum(r[k] for r in x.get('pool',[])) for x in values) for k in ['calls','park_calls','wait_ms','park_ms']}
 phase.append(item)
R.save(R.OUT/(args.version+'_diagnostic_phases.json'),phase)
print('Diagnostic intervals and phase summaries written for '+args.version)
