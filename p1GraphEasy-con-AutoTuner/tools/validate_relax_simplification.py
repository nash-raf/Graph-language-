#!/usr/bin/env python3
"""Correctness and work-count evidence for proof-gated minimum relaxation.

No timing blocks. Baseline is the frozen R1--R5 implementation. Builds and raw
outputs are external; compiler/runtime inputs are frozen by the shared runner.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys

import validate_execution_simplification as V
import validate_frontier_simplification as F

R = V.R
V.BASE = Path('/home/kazi/p1_execution_simplification/versions/frontier01')


def focused(d):
    src = d / 'src'
    shutil.copytree(V.REPO / 'test', src / 'test', dirs_exist_ok=True)
    for path in (src / 'test').glob('*.sh'):
        path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n'))
    out = d / 'relax_contracts'
    out.mkdir(exist_ok=True)
    env = R.M.env()
    env.update(SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1', P1_TEST_WIDTHS='1 2 6',
               P1_RELAX_SIMPLIFY_TEST_OUTPUT=str(out))
    R.checked(['bash', 'test/run_relax_simplify_tests.sh'], src, out / 'contracts.log', env)
    text = (out / 'contracts.log').read_text()
    checks = int(re.search(r'PASS relaxation IR checks=(\d+)', text)[1])
    native = re.findall(r'relaxation runtime: (\d+) comparisons, (\d+) reduced preparations, (\d+) failures', text)
    assert len(native) == 9 and all(int(x[0]) > 0 and x[2] == '0' for x in native)
    assert all(int(x[1]) > 0 for x in native[:6])
    R.save(out / 'summary.json', dict(passed=True, ir_checks=checks, rule_subsets=8,
           native_runs=9, comparisons=sum(int(x[0]) for x in native),
           workers=[1, 2, 6], modes=['normal', 'ASan/UBSan', 'no_scratch'],
           results=native, benchmark_runs=0))


def detector(d):
    out = d / 'relax_tsan'
    out.mkdir(exist_ok=True)
    src = d / 'src'
    exe = out / 'runtime'
    R.checked(['gcc', '-O1', '-g', '-no-pie', '-fsanitize=thread', '-fno-omit-frame-pointer',
               '-DSGPL_CUT_PARALLEL_GRAIN=1', '-std=gnu11', '-fopenmp', '-pthread', '-I.', '-Itest',
               'test/relax_simplify_runtime_test.c', 'autotuner_runtime.c', 'parallel_runtime.c',
               '-lnlopt', '-lm', '-Wl,--wrap=malloc', '-Wl,--wrap=calloc', '-Wl,--wrap=pthread_create',
               '-o', exe], src, out / 'build.log')
    results = []
    for width in [1, 2, 6]:
        for attempt in range(3):
            env = R.M.env(workers=width)
            env.update(TSAN_OPTIONS='halt_on_error=1', SGPL_TEST_EXPECT_SIMPLIFY='1')
            p = subprocess.run(['bash', '-c', 'ulimit -s 8192; exec "$@"', 'tsan', sys.executable,
                                V.REPO / 'test/physical_run.py', str(width), exe],
                               cwd=src, env=env, capture_output=True, text=True, timeout=180)
            text = p.stdout + p.stderr
            log = out / f'w{width}_{attempt}.log'
            log.write_text(text)
            mapping = 'unexpected memory mapping' in text or 'can not mmap the shadow memory' in text
            status = ('passed' if p.returncode == 0 else 'startup_unavailable' if mapping else
                      'unresolved_launch' if p.returncode == -11 and not text else 'failure')
            results.append(dict(workers=width, attempt=attempt, status=status, exit_code=p.returncode, log=str(log)))
            R.save(out / 'results.json', results)
            print('RELAX TSAN', width, status, flush=True)
            assert status != 'failure', results[-1]
            if status == 'passed':
                break


def work_counts(d):
    out = d / 'relax_work'
    out.mkdir(exist_ok=True)
    results = []
    for alg in ['sssp', 'cc']:
        for row in R.M.ROWS:
            case = d / 'cases' / alg / row['graph'].removesuffix('.edges')
            env = R.M.env(workers=6)
            env['SGPL_RELAX_SIMPLIFY_STATS'] = '1'
            p = subprocess.run([sys.executable, V.REPO / 'test/physical_run.py', '6', case / 'validate'],
                               cwd=case, env=env, capture_output=True, text=True, timeout=190)
            stem = out / alg / row['graph'].removesuffix('.edges')
            stem.parent.mkdir(parents=True, exist_ok=True)
            stem.with_suffix('.out').write_text(p.stdout)
            stem.with_suffix('.err').write_text(p.stderr)
            assert p.returncode == 0, (alg, row['graph'], p.stderr[-1000:])
            invocations = [dict(zip(['rules', 'original', 'unique', 'retained', 'groups'], map(int, x)))
                for x in re.findall(r'\[relax-simplify\] rules=(\d+) original=(\d+) unique=(\d+) retained=(\d+) groups=(\d+)', p.stderr)]
            baseline_work = list(map(int, re.findall(r'\[relax-simplify\] baseline reason=small-work original=(\d+)', p.stderr)))
            assert invocations or baseline_work, (alg, row['graph'], p.stderr[-1000:])
            results.append(dict(algorithm=alg, graph=row['graph'], invocations=invocations,
                                small_work_baseline=baseline_work,
                                original=sum(x['original'] for x in invocations)+sum(baseline_work),
                                retained=sum(x['retained'] for x in invocations)+sum(baseline_work)))
        print('WORK COUNTS', alg, flush=True)
    R.save(out / 'summary.json', dict(passed=True, checks=results, benchmark_runs=0))


def report(d):
    build = R.read(d / 'correctness_build.json')
    files = [p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c', '.cpp', '.h', '.inc', '.ll'}]
    files += [p for p in (V.REPO / 'generated').glob('*') if p.suffix in {'.cpp', '.h'}]
    assert R.key({str(p.relative_to(V.REPO)): hashlib.sha256(p.read_text().encode()).hexdigest()
                  for p in files}) == build['identity']['source_hash']
    validation = R.read(d / 'validation/summary.json')
    focused_result = R.read(d / 'relax_contracts/summary.json')
    assert validation['passed'] and focused_result['passed']
    existing = {n: R.read(d / 'contracts' / (n + '.json')) for n in ['simplification', 'existing', 'worker_pool']}
    assert all(x and x['passed'] for x in existing.values())
    activation = {alg: {rule: sum('[exec-simplify] ' + rule + ' activate' in p.read_text()
                                  for p in (d / 'cases' / alg).rglob('compile.log'))
                        for rule in ['R1', 'R2', 'R3', 'R4', 'R5', 'R6', 'R7', 'R8']} for alg in R.ALGS}
    assert all(activation[a]['R6'] == activation[a]['R7'] == 61 for a in ['sssp', 'cc']), activation
    groups = 0
    for alg in R.ALGS:
        for case in (d / 'cases' / alg).rglob('validate.graph'):
            for width in [1, 2, 6]:
                arrays = [(case.parent / f'{mode}_w{width}.out').read_text().split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split()
                          for mode in ['baseline', 'enabled', 'disabled']]
                assert arrays[0] == arrays[1] == arrays[2], (alg, case, width)
                groups += 1
    detectors = R.read(d / 'relax_tsan/results.json')
    assert detectors and all(x['status'] != 'failure' for x in detectors)
    work = R.read(d / 'relax_work/summary.json')
    assert work['passed']
    names = ['relax_simplify_ir_test.cpp', 'relax_simplify_runtime_test.c', 'run_relax_simplify_tests.sh']
    test_sources = {name: R.sha(d / 'src/test' / name) for name in names}
    assert all(test_sources[name] == R.sha(V.REPO / 'test' / name) for name in names)
    payload = dict(branch=R.git('branch', '--show-current'), source_commit=build['identity']['commit'],
        baseline_commit='2b46cb5ded54132e98dc052bd2caf548d1acfcb4',
        previous_implementation_commit='52841286', previous_build=str(V.BASE),
        previous_source_hash=R.read(V.BASE / 'identity.json')['source_hash'],
        build=build, source_hash=build['identity']['source_hash'], focused=focused_result,
        validation=validation, existing_contracts=existing, activation_graphs=activation,
        exact_printed_array_groups=groups, tsan=detectors,
        frontier_contracts=R.read(d / 'frontier_contracts/summary.json'),
        work_counts=work, R8_scope='Restricted complete set-consumer fixtures; unknown initial-buffer or count consumers refuse.',
        public_context_header_unchanged=not R.git('diff', '2b46cb5ded54132e98dc052bd2caf548d1acfcb4', '--', 'autotuner_runtime.h'),
        artifact_root=str(d), test_sources=test_sources, correctness_tool_sha256=R.sha(Path(__file__)),
        benchmark_runs=0, performance_claim=None)
    R.save(d / 'relax_correctness_report.json', payload)
    R.save(V.REPO / 'reports/relax_execution_simplification_bitsets_correctness.json', payload)
    print(json.dumps(dict(activation=activation, validation=validation, exact_arrays=groups), indent=2), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('stage', choices=['build', 'all', 'focused', 'contracts', 'programs', 'detector', 'work', 'report'])
    parser.add_argument('--version', required=True)
    args = parser.parse_args()
    V.prepare()
    with R.locked('relax_correctness_' + args.stage):
        d = V.ROOT / 'versions' / args.version
        if args.stage == 'build': V.build(args.version)
        elif args.stage == 'all':
            V.programs(d)
            focused(d)
            F.frontier_contracts(d)
            V.contracts(d)
            detector(d)
            work_counts(d)
            report(d)
        elif args.stage == 'focused': focused(d)
        elif args.stage == 'contracts':
            focused(d)
            F.frontier_contracts(d)
            V.contracts(d)
        elif args.stage == 'programs': V.programs(d)
        elif args.stage == 'detector': detector(d)
        elif args.stage == 'work': work_counts(d)
        else: report(d)
