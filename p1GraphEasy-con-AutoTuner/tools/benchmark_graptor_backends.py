#!/usr/bin/env python3
"""Controlled same-GCC Graptor OpenMP/Cilk API comparison; no SGPL changes."""
from pathlib import Path
import argparse,collections,contextlib,fcntl,hashlib,html,importlib.util,json,math,os,re,shutil,statistics,subprocess,sys,time

REPO=Path(__file__).resolve().parents[1]
ROOT=Path('/home/kazi/p1_graptor_backends_20261026')
OUT=Path('/mnt/c/p1-graptor-backends-benchmark-20261026')
HIST=Path('/home/kazi/p1_effect_fusion_local_20261022')
SRC=Path('/mnt/c/p1-effect-fusion-20261021/B3safe2-export/export/bundle/graptor_source')
RTS=ROOT/'cilkrts-install';DEP=HIST/'dependency'
CPUS=[0,2,4,6,8,10];ALGS=['bfs','sssp','cc','pagerank']
TARGETS={'bfs':('BFSExport','BFSvExport.C'),'sssp':('BFExport','BFvExport.C'),'cc':('CCv_VEBOGraptorPullDataParCached','CCv.C'),'pagerank':('PRdouble','PRdouble.C')}
VARIANTS=['openmp','cilk','openmp_rebuilt','openmp_AA','cilk_AA']
ROOT.mkdir(exist_ok=True);OUT.mkdir(exist_ok=True)
spec=importlib.util.spec_from_file_location('frozen_graptor_common',HIST/'support/common.py');M=importlib.util.module_from_spec(spec);spec.loader.exec_module(M)
ROWS=M.ROWS
sys.path.insert(0,str(REPO/'tools/p1_campaign'))
from portable_campaign import permutation_pvalue

def sha(p):
    h=hashlib.sha256()
    with open(p,'rb') as f:
        for b in iter(lambda:f.read(1024*1024),b''):h.update(b)
    return h.hexdigest()
def save(p,data):
    p.parent.mkdir(exist_ok=True,parents=True);temp=p.with_suffix(p.suffix+'.tmp');temp.write_text(json.dumps(data,indent=2));temp.replace(p)
def checked(cmd,log,cwd=ROOT,env=None):
    log.parent.mkdir(exist_ok=True,parents=True)
    with open(log,'w') as f:p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=1800)
    if p.returncode:raise RuntimeError(str(log)+'\n'+log.read_text()[-3500:])
def progress(stage,**kw):
    data=dict(stage=stage,updated=time.time(),**kw);save(OUT/'status.json',data);print(json.dumps(data),flush=True)
def exe(label,alg):
    label={'openmp_AA':'openmp','cilk_AA':'cilk'}.get(label,label)
    return (HIST/'graptor' if label=='openmp' else ROOT/'build'/label)/TARGETS[alg][0]
def env(label,width):
    e=M.env('graptor',width)
    for k in list(e):
        if k.startswith(('CILK_','PARLAY_')):e.pop(k)
    e.update(CILK_NWORKERS=str(width),LD_LIBRARY_PATH=str(RTS/'lib')+':'+str(DEP/'lib'),LD_PRELOAD=str(HIST/'gclock.so'))
    return e
def source_hashes():return {str(p.relative_to(SRC)):sha(p) for p in sorted(SRC.rglob('*')) if p.is_file()}
def frozen_protocol():
    p=OUT/'protocol.json';data=json.loads(p.read_text())
    assert data['source_hashes']==source_hashes(),'Frozen Graptor source changed'
    for key,digest in data['binaries'].items():
        label,alg=key.split(':');assert sha(exe(label,alg))==digest,key
    for path,digest in data['input_hashes'].items():assert sha(path)==digest,path
    for path,digest in data['dependencies'].items():assert sha(path)==digest,path
    for path,digest in data['configurations'].items():assert sha(path)==digest,path
    for path,digest in data['preflight_evidence'].items():assert sha(path)==digest,path
    assert sha(Path(__file__))==data['harness_sha256'],'Harness changed after freeze'
    return data

