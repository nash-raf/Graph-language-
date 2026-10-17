#!/usr/bin/env python3
"""Fresh-process CPU benchmark: BFS, SSSP, CC, PageRank; P1, Graptor, GraphIt, Ligra.

Default: 100 smallest graphs, 32 workers, one warm-up then five measured processes per case.
Compilation and independent output validation are outside the measured interval.
Run from the prepared lab bundle: python3 -u run_benchmarks.py --resume
"""
import argparse, ast, concurrent.futures, csv, fcntl, hashlib, json, os, re
import resource, shutil, signal, statistics, subprocess, sys, time, uuid, threading
from contextlib import contextmanager
from pathlib import Path
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import connected_components, dijkstra

ALGS = ['bfs', 'sssp', 'cc', 'pagerank']
FRAMEWORKS = ['p1', 'graptor', 'graphit', 'ligra']
OBJECTS = ['runtime.o', 'parallel_runtime.o', 'gpu_runtime.o', 'autotuner_runtime.o',
           'graph_mutation_runtime.o', 'roaring_bitmap.o', 'graph_loader_runtime.o',
           'graph_runtime.o', 'semiring_runtime.o', 'monotonic_timer.o']
FIELDS = ['algorithm','framework','graph','vertices','arcs','source','workers','weight_provenance',
          'p1_directed','status','validation_passed','samples','median_seconds','mean_seconds',
          'stddev_seconds','min_seconds','max_seconds','q1_seconds','q3_seconds','max_rss_kib','error']
ACTIVE = {}; ACTIVE_LOCK = threading.RLock(); STOP = threading.Event()

def terminate(p, unit):
    try:
        subprocess.run(['systemctl','--user','kill','--kill-who=all','--signal=KILL',unit],
                       stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15)
    except subprocess.TimeoutExpired: pass
    finally:
        try: os.killpg(p.pid,signal.SIGKILL)
        except ProcessLookupError: pass

def interrupt(signum, frame):
    STOP.set()
    with ACTIVE_LOCK: active=list(ACTIVE.values())
    for p,unit in active: terminate(p,unit)
    raise KeyboardInterrupt

def sha(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1024*1024), b''): h.update(block)
    return h.hexdigest()

def atomic_json(path, obj):
    path = Path(path); temp = path.with_suffix(path.suffix+'.tmp')
    with open(temp, 'w') as f:
        json.dump(obj, f, indent=2, allow_nan=False); f.write('\n'); f.flush(); os.fsync(f.fileno())
    os.replace(temp, path)

def append(path, obj):
    with open(path, 'a') as f:
        f.write(json.dumps(obj, allow_nan=False)+'\n'); f.flush(); os.fsync(f.fileno())

def journal(path):
    """Recover a last append interrupted by power loss; reject corruption earlier in the file."""
    if not path.exists(): return []
    data = path.read_bytes(); lines = data.splitlines(keepends=True); records = []; offset = 0
    for i, line in enumerate(lines):
        try: record = json.loads(line)
        except (ValueError, UnicodeError):
            if i != len(lines)-1: raise RuntimeError('Corrupt journal: '+str(path))
            with open(path, 'r+b') as f: f.truncate(offset)
            print('Recovered incomplete journal append', path, flush=True); break
        records.append(record); offset += len(line)
    # A valid JSON object without its final newline must be separated from the next append.
    if records and path.stat().st_size and not path.read_bytes().endswith(b'\n'):
        with open(path, 'ab') as f: f.write(b'\n')
    return records

def clean_env(framework, threads):
    env = {k:v for k,v in os.environ.items() if not k.startswith(
        ('OMP_', 'GOMP_', 'KMP_', 'SGPL_', 'BENCH_', 'GRAPH_', 'FORCE_'))}
    env['OMP_NUM_THREADS'] = str(threads)
    if framework == 'p1':
        env.update(OMP_WAIT_POLICY='PASSIVE', GOMP_SPINCOUNT='0', SGPL_ROARING_THREADS='4',
                   SGPL_NUM_THREADS=str(threads), SGPL_ALLOW_HT='1', SGPL_GPU_BACKEND='0')
    else:
        env.update(OMP_DYNAMIC='FALSE', OMP_PROC_BIND='TRUE', OMP_PLACES='threads')
    return env

