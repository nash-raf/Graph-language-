"""Full-matrix cursor repair campaign; preserve both historical P1 controls."""
from pathlib import Path
import argparse, html, json, sys
import portable_campaign as P

ALGS=P.ALGS
setup=P.setup

def specs(a):
    s=[('Accepted','accepted',{}),('StaticSchedule','candidate',{}),
       ('Accepted','AA',{}),('Unfixed','unfixed',{}),('Unfixed','unfixed_AA',{}),
       ('StaticSchedule','disabled',{'SGPL_NO_CURSOR_ALIGNMENT':'1'}),
       ('graptor_double' if a=='pagerank' else 'graptor','graptor',{'LD_PRELOAD':str(P.WORK/'gclock.so')})]
    if a=='pagerank':s.append(('graptor','graptor_float',{'LD_PRELOAD':str(P.WORK/'gclock.so')}))
    return s

def build(R):
    P.build(R)
    R.REPO=P.BUNDLE/'unfixed';R.build('Unfixed')
    m=R.read(R.OUT/'manifest.json');m['unfixed_fingerprint']=R.verify_fingerprint('Unfixed')
    m['labels']={'accepted':'pre-static-schedule accepted P1', 'candidate':'static schedule plus cursor repair',
                 'unfixed':'original static schedule candidate', 'disabled':'repair binary with SGPL_NO_CURSOR_ALIGNMENT=1',
                 'AA':'identical accepted binary', 'unfixed_AA':'identical unfixed binary'}
    P.save(R.OUT/'manifest.json',m)

def validate(R,cpus):
    if not (R.OUT/'cursor_contracts_passed.json').exists():
        env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,sorted(set([1,min(2,len(cpus)),len(cpus)]))))
        R.checked(['bash','test/run_cursor_alignment_tests.sh'],P.WORK/'versions/StaticSchedule/src',
                  R.OUT/'cursor_runtime_contracts.log',env)
        P.save(R.OUT/'cursor_contracts_passed.json',{'passed':True,'bundle_manifest':P.sha(P.BUNDLE/'bundle_manifest.json')})
    P.validate(R,cpus)
    # P.validate validates every label; exact vector comparison also includes
    # the unfixed predecessor, rather than assuming runtime-only equivalence.
    import numpy as np
    exact=[]
    for w in sorted(set([1,min(2,len(cpus)),len(cpus)])):
        for row in R.M.ROWS:
            vectors=[]
            for v,label,extra in [x for x in specs('pagerank') if x[1] in ['accepted','candidate','unfixed','disabled']]:
                x=R.invoke('static_schedule','validate',v,'pagerank',row,0,label=label,workers=w,extra=extra)
                t=Path(x['base']+'.out').read_text().split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0]
                vectors.append(np.array(t.split(),dtype=np.float64).tobytes())
            assert all(v==vectors[0] for v in vectors),'Exact predecessor vector changed: '+row['graph']
            exact.append({'graph':row['graph'],'workers':w,'passed':True})
    P.save(R.OUT/'exact_cursor_pagerank_vectors.json',exact)
    valid=R.read(R.OUT/'validated.json');valid['fingerprints']['Unfixed']=R.verify_fingerprint('Unfixed')
    P.save(R.OUT/'validated.json',valid)

