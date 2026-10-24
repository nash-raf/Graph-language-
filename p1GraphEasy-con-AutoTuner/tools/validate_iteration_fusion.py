#!/usr/bin/env python3
"""R19 correctness only; frozen accepted baseline, exact bits, no timing."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,sys
import validate_iteration_lifetimes as L
I=L.I;C=L.C;S=L.S;V=L.V;R=L.R
V.BASE=V.ROOT/'versions/iteration_lifetimes02'

def structural(d):
    out=d/'fusion_structural';out.mkdir(exist_ok=True)
    for name in ['iteration_fusion_ir_test','iteration_lifetime_ir_test']:
        exe=out/name
        R.checked(['g++','-O0','-g0','-I.',V.REPO/'test'/(name+'.cpp'),*R.cfg('--cxxflags'),*R.cfg('--ldflags'),*R.cfg('--libs','all'),*R.cfg('--system-libs'),'-o',exe],V.REPO,out/(name+'.build.log'))
        R.checked([exe,'/home/kazi/p1_iteration_memory_inspect/pre.ll'],V.REPO,out/(name+'.log'))
        print((out/(name+'.log')).read_text()[-1000:],flush=True)
    C.worklist(d)
    R.save(out/'summary.json',dict(passed=True,rule_subsets=32,negative_premises=7,worklist_subsets=64,retained_lifetime_tests=True,benchmark_runs=0))

def native(d):
    out=d/'fusion_native';out.mkdir(exist_ok=True);checks=[]
    for mode,flags in [('normal',['-O2']),('sanitized',['-O1','-g','-no-pie','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
        exe=out/('check-'+mode)
        R.checked(['gcc',*flags,'-fopenmp','-pthread','-I.','-Itest',V.REPO/'test/iteration_fusion_runtime_test.c','autotuner_runtime.c','parallel_runtime.c','-lnlopt','-lm','-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create','-o',exe],V.REPO,out/(mode+'.build.log'))
        for width in [1,2,6]:
            env=R.M.env(workers=width);env.update(ASAN_OPTIONS='detect_leaks=0',SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1')
            R.checked(['bash','-c','ulimit -s 8192; exec "$@"','native',sys.executable,V.REPO/'test/physical_run.py',str(width),exe],V.REPO,out/f'{mode}.w{width}.log',env,timeout=1200)
            checks.append(dict(mode=mode,workers=width,passed=True));print('FUSION NATIVE PASS',mode,width,flush=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))

def fixtures(d):
    out=d/'fusion_fixtures';out.mkdir(exist_ok=True)
    edge=out/'edges.txt';edge.write_text('0 2\n1 2\n1 2\n2 2\n4 4\n')
    helper=out/'bits.o';R.checked(['gcc','-O2','-c',V.REPO/'tools/p1_campaign/exact_bits.c','-o',helper],out,out/'bits-build.log');checks=[]
    for name in ['plain','one','nan','infinity','signed_zero','mixed_gate','mutable_gate','kernel_read','empty','observable_round']:
      empty=out/'empty.txt';empty.write_text('');edges=empty if name=='empty' else edge
      init='current[v] = 1.0;'
      extra='current[0] = 0.0/0.0;' if name=='nan' else 'current[0] = 1.0/0.0; current[1] = -1.0/0.0;' if name=='infinity' else 'current[0] = -0.0; current[1] = 0.0;' if name=='signed_zero' else ''
      predicate='current[u] > 0.0' if name=='mutable_gate' else 'out_count[u] > 0 && u > 0' if name=='mixed_gate' else 'out_count[u] > 0'
      contribution='contribution + current[v]' if name=='kernel_read' else 'contribution'
      source=f'''graph G {{ directed: true; edges: file "{edges}"; }};
int n = numVertices(G); real current[n]; real next[n]; int out_count[n];
for v in G.nodes {{ {init} next[v] = -0.0; out_count[v] = 0; }}
{extra}
for u in G.nodes {{ for v in G.neighbors(u) {{ out_count[u] = out_count[u] + 1; }} }}
real seed = -0.0; real damp = 0.85; int round = 0; int limit = {1 if name=='one' else 3};
while (round < limit) {{ round = round + 1;
 for v in G.nodes {{ next[v] = seed; }}
 for u in G.nodes {{ if ({predicate}) {{ real contribution = damp * current[u] / out_count[u]; for v in G.neighbors(u) {{ next[v] = next[v] + ({contribution}); }} }} }}
 {'print next[0];' if name=='observable_round' else ''}
 for v in G.nodes {{ current[v] = next[v]; }}
}}
print "VALUES_BEGIN"; int z = 0; while (z < n) {{ print current[z]; print next[z]; z = z + 1; }} print round; print "VALUES_END";
'''
      reference=None;refbits=None
      for gate in [False,True]:
       for subset in range(8):
        rules='R1,R2,R3,R16'+(',R13' if gate else '')+''.join(',R'+str(r) for i,r in enumerate([17,18,19]) if subset&(1<<i))
        case=out/f'{name}_gate{int(gate)}_{subset}';case.mkdir(exist_ok=True)
        for ll in (d/'src').glob('*.ll'):
            if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
        (case/'source.graph').write_text(source);env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1',SGPL_EXEC_SIMPLIFY_RULES=rules)
        R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'source.ll'),case/'source.graph'],case,case/'compile.log',env)
        R.checked(['g++','-O3','-no-pie','-fopenmp','-pthread',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],helper,'-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-ldl','-lnlopt','-o',case/'check'],case,case/'link.log')
        for width in [1,2,6]:
         for disabled in [False,True]:
          env=R.M.env(workers=width);bits=case/f'w{width}.{disabled}.bits';env.update(SGPL_BITS_OUTPUT=str(bits),SGPL_SEMANTIC_STAGE_STATS='1')
          if disabled:env['SGPL_NO_ITERATION_FUSION']='1'
          run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py',str(width),case/'check'],cwd=case,env=env,capture_output=True,text=True,timeout=90)
          (case/f'w{width}.{disabled}.out').write_text(run.stdout);(case/f'w{width}.{disabled}.err').write_text(run.stderr)
          assert run.returncode==0,(name,subset,gate,width,disabled,run.stderr[-2000:])
          text=run.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split();raw=bits.read_bytes()
          if reference is None:reference=text;refbits=raw
          assert text==reference and raw==refbits,(name,subset,gate,width,disabled,'exact mismatch')
          wanted=gate and bool(subset&4) and name not in ['mixed_gate','mutable_gate','kernel_read','observable_round'] and not disabled
          assert ('[iteration-fusion] R19 activate' in run.stderr)==wanted,(name,subset,gate,width,disabled,run.stderr[-2000:])
          checks.append(dict(fixture=name,subset=subset,R13=gate,workers=width,disabled=disabled,passed=True))
      print('FUSION FIXTURE PASS',name,flush=True)
    R.save(out/'summary.json',dict(passed=True,checks=checks,benchmark_runs=0))

def activation(d):
    out=d/'fusion_activation';out.mkdir(exist_ok=True);checks=[];totals={a:0 for a in R.ALGS}
    for alg in R.ALGS:
      for row in R.M.ROWS:
        case=d/'cases'/alg/row['graph'].removesuffix('.edges');env=R.M.env(workers=6);env['SGPL_SEMANTIC_STAGE_STATS']='1'
        run=subprocess.run([sys.executable,V.REPO/'test/physical_run.py','6',case/'validate'],cwd=case,env=env,capture_output=True,text=True,timeout=190)
        assert run.returncode==0,(alg,row['graph'],run.stderr[-1500:]);active='[iteration-fusion] R19 activate' in run.stderr
        assert active==(alg=='pagerank'),(alg,row['graph'],run.stderr[-2000:]);totals[alg]+=active
        log=out/alg/row['graph'].removesuffix('.edges');log.parent.mkdir(exist_ok=True,parents=True);log.with_suffix('.err').write_text(run.stderr)
        checks.append(dict(algorithm=alg,graph=row['graph'],R19=active,passed=True))
    R.save(out/'summary.json',dict(passed=True,totals=totals,checks=checks,benchmark_runs=0));print('ACTIVATION',totals,flush=True)

def detector(d):
    import check_cc_road_races as T
    T.FROZEN=d;T.OUT=d/'fusion_tsan';T.OUT.mkdir(exist_ok=True)
    T.DISABLE_ENV='SGPL_NO_ITERATION_FUSION';T.EXTRA_ENV={'SGPL_SEMANTIC_STAGE_STATS':'1'}
    T.GRAPHS=['lifetime-pagerank','lifetime-sssp']
    for name,alg in [('lifetime-pagerank','pagerank'),('lifetime-sssp','sssp')]:
        folder=d/'cases/cc'/name;folder.mkdir(exist_ok=True,parents=True);source=d/'cases'/alg/'dimacs/DSJC1000-5'
        shutil.copy2(source/'validate.ll',folder/'validate.ll')
        for width in [1,2,6]:shutil.copy2(source/f'baseline_w{width}.out',folder/f'enabled_w{width}.out')
    T.build();T.control();T.check(serial_openmp=True)
    result=R.read(T.OUT/'graph_pool_only_summary.json');control=R.read(T.OUT/'positive_control.json')
    assert all(x['status'] in ['passed','startup_unavailable','empty_launch_crash'] for x in result['checks'])
    R.save(T.OUT/'summary.json',dict(passed=True,all_detector_checks_passed=result['all_passed'],checks=result,positive_control=control,benchmark_runs=0))

def report(d):
    paths={'structural':'fusion_structural/summary.json','native':'fusion_native/summary.json','fixtures':'fusion_fixtures/summary.json','iteration':'iteration_fixtures/summary.json','validation':'validation/summary.json','activation':'fusion_activation/summary.json','detector':'fusion_tsan/summary.json','existing':'contracts/existing.json','workers':'contracts/worker_pool.json','fullgraph':'contracts/simplification.json','frontier':'frontier_contracts/summary.json','relaxation':'relax_contracts/summary.json'}
    checks={name:R.read(d/path) for name,path in paths.items()};assert all(x and x['passed'] for x in checks.values())
    build=R.read(d/'correctness_build.json');files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]+[p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    source_hash=R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files});assert source_hash==build['identity']['source_hash']
    payload=dict(passed=True,branch=R.git('branch','--show-current'),source_commit=build['identity']['commit'],source_hash=source_hash,build=build,baseline=str(V.BASE),baseline_identity=R.read(V.BASE/'identity.json'),checks=checks,retained_ARS1_commit=R.git('rev-parse','ARS-1'),benchmark_runs=0,new_cost_models=0,rule='R19 exact producer-map fusion',performance_claim=None)
    R.save(d/'iteration_fusion_correctness.json',payload);R.save(V.REPO/'reports/iteration_fusion_correctness.json',payload)
    print(json.dumps(dict(passed=True,checks=checks['validation'],activation=checks['activation']['totals'])),flush=True)

def main():
    stages=['build','structural','native','fixtures','iteration','programs','existing','activation','detector','report']
    p=argparse.ArgumentParser();p.add_argument('stage',choices=[*stages,'all']);p.add_argument('--version',default='iteration_fusion01');p.add_argument('--from-stage',choices=stages,default='build');a=p.parse_args()
    V.prepare();original=R.M.make_source;R.M.make_source=lambda alg,row,val:S.without_timing(original(alg,row,val));d=V.ROOT/'versions'/a.version
    def stage(n):
        if n=='build':V.build(a.version)
        elif n=='structural':structural(d)
        elif n=='native':native(d)
        elif n=='fixtures':fixtures(d)
        elif n=='iteration':I.iteration_fixtures(d)
        elif n=='programs':V.programs(d,'SGPL_NO_ITERATION_FUSION')
        elif n=='existing':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif n=='activation':activation(d)
        elif n=='detector':detector(d)
        elif n=='report':report(d)
    with R.locked('iteration_fusion_'+a.stage):
        for n in (stages[stages.index(a.from_stage):] if a.stage=='all' else [a.stage]):stage(n)
if __name__=='__main__':main()
