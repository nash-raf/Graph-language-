#!/usr/bin/env python3
"""Paired semantic-stage measurements against the previously tested R1--R9 build.

Retain the previous campaign and read its Graptor observations without executing
Graptor. Reuse its timing/statistical engine with separate immutable artifacts.
"""
from pathlib import Path
import argparse
import collections
import csv
import fcntl
import hashlib
import html
import json
import math
import os
import platform
import shutil

import benchmark_execution_simplification as B

R = B.R
REPO = B.REPO
ROOT = Path('/home/kazi/p1_semantic_stage_benchmark_20261023')
OUT = Path('/mnt/c/p1-semantic-stage-benchmark-20261023')
BASE = Path('/home/kazi/p1_execution_simplification/versions/relax06')
CAND = Path('/home/kazi/p1_execution_simplification/versions/semantic08')
OLD_TIMED = Path('/home/kazi/p1_relax_realized_simplification_benchmark_20261022/versions/candidate/cases')
CORRECT = REPO / 'reports/semantic_stage_simplification_correctness.json'
ROOT.mkdir(exist_ok=True)
OUT.mkdir(exist_ok=True)
B.ROOT = R.ROOT = ROOT
B.OUT = R.OUT = OUT
B.BASE, B.CAND = BASE, CAND
B.VARIANTS = [('baseline', BASE, {}), ('candidate', CAND, {}),
              ('disabled', CAND, {'SGPL_NO_SEMANTIC_STAGE': '1'}),
              ('baseline_AA', BASE, {}), ('candidate_AA', CAND, {})]
os.environ.update(P1_CAMPAIGN_ROOT=str(ROOT), P1_CAMPAIGN_OUTPUT=str(OUT),
                  P1_CAMPAIGN_CPUS='0,2,4,6,8,10')


def prepare():
    old = Path('/home/kazi/p1_c3_integration_20261022')
    for name in ['references', 'frozen_reports', 'historical_harness', 'support']:
        if not (ROOT / name).exists():
            (ROOT / name).symlink_to(old / name, target_is_directory=True)
    if not (ROOT / 'inputs.json').exists():
        shutil.copy2(old / 'inputs.json', ROOT / 'inputs.json')
    if not (ROOT / 'walltime').exists():
        shutil.copy2(old / 'walltime', ROOT / 'walltime')
    R.M = R.bootstrap()
    manifest = R.read(B.HIST / 'results/manifest.json')
    assert R.M.ROWS == manifest['cases']
    for name, digest in manifest['inputs'].items():
        assert R.sha(name) == digest, ('Input changed', name)
    correct = R.read(CORRECT)
    previous = R.read(REPO / 'reports/relax_execution_simplification_realized_correctness.json')
    assert correct['validation']['passed'] and correct['focused']['passed']
    for source, evidence in [(BASE, previous), (CAND, correct)]:
        assert R.read(source / 'identity.json')['source_hash'] == evidence['source_hash']
        assert R.sha(source / 'GraphProgram') == evidence['build']['compiler_sha256']
        for name, digest in evidence['build']['objects'].items():
            assert R.sha(source / (name + '.o')) == digest
    files = [p for p in REPO.iterdir() if p.is_file() and p.suffix in {'.c', '.cpp', '.h', '.inc', '.ll'}]
    files += [p for p in (REPO / 'generated').glob('*') if p.suffix in {'.cpp', '.h'}]
    assert R.key({str(p.relative_to(REPO)): hashlib.sha256(p.read_text().encode()).hexdigest()
                  for p in files}) == correct['source_hash']
    topology, seen = [], set()
    for cpu in R.CPUS:
        d = Path('/sys/devices/system/cpu') / ('cpu' + str(cpu)) / 'topology'
        key = tuple((d / n).read_text().strip() for n in ['physical_package_id', 'core_id'])
        assert key not in seen
        seen.add(key)
        topology.append(dict(cpu=cpu, package=key[0], core=key[1]))
    host = dict(platform=platform.platform(), cpu_model=next(x for x in Path('/proc/cpuinfo').read_text().splitlines()
                if x.startswith('model name')), physical_cpus=R.CPUS, workers=6, topology=topology,
                boot_id=Path('/proc/sys/kernel/random/boot_id').read_text().strip())
    assert host['cpu_model'] == R.read(B.HIST / 'host.json')['cpu_model']
    graptor = [x for x in R.read(B.HIST / 'results/primary_w6.json') if x['label'] == 'graptor']
    assert len(graptor) == 1220
    for x in graptor:
        assert x['workers'] == 6 and not x.get('error') and x['seconds'] > 0
        assert x['version'] == ('graptor_double' if x['algorithm'] == 'pagerank' else 'graptor')
        assert R.read(Path(x['base'] + '.record.json')) == x
    protocol = dict(
        baseline_commit=correct['previous_identity']['commit'],
        baseline_label='Previous implementation: R1 through R9 enabled',
        candidate_commit=correct['source_commit'], candidate_label='Current implementation: R1 through R12 enabled where proved',
        accepted_ARS1_checkpoint=R.git('rev-parse', 'ARS-1'), source_hash=correct['source_hash'],
        host=host, dataset_hash=manifest['dataset_hash'], cases=R.M.ROWS,
        primary_repetitions=5, confirmation_repetitions=20, warmups=1,
        variants=[x[0] for x in B.VARIANTS],
        disabled_control='Current build with R10--R12 disabled; R1--R9 remain enabled.',
        timer='whole-process CLOCK_MONOTONIC; internal timer_runtime interposer CLOCK_MONOTONIC',
        historical_graptor=dict(records=str(B.HIST / 'results/primary_w6.json'),
            sha256=R.sha(B.HIST / 'results/primary_w6.json'), host=R.read(B.HIST / 'host.json'),
            precision='double PageRank', note='Prior same-PC observations; unpaired descriptive comparison only. No Graptor launches.'),
        correctness_report_sha256=R.sha(CORRECT),
        previous_correctness_report_sha256=R.sha(REPO / 'reports/relax_execution_simplification_realized_correctness.json'),
        native_fingerprints={label: dict(compiler=R.sha(d / 'GraphProgram'),
            objects={n: R.sha(d / (n + '.o')) for n in R.OBJECTS}) for label, d in [('baseline', BASE), ('candidate', CAND)]},
        harnesses={str(p.relative_to(REPO)): R.sha(p) for p in [Path(__file__), Path(B.__file__),
            REPO / 'tools/p1_campaign/runner.py', REPO / 'tools/p1_campaign/portable_campaign.py']},
        calibration_sha256=R.sha(Path.home() / '.config/sgpl/hw_calib.json'))
    if (OUT / 'protocol.json').exists():
        assert R.read(OUT / 'protocol.json') == protocol, 'Protocol changed; preserve previous observations'
    else:
        R.save(OUT / 'protocol.json', protocol)
    return protocol, graptor


