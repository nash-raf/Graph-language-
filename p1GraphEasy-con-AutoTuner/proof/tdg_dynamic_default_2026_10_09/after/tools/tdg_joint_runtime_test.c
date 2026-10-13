/* Resource, model and adaptation tests for the default scheduler. No tuner linked. */
#define main benchmark_main
#include "tdg_joint_probe.c"
#undef main

static void model_cases(void)
{
    sgpl_joint_job jobs[5] = {0};
    sgpl_joint_plan plan = {.count=5,.budget=4,.max_parents=2,.backfill=1,.order={4,1,2,3,0}};
    jobs[0].tail_ns=8e6;
    for (int j=1;j<4;++j) jobs[j].tail_ns=.2e6;
    jobs[4].count=1;
    jobs[4].events[0]=(sgpl_joint_event){.id=300,.mode=SGPL_LOOP_DOALL};
    jobs[4].events[0].costs[1]=12e6;
    jobs[4].events[0].costs[2]=5e6;
    jobs[4].events[0].costs[3]=4e6;
    plan.widths[4][0]=2;
    assert(sgpl_joint_simulate(jobs,&plan)==8.6e6);
    plan.order[0]=0; plan.order[1]=4;
    for(int j=1;j<4;++j) plan.order[j+1]=j;
    assert(sgpl_joint_simulate(jobs,&plan)==8e6);
    // More loop workers delay the ordinary critical task: allocation and
    // order must be evaluated together, despite an individually faster loop.
    plan.widths[4][0]=3;
    assert(sgpl_joint_simulate(jobs,&plan)>8e6);

    memset(jobs,0,sizeof(jobs));
    plan=(sgpl_joint_plan){.count=2,.budget=6,.order={0,1},.backfill=1};
    for(int j=0;j<2;++j) {
        jobs[j].count=1;
        jobs[j].events[0]=(sgpl_joint_event){.id=300+j,.mode=SGPL_LOOP_DOALL};
        jobs[j].events[0].costs[2]=5e6;
        plan.widths[j][0]=2;
    }
    assert(sgpl_joint_simulate(jobs,&plan)==10e6);
    for(int j=0;j<2;++j) jobs[j].events[0].mode=SGPL_LOOP_DOACROSS;
    assert(sgpl_joint_simulate(jobs,&plan)==5e6);

    // Sequential sites reserve max width, not the sum of all their teams.
    plan=(sgpl_joint_plan){.count=2,.budget=4,.order={0,1},.backfill=1};
    jobs[0].count=3;
    jobs[1]=(sgpl_joint_job){.tail_ns=10e6};
    for(int i=0;i<3;++i) {
        jobs[0].events[i]=(sgpl_joint_event){.id=300+i,.mode=SGPL_LOOP_DOALL};
        jobs[0].events[i].costs[2]=2e6;
        plan.widths[0][i]=2;
    }
    assert(sgpl_joint_simulate(jobs,&plan)==10e6 && plan.peak[0]==2);
    jobs[0].events[2].id=300;
    sgpl_joint_set_width(jobs,&plan,0,0,1);
    assert(plan.widths[0][0]==1 && plan.widths[0][2]==1);
    // An elapsed time budget stops broad search without changing the
    // mathematical objective or permitting an infeasible resource grant.
    sgpl_tdg_task_desc bounded[2] = {{0},{0}};
    g_joint_search_deadline = 1;
    sgpl_joint_plan bounded_plan = sgpl_joint_optimize(bounded,jobs,2,4,0);
    g_joint_search_deadline = 0;
    assert(bounded_plan.evaluations <= 16);
    // Sparse timing still detects an overlapping/nested dispatch interval.
    sgpl_joint_trace unsampled = {0};
    g_tls_joint_trace = &unsampled;
    sgpl_joint_trace_loop(300,100,10,20);
    sgpl_joint_trace_loop(301,100,15,25);
    g_tls_joint_trace = NULL;
    assert(unsampled.invalid);
    sgpl_joint_plan feedback = {.count=1,.order={0},.workers=1,.prediction_ns=50,.reference_ns=300};
    sgpl_joint_observe(991,&feedback,1,150);
    sgpl_joint_observe(991,&feedback,1,100);
    assert(!sgpl_joint_observed_guard(991,&feedback,1));
    sgpl_joint_observe(991,&feedback,0,125);
    sgpl_joint_observe(991,&feedback,0,125);
    assert(sgpl_joint_observed_guard(991,&feedback,1));
    feedback.workers=2; // A different admission plan gets fresh feedback.
    assert(!sgpl_joint_observed_guard(991,&feedback,1));
    feedback.prediction_ns=110;
    assert(sgpl_joint_observed_guard(991,&feedback,1));
    feedback.prediction_ns=50;
    pthread_mutex_lock(&g_joint_observed_lock);
    sgpl_joint_find_observation(991)->calls=16;
    pthread_mutex_unlock(&g_joint_observed_lock);
    assert(sgpl_joint_observed_guard(991,&feedback,1));
    puts("PASS model: coupled choices, occupied parents, pool serialization, ephemeral teams, site reuse");
}

