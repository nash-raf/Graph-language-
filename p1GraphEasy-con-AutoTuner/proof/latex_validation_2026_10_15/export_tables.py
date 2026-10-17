"""Export historical benchmark results to separate LaTeX table snippets.

Run from any directory with Python 3. No benchmarks or production code are run.
"""
from pathlib import Path
import csv
import json
import math
import statistics as st

OUT = Path(__file__).resolve().parent
ROOT = OUT.parent.parent
SOURCES = {}

def read(path):
    SOURCES[path] = None
    with (ROOT / path).open(newline='', encoding='utf-8-sig') as f:
        return list(csv.DictReader(f))

def esc(s):
    return str(s).replace('_', r'\_').replace('%', r'\%')

def table(caption, label, headers, rows, notes='', long=False):
    env = 'longtable' if long else 'tabular'
    spec = 'l' + 'r' * (len(headers)-1)
    body = ['% Requires booktabs; detailed tables also require longtable.',
            r'\begin{center}' if long else r'\begin{table}[htbp]\centering',
            r'\small', r'\begin{' + env + '}{' + spec + '}']
    if long:
        body += [r'\caption{' + caption + r'}\label{' + label + r'}\\']
    head = ' & '.join(headers) + r' \\'
    body += [r'\toprule', head, r'\midrule']
    if long:
        body += [r'\endfirsthead', r'\toprule', head, r'\midrule', r'\endhead',
                 r'\bottomrule\endfoot']
    body += [' & '.join(map(str, row)) + r' \\' for row in rows]
    body += [r'\bottomrule', r'\end{' + env + '}']
    if not long:
        body += [r'\caption{' + caption + r'}\label{' + label + '}']
    if notes:
        body += [r'\par\medskip\begin{minipage}{0.98\linewidth}\footnotesize ' + notes + r'\end{minipage}']
    body += [r'\end{center}' if long else r'\end{table}', '']
    return '\n'.join(body)

def gm(v):
    return math.exp(st.mean(math.log(x) for x in v))

# Universal budgeting: matched policy ablations, then allocation validation.
ab = read('proof/tdg_joint_schedule/overhead_revision/benchmark.csv')
groups = {}
for r in ab:
    groups.setdefault((int(r['p']), r['case']), {}).setdefault(r['policy'], []).append(r)
policies = [('reference', 'Ordinary-first reference'), ('ordering', 'Ordering only'),
            ('overload', 'Joint search only on overload'), ('joint', 'Joint allocation and ordering')]
rows = []
for policy, name in policies:
    ratios = []
    for arms in groups.values():
        ratios.append(st.median(float(r['steady_ns']) for r in arms[policy]) /
                      st.median(float(r['steady_ns']) for r in arms['reference']))
    paired = []
    cold = []
    for arms in groups.values():
        refs = {r['run']: r for r in arms['reference']}
        for r in arms[policy]:
            paired.append(float(r['all14_ns']) / float(refs[r['run']]['all14_ns']))
            cold.append(float(r['first_ns']) / float(refs[r['run']]['first_ns']))
    rows.append([name, len(groups), f'{gm(ratios):.3f}', f'{gm(cold):.3f}', f'{gm(paired):.3f}'])
utb_summary = table('Universal thread budgeting ablation: geometric mean runtime ratios relative to the ordinary-first reference.',
    'tab:utb-ablation', ['Policy', 'Settings', 'Steady ratio', 'First-call ratio', '14-call ratio'], rows,
    r'Lower ratios are better. Steady ratios use per-setting process medians; first-call and 14-call ratios use paired process runs. These are historical Phase A measurements, not measurements of the later diverse-allocation revision.')
utb = utb_summary
detail = []
for (b, case), arms in sorted(groups.items()):
    times = {p: st.median(float(r['steady_ns']) for r in arms[p])/1e6 for p, _ in policies}
    detail.append([esc(case), b] + [f'{times[p]:.3f}' for p, _ in policies])
utb += table('Universal budgeting ablation: steady execution time in milliseconds.', 'tab:utb-ablation-detail',
    ['Workload', '$B$', 'Ordinary-first', 'Order only', 'Overload only', 'Joint'], detail, long=True)
