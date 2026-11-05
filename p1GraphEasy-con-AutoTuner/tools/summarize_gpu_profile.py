#!/usr/bin/env python3
"""Render the measured profile and its existing-model optimization priorities."""
from pathlib import Path
import csv,json,statistics
import benchmark_gpu_saved_cpu as B
OUT=B.REPO/'bench_results/gpu_profile_20261104'
rows=json.loads((OUT/'run03/results.json').read_text())
drill=json.loads((OUT/'prep04/results.json').read_text())
groups={
 'CUDA initialization':['cuda_init'],
 'CPU preparation/finalize':['cpu_preparation','cpu_finalize'],
 'Transfer calls':['host_to_device','device_to_host','status_download'],
 'Staging/cache':['upload_staging','download_publication','resident_cache','host_coherence'],
 'Allocation/streams':['device_allocation','device_free','stream_management'],
 'Submission/completion':['execute_wait_and_scheduler','poll_cpu','stream_query','explicit_sync','launch_api'],
 'Other host/runtime':['other_host','plan_and_descriptors','cleanup','cuda_module_and_lookup','context_set','profiler_events']}
labels=['PR / hamming8-2','PR / Slashdot','BFS / Slashdot','CC / fe-4elt2','CC / Luxembourg','SSSP / Slashdot (CPU)']
summaries=[]
for row,label in zip(rows,labels):
    baseline=[s for s in row['samples'] if s['mode']=='baseline']
    profile=[s for s in row['samples'] if s['mode']=='profile']
    p=[s['profile'] for s in profile]
    assert set(sum(groups.values(),[]))==set(p[0]['buckets'])
    means={name:{metric:statistics.mean(s['buckets'][name][metric] for s in p)
                 for metric in ['main_ms','compute_ms','calls','compute_calls','bytes','compute_bytes']}
           for name in p[0]['buckets']}
    grouped={name:{metric:sum(means[b][metric] for b in members)
                  for metric in ['main_ms','compute_ms']} for name,members in groups.items()}
    entry=dict(algorithm=row['algorithm'],graph=row['graph'],label=label,
       baseline_process_ms=statistics.mean(s['seconds']*1000 for s in baseline),
       baseline_compute_ms=statistics.mean(s['compute_ms'] for s in baseline),
       profile_process_ms=statistics.mean(s['seconds']*1000 for s in profile),
       profile_main_ms=statistics.mean(s['main_ms'] for s in p),
       profile_compute_ms=statistics.mean(s['compute_ms'] for s in p),buckets=means,groups=grouped,
       gpu_event_compute_ms=row['events']['event_compute_ms'],gpu_event_all_ms=row['events']['event_ms'],
       gpu_launches=row['events']['launches'])
    entry['outside_main_ms']=entry['profile_process_ms']-entry['profile_main_ms']
    entry['profile_compute_delta_pct']=100*(entry['profile_compute_ms']/entry['baseline_compute_ms']-1)
    assert abs(sum(g['compute_ms'] for g in grouped.values())-entry['profile_compute_ms'])<.001
    assert row['validation_passed'] and row['events']['event_errors']==0
    summaries.append(entry)
B.save(OUT/'timing_summary.json',summaries)
with (OUT/'timing_breakdown.csv').open('w',newline='') as f:
    columns=['algorithm','graph','baseline_process_ms','baseline_compute_ms','profile_process_ms','profile_main_ms','profile_compute_ms','outside_main_ms','profile_compute_delta_pct','gpu_event_compute_ms']
    writer=csv.DictWriter(f,columns+list(groups),extrasaction='ignore');writer.writeheader()
    for x in summaries:writer.writerow(dict(x,**{name:v['compute_ms'] for name,v in x['groups'].items()}))
with (OUT/'detailed_buckets.csv').open('w',newline='') as f:
    writer=csv.writer(f);writer.writerow(['algorithm','graph','bucket','main_ms','compute_ms','calls','compute_calls','bytes','compute_bytes'])
    for x in summaries:
        for name,v in x['buckets'].items():writer.writerow([x['algorithm'],x['graph'],name,*[v[k] for k in ['main_ms','compute_ms','calls','compute_calls','bytes','compute_bytes']]])
lines=['Current GPU execution: measured timing breakdown, 2026-11-04','',
 'Six selected cases, three alternating baseline/profile repetitions each. All six profiled validation programs passed independent output checks.',
 'Instrumentation exists only in isolated profiling builds. Production execution/model sources and GPU objects were not changed.',
 'Host buckets are exclusive main-thread CLOCK_MONOTONIC wall times; phase times include waits for CPU workers. They add to main/compute duration.',
 'CUDA API time includes blocking within that API; it is not a pure bandwidth or hardware execution measurement.',
 'GPU event intervals come from one separate diagnostic run and overlap host buckets: do not add them to the CPU wall-time stack.',
 'Event intervals include stream scheduling and host-submission gaps. Nsight Systems was not installed.',
 'Outside-main time includes loader/exit and report I/O; it was not attributed specifically to CUDA teardown.',
 'Only six representative cases were profiled; these are descriptive results, not a full-corpus performance/significance claim. The GPU serves the Windows desktop.','']
for x in summaries:
    lines += [x['label'],
      '  baseline process %.3f ms; baseline compute %.3f ms; profiled compute %.3f ms (delta %+.2f%%)'%(x['baseline_process_ms'],x['baseline_compute_ms'],x['profile_compute_ms'],x['profile_compute_delta_pct']),
      '  separately measured device stream interval %.3f ms within compute; launches %d'%(x['gpu_event_compute_ms'],x['gpu_launches'])]
    for name,v in x['groups'].items():lines.append('  %-27s %10.3f ms (%5.1f%%)'%(name,v['compute_ms'],100*v['compute_ms']/x['profile_compute_ms']))
