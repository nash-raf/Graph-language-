"""Paired fresh-process comparison of early-diversity search and its exact predecessor.

Reuse saved SGPL objects and fixed exhaustive winners; never feed their timings
into the model. Compile the same test adapter against each frozen runtime.
Additional adapter counters run outside the timed execution interval.
"""
import argparse
from collections import defaultdict
import csv
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import shutil
import statistics as st
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
HERE = ROOT/'test/tdg_allocation_sweep'
sys.path.insert(0, str(HERE))
import mixed
import run as sweep

OUT = ROOT/'proof/tdg_diverse_allocations_2026_10_09'
ORIGINAL = ROOT/'proof/tdg_allocation_sweep'
PROMOTED = ROOT/'proof/tdg_dynamic_default_2026_10_09/mixed'

COUNTERS = r'''
static unsigned measured_native_calls;
static unsigned long long native_first_ns;
static unsigned long previous_learning_ns, steady_learning_ns;
static void sweep_record_native(unsigned long long elapsed_ns) {
    if (forced) return;
    unsigned long learning=atomic_load(&g_joint_learning_ns);
    if (!measured_native_calls++) native_first_ns=elapsed_ns;
    if (phase==1) steady_learning_ns += learning-previous_learning_ns;
    previous_learning_ns=learning;
}
static void sweep_report_stats(void) {
    fprintf(stderr,"SCHED_STATS,%llu,%lu,%lu,%lu,%lu,%lu,%u\n",
        native_first_ns,atomic_load(&g_joint_learning_ns),steady_learning_ns,
        atomic_load(&g_joint_plans),atomic_load(&g_joint_search_evals),
        atomic_load(&g_joint_calibration_runs),measured_native_calls);
}
'''


def workload_records():
    simple=json.loads((ORIGINAL/'manifest.json').read_text())
    oldmixed=json.loads((ORIGINAL/'mixed_manifest.json').read_text())
    promoted=json.loads((PROMOTED/'mixed_manifest.json').read_text())
    sources={False:(simple,ORIGINAL),True:(oldmixed,ORIGINAL)}
    records=[]
    summary={}
    for source in (ORIGINAL,PROMOTED):
        with (source/'summary.csv').open() as f:
            for row in csv.DictReader(f):
                if row['policy']=='dynamic': summary[row['case'],int(row['budget'])]=row
    promoted_names={c['name'] for c in promoted['cases']}
    for is_mixed,(meta,source) in sources.items():
        for original_case in meta['cases']:
            case=dict(original_case)
            budgets=case.get('budgets',meta.get('arguments',{}).get('budgets',[]))
            selected_meta=promoted if is_mixed and case['name'] in promoted_names else meta
            if selected_meta is promoted:
                case=dict(next(c for c in promoted['cases'] if c['name']==case['name']))
            record=next(r for r in selected_meta['generated'] if r['case']==case['name'])
            build=Path(record.get('build_directory',selected_meta.get('build_directory',str(source/'build'))))
            assert sweep.sha(build/(case['name']+'.o'))==record['object_sha256']
            if is_mixed: case['orders']=mixed.orders(case)
            for budget in budgets:
                records.append(dict(case=case,budget=budget,is_mixed=is_mixed,source_build=build,
                                    winner=summary[case['name'],budget]['best_config']))
    return records


def instrument(text):
    assert text.count('elapsed=sgpl_now_ns()-begin;')==1
    text=text.replace('elapsed=sgpl_now_ns()-begin;',
                      'elapsed=sgpl_now_ns()-begin;\n    sweep_record_native(elapsed);')
    if 'static double prediction;' in text:
        text=text.replace('static double prediction;','static double prediction;\n'+COUNTERS)
    end=text.rfind('    return 0;')
    assert end>=0
    return text[:end]+'    sweep_report_stats();\n'+text[end:]


