"""Private, controlled Kaggle follow-up for uncertified-program regressions.

Uses the original frozen campaign bundle. No production patch is reverted.
Component variants are diagnostic, ABI-compatible and validated before timing.
"""
from pathlib import Path
import collections
import hashlib
import html
import importlib.util
import json
import math
import os
import random
import re
import shutil
import statistics
import struct
import subprocess
import sys
import time

BUNDLE = Path(os.environ['P1_DIAGNOSTIC_BUNDLE'])
sys.path.insert(0, str(BUNDLE / 'candidate/tools/p1_campaign'))
import portable_campaign as P

REGRESSIONS = {
    'bfs': ['dimacs/brock400-1.edges', 'socfb/socfb-USFCA72.edges',
            'socfb/socfb-Amherst41.edges', 'socfb/socfb-Williams40.edges',
            'dimacs10/wing.edges', 'socfb/socfb-Brandeis99.edges'],
    'sssp': ['bio/bio-WormNet-v3-benchmark.edges', 'dimacs10/wing.edges',
             'socfb/socfb-Brandeis99.edges', 'soc/soc-slashdot.edges'],
    'cc': ['socfb/socfb-Haverford76.edges', 'dimacs/brock400-4.edges',
           'dimacs10/wing_nodal.edges', 'socfb/socfb-Williams40.edges',
           'dimacs10/inf-luxembourg_osm.edges', 'road/road-luxembourg-osm.edges'],
}
CONTROLS = {
    'bfs': ['dimacs/hamming8-2.edges', 'road/road-usroads-48.edges'],
    'sssp': ['dimacs/hamming8-2.edges', 'road/road-usroads-48.edges'],
    'cc': ['dimacs/hamming8-2.edges', 'road/road-usroads-48.edges'],
}
VARIANTS = ['accepted', 'candidate', 'layout_only', 'api_only', 'outlined', 'guarded']
CONTRASTS = [
    ('accepted', 'candidate', 'full change'),
    ('accepted', 'layout_only', 'context/ABI only'),
    ('layout_only', 'api_only', 'unexecuted API/code placement'),
    ('api_only', 'candidate', 'dispatcher change'),
    ('candidate', 'outlined', 'outline helper'),
    ('outlined', 'guarded', 'avoid call for uncertified jobs'),
    ('accepted', 'guarded', 'combined recovery'),
]


def save(p, obj):
    P.save(p, obj)


def source_files(src, dst):
    dst.mkdir(parents=True, exist_ok=False)
    for p in src.iterdir():
        if p.is_file() and p.suffix in ['.c', '.cpp', '.h', '.inc', '.ll']:
            shutil.copy2(p, dst / p.name)


def elf_sections(path):
    data = Path(path).read_bytes()
    assert data[:6] == b'\x7fELF\x02\x01'
    offset = struct.unpack_from('<Q', data, 40)[0]
    stride, count, strings = struct.unpack_from('<HHH', data, 58)
    headers = [struct.unpack_from('<IIQQQQIIQQ', data, offset + i * stride)
               for i in range(count)]
    h = headers[strings]
    names = data[h[4]:h[4]+h[5]]
    result = {}
    for h in headers:
        name = names[h[0]:].split(b'\0', 1)[0].decode()
        if h[1] != 8:
            result[name] = data[h[4]:h[4]+h[5]]
    return result


def text_audit(a, b):
    left, right = elf_sections(a), elf_sections(b)
    names = sorted(n for n in left if n.startswith('.text'))
    return {n: dict(accepted_bytes=len(left[n]), candidate_bytes=len(right.get(n, b'')),
                    differences=[dict(offset=i, accepted=x, candidate=y)
                                 for i, (x, y) in enumerate(zip(left[n], right.get(n, b'')))
                                 if x != y]) for n in names}


def function_symbols(path):
    result = {}
    for line in subprocess.check_output(['nm', '-S', str(path)], text=True).splitlines():
        parts = line.split()
        if len(parts) == 4 and parts[2].lower() == 't':
            result[parts[3]] = dict(address=int(parts[0], 16), bytes=int(parts[1], 16))
    return result


