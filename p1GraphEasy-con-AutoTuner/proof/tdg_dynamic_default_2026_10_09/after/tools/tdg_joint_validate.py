#!/usr/bin/env python3
"""Validate the integrated dynamic default and the frozen benchmark reference."""
import argparse
import os
import subprocess
from tdg_joint_build import ROOT, BUILD, OUT, SNAP, run
from tdg_joint_benchmark import environment

def checked(args, report, **kwargs):
    result=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True,**kwargs)
    report.write('$ '+' '.join(map(str,args))+'\n'+result.stdout+result.stderr+'\n')
    report.flush()
    if result.returncode:
        raise RuntimeError(f'{args[0]} failed ({result.returncode}): '+result.stderr[-2000:])
    return result

def focused(report):
    obj=BUILD/'joint_test.o'; exe=BUILD/'joint_test'
    checked(['gcc','-O2','-mavx2','-fopenmp','-pthread','-I.','-c',
             'tools/tdg_joint_runtime_test.c','-o',obj],report)
    checked(['g++','-pthread','-fopenmp',obj,BUILD/'bitmap.o','-lnlopt','-lm','-o',exe],report)
    for p in (1,2,4,8):
        checked([exe],report,env=environment(p,'joint',False),timeout=60)
        print(f'PASS structural B={p}',flush=True)
    checked(['python3','tools/tdg_joint_oracle.py'],report,timeout=60)
    gpu=BUILD/'gpu.o'
    checked(['gcc','-O2','-pthread','-c','gpu_runtime.c','-o',gpu],report)
    checked(['gcc','-O2','-pthread','-c',
             'parallel_runtime.c','-o',BUILD/'joint.o'],report)
    checked(['gcc','-O2','-pthread','-c',
             'tdg_budget_test.c','-o',BUILD/'gate.o'],report)
    checked(['g++','-pthread','-fopenmp',BUILD/'gate.o',BUILD/'joint.o',gpu,BUILD/'bitmap.o',
             '-lnlopt','-lm','-o',BUILD/'gate'],report)
    for p in (1,2,4,8):
        checked([BUILD/'gate'],report,env=environment(p,'joint',False),timeout=60)
    env=environment(4,'joint',False);env['SGPL_NO_UNREGISTERED_POOL_SHARE']='1'
    checked([BUILD/'gate'],report,env=env,timeout=60)
    checked(['python3','tools/test_cost_model_equations.py'],report,timeout=30)
    # The append test carries bitmap stubs. Check against this exact reference,
    # not an older snapshot, if its pre-existing order assertion still fails.
    outputs=[]
    for name,source in [('current',ROOT/'parallel_runtime.c'),('reference',SNAP/'source/parallel_runtime.c')]:
        append=BUILD/f'append_{name}'
        checked(['gcc','-O2','-pthread','-fopenmp','-I.','test/parallel_append_runtime_test.c',
                 source,'-lnlopt','-lm','-o',append],report)
        result=subprocess.run([str(append)],cwd=ROOT,capture_output=True,text=True,timeout=30)
        report.write(f'Append {name}: exit={result.returncode}\n'+result.stdout+result.stderr)
        outputs.append((result.returncode,result.stdout,result.stderr))
    assert outputs[0]==outputs[1], 'Append behavior differs from frozen reference'
    report.write('PASS append behavior identical to frozen reference.\n')
    checked(['git','-c','core.autocrlf=true','diff','--check','--','parallel_runtime.c',
             'parallel_runtime.h','main.cpp','pdg.cpp','test/run_tdg_shared_graph_levels.sh'],report)

def generated(report):
    checked(['bash','test/build_tdg_validation.sh'],report,timeout=300)
    alias=BUILD/'reference_alias.o'
    checked(['gcc','-O2','-c','tools/tdg_ordinary_first_reference_alias.c','-o',alias],report)
    for name, flags, source, compiler in (
        ('dynamic','','parallel_runtime.c','dynamic'),
        ('reference','-Dsgpl_run_tdg_level=sgpl_run_tdg_level_ordinary_first_reference',
         str(SNAP/'source/parallel_runtime.c'),'dynamic'),
    ):
        env=dict(os.environ,SGPL_TDG_RUNTIME_FLAGS=flags,SGPL_TDG_RUNTIME_SOURCE=source,
                 SGPL_TDG_COMPILER_FLAGS=f'--tdg-scheduler={compiler}',
                 SGPL_TDG_EXTRA_RUNTIME_OBJECTS=str(alias) if name=='reference' else '')
        checked(['bash','test/run_tdg_shared_graph_levels.sh'],report,env=env,timeout=300)
        print(f'PASS generated sibling-graph fixtures, {name}',flush=True)
    checked(['bash','test/run_tdg_formal.sh'],report,timeout=300)
    checked(['python3','tools/tdg_joint_numeric_test.py'],report,timeout=120)

def sanitizers(report):
    obj=BUILD/'sanitized.o';exe=BUILD/'sanitized'
    checked(['gcc','-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined',
             '-pthread','-fopenmp','-I.','-c','tools/tdg_joint_runtime_test.c','-o',obj],report)
    checked(['g++','-pthread','-fopenmp','-fsanitize=address,undefined',obj,BUILD/'bitmap.o',
             '-lnlopt','-lm','-o',exe],report)
    env=environment(4,'joint',False)
    env['ASAN_OPTIONS']='detect_leaks=0:abort_on_error=1'
    env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
    checked([exe],report,env=env,timeout=60)
    print('PASS address/undefined behavior sanitizers',flush=True)

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--generated',action='store_true')
    parser.add_argument('--sanitizers',action='store_true')
    args=parser.parse_args()
    BUILD.mkdir(parents=True,exist_ok=True)
    name='generated_validation.txt' if args.generated else 'sanitizer_validation.txt' if args.sanitizers else 'validation.txt'
    with (OUT/name).open('w') as report:
        if args.generated: generated(report)
        elif args.sanitizers: sanitizers(report)
        else: focused(report)

if __name__=='__main__': main()
