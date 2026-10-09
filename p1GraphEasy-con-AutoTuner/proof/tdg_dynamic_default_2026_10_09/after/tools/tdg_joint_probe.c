/* A/B benchmark of the public TDG executor. No autotuner is linked.
 * Include a snapshot of the runtime to compare policies with identical bodies. */
#ifndef SGPL_RUNTIME_SOURCE
#define SGPL_RUNTIME_SOURCE "../parallel_runtime.c"
#endif
#include SGPL_RUNTIME_SOURCE
#include <assert.h>
#ifdef JOINT
#define RUN_LEVEL sgpl_run_tdg_level_joint
#else
#define RUN_LEVEL sgpl_run_tdg_level
#endif

#define MAX_TASKS 32
typedef struct {
    int id, loop, chain, width, scope, spare;
    int64_t n;
    double *values;
    sgpl_loop_profile_desc desc;
    pthread_t thread;
    uint64_t begin, end;
    int calls;
} test_task;
static test_task task_data[MAX_TASKS];
static int ordinary_count;
static int combined_loops;
static atomic_int ordinary_done, early_loops;
static pthread_barrier_t ordinary_barrier;
static int barrier_test;

static void body(int64_t i, void *env)
{
    test_task *t = env;
    double x = (double)(i + 1) * .37, s = x * .5;
    for (int k = 0; k < t->chain; ++k) {
        s += x / (1.0 + s * s + k);
        x = x * 1.0000001 + s;
    }
    t->values[i] = s;
}
static void range(int64_t first, int64_t count, int64_t step, void *env)
{
    for (int64_t k = 0; k < count; ++k) body(first + k * step, env);
}
static double median(double *x, int n)
{
    for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j)
        if (x[j] < x[i]) { double tmp = x[i]; x[i] = x[j]; x[j] = tmp; }
    return x[n / 2];
}
static void *task(void *arg)
{
    test_task *t = arg;
    t->thread = pthread_self();
    ++t->calls;
    t->begin = sgpl_now_ns();
    t->scope = g_tls_thread_budget_scope_reserved;
    t->spare = g_tls_active_loop_pool ? g_tls_active_loop_pool->loop_budget : -1;
    if (t->loop) {
        if (atomic_load(&ordinary_done) < ordinary_count) atomic_fetch_add(&early_loops, 1);
        t->width = sgpl_loop_effective_decision_threads_for_loop_id(t->desc.loop_id);
        int across = t->desc.mode == SGPL_LOOP_DOACROSS;
        int parallel = across ? sgpl_should_parallelize_doacross(&t->desc, 0, t->n, 1)
                              : sgpl_should_parallelize_doall(&t->desc, 0, t->n, 1);
        if (!parallel) t->width = 1;
        if (parallel) {
            sgpl_set_pending_loop_id(t->desc.loop_id);
            sgpl_set_pending_range_body(range);
            parallel_for_runtime(0, t->n, 1, body, t, across, across);
        } else {
            uint64_t serial_begin = sgpl_now_ns();
            range(0, t->n, 1, t);
            uint64_t elapsed = sgpl_now_ns() - serial_begin;
            if (across) sgpl_record_doacross_serial_sample(&t->desc,0,t->n,1,0,elapsed);
            else sgpl_record_doall_serial_sample(&t->desc,0,t->n,1,elapsed);
        }
    } else {
        if (barrier_test) pthread_barrier_wait(&ordinary_barrier);
        range(0, t->n, 1, t);
        atomic_fetch_add(&ordinary_done, 1);
    }
    t->end = sgpl_now_ns();
    return NULL;
}
static void *combined_task(void *arg)
{
    (void)arg;
    for (int i = 0; i < combined_loops; ++i) task(&task_data[i]);
    return NULL;
}
static void prepare(test_task *t, int id, int loop, int64_t n, int chain)
{
    memset(t, 0, sizeof(*t));
    t->id = id; t->loop = loop; t->n = n; t->chain = chain;
    t->values = calloc(n, sizeof(double)); assert(t->values);
    t->desc.loop_id = 300 + id;
    t->desc.mode = SGPL_LOOP_DOALL;
    if (loop && !getenv("SGPL_JOINT_PROBE_COLD")) {
        double times[7];
        for (int r = 0; r < 7; ++r) {
            uint64_t begin = sgpl_now_ns(); range(0, n, 1, t);
            times[r] = sgpl_now_ns() - begin;
        }
        uint64_t elapsed = (uint64_t)median(times, 7);
        for (int r = 0; r < 6; ++r)
            sgpl_record_doall_serial_sample(&t->desc, 0, n, 1, elapsed);
        /* The generated serial path also calls the decision helper, which
         * observes N. Recording c alone otherwise leaves the planner using
         * the coarse >2048 bucket representative on its first level. */
        (void)sgpl_should_parallelize_doall(&t->desc, 0, n, 1);
        sgpl_set_pending_loop_id(-1);
    }
}