def build(R, selected):
    graphs = {x['graph'] for x in selected}
    R.M.ROWS = [r for r in R.M.ROWS if r['graph'] in graphs]
    R.ALGS = ['bfs', 'sssp', 'cc']
    for version, source in [('Accepted', 'accepted'), ('Candidate', 'candidate')]:
        R.REPO = BUNDLE / source
        R.build(version)
    versions = {'accepted': 'Accepted', 'candidate': 'Candidate'}
    candidate = (BUNDLE / 'candidate/autotuner_runtime.c').read_text()
    original = (BUNDLE / 'accepted/autotuner_runtime.c').read_text()
    include = '#include "static_schedule_runtime.inc"'
    call = 'int static_prepared=sgpl_static_prepare_owners(ctx,partitions,relation_count,worker_budget);'
    assert candidate.count(call) == 1
    for name in ['layout_only', 'api_only', 'outlined', 'guarded']:
        version = 'Diagnostic_' + name
        versions[name] = version
        d = R.ROOT / 'versions' / version
        d.mkdir()
        src = d / 'src'
        source_files(BUNDLE / 'candidate', src)
        if name == 'layout_only':
            text = original
        elif name == 'api_only':
            text = original.replace('#include "fullgraph_dispatch.inc"',
                                    '#include "fullgraph_dispatch.inc"\n' + include, 1)
        else:
            text = candidate
            p = src / 'static_schedule_runtime.inc'
            code = p.read_text()
            needle = 'static int sgpl_static_prepare_owners('
            assert code.count(needle) == 1
            p.write_text(code.replace(needle, 'static __attribute__((noinline)) int sgpl_static_prepare_owners(', 1))
            if name == 'guarded':
                text = text.replace(call, 'int static_prepared=0;\n  if (__builtin_expect(ctx->static_schedule!=NULL,0))\n    static_prepared=sgpl_static_prepare_owners(ctx,partitions,relation_count,worker_budget);', 1)
        (src / 'autotuner_runtime.c').write_text(text)
        logs = d / 'build_logs'
        logs.mkdir()
        for obj in R.OBJECTS:
            ext = '.cpp' if obj in ['roaring_bitmap', 'graph_loader_runtime', 'graph_runtime'] else '.c'
            flags = ['g++', '-std=c++17', '-mavx2', '-march=native'] if ext == '.cpp' else ['gcc']
            output = R.compile_cache(src / (obj + ext), src,
                                     [*flags, '-O3', '-g', '-fopenmp', '-pthread', '-iquote', '.'],
                                     logs / (obj + '.log'))
            (d / (obj + '.o')).symlink_to(output)
        executables = {}
        for item in selected:
            row = next(r for r in R.M.ROWS if r['graph'] == item['graph'])
            folder = R.case(version, item['algorithm'], row)
            folder.mkdir(parents=True)
            driver = R.case('Candidate', item['algorithm'], row)
            for mode in ['timed', 'validate']:
                (folder / (mode + '.o')).symlink_to(driver / (mode + '.o'))
                R.checked(['g++', '-O3', '-fopenmp', '-no-pie', driver / (mode + '.o'),
                           *[d / (obj + '.o') for obj in R.OBJECTS], R.ROOT / 'monotonic_timer.o',
                           '-Wl,--wrap=timer_runtime', '-pthread', '-ldl', '-lnlopt', '-o', folder / mode],
                          folder, folder / (mode + '.link.log'))
                executables[str((folder / mode).relative_to(d))] = R.sha(folder / mode)
        save(d / 'build.json', dict(fingerprint=R.key(executables), executables=executables,
                                    diagnostic=True, mutation=name))
        R.progress(stage='component_build', version=version, completed=len(executables))
    audit = []
    for item in selected:
        row = next(r for r in R.M.ROWS if r['graph'] == item['graph'])
        for mode in ['timed', 'validate']:
            a = R.case('Accepted', item['algorithm'], row) / (mode + '.o')
            b = R.case('Candidate', item['algorithm'], row) / (mode + '.o')
            audit.append(dict(item, mode=mode, sections=text_audit(a, b)))
            undefined = subprocess.check_output(['nm', '-u', str(b)], text=True)
            assert 'bind_static_schedule' not in undefined, item
    save(R.OUT / 'inactive_program_object_audit.json', audit)
    metadata = {}
    for name, version in versions.items():
        d = R.ROOT / 'versions' / version
        p = d / 'autotuner_runtime.o'
        symbols = function_symbols(p)
        metadata[name] = dict(runtime_sha256=R.sha(p), symbols=symbols)
        R.checked(['objdump', '-drwC', '-Mintel', p], d, d / 'autotuner_runtime.asm')
        c = d / 'context_size.c'
        c.write_text('#include "autotuner_runtime.h"\n#include <stddef.h>\n#include <stdio.h>\nint main(){printf("{\\"context_bytes\\":%zu,\\"operation_bytes\\":%zu,\\"last_old_field_offset\\":%zu}\\n",sizeof(sgpl_exec_ctx),sizeof(sgpl_runtime_op),offsetof(sgpl_exec_ctx,array_compact_records));}\n')
        R.checked(['gcc', '-I', d / 'src', c, '-o', d / 'context_size'], d, d / 'context_size.log')
        metadata[name]['context'] = json.loads(subprocess.check_output([d / 'context_size'], text=True))
    save(R.OUT / 'runtime_code_audit.json', metadata)
    return versions


