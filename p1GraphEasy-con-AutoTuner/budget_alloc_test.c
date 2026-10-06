/* budget_alloc_test.c -- universal thread-budget validation, timing version.
 *
 * The model under test (sgpl_run_tdg_level + sgpl_choose_tdg_threads_with_loop_budget):
 *   level_budget   = threads available to the level
 *   chosen_threads = the level's own width (one per task)
 *   loop_budget    = level_budget - chosen_threads          <- "subtract the level width"
 *   the plan then distributes loop_budget across the level's task loops, choosing the
 *   allocation that minimises the modelled total time.
 *
 * This program builds a level with TWO registered loop sites over real work -- the
 * light site and the heavy site differ by SGPL_ALLOC_RATIO -- runs the level for
 * SGPL_ALLOC_ROUNDS rounds, and reports the wall time plus each site's observed
 * worker/trip counts.  SGPL_FORCE_WIDTHS pins the two allocations, which is how the
 * sweep enumerates every split; with it unset the model's own plan runs, and
 * [tdg.plan] prints the allocation it chose.
 *
 * Build (see budget_alloc_sweep.sh):
 *   gcc -O2 budget_alloc_test.c parallel_runtime.c gpu_runtime.c roaring_bitmap.cpp
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parallel_runtime.h"

#define SITE_LIGHT 101
#define SITE_HEAVY 102

static long g_rounds = 20;
static int64_t g_trips = 1 << 20; /* per site per round */
static int g_ratio = 3;           /* heavy does ~ratio x the arithmetic of light */

static double g_light_sink;
static double g_heavy_sink;
static int g_light_workers;
static int g_heavy_workers;
static int64_t g_light_trips;
static int64_t g_heavy_trips;
static unsigned char g_light_seen[256];
static unsigned char g_heavy_seen[256];

static double light_chain(double x)
{
    return x * 1.0000001 + 0.5;
}

static double heavy_chain(double x)
{
    double s = x * 0.5;
    int k;
    /* The same dependent, non-vectorisable shape the DSL fixtures use:
     * a division chain whose length is the light:heavy ratio. */
    for (k = 0; k < g_ratio * 6; ++k)
    {
        s = s + x / (1.0 + s * s + (double)k);
        x = x * 1.0000001 + s;
    }
    return s;
}

static void light_body(int64_t i, void *env)
{
    double x = (double)(i + 1) * 0.37;
    double r;
    int w;
    (void)env;
    r = light_chain(x);
    r = light_chain(r);
    if (r == -1.0)
        g_light_sink = r;
    w = sgpl_current_worker_index();
    if (w >= 0 && w < 256 && !__atomic_exchange_n(&g_light_seen[w], 1, __ATOMIC_RELAXED))
        __atomic_fetch_add(&g_light_workers, 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&g_light_trips, 1, __ATOMIC_RELAXED);
}

static void heavy_body(int64_t i, void *env)
{
    double x = (double)(i + 1) * 0.37;
    double r;
    int w;
    (void)env;
    r = heavy_chain(x);
    if (r == -1.0)
        g_heavy_sink = r;
    w = sgpl_current_worker_index();
    if (w >= 0 && w < 256 && !__atomic_exchange_n(&g_heavy_seen[w], 1, __ATOMIC_RELAXED))
        __atomic_fetch_add(&g_heavy_workers, 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&g_heavy_trips, 1, __ATOMIC_RELAXED);
}


static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

/* Give the level planner what a real build gives it: a stable, sampled cost
 * estimate per loop site.  The costs are measured here (not assumed), so the
 * model's inputs are accurate; any allocation error is then the model's. */
static double measure_ns_per_iter(void (*body)(int64_t, void *), int64_t trips)
{
    uint64_t t0 = now_ns();
    int64_t i;
    for (i = 0; i < trips; ++i)
        body(i, NULL);
    return (double)(now_ns() - t0) / (double)trips;
}

static void train_site(int32_t loop_id, int64_t trips, double ns_per_iter)
{
    sgpl_loop_profile_desc desc;
    int i;

    memset(&desc, 0, sizeof(desc));
    desc.loop_id = loop_id;
    desc.mode = SGPL_LOOP_DOALL;
    desc.runtime_kind = SGPL_RUNTIME_PLAIN;
    desc.env_size = 0;
    desc.debug_name = "budget-alloc-test";
    for (i = 0; i < 6; ++i)
        sgpl_record_doall_serial_sample(&desc, 0, trips, 1,
                                        (uint64_t)((double)trips * ns_per_iter));
}