def unlimited_stack():
    resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))

def idle_before_batch(allow_busy):
    """Sample current CPU use; load averages retain our own previous compiler activity."""
    def ticks():
        x=list(map(int,Path('/proc/stat').read_text().splitlines()[0].split()[1:9]))
        return sum(x),x[3]+x[4]
    started=time.monotonic()
    while True:
        total,idle=ticks();time.sleep(.5);new_total,new_idle=ticks()
        busy=1-(new_idle-idle)/max(1,new_total-total)
        if allow_busy or busy<.125: return busy
        if time.monotonic()-started>180:
            raise RuntimeError('CPU remains busy; stop other compute jobs or explicitly use --allow-busy')
        print(f'WAIT current CPU use {busy:.0%}; timing has not started.',flush=True)
        time.sleep(5)

class Launcher:
    def __init__(self, root): self.root = root; self.serial = 0

    def run(self, command, cwd, env, log, timeout, stamp=None, memory='54G'):
        """The systemd scope starts outside walltime2; walltime2 times fork through wait4."""
        if STOP.is_set(): raise InterruptedError('Runner interrupted')
        self.serial += 1
        unit = f'sgpl-e2e150-{os.getpid()}-{uuid.uuid4().hex[:12]}.scope'
        scope = ['systemd-run','--user','--scope','--quiet','--collect','--unit='+unit,
                 '-p','MemoryMax='+memory,'-p','MemorySwapMax=0']
        cmd = scope + ([str(self.root/'bin/walltime2'), str(stamp)] if stamp else []) + command
        log.parent.mkdir(parents=True, exist_ok=True)
        if stamp and stamp.exists(): stamp.unlink()
        started = time.monotonic(); timed_out = False
        # Keep diagnostics separate: buffered stdout value dumps can otherwise
        # be split mid-number by an unbuffered runtime message on stderr.
        stderr_log=log.with_suffix('.stderr.log')
        with open(log, 'wb') as output, open(stderr_log,'wb') as errors:
            p = subprocess.Popen(cmd, cwd=cwd, env=env, stdout=output, stderr=errors,
                                 start_new_session=True)
            with ACTIVE_LOCK: ACTIVE[p.pid]=(p,unit)
            try:
                if STOP.is_set(): terminate(p,unit)
                code = p.wait(timeout=timeout)
            except (subprocess.TimeoutExpired, KeyboardInterrupt) as exc:
                # Exact scope and exact session only: never match unrelated jobs by command text.
                terminate(p,unit)
                p.wait(timeout=15)
                if isinstance(exc, KeyboardInterrupt): raise
                code = 124; timed_out = True
            finally:
                with ACTIVE_LOCK: ACTIVE.pop(p.pid,None)
        result = {'command':command,'exit_code':code,'timeout':timed_out,'log':str(log),'stderr_log':str(stderr_log),
                  'driver_elapsed_seconds':time.monotonic()-started,'unit':unit}
        if stamp and stamp.exists():
            fields = stamp.read_text().split()
            if len(fields) != 9: raise RuntimeError('Invalid walltime2 output: '+str(stamp))
            keys = ['seconds','rss_kib','child_exit','signal','nvcsw','nivcsw','minflt','majflt','start_unix']
            result.update(dict(zip(keys, [float(fields[0])]+[int(x) for x in fields[1:8]]+[float(fields[8])])) )
        if stamp and not timed_out and code == 0 and ('seconds' not in result or result['child_exit'] or result['signal']):
            raise RuntimeError('Missing or failed child timing: '+str(stamp))
        return result

