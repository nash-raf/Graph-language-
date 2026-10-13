#!/usr/bin/env python3
"""Compare model-selected widths with legal measured allocations at fixed h=2."""
import csv
import argparse
import os
from pathlib import Path
import random
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'proof/tdg_level_design'

def main():
    cases = [(p,n,chain,ratio) for p in (4,8)
             for n,chain,ratio in ((65536,12,1),(200000,12,3),(1000000,0,1))]
    parser = argparse.ArgumentParser()
    parser.add_argument('--summarize-only',action='store_true')
    if parser.parse_args().summarize_only:
        summarize(cases)
        return
    jobs = []
    for p,n,chain,ratio in cases:
        extra = p-2
        choices = [(a,b) for a in range(1,extra+1) for b in range(1,extra+1)
                   if (a if a>1 else 0)+(b if b>1 else 0)<=extra]
        for run in range(2):
            jobs += [(p,n,chain,ratio,run,a,b) for a,b in choices+[(0,0)]]
    random.Random(4097).shuffle(jobs)
    with (OUT/'sweep.csv').open('w',newline='') as file:
        writer = csv.writer(file)
        writer.writerow(('p','n','chain','ratio','run','forced_a','forced_b','level_ns',
                         'loop_ns','sum_prediction_ns','resource_prediction_ns','chosen_a','chosen_b'))
        for idx,(p,n,chain,ratio,run,a,b) in enumerate(jobs):
            env = dict(os.environ,SGPL_NUM_THREADS=str(p),OMP_NUM_THREADS=str(p))
            for key in ('SGPL_FORCE_WIDTHS','SGPL_TDG_DEBUG','SGPL_BUDGET_DEBUG',
                        'GRAPH_PARALLEL_DEBUG','SGPL_FORCE_DOALL_PARALLEL','SGPL_TDG_PLAN_DUMP'):
                env.pop(key,None)
            # Forced cases always print the model plan. Enable that same
            # logging on free choices so the measured overhead is comparable.
            env['SGPL_TDG_PLAN_DUMP'] = '1'
            if a: env['SGPL_FORCE_WIDTHS'] = f'300:{a},301:{b}'
            result = subprocess.run([str(OUT/'probe_new'),'0','2',str(n),'16',str(ratio),'0',str(chain)],
                                    env=env,capture_output=True,text=True,check=True,timeout=40)
            row = result.stdout.strip().split(',')
            writer.writerow((p,n,chain,ratio,run,a,b,*row[7:11],row[12],row[15]))
            file.flush()
            print(f'{idx+1}/{len(jobs)} P={p} N={n} chain={chain} ratio={ratio} forced={a}+{b}: {row[7]}ns',flush=True)
    summarize(cases)

def summarize(cases):
    rows = list(csv.DictReader((OUT/'sweep.csv').open()))
    with (OUT/'sweep_summary.txt').open('w') as report:
        regrets = []
        for p,n,chain,ratio in cases:
            group = [r for r in rows if tuple(int(r[k]) for k in ('p','n','chain','ratio')) == (p,n,chain,ratio)]
            choices = {}
            for r in group:
                key = (int(r['forced_a']),int(r['forced_b']))
                choices.setdefault(key,[]).append(float(r['level_ns']))
            times = {k:statistics.median(v) for k,v in choices.items()}
            best = min((k for k in times if k!=(0,0)),key=times.get)
            model_widths = [(int(r['chosen_a']),int(r['chosen_b'])) for r in group if r['forced_a']=='0']
            selected_time = statistics.median(times[k] for k in model_widths)
            regret = max(0,selected_time/times[best]-1)*100
            full_gap = max(0,times[(0,0)]/times[best]-1)*100
            regrets.append(regret)
            report.write(f'P={p} N={n} chain={chain} ratio={ratio}: model={model_widths} '
                         f'default_run={times[(0,0)]/1e6:.3f}ms; forced_selected={selected_time/1e6:.3f}ms; '
                         f'measured_best={best} {times[best]/1e6:.3f}ms; '
                         f'width_selection_regret={regret:.1f}%; default_run_gap={full_gap:.1f}%\n')
        report.write(f'Median regret={statistics.median(regrets):.1f}%; max={max(regrets):.1f}%\n')

if __name__ == '__main__':
    main()
