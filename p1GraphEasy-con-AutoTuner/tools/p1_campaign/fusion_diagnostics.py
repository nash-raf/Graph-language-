"""Kaggle-only component/resource/phase diagnostics. Never acceptance samples."""
from pathlib import Path
import collections,difflib,json,math,os,re,shutil,signal,statistics,subprocess,time
import portable_campaign as P
PHASE_NAMES=['records','compact','full_begin','source_map','owner_execution','native_ordinal_index','descriptors','journal_production','journal_merge','source_coverage_including_fused_map','journal_reservation','cleancut_topology_materialization']
WORK_NAMES=['record_preparations','logical_pair_occurrences','stored_records_or_descriptors','record_or_descriptor_bytes','frontier_occurrences','cold_native_metadata_bytes','successful_journal_appends','full_source_rounds','full_source_domain_vertices','separate_map_loop_vertices','dense_append_slot_domain','journal_reserved_slots','journal_entries_merged','journal_bitmap_words_scanned','journal_bitmap_bytes_reserved','proven_append_entries','proven_previous_key_reads_removed']

PRELUDE=r'''
#include <time.h>
static _Atomic unsigned long long fusion_probe_ns[12],fusion_probe_calls[12];
static _Atomic unsigned long long fusion_work[17];
static __attribute__((no_instrument_function)) unsigned long long fusion_probe_clock(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned long long)t.tv_sec*1000000000ull+t.tv_nsec;}
static __attribute__((no_instrument_function)) void fusion_probe_add(int k,unsigned long long t){atomic_fetch_add(&fusion_probe_ns[k],fusion_probe_clock()-t);atomic_fetch_add(&fusion_probe_calls[k],1);}
/* GCC instruments only these existing batch functions. No protected source edit,
 * public context change, per-vertex timer or additional user callback. */
static void sgpl_exec_source_coverage(sgpl_exec_ctx *,int);
static void sgpl_exec_partition_body(int64_t,void *);
static void sgpl_exec_partition_body_packed(int64_t,void *);
static void sgpl_exec_partition_body_lean(int64_t,void *);
static void sgpl_exec_partition_body_lean_weighted(int64_t,void *);
static _Thread_local unsigned fusion_coverage_depth;
static _Thread_local unsigned long long fusion_coverage_starts[32];
static __attribute__((no_instrument_function)) int fusion_batch_kind(void *fn){if(fn==(void *)sgpl_exec_source_coverage)return 9;if(fn==(void *)autograph_build_clean_cut)return 11;if(fn==(void *)sgpl_exec_partition_body||fn==(void *)sgpl_exec_partition_body_packed||fn==(void *)sgpl_exec_partition_body_lean||fn==(void *)sgpl_exec_partition_body_lean_weighted)return 4;return -1;}
void __cyg_profile_func_enter(void *,void *) __attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *,void *) __attribute__((no_instrument_function));
void __cyg_profile_func_enter(void *fn,void *site){(void)site;if(fusion_batch_kind(fn)>=0){unsigned n=fusion_coverage_depth++;if(n<32)fusion_coverage_starts[n]=fusion_probe_clock();}}
void __cyg_profile_func_exit(void *fn,void *site){(void)site;int kind=fusion_batch_kind(fn);if(kind>=0&&fusion_coverage_depth){unsigned n=--fusion_coverage_depth;if(n<32)fusion_probe_add(kind,fusion_coverage_starts[n]);}}
static void fusion_probe_records(const sgpl_exec_ctx *c){int bytes=c->array_compact_records==6?16:(c->array_compact_records==4?16:(c->array_compact_records==3?12:(c->array_compact_records==2?20:(c->array_compact_records?16:28))));if(c->array_record_edges&&c->array_compact_records!=6)bytes+=8;atomic_fetch_add(&fusion_work[0],1);atomic_fetch_add(&fusion_work[1],c->array_work_count);atomic_fetch_add(&fusion_work[2],c->gpu_record_count);atomic_fetch_add(&fusion_work[3],c->gpu_record_count*bytes);atomic_fetch_add(&fusion_work[4],c->array_source_count);}
__attribute__((destructor)) static void fusion_probe_save(void){const char *p=getenv("SGPL_FUSION_PHASE_OUTPUT");if(!p)return;FILE *f=fopen(p,"w");if(!f)return;fputs("{\"instrumented\":true,\"phases\":[",f);for(int k=0;k<12;++k)fprintf(f,"%s{\"nanoseconds\":%llu,\"calls\":%llu}",k?",":"",atomic_load(&fusion_probe_ns[k]),atomic_load(&fusion_probe_calls[k]));fputs("],\"work\":[",f);for(unsigned k=0;k<sizeof fusion_work/sizeof fusion_work[0];++k)fprintf(f,"%s%llu",k?",":"",atomic_load(&fusion_work[k]));fputs("]}",f);fclose(f);}
'''
RESOURCE=r'''
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
static double clock_now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(int argc,char **argv){if(argc<3)return 64;double t=clock_now();pid_t p=fork();if(p<0)return 71;if(!p){execvp(argv[2],argv+2);_exit(127);}struct rusage r;int status;while(wait4(p,&status,0,&r)<0)if(errno!=EINTR)return 71;FILE *f=fopen(argv[1],"w");if(!f)return 74;fprintf(f,"{\"instrumented\":true,\"seconds\":%.17g,\"user_seconds\":%.17g,\"system_seconds\":%.17g,\"voluntary_switches\":%ld,\"involuntary_switches\":%ld,\"max_rss_kib\":%ld}\n",clock_now()-t,r.ru_utime.tv_sec+r.ru_utime.tv_usec*1e-6,r.ru_stime.tv_sec+r.ru_stime.tv_usec*1e-6,r.ru_nvcsw,r.ru_nivcsw,r.ru_maxrss);fclose(f);return WIFEXITED(status)?WEXITSTATUS(status):128+WTERMSIG(status);}
'''
POOL_PROBE=r'''
#include <stdatomic.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include "parallel_runtime.h"
#include "autotuner_runtime.h"
static _Atomic unsigned long long pool_calls,worker_calls,worker_cpu_ns,worker_wall_ns,wait_calls,park_returns,wake_calls,wait_ns,append_calls,append_successes,append_ns;
static unsigned long long probe_ns(clockid_t id){struct timespec t;clock_gettime(id,&t);return (unsigned long long)t.tv_sec*1000000000ull+t.tv_nsec;}
/* Only parallel_runtime.c's two known, six-argument futex calls are redirected.
 * Other syscall users retain libc's untouched variadic entry point. */
long sgpl_profile_pool_syscall(long n,...){va_list a;va_start(a,n);unsigned *word=va_arg(a,unsigned *);int op=va_arg(a,int);unsigned value=va_arg(a,unsigned);void *timeout=va_arg(a,void *),*other=va_arg(a,void *);int third=va_arg(a,int);va_end(a);unsigned long long begin=probe_ns(CLOCK_MONOTONIC);long r=syscall(n,word,op,value,timeout,other,third);int saved=errno;if(n==SYS_futex){if((op&FUTEX_CMD_MASK)==FUTEX_WAIT){atomic_fetch_add(&wait_calls,1);atomic_fetch_add(&wait_ns,probe_ns(CLOCK_MONOTONIC)-begin);if(r==0)atomic_fetch_add(&park_returns,1);}else if((op&FUTEX_CMD_MASK)==FUTEX_WAKE)atomic_fetch_add(&wake_calls,1);}errno=saved;return r;}
typedef struct {sgpl_pool_work_fn fn;void *state;} ProbeJob;
extern int32_t __real_sgpl_run_on_worker_pool(sgpl_pool_work_fn,void *,int32_t);
static void probe_work(void *opaque,int32_t worker){ProbeJob *j=opaque;unsigned long long wall=probe_ns(CLOCK_MONOTONIC),cpu=probe_ns(CLOCK_THREAD_CPUTIME_ID);j->fn(j->state,worker);atomic_fetch_add(&worker_calls,1);atomic_fetch_add(&worker_cpu_ns,probe_ns(CLOCK_THREAD_CPUTIME_ID)-cpu);atomic_fetch_add(&worker_wall_ns,probe_ns(CLOCK_MONOTONIC)-wall);}
int32_t __wrap_sgpl_run_on_worker_pool(sgpl_pool_work_fn fn,void *state,int32_t width){ProbeJob j={fn,state};atomic_fetch_add(&pool_calls,1);return __real_sgpl_run_on_worker_pool(probe_work,&j,width);}
extern int32_t __real_autograph_frontier_activate(sgpl_exec_ctx *,int32_t);
int32_t __wrap_autograph_frontier_activate(sgpl_exec_ctx *c,int32_t v){unsigned long long t=probe_ns(CLOCK_MONOTONIC);int32_t r=__real_autograph_frontier_activate(c,v);atomic_fetch_add(&append_calls,1);if(r==1)atomic_fetch_add(&append_successes,1);atomic_fetch_add(&append_ns,probe_ns(CLOCK_MONOTONIC)-t);return r;}
__attribute__((destructor(101))) static void probe_save(void){const char *p=getenv("SGPL_FUSION_POOL_OUTPUT");if(!p)return;FILE *f=fopen(p,"w");if(!f)return;fprintf(f,"{\"instrumented\":true,\"pool_calls\":%llu,\"worker_calls\":%llu,\"worker_cpu_ns\":%llu,\"worker_wall_ns\":%llu,\"futex_wait_attempts\":%llu,\"successful_wait_returns\":%llu,\"futex_wakes\":%llu,\"completed_futex_wait_ns\":%llu,\"dense_append_calls\":%llu,\"dense_append_successes\":%llu,\"dense_append_ns\":%llu}",atomic_load(&pool_calls),atomic_load(&worker_calls),atomic_load(&worker_cpu_ns),atomic_load(&worker_wall_ns),atomic_load(&wait_calls),atomic_load(&park_returns),atomic_load(&wake_calls),atomic_load(&wait_ns),atomic_load(&append_calls),atomic_load(&append_successes),atomic_load(&append_ns));fclose(f);}
'''

