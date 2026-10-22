"""Frontend certificates and exact enabled/disabled fixtures on authorized hosts."""
from pathlib import Path
import os,subprocess,tempfile
root=Path.cwd();version=root.parent
assert ((Path('/kaggle/working').is_dir() and Path('/kaggle/input').is_dir()) or
        os.environ.get('SGPL_ALLOW_LOCAL_CONTRACT_TESTS')=='1'), 'Explicit local contract opt-in required outside Kaggle'
widths=sorted(set(map(int,os.environ['P1_TEST_WIDTHS'].split())))
env=dict(os.environ,SGPL_ALLOW_HT='0',FORCE_CPU='1',SGPL_FUSION_STATS='1',SGPL_WITNESS_DUMP='1')
def run(command,cwd,e):
    p=subprocess.run(list(map(str,command)),cwd=cwd,env=e,capture_output=True,text=True,timeout=180)
    assert not p.returncode,(command,p.returncode,p.stdout[-2000:],p.stderr[-4000:]);return p
objects=[version/(x+'.o') for x in ['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime','roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']]
evidence=Path(os.environ['P1_CAMPAIGN_OUTPUT'])/'execution_fusion_frontend';evidence.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory() as temporary:
    work=Path(temporary)
    for f in root.glob('*.ll'):(work/f.name).symlink_to(f)
    fixtures={name:(root/'test'/(name+'.graph')).read_text() for name in
        ['array_frontier_order','array_frontier_weights','array_program_snapshot','zero_program_array']}
    fixtures['ordinal_observer']=fixtures['array_frontier_order'].replace('hits[v]=hits[v]+1;','hits[v]=hits[v]+p+1;')
    fixtures['effect_observer']=fixtures['array_frontier_order'].replace('hits[v]=hits[v]+1;','print next_size; hits[v]=hits[v]+1;')
    for name,source in fixtures.items():
        for f in (root/'test').glob('*.txt'):source=source.replace('"test/'+f.name+'"','"'+str(f)+'"')
        program=work/(name+'.graph');program.write_text(source);outputs=[]
        for mode in ['sequential','generic','enabled']:
            e=dict(env)
            if mode=='sequential':e['GRAPH_DISABLE_PDG']='1'
            if mode=='generic':e['SGPL_NO_EFFECT_FUSION_LOWERING']='1'
            ir=work/'fixture.ll';compiled=run([version/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(ir),program],work,e)
            (evidence/(name+'_'+mode+'.log')).write_text(compiled.stderr)
            (evidence/(name+'_'+mode+'.ll')).write_text(ir.read_text())
            text=compiled.stderr
            active_b='[frontier-fusion] activate' in text;active_c='[append-journal] activate' in text
            if mode!='enabled' or name in ['ordinal_observer','effect_observer']:
                assert not active_b and not active_c,(name,mode,text[-4000:])
            if name in ['array_frontier_order','array_program_snapshot'] and mode=='enabled':
                if (root/'frontier_fusion_runtime.inc').exists():assert active_b,(name,text[-4000:])
                if (root/'append_journal_runtime.inc').exists():assert active_c,(name,text[-4000:])
            exe=work/'check';run(['g++','-O3','-fopenmp','-no-pie',work/'program.o',*objects,'-pthread','-ldl','-lnlopt','-o',exe],work,e)
            for width in widths:
                for disabled in ([False,True] if mode=='enabled' else [False]):
                    ee=dict(e,SGPL_NUM_THREADS=str(width),OMP_NUM_THREADS=str(width))
                    if disabled:ee['SGPL_NO_EFFECT_FUSION']='1'
                    outputs.append(run(['python3',root/'test/physical_run.py',str(width),exe],work,ee).stdout)
        assert all(x==outputs[0] for x in outputs),(name,outputs)
        print('PASS execution fusion frontend activation/refusal and exact output',name,flush=True)
