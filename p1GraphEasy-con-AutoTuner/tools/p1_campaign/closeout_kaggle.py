"""Verify downloaded campaign evidence and write a readable complete time table."""
from pathlib import Path
import argparse,collections,html,json,math,statistics
def read(p):return json.loads(Path(p).read_text())
def gm(xs):return math.exp(statistics.mean(math.log(x) for x in xs))
def main():
 p=argparse.ArgumentParser();p.add_argument('evidence');p.add_argument('output');args=p.parse_args()
 root=Path(args.evidence)/'p1-campaign';out=Path(args.output);out.mkdir(parents=True,exist_ok=True)
 result=read(root/'results/comparison.json');manifest=read(root/'results/manifest.json')
 assert len(manifest['cases'])==61
 assert read(root/'results/contracts_passed.json')['passed']
 valid=read(root/'results/validated.json');assert valid['workers']==2
 exact=read(root/'results/exact_pagerank_vectors.json');assert len(exact)==122 and all(x['passed'] for x in exact)
 for phase,n in [('primary',5),('confirmation',20)]:
  rows=read(root/'results'/f'{phase}.json')
  assert len(rows)==(244*5+61)*n
  assert all(not x.get('error') and x['seconds']>0 and x['workers']==2 for x in rows)
  counts=collections.Counter((x['algorithm'],x['graph'],x['label']) for x in rows)
  assert len(counts)==1281 and set(counts.values())=={n}
  assert result['summary'][phase]['valid_for_inference'] and not result['summary'][phase]['aa_failures']
 phase='confirmation';cases=[x for x in result['cases'] if x['phase']==phase]
 assert len(cases)==244 and all(sum(x['algorithm']==a for x in cases)==61 for a in ['bfs','sssp','cc','pagerank'])
 summary=dict(workers=2,primary_measurements=6405,confirmation_measurements=25620,exact_pr_vectors=122,
  candidate_retained=True,accepted_total_ms=sum(x['mean_ms']['accepted'] for x in cases),
  candidate_total_ms=sum(x['mean_ms']['candidate'] for x in cases),graptor_total_ms=sum(x['mean_ms']['graptor'] for x in cases),
  mean_geomean_speedup=result['summary'][phase]['mean_geomean_speedup'],
  median_geomean_speedup=result['summary'][phase]['median_geomean_speedup'],
  confirmed_regressions=[x for x in cases if x['confirmed_regression']],by_algorithm={})
 for a in ['bfs','sssp','cc','pagerank']:
  subset=[x for x in cases if x['algorithm']==a]
  summary['by_algorithm'][a]=dict(accepted_ms=sum(x['mean_ms']['accepted'] for x in subset),candidate_ms=sum(x['mean_ms']['candidate'] for x in subset),
   graptor_ms=sum(x['mean_ms']['graptor'] for x in subset),confirmed_regressions=sum(x['confirmed_regression'] for x in subset),
   candidate_graptor_wins=sum(x['mean_ms']['candidate']<x['mean_ms']['graptor'] for x in subset),
   accepted_graptor_wins=sum(x['mean_ms']['accepted']<x['mean_ms']['graptor'] for x in subset),
   case_mean_geomean_speedup=gm(x['mean_ms']['accepted']/x['mean_ms']['candidate'] for x in subset),
   same_binary_disabled_over_candidate=gm(x['mean_ms']['disabled']/x['mean_ms']['candidate'] for x in subset),
   kernel_p1_over_graptor=gm(x['kernel_mean_ms']['candidate']/x['kernel_mean_ms']['graptor'] for x in subset),
   kernel_accepted_over_candidate=gm(x['kernel_mean_ms']['accepted']/x['kernel_mean_ms']['candidate'] for x in subset))
 # Both source occurrence order and the Graptor CSR must agree for a duplicate
 # sensitivity group. Do not silently treat reordered inputs as identical.
 unique={}
 for x in cases:
  row=next(r for r in manifest['cases'] if r['graph']==x['graph'])
  key=(x['algorithm'],Path(row['original_file']).name,Path(row['bin']).name,row['p1_directed'],
   row['source'] if x['algorithm'] in ['bfs','sssp'] else None,
   Path(row['p1_weighted_file']).name if x['algorithm']=='sssp' else None)
  unique.setdefault(key,[]).append(x)
 summary['ordered_input_duplicate_sensitivity']=dict(unique_cases=len(unique),
  accepted_ms=sum(statistics.mean(x['mean_ms']['accepted'] for x in xs) for xs in unique.values()),
  candidate_ms=sum(statistics.mean(x['mean_ms']['candidate'] for x in xs) for xs in unique.values()),
  duplicate_groups=[[x['graph'] for x in xs] for xs in unique.values() if len(xs)>1])
 summary['total_change_percent']=(summary['candidate_total_ms']/summary['accepted_total_ms']-1)*100
 summary['original50_mean_geomean_speedup']=result['summary'][phase]['original50_mean_geomean_speedup']
 summary['graptor_total_speedup']=summary['candidate_total_ms']/summary['graptor_total_ms']
 summary['aa_failures']=0
 (out/'final_summary.json').write_text(json.dumps(summary,indent=2))
 lux=next(x for x in cases if x['algorithm']=='cc' and x['graph']=='dimacs10/inf-luxembourg_osm.edges')
 sensitivity=summary['ordered_input_duplicate_sensitivity']
 text=["P1 effect-derived static schedules: Kaggle results, 2026-10-20",'',
  f"The candidate is retained. It has {len(summary['confirmed_regressions'])} statistically confirmed per-case regressions and does not meet the previous no-regression promotion rule.",
  f"Two physical-core representatives (0 and 1); SMT siblings excluded. {summary['primary_measurements']:,} primary and {summary['confirmation_measurements']:,} confirmation fresh-process observations. A/A controls pass.",
  f"Full 61-graph matrix: accepted {summary['accepted_total_ms']/1000:.6f} s; candidate {summary['candidate_total_ms']/1000:.6f} s; summed case means {summary['total_change_percent']:+.3f}% slower.",
  f"Equal-case mean/median geometric speedups: {summary['mean_geomean_speedup']:.6f}x / {summary['median_geomean_speedup']:.6f}x. These differ from the sum-of-means metric.",
  f"Original 50-graph subset mean geometric speedup: {summary['original50_mean_geomean_speedup']:.6f}x. Duplicate sensitivity (native occurrence order, CSR, direction, source and exact weights): {sensitivity['unique_cases']} unique algorithm/input cases; summed means {sensitivity['accepted_ms']/1000:.6f} -> {sensitivity['candidate_ms']/1000:.6f} s ({(sensitivity['candidate_ms']/sensitivity['accepted_ms']-1)*100:+.3f}%). The repeated Luxembourg input materially affects the full-dataset sum; all 61 graphs remain in the main result.",
  f"Graptor: {summary['graptor_total_ms']/1000:.6f} s; {summary['graptor_total_speedup']:.3f}x faster than candidate by summed means on this host. Main PageRank comparison uses double precision; historical float is separate.",
  '', 'The new schedule is certified for the full-domain preparation -> independent owners -> join family. Local activation audit finds it in all 61 PageRank programs and none of the other 183 programs.',
  'The 16 confirmed slowdowns are in BFS, SSSP and CC. The main regressions persist in the same-binary disabled control. This rules out an explanation based only on executing the preparation shortcut. The added common dispatch guard/context ABI and resulting compiled-code changes have not been individually isolated; the exact cause remains unresolved. No cache/scheduling explanation is asserted without an intervention.',
  f"On CC dimacs10/inf-luxembourg_osm, accepted/candidate/disabled are {lux['mean_ms']['accepted']:.3f} / {lux['mean_ms']['candidate']:.3f} / {lux['mean_ms']['disabled']:.3f} ms. Disabling the feature does not restore baseline performance.",
  '', 'All 244 cases validated on Kaggle at 1 and 2 physical workers for accepted P1, candidate, disabled candidate and Graptor, plus historical float PageRank. 122 exact accepted/candidate/disabled PageRank vector groups pass. Runtime/compiler contracts pass.',
  'One warmup, 5 primary measurements and independent 20-pair confirmation. CLOCK_MONOTONIC whole-process time is primary. Kernel times are separate; instrumented observations are excluded. Paired one-sided permutation tests use 100,000 resamples, seed 2358, Holm correction across 244 case and 4 algorithm tests. Identical-binary A/A tests are two-sided. No algorithm-level slowdown is confirmed after Holm correction.',
  'This native Linux two-core host differs from the prior six-core local campaign. Absolute timings and historical win counts must not be transferred between them.',
  '', 'Full per-graph/algorithm confirmation table: final_report.html. Primary data, full medians and tests: comparison.json / original comparison.html. Raw records, validation and build logs: p1-results.tar.gz.',
  '', 'Algorithm totals (sum of 61 case means, milliseconds):']
 for a,x in summary['by_algorithm'].items():
  text.append(f"{a}: accepted={x['accepted_ms']:.6f}, candidate={x['candidate_ms']:.6f}, change={(x['candidate_ms']/x['accepted_ms']-1)*100:+.3f}%, Graptor={x['graptor_ms']:.6f}, confirmed regressions={x['confirmed_regressions']}, candidate/Graptor wins={x['candidate_graptor_wins']}/61")
 text+=['','Confirmed regressions (algorithm, graph, accepted ms, candidate ms, disabled ms, change %, Holm p):']
 for x in summary['confirmed_regressions']:
  text.append(f"{x['algorithm']} | {x['graph']} | {x['mean_ms']['accepted']:.6f} | {x['mean_ms']['candidate']:.6f} | {x['mean_ms']['disabled']:.6f} | {x['change_percent']:+.3f} | {x['test']['holm_p']:.6g}")
 (out/'final_report.txt').write_text('\n'.join(text)+'\n')
 doc=['<!doctype html><html><meta charset="utf-8"><title>P1 static schedules: full Kaggle results</title>',
  '<style>body{font:15px system-ui;margin:24px;color:#18202b}table{border-collapse:collapse;font-size:13px}td,th{border:1px solid #ccd1d8;padding:6px;text-align:right}td:first-child{text-align:left}thead{background:#e9eef5}.regression{background:#ffe0e0}.observed{background:#fff7dc}.scroll{overflow:auto}pre{white-space:pre-wrap;max-width:1200px}</style>',
  '<h1>Mixed result: 16 confirmed regressions; candidate retained</h1>',
  '<p>All 61 graphs × four algorithms. Two physical cores on Kaggle; no SMT siblings. Confirmation: 20 fresh-process paired measurements per case. Times below are mean milliseconds. Positive change means slower.</p>',
  '<p>Summed means: accepted <b>'+f"{summary['accepted_total_ms']/1000:.6f}"+'</b> s → candidate <b>'+f"{summary['candidate_total_ms']/1000:.6f}"+'</b> s ('+f"{summary['total_change_percent']:+.3f}%"+'). Equal-case geometric mean improves '+f"{(1-1/summary['mean_geomean_speedup'])*100:.2f}%"+'. A/A controls pass.</p>',
  '<p>Graptor takes '+f"{summary['graptor_total_ms']/1000:.6f}"+' s by summed means on this host. Main PageRank comparison is double precision; historical float appears in its own column. These results are separate from the prior six-core PC campaign.</p>',
  '<p><b>Interpretation:</b> scheduling certification activates for PageRank. The confirmed regressions are in the other algorithms and persist with the feature disabled. The exact common-runtime/compiler cost has not been isolated. All changes remain on the candidate branch.</p>',
  '<p>Red rows: confirmed regression after Holm correction. Yellow: raw mean increase without confirmation. Whole-process and kernel time are separate.</p>']
 for a in ['bfs','sssp','cc','pagerank']:
  doc+=['<h2>'+html.escape(a.upper())+'</h2><div class="scroll"><table><thead><tr><th>Graph</th><th>Accepted whole ms</th><th>Candidate whole ms</th><th>Change %</th><th>Disabled whole ms</th><th>Graptor whole ms</th>']
  if a=='pagerank':doc.append('<th>Graptor historical float whole ms</th>')
  doc.append('<th>Accepted kernel ms</th><th>Candidate kernel ms</th><th>Graptor kernel ms</th><th>Holm p (slowdown)</th></tr></thead><tbody>')
  for x in cases:
   if x['algorithm']!=a:continue
   cls='regression' if x['confirmed_regression'] else 'observed' if x['change_percent']>0 else ''
   values=[html.escape(x['graph']),*[f"{x['mean_ms'][k]:.6f}" for k in ['accepted','candidate']],f"{x['change_percent']:+.2f}",*[f"{x['mean_ms'][k]:.6f}" for k in ['disabled','graptor']]]
   if a=='pagerank':values.append(f"{x['mean_ms']['graptor_float']:.6f}")
   values += [f"{x['kernel_mean_ms'][k]:.6f}" for k in ['accepted','candidate','graptor']]+[f"{x['test']['holm_p']:.6g}"]
   doc.append('<tr class="'+cls+'">'+''.join('<td>'+v+'</td>' for v in values)+'</tr>')
  doc.append('</tbody></table></div>')
 doc+=['<h2>Method and limitations</h2><pre>'+html.escape('\n'.join(text))+'</pre></html>']
 (out/'final_report.html').write_text('\n'.join(doc))
 print(json.dumps({k:v for k,v in summary.items() if k!='confirmed_regressions'},indent=2))
 print('Verified 244 complete cases, 32,025 timed fresh processes, stable A/A controls and 122 exact rank-vector groups.')
if __name__=='__main__':main()
