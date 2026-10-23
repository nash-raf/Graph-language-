#!/usr/bin/env python3
"""Iteration memory correctness only; no benchmark or warmup entry points."""
from pathlib import Path
import argparse, hashlib, json, os, re, shutil, subprocess, sys
import validate_execution_composition as C
S=C.S; V=C.V; R=C.R
V.BASE=V.ROOT/'versions/composition01'
PROBE=Path('/home/kazi/p1_iteration_memory_inspect')

def probe(d=None):
    out=PROBE;out.mkdir(exist_ok=True)
    if not (out/'pre.ll').exists():
        for ll in (V.BASE/'src').glob('*.ll'):
            if not (out/ll.name).exists():(out/ll.name).symlink_to(ll)
        (out/'probe.graph').write_text(R.M.make_source_original('pagerank',R.M.ROWS[0],True))
        env=R.M.env();env.update(FORCE_CPU='1',DUMP_LLVM_BC_PRE_PDG=str(out/'pre.bc'))
        R.checked([V.BASE/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(out/'original.ll'),out/'probe.graph'],out,out/'original-build.log',env)
        R.checked(['llvm-dis-20',out/'pre.bc','-o',out/'pre.ll'],out,out/'disassemble.log')
    R.checked(['g++','-O0','-g0','-I.',V.REPO/'test/iteration_memory_ir_test.cpp',
        *R.cfg('--cxxflags'),*R.cfg('--ldflags'),*R.cfg('--libs','all'),*R.cfg('--system-libs'),
        '-o',out/'ir-test'],V.REPO,out/'ir-build.log')
    R.checked([out/'ir-test',out/'pre.ll'],V.REPO,out/'ir-test.log')
    R.save(out/'structural_summary.json',dict(passed=True,fixture_sha256=R.sha(out/'pre.ll'),test_sha256=R.sha(V.REPO/'test/iteration_memory_ir_test.cpp'),benchmark_runs=0))
    if d:
        target=d/'iteration_structural';target.mkdir(exist_ok=True)
        for name in ['pre.ll','ir-test.log','ir-build.log','structural_summary.json']:shutil.copy2(out/name,target/name)
    print((out/'ir-test.log').read_text(),flush=True)

