#!/usr/bin/env python3
"""Frozen paired R16--R18 benchmark. Saved Graptor is read, never run."""
from pathlib import Path
import argparse,collections,csv,fcntl,hashlib,html,json,os,platform,shutil
import benchmark_execution_simplification as B
R=B.R;REPO=B.REPO
ROOT=Path('/home/kazi/p1_iteration_lifetimes_benchmark_20261024')
OUT=Path('/mnt/c/p1-iteration-lifetimes-benchmark-20261024')
BASE=Path('/home/kazi/p1_execution_simplification/versions/proof_removals04')
CAND=Path('/home/kazi/p1_execution_simplification/versions/iteration_lifetimes01')
OLD=Path('/home/kazi/p1_proof_removals_benchmark_20261024')
CORRECT=REPO/'reports/iteration_lifetimes_correctness.json'
ROOT.mkdir(exist_ok=True);OUT.mkdir(exist_ok=True)
B.ROOT=R.ROOT=ROOT;B.OUT=R.OUT=OUT;B.BASE=BASE;B.CAND=CAND
B.VARIANTS=[('baseline',BASE,{}),('candidate',CAND,{}),('disabled',CAND,{'SGPL_NO_ITERATION_LIFETIMES':'1'}),('baseline_AA',BASE,{}),('candidate_AA',CAND,{})]
os.environ.update(P1_CAMPAIGN_ROOT=str(ROOT),P1_CAMPAIGN_OUTPUT=str(OUT),P1_CAMPAIGN_CPUS='0,2,4,6,8,10')

