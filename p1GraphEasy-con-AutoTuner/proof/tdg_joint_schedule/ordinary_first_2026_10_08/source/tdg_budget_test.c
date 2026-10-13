/* tdg_budget_test.c -- engine/budget integration checks.
 *
 * Verifies the runtime behaviour of the TDG thread-budget integration:
 *   T1  a dispatch never asks for more threads than work items (trip clamp)
 *   T2  sgpl_set_desired_threads(1,1) forces a one-thread dispatch
 *   T3  an UNREGISTERED dispatch (no pending loop id) inside an active TDG
 *       level pool shares the pool's remaining budget instead of being forced
 *       serial -- and the old forced-1 behaviour returns under
 *       SGPL_NO_UNREGISTERED_POOL_SHARE=1
 *   T4  the reservation ledger is balanced after every construct
 *
 * Build (from p1GraphEasy-con-AutoTuner):
 *   NLOPT=$(test -d .deps/nlopt && echo .deps/nlopt || echo /usr)
 *   gcc -O2 -I"$NLOPT/include" tdg_budget_test.c parallel_runtime.c \
 *       -o /tmp/tdg_budget_test -L"$NLOPT/lib" -L"$NLOPT/lib64" -lnlopt \
 *       -lpthread -lm -Wl,-rpath,"$NLOPT/lib" -Wl,-rpath,"$NLOPT/lib64"
 * Run: SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 /tmp/tdg_budget_test
 */
#include <stdint.h>
#include <stdio.h>

#include "gpu_runtime.h" /* sgpl_gpu_policy_verdict: device cost model, unit-tested here */
#include <stdlib.h>
#include <string.h>

#include "parallel_runtime.h"

static int g_seen[1024];
static int g_workers;
static int64_t g_trips;
static int32_t g_reserved_seen;

static void body(int64_t i, void *env)
{
    int w;
    (void)i;
    (void)env;
    w = sgpl_current_worker_index();
    if (w >= 0 && w < 1024 &&
        !__atomic_exchange_n(&g_seen[w], 1, __ATOMIC_RELAXED))
        __atomic_fetch_add(&g_workers, 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&g_trips, 1, __ATOMIC_RELAXED);
    g_reserved_seen = sgpl_debug_reserved_threads();
}

static void reset_counters(void)
{
    memset(g_seen, 0, sizeof(g_seen));
    g_workers = 0;
    g_trips = 0;
    g_reserved_seen = 0;
}

static int failures = 0;

static void check(const char *name, int ok, const char *detail)
{
    printf("%s %-46s %s\n", ok ? "PASS" : "FAIL", name, detail ? detail : "");
    if (!ok)
        failures++;
}

/* T1: no more threads than trips. */
static void t1(void)
{
    char buf[96];
    reset_counters();
    parallel_for_runtime(0, 2, 1, body, NULL, 0, 0);
    snprintf(buf, sizeof(buf), "workers=%d trips=%lld", g_workers, (long long)g_trips);
    check("T1 trip clamp (2 trips, 4 threads)", g_workers <= 2 && g_workers >= 1, buf);
}

/* T2: desired width clamp. */
static void t2(void)
{
    char buf[96];
    reset_counters();
    sgpl_set_desired_threads(1, 1);
    parallel_for_runtime(0, 64, 1, body, NULL, 0, 0);
    sgpl_set_desired_threads(0, 0);
    snprintf(buf, sizeof(buf), "workers=%d", g_workers);
    check("T2 desired threads (1,1) -> one worker", g_workers == 1, buf);
}

/* T3: an unregistered dispatch inside a TDG level shares the pool budget. */
struct level_env
{
    int workers;
    int64_t trips;
};

static void *level_task(void *arg)
{
    struct level_env *le = (struct level_env *)arg;

    /* Simulate an engine step that has no level entry of its own. */
    sgpl_set_pending_loop_id(-1);
    reset_counters();
    parallel_for_runtime(0, 64, 1, body, NULL, 0, 0);
    le->workers = g_workers;
    le->trips = g_trips;
    return NULL;
}

static void t3(void)
{
    sgpl_tdg_task_desc task;
    struct level_env le;
    char buf[160];
    int disabled;

    memset(&task, 0, sizeof(task));
    memset(&le, 0, sizeof(le));
    task.fn = level_task;
    task.arg = &le;
    task.profile_id = -1;
    task.static_work_units = 400000;
    task.num_loop_sites = 0;
    task.loop_site_ids = NULL;

    sgpl_run_tdg_level(&task, 1, 400000, 1000);

    disabled = (getenv("SGPL_NO_UNREGISTERED_POOL_SHARE") != NULL);
    snprintf(buf, sizeof(buf), "workers=%d trips=%lld threads=%d", le.workers,
             (long long)le.trips, sgpl_configured_worker_count());
    /* The task's blocked parent still occupies one reservation. A separate
     * two-worker loop team therefore needs a total budget of at least three. */
    if (disabled || sgpl_configured_worker_count() < 3)
        check("T3 unregistered in level, sharing DISABLED", le.workers == 1, buf);
    else
        check("T3 unregistered in level shares pool budget",
              le.workers >= 2 && le.workers <= sgpl_configured_worker_count(), buf);
}

