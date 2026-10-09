#!/usr/bin/env python3
"""Reproduce full-loop DOACROSS prediction validation and Matplotlib tables.

Run in Linux/WSL Ubuntu. Production files are read-only. No TDG allocator or
graph-layout autotuner is invoked. Frozen seeded/learned predictions are made
before held-out timings and compared with the same measured parallel loops.
"""
import argparse
from collections import defaultdict
import csv
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics as st
import subprocess

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
DEFAULT=ROOT/'proof/doacross_cost_worker_work_2026_10_09'
PROTECTED=['parallel_runtime.c','parallel_runtime.h','tdg_scheduler.inc','tdg_trace.inc',
           'main.cpp','parallel_loop_outline.cpp','autotuner_runtime.c','AutoTunerPass.cpp',
           'graph_frontier_lowering.cpp','roaring_bitmap.cpp','roaring_bitmap.h']
CASES=[
    dict(name='tiny_chain',label='Tiny chain',trips=257,distance=1,dependent=0,independent=0,after=0,streams=1),
    dict(name='short_chain',label='Short chain',trips=20000,distance=1,dependent=0,independent=0,after=0,streams=1),
    dict(name='short_four_chains',label='Short, four chains',trips=20000,distance=4,dependent=0,independent=0,after=0,streams=1),
    dict(name='short_many_chains',label='Short, 64 chains',trips=20000,distance=64,dependent=0,independent=0,after=0,streams=1),
    dict(name='long_chain',label='Long chain',trips=200000,distance=1,dependent=0,independent=0,after=0,streams=1),
    dict(name='long_four_chains',label='Long, four chains',trips=200000,distance=4,dependent=0,independent=0,after=0,streams=1),
    dict(name='long_many_chains',label='Long, 64 chains',trips=200000,distance=64,dependent=0,independent=0,after=0,streams=1),
    dict(name='dependent_compute',label='Compute in dependency',trips=32768,distance=1,dependent=16,independent=0,after=0,streams=1),
    dict(name='independent_before',label='Compute before wait',trips=32768,distance=4,dependent=0,independent=16,after=0,streams=1),
    dict(name='independent_after',label='Compute after post',trips=32768,distance=4,dependent=0,independent=16,after=1,streams=1),
    dict(name='two_streams',label='Two dependent streams',trips=32768,distance=4,dependent=0,independent=0,after=0,streams=2),
    dict(name='uneven',label='Uneven lanes + compute',trips=8193,distance=3,dependent=4,independent=8,after=0,streams=1),
    dict(name='large_arrays',label='Large arrays, 64 chains',trips=1000000,distance=64,dependent=0,independent=0,after=0,streams=1),
]
FIELDS=['stage','sample','threads','trips','distance','dependent_steps','independent_steps','after_post','streams',
        'prediction_ns','actual_ns','c_dep_ns','c_ind_ns','sigma_wait_ns','sigma_post_ns','launch_ns','sync_samples']


def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()


def command(args,**kw):
    return subprocess.run(list(map(str,args)),cwd=ROOT,check=True,**kw)


def write_csv(path,rows):
    with path.open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)


def build(out,sanitize=False):
    source=out/'source';source.mkdir(exist_ok=True)
    hashes={name:sha(ROOT/name) for name in PROTECTED}
    for name in PROTECTED: shutil.copy2(ROOT/name,source/name)
    shutil.copy2(HERE/'probe.c',source/'probe.c')
    hashes['test_probe.c']=sha(HERE/'probe.c')
    (out/'source_manifest.json').write_text(json.dumps(hashes,indent=2)+'\n')
    build=out/'build';build.mkdir(exist_ok=True)
    flags=['-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined'] if sanitize else ['-O3']
    suffix='sanitized' if sanitize else 'probe'
    with (out/(suffix+'.build.log')).open('w') as log:
        command(['gcc',*flags,'-mavx2','-pthread','-fopenmp','-I',source,'-I',ROOT,
                 f'-DDOACROSS_RUNTIME_SOURCE="{source/"parallel_runtime.c"}"','-c',source/'probe.c',
                 '-o',build/(suffix+'.o')],stdout=log,stderr=subprocess.STDOUT)
        command(['g++','-O3','-mavx2','-pthread','-fopenmp','-I',ROOT,'-c',source/'roaring_bitmap.cpp',
                 '-o',build/'bitmap.o'],stdout=log,stderr=subprocess.STDOUT)
        command(['g++',*flags,'-pthread','-fopenmp',build/(suffix+'.o'),build/'bitmap.o',
                 '-lnlopt','-lm','-o',build/suffix],stdout=log,stderr=subprocess.STDOUT)
    return build/suffix,hashes


