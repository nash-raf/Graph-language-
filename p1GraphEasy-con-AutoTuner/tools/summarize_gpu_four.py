#!/usr/bin/env python3
"""Summarize preserved benchmark samples; do not execute benchmark programs."""
from pathlib import Path
import hashlib,json,math,statistics,sys
import numpy as np

REPO=Path(__file__).resolve().parents[1]
OUT=REPO/'bench_results/gpu_four_benchmark_20261104/attempt01'
if len(sys.argv)>1:OUT=Path(sys.argv[1]).resolve()
FLAGS=['SGPL_GPU_REUSE_RECORDS','SGPL_GPU_EARLY_REJECT','SGPL_GPU_TRANSFER_REDUCE','SGPL_GPU_REUSE_CAPACITY']
def gm(xs):return math.exp(statistics.mean(map(math.log,xs)))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def interval(rows,field):
    # Descriptive hierarchical bootstrap: cases, then paired repetition indices.
    # This is not a significance test or a correction for multiple comparisons.
    before=np.array([[s[field] for s in r['samples']['previous']] for r in rows])
    after=np.array([[s[field] for s in r['samples']['current']] for r in rows])
    rng=np.random.default_rng(20261104);n=len(rows);reps=before.shape[1]
    cases=rng.integers(n,size=(3000,n));runs=rng.integers(reps,size=(3000,n,reps))
    a=before[cases[:,:,None],runs].mean(axis=2);b=after[cases[:,:,None],runs].mean(axis=2)
    values=np.exp(np.log(a/b).mean(axis=1))
    return list(map(float,np.quantile(values,[.025,.975])))