def helper(root):
    # Load only the source-generation functions: old bench_views must never choose our inputs.
    source = ast.parse((root/'lib/p1_bench_snapshot.py').read_text())
    names = {'SOURCES','RESULT','CUT','DECL','ARRAY'}
    nodes = [n for n in source.body if isinstance(n, ast.FunctionDef) and n.name in
             {'replace_once','depths','make_source'} or isinstance(n, ast.Assign) and
             any(isinstance(t, ast.Name) and t.id in names for t in n.targets)]
    ns = {'re':re,'ALGO':root/'algorithms'}
    exec(compile(ast.Module(body=nodes, type_ignores=[]), '<frozen-p1-source-generator>', 'exec'), ns)
    return ns['make_source']

def case_dir(out, alg, row): return out/'p1'/alg/row['graph'].removesuffix('.edges')

def build_p1(root, out, alg, row, args, source_fn, fingerprint):
    folder = case_dir(out, alg, row); folder.mkdir(parents=True, exist_ok=True)
    marker = folder/'build.json'
    if marker.exists():
        old = json.loads(marker.read_text())
        if old['fingerprint'] == fingerprint and all((folder/k).exists() and sha(folder/k)==v for k,v in old['binaries'].items()):
            return {'graph':row['graph'],'ok':True,'cached':True}
    inp = folder/('input.wel' if alg=='sssp' else 'input.el')
    origin = Path(row['p1_weighted_file'] if alg=='sssp' else row['original_file'])
    if not inp.exists():
        try: os.link(origin, inp)
        except OSError: shutil.copy2(origin, inp)
    assert sha(inp)==sha(origin), 'Private P1 input differs'
    for file in (root/'p1').glob('*.ll'):
        link = folder/file.name
        if not link.exists(): link.symlink_to(file)
    launcher = Launcher(root); env = clean_env('p1', args.threads); env['FORCE_CPU']='1'
    start = time.monotonic(); records = []; binaries = {}
    for export, name in [(False, 'timed'), (True, 'validate')]:
        text = source_fn(alg, inp, int(row['source']), 0, int(row['vertices']), int(row['arcs_both_directions']), export)
        assert text.count('directed: true;')==1
        text = text.replace('directed: true;', 'directed: '+row['p1_directed'].lower()+';')
        program = folder/(name+'.graph'); program.write_text(text)
        obj = folder/'program.o'
        if obj.exists(): obj.unlink()
        rec = launcher.run([str(root/'p1/GraphProgram'),'--ir-backend=cpu',str(program)], folder, env,
                           folder/(name+'.compile.log'), args.compile_timeout, memory=args.compile_memory)
        records.append(rec)
        if rec['exit_code'] or not obj.exists():
            return {'graph':row['graph'],'ok':False,'error':'P1 '+name+' compile failed; '+rec['log']}
        obj.rename(folder/(name+'.o'))
        cmd = ['g++','-O3','-fopenmp','-no-pie',str(folder/(name+'.o'))]+[str(root/'p1'/o) for o in OBJECTS]
        cmd += ['-Wl,--wrap=timer_runtime','-ldl','-lnlopt','-o',str(folder/name)]
        rec = launcher.run(cmd, folder, env, folder/(name+'.link.log'), 120, memory=args.compile_memory); records.append(rec)
        if rec['exit_code']: return {'graph':row['graph'],'ok':False,'error':'P1 link failed; '+rec['log']}
        binaries[name] = sha(folder/name)
    atomic_json(marker, {'fingerprint':fingerprint,'binaries':binaries,'commands':records,
                         'compile_seconds_excluded':time.monotonic()-start,'copies_per_process':1,
                         'p1_directed':row['p1_directed'],'input':str(origin)})
    return {'graph':row['graph'],'ok':True,'cached':False}

