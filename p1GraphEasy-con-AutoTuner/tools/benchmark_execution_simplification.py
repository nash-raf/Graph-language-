#!/usr/bin/env python3
"""Local paired P1 measurements; Graptor is read from prior local observations only."""
from pathlib import Path
import argparse, collections, csv, fcntl, hashlib, html, json, math, os, platform, re, shutil, statistics, subprocess, sys, time

REPO=Path(__file__).resolve().parents[1]
ROOT=Path('/home/kazi/p1_execution_simplification_benchmark_20261022')
OUT=Path('/mnt/c/p1-execution-simplification-benchmark-20261022')
BASE=Path('/home/kazi/p1_c3_integration_20261022/versions/C3_integrated')
CAND=Path('/home/kazi/p1_execution_simplification/versions/candidate03')
HIST=Path('/home/kazi/p1_effect_fusion_local_20261022')
os.environ.update(P1_CAMPAIGN_ROOT=str(ROOT),P1_CAMPAIGN_OUTPUT=str(OUT),P1_CAMPAIGN_CPUS='0,2,4,6,8,10')
ROOT.mkdir(exist_ok=True);OUT.mkdir(exist_ok=True)
sys.path.insert(0,str(REPO/'tools/p1_campaign'))
import runner as R
from portable_campaign import permutation_pvalue

VARIANTS=[('baseline',BASE,{}),('candidate',CAND,{}),('disabled',CAND,{'SGPL_NO_EXEC_SIMPLIFY':'1'}),('baseline_AA',BASE,{}),('candidate_AA',CAND,{})]

def gm(values):return math.exp(statistics.mean(math.log(x) for x in values))
def logrun(cmd,cwd,log,env=None):R.checked(cmd,cwd,log,env,timeout=1200)
def path(version,a,row):return ROOT/'versions'/version/'cases'/a/row['graph'].removesuffix('.edges')

