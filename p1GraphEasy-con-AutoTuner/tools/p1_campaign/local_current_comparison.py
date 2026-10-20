"""Fresh local full-dataset P1/Graptor comparison, using physical cores only."""
from pathlib import Path
import argparse,collections,datetime,hashlib,html,json,math,os,platform,statistics,time

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--output',required=True);p.add_argument('--current',default='CursorAligned1')
p.add_argument('--baseline',default='R4a3');p.add_argument('--attempt',default='LocalCurrentPC20261020')
p.add_argument('--stage',choices=['all','validate','benchmark','report','status'],default='all')
args=p.parse_args();os.environ['P1_CAMPAIGN_OUTPUT']=args.output
import runner as R
from portable_campaign import permutation_pvalue
import numpy as np

def gm(xs):return math.exp(statistics.mean(math.log(x) for x in xs))

def specs(a):
    s=[('accepted',args.baseline),('candidate',args.current),('accepted_AA',args.baseline),
       ('candidate_AA',args.current),('graptor','graptor_double' if a=='pagerank' else 'graptor')]
    if a=='pagerank':s.append(('graptor_float','graptor'))
    return s

def host():
    seen=set();topology=[]
    assert len(R.CPUS)==6,'This comparison requires six physical cores'
    for cpu in R.CPUS:
        d=Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'topology'
        pair=tuple((d/n).read_text().strip() for n in ['physical_package_id','core_id'])
        assert pair not in seen,'SMT siblings must not be selected';seen.add(pair)
        topology.append({'cpu':cpu,'package':pair[0],'core':pair[1],
                         'siblings':(d/'thread_siblings_list').read_text().strip()})
    assert set(os.sched_getaffinity(0))==set(R.CPUS)
    return {'platform':platform.platform(),'physical_cpus':R.CPUS,'workers':6,'topology':topology,
            'cpu_model':next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name'))}

def prepare():
    info=host()
    if not (R.OUT/'manifest.json').exists():R.init()
    R.verify_inputs()
    # freeze verifies all current native source inputs against the previously
    # built version. Tooling-only commits do not relabel the native checkpoint.
    R.freeze(args.current)
    fp={v:R.verify_fingerprint(v) for v in [args.baseline,args.current]}
    protocol={'baseline':args.baseline,'current':args.current,'fingerprints':fp,
              'host':info,'dataset_hash':R.read(R.OUT/'manifest.json')['dataset_hash'],
              'graptor':R.read(R.OUT/'manifest.json')['graptor'],
              'primary_repetitions':5,'confirmation_pairs':20,'warmups_per_phase':1,
              'precision':'double PageRank main; historical float separate',
              'timer':'whole-process CLOCK_MONOTONIC; internal timing separate',
              'harness_sha256':R.sha(Path(__file__)),'commit':R.git('rev-parse','HEAD'),
              'source_identity':R.read(R.ROOT/'versions'/args.current/'identity.json')}
    path=R.OUT/'protocol.json'
    if path.exists():assert R.read(path)==protocol,'Protocol changed; start a new attempt'
    else:R.save(path,protocol)
    R.save(R.OUT/'host.json',info)
    return protocol

def validate(protocol):
    path=R.OUT/'validated.json'
    if path.exists():
        assert R.read(path)['fingerprints']==protocol['fingerprints'];return
    previous=R.read(Path('/mnt/c/p1-static-schedules-20261020/cursor-repair-local')/(args.current+'_validation_workers.json'))
    assert previous and len(previous)==732 and all(x.get('validation') and not x.get('error') for x in previous)
    records=[];exact=[]
    for a in R.ALGS:
        started=time.monotonic()
        for i,row in enumerate(R.M.ROWS,1):
            outputs={}
            for label,v in specs(a):
                if label.endswith('_AA'):continue
                x=R.invoke(args.attempt,'validate_current',v,a,row,0,label=label,workers=6)
                assert x.get('validation') and not x.get('error'),x
                records.append(x)
                if a=='pagerank' and label in ['accepted','candidate']:
                    text=Path(x['base']+'.out').read_text().split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0]
                    outputs[label]=np.array(text.split(),dtype=np.float64).tobytes()
            if a=='pagerank':
                assert outputs['accepted']==outputs['candidate'],('Exact printed PageRank mismatch',row['graph'])
                exact.append({'graph':row['graph'],'passed':True,'sha256':hashlib.sha256(outputs['candidate']).hexdigest()})
            R.progress(stage='validate_current',algorithm=a,completed=i,total=61,failures=0,elapsed_seconds=time.monotonic()-started)
    R.save(R.OUT/'validation_records.json',records);R.save(R.OUT/'exact_pagerank_vectors.json',exact)
    R.save(path,{'fingerprints':protocol['fingerprints'],'workers':6,'passed':True,'cases':244})

