#!/usr/bin/env python3
"""Proof removals: correctness/structure only. No benchmark entry points."""
from pathlib import Path
import argparse, hashlib, json, re, shutil, subprocess, sys
import validate_iteration_memory as I
C=I.C; S=I.S; V=I.V; R=I.R
V.BASE=V.ROOT/'versions/iteration_memory06'
I.PROBE=Path('/home/kazi/p1_proof_removals_inspect')

def structural(d):
    I.PROBE.mkdir(exist_ok=True)
    if not (I.PROBE/'pre.ll').exists():
        shutil.copy2('/home/kazi/p1_iteration_memory_inspect/pre.ll',I.PROBE/'pre.ll')
    I.probe(d);C.worklist(d)
    worklist=d/'composition_worklist';result=R.read(worklist/'summary.json')
    log=(worklist/'check.log').read_text()
    assert 'cases=524288 runtime_subsets=512' in log
    result.update(composed_cases=524288,runtime_proof_subsets=512,missing_premises_checked=True)
    R.save(worklist/'summary.json',result)
    # Inspect GCC's optimized native representation of the actual direct
    # workers. Their common source is instantiated with a constant index mode.
    out=d/'proof_structural';out.mkdir(exist_ok=True)
    dump=out/'runtime.optimized'
    R.checked(['gcc','-O2','-fopenmp','-pthread','-I.', '-fdump-tree-optimized='+str(dump),
               '-c','autotuner_runtime.c','-o',out/'runtime.o'],d/'src',out/'build.log')
    text=dump.read_text();checks={}
    for name in ['sgpl_semantic_set_prepare_direct','sgpl_semantic_set_fill_direct']:
        body=text.split(';; Function '+name+' ',1)[1].split(';; Function ',1)[0]
        assert '->weights' not in body,(name,'first-match mapping survived')
        assert 'csr_weights' in body,(name,'live weight read absent')
        checks[name]=dict(mapping_loads=0,live_weight_reads=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))

