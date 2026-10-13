#!/usr/bin/env python3
"""Exhaustive, SGPL-derived two-loop allocation/schedule validation (Linux/WSL)."""
import argparse
import csv
import hashlib
import html
import itertools
import json
import os
from pathlib import Path
import random
import re
import resource
import statistics as st
import subprocess
import sys
import time
from collections import defaultdict

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
CASES = [
    dict(name='tiny', n=[64,64], depth=[4,4], purpose='launch overhead dominates'),
    dict(name='chunk_edge', n=[257,513], depth=[8,8], purpose='256-iteration chunk imbalance'),
    dict(name='short', n=[4096,4096], depth=[4,4], purpose='serial/parallel crossover'),
    dict(name='balanced_compute', n=[65536,65536], depth=[16,16], purpose='equal substantial compute'),
    dict(name='trip_skew', n=[262144,16384], depth=[8,8], purpose='16:1 iteration-count skew'),
    dict(name='work_skew', n=[65536,65536], depth=[32,4], purpose='different work per iteration'),
    dict(name='work_skew_reversed', n=[65536,65536], depth=[4,32], purpose='reverse admission-order sensitivity'),
    dict(name='large_arrays', n=[1048576,1048576], depth=[4,4], purpose='large streaming arrays plus compute'),
    dict(name='uneven_chunks', n=[1025,4097], depth=[8,8], purpose='uneven trip/chunk counts'),
]
PROTECTED = ['parallel_runtime.c','parallel_runtime.h','tdg_scheduler.inc',
             'tdg_trace.inc','main.cpp','pdg.cpp','parallel_loop_outline.cpp',
             'autotuner_runtime.c','AutoTunerPass.cpp','graph_frontier_lowering.cpp']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def verify_recorded_source(name,digest):
    """Historical measurements must match their exact source, including a frozen copy."""
    current=ROOT/name
    if current.exists() and sha(current)==digest: return
    frozen=ROOT/'proof/tdg_dynamic_default_2026_10_09/before'/name
    assert frozen.exists() and sha(frozen)==digest, name

def run(args, **kw):
    return subprocess.run(list(map(str,args)),cwd=ROOT,check=True,**kw)

def generate():
    folder=HERE/'cases'; folder.mkdir(exist_ok=True)
    for case in CASES:
        lines=[f'// {case["purpose"]}. Each loop writes a separate allocation.',
               f'int nl = {case["n"][0]};',f'int nr = {case["n"][1]};',
               'int left[nl];','int right[nr];','int i = 0;','int j = 0;']
        for side,var,j in [('left','i',0),('right','j',1)]:
            lines += [f'while ({var} < {"nr" if j else "nl"}) {{',
                      f'  {side}[{var}] = ({var} * {31 if j else 17} + {5 if j else 3}) % {1013 if j else 1009};']
            for k in range(1,case['depth'][j]):
                lines += [f'  {side}[{var}] = ({side}[{var}] * {19+2*k} + {7+k}) % {1009+2*k};']
            lines += [f'  {var} = {var} + 1;','}']
        lines += [f'print left[{case["n"][0]-1}];',f'print right[{case["n"][1]-1}];']
        (folder/(case['name']+'.graph')).write_text('\n'.join(lines)+'\n')
    (folder/'manifest.json').write_text(json.dumps(CASES,indent=2)+'\n')

def ordinary_serial_ir(text):
    bodies={m.group(1):m.group(2) for m in re.finditer(
        r'^define [^\n@]+@([\w.$]+)\([^\n]*\)[^\n{]* \{(.*?)\n\}',text,re.S|re.M)}
    pending=['sgpl_ordinary_work']; visited=set()
    while pending:
        name=pending.pop()
        if name in visited: continue
        assert name in bodies, ('Missing ordinary SGPL function',name)
        visited.add(name)
        for callee in re.findall(r'call [^\n@]*@([\w.$]+)\(',bodies[name]):
            assert callee.startswith('llvm.') or callee in bodies, ('Ordinary work invokes external runtime',callee)
            if callee in bodies: pending.append(callee)
    return sorted(visited)