def command(root, out, alg, fw, row, threads, validate=False):
    src = str(row['source'] if alg in ['bfs','sssp'] else 0)
    env = clean_env(fw, threads); binary = str(root/'bin'/(fw+'_'+alg))
    if fw=='p1':
        folder = case_dir(out, alg, row)
        return [str(folder/('validate' if validate else 'timed'))], env, folder
    if fw=='graptor':
        cmd = [binary,'-rounds','1','-b','-c',str(threads),'-l','8','-start',src]
        if alg=='sssp': cmd += ['-weights',row['weights']]
        if alg=='pagerank': cmd += ['-pr:maxiter','20']
        cmd += ['-s',row['bin']]
    elif fw=='graphit': cmd = [binary,row['wel'] if alg=='sssp' else row['el'],src,'1','1']
    else:
        cmd = [binary,'-rounds','0','-r',src]
        if alg=='pagerank': cmd += ['-maxiters','20']
        cmd += ['-s',row['wadj'] if alg=='sssp' else row['adj']]
    return cmd, env, root

@contextmanager
def reference_arrays(row):
    if row.get('reference_format')=='graptor_csr':
        n,m=int(row['vertices']),int(row['arcs_both_directions'])
        stored=np.memmap(row['bin'],mode='r',dtype='<u8',offset=64,shape=(n,))
        ip=np.empty(n+1,dtype=np.int64);ip[:n]=stored;ip[n]=m
        dst=np.memmap(row['bin'],mode='r',dtype='<u4',offset=64+8*n,shape=(m,))
        arrays={'indptr':ip,'dst':dst}
        if row.get('sssp_reference_path'):arrays['distances']=np.load(row['sssp_reference_path'],mmap_mode='r')
        yield arrays
    else:
        with np.load(row['reference']) as arrays:yield arrays

def reference(alg, row, cache):
    path = cache/(alg+'_'+row['graph'].replace('/','_')+'.npy')
    if path.exists(): return np.load(path)
    with reference_arrays(row) as z:
        n = int(row['vertices']); ip = z['indptr']; dst = z['dst']
        if alg=='sssp': expected = z['distances']
        elif alg=='bfs':
            expected = dijkstra(csr_matrix((np.ones(len(dst)),dst,ip),shape=(n,n)),
                                directed=True,indices=int(row['source']),unweighted=True)
        elif alg=='cc':
            _, labels = connected_components(csr_matrix((np.ones(len(dst),dtype=np.int8),dst,ip),shape=(n,n)),directed=False)
            first = np.full(labels.max()+1,n,dtype=np.int64); np.minimum.at(first, labels, np.arange(n))
            expected = first[labels]
        else:
            deg = np.diff(ip); src = np.repeat(np.arange(n),deg); expected = np.full(n,1.0/n)
            for _ in range(20):
                share = .85*expected/np.maximum(deg,1)
                expected = .15/n + np.bincount(dst,weights=share[src],minlength=n)
    cache.mkdir(exist_ok=True); np.save(path,expected); return expected

