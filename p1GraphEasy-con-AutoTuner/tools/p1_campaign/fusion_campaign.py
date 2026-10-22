"""Kaggle-only proof-driven execution fusion experiment; no local native execution."""
from pathlib import Path
import argparse,collections,csv,html,importlib.util,json,math,os,statistics,subprocess,sys,tarfile,time
import portable_campaign as P
ALGS=P.ALGS
TRANSFORMATION='A'

def p1_versions():return (['OriginB0'] if (P.BUNDLE/'origin_b0').exists() else [])+['Accepted','StaticSchedule']

def guard():
    if not Path('/kaggle/working').is_dir() or not Path('/kaggle/input').is_dir():
        raise RuntimeError('This campaign permits native execution only on Kaggle')

def durable_checkpoint(R,stage,algorithm=None,phases=None):
    """Persist completed blocks before a session can expire; no child is live.
    Archives are append-only, atomically published in Kaggle's output folder.
    No compression or archive scan overlaps a measured native child.
    """
    guard();directory=Path('/kaggle/working/campaign-checkpoints');directory.mkdir(exist_ok=True)
    name=stage+'_'+(algorithm or 'all')+'_'+str(time.time_ns())
    temporary=directory/(name+'.partial');destination=directory/(name+'.tar.gz')
    started=time.time();R.state(stage='checkpoint',algorithm=algorithm,completed=0,total=1,
        expected_completion_unix=None,elapsed_seconds=0)
    paths=[p for p in R.OUT.rglob('*') if p.is_file() and not p.name.endswith('.tmp')]
    for version in (P.WORK/'versions').glob('*'):
        paths.extend(p for p in version.glob('*.json') if p.is_file())
        if stage=='build':paths.extend(p for p in version.rglob('*.log') if p.is_file())
    for phase in phases or []:
        for root in (P.WORK/'runs').glob('*/'+phase+('/'+algorithm if algorithm else '')):
            paths.extend(p for p in root.rglob('*') if p.is_file() and not p.name.endswith('.tmp'))
    paths.extend(p for p in [P.WORK/'host.json',P.WORK/'campaign-input-identity.json'] if p.exists())
    with tarfile.open(temporary,'w:gz',compresslevel=1) as archive:
        for path in sorted(set(paths)):archive.add(path,arcname='p1-campaign/'+str(path.relative_to(P.WORK)),recursive=False)
        for name in ['bundle_manifest.json','provenance.json']:
            archive.add(P.BUNDLE/name,arcname='p1-bundle/'+name,recursive=False)
    with temporary.open('rb') as stream:os.fsync(stream.fileno())
    temporary.replace(destination)
    P.save(destination.with_suffix('.identity.json'),{'stage':stage,'algorithm':algorithm,'sha256':P.sha(destination),
        'bundle_manifest':P.sha(P.BUNDLE/'bundle_manifest.json'),'host':R.read(P.WORK/'host.json'),
        'scope':'completed blocks; active-block partials remain in the atomic run journal until the next checkpoint'})
    R.progress(stage='checkpoint_complete',algorithm=algorithm,completed=1,total=1,elapsed_seconds=time.time()-started)

def setup():
    global TRANSFORMATION
    guard();R,cpus=P.setup()
    provenance=json.loads((P.BUNDLE/'provenance.json').read_text())
    TRANSFORMATION=provenance.get('transformation',provenance['attempt'][0]).upper()
    assert TRANSFORMATION in ['A','B','C'],('Unknown transformation',TRANSFORMATION)
    for name,h in provenance['protected_ars'].items():
        for version in ['accepted','candidate']+(['origin_b0'] if (P.BUNDLE/'origin_b0').exists() else []):
            import hashlib
            assert hashlib.sha256((P.BUNDLE/version/name).read_text().encode()).hexdigest()==h,('ARS changed',version,name)
    original=R.checked
    calibration=Path(os.environ.get('AUTOTUNER_HW_CALIB',str(Path.home()/'.config/sgpl/hw_calib.json')))
    calibration_identity={'path':str(calibration),'sha256':P.sha(calibration) if calibration.exists() else None,
        'note':'Existing calibration retained; absent file means unchanged compiled defaults.'}
    old=R.read(R.OUT/'ars_calibration_identity.json')
    if old is not None:assert old==calibration_identity,'ARS calibration input changed'
    else:P.save(R.OUT/'ars_calibration_identity.json',calibration_identity)
    def checked(cmd,cwd,log,env=None,timeout=1800):
        e=dict(os.environ if env is None else env)
        if any(str(x).endswith('GraphProgram') for x in cmd):
            assert (P.sha(calibration) if calibration.exists() else None)==calibration_identity['sha256']
            e.update(SGPL_FUSION_STATS='1',SGPL_WITNESS_DUMP='1')
        return original(cmd,cwd,log,e,timeout)
    R.checked=checked
    R.evidence_checkpoint=lambda stage,algorithm=None,phases=None:durable_checkpoint(R,stage,algorithm,phases)
    def compiler_ready(version,src):
        if version!='StaticSchedule':return
        marker=R.OUT/'fusion_contracts_passed.json';passed=R.read(marker)
        fingerprint=R.sha(src.parent/'GraphProgram')
        if passed:
            assert passed['manifest']==P.sha(P.BUNDLE/'bundle_manifest.json') and passed.get('compiler_sha256')==fingerprint,'Compiler contract inputs changed; retain evidence and start a separate attempt'
            return
        env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,widths(cpus)))
        R.progress(stage='frontend_contracts',algorithm=None,completed=0,total=1,failures=0)
        R.checked([sys.executable,'test/check_fullgraph.py'],src,R.OUT/'fusion_frontend_contracts.log',env)
        if (src/'test/check_execution_fusion.py').exists():
            R.checked([sys.executable,'test/check_execution_fusion.py'],src,R.OUT/'execution_fusion_frontend_contracts.log',env)
        P.save(marker,{'passed':True,'manifest':P.sha(P.BUNDLE/'bundle_manifest.json'),'compiler_sha256':fingerprint})
        R.evidence_checkpoint('frontend_contracts')
    R.COMPILER_READY_HOOK=compiler_ready
    return R,cpus