def build():
    assert len(ROWS)==61
    saved=json.loads((HIST/'results/manifest.json').read_text())
    bundle_manifest=SRC.parent/'bundle_manifest.json';original=json.loads(bundle_manifest.read_text())
    for p in SRC.rglob('*'):
        if p.is_file():assert sha(p)==original[str(p.relative_to(SRC.parent))],p
    for alg,(name,_) in TARGETS.items():assert sha(exe('openmp',alg))==saved['graptor'][name],name
    worker_probe=json.loads((OUT/'worker_probe.json').read_text());assert worker_probe['passed']
    assert all(c['configured_workers']==c['observed_workers'] and not c['affinity_violations'] for c in worker_probe['checks'])
    hashes=source_hashes();commands={};base=(SRC/'config.h').read_text()
    assert base.count('#define GRAPTOR_PARALLEL BACKEND_openmp')==1
    common=['g++','-O3','-DNDEBUG','-std=c++23','-DGRAPHTYPE=VEBOGraptorPullDataParCached','-O4','-Wno-ignored-attributes','-ftemplate-backtrace-limit=0','-g','-DLONG','-UNDEBUG','-march=native','-DMAX_VL=8','-I'+str(DEP/'include')]
    for label in ['cilk','openmp_rebuilt']:
        d=ROOT/'build'/label;config=d/'config';config.mkdir(exist_ok=True,parents=True)
        (config/'config.h').write_text(base.replace('#define GRAPTOR_PARALLEL BACKEND_openmp','#define GRAPTOR_PARALLEL BACKEND_cilk') if label=='cilk' else base)
        flags=[*common,'-I'+str(config),'-I'+str(RTS/'include'),'-I'+str(SRC),'-I'+str(SRC/'include'),'-I'+str(SRC/'bench')]
        if label=='openmp_rebuilt':flags.insert(1,'-fopenmp')
        constants=d/'constants.o';cmd=[*flags,'-c',SRC/'src/constants.C','-o',constants]
        if not constants.exists():checked(cmd,d/'constants.log')
        commands[label+':constants']=list(map(str,cmd))
        for alg,(target,file) in TARGETS.items():
            cmd=[*flags,SRC/'bench'/file,constants,'-L'+str(DEP/'lib'),'-lconfig++']
            if label=='cilk':cmd+=['-L'+str(RTS/'lib'),'-lcilkrts','-ldl','-pthread']
            cmd+=['-o',d/target];commands[label+':'+alg]=list(map(str,cmd))
            progress('build',variant=label,algorithm=alg)
            if not (d/target).exists():checked(cmd,d/(target+'.build.log'))
    assert hashes==source_hashes()
    inputs={}
    for r in ROWS:
        for k in ['original_file','bin','p1_weighted_file','weights','weights_i32']:inputs[r[k]]=sha(r[k])
        assert inputs[r['original_file']]==r['original_sha256']
    for path,digest in inputs.items():assert digest==saved['inputs'][path],path
    dependencies={str(p):sha(p) for p in [RTS/'lib/libcilkrts.so.5',HIST/'gclock.so',HIST/'walltime',*DEP.glob('lib/*.so*')] if p.is_file()}
    binaries={label+':'+alg:sha(exe(label,alg)) for label in ['openmp','cilk','openmp_rebuilt'] for alg in ALGS}
    for label in ['openmp','cilk','openmp_rebuilt']:
      for alg in ALGS:
        checked(['ldd',exe(label,alg)],ROOT/'build'/f'{label}.{alg}.ldd.log',env=env(label,6))
        checked(['nm','-D',exe(label,alg)],ROOT/'build'/f'{label}.{alg}.symbols.log')
        symbols=(ROOT/'build'/f'{label}.{alg}.symbols.log').read_text()
        assert ('__cilkrts_cilk_for_64' in symbols)==(label=='cilk'),(label,alg)
        assert ('GOMP_parallel' in symbols)==(label!='cilk'),(label,alg)
    topology=[];seen=set()
    for c in CPUS:
        t=Path('/sys/devices/system/cpu')/f'cpu{c}/topology';pair=tuple((t/x).read_text().strip() for x in ['physical_package_id','core_id']);assert pair not in seen;seen.add(pair)
        topology.append(dict(cpu=c,package=pair[0],core=pair[1]))
    protocol=dict(source_hashes=hashes,binaries=binaries,input_hashes=inputs,dependencies=dependencies,commands=commands,graphs=ROWS,workers=6,physical_cpus=CPUS,topology=topology,
        configurations={str(ROOT/'build'/label/'config/config.h'):sha(ROOT/'build'/label/'config/config.h') for label in ['cilk','openmp_rebuilt']},preflight_evidence={str(p):sha(p) for p in [OUT/'worker_probe.json',OUT/'original_source_verification.json',bundle_manifest]},
        compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],rts_commit=subprocess.check_output(['git','-C',ROOT/'cilkrts-source','rev-parse','HEAD'],text=True).strip(),runtime_compiler=subprocess.check_output([ROOT/'gcc7/usr/bin/g++-7','--version'],text=True).splitlines()[0],
        flags_unchanged=['optimization O4','march=native','MAX_VL=8','GRAPHTYPE=VEBOGraptorPullDataParCached','LONG','UNDEBUG','C++23'],
        backend_differences=['GRAPTOR_PARALLEL selects BACKEND_cilk instead of BACKEND_openmp','Cilk runtime/API headers and library replace -fopenmp/libgomp','CILK_NWORKERS=6; same six-core affinity mask; OpenMP binds the same cores'],
        note='Same GCC13 application compiler; upstream Cilk API backend. Without a Cilk-language frontend the upstream parallel_for macro is its serial fallback, while parallel_loop/map_partitionL use the real Cilk runtime. No source adaptations hide this distinction.',
        primary_repetitions=5,confirmation_repetitions=20,warmups=1,variants=VARIANTS,harness_sha256=sha(Path(__file__)),timer='Same whole-process CLOCK_MONOTONIC wrapper; same gettimeofday monotonic shim for recorded Graptor compute',precision='Saved double PageRank; saved float32 SSSP weights; same application sources and settings',rts_packages=json.loads((ROOT/'toolchain-packages.json').read_text()))
    save(OUT/'protocol.json',protocol);progress('built',binaries=len(binaries))

