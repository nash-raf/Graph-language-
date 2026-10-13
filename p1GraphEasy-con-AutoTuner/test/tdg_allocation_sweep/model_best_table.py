"""Render measured model-versus-oracle comparison tables without rerunning benchmarks.

Use Python with Matplotlib (available in WSL). Input CSVs are read-only. Aggregate each
configuration as median(process medians), exactly as the benchmark exporter does.
The common oracle is the fastest measured forced configuration under either
policy. Shared mixed-oracle rows are references to the same physical timings.
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

ROOT = Path(__file__).resolve().parents[2]
DEFAULT = ROOT/'proof/tdg_dynamic_default_2026_10_09/mixed'


def read_csv(path):
    with path.open(newline='') as stream:
        return list(csv.DictReader(stream))


def process_median(groups):
    return st.median(st.median(times) for times in groups.values())


def analyze(source):
    native = defaultdict(lambda: defaultdict(list))
    forced = defaultdict(lambda: defaultdict(list))
    choices = defaultdict(Counter)
    with (source/'raw.csv').open(newline='') as stream:
        for row in csv.DictReader(stream):
            key = row['case'], int(row['budget']), row['policy']
            repeat = int(row['repeat'])
            if row['kind'] == 'model':
                native[key][repeat].append(int(row['time_ns']))
                config = ':'.join(row[name] for name in (
                    'backend', 'actual_left', 'actual_right', 'order', 'parents'))
                choices[key][config] += 1
            else:
                forced[(*key, row['config'])][repeat].append(int(row['time_ns']))
    oracle = {}
    per_policy_best = {}
    for (case, budget, policy, config), times in forced.items():
        candidate = process_median(times), config, policy
        key = case, budget
        if key not in oracle or candidate < oracle[key]:
            oracle[key] = candidate
        pkey = case, budget, policy
        if pkey not in per_policy_best or candidate < per_policy_best[pkey]:
            per_policy_best[pkey] = candidate
    summaries = {(r['case'], int(r['budget']), r['policy']): r
                 for r in read_csv(source/'summary.csv')}
    assert set(summaries) == set(native)
    results = []
    for key in sorted(native):
        case, budget, policy = key
        model_ms = process_median(native[key])/1e6
        summary = summaries[key]
        assert abs(model_ms-float(summary['model_ms'])) < 1e-10, key
        assert abs(per_policy_best[key][0]/1e6-float(summary['best_ms'])) < 1e-10, key
        best_ns, config, observed_policy = oracle[case, budget]
        backend, left, right, order, parents = map(int, config.split(':'))
        pairs = Counter()
        for chosen, count in choices[key].items():
            parts = chosen.split(':')
            pairs[int(parts[1]), int(parts[2])] += count
        # Tie-break deterministically. All choices and full schedules stay in CSV.
        modal = min(pairs, key=lambda pair: (-pairs[pair], pair))
        best_ms = best_ns/1e6
        results.append(dict(
            case=case, budget=budget, policy=policy,
            model_modal_left=modal[0], model_modal_right=modal[1],
            model_modal_fraction=pairs[modal]/sum(pairs.values()),
            model_choice_counts=json.dumps({f'{a},{b}': n for (a,b),n in sorted(pairs.items())}),
            model_schedule_counts=json.dumps(dict(sorted(choices[key].items()))),
            best_left=left, best_right=right, best_backend=backend,
            best_order_code=order, best_parents=parents, best_config=config,
            best_observed_under_policy=observed_policy,
            model_time_ms=model_ms, best_measured_time_ms=best_ms,
            extra_time_ms=model_ms-best_ms,
            slowdown_percent=100*(model_ms/best_ms-1),
            observed_model_pairs=len(pairs),
            ordinary_tasks=int(summary.get('ordinary_tasks', 0)),
            processes=len(native[key]),
        ))
    return results


def label(case):
    labels = {
        'mixed_compute': 'Compute loops + short tasks',
        'mixed_loop_skew': 'Unequal loops + short tasks',
        'mixed_one_long_ordinary': 'One long ordinary task',
        'mixed_short_ordinary': 'Three short ordinary tasks',
    }
    return labels.get(case, case.replace('mixed_', 'Mixed: ').replace('_', ' ').capitalize())


def render(rows, revision, page, pages):
    height = 2.45 + .43*(len(rows)+1)
    fig = plt.figure(figsize=(16, height), facecolor='white')
    policy = rows[0]['policy']
    name = 'Dynamic allocation + scheduling' if policy == 'dynamic' else 'Ordinary-first reference'
    fig.text(.035, 1-.43/height, name+' vs best measured allocation',
             fontsize=19, weight='bold', color='#17334b')
    subtitle = f'{revision}  |  {len(rows)} settings shown'
    if pages > 1:
        subtitle += f'  |  page {page}/{pages}'
    fig.text(.035, 1-.76/height, subtitle, fontsize=11, color='#546777')
    bottom, top = 1.30/height, 1-1.05/height
    ax = fig.add_axes([.035, bottom, .93, top-bottom])
    ax.axis('off')
    headers = ['Workload', 'Worker\nbudget', 'Model loop\nwidths (L,R)',
               'Best loop\nwidths (L,R)', 'Model time\n(ms)', 'Best time\n(ms)',
               'Extra time\n(ms)', 'Slower than\nbest (%)']
    cells = [[label(r['case']), str(r['budget']),
              f'({r["model_modal_left"]}, {r["model_modal_right"]})'+
              (' *' if r['observed_model_pairs'] > 1 else ''),
              f'({r["best_left"]}, {r["best_right"]})',
              f'{r["model_time_ms"]:.4f}', f'{r["best_measured_time_ms"]:.4f}',
              f'{r["extra_time_ms"]:+.4f}', f'{r["slowdown_percent"]:+.1f}%'] for r in rows]
    table = ax.table(cellText=cells, colLabels=headers, cellLoc='center',
                     colWidths=[.25,.065,.125,.125,.11,.11,.10,.115], bbox=[0,0,1,1])
    table.auto_set_font_size(False)
    table.set_fontsize(11)
    for (row, col), cell in table.get_celld().items():
        cell.set_linewidth(.6)
        cell.set_edgecolor('#d9e1e8')
        if row == 0:
            cell.set_facecolor('#17334b')
            cell.set_text_props(color='white', weight='bold', fontsize=10.5)
        else:
            cell.set_facecolor('#f0f4f7' if row%2 == 0 else 'white')
            cell.set_text_props(color='#20364a')
            if col == 0:
                cell.set_text_props(ha='left')
                cell.PAD = .04
            elif col == 7:
                gap = rows[row-1]['slowdown_percent']
                fill, color = ('#e0f0e8','#17644a') if gap <= 10 else (
                    ('#fff0d6','#82570d') if gap <= 25 else ('#fbe4e2','#9f2f26'))
                cell.set_facecolor(fill)
                cell.set_text_props(color=color, weight='bold')
    notes = [
        'Gap = (model time / best measured time - 1) x 100. Times are medians of process medians.',
        'Best time includes the best measured order and parent-worker limit; model time includes planning. Width 1 means serial.',
        'Loop teams may run at different times. * = modal pair shown when choices vary; the CSV includes every observed choice.',
    ]
    for y, text in zip((.95,.67,.39), notes):
        fig.text(.035, y/height, text, fontsize=9.5, color='#546777')
    return fig


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=DEFAULT)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--revision', default='Post-integration runtime | exhaustive allocation sweep')
    parser.add_argument('--rows-per-page', type=int, default=14)
    args = parser.parse_args()
    source = args.source.resolve()
    out = (args.out or source/'model_vs_best').resolve()
    out.mkdir(parents=True, exist_ok=True)
    results = analyze(source)
    with (out/'model_vs_best.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(results[0]))
        writer.writeheader()
        writer.writerows(results)
    metrics = {}
    with PdfPages(out/'model_vs_best.pdf') as pdf:
        for policy in ('dynamic', 'ordinary-first'):
            selected = [r for r in results if r['policy'] == policy]
            metrics[policy] = dict(
                settings=len(selected),
                mean_slowdown_percent=st.mean(r['slowdown_percent'] for r in selected),
                sum_time_slowdown_percent=100*(sum(r['model_time_ms'] for r in selected)/
                                             sum(r['best_measured_time_ms'] for r in selected)-1),
                within_10_percent=sum(r['slowdown_percent'] <= 10 for r in selected),
            )
            pages = (len(selected)+args.rows_per_page-1)//args.rows_per_page
            for page in range(pages):
                chunk = selected[page*args.rows_per_page:(page+1)*args.rows_per_page]
                fig = render(chunk, args.revision, page+1, pages)
                stem = policy+(f'.page{page+1}' if pages > 1 else '')
                fig.savefig(out/(stem+'.png'), dpi=160)
                fig.savefig(out/(stem+'.svg'))
                pdf.savefig(fig)
                plt.close(fig)
    manifest = dict(source=str(source), revision=args.revision, metrics=metrics,
                    aggregation='Median of per-process medians; fastest forced configuration across policy labels.',
                    source_sha256={name: hashlib.sha256((source/name).read_bytes()).hexdigest()
                                   for name in ('raw.csv','summary.csv')})
    (out/'analysis.json').write_text(json.dumps(manifest, indent=2)+'\n')
    (out/'README.txt').write_text(
        'Read-only CSV analysis; no benchmark or production sources changed.\n'
        'Best means fastest measured forced allocation + order + parent-worker limit, not an analytic optimum.\n'
        'Times: median of process medians. Gap (%) = 100 * (model time / best time - 1).\n'
        'Width 1 is serial. Teams need not coexist; each ordinary task uses one parent.\n'
        'PNG tables show modal native loop widths; CSV also stores all observed pairs and schedules with counts.\n'
        'For varying choices, model time aggregates actual native decisions rather than only the modal allocation.\n'
        'Configuration: backend:left_width:right_width:order_code:parent_limit.\n'
        'Mixed order_code = 2 * sequence_index + backfill; sequence list is in mixed_manifest.json.\n'
        'Backend 0 = ordinary-first backup; 1 = dynamic executor; 2 = FIFO (new revision only).\n'
        'Both policy tables share the same measured oracle. Shared mixed forced rows are counted as one result.\n')
    print(json.dumps(metrics, indent=2))


if __name__ == '__main__':
    main()
