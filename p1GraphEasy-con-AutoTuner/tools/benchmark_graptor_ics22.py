#!/usr/bin/env python3
"""Correctness and paired timings of unchanged historical Graptor backends."""
from pathlib import Path
import argparse, collections, contextlib, csv, fcntl, hashlib, html, importlib.util, json, math, os, re, statistics, subprocess, sys, time
REPO=Path(__file__).resolve().parents[1]
ROOT=Path('/home/kazi/p1_graptor_ics22_20261026')
OUT=Path('/mnt/c/p1-graptor-ics22-benchmark-20261026')
HIST=Path('/home/kazi/p1_effect_fusion_local_20261022')
spec=importlib.util.spec_from_file_location('ics_build',REPO/'tools/build_graptor_ics22.py');B=importlib.util.module_from_spec(spec);spec.loader.exec_module(B)
spec=importlib.util.spec_from_file_location('frozen_common',HIST/'support/common.py');M=importlib.util.module_from_spec(spec);spec.loader.exec_module(M)
sys.path.insert(0,str(REPO/'tools/p1_campaign'))
from portable_campaign import permutation_pvalue
CPUS=[0,2,4,6,8,10]; ALGS=['cc','pagerank']; LABELS=['openmp','cilk','openmp_AA','cilk_AA']
def save(p,d):
    p=Path(p);p.parent.mkdir(parents=True,exist_ok=True);q=p.with_suffix(p.suffix+'.tmp');q.write_text(json.dumps(d,indent=2));q.replace(p)
def status(stage,**kw):
    d=dict(stage=stage,updated=time.time(),**kw);save(OUT/'status.json',d);print(json.dumps(d),flush=True)
def backend(label):return label.removesuffix('_AA')
def exe(label,alg):return ROOT/'build'/backend(label)/alg
def env(label,width):
    e=M.env('graptor',width)
    for k in list(e):
        if k.startswith(('CILK_','PARLAY_')):e.pop(k)
    e.update(CILK_NWORKERS=str(width),LD_LIBRARY_PATH=str(ROOT/'runtime')+':'+str(B.GCC/'lib64'),LD_PRELOAD=str(HIST/'gclock.so'))
    return e
def command(label,alg,row,output=None):
    cmd=[str(exe(label,alg)),'-rounds','1','-b','-c','8','-l','8','-start',str(row['source'] if alg in ['bfs','sssp'] else 0)]
    if alg=='sssp':cmd+=['-weights',row['weights']]
    if alg=='pagerank':cmd+=['-pr:maxiter','20']
    if output and alg in ['cc','pagerank']:cmd+=['-cc:outfile' if alg=='cc' else '-pr:outfile',str(output)]
    return cmd+['-s',row['bin']]