#ifndef BASELINE
typedef struct { sgpl_level_loop_pool *pool; int id, grant; pthread_barrier_t *barrier; } reserve_arg;
static void *reserve_racer(void *arg)
{
    reserve_arg *a = arg;
    g_tls_active_loop_pool = a->pool; g_tls_current_tdg_task_slot = 0;
    for (int i = 0; i < 1000; ++i) {
        pthread_barrier_wait(a->barrier);
        a->grant = sgpl_loop_pool_try_acquire(a->id, 2);
        pthread_barrier_wait(a->barrier);
        pthread_barrier_wait(a->barrier);
        sgpl_loop_pool_release(a->id, a->grant);
        pthread_barrier_wait(a->barrier);
    }
    return NULL;
}
static void structural(void)
{
    int p = sgpl_configured_worker_count(), k = p < 8 ? p : 8;
    sgpl_tdg_task_desc desc[MAX_TASKS];
    sgpl_level_loop_plan plan;
    /* A skewed profile must not serialise an otherwise runnable level. */
    for (int i = 0; i < k; ++i) {
        prepare(&task_data[i], i, 0, 16, 4);
        desc[i] = (sgpl_tdg_task_desc){task, &task_data[i], -1, i ? 1 : 1000000, 0, NULL};
    }
    assert(sgpl_choose_tdg_threads(1000000, 1000000, k) == k);
    assert(sgpl_choose_tdg_threads_with_loop_budget(1000000, 1000000, desc, k, p, &plan) == k);
    barrier_test = 1; ordinary_count = k;
    pthread_barrier_init(&ordinary_barrier, NULL, k);
    RUN_LEVEL(desc, k, 1000000, 1000000);
    pthread_barrier_destroy(&ordinary_barrier);
    barrier_test = 0;
    for (int i = 0; i < k; ++i) {
        assert(task_data[i].calls == 1);
        for (int64_t j = 0; j < task_data[i].n; ++j) assert(task_data[i].values[j] > 0);
        for (int j = 0; j < i; ++j) assert(!pthread_equal(task_data[i].thread, task_data[j].thread));
        free(task_data[i].values);
    }
    assert(sgpl_debug_reserved_threads() == 0);

    /* Two sequential loop sites can reuse one task's team reservation. */
    int32_t ids[3] = {300,301,302};
    sgpl_tdg_task_desc combined = {combined_task,NULL,-1,100,3,ids};
    combined_loops = 3; ordinary_count = 0;
    for (int i = 0; i < 3; ++i) prepare(&task_data[i],i,1,65536,12);
    RUN_LEVEL(&combined,1,100,100);
    for (int i = 0; i < 3; ++i) {
        assert(task_data[i].calls == 1);
        if (p >= 3) assert(task_data[i].width >= 2 && task_data[i].width <= p-1);
        free(task_data[i].values);
    }
    assert(sgpl_debug_reserved_threads() == 0);

    /* Independent DOACROSS work uses ephemeral teams, not the exclusive
     * DOALL pool. Exercise the same task budget with both runtime paths. */
    for (int i = 0; i < 2; ++i) {
        test_task *t = &task_data[i];
        prepare(t,i,1,65536,12);
        uint64_t elapsed = (uint64_t)(g_loop_states[300+i].c_ns_per_iter_ewma * t->n);
        t->desc.mode = SGPL_LOOP_DOACROSS;
        t->desc.has_doacross_profile = 1;
        t->desc.doacross_num_sync_ids = 1;
        for (int r = 0; r < 6; ++r)
            sgpl_record_doacross_serial_sample(&t->desc,0,t->n,1,0,elapsed);
        (void)sgpl_should_parallelize_doacross(&t->desc,0,t->n,1);
        sgpl_set_pending_loop_id(-1);
        memset(t->values,0,t->n*sizeof(double));
        desc[i] = (sgpl_tdg_task_desc){task,t,-1,100,1,&t->desc.loop_id};
    }
    RUN_LEVEL(desc,2,200,100);
    for (int i = 0; i < 2; ++i) {
        assert(task_data[i].calls == 1);
        for (int64_t j = 0; j < task_data[i].n; ++j) assert(task_data[i].values[j] > 0);
        free(task_data[i].values);
    }
    assert(sgpl_debug_reserved_threads() == 0);

    /* Capacity already reserved by another construct can overload a level
     * whose task count would fit the configured machine budget. */
    if (p >= 4) {
        ordinary_count = 2;
        atomic_store(&ordinary_done,0); atomic_store(&early_loops,0);
        for (int i = 0; i < 3; ++i) {
            prepare(&task_data[i],i,i==0,i==0?65536:16,12);
            desc[i] = (sgpl_tdg_task_desc){task,&task_data[i],-1,100,i==0,
                                         i==0?&task_data[i].desc.loop_id:NULL};
        }
        int held = sgpl_budget_try_reserve(p-2);
        assert(held == p-2);
        RUN_LEVEL(desc,3,300,100);
        assert(task_data[0].scope == 2 && task_data[0].spare == 0);
        for (int i = 0; i < 3; ++i) assert(task_data[i].calls == 1);
        assert(sgpl_debug_reserved_threads() == held);
        sgpl_budget_release(held);
        for (int i = 0; i < 3; ++i) free(task_data[i].values);
        assert(sgpl_debug_reserved_threads() == 0);
    }

    /* Serial choices are binding; a forced width cannot mint pool capacity. */
    sgpl_level_loop_candidate candidate = {0};
    prepare(&task_data[0], 0, 1, 1000, 8);
    candidate.task_slot = 0; candidate.loop_id = 300;
    candidate.desc = task_data[0].desc;
    candidate.representative_trip_count = 1000;
    sgpl_evaluate_level_loop_plan(&candidate, 1, 1, 1, &plan);
    assert(plan.entry_count == 1 && plan.entries[0].assigned_threads == 1);
    assert(plan.total_model_ns > 0);
    setenv("SGPL_FORCE_WIDTHS", "300:100", 1);
    sgpl_tdg_apply_forced_widths(&plan);
    assert(plan.loop_budget == 1);
    unsetenv("SGPL_FORCE_WIDTHS");
    sgpl_level_loop_pool pool;
    sgpl_loop_pool_init(&pool, &plan);
    g_tls_active_loop_pool = &pool; g_tls_current_tdg_task_slot = 0;
    assert(sgpl_loop_pool_try_acquire(300, 100) == 1);
    g_tls_active_loop_pool = NULL; g_tls_current_tdg_task_slot = -1;
    sgpl_loop_pool_destroy(&pool);
    free(task_data[0].values);
    memset(&plan, 0, sizeof(plan));
    plan.loop_budget = 3; plan.entry_count = 1;
    plan.entries[0].loop_id = 300; plan.entries[0].task_slot = 0;
    plan.entries[0].assigned_threads = 2;
    sgpl_loop_pool_init(&pool, &plan);
    pthread_barrier_t race_barrier;
    pthread_barrier_init(&race_barrier, NULL, 3);
    reserve_arg racers[2] = {{&pool,300,0,&race_barrier},{&pool,-1,0,&race_barrier}};
    pthread_t threads[2];
    for (int i = 0; i < 2; ++i) assert(pthread_create(&threads[i], NULL, reserve_racer, &racers[i]) == 0);
    for (int i = 0; i < 1000; ++i) {
        pthread_barrier_wait(&race_barrier);
        pthread_barrier_wait(&race_barrier);
        assert((racers[0].grant >= 2 ? racers[0].grant : 0) +
               (racers[1].grant >= 2 ? racers[1].grant : 0) <= 3);
        pthread_barrier_wait(&race_barrier);
        pthread_barrier_wait(&race_barrier);
        assert(atomic_load(&pool.remaining_threads) == 3);
    }
    for (int i = 0; i < 2; ++i) pthread_join(threads[i], NULL);
    pthread_barrier_destroy(&race_barrier);
    sgpl_loop_pool_destroy(&pool);
    puts("PASS: fixed workers, unique task threads, barrier progress, sequential site reuse, serial plans, forced budget, concurrent grants, balanced ledger");
}
#endif