def timing(R, cpus, selected, versions):
    configurations = [(name, v, {}) for name, v in versions.items()]
    configurations += [('AA', 'Accepted', {}), ('disabled', 'Candidate', {'SGPL_NO_STATIC_SCHEDULE': '1'})]
    records = []
    rng = random.Random(20261020)
    for workers in sorted(set([1, len(cpus)])):
        for item in selected:
            row = next(r for r in R.M.ROWS if r['graph'] == item['graph'])
            for name, version, extra in configurations:
                x = R.invoke('inactive_diagnosis', 'validate_components', version, item['algorithm'], row, 0,
                             label=name, workers=workers, extra=extra)
                assert x.get('validation') and not x.get('error'), x
        for block in range(2):
            for a in ['bfs', 'sssp', 'cc']:
                cases = [x for x in selected if x['algorithm'] == a]
                started = time.monotonic()
                for i, item in enumerate(cases):
                    row = next(r for r in R.M.ROWS if r['graph'] == item['graph'])
                    for name, version, extra in configurations:
                        x = R.invoke('inactive_diagnosis', f'warmup_components_{block}', version, a, row, 0,
                                     label=name, workers=workers, extra=extra)
                        assert not x.get('error'), x
                    # Independent blocks and randomized complete cycles. Exact
                    # invocation order and every resource measurement are retained.
                    for rep in range(20):
                        order = list(configurations)
                        rng.shuffle(order)
                        for position, (name, version, extra) in enumerate(order):
                            x = R.invoke('inactive_diagnosis', f'components_{block}', version, a, row, rep,
                                         label=name, workers=workers, extra=extra)
                            assert not x.get('error'), x
                            x = dict(x, position=position, independent_block=block)
                            records.append(x)
                    save(R.OUT / 'components.json', records)
                    R.progress(stage='component_timing', algorithm=a, workers=workers, block=block,
                               completed=i+1, total=len(cases), elapsed_seconds=time.monotonic()-started)
                save(R.OUT / f'components_w{workers}_b{block}_{a}_duration.json',
                     dict(seconds=time.monotonic()-started, cases=len(cases), repetitions=20, variants=8))
    return records


