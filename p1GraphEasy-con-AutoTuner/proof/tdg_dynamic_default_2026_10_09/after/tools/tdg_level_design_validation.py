#!/usr/bin/env python3
"""Build/run focused correctness checks without linking the layout autotuner."""
import json
import hashlib
import difflib
import os
from pathlib import Path
import subprocess
from tdg_level_design_audit import ensure_inputs

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'proof/tdg_level_design'

def run(command, **kwargs):
    return subprocess.run(command, check=True, text=True, capture_output=True, **kwargs)

def main():
    ensure_inputs()
    if not (OUT/'gpu.o').exists():
        run(['gcc','-O3','-fopenmp','-mavx2','-pthread','-c',str(ROOT/'gpu_runtime.c'),'-o',str(OUT/'gpu.o')])
    runtime = OUT/'runtime.o'
    run(['gcc','-O3','-fopenmp','-mavx2','-pthread','-Wall','-Wextra','-c',str(ROOT/'parallel_runtime.c'),'-o',str(runtime)])
    run(['gcc','-O3','-fopenmp','-mavx2','-pthread','-c',str(ROOT/'tdg_budget_test.c'),'-o',str(OUT/'gate.o')])
    run(['g++','-fopenmp','-pthread',str(OUT/'gate.o'),str(runtime),
         str(OUT/'bitmap.o'),str(OUT/'gpu.o'),
         '-lnlopt','-lm','-o',str(OUT/'gate')])
    run(['gcc','-O3','-fopenmp','-mavx2','-pthread','-I',str(ROOT),'-c',
         str(ROOT/'tools/tdg_level_design_probe.c'),'-o',str(OUT/'current.o')])
    run(['g++','-fopenmp','-pthread',str(OUT/'current.o'),str(OUT/'bitmap.o'),
         '-lnlopt','-lm','-o',str(OUT/'probe_current')])
    with (OUT/'validation.txt').open('w') as report:
        for p in (1,2,4,8):
            env = dict(os.environ,SGPL_NUM_THREADS=str(p),OMP_NUM_THREADS=str(p))
            for key in ('SGPL_FORCE_WIDTHS','GRAPH_PARALLEL_DEBUG','SGPL_TDG_DEBUG','SGPL_BUDGET_DEBUG',
                        'SGPL_TDG_PLAN_DUMP','SGPL_FORCE_DOALL_PARALLEL','SGPL_FORCE_DOACROSS_PARALLEL',
                        'SGPL_NO_UNREGISTERED_POOL_SHARE'):
                env.pop(key,None)
            for program,args in (('gate',[]),('probe_current',['test'])):
                result = run([str(OUT/program),*args],env=env,timeout=40)
                report.write(f'P={p} {program}\n'+result.stdout)
                report.flush()
                print(f'P={p} {program}: '+result.stdout.splitlines()[-1],flush=True)
            if p==4:
                result = run([str(OUT/'gate')],env=dict(env,SGPL_NO_UNREGISTERED_POOL_SHARE='1'),timeout=40)
                report.write('P=4 sharing disabled\n'+result.stdout)
                print('P=4 sharing disabled: '+result.stdout.splitlines()[-1],flush=True)
        run(['gcc','-O2','-pthread',str(ROOT/'test/parallel_append_runtime_test.c'),
             str(runtime),'-lnlopt','-lm','-fopenmp','-o',str(OUT/'gate_append')])
        result = subprocess.run([str(OUT/'gate_append')],capture_output=True,text=True,timeout=40)
        report.write('Current append test:\n'+result.stdout+result.stderr)
        if result.returncode:
            run(['gcc','-O2','-pthread','-I',str(ROOT),str(ROOT/'test/parallel_append_runtime_test.c'),
                 str(OUT/'baseline_runtime.c'),'-lnlopt','-lm','-o',str(OUT/'gate_append_baseline')])
            baseline = subprocess.run([str(OUT/'gate_append_baseline')],capture_output=True,text=True,timeout=40)
            report.write('Captured baseline append test:\n'+baseline.stdout+baseline.stderr)
            assert baseline.returncode == result.returncode and baseline.stderr == result.stderr, \
                'Append test failure differs from pre-change runtime'
            report.write('Append failure is identical on the pre-change runtime.\n')
            print('Append test: identical pre-existing baseline failure: '+result.stderr.strip(),flush=True)
        else:
            print(result.stdout.strip(),flush=True)
        result = run(['python3',str(ROOT/'tools/test_cost_model_equations.py')],timeout=40)
        report.write(result.stdout+result.stderr)
        print(result.stderr.strip(),flush=True)
        result = run(['git','-c','core.autocrlf=true','diff','--check','--',
                      'parallel_runtime.c','parallel_runtime.h','tdg_budget_test.c'],cwd=ROOT)
        report.write('git diff --check (Windows CRLF normalization, changed source files): PASS\n')
    head = run(['git','rev-parse','HEAD'],cwd=ROOT).stdout.strip()
    path = run(['git','ls-files','--full-name','parallel_runtime.c'],cwd=ROOT).stdout.strip()
    original = run(['git','show',f'{head}:{path}'],cwd=ROOT).stdout
    baseline = (OUT/'baseline_runtime.c').read_text()
    phased = (OUT/'phased_sum_runtime.c').read_text()
    for name,old,new in (('baseline_from_head.patch',original,baseline),
                         ('phased_sum_from_baseline.patch',baseline,phased)):
        (OUT/name).write_text(''.join(difflib.unified_diff(old.splitlines(keepends=True),
                            new.splitlines(keepends=True),fromfile='parallel_runtime.c',tofile='parallel_runtime.c')))
    metadata = {'head_sha':head,'runtime_git_path':path,
                'sha256':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
                    for p in (ROOT/'parallel_runtime.c',OUT/'baseline_runtime.c',OUT/'phased_sum_runtime.c',
                              ROOT/'tools/tdg_level_design_probe.c')},
                'environment':run(['lscpu']).stdout,
                'compiler':run(['gcc','--version']).stdout.splitlines()[0]}
    (OUT/'metadata.json').write_text(json.dumps(metadata,indent=2))

if __name__ == '__main__':
    main()