def benchmark(protocol,phase,repetitions):
    assert R.read(R.OUT/'validated.json')['fingerprints']==protocol['fingerprints']
    R.verify_inputs()
    for v,h in protocol['fingerprints'].items():assert R.verify_fingerprint(v)==h
    records=[]
    base_phase=phase.split('_retry',1)[0]
    for a in R.ALGS:
        started=time.monotonic();R.state(stage=phase,algorithm=a,completed=0,total=61,block_started=time.time(),expected_completion_unix=None)
        for i,row in enumerate(R.M.ROWS,1):
            choices=specs(a)
            for label,v in choices:
                x=R.invoke(args.attempt,'warmup_'+phase,v,a,row,0,label=label,workers=6)
                assert not x.get('error'),x
            for rep in range(repetitions):
                # Rotate all variants, then alternate traversal direction.
                shift=(i+rep)%len(choices);order=choices[shift:]+choices[:shift]
                if rep%2:order=order[::-1]
                for label,v in order:
                    x=R.invoke(args.attempt,phase,v,a,row,rep,label=label,workers=6)
                    assert not x.get('error') and x['seconds']>0,x
                    records.append(x)
            R.save(R.OUT/(phase+'.json'),records)
            R.progress(stage=phase,algorithm=a,completed=i,total=61,failures=0,elapsed_seconds=time.monotonic()-started)
        R.calibrate(a,base_phase,[61,repetitions,[x[0] for x in specs(a)],6],time.time()-(time.monotonic()-started))
        R.save(R.OUT/(phase+'_'+a+'_duration.json'),{'seconds':time.monotonic()-started,'repetitions':repetitions,'cases':61,'workers':6})

def holm(tests):
    running=0
    for rank,t in enumerate(sorted(tests,key=lambda t:t['p'])):
        running=max(running,min(1,t['p']*(len(tests)-rank)));t['holm_p']=running

