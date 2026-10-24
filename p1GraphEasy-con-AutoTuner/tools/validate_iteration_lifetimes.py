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

def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['build','structural','native','fixtures','programs','iteration','existing','activation','detector','all']);p.add_argument('--version',default='iteration_lifetimes01');a=p.parse_args()
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
        elif n=='activation':I.activation(d)
        elif n=='detector':I.detector(d)
    with R.locked('iteration_lifetimes_'+a.stage):
        for n in (['build','structural','native','fixtures','iteration','programs','existing','activation','detector'] if a.stage=='all' else [a.stage]):stage(n)
if __name__=='__main__':main()
