#!/usr/bin/env python3
"""Run frozen GPU builds against historical same-PC R19 CPU measurements (WSL)."""
from pathlib import Path
import argparse
import csv
import hashlib
import importlib.util
import json
import math
import os
import platform
import re
import shutil
import signal
import statistics
import subprocess
import time

REPO = Path(__file__).resolve().parents[1]
FROZEN = Path('/home/kazi/p1_execution_simplification/versions/iteration_fusion02')
CPU_ROOT = Path('/home/kazi/p1_iteration_fusion_benchmark_20261024')
CPU_REPORT = Path('/mnt/c/p1-iteration-fusion-benchmark-20261024/final_summary.json')
HIST = Path('/home/kazi/p1_effect_fusion_local_20261022')
ROOT = Path('/home/kazi/p1_gpu_vs_saved_cpu_20261104')
OUT = REPO / 'bench_results/gpu_vs_saved_cpu_20261104'
OBJECTS = ['runtime', 'parallel_runtime', 'gpu_runtime', 'autotuner_runtime',
           'graph_mutation_runtime', 'roaring_bitmap', 'graph_loader_runtime',
           'graph_runtime', 'semiring_runtime']


def save(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_suffix(path.suffix + '.tmp')
    temp.write_text(json.dumps(value, indent=2))
    temp.replace(path)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(cmd, directory, tag, env, timeout=190):
    """Preserve all output and kill the complete process group on timeout."""
    with (directory / (tag + '.out')).open('w') as out, (directory / (tag + '.err')).open('w') as err:
        process = subprocess.Popen(list(map(str, cmd)), cwd=directory, env=env,
                                   stdout=out, stderr=err, start_new_session=True)
        try:
            code = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            raise RuntimeError(tag + ' timeout')
    if code:
        raise RuntimeError(tag + ' exit ' + str(code) + ': ' + (directory / (tag + '.err')).read_text()[-2000:])
    return (directory / (tag + '.out')).read_text()


def build(algorithm, row, mode, env):
    directory = ROOT / 'cases' / algorithm / row['graph'].removesuffix('.edges')
    directory.mkdir(parents=True, exist_ok=True)
    for source in (FROZEN / 'src').glob('*.ll'):
        target = directory / source.name
        if not target.exists():
            target.symlink_to(source)
    source = directory / (mode + '.graph')
    original = (CPU_ROOT / 'versions/candidate/cases' / algorithm / row['graph'].removesuffix('.edges') / 'timed.graph'
                if mode == 'timed' else FROZEN / 'cases' / algorithm / row['graph'].removesuffix('.edges') / 'validate.graph')
    assert original.exists(), str(original)
    if source.exists():
        assert source.read_bytes() == original.read_bytes()
    else:
        shutil.copyfile(original, source)
    executable = directory / mode
    if not executable.exists():
        run([FROZEN / 'GraphProgram', '--ir-backend=gpu', source], directory,
            mode + '.compile', env, 1200)
        shutil.move(directory / 'program.o', directory / (mode + '.o'))
        run(['g++', '-O3', '-fopenmp', '-no-pie', directory / (mode + '.o'),
             *[FROZEN / (name + '.o') for name in OBJECTS], HIST / 'monotonic_timer.o',
             '-Wl,--wrap=timer_runtime', '-pthread', '-ldl', '-lnlopt', '-o', executable],
            directory, mode + '.link', env, 1200)
    return directory, executable


def summarize(protocol, results):
    groups = {}
    for algorithm in ['bfs', 'sssp', 'cc', 'pagerank']:
        cases = [x for x in results if x['algorithm'] == algorithm]
        valid = [x for x in cases if x['status'] == 'gpu_validated']
        groups[algorithm] = dict(attempted=len(cases), gpu_validated=len(valid),
                                fallback=sum(x['status'] == 'cpu_fallback' for x in cases),
                                failed=sum(x['status'] == 'failed' for x in cases))
        if valid:
            for key in ['cpu_over_gpu_e2e', 'cpu_over_gpu_compute']:
                groups[algorithm][key + '_geomean'] = math.exp(statistics.mean(math.log(x[key]) for x in valid))
            groups[algorithm]['gpu_e2e_wins'] = sum(x['cpu_over_gpu_e2e'] > 1 for x in valid)
            groups[algorithm]['gpu_compute_wins'] = sum(x['cpu_over_gpu_compute'] > 1 for x in valid)
    report = dict(protocol=protocol, summary=groups, cases=results)
    save(OUT / 'results.json', report)
    columns = ['algorithm', 'graph', 'status', 'saved_cpu_e2e_ms', 'gpu_e2e_ms',
               'saved_cpu_compute_ms', 'gpu_compute_ms', 'cpu_over_gpu_e2e',
               'cpu_over_gpu_compute', 'validation_gpu_tasks', 'diagnostic_gpu_tasks', 'error']
    with (OUT / 'per_graph_times.csv').open('w', newline='') as file:
        writer = csv.DictWriter(file, fieldnames=columns, extrasaction='ignore')
        writer.writeheader()
        writer.writerows(results)
    lines = ['RTX 3050 versus saved Ryzen 5 5500 six-core CPU times', '',
             'Historical, unpaired comparison; CPU was not rebenchmarked.',
             'Frozen R19 compiler and runtime; identical timed sources and inputs.',
             'One warmup; five fresh-process repetitions; arithmetic means.',
             'Times include GPU initialization, transfers and synchronization within each timer region.',
             'GPU debug disabled in timed samples; validated device execution checked separately.',
             'Ratios are saved CPU / GPU; greater than 1 means GPU is faster.', '']
    for algorithm, values in groups.items():
        lines.append(algorithm + ': ' + json.dumps(values))
        if values['gpu_validated']:
            lines.append('  GPU slowdown versus saved CPU: %.2fx whole-process; %.2fx compute region.' % (
                1 / values['cpu_over_gpu_e2e_geomean'],
                1 / values['cpu_over_gpu_compute_geomean']))
    valid = [x for x in results if x['status'] == 'gpu_validated']
    if valid:
        lines += ['', 'Validated GPU cases: %d; whole-process wins: %d; compute-region wins: %d.' % (
            len(valid), sum(x['cpu_over_gpu_e2e'] > 1 for x in valid),
            sum(x['cpu_over_gpu_compute'] > 1 for x in valid))]
    lines += ['', 'Detailed measurements: per_graph_times.csv and results.json',
              'Native executables and logs: ' + str(ROOT),
              'Run with the existing WSL investigation venv:',
              '/home/kazi/p1_local_regressions_20261019/venv/bin/python ' + str(Path(__file__).resolve()),
              'The GPU also serves the Windows desktop; no other applications were stopped.']
    (OUT / 'report.txt').write_text('\n'.join(lines) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--limit', type=int)
    parser.add_argument('--algorithms', nargs='+', default=['bfs', 'sssp', 'cc', 'pagerank'])
    args = parser.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    ROOT.mkdir(parents=True, exist_ok=True)
    spec = importlib.util.spec_from_file_location('saved_common', Path('/home/kazi/p1_c3_integration_20261022/support/common.py'))
    common = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(common)
    cpu = json.loads(CPU_REPORT.read_text())
    assert cpu['protocol']['host']['cpu_model'] == next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name'))
    saved = {(x['algorithm'], x['graph']): x for x in cpu['cases']}
    env = common.env(workers=6)
    env.update(FORCE_CPU='0', FORCE_GPU='1', SGPL_GPU_BACKEND='1')
    protocol = dict(created_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
                    cpu_report=str(CPU_REPORT), cpu_report_sha256=sha(CPU_REPORT),
                    saved_cpu_label='candidate R19, retained R1--R18 and iteration memory',
                    saved_cpu_phase=cpu['selected_phase'], cpu_host=cpu['protocol']['host'],
                    gpu=subprocess.check_output(['nvidia-smi', '--query-gpu=name,driver_version,memory.total', '--format=csv,noheader'], text=True).strip(),
                    platform=platform.platform(), frozen_compiler=str(FROZEN / 'GraphProgram'),
                    compiler_sha256=sha(FROZEN / 'GraphProgram'),
                    objects_sha256={name: sha(FROZEN / (name + '.o')) for name in OBJECTS},
                    timer_sha256=sha(HIST / 'monotonic_timer.o'),
                    walltime_sha256=sha(CPU_ROOT / 'walltime'), repetitions=5, warmups=1,
                    cpus=[0, 2, 4, 6, 8, 10], runtime_environment={key: value for key, value in env.items() if key.startswith(('SGPL_', 'OMP_', 'GOMP_', 'FORCE_'))},
                    comparison='Historical unpaired saved CPU arithmetic means; no new CPU benchmarks.',
                    timing='Same CLOCK_MONOTONIC whole-process wrapper and internal timer interposer as saved CPU.')
    previous = json.loads((OUT / 'results.json').read_text()) if (OUT / 'results.json').exists() else None
    if previous:
        for key in ['cpu_report_sha256', 'compiler_sha256', 'objects_sha256', 'timer_sha256', 'walltime_sha256', 'runtime_environment']:
            assert protocol[key] == previous['protocol'][key], ('Protocol changed', key)
        protocol = previous['protocol']
    results = previous['cases'] if previous else []
    done = {(x['algorithm'], x['graph']) for x in results}
    rows = common.ROWS[:args.limit] if args.limit else common.ROWS
    for row in rows:
        for algorithm in args.algorithms:
            if (algorithm, row['graph']) in done:
                continue
            reference = saved[(algorithm, row['graph'])]
            entry = dict(algorithm=algorithm, graph=row['graph'], vertices=row['vertices'],
                         arcs=row['arcs_both_directions'], status='failed',
                         saved_cpu_e2e_ms=reference['mean_ms']['candidate'],
                         saved_cpu_compute_ms=reference['kernel_mean_ms']['candidate'])
            save(OUT / 'status.json', dict(active=True, completed=len(results), algorithm=algorithm, graph=row['graph'], stage='build/validate', updated=time.time()))
            print('START', len(results) + 1, algorithm, row['graph'], flush=True)
            try:
                graph_path = Path(row['p1_weighted_file'] if algorithm == 'sssp' else row['original_file'])
                expected_hash = cpu['protocol'].get('input_hashes', {}).get(str(graph_path))
                entry['input_sha256'] = sha(graph_path)
                if algorithm != 'sssp':
                    assert entry['input_sha256'] == row['original_sha256'], 'Input hash changed'
                elif expected_hash:
                    assert entry['input_sha256'] == expected_hash, 'Weighted input hash changed'
                directory, validation = build(algorithm, row, 'validate', env)
                debug = dict(env, SGPL_GPU_DEBUG='1')
                output = run([validation], directory, 'validation', debug)
                import numpy as np
                values = np.array(output.split('VALUES_BEGIN\n', 1)[1].split('VALUES_END', 1)[0].split(), dtype=float)
                entry['validation'] = common.validate(algorithm, row, values, 'p1')
                trace = (directory / 'validation.err').read_text()
                entry['validation_gpu_tasks'] = trace.count('[gpu-task]')
                directory, executable = build(algorithm, row, 'timed', env)
                entry.update(source_sha256=sha(directory / 'timed.graph'), executable_sha256=sha(executable))
                # A separate launch establishes that the actual timed binary offloads.
                run([executable], directory, 'diagnostic', debug)
                trace = (directory / 'diagnostic.err').read_text()
                entry['diagnostic_gpu_tasks'] = trace.count('[gpu-task]')
                entry['status'] = 'gpu_validated' if entry['diagnostic_gpu_tasks'] else 'cpu_fallback'
                samples = []
                for repeat in range(-1, 5):
                    tag = 'warmup' if repeat == -1 else 'repeat_' + str(repeat)
                    stamp = directory / (tag + '.stamp.json')
                    output = run([CPU_ROOT / 'walltime', stamp, executable], directory, tag, env)
                    sample = json.loads(stamp.read_text())
                    assert sample['exit_code'] == 0
                    match = re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)', output)
                    assert match, 'Missing compute timer'
                    sample['internal_seconds'] = float(match[1])
                    if repeat >= 0:
                        samples.append(sample)
                entry['samples'] = samples
                entry['gpu_e2e_ms'] = statistics.mean(x['seconds'] for x in samples) * 1000
                entry['gpu_compute_ms'] = statistics.mean(x['internal_seconds'] for x in samples) * 1000
                entry['cpu_over_gpu_e2e'] = entry['saved_cpu_e2e_ms'] / entry['gpu_e2e_ms']
                entry['cpu_over_gpu_compute'] = entry['saved_cpu_compute_ms'] / entry['gpu_compute_ms']
            except Exception as error:
                entry['status'] = 'failed'
                entry['error'] = repr(error)
            results.append(entry)
            summarize(protocol, results)
            print('DONE', len(results), algorithm, row['graph'], entry['status'],
                  entry.get('gpu_compute_ms', entry.get('error')), flush=True)
    save(OUT / 'status.json', dict(active=False, completed=len(results), updated=time.time()))
    summarize(protocol, results)


if __name__ == '__main__':
    main()
