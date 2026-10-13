#!/usr/bin/env python3
"""Check the frozen updated runtime and CSV exports; compare historical accuracy."""
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import statistics as st
import subprocess
from collections import defaultdict

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'proof/doacross_cost_worker_work_2026_10_09'
BASE=ROOT/'proof/doacross_cost_updated_2026_10_09'
ORIGINAL=ROOT/'proof/doacross_cost_validation_2026_10_09'


def scope_checks():
    original=json.loads((ORIGINAL/'source_manifest.json').read_text())
    previous=json.loads((BASE/'source_manifest.json').read_text())
    frozen=json.loads((OUT/'source_manifest.json').read_text())
    allowed={'parallel_runtime.c','parallel_runtime.h','tdg_scheduler.inc'}
    unchanged={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest
               for name,digest in original.items() if name!='test_probe.c' and name not in allowed}
    matches={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest
             for name,digest in frozen.items() if name!='test_probe.c'}
    previous_unchanged={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==digest
                        for name,digest in previous.items() if name not in ('test_probe.c','parallel_runtime.c')}
    old=(BASE/'source/parallel_runtime.c').read_text()
    new=(ROOT/'parallel_runtime.c').read_text()
    def function(text,name):
        start=text.index('static void '+name+'(')
        brace=text.index('{',start);depth=1;end=brace+1
        while depth:
            if text[end]=='{':depth+=1
            elif text[end]=='}':depth-=1
            end+=1
        return text[start:end]
    sync={name:function(old,name)==function(new,name) for name in ('doacross_wait_state','doacross_post_state')}
    assert all(unchanged.values()) and all(matches.values()) and all(sync.values()) and all(previous_unchanged.values())
    (OUT/'change_scope.json').write_text(json.dumps(dict(
        unchanged_protected_sources=unchanged,measured_snapshot_matches_current=matches,
        unchanged_since_previous_model_except_equation=previous_unchanged,
        completion_ring_operations_unchanged=sync),indent=2)+'\n')