def specs(a):
    s=[('Accepted','accepted',{}),('StaticSchedule','candidate',{}),('Accepted','accepted_AA',{}),
       ('StaticSchedule','candidate_AA',{}),('StaticSchedule','disabled',{'SGPL_NO_FUSION_'+TRANSFORMATION:'1'}),
       ('graptor_double' if a=='pagerank' else 'graptor','graptor',{'LD_PRELOAD':str(P.WORK/'gclock.so')})]
    if a=='pagerank':s.append(('graptor','graptor_float',{'LD_PRELOAD':str(P.WORK/'gclock.so')}))
    if (P.BUNDLE/'origin_b0').exists():s.append(('OriginB0','b0',{}))
    return s

def validation_specs(a):
    settings=[x for x in specs(a) if not x[1].endswith('_AA')]+[
        ('StaticSchedule','global_disabled',{'SGPL_NO_EFFECT_FUSION':'1'})]
    header=P.BUNDLE/'candidate/append_journal_plan.h'
    if header.exists() and 'SGPL_JOURNAL_BITMAP_PROTOCOL' in header.read_text():
        settings.append(('StaticSchedule','heap_control',{'SGPL_FUSION_C_HEAP_MERGE':'1'}))
    if header.exists() and 'SGPL_JOURNAL_PROVEN_APPEND' in header.read_text():
        settings.append(('StaticSchedule','checked_control',{'SGPL_FUSION_C_CHECKED_APPEND':'1'}))
    return settings

def widths(cpus):return sorted(set([1,min(2,len(cpus)),len(cpus)]))

def build(R,cpus):
    env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,widths(cpus)))
    R.checked(['gcc','-march=native','-Q','--help=target'],P.WORK,R.OUT/'native_gcc_target_flags.txt',env)
    R.checked(['clang-20','-###','-march=native','-c','-x','c','/dev/null','-o','/dev/null'],P.WORK,R.OUT/'native_clang_target_flags.txt',env)
    existing=R.read(R.OUT/'native_contracts_passed.json')
    if existing:assert existing['manifest']==P.sha(P.BUNDLE/'bundle_manifest.json'),'Native contract inputs changed'
    if not (R.OUT/'native_contracts_passed.json').exists():
        R.progress(stage='native_contracts',workers=len(cpus))
        R.checked(['bash','test/run_effect_fusion_tests.sh','native-only'],P.BUNDLE/'candidate',R.OUT/'fusion_native_preflight.log',env)
        race_detector(R,cpus,P.BUNDLE/'candidate')
        P.save(R.OUT/'native_contracts_passed.json',{'passed':True,'manifest':P.sha(P.BUNDLE/'bundle_manifest.json')})
        R.evidence_checkpoint('native_contracts')
    # Frontend contracts use already-built Accepted programs while the
    # candidate compiler is ready but its full program matrix is not built.
    P.save(R.OUT/'campaign_state.json',{'last_accepted_version':'Accepted'})
    P.build(R)
    activation=[]
    for a in ALGS:
        for row in R.M.ROWS:
            log=R.case('StaticSchedule',a,row)/'timed.compile.log'
            if log.exists():
                lines=[x for x in log.read_text(errors='replace').splitlines() if any(tag in x for tag in ['[effect-fusion]','[frontier-fusion]','[append-journal]'])]
                activation.append({'algorithm':a,'graph':row['graph'],'diagnostics':lines})
    P.save(R.OUT/'fusion_activation.json',activation)
    R.evidence_checkpoint('build')