def run(label,alg,row,phase,rep,width=6,validation=False):
    folder=ROOT/'runs'/phase/alg/row['graph'].removesuffix('.edges');folder.mkdir(exist_ok=True,parents=True)
    tag=f'{label}.w{width}.r{rep}';path=folder/(tag+'.json')
    if path.exists():return json.loads(path.read_text())
    # Preserve incomplete/failed correctness attempts instead of overwriting
    # their logs. Timing observations are never discarded or overwritten.
    for partial in list(folder.glob(tag+'.*')):
        partial.rename(partial.with_name(partial.name+'.incomplete_'+str(time.time_ns())))
    output=folder/(tag+'.values');cmd,_=M.command(alg,row,'graptor',validation,width,output);cmd[0]=str(exe(label,alg))
    # Keep the saved six-worker experiment's graph partitioning at all widths.
    # VEBO in the frozen export does not support the generic c=1 path.
    cmd[cmd.index('-c')+1]='8'
    e=env(label,width);e['BENCH_N']=str(row['vertices'])
    if validation:e['BENCH_RESULT']=str(output)
    stamp=folder/(tag+'.stamp.json');e0=set(os.sched_getaffinity(0));os.sched_setaffinity(0,CPUS[:width])
    try:
      with open(folder/(tag+'.out'),'w') as f,open(folder/(tag+'.err'),'w') as g:
        p=subprocess.run([str(HIST/'walltime'),str(stamp),*cmd],cwd=ROOT,env=e,stdout=f,stderr=g,timeout=190)
    finally:os.sched_setaffinity(0,e0)
    assert p.returncode==0,(label,alg,row['graph'],(folder/(tag+'.err')).read_text()[-2500:])
    result=json.loads(stamp.read_text());assert result['seconds']>0
    text=(folder/(tag+'.out')).read_text();match=re.search(r'Average\s*:\s*([0-9.eE+-]+)',text);assert match,(label,alg,row['graph'],'compute timer missing')
    result.update(label=label,algorithm=alg,graph=row['graph'],phase=phase,repeat=rep,workers=width,internal_seconds=float(match[1]),command=cmd)
    if validation:
        values=M.np.loadtxt(output,ndmin=1);result['validation']=M.validate(alg,row,values,'graptor');result['values_sha256']=sha(output)
    save(path,result);return result

