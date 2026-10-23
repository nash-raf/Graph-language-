#!/usr/bin/env python3
"""R10/R11 repair correctness only. Preserve previous reports and binaries."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import shutil
import validate_semantic_stages as S

V=S.V
R=S.R
V.BASE=V.ROOT/'versions/semantic08'

def activation(d):
    S.activation_checks(d)
    payload=R.read(d/'semantic_activation/summary.json')
    for x in payload['checks']:
        if x['algorithm']=='sssp':
            p=d/'semantic_activation'/x['algorithm']/x['graph'].removesuffix('.edges')
            text=p.with_suffix('.err').read_text()
            x['topology_validations']=text.count('[semantic-stage] topology validated')
            assert x['topology_validations']==1,(x,'topology must be validated once, not once per round')
    R.save(d/'semantic_activation/summary.json',payload)

def detector(d):
    import check_cc_road_races as T
    T.FROZEN=d;T.OUT=d/'semantic_tsan';T.OUT.mkdir(exist_ok=True)
    T.DISABLE_ENV='SGPL_NO_SEMANTIC_STAGE';T.EXTRA_ENV={'SGPL_SEMANTIC_STAGE_STATS':'1'}
    T.GRAPHS=['road/road-usroads-48','road/road-luxembourg-osm','socfb/socfb-Simmons81',
              'semantic-fadd-row','semantic-fadd-permuted','semantic-weighted-sssp']
    for name,source in [('semantic-fadd-row','fadd_full_R11'),('semantic-fadd-permuted','fadd_permuted_R10')]:
        folder=d/'cases/cc'/name;folder.mkdir(exist_ok=True)
        shutil.copy2(d/'semantic_custom'/source/'source.ll',folder/'validate.ll')
        ref=d/'semantic_custom'/('fadd_full_none' if name.endswith('row') else 'fadd_permuted_none')
        for width in [1,2,6]:shutil.copy2(ref/f'w{width}.out',folder/f'enabled_w{width}.out')
    folder=d/'cases/cc/semantic-weighted-sssp';folder.mkdir(exist_ok=True)
    ref=d/'cases/sssp/dimacs/DSJC1000-5'
    shutil.copy2(ref/'validate.ll',folder/'validate.ll')
    for width in [1,2,6]:shutil.copy2(ref/f'enabled_w{width}.out',folder/f'enabled_w{width}.out')
    T.build();T.control();T.check(serial_openmp=True)
    checks=R.read(T.OUT/'graph_pool_only_summary.json');positive=R.read(T.OUT/'positive_control.json')
    assert checks['all_passed'] and positive['passed']
    R.save(T.OUT/'semantic_summary.json',dict(passed=True,checks=checks,positive_control=positive,
        caveat='Auxiliary system libgomp work used OMP_NUM_THREADS=1; actual SGPL worker budgets were 1/2/6. All detector startup failures retained separately. No suppressions.',benchmark_runs=0))

def report(d):
    build=R.read(d/'correctness_build.json')
    files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    assert R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})==build['identity']['source_hash']
    evidence={name:R.read(d/path) for name,path in {
        'validation':'validation/summary.json','focused':'semantic_contracts/summary.json',
        'custom':'semantic_custom/summary.json','activation':'semantic_activation/summary.json',
        'detector':'semantic_tsan/semantic_summary.json','fullgraph':'contracts/simplification.json',
        'existing':'contracts/existing.json','worker_pool':'contracts/worker_pool.json',
        'frontier':'frontier_contracts/summary.json','relaxation':'relax_contracts/summary.json'}.items()}
    assert all(x and x['passed'] for x in evidence.values())
    arrays=0
    for alg in R.ALGS:
      for row in R.M.ROWS:
       folder=d/'cases'/alg/row['graph'].removesuffix('.edges')
       for width in [1,2,6]:
        values=[(folder/f'{mode}_w{width}.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split() for mode in ['baseline','enabled','disabled']]
        assert values[0]==values[1]==values[2],(alg,row['graph'],width)
        arrays+=1
    payload=dict(branch=R.git('branch','--show-current'),source_commit=build['identity']['commit'],
        source_hash=build['identity']['source_hash'],build=build,previous_build=str(V.BASE),
        previous_identity=R.read(V.BASE/'identity.json'),checks=evidence,exact_array_groups=arrays,
        fixes=['topology validation reused by mutation generation/layout/pointer/size identity',
               'constant-time first-match weight access including unequal duplicate weights',
               'destination-owned parallel callbacks with stable contribution/notification order',
               '8-byte occurrence/edge keys replace redundant endpoint/weight input fields'],
        descriptor_version=2,new_cost_models=0,benchmark_runs=0,performance_claim=None,
        r12_worklist_unchanged=True,artifact_root=str(d),
        ARS1_unchanged=R.git('rev-parse','ARS-1')=='2b46cb5ded54132e98dc052bd2caf548d1acfcb4')
    R.save(d/'semantic_repair_correctness.json',payload)
    R.save(V.REPO/'reports/semantic_stage_repair_correctness.json',payload)
    print(json.dumps(dict(passed=True,validation=evidence['validation'],focused=evidence['focused'],benchmark_runs=0),indent=2),flush=True)

if __name__=='__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('stage',choices=['build','programs','focused','custom','contracts','activation','detector','report','all'])
    parser.add_argument('--version',required=True);args=parser.parse_args()
    V.prepare();d=V.ROOT/'versions'/args.version
    maker=R.M.make_source;R.M.make_source=lambda a,r,v:S.without_timing(maker(a,r,v))
    with R.locked('semantic_repair_'+args.stage):
        if args.stage=='build':V.build(args.version)
        elif args.stage=='programs':V.programs(d)
        elif args.stage=='focused':S.focused(d)
        elif args.stage=='custom':S.custom(d)
        elif args.stage=='contracts':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif args.stage=='activation':activation(d)
        elif args.stage=='detector':detector(d)
        elif args.stage=='report':report(d)
        else:
            V.programs(d);S.focused(d);S.custom(d)
            S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
            activation(d);detector(d);report(d)
