/* Test-only adapter: combine two independent SGPL-generated callbacks.
 * Production runtime and SGPL loop bodies are not edited. The build makes a
 * runtime copy with two grant-observation calls; both policies share that copy.
 */
#include <stdint.h>
static void sweep_grant(int id, int width);
#define SGPL_ENABLE_JOINT_SCHEDULER
#include SWEEP_RUNTIME_SOURCE
#include <assert.h>
#include "ordinary_first_reference.inc"

extern int graph_main(void);
extern int32_t __real_sgpl_should_parallelize_doall(const sgpl_loop_profile_desc *, int64_t, int64_t, int64_t);

static sgpl_tdg_task_desc captured[2];
static int captured_count, ids[2], requested[2], effective[2];
static int forced, force_width[2], backend, order, parents, dynamic_model;
static int n[2], depth[2], verify_full, iteration, phase;
static uint64_t elapsed;
static int executed_backend, executed_order, executed_parents;
static double prediction;

static unsigned measured_native_calls;
static unsigned long long native_first_ns;
static unsigned long previous_learning_ns, steady_learning_ns;
static void sweep_record_native(unsigned long long elapsed_ns) {
    if (forced) return;
    unsigned long learning=atomic_load(&g_joint_learning_ns);
    if (!measured_native_calls++) native_first_ns=elapsed_ns;
    if (phase==1) steady_learning_ns += learning-previous_learning_ns;
    previous_learning_ns=learning;
}
static void sweep_report_stats(void) {
    fprintf(stderr,"SCHED_STATS,%llu,%lu,%lu,%lu,%lu,%lu,%u\n",
        native_first_ns,atomic_load(&g_joint_learning_ns),steady_learning_ns,
        atomic_load(&g_joint_plans),atomic_load(&g_joint_search_evals),
        atomic_load(&g_joint_calibration_runs),measured_native_calls);
}


static int index_of(int id) {
    for (int j=0;j<2;++j) if (ids[j]==id) return j;
    assert(!"Unexpected loop site"); return -1;
}
static void sweep_grant(int id, int width) { effective[index_of(id)] = width; }

int32_t __wrap_sgpl_should_parallelize_doall(const sgpl_loop_profile_desc *desc,
                                            int64_t first, int64_t last, int64_t step) {
    int j=index_of(desc->loop_id);
    assert(sgpl_compute_trip_count(first,last,step)==n[j]);
    if (forced) {
        sgpl_level_loop_pool *pool=g_tls_active_loop_pool;
        assert(pool && pool->active);
        pthread_mutex_lock(&pool->lock);
        int found=0;
        for(int e=0;e<pool->entry_count;++e)
            if(pool->entries[e].loop_id==desc->loop_id &&
               pool->entries[e].task_slot==g_tls_current_tdg_task_slot) {
                pool->entries[e].assigned_threads=force_width[j]; found=1;
            }
        /* Cold descriptors are absent from the ordinary planner's entries. */
        if(!found) {
            assert(pool->entry_count<SGPL_MAX_TDG_LEVEL_LOOPS);
            pool->entries[pool->entry_count++]=(sgpl_level_loop_plan_entry){
                .task_slot=g_tls_current_tdg_task_slot,.loop_id=desc->loop_id,
                .assigned_threads=force_width[j]};
        }
        pthread_mutex_unlock(&pool->lock);
    }
    requested[j]=sgpl_loop_effective_decision_threads_for_loop_id(desc->loop_id);
    int choose=__real_sgpl_should_parallelize_doall(desc,first,last,step);
    if(forced) choose=force_width[j]>=2;
    effective[j]=choose ? requested[j] : 1;
    if(choose) sgpl_set_pending_loop_id(desc->loop_id);
    return choose;
}

