#!/usr/bin/env python3
"""Prepare size ranks 101..200 without changing the completed first batch."""
import csv,json,sys
from pathlib import Path
import numpy as np
from prepare_e2e150 import prepare,sha

S=Path('/media/readinggroup/Storage/splash');B=S/'bench';R=B/'e2e150'
D=S/'original_graphs_101_200';D.mkdir(exist_ok=True)
catalog=list(csv.DictReader(open(B/'undirected_prepared_graphs.csv')))
for r in catalog:
    h=np.fromfile(S/r['undirected_bin_path'],dtype='<u8',count=8)
    r['_n'],r['_m']=int(h[2]),int(h[3])
catalog.sort(key=lambda r:(r['_m'],r['original_path']))
previous=json.loads((R/'results/run100_r5/protocol.json').read_text())
assert previous['graphs']==[r['original_path'] for r in catalog[:100]],'Original run selection differs'
old={r['graph']:r for r in csv.DictReader(open(S/'original_graphs_150/graphs150.csv'))}
fresh=[]
for rank,r in enumerate(catalog[150:200],151):
    n,m=r['_n'],r['_m'];p=S/r['undirected_bin_path']
    idx=np.fromfile(p,dtype='<u8',count=n,offset=64)
    degrees=np.diff(np.append(idx,np.uint64(m)))
    source=int(np.flatnonzero(degrees)[0])
    fully=r['storage_fully_reciprocal']=='yes'
    fresh.append({'rank':str(rank),'graph':r['original_path'],'vertices':str(n),
        'arcs_both_directions':str(m),'undirected_edges':str(m//2),
        'original_file':str(S/r['source_path']),
        'original_storage':'both directions' if fully else 'one direction per undirected edge',
        'p1_directed':'true' if fully else 'false','source':str(source),
        'source_policy':'smallest vertex with nonzero degree',
        'weighted':str(r['mode']!='unweighted'),'sssp_eligible':'True',
        **{key:str(S/r[column]) if r[column] else '' for key,column in
           [('el','undirected_el_path'),('adj','undirected_adj_path'),('bin','undirected_bin_path'),
            ('wel','undirected_wel_path'),('wadj','undirected_wadj_path'),('weights','undirected_weights_path')]}})
raw_manifest=D/'new50_raw.csv'
with open(raw_manifest,'w',newline='') as f:
    w=csv.DictWriter(f,fieldnames=list(fresh[0]));w.writeheader();w.writerows(fresh)
new_manifest=prepare(S,raw_manifest,D,2358,151)
new={r['graph']:r for r in csv.DictReader(open(new_manifest))}
rows=[]
for rank,r in enumerate(catalog[100:200],101):
    g=r['original_path'];row=dict(old[g] if g in old else new[g]);row['rank']=str(rank);rows.append(row)
assert len(rows)==100 and not set(previous['graphs'])&{r['graph'] for r in rows}
assert [int(r['rank']) for r in rows]==list(range(101,201))
manifest=D/'graphs101_200.csv';fields=list(dict.fromkeys(k for r in rows for k in r))
with open(manifest,'w',newline='') as f:
    w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
metadata={'count':100,'first_rank':101,'last_rank':200,'first_graph':rows[0]['graph'],
    'last_graph':rows[-1]['graph'],'disjoint_from_previous_run':True,
    'reused_prepared_graphs':50,'newly_prepared_graphs':50,
    'manifest_sha256':sha(manifest),'manifest':str(manifest),
    'weights':'same deterministic canonical-pair positive 1..255 weights; seed 2358'}
(D/'batch101_200.json').write_text(json.dumps(metadata,indent=2)+'\n')
print('BATCH_READY',json.dumps(metadata),flush=True)
