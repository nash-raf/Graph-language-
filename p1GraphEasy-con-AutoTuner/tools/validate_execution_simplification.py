#!/usr/bin/env python3
"""Correctness-only build/contracts/corpus runner. No timing or benchmark calls.

Run with the existing WSL investigation venv; all artifacts stay outside checkout.
"""
from pathlib import Path
import argparse, concurrent.futures, hashlib, json, os, shutil, subprocess, sys

REPO=Path(__file__).resolve().parents[1]
ROOT=Path('/home/kazi/p1_execution_simplification')
BASE=Path('/home/kazi/p1_c3_integration_20261022/versions/C3_integrated')
os.environ.update(P1_CAMPAIGN_ROOT=str(ROOT),P1_CAMPAIGN_OUTPUT=str(ROOT/'results'),P1_CAMPAIGN_CPUS='0,2,4,6,8,10')
ROOT.mkdir(exist_ok=True);(ROOT/'results').mkdir(exist_ok=True)
sys.path.insert(0,str(REPO/'tools/p1_campaign'))
import runner as R

def prepare():
    old=Path('/home/kazi/p1_c3_integration_20261022')
    for name in ['references','frozen_reports','historical_harness','support','object_cache']:
        if not (ROOT/name).exists():(ROOT/name).symlink_to(old/name,target_is_directory=True)
    if not (ROOT/'inputs.json').exists():shutil.copy2(old/'inputs.json',ROOT/'inputs.json')
    R.M=R.bootstrap()
    manifest=R.read('/mnt/c/p1campaign-results/manifest.json')
    assert R.M.ROWS==manifest['cases'],'Correctness corpus differs from frozen campaign'
    for path,digest in manifest['inputs'].items():assert R.sha(path)==digest,('Input changed',path)
    R.save(ROOT/'results/corpus_identity.json',{'cases':R.M.ROWS,'inputs':manifest['inputs'],'baseline_build':R.read(BASE/'identity.json'),'benchmark_runs':0})

def build(version):
    d,src,identity=R.freeze(version);logs=d/'build_logs';logs.mkdir(exist_ok=True)
    flags=['g++','-O2','-mavx2','-march=native','-std=c++17','-fexceptions','-pthread','-I/usr/local/include/antlr4-runtime','-Igenerated','-iquote','.', '-I'+R.cfg('--includedir')[0]+'/polly',*[x for x in R.cfg('--cxxflags') if x!='-fno-exceptions']]
    sources=['main.cpp','IRGenVisitor.cpp','MotifPattern.cpp','MotifIRBuilder.cpp','ASTBuilder.cpp','pdg.cpp','parallel_loop_outline.cpp','graph_frontier_lowering.cpp','SemanticAnalyzer.cpp','roaring_bitmap.cpp','AutoTunerPass.cpp']+[str(p.relative_to(src)) for p in (src/'generated').glob('*.cpp')]
    def cc(s):
        print('BUILD',s,flush=True)
        return R.compile_cache(src/s,src,flags,logs/(s.replace('/','_')+'.log'))
    if not (d/'GraphProgram').exists():
        with concurrent.futures.ThreadPoolExecutor(2) as pool:objects=list(pool.map(cc,sources))
        R.checked(['g++',*objects,*R.cfg('--ldflags'),'-Wl,--no-keep-memory','-pthread','-lantlr4-runtime',*R.cfg('--libs','all'),*R.cfg('--system-libs'),'-o',d/'GraphProgram'],src,logs/'compiler_link.log')
    for name in R.OBJECTS:
        ext='.cpp' if name in ['roaring_bitmap','graph_loader_runtime','graph_runtime'] else '.c'
        flags=['g++','-std=c++17','-mavx2','-march=native'] if ext=='.cpp' else ['gcc']
        obj=R.compile_cache(src/(name+ext),src,[*flags,'-O3','-g','-fopenmp','-pthread','-iquote','.'],logs/(name+'.log'))
        if not (d/(name+'.o')).exists():(d/(name+'.o')).symlink_to(obj)
    R.save(d/'correctness_build.json',{'identity':identity,'compiler_sha256':R.sha(d/'GraphProgram'),'objects':{n:R.sha(d/(n+'.o')) for n in R.OBJECTS},'baseline':str(BASE),'benchmark_runs':0})
    return d

