#!/usr/bin/env python3
"""Evaluate dependency-term alternatives on frozen, matched-width holdout data.

No production changes or new executions. No fitted coefficients. Blocking
wait measurements come from training, not from the holdout timings.
"""
import csv
import json
from collections import defaultdict
from pathlib import Path
import statistics as st

ROOT=Path(__file__).resolve().parents[2]
VALIDATION=ROOT/'proof/doacross_cost_updated_2026_10_09'
OUT=VALIDATION/'dependency_term_ablation'


def main():
    OUT.mkdir(exist_ok=True)
    valid={(r['case'],int(r['threads'])) for r in csv.DictReader((VALIDATION/'summary.csv').open())
           if r['stage']=='learned-sync' and r['validation_status']=='PASS'}
    grouped=defaultdict(lambda:defaultdict(list))
    for r in csv.DictReader((VALIDATION/'raw.csv').open()):
        p=int(r['threads']);n=int(r['trips'])
        if r['stage']!='learned-sync' or (r['case'],p) not in valid:continue
        dep,ind,wait,post,launch=[float(r[k]) for k in
            ('c_dep_ns','c_ind_ns','sigma_wait_ns','sigma_post_ns','launch_ns')]
        lane=(n+p-1)//p
        sync=int(r['streams'])*(wait+post)
        candidates={'current':float(r['prediction_ns']),
                    'dependent_work_per_lane':launch+lane*(dep+ind+sync),
                    'delete_all_dependent_work':launch+lane*(ind+sync)}
        for equation,prediction in candidates.items():
            grouped[r['case'],p,equation][int(r['repeat'])].append((prediction,float(r['actual_ns'])))
    rows=[]
    for (case,p,equation),processes in sorted(grouped.items()):
        predicted=st.median(st.median(x[0] for x in rows) for rows in processes.values())
        actual=st.median(st.median(x[1] for x in rows) for rows in processes.values())
        error=100*(predicted/actual-1)
        rows.append(dict(case=case,threads=p,equation=equation,predicted_ms=predicted/1e6,
                         measured_ms=actual/1e6,signed_error_percent=error,absolute_error_percent=abs(error)))
    with (OUT/'comparisons.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
    metrics=[]
    for equation in sorted({r['equation'] for r in rows}):
        for p in [0,*sorted({r['threads'] for r in rows})]:
            selected=[r for r in rows if r['equation']==equation and (not p or r['threads']==p)]
            metrics.append(dict(equation=equation,threads=p,settings=len(selected),
                median_absolute_error_percent=st.median(r['absolute_error_percent'] for r in selected),
                mean_absolute_error_percent=st.mean(r['absolute_error_percent'] for r in selected),
                within_20_percent=sum(r['absolute_error_percent']<=20 for r in selected),
                worst_absolute_error_percent=max(r['absolute_error_percent'] for r in selected)))
    (OUT/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
    print(json.dumps([r for r in metrics if not r['threads']],indent=2))


if __name__=='__main__':main()