def analyze(phase):
    records=R.read(R.OUT/(phase+'.json'));n=5 if phase.startswith('primary') else 20
    assert len(records)==(244*5+61)*n
    groups=collections.defaultdict(list);bases=set()
    for x in records:
        assert not x.get('error') and x['seconds']>0 and x['workers']==6
        assert x['base'] not in bases;bases.add(x['base'])
        base=Path(x['base']);assert R.read(base.with_suffix('.record.json'))==x
        stamp=R.read(base.with_suffix('.stamp.json'));assert stamp['seconds']==x['seconds'] and stamp['exit_code']==0
        groups[(x['algorithm'],x['graph'],x['label'])].append(x)
    assert len(groups)==1281 and all(len(v)==n for v in groups.values())
    families={k:[] for k in ['code_regressions','graptor_comparison','accepted_AA','candidate_AA']};by_alg={k:{} for k in families}
    rows=[]
    for a in R.ALGS:
        ds={k:[] for k in families};started=time.monotonic()
        for i,r in enumerate(R.M.ROWS,1):
            samples={label:sorted(groups[(a,r['graph'],label)],key=lambda x:x['repeat']) for label,v in specs(a)}
            arrays={label:np.array([x['seconds'] for x in xs]) for label,xs in samples.items()}
            entries={}
            for family,left,right,two in [('code_regressions','accepted','candidate',False),('graptor_comparison','graptor','candidate',True),
                                           ('accepted_AA','accepted','accepted_AA',True),('candidate_AA','candidate','candidate_AA',True)]:
                d=np.log(arrays[right]/arrays[left]);ds[family].append(d)
                t={'algorithm':a,'graph':r['graph'],'kind':'case','p':permutation_pvalue(d,two),
                   'log_ratio':float(np.mean(d))};families[family].append(t);entries[family]=t
            rows.append({'phase':phase,'algorithm':a,'graph':r['graph'],'cohort':r['cohort'],
                'mean_ms':{label:float(np.mean(v))*1000 for label,v in arrays.items()},
                'median_ms':{label:float(np.median(v))*1000 for label,v in arrays.items()},
                'kernel_mean_ms':{label:statistics.mean(x['internal_seconds'] for x in xs)*1000 if all('internal_seconds' in x for x in xs) else None for label,xs in samples.items()},'tests':entries})
            if i%20==0 or i==61:R.progress(stage='report',algorithm=a,phase=phase,completed=i,total=61,elapsed_seconds=time.monotonic()-started)
        for family,values in ds.items():
            d=np.mean(values,axis=0);families[family].append({'algorithm':a,'kind':'algorithm',
                'p':permutation_pvalue(d,family!='code_regressions'),'log_ratio':float(np.mean(d))})
    for tests in families.values():holm(tests)
    aa_failures=[dict(t,family=f) for f in ['accepted_AA','candidate_AA'] for t in families[f] if t['holm_p']<.05]
    stable=not aa_failures
    summary={'phase':phase,'repetitions':n,'observations':len(records),'valid_for_inference':stable,'aa_failures':aa_failures,
        'code_mean_geomean_speedup':gm(x['mean_ms']['accepted']/x['mean_ms']['candidate'] for x in rows),
        'code_median_geomean_speedup':gm(x['median_ms']['accepted']/x['median_ms']['candidate'] for x in rows),
        'confirmed_code_regressions':[t for t in families['code_regressions'] if stable and t['holm_p']<.05],
        'by_algorithm':{},'tests':families}
    for a in R.ALGS:
        xs=[x for x in rows if x['algorithm']==a]
        summary['by_algorithm'][a]={'cases':61,'p1_wins':sum(x['mean_ms']['candidate']<x['mean_ms']['graptor'] for x in xs),
            'accepted_wins':sum(x['mean_ms']['accepted']<x['mean_ms']['graptor'] for x in xs),
            'totals_ms':{label:sum(x['mean_ms'][label] for x in xs) for label,v in specs(a)},
            'confirmed_p1_slower_cases':sum(stable and x['tests']['graptor_comparison']['holm_p']<.05 and x['tests']['graptor_comparison']['log_ratio']>0 for x in xs),
            'confirmed_p1_faster_cases':sum(stable and x['tests']['graptor_comparison']['holm_p']<.05 and x['tests']['graptor_comparison']['log_ratio']<0 for x in xs)}
    summary['totals_ms']={label:sum(x['mean_ms'][label] for x in rows) for label in ['accepted','candidate','graptor']}
    summary['graptor_over_p1_geomean_speedup']=gm(x['mean_ms']['graptor']/x['mean_ms']['candidate'] for x in rows)
    main50=[x for x in rows if x['cohort']=='main50']
    summary['main50_graptor_over_p1_geomean_speedup']=gm(x['mean_ms']['graptor']/x['mean_ms']['candidate'] for x in main50)
    unique={}
    for x in rows:
        r=next(r for r in R.M.ROWS if r['graph']==x['graph'])
        key=(x['algorithm'],r['original_sha256'],R.sha(r['bin']),r['p1_directed'],r['source'] if x['algorithm'] in ['bfs','sssp'] else None,R.sha(r['p1_weighted_file']) if x['algorithm']=='sssp' else None)
        unique.setdefault(key,[]).append(x)
    summary['duplicate_sensitivity']={'unique_cases':len(unique),'totals_ms':{label:sum(statistics.mean(x['mean_ms'][label] for x in group) for group in unique.values()) for label in ['accepted','candidate','graptor']}}
    result={'summary':summary,'cases':rows};R.save(R.OUT/(phase+'_comparison.json'),result);return result

