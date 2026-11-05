#!/usr/bin/env python3
"""Alternating same-binary batching control; run after the main GPU campaign."""
from pathlib import Path
import importlib.util
import json
import os
import re
import statistics
import benchmark_gpu_saved_cpu as B

ROOT = Path('/home/kazi/p1_gpu_batch_20261104/benchmark/cases')
OUT = B.REPO / 'bench_results/gpu_batch_20261104'
spec = importlib.util.spec_from_file_location('common', '/home/kazi/p1_c3_integration_20261022/support/common.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)
env = common.env(workers=6)
env.update(FORCE_CPU='0', FORCE_GPU='1', SGPL_GPU_BACKEND='1')
selected = [('pagerank', 'dimacs/hamming8-2.edges'), ('bfs', 'soc/soc-slashdot.edges'),
            ('cc', 'dimacs10/fe-4elt2.edges')]
results = []
for algorithm, graph in selected:
    directory = ROOT / algorithm / graph.removesuffix('.edges')
    executable = directory / 'timed'
    records = {'0': [], '1': []}
    counts = {}
    for mode in ['0', '1']:
        tag = 'control_warmup_' + mode
        B.run([executable], directory, tag, dict(env, SGPL_GPU_BATCH=mode, SGPL_GPU_DEBUG='1'))
        counts[mode] = (directory / (tag + '.err')).read_text().count('[gpu-task]')
    assert counts['0'] == 64 * counts['1'] and counts['1'] > 0
    for repeat in range(5):
        for mode in (['0', '1'] if repeat % 2 == 0 else ['1', '0']):
            tag = 'control_' + mode + '_' + str(repeat)
            stamp = directory / (tag + '.stamp.json')
            output = B.run([B.CPU_ROOT / 'walltime', stamp, executable], directory, tag,
                           dict(env, SGPL_GPU_BATCH=mode))
            sample = json.loads(stamp.read_text())
            sample['internal_seconds'] = float(re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)', output)[1])
            assert sample['exit_code'] == 0
            records[mode].append(sample)
    entry = dict(algorithm=algorithm, graph=graph, executable_sha256=B.sha(executable),
                 task_counts=counts, samples=records)
    for timer in ['seconds', 'internal_seconds']:
        means = {mode: statistics.mean(x[timer] for x in xs) for mode, xs in records.items()}
        entry[timer + '_means'] = means
        entry[timer + '_speedup'] = means['0'] / means['1']
    results.append(entry)
    B.save(OUT / 'same_binary_control.json', dict(repetitions=5, ordering='Alternating AB/BA fresh-process runs',
                                               changed_environment='SGPL_GPU_BATCH=0 versus 1', cases=results))
    print(algorithm, graph, entry['seconds_speedup'], entry['internal_seconds_speedup'], flush=True)
