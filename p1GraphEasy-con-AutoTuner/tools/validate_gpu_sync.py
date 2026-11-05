#!/usr/bin/env python3
"""Validate sync removal on the saved corpus and time alternating controls."""
from pathlib import Path
import importlib.util,json,os,re,shutil,statistics,subprocess
import benchmark_gpu_saved_cpu as B
ROOT=Path('/home/kazi/p1_gpu_sync_20261104/attempt02')
OUT=B.REPO/'bench_results/gpu_sync_20261104/attempt02'
OLD=Path('/home/kazi/p1_gpu_batch_20261104/versions/candidate02')
CASES=Path('/home/kazi/p1_gpu_batch_20261104/benchmark/cases')
NATIVE=ROOT/'native'
OUT.mkdir(parents=True,exist_ok=True);NATIVE.mkdir(parents=True,exist_ok=True)
source=NATIVE/'src'
if not source.exists():
    shutil.copytree(OLD/'src',source)
    for name in ['gpu_invocation.inc','gpu_residency.inc']:shutil.copy2(B.REPO/name,source/name)
else:
    for name in ['gpu_invocation.inc','gpu_residency.inc']:assert (B.REPO/name).read_bytes()==(source/name).read_bytes()
for name in B.OBJECTS:
    if name!='gpu_runtime' and not (NATIVE/(name+'.o')).exists():shutil.copy2(OLD/(name+'.o'),NATIVE/(name+'.o'))
if not (NATIVE/'gpu_runtime.o').exists():
    subprocess.run(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.', '-c','gpu_runtime.c','-o',str(NATIVE/'gpu_runtime.o')],cwd=source,check=True)
spec=importlib.util.spec_from_file_location('common','/home/kazi/p1_c3_integration_20261022/support/common.py')
common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
env=common.env();env.update(FORCE_CPU='0',FORCE_GPU='1',SGPL_GPU_BACKEND='1',SGPL_GPU_BATCH='1')
timer=Path('/home/kazi/p1_effect_fusion_local_20261022/monotonic_timer.o')
def executable(algorithm,row,mode):
    d=ROOT/'cases'/algorithm/row['graph'].removesuffix('.edges');d.mkdir(parents=True,exist_ok=True)
    target=d/mode
    if not target.exists():
        obj=CASES/algorithm/row['graph'].removesuffix('.edges')/(mode+'.o')
        B.run(['g++','-O3','-fopenmp','-no-pie',obj,*[NATIVE/(n+'.o') for n in B.OBJECTS],timer,'-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',target],d,mode+'.link',env)
    return d,target
path=OUT/'validation.json'
records=json.loads(path.read_text()) if path.exists() else []
done={(x['algorithm'],x['graph']) for x in records}
import numpy as np
for row in common.ROWS:
    for algorithm in ['bfs','sssp','cc','pagerank']:
        if (algorithm,row['graph']) in done:continue
        d,binary=executable(algorithm,row,'validate')
        output=B.run([binary],d,'validation',dict(env,SGPL_GPU_SYNC_ELIDE='1',SGPL_GPU_SYNC_STATS='1',SGPL_GPU_DEBUG='1'))
        values=np.array(output.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split(),dtype=float)
        checked=common.validate(algorithm,row,values,'p1');trace=(d/'validation.err').read_text()
        waits=[tuple(map(int,m)) for m in re.findall(r'\[gpu-sync\] poll_waits=(\d+) poll_elided=(\d+) release_waits=(\d+) release_elided=(\d+) failed=(\d+)',trace)]
        assert all(x[0]==x[2]==x[4]==0 for x in waits)
        record=dict(algorithm=algorithm,graph=row['graph'],validation=checked,gpu_tasks=trace.count('[gpu-task]'),
                    wait_totals=[sum(x[i] for x in waits) for i in range(5)],resident_waits=trace.count('[gpu-sync-resident] wait=1'),
                    resident_elided=trace.count('[gpu-sync-resident] wait=0'),executable_sha256=B.sha(binary))
        records.append(record);B.save(path,records)
        if len(records)%8==0:print('VALIDATED',len(records),'of 244',flush=True)
selected=[('pagerank','dimacs/hamming8-2.edges'),('pagerank','soc/soc-slashdot.edges'),('bfs','soc/soc-slashdot.edges'),('cc','dimacs10/fe-4elt2.edges')]
controls=[]
for algorithm,graph in selected:
    row=next(x for x in common.ROWS if x['graph']==graph)
    d,binary=executable(algorithm,row,'timed');samples={'0':[],'1':[]};diagnostics={}
    for mode in ['0','1']:
        tag='warmup_'+mode
        B.run([binary],d,tag,dict(env,SGPL_GPU_SYNC_ELIDE=mode,SGPL_GPU_SYNC_STATS='1'))
        trace=(d/(tag+'.err')).read_text()
        waits=[tuple(map(int,m)) for m in re.findall(r'\[gpu-sync\] poll_waits=(\d+) poll_elided=(\d+) release_waits=(\d+) release_elided=(\d+) failed=(\d+)',trace)]
        diagnostics[mode]=dict(wait_totals=[sum(x[i] for x in waits) for i in range(5)],resident_waits=trace.count('[gpu-sync-resident] wait=1'),resident_elided=trace.count('[gpu-sync-resident] wait=0'))
    for repeat in range(5):
        for mode in (['0','1'] if repeat%2==0 else ['1','0']):
            tag='repeat_'+mode+'_'+str(repeat);stamp=d/(tag+'.stamp.json')
            output=B.run([B.CPU_ROOT/'walltime',stamp,binary],d,tag,dict(env,SGPL_GPU_SYNC_ELIDE=mode))
            sample=json.loads(stamp.read_text());assert sample['exit_code']==0
            sample['internal_seconds']=float(re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)',output)[1]);samples[mode].append(sample)
    entry=dict(algorithm=algorithm,graph=graph,samples=samples,diagnostics=diagnostics)
    for metric in ['seconds','internal_seconds']:
        means={mode:statistics.mean(s[metric] for s in xs) for mode,xs in samples.items()}
        entry[metric+'_means']=means;entry[metric+'_speedup']=means['0']/means['1']
    controls.append(entry);B.save(OUT/'controls.json',controls)
    print('CONTROL',algorithm,graph,entry['seconds_speedup'],entry['internal_seconds_speedup'],flush=True)
B.save(OUT/'summary.json',dict(passed=True,validated_cases=len(records),gpu_cases=sum(x['gpu_tasks']>0 for x in records),controls=controls,
    native_gpu_runtime_sha256=B.sha(NATIVE/'gpu_runtime.o'),source_sha256={n:B.sha(source/n) for n in ['gpu_invocation.inc','gpu_residency.inc']},
    note='Four selected alternating same-binary controls, five samples per mode; no full-corpus speedup claim.'))
