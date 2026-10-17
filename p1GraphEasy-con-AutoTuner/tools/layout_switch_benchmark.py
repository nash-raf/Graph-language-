#!/usr/bin/env python3
"""Search SGPL layout schedules and measure complete executable wall time.

Run under Linux/WSL from the repository. All output goes into the selected
directory; compiler/runtime sources and existing build artifacts are untouched.
"""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import random
import re
import shutil
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
MODES = ['AUTO', 'CSR', 'PCSR', 'BCSR', 'SET']
NAMES = {0: 'CSR', 1: 'PCSR', 2: 'BCSR', 3: 'SET'}

def run(cmd, cwd, env=None, timeout=600):
    p = subprocess.run(list(map(str, cmd)), cwd=cwd, env=env,
                       capture_output=True, text=True, timeout=timeout)
    if p.returncode:
        raise RuntimeError(f'{cmd}: exit {p.returncode}\n{p.stdout[-1000:]}\n{p.stderr[-2500:]}')
    return p

def environment():
    e = os.environ.copy()
    for key in list(e):
        if key.startswith(('AUTOTUNER_', 'SGPL_', 'GRAPH_')):
            e.pop(key)
    e.update(OMP_NUM_THREADS='1', SGPL_NUM_THREADS='1', SGPL_ROARING_THREADS='1',
             OMP_WAIT_POLICY='PASSIVE', GOMP_SPINCOUNT='0')
    return e

def build(out):
    work = out / 'build'
    work.mkdir(parents=True, exist_ok=True)
    # The compiler reads runtime IR relative to its working directory.
    for source in ROOT.glob('*.ll'):
        target = work / source.name
        if not target.exists():
            target.symlink_to(source)
    objs = []
    for source in ('runtime.c', 'parallel_runtime.c', 'gpu_runtime.c',
                   'autotuner_runtime.c', 'graph_mutation_runtime.c',
                   'semiring_runtime.c', 'roaring_bitmap.cpp',
                   'graph_loader_runtime.cpp', 'graph_runtime.cpp',
                   'bfs_runtime_src.cpp'):
        obj = work / (source + '.o')
        if not obj.exists() or obj.stat().st_mtime < (ROOT / source).stat().st_mtime:
            compiler = 'g++' if source.endswith('.cpp') else 'gcc'
            flags = ['-O3', '-fopenmp', '-iquote', ROOT, '-pthread']
            if compiler == 'g++':
                flags += ['-std=c++17', '-mavx2', '-march=native']
            run([compiler, *flags, '-c', ROOT / source, '-o', obj], work)
        objs.append(obj)
    return work, objs

def data(out, n, degree):
    path = out / 'graphs' / f'ring_n{n}_d{degree}.txt'
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists():
        # Unique undirected ring edges. Reserved update offsets are > degree.
        with path.open('w') as f:
            for u in range(n):
                for delta in range(1, degree + 1):
                    f.write(f'{u} {(u + delta) % n}\n')
    return path

