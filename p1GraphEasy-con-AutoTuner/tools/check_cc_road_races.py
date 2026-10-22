#!/usr/bin/env python3
"""Instrument frozen CC programs and runtimes; never measure performance."""
from pathlib import Path
import csv
import json
import os
import re
import resource
import shutil
import subprocess
import sys

import validate_execution_simplification as V

R = V.R
FROZEN = V.ROOT / 'versions/relax06'
OUT = Path('/home/kazi/p1_cc_road_race_check_20261023')
GRAPHS = ['road/road-usroads-48', 'road/road-luxembourg-osm',
          'socfb/socfb-Simmons81']


def save(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2))


def build():
    src = FROZEN / 'src'
    objects = []
    for name in R.OBJECTS:
        ext = '.cpp' if name in ['roaring_bitmap', 'graph_loader_runtime', 'graph_runtime'] else '.c'
        obj = OUT / (name + '.o')
        compiler = ['g++', '-std=c++17', '-mavx2', '-march=native'] if ext == '.cpp' else ['gcc']
        print('BUILD TSAN', name, flush=True)
        if not obj.exists():
            R.checked([*compiler, '-O1', '-g', '-fsanitize=thread', '-fno-omit-frame-pointer',
                       '-fno-pie', '-fopenmp', '-pthread', '-iquote', '.', '-c', src / (name + ext),
                       '-o', obj], src, OUT / (name + '.build.log'))
        objects.append(obj)
    evidence = {}
    for graph in GRAPHS:
        case = FROZEN / 'cases/cc' / graph
        target = OUT / graph
        target.mkdir(parents=True, exist_ok=True)
        original = (case / 'validate.ll').read_text()
        extra = max(map(int, re.findall(r'^attributes #(\d+)', original, re.M)), default=-1) + 1
        def mark_unattributed(m):
            line = m[0]
            if re.search(r'#\d+', line): return line
            assert line.rstrip().endswith('{')
            at = line.find(' section ')
            if at >= 0: return line[:at] + f' #{extra}' + line[at:]
            return line.rstrip()[:-1] + f' #{extra} {{'
        original_marked = re.sub(r'^define .*$', mark_unattributed, original, flags=re.M)
        original_marked += f'\nattributes #{extra} = {{ sanitize_thread }}\n'
        defined = re.findall(r'^define .*$', original_marked, re.M)
        groups = {int(n) for line in defined for n in re.findall(r'#(\d+)', line)}
        marked = re.sub(r'^attributes #(\d+) = \{',
                        lambda m: m[0] + (' sanitize_thread' if int(m[1]) in groups else ''),
                        original_marked, flags=re.M)
        assert all(re.search(r'#\d+', line) for line in defined), 'Unattributed definition needs explicit marking'
        (target / 'marked.ll').write_text(marked)
        R.checked(['opt-20', '-passes=tsan-module,function(tsan),verify', '-S',
                   target / 'marked.ll', '-o', target / 'instrumented.ll'], src, target / 'opt.log')
        instrumented = (target / 'instrumented.ll').read_text()
        accesses = len(re.findall(r'call .*@__tsan_(?:read|write|unaligned_read|unaligned_write)', instrumented))
        assert accesses > 0, 'User callback and generated kernel must also be instrumented'
        R.checked(['llc-20', '-O2', '-filetype=obj', '-relocation-model=static',
                   target / 'instrumented.ll', '-o', target / 'program.o'], src, target / 'llc.log')
        R.checked(['g++', '-fsanitize=thread', '-fopenmp', '-pthread', '-no-pie',
                   target / 'program.o', *objects, '-ldl', '-lnlopt', '-o', target / 'check'],
                  src, target / 'link.log')
        evidence[graph] = dict(original_ir_sha256=R.sha(case / 'validate.ll'),
                               instrumented_ir_sha256=R.sha(target / 'instrumented.ll'),
                               instrumented_access_sites=accesses, executable_sha256=R.sha(target / 'check'))
    save('build.json', dict(frozen_identity=R.read(FROZEN / 'identity.json'), programs=evidence,
                            runtime_objects={str(p.name): R.sha(p) for p in objects}))


