#!/usr/bin/env python3
"""Resumable CPU build, independent validation and matched end-to-end timing.

All observations are retained on native Linux storage. Benchmark only after
the candidate's correctness and regression results have been reviewed.
"""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import math
import os
import shutil
import statistics
import subprocess
import time
import numpy as np

REPO=Path(__file__).resolve().parents[1]
ROOT=Path('/home/kazi/p1_layout_native_20261105')
OBJECTS=['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime',
         'roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']
spec=importlib.util.spec_from_file_location('common','/home/kazi/p1_c3_integration_20261022/support/common.py')
M=importlib.util.module_from_spec(spec);spec.loader.exec_module(M)

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save(p,x):
    p.parent.mkdir(parents=True,exist_ok=True)
    tmp=p.with_suffix('.tmp');tmp.write_text(json.dumps(x,indent=2));tmp.replace(p)
def checked(cmd,cwd,log,env=None,timeout=300):
    with log.open('w') as output:
        subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,stdout=output,
                       stderr=subprocess.STDOUT,check=True,timeout=timeout)
def directory(version,alg,row):
    return ROOT/'versions'/version/'cases'/alg/row['graph'].removesuffix('.edges')
def environment(workers=6):
    env=M.env(workers=workers)
    env.update(FORCE_CPU='1',FORCE_GPU='0',SGPL_GPU_BACKEND='0')
    return env

def input_identity():
    identities={}
    for row in M.ROWS:
        paths={k:row[k] for k in ['original_file','p1_weighted_file','bin','weights_i32']}
        hashes={k:sha(p) for k,p in paths.items()}
        assert hashes['original_file']==row['original_sha256'],row['graph']
        identities[row['graph']]=dict(paths=paths,hashes=hashes,source=row['source'],
                                      vertices=row['vertices'],directed=row['p1_directed'])
    identities['source_generator']=sha(M.ROOT/'historical_harness/p1_bench_snapshot.py')
    identities['common_helper']=sha(M.ROOT/'support/common.py')
    p=ROOT/'cpu_inputs.json'
    if p.exists():assert json.loads(p.read_text())==identities,'Input changed; do not mix campaigns'
    else:save(p,identities)
    return sha(p)