def prepare():
    old=Path('/home/kazi/p1_c3_integration_20261022')
    for name in ['references','frozen_reports','historical_harness','support']:
        if not (ROOT/name).exists():(ROOT/name).symlink_to(old/name,target_is_directory=True)
    for name in ['inputs.json','walltime']:
        if not (ROOT/name).exists():shutil.copy2(old/name,ROOT/name)
    R.M=R.bootstrap();manifest=R.read(B.HIST/'results/manifest.json')
    assert R.M.ROWS==manifest['cases']
    for name,digest in manifest['inputs'].items():assert R.sha(name)==digest,('Input changed',name)
    correct=R.read(CORRECT);previous=R.read(REPO/'reports/proof_removals_correctness.json')
    assert correct['passed'] and all(x['passed'] for x in correct['checks'].values())
    for source,evidence in [(BASE,previous),(CAND,correct)]:
        assert R.read(source/'identity.json')['source_hash']==evidence['source_hash']
        assert R.sha(source/'GraphProgram')==evidence['build']['compiler_sha256']
        for name,digest in evidence['build']['objects'].items():assert R.sha(source/(name+'.o'))==digest
    files=[p for p in REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    assert R.key({str(p.relative_to(REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})==correct['source_hash']
    topology=[];seen=set()
    for cpu in R.CPUS:
        d=Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'topology';key=tuple((d/n).read_text().strip() for n in ['physical_package_id','core_id'])
        assert key not in seen;seen.add(key);topology.append(dict(cpu=cpu,package=key[0],core=key[1]))
    host=dict(platform=platform.platform(),cpu_model=next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),physical_cpus=R.CPUS,workers=6,topology=topology)
    assert host['cpu_model']==R.read(B.HIST/'host.json')['cpu_model']
    graptor=[x for x in R.read(B.HIST/'results/primary_w6.json') if x['label']=='graptor']
    assert len(graptor)==1220 and all(x['workers']==6 and not x.get('error') and x['seconds']>0 for x in graptor)
    for x in graptor:
        assert x['version']==('graptor_double' if x['algorithm']=='pagerank' else 'graptor')
        assert R.read(Path(x['base']+'.record.json'))==x
    protocol=dict(harness_sha256=R.sha(Path(__file__)),shared_harness_sha256=R.sha(REPO/'tools/benchmark_execution_simplification.py'),runner_sha256=R.sha(REPO/'tools/p1_campaign/runner.py'),
        monotonic_timer_sha256=R.sha(B.HIST/'monotonic_timer.o'),walltime_sha256=R.sha(ROOT/'walltime'),
        baseline_commit=previous['source_commit'],baseline_label='Retained R1--R15 and iteration-memory improvements',
        baseline_integration_commit=R.git('rev-parse','ARS-1'),candidate_commit=correct['source_commit'],source_hash=correct['source_hash'],
        candidate_label='R16 modular source counts, R17 dead final restoration removal, R18 source-local contribution storage reuse; composable proof worklists',
        host=host,dataset_hash=manifest['dataset_hash'],cases=R.M.ROWS,primary_repetitions=5,confirmation_repetitions=20,warmups=1,
        variants=[x[0] for x in B.VARIANTS],disabled_control='Same candidate with SGPL_NO_ITERATION_LIFETIMES=1; all preceding iteration-memory improvements remain enabled.',
        timer='Whole-process CLOCK_MONOTONIC; internal whole-algorithm timer includes preparation.',
        historical_graptor=dict(records=str(B.HIST/'results/primary_w6.json'),sha256=R.sha(B.HIST/'results/primary_w6.json'),precision='double PageRank',note='Saved same-PC six-worker observations; historical/unpaired; zero new Graptor launches.'),
        correctness_report_sha256=R.sha(CORRECT),calibration_sha256=R.sha(Path.home()/'.config/sgpl/hw_calib.json'),
        native_fingerprints={label:dict(compiler=R.sha(d/'GraphProgram'),objects={n:R.sha(d/(n+'.o')) for n in R.OBJECTS}) for label,d in [('baseline',BASE),('candidate',CAND)]})
    if (OUT/'protocol.json').exists():assert R.read(OUT/'protocol.json')==protocol,'Protocol changed'
    else:R.save(OUT/'protocol.json',protocol)
    return protocol,graptor

def build():
    fp=R.read('/mnt/c/p1-proof-removals-benchmark-20261024/timed_fingerprints.json');reuse=[]
    for alg in R.ALGS:
      for row in R.M.ROWS:
        prior=OLD/'versions/candidate/cases'/alg/row['graph'].removesuffix('.edges')
        key='versions/candidate/cases/'+alg+'/'+row['graph'].removesuffix('.edges')+'/timed'
        assert R.sha(prior/'timed')==fp[key] and (prior/'timed.graph').read_text()==R.M.make_source(alg,row,False)
        target=B.path('baseline',alg,row);target.mkdir(exist_ok=True,parents=True)
        for name in ['timed.graph','timed.o','timed','compile.log','link.log']:
            if not (target/name).exists():(target/name).symlink_to(prior/name)
        reuse.append(dict(algorithm=alg,graph=row['graph'],sha256=fp[key],executable=str(prior/'timed')))
    R.save(OUT/'baseline_executable_reuse.json',reuse);B.build()
    for alg in R.ALGS:
      for row in R.M.ROWS:
        log=(B.path('candidate',alg,row)/'compile.log').read_text()
        assert ('[iteration-memory] activate ' in log)==(alg=='pagerank'),(alg,row['graph'],'iteration proof activation')
        if alg=='pagerank':
            assert '[proof-removals] R13 certify' in log,(alg,row['graph'])
            for rule in [16,17,18]:assert f'[iteration-lifetime] R{rule} certify' in log,(alg,row['graph'],rule)
        if alg=='sssp':
            for marker in ['[exec-compose] R8 activate','[proof-removals] R14 certify','[proof-removals] R15 certify']:
                assert marker in log,(alg,row['graph'],marker)

def report(protocol,results):
    phase=next(reversed(results));selected=results[phase];rows=selected['cases'];summary=selected['summary']
    counts={}
    for alg in R.ALGS:
        cases=[x for x in rows if x['algorithm']==alg]
        counts[alg]=dict(graphs=len(cases),wins_vs_retained_baseline=sum(x['mean_ms']['candidate']<x['mean_ms']['baseline'] for x in cases),wins_vs_saved_graptor=sum(x['mean_ms']['candidate']<x['historical_graptor_ms'] for x in cases),
            compute_wins_vs_retained_baseline=sum(x['kernel_mean_ms']['candidate']<x['kernel_mean_ms']['baseline'] for x in cases),
            compute_wins_vs_saved_graptor=sum(x['kernel_mean_ms']['candidate']<x['historical_graptor_kernel_ms'] for x in cases))
    data=dict(protocol=protocol,selected_phase=phase,blocks={k:v['summary'] for k,v in results.items()},cases=rows,win_counts=counts)
    R.save(OUT/'final_summary.json',data)
    columns=['algorithm','graph','baseline_e2e_ms','candidate_e2e_ms','disabled_e2e_ms','baseline_compute_ms','candidate_compute_ms','saved_graptor_e2e_ms','saved_graptor_compute_ms','slowdown_holm_p']
    with (OUT/'per_graph_times.csv').open('w',newline='') as f:
        writer=csv.writer(f);writer.writerow(columns)
        for x in rows:writer.writerow([x['algorithm'],x['graph'],x['mean_ms']['baseline'],x['mean_ms']['candidate'],x['mean_ms']['disabled'],x['kernel_mean_ms']['baseline'],x['kernel_mean_ms']['candidate'],x['historical_graptor_ms'],x['historical_graptor_kernel_ms'],x['tests']['code']['holm_p']])
    lines=['Composable iteration lifetime simplification (R16--R18): local six-core benchmark','',protocol['candidate_label'],
        f"Retained baseline native source {protocol['baseline_commit']}; candidate native source {protocol['candidate_commit']}.",
        f"Selected block: {phase}; identical-binary controls stable: {summary['AA_stable']}.",
        'Six physical cores; five primary and twenty independent confirmation observations; one warmup per variant/case/block.',
        'The measured baseline includes retained iteration-memory and R1--R15 improvements; this campaign adds R16--R18 only. Saved Graptor observations are historical and unpaired. No new cost models.',
        'End-to-end is the whole process. Compute includes the whole algorithm and its preparation, not just callbacks.',
        'Wins count lower mean end-to-end time; they are not necessarily statistically significant.','',
        'Algorithm | end-to-end geomean speedup | summed time reduction | compute geomean speedup | E2E wins vs retained baseline | E2E wins vs saved Graptor | compute wins vs retained baseline | compute wins vs saved Graptor']
    table=[]
    for alg in R.ALGS:
        cases=[x for x in rows if x['algorithm']==alg];times=[x['mean_ms'] for x in cases]
        gm=B.gm(t['baseline']/t['candidate'] for t in times);reduction=100*(1-sum(t['candidate'] for t in times)/sum(t['baseline'] for t in times))
        compute=B.gm(x['kernel_mean_ms']['baseline']/x['kernel_mean_ms']['candidate'] for x in cases)
        row=[alg,f'{gm:.4f}x',f'{reduction:+.2f}%',f'{compute:.4f}x',str(counts[alg]['wins_vs_retained_baseline'])+'/61',str(counts[alg]['wins_vs_saved_graptor'])+'/61',str(counts[alg]['compute_wins_vs_retained_baseline'])+'/61',str(counts[alg]['compute_wins_vs_saved_graptor'])+'/61'];table.append(row);lines.append(' | '.join(row))
    lines+=['','Speedup is retained baseline / candidate; greater than 1 favors R16--R18.',
        'All observations and any control-failure retries are retained. Paired permutation tests use Holm correction; saved Graptor means are descriptive cross-session comparisons.',
        f"Corrected slowdowns: {len(summary['confirmed_slowdowns']) if summary['AA_stable'] else 'inference unavailable: stability control failed'}. No compiler/runtime changes or merge are performed during this campaign."]
    R.save(OUT/'summary_metrics.json',dict(by_algorithm=summary['by_algorithm'],win_counts=counts))
    (OUT/'final_report.txt').write_text('\n'.join(lines)+'\n')
    body='<h1>Composable iteration lifetime simplification (R16--R18)</h1>'+''.join('<p>'+html.escape(x)+'</p>' for x in lines[2:9])
    body+='<table><thead><tr>'+''.join('<th>'+html.escape(x)+'</th>' for x in lines[10].split(' | '))+'</tr></thead><tbody>'
    body+=''.join('<tr>'+''.join('<td>'+html.escape(v)+'</td>' for v in row)+'</tr>' for row in table)+'</tbody></table>'
    body+='<p><a href="per_graph_times.csv">Per-graph CSV</a> · <a href="final_summary.json">Full machine-readable report</a> · <a href="integrity_audit.json">Integrity audit</a></p><h2>Per-graph mean end-to-end times (ms)</h2><table><tr><th>Algorithm</th><th>Graph</th><th>Retained baseline</th><th>Candidate</th><th>Disabled</th><th>Saved Graptor</th></tr>'
    for x in rows:body+='<tr>'+''.join('<td>'+html.escape(v)+'</td>' for v in [x['algorithm'],x['graph'],*[f'{x["mean_ms"][v]:.5f}' for v in ['baseline','candidate','disabled']],f'{x["historical_graptor_ms"]:.5f}'])+'</tr>'
    body+='</table>'
    (OUT/'final_report.html').write_text('<!doctype html><meta charset="utf-8"><title>Composable iteration-lifetime benchmark</title><style>body{font:15px system-ui;margin:32px;color:#18212c}table{border-collapse:collapse;margin:24px 0}th,td{padding:8px 12px;border-bottom:1px solid #dce2e8;text-align:right}th:first-child,td:first-child,td:nth-child(2){text-align:left}thead,th{background:#eef3f8}h1{font-size:28px}</style>'+body)
    return data

def audit(protocol):
    final=R.read(OUT/'final_summary.json');fingerprints=R.read(OUT/'timed_fingerprints.json')
    files=[p for p in REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    files += [p for p in (REPO/'generated').glob('*') if p.suffix in {'.cpp','.h'}]
    assert R.key({str(p.relative_to(REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files})==protocol['source_hash']
    for label,d in [('baseline',BASE),('candidate',CAND)]:
        assert R.sha(d/'GraphProgram')==protocol['native_fingerprints'][label]['compiler']
        for name,digest in protocol['native_fingerprints'][label]['objects'].items():assert R.sha(d/(name+'.o'))==digest
    for name,digest in fingerprints.items():assert R.sha(ROOT/name)==digest
    assert R.git('rev-parse','ARS-1')==protocol['baseline_integration_commit']
    assert R.sha(Path.home()/'.config/sgpl/hw_calib.json')==protocol['calibration_sha256']
    assert R.sha(CORRECT)==protocol['correctness_report_sha256']
    assert R.sha(B.HIST/'results/primary_w6.json')==protocol['historical_graptor']['sha256']
    assert R.sha(Path(__file__))==protocol['harness_sha256']
    assert R.sha(REPO/'tools/benchmark_execution_simplification.py')==protocol['shared_harness_sha256']
    assert R.sha(REPO/'tools/p1_campaign/runner.py')==protocol['runner_sha256']
    assert R.sha(B.HIST/'monotonic_timer.o')==protocol['monotonic_timer_sha256']
    assert R.sha(ROOT/'walltime')==protocol['walltime_sha256']
    total=warmups=0
    for phase in final['blocks']:
        records=R.read(OUT/(phase+'.json'));assert len(records)==244*5*(20 if phase.startswith('confirmation') else 5)
        assert all(not x.get('error') for x in records);total+=len(records);warmups+=244*5
    evidence=dict(passed=True,protocol=protocol,selected_phase=final['selected_phase'],blocks=final['blocks'],win_counts=final['win_counts'],
        timing_observations=total,warmups=warmups,failed_launches=0,graptor_new_launches=0,fingerprints_verified=len(fingerprints),native_changes_during_measurement=False,
        artifacts=dict(root=str(OUT),html=str(OUT/'final_report.html'),csv=str(OUT/'per_graph_times.csv'),summary=str(OUT/'final_summary.json')))
    R.save(OUT/'integrity_audit.json',evidence);R.save(REPO/'reports/iteration_lifetimes_benchmark.json',evidence)
    print(json.dumps(dict(passed=True,observations=total,win_counts=final['win_counts'],by_algorithm=final['blocks'][final['selected_phase']]['by_algorithm']),indent=2),flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['build','measure','report','audit','status']);a=p.parse_args()
    if a.stage=='status':print(json.dumps(R.read(OUT/'status.json'),indent=2));raise SystemExit
    with Path('/home/kazi/p1_campaign_20261019/experiment.lock').open('a+') as global_lock,Path('/home/kazi/p1_execution_simplification/experiment.lock').open('a+') as correctness_lock:
        fcntl.flock(global_lock,fcntl.LOCK_EX|fcntl.LOCK_NB);fcntl.flock(correctness_lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        with R.locked('iteration_lifetimes_'+a.stage):
            protocol,graptor=prepare()
            if a.stage=='build':build()
            elif a.stage=='audit':audit(protocol)
            else:
                results={}
                for phase,reps in [('primary',5),('confirmation',20)]:
                    if a.stage=='measure':B.measure(phase,reps)
                    results[phase]=B.analyze(phase,graptor)
                    if not results[phase]['summary']['AA_stable']:
                        retry=phase+'_retry1'
                        if a.stage=='measure':B.measure(retry,reps)
                        results[retry]=B.analyze(retry,graptor)
                report(protocol,results);audit(protocol);R.progress(stage='complete',completed=244,total=244)