static atomic_int occupied, peak_occupied, executed;
static int grant;
static int capacity_candidate;
static void *capacity_task(void *arg)
{
    int amount=1 + (g_tls_active_loop_pool ? g_tls_active_loop_pool->loop_budget : 0);
    int used=atomic_fetch_add(&occupied,amount)+amount;
    assert(used<=grant);
    int peak=atomic_load(&peak_occupied);
    while(used>peak && !atomic_compare_exchange_weak(&peak_occupied,&peak,used));
    if(capacity_candidate) {
        assert(g_tls_thread_budget_scope_reserved==1);
        assert(sgpl_current_thread_budget() <= (amount>1?amount-1:1));
    }
    struct timespec ts={0,1000000}; nanosleep(&ts,NULL);
    if (arg) {
        sgpl_run_tdg_level((sgpl_tdg_task_desc[]){{capacity_task,NULL,-1,1,0,NULL}},1,1,1);
    }
    atomic_fetch_add(&executed,1);
    atomic_fetch_sub(&occupied,amount);
    return NULL;
}

static void capacity_cases(void)
{
    int b=sgpl_configured_worker_count();
    sgpl_tdg_task_desc tasks[12];
    sgpl_joint_plan plan={.count=12,.budget=b,.workers=b,.backfill=1};
    grant=b;
    for(int j=0;j<12;++j) {
        tasks[j]=(sgpl_tdg_task_desc){capacity_task,NULL,700+j,1,0,NULL};
        plan.order[j]=11-j;
        plan.peak[j]=(j%3==0 && b>=3) ? b-1 : 0;
    }
    for(int fail=0;fail<2;++fail) {
        capacity_candidate=1;
        atomic_store(&executed,0); atomic_store(&peak_occupied,0);
        if(fail) setenv("SGPL_TDG_TEST_THREAD_FAIL","1",1);
        int budget=sgpl_budget_try_reserve(b);
        sgpl_joint_execute(tasks,&plan);
        sgpl_budget_release(budget);
        unsetenv("SGPL_TDG_TEST_THREAD_FAIL");
        assert(atomic_load(&executed)==12 && atomic_load(&occupied)==0);
        assert(atomic_load(&peak_occupied)<=b && sgpl_debug_reserved_threads()==0);
    }
    // Actual grant, not configured width, controls the planner/executor.
    if(b>=4) {
        capacity_candidate=0; grant=2;
        int held=sgpl_budget_try_reserve(b-2);
        sgpl_run_tdg_level(tasks,12,12,1);
        assert(sgpl_debug_reserved_threads()==held);
        sgpl_budget_release(held);
    }
    assert(sgpl_debug_reserved_threads()==0);
    puts("PASS executor: resource peaks, exactly-once callbacks, failed creation, actual grant, ledger");
}