def native(out):
    out.mkdir(exist_ok=True,parents=True)
    for source in ['semantic_stage_runtime_test','execution_simplify_runtime_test']:
      for mode,flags in [('normal',['-O2']),('sanitized',['-O1','-g','-no-pie','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
        exe=out/(source+'-'+mode)
        R.checked(['gcc',*flags,'-fopenmp','-pthread','-I.','-Itest',V.REPO/'test'/(source+'.c'),
            'autotuner_runtime.c','parallel_runtime.c','-lnlopt','-lm','-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create','-o',exe],V.REPO,out/(source+'-'+mode+'.build.log'))
        for width in [1,2,6]:
          env=R.M.env(workers=width);env.update(ASAN_OPTIONS='detect_leaks=0',SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1')
          R.checked(['bash','-c','ulimit -s 8192; exec "$@"','native',sys.executable,V.REPO/'test/physical_run.py',str(width),exe],V.REPO,out/f'{source}-{mode}.w{width}.log',env)
        print('NATIVE PASS',source,mode,flush=True)
    R.save(out/'summary.json',dict(passed=True,workers=[1,2,6],modes=['normal','ASan/UBSan'],benchmark_runs=0))

def iteration_fixtures(d):
    out=d/'iteration_fixtures';out.mkdir(exist_ok=True)
    edge=out/'edges.txt';edge.write_text('0 2\n1 2\n1 2\n2 2\n4 4\n')
    helper=out/'bits.o';R.checked(['gcc','-O2','-c',V.REPO/'tools/p1_campaign/exact_bits.c','-o',helper],out,out/'bits-build.log')
    checks=[]
    for name in ['plain','nan','infinity','signed_zero','one_round','odd_rounds','empty_domain',
                 'observe_between','observe_counter','partial_seed','partial_copy','nonaddition','gate_target','source_target_alias','zero_rounds']:
        rounds=1 if name=='one_round' else 3 if name=='odd_rounds' else 0 if name=='zero_rounds' else 20
        expected=name in ['plain','nan','infinity','signed_zero','one_round','odd_rounds','empty_domain']
        init='for v in G.nodes { current[v] = 1.0; next[v] = 123.0; }'
        if name=='nan':init+=' current[0] = 0.0/0.0;'
        if name=='infinity':init+=' current[0] = 1.0/0.0; current[1] = -1.0/0.0;'
        if name=='signed_zero':init+=' current[0] = -0.0; current[1] = 0.0;'
        seed='-0.0' if name=='signed_zero' else '0.15'
        seed_body='if (v > 0) { next[v] = seed; }' if name=='partial_seed' else 'next[v] = seed;'
        copy_body='if (v > 0) { current[v] = next[v]; }' if name=='partial_copy' else 'current[v] = next[v];'
        update='next[v] - current[u]' if name=='nonaddition' else 'next[v] + current[u]'
        if name=='observe_counter':update='next[v] + current[u] + round'
        if name=='source_target_alias':update='next[v] + (current[u] + next[u])'
        traversal=f'for u in G.nodes {{ for v in G.neighbors(u) {{ next[v] = {update}; }} }}'
        if name=='gate_target':traversal=f'for u in G.nodes {{ if (next[u] > 0.0) {{ for v in G.neighbors(u) {{ next[v] = {update}; }} }} }}'
        edges=edge
        if name=='empty_domain':edges=out/'empty.txt';edges.write_text('')
        source=f'''graph G {{ directed: true; edges: file "{edges}"; }};
int n = numVertices(G); real current[n]; real next[n];
{init}
real seed = {seed}; int round = 0; int limit = {rounds};
{'swap(current,next);' if name=='alias' else ''}
while (round < limit) {{
 round = round + 1;
 for v in G.nodes {{ {seed_body} }}
 {'print next[0];' if name=='observe_between' else ''}
 {traversal}
 for v in G.nodes {{ {copy_body} }}
}}
print "VALUES_BEGIN"; int z = 0;
while (z < n) {{ print current[z]; print next[z]; z = z + 1; }}
print round; print "VALUES_END";
'''
        reference=None;bits=None
        for lowered in [False,True]:
            case=out/(name+('_enabled' if lowered else '_baseline'));case.mkdir(exist_ok=True)
            for ll in (d/'src').glob('*.ll'):
                if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
            (case/'source.graph').write_text(source)
            env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1')
            if not lowered:env['SGPL_NO_ITERATION_MEMORY_LOWERING']='1'
            R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'source.ll'),case/'source.graph'],case,case/'compile.log',env)
            activated='[iteration-memory] activate ' in (case/'compile.log').read_text()
            assert activated==(lowered and expected),(name,lowered,'unexpected proof activation',activated)
            R.checked(['g++','-O3','-no-pie','-fopenmp','-pthread',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],helper,
                '-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-ldl','-lnlopt','-o',case/'check'],case,case/'link.log')
            for width in [1,2,6]:
              for disabled in ([False,True] if lowered else [False]):
                env=R.M.env(workers=width);env.update(SGPL_BITS_OUTPUT=str(case/f'w{width}.{disabled}.bits'),SGPL_SEMANTIC_STAGE_STATS='1')
                if disabled:env['SGPL_NO_ITERATION_MEMORY']='1'
                run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py',str(width),case/'check'],cwd=case,env=env,capture_output=True,text=True,timeout=60)
                (case/f'w{width}.{disabled}.out').write_text(run.stdout);(case/f'w{width}.{disabled}.err').write_text(run.stderr)
                assert run.returncode==0,(name,width,run.stderr[-1000:])
                values=run.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split();raw=(case/f'w{width}.{disabled}.bits').read_bytes()
                if reference is None:reference=values;bits=raw
                assert values==reference and raw==bits,(name,width,disabled,'exact arrays/round counter differ')
                active='[iteration-memory] seed/rotate/map' in run.stderr
                assert active==(lowered and expected and not disabled),(name,width,disabled,'runtime selection mismatch')
                checks.append(dict(fixture=name,compiled=lowered,runtime_disabled=disabled,workers=width,activated=active,passed=True))
        print('ITERATION FIXTURE PASS',name,flush=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))