def cases(R,path=None):
    if path:return R.read(path)
    report=R.read(R.OUT/'comparison.json');assert report,'Generate the full comparison before diagnosis'
    rows=report['cases'];last=rows[-1]['phase'];rows=[x for x in rows if x['phase']==last]
    losses=sorted(rows,key=lambda x:x['mean_ms']['candidate']/x['mean_ms']['accepted'],reverse=True)
    # Retain every case with a small corrected slowdown p-value, including
    # suspect observations from an invalid A/A block; diagnosis does not turn
    # those observations into confirmed regressions. Also cover each algorithm
    # and include improving controls rather than truncating at ten losses.
    suspected=[x for x in rows if x['tests']['code_regressions']['holm_p']<.05]
    by_algorithm=[next(x for x in losses if x['algorithm']==a) for a in P.ALGS]
    chosen=suspected+losses[:10]+by_algorithm+losses[-2:]
    return list({(x['algorithm'],x['graph']):{'algorithm':x['algorithm'],'graph':x['graph']} for x in chosen}.values())

def build_phase_runtime(R,version):
    original=P.WORK/'versions'/version/'src';dst=P.WORK/'profiling'/version
    if not dst.exists():shutil.copytree(original,dst)
    # Only execution-specific includes are instrumented. The protected ARS
    # translation unit, headers and cost-model code remain byte-identical.
    marker=dst/'probe_identity.json'
    if not marker.exists():
        f=dst/'execution_scratch_runtime.inc';f.write_text(PRELUDE+f.read_text())
        f=dst/'array_frontier_runtime.inc';text=f.read_text()
        for name in ['sgpl_array_records','sgpl_array_compact']:
            text=text.replace(name+'(',name+'_profile_impl(',1)
        text=text+r'''
static int32_t sgpl_array_records(sgpl_exec_ctx *c,AutoGraphMeta *m,int b,int e){unsigned long long t=fusion_probe_clock();int32_t r=sgpl_array_records_profile_impl(c,m,b,e);fusion_probe_add(0,t);if(!r)fusion_probe_records(c);return r;}
static int32_t sgpl_array_compact(sgpl_exec_ctx *c){unsigned long long t=fusion_probe_clock();atomic_fetch_add(&fusion_work[10],c->array_work_count*c->array_append_sites);int32_t r=sgpl_array_compact_profile_impl(c);fusion_probe_add(1,t);return r;}
''';f.write_text(text)
        f=dst/'fullgraph_runtime.inc';text=f.read_text()
        for name in ['sgpl_fullgraph_begin','sgpl_fullgraph_prepare_map','sgpl_fullgraph_partition']:
            text=text.replace(name+'(',name+'_profile_impl(',1)
        text+=r'''
static int32_t sgpl_fullgraph_begin(sgpl_exec_ctx *c,AutoGraphMeta *m){unsigned long long t=fusion_probe_clock();int32_t r=sgpl_fullgraph_begin_profile_impl(c,m);fusion_probe_add(2,t);return r;}
static void sgpl_fullgraph_prepare_map(sgpl_exec_ctx *c){unsigned long long t=fusion_probe_clock();
 SgplFullState *s=sgpl_fullgraph_state(c);
 if(s&&s->index&&!s->ready&&s->map&&s->index->n>0){
   unsigned long long n=(unsigned long long)s->index->n;int fused=0;
#ifdef SGPL_EFFECT_FUSION_PLAN_H
   fused=(s->fusion_active&SGPL_FUSION_SOURCE_MAP)!=0;
#endif
   atomic_fetch_add(&fusion_work[7],1);atomic_fetch_add(&fusion_work[8],n);
   if(!fused)atomic_fetch_add(&fusion_work[9],n);
 }
 sgpl_fullgraph_prepare_map_profile_impl(c);fusion_probe_add(3,t);}
static int sgpl_fullgraph_partition(sgpl_exec_ctx *c,int32_t p){unsigned long long t=fusion_probe_clock();int r=sgpl_fullgraph_partition_profile_impl(c,p);fusion_probe_add(4,t);return r;}
''';f.write_text(text)
        for name in ['AutoTunerPass.cpp','AutoTunerPass.h','autotuner_runtime.c','autotuner_runtime.h']:
            assert P.sha(dst/name)==P.sha(original/name),name
        # Optional private executors are timed at their own construction and
        # merge boundaries. Forward declarations keep each original call bound
        # to the wrapper, without editing a protected translation unit.
        optional=[('frontier_fusion_index.inc','static SGPL_PREPARATION_FN SgplFrontierOrdinalIndex *sgpl_frontier_index_build(',
            'static SgplFrontierOrdinalIndex *sgpl_frontier_index_build(AutoGraphMeta *,int);',
            'static SgplFrontierOrdinalIndex *sgpl_frontier_index_build(AutoGraphMeta *m,int w){unsigned long long t=fusion_probe_clock();SgplFrontierOrdinalIndex *r=sgpl_frontier_index_build_profile_impl(m,w);fusion_probe_add(5,t);if(r)atomic_fetch_add(&fusion_work[5],sgpl_frontier_index_bytes(m,w));return r;}'),
          ('frontier_fusion_prepare.inc','static SGPL_PREPARATION_FN SgplFrontierDirect *sgpl_frontier_descriptors(',
            'static SgplFrontierDirect *sgpl_frontier_descriptors(sgpl_exec_ctx *,AutoGraphMeta *,const sgpl_frontier_fusion_plan_v1 *,uint32_t,int32_t *);',
            'static SgplFrontierDirect *sgpl_frontier_descriptors(sgpl_exec_ctx *c,AutoGraphMeta *m,const sgpl_frontier_fusion_plan_v1 *p,uint32_t slot,int32_t *e){unsigned long long t=fusion_probe_clock();SgplFrontierDirect *r=sgpl_frontier_descriptors_profile_impl(c,m,p,slot,e);fusion_probe_add(6,t);return r;}'),
          ('append_journal_runtime.inc','SGPL_JOURNAL_FN int32_t sgpl_frontier_journal_activate_v1(',
            'int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *,int32_t);',
            'int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *c,int32_t v){unsigned long long t=fusion_probe_clock();int32_t r=sgpl_frontier_journal_activate_v1_profile_impl(c,v);fusion_probe_add(7,t);if(r==1)atomic_fetch_add(&fusion_work[6],1);return r;}'),
          ('append_journal_runtime.inc','static SGPL_JOURNAL_FN void sgpl_journal_begin(',
            'static void sgpl_journal_begin(sgpl_exec_ctx *);',
            'static void sgpl_journal_begin(sgpl_exec_ctx *c){unsigned long long t=fusion_probe_clock();sgpl_journal_begin_profile_impl(c);fusion_probe_add(10,t);}'),
          ('append_journal_runtime.inc','static SGPL_JOURNAL_FN int sgpl_journal_compact(',
            'static int sgpl_journal_compact(sgpl_exec_ctx *,int32_t *);',
            r'''static int sgpl_journal_compact(sgpl_exec_ctx *c,int32_t *r){
 unsigned long long t=fusion_probe_clock();int n=sgpl_journal_compact_profile_impl(c,r);fusion_probe_add(8,t);
 if(n){unsigned long long slots=c->array_work_count*c->array_append_sites;
  atomic_fetch_sub(&fusion_work[10],slots);atomic_fetch_add(&fusion_work[11],slots);
  SgplExecutionScratch *scratch=c->execution_scratch;
  SgplAppendJournal *s=scratch&&scratch->fusion?scratch->fusion->journal_state:NULL;
  if(!*r&&s){unsigned long long entries=0,nonempty=0;
   for(int i=0;i<s->parts;++i){entries+=s->owners[i].used;nonempty+=s->owners[i].used!=0;}
   atomic_fetch_add(&fusion_work[12],entries);
#ifdef SGPL_JOURNAL_BITMAP_PROTOCOL
   atomic_fetch_add(&fusion_work[14],s->words*sizeof(*s->bitmap));
   if(entries&&!s->heap_merge)atomic_fetch_add(&fusion_work[13],s->words);
#endif
#ifdef SGPL_JOURNAL_PROVEN_APPEND
   if(s->proven_append){atomic_fetch_add(&fusion_work[15],entries);atomic_fetch_add(&fusion_work[16],entries-nonempty);}
#endif
  }
 }return n;}''')]
        for name,signature,declaration,wrapper in optional:
            f=dst/name
            if not f.exists():continue
            text=f.read_text();assert text.count(signature)==1,(name,signature)
            replacement=signature[:-1]+'_profile_impl('
            f.write_text(text.replace(signature,declaration+'\n'+replacement,1)+'\n'+wrapper+'\n')
        P.save(marker,{'baseline_source':str(original),'phases':PHASE_NAMES,'work':WORK_NAMES,
            'includes':{x:P.sha(dst/x) for x in ['execution_scratch_runtime.inc','array_frontier_runtime.inc','fullgraph_runtime.inc']}})
    obj=dst/'autotuner_profile.o'
    if not obj.exists():
        dump=dst/'function_inventory.tree'
        R.checked(['gcc','-O0','-fopenmp','-pthread','-iquote','.',
            '-fdump-tree-original='+str(dump),'-S','autotuner_runtime.c','-o','/dev/null'],dst,dst/'function_inventory.log')
        names=set(re.findall(r'^;; Function ([A-Za-z_][A-Za-z_0-9]*)',dump.read_text(),re.M))
        targets=['sgpl_exec_source_coverage','autograph_build_clean_cut','sgpl_exec_partition_body','sgpl_exec_partition_body_packed','sgpl_exec_partition_body_lean','sgpl_exec_partition_body_lean_weighted']
        assert set(targets)<=names,('Missing batch function inventory',set(targets)-names)
        excluded=sorted(n for n in names if not any(n in target for target in targets))
        P.save(dst/'function_instrumentation.json',{'targets':targets,'excluded':excluded,'scope':'diagnostic-only runtime object'})
        R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.',
            '-finstrument-functions','-finstrument-functions-exclude-function-list='+','.join(excluded),
            '-c','autotuner_runtime.c','-o',obj],dst,dst/'build.log')
    evidence=R.OUT/'profiling_builds'/version;evidence.mkdir(parents=True,exist_ok=True)
    shutil.copy2(marker,evidence/marker.name)
    if (dst/'function_instrumentation.json').exists():shutil.copy2(dst/'function_instrumentation.json',evidence/'function_instrumentation.json')
    patch=[];identities={}
    for file in dst.glob('*.inc'):
        if not (original/file.name).exists() or P.sha(file)==P.sha(original/file.name):continue
        identities[file.name]=P.sha(file)
        patch.extend(difflib.unified_diff((original/file.name).read_text().splitlines(True),file.read_text().splitlines(True),
            fromfile='original/'+file.name,tofile='instrumented/'+file.name))
    (evidence/'private_instrumentation.patch').write_text(''.join(patch))
    P.save(evidence/'runtime_identity.json',{'instrumented_runtime_object_sha256':P.sha(obj),'private_includes':identities,
        'protected_translation_unit_sha256':P.sha(dst/'autotuner_runtime.c')})
    return obj

