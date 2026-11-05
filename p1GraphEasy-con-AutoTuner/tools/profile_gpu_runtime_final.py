#!/usr/bin/env python3
"""Exclusive end-to-end wall-time attribution for the final O3 runtime.

All instrumentation is isolated from production objects. Results stay on the
native Linux filesystem until all baseline/profile measurements finish.
"""
from pathlib import Path
import importlib.util,json,shutil,statistics
import profile_gpu_breakdown as P

ROOT=Path('/home/kazi/p1_gpu_runtime_reason_20261105/prod01')
OUT=ROOT/'results'
PUBLISH=P.B.REPO/'bench_results/gpu_runtime_reason_20261105/prod01'
P.ROOT=ROOT;P.OUT=OUT;P.SRC=ROOT/'src'
P.NATIVE=Path('/home/kazi/p1_gpu_four_correctness_20261104/correct03_o3/native')
P.BASE=ROOT/'baseline'
CASES=[('bfs','soc/soc-slashdot.edges'),('pagerank','dimacs/hamming8-2.edges'),
       ('pagerank','soc/soc-slashdot.edges'),('cc','dimacs10/fe-4elt2.edges'),
       ('cc','dimacs10/inf-luxembourg_osm.edges')]
GROUPS={
 'CUDA startup/module':['cuda_init','cuda_module_and_lookup'],
 'CPU setup/finalization':['cpu_preparation','cpu_finalize','prep_clean_cut','prep_records','prep_snapshot','prep_context','prep_frontier','prep_begin','prep_publish'],
 'Transfer API calls':['host_to_device','device_to_host','status_download'],
 'Host staging/coherence':['upload_staging','download_publication','resident_cache','host_coherence'],
 'Device allocation/streams':['device_allocation','device_free','stream_management'],
 'Submission/completion':['execute_wait_and_scheduler','poll_cpu','stream_query','explicit_sync','launch_api'],
 'Other host/input/runtime':['other_host','plan_and_descriptors','cleanup','context_set','profiler_events']}

original_scope=P.scope
def scope(text,name,bucket):
    if name=='sgpl_gpu_prepare_spatial_records':text=original_scope(text,'sgpl_gpu_spatial_preflight','GP_RECORDS')
    if name=='gpu_resident_acquire':text=original_scope(text,'gpu_capacity_acquire','GP_CACHE')
    return original_scope(text,name,bucket)
P.scope=scope
original_run=P.B.run
def run(cmd,directory,tag,env,timeout=190):
    if cmd[0]=='gcc' and 'gpu_runtime.c' in cmd:
        path=P.SRC/'gpu_runtime.c';text=path.read_text()
        text=original_scope(text,'load_cuda','GP_INIT')
        text=original_scope(text,'load_module','GP_MODULE');path.write_text(text)
    return original_run(cmd,directory,tag,env,timeout)
P.B.run=run