lines += ['', 'CPU preparation drill-down: one separate profiled observation per case, exclusive named preparation functions. It is not an average of the three-repetition campaign.']
for row in drill:
    p=next(s['profile'] for s in row['samples'] if s['mode']=='profile')
    lines.append(row['algorithm']+' / '+row['graph'])
    for name,v in p['buckets'].items():
        if name.startswith('prep_'):lines.append('  %-27s %10.3f ms; entries %d'%(name,v['compute_ms'],v['compute_calls']))
lines += ['', 'Strategies constrained to existing model and proofs (proposals only):',
 '1. Prepared-record realization: sgpl_gpu_prepare_spatial_records is the principal CPU preparation cost (about 244 ms on PR Slashdot and 6.30 s on CC Luxembourg in the drill-down).',
 '   Reuse invariant full-domain records only under existing stable topology/layout, traversal/owner identity and valid lifetime guarantees. Use current invalidation and exclusive scratch leases.',
 '   CC has a changing frontier: do not reuse its previous active records. Preserve multiplicity, occurrence identity, per-owner order and error boundaries. Reuse only existing-lifetime storage and invariant metadata; use current touched-group/epoch mechanisms where their existing contracts apply.',
 '2. Earlier existing rejection checks: the current cost threshold runs after record/descriptor preparation. Hoist only when existing exact counts or eligibility facts prove the SAME decision, including fallback. Leave uncertain cases unchanged. This is not a new cost model.',
 '3. Transfer/staging reduction: consume current effect/liveness/read-only/snapshot certificates to omit unnecessary host/device observations and copies; pack compatible existing transfers within the same transfer epoch using the existing relocation/alias contract.',
 '   Keep every host read, mutation, join and status boundary required by current proofs. Reuse existing R1/R4/R5 premises only when they actually cover the GPU path; do not infer a new license from a kernel name.',
 '4. Reuse allocation capacity through existing exclusive scratch/residency lifetimes; prioritize long CC where allocation/free is measurable. Do not introduce a new global resource pool or scheduling policy.',
 '5. Existing certified stage removal/fusion may apply only to already licensed shapes and observations. Preserve round dependence; GPU kernel micro-optimization is low priority given the small device intervals.',
 'CUDA context initialization is a cold-process external runtime cost. An execution rewrite cannot erase it. Existing CPU/GPU placement can avoid paying it for jobs the current model rejects; this proposal adds no persistent service, CUDA Graph mechanism, algorithm or model axiom.',
 '', 'Acceptance for any later implementation: same program observations, exact occurrence/order requirements, ownership, joins, cancellation/error behavior and fallback; retain a disable control and revalidate before timing.',
 'Methodological correction: run02 timed every host-access hook and more than doubled large PR compute time. Its timings were rejected. Its >11 million hook entries are a diagnostic count only. run03 omits per-access timing and shows compute deltas between %.2f%% and %+.2f%%.'%(min(x['profile_compute_delta_pct'] for x in summaries),max(x['profile_compute_delta_pct'] for x in summaries)),
 'Event timing contract: https://docs.nvidia.com/cuda/cuda-driver-api/cuda_driver_api/group__CUDA__EVENT.html']
(OUT/'report.txt').write_text('\n'.join(lines)+'\n')

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
colors=['#5969a6','#df8c32','#4c9b91','#b270a1','#8b9b5d','#5e9ac5','#a9a9a9']
fig,axes=plt.subplots(1,2,figsize=(14,6),sharey=True)
for ax,metric,title in zip(axes,['compute_ms','main_ms'],['Algorithm compute region','Whole process (including outside main)']):
    left=np.zeros(len(summaries));totals=np.array([x['profile_compute_ms'] if metric=='compute_ms' else x['profile_process_ms'] for x in summaries])
    for (name,_),color in zip(groups.items(),colors):
        values=np.array([x['groups'][name][metric] for x in summaries])/totals*100
        ax.barh(range(len(summaries)),values,left=left,color=color,label=name,height=.62)
        left+=values
    if metric=='main_ms':
        values=np.array([x['outside_main_ms'] for x in summaries])/totals*100
        ax.barh(range(len(summaries)),values,left=left,color='#d5d5d5',label='Outside main',height=.62)
    ax.set_xlim(0,100);ax.set_title(title,fontweight='bold');ax.set_xlabel('Exclusive share of profiled wall time (%)');ax.grid(axis='x',alpha=.15);ax.set_axisbelow(True)
axes[0].set_yticks(range(len(summaries)),[x['label'] for x in summaries]);axes[0].invert_yaxis()
fig.suptitle('GPU execution spends most wall time preparing and moving work',fontsize=15,fontweight='bold',y=.99)
handles,legend=axes[1].get_legend_handles_labels();fig.legend(handles,legend,loc='lower center',ncol=4,fontsize=9,bbox_to_anchor=(.53,.015))
fig.text(.5,.145,'Three profiled runs per case. Device event intervals overlap these host buckets and are shown separately in the CSV/report.',ha='center',fontsize=8,color='#444444')
fig.tight_layout(rect=(0,.18,1,.95));fig.savefig(OUT/'timing_breakdown.png',dpi=180);plt.close(fig)
print('\n'.join(lines[:9]));print('Wrote report, CSVs, summary JSON and timing_breakdown.png')