def build(records):
    frozen=json.loads((OUT/'before/manifest.json').read_text())
    for name,digest in frozen.items(): assert sweep.sha(OUT/'before'/name)==digest
    source_hashes={}
    for variant in ('before','after'):
        source=OUT/variant/'source';source.mkdir(parents=True,exist_ok=True)
        origin=OUT/'before' if variant=='before' else ROOT
        for name in ('parallel_runtime.c','parallel_runtime.h','tdg_scheduler.inc','tdg_trace.inc'):
            shutil.copy2(origin/name,source/name)
        source_hashes[variant]={name:sweep.sha(source/name) for name in (
            'parallel_runtime.c','parallel_runtime.h','tdg_scheduler.inc','tdg_trace.inc')}
        runtime=(source/'parallel_runtime.c').read_text()
        needle='granted_threads = sgpl_loop_pool_try_acquire(loop_id, nthreads);'
        assert runtime.count(needle)==2
        (source/'observed_runtime.c').write_text(runtime.replace(needle,needle+'\n    sweep_grant(loop_id, granted_threads);'))
        (source/'driver.c').write_text(instrument((HERE/'driver.c').read_text()))
        (source/'mixed_driver.c').write_text(instrument((HERE/'mixed_driver.c').read_text()))
        for name in ('driver','mixed_driver'):
            obj=source/(name+'.o')
            sweep.run(['gcc','-O2','-pthread','-fopenmp','-iquote',source,'-iquote',ROOT,'-iquote',HERE,
                f'-DSWEEP_RUNTIME_SOURCE="{source/"observed_runtime.c"}"','-c',source/(name+'.c'),'-o',obj])
        binaries=OUT/variant/'bin';binaries.mkdir(exist_ok=True)
        for item in {r['case']['name']:r for r in records}.values():
            old=item['source_build'];name=item['case']['name']
            sdk=[old/(n+'.o') for n in ('runtime.c','autotuner_runtime.c','graph_mutation_runtime.c','gpu_runtime.c',
                'semiring_runtime.c','roaring_bitmap.cpp','graph_loader_runtime.cpp','graph_runtime.cpp')]
            adapter=source/('mixed_driver.o' if item['is_mixed'] else 'driver.o')
            sweep.run(['g++','-O2','-no-pie','-pthread','-fopenmp',old/(name+'.repeat.o'),adapter,*sdk,
                '-Wl,--wrap=sgpl_run_tdg_level','-Wl,--wrap=sgpl_should_parallelize_doall','-Wl,--wrap=printf',
                '-ldl','-lnlopt','-lm','-o',binaries/name])
    return source_hashes


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repeats',type=int,default=5)
    p.add_argument('--samples',type=int,default=15)
    p.add_argument('--build-only',action='store_true')
    p.add_argument('--measure-only',action='store_true')
    p.add_argument('--cases',nargs='+')
    p.add_argument('--budgets',nargs='+',type=int)
    p.add_argument('--tag',help='Save a separate confirmation run below the proof directory.')
    args=p.parse_args();records=workload_records()
    records=[r for r in records if (not args.cases or r['case']['name'] in args.cases)
             and (not args.budgets or r['budget'] in args.budgets)]
    assert records, 'No matching workloads'
    if args.tag: assert Path(args.tag).name==args.tag and args.tag not in ('.','..')
    results=OUT/args.tag if args.tag else OUT
    results.mkdir(parents=True,exist_ok=True);traces=results/'traces';traces.mkdir(exist_ok=True)
    soft,hard=resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK,(64*1024*1024 if hard<0 else min(64*1024*1024,hard),hard))
    hashes=json.loads((OUT/'sources.json').read_text()) if args.measure_only else build(records)
    for variant,files in hashes.items():
        for name,digest in files.items():
            assert sweep.sha(OUT/variant/'source'/name)==digest
    (OUT/'sources.json').write_text(json.dumps(hashes,indent=2)+'\n')
    if args.build_only: return
    rows=[];stats=[]
    for item in records:
        case=item['case'];budget=item['budget'];name=case['name']
        for repeat in range(args.repeats):
            variants=['before','after'];random.Random(f'early-diversity-{name}-{budget}-{repeat}').shuffle(variants)
            for variant in variants:
                env={k:v for k,v in os.environ.items() if not k.startswith(
                    ('SGPL_FORCE','SGPL_JOINT','SGPL_TDG','SGPL_BUDGET','GRAPH_PARALLEL'))}
                env.update(SGPL_NUM_THREADS=str(budget),OMP_NUM_THREADS=str(budget),SWEEP_SAMPLES=str(args.samples))
                data=(mixed.header(case) if item['is_mixed'] else '')+item['winner'].replace(':',' ')+'\n'
                result=sweep.run([OUT/variant/'bin'/name,1,-2,1,1,0,1,*case['n'],case['depth'][0]*100+case['depth'][1]],
                    input=data,env=env,capture_output=True,text=True,timeout=120)
                (traces/f'{name}.B{budget}.{variant}.r{repeat}.txt').write_text(result.stderr)
                parsed=list(csv.reader(result.stdout.splitlines()))
                assert len(parsed)==2*args.samples
                for row in parsed:
                    phase,iteration,rl,rr,al,ar,backend,order,parents,ns,pred=map(int,row)
                    if phase!=1: assert f'{backend}:{al}:{ar}:{order}:{parents}'==item['winner']
                    rows.append(dict(case=name,budget=budget,variant=variant,repeat=repeat,
                        kind='native' if phase==1 else 'fixed_winner',iteration=iteration,
                        left=al,right=ar,backend=backend,order=order,parents=parents,time_ns=ns,prediction_ns=pred))
                line=next(s for s in result.stderr.splitlines() if s.startswith('SCHED_STATS,'))
                cold,learning,steady_learning,plans,evals,calibrations,calls=map(int,line.split(',')[1:])
                assert calls==12+args.samples
                stats.append(dict(case=name,budget=budget,variant=variant,repeat=repeat,cold_native_ns=cold,
                    total_learning_ns=learning,steady_learning_ns=steady_learning,searches=plans,
                    evaluations=evals,parent_calibrations=calibrations,native_calls=calls))
        print(name,'B'+str(budget),flush=True)
    sweep.write_csv(results/'paired_raw.csv',rows);sweep.write_csv(results/'overhead_raw.csv',stats)
    grouped=defaultdict(list)
    for row in rows: grouped[row['case'],row['budget'],row['variant'],row['kind'],row['repeat']].append(row['time_ns'])
    summary=[]
    for item in records:
        name=item['case']['name'];budget=item['budget'];record=dict(case=name,budget=budget)
        for variant in ('before','after'):
            for kind in ('native','fixed_winner'):
                record[variant+'_'+kind+'_ms']=st.median(st.median(grouped[name,budget,variant,kind,r])
                                                       for r in range(args.repeats))/1e6
            selected=[s for s in stats if (s['case'],s['budget'],s['variant'])==(name,budget,variant)]
            for field in ('cold_native_ns','total_learning_ns','steady_learning_ns','searches','evaluations'):
                record[variant+'_'+field]=st.median(s[field] for s in selected)
            choices=sorted({f'{r["left"]},{r["right"]}' for r in rows if
                (r['case'],r['budget'],r['variant'],r['kind'])==(name,budget,variant,'native')})
            record[variant+'_allocations']=';'.join(choices)
        record['after_over_before']=record['after_native_ms']/record['before_native_ms']
        record['after_over_fixed_winner']=record['after_native_ms']/record['after_fixed_winner_ms']
        summary.append(record)
    sweep.write_csv(results/'paired_summary.csv',summary)
    report=dict(settings=len(summary),timed_samples=len(rows),repeats=args.repeats,samples=args.samples,
        wins=sum(s['after_over_before']<1 for s in summary),
        total_time_after_over_before=sum(s['after_native_ms'] for s in summary)/sum(s['before_native_ms'] for s in summary),
        method='Same adapters and saved SGPL objects, randomized paired fresh processes; native timings precede frozen oracle replay.')
    (results/'benchmark_manifest.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2),flush=True)


if __name__=='__main__': main()
