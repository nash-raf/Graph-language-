#!/usr/bin/env python3
"""Iteration memory correctness only; no benchmark or warmup entry points."""
from pathlib import Path
import argparse, json, os, re, subprocess, sys
import validate_execution_composition as C
S=C.S; V=C.V; R=C.R
V.BASE=V.ROOT/'versions/composition01'
PROBE=Path('/home/kazi/p1_iteration_memory_inspect')

def probe():
    out=PROBE;out.mkdir(exist_ok=True)
    R.checked(['g++','-O0','-g0','-I.',V.REPO/'test/iteration_memory_ir_test.cpp',
        *R.cfg('--cxxflags'),*R.cfg('--ldflags'),*R.cfg('--libs','all'),*R.cfg('--system-libs'),
        '-o',out/'ir-test'],V.REPO,out/'ir-build.log')
    R.checked([out/'ir-test',out/'pre.ll'],V.REPO,out/'ir-test.log')
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

def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['probe','native','build','programs','focused','existing','custom','activation','report']);p.add_argument('--version',default='iteration_memory02');a=p.parse_args()
    V.prepare();R.M.make_source_original=R.M.make_source
    R.M.make_source=lambda alg,row,val:S.without_timing(R.M.make_source_original(alg,row,val))
    d=V.ROOT/'versions'/a.version
    with R.locked('iteration_memory_'+a.stage):
        if a.stage=='probe':probe()
        elif a.stage=='native':native(PROBE/'native')
        elif a.stage=='build':V.build(a.version)
        elif a.stage=='programs':V.programs(d)
        elif a.stage=='focused':S.focused(d)
        elif a.stage=='existing':S.Old.focused(d);S.F.frontier_contracts(d);V.contracts(d)
        elif a.stage=='custom':S.custom(d);C.weighted_custom(d)
        elif a.stage=='activation':S.activation_checks(d)
        else:raise NotImplementedError('complete evidence before report')
if __name__=='__main__':main()