def report(protocol,results):
    chosen=next((phase for phase in ['confirmation_retry1','confirmation'] if phase in results),None)
    if not chosen:chosen=next(reversed(results))
    final=results[chosen];s=final['summary'];ts=s['totals_ms'];rat=ts['candidate']/ts['graptor']
    lines=['Current P1 vs Graptor on this PC','',f"Six physical cores: {R.CPUS}; SMT siblings excluded. CPU: {protocol['host']['cpu_model']}.",
        f"Current: {args.current}; previous accepted: {args.baseline}. PageRank uses double precision; float is reported separately.",
        f"Chosen block: {chosen}; A/A stable: {s['valid_for_inference']}; {s['observations']} observations in this block.",
        f"Sum of 244 mean whole-process times: previous accepted {ts['accepted']/1000:.6f} s; current P1 {ts['candidate']/1000:.6f} s; Graptor {ts['graptor']/1000:.6f} s.",
        f"P1 / Graptor total time: {rat:.6f}x; current P1 graph-case wins {sum(x['p1_wins'] for x in s['by_algorithm'].values())}/244.",
        f"Current vs previous accepted sum change: {(ts['candidate']/ts['accepted']-1)*100:+.3f}%; mean/median case geometric speedups {s['code_mean_geomean_speedup']:.6f}x / {s['code_median_geomean_speedup']:.6f}x.",
        f"Confirmed code regressions (cases and algorithms): {len(s['confirmed_code_regressions'])}. All code and raw records retained.",
        'Win counts mean lower measured means; statistically confirmed Graptor comparisons are separate. Whole-process CLOCK_MONOTONIC includes loading, setup and runtime. Instrumentation is excluded.',
        'One warmup, five primary measurements, then independent twenty-pair confirmation. Fixed seed 2358, 100,000 resamples, Holm correction over 244 case plus four algorithm tests in each family. Two-sided Graptor and A/A tests; one-sided code slowdown tests. One retained retry if A/A fails.']
    for a,x in s['by_algorithm'].items():
        t=x['totals_ms'];lines.append(f"{a}: P1={t['candidate']/1000:.6f} s, Graptor={t['graptor']/1000:.6f} s, P1/Graptor={t['candidate']/t['graptor']:.6f}x, wins={x['p1_wins']}/61, previous wins={x['accepted_wins']}/61")
    (R.OUT/'final_report.txt').write_text('\n'.join(lines)+'\n');R.save(R.OUT/'final_summary.json',{'protocol':protocol,**s})
    doc=['<!doctype html><meta charset="utf-8"><title>Current P1 vs Graptor: six physical cores</title>',
        '<style>body{font:15px system-ui;margin:24px}pre{white-space:pre-wrap;max-width:1200px}table{border-collapse:collapse}td,th{border:1px solid #ccc;padding:6px;text-align:right}td:first-child{text-align:left}.loss{background:#fff1d6}.confirmed{background:#ffdddd}</style>',
        '<h1>Current P1 vs Graptor on this PC</h1><pre>'+html.escape('\n'.join(lines))+'</pre>',
        '<h2>Algorithm totals: sum of 61 case means</h2><table><tr><th>Algorithm</th><th>Previous P1 s</th><th>Current P1 s</th><th>Graptor s</th><th>P1 / Graptor</th><th>Current wins / 61</th></tr>']
    for a,x in s['by_algorithm'].items():
        t=x['totals_ms'];doc.append('<tr><td>'+a+'</td>'+''.join('<td>'+v+'</td>' for v in [f"{t['accepted']/1000:.6f}",f"{t['candidate']/1000:.6f}",f"{t['graptor']/1000:.6f}",f"{t['candidate']/t['graptor']:.3f}x",str(x['p1_wins'])])+'</tr>')
    doc.append('</table>')
    for phase,result in results.items():
        for a in R.ALGS:
            doc.append('<h2>'+html.escape(phase+' / '+a)+'</h2><table><tr><th>Graph</th><th>Previous ms</th><th>Current ms</th><th>Graptor double ms</th><th>Graptor float ms</th><th>P1 / Graptor</th><th>P1 kernel ms</th><th>Graptor kernel ms</th><th>Graptor test Holm p</th><th>Code slowdown Holm p</th></tr>')
            for x in result['cases']:
                if x['algorithm']!=a:continue
                t=x['mean_ms'];test=x['tests']['graptor_comparison'];loss=t['candidate']>t['graptor'];confirmed=result['summary']['valid_for_inference'] and test['holm_p']<.05 and test['log_ratio']>0
                vals=[html.escape(x['graph'])]+[f'{t[k]:.6f}' for k in ['accepted','candidate','graptor']]+[f"{t['graptor_float']:.6f}" if a=='pagerank' else 'n/a',f"{t['candidate']/t['graptor']:.3f}x"]
                vals += [f"{x['kernel_mean_ms'][k]:.6f}" if x['kernel_mean_ms'][k] is not None else 'missing' for k in ['candidate','graptor']]
                vals += [f"{test['holm_p']:.6g}",f"{x['tests']['code_regressions']['holm_p']:.6g}"]
                doc.append('<tr'+(' class="confirmed"' if confirmed else (' class="loss"' if loss else ''))+'>'+''.join('<td>'+v+'</td>' for v in vals)+'</tr>')
            doc.append('</table>')
    (R.OUT/'final_report.html').write_text('\n'.join(doc))
    print(json.dumps({'stage':'reported','valid_for_inference':s['valid_for_inference'],'totals_ms':ts,'by_algorithm':s['by_algorithm']}),flush=True)

def main():
    if args.stage=='status':
        print(json.dumps(R.read(R.OUT/'status.json',{}),indent=2));return
    R.M=R.bootstrap()
    with R.locked('local_current_'+args.stage):
        protocol=prepare()
        if args.stage in ['all','validate']:validate(protocol)
        if args.stage=='validate':return
        results={}
        for phase,reps in [('primary',5),('confirmation',20)]:
            if args.stage in ['all','benchmark']:benchmark(protocol,phase,reps)
            results[phase]=analyze(phase)
            if not results[phase]['summary']['valid_for_inference']:
                retry=phase+'_retry1'
                if args.stage in ['all','benchmark']:benchmark(protocol,retry,reps)
                results[retry]=analyze(retry)
        report(protocol,results)
        R.progress(stage='complete',completed=244,total=244,failures=0)
if __name__=='__main__':main()