def exact_bits(R,cpus):
    src=P.BUNDLE/'exact_bits.c';helper=P.WORK/'exact_bits.o'
    R.checked(['gcc','-O2','-c',src,'-o',helper],P.WORK,R.OUT/'exact_bits_build.log')
    identity=P.sha(src)[:16];results=[]
    for version in p1_versions():
        d=P.WORK/'versions'/version
        for row in R.M.ROWS:
            case=R.case(version,'pagerank',row);exe=case/('exact_validate.'+identity)
            if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',case/'validate.o',*[d/(x+'.o') for x in R.OBJECTS],
                helper,'-Wl,--wrap=printf','-Wl,--wrap=puts','-Wl,--wrap=__printf_chk','-pthread','-ldl','-lnlopt','-o',exe],case,case/'exact_bits_link.log')
    for w in widths(cpus):
        for i,row in enumerate(R.M.ROWS,1):
            values=[]
            for version,label,extra in [x for x in validation_specs('pagerank') if x[1] in ['accepted','candidate','disabled','global_disabled','b0','heap_control','checked_control']]:
                e=dict(extra,SGPL_BITS_HELPER_ID=identity)
                x=R.invoke('fusion','validate_bits',version,'pagerank',row,0,label=label,workers=w,extra=e,profile='bits')
                assert not x.get('error'),x
                stdout=Path(x['base']+'.out').read_text()
                count=len(stdout.split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0].split())
                bits=Path(x['base']+'.bits');assert bits.exists() and bits.stat().st_size==count*8
                values.append(P.sha(bits))
            assert len(set(values))==1,('Full-bit PageRank mismatch',row['graph'],w,values)
            results.append({'graph':row['graph'],'workers':w,'sha256':values[0],'passed':True})
            R.progress(stage='exact_bits',algorithm='pagerank',completed=i,total=61,workers=w)
        R.evidence_checkpoint('exact_bits_w'+str(w),'pagerank',['validate_bits'])
    P.save(R.OUT/'exact_fusion_pagerank.json',results)

def race_detector(R,cpus,src):
    env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,widths(cpus)))
    # Optional detector support is established by a standalone smoke probe.
    # An unsupported virtual address layout is not recorded as a passing test.
    import shutil
    tsan=R.OUT/'tsan';tsan.mkdir(exist_ok=True)
    code=tsan/'smoke.c';code.write_text('#include <pthread.h>\nstatic int x;static void *f(void *p){(void)p;x=1;return 0;}int main(void){pthread_t t;pthread_create(&t,0,f,0);pthread_join(t,0);return x!=1;}\n')
    compiled=subprocess.run(['gcc','-O1','-g','-no-pie','-fsanitize=thread','-fno-omit-frame-pointer','-pthread',code,'-o',tsan/'smoke'],capture_output=True,text=True)
    detector={'compiler_returncode':compiled.returncode,'compiler_output':compiled.stderr,
        'configuration':{'non_pie':True,'stack_limit_kib':8192}}
    def bounded_stack(command,**kwargs):
        return subprocess.run(['bash','-c','ulimit -s 8192; exec "$@"','p1-tsan',*map(str,command)],**kwargs)
    def launch(command,label,cwd=None,timeout=180):
        # Retry only sanitizer startup address-map failures. Preserve every
        # launch; a race or any functional failure is never retried or hidden.
        attempts=[]
        for attempt in range(5):
            result=bounded_stack(command,cwd=cwd,env=dict(env,TSAN_OPTIONS='halt_on_error=1'),capture_output=True,text=True,timeout=timeout)
            text=result.stdout+result.stderr
            path=tsan/(label+'_startup'+str(attempt)+'.log');path.write_text(text)
            mapping='WARNING: ThreadSanitizer: data race' not in text and ('FATAL: ThreadSanitizer: unexpected memory mapping' in text or 'ThreadSanitizer can not mmap the shadow memory' in text)
            attempts.append({'returncode':result.returncode,'startup_mapping_failure':mapping,'log':str(path)})
            if not mapping:break
        detector.setdefault('launches',{})[label]=attempts
        return result,text,mapping
    if not compiled.returncode:
        probe,text,mapping=launch([tsan/'smoke'],'smoke',timeout=30)
        detector.update(returncode=probe.returncode,output=text,supported=probe.returncode==0)
        if not probe.returncode:
            flags=['gcc','-O1','-g','-no-pie','-fsanitize=thread','-fno-omit-frame-pointer','-std=gnu11','-fopenmp','-pthread','-I.','-Itest']
            links=['-lnlopt','-lm','-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create']
            # Establish whether the new fixture also reports the original
            # daa63293 race. This negative control is not a candidate pass.
            origin=P.BUNDLE/'origin_b0'
            if not origin.exists():origin=P.BUNDLE/'accepted'
            fixture=src/'test/execution_context_race_test.c'
            if fixture.exists() and not (origin/'execution_context_copy.inc').exists():
                exe=tsan/'original_context_control'
                R.checked([*flags,fixture,'autotuner_runtime.c','parallel_runtime.c',*links,'-o',exe],origin,tsan/'original_context_control.build.log',env)
                rr,tt,mm=launch([sys.executable,src/'test/physical_run.py',str(len(cpus)),exe],'original_context_control',cwd=origin)
                detector['original_context_control']={'returncode':rr.returncode,'startup_unavailable':mm,
                    'race_reproduced':'WARNING: ThreadSanitizer: data race' in tt,'native_source':'origin_b0' if (P.BUNDLE/'origin_b0').exists() else 'accepted'}
            plans={'append_journal_test.c':'append_journal_plan.h','frontier_fusion_test.c':'frontier_fusion_plan.h','effect_fusion_runtime_test.c':'effect_fusion_plan.h'}
            fixtures=[n for n in ['execution_context_race_test.c',*plans] if (src/'test'/n).exists() and (n not in plans or (src/plans[n]).exists())]
            detector['fixtures']=fixtures;detector['contracts_passed']=True;detector['contract_runs']=[]
            for fixture in fixtures:
                exe=tsan/fixture.removesuffix('.c')
                R.checked([*flags,'test/'+fixture,'autotuner_runtime.c','parallel_runtime.c',*links,'-o',exe],src,tsan/(fixture+'.build.log'),env)
                for w in widths(cpus):
                    label=fixture+'_w'+str(w)
                    result,text,mapping=launch([sys.executable,'test/physical_run.py',str(w),exe],label,cwd=src)
                    detector['contract_runs'].append({'fixture':fixture,'workers':w,'returncode':result.returncode,'startup_unavailable':mapping})
                    if result.returncode:
                        detector['contracts_passed']=False
                        if mapping:
                            detector.update(supported=False,reason='Full executable cannot reserve ThreadSanitizer shadow memory after five retained startup attempts');break
                        P.save(R.OUT/'race_detector.json',detector)
                        raise RuntimeError('Race-detector contract failed: '+label)
                if not detector.get('supported'):break
    else:detector['supported']=False
    P.save(R.OUT/'race_detector.json',detector)