def build_pool_probe(R,version):
    src=P.WORK/'versions'/version/'src';dst=P.WORK/'profiling'/version;dst.mkdir(parents=True,exist_ok=True)
    helper=dst/'pool_probe.o';source=dst/'pool_probe.c';source.write_text(POOL_PROBE)
    # Reuse one identical protected-source object across both versions. Its
    # compile timestamp participates in the existing launch-cache key.
    parallel=R.compile_cache(src/'parallel_runtime.c',src,['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.',
        '-Dsyscall=sgpl_profile_pool_syscall'],dst/'parallel_probe.build.log')
    if not helper.exists():R.checked(['gcc','-O2','-std=gnu11','-pthread','-I',src,'-c',source,'-o',helper],dst,dst/'pool_probe.build.log')
    P.save(dst/'pool_probe_identity.json',{'protected_parallel_source_sha256':P.sha(src/'parallel_runtime.c'),
        'probe_source_sha256':P.sha(source),'parallel_object_sha256':P.sha(parallel),'helper_object_sha256':P.sha(helper),
        'note':'Source bytes unchanged; diagnostic-only syscall redirection and pool wrapper. Completed waits include idle waits and are not a wall-time decomposition.'})
    evidence=R.OUT/'profiling_builds'/version;evidence.mkdir(parents=True,exist_ok=True)
    shutil.copy2(dst/'pool_probe_identity.json',evidence/'pool_probe_identity.json');shutil.copy2(source,evidence/'pool_probe.c')
    return parallel,helper