def checks():
    source=OUT/'source';build=OUT/'build'
    shutil.copy2(ROOT/'test/doacross_cost_validation/model_test.c',source/'model_test.c')
    env={k:v for k,v in os.environ.items() if not k.startswith(('SGPL_','OMP_','GRAPH_PARALLEL'))}
    env.update(SGPL_NUM_THREADS='4',OMP_NUM_THREADS='4')
    with (OUT/'model_checks.txt').open('w') as log:
        for sanitized in (False,True):
            name='model_test_sanitized' if sanitized else 'model_test'
            flags=['-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined'] if sanitized else ['-O3']
            commands=[['gcc',*flags,'-mavx2','-pthread','-fopenmp','-I',source,'-I',ROOT,
                f'-DDOACROSS_RUNTIME_SOURCE="{source/"parallel_runtime.c"}"','-c',source/'model_test.c','-o',build/(name+'.o')],
                ['g++',*flags,'-pthread','-fopenmp',build/(name+'.o'),build/'bitmap.o','-lnlopt','-lm','-o',build/name]]
            for command in commands:
                subprocess.run(list(map(str,command)),cwd=ROOT,check=True,stdout=log,stderr=subprocess.STDOUT)
            if sanitized:
                env.update(ASAN_OPTIONS='detect_leaks=0:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
            subprocess.run([str(build/name)],env=env,check=True,timeout=60,stdout=log,stderr=subprocess.STDOUT)
            print('PASS',name,flush=True)


def export_checks():
    processes=defaultdict(list)
    pairs=defaultdict(dict)
    maximum=0.0
    for r in csv.DictReader((OUT/'raw.csv').open()):
        n,p,streams=[int(r[k]) for k in ('trips','threads','streams')]
        dep,ind,wait,post,launch=[float(r[k]) for k in ('c_dep_ns','c_ind_ns','sigma_wait_ns','sigma_post_ns','launch_ns')]
        reconstructed=launch+math.ceil(n/p)*(dep+ind+streams*(wait+post))
        maximum=max(maximum,abs(reconstructed-float(r['prediction_ns'])))
        assert abs(reconstructed-float(r['prediction_ns']))<.002,(r,reconstructed)
        assert int(r['sync_samples'])==(4 if r['stage']=='learned-sync' else 0)
        processes[r['case'],p,r['repeat'],r['stage']].append(r)
        pairs[r['case'],p,r['repeat'],r['sample']][r['stage']]=int(r['actual_ns'])
    assert all(len(set(pair.values()))==1 for pair in pairs.values())
    for rows in processes.values():
        for field in ('prediction_ns','c_dep_ns','c_ind_ns','sigma_wait_ns','sigma_post_ns','launch_ns','sync_samples'):
            assert len({r[field] for r in rows})==1,(field,rows)
    for r in csv.DictReader((OUT/'summary.csv').open()):
        if r['validation_status']!='PASS':continue
        signed=100*(float(r['predicted_ms'])/float(r['measured_ms'])-1)
        assert abs(signed-float(r['signed_error_percent']))<1e-9
        assert abs(abs(signed)-float(r['absolute_error_percent']))<1e-9
    (OUT/'export_validation.txt').write_text(
        f'PASS reconstructed predictions; max discrepancy {maximum:.9g} ns\n'
        'PASS frozen forecasts/profile inputs; width samples = 4 learned / 0 seeded\n'
        'PASS seeded/learned forecasts share actual timings\n'
        'PASS independent signed and absolute error reconstruction\n')


def compare():
    def read(path):
        return {(r['case'],int(r['threads'])):r for r in csv.DictReader(path.open())
                if r['stage']=='learned-sync' and r['validation_status']=='PASS'}
    old=read(BASE/'summary.csv');new=read(OUT/'summary.csv')
    keys=sorted(old.keys()&new.keys());rows=[]
    for key in keys:
        a,b=old[key],new[key]
        rows.append(dict(case=key[0],threads=key[1],old_prediction_ms=a['predicted_ms'],old_actual_ms=a['measured_ms'],
            old_absolute_error_percent=a['absolute_error_percent'],updated_prediction_ms=b['predicted_ms'],
            updated_actual_ms=b['measured_ms'],updated_absolute_error_percent=b['absolute_error_percent']))
    with (OUT/'before_after.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
    metrics=[]
    for p in [0,*sorted({r['threads'] for r in rows})]:
        selected=[r for r in rows if not p or r['threads']==p]
        for name,prefix in [('Previous','old'),('Per-worker work','updated')]:
            errors=[float(r[prefix+'_absolute_error_percent']) for r in selected]
            metrics.append(dict(threads=p,model=name,settings=len(errors),median_ape=st.median(errors),
                mean_ape=st.mean(errors),within_20=sum(e<=20 for e in errors),worst_ape=max(errors)))
    (OUT/'before_after_metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
    fig,ax=plt.subplots(figsize=(10,1.8+.43*(len(metrics)+1)));ax.axis('off')
    fig.suptitle('DOACROSS accuracy: dependent computation per worker',fontsize=17,weight='bold',color='#16354a',y=.97)
    cells=[[str(r['threads']) if r['threads'] else 'All shared',r['model'],str(r['settings']),
            f"{r['median_ape']:.1f}%",f"{r['mean_ape']:.1f}%",f"{r['within_20']}/{r['settings']}"] for r in metrics]
    table=ax.table(cellText=cells,colLabels=['Workers','Model','Settings','Median absolute\nerror','Mean absolute\nerror','Within 20%'],
        cellLoc='center',bbox=[0,.12,1,.84],colWidths=[.13,.24,.10,.18,.18,.17])
    table.auto_set_font_size(False);table.set_fontsize(11)
    for (row,col),cell in table.get_celld().items():
        cell.set_edgecolor('#d7e1e8')
        if row==0:cell.set_facecolor('#16354a');cell.set_text_props(color='white',weight='bold')
        elif row%2==0:cell.set_facecolor('#e1f1e8')
        else:cell.set_facecolor('#f0f4f7')
    fig.text(.08,.066,'Only settings completed in both runs; failed settings excluded. Each setting has equal weight.',fontsize=9,color='#506676')
    fig.text(.08,.035,'Previous measured model vs fresh per-worker work model. Each uses its own pre-holdout calibration.',fontsize=9,color='#506676')
    fig.savefig(OUT/'tables/before_after_accuracy.png',dpi=160)
    fig.savefig(OUT/'tables/before_after_accuracy.pdf')
    plt.close(fig)
    print(json.dumps(metrics[:2],indent=2))


if __name__=='__main__':
    scope_checks();checks();export_checks();compare()