def summarize(records):
    summaries=[]
    for r in records:
        baseline=[s for s in r['samples'] if s['mode']=='baseline']
        profile=[s for s in r['samples'] if s['mode']=='profile']
        profiles=[s['profile'] for s in profile]
        assert set(sum(GROUPS.values(),[]))==set(profiles[0]['buckets'])
        wall=statistics.mean(s['seconds']*1000 for s in profile)
        base_wall=statistics.mean(s['seconds']*1000 for s in baseline)
        main=statistics.mean(p['main_ms'] for p in profiles)
        groups={name:sum(statistics.mean(p['buckets'][b]['main_ms'] for p in profiles) for b in buckets)
                for name,buckets in GROUPS.items()}
        groups['Outside instrumented main']=wall-main
        assert abs(sum(groups.values())-wall)<.001
        summaries.append(dict(algorithm=r['algorithm'],graph=r['graph'],baseline_e2e_ms=base_wall,
            profiled_e2e_ms=wall,profiling_delta_pct=100*(wall/base_wall-1),
            groups_ms=groups,groups_pct={k:100*v/wall for k,v in groups.items()},
            detailed_main_ms={b:statistics.mean(p['buckets'][b]['main_ms'] for p in profiles) for b in profiles[0]['buckets']},
            transfer_bytes={b:statistics.mean(p['buckets'][b]['bytes'] for p in profiles) for b in ['host_to_device','device_to_host','status_download']},
            gpu_launches=r['events']['launches'],device_event_interval_ms=r['events']['event_ms']))
    P.B.save(OUT/'summary.json',summaries)
    lines=['# Final GPU runtime: end-to-end timing attribution','',
        'Five selected cases, three alternating uninstrumented/profiled fresh-process pairs per case, after one warmup per variant. Final correctness-checked runtime source and O3 flags. Instrumented output-validation programs passed independent full-vector checks. Production source, production objects and GPU kernel code are unchanged. All intermediate writes stayed on native Linux storage.','',
        'Buckets are exclusive wall time on the main thread, including time waiting for CPU workers or device completion inside each call. They sum to profiled whole-process wall time. CUDA API time includes blocking; transfer-call time is not pure PCIe bandwidth. Setup/finalization includes lifecycle callbacks and result publication, so it is not all disposable bookkeeping. Other host/input/runtime can include useful CPU fallback algorithm work.','',
        'Outside instrumented main includes executable/library loading, exit and profile report I/O. It was not attributed specifically to CUDA teardown. Device event intervals are separate, overlap the wall-time buckets, and include stream scheduling/submission gaps; they are not isolated arithmetic time.','',
        '| Case | Uninstrumented process ms | Profiled process ms | Difference |',
        '|---|---:|---:|---:|']
    for x in summaries:lines.append(f"| {x['algorithm']}, {x['graph']} | {x['baseline_e2e_ms']:.2f} | {x['profiled_e2e_ms']:.2f} | {x['profiling_delta_pct']:+.1f}% |")
    lines+=['','## Exclusive shares of whole-process time','',
        '| Case | CUDA startup/module | CPU setup/finalize | Transfers | Host staging/coherence | Alloc/streams | Submit/complete | Other host/input/runtime | Outside main |',
        '|---|---:|---:|---:|---:|---:|---:|---:|---:|']
    for x in summaries:lines.append('| '+x['algorithm']+', '+x['graph']+' | '+' | '.join(f'{v:.1f}%' for v in x['groups_pct'].values())+' |')
    lines+=['','## Detailed observations','']
    for x in summaries:
        lines.append(x['algorithm']+' / '+x['graph'])
        for name,value in x['groups_ms'].items():lines.append(f'- {name}: {value:.2f} ms ({x["groups_pct"][name]:.1f}%)')
        lines.append(f'- GPU launches: {x["gpu_launches"]}. Separately measured stream event interval: {x["device_event_interval_ms"]:.2f} ms; overlaps the wall-time buckets.')
        lines.append('')
    lines+=['These are selected-case diagnostics, not a new full-corpus speed comparison. Profiling changes execution overhead; the paired uninstrumented/profiled differences above bound how literally to read the attributed shares. Use the uninstrumented primary benchmark for end-to-end speedup claims.','']
    (OUT/'report.md').write_text('\n'.join(lines))

def main():
    P.prepare()
    for name in ['profile_gpu_runtime_final.py','profile_gpu_breakdown.py','gpu_profile_runtime.inc','gpu_profile_scope.h']:
        shutil.copy2(P.B.REPO/'tools'/name,ROOT/name)
    P.B.save(OUT/'harness.json',{name:P.B.sha(ROOT/name) for name in ['profile_gpu_runtime_final.py','profile_gpu_breakdown.py','gpu_profile_runtime.inc','gpu_profile_scope.h']})
    spec=importlib.util.spec_from_file_location('common','/home/kazi/p1_c3_integration_20261022/support/common.py')
    common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
    records=[]
    for algorithm,graph in CASES:
        d=ROOT/'cases'/algorithm/graph.removesuffix('.edges')
        record=json.loads((d/'result.json').read_text()) if (d/'result.json').exists() else P.run_case(algorithm,graph,common,3)
        records.append(record);P.B.save(OUT/'results.json',records)
    summarize(records)
    PUBLISH.mkdir(parents=True,exist_ok=True)
    for path in OUT.iterdir():
        if path.is_file():shutil.copy2(path,PUBLISH/path.name)
    print('PROFILE COMPLETE',PUBLISH,flush=True)

if __name__=='__main__':main()