def preflight():
    build=json.loads((OUT/'build.json').read_text())
    for mode in ['cilk','openmp']:
        for alg in ALGS:assert build['results'][mode+'-'+alg]==0 and mode+':'+alg in build['binaries']
    assert B.inventory(B.SRC)==build['source_hashes']
    assert B.inventory(ROOT/'gcc-7.2.0')==json.loads((ROOT/'gcc-source-before.json').read_text()),'GCC source modified'
    runtime=ROOT/'runtime';runtime.mkdir(exist_ok=True)
    link=runtime/'libcilkrts.so.5'
    if not link.exists():link.symlink_to(B.SRC/'reproduce/lib/libcilkrts.so')
    assert link.resolve()==(B.SRC/'reproduce/lib/libcilkrts.so').resolve()
    probe=REPO/'tools/p1_campaign/ics22_worker_probe.cpp';checks=[]
    for mode in ['cilk','openmp']:
        target=ROOT/'build'/mode/'worker_probe'
        cmd=[B.GCC/'bin/g++','-O2','-std=c++17','-fcilkplus' if mode=='cilk' else '-fopenmp',probe]
        if mode=='cilk':cmd+=[B.SRC/'reproduce/lib/libcilkrts.so','-l:libnuma.so.1','-ldl','-pthread']
        subprocess.run(list(map(str,cmd+['-o',target])),check=True,env=env(mode,6))
        for width in [1,2,6]:
            e=env(mode,width)
            output=subprocess.check_output(['taskset','-c',','.join(map(str,CPUS[:width])),target],env=e,text=True)
            d=json.loads(output);assert d['workers_mask'].bit_count()==width,d
            assert d['cpu_mask']&~sum(1<<c for c in CPUS[:width])==0,d
            checks.append(dict(backend=mode,workers=width,**d))
    inputs={str(r[k]):B.digest(Path(r[k])) for r in M.ROWS for k in ['bin','weights','original_file','weights_i32','p1_weighted_file']}
    dependencies={str(p.resolve()):B.digest(p.resolve()) for p in [B.SRC/'reproduce/lib/libcilkrts.so',HIST/'gclock.so',HIST/'walltime',B.GCC/'lib64/libstdc++.so.6',B.GCC/'lib64/libgomp.so.1',B.GCC/'lib64/libgcc_s.so.1']}
    symbols={}
    for mode in ['cilk','openmp']:
        for alg in ALGS:
            s=subprocess.check_output(['nm','-D',exe(mode,alg)],text=True)
            assert ('__cilkrts_cilk_for_' in s)==(mode=='cilk'),(mode,alg)
            assert ('GOMP_parallel' in s)==(mode=='openmp'),(mode,alg)
            deps=subprocess.check_output(['ldd',exe(mode,alg)],env=env(mode,6),text=True)
            if mode=='cilk':assert str(runtime/'libcilkrts.so.5') in deps,deps
            symbols[mode+':'+alg]=dict(symbols=s,dependencies=deps)
    p=dict(build=build,workers=6,physical_cpus=CPUS,worker_probes=checks,inputs=inputs,dependencies=dependencies,symbols=symbols,
        harness_sha256=B.digest(Path(__file__)),extractor_sha256=B.digest(REPO/'tools/p1_campaign/ics22_extract.gdb'),
        primary_repetitions=5,confirmation_repetitions=20,warmups_per_block=1,
        excluded_algorithms={'bfs':'Default historical BFS failed independent shortest-path parent validation; documented UNCOND_EXEC=0 alternative fails compilation in the unchanged historical source.','sssp':'Historical BFv aborts with active frontier mismatch on the first corpus graph, even with one OpenMP worker. No timings accepted.'},
        note='Historical ICS22 revision, GCC 7.2, Cilk Plus language frontend and bundled runtime; matched historical OpenMP control. Native AVX2 target replaces unavailable AVX512. Current GCC13 builds are not a source-matched control. PageRank uses published double variant 11 with maxiter20; convergence can stop it earlier. Its normalization uses the historical padded vertex count; padded output slots are zero. BFS/SSSP are excluded after correctness failures, without source patches.')
    save(OUT/'protocol.json',p);status('preflight_passed')
def audit():
    p=json.loads((OUT/'protocol.json').read_text())
    assert B.inventory(B.SRC)==p['build']['source_hashes']
    for d in p['build']['binaries'].values():assert B.digest(Path(d['path']))==d['sha256']
    for k in ['inputs','dependencies']:
        for path,sha in p[k].items():assert B.digest(Path(path))==sha,path
    assert B.digest(Path(__file__))==p['harness_sha256']
    assert B.digest(REPO/'tools/p1_campaign/ics22_extract.gdb')==p['extractor_sha256']
    return p
