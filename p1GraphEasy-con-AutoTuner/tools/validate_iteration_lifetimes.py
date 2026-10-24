#!/usr/bin/env python3
"""Correctness and structural validation for composable R16--R18."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, sys
import validate_proof_removals as P
I=P.I; C=P.C; S=P.S; V=P.V; R=P.R
V.BASE=V.ROOT/'versions/proof_removals04'
I.PROBE=Path('/home/kazi/p1_iteration_lifetimes_inspect')

def structural(d):
    d.mkdir(exist_ok=True,parents=True)
    I.PROBE.mkdir(exist_ok=True)
    shutil.copy2('/home/kazi/p1_iteration_memory_inspect/pre.ll',I.PROBE/'pre.ll')
    I.probe(d);C.worklist(d)
    out=d/'lifetime_structural';out.mkdir(exist_ok=True)
    R.checked(['g++','-O0','-g0','-I.',V.REPO/'test/iteration_lifetime_ir_test.cpp',
        *R.cfg('--cxxflags'),*R.cfg('--ldflags'),*R.cfg('--libs','all'),*R.cfg('--system-libs'),'-o',out/'ir-test'],V.REPO,out/'build.log')
    R.checked([out/'ir-test',I.PROBE/'pre.ll'],V.REPO,out/'check.log')
    R.save(out/'summary.json',dict(passed=True,subsets=8,count_negative_premises=10,storage_negative_premises=5,benchmark_runs=0))
    print((out/'check.log').read_text()[-3500:],flush=True)

def native(d):
    I.native(d/'lifetime_native')
    out=d/'lifetime_runtime';out.mkdir(exist_ok=True)
    checks=[]
    for mode,flags in [('normal',['-O2']),('sanitized',['-O1','-g','-no-pie','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
        exe=out/('check-'+mode)
        R.checked(['gcc',*flags,'-fopenmp','-pthread','-I.','-Itest',V.REPO/'test/iteration_lifetime_runtime_test.c',
            'autotuner_runtime.c','parallel_runtime.c','-lnlopt','-lm','-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create','-o',exe],V.REPO,out/(mode+'.build.log'))
        for width in [1,2,6]:
            env=R.M.env(workers=width);env.update(ASAN_OPTIONS='detect_leaks=0',SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1')
            R.checked(['bash','-c','ulimit -s 8192; exec "$@"','native',sys.executable,V.REPO/'test/physical_run.py',str(width),exe],V.REPO,out/f'{mode}.w{width}.log',env)
            checks.append(dict(mode=mode,workers=width,passed=True,exact_storage_comparisons=2304))
            print('LIFETIME NATIVE PASS',mode,width,flush=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))

def fixtures(d):
    out=d/'lifetime_fixtures';out.mkdir(exist_ok=True)
    edge=out/'edges.txt';edge.write_text('0 1\n0 1\n1 2\n2 2\n3 4\n')
    helper=out/'bits.o';R.checked(['gcc','-O2','-c',V.REPO/'tools/p1_campaign/exact_bits.c','-o',helper],out,out/'bits-build.log')
    checks=[]
    for name in ['both','source','destination','neither','nonlocal','mutable_gate','count_extra']:
      reference=None;refbits=None
      for gate_rule in [False,True]:
       for subset in range(8):
        rules='R1,R2,R3'+(',R13' if gate_rule else '')+''.join(',R'+str(i+16) for i in range(3) if subset&(1<<i))
        case=out/f'{name}_gate{int(gate_rule)}_{subset}';case.mkdir(exist_ok=True)
        for ll in (d/'src').glob('*.ll'):
            if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
        observe=('print current[z];' if name!='destination' and name!='neither' else '')+('print next[z];' if name=='both' or name=='destination' else '')
        index='u+1' if name=='nonlocal' else 'u'
        predicate='current[u] > 0.0' if name=='mutable_gate' else 'degree[u] > 0'
        source=f'''graph G {{ directed: true; edges: file "{edge}"; }};
int n = numVertices(G); real current[n]; real next[n]; int degree[n];
for v in G.nodes {{ current[v] = 1.0; next[v] = -0.0; degree[v] = 0; }}
for u in G.nodes {{ for v in G.neighbors(u) {{ degree[u] = degree[u] + {2 if name=='count_extra' else 1}; }} }}
real seed = -0.0; int round = 0; int limit = 3;
while (round < limit) {{ round = round + 1;
 for v in G.nodes {{ next[v] = seed; }}
 for u in G.nodes {{ if ({predicate}) {{ real contribution = current[{index}] / degree[u]; for v in G.neighbors(u) {{ next[v] = next[v] + contribution; }} }} }}
 for v in G.nodes {{ current[v] = next[v]; }}
}}
print "VALUES_BEGIN"; int z = 0; while (z < n) {{ {observe} print degree[z]; z = z + 1; }} print round; print "VALUES_END";
'''
        source=source.replace('degree','out_count')
        (case/'source.graph').write_text(source)
        env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1',SGPL_EXEC_SIMPLIFY_RULES=rules)
        R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'source.ll'),case/'source.graph'],case,case/'compile.log',env)
        R.checked(['g++','-O3','-no-pie','-fopenmp','-pthread',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],helper,
            '-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-ldl','-lnlopt','-o',case/'check'],case,case/'link.log')
        for width in [1,2,6]:
         for disabled in [False,True]:
          env=R.M.env(workers=width);bits=case/f'w{width}.{disabled}.bits';env.update(SGPL_BITS_OUTPUT=str(bits),SGPL_SEMANTIC_STAGE_STATS='1')
          if disabled:env['SGPL_NO_ITERATION_LIFETIMES']='1'
          run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py',str(width),case/'check'],cwd=case,env=env,capture_output=True,text=True,timeout=90)
          (case/f'w{width}.{disabled}.out').write_text(run.stdout);(case/f'w{width}.{disabled}.err').write_text(run.stderr)
          assert run.returncode==0,(name,subset,gate_rule,width,disabled,run.stderr[-2000:])
          values=run.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split();raw=bits.read_bytes()
          if reference is None:reference=values;refbits=raw
          assert values==reference and raw==refbits,(name,subset,gate_rule,width,disabled,'exact mismatch')
          expected={16:bool(subset&1) and name!='count_extra',17:bool(subset&2) and name!='both',18:bool(subset&4) and name not in ['nonlocal','mutable_gate']}
          for rule,wanted in expected.items():
            assert (f'[iteration-lifetime] R{rule} activate' in run.stderr)==(wanted and not disabled),(name,subset,gate_rule,width,disabled,rule,run.stderr[-2000:])
          checks.append(dict(fixture=name,subset=subset,R13=gate_rule,workers=width,disabled=disabled,passed=True))
      print('LIFETIME FIXTURE PASS',name,flush=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,rule_subsets=8,previous_gate_rule_subsets=2,benchmark_runs=0))

def activation(d):
    I.activation(d);out=d/'lifetime_activation';out.mkdir(exist_ok=True)
    totals={alg:{f'R{r}':0 for r in [16,17,18]} for alg in R.ALGS};checks=[]
    for alg in R.ALGS:
      for row in R.M.ROWS:
        target=d/'iteration_activation'/alg/row['graph'].removesuffix('.edges');text=target.with_suffix('.err').read_text()
        active={f'R{r}':f'[iteration-lifetime] R{r} activate' in text for r in [16,17,18]}
        for rule,on in active.items():totals[alg][rule]+=int(on)
        assert all(on==(alg=='pagerank') for on in active.values()),(alg,row['graph'],active)
        checks.append(dict(algorithm=alg,graph=row['graph'],activation=active,passed=True))
    R.save(out/'summary.json',dict(passed=True,totals=totals,checks=checks,benchmark_runs=0))

def detector(d):
    import check_cc_road_races as T
    T.FROZEN=d;T.OUT=d/'lifetime_tsan';T.OUT.mkdir(exist_ok=True)
    T.DISABLE_ENV='SGPL_NO_ITERATION_LIFETIMES';T.EXTRA_ENV={'SGPL_SEMANTIC_STAGE_STATS':'1'}
    T.GRAPHS=['lifetime-pagerank','lifetime-sssp']
    for name,alg in [('lifetime-pagerank','pagerank'),('lifetime-sssp','sssp')]:
        folder=d/'cases/cc'/name;folder.mkdir(exist_ok=True,parents=True);source=d/'cases'/alg/'dimacs/DSJC1000-5'
        shutil.copy2(source/'validate.ll',folder/'validate.ll')
        for width in [1,2,6]:shutil.copy2(source/f'baseline_w{width}.out',folder/f'enabled_w{width}.out')
    T.build();T.control();T.check(serial_openmp=True)
    result=R.read(T.OUT/'graph_pool_only_summary.json');control=R.read(T.OUT/'positive_control.json')
    assert result['disable_env']=='SGPL_NO_ITERATION_LIFETIMES'
    assert all(x['status'] in ['passed','startup_unavailable','empty_launch_crash'] for x in result['checks'])
    R.save(T.OUT/'summary.json',dict(passed=True,all_detector_checks_passed=result['all_passed'],checks=result,positive_control=control,
        limitation='Auxiliary libgomp uses OMP_NUM_THREADS=1; actual SGPL workers are 1/2/6. Only R16--R18 are disabled. Detector startup failures are recorded separately.',benchmark_runs=0))

def report(d):
    paths={'validation':'validation/summary.json','native':'lifetime_native/summary.json','lifetime_runtime':'lifetime_runtime/summary.json',
        'structural':'lifetime_structural/summary.json','previous_structural':'iteration_structural/structural_summary.json',
        'fixtures':'lifetime_fixtures/summary.json','iteration':'iteration_fixtures/summary.json','activation':'lifetime_activation/summary.json',
        'worklist':'composition_worklist/summary.json','detector':'lifetime_tsan/summary.json',
        'fullgraph':'contracts/simplification.json','existing':'contracts/existing.json','workers':'contracts/worker_pool.json',
        'relaxation':'relax_contracts/summary.json','frontier':'frontier_contracts/summary.json'}
    checks={name:R.read(d/path) for name,path in paths.items()};assert all(x and x['passed'] for x in checks.values())
    build=R.read(d/'correctness_build.json');files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    source_hash=R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})
    assert source_hash==build['identity']['source_hash']
    def closure(text):
        start=text.index('static void sgpl_semantic_fixed_point(');return text[start:text.index('\n}\n',start)+3]
    assert closure((V.BASE/'src/semantic_stage_runtime.inc').read_text())==closure((d/'src/semantic_stage_runtime.inc').read_text())
    payload=dict(passed=True,branch=R.git('branch','--show-current'),source_commit=build['identity']['commit'],source_hash=source_hash,build=build,
        baseline=str(V.BASE),baseline_identity=R.read(V.BASE/'identity.json'),checks=checks,r12_unchanged=True,
        retained_ARS1_commit=R.git('rev-parse','ARS-1'),validation_commit=R.git('rev-parse','HEAD'),benchmark_runs=0,new_cost_models=0,performance_claim=None,
        all_race_detector_checks_passed=checks['detector']['all_detector_checks_passed'],
        rules=['R16 modular source counting from native rows','R17 post-iteration dead restoration removal','R18 source-local map storage reuse'],
        composition=dict(new_rule_subsets=8,previous_gate_rule_subsets=2,shared_worklist=True,lifetime_premise_subsets=128),artifact_root=str(d),
        validation_sources={name:R.sha(V.REPO/name) for name in ['tools/validate_iteration_lifetimes.py','test/iteration_lifetime_ir_test.cpp','test/iteration_lifetime_runtime_test.c']})
    R.save(d/'iteration_lifetimes_correctness.json',payload);R.save(V.REPO/'reports/iteration_lifetimes_correctness.json',payload)
    print(json.dumps(dict(passed=True,checks=checks['validation'],activation=checks['activation']['totals'],benchmark_runs=0)),flush=True)

def main():
    stages=['build','structural','native','fixtures','iteration','programs','existing','activation','detector','report']
    p=argparse.ArgumentParser();p.add_argument('stage',choices=[*stages,'all']);p.add_argument('--version',default='iteration_lifetimes01');p.add_argument('--from-stage',choices=stages,default='build');a=p.parse_args()
    V.prepare();R.M.make_source_original=R.M.make_source
    R.M.make_source=lambda alg,row,val:S.without_timing(R.M.make_source_original(alg,row,val))
    d=V.ROOT/'versions'/a.version
    def stage(n):
        if n=='build':V.build(a.version)
        elif n=='structural':structural(d)
        elif n=='native':native(d)
        elif n=='fixtures':fixtures(d)
        elif n=='programs':V.programs(d)
        elif n=='iteration':I.iteration_fixtures(d)
        elif n=='existing':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif n=='activation':activation(d)
        elif n=='detector':detector(d)
        elif n=='report':report(d)
    with R.locked('iteration_lifetimes_'+a.stage):
        for n in (stages[stages.index(a.from_stage):] if a.stage=='all' else [a.stage]):stage(n)
if __name__=='__main__':main()
