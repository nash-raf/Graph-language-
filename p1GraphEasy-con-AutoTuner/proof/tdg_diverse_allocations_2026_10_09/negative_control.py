"""Verify that the new bounded-search regression detects the saved old search."""
from pathlib import Path
import os
import subprocess

OUT=Path(__file__).resolve().parent
ROOT=OUT.parents[1]
obj=OUT/'build/old_search_regression.o'
exe=OUT/'build/old_search_regression'
commands=[['gcc','-O2','-mavx2','-fopenmp','-pthread','-I.',
           f'-DSGPL_RUNTIME_SOURCE="{OUT}/before/source/parallel_runtime.c"',
           '-c','tools/tdg_joint_runtime_test.c','-o',str(obj)],
          ['g++','-pthread','-fopenmp',str(obj),str(OUT/'build/bitmap.o'),
           '-lnlopt','-lm','-o',str(exe)]]
for command in commands: subprocess.run(command,cwd=ROOT,check=True,capture_output=True,text=True)
env=dict(os.environ,SGPL_NUM_THREADS='6',OMP_NUM_THREADS='6')
result=subprocess.run([str(exe)],cwd=ROOT,env=env,capture_output=True,text=True)
assert result.returncode!=0 and 'diverse.widths[0][0]==5' in result.stderr,result
(OUT/'negative_control.txt').write_text(result.stderr+'\nPASS: the saved old search fails the new wide-allocation regression.\n')
print('PASS negative control: old search misses the beneficial wide allocation.')