def env(threads,sanitize=False):
    result={k:v for k,v in os.environ.items() if not k.startswith(
        ('SGPL_','GRAPH_PARALLEL','OMP_'))}
    result.update(SGPL_NUM_THREADS=str(threads),OMP_NUM_THREADS=str(threads))
    if sanitize:
        result.update(ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    return result


def arguments(case,samples):
    return [case[k] for k in ('trips','distance','dependent','independent','after','streams')]+[samples]


def aggregate(raw):
    grouped=defaultdict(lambda:defaultdict(list))
    for r in raw: grouped[r['case'],int(r['threads']),r['stage']][int(r['repeat'])].append(r)
    summary=[]
    for (case,p,stage),processes in sorted(grouped.items()):
        predicted=st.median(st.median(float(r['prediction_ns']) for r in rows) for rows in processes.values())/1e6
        actual=st.median(st.median(int(r['actual_ns']) for r in rows) for rows in processes.values())/1e6
        error=100*(predicted/actual-1)
        sample=next(iter(processes.values()))[0]
        summary.append(dict(case=case,threads=p,stage=stage,trips=int(sample['trips']),distance=int(sample['distance']),
            streams=int(sample['streams']),dependent_steps=int(sample['dependent_steps']),
            independent_steps=int(sample['independent_steps']),after_post=int(sample['after_post']),
            predicted_ms=predicted,measured_ms=actual,extra_predicted_ms=predicted-actual,
            signed_error_percent=error,absolute_error_percent=abs(error),
            process_min_ms=min(st.median(int(r['actual_ns']) for r in rows)/1e6 for rows in processes.values()),
            process_max_ms=max(st.median(int(r['actual_ns']) for r in rows)/1e6 for rows in processes.values()),
            processes=len(processes),samples_per_process=len(next(iter(processes.values()))),validation_status='PASS'))
    return summary


def add_failures(summary,failures):
    failed={(r['case'],int(r['threads'])) for r in failures}
    summary=[r for r in summary if (r['case'],r['threads']) not in failed]
    assert summary
    fields=list(summary[0])
    for name,p in sorted(failed):
        case=next(c for c in CASES if c['name']==name)
        for stage in ('seeded-sync','learned-sync'):
            record={key:None for key in fields}
            record.update(case=name,threads=p,stage=stage,trips=case['trips'],distance=case['distance'],
                streams=case['streams'],dependent_steps=case['dependent'],independent_steps=case['independent'],
                after_post=case['after'],processes=0,samples_per_process=0,validation_status='FAILED')
            summary.append(record)
    return summary


def render(rows,stage,p,model_label):
    height=2.0+.43*(len(rows)+1)
    fig=plt.figure(figsize=(15,height),facecolor='white')
    name='Learned synchronization costs' if stage=='learned-sync' else 'Seeded synchronization costs'
    fig.text(.03,1-.40/height,f'DOACROSS cost-model accuracy — {p} workers',fontsize=17,weight='bold',color='#16354a')
    fig.text(.03,1-.72/height,name+'; full parallel loop, including launch and real wait/post dependencies.',
             fontsize=10.5,color='#506676')
    ax=fig.add_axes([.03,1.02/height,.94,1-1.98/height]);ax.axis('off')
    labels={c['name']:c['label'] for c in CASES}
    columns=['Workload','Iterations\nN','Dependency\ndistance','Sync\nstreams','Predicted\ntime (ms)',
             'Measured\ntime (ms)','Signed\nerror (%)','Absolute\nerror (%)']
    cells=[]
    for r in rows:
        if r['validation_status']=='FAILED':
            cells.append([labels[r['case']]+' †',f'{r["trips"]:,}',str(r['distance']),str(r['streams']),
                          'n.a.','FAILED','n.a.','n.a.'])
        else:
            cells.append([labels[r['case']],f'{r["trips"]:,}',str(r['distance']),str(r['streams']),f'{r["predicted_ms"]:.4f}',
                          f'{r["measured_ms"]:.4f}',f'{r["signed_error_percent"]:+.1f}%',f'{r["absolute_error_percent"]:.1f}%'])
    table=ax.table(cellText=cells,colLabels=columns,cellLoc='center',
        colWidths=[.25,.10,.10,.075,.12,.12,.12,.115],bbox=[0,0,1,1])
    table.auto_set_font_size(False);table.set_fontsize(10.5)
    for (row,col),cell in table.get_celld().items():
        cell.set_edgecolor('#d7e1e8');cell.set_linewidth(.5)
        if row==0: cell.set_facecolor('#16354a');cell.set_text_props(color='white',weight='bold',fontsize=10)
        else:
            cell.set_facecolor('#f0f4f7' if row%2==0 else 'white');cell.set_text_props(color='#20364a')
            if col==0: cell.set_text_props(ha='left');cell.PAD=.035
            if col==7:
                error=rows[row-1]['absolute_error_percent']
                if error is None:
                    cell.set_facecolor('#fbe4e2');cell.set_text_props(color='#9f2f26',weight='bold');continue
                fill,color=('#e1f1e8','#17644a') if error<=20 else (
                    ('#fff0d6','#82570d') if error<=30 else ('#fbe4e2','#9f2f26'))
                cell.set_facecolor(fill);cell.set_text_props(color=color,weight='bold')
    notes=[
        'Signed error = 100 × (predicted / measured − 1). Negative = underprediction. Absolute error is its magnitude.',
        'Times are medians of process medians. Predictions frozen before holdout. Model: '+model_label+'.',
        'Direct C probes use production model/dispatch/profiling APIs. † = failed output check or timeout; excluded from accuracy statistics.',
    ]
    for y,note in zip((.74,.48,.22),notes): fig.text(.03,y/height,note,fontsize=9,color='#506676')
    return fig


def export(out,summary):
    snapshot=(out/'source/parallel_runtime.c').read_text()
    updated='SGPL_DOACROSS_WIDTH_PROFILES' in snapshot
    worker_work='state->c_dep_ns_per_iter_ewma + state->c_ind_ns_per_iter_ewma + sync_per_iter' in snapshot
    model_label=('per-lane computation and waits; width-specific profiles' if worker_work else
                 'per-lane synchronization; width-specific batch profiles' if updated else 'original N × synchronization')
    write_csv(out/'summary.csv',summary)
    table_dir=out/'tables';table_dir.mkdir(exist_ok=True)
    metrics={}
    with PdfPages(table_dir/'doacross_cost_accuracy.pdf') as pdf:
        for stage in ('learned-sync','seeded-sync'):
            rows=[r for r in summary if r['stage']==stage]
            valid=[r for r in rows if r['validation_status']=='PASS']
            metrics[stage]=dict(settings=len(rows),valid_settings=len(valid),failed_settings=len(rows)-len(valid),
                median_absolute_error_percent=st.median(r['absolute_error_percent'] for r in valid),
                mean_absolute_error_percent=st.mean(r['absolute_error_percent'] for r in valid),
                median_signed_error_percent=st.median(r['signed_error_percent'] for r in valid),
                within_10_percent=sum(r['absolute_error_percent']<=10 for r in valid),
                within_20_percent=sum(r['absolute_error_percent']<=20 for r in valid),
                within_30_percent=sum(r['absolute_error_percent']<=30 for r in valid))
            for p in sorted({r['threads'] for r in rows}):
                selected=[r for r in rows if r['threads']==p]
                order={c['name']:i for i,c in enumerate(CASES)}
                selected.sort(key=lambda r:order[r['case']])
                fig=render(selected,stage,p,model_label);stem=f'{stage}.P{p}'
                fig.savefig(table_dir/(stem+'.png'),dpi=160);fig.savefig(table_dir/(stem+'.svg'));pdf.savefig(fig);plt.close(fig)
    (out/'accuracy.json').write_text(json.dumps(metrics,indent=2)+'\n')
    by_width=[]
    for stage in ('learned-sync','seeded-sync'):
        for p in sorted({r['threads'] for r in summary}):
            valid=[r for r in summary if r['stage']==stage and r['threads']==p and r['validation_status']=='PASS']
            by_width.append(dict(stage=stage,threads=p,valid_settings=len(valid),
                median_absolute_error_percent=st.median(r['absolute_error_percent'] for r in valid),
                mean_absolute_error_percent=st.mean(r['absolute_error_percent'] for r in valid),
                within_20_percent=sum(r['absolute_error_percent']<=20 for r in valid)))
    write_csv(out/'accuracy_by_threads.csv',by_width)
    lines=['DOACROSS parallel cost-model validation',
        'Prediction is the exact helper in the frozen runtime; sources unchanged during the run.',
        'Model: '+model_label+'.',
        'Direct C recurrences with actual wait/post dependencies and production profiling, not SGPL compiler integration.',
        'Three fresh processes and seven held-out parallel timings per completed setting in the default run.',
        'Both forecasts are frozen before held-out timing; seeded and learned forecasts share actual timings.',
        'Signed error = 100*(predicted/measured-1); aggregate errors weight each completed setting equally.',
        '', 'Synchronization input       Valid  Failed  Median absolute error  Mean absolute error  Within 20%']
    for stage in ('learned-sync','seeded-sync'):
        m=metrics[stage]
        lines.append(f'{stage:27} {m["valid_settings"]:5} {m["failed_settings"]:7} '
                     f'{m["median_absolute_error_percent"]:20.2f}% {m["mean_absolute_error_percent"]:19.2f}% '
                     f'{m["within_20_percent"]:5}/{m["valid_settings"]}')
    lines += ['', 'Learned sync costs, by worker width:', 'Workers  Valid  Median absolute error  Mean absolute error  Within 20%']
    for row in by_width:
        if row['stage']=='learned-sync':
            lines.append(f'{row["threads"]:7} {row["valid_settings"]:6} '
                         f'{row["median_absolute_error_percent"]:20.2f}% {row["mean_absolute_error_percent"]:19.2f}% '
                         f'{row["within_20_percent"]:5}/{row["valid_settings"]}')
    valid=[r for r in summary if r['stage']=='learned-sync' and r['validation_status']=='PASS']
    lines += ['', 'Largest learned-cost errors:', 'Case                         P  Predicted ms  Measured ms  Signed error']
    for r in sorted(valid,key=lambda r:r['absolute_error_percent'],reverse=True)[:6]:
        lines.append(f'{r["case"]:28} {r["threads"]:2} {r["predicted_ms"]:12.4f} {r["measured_ms"]:12.4f} '
                     f'{r["signed_error_percent"]:+11.2f}%')
    failed=[r for r in summary if r['stage']=='learned-sync' and r['validation_status']=='FAILED']
    if failed:
        lines += ['', 'Failed configurations (excluded from accuracy metrics; see failures.csv and traces):']
        lines.extend(f'{r["case"]}, workers={r["threads"]}, N={r["trips"]}, distance={r["distance"]}' for r in failed)
    if worker_work:
        lines += ['', 'Dependent and independent useful work plus measured synchronization are charged per cyclic worker.',
            'There is no separate global dependent-work/critical-path charge. Blocking waits include dependency delays.',
            'Matched-width learned inputs are measured before holdout; unseen widths still use synchronization seeds.',
            'Residual errors can include serial-to-parallel work-cost differences, profiling overhead and uneven lanes.',
            'No machine-specific constants were fitted. A runtime timeout is excluded from prediction statistics.']
    elif updated:
        lines += ['', 'Remaining error concentrates in many-chain workloads at larger widths.',
            'The N*c_dep approximation still treats all dependent work as serial, even when chains overlap.',
            'The profiling change also changes measured execution overhead, so historical equation-only',
            'ablations do not predict these fresh errors exactly. No machine-specific constants were fitted.',
            'A timeout is a runtime correctness failure, not a prediction error; failed settings are excluded.']
    else:
        lines += ['', 'These measurements establish poor accuracy across this suite, particularly for dependent chains at larger widths.',
        'Separate training measures synchronization blocking as well as service. The existing equation multiplies',
        'its wait/post estimates by N; this can accumulate overlapping lane waits as elapsed time. That is a',
        'mechanism consistent with the width-dependent overpredictions, not an isolated causal experiment.']
    lines += ['', 'Reproduce timings and tables: python3 test/doacross_cost_validation/run.py',
        'Regenerate tables: python3 test/doacross_cost_validation/run.py --render-only',
        'Protocol and scope: test/doacross_cost_validation/README.txt']
    (out/'report.txt').write_text('\n'.join(lines)+'\n')
    print(json.dumps(metrics,indent=2),flush=True)
    return metrics


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out',type=Path,default=DEFAULT)
    parser.add_argument('--threads',nargs='+',type=int,default=[2,3,4,5,6,7,8])
    parser.add_argument('--repeats',type=int,default=3)
    parser.add_argument('--samples',type=int,default=7)
    parser.add_argument('--render-only',action='store_true')
    parser.add_argument('--sanitizers',action='store_true')
    args=parser.parse_args();out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    if args.render_only:
        with (out/'raw.csv').open() as f: summary=aggregate(list(csv.DictReader(f)))
        failures=[]
        if (out/'failures.csv').exists():
            with (out/'failures.csv').open() as f: failures=list(csv.DictReader(f))
        summary=add_failures(summary,failures)
        metrics=export(out,summary)
        if (out/'manifest.json').exists():
            manifest=json.loads((out/'manifest.json').read_text())
            manifest['metrics']=metrics
            manifest['valid_heldout_timings']=sum(r['processes']*r['samples_per_process'] for r in summary if r['stage']=='learned-sync')
            (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        return
    binary,hashes=build(out,args.sanitizers)
    command(['lscpu'],stdout=(out/'environment.txt').open('w'))
    if args.sanitizers:
        checks=[]
        for name in ('tiny_chain','two_streams','uneven'):
            case=next(c for c in CASES if c['name']==name)
            result=command([binary,*arguments(case,3)],env=env(4,True),capture_output=True,text=True,timeout=120)
            checks.append(name+': '+result.stderr)
        (out/'sanitizer_validation.txt').write_text('\n'.join(checks));print('PASS ASan/UBSan representative dependent kernels')
        return
    tasks=[(case,p,repeat) for case in CASES for p in args.threads for repeat in range(args.repeats)]
    random.Random(20261009).shuffle(tasks)
    traces=out/'traces';traces.mkdir(exist_ok=True);rows=[];failures=[];failed_settings=set()
    with (out/'raw.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=['case','repeat',*FIELDS]);writer.writeheader()
        for number,(case,p,repeat) in enumerate(tasks,1):
            if (case['name'],p) in failed_settings: continue
            try:
                result=command([binary,*arguments(case,args.samples)],env=env(p),capture_output=True,text=True,timeout=30)
            except (subprocess.CalledProcessError,subprocess.TimeoutExpired) as failure:
                stdout=failure.stdout or '';stderr=failure.stderr or ''
                if isinstance(stdout,bytes): stdout=stdout.decode(errors='replace')
                if isinstance(stderr,bytes): stderr=stderr.decode(errors='replace')
                (traces/f'{case["name"]}.P{p}.r{repeat}.failure.txt').write_text(stdout+'\n'+stderr+'\n'+str(failure))
                failures.append(dict(case=case['name'],threads=p,repeat=repeat,reason=str(failure)))
                failed_settings.add((case['name'],p))
                print(f'FAILED {case["name"]} P={p}: {failure}',flush=True)
                continue
            assert 'PASS full-output' in result.stderr and 'no_TDG_plans' in result.stderr
            (traces/f'{case["name"]}.P{p}.r{repeat}.txt').write_text(result.stderr)
            parsed=list(csv.reader(result.stdout.splitlines()));assert len(parsed)==2*args.samples
            for values in parsed:
                assert len(values)==len(FIELDS)
                row=dict(case=case['name'],repeat=repeat,**dict(zip(FIELDS,values)));writer.writerow(row);rows.append(row)
            f.flush()
            print(f'{number}/{len(tasks)} {case["name"]} P={p} repeat={repeat}',flush=True)
    unchanged={name:sha(ROOT/name)==value for name,value in hashes.items() if name!='test_probe.c'}
    assert all(unchanged.values()),unchanged
    (out/'scope_validation.json').write_text(json.dumps(unchanged,indent=2)+'\n')
    if failures: write_csv(out/'failures.csv',failures)
    else: (out/'failures.csv').write_text('case,threads,repeat,reason\n')
    complete=[r for r in rows if (r['case'],int(r['threads'])) not in failed_settings]
    summary=add_failures(aggregate(complete),failures);metrics=export(out,summary)
    manifest=dict(cases=CASES,threads=args.threads,repeats=args.repeats,samples=args.samples,
        physical_heldout_timings=len(rows)//2,forecast_rows=len(rows),
        method='8 serial profile runs; seeded forecast; 4 forced parallel calibration runs; frozen learned forecast; held-out parallel timings.',
        actual='Public parallel_for_runtime, real wait/post dependencies, per-iteration production profiling wrapper; setup and full-output checks outside timer.',
        prediction='Exact sgpl_loop_parallel_model_time_ns production helper; same equation as per-loop chooser parallel_rhs.',
        equation='L(P) + ceil(N/P)*(c_dep + c_ind + waits*sigma_wait(P) + posts*sigma_post(P))',
        sync_learning='Worker-local counts/totals merged after join; per-width and trip-regime EWMA, frozen before holdout.',
        limits='Handwritten C kernels with compiler-like profiling, not SGPL compiler integration or uninstrumented application-time validation. Sync inputs are frozen before holdout.',
        failed_settings=failures,excluded_partial_timings=len(rows)-len(complete),
        valid_heldout_timings=len(complete)//2,source_sha256=hashes,metrics=metrics)
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')


if __name__=='__main__': main()