def activation(d):
    out=d/'iteration_activation';out.mkdir(exist_ok=True);checks=[]
    for alg in R.ALGS:
      for row in R.M.ROWS:
        case=d/'cases'/alg/row['graph'].removesuffix('.edges');log=(case/'compile.log').read_text()
        assert ('[iteration-memory] activate ' in log)==(alg=='pagerank'),(alg,row['graph'],'compile activation')
        if alg=='sssp':assert '[exec-compose] R8 activate' in log,(alg,row['graph'])
        env=R.M.env(workers=6);env['SGPL_SEMANTIC_STAGE_STATS']='1'
        run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py','6',case/'validate'],cwd=case,env=env,capture_output=True,text=True,timeout=190)
        target=out/alg/row['graph'].removesuffix('.edges');target.parent.mkdir(exist_ok=True,parents=True)
        target.with_suffix('.out').write_text(run.stdout);target.with_suffix('.err').write_text(run.stderr)
        assert run.returncode==0,(alg,row['graph'],run.stderr[-1000:])
        assert ('[iteration-memory] seed/rotate/map' in run.stderr)==(alg=='pagerank')
        assert ('[iteration-memory] R8 set' in run.stderr)==(alg=='sssp')
        if alg=='cc':assert run.stderr.count('[semantic-stage] R12 complete')==1
        values=run.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
        reference=(case/'baseline_w6.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
        assert values==reference
        checks.append(dict(algorithm=alg,graph=row['graph'],workers=6,passed=True,
            seed_rotation_map=alg=='pagerank',set_frontier=alg=='sssp',r12_retained=alg=='cc'))
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))

def detector(d):
    import check_cc_road_races as T
    T.FROZEN=d;T.OUT=d/'iteration_tsan';T.OUT.mkdir(exist_ok=True)
    T.DISABLE_ENV='SGPL_NO_ITERATION_MEMORY';T.EXTRA_ENV={'SGPL_SEMANTIC_STAGE_STATS':'1'}
    T.GRAPHS=['iteration-pagerank','iteration-sssp']
    for name,alg in [('iteration-pagerank','pagerank'),('iteration-sssp','sssp')]:
        folder=d/'cases/cc'/name;folder.mkdir(exist_ok=True,parents=True)
        source=d/'cases'/alg/'dimacs/DSJC1000-5'
        shutil.copy2(source/'validate.ll',folder/'validate.ll')
        for width in [1,2,6]:shutil.copy2(source/f'baseline_w{width}.out',folder/f'enabled_w{width}.out')
    T.build();T.control();T.check(serial_openmp=True)
    result=R.read(T.OUT/'graph_pool_only_summary.json');control=R.read(T.OUT/'positive_control.json')
    assert all(x['status'] in ['passed','startup_unavailable','empty_launch_crash'] for x in result['checks']),result['checks']
    R.save(T.OUT/'summary.json',dict(passed=True,all_detector_checks_passed=result['all_passed'],checks=result,positive_control=control,
        limitation='Auxiliary libgomp runs with OMP_NUM_THREADS=1; actual SGPL workers are 1/2/6. Startup failures are retained separately, never counted as detector passes.',benchmark_runs=0))