struct site_env
{
    int32_t loop_id;
    void (*body)(int64_t, void *);
    uint64_t last_ns; /* time this task's own body took */
};

static void *site_task(void *arg)
{
    struct site_env *se = (struct site_env *)arg;
    uint64_t t0;

    sgpl_set_pending_loop_id(se->loop_id);
    t0 = now_ns();
    parallel_for_runtime(0, g_trips, 1, se->body, NULL, 0, 0);
    se->last_ns = now_ns() - t0;
    return NULL;
}

int main(void)
{
    static const int32_t ids_light[1] = {SITE_LIGHT};
    static const int32_t ids_heavy[1] = {SITE_HEAVY};
    sgpl_tdg_task_desc tasks[2];
    struct site_env e_light;
    struct site_env e_heavy;
    const char *rounds_env = getenv("SGPL_ALLOC_ROUNDS");
    const char *trips_env = getenv("SGPL_ALLOC_TRIPS");
    const char *ratio_env = getenv("SGPL_ALLOC_RATIO");
    const char *force = getenv("SGPL_FORCE_WIDTHS");
    int64_t work;
    uint64_t t0, t1;
    long r;
    char buf[512];

    if (rounds_env && *rounds_env)
        g_rounds = atol(rounds_env);
    if (trips_env && *trips_env)
        g_trips = atoll(trips_env);
    if (ratio_env && *ratio_env)
        g_ratio = atoi(ratio_env);

    memset(&e_light, 0, sizeof(e_light));
    memset(&e_heavy, 0, sizeof(e_heavy));
    e_light.loop_id = SITE_LIGHT;
    e_light.body = light_body;
    e_heavy.loop_id = SITE_HEAVY;
    e_heavy.body = heavy_body;

    memset(tasks, 0, sizeof(tasks));
    tasks[0].fn = site_task;
    tasks[0].arg = &e_light;
    tasks[0].profile_id = SITE_LIGHT;
    tasks[0].static_work_units = g_trips * 3;
    tasks[0].num_loop_sites = 1;
    tasks[0].loop_site_ids = ids_light;
    tasks[1].fn = site_task;
    tasks[1].arg = &e_heavy;
    tasks[1].profile_id = SITE_HEAVY;
    tasks[1].static_work_units = g_trips * 3 * (int64_t)g_ratio;
    tasks[1].num_loop_sites = 1;
    tasks[1].loop_site_ids = ids_heavy;

    work = tasks[0].static_work_units + tasks[1].static_work_units;

    memset(g_light_seen, 0, sizeof(g_light_seen));
    memset(g_heavy_seen, 0, sizeof(g_heavy_seen));

    /* measure the real serial cost of each site, then feed the planner */
    {
        int64_t probe = g_trips < 65536 ? g_trips : 65536;
        double light_ns = measure_ns_per_iter(light_body, probe);
        double heavy_ns = measure_ns_per_iter(heavy_body, probe / (g_ratio > 0 ? g_ratio : 1) + 1);
        train_site(SITE_LIGHT, g_trips, light_ns);
        train_site(SITE_HEAVY, g_trips, heavy_ns);
        printf("COSTS light_ns_per_iter=%.3f heavy_ns_per_iter=%.3f\n", light_ns, heavy_ns);
        fflush(stdout);
    }

    t0 = now_ns();
    for (r = 0; r < g_rounds; ++r)
        sgpl_run_tdg_level(tasks, 2, work, work);
    t1 = now_ns();

    snprintf(buf, sizeof(buf),
             "RESULT rounds=%ld trips=%lld ratio=%d threads=%d forced=%s time_ns=%llu "
             "light_workers=%d light_trips=%lld heavy_workers=%d heavy_trips=%lld "
             "last_light_ns=%llu last_heavy_ns=%llu",
             g_rounds, (long long)g_trips, g_ratio, sgpl_configured_worker_count(),
             force ? force : "-", (unsigned long long)(t1 - t0),
             g_light_workers, (long long)g_light_trips,
             g_heavy_workers, (long long)g_heavy_trips,
             (unsigned long long)e_light.last_ns,
             (unsigned long long)e_heavy.last_ns);
    printf("%s\n", buf);
    return 0;
}