def analyse(R, records, selected):
    import numpy as np
    from scipy.stats import permutation_test
    grouped = collections.defaultdict(list)
    for x in records:
        grouped[(x['algorithm'], x['graph'], x['workers'], x['independent_block'], x['label'])].append(x)
    values, tests, controls = [], [], []
    for a, graph, workers, block in sorted({k[:4] for k in grouped}):
        rows = {}
        for label in [*VARIANTS, 'AA', 'disabled']:
            xs = sorted(grouped[(a, graph, workers, block, label)], key=lambda x: x['repeat'])
            assert len(xs) == 20
            rows[label] = xs
        value = dict(algorithm=a, graph=graph, workers=workers, block=block, variants={})
        for label, xs in rows.items():
            value['variants'][label] = {
                'whole_ms': statistics.mean(x['seconds'] for x in xs)*1000,
                'kernel_ms': statistics.mean(x['internal_seconds'] for x in xs)*1000,
                'cpu_ms': statistics.mean(x['user_seconds']+x['system_seconds'] for x in xs)*1000,
                'voluntary_switches': statistics.mean(x['nvcsw'] for x in xs),
                'involuntary_switches': statistics.mean(x['nivcsw'] for x in xs),
            }
        values.append(value)
        for left, right, meaning in [*CONTRASTS, ('candidate', 'disabled', 'feature disable'), ('accepted', 'AA', 'A/A')]:
            delta = np.log([x['seconds'] for x in rows[right]]) - np.log([x['seconds'] for x in rows[left]])
            p = 1.0 if np.all(delta == 0) else float(permutation_test((delta,), np.mean,
                permutation_type='samples', alternative='two-sided', n_resamples=100000,
                random_state=np.random.default_rng(2358)).pvalue)
            test = dict(algorithm=a, graph=graph, workers=workers, block=block,
                        left=left, right=right, meaning=meaning, p=p,
                        geometric_percent=(math.exp(float(np.mean(delta)))-1)*100)
            (controls if meaning == 'A/A' else tests).append(test)
    for family in [tests, controls]:
        running = 0
        for i, t in enumerate(sorted(family, key=lambda t: t['p'])):
            running = max(running, min(1, t['p']*(len(family)-i)))
            t['holm_p'] = running
    save(R.OUT / 'component_summary.json', dict(cases=values, tests=tests, aa_tests=controls,
         method='Independent 20-pair blocks; two-sided paired log-ratio sign-flip permutation, 100000 resamples seed2358; Holm across all component contrasts, separate A/A family.',
         limitation='A new Kaggle VM can differ from the original host. An unreproduced slowdown is not an established source-code cause. Instrumented runs are separate.'))
    return values


def profile(R, cpus, selected, versions):
    module_path = Path(os.environ['P1_DIAGNOSTIC_PROFILE_MODULE'])
    spec = importlib.util.spec_from_file_location('diagnostic_profiles', module_path)
    profiles = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(profiles)
    targets = [x for x in selected if (x['algorithm'], x['graph']) in {
        ('cc', 'dimacs10/inf-luxembourg_osm.edges'),
        ('cc', 'dimacs10/wing_nodal.edges'),
        ('bfs', 'dimacs10/wing.edges'),
        ('sssp', 'dimacs10/wing.edges')}]
    observations = []
    for name in ['accepted', 'candidate', 'layout_only', 'api_only', 'guarded']:
        version = versions[name]
        profiles.instrument(R, version, targets)
        for workers in sorted(set([1, len(cpus)])):
            started = time.monotonic()
            for item in targets:
                row = next(r for r in R.M.ROWS if r['graph'] == item['graph'])
                valid = R.invoke('inactive_diagnosis', 'validate_profile', version, item['algorithm'], row, 0,
                                 label=name, workers=workers, profile='phases')
                assert valid.get('validation') and not valid.get('error'), valid
                for rep in range(5):
                    x = R.invoke('inactive_diagnosis', 'profile_components', version, item['algorithm'], row,
                                 rep, label=name, workers=workers, profile='phases')
                    assert not x.get('error'), x
                    observations.append(x)
                save(R.OUT / 'component_profiles.json', observations)
            R.progress(stage='component_profile', version=version, workers=workers,
                       completed=len(targets), total=len(targets), elapsed_seconds=time.monotonic()-started)
    # Bounded traces and counter availability are evidence, never acceptance data.
    diagnostics = []
    for name in ['accepted', 'candidate', 'layout_only', 'guarded']:
        item = next(x for x in targets if x['algorithm'] == 'cc' and 'luxembourg' in x['graph'])
        row = next(r for r in R.M.ROWS if r['graph'] == item['graph'])
        exe = R.case(versions[name], item['algorithm'], row) / 'timed'
        for tool, args in [('perf', ['stat', '-x,', '-e', 'cycles:u,instructions:u,cache-misses:u,branch-misses:u', '--']),
                           ('strace', ['-f', '-c', '-e', 'futex'])]:
            if not shutil.which(tool):
                diagnostics.append(dict(variant=name, tool=tool, unavailable=True))
                continue
            env = R.M.env(workers=len(cpus))
            os.sched_setaffinity(0, cpus)
            p = subprocess.run(['timeout', '20', tool, *args, str(exe)], cwd=exe.parent,
                               env=env, capture_output=True, text=True, timeout=25)
            path = R.OUT / f'{tool}_{name}.txt'
            path.write_text(p.stdout + '\n' + p.stderr)
            diagnostics.append(dict(variant=name, tool=tool, returncode=p.returncode, output=str(path)))
    save(R.OUT / 'counter_trace_availability.json', diagnostics)