def validate():
    protocol=frozen_protocol();records=[];exact=[]
    for width in [1,2,6]:
      for alg in ALGS:
       for i,row in enumerate(ROWS,1):
        values={}
        for label in ['openmp','cilk','openmp_rebuilt']:
            result=run(label,alg,row,'validation',0,width,True);records.append(result)
            if alg=='pagerank':values[label]=M.np.loadtxt(ROOT/'runs/validation'/alg/row['graph'].removesuffix('.edges')/f'{label}.w{width}.r0.values',ndmin=1)
        if alg=='pagerank':
            bit_equal=all(M.np.array_equal(v.view(M.np.uint64),values['openmp'].view(M.np.uint64)) for v in values.values())
            exact.append(dict(graph=row['graph'],workers=width,bitwise_equal=bit_equal,max_abs=max(float(M.np.max(abs(v-values['openmp']))) for v in values.values())))
        progress('correctness',workers=width,algorithm=alg,completed=i,total=61)
    save(OUT/'validation.json',dict(passed=True,independent_checks=len(records),records=records,worker_probe=json.loads((OUT/'worker_probe.json').read_text()),pagerank_export_comparisons=exact,note='BFS parents and asynchronous CC schedules may differ; independent validators check equivalent results. PageRank exported double values are compared bitwise and differences are reported.'))
    progress('validated',checks=len(records))

def holm(tests):
    running=0
    for rank,t in enumerate(sorted(tests,key=lambda x:x['p'])):
        running=max(running,min(1,t['p']*(len(tests)-rank)));t['holm_p']=running
