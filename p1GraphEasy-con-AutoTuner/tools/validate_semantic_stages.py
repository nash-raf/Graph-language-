#!/usr/bin/env python3
"""Correctness only: no benchmark, warmup, profiling or performance acceptance."""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import json
import re
import validate_execution_simplification as V
import validate_frontier_simplification as F
import validate_relax_simplification as Old

R=V.R
V.BASE=V.ROOT/'versions/relax06'

def without_timing(text):
    text=re.sub(r'^real bench_(?:start|stop)\d+\s*=\s*timer\(\);\s*$', '', text, flags=re.M)
    text=re.sub(r'^print "P1_COMPUTE_SECONDS";\s*$', '', text, flags=re.M)
    text=re.sub(r'^print bench_stop\d+ - bench_start\d+;\s*$', '', text, flags=re.M)
    assert 'timer()' not in text
    return text

def focused(d):
    out=d/'semantic_contracts';out.mkdir(exist_ok=True)
    R.checked(['g++','-O1','-g','-I.',V.REPO/'test/semantic_stage_ir_test.cpp',
               *R.cfg('--cxxflags'),*R.cfg('--ldflags'),*R.cfg('--libs','all'),*R.cfg('--system-libs'),
               '-o',out/'ir-test'],V.REPO,out/'ir-build.log')
    R.checked([out/'ir-test',out],V.REPO,out/'ir.log')
    for mode,flags in [('normal',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
        R.checked(['gcc',*flags,'-fopenmp','-pthread','-I.','-Itest','test/semantic_stage_runtime_test.c',
                   'autotuner_runtime.c','parallel_runtime.c','-lnlopt','-lm',
                   '-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create','-o',out/('runtime-'+mode)],V.REPO,out/(mode+'-build.log'))
        for width in [1,2,6]:
            env=R.M.env(workers=width);env['ASAN_OPTIONS']='detect_leaks=0'
            R.checked(['bash','-c','ulimit -s 8192; exec "$@"','native',os.sys.executable,V.REPO/'test/physical_run.py',str(width),out/('runtime-'+mode)],V.REPO,out/f'{mode}_w{width}.log',env)
    checks=int(re.search(r'PASS semantic IR checks=(\d+)',(out/'ir.log').read_text())[1])
    native=[int(re.search(r'semantic runtime: (\d+) cases, 0 failures',p.read_text())[1]) for p in out.glob('*_w*.log')]
    assert len(native)==6
    R.save(out/'summary.json',dict(passed=True,ir_checks=checks,rule_subsets=8,native_cases=sum(native),workers=[1,2,6],modes=['normal','ASan/UBSan'],benchmark_runs=0))
    print('FOCUSED PASS',flush=True)

def custom(d):
    out=d/'semantic_custom';out.mkdir(exist_ok=True)
    edge=out/'edges.txt';edge.write_text('0 3\n1 3\n2 3\n2 3\n')
    helper=out/'bits.o';R.checked(['gcc','-O2','-c',V.REPO/'tools/p1_campaign/exact_bits.c','-o',helper],out,out/'bits-build.log')
    results=[]
    for kind in ['sub','add','fadd','fsub','min','max','fadd_nan','fsub_inf','fadd_zero']:
      for order in ['full','permuted','duplicates']:
        real=kind.startswith('f');ty='real' if real else 'int'
        op=kind.split('_')[0]
        values=['10000000000000000.0','-10000000000000000.0','1.0','0.0'] if real else ['7','3','1','2147483647']
        if kind.endswith('_nan'):values=['0.0 / 0.0','1.0','2.0','3.0']
        if kind.endswith('_inf'):values=['1.0 / 0.0','-1.0 / 0.0','1.0','0.0']
        if kind.endswith('_zero'):values=['-0.0','0.0','-0.0','-0.0']
        init=''.join(f'id[{i}] = {v};\n' for i,v in enumerate(values))
        seq={'full':[0,1,2,3],'permuted':[0,2,1,3],'duplicates':[2,0,1,2,3]}[order]
        init+=''.join(f'frontier[{i}] = {v};\n' for i,v in enumerate(seq))
        if op in ['sub','fsub']:body='id[v] = id[v] - id[u];'
        elif op in ['add','fadd']:body='id[v] = id[v] + id[u];'
        elif kind=='min':body='if (id[u] < id[v]) { id[v] = id[u];'
        elif kind=='max':body='if (id[u] > id[v]) { id[v] = id[u];'
        elif kind=='or':body='if ((id[v] | id[u]) != id[v]) { id[v] = id[v] | id[u];'
        else:body='if ((id[v] & id[u]) != id[v]) { id[v] = id[v] & id[u];'
        body+=' next_frontier[next_size] = v; next_size = next_size + 1;'
        if kind in ['min','max','or','and']:body+=' }'
        source=f'''graph G {{ directed: true; edges: file "{edge}"; }};
int n = numVertices(G);
int storage8 = 8; int storage64 = 64;
{ty} id[n]; int frontier[storage8]; int next_frontier[storage64];
{init}
int frontier_size = {len(seq)}; int next_size = 0;
while (frontier_size > 0) {{
 next_size = 0; int k = 0;
 while (k < frontier_size) {{ int u = frontier[k]; for v in G.neighbors(u) {{ {body} }} k = k + 1; }}
 swap(frontier, next_frontier); frontier_size = next_size;
}}
print "VALUES_BEGIN"; int z = 0; while (z < n) {{ print id[z]; z = z + 1; }} print "VALUES_END";
'''
        reference=None;reference_bits=None
        for rules in ['none','R10','R11','R12','all']:
          case=out/f'{kind}_{order}_{rules}';case.mkdir(exist_ok=True)
          for ll in (d/'src').glob('*.ll'):
            if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
          (case/'source.graph').write_text(source)
          env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1',SGPL_EXEC_SIMPLIFY_RULES=rules)
          R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'source.ll'),case/'source.graph'],case,case/'compile.log',env)
          R.checked(['g++','-O3','-no-pie','-fopenmp','-pthread',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],helper,
                     '-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-ldl','-lnlopt','-o',case/'check'],case,case/'link.log')
          for width in [1,2,6]:
            env=R.M.env(workers=width);env['SGPL_SEMANTIC_STAGE_STATS']='1'
            env['SGPL_BITS_OUTPUT']=str(case/f'w{width}.bits')
            p=subprocess.run([str(case/'check')],cwd=case,env=env,capture_output=True,text=True,timeout=60)
            (case/f'w{width}.out').write_text(p.stdout);(case/f'w{width}.err').write_text(p.stderr)
            assert p.returncode==0,(case,width,p.stderr[-1500:])
            vals=p.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
            raw=(case/f'w{width}.bits').read_bytes()
            if rules=='none':
              if reference is None:reference=vals;reference_bits=raw
              assert vals==reference
            else:assert vals==reference,(case,width,vals,reference)
            assert raw==reference_bits,(case,width,'exact floating bits differ')
            results.append(dict(kind=kind,order=order,rules=rules,workers=width,passed=True,activation=re.findall(r'\[semantic-stage\] R(\d+) complete',p.stderr)))
    R.save(out/'summary.json',dict(passed=True,checks=results,benchmark_runs=0))
    print('CUSTOM PASS',len(results),flush=True)

