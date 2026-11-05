#!/usr/bin/env python3
"""Benchmark the correctness-checked four changes; preserve raw measurements."""
from pathlib import Path
import collections,csv,importlib.util,json,math,platform,re,shutil,statistics,subprocess,time
import benchmark_gpu_saved_cpu as B

REPO=B.REPO
ROOT=Path('/home/kazi/p1_gpu_four_benchmark_20261104/attempt01')
OUT=REPO/'bench_results/gpu_four_benchmark_20261104/attempt01'
CANDIDATE=Path('/home/kazi/p1_gpu_four_correctness_20261104/correct02/native')
PREVIOUS=Path('/home/kazi/p1_gpu_sync_20261104/attempt02/native')
BASELINE=ROOT/'baseline'
CASES=Path('/home/kazi/p1_gpu_batch_20261104/benchmark/cases')
CORRECTNESS=REPO/'bench_results/gpu_four_correctness_20261104/correct02'
SAVED_GPU=REPO/'bench_results/gpu_batch_20261104/benchmark/comparison.json'
FLAGS=['SGPL_GPU_REUSE_RECORDS','SGPL_GPU_EARLY_REJECT','SGPL_GPU_TRANSFER_REDUCE','SGPL_GPU_REUSE_CAPACITY']
SELECTED={('pagerank','dimacs/hamming8-2.edges'),('pagerank','soc/soc-slashdot.edges'),
          ('bfs','soc/soc-slashdot.edges'),('cc','dimacs10/fe-4elt2.edges')}
REPETITIONS=5
OPTIMIZATION='-O2'
BASELINE_LABEL=None

# Windows readers can briefly deny DrvFS atomic replacement. These writes are
# outside measured child execution; retry bookkeeping without replaying samples.
_save=B.save
def resilient_save(path,value):
    for attempt in range(50):
        try:return _save(path,value)
        except PermissionError:
            if attempt==49:raise
            time.sleep(.1)
B.save=resilient_save

def load(p):return json.loads(p.read_text())
def gm(xs):return math.exp(statistics.mean(math.log(x) for x in xs))
def save_status(stage,**extra):B.save(OUT/'status.json',dict(stage=stage,updated_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),**extra))