def validate(R,cpus):
    R.verify_inputs()
    env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,widths(cpus)))
    src=P.WORK/'versions/StaticSchedule/src'
    existing=R.read(R.OUT/'fusion_contracts_passed.json')
    if existing:assert existing['manifest']==P.sha(P.BUNDLE/'bundle_manifest.json') and existing.get('compiler_sha256')==R.sha(src.parent/'GraphProgram'),'Compiler contract inputs changed'
    if not (R.OUT/'fusion_contracts_passed.json').exists():
        if (R.OUT/'native_contracts_passed.json').exists():
            assert R.read(R.OUT/'native_contracts_passed.json')['manifest']==P.sha(P.BUNDLE/'bundle_manifest.json')
            R.checked([sys.executable,'test/check_fullgraph.py'],src,R.OUT/'fusion_frontend_contracts.log',env)
            if (src/'test/check_execution_fusion.py').exists():
                R.checked([sys.executable,'test/check_execution_fusion.py'],src,R.OUT/'execution_fusion_frontend_contracts.log',env)
        else:
            R.checked(['bash','test/run_effect_fusion_tests.sh'],src,R.OUT/'fusion_contracts.log',env)
            race_detector(R,cpus,src)
        P.save(R.OUT/'fusion_contracts_passed.json',{'passed':True,'manifest':P.sha(P.BUNDLE/'bundle_manifest.json'),'compiler_sha256':R.sha(src.parent/'GraphProgram')})
    records=[]
    for w in widths(cpus):
        for a in ALGS:
            started=time.monotonic()
            for i,row in enumerate(R.M.ROWS,1):
                for version,label,extra in validation_specs(a):
                    x=R.invoke('fusion','validate',version,a,row,0,label=label,workers=w,extra=extra)
                    assert x.get('validation') and not x.get('error'),x;records.append(x)
                R.progress(stage='validate',algorithm=a,workers=w,completed=i,total=61,failures=0,elapsed_seconds=time.monotonic()-started)
            R.evidence_checkpoint('validate_w'+str(w),a,['validate'])
    P.save(R.OUT/'validation_records.json',records)
    exact_bits(R,cpus)
    P.save(R.OUT/'validated.json',{'fingerprints':{v:R.verify_fingerprint(v) for v in p1_versions()},'workers':len(cpus),'passed':True})

def calibration_snapshot(R,cpus):
    # Preserve the existing ARS configuration and its natural warmup. Snapshot
    # every persisted launch-calibration input as well as the offline file.
    e=R.M.env(workers=len(cpus));base=Path(e.get('XDG_CACHE_HOME') or str(Path(e['HOME'])/'.cache'))
    directory=base/'sgpl-launch-v1'
    offline=R.read(R.OUT/'ars_calibration_identity.json');file=Path(offline['path'])
    assert (P.sha(file) if file.exists() else None)==offline['sha256'],'Offline calibration input changed'
    return {'offline':offline, 'launch_directory':str(directory),
        'launch_files':{str(p.relative_to(directory)):P.sha(p) for p in sorted(directory.glob('*.bin'))} if directory.exists() else {}}

def freeze_calibration(R,cpus,phase):
    snapshot=calibration_snapshot(R,cpus);frozen=R.OUT/'frozen_calibration_inputs.json'
    old=R.read(frozen)
    if old is None:
        P.save(frozen,snapshot)
        import shutil
        d=R.OUT/'frozen_calibration_files';d.mkdir(exist_ok=True)
        for name in snapshot['launch_files']:shutil.copy2(Path(snapshot['launch_directory'])/name,d/name)
    elif old!=snapshot:
        P.save(R.OUT/(phase+'_changed_calibration_inputs.json'),{'before':old,'after':snapshot,'valid_for_acceptance':False})
        raise RuntimeError('Frozen ARS calibration inputs changed; preserve this block and investigate before promotion')
    return snapshot