def main():
    data=json.loads((OUT/'results.json').read_text());controls=json.loads((OUT/'controls.json').read_text())
    status=json.loads((OUT/'status.json').read_text());assert status['stage']=='complete'
    rows=data['cases'];assert len(rows)==244 and len(controls)==4 and all(r['complete'] for r in rows)
    assert all(len(r['samples'][m])==5 for r in rows for m in ['previous','current'])
    assert all(r['diagnostics']['previous']['gpu_tasks']==r['diagnostics']['current']['gpu_tasks'] for r in rows)
    assert json.loads((OUT/'baseline_audit.json').read_text())['passed']
    optimization='-O3' if ' -O3 ' in data['protocol']['baseline'] else '-O2'
    summary={};lines=['# GPU execution realization benchmarks','',
        f'61 graphs × four algorithms; five fresh-process samples per version, with alternating AB/BA order and one diagnostic warmup per version. Six CPU workers. The previous runtime includes batching and synchronization elision. Both runtimes use the same native compiler flags (`{optimization} -g -fopenmp -pthread`), generated program objects, inputs, and timing interposers. No runtime implementation changed during the campaign.','',
        'Ratios above 1 mean the new runtime is faster. Each case uses arithmetic mean time; aggregate ratios are geometric means. Compute-region time includes any GPU initialization, preparation, transfers and synchronization performed inside that region; it is not isolated kernel time. Whole-process time also includes process startup, input loading and cleanup.','',
        'The saved CPU and saved batched GPU comparisons are historical and unpaired. The CPU was not rebenchmarked. The GPU also serves the Windows desktop; five-sample means and descriptive bootstrap intervals do not remove that source of variation.','',
        '## Matched pre-change runtime versus current runtime','',
        '| Algorithm | Cases using GPU | Whole-process speedup | Compute-region speedup | Compute 95% descriptive interval |',
        '|---|---:|---:|---:|---:|']
    if optimization=='-O2':
        lines[2:2]=['This is the supplementary matched `-O2` campaign. It used correctness-build flags, while the saved performance artifacts used `-O3`. Its relative matched comparison is valid, but its historical ratios mix compiler configurations. Use the [production-flag rerun](../attempt03_o3_native_io/report.md) for the primary performance result.','']
    for algorithm in ['bfs','sssp','cc','pagerank']:
        cases=[r for r in rows if r['algorithm']==algorithm];gpu=[r for r in cases if r['gpu_tasks']>0];fallback=[r for r in cases if not r['gpu_tasks']]
        s=dict(cases=len(cases),gpu_cases=len(gpu),cpu_fallback_cases=len(fallback))
        if gpu:
            for metric,field in [('e2e','seconds'),('compute','internal_seconds')]:
                s[metric+'_speedup']=gm(r['previous_over_current_'+metric] for r in gpu)
                s[metric+'_descriptive_95pct_interval']=interval(gpu,field)
                s[metric+'_wins']=sum(r['previous_over_current_'+metric]>1 for r in gpu)
                s['saved_gpu_over_current_'+metric]=gm(r['saved_gpu_over_current_'+metric] for r in gpu)
                s['saved_cpu_over_current_'+metric]=gm(r['saved_cpu_over_current_'+metric] for r in gpu)
            lo,hi=s['compute_descriptive_95pct_interval']
            lines.append(f"| {algorithm} | {len(gpu)} | {s['e2e_speedup']:.3f}× | {s['compute_speedup']:.3f}× | {lo:.3f}–{hi:.3f}× |")
        else:lines.append(f'| {algorithm} | 0 | CPU fallback | CPU fallback | — |')
        if fallback:
            s['fallback_compute_speedup']=gm(r['previous_over_current_compute'] for r in fallback)
            s['fallback_e2e_speedup']=gm(r['previous_over_current_e2e'] for r in fallback)
            s['fallback_saved_cpu_over_current_compute']=gm(r['saved_cpu_over_current_compute'] for r in fallback)
        summary[algorithm]=s
    gpu=[r for r in rows if r['gpu_tasks']>0]
    summary['all_gpu']=dict(cases=len(gpu),e2e_speedup=gm(r['previous_over_current_e2e'] for r in gpu),
        compute_speedup=gm(r['previous_over_current_compute'] for r in gpu),
        saved_gpu_over_current_e2e=gm(r['saved_gpu_over_current_e2e'] for r in gpu),
        saved_gpu_over_current_compute=gm(r['saved_gpu_over_current_compute'] for r in gpu),
        saved_cpu_over_current_e2e=gm(r['saved_cpu_over_current_e2e'] for r in gpu),
        saved_cpu_over_current_compute=gm(r['saved_cpu_over_current_compute'] for r in gpu))
    total=summary['all_gpu']
    lines.append(f"| All GPU-executing cases | {len(gpu)} | {total['e2e_speedup']:.3f}× | {total['compute_speedup']:.3f}× | — |")
    lines+=['','SSSP’s saved fixtures use the existing CPU fallback. GPU execution counts match the correctness corpus and the pre-change runtime for every case. CPU fallback cases are excluded from the GPU aggregates above.','',
        '## Historical comparisons for cases using the GPU','',
        '| Algorithm | Saved GPU / current, process | Saved GPU / current, compute | Saved CPU / current, process | Saved CPU / current, compute |',
        '|---|---:|---:|---:|---:|']
    for alg,s in summary.items():
        if alg=='all_gpu' or not s['gpu_cases']:continue
        lines.append(f"| {alg} | {s['saved_gpu_over_current_e2e']:.3f}× | {s['saved_gpu_over_current_compute']:.3f}× | {s['saved_cpu_over_current_e2e']:.4f}× | {s['saved_cpu_over_current_compute']:.4f}× |")
    lines+=['',f"Across the {len(gpu)} GPU-executing cases, the new runtime is {total['saved_gpu_over_current_compute']:.3f}× faster in compute time and {total['saved_gpu_over_current_e2e']:.3f}× faster in whole-process time than the saved batched-GPU measurements. Its compute-region time is still {1/total['saved_cpu_over_current_compute']:.1f}× the saved CPU time; whole-process time is {1/total['saved_cpu_over_current_e2e']:.1f}× the saved CPU time. These are historical, unpaired geometric-mean comparisons."]
    lines+=['','## CPU fallback cases','',
        '| Algorithm | Fallback cases | Previous / current, process | Previous / current, compute | Saved CPU / current, compute |',
        '|---|---:|---:|---:|---:|']
    for alg in ['bfs','sssp','cc','pagerank']:
        s=summary[alg]
        if s['cpu_fallback_cases']:
            lines.append(f"| {alg} | {s['cpu_fallback_cases']} | {s['fallback_e2e_speedup']:.3f}× | {s['fallback_compute_speedup']:.3f}× | {s['fallback_saved_cpu_over_current_compute']:.3f}× |")
    lines+=['','## Selected same-binary controls','',
        'These five-sample controls disable all four switches, or one switch at a time, in the current binary. Ratios are the disabled mode’s compute time divided by all-on compute time. They cover selected cases and interactions; they are not additive contributions or a full-corpus attribution. Unconditional preparation cleanup remains enabled in every same-binary mode; the separate pre-change binary captures its effect.','',
        '| Case | All off / on | Records off / on | Early gate off / on | Transfers off / on | Capacity off / on |',
        '|---|---:|---:|---:|---:|---:|']
    for r in controls:
        x=r['compute_off_over_all_on'];modes=['all_off']+[f+'_off' for f in FLAGS]
        lines.append('| '+r['algorithm']+', '+r['graph']+' | '+' | '.join(f'{x[m]:.3f}×' for m in modes)+' |')
    lines+=['','## Concrete selected timings','',
        '| Case | GPU tasks in warmup | Previous process ms | Current process ms | Previous compute ms | Current compute ms | Saved CPU compute ms |',
        '|---|---:|---:|---:|---:|---:|---:|']
    selected={(r['algorithm'],r['graph']) for r in controls}|{
        ('bfs','dimacs10/inf-luxembourg_osm.edges'),('cc','dimacs10/inf-luxembourg_osm.edges')}
    for r in rows:
        if (r['algorithm'],r['graph']) in selected:
            lines.append('| '+r['algorithm']+', '+r['graph']+' | '+str(r['gpu_tasks'])+' | '+' | '.join(f'{r[k]:.3f}' for k in ['previous_e2e_ms','current_e2e_ms','previous_compute_ms','current_compute_ms','saved_cpu_compute_ms'])+' |')
    saved_sync_path=REPO/'bench_results/gpu_sync_20261104/attempt02/controls.json'
    saved_sync={(r['algorithm'],r['graph']):r for r in json.loads(saved_sync_path.read_text())}
    sync_comparison=[]
    lines+=['','## More recent saved synchronization-elision controls','',
        'These four saved measurements used synchronization elision enabled. They are historical, unpaired, and used the previously frozen runtime build; the fresh matched comparison above is the basis for attributing the four new changes.','',
        '| Case | Saved sync process ms | Current process ms | Saved sync compute ms | Current compute ms |',
        '|---|---:|---:|---:|---:|']
    for r in rows:
        key=(r['algorithm'],r['graph'])
        if key not in saved_sync:continue
        old=saved_sync[key]
        a=old['seconds_means']['1']*1000;b=old['internal_seconds_means']['1']*1000
        sync_comparison.append(dict(algorithm=key[0],graph=key[1],saved_sync_e2e_ms=a,saved_sync_compute_ms=b,
            current_e2e_ms=r['current_e2e_ms'],current_compute_ms=r['current_compute_ms'],
            saved_sync_over_current_e2e=a/r['current_e2e_ms'],saved_sync_over_current_compute=b/r['current_compute_ms']))
        lines.append(f"| {key[0]}, {key[1]} | {a:.3f} | {r['current_e2e_ms']:.3f} | {b:.3f} | {r['current_compute_ms']:.3f} |")
    lines+=['','## GPU utilization observation','',
        'The runtime can alternate GPU steps with CPU fallback steps inside one algorithm. During the earlier exploratory `-O2` geo1k_50k CC warmup, the trace showed five completed GPU tasks followed by an existing-threshold CPU fallback for 1,511 records, below the 4,096 minimum. A contemporaneous one-time `nvidia-smi` query reported 3% utilization. This is not an average for the primary run or a GPU kernel-time measurement. CPU preparation, driver work, transfers, polling and CPU fallback remain part of execution. The four changes do not change offload selection or increase the work inside the GPU kernels.']
    audit=json.loads((OUT/'baseline_audit.json').read_text())
    if audit.get('workspace_report_writes_during_sampling') is False:
        lines+=['','## Primary-run method','',
            'The primary run uses the original frozen pre-change `-O3` runtime objects and a correctness-checked new `-O3` runtime. Compiler settings are checked from both objects’ DWARF metadata. Generated program objects, including GPU kernel code, are identical between the two versions. All per-sample progress files and intermediate reports remain in the native Linux benchmark directory. The completed results are copied to the watched workspace after sampling ends.','',
            'The initial matched `-O2` campaign is supplementary because it used validation-build flags rather than the saved performance flags. The partial workspace-I/O `-O3` campaign was superseded to keep file watching out of the sampling loop. Neither contributes samples to the primary results. Their raw measurements remain preserved separately. A separate 20-sample rerun of the untouched saved executables is in [saved-binary calibration](../attempt01/saved_binary_calibration.json); those samples are outside the primary 2,560 timed executions.']
    lines+=['','## Audit','',
        '- Main campaign: 244 cases × two runtimes × five timed samples = 2,440 timed executions.',
        '- Selected controls: four cases × six switch modes × five timed samples = 120 timed executions.',
        '- Existing correctness audit: 488 independent output validations and 160 feature-subset CPU comparisons passed before timing.',
        '- `baseline_audit.json` verifies source isolation, identical shared native objects and historical timer identity.',
        '- `results.json` preserves protocol, executable hashes, diagnostics and all main raw samples; `controls.json` preserves selected control samples.',
        '- `comparison.csv` contains per-case arithmetic mean times and ratios.',
        f'- Native binaries, compiler output and stdout/stderr are under `/home/kazi/p1_gpu_four_benchmark_20261104/{OUT.name}/`.',
        '- Speedup means an elapsed-time ratio; no kernel-only throughput measurement or new profiling campaign was performed.','']
    recovery=OUT/'recovery.json'
    if recovery.exists():
        lines+=['A Windows sharing lock interrupted a status-file save before a timed launch. The final control reused 23 valid raw samples and collected the seven missing samples. No successful timed sample was rerun or discarded. `recovery.json` records the recovery.','']
    analysis=dict(summary=summary,main_timed_samples=2440,control_timed_samples=120,saved_sync_comparison=sync_comparison,
        audit_sha256={n:sha(OUT/n) for n in ['results.json','controls.json','baseline_audit.json','status.json']})
    analysis['audit_sha256']['saved_sync_controls']=sha(saved_sync_path)
    if recovery.exists():analysis['audit_sha256']['recovery.json']=sha(recovery)
    (OUT/'analysis.json').write_text(json.dumps(analysis,indent=2)+'\n')
    (OUT/'report.md').write_text('\n'.join(lines))
    print(json.dumps(summary,indent=2))

if __name__=='__main__':main()