static int value_at(int j, int i) {
    int x=(i*(j?31:17)+(j?5:3))%(j?1013:1009);
    for(int k=1;k<depth[j];++k) x=(x*(19+2*k)+7+k)%(1009+2*k);
    return x;
}
static void verify_arrays(void) {
    int *a=*(int **)captured[0].arg, *b=*(int **)captured[1].arg;
    assert(a && b && a!=b);
    for(int j=0;j<2;++j) {
        int *data=j?b:a;
        if(verify_full) for(int i=0;i<n[j];++i) assert(data[i]==value_at(j,i));
        else for(int k=0;k<16;++k) {
            int i=(int)((int64_t)k*(n[j]-1)/15);
            assert(data[i]==value_at(j,i));
        }
    }
}
static void execute_level(void) {
    int budget=sgpl_configured_worker_count();
    memset(requested,0,sizeof(requested)); memset(effective,0,sizeof(effective));
    uint64_t references=atomic_load(&g_joint_reference_calls);
    uint64_t begin=sgpl_now_ns();
    if(!forced) {
        if(dynamic_model) sgpl_run_tdg_level(captured,2,258,129);
        else sweep_ordinary_first(captured,2,258,129);
    } else if(backend==0) sweep_ordinary_first(captured,2,258,129);
    else if(backend==2) sgpl_run_tdg_level_fifo(captured,2,258,129);
    else {
        sgpl_joint_plan plan={.count=2,.budget=budget,.workers=parents,
            .max_parents=parents,.backfill=1};
        plan.order[0]=order; plan.order[1]=1-order;
        for(int j=0;j<2;++j) {
            plan.event_counts[j]=1; plan.event_ids[j][0]=ids[j];
            plan.widths[j][0]=force_width[j];
            plan.peak[j]=force_width[j]>=2?force_width[j]:0;
            assert(1+plan.peak[j]<=budget);
        }
        int grant=sgpl_budget_try_reserve(budget); assert(grant==budget);
        sgpl_joint_execute(captured,&plan); sgpl_budget_release(grant);
    }
    elapsed=sgpl_now_ns()-begin;
    sweep_record_native(elapsed);
    assert(sgpl_debug_reserved_threads()==0);
    for(int j=0;j<2;++j) {
        assert(effective[j]>=1 && effective[j]<=budget);
        if(forced) assert(effective[j]==force_width[j]);
    }
    executed_backend=backend; executed_order=order; executed_parents=parents;
    prediction=0;
    if(!forced) {
        executed_backend=dynamic_model ? (references==atomic_load(&g_joint_reference_calls) ? 1 : 2) : 0;
        executed_order=0; executed_parents=budget>1?2:1;
        prediction=g_tls_joint_last_prediction_ns;
        if(executed_backend==1) for(int c=0;c<SGPL_JOINT_CACHE_SIZE;++c) {
            sgpl_joint_cached_plan *cache=&g_joint_cache[c];
            if(cache->valid && cache->count==2 && cache->budget==budget &&
               cache->functions[0]==captured[0].fn && cache->functions[1]==captured[1].fn) {
                executed_order=cache->plan.order[0];
                executed_parents=cache->plan.workers;
                break;
            }
        }
    }
    verify_arrays();
    if(phase) fprintf(stdout,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%llu,%.0f\n",phase,iteration,
        requested[0],requested[1],effective[0],effective[1],executed_backend,
        executed_order,executed_parents,(unsigned long long)elapsed,prediction);
}

void __wrap_sgpl_run_tdg_level(const sgpl_tdg_task_desc *tasks,int32_t count,
                              int64_t work,int64_t span) {
    if(count==1 && tasks[0].num_loop_sites==1) {
        assert(captured_count<2);
        captured[captured_count]=tasks[0]; ids[captured_count]=tasks[0].loop_site_ids[0];
        ++captured_count;
        if(captured_count==2) execute_level();
    } else {
        assert(captured_count==0 || captured_count==2);
        /* Array initialization and printing stay outside the timed level. */
        for(int j=0;j<count;++j) tasks[j].fn(tasks[j].arg);
    }
}
/* SGPL prints are suppressed by a linker wrapper, not by editing its source. */
int __wrap_printf(const char *fmt,...) { (void)fmt; return 0; }
static void run_once(void) {
    captured_count=0; graph_main(); assert(captured_count==2);
}

/* Diagnostics AFTER native choices are recorded, BEFORE any oracle timings.
 * Exhaustively score frozen learned profiles to distinguish curve error from a
 * bounded search that never reaches the best predicted candidate. This does
 * not feed a different plan back to the production model. */
