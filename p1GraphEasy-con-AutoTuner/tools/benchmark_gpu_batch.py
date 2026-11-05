#!/usr/bin/env python3
"""Benchmark spatial batching against the immutable saved GPU and CPU runs."""
from pathlib import Path
import json
import math
import statistics
import benchmark_gpu_saved_cpu as B

NATIVE = Path('/home/kazi/p1_gpu_batch_20261104/versions/candidate02')
SAVED_GPU = B.OUT / 'results.json'
OLD_NATIVE = B.FROZEN
B.FROZEN = NATIVE
B.ROOT = Path('/home/kazi/p1_gpu_batch_20261104/benchmark')
B.OUT = B.REPO / 'bench_results/gpu_batch_20261104/benchmark'
if not (NATIVE / 'cases').exists():
    (NATIVE / 'cases').symlink_to(OLD_NATIVE / 'cases', target_is_directory=True)
original_summarize = B.summarize


def summarize(protocol, results):
    protocol = dict(protocol, optimization='Independent spatial antichain batching v1',
                    saved_gpu_report=str(SAVED_GPU), saved_gpu_sha256=B.sha(SAVED_GPU))
    original_summarize(protocol, results)
    baseline = json.loads(SAVED_GPU.read_text())
    saved = {(x['algorithm'], x['graph']): x for x in baseline['cases']}
    comparison = []
    for result in results:
        old = saved[result['algorithm'], result['graph']]
        item = dict(result)
        item['saved_gpu_status'] = old['status']
        if result['status'] == old['status'] == 'gpu_validated':
            item.update(saved_gpu_e2e_ms=old['gpu_e2e_ms'], saved_gpu_compute_ms=old['gpu_compute_ms'],
                        saved_gpu_tasks=old['diagnostic_gpu_tasks'],
                        saved_gpu_over_batch_e2e=old['gpu_e2e_ms'] / result['gpu_e2e_ms'],
                        saved_gpu_over_batch_compute=old['gpu_compute_ms'] / result['gpu_compute_ms'])
        comparison.append(item)
    summary = {}
    for algorithm in ['bfs', 'sssp', 'cc', 'pagerank']:
        rows = [x for x in comparison if x['algorithm'] == algorithm and 'saved_gpu_over_batch_e2e' in x]
        summary[algorithm] = dict(paired_gpu_cases=len(rows))
        if rows:
            for key in ['saved_gpu_over_batch_e2e', 'saved_gpu_over_batch_compute', 'cpu_over_gpu_e2e', 'cpu_over_gpu_compute']:
                summary[algorithm][key + '_geomean'] = math.exp(statistics.mean(math.log(x[key]) for x in rows))
            summary[algorithm]['wins_vs_saved_gpu'] = sum(x['saved_gpu_over_batch_e2e'] > 1 for x in rows)
            summary[algorithm]['wins_vs_saved_cpu'] = sum(x['cpu_over_gpu_e2e'] > 1 for x in rows)
    B.save(B.OUT / 'comparison.json', dict(protocol=protocol, summary=summary, cases=comparison))
    import csv
    columns = ['algorithm', 'graph', 'status', 'saved_gpu_status', 'saved_cpu_e2e_ms',
               'saved_gpu_e2e_ms', 'gpu_e2e_ms', 'saved_cpu_compute_ms', 'saved_gpu_compute_ms',
               'gpu_compute_ms', 'saved_gpu_tasks', 'diagnostic_gpu_tasks',
               'saved_gpu_over_batch_e2e', 'saved_gpu_over_batch_compute', 'cpu_over_gpu_e2e', 'error']
    with (B.OUT / 'comparison.csv').open('w', newline='') as file:
        writer = csv.DictWriter(file, columns, extrasaction='ignore')
        writer.writeheader()
        writer.writerows(comparison)
    text = (B.OUT / 'report.txt').read_text().replace('Frozen R19 compiler and runtime;', 'R19 plus spatial batching v1 compiler/runtime;')
    text += '\nComparison against saved GPU (historical/unpaired):\n'
    text += '\n'.join(a + ': ' + json.dumps(s) for a, s in summary.items()) + '\n'
    (B.OUT / 'report.txt').write_text(text)


B.summarize = summarize
if __name__ == '__main__':
    B.main()