def detector(d):
    import check_cc_road_races as T
    T.FROZEN=d;T.OUT=d/'semantic_tsan';T.OUT.mkdir(exist_ok=True)
    T.DISABLE_ENV='SGPL_NO_SEMANTIC_STAGE';T.EXTRA_ENV={'SGPL_SEMANTIC_STAGE_STATS':'1'}
    T.GRAPHS=['road/road-usroads-48','road/road-luxembourg-osm','socfb/socfb-Simmons81',
              'semantic-fadd-row','semantic-fadd-permuted']
    for name,source in [('semantic-fadd-row','fadd_full_R11'),('semantic-fadd-permuted','fadd_permuted_R10')]:
        folder=d/'cases/cc'/name;folder.mkdir(exist_ok=True)
        shutil.copy2(d/'semantic_custom'/source/'source.ll',folder/'validate.ll')
        ref=d/'semantic_custom'/('fadd_full_none' if name.endswith('row') else 'fadd_permuted_none')
        for width in [1,2,6]:shutil.copy2(ref/f'w{width}.out',folder/f'enabled_w{width}.out')
    T.build();T.control();T.check(serial_openmp=True)
    checks=R.read(T.OUT/'graph_pool_only_summary.json');positive=R.read(T.OUT/'positive_control.json')
    assert checks['all_passed'] and positive['passed']
    R.save(T.OUT/'semantic_summary.json',dict(passed=True,checks=checks,positive_control=positive,
             caveat='System libgomp implicit-barrier visibility was isolated with OMP_NUM_THREADS=1; SGPL graph worker budgets remained 1/2/6. No suppressions.',benchmark_runs=0))

