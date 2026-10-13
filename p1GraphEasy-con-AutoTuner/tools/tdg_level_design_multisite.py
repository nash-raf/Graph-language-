#!/usr/bin/env python3
"""Record the three-sequential-site optimizer repair with identical bodies."""
import csv
import os
from pathlib import Path
import statistics
import subprocess
from tdg_level_design_audit import ensure_inputs

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'proof/tdg_level_design'

def main():
    ensure_inputs()
    prior = OUT/'pre_polish_runtime.c'
    if not prior.exists():
        subprocess.run(['patch','--silent','-o',str(prior),str(ROOT/'parallel_runtime.c'),
                        str(OUT/'pre_polish_from_final.patch')],check=True)
    for policy, source in (('pre_polish',prior),('current',ROOT/'parallel_runtime.c')):
        obj = OUT/f'{policy}.o'
        subprocess.run(['gcc','-O3','-fopenmp','-mavx2','-pthread','-I',str(ROOT),'-DBASELINE',
                        f'-DSGPL_RUNTIME_SOURCE="{source}"','-c',str(ROOT/'tools/tdg_level_design_probe.c'),
                        '-o',str(obj)],check=True)
        subprocess.run(['g++','-fopenmp','-pthread',str(obj),str(OUT/'bitmap.o'),'-lnlopt','-lm',
                        '-o',str(OUT/f'probe_{policy}')],check=True)
    env = dict(os.environ,SGPL_NUM_THREADS='4',OMP_NUM_THREADS='4')
    for key in ('SGPL_FORCE_WIDTHS','SGPL_TDG_DEBUG','SGPL_BUDGET_DEBUG','GRAPH_PARALLEL_DEBUG',
                'SGPL_TDG_PLAN_DUMP','SGPL_FORCE_DOALL_PARALLEL'):
        env.pop(key,None)
    rows = []
    for run in range(3):
        for policy in ('pre_polish','current') if run%2==0 else ('current','pre_polish'):
            result = subprocess.run([str(OUT/f'probe_{policy}'),'0','3','100000','16','3','0','12','1'],
                                    env=env,capture_output=True,text=True,check=True,timeout=40)
            row = result.stdout.strip().split(',')
            rows.append([policy,run,*row])
    with (OUT/'multisite.csv').open('w',newline='') as output:
        csv.writer(output).writerows(rows)
    times = {policy:statistics.median(float(r[9]) for r in rows if r[0]==policy)
             for policy in ('pre_polish','current')}
    with (OUT/'multisite_summary.txt').open('w') as report:
        for policy in times:
            chosen = [tuple(r[i] for i in (14,17,20)) for r in rows if r[0]==policy]
            report.write(f'{policy}: {times[policy]/1e6:.3f}ms, site widths={chosen}\n')
        report.write(f"Time reduction={(1-times['current']/times['pre_polish'])*100:.1f}%\n")
    print((OUT/'multisite_summary.txt').read_text())

if __name__ == '__main__':
    main()