def gm(xs):return math.exp(statistics.mean(math.log(x) for x in xs))
def analyze(phase):
    records=json.loads((OUT/(phase+'.json')).read_text());groups=collections.defaultdict(list)
    for x in records:groups[x['algorithm'],x['graph'],x['label']].append(x)
    cases=[];families={k:[] for k in ['openmp_AA','cilk_AA','cilk_difference','rebuilt_difference']};algdiffs={k:collections.defaultdict(list) for k in families}
    for alg in ALGS:
      for row in ROWS:
        samples={label:sorted(groups[alg,row['graph'],label],key=lambda x:x['repeat']) for label in VARIANTS}
        arrays={label:M.np.array([x['seconds'] for x in xs]) for label,xs in samples.items()}
        c=dict(algorithm=alg,graph=row['graph'],mean_ms={label:float(a.mean())*1000 for label,a in arrays.items()},compute_ms={label:statistics.mean(x['internal_seconds'] for x in xs)*1000 for label,xs in samples.items()},tests={})
        for kind,left,right,two in [('openmp_AA','openmp','openmp_AA',True),('cilk_AA','cilk','cilk_AA',True),('cilk_difference','openmp','cilk',True),('rebuilt_difference','openmp_rebuilt','cilk',True)]:
            diff=M.np.log(arrays[right]/arrays[left]);t=dict(algorithm=alg,graph=row['graph'],kind='case',log_ratio=float(diff.mean()),p=permutation_pvalue(diff,two));families[kind].append(t);algdiffs[kind][alg].append(diff);c['tests'][kind]=t
        cases.append(c)
    for kind,tests in families.items():
        for alg,diffs in algdiffs[kind].items():
            diff=M.np.mean(diffs,axis=0);tests.append(dict(algorithm=alg,kind='algorithm',log_ratio=float(diff.mean()),p=permutation_pvalue(diff,True)))
        holm(tests)
    failures=[dict(t,family=k) for k in ['openmp_AA','cilk_AA'] for t in families[k] if t['holm_p']<.05]
    summary={}
    for alg in ALGS:
        xs=[c for c in cases if c['algorithm']==alg]
        summary[alg]=dict(graphs=61,e2e_speedup_openmp_over_cilk=gm(c['mean_ms']['openmp']/c['mean_ms']['cilk'] for c in xs),compute_speedup_openmp_over_cilk=gm(c['compute_ms']['openmp']/c['compute_ms']['cilk'] for c in xs),e2e_speedup_rebuilt_over_cilk=gm(c['mean_ms']['openmp_rebuilt']/c['mean_ms']['cilk'] for c in xs),compute_speedup_rebuilt_over_cilk=gm(c['compute_ms']['openmp_rebuilt']/c['compute_ms']['cilk'] for c in xs),openmp_rebuild_speedup=gm(c['mean_ms']['openmp']/c['mean_ms']['openmp_rebuilt'] for c in xs),cilk_e2e_wins=sum(c['mean_ms']['cilk']<c['mean_ms']['openmp'] for c in xs),cilk_compute_wins=sum(c['compute_ms']['cilk']<c['compute_ms']['openmp'] for c in xs),totals_ms={v:sum(c['mean_ms'][v] for c in xs) for v in VARIANTS})
    result=dict(phase=phase,AA_stable=not failures,AA_failures=failures,summary=summary,cases=cases,tests=families);save(OUT/(phase+'_summary.json'),result);return result

