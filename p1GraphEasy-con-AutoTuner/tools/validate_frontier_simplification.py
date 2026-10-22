#!/usr/bin/env python3
"""Correctness-only frontier extension validation. No timing entry points.

Reuse the frozen corpus, independent validators, isolated build cache and locks.
All build versions and raw output remain outside the checkout.
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

sys.path.insert(0, str(Path(__file__).resolve().parent))
import validate_execution_simplification as V

R = V.R


def frontier_contracts(d):
    src = d / 'src'
    shutil.copytree(V.REPO / 'test', src / 'test', dirs_exist_ok=True)
    for path in (src / 'test').glob('*.sh'):
        path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n'))
    out = d / 'frontier_contracts'
    out.mkdir(exist_ok=True)
    env = R.M.env()
    env.update(SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1', P1_TEST_WIDTHS='1 2 6',
               P1_FRONTIER_SIMPLIFY_TEST_OUTPUT=str(out))
    R.checked(['bash', 'test/run_frontier_simplify_tests.sh'], src, out / 'contracts.log', env)
    text = (out / 'contracts.log').read_text()
    comparisons = int(re.search(r'PASS frontier IR/JIT comparisons=(\d+)', text)[1])
    outcomes = re.findall(r'frontier simplification runtime: (\d+) activations, (\d+) failures', text)
    assert len(outcomes) == 9 and all(failures == '0' for _, failures in outcomes)
    assert all(int(activation) > 0 for activation, _ in outcomes[:6])
    R.save(out / 'summary.json', dict(passed=True, jit_comparisons=comparisons,
           native_runs=9, workers=[1, 2, 6], modes=['normal', 'ASan/UBSan', 'no_scratch'],
           runtime_outcomes=outcomes, benchmark_runs=0))


def frontier_detector(d):
    out = d / 'frontier_tsan'
    out.mkdir(exist_ok=True)
    src = d / 'src'
    exe = out / 'runtime'
    R.checked(['gcc', '-O1', '-g', '-no-pie', '-fsanitize=thread', '-fno-omit-frame-pointer',
               '-DSGPL_CUT_PARALLEL_GRAIN=1', '-std=gnu11', '-fopenmp', '-pthread', '-I.', '-Itest',
               'test/frontier_simplify_runtime_test.c', 'autotuner_runtime.c', 'parallel_runtime.c',
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
            result = dict(workers=width, attempt=attempt, status=status, exit_code=p.returncode, log=str(log))
            results.append(result)
            R.save(out / 'results.json', results)
            print('FRONTIER TSAN', width, status, flush=True)
            assert status != 'failure', result
            if not mapping:
                break


def activation_checks(d):
    out = d / 'frontier_activation'
    out.mkdir(exist_ok=True)
    helper = out / 'activation.o'
    R.checked(['gcc', '-O2', '-std=gnu11', '-I' + str(V.REPO), '-c',
               V.REPO / 'test/frontier_simplify_activation.c', '-o', helper], d, out / 'build.log')
    row = next(r for r in R.M.ROWS if 'ia-email-univ' in r['graph'])
    results = []
    for alg in ['bfs', 'sssp', 'cc']:
        case = d / 'cases' / alg / row['graph'].removesuffix('.edges')
        exe = out / alg
        R.checked(['g++', '-O3', '-fopenmp', '-no-pie', case / 'validate.o', helper,
                   *[d / (n + '.o') for n in R.OBJECTS], '-Wl,--wrap=sgpl_exec_bind_simplified_frontier_v1',
                   '-pthread', '-ldl', '-lnlopt', '-o', exe], d, out / (alg + '.link.log'))
        for width in [1, 2, 6]:
            for mode, extra in [('journal', {}), ('plain', {'SGPL_NO_FUSION_C': '1'}),
                                ('disabled', {'SGPL_NO_EXEC_SIMPLIFY': '1'})]:
                counts = out / f'{alg}_{width}_{mode}.json'
                env = R.M.env(workers=width)
                env.update(extra, SGPL_FRONTIER_ACTIVATION_FILE=str(counts))
                p = subprocess.run([sys.executable, V.REPO / 'test/physical_run.py', str(width), exe],
                                   cwd=case, env=env, capture_output=True, text=True, timeout=190)
                (out / f'{alg}_{width}_{mode}.out').write_text(p.stdout)
                (out / f'{alg}_{width}_{mode}.err').write_text(p.stderr)
                assert p.returncode == 0, (alg, width, mode, p.stderr)
                values = R.np.array(p.stdout.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split(), dtype=float)
                assert R.M.validate(alg, row, values, 'p1')
                observed = R.read(counts)
                assert observed['bindings'] > 0
                total = sum(observed[n] for n in ['plain', 'weighted', 'journal_plain', 'journal_weighted'])
                assert (total > 0) == (mode != 'disabled'), (alg, width, mode, observed)
                if mode == 'journal':
                    assert observed['journal_weighted' if alg == 'sssp' else 'journal_plain'] > 0, observed
                if mode == 'plain':
                    assert observed['weighted' if alg == 'sssp' else 'plain'] > 0, observed
                results.append(dict(algorithm=alg, graph=row['graph'], workers=width, mode=mode,
                                    counts=observed, passed=True))
    R.save(out / 'summary.json', dict(passed=True, checks=results, benchmark_runs=0))


def report(d):
    build = R.read(d / 'correctness_build.json')
    identity = build['identity']
    files = [p for p in V.REPO.iterdir() if p.is_file() and p.suffix in {'.c', '.cpp', '.h', '.inc', '.ll'}]
    files += [p for p in (V.REPO / 'generated').glob('*') if p.suffix in {'.cpp', '.h'}]
    current = {str(p.relative_to(V.REPO)): hashlib.sha256(p.read_text().encode()).hexdigest() for p in files}
    assert R.key(current) == identity['source_hash'], 'Native sources differ from the validated build'
    validation = R.read(d / 'validation/summary.json')
    focused = R.read(d / 'frontier_contracts/summary.json')
    existing = {n: R.read(d / 'contracts' / (n + '.json')) for n in ['simplification', 'existing', 'worker_pool']}
    assert validation['passed'] and focused['passed'] and all(x and x['passed'] for x in existing.values())
    activation = {}
    for alg in R.ALGS:
        logs = list((d / 'cases' / alg).rglob('compile.log'))
        activation[alg] = {rule: sum('[exec-simplify] ' + rule + ' activate' in p.read_text()
                                    for p in logs) for rule in ['R1', 'R2', 'R3', 'R4', 'R5']}
    assert all(activation[a]['R4'] == 61 for a in ['bfs', 'sssp', 'cc']), activation
    assert all(activation[a]['R5'] == 61 for a in ['sssp', 'cc']), activation
    # Compare printed final arrays exactly as well as the independent validators.
    # PageRank additionally has a full IEEE-bit comparison from the corpus runner.
    array_groups = 0
    for alg in R.ALGS:
        for case in (d / 'cases' / alg).rglob('validate.graph'):
            for width in [1, 2, 6]:
                values = []
                for mode in ['baseline', 'enabled', 'disabled']:
                    text = (case.parent / f'{mode}_w{width}.out').read_text()
                    values.append(text.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split())
                assert values[0] == values[1] == values[2], (alg, case, width)
                array_groups += 1
    detector = R.read(d / 'frontier_tsan/results.json')
    assert detector and all(x['status'] != 'failure' for x in detector)
    actual_activation = R.read(d / 'frontier_activation/summary.json')
    assert actual_activation and actual_activation['passed']
    payload = dict(branch=R.git('branch', '--show-current'), source_commit=R.git('rev-parse', 'HEAD'),
                   baseline_commit='2b46cb5ded54132e98dc052bd2caf548d1acfcb4',
                   previous_implementation_commit='027c6397968aa7869f097bbf8434b49c215fa5f3',
                   build=build, activation_graphs=activation, focused=focused,
                   existing_contracts=existing, validation=validation, exact_printed_array_groups=array_groups,
                   frontier_tsan=detector, existing_tsan=R.read(d / 'tsan/results.json'),
                   runtime_activation=actual_activation,
                   public_context_header_unchanged=not R.git('diff', 'HEAD', '--', 'autotuner_runtime.h'),
                   benchmark_runs=0, warmup_runs=0, profiling_runs=0, performance_claim=None,
                   artifact_root=str(d), source_hash=identity['source_hash'])
    names = ['frontier_simplify_ir_test.cpp', 'frontier_simplify_runtime_test.c',
             'frontier_simplify_activation.c', 'run_frontier_simplify_tests.sh']
    payload['test_sources'] = {name: hashlib.sha256((d / 'src/test' / name).read_text().encode()).hexdigest()
                               for name in names}
    assert all(payload['test_sources'][name] == hashlib.sha256((V.REPO / 'test' / name).read_text().encode()).hexdigest()
               for name in names), 'Test sources differ from the final validation snapshot'
    calibration = Path.home() / '.config/sgpl/hw_calib.json'
    payload['calibration_sha256'] = R.sha(calibration)
    payload['calibration_unchanged'] = payload['calibration_sha256'] == R.read(
        V.REPO / 'reports/execution_simplification_correctness.json')['calibration_sha256']
    assert payload['public_context_header_unchanged']
    assert payload['calibration_unchanged']
    R.save(d / 'frontier_correctness_report.json', payload)
    target = V.REPO / 'reports/frontier_execution_simplification_correctness.json'
    R.save(target, payload)
    print(json.dumps(dict(report=str(target), activation=activation, validation=validation), indent=2), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('stage', choices=['build', 'contracts', 'focused', 'programs', 'activation', 'detectors', 'report'])
    parser.add_argument('--version', required=True)
    args = parser.parse_args()
    V.prepare()
    with R.locked('frontier_correctness_' + args.stage):
        d = V.ROOT / 'versions' / args.version
        if args.stage == 'build':
            V.build(args.version)
        elif args.stage == 'contracts':
            frontier_contracts(d)
            V.contracts(d)
        elif args.stage == 'focused':
            frontier_contracts(d)
        elif args.stage == 'programs':
            V.programs(d)
        elif args.stage == 'detectors':
            frontier_detector(d)
            V.detectors(d)
        elif args.stage == 'activation':
            activation_checks(d)
        else:
            report(d)
