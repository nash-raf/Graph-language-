"""Plot current model policy versus the remeasured exhaustive-sweep winner.

Read-only analysis of early-diversity paired CSVs. No benchmarks are rerun.
Model time aggregates all executed native policies; show the modal full policy
with its frequency when choices differ, and retain every policy count in CSV.
"""
import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import json
from pathlib import Path
import statistics as st

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

ROOT=Path(__file__).resolve().parents[2]
DEFAULT=ROOT/'proof/tdg_diverse_allocations_2026_10_09'


def read(path):
    with path.open(newline='') as f: return list(csv.DictReader(f))


def config(row):
    return ':'.join(row[k] for k in ('backend','left','right','order','parents'))


def aggregate(groups):
    return st.median(st.median(v) for v in groups.values())/1e6


def analyze(source):
    times=defaultdict(lambda:defaultdict(list)); choices=defaultdict(Counter)
    for r in read(source/'paired_raw.csv'):
        if r['variant']!='after': continue
        key=r['case'],int(r['budget'])
        times[(*key,r['kind'])][int(r['repeat'])].append(int(r['time_ns']))
        choices[(*key,r['kind'])][config(r)]+=1
    summaries={(r['case'],int(r['budget'])):r for r in read(source/'paired_summary.csv')}
    output=[]
    for case,budget in sorted(summaries):
        key=case,budget
        native=choices[(*key,'native')]; best=choices[(*key,'fixed_winner')]
        assert len(best)==1, (key,best)
        modal=min(native,key=lambda p:(-native[p],p)); winner=next(iter(best))
        model_ms=aggregate(times[(*key,'native')]); best_ms=aggregate(times[(*key,'fixed_winner')])
        saved=summaries[key]
        assert abs(model_ms-float(saved['after_native_ms']))<1e-10
        assert abs(best_ms-float(saved['after_fixed_winner_ms']))<1e-10
        output.append(dict(case=case,budget=budget,model_modal_policy=modal,
            model_modal_fraction=native[modal]/sum(native.values()),
            model_policy_counts=json.dumps(dict(sorted(native.items()))),
            model_time_ms=model_ms,sweep_best_policy=winner,best_remeasured_time_ms=best_ms,
            extra_time_ms=model_ms-best_ms,gap_percent=100*(model_ms/best_ms-1),
            native_processes=len(times[(*key,'native')]),best_definition='Previously exhaustive sweep winner, remeasured'))
    return output


def metadata():
    result={}
    for folder in ('tdg_allocation_sweep','tdg_dynamic_default_2026_10_09/mixed'):
        meta=json.loads((ROOT/'proof'/folder/'mixed_manifest.json').read_text())
        for case in meta['cases']: result[case['name']]=case
    return result