def report(R, values):
    doc = ['<!doctype html><meta charset="utf-8"><title>Inactive-program component diagnosis</title>',
           '<style>body{font:14px system-ui;margin:24px}table{border-collapse:collapse}td,th{padding:5px;border:1px solid #bbb;text-align:right}td:first-child{text-align:left}.scroll{overflow:auto}</style>',
           '<h1>Uncertified-program component diagnosis</h1>',
           '<p>Two independent 20-pair blocks, randomized invocation order. Only distinct physical cores. All component binaries validated. Times are mean whole-process milliseconds. These diagnostic variants do not replace the production candidate.</p>']
    for workers in sorted({x['workers'] for x in values}):
        for block in [0, 1]:
            doc += [f'<h2>{workers} workers, block {block+1}</h2><div class="scroll"><table><tr><th>Case</th>',
                    *[f'<th>{html.escape(n)}</th>' for n in [*VARIANTS, 'AA', 'disabled']], '</tr>']
            for x in values:
                if x['workers'] != workers or x['block'] != block:
                    continue
                doc.append('<tr><td>'+html.escape(x['algorithm']+' '+x['graph'])+'</td>'+''.join(
                    f'<td>{x["variants"][n]["whole_ms"]:.4f}</td>' for n in [*VARIANTS, 'AA', 'disabled'])+'</tr>')
            doc.append('</table></div>')
    doc.append('<p>See component_summary.json for corrected contrasts and A/A controls; component_profiles.json for separately instrumented phases, work and waiting. The raw journal and binary/source identities are preserved.</p>')
    (R.OUT / 'component_report.html').write_text('\n'.join(doc))


def main():
    discover = P.physical_cpus
    def two_physical_cpus():
        cpus, host = discover()
        cpus = cpus[:2]
        host.update(physical_cpus=cpus, workers=len(cpus))
        return cpus, host
    P.physical_cpus = two_physical_cpus
    R, cpus = P.setup()
    assert len(cpus) >= 2, 'Need two distinct physical cores to match the original worker count'
    cpus = cpus[:2]
    assert R.CPUS == cpus
    selected = [dict(algorithm=a, graph=g, cohort='regression' if g in REGRESSIONS[a] else 'control')
                for a in ['bfs', 'sssp', 'cc'] for g in [*REGRESSIONS[a], *CONTROLS[a]]]
    with R.locked('diagnose_static_schedule_inactive'):
        shutil.copy2(BUNDLE / 'walltime.c', R.ROOT / 'walltime.c')
        timer = R.ROOT / 'timer.c'
        timer.write_text('#include <time.h>\ndouble __wrap_timer_runtime(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}\n')
        R.checked(['gcc', '-O2', '-c', timer, '-o', R.ROOT / 'monotonic_timer.o'],
                  R.ROOT, R.OUT / 'timer.build.log')
        save(R.OUT / 'diagnostic_protocol.json', dict(selected=selected, physical_cpus=cpus,
             repetitions=20, independent_blocks=2, widths=[1, 2], variants=VARIANTS,
             seed=20261020, original_bundle_manifest=P.sha(BUNDLE/'bundle_manifest.json'),
             mutations=dict(layout_only='Accepted C body with candidate ABI4 header',
                            api_only='Layout-only plus static API definitions, unchanged original dispatcher',
                            outlined='Candidate, helper forced out of dispatcher',
                            guarded='Outlined plus explicit unlikely nonnull-certificate guard')))
        versions = build(R, selected)
        hashes = {name: R.verify_fingerprint(v) for name, v in versions.items()}
        save(R.OUT / 'diagnostic_fingerprints.json', hashes)
        records = timing(R, cpus, selected, versions)
        values = analyse(R, records, selected)
        report(R, values)
        profile(R, cpus, selected, versions)
        assert hashes == {name: R.verify_fingerprint(v) for name, v in versions.items()}
        R.progress(stage='diagnosis_complete', completed=len(records), failures=0)


if __name__ == '__main__':
    main()