def build(cases,out,driver_source=None):
    build=out/'build'; build.mkdir(parents=True,exist_ok=True)
    run(['bash','test/build_tdg_validation.sh'],stdout=(build/'compiler-build.txt').open('w'))
    source=(ROOT/'parallel_runtime.c').read_text()
    needle='granted_threads = sgpl_loop_pool_try_acquire(loop_id, nthreads);'
    assert source.count(needle)==2, 'Grant instrumentation sites changed; review adapter'
    (build/'observed_runtime.c').write_text(source.replace(needle,needle+'\n    sweep_grant(loop_id, granted_threads);'))
    sdk=[]
    for name in ['runtime.c','autotuner_runtime.c','graph_mutation_runtime.c','gpu_runtime.c',
                 'semiring_runtime.c','roaring_bitmap.cpp','graph_loader_runtime.cpp','graph_runtime.cpp']:
        obj=build/(name+'.o')
        run(['g++' if name.endswith('.cpp') else 'gcc','-O2','-mavx2','-pthread','-fopenmp',
             '-I.', '-c',name,'-o',obj]); sdk.append(obj)
    driver=build/'driver.o'
    run(['gcc','-O2','-pthread','-fopenmp','-iquote',ROOT,
         f'-DSWEEP_RUNTIME_SOURCE="{build / "observed_runtime.c"}"','-c',driver_source or HERE/'driver.c','-o',driver])
    records=[]
    for case in cases:
        ir=build/(case['name']+'.ll')
        env=dict(os.environ,GRAPH_DISABLE_POLLY='1',SGPL_TDG_DEBUG='1')
        compiled=run([ROOT/'build_tdg_validation/GraphProgram_tdg','--ir-backend=cpu',
                      '--tdg-scheduler=current',f'--emit-ir-to={ir}',HERE/'cases'/(case['name']+'.graph')],
                     env=env,capture_output=True,text=True)
        (build/(case['name']+'.compile.txt')).write_text(compiled.stdout+compiled.stderr)
        text=ir.read_text()
        assert text.count('call i32 @sgpl_should_parallelize_doall')==2, case['name']
        assert 'call i32 @sgpl_should_parallelize_doacross' not in text
        if 'ordinary' in case:
            ordinary_serial_ir(text)
        # The adapter relies on exactly one pointer field (the private array)
        # in each loop callback's environment; fail closed on compiler changes.
        loop_callbacks=[]
        for task in re.findall(r'@sgpl.tdg.loop_ids\.(\w+) = private unnamed_addr constant \[1 x i32\]',text):
            wrapper=re.search(r'define internal ptr @'+task+r'\(ptr %0\) \{(.*?)\n\}',text,re.S)
            assert wrapper and wrapper.group(1).count('load ptr')==1
            called=re.search(r'call void @(\w+)\(ptr %field_0\)',wrapper.group(1))
            assert called, task
            loop_callbacks.append(called.group(1))
        assert len(loop_callbacks)==2,loop_callbacks
        counts=re.findall(r'\[tdg.emitted\] level=(\d+) count=(\d+)',compiled.stderr)
        assert all(int(count)==1 for _,count in counts), 'Review adapter: grouping changed'
        obj=build/(case['name']+'.o'); renamed=build/(case['name']+'.repeat.o')
        obj.write_bytes((ROOT/'program.o').read_bytes())
        run(['objcopy','--redefine-sym','main=graph_main',obj,renamed])
        run(['g++','-O2','-no-pie','-pthread','-fopenmp',renamed,driver,*sdk,
             '-Wl,--wrap=sgpl_run_tdg_level','-Wl,--wrap=sgpl_should_parallelize_doall',
             '-Wl,--wrap=printf','-ldl','-lnlopt','-lm','-o',build/case['name']])
        records.append(dict(case=case['name'],source_sha256=sha(HERE/'cases'/(case['name']+'.graph')),
                            object_sha256=sha(obj),ir_sha256=sha(ir),loop_callbacks=loop_callbacks,
                            compiler_level_counts=counts,adapter='two separate levels combined for runtime validation'))
        print('Built',case['name'],flush=True)
    return records

