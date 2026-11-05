#!/usr/bin/env python3
"""Compile forced feasible layouts; compare complete stdout with the checkpoint."""
from pathlib import Path
import os,sys,json,subprocess
root=Path('/home/kazi/p1_layout_native_20261105')
candidate=sys.argv[1]
family=sys.argv[2] if len(sys.argv)>2 else 'layouts'
names=['spatial_source','spatial_destination','spatial_snapshot','spatial_source_acc',
       'spatial_d_private','spatial_global_fold','array_frontier_order',
       'spatial_equal_dual','round_spatial_fold']
layouts=['CSR','PCSR','BCSR','SET']
if family=='temporal':
    names=['temporal_independent','temporal_pb_q','temporal_p_bq','temporal_connected',
           'temporal_empty','temporal_source_order','temporal_claim','temporal_rounds',
           'temporal_round_one','temporal_guarded','temporal_round_carried','temporal_round_waw']
    layouts=['AUTO']
if family=='claims':
    names=['spatial_equal_dual','spatial_different_dual','spatial_subset_dual',
           'spatial_claim_repeat','spatial_claim_repeat_other','nested_step','mutual_deg',
           'reduce_plus_write','priv_reduce_write_big','carried_read_state','dual_same_base',
           'shadow_snapshot','reduce_real_ops']
    layouts=['AUTO']
objects=['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime',
         'roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']
out=root/'versions'/candidate/('layout_checks' if family=='layouts' else family+'_differential');out.mkdir(exist_ok=True)
results=[]
for name in names:
    for layout in layouts:
        outputs=[]
        for version in ['baseline',candidate]:
            v=root/'versions'/version;d=out/name/layout/version;d.mkdir(parents=True,exist_ok=True)
            if family=='claims' and not (d.parent/'verify').exists():
                (d.parent/'verify').symlink_to(root/'versions/baseline/verify',target_is_directory=True)
            for ll in [*(v/'src').glob('*.ll'),*(v/'src').glob('*.txt')]:
                if not (d/ll.name).exists():(d/ll.name).symlink_to(ll)
            if not (d/'test').exists():(d/'test').symlink_to(v/'src/test',target_is_directory=True)
            env=dict(os.environ,FORCE_CPU='1',FORCE_GPU='0',SGPL_GPU_BACKEND='0',
                     AUTOTUNER_FORCE_LAYOUT=layout,SGPL_NUM_THREADS='6',SGPL_ALLOW_HT='0',SGPL_LAYOUT_EXEC_DEBUG='1')
            source=v/'src/test'/(name+'.graph')
            if not source.exists():source=v/'verify/cases/parallel'/(name+'.graph')
            commands=[[v/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(d/'program.ll'),source],
                      ['g++','-O3','-fopenmp','-no-pie',d/'program.o',*[v/(n+'.o') for n in objects],'-pthread','-ldl','-lnlopt','-o',d/'check']]
            for i,cmd in enumerate(commands):
                with (d/f'build{i}.log').open('w') as log:
                    subprocess.run(list(map(str,cmd)),cwd=d,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=300)
            for width in [1,2,6]:
                env['SGPL_NUM_THREADS']=str(width)
                with (d/f'w{width}.out').open('w') as stdout,(d/f'w{width}.err').open('w') as stderr:
                    subprocess.run([str(d/'check')],cwd=d,env=env,stdout=stdout,stderr=stderr,check=True,timeout=60)
                outputs.append((d/f'w{width}.out').read_text())
        assert all(o==outputs[0] for o in outputs),(name,layout,'stdout differs')
        results.append(dict(fixture=name,layout=layout,passed=True))
        (out/'results.json').write_text(json.dumps(results,indent=2))
        print('PASS',name,layout,flush=True)
