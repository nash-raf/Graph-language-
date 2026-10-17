#!/usr/bin/env python3
"""Rebenchmark the exact five saved workflows with default compiler lowering."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess

import layout_application_cases as cases
import layout_application_benchmark as bench
from layout_switch_benchmark import ROOT, MODES, run, environment
from audit_layout_applications import signature

OUT = ROOT / 'bench_results/layout_applications_default_20261015'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def audit(items):
    work, objs = bench.objects()
    wrapper = work / 'default_application_audit.o'
    run(['gcc', '-O2', '-iquote', ROOT, '-iquote', ROOT/'tools', '-c',
         ROOT/'tools/default_layout_application_audit.c', '-o', wrapper], work)
    wraps = ['autograph_ensure_layout', 'graph_add_edge', 'graph_remove_edge',
             'bfs_runtime_src', 'dfs_runtime_src', 'autograph_edgemap',
             'autograph_frontier_execute', 'autograph_frontier_fork_join',
             'autograph_neighbor_iter_init', 'autograph_neighbor_iter_next']
    for base, src, meta in items:
        dest = OUT/'binaries'/(base+'_default')
        pairs = set(map(tuple, meta['final_pairs']))
        sigs = {signature(pairs)}
        if meta['kind'] == 'route_failover':
            sigs.add(signature(pairs-{(1, 512)}))
        multiplier = 4 if meta['kind'] in ('contact_trace', 'route_failover') else 2
        stream_ops = meta['rounds']*meta['batch']*multiplier
        adds = removes = stream_ops//2
        if meta['kind'] == 'contact_trace': adds += 8; removes += 8
        elif meta['kind'] == 'sensor_backbone': removes += 13
        elif meta['kind'] == 'route_failover': adds += 1; removes += 1
        else: removes += 1
        audits = {}
        for mode in MODES:
            exe = dest/(mode+'.audit')
            run(['g++', '-fopenmp', '-pthread', '-no-pie', dest/(mode+'.program.o'),
                 wrapper, *objs, *[f'-Wl,--wrap={symbol}' for symbol in wraps],
                 '-ldl', '-lnlopt', '-o', exe], work)
            env = environment()
            if mode != 'AUTO': env['BENCH_FIXED_LAYOUT'] = str(MODES.index(mode)-1)
            env.update(AUDIT_STREAM_OPERATIONS=str(stream_ops),
                       AUDIT_WINDOW_OPERATIONS=str(meta['batch']*multiplier),
                       AUDIT_FULL_NATIVE='0' if mode in ('CSR', 'BCSR') else '1')
            cp = run([exe], OUT/'codes', env, 600)
            (dest/(mode+'.audit.log')).write_text(cp.stdout+cp.stderr)
            bench.verify(cp.stdout, bench.expected(meta))
            observed = [(int(a), int(b)) for a, b in re.findall(
                r'AUDIT snapshot.*arcs=(\d+) digest=(\d+)', cp.stderr)]
            assert observed and all(s in sigs for s in observed), (base, mode, observed)
            counts = list(map(int, re.search(
                r'AUDIT counts adds=(\d+) removes=(\d+) samples=(\d+) native=(\d+)', cp.stderr).groups()))
            assert counts[:2] == [adds, removes], (base, mode, counts)
            transitions = re.findall(r'AUDIT layout (\w+) -> (\w+)', cp.stderr)
            if mode == 'AUTO': assert transitions == [('CSR', 'SET'), ('SET', 'CSR')], (base, transitions)
            assert 'AUDIT iterator epochs stable' in cp.stderr
            audits[mode] = dict(correct=True, layouts_checked_on_all_calls=True,
                graph_fingerprints_verified=True, semantic_outputs_verified=True,
                iterator_epochs_verified=True, transitions=transitions,
                adds=adds, removes=removes, mutation_effect_samples=counts[2],
                native_mutations=counts[3], full_native_updates=mode not in ('CSR', 'BCSR'))
            print('AUDIT', base, mode, transitions, flush=True)
        (dest/'audit.json').write_text(json.dumps(audits, indent=2)+'\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--resume', action='store_true')
    parser.add_argument('--audit-only', action='store_true')
    args = parser.parse_args()
    cases.OUT = OUT
    bench.OUT = OUT
    bench.SETTINGS = {}
    OUT.mkdir(parents=True, exist_ok=True)
    inputs = cases.generate(80000, 8)
    # This experiment changes the backend, not the programs or input graph.
    old = ROOT/'bench_results/layout_applications_20261015'
    for base, src, meta in inputs:
        assert src.read_bytes() == (old/'winning_codes'/(meta['kind']+'.sgpl')).read_bytes()
    assert sha(OUT/'graphs/community_projection.txt') == sha(old/'graphs/community_projection.txt')
    stamp = OUT/'experiment.json'
    sources = ['GraphProgram', 'AutoTunerPass.cpp', 'autotuner_runtime.c', 'parallel_runtime.c',
               'graph_mutation_runtime.c', 'graph_frontier_lowering.cpp', 'bfs_runtime_src.cpp', 'dfs_runtime_src.cpp']
    provenance = dict(backend='Default; GRAPH_FRONTIER_REWRITE_OFF unset for all variants',
        settings={}, input_sha256=sha(OUT/'graphs/community_projection.txt'),
        source_sha256={name:sha(ROOT/name) for name in sources},
        platform=platform.platform(), cpu=subprocess.check_output(['lscpu'], text=True),
        threads=1, sampling={'AUTO':7, 'CSR':1, 'PCSR':7, 'BCSR':1, 'SET':7})
    if stamp.exists():
        previous = json.loads(stamp.read_text())
        assert previous['source_sha256'] == provenance['source_sha256'], 'Compiler/runtime changed; use a fresh experiment directory'
    else:
        stamp.write_text(json.dumps(provenance, indent=2)+'\n')
    if not args.audit_only:
        for item in inputs:
            dest = OUT/'binaries'/(item[0]+'_default')
            if args.resume and (dest/'result.json').exists():
                print('RESUME', item[0], flush=True)
                continue
            result = bench.benchmark(item, MODES, 7, 0, 600,
                {'AUTO':7, 'CSR':1, 'PCSR':7, 'BCSR':1, 'SET':7}, case_suffix='_default')
            assert result['settings'] == {}
            for mode in MODES:
                text = (dest/(mode+'.ll')).read_text()
                assert not re.search(r'call .*@autograph_edgemap\(', text), (item[0], mode)
            print('DEFAULT RESULT', result['kind'], result['speedup_vs_best_fixed'], flush=True)
    audit(inputs)
    print('COMPLETE: five default-backend cases, 115 timed launches, 25 audited variants', flush=True)


if __name__ == '__main__':
    main()