def policy_text(case,encoded,cases):
    backend,left,right,order,parents=map(int,encoded.split(':'))
    width=f'({left}, {right})'
    if backend==0:
        return f'Ordinary-first {width}\nparents={parents}'
    if backend==2:
        return f'FIFO {width}\nparents={parents}'
    if case.startswith('mixed_'):
        sequence=cases[case]['orders'][order//2]
        priority=''.join('L' if i==0 else 'R' if i==1 else chr(ord('a')+i-2) for i in sequence)
        backfill=order%2
    else:
        assert order in (0,1)
        priority='LR' if order==0 else 'RL';backfill=1
    return f'Dynamic {width}\nP={parents}; order={priority}; BF={backfill}'


def render(rows,title,cases):
    height=2.45+.64*(len(rows)+1)
    fig=plt.figure(figsize=(18,height),facecolor='white')
    fig.text(.025,1-.43/height,title,fontsize=18,weight='bold',color='#16354a')
    fig.text(.025,1-.79/height,'Current model after early-diversity change. Gap = 100 × (model time / best time − 1).',
             fontsize=11,color='#506676')
    ax=fig.add_axes([.025,1.4/height,.95,1-2.5/height]);ax.axis('off')
    headers=['Workload','Worker\nbudget','Model allocated policy\n(modal if varying)',
             'Model time\n(ms)','Sweep-best policy\n(remeasured)','Best time\n(ms)',
             'Extra time\n(ms)','Gap from\nbest (%)']
    cells=[]
    for r in rows:
        name=r['case'].replace('mixed_','Mixed: ').replace('_',' ')
        chosen=policy_text(r['case'],r['model_modal_policy'],cases)
        if r['model_modal_fraction']<1:
            chosen+=f'\n* {100*r["model_modal_fraction"]:.0f}% of native samples'
        cells.append([name,str(r['budget']),chosen,f'{r["model_time_ms"]:.4f}',
            policy_text(r['case'],r['sweep_best_policy'],cases),f'{r["best_remeasured_time_ms"]:.4f}',
            f'{r["extra_time_ms"]:+.4f}',f'{r["gap_percent"]:+.1f}%'])
    table=ax.table(cellText=cells,colLabels=headers,cellLoc='center',
        colWidths=[.19,.045,.235,.0775,.235,.0775,.07,.07],bbox=[0,0,1,1])
    table.auto_set_font_size(False);table.set_fontsize(10.5)
    for (row,col),cell in table.get_celld().items():
        cell.set_linewidth(.5);cell.set_edgecolor('#d7e1e8')
        if row==0:
            cell.set_facecolor('#16354a');cell.set_text_props(color='white',weight='bold')
        else:
            cell.set_facecolor('#f0f4f7' if row%2==0 else 'white')
            cell.set_text_props(color='#20364a')
            if col==0: cell.set_text_props(ha='left');cell.PAD=.04
            if col==7:
                gap=rows[row-1]['gap_percent']
                fill,color=('#e1f1e8','#17644a') if gap<=10 else (
                    ('#fff0d6','#82570d') if gap<=25 else ('#fbe4e2','#9f2f26'))
                cell.set_facecolor(fill);cell.set_text_props(color=color,weight='bold')
    notes=[
        'Policy: (left, right) loop widths; P = parent-worker limit; order = task priority; BF = backfill (1 on, 0 off).',
        'L/R = loops; a/b/c = ordinary tasks. Priority order is not a serial execution sequence. Width 1 runs inline.',
        'Times: median of process medians. Native time includes policy overhead. * = varying policies; time includes all native choices.',
        'Best is the prior exhaustive-sweep winner, remeasured here. No new full sweep; negative gaps can reflect timing variation or a better native policy.',
    ]
    for offset,note in zip((1.08,.82,.56,.30),notes):
        fig.text(.025,offset/height,note,fontsize=9,color='#506676')
    return fig


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source',type=Path,default=DEFAULT)
    p.add_argument('--out',type=Path)
    args=p.parse_args();source=args.source.resolve();out=(args.out or source/'model_vs_best').resolve()
    out.mkdir(parents=True,exist_ok=True)
    rows=analyze(source);cases=metadata()
    with (out/'model_vs_best.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    groups=[]
    mixed=[r for r in rows if r['case'].startswith('mixed_')]
    loops=[r for r in rows if not r['case'].startswith('mixed_')]
    if len(rows)<=8:
        groups=[('confirmation',rows,'Model policy vs sweep-best policy — repeat measurements')]
    else:
        if mixed: groups.append(('mixed',mixed,'Model policy vs sweep-best policy — mixed workloads'))
        for offset in range(0,len(loops),12):
            groups.append((f'loops.page{offset//12+1}',loops[offset:offset+12],
                           f'Model policy vs sweep-best policy — loop workloads ({offset//12+1}/3)'))
    with PdfPages(out/'model_vs_best.pdf') as pdf:
        for stem,chunk,title in groups:
            fig=render(chunk,title,cases)
            fig.savefig(out/(stem+'.png'),dpi=170)
            fig.savefig(out/(stem+'.svg'));pdf.savefig(fig);plt.close(fig)
    manifest=dict(source=str(source),rows=len(rows),images=[stem+'.png' for stem,_,_ in groups],
        definition='Prior exhaustive winner, remeasured against current model in fresh paired processes; not a new full sweep.',
        formula='100 * (model_time / best_remeasured_time - 1)',
        source_sha256={name:hashlib.sha256((source/name).read_bytes()).hexdigest()
                       for name in ('paired_raw.csv','paired_summary.csv')})
    (out/'analysis.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(json.dumps(manifest,indent=2))


if __name__=='__main__': main()