def main():
    parser=argparse.ArgumentParser();parser.add_argument('phase',choices=['build','validate','benchmark'])
    parser.add_argument('version');parser.add_argument('--limit',type=int,default=0)
    parser.add_argument('--repetitions',type=int,default=5)
    parser.add_argument('--reuse-compiled-from',default='')
    args=parser.parse_args();assert args.version.replace('_','').isalnum()
    os.sched_setaffinity(0,[0,2,4,6,8,10])
    inputs_sha256=input_identity()
    cases=[(a,r) for a in ['bfs','sssp','cc','pagerank'] for r in M.ROWS]
    if args.limit:cases=cases[:args.limit]
    version=ROOT/'versions'/args.version
    assert (version/'build.json').exists()
    identity=dict(compiler=sha(version/'GraphProgram'),
                  objects={n:sha(version/(n+'.o')) for n in OBJECTS})
    protocol=version/'cpu_identity.json'
    if protocol.exists():assert json.loads(protocol.read_text())==identity
    else:save(protocol,identity)
    if args.phase=='build':
        reuse=None
        if args.reuse_compiled_from:
            reuse=ROOT/'versions'/args.reuse_compiled_from
            assert sha(reuse/'GraphProgram')==identity['compiler'],'Only reuse an identical compiler output'
        # Neither executable is run during this phase.
        timer=version/'cpu_timer.c'
        timer.write_text('#include <time.h>\ndouble __wrap_timer_runtime(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}\n')
        checked(['gcc','-O2','-c',timer,'-o',version/'cpu_timer.o'],version,version/'cpu_timer.build.log')
        for index,(alg,row) in enumerate(cases):
            d=directory(args.version,alg,row);d.mkdir(parents=True,exist_ok=True)
            for ll in (version/'src').glob('*.ll'):
                target=d/ll.name
                if not target.exists():target.symlink_to(ll)
            for mode in ['validate','timed']:
                source=d/(mode+'.graph');contents=M.make_source(alg,row,mode=='validate')
                if source.exists():assert source.read_text()==contents
                else:source.write_text(contents)
                if reuse and not (d/(mode+'.o')).exists():
                    old=directory(args.reuse_compiled_from,alg,row)
                    assert (old/(mode+'.graph')).read_text()==contents
                    assert (old/mode).exists(),'Only reuse successfully compiled/linked cases'
                    shutil.copy2(old/(mode+'.o'),d/(mode+'.o'))
                    save(d/(mode+'.compile_reuse.json'),dict(from_version=args.reuse_compiled_from,
                         compiler_sha256=identity['compiler'],object_sha256=sha(d/(mode+'.o')),
                         reason='Identical compiler, flags and source; relink all candidate runtime objects'))
                if not (d/(mode+'.o')).exists():
                    checked([version/'GraphProgram','--ir-backend=cpu',source],d,d/(mode+'.compile.log'),environment(),1200)
                    shutil.move(d/'program.o',d/(mode+'.o'))
                if not (d/mode).exists():
                    checked(['g++','-O3','-fopenmp','-no-pie',d/(mode+'.o'),
                             *[version/(n+'.o') for n in OBJECTS],version/'cpu_timer.o',
                             '-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',d/mode],d,d/(mode+'.link.log'))
            save(version/'cpu_build_progress.json',dict(completed=index+1,total=len(cases),algorithm=alg,graph=row['graph']))
            print('BUILD',args.version,index+1,len(cases),alg,row['graph'],flush=True)
        return
    if args.phase=='validate':
        output=version/'cpu_validation.json'
        results=json.loads(output.read_text()) if output.exists() else []
        done={(x['algorithm'],x['graph'],x['workers']) for x in results if x['passed']}
        for alg,row in cases:
            d=directory(args.version,alg,row)
            for workers in [1,2,6]:
                if (alg,row['graph'],workers) in done:continue
                env=environment(workers);env['SGPL_LAYOUT_EXEC_DEBUG']='1'
                log=d/('validate_w'+str(workers)+'.log')
                # Keep diagnostics separate: buffered stdout and stderr can
                # otherwise interleave in the middle of a printed vector.
                with log.open('w') as stdout, log.with_suffix('.stderr').open('w') as stderr:
                    subprocess.run([str(d/'validate')],cwd=d,env=env,stdout=stdout,
                                   stderr=stderr,check=True,timeout=300)
                text=log.read_text();values=np.array(text.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split(),dtype=float)
                validation=M.validate(alg,row,values,'p1')
                result=dict(algorithm=alg,graph=row['graph'],workers=workers,passed=True,
                            validation=validation,layout_invocations=log.with_suffix('.stderr').read_text().count('[layout-execution]'),
                            executable_sha256=sha(d/'validate'))
                results.append(result);save(output,results)
                print('VALIDATE',args.version,len(results),alg,row['graph'],workers,flush=True)
        return
    # No benchmark runs until independent full-vector validation covers every case.
    for v in ['baseline',args.version]:
        rows=json.loads((ROOT/'versions'/v/'cpu_validation.json').read_text())
        passed={(x['algorithm'],x['graph'],x['workers']) for x in rows if x['passed']}
        assert all((a,r['graph'],w) in passed for a,r in cases for w in [1,2,6])
    out=ROOT/'benchmarks'/args.version;out.mkdir(parents=True,exist_ok=True)
    wrapper=ROOT/'walltime'
    if not wrapper.exists():
        shutil.copy2('/home/kazi/p1_c3_integration_20261022/walltime',wrapper)
        shutil.copy2('/home/kazi/p1_c3_integration_20261022/walltime.c',ROOT/'walltime.c')
    protocol=dict(baseline='baseline',candidate=args.version,workers=6,cpus=[0,2,4,6,8,10],
                  repetitions=args.repetitions,cases=[(a,r['graph']) for a,r in cases],
                  metric='CLOCK_MONOTONIC around fork/exec and blocking wait4 in the frozen native wrapper; stdout/stderr discarded',
                  wrapper_sha256=sha(wrapper),
                  inputs_sha256=inputs_sha256,
                  runtime_environment={k:v for k,v in environment().items() if k.startswith(('SGPL_','FORCE_','OMP_','GOMP_'))},
                  ordering='one warmup per variant per case, then alternating AB/BA',
                  note='Includes process launch, loader, graph input/conversion, setup, execution and exit.')
    p=out/'protocol.json'
    if p.exists():assert json.loads(p.read_text())==protocol
    else:save(p,protocol)
    results=json.loads((out/'samples.json').read_text()) if (out/'samples.json').exists() else []
    done={(x['algorithm'],x['graph'],x['version'],x['repeat']) for x in results}
    for alg,row in cases:
        variants=['baseline',args.version]
        for v in variants:
            d=directory(v,alg,row)
            subprocess.run([str(d/'timed')],cwd=d,env=environment(),check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=300)
        for repeat in range(args.repetitions):
            for v in variants[::1 if repeat%2==0 else -1]:
                key=(alg,row['graph'],v,repeat)
                if key in done:continue
                d=directory(v,alg,row);env=environment()
                stamp=out/('stamp_'+hashlib.sha256(repr(key).encode()).hexdigest()+'.json')
                if not stamp.exists():
                    subprocess.run([str(wrapper),str(stamp),str(d/'timed')],cwd=d,env=env,check=True,
                                   stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=200)
                measured=json.loads(stamp.read_text());assert measured['exit_code']==0
                elapsed=measured['seconds']*1e9
                results.append(dict(algorithm=alg,graph=row['graph'],version=v,repeat=repeat,wall_ns=elapsed,
                                    executable_sha256=sha(d/'timed')))
                save(out/'samples.json',results)
        print('BENCHMARK',alg,row['graph'],flush=True)
    comparisons=[]
    for alg,row in cases:
        medians={v:statistics.median(x['wall_ns'] for x in results if x['algorithm']==alg and x['graph']==row['graph'] and x['version']==v)/1e6 for v in ['baseline',args.version]}
        comparisons.append(dict(algorithm=alg,graph=row['graph'],baseline_ms=medians['baseline'],candidate_ms=medians[args.version],speedup=medians['baseline']/medians[args.version]))
    save(out/'comparison.json',comparisons)
    save(out/'summary.json',dict(cases=len(cases),samples=len(results),end_to_end_geomean_speedup=math.exp(statistics.mean(math.log(x['speedup']) for x in comparisons))))

if __name__=='__main__':main()
