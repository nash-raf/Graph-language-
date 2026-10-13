#!/usr/bin/env python3
"""Run related resource/model regressions without changing earlier proof outputs."""
import os
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'proof/doacross_cost_worker_work_2026_10_09'
BUILD=OUT/'build'


def main():
    with (OUT/'regressions.txt').open('w') as log:
        def run(args,**kwargs):
            log.write('$ '+' '.join(map(str,args))+'\n');log.flush()
            return subprocess.run(list(map(str,args)),cwd=ROOT,check=True,stdout=log,
                                  stderr=subprocess.STDOUT,**kwargs)
        run(['gcc','-O2','-mavx2','-pthread','-fopenmp','-I.', '-c','tools/tdg_joint_runtime_test.c',
             '-o',BUILD/'joint_test.o'])
        run(['g++','-pthread','-fopenmp',BUILD/'joint_test.o',BUILD/'bitmap.o','-lnlopt','-lm',
             '-o',BUILD/'joint_test'])
        run(['gcc','-O2','-pthread','-c','gpu_runtime.c','-o',BUILD/'gpu.o'])
        run(['gcc','-O2','-pthread','-c','parallel_runtime.c','-o',BUILD/'runtime.o'])
        run(['gcc','-O2','-pthread','-c','tdg_budget_test.c','-o',BUILD/'budget_test.o'])
        run(['g++','-pthread','-fopenmp',BUILD/'budget_test.o',BUILD/'runtime.o',BUILD/'gpu.o',
             BUILD/'bitmap.o','-lnlopt','-lm','-o',BUILD/'budget_test'])
        for p in (1,2,4,8):
            env={k:v for k,v in os.environ.items() if not k.startswith(('SGPL_','OMP_','GRAPH_PARALLEL'))}
            env.update(SGPL_NUM_THREADS=str(p),OMP_NUM_THREADS=str(p))
            run([BUILD/'joint_test'],env=env,timeout=60)
            run([BUILD/'budget_test'],env=env,timeout=60)
            print('PASS scheduler/resource/budget',p,flush=True)
        run(['python3','-c',
             "import sys; from pathlib import Path; sys.path.insert(0,'tools'); "
             "import tdg_joint_oracle as m; "
             "m.BUILD=Path('proof/doacross_cost_worker_work_2026_10_09/build'); "
             "m.OUT=Path('proof/doacross_cost_worker_work_2026_10_09'); m.main()"],timeout=60)
        run(['python3','tools/test_cost_model_equations.py'],timeout=30)
        run(['git','-c','core.autocrlf=true','diff','--check','--',
             'parallel_runtime.c','parallel_runtime.h','tdg_scheduler.inc'])


if __name__=='__main__':main()