def check(serial_openmp=False):
    R.M = R.bootstrap()
    results = []
    prefix = 'graph_pool_only_' if serial_openmp else ''
    for graph in GRAPHS:
        for width in [1, 2, 6]:
            reference = (FROZEN / 'cases/cc' / graph / f'enabled_w{width}.out').read_text()
            reference_values = reference.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
            for mode in ['enabled', 'new_rules_disabled']:
                key = prefix + graph.replace('/', '_') + f'_w{width}_{mode}'
                for attempt in range(12):
                    env = R.M.env(workers=width)
                    env.update(TSAN_OPTIONS='halt_on_error=1:history_size=7', SGPL_RELAX_SIMPLIFY_STATS='1')
                    if mode == 'new_rules_disabled': env['SGPL_NO_RELAX_SIMPLIFY'] = '1'
                    log = OUT / (key + f'_attempt{attempt}')
                    try:
                        p = subprocess.run(['bash', '-c', 'ulimit -s 8192; ulimit -c 0; exec "$@"',
                                            'tsan', sys.executable, V.REPO / 'test/physical_run.py',
                                            str(width), *(['env', 'OMP_NUM_THREADS=1'] if serial_openmp else []),
                                            OUT / graph / 'check'],
                                           cwd=FROZEN / 'cases/cc' / graph, env=env,
                                           capture_output=True, text=True, timeout=240)
                    except subprocess.TimeoutExpired as e:
                        log.with_suffix('.out').write_bytes(e.stdout or b'')
                        log.with_suffix('.err').write_bytes(e.stderr or b'')
                        results.append(dict(graph=graph, workers=width, mode=mode, attempt=attempt, status='timeout'))
                        break
                    log.with_suffix('.out').write_text(p.stdout)
                    log.with_suffix('.err').write_text(p.stderr)
                    output = p.stdout + p.stderr
                    if 'WARNING: ThreadSanitizer:' in output:
                        status = 'detector_finding'
                    elif 'unexpected memory mapping' in output or 'can not mmap the shadow memory' in output:
                        status = 'startup_unavailable'
                    elif p.returncode == -11 and not output:
                        status = 'empty_launch_crash'
                    elif p.returncode:
                        status = 'execution_failure'
                    else:
                        values = p.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
                        status = 'passed' if values == reference_values else 'array_mismatch'
                    row = dict(graph=graph, workers=width, mode=mode, attempt=attempt, status=status,
                               exit_code=p.returncode, stdout=str(log.with_suffix('.out')), stderr=str(log.with_suffix('.err')))
                    results.append(row)
                    save(prefix + 'results.json', results)
                    print('CHECK', graph, width, mode, attempt, status, flush=True)
                    if status not in ['startup_unavailable', 'empty_launch_crash']: break
    save(prefix + 'results.json', results)
    final = [next((r for r in reversed(results) if r['graph'] == g and r['workers'] == w and r['mode'] == m), None)
             for g in GRAPHS for w in [1, 2, 6] for m in ['enabled', 'new_rules_disabled']]
    save(prefix + 'summary.json', dict(benchmark_runs=0, build=R.read(OUT / 'build.json'),
                              expected_checks=18, checks=final, attempts=results,
                              all_passed=all(r and r['status'] == 'passed' for r in final),
                              openmp_serial=serial_openmp, sgpl_workers=[1, 2, 6],
                              no_suppressions=True, retries_only_for_detector_startup=True))


