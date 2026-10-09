/* Mixed-level extension of the test-only SGPL callback adapter. Ordinary work
 * is compiled from SGPL, never divided between workers, and independently
 * checked. Production runtime, compiler, models and autotuner are untouched. */
#define main two_loop_main_unused
#define __wrap_sgpl_run_tdg_level two_loop_capture_unused
#include "driver.c"
#undef main
#undef __wrap_sgpl_run_tdg_level

extern int32_t sgpl_ordinary_work(int32_t rounds, int32_t seed);
#define MIX_MAX 8
#define MIX_ORDERS 720
static int ordinary_count, total, rounds[MIX_MAX], expected[MIX_MAX];
static int sequences[MIX_ORDERS][MIX_MAX], sequence_count;
static int calls[MIX_MAX], outputs[MIX_MAX];
static uint64_t starts[MIX_MAX], ends[MIX_MAX];
static sgpl_tdg_task_desc mixed_tasks[MIX_MAX];
static int task_indices[MIX_MAX];
static unsigned long validations, phased_validations;

static void *mixed_callback(void *argument) {
    int j=*(int *)argument;
    starts[j]=sgpl_now_ns();
    ++calls[j];
    if(j<2) captured[j].fn(captured[j].arg);
    else outputs[j]=sgpl_ordinary_work(rounds[j-2],1);
    ends[j]=sgpl_now_ns();
    return NULL;
}

static int encode_sequence(const sgpl_joint_plan *plan) {
    int canonical[MIX_MAX], used[MIX_MAX]={0};
    for(int r=0;r<total;++r) {
        int j=plan->order[r];
        if(j>=2) for(int k=2;k<total;++k)
            if(!used[k] && rounds[k-2]==rounds[j-2]) { j=k; break; }
        canonical[r]=j; used[j]=1;
    }
    for(int s=0;s<sequence_count;++s)
        if(!memcmp(canonical,sequences[s],total*sizeof(int))) return s*2+plan->backfill;
    assert(!"Native ordering absent from exhaustive oracle"); return -1;
}

static void mixed_execute_level(void) {
    int budget=sgpl_configured_worker_count();
    int64_t work=0,span=0;
    for(int j=0;j<total;++j) {
        task_indices[j]=j;
        mixed_tasks[j]=j<2 ? captured[j] : (sgpl_tdg_task_desc){
            .profile_id=7000+j,.static_work_units=18,.num_loop_sites=0};
        /* 18 units is the existing instruction heuristic for call+return:
         * call=10, call adjustment=2, return=3, control=1, block=2.
         * Actual task durations are learned through normal runtime profiles. */
        mixed_tasks[j].fn=mixed_callback; mixed_tasks[j].arg=&task_indices[j];
        work+=mixed_tasks[j].static_work_units;
        if(mixed_tasks[j].static_work_units>span) span=mixed_tasks[j].static_work_units;
    }
    memset(calls,0,sizeof(calls)); memset(outputs,0,sizeof(outputs));
    memset(requested,0,sizeof(requested)); memset(effective,0,sizeof(effective));
    uint64_t references=atomic_load(&g_joint_reference_calls),begin=sgpl_now_ns();
    if(!forced) {
        if(dynamic_model) sgpl_run_tdg_level(mixed_tasks,total,work,span);
        else sweep_ordinary_first(mixed_tasks,total,work,span);
    } else if(backend==0) sweep_ordinary_first(mixed_tasks,total,work,span);
    else if(backend==2) sgpl_run_tdg_level_fifo(mixed_tasks,total,work,span);
    else {
        assert(order>=0 && order/2<sequence_count);
        sgpl_joint_plan plan={.count=total,.budget=budget,.workers=parents,
            .max_parents=parents,.backfill=order%2};
        memcpy(plan.order,sequences[order/2],total*sizeof(int));
        for(int j=0;j<2;++j) {
            plan.event_counts[j]=1; plan.event_ids[j][0]=ids[j];
            plan.widths[j][0]=force_width[j];
            plan.peak[j]=force_width[j]>=2?force_width[j]:0;
            assert(1+plan.peak[j]<=budget);
        }
        int grant=sgpl_budget_try_reserve(budget); assert(grant==budget);
        sgpl_joint_execute(mixed_tasks,&plan); sgpl_budget_release(grant);
    }
    elapsed=sgpl_now_ns()-begin;
    assert(sgpl_debug_reserved_threads()==0);
    executed_backend=backend; executed_order=order; executed_parents=parents; prediction=0;
    if(!forced) {
        executed_backend=dynamic_model ? (references==atomic_load(&g_joint_reference_calls) ? 1 : 2) : 0;
        executed_order=0;
        executed_parents=executed_backend==2 ? (total<budget?total:budget) :
            total>budget ? (ordinary_count<budget?ordinary_count:budget) : total;
        if(executed_parents<2) executed_parents=2;
        prediction=g_tls_joint_last_prediction_ns;
        if(executed_backend==1) {
            int found=0;
            for(int c=0;c<SGPL_JOINT_CACHE_SIZE;++c) {
                sgpl_joint_cached_plan *cache=&g_joint_cache[c];
                if(cache->valid && cache->count==total && cache->budget==budget &&
                   cache->functions[0]==mixed_callback && cache->profile_ids[0]==mixed_tasks[0].profile_id) {
                    executed_order=encode_sequence(&cache->plan);
                    executed_parents=cache->plan.workers; found=1; break;
                }
            }
            assert(found);
        }
    }
    for(int j=0;j<total;++j) {
        assert(calls[j]==1 && starts[j]<=ends[j]);
        if(j>=2) assert(outputs[j]==expected[j-2]);
    }
    for(int j=0;j<2;++j) {
        assert(effective[j]>=1 && effective[j]<=budget);
        if(forced) assert(effective[j]==force_width[j]);
    }
    if(total>budget && executed_backend==0) {
        for(int j=2;j<total;++j) for(int k=0;k<2;++k) assert(ends[j]<=starts[k]);
        ++phased_validations;
    }
    ++validations; verify_arrays();
    if(phase) fprintf(stdout,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%llu,%.0f\n",phase,iteration,
        requested[0],requested[1],effective[0],effective[1],executed_backend,
        executed_order,executed_parents,(unsigned long long)elapsed,prediction);
}