def build():
    # The old candidate executables are exactly the new baseline. Verify their
    # source text and pinned fingerprints, then measure them afresh in this run.
    old_fp = R.read('/mnt/c/p1-relax-realized-simplification-benchmark-20261022/timed_fingerprints.json')
    reused = []
    for alg in R.ALGS:
        for row in R.M.ROWS:
            prior = OLD_TIMED / alg / row['graph'].removesuffix('.edges')
            source = R.M.make_source(alg, row, False)
            assert (prior / 'timed.graph').read_text() == source
            key = 'versions/candidate/cases/' + alg + '/' + row['graph'].removesuffix('.edges') + '/timed'
            assert R.sha(prior / 'timed') == old_fp[key]
            target = B.path('baseline', alg, row)
            target.mkdir(parents=True, exist_ok=True)
            for name in ['timed.graph', 'timed.o', 'timed', 'compile.log', 'link.log']:
                if not (target / name).exists():
                    (target / name).symlink_to(prior / name)
            reused.append(dict(algorithm=alg, graph=row['graph'], executable=str(prior / 'timed'), sha256=old_fp[key]))
    R.save(OUT / 'baseline_executable_reuse.json', reused)
    B.build()
    for alg in R.ALGS:
        logs = [B.path('candidate', alg, row) / 'compile.log' for row in R.M.ROWS]
        for rule in ['R10', 'R11', 'R12']:
            expected = alg in ['cc', 'sssp'] if rule != 'R12' else alg == 'cc'
            assert all((('[exec-simplify] ' + rule + ' activate') in p.read_text()) == expected for p in logs), (alg, rule)