int main(int argc, char **argv)
{
#ifndef BASELINE
    if (argc == 2 && strcmp(argv[1], "test") == 0) { structural(); return 0; }
#endif
    assert(argc >= 7);
    int o = atoi(argv[1]), l = atoi(argv[2]), n = atoi(argv[3]);
    int ordinary_n = atoi(argv[4]), ratio = atoi(argv[5]), order = atoi(argv[6]);
    int chain = argc > 7 ? atoi(argv[7]) : 12;
    int combine = argc > 8 ? atoi(argv[8]) : 0;
    int long_ordinary_n = argc > 9 ? atoi(argv[9]) : 0;
    int across_mode = argc > 10 ? atoi(argv[10]) : 0;
    int p = sgpl_configured_worker_count(), count = o + l;
    assert(count <= MAX_TASKS && l <= 8 && count > 0);
    sgpl_tdg_task_desc desc[MAX_TASKS];
    double samples[11], loop_samples[11], loop_predictions[11], resource_predictions[11];
    double level_predictions[11];
    int predicted_samples = 0;
    ordinary_count = o;
    sgpl_thread_pool_available();
    for (int i = 0; i < count; ++i)
        prepare(&task_data[i], i, i < l, i < l ? n : (i == l && long_ordinary_n ? long_ordinary_n : ordinary_n),
                i == 1 && i < l ? chain * ratio : chain);
    if (across_mode) for (int i = 0; i < l; ++i) {
        test_task *t=&task_data[i];
        uint64_t elapsed=(uint64_t)(g_loop_states[t->desc.loop_id].c_ns_per_iter_ewma*t->n);
        t->desc.mode=SGPL_LOOP_DOACROSS; t->desc.has_doacross_profile=1;
        t->desc.doacross_num_sync_ids=1;
        for(int r=0;r<6;++r) sgpl_record_doacross_serial_sample(&t->desc,0,t->n,1,0,elapsed);
        (void)sgpl_should_parallelize_doacross(&t->desc,0,t->n,1);
    }
    for (int i = 0; i < count; ++i) {
        int idx = order == 0 ? i : (i < o ? l + i : i - o);
        test_task *t = &task_data[idx];
        desc[i] = (sgpl_tdg_task_desc){task, t, idx, 100, t->loop,
                                     t->loop ? &t->desc.loop_id : NULL};
    }
    int32_t ids[8];
    int level_count = count;
    if (combine) {
        assert(o == 0);
        combined_loops = l; level_count = 1;
        for (int i = 0; i < l; ++i) ids[i] = task_data[i].desc.loop_id;
        desc[0] = (sgpl_tdg_task_desc){combined_task,NULL,0,100,l,ids};
    }
    int early = 0;
    double first_ns = 0, total_ns = 0;
    for (int r = 0; r < 14; ++r) {
        atomic_store(&ordinary_done, 0); atomic_store(&early_loops, 0);
        for (int i = 0; i < count; ++i) task_data[i].calls = 0;
        uint64_t begin = sgpl_now_ns();
        RUN_LEVEL(desc, level_count, 100 * level_count, 100);
        double elapsed = sgpl_now_ns() - begin;
        if (r==0) first_ns=elapsed;
        total_ns+=elapsed;
        assert(sgpl_debug_reserved_threads() == 0);
        uint64_t lb = UINT64_MAX, le = 0;
        double pred = 0, longest = 0, pool_cost = 0;
        for (int i = 0; i < count; ++i) {
            test_task *t = &task_data[i];
            assert(t->calls == 1 && t->values[t->n - 1] > 0);
            if (t->loop) {
                if (t->begin < lb) lb = t->begin;
                if (t->end > le) le = t->end;
                double cost = sgpl_loop_candidate_model_time_ns(&(sgpl_level_loop_candidate){
                    .loop_id=t->desc.loop_id, .desc=t->desc,
                    .representative_trip_count=t->n}, t->width);
                pred += cost;
                if (cost > longest) longest = cost;
                if (t->width >= 2 && t->desc.mode==SGPL_LOOP_DOALL) pool_cost += cost;
            }
        }
#ifdef CHECK_PHASES
        if (count > p && o && l) assert(atomic_load(&early_loops) == 0);
#endif
        if (r >= 3) {
#ifdef JOINT
            if (g_tls_joint_last_prediction_ns > 0)
                level_predictions[predicted_samples++] = g_tls_joint_last_prediction_ns;
#endif
            samples[r-3] = elapsed;
            loop_samples[r-3] = l ? le - lb : 0;
            loop_predictions[r-3] = pred;
            resource_predictions[r-3] = fmax(pool_cost, fmax(longest,
                pred / (l && task_data[0].scope > 0 ? task_data[0].scope : 1)));
            early += atomic_load(&early_loops);
        }
    }
    printf("%d,%d,%d,%d,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%d", p,o,l,n,ordinary_n,ratio,order,
            median(samples,11),median(loop_samples,11),median(loop_predictions,11),
            median(resource_predictions,11),early);
    for (int i = 0; i < l; ++i) printf(",%d,%d,%d", task_data[i].width,task_data[i].scope,task_data[i].spare);
    printf(",%.0f,%.0f",first_ns,total_ns);
#ifdef JOINT
    printf(",%lu,%lu,%lu",atomic_load(&g_joint_cache_hits),atomic_load(&g_joint_plans),atomic_load(&g_joint_reference_calls));
    printf(",%.0f,%d",predicted_samples ? median(level_predictions,predicted_samples) : 0,predicted_samples);
#ifdef SGPL_JOINT_LOW_OVERHEAD
    printf(",%lu,%lu,%lu,%lu",atomic_load(&g_joint_calibration_runs),atomic_load(&g_joint_revalidations),
           atomic_load(&g_joint_learning_ns),atomic_load(&g_joint_search_evals));
#else
    printf(",0,0,0,0");
#endif
#else
    printf(",0,0,0");
    printf(",0,0");
    printf(",0,0,0,0");
#endif
    puts("");
    for (int i = 0; i < count; ++i) free(task_data[i].values);
    return 0;
}
