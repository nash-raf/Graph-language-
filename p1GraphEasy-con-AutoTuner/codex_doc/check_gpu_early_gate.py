#!/usr/bin/env python3
"""Durably reproduce the >64 descriptor gate on checkpoint and final runtime."""
from pathlib import Path
import os,subprocess,json,hashlib
root=Path('/home/kazi/p1_layout_native_20261105')
out=root/'versions/native07/known_failures';out.mkdir(exist_ok=True)
edges=out/'large.edges';edges.write_text(''.join(f'{u} {(u+1)%129}\n{u} {(u+1)%129}\n{u} {u}\n' for u in range(129)))
objects=['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime',
         'roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']
answers=[];results=[]
for version in ['baseline','native07']:
    v=root/'versions'/version;d=out/('gpu_gate_'+version);d.mkdir(exist_ok=True)
    for p in (v/'src').glob('*.ll'):
        if not (d/p.name).exists():(d/p.name).symlink_to(p)
    source=(v/'src/test/spatial_destination.graph').read_text().replace('test/spatial_pairs.txt',str(edges))
    (d/'large.graph').write_text(source)
    env=dict(os.environ,FORCE_GPU='1',FORCE_CPU='0',SGPL_GPU_MIN_TRIPS='0',
             SGPL_CLEANCUT_PARTITIONS='65',SGPL_NUM_THREADS='1',SGPL_GPU_DEBUG='1')
    for i,cmd in enumerate([[v/'GraphProgram','--ir-backend=gpu',d/'large.graph'],
         ['g++','-fopenmp','-no-pie',d/'program.o',*[v/(n+'.o') for n in objects],'-pthread','-ldl','-lnlopt','-o',d/'check']]):
        with (d/f'build{i}.log').open('w') as log:
            subprocess.run(list(map(str,cmd)),cwd=d,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
    for batch in ['0','1']:
        env['SGPL_GPU_BATCH']=batch
        run=subprocess.run([str(d/'check')],cwd=d,env=env,capture_output=True,text=True,check=True,timeout=60)
        (out/f'{version}_gpu65_batch{batch}.log').write_text(run.stderr)
        (out/f'{version}_gpu65_batch{batch}.out').write_text(run.stdout)
        assert '[gpu-capability] existing early gate work=-1; CPU before preparation' in run.stderr
        assert '[gpu-task]' not in run.stderr
        answers.append(run.stdout)
        results.append(dict(version=version,batch=batch,passed=True,source_sha256=hashlib.sha256(source.encode()).hexdigest()))
assert all(x==answers[0] for x in answers)
(out/'gpu65_gate.json').write_text(json.dumps(results,indent=2))
print('PASS identical pre-effect capability rejection and complete outputs, checkpoint/final, batch off/on')