def fixtures(d):
    out=d/'proof_fixtures';out.mkdir(exist_ok=True);checks=[]
    helper=out/'bits.o';R.checked(['gcc','-O2','-c',V.REPO/'tools/p1_campaign/exact_bits.c','-o',helper],out,out/'bits-build.log')
    sources={}
    edge=out/'gate.edges';edge.write_text('0 1\n0 1\n1 2\n2 3\n3 3\n')
    for name in ['entailed','false','mutable','all_false']:
        predicate='current[u] > 0.0' if name=='mutable' else 'mask[u] > 0'
        init='mask[v] = 1;'
        if name=='false':init+=' mask[0] = 0;'
        if name=='all_false':init='mask[v] = 0;'
        sources['gate_'+name]=f'''graph G {{ directed: true; edges: file "{edge}"; }};
int n = numVertices(G); real current[n]; real next[n]; int mask[n];
for v in G.nodes {{ current[v] = 1.0; next[v] = 0.0; {init} }}
real seed = -0.0; int round = 0; int limit = 3;
while (round < limit) {{ round = round + 1;
 for v in G.nodes {{ next[v] = seed; }}
 for u in G.nodes {{ if ({predicate}) {{ for v in G.neighbors(u) {{ next[v] = next[v] + current[u]; }} }} }}
 for v in G.nodes {{ current[v] = next[v]; }}
}}
print "VALUES_BEGIN"; int z = 0; while (z < n) {{ print current[z]; print next[z]; z = z + 1; }} print "VALUES_END";
'''
    for kind in ['unique','duplicate','nonfoldable']:
        edge=out/(kind+'.wel');edge.write_text('0 1 3\n'+('0 1 999\n' if kind=='duplicate' else '')+'1 2 4\n2 3 5\n3 4 6\n')
        body='int candidate = value[u] + weight(G,u,v); if (candidate < value[v]) { value[v] = candidate;'
        if kind=='nonfoldable':body='int candidate = value[u] + weight(G,u,v); value[v] = value[v] - candidate;'
        body+=' next_frontier[next_size] = v; next_size = next_size + 1;'
        if kind!='nonfoldable':body+=' }'
        sources['weighted_'+kind]=f'''graph G {{ directed: true; edges: file "{edge}"; TRUE }};
int n = numVertices(G); int cap = 64; int value[n]; int frontier[cap]; int next_frontier[cap];
for v in G.nodes {{ value[v] = 2147483647; }} value[0] = 0; frontier[0] = 0;
int frontier_size = 1; int next_size = 0;
while (frontier_size > 0) {{ next_size = 0; int k = 0;
 while (k < frontier_size) {{ int u = frontier[k]; for v in G.neighbors(u) {{ {body} }} k = k + 1; }}
 swap(frontier,next_frontier); frontier_size = next_size;
}}
print "VALUES_BEGIN"; int z = 0; while (z < n) {{ print value[z]; z = z + 1; }} print "VALUES_END";
'''
    sources['weighted_other_graph']=sources['weighted_unique'].replace('int n =',f'graph H {{ directed: true; edges: file "{out / "unique.wel"}"; TRUE }};\nint n =').replace('weight(G,u,v)','weight(H,u,v)')
    for name,source in sources.items():
      reference=None;reference_bits=None
      for subset in range(8):
        rules='R1,R2,R3,R6,R7,R8,R10'+''.join(',R'+str(i+13) for i in range(3) if subset&(1<<i))
        case=out/(name+'_'+str(subset));case.mkdir(exist_ok=True)
        for ll in (d/'src').glob('*.ll'):
            if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
        (case/'source.graph').write_text(source)
        env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1',SGPL_EXEC_SIMPLIFY_RULES=rules)
        R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'source.ll'),case/'source.graph'],case,case/'compile.log',env)
        R.checked(['g++','-O3','-no-pie','-fopenmp','-pthread',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],helper,
            '-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-ldl','-lnlopt','-o',case/'check'],case,case/'link.log')
        for width in [1,2,6]:
          for disabled in [False,True]:
            env=R.M.env(workers=width);env.update(SGPL_SEMANTIC_STAGE_STATS='1',SGPL_BITS_OUTPUT=str(case/f'w{width}.{disabled}.bits'))
            if disabled:env['SGPL_NO_PROOF_REMOVALS']='1'
            run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py',str(width),case/'check'],cwd=case,env=env,capture_output=True,text=True,timeout=60)
            (case/f'w{width}.{disabled}.out').write_text(run.stdout);(case/f'w{width}.{disabled}.err').write_text(run.stderr)
            assert run.returncode==0,(name,subset,width,disabled,run.stderr[-2000:])
            values=run.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split();bits=(case/f'w{width}.{disabled}.bits').read_bytes()
            if reference is None:reference=values;reference_bits=bits
            assert values==reference and bits==reference_bits,(name,subset,width,disabled,'exact differential mismatch')
            r13=name=='gate_entailed' and bool(subset&1) and not disabled
            r14=name in ['weighted_unique','weighted_duplicate'] and bool(subset&2) and not disabled
            r15=name in ['weighted_unique','weighted_nonfoldable'] and bool(subset&4) and not disabled
            for rule,expected in [(13,r13),(14,r14),(15,r15)]:
                assert (f'[proof-removals] R{rule} activate' in run.stderr)==expected,(name,subset,width,disabled,rule,run.stderr[-1500:])
            if r14:assert 'input=producer-proven' in run.stderr,(name,'induction failed to propagate')
            checks.append(dict(fixture=name,subset=subset,workers=width,disabled=disabled,passed=True,R13=r13,R14=r14,R15=r15))
      print('PROOF FIXTURE PASS',name,flush=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,rule_subsets=8,benchmark_runs=0))

