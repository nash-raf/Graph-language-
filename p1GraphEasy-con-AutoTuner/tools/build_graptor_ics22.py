#!/usr/bin/env python3
"""Build untouched ICS22 Graptor with the published GCC 7.2 CilkPlus frontend."""
from pathlib import Path
import hashlib, json, os, subprocess, sys, time

ROOT = Path('/home/kazi/p1_graptor_ics22_20261026')
OUT = Path('/mnt/c/p1-graptor-ics22-benchmark-20261026')
SRC = ROOT/'graptor-ics22'
GCC = ROOT/'gcc-7.2-install'
FROZEN = Path('/mnt/c/p1-effect-fusion-20261021/B3safe2-export/export/bundle/graptor_source')
DEP = Path('/home/kazi/p1_effect_fusion_local_20261022/dependency')

def digest(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(1024*1024),b''): h.update(b)
    return h.hexdigest()

def inventory(root):
    return {str(p.relative_to(root)):digest(p) for p in sorted(root.rglob('*')) if p.is_file()}

def save(p,d): p.write_text(json.dumps(d,indent=2))

def main():
    compiler=GCC/'bin/g++'
    assert compiler.exists()
    env=os.environ.copy()
    env['LD_LIBRARY_PATH']=str(GCC/'lib64')+':'+str(DEP/'lib')
    manifest=ROOT/'graptor-source-before.json'
    if not manifest.exists(): save(manifest,inventory(SRC))
    before=json.loads(manifest.read_text())
    assert inventory(SRC)==before
    logs=ROOT/'build'; logs.mkdir(exist_ok=True)
    previous=json.loads((OUT/'build.json').read_text()) if (OUT/'build.json').exists() else {}
    commands=previous.get('commands',{}); results=previous.get('results',{})
    def run(key,cmd,allow_failure=False):
        commands[key]=list(map(str,cmd))
        old=logs/(key+'.log')
        if old.exists():old.rename(old.with_name(old.name+'.previous_'+str(time.time_ns())))
        with (logs/(key+'.log')).open('w') as f:
            p=subprocess.run(commands[key],env=env,stdout=f,stderr=subprocess.STDOUT)
        results[key]=p.returncode
        save(OUT/'build-status.json',dict(commands=commands,results=results,current=key))
        print(key,p.returncode,flush=True)
        if p.returncode and not allow_failure:
            print((logs/(key+'.log')).read_text()[-5000:],flush=True)
            raise RuntimeError(key)
    base=['-g','-Wno-ignored-attributes','-ftemplate-backtrace-limit=0','-O4','-DLONG','-std=c++17','-march=native','-DMAX_VL=8']
    # The C++23 configuration is unsupported by the historical compiler.
    run('modern-cxx23',[compiler,'-std=c++23','-x','c++','-fsyntax-only','/dev/null'],True)
    # Try the modern benchmark unmodified under the published language standard.
    run('modern-cxx17',[compiler,*base,'-fmax-errors=8','-fopenmp','-I'+str(FROZEN),'-I'+str(FROZEN/'include'),'-I'+str(FROZEN/'bench'),'-I'+str(DEP/'include'),'-fsyntax-only',FROZEN/'bench/PRdouble.C'],True)
    template=(SRC/'include/graptor/config.h.in').read_text()
    # These are the base reproduction-script settings; AVX-512 is replaced only
    # by the host ISA target because the local CPU has AVX2, not AVX-512.
    defines=dict(HUGE_PAGES=1,VMAP_TIMING=1,PAPI_CACHE=0,WIDEN_APPROACH=0,GRAPTOR=2,
        GRAPTOR_CACHED=1,GRAPTOR_CSC=1,GRAPTOR_EXTRACT_OPT=1,ANALYSE_VALUES=0,
        VEBO_FORCE=1,GRAPTOR_DEGREE_BITS=8,GRAPTOR_USE_MMX=0,MEMORY_CLOBBER_WORKAROUND=0,
        SWITCH_DYNAMIC=0,SCALE_CONTRIB=1,NUM_BUCKETS=127,DENSE_COPY=1,DEFERRED_UDPATE=1,SP_THRESHOLD=-1)
    flags=[*base,*['-D'+k+'='+str(v) for k,v in defines.items()],'-DGRAPHTYPE=VEBOGraptorT']
    targets={'pagerank':('PR_vary.C',['-DVARIANT=11']),
             'bfs':('BFSv.C',['-DUNCOND_EXEC=0']),'sssp':('BFv.C',[]),'cc':('CCv.C',[])}
    binaries=previous.get('binaries',{})
    for backend in (sys.argv[1:] or ['cilk','openmp']):
        assert backend in ['cilk','openmp']
        d=logs/backend; d.mkdir(exist_ok=True)
        config=template.replace('@Graptor_VERSION_MAJOR@','0').replace('@Graptor_VERSION_MINOR@','1').replace('#cmakedefine GRAPTOR_PARALLEL BACKEND_@GRAPTOR_PARALLEL@','#define GRAPTOR_PARALLEL BACKEND_'+backend)
        (d/'config.h').write_text(config)
        incl=['-I'+str(d),'-I'+str(SRC/'include'),'-I'+str(SRC/'bench')]
        mode=['-fcilkplus'] if backend=='cilk' else ['-fopenmp']
        const=d/'constants.o'
        run(backend+'-constants',[compiler,*flags,*incl,*mode,'-c',SRC/'src/constants.C','-o',const])
        for alg,(file,extra) in targets.items():
            if os.environ.get('ICS22_ALGS') and alg not in os.environ['ICS22_ALGS'].split(','):continue
            exe=d/alg
            libs=([SRC/'reproduce/lib/libcilkrts.so'] if backend=='cilk' else [])+['-l:libnuma.so.1','-ldl','-pthread']
            run(backend+'-'+alg,[compiler,*flags,*incl,*mode,*extra,SRC/'bench'/file,const,*libs,'-o',exe],True)
            if results[backend+'-'+alg]==0: binaries[backend+':'+alg]=dict(path=str(exe),sha256=digest(exe))
    assert inventory(SRC)==before, 'Graptor source changed'
    report=dict(compiler=subprocess.check_output([compiler,'--version'],env=env,text=True).splitlines()[0],
        compiler_path=str(compiler),compiler_sha256=digest(compiler),source_commit='7d3f561978574fbb6e989a905473e408d278812b',
        source_hashes=before,commands=commands,results=results,binaries=binaries,
        bundled_runtime_sha256=digest(SRC/'reproduce/lib/libcilkrts.so'),source_unchanged=True,
        cpu_target='-march=native (AVX2 Ryzen 5500; published target is skylake-avx512)',
        note='No GCC or Graptor source patches. Both historical backends use GCC 7.2 and the same application flags. Modern-source compatibility failures are retained.')
    save(OUT/'build.json',report)

if __name__=='__main__': main()