def report(protocol, results):
    chosen = next(reversed(results))
    result = results[chosen]
    s, rows = result['summary'], result['cases']
    columns = ['algorithm', 'graph', 'cohort', 'previous_e2e_ms', 'current_e2e_ms', 'new_rules_disabled_e2e_ms',
               'change_percent', 'previous_kernel_ms', 'current_kernel_ms', 'historical_graptor_e2e_ms',
               'historical_graptor_kernel_ms', 'current_over_historical_graptor', 'slowdown_holm_p']
    with (OUT / 'per_graph_times.csv').open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=columns)
        writer.writeheader()
        for x in rows:
            t, k = x['mean_ms'], x['kernel_mean_ms']
            writer.writerow(dict(zip(columns, [x['algorithm'], x['graph'], x['cohort'], t['baseline'],
                t['candidate'], t['disabled'], 100 * (t['candidate'] / t['baseline'] - 1), k['baseline'],
                k['candidate'], x['historical_graptor_ms'], x['historical_graptor_kernel_ms'],
                t['candidate'] / x['historical_graptor_ms'], x['tests']['code']['holm_p']])))
    lines = ['Semantic-stage work elimination: local six-core benchmark', '',
             'Previous = R1--R9 build; current adds proof-gated direct traversal (R10), ordered destination state (R11), and finite-lattice convergence (R12). CC executes R12; SSSP executes R10. No cost model was added.',
             f"Previous commit {protocol['baseline_commit']}; current source commit {protocol['candidate_commit']}.",
             f"Selected block: {chosen}; identical-binary controls stable: {s['AA_stable']}.",
             'Speedup = previous/current; above 1 favors the current build. Positive reduction means less time.',
             'Disabled control disables R10--R12 and retains R1--R9.',
             'Graptor values are saved same-PC six-worker measurements, with double PageRank. They are unpaired descriptive references.', '',
             'Algorithm | E2E geomean speedup | Summed E2E reduction | Kernel geomean speedup | Summed kernel reduction']
    for a, x in s['by_algorithm'].items():
        lines.append(f"{a} | {x['geomean_speedup_baseline_over_candidate']:.4f}x | {x['summed_time_reduction_percent']:+.2f}% | {x['kernel_geomean_speedup']:.4f}x | {x['kernel_summed_time_reduction_percent']:+.2f}%")
    regressions = s['confirmed_slowdowns']
    lines += ['', f"Corrected slowdown findings: {len(regressions) if regressions is not None else 'inference unavailable because stability controls failed'}.",
              'One warmup per variant/case/block; five primary and twenty independent confirmation observations.',
              'Interleaved rotating/reversed order. Paired log-ratio sign permutations: seed 2358, 100000 resamples, Holm correction over 244 cases plus four algorithm summaries per family.',
              'Every raw observation and any bounded stability-control retry are retained. No implementation changes or merges are made from these measurements. Graptor launches: zero.']
    text = '\n'.join(lines) + '\n'
    (OUT / 'final_report.txt').write_text(text)
    doc = '<!doctype html><meta charset="utf-8"><title>Semantic-stage work elimination benchmark</title><style>body{font:16px system-ui;margin:30px;line-height:1.5}pre{white-space:pre-wrap}td,th{padding:6px;border:1px solid #bbb}table{border-collapse:collapse}</style><h1>Semantic-stage work elimination benchmark</h1><pre>' + html.escape(text) + '</pre><p><a href="per_graph_times.csv">Per-graph CSV</a> · <a href="final_summary.json">Complete summary</a></p>'
    for a in R.ALGS:
        doc += '<h2>' + a + '</h2><table><tr><th>Graph</th><th>Previous ms</th><th>Current ms</th><th>Change</th><th>Saved Graptor ms</th><th>Kernel speedup</th></tr>'
        for x in rows:
            if x['algorithm'] != a:
                continue
            t, k = x['mean_ms'], x['kernel_mean_ms']
            values = [x['graph'], f"{t['baseline']:.4f}", f"{t['candidate']:.4f}",
                      f"{100*(t['candidate']/t['baseline']-1):+.2f}%", f"{x['historical_graptor_ms']:.4f}", f"{k['baseline']/k['candidate']:.3f}x"]
            doc += '<tr>' + ''.join('<td>' + html.escape(v) + '</td>' for v in values) + '</tr>'
        doc += '</table>'
    (OUT / 'final_report.html').write_text(doc)
    R.save(OUT / 'final_summary.json', dict(protocol=protocol, selected_phase=chosen,
        blocks={k: v['summary'] for k, v in results.items()}, cases=rows, graptor_new_launches=0))
    print(text, flush=True)