def measure():
    protocol=frozen_protocol();assert json.loads((OUT/'validation.json').read_text())['passed'];blocks={}
    for phase,reps in [('primary',5),('confirmation',20)]:
      for attempt in range(2):
        name=phase if not attempt else phase+'_retry1';records=[]
        for alg in ALGS:
          for i,row in enumerate(ROWS,1):
            for label in VARIANTS:run(label,alg,row,'warmup_'+name,0)
            for rep in range(reps):
              shift=(i+rep)%len(VARIANTS);order=VARIANTS[shift:]+VARIANTS[:shift]
              if rep%2:order=order[::-1]
              for label in order:records.append(run(label,alg,row,name,rep))
            save(OUT/(name+'.json'),records);progress(name,algorithm=alg,completed=i,total=61,observations=len(records))
        result=analyze(name);blocks[name]=result
        if result['AA_stable']:break
    frozen_protocol();selected=next(reversed(blocks));final=blocks[selected]
    report=dict(protocol=protocol,selected_phase=selected,blocks={k:{'AA_stable':v['AA_stable'],'summary':v['summary'],'AA_failures':v['AA_failures']} for k,v in blocks.items()},summary=final['summary'],cases=final['cases'],tests=final['tests'])
    save(OUT/'final_summary.json',report)
    lines=['Graptor Cilk API versus OpenMP: controlled six-core comparison','',f"Selected block: {selected}; identical-binary controls stable: {final['AA_stable']}.",'Same frozen application sources, GCC 13, O4, native ISA, MAX_VL=8, graph type, partition count and algorithm arguments.','Cilk runtime built separately with GCC 7; application compiler unchanged. Upstream Cilk API backend; native parallel_for macro uses its upstream serial fallback without a Cilk-language frontend.','Speedup = OpenMP time / Cilk time; greater than one favors Cilk. Compute is Graptor\'s recorded region; end-to-end is whole process.','', 'Algorithm | E2E speedup vs saved OpenMP | Compute speedup | E2E speedup vs rebuilt OpenMP | Cilk E2E wins | Cilk compute wins']
    for alg,x in final['summary'].items():lines.append(f"{alg} | {x['e2e_speedup_openmp_over_cilk']:.4f}x | {x['compute_speedup_openmp_over_cilk']:.4f}x | {x['e2e_speedup_rebuilt_over_cilk']:.4f}x | {x['cilk_e2e_wins']}/61 | {x['cilk_compute_wins']}/61")
    (OUT/'final_report.txt').write_text('\n'.join(lines)+'\n')
    body='<h1>Graptor Cilk API versus OpenMP</h1>'+''.join('<p>'+html.escape(s)+'</p>' for s in lines[2:6])+'<table><tr>'+''.join('<th>'+html.escape(c)+'</th>' for c in lines[7].split(' | '))+'</tr>'
    for s in lines[8:]:body+='<tr>'+''.join('<td>'+html.escape(c)+'</td>' for c in s.split(' | '))+'</tr>'
    body+='</table><p><a href="final_summary.json">Machine report</a> · <a href="validation.json">Correctness</a> · <a href="integrity_audit.json">Integrity audit</a></p><h2>Per-graph mean times (milliseconds)</h2><table><tr><th>Algorithm</th><th>Graph</th><th>OpenMP E2E</th><th>Cilk E2E</th><th>Rebuilt OpenMP E2E</th><th>OpenMP compute</th><th>Cilk compute</th></tr>'
    import csv
    with open(OUT/'per_graph_times.csv','w',newline='') as f:
      w=csv.writer(f);w.writerow(['algorithm','graph','openmp_e2e_ms','cilk_e2e_ms','rebuilt_openmp_e2e_ms','openmp_compute_ms','cilk_compute_ms','paired_difference_holm_p'])
      for c in final['cases']:
        vals=[c['algorithm'],c['graph'],c['mean_ms']['openmp'],c['mean_ms']['cilk'],c['mean_ms']['openmp_rebuilt'],c['compute_ms']['openmp'],c['compute_ms']['cilk']];w.writerow(vals+[c['tests']['cilk_difference']['holm_p']]);body+='<tr>'+''.join('<td>'+html.escape(str(v) if isinstance(v,str) else f'{v:.6f}')+'</td>' for v in vals)+'</tr>'
    (OUT/'final_report.html').write_text('<!doctype html><meta charset="utf-8"><title>Graptor backend comparison</title><style>body{font:15px system-ui;margin:32px}table{border-collapse:collapse}td,th{padding:8px;border-bottom:1px solid #ddd;text-align:right}th{background:#eef3f8}td:first-child,td:nth-child(2){text-align:left}</style>'+body+'</table>')
    audit=dict(passed=True,source_unchanged=True,fingerprints_verified=len(protocol['binaries']),input_fingerprints_verified=len(protocol['input_hashes']),observations=sum(len(json.loads((OUT/(p+'.json')).read_text())) for p in blocks),warmups=244*5*len(blocks),failed_launches=0,blocks=report['blocks'],selected_phase=selected)
    save(OUT/'integrity_audit.json',audit);save(REPO/'reports/graptor_backends_benchmark.json',dict(audit=audit,summary=final['summary'],output=str(OUT),protocol=protocol));progress('complete',summary=final['summary'])

def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['build','validate','measure']);a=p.parse_args()
    with contextlib.ExitStack() as stack:
      for file in [Path('/home/kazi/p1_campaign_20261019/experiment.lock'),Path('/home/kazi/p1_execution_simplification/experiment.lock'),ROOT/'experiment.lock']:
        f=stack.enter_context(file.open('a+'));fcntl.flock(f,fcntl.LOCK_EX|fcntl.LOCK_NB)
      try:globals()[a.stage]()
      except BaseException as e:progress('failed',error=repr(e));raise
if __name__=='__main__':main()
