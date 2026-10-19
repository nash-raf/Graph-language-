"""Check full-domain counters against frozen graph sizes and PR program rounds."""
import argparse
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--accepted',required=True);args=p.parse_args();R.M=R.bootstrap()
with R.locked('verify_profile_work'):
 R.verify_inputs();matches=[]
 for path in R.OUT.glob(args.accepted+'_profile_phases_w6_*.json'):
  records=R.read(path)
  if len(records)==2440 and all('full_domain_work' in x for x in records):matches.append((path,records))
 assert len(matches)==1,'Need exactly one complete two-version 244-case profile with full-domain counters'
 path,records=matches[0];checked=[]
 for version in ['B0_final',args.accepted]:
  for row in R.M.ROWS:
   source=R.case(version,'pagerank',row)/'timed.graph';text=source.read_text()
   assert 'int max_rounds = 20;' in text and 'out_degree[u] = out_degree[u] + 1;' in text
   values=[x for x in records if x['version']==version and x['algorithm']=='pagerank' and x['graph']==row['graph']];assert len(values)==5
   expected=dict(rounds=21,source_occurrences=21*int(row['vertices']),pair_occurrences=21*int(row['arcs_both_directions']))
   for x in values:
    assert not x.get('error'),x
    for name,value in expected.items():assert x['work'][name]==value and x['full_domain_work'][name]==value,(version,row['graph'],name,value,x['work'],x['full_domain_work'])
    if version=='B0_final':assert x['full_domain_work']['source_map_allocation_bytes']==0
    elif (R.ROOT/'versions'/version/'src/fullgraph_runtime.inc').exists():
     assert x['full_domain_work']['source_map_allocation_bytes']==20*max(int(row['vertices']),1)*8
     assert x['full_domain_work']['successful_index_allocation_bytes']>0
   checked.append(dict(version=version,graph=row['graph'],expected=expected,observations=5))
 result=dict(profile=str(path),profile_sha256=R.sha(path),checked=checked,interpretation='Potential logical source/native-arc occurrences before operation gates; PageRank includes one degree-construction traversal and 20 contribution traversals. No record is needed for either full-graph path.')
 target=R.OUT/'profile_work_audit.json'
 if target.exists():assert R.read(target)==result
 else:R.save(target,result)
 print(target)
