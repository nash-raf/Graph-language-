#!/usr/bin/env python3
"""Freeze the tested P1 runtime and construct fair four-framework binaries."""
import argparse,fcntl,hashlib,json,os,shutil,subprocess,time
from pathlib import Path

ALGS=['bfs','sssp','cc','pagerank']
OBJECTS=['runtime.o','parallel_runtime.o','gpu_runtime.o','autotuner_runtime.o','graph_mutation_runtime.o','roaring_bitmap.o','graph_loader_runtime.o','graph_runtime.o','semiring_runtime.o']
def sha(p):
    h=hashlib.sha256()
    with open(p,'rb') as f:
        for x in iter(lambda:f.read(1024*1024),b''):h.update(x)
    return h.hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--base',type=Path,default=Path('/media/readinggroup/Storage/splash'))
    ap.add_argument('--root',type=Path);ap.add_argument('--expected-runtime',type=Path)
    a=ap.parse_args();S=a.base.resolve();B=S/'bench';C=B/'p1graph_audit/p1GraphEasy-con-AutoTuner';R=(a.root or B/'e2e150').resolve()
    R.mkdir(parents=True,exist_ok=True)
    lock=open(R/'.bundle.lock','a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
    assert (S/'original_graphs_150/manifest.json').is_file(),'Prepare the 150 inputs first'
    if (R/'bundle.json').exists():
        old=json.loads((R/'bundle.json').read_text())
        assert all(sha(R/name)==digest for name,digest in old['files'].items()),'Frozen bundle was modified'
        print('BUNDLE_READY',R);return
    for folder in ['bin','logs','p1','algorithms','lib','sources/graptor','sources/common','sources/overrides']:(R/folder).mkdir(parents=True,exist_ok=True)
    if a.expected_runtime:
        expected=json.loads(a.expected_runtime.read_text())['sha256']
        for name in ['parallel_runtime.c','autotuner_runtime.c','autotuner_runtime.h','array_frontier_runtime.inc','array_preparation_runtime.inc']:
            assert sha(C/name)==expected[name],('Lab runtime differs from latest validated version',name)
        shutil.copy2(a.expected_runtime,R/'latest_runtime_expected.json')
    # No original benchmark binaries or compiler working files are replaced.
    for f in C.iterdir():
        if f.is_file() and f.suffix in ['.h','.inc','.ll','.c','.cpp']:shutil.copy2(f,R/'p1'/f.name)
    shutil.copy2(C/'GraphProgram',R/'p1/GraphProgram')
    shutil.copy2(B/'p1work/monotonic_timer.o',R/'p1/monotonic_timer.o')
    shutil.copy2(B/'p1_bench.py',R/'lib/p1_bench_snapshot.py')
    for name in ['bfs_parent','sssp','cc','pagerank_ordinary']:shutil.copy2(S/'graph-language/algo'/(name+'.graph'),R/'algorithms'/(name+'.graph'))
    commands=[]
    def build(cmd,label):
        started=time.monotonic()
        with open(R/'logs'/(label+'.build.log'),'w') as f:subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,check=True)
        commands.append({'name':label,'command':cmd,'seconds':time.monotonic()-started})
        print('BUILT',label,flush=True)
    for obj in OBJECTS:
        name=Path(obj).stem;cpp=name in ['roaring_bitmap','graph_loader_runtime','graph_runtime'];source=R/'p1'/(name+('.cpp' if cpp else '.c'))
        cmd=['g++' if cpp else 'gcc','-O2' if name in ['graph_loader_runtime','graph_runtime'] else '-O3','-pthread']
        if cpp:cmd+=['-std=c++17']
        if name in ['semiring_runtime','roaring_bitmap','graph_loader_runtime']:cmd+=['-fopenmp']
        if name=='roaring_bitmap':cmd+=['-mavx2','-march=native']
        build(cmd+['-c',str(source),'-o',str(R/'p1'/obj)],'p1_'+name)
    shutil.copytree(B/'sources/graptor/e2e',R/'sources/graptor',dirs_exist_ok=True)
    shutil.copytree(B/'sources/common',R/'sources/common',dirs_exist_ok=True)
    shutil.copytree(B/'overrides',R/'sources/overrides',dirs_exist_ok=True)
    driver=(R/'sources/graptor/driver.C').read_text()
    assert '#ifndef BENCH_SKIP_VALIDATE' in driver and 'omp_set_num_threads(1)' not in driver
    apps={'bfs':'BFS_parent_sparse.C','sssp':'BFv.C','cc':'CCv.C','pagerank':'PR_sparse.C'}
    G=S/'graptor';lib=G/'build-bench/external/Build/libconfig_project'
    # The installed library and existing working SSSP binary use the cached
    # graph layout. Its obsolete noncached layout no longer compiles upstream.
    obsolete=R/'sources/overrides/graptor/dsl/emap/GraptorCSCDataParNotCached.h'
    if obsolete.exists():obsolete.unlink()
    for alg in ALGS:
        cmd=['g++','-O3','-std=c++23','-fopenmp','-DLONG','-march=native','-Wno-ignored-attributes','-DBENCH_SKIP_VALIDATE=1',
             '-DGRAPHTYPE=VEBOGraptorPullDataParCached']
        cmd+=['-I'+str(R/'sources/overrides')]
        if alg=='bfs':cmd+=['-DDEFERRED_UPDATE=1','-DLEVEL_ASYNC=0']
        if alg=='pagerank':cmd+=['-DMEMO=1']
        cmd+=['-I'+str(G/'build-bench'),'-I'+str(G/'include'),'-I'+str(lib/'include'),'-I'+str(R/'sources/common'),'-I'+str(R/'sources/graptor'),'-I'+str(G/'bench'),
              str(R/'sources/graptor'/apps[alg]),str(G/'build-bench/libgraptorlib.a'),'-L'+str(lib/'lib'),'-Wl,-rpath,'+str(lib/'lib'),'-lconfig++','-ldl','-o',str(R/'bin'/('graptor_'+alg))]
        build(cmd,'graptor_'+alg)
        for fw in ['graphit','ligra']:shutil.copy2(B/'bin'/(fw+'_'+alg),R/'bin'/(fw+'_'+alg))
    shutil.copy2(B/'bin/walltime2',R/'bin/walltime2')
    shutil.copy2(B/'walltime2.c',R/'sources/walltime2.c')
    files={}
    for root in ['bin','p1','algorithms','lib','sources']:
        for p in (R/root).rglob('*'):
            if p.is_file():files[str(p.relative_to(R))]=sha(p)
    (R/'build_commands.json').write_text(json.dumps(commands,indent=2)+'\n')
    record={'p1_origin':str(C),'input_manifest':str(S/'original_graphs_150/graphs150.csv'),
            'p1_backend':'cpu','graptor_native_validation':False,'graptor_preparation_threads':'OpenMP configured width',
            'files':files,'built_unix':time.time()}
    (R/'bundle.json').write_text(json.dumps(record,indent=2)+'\n')
    print('BUNDLE_READY',R,flush=True)

if __name__=='__main__':main()