static void adaptation_cases(void)
{
    int p=sgpl_configured_worker_count();
    sgpl_tdg_task_desc tasks[8];
    for(int j=0;j<8;++j) {
        prepare(&task_data[j],j,j==0, j==0?65536:1000,12);
        tasks[j]=(sgpl_tdg_task_desc){task,&task_data[j],j,100,j==0,j==0?&task_data[j].desc.loop_id:NULL};
    }
    ordinary_count=7;
    for(int r=0;r<24;++r) {
        for(int j=0;j<8;++j) task_data[j].calls=0;
        atomic_store(&ordinary_done,0);
        sgpl_run_tdg_level(tasks,8,800,100);
        for(int j=0;j<8;++j) assert(task_data[j].calls==1);
        assert(sgpl_debug_reserved_threads()==0);
    }
    if(p>1) {
        assert(atomic_load(&g_joint_plans)>0);
        sgpl_joint_task_profile profiles[SGPL_JOINT_MAX_TASKS];
        assert(sgpl_joint_profiles_ready(tasks,8,profiles));
        assert(profiles[0].event_count==1);
        sgpl_joint_plan first;
        assert(sgpl_joint_get_plan(tasks,8,p,&first));
        unsigned long hits=atomic_load(&g_joint_cache_hits);
        assert(sgpl_joint_get_plan(tasks,8,p,&first));
        assert(atomic_load(&g_joint_cache_hits)>hits);
        // Ordinary-time jitter can update costs without another full search.
        unsigned long searches=atomic_load(&g_joint_plans);
        unsigned long revalidations=atomic_load(&g_joint_revalidations);
        pthread_mutex_lock(&g_joint_profile_lock);
        ++g_joint_profiles[1].revision;
        g_joint_profiles[1].tail_ns *= 1.01;
        pthread_mutex_unlock(&g_joint_profile_lock);
        assert(sgpl_joint_get_plan(tasks,8,p,&first));
        assert(atomic_load(&g_joint_plans)==searches);
        assert(atomic_load(&g_joint_revalidations)>revalidations);
        g_joint_measurement_enabled = 1;
        assert(sgpl_joint_dispatch_cost(2,5,0)>0);
        unsigned long calibrations=atomic_load(&g_joint_calibration_runs);
        assert(sgpl_joint_dispatch_cost(3,7,0)>0);
        assert(atomic_load(&g_joint_calibration_runs)==calibrations);
        g_joint_measurement_enabled = 0;
        // Same site/work but a different execution regime cannot reuse a plan.
        unsigned long plans=atomic_load(&g_joint_plans);
        int64_t env_size=g_loop_states[300].regime_env_size;
        g_loop_states[300].regime_env_size=env_size+8;
        assert(sgpl_joint_get_plan(tasks,8,p,&first));
        assert(atomic_load(&g_joint_plans)>plans);
        g_loop_states[300].regime_env_size=env_size;
        // Metadata changes invalidate the plan and refuse undeclared work.
        tasks[0].num_loop_sites=0; tasks[0].loop_site_ids=NULL;
        assert(!sgpl_joint_profiles_ready(tasks,8,profiles));
        tasks[0].num_loop_sites=1; tasks[0].loop_site_ids=&task_data[0].desc.loop_id;
        uint32_t revision=g_joint_profiles[0].revision;
        atomic_store(&g_joint_observations[0],9); // Not a timing-sample invocation.
        task_data[0].n=1024;
        sgpl_run_tdg_level(tasks,8,800,100);
        assert(g_joint_profiles[0].revision!=revision);
        assert(!g_joint_profiles[0].valid);
        assert(!sgpl_joint_profiles_ready(tasks,8,profiles));
        sgpl_run_tdg_level(tasks,8,800,100);
        assert(g_joint_profiles[0].valid && g_joint_profiles[0].samples==1);
    }
    for(int j=0;j<8;++j) free(task_data[j].values);
    // Too many tasks and missing identities use the conservative FIFO fallback.
    sgpl_tdg_task_desc many[65];
    for(int j=0;j<65;++j) many[j]=(sgpl_tdg_task_desc){capacity_task,NULL,-1,1,0,NULL};
    // Reference workers have a group scope, so use a callback without the
    // candidate-specific scope assertion for this fallback check.
    for(int j=0;j<65;++j) many[j].fn=NULL;
    sgpl_run_tdg_level(many,65,65,1);
    assert(sgpl_debug_reserved_threads()==0);
    puts("PASS adaptation: calibrated traces, cache reuse, changed work, metadata refusal, bounded fallback");
}

static int cold_order[3], cold_calls;
static void *cold_task(void *arg)
{
    cold_order[cold_calls++] = *(int *)arg;
    return NULL;
}

static void cold_fifo_case(void)
{
    int ids[3]={0,1,2}, loop_id=300;
    sgpl_tdg_task_desc tasks[3];
    for(int i=0;i<3;++i)
        tasks[i]=(sgpl_tdg_task_desc){cold_task,&ids[i],-1,1,i==0,i==0?&loop_id:NULL};
    setenv("SGPL_TDG_TEST_ALLOC_FAIL","1",1);
    sgpl_run_tdg_level(tasks,3,3,1);
    unsetenv("SGPL_TDG_TEST_ALLOC_FAIL");
    assert(cold_calls==3);
    for(int i=0;i<3;++i) assert(cold_order[i]==i);
    assert(sgpl_debug_reserved_threads()==0);
    puts("PASS default: cold fallback preserves source order without an ordinary-first phase");
}

int main(void)
{
    sgpl_configured_worker_count();
    model_cases();
    capacity_cases();
    adaptation_cases();
    cold_fifo_case();
    structural();
    puts("ALL PHASE A STRUCTURAL TESTS PASS");
    return 0;
}