def bounded_traces(R,cpus,selected):
    """Probe permissions first; keep unavailable counters explicit."""
    d=R.OUT/'bounded_traces';d.mkdir(exist_ok=True);capabilities={}
    R.progress(stage='profile_tool_setup',completed=0,total=1)
    # Optional tools are installed only on the guarded Kaggle host, after
    # acceptance timing. Installation failures are evidence, not a reason to
    # run locally or to discard completed timing blocks.
    assert Path('/kaggle/working').is_dir() and Path('/kaggle/input').is_dir()
    installed=[]
    for package,needed in [('strace',not shutil.which('strace')),('linux-tools-generic',not list(Path('/usr/lib/linux-tools').glob('*/perf')))]:
        if not needed:continue
        try:
            result=subprocess.run(['apt-get','install','-y','-qq',package],capture_output=True,text=True,timeout=180)
            (d/(package+'.install.log')).write_text(result.stdout+result.stderr)
            installed.append({'package':package,'returncode':result.returncode})
        except subprocess.TimeoutExpired as error:
            (d/(package+'.install.log')).write_text(str(error));installed.append({'package':package,'timeout':True})
    perf=next((str(p) for p in sorted(Path('/usr/lib/linux-tools').glob('*/perf')) if p.is_file()),shutil.which('perf'));trace=shutil.which('strace')
    capabilities['optional_installations']=installed
    capabilities['perf_path']=perf;capabilities['strace_path']=trace
    capabilities['tool_sha256']={name:P.sha(path) for name,path in [('perf',perf),('strace',trace)] if path}
    if perf:
        probe=subprocess.run([perf,'stat','-e','cycles,instructions,cache-misses','--','true'],capture_output=True,text=True,timeout=10)
        (d/'perf_probe.log').write_text(probe.stdout+probe.stderr)
        capabilities['hardware_counters_available']=probe.returncode==0
    else:capabilities['hardware_counters_available']=False
    capabilities['scheduler_trace_available']=False
    if perf:
        probe=subprocess.run([perf,'record','-o',str(d/'scheduler_probe.data'),'-e','sched:sched_switch','-e','sched:sched_wakeup','--','true'],capture_output=True,text=True,timeout=10)
        (d/'scheduler_probe.log').write_text(probe.stdout+probe.stderr)
        capabilities['scheduler_trace_available']=probe.returncode==0
    # These diagnostics never enter the timed observation list. Each child
    # has a strict bound; a denied perf facility is not retried per graph.
    algorithm=None;started=None;completed=0
    def signature(a):return {'cases':[x['graph'] for x in selected if x['algorithm']==a],
        'workers':len(cpus),'per_child_bound_seconds':8,'tools':capabilities['tool_sha256'],
        'hardware':capabilities['hardware_counters_available'],'scheduler':capabilities['scheduler_trace_available']}
    for item in selected:
        row=next(x for x in R.M.ROWS if x['graph']==item['graph']);a=item['algorithm']
        if a!=algorithm:
            if algorithm is not None:R.calibrate(algorithm,'fusion_bounded_traces',signature(algorithm),started)
            algorithm=a;started=time.time();completed=0
            previous=R.read(R.OUT/'calibration.json',{}).get(R.key([a,'fusion_bounded_traces',signature(a)]))
            R.state(stage='profile_traces',algorithm=a,block_started=started,
                monitor_interval_seconds=previous['seconds'] if previous else None,
                expected_completion_unix=started+previous['seconds'] if previous else None)
        for version in ['Accepted','StaticSchedule']:
            tag=R.key([version,a,row['graph']])[:16]
            command,env,cwd=R.binary_cmd(version,a,row,False,len(cpus),d/(tag+'.values'))
            os.sched_setaffinity(0,cpus)
            choices=[]
            if capabilities['hardware_counters_available']:
                choices.append(('hardware',[perf,'stat','-x,','-e','cycles,instructions,branches,branch-misses,cache-references,cache-misses','-o',str(d/(tag+'.perf.csv')),'--',*map(str,command)]))
            if trace:choices.append(('futex',[trace,'-f','-c','-e','futex','-o',str(d/(tag+'.futex.txt')),*map(str,command)]))
            if capabilities['scheduler_trace_available']:
                choices.append(('scheduler',[perf,'record','-o',str(d/(tag+'.sched.data')),'-e','sched:sched_switch','-e','sched:sched_wakeup','--',*map(str,command)]))
            for kind,cmd in choices:
                p=subprocess.Popen(cmd,cwd=cwd,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True)
                try:
                    stdout,stderr=p.communicate(timeout=8)
                    (d/(tag+'.'+kind+'.log')).write_text(stdout+stderr)
                    capabilities[tag+'_'+kind]={'returncode':p.returncode,'bounded_seconds':8}
                except subprocess.TimeoutExpired:
                    os.killpg(p.pid,signal.SIGKILL);stdout,stderr=p.communicate()
                    (d/(tag+'.'+kind+'.log')).write_text(stdout+stderr)
                    capabilities[tag+'_'+kind]={'timeout':True,'bounded_seconds':8}
        completed+=1;R.progress(stage='profile_traces',algorithm=a,completed=completed,
            total=sum(x['algorithm']==a for x in selected),elapsed_seconds=time.time()-started)
    if algorithm is not None:R.calibrate(algorithm,'fusion_bounded_traces',signature(algorithm),started)
    capabilities['scheduler_trace']='per-process tracepoints, inherited by helpers, independently permission-probed; availability alone does not establish a regression cause'
    P.save(d/'capabilities.json',capabilities)
    return capabilities

def crosslinks(R,cpus,selected):
    """Exchange only the execution runtime when ordinary linking permits it.
    Unresolved new helpers make a control unavailable, never a stubbed run."""
    controls={};evidence=[];original=R.binary_cmd
    for item in selected:
        a=item['algorithm'];row=next(x for x in R.M.ROWS if x['graph']==item['graph'])
        for program,runtime,label in [('Accepted','StaticSchedule','baseline_program_candidate_runtime'),
                ('StaticSchedule','Accepted','candidate_program_baseline_runtime')]:
            case=R.case(program,a,row);destination=case/label;available=True
            for mode in ['validate','timed']:
                exe=destination.with_name(destination.name+'_'+mode)
                vd=P.WORK/'versions'/program;rd=P.WORK/'versions'/runtime
                cmd=['g++','-O3','-fopenmp','-no-pie',case/(mode+'.o'),
                    *[(rd if x=='autotuner_runtime' else vd)/(x+'.o') for x in R.OBJECTS],
                    P.WORK/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe]
                log=destination.with_name(destination.name+'_'+mode+'.link.log')
                if not exe.exists():
                    with log.open('w') as f:p=subprocess.run(list(map(str,cmd)),cwd=case,stdout=f,stderr=subprocess.STDOUT,timeout=120)
                    if p.returncode:
                        evidence.append(dict(item,label=label,available=False,reason='Internal helper/ABI link control unavailable',log=str(log)));available=False;break
            if not available:continue
            validate=destination.with_name(destination.name+'_validate')
            def command(v,aa,rr,validation,w,out):
                cmd,env,cwd=original(v,aa,rr,validation,w,out);cmd[0]=str(validate);return cmd,env,cwd
            R.binary_cmd=command
            try:x=R.invoke('fusion_crosslink','validate_crosslink',program,a,row,0,label=label,workers=len(cpus))
            finally:R.binary_cmd=original
            if x.get('error') or not x.get('validation'):
                evidence.append(dict(item,label=label,available=False,reason='Cross-link graph result validation failed',record=x));continue
            exe=destination.with_name(destination.name+'_timed');controls[(a,row['graph'],label)]=exe
            evidence.append(dict(item,label=label,available=True,executable=str(exe),sha256=P.sha(exe),validation=x))
    P.save(R.OUT/'crosslink_controls.json',evidence);return controls

