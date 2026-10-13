#!/usr/bin/env python3
"""Paired Phase A summaries, retaining startup costs and every workload."""
import csv
import hashlib
import json
import math
import random
import statistics as stats
import subprocess
from tdg_joint_build import ROOT, OUT, SNAP


def geometric(values):
    return math.exp(stats.mean(math.log(v) for v in values))


def analyze(path):
    rows = list(csv.DictReader(path.open()))
    groups = {}
    for row in rows:
        key = (int(row['p']), row['case'])
        groups.setdefault(key, {}).setdefault(int(row['run']), {})[row['policy']] = row
    result = {}
    for metric in ('steady_ns', 'first_ns', 'all14_ns'):
        paired = []
        for key, runs in groups.items():
            paired.append([float(arms['joint'][metric]) / float(arms['reference'][metric])
                           for arms in runs.values()])
        estimate = geometric([geometric(v) for v in paired])
        rng = random.Random(20261008)
        bootstrap = []
        # Resample workload/budget strata, then paired process runs within them.
        # This includes workload variation as well as timing noise on this host.
        for _ in range(10000):
            sample = [rng.choice(paired) for _ in paired]
            bootstrap.append(geometric([geometric([rng.choice(v) for _ in v]) for v in sample]))
        bootstrap.sort()
        result[metric] = dict(ratio=estimate, ci95=[bootstrap[249], bootstrap[9749]])
    cases = []
    for (budget, name), runs in sorted(groups.items()):
        policies = list(next(iter(runs.values())))
        medians = {p: stats.median(float(v[p]['steady_ns']) for v in runs.values()) for p in policies}
        paired = [float(v['joint']['steady_ns']) / float(v['reference']['steady_ns']) for v in runs.values()]
        cases.append(dict(budget=budget, case=name, median_ns=medians,
                          median_ratio=medians['joint']/medians['reference'],
                          paired_ratios=paired,
                          repeatable_10pct_regression=all(v > 1.1 for v in paired)))
    result['cases'] = cases
    result['non_tiny_steady_ratio'] = geometric([
        float(v['joint']['steady_ns'])/float(v['reference']['steady_ns'])
        for key, runs in groups.items() if key[1] != 'tiny' for v in runs.values()])
    result['ablation_geomeans'] = {}
    result['paired_policy_metrics'] = {}
    for policy in ('ordering', 'overload', 'previous'):
        if all(policy in v for runs in groups.values() for v in runs.values()):
            result['ablation_geomeans'][policy] = geometric([
                float(v[policy]['steady_ns'])/float(v['reference']['steady_ns'])
                for runs in groups.values() for v in runs.values()])
            result['paired_policy_metrics'][policy] = {
                metric: dict(policy_reference_ratio=geometric([
                    float(v[policy][metric])/float(v['reference'][metric])
                    for runs in groups.values() for v in runs.values()]),
                    joint_policy_ratio=geometric([
                    float(v['joint'][metric])/float(v[policy][metric])
                    for runs in groups.values() for v in runs.values()]))
                for metric in ('steady_ns', 'first_ns', 'all14_ns')}
    # A descriptive comparison of medians, not per-invocation prediction error:
    # the predictor median covers known-profile invocations only.
    proxy = []
    for row in rows:
        if row['policy'] == 'joint' and int(row['predicted_samples']) >= 9 and float(row['steady_ns']) >= 1e6:
            proxy.append(abs(float(row['level_prediction_ns']) / float(row['steady_ns']) - 1))
    result['prediction_median_proxy'] = dict(eligible_processes=len(proxy),
        median_relative_error=stats.median(proxy) if proxy else None)
    result['policy_counts'] = {p: {key: sum(int(r[key]) for r in rows if r['policy'] == p)
        for key in ('cache_hits', 'plans', 'reference_calls', 'calibration_runs', 'revalidations',
                    'learning_ns', 'evaluations') if key in rows[0]}
        for p in ('reference', 'joint', 'ordering', 'overload') if any(r['policy'] == p for r in rows)}
    return result


def main():
    results = {name: analyze(OUT/(name+'.csv')) for name in ('benchmark', 'cold')}
    (OUT/'analysis.json').write_text(json.dumps(results, indent=2)+'\n')
    lines = []
    for name, result in results.items():
        lines.append(name)
        for metric in ('steady_ns', 'first_ns', 'all14_ns'):
            value = result[metric]
            lines.append(f"  {metric}: joint/reference={value['ratio']:.4f}, hierarchical paired 95% CI={value['ci95']}")
        lines.append(f"  ablations: {result['ablation_geomeans']}")
        lines.append(f"  excluding tiny: {result['non_tiny_steady_ratio']:.4f}")
        if 'previous' in result['paired_policy_metrics']:
            lines.append('  paired previous implementation: '+str(result['paired_policy_metrics']['previous']))
        lines.append('  joint diagnostic totals: '+str(result['policy_counts'].get('joint')))
        bad = [f"B={c['budget']} {c['case']}" for c in result['cases'] if c['repeatable_10pct_regression']]
        lines.append('  all-three-process >10% regressions: '+(', '.join(bad) or 'none'))
        lines.append('  prediction median proxy: '+str(result['prediction_median_proxy']))
    (OUT/'analysis.txt').write_text('\n'.join(lines)+'\n')
    manifest = json.loads((SNAP/'manifest.json').read_text())
    for name, digest in manifest['snapshot_sha256'].items():
        assert hashlib.sha256((SNAP/'source'/name).read_bytes()).hexdigest() == digest, name
    files = ['main.cpp', 'pdg.cpp', 'parallel_runtime.c', 'parallel_runtime.h',
             'experimental/tdg_joint_trace.inc', 'experimental/tdg_joint_schedule.inc',
             'autotuner_runtime.c', 'AutoTunerPass.cpp', 'parallel_loop_outline.cpp',
             'graph_frontier_lowering.cpp']
    files += [str(p.relative_to(ROOT)) for p in sorted((ROOT/'tools').glob('tdg_joint*')) if p.is_file()]
    metadata = dict(source_sha256={f: hashlib.sha256((ROOT/f).read_bytes()).hexdigest() for f in files},
        machine=subprocess.check_output(['uname', '-a'], text=True).strip(),
        cpu=subprocess.check_output(['lscpu'], text=True),
        gcc=subprocess.check_output(['gcc', '--version'], text=True).splitlines()[0],
        frozen_reference_verified=True)
    for name in ('parallel_loop_outline.cpp', 'graph_frontier_lowering.cpp'):
        assert metadata['source_sha256'][name] == manifest['context_sha256'][name], name
    (OUT/'final_source_manifest.json').write_text(json.dumps(metadata, indent=2)+'\n')
    print('\n'.join(lines))


if __name__ == '__main__':
    main()