/* T4: the ledger is balanced after everything. */
static void t4(void)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "reserved=%d", sgpl_debug_reserved_threads());
    check("T4 reservation ledger balanced", sgpl_debug_reserved_threads() == 0, buf);
}

static void nested_body(int64_t i, void *env)
{
    (void)i;
    (void)env;
}

/* T5: a nested dispatch inside a worker of a region must not fan out: it shares
 * the region's grant (observable via the nested-parallel counter). */
static void outer_body(int64_t i, void *env)
{
    (void)env;
    (void)i;
    parallel_for_runtime(0, 64, 1, nested_body, NULL, 0, 0);
}

static void t5(void)
{
    long before = sgpl_debug_nested_parallel_calls();
    long after;
    char buf[128];

    parallel_for_runtime(0, 64, 1, outer_body, NULL, 0, 0);
    after = sgpl_debug_nested_parallel_calls();

    snprintf(buf, sizeof(buf), "nested_parallel_calls=%ld outer_threads=%d",
             after - before, sgpl_configured_worker_count());
    check("T5 nested dispatch inside a worker is bounded", (after - before) == 0, buf);
}

/* T6: two REGISTERED loop sites in one level -- each site's dispatch sets its
 * pending loop id and draws the width the level plan assigned to that id (the
 * registered pool path, the one engine steps and outlined loops use).  The two
 * tasks run concurrently on the level's workers, so each site counts into its
 * own struct (the process-wide counters below are for single-dispatch tests). */
struct counters
{
    unsigned char seen[64];
    int workers;
    int64_t trips;
};

static void cbody(int64_t i, void *env)
{
    struct counters *c = (struct counters *)env;
    int w = sgpl_current_worker_index();
    (void)i;
    if (w >= 0 && w < 64 &&
        !__atomic_exchange_n(&c->seen[w], 1, __ATOMIC_RELAXED))
        __atomic_fetch_add(&c->workers, 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&c->trips, 1, __ATOMIC_RELAXED);
}

struct site_env
{
    int32_t loop_id;
    struct counters c;
};

static void *site_task(void *arg)
{
    struct site_env *se = (struct site_env *)arg;

    sgpl_set_pending_loop_id(se->loop_id);
    memset(&se->c, 0, sizeof(se->c));
    parallel_for_runtime(0, 64, 1, cbody, &se->c, 0, 0);
    return NULL;
}

static void t6(void)
{
    static const int32_t ids0[1] = {101};
    static const int32_t ids1[1] = {102};
    sgpl_tdg_task_desc tasks[2];
    struct site_env e0;
    struct site_env e1;
    char buf[160];

    memset(&e0, 0, sizeof(e0));
    memset(&e1, 0, sizeof(e1));
    e0.loop_id = 101;
    e1.loop_id = 102;

    memset(tasks, 0, sizeof(tasks));
    tasks[0].fn = site_task;
    tasks[0].arg = &e0;
    tasks[0].profile_id = 101;
    tasks[0].static_work_units = 400000;
    tasks[0].num_loop_sites = 1;
    tasks[0].loop_site_ids = ids0;
    tasks[1].fn = site_task;
    tasks[1].arg = &e1;
    tasks[1].profile_id = 102;
    tasks[1].static_work_units = 400000;
    tasks[1].num_loop_sites = 1;
    tasks[1].loop_site_ids = ids1;

    sgpl_run_tdg_level(tasks, 2, 800000, 1000);

    snprintf(buf, sizeof(buf), "site101 workers=%d trips=%lld; site102 workers=%d trips=%lld",
             e0.c.workers, (long long)e0.c.trips, e1.c.workers, (long long)e1.c.trips);
    check("T6 registered sites draw their planned width",
          e0.c.trips == 64 && e1.c.trips == 64 && e0.c.workers >= 1 &&
              e1.c.workers >= 1 &&
              (sgpl_configured_worker_count() < 2 ||
               e0.c.workers + e1.c.workers >= 2),
          buf);
}

/* T7: a single-task level whose site is PROFILED gets planned (the single-site
 * mode): one level worker runs the task and the site's dispatch draws the width
 * the plan assigned.  This is the engine-step shape: one step per driver. */
static void train_site(int32_t loop_id, int64_t trips, double ns_per_iter)
{
    sgpl_loop_profile_desc desc;
    int i;

    memset(&desc, 0, sizeof(desc));
    desc.loop_id = loop_id;
    desc.mode = SGPL_LOOP_DOALL;
    desc.runtime_kind = SGPL_RUNTIME_PLAIN;
    desc.env_size = 0;
    desc.debug_name = "tdg-budget-test";

    for (i = 0; i < 6; ++i)
        sgpl_record_doall_serial_sample(&desc, 0, trips, 1,
                                        (uint64_t)((double)trips * ns_per_iter));
}