def source_codegen_control(R,cpus,selected):
    """Isolate the source-wrapper mechanism without changing the accumulator.
    This is a retained diagnostic component, never an accepted revision."""
    provenance=R.read(P.BUNDLE/'provenance.json');controls={}
    if provenance['attempt'][0].upper()!='A':return controls
    src=P.WORK/'component_source_codegen/src';original=P.WORK/'versions/StaticSchedule/src'
    target='    if(Enable&&fusionGateEligible(Gate))'
    if target not in (original/'effect_fusion_lowering.inc').read_text():return controls
    if not src.exists():
        shutil.copytree(original,src)
        f=src/'effect_fusion_lowering.inc';text=f.read_text();assert text.count(target)==1
        f.write_text(text.replace(target,'    if(false&&Enable&&fusionGateEligible(Gate))',1))
        patch=''.join(difflib.unified_diff(text.splitlines(True),f.read_text().splitlines(True),
            fromfile='candidate/effect_fusion_lowering.inc',tofile='component/effect_fusion_lowering.inc'))
        (R.OUT/'source_codegen_component.patch').write_text(patch)
    for name in provenance['protected_ars']:assert P.sha(src/name)==P.sha(original/name),name
    flags=['g++','-O2','-mavx2','-march=native','-std=c++17','-fexceptions','-pthread','-I/usr/local/include/antlr4-runtime','-Igenerated','-iquote','.',
        '-I'+R.cfg('--includedir')[0]+'/polly',*[x for x in R.cfg('--cxxflags') if x!='-fno-exceptions']]
    sources=['main.cpp','IRGenVisitor.cpp','MotifPattern.cpp','MotifIRBuilder.cpp','ASTBuilder.cpp','pdg.cpp','parallel_loop_outline.cpp','graph_frontier_lowering.cpp','SemanticAnalyzer.cpp','roaring_bitmap.cpp','AutoTunerPass.cpp']+[str(p.relative_to(src)) for p in (src/'generated').glob('*.cpp')]
    compiler=src.parent/'GraphProgram'
    if not compiler.exists():
        objects=[]
        for file in sources:objects.append(R.compile_cache(src/file,src,flags,src.parent/(file.replace('/','_')+'.log')))
        R.checked(['g++',*objects,*R.cfg('--ldflags'),'-Wl,--no-keep-memory','-pthread','-lantlr4-runtime',*R.cfg('--libs','all'),*R.cfg('--system-libs'),'-o',compiler],src,src.parent/'compiler_link.log')
    evidence=[];original_cmd=R.binary_cmd;label='source_map_codegen_off'
    for item in selected:
        if item['algorithm']!='pagerank':continue
        a=item['algorithm'];row=next(x for x in R.M.ROWS if x['graph']==item['graph']);case=R.case('StaticSchedule',a,row)
        work=src.parent/'cases'/a/row['graph'].removesuffix('.edges');work.mkdir(parents=True,exist_ok=True)
        for f in src.glob('*.ll'):
            if not (work/f.name).exists():(work/f.name).symlink_to(f)
        vd=P.WORK/'versions/StaticSchedule'
        for mode in ['validate','timed']:
            exe=work/mode
            if exe.exists():continue
            source=case/(mode+'.graph');env=dict(R.M.env(workers=len(cpus)),FORCE_CPU='1',SGPL_FUSION_STATS='1')
            R.checked([compiler,'--ir-backend=cpu',source],work,work/(mode+'.compile.log'),env,1200)
            (work/'program.o').rename(work/(mode+'.o'))
            R.checked(['g++','-O3','-fopenmp','-no-pie',work/(mode+'.o'),*[vd/(x+'.o') for x in R.OBJECTS],
                P.WORK/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],work,work/(mode+'.link.log'))
        for w in sorted(set([1,min(2,len(cpus)),len(cpus)])):
            def command(v,aa,rr,validation,workers,out):
                cmd,env,cwd=original_cmd(v,aa,rr,validation,workers,out);cmd[0]=str(work/'validate');return cmd,env,cwd
            R.binary_cmd=command
            try:x=R.invoke('fusion_source_component','validate_source_component','StaticSchedule',a,row,0,label=label,workers=w)
            finally:R.binary_cmd=original_cmd
            assert x.get('validation') and not x.get('error'),x
        controls[(a,row['graph'],label)]=work/'timed'
        evidence.append(dict(item,label=label,executable=str(work/'timed'),object=str(work/'timed.o'),sha256=P.sha(work/'timed'),compiler_sha256=P.sha(compiler)))
    P.save(R.OUT/'source_codegen_components.json',evidence);return controls

def lowering_controls(R,cpus,selected):
    """Recompile only the current stage off using the identical compiler.
    Unlike runtime-disable controls, this also removes bind calls and unused
    specialized kernels. It does not modify any compiler or runtime source.
    """
    stage=R.read(P.BUNDLE/'provenance.json')['attempt'][0].upper()
    if stage not in ['B','C']:return {}
    vd=P.WORK/'versions/StaticSchedule';src=vd/'src';compiler=vd/'GraphProgram'
    controls={};evidence=[];original=R.binary_cmd;label='stage_codegen_off'
    for item in selected:
        a=item['algorithm'];row=next(x for x in R.M.ROWS if x['graph']==item['graph'])
        case=R.case('StaticSchedule',a,row);work=P.WORK/'lowering_controls'/a/row['graph'].removesuffix('.edges')
        work.mkdir(parents=True,exist_ok=True)
        for f in src.glob('*.ll'):
            if not (work/f.name).exists():(work/f.name).symlink_to(f)
        env=dict(R.M.env(workers=len(cpus)),FORCE_CPU='1',SGPL_FUSION_STATS='1')
        env['SGPL_NO_FUSION_'+stage+'_LOWERING']='1'
        for mode in ['validate','timed']:
            if (work/mode).exists():continue
            R.checked([compiler,'--ir-backend=cpu',case/(mode+'.graph')],work,work/(mode+'.compile.log'),env,1200)
            (work/'program.o').rename(work/(mode+'.o'))
            R.checked(['g++','-O3','-fopenmp','-no-pie',work/(mode+'.o'),*[vd/(x+'.o') for x in R.OBJECTS],
                P.WORK/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',work/mode],work,work/(mode+'.link.log'))
        for width in sorted(set([1,min(2,len(cpus)),len(cpus)])):
            def command(v,aa,rr,validation,w,out):
                cmd,e,cwd=original(v,aa,rr,validation,w,out);cmd[0]=str(work/'validate');return cmd,e,cwd
            R.binary_cmd=command
            try:x=R.invoke('fusion_lowering_component','validate_lowering_component','StaticSchedule',a,row,0,label=label,workers=width)
            finally:R.binary_cmd=original
            assert x.get('validation') and not x.get('error'),x
        controls[(a,row['graph'],label)]=work/'timed'
        evidence.append(dict(item,label=label,stage=stage,compiler_sha256=P.sha(compiler),
            executable_sha256=P.sha(work/'timed'),object_sha256=P.sha(work/'timed.o'),
            unchanged_runtime_objects={x:P.sha(vd/(x+'.o')) for x in R.OBJECTS}))
    P.save(R.OUT/'lowering_components.json',evidence);return controls

def gate_layout_components(R,cpus,selected):
    """One scoped function-placement control; protected source is unchanged."""
    original=P.WORK/'versions/StaticSchedule/src'
    if not (original/'execution_code_layout.inc').exists():return {}
    dst=P.WORK/'gate_layout_control';dst.mkdir(exist_ok=True)
    evidence=R.OUT/'runtime_components/gate_alignment_16';evidence.mkdir(parents=True,exist_ok=True)
    obj=dst/'autotuner_runtime.o'
    flags=['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.',
        '-DSGPL_SOURCE_GATE_ALIGNMENT=16','-c','autotuner_runtime.c','-o',obj]
    if not obj.exists():R.checked(flags,original,dst/'build.log')
    shutil.copy2(dst/'build.log',evidence/'build.log')
    for tool,args,name in [('objdump',['-drwC'],'runtime_disassembly.txt'),('nm',['-a','-S','--size-sort'],'runtime_symbols.txt')]:
        R.checked([tool,*args,obj],dst,evidence/name)
    P.save(evidence/'identity.json',{'object_sha256':P.sha(obj),'flags':list(map(str,flags)),
        'protected_translation_unit_sha256':P.sha(original/'autotuner_runtime.c'),
        'candidate_translation_unit_sha256':P.sha(P.BUNDLE/'candidate/autotuner_runtime.c'),
        'scope':'Same program and producer; only source-gate function alignment changes from 32 to 16. Checked variant changes only the producer control.'})
    controls={};vd=P.WORK/'versions/StaticSchedule';original_cmd=R.binary_cmd
    for item in selected:
        a=item['algorithm'];row=next(x for x in R.M.ROWS if x['graph']==item['graph']);case=R.case('StaticSchedule',a,row)
        work=dst/'cases'/a/row['graph'].removesuffix('.edges');work.mkdir(parents=True,exist_ok=True)
        for mode in ['validate','timed']:
            exe=work/mode
            if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',case/(mode+'.o'),
                *[obj if x=='autotuner_runtime' else vd/(x+'.o') for x in R.OBJECTS],
                P.WORK/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],work,work/(mode+'.link.log'))
        symbols=evidence/'linked_symbols'/(a+'_'+row['graph'].replace('/','_')+'.txt');symbols.parent.mkdir(exist_ok=True)
        R.checked(['nm','-a','-S','--size-sort',work/'timed'],work,symbols)
        for label,checked in [('gate_alignment_16',False),('gate_alignment_16_checked',True)]:
            for width in sorted(set([1,min(2,len(cpus)),len(cpus)])):
                def command(v,aa,rr,validation,w,out):
                    cmd,env,cwd=original_cmd(v,aa,rr,validation,w,out);cmd[0]=str(work/'validate')
                    if checked:env['SGPL_FUSION_C_CHECKED_APPEND']='1'
                    return cmd,env,cwd
                R.binary_cmd=command
                try:x=R.invoke('fusion_runtime_component','validate_runtime_component','StaticSchedule',a,row,0,label=label,workers=width,
                    extra={'SGPL_FUSION_C_CHECKED_APPEND':'1'} if checked else {})
                finally:R.binary_cmd=original_cmd
                assert x.get('validation') and not x.get('error'),x
            controls[(a,row['graph'],label)]=work/'timed'
    return controls