def validate_values(alg, fw, row, values, ref):
    n = int(row['vertices']); got = np.asarray(values,dtype=np.float64)
    assert len(got)==n, f'Expected {n} values; got {len(got)}'
    if alg=='sssp':
        reached = np.isfinite(ref)
        assert np.all(np.isfinite(got[reached])), 'Nonfinite reachable distance'
        assert np.array_equal(got[reached],ref[reached]), 'Shortest-path distances differ'
        u = got[~reached]
        assert np.all(np.isinf(u) | (u<0) | (u>=2**30-1)), 'Unreachable distances differ'
        return {'reachable_vertices':int(reached.sum()),'reference':'independent SciPy Dijkstra; shared synthetic weights'}
    assert np.all(np.isfinite(got)), 'Nonfinite result'
    if alg=='pagerank':
        error = float(np.max(np.abs(got-ref))); tolerance = 1e-10 if fw=='p1' else 2e-6
        assert error<tolerance, f'PageRank absolute error {error}'
        assert np.allclose(got,ref,rtol=2e-4,atol=max(1e-12,1e-6/n)), 'PageRank relative error'
        return {'max_absolute_error':error,'iterations':20,'damping':.85,'dangling_mass':'not redistributed'}
    assert np.all(got==np.floor(got)), 'Fractional integer result'
    ints = got.astype(np.int64)
    if alg=='cc':
        _, inv = np.unique(ints,return_inverse=True); first = np.full(inv.max()+1,n,dtype=np.int64)
        np.minimum.at(first,inv,np.arange(n)); normalized = first[inv]
        assert np.array_equal(normalized,ref), 'Connected-component partition differs'
        return {'components':int(np.unique(ref).size)}
    reached = np.isfinite(ref); actual = (ints>=0)&(ints<n)
    assert np.array_equal(actual,reached), 'BFS reachability differs'
    src = int(row['source']); assert ints[src]==src, 'BFS source parent'
    vs = np.flatnonzero(reached); vs = vs[vs!=src]; parents = ints[vs]
    assert np.all(ref[parents]+1==ref[vs]), 'BFS parent depth differs'
    with reference_arrays(row) as z:
        ip=z['indptr']; dst=z['dst']; owners=np.repeat(np.arange(n,dtype=np.int64),np.diff(ip))
        keys=np.sort(owners*(n+1)+dst); want=parents*(n+1)+vs; pos=np.searchsorted(keys,want)
        assert np.all(pos<len(keys)) and np.array_equal(keys[pos],want), 'Missing BFS parent edge'
    return {'reachable_vertices':int(reached.sum()),'levels':int(np.max(ref[reached]))}

def check_case(root, out, alg, fw, row, args, launcher, folder):
    cmd, env, cwd = command(root,out,alg,fw,row,args.threads,True)
    dest = folder/'validation.values'
    if dest.exists(): dest.unlink()
    if fw!='p1': env['BENCH_OUTPUT']=str(dest)
    rec = launcher.run(cmd,cwd,env,folder/'validation.log',args.timeout)
    assert not rec['exit_code'], 'Validation process failed: '+str(rec)
    if fw=='p1':
        text=(folder/'validation.log').read_text()
        assert 'VALUES_BEGIN\n' in text and 'VALUES_END' in text, 'Missing P1 output markers'
        values=np.asarray(text.split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0].split(),dtype=np.float64)
    else:
        assert dest.exists(), 'Missing baseline BENCH_OUTPUT'
        values=np.loadtxt(dest,ndmin=1)
    ref=reference(alg,row,out/'references')
    detail=validate_values(alg,fw,row,values,ref)
    rec.update(passed=True,detail=detail)
    atomic_json(folder/'validation.json',rec)
    # Keep exact validation values for audit; never produce them in timed launches.
    return rec

def make_summary(alg, fw, row, args, records, valid, error):
    good=[r for r in records if r['phase']=='measure' and r['exit_code']==0 and not r['timeout'] and 'seconds' in r]
    times=[r['seconds'] for r in good]; complete=len(times)==args.repeats and valid
    result={'algorithm':alg,'framework':fw,'graph':row['graph'],'vertices':int(row['vertices']),
            'arcs':int(row['arcs_both_directions']),'source':int(row['source']) if alg in ['bfs','sssp'] else 0,
            'workers':args.threads,'weight_provenance':row['weight_provenance'] if alg=='sssp' else 'unweighted',
            'p1_directed':row['p1_directed'] if fw=='p1' else 'symmetric prepared input',
            'status':'ok' if complete else 'failed','validation_passed':valid,'samples':len(times),'error':error}
    if complete:
        result.update(median_seconds=statistics.median(times),mean_seconds=statistics.mean(times),
            stddev_seconds=statistics.stdev(times) if len(times)>1 else 0,min_seconds=min(times),max_seconds=max(times),
            q1_seconds=float(np.percentile(times,25)),q3_seconds=float(np.percentile(times,75)),max_rss_kib=max(r['rss_kib'] for r in good))
    return result

