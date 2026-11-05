#!/usr/bin/env python3
"""Summarize all retained samples without excluding slow cases or outliers."""
from pathlib import Path
import json,math,statistics
root=Path('/home/kazi/p1_layout_native_20261105/benchmarks/native07')
rows=json.loads((root/'comparison.json').read_text())
samples=json.loads((root/'samples.json').read_text())
assert len(rows)==244 and len(samples)==2440
summary=[]
for name in ['bfs','sssp','cc','pagerank','all']:
    group=[r for r in rows if name=='all' or r['algorithm']==name]
    b=sum(r['baseline_ms'] for r in group);c=sum(r['candidate_ms'] for r in group)
    summary.append(dict(algorithm=name,cases=len(group),baseline_sum_medians_ms=b,
                       candidate_sum_medians_ms=c,aggregate_speedup=b/c,
                       geomean_speedup=math.exp(statistics.mean(math.log(r['speedup']) for r in group)),
                       faster_over_5pct=sum(r['speedup']>1/.95 for r in group),
                       slower_over_5pct=sum(r['speedup']<1/1.05 for r in group)))
variability=[]
for r in rows:
    for version in ['baseline','native07']:
        x=[s['wall_ns'] for s in samples if s['algorithm']==r['algorithm'] and s['graph']==r['graph'] and s['version']==version]
        assert len(x)==5
        med=statistics.median(x)
        variability.append(dict(algorithm=r['algorithm'],graph=r['graph'],version=version,
                                min_ms=min(x)/1e6,median_ms=med/1e6,max_ms=max(x)/1e6,
                                relative_mad=statistics.median(abs(y-med) for y in x)/med))
result=dict(groups=summary,largest_slowdowns=sorted(rows,key=lambda r:r['speedup'])[:10],
            largest_speedups=sorted(rows,key=lambda r:r['speedup'],reverse=True)[:10],
            median_relative_sample_mad=statistics.median(r['relative_mad'] for r in variability),
            caveat='Five warm process samples per version; small differences are not established significance. No samples excluded.')
(root/'analysis.json').write_text(json.dumps(result,indent=2))
(root/'variability.json').write_text(json.dumps(variability,indent=2))
print(json.dumps(result,indent=2))