def control():
    """Require a finding from deliberately racy LLVM stores in two pthreads."""
    folder = OUT / 'positive_control'
    folder.mkdir(exist_ok=True)
    (folder / 'worker.ll').write_text('''@race_target = external global i32
define ptr @race_worker(ptr %arg) sanitize_thread {
  store volatile i32 1, ptr @race_target
  ret ptr null
}
''')
    (folder / 'driver.c').write_text('''#include <pthread.h>
int race_target;
extern void *race_worker(void *);
int main(void) {
  pthread_t first, second;
  pthread_create(&first, 0, race_worker, 0);
  pthread_create(&second, 0, race_worker, 0);
  pthread_join(first, 0); pthread_join(second, 0);
  return 0;
}
''')
    R.checked(['opt-20', '-passes=tsan-module,function(tsan),verify', folder / 'worker.ll',
               '-o', folder / 'worker.bc'], folder, folder / 'opt.log')
    R.checked(['llc-20', '-filetype=obj', '-relocation-model=static', folder / 'worker.bc',
               '-o', folder / 'worker.o'], folder, folder / 'llc.log')
    R.checked(['gcc', '-fsanitize=thread', '-g', '-no-pie', '-pthread', folder / 'driver.c',
               folder / 'worker.o', '-o', folder / 'control'], folder, folder / 'build.log')
    attempts = []
    for attempt in range(12):
        p = subprocess.run(['bash', '-c', 'ulimit -s 8192; ulimit -c 0; exec "$@"', 'control',
                            folder / 'control'], capture_output=True, text=True,
                           env=dict(os.environ, TSAN_OPTIONS='halt_on_error=1'), timeout=30)
        output = p.stdout + p.stderr
        (folder / f'attempt{attempt}.log').write_text(output)
        expected = 'WARNING: ThreadSanitizer: data race' in output and 'race_worker' in output
        attempts.append(dict(attempt=attempt, exit_code=p.returncode, expected_race_detected=expected,
                             startup_unavailable='unexpected memory mapping' in output,
                             empty_launch_crash=p.returncode == -11 and not output))
        if expected: break
        assert attempts[-1]['startup_unavailable'] or attempts[-1]['empty_launch_crash'], output
    save('positive_control.json', dict(passed=any(x['expected_race_detected'] for x in attempts),
                                       description='LLVM-instrumented unsynchronized stores in two pthreads',
                                       attempts=attempts))


def openmp_control():
    """Characterize the system libgomp barrier visibility with a legal program."""
    folder = OUT / 'openmp_control'
    folder.mkdir(exist_ok=True)
    (folder / 'driver.c').write_text('''#include <stdint.h>
#include <stdlib.h>
int main(void) {
  int data[4096] = {0};
  unsigned char checks[8] = {0};
  const int n = 4096;
  #pragma omp parallel for schedule(static)
  for (int section = 0; section < 8; ++section) {
    int sum = 0;
    for (int i = section * 512; i < (section + 1) * 512; ++i) {
      data[i] = i; sum += i;
    }
    checks[section] = sum > 0;
  }
  /* Both parallel loops have their mandatory implicit join barrier. */
  for (int section = 0; section < 8; ++section) if (!checks[section]) abort();
  int valid = 1;
  #pragma omp parallel for reduction(&:valid) schedule(static)
  for (int edge = 0; edge < n; ++edge) valid &= data[edge] == edge;
  return !valid;
}
''')
    R.checked(['gcc', '-O1', '-g', '-fsanitize=thread', '-no-pie', '-fopenmp',
               folder / 'driver.c', '-o', folder / 'control'], folder, folder / 'build.log')
    attempts = []
    for attempt in range(12):
        env = dict(os.environ, TSAN_OPTIONS='halt_on_error=1', OMP_NUM_THREADS='6', OMP_DYNAMIC='FALSE')
        p = subprocess.run(['bash', '-c', 'ulimit -s 8192; ulimit -c 0; exec "$@"', 'omp-control',
                            folder / 'control'], capture_output=True, text=True, env=env, timeout=30)
        output = p.stdout + p.stderr
        (folder / f'attempt{attempt}.log').write_text(output)
        finding = 'WARNING: ThreadSanitizer: data race' in output
        startup = 'unexpected memory mapping' in output or (p.returncode == -11 and not output)
        attempts.append(dict(attempt=attempt, exit_code=p.returncode, finding_on_legal_openmp=finding,
                             startup_unavailable=startup))
        if not startup: break
    save('openmp_control.json', dict(implicit_barriers_preserved=True,
                                     runtime='system libgomp, GCC ThreadSanitizer', attempts=attempts))


