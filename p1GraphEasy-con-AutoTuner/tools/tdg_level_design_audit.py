#!/usr/bin/env python3
"""Run the same public TDG workloads under captured and current policies."""
import argparse
import csv
import os
from pathlib import Path
import random
import subprocess
import statistics
import json

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'proof/tdg_level_design'

def ensure_inputs():
    if not (OUT/'baseline_runtime.c').exists() or not (OUT/'phased_sum_runtime.c').exists():
        meta = json.loads((OUT/'metadata.json').read_text())
        head = subprocess.run(['git','show',f"{meta['head_sha']}:{meta['runtime_git_path']}"],
                              cwd=ROOT,capture_output=True,text=True,check=True).stdout
        scratch = OUT/'head_runtime.c'
        scratch.write_text(head)
        for source,target,patch in ((scratch,OUT/'baseline_runtime.c',OUT/'baseline_from_head.patch'),
                                    (OUT/'baseline_runtime.c',OUT/'phased_sum_runtime.c',OUT/'phased_sum_from_baseline.patch')):
            subprocess.run(['patch','--silent','-o',str(target),str(source),str(patch)],check=True)
        scratch.unlink()
    if not (OUT/'bitmap.o').exists():
        subprocess.run(['g++','-O3','-fopenmp','-mavx2','-pthread','-c',str(ROOT/'roaring_bitmap.cpp'),
                        '-o',str(OUT/'bitmap.o')],check=True)

def build():
    ensure_inputs()
    current = (OUT / 'phased_sum_runtime.c').read_text()
    start = current.index('void sgpl_run_tdg_level(')
    unphased = current[:start] + current[start:].replace(
        'if (task_count <= sgpl_budget_available_threads())', 'if (1)', 1)
    (OUT / 'no_phase_runtime.c').write_text(unphased)
    sources = {'old': 'baseline_runtime.c', 'fixed': 'no_phase_runtime.c',
               'phased_sum': 'phased_sum_runtime.c', 'new': None}
    for name, source in sources.items():
        args = ['gcc', '-O3', '-fopenmp', '-mavx2', '-pthread', '-I', str(ROOT)]
        if source:
            args += ['-DBASELINE', f'-DSGPL_RUNTIME_SOURCE="{OUT / source}"']
        subprocess.run(args + ['-c', str(ROOT / 'tools/tdg_level_design_probe.c'),
                              '-o', str(OUT / (name + '.o'))], check=True)
        subprocess.run(['g++', '-fopenmp', '-pthread', str(OUT / (name + '.o')),
                        str(OUT / 'bitmap.o'), '-lnlopt', '-lm',
                        '-o', str(OUT / ('probe_' + name))], check=True)
    return list(sources)

def env(p):
    result = dict(os.environ, SGPL_NUM_THREADS=str(p), OMP_NUM_THREADS=str(p))
    for key in ('SGPL_FORCE_WIDTHS', 'SGPL_TDG_DEBUG', 'GRAPH_PARALLEL_DEBUG',
                'SGPL_BUDGET_DEBUG', 'SGPL_TDG_PLAN_DUMP', 'SGPL_FORCE_DOALL_PARALLEL',
                'SGPL_NO_UNREGISTERED_POOL_SHARE'):
        result.pop(key, None)
    return result

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--runs', type=int, default=3)
    parser.add_argument('--build-only', action='store_true')
    args = parser.parse_args()
    names = build()
    if args.build_only:
        return
    cases = []
    for p in (2, 4, 8):
        cases.extend((p, label, case) for label, case in (
            ('fits-mixed', (1, 1, 200000, 1000, 1, 0)),
            ('fits-loops', (0, 2, 200000, 1000, 3, 0)),
            ('overflow-one-short', (p+2, 1, 200000, 1000, 1, 0)),
            ('overflow-two-short', (p+2, 2, 200000, 1000, 3, 0)),
            ('overflow-two-ordinary-first', (p+2, 2, 200000, 1000, 3, 1)),
            ('overflow-one-long', (p+2, 1, 200000, 300000, 1, 0)),
            ('overflow-many-loops', (2, p, 100000, 1000, 1, 0)),
            ('tiny-overflow', (p+2, 1, 1025, 16, 1, 0)),
        ))
    jobs = [(p, label, case, name, r) for p,label,case in cases
            for name in names for r in range(args.runs)]
    random.Random(1025).shuffle(jobs)
    with (OUT / 'raw.csv').open('w', newline='') as output:
        writer = csv.writer(output)
        writer.writerow(('policy','case','run','p','ordinary','loops','n','ordinary_n','ratio','order',
                         'level_ns','loop_ns','sum_prediction_ns','resource_prediction_ns','early_loop_count',
                         'w0','parents0','spare0','w1','parents1','spare1',
                         'w2','parents2','spare2','w3','parents3','spare3',
                         'w4','parents4','spare4','w5','parents5','spare5',
                         'w6','parents6','spare6','w7','parents7','spare7'))
        for idx,(p,label,case,name,r) in enumerate(jobs):
            result = subprocess.run([str(OUT / ('probe_' + name)), *map(str,case)],
                                    env=env(p), capture_output=True, text=True, check=True, timeout=40)
            writer.writerow([name,label,r,*result.stdout.strip().split(',')])
            output.flush()
            print(f'{idx+1}/{len(jobs)} P={p} {label} {name}: {result.stdout.strip()}', flush=True)
    rows = list(csv.DictReader((OUT / 'raw.csv').open()))
    with (OUT / 'summary.txt').open('w') as report:
        report.write('Median of process medians; each process warms 3 levels then measures 11.\n')
        for p,label,case in cases:
            selected = [r for r in rows if int(r['p']) == p and r['case'] == label]
            times = {name: statistics.median(float(r['level_ns']) for r in selected if r['policy']==name)
                     for name in names}
            report.write(f'P={p} {label}: ' + ', '.join(f'{name}={t/1e6:.3f}ms' for name,t in times.items()) +
                         f"; new vs fixed time change={(times['new']/times['fixed']-1)*100:+.1f}%\n")

if __name__ == '__main__':
    main()
