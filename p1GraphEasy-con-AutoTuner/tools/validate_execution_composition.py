#!/usr/bin/env python3
"""Composable execution simplification correctness only. Preserve previous reports and binaries."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import validate_semantic_stages as S

V=S.V
R=S.R
V.BASE=V.ROOT/'versions/semantic_repair01'

def worklist(d):
    out=d/'composition_worklist';out.mkdir(exist_ok=True)
    R.checked(['g++','-O2','-std=c++17',V.REPO/'test/execution_simplification_worklist_test.cpp','-o',out/'check'],V.REPO,out/'build.log')
    R.checked([out/'check'],V.REPO,out/'check.log')
    checks=int(re.search(r'checks=(\d+)',(out/'check.log').read_text())[1])
    assert checks==30720
    R.save(out/'summary.json',dict(passed=True,checks=checks,rule_registration_orders=120,rule_subsets=32,benchmark_runs=0))

def weighted_custom(d, allow_set=False):
    out=d/'semantic_weighted_custom';out.mkdir(exist_ok=True)
    edge=out/'edges.wel';edge.write_text('0 3 3\n0 3 999\n1 3 7\n2 3 4\n')
    source=f'''graph G {{ directed: true; edges: file "{edge}"; TRUE }};
int n = numVertices(G); int cap = 64;
int dist[n]; int frontier[cap]; int next_frontier[cap];
dist[0] = 2147483646; dist[1] = 0; dist[2] = 8; dist[3] = 2147483647;
frontier[0] = 0; frontier[1] = 1; frontier[2] = 2; frontier[3] = 3;
int frontier_size = 4; int next_size = 0;
while (frontier_size > 0) {{
 next_size = 0; int k = 0;
 while (k < frontier_size) {{ int u = frontier[k];
  for v in G.neighbors(u) {{ int nd = dist[u] + weight(G,u,v);
   if (nd < dist[v]) {{ dist[v] = nd; next_frontier[next_size] = v; next_size = next_size + 1; }}
  }} k = k + 1;
 }} swap(frontier,next_frontier); frontier_size = next_size;
}}
print "VALUES_BEGIN"; int z = 0; while (z < n) {{ print dist[z]; z = z + 1; }} print "VALUES_END";
'''
    reference=None;checks=[]
    for rules in ['none','R10','R11','R6,R10','R7,R10','R6,R7,R10','R6,R7,R11','all']:
        case=out/rules;case.mkdir(exist_ok=True)
        for ll in (d/'src').glob('*.ll'):
            if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
        (case/'source.graph').write_text(source)
        env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1',SGPL_EXEC_SIMPLIFY_RULES=rules)
        R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'source.ll'),case/'source.graph'],case,case/'compile.log',env)
        R.checked(['g++','-O3','-no-pie','-fopenmp','-pthread',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],'-ldl','-lnlopt','-o',case/'check'],case,case/'link.log')
        for width in [1,2,6]:
            env=R.M.env(workers=width);env['SGPL_SEMANTIC_STAGE_STATS']='1'
            run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py',str(width),case/'check'],cwd=case,env=env,capture_output=True,text=True,timeout=60)
            (case/f'w{width}.out').write_text(run.stdout);(case/f'w{width}.err').write_text(run.stderr)
            assert run.returncode==0,(rules,width,run.stderr[-1000:])
            values=run.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
            if reference is None:reference=values
            assert values==reference,(rules,width,values,reference)
            active=re.findall(r'\[semantic-stage\] R(\d+) complete',run.stderr)
            if rules in ['R11','R6,R7,R11','all']:assert '11' in active or (allow_set and rules=='all' and '[iteration-memory] R8 set' in run.stderr),(rules,width,'weighted generated row/set did not execute')
            checks.append(dict(rules=rules,workers=width,activation=active,passed=True))
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))
    print('WEIGHTED GENERATED ROW PASS',len(checks),flush=True)

def activation(d):
    S.activation_checks(d)
    payload=R.read(d/'semantic_activation/summary.json')
    for x in payload['checks']:
        if x['algorithm']=='sssp':
            p=d/'semantic_activation'/x['algorithm']/x['graph'].removesuffix('.edges')
            text=p.with_suffix('.err').read_text()
            x['topology_validations']=text.count('[semantic-stage] topology validated')
            assert x['topology_validations']==1,(x,'topology must be validated once, not once per round')
            calls=[tuple(map(int,m)) for m in re.findall(r'\[exec-compose\] R(\d+) rules=(\d+) original=(\d+) prepared=(\d+)',text)]
            assert calls and all(mode==10 and rules==96 and prepared<=original for mode,rules,original,prepared in calls),x
            x['original_pairs']=sum(c[2] for c in calls);x['prepared_pairs']=sum(c[3] for c in calls)
            x['composed_rules']=['R6','R7','R10']
    assert any(x.get('prepared_pairs',0)<x.get('original_pairs',0) for x in payload['checks'])
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
    def fixed_point(path):
        text=path.read_text();start=text.index('static void sgpl_semantic_fixed_point(')
        return text[start:text.index('\n}\n',start)+3]
    old_worklist=fixed_point(V.BASE/'src/semantic_stage_runtime.inc')
    new_worklist=fixed_point(d/'src/semantic_stage_runtime.inc')
    assert old_worklist==new_worklist,'Retain the accepted R12 worklist'
    files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    assert R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})==build['identity']['source_hash']
    evidence={name:R.read(d/path) for name,path in {
        'validation':'validation/summary.json','focused':'semantic_contracts/summary.json','worklist':'composition_worklist/summary.json',
        'custom':'semantic_custom/summary.json','weighted_custom':'semantic_weighted_custom/summary.json','activation':'semantic_activation/summary.json',
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
               '8-byte occurrence/edge keys replace redundant endpoint/weight input fields',
               'monotone dependency worklist converges before one lowering',
               'existing R6/R7 proofs prune before allocation/count/fill of retained keys'],
        descriptor_version=3,new_cost_models=0,benchmark_runs=0,performance_claim=None,
        remaining_costs=['required source snapshots','stable ownership preparation and notification compaction',
                         'R8/R9 retain their separate stronger premises; no new frontier-collapse or weighted-closure license'],
        test_sources={name:R.sha(V.REPO/'test'/name) for name in ['semantic_stage_ir_test.cpp','semantic_stage_runtime_test.c','execution_simplification_worklist_test.cpp']},
        validation_tool_sha256=R.sha(Path(__file__)),
        r12_worklist_unchanged=True,r12_worklist_sha256=hashlib.sha256(new_worklist.encode()).hexdigest(),artifact_root=str(d),
        public_context_header_unchanged=not R.git('diff','2b46cb5ded54132e98dc052bd2caf548d1acfcb4','--','autotuner_runtime.h'),
        ARS1_unchanged=R.git('rev-parse','ARS-1')=='2b46cb5ded54132e98dc052bd2caf548d1acfcb4')
    R.save(d/'execution_composition_correctness.json',payload)
    R.save(V.REPO/'reports/execution_composition_correctness.json',payload)
    print(json.dumps(dict(passed=True,validation=evidence['validation'],focused=evidence['focused'],benchmark_runs=0),indent=2),flush=True)

if __name__=='__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('stage',choices=['build','programs','focused','custom','weighted','worklist','contracts','activation','detector','report','all'])
    parser.add_argument('--version',required=True);args=parser.parse_args()
    V.prepare();d=V.ROOT/'versions'/args.version
    maker=R.M.make_source;R.M.make_source=lambda a,r,v:S.without_timing(maker(a,r,v))
    with R.locked('execution_composition_'+args.stage):
        if args.stage=='build':V.build(args.version)
        elif args.stage=='programs':V.programs(d)
        elif args.stage=='focused':S.focused(d)
        elif args.stage=='custom':S.custom(d)
        elif args.stage=='weighted':weighted_custom(d)
        elif args.stage=='worklist':worklist(d)
        elif args.stage=='contracts':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif args.stage=='activation':activation(d)
        elif args.stage=='detector':detector(d)
        elif args.stage=='report':report(d)
        else:
            V.programs(d);S.focused(d);worklist(d);S.custom(d);weighted_custom(d)
            S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
            activation(d);detector(d);report(d)