def source(out, n, degree, inserts, scans, shape='count', cycles=1, churn=0, tail_scans=None, query_loops=False):
    name = f'{shape}_n{n}_d{degree}_i{inserts}_s{scans}_c{cycles}_r{churn}'
    if tail_scans is not None: name += f'_t{tail_scans}'
    if query_loops: name += '_loop'
    post_scans = scans if tail_scans is None else tail_scans
    edges = data(out, n, degree)
    p = out / 'codes' / (name + '.sgpl')
    p.parent.mkdir(parents=True, exist_ok=True)
    lines = [f'// SGPL workload: {name}', '// Run all variants on this identical source.',
             'graph G {', f'  edges: file "../graphs/{edges.name}";', '};',
             'int checksum = 0;']
    scan = ['for u in G.nodes {', '  for v in G.neighbors(u) {',
            '    checksum = checksum + 1;', '  }', '}']
    if shape == 'weighted':
        scan[2] = '    checksum = checksum + (v % 7);'
    for cycle in range(cycles):
        if shape in ('bfs', 'dfs', 'mixed'):
            algo = 'dfs' if shape == 'dfs' or (shape == 'mixed' and cycle % 2) else 'bfs'
            scan = [f'query scan_{cycle}_{{index}}: "{algo}_src" of G;']
            initial = [scan[0].format(index=f'pre_{j}') for j in range(scans)]
            final = [scan[0].format(index=f'post_{j}') for j in range(post_scans)]
            if query_loops:
                def query_loop(phase, repeats):
                    return [f'int probe_{cycle}_{phase} = 0;', f'while (probe_{cycle}_{phase} < {repeats}) {{',
                            scan[0].format(index=phase),f'  probe_{cycle}_{phase} = probe_{cycle}_{phase} + 1;', '}']
                initial=query_loop('pre',scans)
                final=query_loop('post',post_scans)
        else:
            initial, final = scan * scans, scan * post_scans
        lines += [f'// Phase {cycle}: initial scans.'] + initial
        lines += [f'// Phase {cycle}: insert a batch of absent in-bounds edges.']
        if churn:
            lines += [f'int batch_{cycle} = 0;', f'while (batch_{cycle} < {churn}) {{']
        for i in range(inserts):
            u = (i * 37) % n
            v = (u + degree + 31 + (i // n) + cycle * 11) % n
            lines.append(f'G.addEdges({u}->{v});')
            if churn:
                lines.append(f'G.removeEdges({u}->{v});')
        if churn:
            lines += [f'  batch_{cycle} = batch_{cycle} + 1;', '}']
        lines += [f'// Phase {cycle}: scan the updated graph.'] + final
    lines += ['print checksum;', 'print numEdges(G);']
    content = '\n'.join(lines) + '\n'
    if not p.exists() or p.read_text() != content:
        p.write_text(content)
    # Independently calculate the exact checksum for every scan and update.
    pairs = {tuple(sorted((u, (u+d)%n))) for u in range(n) for d in range(1,degree+1)}
    def value():
        if shape in ('bfs','dfs','mixed'): return 0
        return 2*len(pairs) if shape == 'count' else sum(u%7+v%7 for u,v in pairs)
    checksum = 0
    for cycle in range(cycles):
        checksum += scans * value()
        for i in range(inserts):
            u = (i*37)%n
            v = (u+degree+31+(i//n)+cycle*11)%n
            if not churn: pairs.add(tuple(sorted((u,v))))
        checksum += post_scans * value()
    return name, p, f'{checksum}\n{len(pairs)}\n'

def compile_case(out, work, objs, name, src, mode, compiler_settings=None):
    dest = out / 'binaries' / name
    dest.mkdir(parents=True, exist_ok=True)
    exe = dest / mode
    ir = dest / (mode + '.ll')
    env = environment()
    if compiler_settings:
        env.update(compiler_settings)
    if mode != 'AUTO':
        env['AUTOTUNER_FORCE_LAYOUT'] = mode
    if not exe.exists() or exe.stat().st_mtime < src.stat().st_mtime:
        cp = run([ROOT/'GraphProgram', src, f'--emit-ir-to={ir}'], work, env)
        (dest/(mode+'.compile.log')).write_text(cp.stdout+cp.stderr)
        shutil.copy2(work/'program.o', dest/(mode+'.program.o'))
        run(['g++', '-O3', '-fopenmp', '-pthread', '-no-pie', work/'program.o',
             *objs, '-ldl', '-lnlopt', '-o', exe], work)
    text = ir.read_text()
    calls = []
    for line in text.splitlines():
        if 'call ' in line and '@autograph_ensure_layout(' in line:
            m = re.search(r'i32(?: noundef)? (\d+)\)', line)
            if m:
                calls.append(NAMES[int(m[1])])
    if mode != 'AUTO' and any(x != mode for x in calls):
        raise RuntimeError(f'{name}/{mode} is not fixed: {calls}')
    if mode != 'AUTO' and mode != 'CSR' and not calls:
        raise RuntimeError(f'{name}/{mode}: force was not installed')
    return exe, calls

def time_once(exe, expected, timeout):
    start = time.perf_counter_ns()
    p = run([exe], exe.parents[2]/'codes', environment(), timeout)
    elapsed = (time.perf_counter_ns()-start)/1e6
    if p.stdout != expected:
        raise RuntimeError(f'Incorrect output {exe}: expected {expected!r}, got {p.stdout!r}')
    return elapsed, p

def stage(out, name, executables):
    # Benchmark on WSL's local filesystem to avoid Windows mount metadata costs.
    # Loading and all conversions remain inside the measured subprocess.
    local = Path('/tmp/sgpl_layout_switch_20261015') / name
    (local/'codes').mkdir(parents=True, exist_ok=True)
    (local/'graphs').mkdir(exist_ok=True)
    bins = local/'binaries'/name
    bins.mkdir(parents=True, exist_ok=True)
    for graph in (out/'graphs').iterdir():
        shutil.copy2(graph, local/'graphs'/graph.name)
    staged={}
    for mode, exe in executables.items():
        target=bins/mode
        shutil.copy2(exe,target)
        staged[mode]=target
    return staged

def audit(out, config, work, objs, name, expected, schedules):
    wrapper=work/'audit.o'
    run(['gcc','-O2','-iquote',ROOT,'-c',ROOT/'tools/layout_switch_audit.c','-o',wrapper],work)
    mask=(1<<64)-1
    def mix(x):
        x=(x^(x>>30))*0xbf58476d1ce4e5b9 & mask
        x=(x^(x>>27))*0x94d049bb133111eb & mask
        return x^(x>>31)
    n,d=config['n'],config['degree']
    pairs={tuple(sorted((u,(u+delta)%n))) for u in range(n) for delta in range(1,d+1)}
    def signature():
        return (2*len(pairs),sum(mix((u<<32)|v)+mix((v<<32)|u) for u,v in pairs)&mask)
    reference=[]
    for cycle in range(config['cycles']):
        reference += [signature()]*config['scans']
        for i in range(config['inserts']):
            u=(i*37)%n
            v=(u+d+31+(i//n)+cycle*11)%n
            if not config['churn']: pairs.add(tuple(sorted((u,v))))
        reference += [signature()]*(config['scans'] if config.get('tail_scans') is None else config['tail_scans'])
    traces={}
    for mode in MODES:
        dest=out/'binaries'/name
        exe=dest/(mode+'.audit')
        wraps=['autograph_ensure_layout','graph_add_edge','graph_remove_edge',
               'autograph_neighbor_iter_init','autograph_neighbor_iter_next','bfs_runtime_src']
        run(['g++','-fopenmp','-pthread','-no-pie',dest/(mode+'.program.o'),
             wrapper,*objs,*[f'-Wl,--wrap={symbol}' for symbol in wraps],'-ldl','-lnlopt','-o',exe],work)
        env=environment()
        if mode!='AUTO': env['BENCH_FIXED_LAYOUT']=str(MODES.index(mode)-1)
        cp=run([exe],out/'codes',env,600)
        (dest/(mode+'.audit.log')).write_text(cp.stdout+cp.stderr)
        if cp.stdout!=expected: raise RuntimeError(f'{name}/{mode} audit output mismatch')
        observations=re.findall(r'AUDIT bfs layout=(\w+) visited=(\d+) arcs=(\d+) digest=(\d+)',cp.stderr)
        actual=[(int(row[2]),int(row[3])) for row in observations]
        if actual!=reference: raise RuntimeError(f'{name}/{mode} traversal digest mismatch')
        changes=re.findall(r'AUDIT layout (\w+) -> (\w+)',cp.stderr)
        if mode=='AUTO' and not changes: raise RuntimeError('AUTO did not change layout at runtime')
        mutations=re.search(r'AUDIT mutations adds=(\d+) removes=(\d+)',cp.stderr)
        adds=config['inserts']*config['cycles']*max(1,config['churn'])
        removes=adds if config['churn'] else 0
        if not mutations or tuple(map(int,mutations.groups()))!=(adds,removes):
            raise RuntimeError(f'{name}/{mode} mutation count mismatch')
        traces[mode]=dict(transitions=changes,query_layouts=[r[0] for r in observations],
                          queries=len(actual),adds=adds,removes=removes,correct=True,
                          mutation_effects_checked=True)
        print(f'AUDIT {name}/{mode}: transitions={changes}, {len(actual)} correct graph scans',flush=True)
    (out/'binaries'/name/'audit.json').write_text(json.dumps(traces,indent=2)+'\n')

def benchmark(out, config, repeats, warmups, timeout, raw_name='raw_runs.csv'):
    work, objs = build(out)
    name, src, expected = source(out, **config)
    executables, schedules = {}, {}
    for mode in MODES:
        executables[mode], schedules[mode] = compile_case(out, work, objs, name, src, mode)
        print(f'{name} {mode} schedule={schedules[mode]}', flush=True)
    if not any(x != 'CSR' for x in schedules['AUTO']):
        print('SKIP: no AutoTuner layout change', flush=True)
        return
    executables = stage(out,name,executables)
    rng = random.Random(20261015)
    samples = {m: [] for m in MODES}
    raw = []
    for iteration in range(warmups+repeats):
        order = MODES.copy()
        rng.shuffle(order)
        for mode in order:
            elapsed, cp = time_once(executables[mode], expected, timeout)
            if iteration >= warmups:
                samples[mode].append(elapsed)
                raw.append(dict(case=name, variant=mode, run=iteration-warmups,
                                total_runtime_ms=elapsed, stdout_sha256=hashlib.sha256(cp.stdout.encode()).hexdigest()))
            (out/'binaries'/name/(mode+'.run.log')).write_text(cp.stdout+cp.stderr)
        print(f'{name} round {iteration+1}/{warmups+repeats}: ' +
              ' '.join(f'{m}={statistics.median(v):.3f}ms' for m,v in samples.items() if v), flush=True)
    result = dict(case=name, config=config, source=str(src), expected_stdout=expected,
                  schedules=schedules, samples_ms=samples,
                  medians_ms={m:statistics.median(samples[m]) for m in MODES})
    result['speedup_vs_best_fixed'] = min(result['medians_ms'][m] for m in MODES[1:])/result['medians_ms']['AUTO']
    (out/'binaries'/name/'result.json').write_text(json.dumps(result,indent=2)+'\n')
    # CSV here is measurement interchange; final report is authored/checked separately.
    path=out/raw_name
    with path.open('a',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(raw[0]))
        if f.tell()==0: writer.writeheader()
        writer.writerows(raw)
    print('RESULT '+json.dumps(result['medians_ms'])+f" best-fixed speedup={result['speedup_vs_best_fixed']:.3f}x",flush=True)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--out',default='bench_results/layout_switch_20261015')
    ap.add_argument('--n',type=int,default=4000)
    ap.add_argument('--degree',type=int,default=8)
    ap.add_argument('--inserts',type=int,default=500)
    ap.add_argument('--scans',type=int,default=2)
    ap.add_argument('--cycles',type=int,default=1)
    ap.add_argument('--churn',type=int,default=0)
    ap.add_argument('--tail-scans',type=int)
    ap.add_argument('--shape',choices=['count','weighted','bfs','dfs','mixed'],default='bfs')
    ap.add_argument('--repeats',type=int,default=3)
    ap.add_argument('--warmups',type=int,default=1)
    ap.add_argument('--timeout',type=int,default=180)
    ap.add_argument('--audit-only',action='store_true')
    ap.add_argument('--query-loops',action='store_true')
    a=ap.parse_args()
    out=(ROOT/a.out).resolve()
    out.mkdir(parents=True,exist_ok=True)
    config=dict(n=a.n,degree=a.degree,inserts=a.inserts,scans=a.scans,cycles=a.cycles,shape=a.shape,churn=a.churn,tail_scans=a.tail_scans,query_loops=a.query_loops)
    if a.audit_only:
        work,objs=build(out)
        name,src,expected=source(out,**config)
        schedules={m:compile_case(out,work,objs,name,src,m)[1] for m in MODES}
        audit(out,config,work,objs,name,expected,schedules)
    else:
        benchmark(out,config,a.repeats,a.warmups,a.timeout)

if __name__=='__main__': main()