def validate_values(alg,row,v):
    if alg=='sssp':
        n=row['vertices'];m=row['arcs_both_directions'];np=M.np
        assert len(v)==n and not np.any(np.isnan(v))
        ip=np.empty(n+1,dtype=np.int64);ip[:n]=np.memmap(row['bin'],mode='r',dtype='<u8',offset=64,shape=(n,));ip[n]=m
        dst=np.memmap(row['bin'],mode='r',dtype='<u4',offset=64+8*n,shape=(m,));src=np.repeat(np.arange(n),np.diff(ip))
        weights=.125+(((src+dst)&127).astype(float)-64)/1024.
        ref=M.dijkstra(M.csr_matrix((weights,dst,ip),shape=(n,n)),directed=True,indices=row['source'])
        reached=np.isfinite(ref);assert np.array_equal(v[reached],ref[reached]),'Historical SSSP distances'
        assert np.all(np.isinf(v[~reached])|(v[~reached]>1e30)), 'Historical SSSP unreachable'
        return dict(passed=True,values=n,reference='Independent SciPy Dijkstra, full vector, unchanged historical synthetic dyadic weights')
    if alg!='pagerank':return M.validate(alg,row,v,'graptor')
    # Preserve the paper benchmark's dangling-mass redistribution,
    # normalization and L1 convergence semantics instead of imposing the
    # modern SGPL fixed-iteration recurrence on a different program.
    n=row['vertices'];m=row['arcs_both_directions'];np=M.np
    assert n<=len(v)<=n+64 and np.all(np.isfinite(v))
    padded=len(v);assert np.all(v[n:]==0)
    ip=np.empty(n+1,dtype=np.int64);ip[:n]=np.memmap(row['bin'],mode='r',dtype='<u8',offset=64,shape=(n,));ip[n]=m
    dst=np.memmap(row['bin'],mode='r',dtype='<u4',offset=64+8*n,shape=(m,));degree=np.diff(ip);src=np.repeat(np.arange(n),degree)
    x=np.full(n,1./padded);iterations=0
    while iterations<20:
        y=np.bincount(dst,weights=(.85*x/np.maximum(degree,1))[src],minlength=n)
        y+=(1.-y.sum())/padded;y/=y.sum();delta=np.abs(x-y).sum();x=y;iterations+=1
        if delta<=1e-7:break
    error=float(np.max(np.abs(v[:n]-x)));assert np.allclose(v[:n],x,rtol=1e-8,atol=1e-12),('Historical PageRank mismatch',error)
    return dict(passed=True,values=n,padded_slots=padded-n,reference='Independent NumPy, full vector; historical padded denominator, redistribution, normalization, L1 convergence, maxiter20',iterations=iterations,max_abs_error=error)
def run(label,alg,row,phase,rep,width=6,validation=False):
    d=ROOT/'runs'/phase/alg/row['graph'].removesuffix('.edges');d.mkdir(parents=True,exist_ok=True)
    tag=f'{label}.w{width}.r{rep}';record=d/(tag+'.json')
    if record.exists():return json.loads(record.read_text())
    for old in list(d.glob(tag+'.*')):old.rename(old.with_name(old.name+'.incomplete_'+str(time.time_ns())))
    output=d/(tag+'.values');cmd=command(label,alg,row,output if validation else None);e=env(label,width);runner_env=e
    if validation and alg in ['bfs','sssp']:
        e.update(ICS22_VERTICES=str(row['vertices']),ICS22_ALGORITHM=alg,ICS22_OUTPUT=str(output))
        # The modern debugger needs system libstdc++; only its inferior uses
        # the historical libraries. This does not alter timed execution.
        runner_env=e.copy();runner_env.pop('LD_LIBRARY_PATH');runner_env.pop('LD_PRELOAD')
        cmd=['gdb','-q','-batch','-iex','set environment LD_LIBRARY_PATH '+e['LD_LIBRARY_PATH'],'-iex','set environment LD_PRELOAD '+e['LD_PRELOAD'],'-x',str(REPO/'tools/p1_campaign/ics22_extract.gdb'),'--args',*cmd]
    stamp=d/(tag+'.stamp.json')
    start=time.monotonic()
    with (d/(tag+'.out')).open('w') as f,(d/(tag+'.err')).open('w') as g:
        p=subprocess.run(['taskset','-c',','.join(map(str,CPUS[:width])),str(HIST/'walltime'),str(stamp),*cmd],env=runner_env,cwd=ROOT,stdout=f,stderr=g,timeout=300)
    assert p.returncode==0,(label,alg,row['graph'],str(d/(tag+'.err')))
    result=json.loads(stamp.read_text());text=(d/(tag+'.out')).read_text()+(d/(tag+'.err')).read_text()
    if validation:
        if alg in ['bfs','sssp']:
            assert Path(str(output)+'.json').exists(),('GDB extraction failed',d)
            mapping=M.np.fromfile(str(output)+'.mapping',dtype='<u4')
            v=M.np.fromfile(str(output)+'.data',dtype='<u4' if alg=='bfs' else '<f4')[mapping]
            if alg=='bfs':
                reverse=M.np.fromfile(str(output)+'.reverse',dtype='<u4');valid=v<len(reverse)
                v=M.np.where(valid,reverse[M.np.minimum(v,len(reverse)-1)].astype(M.np.int64),-1)
        else:v=M.np.loadtxt(output,ndmin=1)
        result['validation']=validate_values(alg,row,v)
        M.np.save(str(output)+'.npy',v)
        result['values_sha256']=B.digest(Path(str(output)+'.npy'))
    else:
        m=re.search(r'Average\s*:\s*([0-9.eE+-]+)',text);assert m,('Missing timer',d)
        result['compute_seconds']=float(m[1]);assert result['compute_seconds']>0
    result.update(label=label,algorithm=alg,graph=row['graph'],phase=phase,repeat=rep,workers=width,command=cmd)
    save(record,result);return result