void __wrap_sgpl_run_tdg_level(const sgpl_tdg_task_desc *tasks,int32_t count,int64_t work,int64_t span) {
    (void)work; (void)span;
    if(count==1 && tasks[0].num_loop_sites==1) {
        assert(captured_count<2);
        captured[captured_count]=tasks[0]; ids[captured_count]=tasks[0].loop_site_ids[0];
        if(++captured_count==2) mixed_execute_level();
    } else {
        assert(captured_count==0 || captured_count==2);
        for(int j=0;j<count;++j) tasks[j].fn(tasks[j].arg);
    }
}

int main(int argc,char **argv) {
    assert(argc==10);
    dynamic_model=atoi(argv[1]); int mode=atoi(argv[2]);
    n[0]=atoi(argv[7]); n[1]=atoi(argv[8]);
    int packed=atoi(argv[9]); depth[0]=packed/100; depth[1]=packed%100;
    int samples=atoi(getenv("SWEEP_SAMPLES")?getenv("SWEEP_SAMPLES"):"5");
    assert(samples>=3);
    assert(scanf("%d %d",&ordinary_count,&sequence_count)==2);
    total=ordinary_count+2; assert(total<=MIX_MAX && sequence_count<=MIX_ORDERS && sequence_count>0);
    for(int j=0;j<ordinary_count;++j) {
        assert(scanf("%d",&rounds[j])==1 && rounds[j]>=1);
        int x=1; for(int k=0;k<rounds[j];++k) x=(x*17+11)%1009;
        expected[j]=x;
    }
    for(int s=0;s<sequence_count;++s) {
        int mask=0;
        for(int j=0;j<total;++j) {
            assert(scanf("%d",&sequences[s][j])==1);
            int k=sequences[s][j]; assert(k>=0 && k<total && !(mask&(1<<k))); mask|=1<<k;
        }
    }
    forced=1; backend=1; parents=1; order=1;
    force_width[0]=force_width[1]=1;
    atomic_store(&g_joint_collect_enabled,1);
    for(int i=0;i<8;++i) { verify_full=i==0; run_once(); }
    forced=0; verify_full=0; atomic_store(&g_joint_collect_enabled,dynamic_model);
    for(int i=0;i<12;++i) run_once();
    phase=1; for(iteration=0;iteration<samples;++iteration) run_once();
    if(mode==-2) {
        int config=0;
        while(scanf("%d %d %d %d %d",&backend,&force_width[0],&force_width[1],&order,&parents)==5) {
            forced=1; phase=0;
            atomic_store(&g_joint_collect_enabled,backend==1);
            for(int i=0;i<2;++i) run_once();
            phase=100+config++;
            for(iteration=0;iteration<samples;++iteration) run_once();
        }
    }
    fprintf(stderr,"MIX_VALIDATION,%lu,%lu,%d,%d\n",validations,phased_validations,ordinary_count,total);
    return 0;
}