def runtime_components(R,cpus,selected):
    """One-mechanism private-runtime interventions, never promotion samples."""
    provenance=R.read(P.BUNDLE/'provenance.json');controls={};evidence=[]
    original=P.WORK/'versions/StaticSchedule/src';text=(original/'fullgraph_runtime.inc').read_text()
    allocation='SgplFullState *s=calloc(1,sizeof(*s));if(!s)return;'
    lazy='if(!s->source_view)s->source_view=malloc(sizeof(*s->source_view));'
    if allocation not in text or lazy not in text:return gate_layout_components(R,cpus,selected)
    match='if((void *)op->source_begin!=p->source_begin&&\n       !sgpl_fusion_gate_matches(c,op,p->source_begin))return 0;'
    for label,coallocate,legacy in [('view_coallocated',True,False),('legacy_static_match',False,True),('view_coallocated_legacy_match',True,True)]:
        dst=P.WORK/'runtime_components'/label;src=dst/'src'
        if not src.exists():
            shutil.copytree(original,src)
            if coallocate:
                f=src/'fullgraph_runtime.inc';s=f.read_text();assert s.count(allocation)==s.count(lazy)==1
                s=s.replace(allocation,allocation.replace('sizeof(*s)','sizeof(*s)+sizeof(sgpl_fusion_source_view_v2)')+'s->source_view=(sgpl_fusion_source_view_v2 *)(s+1);',1)
                s=s.replace(lazy,'/* source view reserved with the private state */',1).replace('free(s->source_view);','',1);f.write_text(s)
            if legacy:
                f=src/'static_schedule_runtime.inc';s=f.read_text();assert s.count(match)==1
                f.write_text(s.replace(match,'if((void *)op->source_begin!=p->source_begin)return 0;',1))
        for name in provenance['protected_ars']:assert P.sha(src/name)==P.sha(original/name),name
        obj=dst/'autotuner_runtime.o'
        if not obj.exists():R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',obj],src,dst/'build.log')
        for item in selected:
            a=item['algorithm'];row=next(x for x in R.M.ROWS if x['graph']==item['graph']);case=R.case('StaticSchedule',a,row)
            work=dst/'cases'/a/row['graph'].removesuffix('.edges');work.mkdir(parents=True,exist_ok=True)
            vd=P.WORK/'versions/StaticSchedule'
            for mode in ['validate','timed']:
                exe=work/mode
                if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',case/(mode+'.o'),
                    *[obj if x=='autotuner_runtime' else vd/(x+'.o') for x in R.OBJECTS],P.WORK/'monotonic_timer.o',
                    '-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],work,work/(mode+'.link.log'))
            original_cmd=R.binary_cmd
            for width in sorted(set([1,min(2,len(cpus)),len(cpus)])):
                def command(v,aa,rr,validation,w,out):
                    cmd,env,cwd=original_cmd(v,aa,rr,validation,w,out);cmd[0]=str(work/'validate');return cmd,env,cwd
                R.binary_cmd=command
                try:x=R.invoke('fusion_runtime_component','validate_runtime_component','StaticSchedule',a,row,0,label=label,workers=width)
                finally:R.binary_cmd=original_cmd
                assert x.get('validation') and not x.get('error'),x
            controls[(a,row['graph'],label)]=work/'timed'
        path=R.OUT/'runtime_components'/label;path.mkdir(parents=True,exist_ok=True);patch=[]
        for name in ['fullgraph_runtime.inc','static_schedule_runtime.inc']:
            patch.extend(difflib.unified_diff((original/name).read_text().splitlines(True),(src/name).read_text().splitlines(True),fromfile='candidate/'+name,tofile=label+'/'+name))
        (path/'component.patch').write_text(''.join(patch));shutil.copy2(dst/'build.log',path/'build.log')
        evidence.append({'label':label,'object_sha256':P.sha(obj),'same_program_objects':True,'coallocated_view':coallocate,'legacy_static_match':legacy})
        for tool,args,name in [('objdump',['-drwC'],'runtime_disassembly.txt'),('nm',['-a','-S','--size-sort'],'runtime_symbols.txt')]:
            R.checked([tool,*args,obj],dst,path/name)
    P.save(R.OUT/'runtime_components.json',evidence)
    return controls

def preserve_native_code(R,selected):
    """Keep code-placement evidence before the ephemeral host disappears."""
    for version in ['Accepted','StaticSchedule']:
        out=R.OUT/'native_code'/version;out.mkdir(parents=True,exist_ok=True)
        vd=P.WORK/'versions'/version;objects={'runtime':vd/'autotuner_runtime.o'}
        objects.update({item['algorithm']+'_'+item['graph'].replace('/','_'):R.case(version,item['algorithm'],next(r for r in R.M.ROWS if r['graph']==item['graph']))/'timed.o' for item in selected})
        identities={}
        for key,obj in objects.items():
            identities[key]=P.sha(obj)
            for tool,args,suffix in [('objdump',['-drwC'],'disassembly'),('nm',['-a','-S','--size-sort'],'symbols')]:
                target=out/(key+'.'+suffix+'.txt')
                if not target.exists():R.checked([tool,*args,obj],vd,target)
        for item in selected:
            row=next(r for r in R.M.ROWS if r['graph']==item['graph']);exe=R.case(version,item['algorithm'],row)/'timed'
            key=item['algorithm']+'_'+item['graph'].replace('/','_')+'.linked'
            identities[key]=P.sha(exe)
            for tool,args,suffix in [('objdump',['-dwC'],'disassembly'),('nm',['-a','-S','--size-sort'],'symbols')]:
                target=out/(key+'.'+suffix+'.txt')
                if not target.exists():R.checked([tool,*args,exe],vd,target)
        P.save(out/'identity.json',identities)