def validate():
    audit();records=[];pairs=[]
    for width in [1,2,6]:
      for alg in ALGS:
       for i,row in enumerate(M.ROWS,1):
        values={}
        for label in ['openmp','cilk']:
            r=run(label,alg,row,'validation',0,width,True);records.append(r)
            path=ROOT/'runs/validation'/alg/row['graph'].removesuffix('.edges')/f'{label}.w{width}.r0.values.npy'
            values[label]=M.np.load(path)
        pairs.append(dict(algorithm=alg,graph=row['graph'],workers=width,exact_equal=bool(M.np.array_equal(values['openmp'],values['cilk']))))
        status('correctness',workers=width,algorithm=alg,completed=i,total=61)
    audit();save(OUT/'validation.json',dict(passed=True,checks=len(records),records=records,pairs=pairs));status('validated',checks=len(records))
def smoke():
    for mode in ['openmp','cilk']:
        for alg in ALGS:
            if os.environ.get('ICS22_ALGS') and alg not in os.environ['ICS22_ALGS'].split(','):continue
            if exe(mode,alg).exists():
                r=run(mode,alg,M.ROWS[0],'smoke',0,1,True)
                status('smoke_passed',backend=mode,algorithm=alg,validation=r['validation'])
def gm(xs):return math.exp(statistics.mean(math.log(x) for x in xs))
def analyze(name,records):
    groups=collections.defaultdict(list)
    for r in records:groups[r['algorithm'],r['graph'],r['label']].append(r)
    cases=[];tests={k:[] for k in ['openmp_AA','cilk_AA','difference']}
    for alg in ALGS:
      for row in M.ROWS:
        samples={l:sorted(groups[alg,row['graph'],l],key=lambda x:x['repeat']) for l in LABELS}
        c=dict(algorithm=alg,graph=row['graph'],e2e={l:statistics.mean(r['seconds'] for r in xs) for l,xs in samples.items()},compute={l:statistics.mean(r['compute_seconds'] for r in xs) for l,xs in samples.items()})
        for kind,left,right in [('openmp_AA','openmp','openmp_AA'),('cilk_AA','cilk','cilk_AA'),('difference','openmp','cilk')]:
            diff=M.np.log(M.np.array([r['seconds'] for r in samples[right]])/M.np.array([r['seconds'] for r in samples[left]]))
            tests[kind].append(dict(algorithm=alg,graph=row['graph'],log_ratio=float(diff.mean()),p=permutation_pvalue(diff,True)))
        cases.append(c)
    for family in tests.values():
        running=0
        for i,t in enumerate(sorted(family,key=lambda t:t['p'])):running=max(running,min(1,t['p']*(len(family)-i)));t['holm_p']=running
    failures=[dict(t,family=k) for k in ['openmp_AA','cilk_AA'] for t in tests[k] if t['holm_p']<.05]
    summary={alg:dict(e2e_openmp_over_cilk=gm(c['e2e']['openmp']/c['e2e']['cilk'] for c in cases if c['algorithm']==alg),compute_openmp_over_cilk=gm(c['compute']['openmp']/c['compute']['cilk'] for c in cases if c['algorithm']==alg),cilk_e2e_wins=sum(c['e2e']['cilk']<c['e2e']['openmp'] for c in cases if c['algorithm']==alg)) for alg in ALGS}
    r=dict(phase=name,AA_stable=not failures,AA_failures=failures,summary=summary,cases=cases,tests=tests);save(OUT/(name+'_summary.json'),r);return r