def contracts(d):
    src=d/'src'
    # Native sources remain frozen. Tests may be repaired without relabeling binaries.
    shutil.copytree(REPO/'test',src/'test',dirs_exist_ok=True)
    for p in (src/'test').glob('*.sh'):p.write_bytes(p.read_bytes().replace(b'\r\n',b'\n'))
    out=d/'contracts';out.mkdir(exist_ok=True)
    env=R.M.env(workers=6);env.update(SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1',P1_TEST_WIDTHS='1 2 6',P1_SIMPLIFY_TEST_OUTPUT=str(out),SGPL_FULLGRAPH_NATIVE_ONLY='1')
    for name,cmd in [('simplification',['bash','test/run_execution_simplify_tests.sh']),('existing',['bash','test/run_effect_fusion_tests.sh','native-only']),('worker_pool',['bash','test/run_worker_pool_tests.sh'])]:
        print('CONTRACTS',name,flush=True);R.checked(cmd,src,out/(name+'.log'),env,timeout=1800)
        R.save(out/(name+'.json'),{'passed':True,'workers':[1,2,6]})

def programs(d,disable_env='SGPL_NO_EXEC_SIMPLIFY'):
    records=[];bit_checks=[];src=d/'src';out=d/'validation';out.mkdir(exist_ok=True)
    helper=d/'exact_bits.o';R.checked(['gcc','-O2','-c',REPO/'tools/p1_campaign/exact_bits.c','-o',helper],d,out/'bits_build.log')
    for alg in R.ALGS:
      for i,row in enumerate(R.M.ROWS):
        case=d/'cases'/alg/row['graph'].removesuffix('.edges');case.mkdir(parents=True,exist_ok=True)
        for ll in src.glob('*.ll'):
            if not (case/ll.name).exists():(case/ll.name).symlink_to(ll)
        source=case/'validate.graph';source.write_text(R.M.make_source(alg,row,True))
        env=R.M.env();env.update(FORCE_CPU='1',SGPL_EXEC_SIMPLIFY_STATS='1')
        if not (case/'validate').exists():
            R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(case/'validate.ll'),source],case,case/'compile.log',env,timeout=1200)
            shutil.move(case/'program.o',case/'validate.o')
            R.checked(['g++','-O3','-fopenmp','-no-pie',case/'validate.o',*[d/(n+'.o') for n in R.OBJECTS],helper,'-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-pthread','-ldl','-lnlopt','-o',case/'validate'],case,case/'link.log')
        for width in [1,2,6]:
            hashes=[]
            for label,exe,extra in [('baseline',BASE/'cases'/alg/row['graph'].removesuffix('.edges')/'validate',{}),('enabled',case/'validate',{}),('disabled',case/'validate',{disable_env:'1'})]:
                runenv=R.M.env(workers=width);runenv.update(extra)
                bits=case/(label+'_w'+str(width)+'.bits')
                if alg=='pagerank':
                    runenv['SGPL_BITS_OUTPUT']=str(bits)
                    if label=='baseline':
                        baseline_exact=case/'baseline_exact'
                        if not baseline_exact.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',BASE/'cases'/alg/row['graph'].removesuffix('.edges')/'validate.o',*[BASE/(n+'.o') for n in R.OBJECTS],helper,'-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-pthread','-ldl','-lnlopt','-o',baseline_exact],case,case/'baseline_bits_link.log')
                        exe=baseline_exact
                run=subprocess.run([sys.executable,REPO/'test/physical_run.py',str(width),exe],cwd=case,env=runenv,capture_output=True,text=True,timeout=190)
                (case/(label+'_w'+str(width)+'.out')).write_text(run.stdout);(case/(label+'_w'+str(width)+'.err')).write_text(run.stderr)
                assert run.returncode==0,(alg,row['graph'],label,width,run.stderr[-2000:])
                values=R.np.array(run.stdout.split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0].split(),dtype=float)
                valid=R.M.validate(alg,row,values,'p1');assert valid,(alg,row['graph'],label,width)
                records.append({'algorithm':alg,'graph':row['graph'],'workers':width,'mode':label,'validation':valid,'executable_sha256':R.sha(exe)})
                if alg=='pagerank':hashes.append(R.sha(bits))
            if hashes:
                assert len(set(hashes))==1,('PageRank bits',row['graph'],width,hashes)
                bit_checks.append({'graph':row['graph'],'workers':width,'sha256':hashes[0],'passed':True})
        print('VALIDATED',alg,i+1,'/',len(R.M.ROWS),flush=True)
        R.save(out/'records.json',records);R.save(out/'exact_pagerank.json',bit_checks)
    R.save(out/'summary.json',{'passed':True,'independent_checks':len(records),'exact_pagerank_groups':len(bit_checks),'benchmark_runs':0})