def report(d):
    build=R.read(d/'correctness_build.json')
    paths={'validation':'validation/summary.json','focused':'semantic_contracts/summary.json','iteration_fixtures':'iteration_fixtures/summary.json',
        'structural':'iteration_structural/structural_summary.json',
        'custom':'semantic_custom/summary.json','weighted_custom':'semantic_weighted_custom/summary.json','activation':'iteration_activation/summary.json',
        'worklist':'composition_worklist/summary.json','detector':'iteration_tsan/summary.json',
        'fullgraph':'contracts/simplification.json','existing':'contracts/existing.json','worker_pool':'contracts/worker_pool.json',
        'frontier':'frontier_contracts/summary.json','relaxation':'relax_contracts/summary.json'}
    checks={name:R.read(d/path) for name,path in paths.items()};assert all(x and x['passed'] for x in checks.values())
    arrays=0
    for alg in R.ALGS:
      for row in R.M.ROWS:
        case=d/'cases'/alg/row['graph'].removesuffix('.edges')
        for width in [1,2,6]:
          values=[(case/f'{mode}_w{width}.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split() for mode in ['baseline','enabled','disabled']]
          assert values[0]==values[1]==values[2];arrays+=1
    files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    source_hash=R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})
    assert source_hash==build['identity']['source_hash']
    old=(V.BASE/'src/semantic_stage_runtime.inc').read_text();new=(d/'src/semantic_stage_runtime.inc').read_text()
    def closure(t):
        start=t.index('static void sgpl_semantic_fixed_point(');return t[start:t.index('\n}\n',start)+3]
    assert closure(old)==closure(new),'retain accepted R12'
    payload=dict(passed=True,branch=R.git('branch','--show-current'),source_commit=build['identity']['commit'],source_hash=source_hash,build=build,
        accepted_ARS1_commit=R.git('rev-parse','ARS-1'),baseline_build=str(V.BASE),baseline_identity=R.read(V.BASE/'identity.json'),checks=checks,
        exact_array_groups=arrays,descriptor_version=4,benchmark_runs=0,new_cost_models=0,r12_unchanged=True,artifact_root=str(d),
        new_licenses=['complete bounded seed/traversal/copy window','restored private buffer identities','parallel disjoint source map cells','exact signed minimum with unobserved circulating frontier'],
        new_realizations=['register seed initialization including empty destinations','buffer rotation and final identity restoration','parallel source preparation',
            'per-round minimum and changed-destination set','active-source capture without full-array copy','exclusive reusable thread workspace'],
        fallback='Original loop/context path retained; all allocation/refusal before user effects; SGPL_NO_ITERATION_MEMORY=1 retains accepted execution.',performance_claim=None)
    R.save(d/'iteration_memory_correctness.json',payload);R.save(V.REPO/'reports/iteration_memory_correctness.json',payload)
    print(json.dumps(dict(passed=True,exact_array_groups=arrays,independent_checks=checks['validation']['independent_checks'],exact_pagerank_groups=checks['validation']['exact_pagerank_groups']),indent=2),flush=True)

def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['probe','native','build','smoke','programs','fixtures','focused','existing','custom','activation','detector','report','all']);p.add_argument('--version',default='iteration_memory06');a=p.parse_args()
    V.prepare();R.M.make_source_original=R.M.make_source
    R.M.make_source=lambda alg,row,val:S.without_timing(R.M.make_source_original(alg,row,val))
    d=V.ROOT/'versions'/a.version
    with R.locked('iteration_memory_'+a.stage):
        if a.stage=='probe':probe()
        elif a.stage=='native':native(PROBE/'native')
        elif a.stage=='build':V.build(a.version)
        elif a.stage=='programs':V.programs(d)
        elif a.stage=='smoke':
            R.ALGS=['sssp','pagerank'];R.M.ROWS=[r for r in R.M.ROWS if r['graph'] in ['ia/ia-email-univ.edges','dimacs/DSJC1000-5.edges']];V.programs(d)
        elif a.stage=='focused':S.focused(d)
        elif a.stage=='fixtures':iteration_fixtures(d)
        elif a.stage=='existing':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif a.stage=='custom':S.custom(d);C.weighted_custom(d,allow_set=True)
        elif a.stage=='activation':activation(d)
        elif a.stage=='detector':detector(d)
        elif a.stage=='report':report(d)
        else:
            probe(d);iteration_fixtures(d);V.programs(d);S.focused(d);C.worklist(d)
            S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
            S.custom(d);C.weighted_custom(d,allow_set=True);activation(d);detector(d);report(d)
if __name__=='__main__':main()