def activation_checks(d):
    out=d/'semantic_activation';out.mkdir(exist_ok=True);results=[]
    for alg,rule in [('cc','12'),('sssp','10')]:
      for row in R.M.ROWS:
        folder=d/'cases'/alg/row['graph'].removesuffix('.edges')
        env=R.M.env(workers=6);env['SGPL_SEMANTIC_STAGE_STATS']='1'
        p=subprocess.run([os.sys.executable,V.REPO/'test/physical_run.py','6',folder/'validate'],cwd=folder,env=env,capture_output=True,text=True,timeout=180)
        stem=out/alg/row['graph'].removesuffix('.edges');stem.parent.mkdir(parents=True,exist_ok=True)
        stem.with_suffix('.out').write_text(p.stdout);stem.with_suffix('.err').write_text(p.stderr)
        assert p.returncode==0,(alg,row['graph'],p.stderr[-1000:])
        values=p.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
        reference=(folder/'baseline_w6.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
        assert values==reference,(alg,row['graph'])
        active=re.findall(r'\[semantic-stage\] R(\d+) complete',p.stderr)
        assert rule in active,(alg,row['graph'],'certified rule did not activate')
        if rule=='12':assert active==['12'],(row['graph'],active)
        results.append(dict(algorithm=alg,graph=row['graph'],rule='R'+rule,invocations=active,exact_array=True))
    R.save(out/'summary.json',dict(passed=True,checks=results,benchmark_runs=0))
    print('RUNTIME ACTIVATION PASS',len(results),flush=True)

def report(d):
    build=R.read(d/'correctness_build.json');validation=R.read(d/'validation/summary.json')
    focus=R.read(d/'semantic_contracts/summary.json');customs=R.read(d/'semantic_custom/summary.json')
    tsan=R.read(d/'semantic_tsan/semantic_summary.json')
    assert all(x['passed'] for x in [validation,focus,customs,tsan])
    files=[p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (V.REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    import hashlib
    assert R.key({str(p.relative_to(V.REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})==build['identity']['source_hash']
    activation={alg:{rule:sum(f'[exec-simplify] {rule} activate' in p.read_text() for p in (d/'cases'/alg).rglob('compile.log')) for rule in ['R10','R11','R12']} for alg in R.ALGS}
    assert activation['cc']['R12']==61 and activation['sssp']['R10']==61 and activation['sssp']['R12']==0,activation
    assert all(x['benchmark_runs']==0 for x in [validation,focus,customs,tsan])
    old={name:R.read(d/'contracts'/f'{name}.json') for name in ['simplification','existing','worker_pool']}
    assert all(x['passed'] for x in old.values())
    arrays=0
    for alg in R.ALGS:
      for row in R.M.ROWS:
       folder=d/'cases'/alg/row['graph'].removesuffix('.edges')
       for width in [1,2,6]:
        values=[(folder/f'{mode}_w{width}.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split() for mode in ['baseline','enabled','disabled']]
        assert values[0]==values[1]==values[2],(alg,row['graph'],width)
        arrays+=1
    runtime=R.read(d/'semantic_activation/summary.json');assert runtime['passed']
    payload=dict(branch=R.git('branch','--show-current'),source_commit=build['identity']['commit'],
       source_hash=build['identity']['source_hash'],build=build,previous_build=str(V.BASE),
       previous_identity=R.read(V.BASE/'identity.json'),activation_graphs=activation,validation=validation,
       focused=focus,custom=customs,detector=tsan,existing_contracts=old,runtime_activation=runtime,exact_array_groups=arrays,
       previous_frontier_contracts=R.read(d/'frontier_contracts/summary.json'),
       previous_relax_contracts=R.read(d/'relax_contracts/summary.json'),
       new_cost_models=0,benchmark_runs=0,performance_claim=None,artifact_root=str(d),
       original_ARS1_unchanged=R.git('rev-parse','ARS-1')=='2b46cb5ded54132e98dc052bd2caf548d1acfcb4',
       public_context_header_unchanged=not R.git('diff','2b46cb5ded54132e98dc052bd2caf548d1acfcb4','--','autotuner_runtime.h'))
    payload['test_sources']={name:R.sha(V.REPO/'test'/name) for name in ['semantic_stage_ir_test.cpp','semantic_stage_runtime_test.c']}
    payload['validation_tool_sha256']=R.sha(Path(__file__))
    R.save(d/'semantic_correctness_report.json',payload)
    R.save(V.REPO/'reports/semantic_stage_simplification_correctness.json',payload)
    print(json.dumps(dict(passed=True,activation=activation,validation=validation,focused=focus,benchmark_runs=0),indent=2),flush=True)

def compile_case(d,graph,algorithm='cc'):
    case=d/'semantic_probe'/algorithm/Path(graph).stem
    case.mkdir(parents=True,exist_ok=True)
    for ll in (d/'src').glob('*.ll'):
        if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
    source=case/'validate.graph'
    source.write_text(without_timing((V.BASE/'cases'/algorithm/graph/'validate.graph').read_text()))
    env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1')
    R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'validate.ll'),source],case,case/'compile.log',env)
    R.checked(['g++','-O3','-fopenmp','-no-pie',case/'program.o',*[d/(n+'.o') for n in R.OBJECTS],'-pthread','-ldl','-lnlopt','-o',case/'validate'],case,case/'link.log')
    for width in [1,2,6]:
        for mode in ['enabled','disabled']:
            env=R.M.env(workers=width);env['SGPL_SEMANTIC_STAGE_STATS']='1'
            if mode=='disabled':env['SGPL_NO_SEMANTIC_STAGE']='1'
            p=subprocess.run([str(case/'validate')],cwd=case,env=env,capture_output=True,text=True,timeout=180)
            (case/f'{mode}_w{width}.out').write_text(p.stdout);(case/f'{mode}_w{width}.err').write_text(p.stderr)
            assert p.returncode==0,(graph,width,mode,p.stderr[-1500:])
            reference=(V.BASE/'cases'/algorithm/graph/f'enabled_w{width}.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
            values=p.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
            assert values==reference,(graph,width,mode)
    print('PROBE PASS',algorithm,graph,flush=True)
    print((case/'compile.log').read_text()[:2000],flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['probe','programs','focused','custom','contracts','detector','activation','report','all']);p.add_argument('--version',required=True);args=p.parse_args()
    V.prepare();d=V.ROOT/'versions'/args.version
    maker=R.M.make_source
    R.M.make_source=lambda algorithm,row,validation:without_timing(maker(algorithm,row,validation))
    with R.locked('semantic_'+args.stage):
        if args.stage=='probe':
            compile_case(d,'road/road-usroads-48')
            compile_case(d,'road/road-usroads-48','sssp')
        elif args.stage=='focused':focused(d)
        elif args.stage=='custom':custom(d)
        elif args.stage=='contracts':Old.focused(d);F.frontier_contracts(d);V.contracts(d)
        elif args.stage=='detector':detector(d)
        elif args.stage=='activation':activation_checks(d)
        elif args.stage=='report':report(d)
        elif args.stage=='all':
            V.programs(d);focused(d);custom(d);Old.focused(d);F.frontier_contracts(d);V.contracts(d);detector(d);activation_checks(d);report(d)
        else:V.programs(d)
