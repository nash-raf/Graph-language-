#!/usr/bin/env python3
"""Confirm five discovered dense-graph layout-switch workload families."""
import hashlib
import json
import os
import platform
import subprocess
import time
from pathlib import Path
from layout_switch_benchmark import ROOT, MODES, benchmark, build, source, compile_case, audit, environment

OUT=ROOT/'bench_results/layout_switch_20261015'
CONFIGS=[
    dict(n=200,degree=64,inserts=4,churn=1000,scans=300,tail_scans=4,cycles=1,shape='bfs'),
    dict(n=256,degree=96,inserts=4,churn=1000,scans=240,tail_scans=4,cycles=1,shape='bfs'),
    dict(n=300,degree=100,inserts=4,churn=1000,scans=180,tail_scans=4,cycles=1,shape='bfs'),
    dict(n=384,degree=128,inserts=4,churn=1000,scans=140,tail_scans=4,cycles=1,shape='bfs'),
    dict(n=512,degree=192,inserts=4,churn=1000,scans=100,tail_scans=4,cycles=1,shape='bfs'),
]
for config in CONFIGS:
    config['query_loops']=True

def main():
    raw=OUT/'validation_raw_runs.csv'
    if raw.exists():
        raw.rename(OUT/f'validation_raw_runs_{time.time_ns()}.csv')
    selected=[]
    for config in CONFIGS:
        benchmark(OUT,config,7,1,300,raw_name='validation_raw_runs.csv')
        work,objs=build(OUT)
        name,src,expected=source(OUT,**config)
        schedules={m:compile_case(OUT,work,objs,name,src,m)[1] for m in MODES}
        audit(OUT,config,work,objs,name,expected,schedules)
        result=json.loads((OUT/'binaries'/name/'result.json').read_text())
        if result['speedup_vs_best_fixed']<=1:
            raise RuntimeError(f'{name}: no confirmed win against best fixed layout')
        selected.append(name)
        (OUT/'selected_cases.json').write_text(json.dumps(selected,indent=2)+'\n')
        print(f'CONFIRMED {len(selected)}/5 {name}: {result["speedup_vs_best_fixed"]:.3f}x',flush=True)
    files=['GraphProgram','AutoTunerPass.cpp','autotuner_runtime.c','graph_mutation_runtime.c',
           'graph_loader_runtime.cpp','roaring_bitmap.cpp','parallel_runtime.c','bfs_runtime_src.cpp',
           'tools/layout_switch_benchmark.py','tools/layout_switch_audit.c','tools/run_layout_switch_suite.py']
    files.append('tools/summarize_layout_switch.py')
    calibration=Path.home()/'.config/sgpl/hw_calib.json'
    if calibration.exists():
        (OUT/'hardware_calibration.json').write_bytes(calibration.read_bytes())
    manifest=dict(configs=CONFIGS,selected=selected,platform=platform.platform(),
        cpu=subprocess.check_output(['lscpu'],text=True),
        git_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        compiler='Existing GraphProgram, LLVM 20.1; runtimes built with GCC/G++ -O3',
        source_sha256={f:hashlib.sha256((ROOT/f).read_bytes()).hexdigest() for f in files},
        workload_sha256={name:hashlib.sha256((OUT/'codes'/(name+'.sgpl')).read_bytes()).hexdigest() for name in selected},
        graph_sha256={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in (OUT/'graphs').glob('*.txt')},
        linked_runtime_ir_sha256={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in ROOT.glob('*.ll')
                                  if f.name in ('bfs_runtime.ll','dfs_runtime.ll','bk_runtime.ll','floyd_runtime.ll','chromacity_runtime.ll')},
        run_environment={k:v for k,v in environment().items() if k.startswith(('OMP_','GOMP_','SGPL_'))},
        calibration_sha256=hashlib.sha256(calibration.read_bytes()).hexdigest() if calibration.exists() else None,
        timing='perf_counter_ns before subprocess.run through process exit; includes loading, runtime layout conversions, computation, allocation, profiler and output; excludes compilation and diagnostic audit',
        repetitions=7,warmups_per_variant=1,random_order_seed=20261015,
        filesystem='WSL local /tmp/sgpl_layout_switch_20261015; all timed variants use the same graph and warm .sgplbin loader cache',
        fixed_layout='Graphs initially load as CSR. Install requested fixed layout before first workload operation; include this initial conversion in total time. Assert layout at every traversal and mutation.',
        selection='Search-selected synthetic workloads. Final seven-run measurements and audits confirm a win over every fixed-layout median; results are not representative of all SGPL programs.')
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')

if __name__=='__main__': main()
