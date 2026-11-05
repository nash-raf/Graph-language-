#!/usr/bin/env python3
"""Correctness only: build four GPU realization changes and validate outputs.

No timed executable, warmup, wall-time wrapper, profiler or performance report.
The validation fixture's unused timer is replaced with a constant.
"""
from pathlib import Path
import argparse,hashlib,importlib.util,json,os,re,shutil,subprocess
REPO=Path(__file__).resolve().parents[1]
BASE=Path('/home/kazi/p1_gpu_batch_20261104/versions/candidate02')
OBJECTS=['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime','roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']
FLAGS=['SGPL_GPU_REUSE_RECORDS','SGPL_GPU_EARLY_REJECT','SGPL_GPU_TRANSFER_REDUCE','SGPL_GPU_REUSE_CAPACITY']
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save(p,x):
    p.parent.mkdir(parents=True,exist_ok=True);tmp=p.with_suffix(p.suffix+'.tmp');tmp.write_text(json.dumps(x,indent=2));tmp.replace(p)
def checked(cmd,cwd,tag,env=None):
    with (cwd/(tag+'.out')).open('w') as out,(cwd/(tag+'.err')).open('w') as err:
        result=subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,stdout=out,stderr=err,timeout=240)
    assert result.returncode==0,(str(cwd/tag),(cwd/(tag+'.err')).read_text()[-2500:])
    return (cwd/(tag+'.out')).read_text()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--version',default='correct01');parser.add_argument('--build-only',action='store_true');parser.add_argument('--validate-only',action='store_true');parser.add_argument('--optimization',choices=['O2','O3'],default='O2')
    args=parser.parse_args();assert re.fullmatch(r'[A-Za-z0-9_-]+',args.version)
    root=Path('/home/kazi/p1_gpu_four_correctness_20261104')/args.version
    out=REPO/'bench_results/gpu_four_correctness_20261104'/args.version
    native=root/'native';source=native/'src';out.mkdir(parents=True,exist_ok=True)
    files=[p for p in REPO.iterdir() if p.is_file() and p.suffix in {'.c','.cpp','.h','.inc','.ll'}]
    fingerprint={p.name:sha(p) for p in files}
    if source.exists():assert json.loads((native/'sources.json').read_text())==fingerprint,'Changed native source: use a fresh --version'
    else:
        source.mkdir(parents=True)
        for p in files:shutil.copy2(p,source/p.name)
        save(native/'sources.json',fingerprint)
    for name in OBJECTS:
        target=native/(name+'.o')
        if target.exists():continue
        if name in ['gpu_runtime','autotuner_runtime']:
            checked(['gcc','-'+args.optimization,'-g','-fopenmp','-pthread','-iquote','.', '-c',name+'.c','-o',target],source,name+'.build')
        else:shutil.copy2(BASE/(name+'.o'),target)
    compiler=native/'GraphProgram'
    if not compiler.exists():compiler.symlink_to(BASE/'GraphProgram')
    timer=source/'validation_timer.c';timer.write_text('double __wrap_timer_runtime(void) { return 0.0; }\n')
    checked(['gcc','-c',timer,'-o',native/'validation_timer.o'],source,'validation_timer.build')
    build_flags=['-'+args.optimization,'-g','-fopenmp','-pthread']
    if (out/'build.json').exists():
        previous=json.loads((out/'build.json').read_text());assert previous.get('native_flags',['-O2','-g','-fopenmp','-pthread'])==build_flags,'Changed build flags: use a fresh --version'
    identity=dict(correctness_only=True,benchmarks_run=False,profiling_run=False,native_flags=build_flags,source_sha256=fingerprint,
                  compiler_sha256=sha(compiler),objects_sha256={n:sha(native/(n+'.o')) for n in OBJECTS})
    save(out/'build.json',identity)
    if args.build_only:print('CORRECTNESS BUILD',native,flush=True);return
    spec=importlib.util.spec_from_file_location('common','/home/kazi/p1_c3_integration_20261022/support/common.py')
    common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
    import numpy as np
    records=json.loads((out/'validation.json').read_text()) if (out/'validation.json').exists() else []
    done={(x['algorithm'],x['graph'],x['controls']) for x in records}
    for row in common.ROWS:
      for algorithm in ['bfs','sssp','cc','pagerank']:
        d=root/'cases'/algorithm/row['graph'].removesuffix('.edges');d.mkdir(parents=True,exist_ok=True)
        binary=d/'validate'
        if not binary.exists():
            obj=Path('/home/kazi/p1_gpu_batch_20261104/benchmark/cases')/algorithm/row['graph'].removesuffix('.edges')/'validate.o'
            checked(['g++','-fopenmp','-no-pie',obj,*[native/(n+'.o') for n in OBJECTS],native/'validation_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',binary],d,'link')
        for controls in ['enabled','disabled']:
            if (algorithm,row['graph'],controls) in done:continue
            env=common.env();env.update(FORCE_CPU='0',FORCE_GPU='1',SGPL_GPU_BACKEND='1',SGPL_GPU_DEBUG='1',SGPL_GPU_PREPARATION_STATS='1')
            env.update({name:('1' if controls=='enabled' else '0') for name in FLAGS})
            output=checked([binary],d,controls,env)
            values=np.array(output.split('VALUES_BEGIN\n')[1].split('VALUES_END')[0].split(),dtype=float)
            validation=common.validate(algorithm,row,values,'p1');trace=(d/(controls+'.err')).read_text()
            records.append(dict(algorithm=algorithm,graph=row['graph'],controls=controls,validation=validation,
                gpu_tasks=trace.count('[gpu-task]'),template_hits=trace.count('[gpu-record-reuse] hit'),
                early_refusals=trace.count('existing early gate'),executable_sha256=sha(binary)))
            save(out/'validation.json',records)
        if len(records)%16==0:print('CORRECTNESS',len(records),'of 488 checks',flush=True)
    assert len(records)==488
    save(out/'summary.json',dict(passed=True,correctness_only=True,benchmarks_run=False,profiling_run=False,
      algorithm_graph_cases=244,independent_validation_runs=len(records),controls=FLAGS,
      template_hits=sum(x['template_hits'] for x in records if x['controls']=='enabled'),
      early_refusals=sum(x['early_refusals'] for x in records if x['controls']=='enabled')))
    print('PASS 244 cases, enabled/disabled: correctness only',flush=True)
if __name__=='__main__':main()
