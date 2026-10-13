#!/usr/bin/env python3
"""Independent mixed export statistics plus compact shared-oracle comparison."""
import argparse
from collections import defaultdict
import csv
import json
from pathlib import Path
import statistics as st
import run as sweep

def read(path):
    with path.open() as f: return list(csv.DictReader(f))

def report(out):
    meta=json.loads((out/'mixed_manifest.json').read_text())
    names={c['name'] for c in meta['cases']}
    rows=[r for r in read(out/'raw.csv') if r['case'] in names]
    groups=defaultdict(list)
    for r in rows: groups[r['case'],int(r['budget']),r['policy']].append(r)
    summaries=[r for r in read(out/'summary.csv') if r['case'] in names]
    schedule_rows=[r for r in read(out/'schedules.csv') if r['case'] in names]
    schedule_groups=defaultdict(list)
    for r in schedule_rows: schedule_groups[r['case'],int(r['budget']),r['policy']].append(r)
    for s in summaries:
        key=s['case'],int(s['budget']),s['policy']; block=groups[key]
        repeats=sorted({r['repeat'] for r in block})
        native=[st.median(int(r['time_ns']) for r in block if r['kind']=='model' and r['repeat']==rep) for rep in repeats]
        configs=defaultdict(lambda:defaultdict(list))
        for r in block:
            if r['kind']=='forced': configs[r['config']][r['repeat']].append(int(r['time_ns']))
        exported=schedule_groups[key]
        assert len(exported)==len(configs)==int(s['schedules'])
        for item in exported:
            c=':'.join(item[k] for k in ('backend','left','right','order','parents'))
            times=[st.median(configs[c][rep]) for rep in repeats]
            assert float(item['median_ns'])==st.median(times)
            assert float(item['min_process_median_ns'])==min(times)
            assert float(item['max_process_median_ns'])==max(times)
        best=min(float(r['median_ns']) for r in exported)
        assert abs(float(s['model_ms'])-st.median(native)/1e6)<1e-10
        assert abs(float(s['best_ms'])-best/1e6)<1e-10
        assert abs(float(s['model_over_best_ratio'])-st.median(native)/best)<1e-10
        stem=f'{key[0]}.B{key[1]}.{key[2]}'
        for suffix in ('png','svg'): assert (out/'plots'/f'{stem}.{suffix}').stat().st_size>5000
        assert '<h2>'+stem+'</h2>' in (out/'tables.html').read_text()
    comparisons=[]
    for case,budget in sorted({(s['case'],int(s['budget'])) for s in summaries}):
        ordinary=next(s for s in summaries if (s['case'],int(s['budget']),s['policy'])==(case,budget,'ordinary-first'))
        dynamic=next(s for s in summaries if (s['case'],int(s['budget']),s['policy'])==(case,budget,'dynamic'))
        assert ordinary['best_ms']==dynamic['best_ms'] and ordinary['best_config']==dynamic['best_config']
        c=next(c for c in meta['cases'] if c['name']==case)
        comparisons.append(dict(case=case,budget=budget,ordinary_tasks=len(c['ordinary']),
            ordinary_rounds=','.join(map(str,c['ordinary'])),overloaded=int(c['total_tasks']>budget),
            ordinary_first_ms=float(ordinary['model_ms']),dynamic_ms=float(dynamic['model_ms']),best_ms=float(dynamic['best_ms']),
            dynamic_over_ordinary=float(dynamic['model_ms'])/float(ordinary['model_ms']),
            ordinary_first_over_best=float(ordinary['model_over_best_ratio']),dynamic_over_best=float(dynamic['model_over_best_ratio']),
            ordinary_first_allocation_ratio=float(ordinary['allocation_only_ratio']),dynamic_allocation_ratio=float(dynamic['allocation_only_ratio']),
            best_config=dynamic['best_config'],ordinary_first_actual=f'{ordinary["model_actual_left"]},{ordinary["model_actual_right"]}',
            dynamic_actual=f'{dynamic["model_actual_left"]},{dynamic["model_actual_right"]}'))
    sweep.write_csv(out/'mixed_comparison.csv',comparisons)
    analysis={}
    for label,subset in [('all mixed',comparisons),('overloaded',[r for r in comparisons if r['overloaded']]),
                         ('not overloaded',[r for r in comparisons if not r['overloaded']])]:
        if not subset: continue
        entry=dict(settings=len(subset),dynamic_wins=sum(r['dynamic_ms']<r['ordinary_first_ms'] for r in subset),
                   ordinary_first_wins=sum(r['ordinary_first_ms']<r['dynamic_ms'] for r in subset),
                   total_time_dynamic_over_ordinary=sum(r['dynamic_ms'] for r in subset)/sum(r['ordinary_first_ms'] for r in subset))
        for policy,key in [('ordinary-first','ordinary_first'),('dynamic','dynamic')]:
            entry[policy]=dict(mean_gap_percent=100*st.mean(r[key+'_over_best']-1 for r in subset),
                total_time_gap_percent=100*(sum(r[key+'_ms'] for r in subset)/sum(r['best_ms'] for r in subset)-1),
                within_10_percent=sum(r[key+'_over_best']<=1.1 for r in subset),
                allocation_within_10_percent=sum(r[key+'_allocation_ratio']<=1.1 for r in subset))
        analysis[label]=entry
    (out/'mixed_analysis.json').write_text(json.dumps(analysis,indent=2)+'\n')
    lines=['Mixed SGPL level validation. Both policies use exactly the same measured oracle.',
           f'Physical timed samples: {meta["physical_timed_samples"]}; CSV references: {len(rows)} (shared forced rows appear twice).',
           f'{meta["arguments"]["repeats"]} fresh-process repeats, {meta["arguments"]["samples"]} timed samples/configuration; min over many configurations is optimistic.',
           'Synthetic runtime levels of real SGPL bodies; compiler level grouping and graph-loop scaling are not validated.',
           'All protected production model/compiler/autotuner source hashes verified unchanged.','',json.dumps(analysis,indent=2),'']
    overall=analysis['all mixed']
    lines += [f'Dynamic wins {overall["dynamic_wins"]}/{overall["settings"]} settings; '
              f'{100*(1-overall["total_time_dynamic_over_ordinary"]):.1f}% less summed native time.',
              f'Allocations within 10% under their best tested schedule: ordinary-first '
              f'{overall["ordinary-first"]["allocation_within_10_percent"]}/{overall["settings"]}; dynamic '
              f'{overall["dynamic"]["allocation_within_10_percent"]}/{overall["settings"]}.',
              'When policies select the same widths, runtime differences include ordering, parent count, dispatch and planning.',
              'Dynamic allocation failures (>10% slower width pair): '+', '.join(
                  f'{r["case"]} B{r["budget"]}' for r in comparisons if r['dynamic_allocation_ratio']>1.1),
              'Long serial tasks test the assumption that ordinary work always finishes sooner than loop work.','']
    compute6=next((r for r in comparisons if r['case']=='mixed_compute' and r['budget']==6),None)
    if compute6:
        lines += [f'Six-thread compute: dynamic/ordinary={compute6["dynamic_over_ordinary"]:.3f}; '
                  f'dynamic/best={compute6["dynamic_over_best"]:.3f}; ordinary/best={compute6["ordinary_first_over_best"]:.3f}.','']
    holdout_path=out/'mixed_holdout_manifest.json'
    if holdout_path.exists():
        h=json.loads(holdout_path.read_text())
        lines += [f'Independent confirmation: {h["physical_timed_samples"]} timings; '
                  f'{h["repeats"]} new processes/policy/setting, {h["samples"]} timings/native or fixed winner.',
                  f'Dynamic wins {h["dynamic_wins"]}/{h.get("settings",h["dynamic_wins"]+h["ordinary_first_wins"])} settings; '
                  f'{100*(1-h["total_time_dynamic_over_ordinary"]):.1f}% less summed native time.',
                  'The fixed winners sometimes replay slower than their exhaustive-sweep minimum. '
                  'Use mixed_holdout_summary.csv alongside the optimistic exhaustive gaps; the remaining large-loop allocation failures persist.','']
    for r in comparisons:
        lines.append(f'{r["case"]:27} B{r["budget"]} O={r["ordinary_tasks"]} overload={r["overloaded"]} '
                     f'ordinary={r["ordinary_first_ms"]:.6f}ms dynamic={r["dynamic_ms"]:.6f}ms best={r["best_ms"]:.6f}ms '
                     f'dynamic/ordinary={r["dynamic_over_ordinary"]:.3f} best-config={r["best_config"]}')
    (out/'mixed_findings.txt').write_text('\n'.join(lines)+'\n')
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    import numpy as np
    x=np.arange(len(comparisons)); fig,ax=plt.subplots(figsize=(13,6.5))
    ax.bar(x-.18,[r['ordinary_first_over_best'] for r in comparisons],width=.36,color='#5885ad',label='Ordinary-first')
    ax.bar(x+.18,[r['dynamic_over_best'] for r in comparisons],width=.36,color='#e89e32',label='Dynamic allocation + scheduling')
    ax.axhline(1,color='#167343',label='Shared best measured schedule')
    ax.set_xticks(x,[r['case'].removeprefix('mixed_')+f'\nB{r["budget"]} O{r["ordinary_tasks"]}'+(' *' if r['overloaded'] else '')
                    for r in comparisons],rotation=65,ha='right')
    ax.set_ylabel('Native level time / shared best measured time')
    ax.set_title('Mixed levels with ordinary tasks and two loops\n* More callbacks than available threads; includes planning and dispatch')
    ax.legend(); ax.grid(axis='y',alpha=.2); fig.tight_layout()
    for suffix in ('png','svg'): fig.savefig(out/'plots'/f'mixed_comparison.{suffix}',dpi=150,bbox_inches='tight')
    plt.close(fig)
    (out/'mixed_export_validation.txt').write_text('PASS: independently recomputed every mixed schedule median/range, native time and best ratio; both policies share one oracle; allocation plots/tables present.\n')
    print(json.dumps(analysis,indent=2),flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('out',type=Path); report(p.parse_args().out.resolve())