def configurations(budget,n,policy,include_fifo=True):
    extra=lambda w:w if w>=2 else 0
    ordinary=[]
    for a in range(1,min(n[0],max(1,budget-2))+1):
        for b in range(1,min(n[1],max(1,budget-2))+1):
            if extra(a)+extra(b)<=max(0,budget-2):
                ordinary.append((0,a,b,0,min(2,budget)))
    if policy=='ordinary-first': return ordinary
    joint=[]
    for a in range(1,min(n[0],max(1,budget-1))+1):
        for b in range(1,min(n[1],max(1,budget-1))+1):
            for order in (0,1):
                joint.append((1,a,b,order,1))
                if 2+extra(a)+extra(b)<=budget:
                    joint.append((1,a,b,order,2))
    return ordinary+([(2,*c[1:]) for c in ordinary] if include_fifo else [])+joint

FIELDS=['case','budget','policy','repeat','kind','config','backend','order','parents',
        'requested_left','requested_right','actual_left','actual_right','time_ns','prediction_ns']

def measure(cases,args,out):
    raw=out/'raw.csv'
    with raw.open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=FIELDS); writer.writeheader()
        for case in cases:
            for budget in args.budgets:
                for repeat in range(args.repeats):
                    policies=['ordinary-first','dynamic']; random.Random(args.seed+repeat).shuffle(policies)
                    for policy in policies:
                        configs=configurations(budget,case['n'],policy)
                        random.Random(f'{args.seed}-{case["name"]}-{budget}-{repeat}-{policy}').shuffle(configs)
                        stdin=''.join(' '.join(map(str,c))+'\n' for c in configs)
                        env=dict(os.environ,SGPL_NUM_THREADS=str(budget),OMP_NUM_THREADS=str(budget),
                                 SWEEP_SAMPLES=str(args.samples))
                        for key in list(env):
                            if key.startswith(('SGPL_FORCE','SGPL_JOINT','SGPL_TDG','SGPL_BUDGET','GRAPH_PARALLEL')):
                                del env[key]
                        env.update(SGPL_NUM_THREADS=str(budget),OMP_NUM_THREADS=str(budget),SWEEP_SAMPLES=str(args.samples))
                        if args.diagnose: env['SWEEP_DIAGNOSE']='1'
                        begin=time.monotonic()
                        result=run([out/'build'/case['name'],int(policy=='dynamic'),-2,1,1,0,1,
                                    *case['n'],case['depth'][0]*100+case['depth'][1]],
                                   input=stdin,env=env,capture_output=True,text=True,timeout=args.timeout)
                        (out/'traces').mkdir(exist_ok=True)
                        (out/'traces'/f'{case["name"]}.B{budget}.{policy}.r{repeat}.txt').write_text(result.stderr)
                        rows=list(csv.reader(result.stdout.splitlines()))
                        assert len(rows)==args.samples*(1+len(configs)), (case,budget,policy,len(rows))
                        for row in rows:
                            phase,it,rl,rr,al,ar,backend,order,parents,ns,pred=map(int,row)
                            config=None if phase==1 else configs[phase-100]
                            if config:
                                assert (backend,al,ar,order,parents)==config, (config,row)
                            writer.writerow(dict(case=case['name'],budget=budget,policy=policy,repeat=repeat,
                                kind='model' if phase==1 else 'forced',config='' if config is None else ':'.join(map(str,config)),
                                backend=backend,order=order,parents=parents,requested_left=rl,requested_right=rr,
                                actual_left=al,actual_right=ar,time_ns=ns,prediction_ns=pred))
                        f.flush()
                        print(f'{case["name"]} B{budget} {policy} repeat {repeat+1}/{args.repeats}: '
                              f'{len(configs)} schedules, {len(rows)} samples, {time.monotonic()-begin:.1f}s',flush=True)

def write_csv(path,rows):
    with path.open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)

