#!/usr/bin/env python3
"""Audit completed local observations and save a compact repository report."""
import collections, json, math
from pathlib import Path
import benchmark_execution_simplification as B

R=B.R
protocol,old=B.prepare()
selected=R.read(B.OUT/'final_summary.json')
assert selected['selected_phase']=='confirmation'
assert selected['graptor_new_launches']==0
controls={};total=0;all_bases=set()
for phase,n in [('primary',5),('confirmation',20)]:
    observations=R.read(B.OUT/(phase+'.json'))
    summary=R.read(B.OUT/(phase+'_summary.json'))['summary']
    assert len(observations)==244*5*n and summary['observations']==len(observations)
    assert summary['AA_stable'] and not summary['AA_failures']
    groups=collections.defaultdict(list)
    for x in observations:
        assert x['phase']==phase and x['workers']==6 and not x.get('error')
        assert x['version'] in ['baseline','candidate'] and x['seconds']>0 and math.isfinite(x['seconds'])
        assert x['base'] not in all_bases;all_bases.add(x['base'])
        assert R.read(Path(x['base']+'.record.json'))==x
        stamp=R.read(Path(x['base']+'.stamp.json'))
        assert stamp['exit_code']==0 and stamp['seconds']==x['seconds']
        groups[x['algorithm'],x['graph'],x['label']].append(x['repeat'])
    assert len(groups)==1220 and all(sorted(v)==list(range(n)) for v in groups.values())
    total+=len(observations);controls[phase]={'observations':len(observations),'AA_stable':True,'confirmed_slowdowns':summary['confirmed_slowdowns']}
fingerprints=R.read(B.OUT/'timed_fingerprints.json')
assert len(fingerprints)==488
for name,digest in fingerprints.items():assert R.sha(B.ROOT/name)==digest
final=selected['blocks']['confirmation']
assert not final['confirmed_slowdowns']
assert R.git('rev-parse','ARS-1')==protocol['baseline_commit']
report={'native_candidate_commit':protocol['candidate_commit'],'baseline_commit':protocol['baseline_commit'],'candidate_source_hash':protocol['source_hash'],'host':protocol['host'],'cases':244,'graphs_per_algorithm':61,'primary_repetitions':5,'confirmation_repetitions':20,'timing_observations':total,'warmup_launches':2440,'failed_launches':0,'graptor_new_launches':0,'historical_graptor':protocol['historical_graptor'],'paired_controls':controls,'by_algorithm':final['by_algorithm'],'zero_corrected_slowdowns':True,'timed_executable_fingerprints_verified':488,'ratio_direction':'Speedup = baseline/candidate; above one favors simplification. Graptor time ratios are candidate/historical Graptor; below one favors candidate.','historical_comparison_limitation':'Saved Graptor data are same-PC, same-input, six-worker observations from a previous run. No paired or statistical superiority claim is made against them.','artifacts':{'root':str(B.OUT),'html':str(B.OUT/'final_report.html'),'csv':str(B.OUT/'per_graph_times.csv'),'full_summary':str(B.OUT/'final_summary.json'),'raw_records':str(B.ROOT/'runs/execution_simplification')},'native_changes_during_measurement':False,'merged_into_ARS1':False}
R.save(B.OUT/'integrity_audit.json',report)
R.save(B.REPO/'reports/execution_simplification_benchmark.json',report)
R.save(B.OUT/'SHA256SUMS.json',{p.name:R.sha(p) for p in B.OUT.iterdir() if p.is_file() and p.suffix in ['.json','.html','.txt','.csv'] and p.name not in ['SHA256SUMS.json','status.json']})
print(json.dumps({'audit_passed':True,'timing_observations':total,'by_algorithm':final['by_algorithm']},indent=2))