static void t7(void)
{
    static const int32_t ids[1] = {210};
    sgpl_tdg_task_desc task;
    struct site_env se;

    memset(&se, 0, sizeof(se));
    se.loop_id = 210;
    char buf[160];

    train_site(210, 200000, 250.0);

    memset(&task, 0, sizeof(task));
    task.fn = site_task;
    task.arg = &se;
    task.profile_id = 210;
    task.static_work_units = 200000;
    task.num_loop_sites = 1;
    task.loop_site_ids = ids;

    sgpl_run_tdg_level(&task, 1, 200000, 1000);

    snprintf(buf, sizeof(buf), "site210 workers=%d trips=%lld threads=%d",
             se.c.workers, (long long)se.c.trips, sgpl_configured_worker_count());
    check("T7 single-site level plans its site",
          se.c.trips == 64 &&
              (sgpl_configured_worker_count() < 3 || se.c.workers >= 2),
          buf);
}

static void test_gpu_cost_model(void);

int main(void)
{
    printf("tdg_budget_test: threads=%d\n", sgpl_configured_worker_count());
    test_gpu_cost_model();
    t1();
    t2();
    t3();
    t6();
    t7();
    t5();
    t4();
    printf("%s (%d failure%s)\n", failures == 0 ? "ALL PASS" : "FAILURES", failures,
           failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}

/* T8/T9: device cost model (pure, so it runs everywhere).  DOALL: below the
 * launch break-even the host pool wins.  DOACROSS: the wave kernel pays one
 * cooperative grid sync per wave, so a distance-1 recurrence (one sync per
 * iteration) is a wave storm and the CPU doacross path is the right verdict,
 * while a large distance offloads. */
static void test_gpu_cost_model(void)
{
    char buf[96];
    int v;

    v = sgpl_gpu_policy_verdict(4095, 0, 0, 4096, 8192);
    snprintf(buf, sizeof(buf), "trip=4095 verdict=%d", v);
    check("T8 doall below min_trips -> CPU", v == SGPL_GPU_SMALL_TRIPS, buf);

    v = sgpl_gpu_policy_verdict(4096, 0, 0, 4096, 8192);
    snprintf(buf, sizeof(buf), "trip=4096 verdict=%d", v);
    check("T8 doall at min_trips -> device", v == SGPL_GPU_OFFLOAD, buf);

    v = sgpl_gpu_policy_verdict(200000, 1, 1, 4096, 8192);
    snprintf(buf, sizeof(buf), "trip=200000 dist=1 waves=200000 verdict=%d", v);
    check("T9 doacross distance 1 -> wave storm", v == SGPL_GPU_WAVE_STORM, buf);

    v = sgpl_gpu_policy_verdict(200000, 1, 64, 4096, 8192);
    snprintf(buf, sizeof(buf), "trip=200000 dist=64 waves=3125 verdict=%d", v);
    check("T9 doacross distance 64 -> device", v == SGPL_GPU_OFFLOAD, buf);

    v = sgpl_gpu_policy_verdict(64, 1, 64, 4096, 8192);
    snprintf(buf, sizeof(buf), "trip=64 verdict=%d", v);
    check("T9 doacross below min_trips -> CPU", v == SGPL_GPU_SMALL_TRIPS, buf);

    /* T10: the engine-step gate (one thread per owned source/row).  Same
     * boundary discipline: just below the floor stays on the CPU partitions,
     * at the floor and above runs on the device, and a non-positive floor
     * disables the gate entirely (the harness forces device execution that
     * way). */
    v = sgpl_gpu_engine_step_verdict(1999999, 2000000);
    snprintf(buf, sizeof(buf), "arcs=1999999 min=2000000 verdict=%d", v);
    check("T10 engine step just below floor -> CPU", v == SGPL_GPU_SMALL_TRIPS, buf);

    v = sgpl_gpu_engine_step_verdict(2000000, 2000000);
    snprintf(buf, sizeof(buf), "arcs=2000000 min=2000000 verdict=%d", v);
    check("T10 engine step at floor -> device", v == SGPL_GPU_OFFLOAD, buf);

    v = sgpl_gpu_engine_step_verdict(0, 0);
    snprintf(buf, sizeof(buf), "arcs=0 min=0 verdict=%d", v);
    check("T10 engine step gate disabled -> device", v == SGPL_GPU_OFFLOAD, buf);

    v = sgpl_gpu_engine_step_verdict(0, 2000000);
    snprintf(buf, sizeof(buf), "arcs=0 min=2000000 verdict=%d", v);
    check("T10 empty step -> CPU", v == SGPL_GPU_SMALL_TRIPS, buf);
}