def sanitizers(out):
    build=out/'build'; case=next(c for c in CASES if (build/(c['name']+'.repeat.o')).exists())
    obj=build/'sanitized.o'; exe=build/'sanitized'
    flags=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
    run(['gcc','-O1','-g','-pthread','-fopenmp','-iquote',ROOT,*flags,
         f'-DSWEEP_RUNTIME_SOURCE="{build / "observed_runtime.c"}"','-c',HERE/'driver.c','-o',obj])
    sdk=[build/(name+'.o') for name in ['runtime.c','autotuner_runtime.c','graph_mutation_runtime.c','gpu_runtime.c',
        'semiring_runtime.c','roaring_bitmap.cpp','graph_loader_runtime.cpp','graph_runtime.cpp']]
    run(['g++','-no-pie','-pthread','-fopenmp',*flags,build/(case['name']+'.repeat.o'),obj,*sdk,
         '-Wl,--wrap=sgpl_run_tdg_level','-Wl,--wrap=sgpl_should_parallelize_doall','-Wl,--wrap=printf',
         '-ldl','-lnlopt','-lm','-o',exe])
    for policy in ('ordinary-first','dynamic'):
        configs=configurations(4,case['n'],policy)
        env=dict(os.environ,SGPL_NUM_THREADS='4',OMP_NUM_THREADS='4',SWEEP_SAMPLES='3',
                 ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
        result=run([exe,int(policy=='dynamic'),-2,1,1,0,1,*case['n'],case['depth'][0]*100+case['depth'][1]],
                   input=''.join(' '.join(map(str,c))+'\n' for c in configs),env=env,capture_output=True,text=True,timeout=60)
        assert len(result.stdout.splitlines())==3*(1+len(configs))
        assert not result.stderr, result.stderr
    (out/'sanitizer_validation.txt').write_text('PASS: ASan/UBSan test adapter and runtime, both policies, all B4 schedules.\n'
        'SGPL-generated objects and SDK objects are not sanitizer-instrumented.\n'
        'Leak detection disabled: existing generated dispatch environments and persistent runtime pools outlive calls.\n')

def analyze(out):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib.lines import Line2D
    import numpy as np
    mixed_path=out/'mixed_manifest.json'
    mixed=json.loads(mixed_path.read_text()) if mixed_path.exists() else {}
    mixed_cases={c['name']:c for c in mixed.get('cases',[])}
    def order_label(case,backend,code):
        if case not in mixed_cases: return 'left,right' if code==0 else 'right,left'
        if backend==0: return 'ordinary phase, then loops' if mixed_cases[case]['total_tasks']>budget else 'reference task queue'
        if backend==2: return 'FIFO task queue'
        sequence=mixed_cases[case]['orders'][code//2]
        return ','.join('left' if j==0 else 'right' if j==1 else f'ordinary{j-1}' for j in sequence)+f'; backfill={code%2}'
    with (out/'raw.csv').open() as f: raw=list(csv.DictReader(f))
    for row in raw:
        for key in FIELDS[3:]:
            if key not in ('kind','config'): row[key]=int(row[key])
        row['budget']=int(row['budget'])
    grouped=defaultdict(list)
    for r in raw: grouped[r['case'],r['budget'],r['policy']].append(r)
    keys=sorted(grouped)
    summaries=[]; allocations=[]; schedules=[]; pages=[]; wide=[]
    diagnostic_scores=[]; diagnostic_searches=[]
    (out/'plots').mkdir(exist_ok=True)
    rng=np.random.default_rng(913)
    for case,budget,policy in keys:
        rows=grouped[case,budget,policy]
        models=[r for r in rows if r['kind']=='model']; forced=[r for r in rows if r['kind']=='forced']
        repetitions=sorted(set(r['repeat'] for r in rows))
        by_config={}; config_samples=defaultdict(list); configs_by_pair=defaultdict(list)
        for r in forced: config_samples[r['config']].append(r)
        for config in sorted(config_samples):
            sample=config_samples[config]
            medians=[st.median(r['time_ns'] for r in sample if r['repeat']==rep) for rep in repetitions]
            by_config[config]=medians
            c=list(map(int,config.split(':')))
            configs_by_pair[c[1],c[2]].append(config)
            schedules.append(dict(case=case,budget=budget,policy=policy,backend=c[0],left=c[1],right=c[2],
                                  order=c[3],parents=c[4],median_ns=st.median(medians),
                                  min_process_median_ns=min(medians),max_process_median_ns=max(medians)))
        best_config=min(by_config,key=lambda c:st.median(by_config[c]))
        best_ns=st.median(by_config[best_config])
        for rep in repetitions:
            trace=out/'traces'/f'{case}.B{budget}.{policy}.r{rep}.txt'
            scores=[]; search=None
            for line in trace.read_text().splitlines():
                cells=line.split(',')
                if cells[0]=='DIAG_SCORE':
                    a,b,o,h,pred=map(int,cells[1:])
                    config=f'1:{a}:{b}:{o}:{h}'
                    measured=by_config[config][repetitions.index(rep)]
                    scores.append((pred,config))
                    diagnostic_scores.append(dict(case=case,budget=budget,repeat=rep,left=a,right=b,order=o,parents=h,
                                                  prediction_ns=pred,measured_ns=measured))
                elif cells[0]=='DIAG_SEARCH': search=list(map(int,cells[1:]))
            if search and scores:
                pred,config=min(scores)
                evaluations,planning,chosen,reference,a,b=search
                diagnostic_searches.append(dict(case=case,budget=budget,repeat=rep,evaluations=evaluations,
                    last_planning_ns=planning,chosen_left=a,chosen_right=b,chosen_prediction_ns=chosen,
                    reference_prediction_ns=reference,best_predicted_config=config,best_prediction_ns=pred,
                    chosen_over_best_predicted=chosen/pred))
        model_medians=[st.median(r['time_ns'] for r in models if r['repeat']==rep) for rep in repetitions]
        choices={}
        assigned_choices={}
        for r in models:
            choice=(r['actual_left'],r['actual_right']); choices[choice]=choices.get(choice,0)+1
            assigned=(r['requested_left'],r['requested_right'])
            assigned_choices[assigned]=assigned_choices.get(assigned,0)+1
        modal=max(choices,key=lambda c:choices[c]); modal_fraction=choices[modal]/len(models)
        pairs=sorted(set((int(c.split(':')[1]),int(c.split(':')[2])) for c in by_config))
        bars=[]; low=[]; high=[]; table_rows=[]; pair_configs={}
        for a,b in pairs:
            configs=configs_by_pair[a,b]
            c=min(configs,key=lambda c:st.median(by_config[c])); times=by_config[c]
            pair_configs[a,b]=c
            median=st.median(times); bars.append(median/1e6)
            low.append((median-min(times))/1e6); high.append((max(times)-median)/1e6)
            params=list(map(int,c.split(':')))
            row=dict(case=case,budget=budget,policy=policy,left=a,right=b,median_ns=median,
                     median_ms=median/1e6,
                     min_process_median_ns=min(times),max_process_median_ns=max(times),
                     best_backend='ordinary reference' if params[0]==0 else 'joint' if params[0]==1 else 'fifo',
                     best_order=order_label(case,params[0],params[3]),best_parents=params[4],
                     model_selection_fraction=choices.get((a,b),0)/len(models),empirical_best=int(c==best_config))
            row['planner_assignment_fraction']=assigned_choices.get((a,b),0)/len(models)
            allocations.append(row); table_rows.append(row)
        models_array=np.array(model_medians); best_array=np.array(by_config[best_config])
        indices=rng.integers(0,len(model_medians),(4000,len(model_medians)))
        boots=np.median(models_array[indices],axis=1)/np.median(best_array[indices],axis=1)
        ci=np.quantile(boots,[.025,.975])
        ratio=st.median(model_medians)/best_ns
        replay_ratios=[]; pair_ratios=[]; exact_pair=[]
        for i,rep in enumerate(repetitions):
            native=[r for r in models if r['repeat']==rep]
            counts={}
            for r in native:
                config=':'.join(str(r[k]) for k in ('backend','actual_left','actual_right','order','parents'))
                counts[config]=counts.get(config,0)+1
            selected=max(counts,key=counts.get)
            assert selected in by_config, ('Native model used a schedule absent from exhaustive oracle',selected)
            pair=tuple(map(int,selected.split(':')[1:3]))
            replay_ratios.append(by_config[selected][i]/by_config[best_config][i])
            pair_ratios.append(by_config[pair_configs[pair]][i]/by_config[best_config][i])
            exact_pair.append(pair==tuple(map(int,best_config.split(':')[1:3])))
        # A repeat-to-repeat interval is deliberately separate from the
        # optimistic minimum over many allocations; it is not a proof of optimum.
        verdict='within 10%' if ratio<=1.1 else '10–25% slower' if ratio<=1.25 else '>25% slower'
        summary=dict(case=case,budget=budget,policy=policy,allocations=len(pairs),schedules=len(by_config),
                     model_actual_left=modal[0],model_actual_right=modal[1],modal_selection_fraction=modal_fraction,
                     requested_allocations=';'.join(sorted(set(f'{r["requested_left"]},{r["requested_right"]}' for r in models))),
                     best_config=best_config,best_ms=best_ns/1e6,model_ms=st.median(model_medians)/1e6,
                     model_over_best_ratio=ratio,ratio_bootstrap_low=float(ci[0]),ratio_bootstrap_high=float(ci[1]),
                     allocation_only_ratio=st.median(pair_ratios),selected_schedule_replay_ratio=st.median(replay_ratios),
                     exact_best_allocation_repeat_fraction=st.mean(exact_pair),
                     fallback_fraction=sum(r['backend']!=1 for r in models)/len(models) if policy=='dynamic' else 0,
                     verdict=verdict)
        ordinary_tasks=len(mixed_cases.get(case,{}).get('ordinary',[]))
        summary.update(ordinary_tasks=ordinary_tasks,total_tasks=ordinary_tasks+2,
                       overloaded=int(ordinary_tasks+2>budget))
        summaries.append(summary)
        wide.append(dict(case=case,budget=budget,policy=policy,**{f'({a},{b}) ms':bars[i] for i,(a,b) in enumerate(pairs)}))
        stem=f'{case}.B{budget}.{policy}'
        fig,ax=plt.subplots(figsize=(max(9,len(pairs)*.28),6.5))
        colors=['#e89e32' if choices.get(pair,0) else '#5885ad' for pair in pairs]
        rects=ax.bar(range(len(pairs)),bars,color=colors,yerr=[low,high],capsize=2)
        for i,pair in enumerate(pairs):
            if assigned_choices.get(pair,0):
                ax.scatter(i,bars[i]+high[i],marker='D',s=32,color='#7844a1',zorder=4)
            if pair==tuple(map(int,best_config.split(':')[1:3])):
                rects[i].set_edgecolor('#167343'); rects[i].set_linewidth(3)
        ax.axhline(st.median(model_medians)/1e6,color='#ad352b',ls='--',lw=1.5)
        ax.set_xticks(range(len(pairs)),[f'{a},{b}' for a,b in pairs],rotation=90 if len(pairs)>12 else 0)
        ax.set_xlabel('Loop worker widths (left,right); width 1 executes on its parent')
        ax.set_ylabel('Median level wall time (ms)')
        level_note=f' · {ordinary_tasks} ordinary tasks' if ordinary_tasks else ''
        ax.set_title(f'{case} · B={budget} · {policy}{level_note}\n'
                     f'All {len(pairs)} legal allocations; best legal schedule per pair; model/best={ratio:.2f}×',pad=60)
        ax.legend(handles=[Line2D([0],[0],color='#7844a1',marker='D',ls='',label='Planner assigned these widths'),
                           Line2D([0],[0],color='#e89e32',lw=8,label='Model executed this allocation'),
                           Line2D([0],[0],color='#167343',lw=3,label='Fastest measured allocation'),
                           Line2D([0],[0],color='#ad352b',ls='--',label='Actual model time, including planning')],
                  fontsize=8,loc='lower center',bbox_to_anchor=(.5,1.01),ncol=2)
        ax.grid(axis='y',alpha=.2); fig.tight_layout()
        fig.savefig(out/'plots'/(stem+'.png'),dpi=150,bbox_inches='tight')
        fig.savefig(out/'plots'/(stem+'.svg'),bbox_inches='tight'); plt.close(fig)
        headers=['left','right','median_ms','best_backend','best_order','best_parents','planner_assignment_fraction','model_selection_fraction','empirical_best']
        table='<table><tr>'+''.join('<th>'+h+'</th>' for h in headers)+'</tr>'
        for r in table_rows:
            table+='<tr'+(' class="chosen"' if r['model_selection_fraction'] else '')+'>'+''.join('<td>'+html.escape(str(r[h]))+'</td>' for h in headers)+'</tr>'
        table+='</table>'
        pages.append(f'<h2>{html.escape(stem)}</h2><p>{html.escape(str(summary))}</p>'
                     f'<a href="plots/{stem}.svg"><img src="plots/{stem}.png"></a>{table}')
    write_csv(out/'allocations.csv',allocations); write_csv(out/'schedules.csv',schedules)
    write_csv(out/'summary.csv',summaries)
    if diagnostic_scores: write_csv(out/'diagnostic_scores.csv',diagnostic_scores)
    if diagnostic_searches: write_csv(out/'search_diagnostics.csv',diagnostic_searches)
    fields=['case','budget','policy']+sorted(set(k for r in wide for k in r if k not in ('case','budget','policy')))
    with (out/'allocation_columns.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=fields); w.writeheader(); w.writerows(wide)
    intro=(HERE/'README.txt').read_text()
    (out/'tables.html').write_text('<!doctype html><meta charset="utf-8"><title>SGPL allocation sweep</title>'
        '<style>body{font:14px system-ui;margin:30px}table{border-collapse:collapse}td,th{padding:6px;border:1px solid #ddd}'
        '.chosen{background:#fff1d9}img{max-width:100%}pre{white-space:pre-wrap}</style><h1>SGPL allocation and scheduling validation</h1>'
        '<pre>'+html.escape(intro)+'</pre>'+''.join(pages))
    plt.rcParams.update({'font.size':9})
    for policy in ('ordinary-first','dynamic'):
        data=[s for s in summaries if s['policy']==policy]
        fig,ax=plt.subplots(figsize=(max(10,len(data)*.35),5))
        ax.bar(range(len(data)),[s['model_over_best_ratio'] for s in data],
               color=['#41986d' if s['model_over_best_ratio']<=1.1 else '#e8a13b' if s['model_over_best_ratio']<=1.25 else '#bb5350' for s in data])
        ax.axhline(1,color='black',lw=1); ax.axhline(1.1,color='#555',ls='--',lw=1)
        ax.set_xticks(range(len(data)),[f'{s["case"]} B{s["budget"]}' for s in data],rotation=90)
        ax.set_ylabel('Model time / best measured legal allocation and schedule')
        ax.set_title(policy+' · includes model planning time'); fig.tight_layout()
        fig.savefig(out/'plots'/('overview.'+policy+'.png'),dpi=150); plt.close(fig)
    comparisons=[]
    for case,budget in sorted(set((s['case'],s['budget']) for s in summaries)):
        current=next(s for s in summaries if (s['case'],s['budget'],s['policy'])==(case,budget,'ordinary-first'))
        joint=next(s for s in summaries if (s['case'],s['budget'],s['policy'])==(case,budget,'dynamic'))
        comparisons.append(dict(case=case,budget=budget,ordinary_first_ms=current['model_ms'],
                                dynamic_ms=joint['model_ms'],dynamic_over_ordinary=joint['model_ms']/current['model_ms'],
                                common_best_ms=min(current['best_ms'],joint['best_ms']),
                                ordinary_tasks=current['ordinary_tasks'],total_tasks=current['total_tasks'],
                                overloaded=current['overloaded']))
    write_csv(out/'policy_comparison.csv',comparisons)
    counts={policy:{label:sum(s['policy']==policy and s['verdict']==label for s in summaries)
                   for label in ('within 10%','10–25% slower','>25% slower')}
            for policy in ('ordinary-first','dynamic')}
    allocation_counts={policy:dict(within_10_percent=sum(s['policy']==policy and s['allocation_only_ratio']<=1.1 for s in summaries),
                                  within_25_percent=sum(s['policy']==policy and s['allocation_only_ratio']<=1.25 for s in summaries),
                                  nontrivial_within_10_percent=sum(s['policy']==policy and s['allocations']>1 and s['allocation_only_ratio']<=1.1 for s in summaries),
                                  nontrivial_tested=sum(s['policy']==policy and s['allocations']>1 for s in summaries),
                                  tested=sum(s['policy']==policy for s in summaries)) for policy in ('ordinary-first','dynamic')}
    (out/'analysis.json').write_text(json.dumps(dict(classification=counts,allocation_quality=allocation_counts,
                                                    summary=summaries,policy_comparison=comparisons,
                                                    search_diagnostics=diagnostic_searches),indent=2)+'\n')
    (out/'report.txt').write_text('SGPL allocation and scheduling validation\n\n'+intro+'\n\n'
        'Allocation quality (best measured schedule at the selected actual width pair):\n'+json.dumps(allocation_counts,indent=2)+
        '\n\nNative model times, including planning, relative to the exhaustive oracle:\n'+json.dumps(counts,indent=2)+'\n\n'+
        '\n'.join(f'{s["case"]:22} B{s["budget"]} {s["policy"]:14} chosen=({s["model_actual_left"]},{s["model_actual_right"]}) '
                  f'best={s["best_config"]} allocation={s["allocation_only_ratio"]:.3f}x '
                  f'schedule={s["selected_schedule_replay_ratio"]:.3f}x native={s["model_over_best_ratio"]:.3f}x' for s in summaries)+'\n\n'+
        ('Frozen-profile search diagnostics (see search_diagnostics.csv and diagnostic_scores.csv):\n'+
         '\n'.join(f'{d["case"]:22} B{d["budget"]} repeat={d["repeat"]} evaluations={d["evaluations"]} '
                   f'chosen=({d["chosen_left"]},{d["chosen_right"]}) prediction={d["chosen_prediction_ns"]/1e6:.3f}ms '
                   f'best-predicted={d["best_predicted_config"]} prediction={d["best_prediction_ns"]/1e6:.3f}ms '
                   f'ratio={d["chosen_over_best_predicted"]:.3f}x' for d in diagnostic_searches) if diagnostic_searches else '')+'\n')
    print(json.dumps(counts,indent=2),flush=True)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out',type=Path,default=ROOT/'proof/tdg_allocation_sweep')
    p.add_argument('--cases',nargs='+',default=[c['name'] for c in CASES])
    p.add_argument('--budgets',nargs='+',type=int,default=[2,4,6,8])
    p.add_argument('--repeats',type=int,default=3)
    p.add_argument('--samples',type=int,default=9)
    p.add_argument('--seed',type=int,default=7319)
    p.add_argument('--timeout',type=int,default=600)
    p.add_argument('--analyze-only',action='store_true')
    p.add_argument('--diagnose',action='store_true',help='Score frozen profiles exhaustively after recording native choices')
    p.add_argument('--sanitizers-only',action='store_true',help='Check adapter/runtime using an existing build; no benchmark timings')
    args=p.parse_args(); args.out=args.out.resolve(); args.out.mkdir(parents=True,exist_ok=True)
    assert args.repeats>=3 and args.samples>=3 and min(args.budgets)>=2
    assert max(args.budgets)<=64, 'The experimental model supports at most 64 workers'
    if args.analyze_only: analyze(args.out); return
    if args.sanitizers_only: sanitizers(args.out); return
    assert set(args.cases)<=set(c['name'] for c in CASES)
    cases=[c for c in CASES if c['name'] in args.cases]
    before={name:sha(ROOT/name) for name in PROTECTED}
    soft,hard=resource.getrlimit(resource.RLIMIT_STACK)
    desired=64*1024*1024
    resource.setrlimit(resource.RLIMIT_STACK,(desired if hard<0 else min(desired,hard),hard))
    generate(); records=build(cases,args.out)
    manifest=dict(oracle_version=2,arguments={k:str(v) if isinstance(v,Path) else v for k,v in vars(args).items()},
                  cases=cases,generated=records,protected_sha256=before,platform=sys.platform,
                  cpu=run(['lscpu'],capture_output=True,text=True).stdout,
                  affinity=sorted(os.sched_getaffinity(0)),python=sys.version,started_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()))
    manifest['stack_limit_bytes']=resource.getrlimit(resource.RLIMIT_STACK)[0]
    manifest['test_source_sha256']={name:sha(HERE/name) for name in ('driver.c','run.py','README.txt')}
    (args.out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    measure(cases,args,args.out); analyze(args.out)
    after={name:sha(ROOT/name) for name in PROTECTED}
    assert before==after, 'Protected production sources changed during sweep'
    (args.out/'validation.txt').write_text('PASS: all requested widths equal actual grants.\n'
        'PASS: SGPL output arrays checked in full during training and at sampled indices every execution.\n'
        'PASS: protected model, compiler, loop and autotuner sources unchanged.\n'
        'PASS: expected sample counts for every legal allocation/schedule in every repeat.\n')

if __name__=='__main__': main()
