#!/usr/bin/env python3
"""ASan/UBSan verification of mixed adapter/runtime using saved SGPL objects."""
import argparse
import csv
import json
import os
from pathlib import Path
import resource
import mixed
import run as sweep

def main(out):
    meta=json.loads((out/'mixed_manifest.json').read_text())
    case=next(c for c in meta['cases'] if c['name']=='mixed_short_ordinary')
    build=Path(case.get('build_directory',meta['build_directory'])); flags=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
    obj=build/'mixed_sanitized.o'; exe=build/'mixed_sanitized'
    sweep.run(['gcc','-O1','-g','-pthread','-fopenmp','-iquote',sweep.ROOT,*flags,
               f'-DSWEEP_RUNTIME_SOURCE="{build / "observed_runtime.c"}"','-c',sweep.HERE/'mixed_driver.c','-o',obj])
    sdk=[build/(name+'.o') for name in ['runtime.c','autotuner_runtime.c','graph_mutation_runtime.c','gpu_runtime.c',
         'semiring_runtime.c','roaring_bitmap.cpp','graph_loader_runtime.cpp','graph_runtime.cpp']]
    sweep.run(['g++','-no-pie','-pthread','-fopenmp',*flags,build/(case['name']+'.repeat.o'),obj,*sdk,
               '-Wl,--wrap=sgpl_run_tdg_level','-Wl,--wrap=sgpl_should_parallelize_doall','-Wl,--wrap=printf',
               '-ldl','-lnlopt','-lm','-o',exe])
    configs=mixed.configs(case,4)
    data=mixed.header(case)+''.join(' '.join(map(str,c))+'\n' for c in configs)
    for policy in ('ordinary-first','dynamic'):
        env={k:v for k,v in os.environ.items() if not k.startswith(('SGPL_FORCE','SGPL_JOINT','SGPL_TDG','SGPL_BUDGET','GRAPH_PARALLEL'))}
        env.update(SGPL_NUM_THREADS='4',OMP_NUM_THREADS='4',SWEEP_SAMPLES='3',
                   ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
        result=sweep.run([exe,int(policy=='dynamic'),-2,1,1,0,1,*case['n'],case['depth'][0]*100+case['depth'][1]],
                         env=env,input=data,capture_output=True,text=True,timeout=180)
        rows=list(csv.reader(result.stdout.splitlines())); assert len(rows)==3*(1+len(configs))
        assert all(line.startswith('MIX_VALIDATION,') for line in result.stderr.splitlines()),result.stderr
        (out/f'mixed_sanitizer.{policy}.txt').write_text(result.stderr)
    (out/'mixed_sanitizer_validation.txt').write_text(
        f'PASS: ASan/UBSan mixed adapter and runtime, both native policies and all {len(configs)} B4 forced schedules.\n'
        'Both overloaded ordinary-first phase ordering and serial SGPL task results asserted.\n'
        'SGPL-generated and SDK objects are not sanitizer instrumented. Leak detection disabled for existing generated dispatch environments and persistent pools.\n')
    print((out/'mixed_sanitizer_validation.txt').read_text())

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__); p.add_argument('out',type=Path)
    soft,hard=resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK,(64*1024*1024 if hard<0 else min(64*1024*1024,hard),hard))
    main(p.parse_args().out.resolve())
