"""Synthetic full-matrix regression reporting and unstable-control contracts."""
from pathlib import Path
from types import SimpleNamespace
import contextlib,io,json,tempfile
import portable_campaign as P
with tempfile.TemporaryDirectory() as directory:
 d=Path(directory);P.WORK=d
 rows=[dict(graph=f'case{i}.edges',cohort='main50' if i<50 else 'historical_diagnostic',bin=f'{i}.bin',p1_directed='false',source=0,weights=f'{i}.weights') for i in range(61)]
 records=[]
 for a in P.ALGS:
  for i,row in enumerate(rows):
   for label in ['accepted','candidate','AA','disabled','graptor']+(['graptor_float'] if a=='pagerank' else []):
    for rep in range(20):
     seconds=1.2 if (a=='bfs' and i==0 and label=='candidate') else 1.0
     records.append(dict(algorithm=a,graph=row['graph'],label=label,repeat=rep,seconds=seconds,internal_seconds=seconds*.1))
 P.save(d/'confirmation.json',records);P.save(d/'host.json',dict(test='synthetic, no measurements'))
 def read(p,default=None):return json.loads(Path(p).read_text()) if Path(p).exists() else default
 R=SimpleNamespace(OUT=d,M=SimpleNamespace(ROWS=rows),read=read)
 with contextlib.redirect_stdout(io.StringIO()):summary=P.report(R)
 cases=read(d/'comparison.json')['cases']
 assert len(cases)==244 and sum(x['confirmed_regression'] for x in cases)==1
 assert summary['confirmation']['valid_for_inference']
 assert summary['confirmation']['by_algorithm']['bfs']['confirmed_slower_cases']==1
 assert (d/'comparison.html').read_text().count('<tr')==248
 # An identical-binary control slowdown invalidates the block; it must not be
 # reported as a confirmed candidate regression or as a candidate victory.
 for x in records:
  if x['algorithm']=='bfs' and x['graph']=='case0.edges' and x['label']=='AA':x['seconds']=1.3
 P.save(d/'confirmation.json',records)
 with contextlib.redirect_stdout(io.StringIO()):summary=P.report(R)
 assert not summary['confirmation']['valid_for_inference']
 assert summary['confirmation']['confirmed_case_regressions']==0
 assert summary['confirmation']['aa_failures']
 print('PASS synthetic 244-case report, Holm regression, complete HTML table and A/A invalidation')