def benchmark(R,cpus,phase,reps):
    valid=R.read(R.OUT/'validated.json');assert valid and valid['passed']
    R.verify_inputs()
    for v,h in valid['fingerprints'].items():assert R.verify_fingerprint(v)==h
    # Run the specified single warmup for every label first. Later calls reuse
    # these same checkpointed warmup records, rather than launching duplicates.
    # This primes every timed-program shape before freezing persisted inputs.
    for a in ALGS:
        for i,row in enumerate(R.M.ROWS,1):
            for v,label,extra in specs(a):
                x=R.invoke('fusion','warmup_'+phase,v,a,row,0,label=label,workers=len(cpus),extra=extra)
                assert not x.get('error'),x
            R.progress(stage='warmup_'+phase,algorithm=a,completed=i,total=61,failures=0)
    freeze_calibration(R,cpus,phase)
    records=[]
    order_scheme='adjacent-reversed-rotation-pairs-v1'
    P.save(R.OUT/(phase+'_timing_protocol.json'),{'order_scheme':order_scheme,'repetitions':reps,
        'note':'Every adjacent pair uses one order and its exact reverse. Even confirmation blocks balance every pairwise order and mean launch position; a five-repeat primary has one unmatched rotation. This does not identify the cause of any earlier regression.'})
    for a in ALGS:
        started=time.monotonic()
        mode=phase.split('_retry')[0];signature=[61,reps,[x[1] for x in specs(a)],len(cpus),order_scheme]
        previous=R.read(R.OUT/'calibration.json',{}).get(R.key([a,mode,signature]))
        R.state(stage=phase,algorithm=a,completed=0,total=61,block_started=time.time(),
            monitor_interval_seconds=previous['seconds'] if previous else None,
            expected_completion_unix=time.time()+previous['seconds'] if previous else None)
        for i,row in enumerate(R.M.ROWS,1):
            choices=specs(a)
            for v,label,extra in choices:
                x=R.invoke('fusion','warmup_'+phase,v,a,row,0,label=label,workers=len(cpus),extra=extra)
                assert not x.get('error'),x
            for rep in range(reps):
                shift=(i+rep//2)%len(choices);order=choices[shift:]+choices[:shift]
                if rep%2:order=order[::-1]
                for v,label,extra in order:
                    x=R.invoke('fusion',phase,v,a,row,rep,label=label,workers=len(cpus),extra=extra)
                    assert not x.get('error') and x['seconds']>0,x;records.append(x)
            P.save(R.OUT/(phase+'.json'),records)
            R.progress(stage=phase,algorithm=a,completed=i,total=61,failures=0,elapsed_seconds=time.monotonic()-started)
        R.evidence_checkpoint(phase,a,[phase,'warmup_'+phase])
        R.calibrate(a,mode,signature,time.time()-(time.monotonic()-started))
        freeze_calibration(R,cpus,phase+'_'+a)
        P.save(R.OUT/(phase+'_'+a+'_duration.json'),{'seconds':time.monotonic()-started,'cases':61,'repetitions':reps,'workers':len(cpus)})

def analyzer(R):
    # Reuse the audited paired statistics and raw-record verification; no local
    # execution is performed by this import. The campaign supplies its labels.
    path=Path(__file__).with_name('local_current_comparison.py');old=sys.argv
    try:
        sys.argv=[str(path),'--output',str(R.OUT),'--baseline','Accepted','--current','StaticSchedule']
        spec=importlib.util.spec_from_file_location('fusion_statistics',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    finally:sys.argv=old
    module.specs=lambda a:[(label,v) for v,label,e in specs(a)]
    return module

def eliminated_work(R):
    profile=R.read(R.OUT/'profile_summary.json')
    if not profile:return {'available':False,'note':'Profiling has not completed; no work-elimination claim yet.'}
    width=R.read(P.WORK/'host.json')['workers'];groups=profile['groups']
    base={(x['algorithm'],x['graph']):x for x in groups if x['workers']==width and x['label']=='baseline'}
    cases=[]
    for c in groups:
        key=(c['algorithm'],c['graph'])
        if c['workers']!=width or c['label']!='candidate' or key not in base:continue
        b=base[key].get('work_means',{});w=c.get('work_means',{})
        cases.append({'algorithm':key[0],'graph':key[1],
            'source_map_loop_vertices_removed':b.get('separate_map_loop_vertices',0)-w.get('separate_map_loop_vertices',0),
            'copied_record_payload_bytes_removed':b.get('record_or_descriptor_bytes',0)-w.get('record_or_descriptor_bytes',0),
            'cold_native_metadata_bytes_added':w.get('cold_native_metadata_bytes',0)-b.get('cold_native_metadata_bytes',0),
            'dense_append_slot_domain_removed':b.get('dense_append_slot_domain',0)-w.get('dense_append_slot_domain',0),
            'journal_entries_merged':w.get('journal_entries_merged',0),
            'proven_append_entries':w.get('proven_append_entries',0),
            'proven_previous_key_reads_removed':w.get('proven_previous_key_reads_removed',0)})
    field={'A':'source_map_loop_vertices_removed','B':'copied_record_payload_bytes_removed','C':'dense_append_slot_domain_removed'}[TRANSFORMATION]
    evidence={'available':True,'workers':width,'profiled_cases':len(cases),'complete_244_coverage':len(cases)==244,
        'cases_with_demonstrated_elimination':sum(c[field]>0 for c in cases),'cases':cases,
        'proven_append_cases':sum(c['proven_append_entries']>0 for c in cases),
        'proven_previous_key_reads_removed':sum(c['proven_previous_key_reads_removed'] for c in cases),
        'note':'Instrumented work counts, excluded from acceptance timing. Record-payload reductions exclude separately reported cold metadata. Dense append slot domain counts the logical compaction domain, not hardware memory transactions.'}
    P.save(R.OUT/'eliminated_work.json',evidence);return evidence

def report(R,results):
    chosen=next((p for p in ['confirmation_retry1','confirmation','primary_retry1','primary'] if p in results))
    s=results[chosen]['summary'];primary=results.get('primary_retry1',results['primary'])['summary']
    passed=lambda x:x['valid_for_inference'] and not x['confirmed_code_regressions'] and x['code_mean_geomean_speedup']>1.01 and x['code_median_geomean_speedup']>1.01
    timing_passed=chosen.startswith('confirmation') and passed(primary) and passed(s)
    work=eliminated_work(R)
    # A's ordered-store elimination is checked in the exact IR contracts;
    # source-map traversal counts alone cannot establish its inactivity.
    work_ready=work.get('available',False) and work.get('complete_244_coverage',False)
    header=P.BUNDLE/'candidate/append_journal_plan.h'
    proven_required=TRANSFORMATION=='C' and header.exists() and 'SGPL_JOURNAL_PROVEN_APPEND' in header.read_text()
    inactive=timing_passed and work_ready and ((TRANSFORMATION in ['B','C'] and not work['cases_with_demonstrated_elimination']) or (proven_required and not work['proven_append_cases']))
    accepted=timing_passed and work_ready and not inactive
    pending=timing_passed and not work_ready
    decision='accepted' if accepted else ('pending_evidence' if pending else ('inconclusive' if not s['valid_for_inference'] or not primary['valid_for_inference'] else 'unaccepted'))
    classification='passing' if accepted else ('inconclusive' if decision=='inconclusive' else
        ('regressed' if s['confirmed_code_regressions'] or primary['confirmed_code_regressions'] else 'no_worthwhile_gain'))
    if inactive:classification='no_demonstrated_elimination'
    if pending:classification='awaiting_full_profile'
    provenance=R.read(P.BUNDLE/'provenance.json')
    output={'decision':decision,'classification':classification,'timing_passed':timing_passed,'full_work_evidence_complete':work_ready,'scope':'single stage attempt','transformation':TRANSFORMATION,
        'chosen':chosen,'candidate_retained':True,'baseline':provenance['baseline_commit'],'origin':provenance.get('origin_commit','daa63293'),
        'attempt':provenance['attempt'],'baseline_note':provenance.get('baseline_note'),'protected_ars':provenance['protected_ars'],'phases':{p:r['summary'] for p,r in results.items()},
        'timing_gates_passed':timing_passed,'eliminated_work':{k:v for k,v in work.items() if k!='cases'},
        'note':'Failed A/A prevents inference and promotion; raw slowdowns remain visible. No ARS code changed.'}
    P.save(R.OUT/'campaign_decision.json',output);P.save(R.OUT/'final_summary.json',output)
    doc=['<!doctype html><meta charset="utf-8"><title>P1 execution fusion results</title><style>body{font:15px system-ui;margin:24px}pre{white-space:pre-wrap}table{border-collapse:collapse}td,th{padding:6px;border:1px solid #bbb;text-align:right}td:first-child{text-align:left}.bad{background:#ffdddd}</style>',
        '<h1>Proof-driven execution fusion: '+html.escape(provenance['attempt'])+'</h1>',
        '<p>Decision: <strong>'+decision+'</strong>. Kaggle only, physical-core representatives, ARS frozen. Whole-process monotonic timing includes setup and construction. PageRank main comparison uses double precision.</p>',
        '<p>Tests permute paired log time ratios; algorithm tests average these ratios across the 61 graphs within each repetition. Seed 2358, 100000 resamples, Holm correction across 244 case and four algorithm tests per family. Code slowdown tests are one-sided; A/A tests are two-sided. Kernel columns contain the existing internal compute timers; separately instrumented owner phases are reported in profiling evidence.</p>']
    doc+=['<p>The five-pair primary block has only 32 distinct sign permutations. Its smallest one-sided p-value is 1/32, insufficient to survive correction over 248 tests. A zero primary regression count therefore supplies no per-case no-regression assurance; promotion requires the independent 20-pair confirmation and its valid A/A controls.</p>']
    if provenance.get('baseline_note'):doc+=['<p><strong>Baseline prerequisite:</strong> '+html.escape(provenance['baseline_note'])+'</p>']
    doc+=['<p><strong>Measured work elimination:</strong> '+html.escape(json.dumps({k:v for k,v in work.items() if k!='cases'}))+'</p>']
    csv_rows=[]
    for phase,result in results.items():
        q=result['summary'];short={k:v for k,v in q.items() if k not in ['tests']}
        rows=result['cases'];origin_label='b0' if 'b0' in rows[0]['mean_ms'] else 'accepted'
        if origin_label=='accepted':assert provenance['baseline_commit'].startswith(provenance.get('origin_commit','daa63293')),'Missing same-host B0 measurements'
        def aggregates(xs):
            return {'cases':len(xs),'incremental_mean_geomean_speedup':math.exp(statistics.mean(math.log(x['mean_ms']['accepted']/x['mean_ms']['candidate']) for x in xs)),
                'incremental_median_geomean_speedup':math.exp(statistics.mean(math.log(x['median_ms']['accepted']/x['median_ms']['candidate']) for x in xs)),
                'cumulative_mean_geomean_speedup_over_b0':math.exp(statistics.mean(math.log(x['mean_ms'][origin_label]/x['mean_ms']['candidate']) for x in xs)),
                'cumulative_median_geomean_speedup_over_b0':math.exp(statistics.mean(math.log(x['median_ms'][origin_label]/x['median_ms']['candidate']) for x in xs)),
                'totals_ms':{label:sum(x['mean_ms'][label] for x in xs) for label in ['accepted','candidate',origin_label,'graptor']}}
        q['campaign_aggregates']={'full61':aggregates(rows),'original50':aggregates([x for x in rows if x['cohort']=='main50'])}
        unique={}
        for x in rows:
            r=next(r for r in R.M.ROWS if r['graph']==x['graph'])
            key=(x['algorithm'],r['original_sha256'],R.sha(r['bin']),r['p1_directed'],r['source'] if x['algorithm'] in ['bfs','sssp'] else None,R.sha(r['p1_weighted_file']) if x['algorithm']=='sssp' else None)
            unique.setdefault(key,[]).append(x)
        reduced=[]
        for group in unique.values():
            reduced.append({kind:{label:statistics.mean(x[kind][label] for x in group) for label in group[0][kind]} for kind in ['mean_ms','median_ms']})
        q['campaign_aggregates']['duplicate_content_sensitivity']=aggregates(reduced)
        short['campaign_aggregates']=q['campaign_aggregates']
        doc+=['<h2>'+phase+'</h2><pre>'+html.escape(json.dumps(short,indent=2))+'</pre>']
        if not q['valid_for_inference']:doc+=['<p class="bad">A/A FAILED: no certified improvement or no-regression claim. Counts suppressed by this gate do not establish absence of regressions.</p>']
        for a in ALGS:
            doc+=['<h3>'+a+'</h3><table><tr><th>Graph</th><th>Last accepted ms</th><th>Candidate ms</th><th>B0 ms</th><th>Stage disabled ms</th><th>Graptor double ms</th><th>Graptor float ms</th><th>Change %</th><th>Baseline kernel ms</th><th>Candidate kernel ms</th><th>B0 kernel ms</th><th>Graptor kernel ms</th><th>Code slowdown Holm p</th><th>Status</th></tr>']
            for x in result['cases']:
                if x['algorithm']!=a:continue
                t=x['mean_ms'];change=(t['candidate']/t['accepted']-1)*100
                cells=[html.escape(x['graph'])]+[f'{t[k]:.6f}' for k in ['accepted','candidate',origin_label,'disabled','graptor']]
                cells += [f"{t['graptor_float']:.6f}" if a=='pagerank' else 'n/a',f'{change:+.3f}']
                cells += [f"{x['kernel_mean_ms'][k]:.6f}" if x['kernel_mean_ms'][k] is not None else 'missing' for k in ['accepted','candidate',origin_label,'graptor']]
                cells += [f"{x['tests']['code_regressions']['holm_p']:.6g}"]
                confirmed=q['valid_for_inference'] and x['tests']['code_regressions']['holm_p']<.05
                status='inconclusive A/A' if not q['valid_for_inference'] else ('confirmed regression' if confirmed else ('nominal slowdown' if change>0 else ('nominal improvement' if change<0 else 'flat')))
                cells += [status]
                csv_row={'phase':phase,'algorithm':a,'graph':x['graph'],'cohort':x['cohort'],
                    'AA_valid':q['valid_for_inference'],'regression_status':status,'change_percent':change,
                    'code_slowdown_holm_p':x['tests']['code_regressions']['holm_p']}
                for label,key in [('last_accepted','accepted'),('candidate','candidate'),('B0',origin_label),
                        ('stage_disabled','disabled'),('graptor_double','graptor'),('graptor_historical_float','graptor_float')]:
                    csv_row[label+'_end_to_end_ms']=t.get(key)
                    csv_row[label+'_internal_compute_ms']=x['kernel_mean_ms'].get(key)
                csv_rows.append(csv_row)
                doc+=['<tr'+(' class="bad"' if change>0 else '')+'>'+''.join('<td>'+c+'</td>' for c in cells)+'</tr>']
            doc+=['</table>']
    if (R.OUT/'profile_summary.json').exists():
        doc+=['<h2>Instrumented execution evidence</h2><p>Separate from acceptance samples. See the raw phase, work, CPU, wait and memory counters in profile_summary.json; component and cross-link recovery tests are in diagnostic_summary.json when applicable.</p>']
        output['profiling_evidence']='profile_summary.json'
    P.save(R.OUT/'campaign_decision.json',output);P.save(R.OUT/'final_summary.json',output)
    (R.OUT/'final_report.html').write_text('\n'.join(doc));P.save(R.OUT/'comparison.json',{'summary':output,'cases':[x for r in results.values() for x in r['cases']]})
    csv_path=R.OUT/'per_graph_algorithm_times.csv';temporary=csv_path.with_suffix('.csv.tmp')
    with temporary.open('w',newline='') as stream:
        writer=csv.DictWriter(stream,fieldnames=list(csv_rows[0]));writer.writeheader();writer.writerows(csv_rows)
    temporary.replace(csv_path)
    return output

def main():
    p=argparse.ArgumentParser();p.add_argument('stage',choices=['all','build','contracts','validate','primary','confirmation','report','status','resume','profile','diagnose','investigate']);p.add_argument('--cases');args=p.parse_args()
    R,cpus=setup();P.specs=specs
    default_cases=P.BUNDLE/'candidate/tools/p1_campaign/diagnostic_cases.json'
    if args.cases is None and default_cases.exists():args.cases=str(default_cases)
    if args.stage=='status':
        current=R.read(R.OUT/'status.json',{})
        status={key:current.get(key) for key in ['stage','algorithm','completed','total','failures','elapsed_seconds','expected_completion_unix']}
        status['log_locations']={'results':str(R.OUT),'raw_runs':str(P.WORK/'runs'),'bootstrap':'/kaggle/working/bootstrap.log'}
        print(json.dumps(status),flush=True);return
    if args.stage=='resume':args.stage='all'
    with R.locked('fusion_'+args.stage):
        if args.stage=='contracts':
            env=R.M.env(workers=len(cpus));env['P1_TEST_WIDTHS']=' '.join(map(str,widths(cpus)))
            R.checked(['bash','test/run_effect_fusion_tests.sh','native-only'],P.BUNDLE/'candidate',R.OUT/'fusion_contracts.log',env)
            race_detector(R,cpus,P.BUNDLE/'candidate')
            P.save(R.OUT/'native_contracts.json',{'passed':True,'bundle_manifest':P.sha(P.BUNDLE/'bundle_manifest.json'),
                'native_inputs':{f.name:P.sha(f) for f in (P.BUNDLE/'candidate').iterdir() if f.is_file() and f.suffix in ['.c','.cpp','.h','.inc','.ll']}})
            R.progress(stage='contracts_complete',workers=len(cpus));return
        if args.stage=='investigate':
            # A new measurement host rebuilds every pinned binary. This stage
            # produces diagnostic evidence only; it cannot promote a candidate.
            assert args.cases,'An investigation requires an explicit frozen case list'
            build(R,cpus);validate(R,cpus)
            import fusion_diagnostics as D
            D.run(R,cpus,phase=False,case_path=args.cases)
            D.run(R,cpus,phase=True,case_path=args.cases)
            R.progress(stage='investigation_complete');return
        if args.stage in ['profile','diagnose']:
            import fusion_diagnostics as D
            D.run(R,cpus,phase=args.stage=='profile',case_path=args.cases);return
        if args.stage in ['all','build']:build(R,cpus)
        if args.stage=='build':return
        if args.stage in ['all','validate']:validate(R,cpus)
        if args.stage=='validate':return
        A=analyzer(R);results={}
        for phase,n in [('primary',5),('confirmation',20)]:
            if phase=='confirmation':
                q=results.get('primary_retry1',results['primary'])['summary']
                if not(q['valid_for_inference'] and not q['confirmed_code_regressions'] and q['code_mean_geomean_speedup']>1.01 and q['code_median_geomean_speedup']>1.01):break
            if args.stage in ['all',phase]:benchmark(R,cpus,phase,n)
            if not (R.OUT/(phase+'.json')).exists():break
            results[phase]=A.analyze(phase)
            if not results[phase]['summary']['valid_for_inference']:
                retry=phase+'_retry1'
                if args.stage!='report':benchmark(R,cpus,retry,n)
                if (R.OUT/(retry+'.json')).exists():results[retry]=A.analyze(retry)
        result=report(R,results)
        print(json.dumps({'stage':'timing_decision','attempt':result['attempt'],'decision':result['decision'],
            'classification':result['classification'],'blocks':{name:{
                'valid_AA':data['summary']['valid_for_inference'],
                'mean_geomean_speedup':data['summary']['code_mean_geomean_speedup'],
                'median_geomean_speedup':data['summary']['code_median_geomean_speedup'],
                'confirmed_slowdowns':len(data['summary']['confirmed_code_regressions'])}
                for name,data in results.items()},'profiling_complete':False}),flush=True)
        if args.stage=='all' and not result['timing_passed']:
            # Diagnose while the exact measured executables and host still
            # exist. Instrumented results remain outside acceptance records.
            import fusion_diagnostics as D
            D.run(R,cpus,phase=False,case_path=args.cases)
            D.run(R,cpus,phase=True,case_path=args.cases)
        elif args.stage=='all':
            import fusion_diagnostics as D
            if args.cases:D.run(R,cpus,phase=False,case_path=args.cases)
            # Passing-stage evidence covers actual runtime activation and
            # removed work over the complete corpus, not just compile markers.
            full_cases=R.OUT/'passing_profile_cases.json'
            P.save(full_cases,[{'algorithm':a,'graph':row['graph']} for a in ALGS for row in R.M.ROWS])
            D.run(R,cpus,phase=True,case_path=str(full_cases))
        if args.stage=='all':
            previous=result['decision'];result=report(R,results)
            if previous in ['accepted','pending_evidence'] and result['decision']=='unaccepted':
                D.run(R,cpus,phase=False,case_path=args.cases)
        R.evidence_checkpoint('complete')
        R.progress(stage='complete',decision=result['decision'],completed=244,total=244,failures=0)

if __name__=='__main__':main()