def prepare():
    old=Path('/home/kazi/p1_c3_integration_20261022')
    for name in ['references','frozen_reports','historical_harness','support']:
        if not (ROOT/name).exists():(ROOT/name).symlink_to(old/name,target_is_directory=True)
    if not (ROOT/'inputs.json').exists():shutil.copy2(old/'inputs.json',ROOT/'inputs.json')
    if not (ROOT/'walltime').exists():shutil.copy2(old/'walltime',ROOT/'walltime')
    R.M=R.bootstrap()
    manifest=R.read(HIST/'results/manifest.json');assert R.M.ROWS==manifest['cases']
    for name,digest in manifest['inputs'].items():assert R.sha(name)==digest,('Input changed',name)
    correct=R.read(REPO/'reports/execution_simplification_correctness.json')
    assert correct['validation']['passed'] and correct['source_hash']==R.read(CAND/'identity.json')['source_hash']
    assert R.sha(CAND/'GraphProgram')==correct['build']['compiler_sha256']
    # Pin current native source to the correctness-tested build, including ignored generated inputs.
    files=[p for p in REPO.iterdir() if p.is_file() and p.suffix in ['.c','.cpp','.h','.inc','.ll']]
    files += [p for p in (REPO/'generated').glob('*') if p.suffix in ['.cpp','.h']]
    assert R.key({str(p.relative_to(REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})==correct['source_hash']
    seen=set();topology=[]
    for cpu in R.CPUS:
        d=Path('/sys/devices/system/cpu')/str('cpu'+str(cpu))/'topology'
        key=tuple((d/name).read_text().strip() for name in ['physical_package_id','core_id'])
        assert key not in seen;seen.add(key);topology.append({'cpu':cpu,'package':key[0],'core':key[1]})
    host={'platform':platform.platform(),'cpu_model':next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),'physical_cpus':R.CPUS,'workers':6,'topology':topology,'boot_id':Path('/proc/sys/kernel/random/boot_id').read_text().strip()}
    assert host['cpu_model']==R.read(HIST/'host.json')['cpu_model']
    saved=R.read(HIST/'results/primary_w6.json');graptor=[x for x in saved if x['label']=='graptor']
    assert len(graptor)==1220 and all(x['workers']==6 and not x.get('error') and x['seconds']>0 for x in graptor)
    for x in graptor:
        assert x['version']==('graptor_double' if x['algorithm']=='pagerank' else 'graptor')
        assert R.read(Path(x['base']+'.record.json'))==x
    protocol={'baseline_commit':correct['baseline_commit'],'candidate_commit':R.git('rev-parse','HEAD'),'source_hash':correct['source_hash'],'host':host,'dataset_hash':manifest['dataset_hash'],'cases':R.M.ROWS,'primary_repetitions':5,'confirmation_repetitions':20,'warmups':1,'variants':[x[0] for x in VARIANTS],'timer':'whole-process CLOCK_MONOTONIC; internal timer_runtime interposer CLOCK_MONOTONIC','historical_graptor':{'records':str(HIST/'results/primary_w6.json'),'sha256':R.sha(HIST/'results/primary_w6.json'),'host':R.read(HIST/'host.json'),'precision':'double PageRank','note':'Prior local measurements, unpaired with this session; descriptive comparisons only. Graptor is never executed.'},'correctness_report_sha256':R.sha(REPO/'reports/execution_simplification_correctness.json'),'native_fingerprints':{label:{'compiler':R.sha(d/'GraphProgram'),'objects':{n:R.sha(d/(n+'.o')) for n in R.OBJECTS}} for label,d in [('baseline',BASE),('candidate',CAND)]},'harness_sha256':R.sha(Path(__file__))}
    if (OUT/'protocol.json').exists():assert R.read(OUT/'protocol.json')==protocol,'Protocol changed; preserve the previous block'
    else:R.save(OUT/'protocol.json',protocol)
    return protocol,graptor

def build():
    for a in R.ALGS:
      for i,row in enumerate(R.M.ROWS,1):
        for version,source in [('baseline',BASE),('candidate',CAND)]:
            d=path(version,a,row);d.mkdir(parents=True,exist_ok=True)
            for ll in (source/'src').glob('*.ll'):
                if not (d/ll.name).exists():(d/ll.name).symlink_to(ll)
            text=R.M.make_source(a,row,False);program=d/'timed.graph'
            if program.exists():assert program.read_text()==text
            else:program.write_text(text)
            if not (d/'timed').exists():
                env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1')
                logrun([source/'GraphProgram','--ir-backend=cpu',program],d,d/'compile.log',env)
                shutil.move(d/'program.o',d/'timed.o')
                logrun(['g++','-O3','-fopenmp','-no-pie',d/'timed.o',*[source/(n+'.o') for n in R.OBJECTS],HIST/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',d/'timed'],d,d/'link.log')
        R.progress(stage='build_timed_programs',algorithm=a,completed=i,total=61)
    fingerprints={str(p.relative_to(ROOT)):R.sha(p) for p in (ROOT/'versions').rglob('timed')}
    assert len(fingerprints)==488
    R.save(OUT/'timed_fingerprints.json',fingerprints)

def measure(phase,reps):
    fp=R.read(OUT/'timed_fingerprints.json');assert fp and len(fp)==488
    for name,digest in fp.items():assert R.sha(ROOT/name)==digest
    records=[]
    for a in R.ALGS:
      for i,row in enumerate(R.M.ROWS,1):
        for label,source,extra in VARIANTS:
            version='baseline' if source==BASE else 'candidate'
            x=R.invoke('execution_simplification','warmup_'+phase,version,a,row,0,label=label,workers=6,extra=extra)
            assert not x.get('error'),x
        for repeat in range(reps):
            shift=(i+repeat)%len(VARIANTS);order=VARIANTS[shift:]+VARIANTS[:shift]
            if repeat%2:order=order[::-1]
            for label,source,extra in order:
                version='baseline' if source==BASE else 'candidate'
                x=R.invoke('execution_simplification',phase,version,a,row,repeat,label=label,workers=6,extra=extra)
                assert not x.get('error') and x['seconds']>0,x;records.append(x)
        R.save(OUT/(phase+'.json'),records)
        R.progress(stage=phase,algorithm=a,completed=i,total=61,observations=len(records))
    return records

def analyze(phase,graptor):
    records=R.read(OUT/(phase+'.json'));reps=5 if phase.startswith('primary') else 20
    assert len(records)==244*len(VARIANTS)*reps
    groups=collections.defaultdict(list);historical=collections.defaultdict(list)
    for x in records:groups[x['algorithm'],x['graph'],x['label']].append(x)
    for x in graptor:historical[x['algorithm'],x['graph']].append(x)
    rows=[];families={name:[] for name in ['code','baseline_AA','candidate_AA','disabled']};algorithm_diffs={name:collections.defaultdict(list) for name in families}
    for a in R.ALGS:
      for row in R.M.ROWS:
        samples={label:sorted(groups[a,row['graph'],label],key=lambda z:z['repeat']) for label,_,_ in VARIANTS}
        assert all(len(xs)==reps for xs in samples.values())
        arrays={label:R.np.array([x['seconds'] for x in xs]) for label,xs in samples.items()}
        entry={'algorithm':a,'graph':row['graph'],'cohort':row['cohort'],'mean_ms':{k:float(v.mean())*1000 for k,v in arrays.items()},'median_ms':{k:float(R.np.median(v))*1000 for k,v in arrays.items()},'kernel_mean_ms':{k:statistics.mean(x['internal_seconds'] for x in xs)*1000 for k,xs in samples.items()},'tests':{}}
        old=historical[a,row['graph']];assert len(old)==5
        entry['historical_graptor_ms']=statistics.mean(x['seconds'] for x in old)*1000
        entry['historical_graptor_kernel_ms']=statistics.mean(x['internal_seconds'] for x in old)*1000
        for family,left,right,two in [('code','baseline','candidate',False),('baseline_AA','baseline','baseline_AA',True),('candidate_AA','candidate','candidate_AA',True),('disabled','disabled','candidate',False)]:
            diff=R.np.log(arrays[right]/arrays[left]);test={'algorithm':a,'graph':row['graph'],'kind':'case','p':permutation_pvalue(diff,two),'log_ratio':float(diff.mean())}
            families[family].append(test);algorithm_diffs[family][a].append(diff);entry['tests'][family]=test
        rows.append(entry)
    for family,tests in families.items():
        for a,diffs in algorithm_diffs[family].items():
            diff=R.np.mean(diffs,axis=0);tests.append({'algorithm':a,'kind':'algorithm','p':permutation_pvalue(diff,family.endswith('_AA')),'log_ratio':float(diff.mean())})
        adjusted=R.holm([x['p'] for x in tests])
        for test,pvalue in zip(tests,adjusted):test['holm_p']=pvalue
    failures=[dict(test,family=name) for name in ['baseline_AA','candidate_AA'] for test in families[name] if test['holm_p']<.05]
    summary={'phase':phase,'repetitions':reps,'observations':len(records),'AA_stable':not failures,'AA_failures':failures,'by_algorithm':{},'tests':families,'confirmed_slowdowns':[x for x in families['code'] if x['holm_p']<.05] if not failures else None}
    for a in R.ALGS+['overall']:
        xs=[x for x in rows if a=='overall' or x['algorithm']==a]
        totals={k:sum(x['mean_ms'][k] for x in xs) for k,_,_ in VARIANTS}
        summary['by_algorithm'][a]={'cases':len(xs),'totals_ms':totals,'historical_graptor_total_ms':sum(x['historical_graptor_ms'] for x in xs),'geomean_speedup_baseline_over_candidate':gm(x['mean_ms']['baseline']/x['mean_ms']['candidate'] for x in xs),'median_geomean_speedup':gm(x['median_ms']['baseline']/x['median_ms']['candidate'] for x in xs),'summed_time_reduction_percent':100*(1-totals['candidate']/totals['baseline']),'kernel_geomean_speedup':gm(x['kernel_mean_ms']['baseline']/x['kernel_mean_ms']['candidate'] for x in xs),'kernel_summed_time_reduction_percent':100*(1-sum(x['kernel_mean_ms']['candidate'] for x in xs)/sum(x['kernel_mean_ms']['baseline'] for x in xs)),'geomean_speedup_disabled_over_candidate':gm(x['mean_ms']['disabled']/x['mean_ms']['candidate'] for x in xs),'candidate_over_historical_graptor_geomean':gm(x['mean_ms']['candidate']/x['historical_graptor_ms'] for x in xs),'candidate_wins_vs_historical_graptor':sum(x['mean_ms']['candidate']<x['historical_graptor_ms'] for x in xs)}
    result={'summary':summary,'cases':rows};R.save(OUT/(phase+'_summary.json'),result);return result

def report(protocol,results):
    chosen=next(reversed(results));result=results[chosen];s=result['summary'];rows=result['cases']
    columns=['algorithm','graph','cohort','baseline_e2e_ms','candidate_e2e_ms','disabled_e2e_ms','change_percent','baseline_kernel_ms','candidate_kernel_ms','historical_graptor_e2e_ms','historical_graptor_kernel_ms','candidate_over_historical_graptor','code_slowdown_holm_p']
    with (OUT/'per_graph_times.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=columns);writer.writeheader()
        for x in rows:
            t=x['mean_ms'];k=x['kernel_mean_ms'];writer.writerow(dict(zip(columns,[x['algorithm'],x['graph'],x['cohort'],t['baseline'],t['candidate'],t['disabled'],100*(t['candidate']/t['baseline']-1),k['baseline'],k['candidate'],x['historical_graptor_ms'],x['historical_graptor_kernel_ms'],t['candidate']/x['historical_graptor_ms'],x['tests']['code']['holm_p']])))
    lines=['Execution simplification: local six-core benchmark','',f"Candidate {protocol['candidate_commit']}; baseline {protocol['baseline_commit']}.",f"Selected block: {chosen}; identical-binary controls stable: {s['AA_stable']}.",'Speedups are baseline/candidate: above 1 favors simplification. Positive reduction percentages mean less time.','Graptor values are reused from the prior local C3 six-worker run, with double PageRank. They are cross-session descriptive comparisons, not paired statistical tests. Graptor was not executed.','', 'Algorithm | E2E geomean speedup | Summed E2E reduction | Kernel geomean speedup | Summed kernel reduction']
    for a,x in s['by_algorithm'].items():lines.append(f"{a} | {x['geomean_speedup_baseline_over_candidate']:.4f}x | {x['summed_time_reduction_percent']:+.2f}% | {x['kernel_geomean_speedup']:.4f}x | {x['kernel_summed_time_reduction_percent']:+.2f}%")
    lines+=['',f"Corrected code slowdowns: {len(s['confirmed_slowdowns']) if s['AA_stable'] else 'inference unavailable: failed stability control'}.",'One warmup per variant/case/block; five primary and twenty independent confirmation observations. All observations retained. Paired log-ratio sign permutation tests: seed 2358, 100000 resamples, Holm correction over 244 cases plus four algorithms per family. No promotion or merge is performed.']
    text='\n'.join(lines)+'\n';(OUT/'final_report.txt').write_text(text)
    doc='<!doctype html><meta charset="utf-8"><title>Execution simplification local benchmark</title><style>body{font:16px system-ui;margin:30px;line-height:1.5}pre{white-space:pre-wrap}td,th{padding:6px;border:1px solid #bbb}table{border-collapse:collapse}</style><h1>Execution simplification local benchmark</h1><pre>'+html.escape(text)+'</pre><p><a href="per_graph_times.csv">Per-graph CSV</a> · <a href="final_summary.json">Complete summary</a></p>'
    for a in R.ALGS:
        doc+='<h2>'+a+'</h2><table><tr><th>Graph</th><th>Baseline ms</th><th>Candidate ms</th><th>Change</th><th>Saved Graptor ms</th><th>Kernel speedup</th></tr>'
        for x in rows:
            if x['algorithm']!=a:continue
            t=x['mean_ms'];k=x['kernel_mean_ms'];vals=[x['graph'],f"{t['baseline']:.4f}",f"{t['candidate']:.4f}",f"{100*(t['candidate']/t['baseline']-1):+.2f}%",f"{x['historical_graptor_ms']:.4f}",f"{k['baseline']/k['candidate']:.3f}x"]
            doc+='<tr>'+''.join('<td>'+html.escape(v)+'</td>' for v in vals)+'</tr>'
        doc+='</table>'
    (OUT/'final_report.html').write_text(doc)
    R.save(OUT/'final_summary.json',{'protocol':protocol,'selected_phase':chosen,'blocks':{k:v['summary'] for k,v in results.items()},'cases':rows,'graptor_new_launches':0})
    print(text,flush=True)

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('stage',choices=['build','measure','report','status']);args=parser.parse_args()
    if args.stage=='status':print(json.dumps(R.read(OUT/'status.json'),indent=2));raise SystemExit
    # Share the existing experiment locks. No compilation/test work overlaps timing.
    with Path('/home/kazi/p1_campaign_20261019/experiment.lock').open('a+') as global_lock,Path('/home/kazi/p1_execution_simplification/experiment.lock').open('a+') as correctness_lock:
      fcntl.flock(global_lock,fcntl.LOCK_EX|fcntl.LOCK_NB);fcntl.flock(correctness_lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
      with R.locked('execution_simplification_'+args.stage):
        protocol,graptor=prepare()
        if args.stage=='build':build()
        else:
          results={}
          for phase,reps in [('primary',5),('confirmation',20)]:
            if args.stage=='measure':measure(phase,reps)
            results[phase]=analyze(phase,graptor)
            if not results[phase]['summary']['AA_stable']:
                retry=phase+'_retry1'
                if args.stage=='measure':measure(retry,reps)
                results[retry]=analyze(retry,graptor)
          report(protocol,results);R.progress(stage='complete',completed=244,total=244)