static void diagnose(void) {
    if(!dynamic_model) return;
    int budget=sgpl_configured_worker_count();
    for(int c=0;c<SGPL_JOINT_CACHE_SIZE;++c) {
        sgpl_joint_cached_plan *cache=&g_joint_cache[c];
        if(cache->valid && cache->count==2 && cache->budget==budget &&
           cache->functions[0]==captured[0].fn && cache->functions[1]==captured[1].fn)
            fprintf(stderr,"DIAG_SEARCH,%d,%.0f,%.0f,%.0f,%d,%d\n",cache->plan.evaluations,
                cache->plan.planning_ns,cache->plan.prediction_ns,cache->plan.reference_ns,
                cache->plan.widths[0][0],cache->plan.widths[1][0]);
    }
    sgpl_joint_task_profile profiles[SGPL_JOINT_MAX_TASKS];
    if(!sgpl_joint_profiles_ready(captured,2,profiles)) return;
    sgpl_joint_job jobs[2]={0};
    for(int j=0;j<2;++j) {
        assert(profiles[j].event_count==1);
        jobs[j].count=1; jobs[j].tail_ns=profiles[j].tail_ns;
        sgpl_joint_event *e=&jobs[j].events[0];
        e->id=profiles[j].events[0].loop_id; e->trips=profiles[j].events[0].trips;
        e->max_threads=profiles[j].events[0].max_threads;
        e->before_ns=profiles[j].events[0].before_ns;
        e->mode=SGPL_LOOP_DOALL; e->runtime_model=1;
        for(int p=1;p<budget;++p) e->costs[p]=p>e->trips?HUGE_VAL:-1.0;
    }
    for(int a=1;a<budget;++a) for(int b=1;b<budget;++b)
        for(int o=0;o<2;++o) for(int h=1;h<=2;++h) {
            if(h==2 && 2+(a>=2?a:0)+(b>=2?b:0)>budget) continue;
            sgpl_joint_plan plan={.count=2,.budget=budget,.max_parents=h,.backfill=1};
            plan.order[0]=o; plan.order[1]=1-o;
            plan.widths[0][0]=a; plan.widths[1][0]=b;
            double cost=sgpl_joint_simulate(jobs,&plan);
            fprintf(stderr,"DIAG_SCORE,%d,%d,%d,%d,%.0f\n",a,b,o,h,cost);
        }
}

int main(int argc,char **argv) {
    assert(argc==10);
    dynamic_model=atoi(argv[1]); backend=atoi(argv[2]);
    force_width[0]=atoi(argv[3]); force_width[1]=atoi(argv[4]);
    order=atoi(argv[5]); parents=atoi(argv[6]);
    n[0]=atoi(argv[7]); n[1]=atoi(argv[8]);
    int packed=atoi(argv[9]); depth[0]=packed/100; depth[1]=packed%100;
    int repetitions=9;
    const char *r=getenv("SWEEP_SAMPLES"); if(r) repetitions=atoi(r);
    assert(repetitions>=3 && n[0]>1 && n[1]>1);
    /* Each process has fresh profiles. Train on useful serial SGPL work; never
     * seed model choices with the exhaustive sweep's measurements. */
    int chosen_backend=backend, w0=force_width[0], w1=force_width[1];
    forced=1; backend=1; parents=1; order=0;
    force_width[0]=force_width[1]=1;
    atomic_store(&g_joint_collect_enabled,1);
    for(int i=0;i<8;++i) { verify_full=i==0; run_once(); }
    forced=0;
    atomic_store(&g_joint_collect_enabled,dynamic_model);
    for(int i=0;i<12;++i) { verify_full=0; run_once(); }
    if(chosen_backend<0) {
        phase=1;
        for(iteration=0;iteration<repetitions;++iteration) run_once();
        if(getenv("SWEEP_DIAGNOSE")) diagnose();
        if(chosen_backend==-2) {
            int config=0;
            while(scanf("%d %d %d %d %d",&backend,&force_width[0],&force_width[1],&order,&parents)==5) {
                forced=1; phase=0;
                atomic_store(&g_joint_collect_enabled,backend==1 || dynamic_model);
                for(int i=0;i<2;++i) { verify_full=i==0 && config==0; run_once(); }
                phase=100+config++; verify_full=0;
                for(iteration=0;iteration<repetitions;++iteration) run_once();
            }
        }
    } else {
        forced=1; backend=chosen_backend; force_width[0]=w0; force_width[1]=w1;
        parents=atoi(argv[6]); order=atoi(argv[5]);
        atomic_store(&g_joint_collect_enabled,backend==1 || dynamic_model);
        for(int i=0;i<2;++i) { verify_full=i==0; run_once(); }
        phase=2; verify_full=0;
        for(iteration=0;iteration<repetitions;++iteration) run_once();
    }
    sweep_report_stats();
    return 0;
}