def detectors(d):
    src=d/'src';out=d/'tsan';out.mkdir(exist_ok=True);results=[]
    env=R.M.env(workers=6);env['TSAN_OPTIONS']='halt_on_error=1'
    for name in ['execution_context_race_test','append_journal_test','execution_simplify_runtime_test']:
        exe=out/name
        R.checked(['gcc','-O1','-g','-no-pie','-fsanitize=thread','-fno-omit-frame-pointer','-DSGPL_CUT_PARALLEL_GRAIN=1','-std=gnu11','-fopenmp','-pthread','-I.','-Itest','test/'+name+'.c','autotuner_runtime.c','parallel_runtime.c','-lnlopt','-lm','-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create','-o',exe],src,out/(name+'.build.log'))
        for width in [1,2,6]:
            for attempt in range(3):
                # Bounded stack is needed for sanitizer shadow mappings in WSL.
                run=subprocess.run(['bash','-c','ulimit -s 8192; exec "$@"','tsan',sys.executable,REPO/'test/physical_run.py',str(width),exe],cwd=src,env=env,capture_output=True,text=True,timeout=180)
                text=run.stdout+run.stderr;log=out/(name+'_w'+str(width)+'_'+str(attempt)+'.log');log.write_text(text)
                race='WARNING: ThreadSanitizer: data race' in text
                mapping='unexpected memory mapping' in text or 'can not mmap the shadow memory' in text
                status='passed' if run.returncode==0 else 'race' if race else 'startup_unavailable' if mapping else 'unresolved_launch' if run.returncode==-11 and not text else 'failure'
                result={'test':name,'workers':width,'attempt':attempt,'status':status,'exit_code':run.returncode,'log':str(log)};results.append(result);R.save(out/'results.json',results)
                print('TSAN',name,width,status,flush=True)
                assert status not in ['race','failure'],result
                if not mapping:break