def audit(protocol):
    selected = R.read(OUT / 'final_summary.json')
    total, warmups, all_bases, blocks = 0, 0, set(), {}
    for phase, summary in selected['blocks'].items():
        n = summary['repetitions']
        for suffix, expected_n in [(phase, n), ('warmup_' + phase, 1)]:
            if suffix == phase:
                observations = R.read(OUT / (phase + '.json'))
            else:
                observations = [R.read(p) for p in (ROOT / 'runs/execution_simplification' / suffix).rglob('*.record.json')]
            assert len(observations) == 244 * 5 * expected_n
            groups = collections.defaultdict(list)
            for x in observations:
                assert x['phase'] == suffix and x['workers'] == 6 and not x.get('error')
                assert x['base'] not in all_bases
                all_bases.add(x['base'])
                assert math.isfinite(x['seconds']) and x['seconds'] > 0
                assert math.isfinite(x['internal_seconds']) and x['internal_seconds'] > 0
                assert R.read(Path(x['base'] + '.record.json')) == x
                stamp = R.read(Path(x['base'] + '.stamp.json'))
                assert stamp['exit_code'] == 0 and stamp['seconds'] == x['seconds']
                groups[x['algorithm'], x['graph'], x['label']].append(x['repeat'])
            assert len(groups) == 1220 and all(sorted(v) == list(range(expected_n)) for v in groups.values())
            if suffix == phase:
                total += len(observations)
            else:
                warmups += len(observations)
        blocks[phase] = dict(observations=summary['observations'], AA_stable=summary['AA_stable'],
                             AA_failures=summary['AA_failures'], confirmed_slowdowns=summary['confirmed_slowdowns'])
    fingerprints = R.read(OUT / 'timed_fingerprints.json')
    assert len(fingerprints) == 488
    for name, digest in fingerprints.items():
        assert R.sha(ROOT / name) == digest
    assert R.git('rev-parse', 'ARS-1') == protocol['accepted_ARS1_checkpoint']
    assert R.sha(Path.home() / '.config/sgpl/hw_calib.json') == protocol['calibration_sha256']
    final = selected['blocks'][selected['selected_phase']]
    evidence = dict(native_candidate_commit=protocol['candidate_commit'], baseline_commit=protocol['baseline_commit'],
        comparison='Incremental semantic-stage simplification: previous R1--R9 versus eligible R10--R12.',
        candidate_source_hash=protocol['source_hash'], host=protocol['host'], cases=244, graphs_per_algorithm=61,
        primary_repetitions=5, confirmation_repetitions=20, selected_phase=selected['selected_phase'],
        timing_observations=total, warmup_launches=warmups, failed_launches=0, graptor_new_launches=0,
        historical_graptor=protocol['historical_graptor'], paired_controls=blocks,
        by_algorithm=final['by_algorithm'], confirmed_slowdowns=final['confirmed_slowdowns'],
        inference_available=final['AA_stable'], timed_executable_fingerprints_verified=488,
        ratio_direction='Speedup=previous/current; above one favors current. Graptor ratio=current/saved Graptor; below one favors current.',
        historical_comparison_limitation='Saved same-PC/input six-worker observations from a previous session; descriptive only.',
        artifacts=dict(root=str(OUT), html=str(OUT / 'final_report.html'), csv=str(OUT / 'per_graph_times.csv'),
                       full_summary=str(OUT / 'final_summary.json'), raw_records=str(ROOT / 'runs/execution_simplification')),
        native_changes_during_measurement=False, merged_into_ARS1=False)
    R.save(OUT / 'integrity_audit.json', evidence)
    R.save(REPO / 'reports/semantic_stage_simplification_benchmark.json', evidence)
    R.save(OUT / 'SHA256SUMS.json', {p.name: R.sha(p) for p in OUT.iterdir() if p.is_file()
        and p.suffix in {'.json', '.html', '.txt', '.csv'} and p.name not in {'SHA256SUMS.json', 'status.json'}})
    print(json.dumps(dict(audit_passed=True, timing_observations=total, by_algorithm=final['by_algorithm']), indent=2), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('stage', choices=['build', 'measure', 'report', 'audit', 'status'])
    args = parser.parse_args()
    if args.stage == 'status':
        print(json.dumps(R.read(OUT / 'status.json'), indent=2))
        raise SystemExit
    with Path('/home/kazi/p1_campaign_20261019/experiment.lock').open('a+') as global_lock, \
         Path('/home/kazi/p1_execution_simplification/experiment.lock').open('a+') as correctness_lock:
        fcntl.flock(global_lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        fcntl.flock(correctness_lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        with R.locked('semantic_stage_' + args.stage):
            protocol, graptor = prepare()
            if args.stage == 'build':
                build()
            elif args.stage == 'audit':
                audit(protocol)
            else:
                results = {}
                for phase, reps in [('primary', 5), ('confirmation', 20)]:
                    if args.stage == 'measure':
                        B.measure(phase, reps)
                    results[phase] = B.analyze(phase, graptor)
                    if not results[phase]['summary']['AA_stable']:
                        retry = phase + '_retry1'
                        if args.stage == 'measure':
                            B.measure(retry, reps)
                        results[retry] = B.analyze(retry, graptor)
                report(protocol, results)
                audit(protocol)
                R.progress(stage='complete', completed=244, total=244)
