#!/usr/bin/env python3
"""Read-only equation ablations and isolated timeout diagnostics.

Uses the prior frozen runtime. It never edits production sources or the saved
validation CSV. Ablations reuse pre-holdout inputs, without fitting constants.
The instrumented probe is for debugging only; its timings are not benchmarks.
"""
import csv
import hashlib
import json
import os
from pathlib import Path
import statistics as st
import subprocess
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[2]
VALIDATION = ROOT/'proof/doacross_cost_validation_2026_10_09'
OUT = ROOT/'proof/doacross_cost_diagnosis_2026_10_09'


def ablations():
    valid = {(r['case'], int(r['threads'])) for r in csv.DictReader(
        (VALIDATION/'summary.csv').open()) if r['validation_status']=='PASS'}
    grouped = defaultdict(lambda: defaultdict(list))
    for r in csv.DictReader((VALIDATION/'raw.csv').open()):
        p = int(r['threads']); n = int(r['trips']); d = int(r['distance'])
        if r['stage']!='learned-sync' or (r['case'],p) not in valid:
            continue
        dep, ind, wait, post, launch = [float(r[k]) for k in
            ('c_dep_ns','c_ind_ns','sigma_wait_ns','sigma_post_ns','launch_ns')]
        lane = (n+p-1)//p
        sync = int(r['streams'])*(wait+post)
        candidates = {
            'current': float(r['prediction_ns']),
            'lane_sync': launch+n*dep+lane*(ind+sync),
            'lane_all': launch+lane*(dep+ind+sync),
            'distance_dep_lane_sync': launch+((n+min(p,d)-1)//min(p,d))*dep+lane*(ind+sync),
            # Diagnostic overlap candidate, not a bound on this cyclic executor.
            'max_dep_lane_all': launch+max(n*dep,lane*(dep+ind+sync)),
        }
        for name,pred in candidates.items():
            grouped[r['case'],p,name][int(r['repeat'])].append((pred,float(r['actual_ns'])))
    rows=[]
    for (case,p,name),processes in sorted(grouped.items()):
        pred=st.median(st.median(x[0] for x in proc) for proc in processes.values())
        actual=st.median(st.median(x[1] for x in proc) for proc in processes.values())
        rows.append(dict(case=case,threads=p,equation=name,predicted_ms=pred/1e6,
            actual_ms=actual/1e6,signed_error_percent=100*(pred/actual-1),
            absolute_error_percent=abs(100*(pred/actual-1))))
    with (OUT/'ablations.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
    metrics=[]
    for name in sorted({r['equation'] for r in rows}):
        for p in [0,2,4,6,8]:
            selected=[r for r in rows if r['equation']==name and (not p or r['threads']==p)]
            metrics.append(dict(equation=name,threads=p,settings=len(selected),
                median_ape=st.median(r['absolute_error_percent'] for r in selected),
                mean_ape=st.mean(r['absolute_error_percent'] for r in selected),
                within_20=sum(r['absolute_error_percent']<=20 for r in selected),
                worst_ape=max(r['absolute_error_percent'] for r in selected)))
    (OUT/'ablation_metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
    print(json.dumps([r for r in metrics if not r['threads']],indent=2))


DIAGNOSTIC_GLOBALS = r'''
/* Test-only observations. All production synchronization stays unchanged. */
static atomic_long diag_iter[128],diag_last_post[128],diag_max_post[SYNC_WINDOW];
static atomic_int diag_phase[128],diag_active,diag_run;
static _Atomic(sgpl_doacross_state *) diag_state;
static atomic_ullong diag_started;
static int diag_replay(void) {
    sgpl_doacross_state *s=sgpl_alloc_doacross_state(3,1);
    assert(s);doacross_init_state(s,3);
    doacross_post_state(s,7932,0);
    long before=atomic_load(sgpl_doacross_slot(s,0,7932%SYNC_WINDOW));
    doacross_post_state(s,3836,0);
    long after=atomic_load(sgpl_doacross_slot(s,0,7932%SYNC_WINDOW));
    printf("dependency=7932 slot=%d after_post_7932=%ld after_post_3836=%ld wait_predicate=%d\n",
        7932%SYNC_WINDOW,before,after,after<7932);
    assert(before==7932 && after==3836 && after<7932);
    sgpl_free_doacross_state(s);return 0;
}
static void *diag_watchdog(void *unused) {
    (void)unused;
    for (;;) {
        usleep(100000);
        if (!atomic_load(&diag_active)) continue;
        if (sgpl_now_ns()-atomic_load(&diag_started)<3000000000ULL) continue;
        sgpl_doacross_state *s=atomic_load(&diag_state);
        fprintf(stderr,"STALL run=%d state=%p window=%d\n",atomic_load(&diag_run),(void*)s,s?s->sync_window:0);
        if (s) for (int t=0;t<sgpl_configured_worker_count();++t) {
            long i=atomic_load(&diag_iter[t]),dep=i-3;
            int slot=dep%s->sync_window;
            fprintf(stderr,"lane=%d phase=%d iteration=%ld dependency=%ld slot=%d actual_slot=%ld maximum_posted=%ld last_lane_post=%ld\n",
                t,atomic_load(&diag_phase[t]),i,dep,slot,
                atomic_load(sgpl_doacross_slot(s,0,slot)),atomic_load(&diag_max_post[slot]),atomic_load(&diag_last_post[t]));
        }
        fflush(stderr);_Exit(90);
    }
}
'''


def debug_source():
    text=(VALIDATION/'source/probe.c').read_text()
    text=text.replace('typedef struct {',DIAGNOSTIC_GLOBALS+'\ntypedef struct {',1)
    text=text.replace('kernel *k=argument;', '''kernel *k=argument;
    int t=sgpl_current_worker_index();
    atomic_store(&diag_state,g_tls_doacross_state);
    atomic_store(&diag_iter[t],i);
    atomic_store(&diag_phase[t],1);''')
    text=text.replace('doacross_wait(i,k->distance,0);', '''atomic_store(&diag_phase[t],2);
    doacross_wait(i,k->distance,0);
    atomic_store(&diag_phase[t],3);''')
    text=text.replace('doacross_post(i,0);', '''doacross_post(i,0);
    atomic_store(&diag_last_post[t],i);
    long old=atomic_load(&diag_max_post[i%SYNC_WINDOW]);
    while(old<i && !atomic_compare_exchange_weak(&diag_max_post[i%SYNC_WINDOW],&old,i)) {}
    atomic_store(&diag_phase[t],4);''')
    text=text.replace('uint64_t begin=sgpl_now_ns();\n    /* Direct dispatch', '''for(int t=0;t<128;++t) { atomic_store(&diag_phase[t],0);atomic_store(&diag_iter[t],0);atomic_store(&diag_last_post[t],-1); }
    for(int slot=0;slot<SYNC_WINDOW;++slot) atomic_store(&diag_max_post[slot],-1);
    atomic_fetch_add(&diag_run,1);
    atomic_store(&diag_started,sgpl_now_ns());atomic_store(&diag_active,1);
    uint64_t begin=sgpl_now_ns();
    /* Direct dispatch''')
    text=text.replace('return sgpl_now_ns()-begin;\n}', 'atomic_store(&diag_active,0);\n    return sgpl_now_ns()-begin;\n}')
    text=text.replace('int samples=atoi(argv[7])', 'pthread_t watcher;assert(pthread_create(&watcher,NULL,diag_watchdog,NULL)==0);\n    int samples=atoi(argv[7])')
    text=text.replace('assert(argc==8);','if(argc==2 && strcmp(argv[1],"--slot-replay")==0) return diag_replay();\n    assert(argc==8);')
    path=OUT/'diagnostic_probe.c';path.write_text(text)
    source=VALIDATION/'source'
    commands=[['gcc','-O3','-g','-mavx2','-pthread','-fopenmp','-I',str(source),'-I',str(ROOT),
        f'-DDOACROSS_RUNTIME_SOURCE="{source/"parallel_runtime.c"}"','-c',str(path),'-o',str(OUT/'diagnostic.o')],
        ['g++','-O3','-g','-pthread','-fopenmp',str(OUT/'diagnostic.o'),str(VALIDATION/'build/bitmap.o'),
        '-lnlopt','-lm','-o',str(OUT/'diagnostic_probe')]]
    for cmd in commands: subprocess.run(cmd,check=True,cwd=ROOT)


def reproduce():
    env={k:v for k,v in os.environ.items() if not k.startswith(('SGPL_','OMP_','GRAPH_PARALLEL'))}
    env.update(SGPL_NUM_THREADS='6',OMP_NUM_THREADS='6')
    outcomes=[]
    for r in range(50):
        result=subprocess.run([str(OUT/'diagnostic_probe'),'8193','3','4','8','0','1','7'],
            env=env,capture_output=True,text=True,timeout=10)
        (OUT/f'diagnostic.{r}.stderr.txt').write_text(result.stderr)
        outcomes.append(dict(repeat=r,exit_code=result.returncode))
        if result.returncode:
            (OUT/'diagnostic_failure.txt').write_text(result.stderr)
            print(result.stderr);break
    else:
        (OUT/'diagnostic_failure.txt').write_text('No failure reproduced in these 50 instrumented processes.\n')
    (OUT/'diagnostic_runs.json').write_text(json.dumps(outcomes,indent=2)+'\n')
    print('Diagnostic runs:',len(outcomes),'last exit:',outcomes[-1]['exit_code'])


def verify_sources(before):
    expected=json.loads((VALIDATION/'source_manifest.json').read_text())
    snapshot_checks={name:hashlib.sha256((VALIDATION/'source'/name).read_bytes()).hexdigest()==digest
            for name,digest in expected.items() if name!='test_probe.c'}
    checks={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest for name,digest in before.items()}
    assert all(snapshot_checks.values()),snapshot_checks
    assert all(checks.values()),checks
    (OUT/'frozen_source_integrity.json').write_text(json.dumps(snapshot_checks,indent=2)+'\n')
    (OUT/'production_unchanged.json').write_text(json.dumps(checks,indent=2)+'\n')


if __name__=='__main__':
    OUT.mkdir(exist_ok=True)
    protected=json.loads((VALIDATION/'source_manifest.json').read_text())
    before={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
            for name in protected if name!='test_probe.c'}
    ablations()
    debug_source()
    replay=subprocess.run([str(OUT/'diagnostic_probe'),'--slot-replay'],check=True,capture_output=True,text=True)
    (OUT/'slot_replay.txt').write_text(replay.stdout)
    print(replay.stdout)
    reproduce()
    verify_sources(before)