def summarize(R,records,phase,repeat_block=0):
    grouped=collections.defaultdict(list)
    for x in records:grouped[(x['algorithm'],x['graph'],x['workers'],x['label'])].append(x)
    output=[];tests=[];aa_tests=[]
    def load_extra(x,suffix):
        for p in [Path(x['base']+suffix),Path(x['base']+'.values'+suffix)]:
            if p.exists():return R.read(p)
        return None
    for (a,graph,width,label),rows in grouped.items():
        values=[x['seconds'] for x in rows]
        def resource_record(x):
            # The original monotonic timer already uses wait4. Reuse its
            # counters without another fork/exec or unused environment entry.
            required=['user_seconds','system_seconds','rss_kib','nvcsw','nivcsw','minflt','majflt']
            if all(k in x for k in required):return {
                'seconds':x['seconds'],'user_seconds':x['user_seconds'],'system_seconds':x['system_seconds'],
                'voluntary_switches':x['nvcsw'],'involuntary_switches':x['nivcsw'],
                'max_rss_kib':x['rss_kib'],'minor_faults':x['minflt'],'major_faults':x['majflt']}
            return load_extra(x,'.resource.json')
        resources=[resource_record(x) for x in rows]
        resources=[x for x in resources if x is not None]
        phases=[load_extra(x,'.phases.json') for x in rows];phases=[x for x in phases if x is not None]
        pools=[load_extra(x,'.pool.json') for x in rows];pools=[x for x in pools if x is not None]
        entry={'algorithm':a,'graph':graph,'workers':width,'label':label,'samples':len(rows),
            'instrumented':True,'mean_seconds':statistics.mean(values),'median_seconds':statistics.median(values),
            'resource_samples':len(resources),'phase_samples':len(phases),'feature_controls':rows[0].get('extra',{})}
        if resources:entry['resources_mean']={key:statistics.mean(x[key] for x in resources) for key in
            ['seconds','user_seconds','system_seconds','voluntary_switches','involuntary_switches','max_rss_kib','minor_faults','major_faults'] if all(key in x for x in resources)}
        if phases:entry['phase_means']=[{'nanoseconds':statistics.mean(x['phases'][i]['nanoseconds'] for x in phases),
            'calls':statistics.mean(x['phases'][i]['calls'] for x in phases),'name':PHASE_NAMES[i]} for i in range(len(PHASE_NAMES))]
        if phases and 'work' in phases[0]:entry['work_means']=dict(zip(WORK_NAMES,
            [statistics.mean(x['work'][i] for x in phases) for i in range(len(WORK_NAMES))]))
        if pools:entry['pool_means']={k:statistics.mean(x[k] for x in pools) for k in pools[0] if k!='instrumented'}
        baseline=grouped.get((a,graph,width,'baseline'))
        if baseline:entry['change_from_baseline_percent']=(entry['mean_seconds']/statistics.mean(x['seconds'] for x in baseline)-1)*100
        candidate=grouped.get((a,graph,width,'candidate'))
        if not phase and label in ['baseline_AA','candidate_AA']:
            control=grouped[(a,graph,width,label.removesuffix('_AA'))]
            differences=[math.log(x['seconds']/y['seconds']) for x,y in zip(sorted(rows,key=lambda x:x['repeat']),sorted(control,key=lambda x:x['repeat']))]
            aa_tests.append({'algorithm':a,'graph':graph,'workers':width,'label':label,
                'two_sided_p':P.permutation_pvalue(differences,two=True),'log_ratio':statistics.mean(differences)})
        if not phase and candidate and label not in ['candidate','candidate_AA','baseline_AA'] and len(candidate)==len(rows):
            candidate=sorted(candidate,key=lambda x:x['repeat']);paired=sorted(rows,key=lambda x:x['repeat'])
            diffs=[math.log(x['seconds']/y['seconds']) for x,y in zip(candidate,paired)]
            entry['candidate_over_control_percent']=(math.exp(statistics.mean(diffs))-1)*100
            test={'algorithm':a,'graph':graph,'workers':width,'label':label,
                'candidate_slower_one_sided_p':P.permutation_pvalue(diffs),'log_ratio':statistics.mean(diffs)}
            tests.append(test)
        output.append(entry)
    running=0
    for rank,t in enumerate(sorted(tests,key=lambda x:x['candidate_slower_one_sided_p'])):
        running=max(running,min(1,t['candidate_slower_one_sided_p']*(len(tests)-rank)));t['holm_p']=running
    running=0
    for rank,t in enumerate(sorted(aa_tests,key=lambda x:x['two_sided_p'])):
        running=max(running,min(1,t['two_sided_p']*(len(aa_tests)-rank)));t['holm_p']=running
    result={'instrumented':True,'groups':output,
        'recovery_tests':tests,'seed':2358,'resamples':100000,
        'identical_binary_controls':aa_tests,'AA_valid_for_diagnostic_inference':None if phase else not any(t['holm_p']<.05 for t in aa_tests),
        'note':'Diagnostic component/cross-link tests establish mechanisms only; these samples cannot promote a candidate. Invalid diagnostic A/A controls prevent a causal performance claim.'}
    name=('profile_summary' if phase else 'diagnostic_summary')+('_retry1' if repeat_block else '')+'.json'
    P.save(R.OUT/name,result)
    if not phase:P.save(R.OUT/'diagnostic_inference.json',{'summary':name,'AA_valid':result['AA_valid_for_diagnostic_inference'],'repeat_block':repeat_block})
    return result

def feature_variants(phase=False):
    stage=json.loads((P.BUNDLE/'provenance.json').read_text())['attempt'][0].upper()
    source=P.WORK/'versions/StaticSchedule/src'
    variants=[('Accepted','baseline',{}),('StaticSchedule','candidate',{}),
        ('StaticSchedule','disabled',{'SGPL_NO_EFFECT_FUSION':'1'}),
        ('StaticSchedule',{'A':'stage_off','B':'frontier_off','C':'journal_off'}[stage],{'SGPL_NO_FUSION_'+stage:'1'})]
    if (source/'effect_fusion_plan.h').exists():
        variants += [('StaticSchedule','map_off',{'SGPL_NO_FUSION_SOURCE_MAP':'1'}),
            ('StaticSchedule','accumulator_off',{'SGPL_NO_FUSION_ACCUMULATOR':'1'})]
    if stage!='B' and (source/'frontier_fusion_plan.h').exists():
        variants.append(('StaticSchedule','frontier_off',{'SGPL_NO_FUSION_B':'1'}))
    if stage=='B':
        variants += [('StaticSchedule','frontier_binary_lookup',{'SGPL_FUSION_B_BINARY_LOOKUP':'1'}),
            ('StaticSchedule','frontier_force_cold',{'SGPL_FUSION_B_FORCE_COLD':'1'}),
            ('StaticSchedule','frontier_binary_cold',{'SGPL_FUSION_B_BINARY_LOOKUP':'1','SGPL_FUSION_B_FORCE_COLD':'1'})]
    header=source/'append_journal_plan.h'
    if stage=='C' and header.exists() and 'SGPL_JOURNAL_BITMAP_PROTOCOL' in header.read_text():
        variants.append(('StaticSchedule','journal_heap_control',{'SGPL_FUSION_C_HEAP_MERGE':'1'}))
    if stage=='C' and header.exists() and 'SGPL_JOURNAL_PROVEN_APPEND' in header.read_text():
        variants.append(('StaticSchedule','journal_checked_control',{'SGPL_FUSION_C_CHECKED_APPEND':'1'}))
    if not phase:
        variants += [('Accepted','baseline_AA',{}),('StaticSchedule','candidate_AA',{}),
            ('StaticSchedule','unused_stage_control',{'SGPL_NO_FUSION_Z':'1'}),
            ('StaticSchedule','unused_global_control',{'SGPL_NO_EFFECT_FUSIOZ':'1'})]
    return variants