def report(d):
    paths={'validation':'validation/summary.json','focused':'semantic_contracts/summary.json',
        'iteration_fixtures':'iteration_fixtures/summary.json','structural':'iteration_structural/structural_summary.json',
        'proof_structural':'proof_structural/summary.json','proof_fixtures':'proof_fixtures/summary.json','native':'proof_native/summary.json',
        'custom':'semantic_custom/summary.json','weighted_custom':'semantic_weighted_custom/summary.json','activation':'iteration_activation/summary.json',
        'worklist':'composition_worklist/summary.json','detector':'iteration_tsan/summary.json',
        'fullgraph':'contracts/simplification.json','existing':'contracts/existing.json','worker_pool':'contracts/worker_pool.json',
        'frontier':'frontier_contracts/summary.json','relaxation':'relax_contracts/summary.json'}
    checks={name:R.read(d/path) for name,path in paths.items()};assert all(x and x['passed'] for x in checks.values())
    build=R.read(d/'correctness_build.json')
    files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    source_hash=R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})
    assert source_hash==build['identity']['source_hash']
    def closure(text):
        start=text.index('static void sgpl_semantic_fixed_point(');return text[start:text.index('\n}\n',start)+3]
    assert closure((V.BASE/'src/semantic_stage_runtime.inc').read_text())==closure((d/'src/semantic_stage_runtime.inc').read_text())
    payload=dict(passed=True,branch=R.git('branch','--show-current'),source_commit=build['identity']['commit'],source_hash=source_hash,
        build=build,baseline=str(V.BASE),baseline_identity=R.read(V.BASE/'identity.json'),checks=checks,
        retained_ARS1_commit=R.git('rev-parse','ARS-1'),validation_commit=R.git('rev-parse','HEAD'),
        benchmark_runs=0,new_cost_models=0,performance_claim=None,r12_unchanged=True,
        rules=['R13 immutable edge-entailed gate','R14 bounded set output and circulating range induction','R15 topology identity for first-match weights'],
        composition=dict(shared_compiler_and_runtime_worklist=True,missing_facts_refuse=True,runtime_subsets=512,compiler_order_subset_cases=524288),
        validation_sources={name:R.sha(V.REPO/name) for name in ['tools/validate_proof_removals.py','test/execution_simplification_worklist_test.cpp',
            'test/iteration_memory_ir_test.cpp','test/semantic_stage_ir_test.cpp','test/execution_simplify_runtime_test.c','test/semantic_stage_runtime_test.c']},
        artifact_root=str(d))
    R.save(d/'proof_removals_correctness.json',payload);R.save(V.REPO/'reports/proof_removals_correctness.json',payload)
    print(json.dumps(dict(passed=True,independent_checks=checks['validation']['independent_checks'],exact_pagerank_groups=checks['validation']['exact_pagerank_groups'],benchmark_runs=0)),flush=True)

def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['build','structural','native','focused','fixtures','programs','iteration','existing','custom','activation','detector','report','all']);p.add_argument('--version',default='proof_removals02');a=p.parse_args()
    V.prepare();R.M.make_source_original=R.M.make_source
    R.M.make_source=lambda alg,row,val:S.without_timing(R.M.make_source_original(alg,row,val))
    d=V.ROOT/'versions'/a.version
    def stage(name):
        if name=='build':V.build(a.version)
        elif name=='structural':structural(d)
        elif name=='native':I.native(d/'proof_native')
        elif name=='focused':S.focused(d)
        elif name=='fixtures':fixtures(d)
        elif name=='programs':V.programs(d)
        elif name=='iteration':I.iteration_fixtures(d)
        elif name=='existing':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif name=='custom':S.custom(d);C.weighted_custom(d,allow_set=True)
        elif name=='activation':I.activation(d)
        elif name=='detector':I.detector(d)
        elif name=='report':report(d)
    with R.locked('proof_removals_'+a.stage):
        for name in (['build','structural','native','focused','fixtures','iteration','programs','existing','custom','activation','detector','report'] if a.stage=='all' else [a.stage]):stage(name)
if __name__=='__main__':main()
