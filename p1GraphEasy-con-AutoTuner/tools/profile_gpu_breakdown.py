#!/usr/bin/env python3
"""Isolated, exclusive wall-time profiling of the existing GPU execution model."""
from pathlib import Path
import argparse,importlib.util,json,re,shutil,statistics,subprocess
import benchmark_gpu_saved_cpu as B
ROOT=Path('/home/kazi/p1_gpu_profile_20261104/run03')
OUT=B.REPO/'bench_results/gpu_profile_20261104/run03'
BASE=Path('/home/kazi/p1_gpu_sync_20261104/attempt02')
NATIVE=BASE/'native'
SRC=ROOT/'src'

def scope(text,name,bucket):
    pattern=r'(?m)^([^\n;{}]*\b'+re.escape(name)+r'\s*\([^;{}]*?\)\s*\{)'
    text,n=re.subn(pattern,lambda m:m[1]+'\n    GP_SCOPE('+bucket+');',text,count=1)
    assert n==1,('Missing profiling site',name)
    return text

def prepare():
    ROOT.mkdir(parents=True,exist_ok=True);OUT.mkdir(parents=True,exist_ok=True)
    if not SRC.exists():shutil.copytree(NATIVE/'src',SRC)
    for name in ['gpu_invocation.inc','gpu_residency.inc']:
        assert (NATIVE/'src'/name).read_text()==(B.REPO/name).read_text(),'Current runtime differs from validated snapshot'
    for name in ['gpu_profile_scope.h','gpu_profile_runtime.inc']:shutil.copy2(B.REPO/'tools'/name,SRC/name)
    # Recreate instrumentation from the immutable input on every build.
    gpu=(NATIVE/'src/gpu_runtime.c').read_text().replace('#include "gpu_runtime.h"','#include "gpu_profile_scope.h"\n#include "gpu_runtime.h"',1)
    gpu=gpu.replace('#define LOAD_SYM(name)','#include "gpu_profile_runtime.inc"\n\n#define LOAD_SYM(name)',1)
    (SRC/'gpu_runtime.c').write_text(gpu)
    sites={'sgpl_gpu_reserve':'GP_PLAN','gpu_callback_prepare':'GP_PLAN','gpu_context_buffers':'GP_PLAN',
      'sgpl_gpu_upload':'GP_UPLOAD','sgpl_gpu_download':'GP_DOWNLOAD','sgpl_gpu_release':'GP_CLEANUP',
      'sgpl_gpu_execute':'GP_EXECUTE','gpu_invocation_poll':'GP_POLL'}
    inv=(NATIVE/'src/gpu_invocation.inc').read_text()
    for name,bucket in sites.items():inv=scope(inv,name,bucket)
    inv=inv.replace('s->stream_query(s->streams[instance])','GP_CALL(GP_QUERY,s->stream_query(s->streams[instance]))')
    inv=inv.replace('s->stream_destroy(s->streams[i])','GP_CALL(GP_STREAM,s->stream_destroy(s->streams[i]))')
    inv=inv.replace('p_cuMemcpyDtoH(&s->host_status[instance]','gp_status_download(&s->host_status[instance]')
    needle='    CUresult joined=wait?p_cuStreamSynchronize(s->streams[instance]):CUDA_SUCCESS;'
    assert needle in inv;inv=inv.replace(needle,needle+'\n    if(joined==CUDA_SUCCESS&&sync==CUDA_SUCCESS)gp_event_collect(s->streams[instance]);')
    (SRC/'gpu_invocation.inc').write_text(inv)
    resident=(NATIVE/'src/gpu_residency.inc').read_text()
    for name,bucket in {'gpu_resident_acquire':'GP_CACHE',
      'sgpl_gpu_host_barrier':'GP_COHERENCE','sgpl_gpu_unregister_buffer':'GP_COHERENCE','sgpl_gpu_residency_end':'GP_COHERENCE'}.items():
        resident=scope(resident,name,bucket)
    (SRC/'gpu_residency.inc').write_text(resident)
    auto=(NATIVE/'src/autotuner_runtime.c').read_text().replace('#include "gpu_runtime.h"','#include "gpu_profile_scope.h"\n#include "gpu_runtime.h"',1)
    sites={'autograph_build_clean_cut':'GP_CUT','sgpl_exec_spatial_begin':'GP_BEGIN',
      'sgpl_exec_spatial_publish':'GP_PUBLISH','sgpl_gpu_prepare_spatial_records':'GP_RECORDS',
      'autograph_exec_ctx_create':'GP_CONTEXT_PREP','autograph_exec_ctx_prepare_spatial':'GP_CONTEXT_PREP',
      'autograph_exec_ctx_reserve_frontier':'GP_FRONTIER','autograph_exec_ctx_materialize_frontier_array':'GP_FRONTIER',
      'autograph_exec_ctx_materialize_frontier_bitmap':'GP_FRONTIER','autograph_exec_ctx_materialize_snapshot':'GP_SNAPSHOT',
      'sgpl_exec_spatial_finish':'GP_FINISH','autograph_exec_ctx_destroy':'GP_FINISH'}
    for name,bucket in sites.items():auto=scope(auto,name,bucket)
    (SRC/'autotuner_runtime.c').write_text(auto)
    for name in ['gpu_runtime','autotuner_runtime']:
        B.run(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.', '-c',name+'.c','-o',ROOT/(name+'.o')],SRC,name+'.build',None,1200)
    B.save(OUT/'protocol.json',dict(native=str(NATIVE),compiler_unchanged=True,instrumentation_only=True,
      input_objects={name:B.sha(NATIVE/(name+'.o')) for name in B.OBJECTS},
      instrumented_sources={str(p.relative_to(SRC)):B.sha(p) for p in SRC.iterdir() if p.is_file() and p.name in ['gpu_runtime.c','gpu_invocation.inc','gpu_residency.inc','autotuner_runtime.c','gpu_profile_scope.h','gpu_profile_runtime.inc']},
      attribution='Main-thread exclusive host wall time. CUDA API times include blocking inside those calls. Device event intervals overlap host times and are not additive.',
      timing_source='CLOCK_MONOTONIC, same algorithm timer boundaries',
      device_timing='Separate event run, no event synchronization; collects after the existing stream completion check. Intervals include stream scheduling/submission gaps.',
      event_documentation='https://docs.nvidia.com/cuda/cuda-driver-api/cuda_driver_api/group__CUDA__EVENT.html'))

def binary(algorithm,graph,mode):
    d=ROOT/'cases'/algorithm/graph.removesuffix('.edges');d.mkdir(parents=True,exist_ok=True)
    original=Path('/home/kazi/p1_gpu_batch_20261104/benchmark/cases')/algorithm/graph.removesuffix('.edges')/(mode+'.o')
    objects=[ROOT/(n+'.o') if n in ['gpu_runtime','autotuner_runtime'] else NATIVE/(n+'.o') for n in B.OBJECTS]
    target=d/mode
    B.run(['g++','-O3','-fopenmp','-no-pie',original,*objects,'-Wl,--wrap=timer_runtime','-Wl,--wrap=main','-pthread','-ldl','-lnlopt','-o',target],d,mode+'.link',None)
    return d,target

def run_case(algorithm,graph,common,reps,drill=False):
    import numpy as np
    row=next(x for x in common.ROWS if x['graph']==graph)
    env=common.env();env.update(FORCE_CPU='0',FORCE_GPU='1',SGPL_GPU_BACKEND='1',SGPL_GPU_BATCH='1',SGPL_GPU_SYNC_ELIDE='1')
    d,validate=binary(algorithm,graph,'validate')
    output=B.run([validate],d,'validation',dict(env,SGPL_PROFILE_JSON=str(d/'validation.profile.json')))
    values=np.array(output.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split(),dtype=float)
    common.validate(algorithm,row,values,'p1')
    d,profile=binary(algorithm,graph,'timed')
    baseline=BASE/'cases'/algorithm/graph.removesuffix('.edges')/'timed'
    # The previous control campaign only linked four timed cases; link other
    # identical uninstrumented objects without adding any runtime behavior.
    if not baseline.exists():
        baseline=d/'baseline'
        original=Path('/home/kazi/p1_gpu_batch_20261104/benchmark/cases')/algorithm/graph.removesuffix('.edges')/'timed.o'
        B.run(['g++','-O3','-fopenmp','-no-pie',original,*[NATIVE/(n+'.o') for n in B.OBJECTS],B.HIST/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',baseline],d,'baseline.link',env)
    records=[]
    for executable,tag in ([] if drill else [(baseline,'warmup.baseline'),(profile,'warmup.profile')]):
        B.run([executable],d,tag,dict(env,SGPL_PROFILE_JSON=str(d/(tag+'.json'))))
    for repeat in range(reps):
        for mode in (['baseline','profile'] if repeat%2==0 else ['profile','baseline']):
            tag=mode+'.'+str(repeat);stamp=d/(tag+'.stamp.json');path=d/(tag+'.profile.json')
            output=B.run([B.CPU_ROOT/'walltime',stamp,baseline if mode=='baseline' else profile],d,tag,dict(env,SGPL_PROFILE_JSON=str(path)))
            sample=json.loads(stamp.read_text());sample.update(mode=mode,repeat=repeat,compute_ms=float(re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)',output)[1])*1000)
            if mode=='profile':
                p=json.loads(path.read_text());assert p['timer_calls']==2,p
                assert not any(v for n,v in p['other_thread_buckets'].items() if n.startswith(('cuda_','device_')) or n in ['host_to_device','status_download','launch_api','execute_wait_and_scheduler','plan_and_descriptors']),p
                assert abs(sum(x['main_ms'] for x in p['buckets'].values())-p['main_ms'])<.001
                assert abs(sum(x['compute_ms'] for x in p['buckets'].values())-p['compute_ms'])<.001
                assert abs(p['compute_ms']-sample['compute_ms'])<.001
                sample['profile']=p
            records.append(sample)
    events=None
    if not drill:
        event_path=d/'device_events.json'
        B.run([profile],d,'device_events',dict(env,SGPL_PROFILE_JSON=str(event_path),SGPL_PROFILE_EVENTS='1'))
        events=json.loads(event_path.read_text());assert events['event_errors']==0 and events['event_count']==events['event_collected']==events['launches'],events
    result=dict(algorithm=algorithm,graph=graph,validation_passed=True,samples=records,events=events,
      baseline_sha256=B.sha(baseline),profile_sha256=B.sha(profile))
    B.save(d/'result.json',result)
    print('PROFILED',algorithm,graph,flush=True)
    return result

def main():
    global ROOT,OUT,SRC
    parser=argparse.ArgumentParser();parser.add_argument('--limit',type=int);parser.add_argument('--repetitions',type=int,default=3)
    parser.add_argument('--campaign',default='run04');parser.add_argument('--drilldown',action='store_true')
    args=parser.parse_args()
    assert re.fullmatch(r'[A-Za-z0-9_-]+',args.campaign)
    ROOT=ROOT.parent/args.campaign;OUT=OUT.parent/args.campaign;SRC=ROOT/'src'
    assert not ROOT.exists(),'Preserve existing observations; choose a new --campaign'
    prepare()
    spec=importlib.util.spec_from_file_location('common','/home/kazi/p1_c3_integration_20261022/support/common.py')
    common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
    cases=[('pagerank','dimacs/hamming8-2.edges'),('pagerank','soc/soc-slashdot.edges'),('bfs','soc/soc-slashdot.edges'),('cc','dimacs10/fe-4elt2.edges'),('cc','dimacs10/inf-luxembourg_osm.edges'),('sssp','soc/soc-slashdot.edges')]
    if args.drilldown:cases=[('pagerank','soc/soc-slashdot.edges'),('cc','dimacs10/inf-luxembourg_osm.edges')]
    if args.limit:cases=cases[:args.limit]
    results=[]
    for a,g in cases:
        results.append(run_case(a,g,common,args.repetitions,args.drilldown));B.save(OUT/'results.json',results)

if __name__=='__main__':main()