def run(R,cpus,phase=False,case_path=None,repeat_block=0):
    # An explicit causal case list supplements newly observed losses; it must
    # never hide automatic regression selection from this attempt.
    chosen=cases(R,case_path)+(cases(R) if case_path and (R.OUT/'comparison.json').exists() else [])
    selected=sorted({(x['algorithm'],x['graph']):x for x in chosen}.values(),key=lambda x:(P.ALGS.index(x['algorithm']),x['graph']))
    P.save(R.OUT/'diagnostic_cases.json',selected)
    R.progress(stage='profile_build' if phase else 'diagnostic_build',algorithm=None,
        completed=0,total=len(selected),failures=0,elapsed_seconds=0,block_started=time.time())
    d=P.WORK/'diagnostics';d.mkdir(exist_ok=True)
    phase_objects={v:build_phase_runtime(R,v) for v in ['Accepted','StaticSchedule']} if phase else {}
    pool_objects={v:build_pool_probe(R,v) for v in ['Accepted','StaticSchedule']} if phase else {}
    capabilities=bounded_traces(R,cpus,selected) if phase else {}
    cross=crosslinks(R,cpus,selected) if not phase else {}
    if not phase:
        cross.update(source_codegen_control(R,cpus,selected))
        cross.update(lowering_controls(R,cpus,selected))
        cross.update(runtime_components(R,cpus,selected))
        preserve_native_code(R,selected)
    records=[];original=R.binary_cmd
    method='fusion_profile' if phase else 'fusion_diagnose';repetitions=3 if phase else 20
    worker_widths=sorted(set([1,min(2,len(cpus)),len(cpus)]))
    base_variants=feature_variants(phase)
    total=sum((len(base_variants)+sum(aa==item['algorithm'] and graph==item['graph'] for aa,graph,label in cross))*repetitions*len(worker_widths) for item in selected)
    record_phase=('profile' if phase else 'diagnose')+('_retry1' if repeat_block else '')
    def calibration_signature(algorithm):
        return {'cases':[x['graph'] for x in selected if x['algorithm']==algorithm],
            'repetitions':repetitions,'widths':worker_widths,'phase_names':PHASE_NAMES if phase else [],
            'feature_controls':[(label,extra) for _,label,extra in base_variants],
            'components':sorted([list(x) for x in cross if x[0]==algorithm]),
            'phase_objects':{v:P.sha(p) for v,p in phase_objects.items()}}
    block_algorithm=None;block_started=None;diagnostic_started=time.time()
    def cmd(v,a,row,validation,workers,output):
        command,e,cwd=original(v,a,row,validation,workers,output)
        return command,e,cwd
    R.binary_cmd=cmd
    try:
        for item in selected:
            a=item['algorithm'];row=next(x for x in R.M.ROWS if x['graph']==item['graph'])
            if a!=block_algorithm:
                if block_algorithm is not None:
                    if hasattr(R,'evidence_checkpoint'):R.evidence_checkpoint(record_phase,block_algorithm,[record_phase])
                    R.calibrate(block_algorithm,method,calibration_signature(block_algorithm),block_started)
                block_algorithm=a;block_started=time.time()
                previous=R.read(R.OUT/'calibration.json',{}).get(R.key([a,method,calibration_signature(a)]))
                R.state(stage='profile' if phase else 'diagnose',algorithm=a,block_started=block_started,
                    monitor_interval_seconds=previous['seconds'] if previous else None,
                    expected_completion_unix=block_started+previous['seconds'] if previous else None)
            variants=list(base_variants)
            variants += [('StaticSchedule',label,{'SGPL_FUSION_C_CHECKED_APPEND':'1'} if label=='gate_alignment_16_checked' else {}) for aa,graph,label in cross if aa==a and graph==row['graph']]
            for w in worker_widths:
                for rep in range(repetitions):
                    order=variants if rep%2==0 else variants[::-1]
                    for v,label,extra in order:
                        if (a,row['graph'],label) in cross:
                            target=cross[(a,row['graph'],label)]
                            def cross_command(vv,aa,rr,validation,ww,out,exe=target,checked=label=='gate_alignment_16_checked'):
                                command,env,cwd=original(vv,aa,rr,validation,ww,out);command[0]=str(exe)
                                if checked:env['SGPL_FUSION_C_CHECKED_APPEND']='1'
                                return command,env,cwd
                            R.binary_cmd=cross_command
                        executable=None
                        if phase:
                            case=R.case(v,a,row);exe=case/'profile_fusion'
                            if not exe.exists():
                                vd=P.WORK/'versions'/v
                                R.checked(['g++','-O3','-fopenmp','-no-pie',case/'timed.o',
                                    *[phase_objects[v] if x=='autotuner_runtime' else (pool_objects[v][0] if x=='parallel_runtime' else vd/(x+'.o')) for x in R.OBJECTS],
                                    pool_objects[v][1],P.WORK/'monotonic_timer.o','-Wl,--wrap=sgpl_run_on_worker_pool','-Wl,--wrap=autograph_frontier_activate','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],case,case/'profile_fusion.link.log')
                            base_cmd,env,cwd=original(v,a,row,False,w,exe)
                            base_cmd[0]=str(exe)
                            R.binary_cmd=lambda vv,aa,rr,valid,ww,out,cc=base_cmd,ee=env,dd=cwd:(cc,dict(ee,SGPL_FUSION_PHASE_OUTPUT=str(out)+'.phases.json',SGPL_FUSION_POOL_OUTPUT=str(out)+'.pool.json'),dd)
                        if phase and rep==0:
                            warm=R.invoke('fusion_diagnostic','profile_warmup',v,a,row,0,label=label,workers=w,extra=extra)
                            assert not warm.get('error'),warm
                        x=R.invoke('fusion_diagnostic',record_phase,v,a,row,rep,label=label,workers=w,extra=extra,repeat_block=repeat_block)
                        assert not x.get('error'),x;records.append(x);R.binary_cmd=cmd
                R.progress(stage='profile' if phase else 'diagnose',algorithm=a,graph=row['graph'],workers=w,
                    completed=len(records),total=total,elapsed_seconds=time.time()-diagnostic_started)
                P.save(R.OUT/(('profiles' if phase else 'diagnostics')+('_retry1' if repeat_block else '')+'.json'),records)
        if block_algorithm is not None:
            if hasattr(R,'evidence_checkpoint'):R.evidence_checkpoint(record_phase,block_algorithm,[record_phase])
            R.calibrate(block_algorithm,method,calibration_signature(block_algorithm),block_started)
    finally:R.binary_cmd=original
    summary=summarize(R,records,phase,repeat_block)
    P.save(R.OUT/'profiling_capabilities.json',{'hardware_counters':capabilities,
        'phase_times':'instrumented; nested and parallel CPU durations are not additive wall-time fractions',
        'resource_counts':'wait4 child CPU, context switches and peak RSS; excluded from acceptance timings'})
    if hasattr(R,'evidence_checkpoint'):R.evidence_checkpoint(record_phase+'_summary',phases=[record_phase])
    if not phase and not summary['AA_valid_for_diagnostic_inference'] and not repeat_block:
        R.progress(stage='diagnostic_AA_retry',reason='Retain invalid diagnostic block; repeat once before causal inference')
        return run(R,cpus,phase,case_path,repeat_block=1)
    return records