def summarize(protocol,records):
    complete=[x for x in records if x.get('complete')]
    summaries={}
    for algorithm in ['bfs','sssp','cc','pagerank','all_gpu']:
        rows=[x for x in complete if x['gpu_tasks']>0 and (algorithm=='all_gpu' or x['algorithm']==algorithm)]
        s=dict(gpu_cases=len(rows))
        if rows:
            for metric in ['e2e','compute']:
                for comparison in ['previous_over_current','saved_gpu_over_current','saved_cpu_over_current']:
                    key=comparison+'_'+metric
                    valid=[x[key] for x in rows if key in x]
                    s[key+'_geomean']=gm(valid) if valid else None
                s['wins_vs_previous_'+metric]=sum(x['previous_over_current_'+metric]>1 for x in rows)
                s['wins_vs_saved_cpu_'+metric]=sum(x['saved_cpu_over_current_'+metric]>1 for x in rows)
        summaries[algorithm]=s
    B.save(OUT/'results.json',dict(protocol=protocol,summary=summaries,cases=records))
    columns=['algorithm','graph','gpu_tasks','previous_e2e_ms','current_e2e_ms','previous_compute_ms','current_compute_ms',
             'previous_over_current_e2e','previous_over_current_compute','saved_gpu_e2e_ms','saved_gpu_compute_ms',
             'saved_gpu_over_current_e2e','saved_gpu_over_current_compute','saved_cpu_e2e_ms','saved_cpu_compute_ms',
             'saved_cpu_over_current_e2e','saved_cpu_over_current_compute','complete']
    with (OUT/'comparison.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,columns,extrasaction='ignore');w.writeheader();w.writerows(complete)

def main():
    OUT.mkdir(parents=True,exist_ok=True);ROOT.mkdir(parents=True,exist_ok=True)
    spec=importlib.util.spec_from_file_location('common','/home/kazi/p1_c3_integration_20261022/support/common.py')
    common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
    env=common.env(workers=6);env.update(FORCE_CPU='0',FORCE_GPU='1',SGPL_GPU_BACKEND='1',SGPL_GPU_BATCH='1',SGPL_GPU_SYNC_ELIDE='1')
    env.update({name:'1' for name in FLAGS})
    build=load(CORRECTNESS/'build.json');checks=load(CORRECTNESS/'summary.json');assert checks['passed'] and checks['independent_validation_runs']==488
    assert build.get('native_flags',['-O2','-g','-fopenmp','-pthread'])==[OPTIMIZATION,'-g','-fopenmp','-pthread'],'Candidate/baseline build flags differ'
    for name,digest in build['source_sha256'].items():assert B.sha(REPO/name)==digest,('Native source changed after correctness',name)
    for name,digest in build['objects_sha256'].items():assert B.sha(CANDIDATE/(name+'.o'))==digest
    original_cpu=load(B.CPU_REPORT)
    cpu_model=next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name'))
    assert original_cpu['protocol']['host']['cpu_model']==cpu_model
    save_status('build matched pre-change baseline')
    source=BASELINE/'src'
    if not source.exists():shutil.copytree(PREVIOUS/'src',source)
    for name in B.OBJECTS:
        target=BASELINE/(name+'.o')
        if name not in ['gpu_runtime','autotuner_runtime']:
            if not target.exists():target.symlink_to(CANDIDATE/(name+'.o'))
        elif not target.exists():
            B.run(['gcc',OPTIMIZATION,'-g','-fopenmp','-pthread','-iquote','.', '-c',name+'.c','-o',target],source,name+'.build',env,1200)
    protocol=dict(created_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),repetitions=REPETITIONS,warmups_per_mode=1,
        ordering='Alternating AB/BA fresh processes; start order alternates by case and repetition',
        baseline=BASELINE_LABEL or 'Frozen pre-four runtime with batching and sync elision, rebuilt '+OPTIMIZATION+' -g -fopenmp -pthread',
        candidate='Correctness-checked '+CORRECTNESS.name+' runtime, same native compiler flags',
        candidate_objects_sha256={n:B.sha(CANDIDATE/(n+'.o')) for n in B.OBJECTS},
        baseline_objects_sha256={n:B.sha(BASELINE/(n+'.o')) for n in B.OBJECTS},
        baseline_source_sha256={p.name:B.sha(p) for p in source.iterdir() if p.is_file()},
        candidate_build_sha256=B.sha(CORRECTNESS/'build.json'),correctness_summary_sha256=B.sha(CORRECTNESS/'summary.json'),
        compiler_sha256=B.sha(CANDIDATE/'GraphProgram'),timer_sha256=B.sha(B.HIST/'monotonic_timer.o'),
        walltime_sha256=B.sha(B.CPU_ROOT/'walltime'),saved_cpu_sha256=B.sha(B.CPU_REPORT),saved_gpu_sha256=B.sha(SAVED_GPU),
        runtime_environment={k:v for k,v in env.items() if k.startswith(('SGPL_','OMP_','GOMP_','FORCE_'))},
        gpu=subprocess.check_output(['nvidia-smi','--query-gpu=name,driver_version,memory.total','--format=csv,noheader'],text=True).strip(),
        platform=platform.platform(),cpu_model=cpu_model,
        historical_comparison='Saved CPU and saved batched GPU are unpaired historical means; CPU is not rebenchmarked.',
        timing='Same CLOCK_MONOTONIC wrapper and compute timer interposer as saved runs; no debug/profiling in timed samples.',
        desktop_gpu=True)
    results=OUT/'results.json'
    if results.exists():
        previous=load(results)
        for k,v in protocol.items():
            if k!='created_utc':assert previous['protocol'][k]==v,('Protocol changed',k)
        protocol=previous['protocol'];records=previous['cases']
    else:records=[]
    saved_cpu={(x['algorithm'],x['graph']):x for x in original_cpu['cases']}
    saved_gpu={(x['algorithm'],x['graph']):x for x in load(SAVED_GPU)['cases']}
    correctness={(x['algorithm'],x['graph']):x for x in load(CORRECTNESS/'validation.json') if x['controls']=='enabled'}
    index={(x['algorithm'],x['graph']):x for x in records}
    for row in common.ROWS:
      for algorithm in ['bfs','sssp','cc','pagerank']:
        key=(algorithm,row['graph']);entry=index.get(key)
        if entry and entry.get('complete'):continue
        number=len([x for x in records if x.get('complete')])+1
        print('START',number,'of 244',algorithm,row['graph'],flush=True)
        d=ROOT/'cases'/algorithm/row['graph'].removesuffix('.edges');d.mkdir(parents=True,exist_ok=True)
        obj=CASES/algorithm/row['graph'].removesuffix('.edges')/'timed.o'
        timed_source=obj.with_suffix('.graph')
        cpu_source=B.CPU_ROOT/'versions/candidate/cases'/algorithm/row['graph'].removesuffix('.edges')/'timed.graph'
        assert B.sha(timed_source)==B.sha(cpu_source),'Timed source differs from saved CPU'
        assert B.sha(Path(row['p1_weighted_file'] if algorithm=='sssp' else row['original_file']))==saved_gpu[key]['input_sha256']
        if not entry:
            entry=dict(algorithm=algorithm,graph=row['graph'],samples={'previous':[],'current':[]},diagnostics={},complete=False,
                source_sha256=B.sha(timed_source),timed_object_sha256=B.sha(obj),executables_sha256={})
            records.append(entry);index[key]=entry
        for mode,native in [('previous',BASELINE),('current',CANDIDATE)]:
            executable=d/mode
            if not executable.exists():
                B.run(['g++',OPTIMIZATION,'-fopenmp','-no-pie',obj,*[native/(n+'.o') for n in B.OBJECTS],B.HIST/'monotonic_timer.o',
                    '-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',executable],d,mode+'.link',env,1200)
            digest=B.sha(executable)
            if mode in entry['executables_sha256']:assert entry['executables_sha256'][mode]==digest
            entry['executables_sha256'][mode]=digest
            if mode not in entry['diagnostics']:
                save_status('diagnostic warmup',case=number,algorithm=algorithm,graph=row['graph'],mode=mode)
                B.run([executable],d,'warmup_'+mode,dict(env,SGPL_GPU_DEBUG='1',SGPL_GPU_PREPARATION_STATS='1'),600)
                trace=(d/('warmup_'+mode+'.err')).read_text()
                entry['diagnostics'][mode]=dict(gpu_tasks=trace.count('[gpu-task]'),template_hits=trace.count('[gpu-record-reuse] hit'),early_refusals=trace.count('existing early gate'))
                summarize(protocol,records)
        assert entry['diagnostics']['previous']['gpu_tasks']==entry['diagnostics']['current']['gpu_tasks']==correctness[key]['gpu_tasks'],('Offload selection changed',key)
        for repeat in range(REPETITIONS):
            for mode in (['previous','current'] if (number+repeat)%2==0 else ['current','previous']):
                if len(entry['samples'][mode])>repeat:continue
                save_status('timing',case=number,algorithm=algorithm,graph=row['graph'],mode=mode,repeat=repeat+1)
                tag=mode+'_'+str(repeat);stamp=d/(tag+'.stamp.json')
                if stamp.exists():output=(d/(tag+'.out')).read_text()
                else:output=B.run([B.CPU_ROOT/'walltime',stamp,d/mode],d,tag,env,600)
                sample=load(stamp);assert sample['exit_code']==0
                sample['internal_seconds']=float(re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)',output)[1])
                assert sample['seconds']>0 and sample['internal_seconds']>0
                entry['samples'][mode].append(sample);summarize(protocol,records)
        entry['gpu_tasks']=entry['diagnostics']['current']['gpu_tasks']
        for mode in ['previous','current']:
            for metric,field in [('e2e','seconds'),('compute','internal_seconds')]:
                values=[x[field]*1000 for x in entry['samples'][mode]]
                entry[mode+'_'+metric+'_ms']=statistics.mean(values)
                entry[mode+'_'+metric+'_median_ms']=statistics.median(values)
                entry[mode+'_'+metric+'_stdev_ms']=statistics.stdev(values)
        for metric in ['e2e','compute']:
            entry['previous_over_current_'+metric]=entry['previous_'+metric+'_ms']/entry['current_'+metric+'_ms']
            entry['saved_cpu_'+metric+'_ms']=saved_cpu[key]['mean_ms' if metric=='e2e' else 'kernel_mean_ms']['candidate']
            entry['saved_cpu_over_current_'+metric]=entry['saved_cpu_'+metric+'_ms']/entry['current_'+metric+'_ms']
            if saved_gpu[key]['status']=='gpu_validated':
                entry['saved_gpu_'+metric+'_ms']=saved_gpu[key]['gpu_'+metric+'_ms']
                entry['saved_gpu_over_current_'+metric]=entry['saved_gpu_'+metric+'_ms']/entry['current_'+metric+'_ms']
        entry['complete']=True;summarize(protocol,records)
        print('DONE',number,algorithm,row['graph'],'GPU tasks',entry['gpu_tasks'],'speedup e2e/compute',
            round(entry['previous_over_current_e2e'],3),round(entry['previous_over_current_compute'],3),flush=True)
    assert len(records)==244 and all(x['complete'] for x in records)
    controls_path=OUT/'controls.json';controls=load(controls_path) if controls_path.exists() else []
    for algorithm,graph in sorted(SELECTED):
        if any((x['algorithm'],x['graph'])==(algorithm,graph) for x in controls):continue
        d=ROOT/'cases'/algorithm/graph.removesuffix('.edges');binary=d/'current'
        modes={'all_on':{},'all_off':{f:'0' for f in FLAGS}}
        modes.update({f+'_off':{f:'0'} for f in FLAGS})
        samples={mode:[] for mode in modes};diagnostics={}
        for mode,changes in modes.items():
            if not (d/('ablation_warmup_'+mode+'.out')).exists():
                B.run([binary],d,'ablation_warmup_'+mode,dict(env,**changes,SGPL_GPU_DEBUG='1'),600)
            diagnostics[mode]=(d/('ablation_warmup_'+mode+'.err')).read_text().count('[gpu-task]')
        assert len(set(diagnostics.values()))==1 and next(iter(diagnostics.values()))>0
        names=list(modes)
        for repeat in range(REPETITIONS):
            order=names[repeat%len(names):]+names[:repeat%len(names)]
            if repeat%2:order=order[::-1]
            for mode in order:
                tag='ablation_'+mode+'_'+str(repeat);stamp=d/(tag+'.stamp.json')
                if stamp.exists():
                    output=(d/(tag+'.out')).read_text()
                else:
                    save_status('same-binary controls',algorithm=algorithm,graph=graph,mode=mode,repeat=repeat+1)
                    output=B.run([B.CPU_ROOT/'walltime',stamp,binary],d,tag,dict(env,**modes[mode]),600)
                sample=load(stamp);assert sample['exit_code']==0
                sample['internal_seconds']=float(re.search(r'P1_COMPUTE_SECONDS\s+([0-9.eE+-]+)',output)[1]);samples[mode].append(sample)
        entry=dict(algorithm=algorithm,graph=graph,diagnostic_gpu_tasks=diagnostics,samples=samples,
            note='Same binary switches cover conditional paths; unconditional preparation cleanup remains enabled in all modes.')
        for metric,field in [('e2e','seconds'),('compute','internal_seconds')]:
            means={m:statistics.mean(s[field]*1000 for s in ss) for m,ss in samples.items()}
            entry[metric+'_means_ms']=means;entry[metric+'_off_over_all_on']={m:v/means['all_on'] for m,v in means.items()}
        controls.append(entry);B.save(controls_path,controls)
        print('CONTROL DONE',algorithm,graph,flush=True)
    save_status('complete',cases=244,samples=244*2*REPETITIONS,selected_control_cases=len(controls))
    print('PASS full corpus benchmarks and selected same-binary controls',flush=True)

if __name__=='__main__':main()