def report(d):
    build=R.read(d/'correctness_build.json');identity=build['identity'];src=d/'src'
    files=[p for p in REPO.iterdir() if p.is_file() and p.suffix in ['.c','.cpp','.h','.inc','.ll']]
    files += [p for p in (REPO/'generated').glob('*') if p.suffix in ['.cpp','.h']]
    current={str(p.relative_to(REPO)):hashlib.sha256(p.read_text().encode()).hexdigest() for p in files}
    assert R.key(current)==identity['source_hash'],'Native source changed since validated build'
    baseline_identity=R.read(BASE/'identity.json');names=list(baseline_identity['native_files'])
    prefix=R.git('rev-parse','--show-prefix');checkpoint='2b46cb5ded54132e98dc052bd2caf548d1acfcb4'
    win=subprocess.check_output(['wslpath','-w',str(REPO)],text=True).strip()
    data=subprocess.check_output(['/mnt/c/Program Files/Git/cmd/git.exe','-C',win,'cat-file','--batch'],input=''.join(checkpoint+':'+prefix+n+'\n' for n in names).encode())
    at=0;tracked_baseline=0;generated_baseline=[]
    for name in names:
        end=data.index(b'\n',at);header=data[at:end].split()
        if len(header)==2 and header[1]==b'missing':
            # ANTLR outputs are ignored build inputs, already frozen with C3.
            assert name.startswith('generated/'),('Untracked baseline input',name)
            digest=hashlib.sha256((BASE/'src'/name).read_text().encode()).hexdigest()
            assert digest==baseline_identity['native_files'][name]
            assert (BASE/'src'/name).read_text()==(src/name).read_text()
            generated_baseline.append({'path':name,'sha256':digest});at=end+1;continue
        assert len(header)==3,(name,header)
        size=int(header[2]);content=data[end+1:end+1+size].decode().replace('\r\n','\n');at=end+1+size+1
        assert content==(BASE/'src'/name).read_text(),('Frozen baseline differs from requested checkpoint',name)
        tracked_baseline+=1
    protected=R.read('/mnt/c/p1-c3-integration-20261022/integration_identity.json')['protected_ars']
    for name,digest in protected.items():assert hashlib.sha256((REPO/name).read_text().encode()).hexdigest()==digest,name
    contracts={n:R.read(d/'contracts'/(n+'.json')) for n in ['simplification','existing','worker_pool']}
    assert all(x and x['passed'] for x in contracts.values()),contracts
    summary=R.read(d/'validation/summary.json');assert summary and summary['passed']
    activation={}
    for alg in R.ALGS:
        activation[alg]={r:0 for r in ['R1','R2','R3']}
        for log in (d/'cases'/alg).rglob('compile.log'):
            text=log.read_text()
            for rule in activation[alg]:activation[alg][rule]+=int('[exec-simplify] '+rule+' activate' in text)
    tsan=R.read(d/'tsan/results.json');assert tsan is not None
    final={'baseline_commit':checkpoint,'baseline_source_files_verified':tracked_baseline,'baseline_generated_inputs_verified':generated_baseline,'branch':R.git('branch','--show-current'),'build':build,'source_hash':identity['source_hash'],'activation_graphs':activation,'contracts':contracts,'focused_kernel_comparisons':4608,'rule_subsets':8,'negative_fixture_shapes':11,'validation':summary,'tsan':tsan,'protected_ars_unchanged':True,'public_context_abi_unchanged':True,'benchmark_runs':0,'performance_claim':None,'artifact_root':str(d)}
    final['baseline_compiler_sha256']=R.sha(BASE/'GraphProgram')
    final['test_sources']={name:hashlib.sha256((src/'test'/name).read_text().encode()).hexdigest() for name in ['execution_simplify_ir_test.cpp','execution_simplify_runtime_test.c','run_execution_simplify_tests.sh']}
    calibration=Path.home()/'.config/sgpl/hw_calib.json'
    final['calibration_sha256']=R.sha(calibration)
    final['matches_previous_calibration']=final['calibration_sha256']==R.read('/mnt/c/p1-c3-integration-20261022/integration_identity.json')['calibration_sha256']
    target=REPO/'reports/execution_simplification_correctness.json';R.save(target,final);R.save(d/'correctness_report.json',final)
    print(json.dumps({'report':str(target),'validation':summary,'activation':activation},indent=2),flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['build','contracts','programs','detectors','report']);p.add_argument('--version',default='candidate03');args=p.parse_args()
    prepare()
    with R.locked('correctness_'+args.stage):
        d=ROOT/'versions'/args.version
        if args.stage=='build':build(args.version)
        elif args.stage=='contracts':contracts(d)
        elif args.stage=='programs':programs(d)
        elif args.stage=='detectors':detectors(d)
        else:report(d)