original = read('proof/tdg_allocation_sweep/model_vs_best/model_vs_best.csv')
comparison = []
for p in ('ordinary-first', 'dynamic'):
    rr = [r for r in original if r['policy'] == p]
    v = [float(r['slowdown_percent']) for r in rr]
    comparison.append([esc(p), len(v), f'{st.mean(v):.2f}', f'{st.median(v):.2f}', f'{max(v):.2f}'])
utb += table('Allocation validation before the diverse-allocation revision: excess runtime relative to the common measured sweep winner.',
    'tab:utb-original-validation', ['Policy', 'Settings', 'Mean gap (\%)', 'Median gap (\%)', 'Worst gap (\%)'], comparison,
    r'Gap $=100(T_{\mathrm{model}}/T_{\mathrm{sweep}}-1)$. The oracle is the fastest measured forced policy in the sweep; it is not a proven global optimum.')
latest = read('proof/tdg_diverse_allocations_2026_10_09/model_vs_best/model_vs_best.csv')
utb += table('Diverse-allocation revision: native model decisions versus remeasured prior sweep winners. Times are milliseconds.',
    'tab:utb-diverse-validation', ['Workload', '$B$', 'Model widths', 'Best widths', 'Model', 'Best', 'Gap (\%)'],
    [[esc(r['case']), r['budget'], '('+','.join(r['model_modal_policy'].split(':')[1:3])+')',
      '('+','.join(r['sweep_best_policy'].split(':')[1:3])+')', f"{float(r['model_time_ms']):.3f}",
      f"{float(r['best_remeasured_time_ms']):.3f}", f"{float(r['gap_percent']):+.2f}"] for r in latest],
    r'Widths are modal loop-worker allocations; width 1 means serial. Times include all observed native decisions. The complete policy also includes executor, order, backfill and parent limit. A negative gap means the model was faster than the remeasured earlier winner; timing variation and the changed policy can cause this.', long=True)
confirmation = read('proof/tdg_diverse_allocations_2026_10_09/confirmation/paired_summary.csv')
utb += table('Paired confirmation of diverse allocation seeds. Times are milliseconds.', 'tab:utb-diverse-confirmation',
    ['Workload', '$B$', 'Before', 'After', 'Sweep winner', 'Change (\%)'],
    [[esc(r['case']), r['budget'], f"{float(r['before_native_ms']):.3f}", f"{float(r['after_native_ms']):.3f}",
      f"{float(r['after_fixed_winner_ms']):.3f}", f"{100*(float(r['after_over_before'])-1):+.2f}"] for r in confirmation],
    r'Change is after relative to before; negative is faster. The eight-worker large-array case regresses and is retained.')

# DOALL: the dedicated historical cost audit, not scheduler timings.
path = 'proof/cost_model_audit/raw.csv'
SOURCES[path] = None
raw = list(csv.reader(line for line in (ROOT/path).read_text().splitlines() if line and not line.startswith('#')))
alls = [list(map(float, r[1:])) for r in raw if r[0] == 'all']
doall_rows = []
old_errors, new_errors = [], []
for p,n,chain,stream,serial,pred,actual,launch,measured_launch in alls:
    old = 1200+180*p+serial/p
    old_errors.append(abs(old/actual-1)*100)
    new_errors.append(abs(pred/actual-1)*100)
    doall_rows.append([int(p), int(n), int(chain), int(stream), f'{pred/1e6:.4f}',
                      f'{actual/1e6:.4f}', f'{100*(pred/actual-1):+.2f}', f'{abs(pred/actual-1)*100:.2f}'])
doall_summary = table('DOALL cost-model validation: historical audit with identical inputs for both equations.', 'tab:doall-summary',
    ['Equation', 'Settings', 'Mean APE (\%)', 'Median APE (\%)', 'Within 20\%'],
    [[name, len(v), f'{st.mean(v):.2f}', f'{st.median(v):.2f}', f'{sum(x<=20 for x in v)}/{len(v)}']
     for name,v in [('Original',old_errors),('Corrected',new_errors)]],
    r'APE $=100|\widehat T-T|/T$. These are the existing 18 audit measurements; they do not constitute a fresh validation of any changes made after that audit.')