def measure():
    protocol=audit();assert json.loads((OUT/'validation.json').read_text())['passed'];blocks={}
    for phase,reps in [('primary',5),('confirmation',20)]:
      for attempt in range(2):
        name=phase if not attempt else phase+'_retry1';records=[]
        for alg in ALGS:
          for i,row in enumerate(M.ROWS,1):
            for l in LABELS:run(l,alg,row,'warmup_'+name,0)
            for rep in range(reps):
                shift=(i+rep)%len(LABELS);order=LABELS[shift:]+LABELS[:shift]
                if rep%2:order=order[::-1]
                for l in order:records.append(run(l,alg,row,name,rep))
            save(OUT/(name+'.json'),records);status(name,algorithm=alg,completed=i,total=61,observations=len(records))
        blocks[name]=analyze(name,records)
        if blocks[name]['AA_stable']:break
    audit();selected=next(reversed(blocks));final=blocks[selected]
    report=dict(protocol=protocol,selected_phase=selected,blocks=blocks,summary=final['summary'],audit_passed=True)
    save(OUT/'final_summary.json',report);save(REPO/'reports/graptor_ics22_benchmark.json',report)
    note=protocol['note'];body='<h1>Historical Graptor: GCC 7.2 Cilk Plus versus OpenMP</h1><p>'+html.escape(note)+'</p><p>Speedup = OpenMP / Cilk. Above one favors Cilk. Geometric means across 61 graphs, six physical cores.</p><p>Selected block: '+selected+'; A/A stable: '+str(final['AA_stable'])+'</p><table><tr><th>Algorithm</th><th>E2E speedup</th><th>Compute speedup</th><th>Cilk E2E wins</th></tr>'
    for alg,s in final['summary'].items():body+=f'<tr><td>{alg}</td><td>{s["e2e_openmp_over_cilk"]:.4f}</td><td>{s["compute_openmp_over_cilk"]:.4f}</td><td>{s["cilk_e2e_wins"]}/61</td></tr>'
    body+='</table><p><a href="final_summary.json">Machine report</a> | <a href="validation.json">Correctness</a></p>'
    (OUT/'final_report.html').write_text('<!doctype html><meta charset="utf-8"><title>ICS22 Graptor</title><style>body{font:16px system-ui;margin:32px}td,th{padding:12px;border-bottom:1px solid #ddd}table{border-collapse:collapse}</style>'+body)
    status('complete',summary=final['summary'])
def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['preflight','smoke','validate','measure']);a=p.parse_args()
    with contextlib.ExitStack() as stack:
        for path in [Path('/home/kazi/p1_campaign_20261019/experiment.lock'),Path('/home/kazi/p1_execution_simplification/experiment.lock'),ROOT/'experiment.lock']:
            f=stack.enter_context(path.open('a+'));fcntl.flock(f,fcntl.LOCK_EX|fcntl.LOCK_NB)
        try:globals()[a.stage]()
        except BaseException as e:status('failed',error=repr(e));raise
if __name__=='__main__':main()
