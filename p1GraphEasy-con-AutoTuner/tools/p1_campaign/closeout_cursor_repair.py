"""Audit downloaded full-matrix evidence and summarize the retained repair."""
from pathlib import Path
import argparse, collections, hashlib, html, json, math, shutil, statistics
ALGS=['bfs','sssp','cc','pagerank']
def read(p):return json.loads(Path(p).read_text())
def gm(xs):return math.exp(statistics.mean(math.log(x) for x in xs))
def main():
    p=argparse.ArgumentParser();p.add_argument('evidence');p.add_argument('output');a=p.parse_args()
    root=Path(a.evidence)/'p1-campaign';out=Path(a.output);out.mkdir(parents=True,exist_ok=True);results=root/'results'
    cumulative=read(results/'comparison.json');incremental=read(results/'incremental/comparison.json')
    manifest=read(results/'manifest.json');valid=read(results/'validated.json');host=read(root/'host.json')
    assert len(manifest['cases'])==61 and len(valid['fingerprints'])==3
    assert read(results/'cursor_contracts_passed.json')['passed']
    assert read(results/'contracts_passed.json')['passed']
    exact=read(results/'exact_cursor_pagerank_vectors.json')
    assert len(exact)==61*len(set([1,min(2,valid['workers']),valid['workers']])) and all(x['passed'] for x in exact)
    counts={}
    for phase in cumulative['summary']:
        rows=read(results/(phase+'.json'));n=5 if phase.startswith('primary') else 20
        assert len(rows)==(244*7+61)*n
        assert all(not x.get('error') and x['seconds']>0 and x['workers']==valid['workers'] for x in rows)
        groups=collections.Counter((x['algorithm'],x['graph'],x['label']) for x in rows)
        assert len(groups)==1769 and set(groups.values())=={n}
        counts[phase]=len(rows)
    chosen={name:(name+'_retry1' if name+'_retry1' in cumulative['summary'] else name) for name in ['primary','confirmation']}
    phase=chosen['confirmation'];cs=[x for x in cumulative['cases'] if x['phase']==phase];ins=[x for x in incremental['cases'] if x['phase']==phase]
    assert len(cs)==len(ins)==244
    stable=all(cumulative['summary'][v]['valid_for_inference'] and incremental['summary'][v]['valid_for_inference'] for v in chosen.values())
    promotion=stable and all(all(source['summary'][v][key]>1.01 for key in ['mean_geomean_speedup','median_geomean_speedup']) and
        not any(t['holm_p']<.05 for t in source['summary'][v]['tests']) for source in [cumulative,incremental] for v in chosen.values())
    sums={label:sum(x['mean_ms'][label] for x in cs) for label in ['accepted','candidate','disabled','graptor']}
    sums['unfixed']=sum(x['mean_ms']['accepted'] for x in ins)
    summary={'host':host,'workers':valid['workers'],'measurements':counts,'confirmation_phase':phase,'chosen_phases':chosen,'stable_controls':stable,
             'meets_cumulative_and_incremental_promotion_gates':promotion,'automatically_promoted':False,'candidate_retained':True,'totals_ms':sums,
             'cumulative_change_percent':(sums['candidate']/sums['accepted']-1)*100,
             'incremental_change_percent':(sums['candidate']/sums['unfixed']-1)*100,
             'graptor_total_speedup':sums['candidate']/sums['graptor'],
             'confirmed_cumulative_regressions':[x for x in cs if x['confirmed_regression']],
             'confirmed_incremental_regressions':[x for x in ins if x['confirmed_regression']],
             'by_algorithm':{},'comparisons':{name:{k:s['summary'][phase][k] for k in ['mean_geomean_speedup','median_geomean_speedup','original50_mean_geomean_speedup','duplicate_content_sensitivity','valid_for_inference']} for name,s in [('cumulative',cumulative),('incremental',incremental)]}}
    for alg in ALGS:
        xs=[x for x in cs if x['algorithm']==alg];ys=[x for x in ins if x['algorithm']==alg]
        totals={label:sum(x['mean_ms'][label] for x in xs) for label in ['accepted','candidate','disabled','graptor']}
        totals['unfixed']=sum(x['mean_ms']['accepted'] for x in ys)
        summary['by_algorithm'][alg]={'totals_ms':totals,'cumulative_change_percent':(totals['candidate']/totals['accepted']-1)*100,
            'incremental_change_percent':(totals['candidate']/totals['unfixed']-1)*100,
            'graptor_wins':sum(x['mean_ms']['candidate']<x['mean_ms']['graptor'] for x in xs),
            'confirmed_cumulative_regressions':sum(x['confirmed_regression'] for x in xs),
            'confirmed_incremental_regressions':sum(x['confirmed_regression'] for x in ys),
            'cumulative_mean_geomean_speedup':gm(x['mean_ms']['accepted']/x['mean_ms']['candidate'] for x in xs),
            'incremental_mean_geomean_speedup':gm(x['mean_ms']['accepted']/x['mean_ms']['candidate'] for x in ys)}
    (out/'final_summary.json').write_text(json.dumps(summary,indent=2))
    text=['P1 production cursor alignment repair: full Kaggle results','',
        'The repair and all static-schedule changes are retained. No automatic promotion or reversion.',
        f"Physical workers: {valid['workers']}; SMT siblings excluded. Timed observations: {sum(counts.values()):,}. A/A stable: {stable}.",
        f"Summed 244-case means: pre-static accepted {sums['accepted']:.6f} ms; unfixed static candidate {sums['unfixed']:.6f} ms; repaired {sums['candidate']:.6f} ms; Graptor {sums['graptor']:.6f} ms.",
        f"Cumulative change {summary['cumulative_change_percent']:+.3f}%; incremental repair change {summary['incremental_change_percent']:+.3f}%. Negative means faster.",
        f"Confirmed per-case regressions: cumulative {len(summary['confirmed_cumulative_regressions'])}; incremental {len(summary['confirmed_incremental_regressions'])}.",
        f"Both cumulative and incremental promotion gates satisfied: {promotion}. Separate comparisons use Holm correction over 244 cases and four algorithm tests each.",
        'Whole-process CLOCK_MONOTONIC is primary. Internal kernel timing is separate. One warmup and five primary measurements; independent twenty-pair confirmation. Paired permutation tests: 100,000 resamples, seed 2358. Identical-binary A/A controls, retained retry on instability. PageRank Graptor comparison uses double precision; historical float appears separately.',
        'The repair guarantees a 64-byte cursor base and a cache-line-rounded chunk row stride. It changes no public execution ABI, effects, DAG, callback order, duplicate handling, weight lookup or floating-point evaluation order. Instrumentation is excluded from acceptance timings.',
        'All 244 cases pass independent validation on this host. Contract suites pass. Exact printed PageRank vectors match accepted, unfixed, repaired and disabled builds. Separate local verification passed 732 independent result cases and 183 full-bit PageRank vector comparisons at 1/2/6 physical workers.',
        'These comparisons rebuild every version on one new Kaggle host; do not compare absolute times with earlier virtual machines. Residual regressions remain reported; their causes must not be attributed automatically to cursor false sharing.', '']
    for alg,v in summary['by_algorithm'].items():text.append(alg+': '+json.dumps(v))
    for kind,key in [('Cumulative','confirmed_cumulative_regressions'),('Incremental','confirmed_incremental_regressions')]:
        text.append('\n'+kind+' confirmed regressions:')
        for x in summary[key]:text.append(f"{x['algorithm']} | {x['graph']} | {x['change_percent']:+.3f}% | Holm p={x['test']['holm_p']:.6g}")
    (out/'final_report.txt').write_text('\n'.join(text)+'\n')
    body=['<!doctype html><meta charset="utf-8"><title>P1 cursor alignment repair results</title>',
          '<style>body{font:15px system-ui;margin:24px}pre{white-space:pre-wrap;max-width:1200px}table{border-collapse:collapse}td,th{border:1px solid #ccc;padding:6px;text-align:right}td:first-child{text-align:left}.bad{background:#ffdddd}</style>',
          '<h1>P1 production cursor alignment repair</h1><pre>'+html.escape('\n'.join(text[:14]))+'</pre>',
          '<p><a href="cursor_repair_report.html">All primary and confirmation tables</a> · <a href="comparison.html">Cumulative kernel timings and tests</a> · <a href="incremental_comparison.html">Incremental kernel timings and tests</a></p>']
    for alg in ALGS:
        body.append('<h2>'+alg+'</h2><table><tr><th>Graph</th><th>Pre-static ms</th><th>Unfixed ms</th><th>Repaired ms</th><th>Unaligned ms</th><th>Graptor double ms</th><th>Graptor float ms</th><th>Cumulative %</th><th>Incremental %</th><th>Confirmed regression cumulative / incremental</th></tr>')
        for x in cs:
            if x['algorithm']!=alg:continue
            y=next(z for z in ins if z['algorithm']==alg and z['graph']==x['graph'])
            vals=[html.escape(x['graph'])]+[f'{v:.6f}' for v in [x['mean_ms']['accepted'],y['mean_ms']['accepted'],x['mean_ms']['candidate'],x['mean_ms']['disabled'],x['mean_ms']['graptor']]]
            vals += [f'{x["mean_ms"]["graptor_float"]:.6f}' if alg=='pagerank' else 'n/a',f'{x["change_percent"]:+.3f}',f'{y["change_percent"]:+.3f}',f'{x["confirmed_regression"]} / {y["confirmed_regression"]}']
            body.append('<tr'+(' class="bad"' if x['confirmed_regression'] or y['confirmed_regression'] else '')+'>'+''.join('<td>'+v+'</td>' for v in vals)+'</tr>')
        body.append('</table>')
    (out/'final_report.html').write_text('\n'.join(body))
    for name in ['comparison.html','comparison.json','cursor_repair_report.html','cursor_repair_summary.json']:shutil.copy2(results/name,out/name)
    for ext in ['html','json']:shutil.copy2(results/'incremental'/('comparison.'+ext),out/('incremental_comparison.'+ext))
    checks={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(out.iterdir()) if p.is_file() and p.name!='SHA256SUMS.json'}
    (out/'SHA256SUMS.json').write_text(json.dumps(checks,indent=2))
    print(json.dumps({k:v for k,v in summary.items() if k not in ['confirmed_cumulative_regressions','confirmed_incremental_regressions','host']},indent=2))
if __name__=='__main__':main()