doall = doall_summary + table('DOALL corrected cost predictions versus measured parallel execution. Times are milliseconds.',
    'tab:doall-detail', ['$P$', '$N$', 'Chain', 'Streaming', 'Predicted', 'Measured', 'Error (\%)', 'APE (\%)'], doall_rows,
    r'Chain is the benchmark arithmetic-chain parameter; streaming is a Boolean workload flag. Signed error $=100(\widehat T/T-1)$.', long=True)

# DOACROSS: latest completed equation validation, all worker counts retained.
across = read('proof/doacross_cost_worker_work_2026_10_09/summary.csv')
width = read('proof/doacross_cost_worker_work_2026_10_09/accuracy_by_threads.csv')
learned = [r for r in across if r['stage']=='learned-sync' and r['validation_status']=='PASS']
doacross_summary = table('DOACROSS cost-model validation after removing the separate global dependency charge.', 'tab:doacross-summary',
    ['Workers', 'Valid settings', 'Mean APE (\%)', 'Median APE (\%)', 'Within 20\%'],
    [[r['threads'], r['valid_settings'], f"{float(r['mean_absolute_error_percent']):.2f}",
      f"{float(r['median_absolute_error_percent']):.2f}", f"{r['within_20_percent']}/{r['valid_settings']}"]
     for r in width if r['stage']=='learned-sync'] +
    [['All', len(learned), f"{st.mean(float(r['absolute_error_percent']) for r in learned):.2f}",
      f"{st.median(float(r['absolute_error_percent']) for r in learned):.2f}",
      f"{sum(float(r['absolute_error_percent'])<=20 for r in learned)}/{len(learned)}"]],
    r'Learned synchronization costs. Each completed setting uses three processes and seven holdout timing samples per process. The uneven workload at six workers timed out; it is excluded from accuracy aggregates, not counted as a successful prediction.')
doacross = doacross_summary
stage_rows = []
for stage in ('seeded-sync','learned-sync'):
    rr = [r for r in across if r['stage']==stage and r['validation_status']=='PASS']
    vv = [float(r['absolute_error_percent']) for r in rr]
    stage_rows.append([esc(stage),len(vv),f'{st.mean(vv):.2f}',f'{st.median(vv):.2f}',f'{max(vv):.2f}'])
doacross += table('DOACROSS calibration ablation on the same holdout measurements.', 'tab:doacross-calibration',
    ['Synchronization inputs','Settings','Mean APE (\%)','Median APE (\%)','Worst APE (\%)'], stage_rows)
doacross += table('DOACROSS predictions using learned synchronization costs. Times are milliseconds.', 'tab:doacross-detail',
    ['Workload','$P$','Predicted','Measured','Error (\%)','APE (\%)'],
    [[esc(r['case']),r['threads'],f"{float(r['predicted_ms']):.4f}",f"{float(r['measured_ms']):.4f}",
      f"{float(r['signed_error_percent']):+.2f}",f"{float(r['absolute_error_percent']):.2f}"] for r in learned] +
    [[r'uneven',6,'---','Timeout','---','---']],
    r'Signed error $=100(\widehat T/T-1)$; negative means underprediction. The timeout has no valid prediction-accuracy result.', long=True)
for name, text in [('universal_thread_budgeting',utb),('doall_cost_model',doall),('doacross_cost_model',doacross)]:
    (OUT/(name+'.tex')).write_text(text,encoding='utf-8')
for name, text in [('utb_summary',utb_summary),('doall_summary',doall_summary),('doacross_summary',doacross_summary)]:
    (OUT/(name+'.txt')).write_text(text,encoding='utf-8')
(OUT/'sources.json').write_text(json.dumps(list(SOURCES),indent=2)+'\n')
print(utb_summary + '\n' + doall_summary + '\n' + doacross_summary)
print('Exported 3 LaTeX snippets. Detail rows: budgeting ablation=36, latest validation='+str(len(latest))+', DOALL='+str(len(alls))+', DOACROSS='+str(len(learned))+' valid + 1 timeout.')