def report(R):
    cumulative=P.report(R)
    outer=R.OUT
    try:
        R.OUT=outer/'incremental';R.OUT.mkdir(exist_ok=True)
        for phase in cumulative:
            rows=R.read(outer/(phase+'.json'))
            mapped=[]
            for x in rows:
                if x['label'] in ['accepted','AA']:continue
                z=dict(x);z['label']={'unfixed':'accepted','unfixed_AA':'AA'}.get(x['label'],x['label']);mapped.append(z)
            P.save(R.OUT/(phase+'.json'),mapped)
        incremental=P.report(R)
    finally:R.OUT=outer
    phases={}
    for phase in cumulative:
        if phase not in incremental:continue
        phases[phase]={'cumulative':cumulative[phase],'incremental':incremental[phase]}
    P.save(outer/'cursor_repair_summary.json',{'phases':phases,'candidate_retained':True,'host':R.read(P.WORK/'host.json')})
    cumulative_rows=R.read(outer/'comparison.json')['cases']
    incremental_rows=R.read(outer/'incremental/comparison.json')['cases']
    doc=['<!doctype html><meta charset="utf-8"><title>P1 cursor repair full results</title>',
         '<style>body{font:15px system-ui;margin:24px}table{border-collapse:collapse}td,th{border:1px solid #ccc;padding:6px;text-align:right}td:first-child{text-align:left}.bad{background:#ffdddd}pre{white-space:pre-wrap}</style>',
         '<h1>P1 aligned cursor repair</h1><p>All 61 graphs × four algorithms. Whole-process monotonic mean milliseconds. Static schedules retained. SMT siblings excluded. Unaligned is the repaired binary with alignment disabled. The cumulative comparison uses pre-static P1; the incremental comparison uses the unfixed static-schedule candidate.</p>']
    for phase,pair in phases.items():
        doc.append('<h2>'+html.escape(phase)+'</h2>')
        for name,s in pair.items():
            concise={k:v for k,v in s.items() if k not in ['tests','aa_tests']}
            doc.append('<h3>'+name+'</h3><pre>'+html.escape(json.dumps(concise,indent=2))+'</pre>')
        for a in ALGS:
            doc.append('<h3>'+a+'</h3><table><tr><th>Graph</th><th>Pre-static ms</th><th>Unfixed ms</th><th>Repaired ms</th><th>Unaligned ms</th><th>Graptor ms (PR double)</th><th>PR float ms</th><th>Cumulative change %</th><th>Incremental change %</th><th>Confirmed regression cumulative / incremental</th></tr>')
            for x in cumulative_rows:
                if x['phase']!=phase or x['algorithm']!=a:continue
                y=next(z for z in incremental_rows if z['phase']==phase and z['algorithm']==a and z['graph']==x['graph'])
                bad=x['confirmed_regression'] or y['confirmed_regression']
                cells=[html.escape(x['graph'])]+[f'{v:.6f}' for v in [x['mean_ms']['accepted'],y['mean_ms']['accepted'],x['mean_ms']['candidate'],x['mean_ms']['disabled'],x['mean_ms']['graptor']]]
                cells += [f'{x["mean_ms"]["graptor_float"]:.6f}' if a=='pagerank' else 'n/a',f'{x["change_percent"]:+.3f}',f'{y["change_percent"]:+.3f}',f'{x["confirmed_regression"]} / {y["confirmed_regression"]}']
                doc.append('<tr'+(' class="bad"' if bad else '')+'>'+''.join('<td>'+v+'</td>' for v in cells)+'</tr>')
            doc.append('</table>')
    (outer/'cursor_repair_report.html').write_text('\n'.join(doc))
    return phases

def main():
    parser=argparse.ArgumentParser();parser.add_argument('stage',choices=['build','validate','primary','confirmation','report','all']);args=parser.parse_args()
    P.specs=specs
    R,cpus=setup()
    original_save=P.save
    def save_with_duration(path,value):
        original_save(path,value)
        if str(path).endswith('_duration.json'):
            print(json.dumps({'stage':'block_complete','block':Path(path).stem,**value}),flush=True)
    P.save=save_with_duration
    with R.locked('cursor_repair_'+args.stage):
        for stage in (['build','validate','primary','confirmation','report'] if args.stage=='all' else [args.stage]):
            if stage=='build':build(R)
            elif stage=='validate':validate(R,cpus)
            elif stage in ['primary','confirmation']:P.benchmark(R,cpus,stage,5 if stage=='primary' else 20)
            else:
                phases=report(R)
                repeated=False
                for phase in ['primary','confirmation']:
                    if phase in phases and any(not s['valid_for_inference'] for s in phases[phase].values()) and phase+'_retry1' not in phases:
                        P.benchmark(R,cpus,phase+'_retry1',5 if phase=='primary' else 20)
                        repeated=True
                if repeated:report(R)
if __name__=='__main__':main()