def report():
    initial = R.read(OUT / 'summary.json')
    isolated = R.read(OUT / 'graph_pool_only_summary.json')
    positive = R.read(OUT / 'positive_control.json')
    omp = R.read(OUT / 'openmp_control.json')
    assert isolated['all_passed'] and positive['passed']
    assert any(x['finding_on_legal_openmp'] for x in omp['attempts'])
    findings = [r for r in initial['checks'] if r['status'] == 'detector_finding']
    assert findings
    for r in findings:
        text = Path(r['stderr']).read_text()
        assert 'try_load_graph_cache' in text and 'libgomp.so' in text
    correctness = R.read(V.REPO / 'reports/relax_execution_simplification_realized_correctness.json')
    work = correctness['work_counts']['checks']
    benchmark = Path('/mnt/c/p1-relax-realized-simplification-benchmark-20261022/per_graph_times.csv')
    cases = []
    with benchmark.open() as f:
        for row in csv.DictReader(f):
            graph = row['graph'].removesuffix('.edges')
            if row['algorithm'] != 'cc' or graph not in GRAPHS: continue
            counts = next(x for x in work if x['algorithm'] == 'cc' and x['graph'] == row['graph'])
            bfs = Path('/home/kazi/p1_effect_fusion_local_20261022/references') / ('bfs_' + graph.replace('/', '_') + '.edges.npy')
            distance = R.np.load(bfs)
            cases.append(dict(graph=row['graph'],
                              cc_rounds=len(counts['invocations']) + len(counts['small_work_baseline']),
                              maximum_finite_bfs_distance_from_vertex_zero=float(distance[R.np.isfinite(distance)].max()),
                              original_records=counts['original'], retained_records=counts['retained'],
                              previous_ms=float(row['previous_e2e_ms']), current_ms=float(row['current_e2e_ms']),
                              new_rules_disabled_ms=float(row['new_rules_disabled_e2e_ms']),
                              saved_graptor_ms=float(row['historical_graptor_e2e_ms'])))
    graptor_source = Path('/mnt/c/p1-effect-fusion-20261021/B3safe2-export/export/bundle/graptor_source/bench/CCv.C')
    graptor_log = Path('/home/kazi/p1_effect_fusion_local_20261022/runs/C3_local/primary_w6/cc/road/road-usroads-48/a9068c51c789d5f3f756.err')
    active = {name: name + '=1' in graptor_log.read_text()
              for name in ['LEVEL_ASYNC', 'FUSION', 'PUSH_ZERO', 'PRESET_ZERO', 'CONVERGENCE']}
    assert all(active.values())
    payload = dict(source_commit=correctness['source_commit'], diagnostic_tool_sha256=R.sha(Path(__file__)),
                   benchmark_runs=0, benchmark_units='milliseconds', production_sources_changed=False,
                   conclusion='No race detected in the tested CC graph-compute paths; not a proof of absence.',
                   cases=cases, graph_pool_checks=isolated, original_openmp_checks=initial,
                   positive_race_control=positive, legal_openmp_control=omp,
                   loader_finding_assessment='Original findings involve reused main-thread stack capture storage across implicit OpenMP barriers. The same pattern reproduces in a legal standalone OpenMP program using this system libgomp/TSan. No suppressions were added. Auxiliary OpenMP work was serialized for the isolated graph-pool checks, while SGPL retained its configured 1/2/6 physical workers.',
                   timing_assessment='Round-separated source snapshots move the minimum label one hop per round. The measured CC rounds equal the saved maximum finite BFS distance plus one for these cases. New rules reduce records inside rounds, retain round count, and add preparation/bitset work. Existing timings establish a regression but do not apportion its overhead without further profiling.',
                   graptor=dict(saved_run_active_options=active, source=str(graptor_source),
                                source_sha256=R.sha(graptor_source), saved_log=str(graptor_log),
                                note='Asynchronous label reads, fused sparse propagation and zero-label prioritization are enabled; no Graptor execution was performed.'),
                   artifacts=str(OUT))
    save('investigation.json', payload)
    R.save(V.REPO / 'reports/cc_road_race_investigation_20261023.json', payload)
    windows = Path('/mnt/c/p1-cc-road-race-check-20261023')
    for p in OUT.rglob('*'):
        if p.is_file() and p.suffix in ['.json', '.err', '.out', '.log', '.c']:
            target = windows / p.relative_to(OUT)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(p, target)
    print(json.dumps(dict(graph_compute_checks_passed=18, original_loader_findings=len(findings),
                          positive_control_passed=True, benchmark_runs=0, cases=cases), indent=2))


if __name__ == '__main__':
    OUT.mkdir(parents=True, exist_ok=True)
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    if sys.argv[1] == 'build': build()
    elif sys.argv[1] == 'check': check()
    elif sys.argv[1] == 'graph-check': check(serial_openmp=True)
    elif sys.argv[1] == 'control': control()
    elif sys.argv[1] == 'openmp-control': openmp_control()
    elif sys.argv[1] == 'report': report()
    else: raise ValueError(sys.argv[1])