def write_summary(out, results):
    temp=out/'summary.csv.tmp'
    with open(temp,'w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=FIELDS);w.writeheader();w.writerows(results.values())
    os.replace(temp,out/'summary.csv')
    atomic_json(out/'status.json',{'completed_cases':len(results),'passed_cases':sum(r['status']=='ok' for r in results.values()),
                                 'failed_cases':sum(r['status']!='ok' for r in results.values()),'updated_unix':time.time()})

def main():
    p=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parent)
    p.add_argument('--manifest',type=Path);p.add_argument('--output',type=Path,default=Path('results/run100_r5'))
    p.add_argument('--resume',action='store_true',help='Continue an existing run; also works for a new output folder')
    p.add_argument('--threads',type=int,default=32);p.add_argument('--repeats',type=int,default=5)
    p.add_argument('--timeout',type=float,default=300);p.add_argument('--compile-timeout',type=float,default=600)
    p.add_argument('--compile-jobs',type=int,default=2)
    p.add_argument('--compile-memory',default='20G',help='Per-compiler systemd memory cap; compilation is excluded from timing')
    p.add_argument('--graph',action='append',default=[]);p.add_argument('--limit',type=int,default=100)
    p.add_argument('--algorithms',nargs='+',choices=ALGS,default=ALGS)
    p.add_argument('--frameworks',nargs='+',choices=FRAMEWORKS,default=FRAMEWORKS)
    p.add_argument('--plan',action='store_true',help='Check inputs and print plan without compiling or launching workloads')
    p.add_argument('--allow-busy',action='store_true',help='Allow starting a batch with current CPU use above 12.5 percent')
    a=p.parse_args();root=a.root.resolve();out=(a.output if a.output.is_absolute() else root/a.output).resolve()
    unlimited_stack()
    signal.signal(signal.SIGINT,interrupt);signal.signal(signal.SIGTERM,interrupt)
    assert 1<=a.threads<=len(os.sched_getaffinity(0)), 'Worker count exceeds available logical CPUs'
    assert a.repeats>=1 and 1<=a.compile_jobs<=2 and a.timeout>0 and a.compile_timeout>0
    assert a.limit is None or a.limit>0, '--limit must be positive'
    bundle_lock=open(root/'.bundle.lock','a');fcntl.flock(bundle_lock,fcntl.LOCK_SH|fcntl.LOCK_NB)
    bundle=json.loads((root/'bundle.json').read_text());manifest=(a.manifest or Path(bundle['input_manifest'])).resolve()
    rows=list(csv.DictReader(open(manifest)))
    assert rows and len({r['graph'] for r in rows})==len(rows), 'Empty manifest or duplicate graphs'
    rows.sort(key=lambda r:(int(r['arcs_both_directions']),r['graph']))
    if a.graph:
        assert set(a.graph)<=set(r['graph'] for r in rows), 'Unknown --graph'
        rows=[r for r in rows if r['graph'] in a.graph]
    if a.limit: rows=rows[:a.limit]
    assert rows, 'No selected graphs'
    algs=[x for x in ALGS if x in a.algorithms];fws=[x for x in FRAMEWORKS if x in a.frameworks]
    cases=[(alg,fw,r) for alg in algs for r in rows for fw in fws]
    total=len(cases)*(a.repeats+1); measured_total=len(cases)*a.repeats
    print(f'PLAN: {len(rows)} graphs x {len(algs)} algorithms x {len(fws)} frameworks = {len(cases)} cases; '
          f'{total} launches ({measured_total} timed + {len(cases)} warm-ups), plus {len(cases)} untimed validations.',flush=True)
    print('ORDER: algorithms '+', '.join(algs)+'; within each, smallest to largest graph; on each graph '+', '.join(fws)+'.',flush=True)
    print('SSSP: shared deterministic positive integer weights 1..255, seed 2358, on every graph.',flush=True)
    print('Verifying frozen binaries, runtime sources, and every selected input checksum...',flush=True)
    for name,digest in bundle['files'].items(): assert sha(root/name)==digest, 'Bundle changed: '+name
    inputs={}
    for r in rows: inputs.update(json.loads(r['format_sha256']))
    for i,(name,digest) in enumerate(inputs.items(),1):
        assert sha(name)==digest, 'Input changed: '+name
        if i%300==0: print(f'Input checksums {i}/{len(inputs)}',flush=True)
    # Detect accidental caller environment differences without storing secrets.
    protocol={'manifest':str(manifest),'manifest_sha256':sha(manifest),'bundle_sha256':sha(root/'bundle.json'),
              'runner_sha256':sha(__file__),'graphs':[r['graph'] for r in rows],'algorithms':algs,'frameworks':fws,
              'threads':a.threads,'repeats':a.repeats,'warmups_per_case':1,'timeout':a.timeout,
              'timing':'CLOCK_MONOTONIC immediately before fork through wait4 child reaping; compilation, scope setup and validation excluded',
              'order':'algorithm -> ascending arcs -> framework -> warmup -> measured launches -> separate validation',
              'compilation':'P1 per-graph specialization and persistent input cache preparation are excluded; all build jobs finish before algorithm timing',
              'validation':'Independent BFS depths and parent edges, Dijkstra distances, CC partitions, and 20-step PageRank; validation output excluded from timed runs',
              'warm_cache':True,'cpu_affinity':sorted(os.sched_getaffinity(0)),
              'allow_busy':a.allow_busy,
              'thread_settings':{fw:{k:v for k,v in clean_env(fw,a.threads).items() if k.startswith(('OMP_','SGPL_','GOMP_'))} for fw in fws},
              'pagerank':{'iterations':20,'damping':.85,'redistribute_dangling':False},
              'sssp':'synthetic positive 1..255 canonical undirected-edge weights, seed 2358',
              'planned_launches':total,'timed_launches':measured_total,'validation_launches':len(cases),
              'machine':subprocess.check_output(['uname','-a'],text=True).strip(),
              'cpu_model':next((x.split(':',1)[1].strip() for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),'unknown')}
    if a.plan: print('PLAN_CHECK_PASSED',flush=True);return 0
    benchmark_lock=open(root/'.benchmark.lock','a')
    try:fcntl.flock(benchmark_lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
    except BlockingIOError:raise RuntimeError('Another benchmark is running from this bundle; wait for it to finish')
    out.mkdir(parents=True,exist_ok=True);lock=open(out/'.run.lock','a')
    try: fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
    except BlockingIOError: raise RuntimeError('Another runner holds '+str(out))
    old_path=out/'protocol.json'
    if old_path.exists():
        assert a.resume, 'Output exists; use --resume or a different --output'
        assert json.loads(old_path.read_text())==protocol, 'Resume protocol/binaries/data differ; use a new --output'
    else: atomic_json(old_path,protocol)
    fingerprint=hashlib.sha256(json.dumps(protocol,sort_keys=True).encode()).hexdigest()
    events=journal(out/'launches.jsonl');finished=journal(out/'cases.jsonl')
    results={r['algorithm']+'|'+r['framework']+'|'+r['graph']:r for r in finished}
    source_fn=helper(root);launcher=Launcher(root)
    resolved=sum((a.repeats+1) for _ in results)
    resolved+=sum(1 for e in events if e['case'] not in results and e['phase'] in ['warmup','measure'] and e['exit_code']==0)
    measured=sum(1 for e in events if e['phase']=='measure' and e['exit_code']==0)
    failures=sum(r['status']!='ok' for r in results.values())
    def progress(label):
        print(f'PROGRESS {resolved}/{total} planned launch slots resolved; {total-resolved} remaining; '
              f'{measured}/{measured_total} successful timed launches; cases {len(results)}/{len(cases)}; failures {failures} | {label}',flush=True)
    write_summary(out,results);progress('resuming' if events else 'starting')
    for alg in algs:
        builds={}
        if 'p1' in fws:
            needed=[r for r in rows if alg+'|p1|'+r['graph'] not in results]
            if needed: print(f'PREPARE {alg}: compiling {len(needed)} P1 cases before any {alg} timing; {a.compile_jobs} build jobs.',flush=True)
            # Threads coordinate separate compiler processes; no preexec_fn from these threads.
            # Each compilation has a private directory. Build jobs finish before any measurements.
            with concurrent.futures.ThreadPoolExecutor(max_workers=a.compile_jobs) as pool:
                future={pool.submit(build_p1,root,out,alg,r,a,source_fn,fingerprint):r for r in needed}
                for i,f in enumerate(concurrent.futures.as_completed(future),1):
                    r=future[f]
                    try: builds[r['graph']]=f.result()
                    except Exception as exc: builds[r['graph']]={'ok':False,'error':str(exc)}
                    print(f'COMPILE {alg} {i}/{len(needed)} {r["graph"]}: '+('ready' if builds[r['graph']]['ok'] else builds[r['graph']]['error']),flush=True)
        if any(alg+'|'+fw+'|'+r['graph'] not in results for r in rows for fw in fws):
            busy=idle_before_batch(a.allow_busy)
            append(out/'batches.jsonl',{'algorithm':alg,'frameworks':fws,'cpu_busy_fraction_before_batch':busy,
                                      'start_unix':time.time()})
        for idx,row in enumerate(rows,1):
            for fw in fws:
                key=alg+'|'+fw+'|'+row['graph']
                if key in results: continue
                folder=out/'logs'/alg/fw/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
                existing=[e for e in events if e['case']==key and e['phase'] in ['warmup','measure'] and e['exit_code']==0]
                error='';valid=False
                try:
                    if fw=='p1' and not builds[row['graph']]['ok']: raise RuntimeError(builds[row['graph']]['error'])
                    cmd,env,cwd=command(root,out,alg,fw,row,a.threads)
                    phases=[('warmup',-1)]+[('measure',i) for i in range(a.repeats)]
                    done={(e['phase'],e['sample']) for e in existing}
                    if existing and len(done)<len(phases):
                        rec=launcher.run(cmd,cwd,env,folder/'resume_warmup.log',a.timeout,folder/'resume_warmup.stamp')
                        rec.update(case=key,phase='resume_warmup',sample=-1);append(out/'launches.jsonl',rec)
                        assert not rec['exit_code'], 'Resume warm-up failed'
                    for phase,sample in phases:
                        if (phase,sample) in done: continue
                        stem=phase+'_'+str(sample)
                        rec=launcher.run(cmd,cwd,env,folder/(stem+'.log'),a.timeout,folder/(stem+'.stamp'))
                        rec.update(case=key,phase=phase,sample=sample,loadavg=list(os.getloadavg()),environment=protocol['thread_settings'][fw])
                        append(out/'launches.jsonl',rec);events.append(rec)
                        if rec['exit_code']: raise RuntimeError(f'{phase} failed (exit {rec["exit_code"]}); see {rec["log"]}')
                        existing.append(rec);resolved+=1
                        if phase=='measure':measured+=1
                        progress(f'{alg}/{fw} graph {idx}/{len(rows)} {row["graph"]} {phase} {sample+1 if sample>=0 else 1}')
                    check_case(root,out,alg,fw,row,a,launcher,folder);valid=True
                except KeyboardInterrupt: raise
                except Exception as exc:
                    error=repr(exc); print('FAILED',key,error,flush=True)
                result=make_summary(alg,fw,row,a,existing,valid,error)
                append(out/'cases.jsonl',result);results[key]=result
                if result['status']!='ok':
                    failures+=1;skipped=a.repeats+1-len(existing);resolved+=skipped
                    result['error']+=f' ({skipped} planned launch slots failed/skipped)'
                write_summary(out,results);progress(key+' '+result['status'])
    print('FINISHED',out/'summary.csv',f'{len(results)-failures} passed, {failures} failed.',flush=True)
    return 1 if failures else 0

if __name__=='__main__':
    try: sys.exit(main())
    except KeyboardInterrupt:
        print('\nINTERRUPTED. Checkpoints saved. Run the same command with --resume.',flush=True);sys.exit(130)
