#define _GNU_SOURCE
#include "parallel_runtime.h"

#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <nlopt.h>

typedef struct RoaringBitmap RoaringBitmap;
extern RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_capacity);
extern RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *prototype);
extern void roaring_bitmap_or_inplace(RoaringBitmap *dst, RoaringBitmap *src);
extern void roaring_bitmap_free(RoaringBitmap *bm);
extern void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **originals,
                                                      RoaringBitmap **replacements,
                                                      int32_t count);
extern void roaring_bitmap_clear_thread_local_overrides(void);

#define SYNC_WINDOW 4096
#define SGPL_MAX_PROFILED_LOOPS 4096
#define SGPL_MAX_LAUNCH_KEYS 128
#define SGPL_DOACROSS_WIDTH_PROFILES 16
#define SGPL_TDG_MAX_REPORTED_THREADS 1024
#define SGPL_THREAD_BUDGET_STACK_DEPTH 16
#define SGPL_MAX_TDG_LEVEL_LOOPS 256
#define SGPL_APPEND_PARALLEL_COPY_THRESHOLD 16384

static const double SGPL_TDG_UNIT_COST_NS = 250.0;
static const double SGPL_TDG_HT_EFFICIENCY = 0.42;
static const double SGPL_TDG_MIN_GAIN_FRACTION = 0.12;
static const int64_t SGPL_TDG_MIN_WORK_FOR_HT = 64;
static int sgpl_tdg_runtime_threads = 0;
static int sgpl_tdg_physical_threads = 0;
static int sgpl_tdg_allow_hyperthread = 0;
static int sgpl_tdg_ht_disallow_threshold = 1;
static double sgpl_tdg_unit_cost_ns = SGPL_TDG_UNIT_COST_NS;
static int sgpl_tdg_is_initialized = 0;
static atomic_int g_sgpl_reserved_threads = 0;

/* Debug introspection: current global budget reservations.  Used by the
 * fork/join resources-and-budget tests to verify the ledger is balanced. */
int32_t sgpl_debug_reserved_threads(void) {
    return (int32_t)atomic_load(&g_sgpl_reserved_threads);
}

static int runtime_debug_enabled(void);
static int runtime_iter_debug_enabled(void);
static int sgpl_runtime_thread_count(void);
static int tdg_debug_enabled(void);
static int budget_debug_enabled(void);
static void sgpl_record_tdg_profile_sample(const sgpl_tdg_task_desc *task,
                                           int32_t idx,
                                           double elapsed_ns,
                                           const char *path);
static int32_t sgpl_budget_available_threads(void);
static int32_t sgpl_budget_try_reserve(int32_t requested);
static void sgpl_budget_release(int32_t granted);
static struct sgpl_doacross_state *sgpl_alloc_doacross_state(int64_t start, int32_t num_ids);
static void sgpl_free_doacross_state(struct sgpl_doacross_state *state);
static void doacross_init_state(struct sgpl_doacross_state *state, int64_t start);
static void sgpl_commit_doacross_profile(struct sgpl_doacross_state *state);
static void sgpl_prepare_doacross_profile(struct sgpl_doacross_state *state,
                                          int32_t threads, int64_t trips);
static int32_t sgpl_trip_count_bucket(int64_t trip_count);

static int sgpl_isdigit_strict(const char *s, int64_t *value)
{
    if (!s || *s == '\0')
        return 0;
    const char *start = s;

    while (*s)
    {
        if (!isdigit((unsigned char)*s))
            return 0;
        ++s;
    }

    if (value)
        *value = atoll(start);

    return 1;
}

static int sgpl_tdg_env_enabled(const char *name, int default_value)
{
    const char *raw = getenv(name);
    if (!raw)
        return default_value;

    if (strcmp(raw, "0") == 0 || strcmp(raw, "false") == 0 || strcmp(raw, "FALSE") == 0)
        return 0;
    if (strcmp(raw, "1") == 0 || strcmp(raw, "true") == 0 || strcmp(raw, "TRUE") == 0)
        return 1;
    return default_value;
}

static int tdg_debug_enabled(void)
{
    static int cached = -1;
    if (cached != -1)
        return cached;

    cached = sgpl_tdg_env_enabled("SGPL_TDG_DEBUG", 0);
    return cached;
}

static int budget_debug_enabled(void)
{
    static int cached = -1;
    if (cached != -1)
        return cached;

    cached = sgpl_tdg_env_enabled("SGPL_BUDGET_DEBUG", 0) || tdg_debug_enabled();
    return cached;
}

/* Kill switches for the engine-budget integration: an unregistered caller
 * (engine step without a level entry) shares the active level pool's remaining
 * budget instead of being forced serial, and a parallel region publishes its
 * grant as a budget scope for nested work. */
static int unregistered_pool_share_disabled(void)
{
    static int cached = -1;
    if (cached == -1)
        cached = sgpl_tdg_env_enabled("SGPL_NO_UNREGISTERED_POOL_SHARE", 0);
    return cached;
}

static int region_scope_disabled(void)
{
    static int cached = -1;
    if (cached == -1)
        cached = sgpl_tdg_env_enabled("SGPL_NO_REGION_SCOPE", 0);
    return cached;
}

static int sgpl_tdg_read_core_ids_once(void)
{
    DIR *cpu_dir = NULL;
    struct dirent *entry = NULL;
    int seen[SGPL_TDG_MAX_REPORTED_THREADS] = {0};
    int physical = 0;
    int found = 0;

    cpu_dir = opendir("/sys/devices/system/cpu");
    if (!cpu_dir)
        return -1;

    while ((entry = readdir(cpu_dir)) != NULL)
    {
        const char *name = entry->d_name;
        if (strncmp(name, "cpu", 3) != 0)
            continue;

        const char *id = name + 3;
        int64_t cpu_idx = 0;
        if (!sgpl_isdigit_strict(id, &cpu_idx))
            continue;

        (void)cpu_idx;
        char path[256];
        char buf[64];
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/%s/topology/core_id", name);

        FILE *fp = fopen(path, "r");
        if (!fp)
            continue;

        if (!fgets(buf, sizeof(buf), fp))
        {
            fclose(fp);
            continue;
        }

        fclose(fp);
        long core_id = -1;
        char *end = NULL;
        core_id = strtol(buf, &end, 10);
        if (end == buf)
            continue;

        if (core_id >= 0 && core_id < SGPL_TDG_MAX_REPORTED_THREADS)
        {
            if (!seen[core_id])
            {
                seen[core_id] = 1;
                physical++;
                found = 1;
            }
        }
    }

    closedir(cpu_dir);
    if (!found)
        return -1;
    return physical;
}

static int sgpl_tdg_detect_physical_threads(void)
{
    int physical = sgpl_tdg_read_core_ids_once();
    if (physical > 0)
        return physical;
    return (sgpl_tdg_runtime_threads + 1) / 2;
}

static void sgpl_tdg_init_runtime_config(void)
{
    if (sgpl_tdg_is_initialized)
        return;

    sgpl_tdg_runtime_threads = sgpl_runtime_thread_count();
    sgpl_tdg_physical_threads = sgpl_tdg_detect_physical_threads();
    if (sgpl_tdg_physical_threads <= 0)
        sgpl_tdg_physical_threads = sgpl_tdg_runtime_threads;
    if (sgpl_tdg_physical_threads > sgpl_tdg_runtime_threads)
        sgpl_tdg_physical_threads = sgpl_tdg_runtime_threads;

    sgpl_tdg_allow_hyperthread = sgpl_tdg_env_enabled("SGPL_ALLOW_HT", 0);
    sgpl_tdg_ht_disallow_threshold = sgpl_tdg_env_enabled("SGPL_HT_DISALLOW", 0);
    {
        const char *override = getenv("SGPL_TDG_UNIT_COST_NS");
        if (override)
        {
            double v = atof(override);
            if (v > 0.0)
                sgpl_tdg_unit_cost_ns = v;
        }
        else
        {
            sgpl_tdg_unit_cost_ns = SGPL_TDG_UNIT_COST_NS;
        }
    }
    sgpl_tdg_is_initialized = 1;

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.init] logical_threads=%d physical_threads=%d allow_ht=%d ht_disallow=%d unit_cost_ns=%.2f alpha=%.2f min_gain=%.2f min_work_for_ht=%lld\n",
                sgpl_tdg_runtime_threads,
                sgpl_tdg_physical_threads,
                sgpl_tdg_allow_hyperthread,
                sgpl_tdg_ht_disallow_threshold,
                sgpl_tdg_unit_cost_ns,
                SGPL_TDG_HT_EFFICIENCY,
                SGPL_TDG_MIN_GAIN_FRACTION,
                (long long)SGPL_TDG_MIN_WORK_FOR_HT);
    }
}

static _Thread_local int32_t g_tls_thread_budget_depth = 0;
static _Thread_local int32_t g_tls_thread_budget_stack[SGPL_THREAD_BUDGET_STACK_DEPTH] = {0};
static _Thread_local int32_t g_tls_thread_budget_scope_reserved = 0;

int32_t sgpl_current_thread_budget(void)
{
    if (g_tls_thread_budget_depth <= 0)
        return 0;

    return g_tls_thread_budget_stack[g_tls_thread_budget_depth - 1];
}

void sgpl_push_thread_budget(int32_t max_threads)
{
    int32_t requested = max_threads;
    if (requested <= 1)
        requested = 1;

    if (requested > sgpl_runtime_thread_count())
        requested = sgpl_runtime_thread_count();

    if (g_tls_thread_budget_depth > 0)
    {
        int32_t parent = g_tls_thread_budget_stack[g_tls_thread_budget_depth - 1];
        if (parent > 0 && parent < requested)
            requested = parent;
    }

    if (g_tls_thread_budget_depth < SGPL_THREAD_BUDGET_STACK_DEPTH)
    {
        g_tls_thread_budget_stack[g_tls_thread_budget_depth++] = requested;
        if (budget_debug_enabled())
        {
            fprintf(stderr,
                    "[budget.context] push id=%d max_threads=%d stack_depth=%d\n",
                    g_tls_thread_budget_depth - 1,
                    requested,
                    g_tls_thread_budget_depth);
        }
    }
    else
    {
        if (budget_debug_enabled())
        {
            fprintf(stderr,
                    "[budget.context] push-depth-overflow max_threads=%d stack_depth=%d\n",
                    requested,
                    g_tls_thread_budget_depth);
        }
    }
}

void sgpl_pop_thread_budget(void)
{
    if (g_tls_thread_budget_depth > 0)
        g_tls_thread_budget_depth--;
}

static int32_t sgpl_thread_budget_cap(void)
{
    int32_t cap = sgpl_runtime_thread_count();
    int32_t requested = sgpl_current_thread_budget();

    if (requested > 0 && requested < cap)
        cap = requested;
    if (cap < 1)
        cap = 1;
    return cap;
}

static int32_t sgpl_thread_budget_share_for_scope(int32_t total)
{
    int32_t scope_threads = g_tls_thread_budget_scope_reserved;

    if (scope_threads > 1)
    {
        total = total / scope_threads;
        if (total < 1)
            total = 1;
    }

    return total;
}

static int32_t sgpl_budget_available_threads(void)
{
    int32_t total = sgpl_thread_budget_cap();
    int32_t available = 0;

    if (g_tls_thread_budget_scope_reserved > 0)
    {
        total = sgpl_thread_budget_share_for_scope(total);
        available = total;
    }
    else
    {
        int32_t reserved = atomic_load(&g_sgpl_reserved_threads);
        available = total - reserved;
    }

    if (available < 1)
        available = 1;
    return (int32_t)available;
}

static int32_t sgpl_budget_try_reserve(int32_t requested)
{
    int32_t total = sgpl_thread_budget_cap();
    int32_t scope_threads = g_tls_thread_budget_scope_reserved;
    int current = atomic_load(&g_sgpl_reserved_threads);

    if (requested <= 1 || total <= 1)
        return 1;
    if (requested > total)
        requested = total;
    if (scope_threads > 0)
    {
        total = sgpl_thread_budget_share_for_scope(total);
        if (requested > total)
            requested = total;
        if (requested <= 1)
            return 1;
        return requested;
    }

    for (;;)
    {
        int available = total - current;
        int granted = requested;

        if (available <= 1)
        {
            if (budget_debug_enabled())
            {
                fprintf(stderr,
                        "[budget.reserve] requested=%d granted=1 total=%d available_before=%d reserved_before=%d reserved_after=%d reason=insufficient-available\n",
                        requested,
                        total,
                        available,
                        current,
                        current);
            }
            return 1;
        }
        if (granted > available)
            granted = available;
        if (granted <= 1)
        {
            if (budget_debug_enabled())
            {
                fprintf(stderr,
                        "[budget.reserve] requested=%d granted=1 total=%d available_before=%d reserved_before=%d reserved_after=%d reason=clamped-to-serial\n",
                        requested,
                        total,
                        available,
                        current,
                        current);
            }
            return 1;
        }

        if (atomic_compare_exchange_weak(&g_sgpl_reserved_threads, &current, current + granted))
        {
            if (budget_debug_enabled())
            {
                fprintf(stderr,
                        "[budget.reserve] requested=%d granted=%d total=%d available_before=%d reserved_before=%d reserved_after=%d\n",
                        requested,
                        granted,
                        total,
                        available,
                        current,
                        current + granted);
            }
            return granted;
        }
    }
}

static void sgpl_budget_release(int32_t granted)
{
    if (g_tls_thread_budget_scope_reserved > 0)
        return;

    if (granted <= 1)
        return;
    {
        int before = atomic_fetch_sub(&g_sgpl_reserved_threads, granted);
        int after = before - granted;
        if (budget_debug_enabled())
        {
            fprintf(stderr,
                    "[budget.release] released=%d reserved_before=%d reserved_after=%d total=%d available_after=%d\n",
                    granted,
                    before,
                    after,
                    sgpl_runtime_thread_count(),
                    sgpl_runtime_thread_count() - after);
        }
    }
}

/* Each worker owns one row until join. Padding keeps hot counters apart. */
typedef struct sgpl_doacross_sync_lane_sample
{
    const sgpl_loop_profile_desc *desc;
    uint64_t wait_ns, post_ns, waits, posts, samples;
    unsigned char padding[80];
} sgpl_doacross_sync_lane_sample;

typedef struct sgpl_doacross_state
{
    atomic_long *completion;
    int32_t num_ids;
    int32_t sync_window;
    int64_t loop_start;
    int32_t profile_threads;
    int32_t profile_trip_bucket;
    sgpl_doacross_sync_lane_sample *profile_lanes;
} sgpl_doacross_state;

typedef void (*loop_body_fn)(int64_t i, void *env);

/* Range form of the same body: runs `count` iterations starting at `lo`,
 * stepping by `step`.  The compiler emits this alongside the per-index form
 * and it is the reason parallel execution can be faster than serial at all --
 * see the comment on the chunk loop in worker_main(). */
typedef void (*loop_range_fn)(int64_t lo, int64_t count, int64_t step, void *env);

typedef struct
{
    int64_t start;
    int64_t end;
    int64_t step;
    loop_body_fn body;
    loop_range_fn range_body;
    void *env;
    int tid;
    int nthreads;
    int ncpus;
    sgpl_doacross_state *doacross_state;
    RoaringBitmap **override_originals;
    RoaringBitmap **override_replacements;
    int32_t num_override_targets;
} workers_args_t;

/* Range entry point for the loop currently being dispatched.
 *
 * Carried in a thread-local rather than through every launch signature, the
 * same way g_tls_pending_loop_id already is: the compiler calls
 * sgpl_set_pending_range_body() immediately before parallel_for_runtime(), and
 * the launch path consumes and clears it.  It is always safe to ignore -- the
 * per-index body computes the identical result -- so a mismatched or stale
 * value can only cost speed, never correctness, and the consumer clears the
 * slot on every path including the serial fallback. */
static _Thread_local loop_range_fn g_tls_pending_range_body = NULL;

void sgpl_set_pending_range_body(void *fn)
{
    g_tls_pending_range_body = (loop_range_fn)fn;
}

static loop_range_fn sgpl_take_pending_range_body(void)
{
    loop_range_fn fn = g_tls_pending_range_body;
    g_tls_pending_range_body = NULL;
    return fn;
}

typedef struct
{
    int32_t **sources;
    const int64_t *offsets;
    int32_t *destination;
} sgpl_append_copy_context;

static _Thread_local int g_tls_is_pool_worker = 0;
static _Thread_local int32_t g_tls_worker_index = 0;

static pthread_barrier_t g_sgpl_thread_pool_barrier;
static pthread_mutex_t g_sgpl_thread_pool_run_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t g_sgpl_thread_pool_once = PTHREAD_ONCE_INIT;
static pthread_t *g_sgpl_thread_pool_threads = NULL;
static int g_sgpl_thread_pool_size = 0;
static atomic_int g_sgpl_thread_pool_shutdown = 0;
static workers_args_t *g_sgpl_thread_pool_job_args = NULL;
static int g_sgpl_thread_pool_job_nthreads = 0;

typedef struct
{
    const sgpl_tdg_task_desc *tasks;
    atomic_int next_index;
    atomic_int active_count;
    atomic_int max_active;
    int32_t task_count;
    void *loop_pool;
} sgpl_tdg_level_state;

typedef struct
{
    sgpl_tdg_level_state *state;
    int32_t loop_threads;
    int32_t scope_threads;
    int32_t first_index;
} sgpl_tdg_worker_arg;

typedef struct sgpl_level_loop_plan_entry
{
    int32_t task_slot;
    int32_t loop_id;
    int32_t assigned_threads;
    int32_t active_threads;
    double c_rank_ns;
    double model_ns;
} sgpl_level_loop_plan_entry;

typedef struct sgpl_level_loop_plan
{
    int32_t loop_budget;
    int32_t remaining_threads;
    int32_t idle_threads;
    int32_t entry_count;
    int32_t dominant_loop_id;
    double dominant_model_ns;
    double total_model_ns;
    int32_t slack_threads;
    sgpl_level_loop_plan_entry entries[SGPL_MAX_TDG_LEVEL_LOOPS];
} sgpl_level_loop_plan;

typedef struct sgpl_level_loop_pool
{
    int32_t active;
    int32_t task_count;
    int32_t loop_budget;
    atomic_int remaining_threads;
    int32_t entry_count;
    int32_t dominant_loop_id;
    double dominant_model_ns;
    pthread_mutex_t lock;
    sgpl_level_loop_plan_entry entries[SGPL_MAX_TDG_LEVEL_LOOPS];
} sgpl_level_loop_pool;

typedef struct sgpl_level_loop_candidate
{
    int32_t task_slot;
    int32_t loop_id;
    sgpl_loop_profile_desc desc;
    double c_rank_ns;
    double serial_model_ns;
    int64_t representative_trip_count;
} sgpl_level_loop_candidate;

/* Bounded cache: an unobserved/evicted width uses seeds, never another width. */
typedef struct sgpl_doacross_width_profile
{
    int32_t threads, trip_bucket, runtime_kind, waits, posts, sync_ids, priv_targets;
    int64_t env_size;
    uint32_t samples;
    uint64_t stamp;
    double wait_ns, post_ns;
} sgpl_doacross_width_profile;

typedef struct
{
    uint32_t serial_samples;
    double c_ns_per_iter_ewma;
    uint64_t invocation_count;
    double observed_trip_count_ewma;
    int64_t last_observed_trip_count;
    double c_dep_ns_per_iter_ewma;
    double c_ind_ns_per_iter_ewma;
    sgpl_doacross_width_profile sync_widths[SGPL_DOACROSS_WIDTH_PROFILES];
    uint64_t sync_width_clock;
    uint32_t doacross_serial_samples;
    atomic_uint doacross_sync_samples;
    int32_t regime_valid;
    int32_t regime_mode;
    int32_t regime_runtime_kind;
    int32_t regime_trip_bucket;
    int64_t regime_env_size;
    int32_t regime_num_priv_targets;
    int32_t regime_waits_per_iter;
    int32_t regime_posts_per_iter;
    int32_t regime_doacross_num_sync_ids;
    int32_t c_sampling_state;
    uint32_t c_batch_samples;
    uint32_t c_probe_skip_counter;
    uint32_t c_consecutive_drift;
    uint32_t c_sampling_epoch;
    atomic_uint sync_sampling_epoch;
    atomic_int decision_cache_valid;
    int32_t cached_choose_parallel;
    int32_t cached_threads;
    int32_t cached_budget_bucket;
    int32_t cached_trip_bucket;
    uint32_t cached_sampling_epoch;
    uint32_t cached_sync_sampling_epoch;
    int32_t cached_runtime_kind;
    int32_t cached_mode;
    int64_t cached_env_size;
    int32_t cached_num_priv_targets;
    int32_t cached_waits_per_iter;
    int32_t cached_posts_per_iter;
    int32_t cached_doacross_num_sync_ids;
} sgpl_loop_runtime_state;

typedef struct
{
    int in_use;
    int ephemeral;
    int mode;
    int runtime_kind;
    int threads;
    int64_t env_size;
    int32_t num_priv_targets;
    int32_t doacross_num_sync_ids;
    double path_ns;
} sgpl_launch_path_cost_entry;

typedef struct sgpl_launch_overhead_detail
{
    double thread_ns;
    double path_ns;
    double total_ns;
} sgpl_launch_overhead_detail;

static void sgpl_bind_level_loop_pool(sgpl_level_loop_pool *pool,
                                      sgpl_level_loop_pool **saved_pool,
                                      int32_t *saved_pending_loop);
static void sgpl_unbind_level_loop_pool(sgpl_level_loop_pool *saved_pool,
                                        int32_t saved_pending_loop);
static sgpl_launch_overhead_detail sgpl_get_launch_overhead_detail(const sgpl_loop_profile_desc *desc, int threads);

static sgpl_loop_runtime_state g_loop_states[SGPL_MAX_PROFILED_LOOPS];
static pthread_mutex_t g_doacross_profile_lock = PTHREAD_MUTEX_INITIALIZER;
static double g_launch_thread_costs[2][SGPL_TDG_MAX_REPORTED_THREADS];
static unsigned char g_launch_thread_cost_valid[2][SGPL_TDG_MAX_REPORTED_THREADS];
static pthread_mutex_t g_launch_model_lock = PTHREAD_MUTEX_INITIALIZER;
static sgpl_launch_path_cost_entry g_launch_path_costs[SGPL_MAX_LAUNCH_KEYS];

static atomic_long g_parallel_plain_calls = 0;
static atomic_long g_parallel_ex_calls = 0;
static atomic_long g_parallel_plain_trip_total = 0;
static atomic_long g_parallel_ex_trip_total = 0;
static atomic_long g_doall_decision_calls = 0;
static atomic_long g_doacross_decision_calls = 0;

static _Thread_local const sgpl_loop_profile_desc *g_tls_doacross_profile_desc = NULL;
static _Thread_local sgpl_doacross_state *g_tls_doacross_state = NULL;
static _Thread_local uint64_t g_tls_doacross_wait_total_ns = 0;
static _Thread_local uint64_t g_tls_doacross_post_total_ns = 0;
static _Thread_local uint64_t g_tls_doacross_wait_count = 0;
static _Thread_local uint64_t g_tls_doacross_post_count = 0;
static _Thread_local sgpl_doacross_sync_lane_sample *g_tls_doacross_sync_lane = NULL;
static _Thread_local sgpl_level_loop_pool *g_tls_active_loop_pool = NULL;
static _Thread_local int32_t g_tls_pending_loop_id = -1;
static _Thread_local int32_t g_tls_current_tdg_task_slot = -1;
static _Thread_local int32_t g_tls_desired_threads_min = 0;
static _Thread_local int32_t g_tls_desired_threads_max = 0;
static _Thread_local int32_t g_tls_dispatch_depth = 0;

/* Debug introspection: number of nested dispatches (depth > 1) that were
 * granted more than one thread.  A bounded budget model keeps this at zero:
 * nested work inside a parallel region shares the region's grant. */
static atomic_long g_debug_nested_parallel_calls = 0;

long sgpl_debug_nested_parallel_calls(void)
{
    return atomic_load(&g_debug_nested_parallel_calls);
}

void sgpl_set_pending_loop_id(int32_t loop_id)
{
    g_tls_pending_loop_id = loop_id;
}

int32_t sgpl_tdg_level_active(void)
{
    return g_tls_active_loop_pool && g_tls_active_loop_pool->active;
}

int32_t sgpl_tdg_level_task_count(void)
{
    return sgpl_tdg_level_active() ? g_tls_active_loop_pool->task_count : 0;
}

void sgpl_set_desired_threads(int32_t min_threads, int32_t max_threads)
{
    g_tls_desired_threads_min = min_threads;
    g_tls_desired_threads_max = max_threads;
}

static void sgpl_tdg_enter_budget_scope(int32_t scope_threads, int32_t loop_threads, int32_t *saved_scope, int32_t *saved_depth)
{
    if (saved_scope)
        *saved_scope = g_tls_thread_budget_scope_reserved;
    if (saved_depth)
        *saved_depth = g_tls_thread_budget_depth;

    if (scope_threads > 0)
        g_tls_thread_budget_scope_reserved = scope_threads;

    sgpl_push_thread_budget(loop_threads);
}

static void sgpl_tdg_exit_budget_scope(int32_t saved_scope, int32_t saved_depth)
{
    sgpl_pop_thread_budget();
    g_tls_thread_budget_scope_reserved = saved_scope;
    while (g_tls_thread_budget_depth > saved_depth)
        sgpl_pop_thread_budget();
}

static const double SGPL_DOACROSS_SIGMA_WAIT_SEED_NS = 250.0;
static const double SGPL_DOACROSS_SIGMA_POST_SEED_NS = 80.0;
static const double SGPL_TDG_PROFILE_ALPHA = 0.25;
#define SGPL_C_INITIAL_BATCH_SAMPLES 4
#define SGPL_C_RECALIBRATION_BATCH_SAMPLES 4
#define SGPL_C_PROBE_INTERVAL 64
#define SGPL_C_DRIFT_THRESHOLD 0.25
#define SGPL_C_DRIFT_CONFIRMATIONS 2
#define SGPL_C_SAMPLING_WARMING 0
#define SGPL_C_SAMPLING_STABLE 1
#define SGPL_C_SAMPLING_RECALIBRATING 2

/* Has the TDG profile for this site collected its calibration batch?  The
 * engine uses this to run a bounded number of honest serial passes before the
 * model may plan the site; the batch size is the runtime's own
 * SGPL_C_INITIAL_BATCH_SAMPLES, not a per-site constant. */
int32_t sgpl_step_site_ready(int32_t loop_id)
{
    if (loop_id < 0 || loop_id >= SGPL_MAX_PROFILED_LOOPS)
        return 0;
    return g_loop_states[loop_id].regime_valid &&
           g_loop_states[loop_id].c_sampling_state == SGPL_C_SAMPLING_STABLE;
}
#define SGPL_MAX_TDG_PROFILE_SLOTS 8192

typedef struct
{
    uint32_t sample_count;
    double ewma_ns;
} sgpl_tdg_profile_state;

static sgpl_tdg_profile_state g_tdg_profiles[SGPL_MAX_TDG_PROFILE_SLOTS];
static pthread_mutex_t g_tdg_profile_lock = PTHREAD_MUTEX_INITIALIZER;

static int32_t sgpl_trip_count_bucket(int64_t trip_count);
#include "tdg_trace.inc"
#define SGPL_TDG_INVOKE(task) sgpl_joint_invoke_task(task)

static inline atomic_long *sgpl_doacross_slot(sgpl_doacross_state *state, int32_t id, int slot)
{
    return &state->completion[(size_t)id * (size_t)state->sync_window + (size_t)slot];
}

static int runtime_debug_enabled(void)
{
    const char *V = getenv("GRAPH_PARALLEL_DEBUG");
    if (!V)
        return 0;
    return strcmp(V, "0") != 0 && strcmp(V, "") != 0;
}

static int runtime_iter_debug_enabled(void)
{
    /* Iteration logging disabled. Restore getenv("GRAPH_PARALLEL_ITER_DEBUG") here to re-enable. */
    return 0;
}

static int force_doacross_parallel_enabled(void)
{
    const char *V = getenv("SGPL_FORCE_DOACROSS_PARALLEL");
    if (!V)
        return 0;
    return strcmp(V, "0") != 0 && strcmp(V, "") != 0;
}

static int force_doall_parallel_enabled(void)
{
    const char *V = getenv("SGPL_FORCE_DOALL_PARALLEL");
    if (!V)
        return 0;
    return strcmp(V, "0") != 0 && strcmp(V, "") != 0;
}

uint64_t sgpl_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static int sgpl_runtime_thread_count(void)
{
    static int cached_nthreads = 0;

    if (cached_nthreads > 0)
        return cached_nthreads;

    int online_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (online_threads <= 0)
        online_threads = 1;

    cached_nthreads = online_threads;
    {
        const char *requested = getenv("SGPL_NUM_THREADS");
        if (!requested || requested[0] == '\0')
            requested = getenv("OMP_NUM_THREADS");

        if (requested && requested[0] != '\0')
        {
            char *end = NULL;
            long parsed = strtol(requested, &end, 10);
            if (end != requested && *end == '\0' && parsed > 0)
            {
                if (parsed > online_threads)
                    parsed = online_threads;
                cached_nthreads = (int)parsed;
            }
        }
    }
    return cached_nthreads;
}

int32_t sgpl_configured_worker_count(void)
{
    return (int32_t)sgpl_runtime_thread_count();
}

int32_t sgpl_current_worker_index(void)
{
    return g_tls_worker_index;
}

static int64_t sgpl_compute_trip_count(int64_t start, int64_t end, int64_t step)
{
    if (step == 0)
        return 0;

    if (step > 0)
    {
        if (start >= end)
            return 0;
        return ((end - start) + step - 1) / step;
    }

    if (start <= end)
        return 0;

    {
        int64_t abs_step = -step;
        return ((start - end) + abs_step - 1) / abs_step;
    }
}

static void sgpl_run_loop_serial(int64_t start,
                                 int64_t end,
                                 int64_t step,
                                 loop_body_fn body,
                                 void *env,
                                 int32_t needs_doacross,
                                 int32_t doacross_num_sync_ids)
{
    int64_t i = 0;
    sgpl_doacross_state *doacross_state = NULL;
    sgpl_doacross_sync_lane_sample *saved_sync_lane = g_tls_doacross_sync_lane;

    if (step == 0 || !body)
        return;

    if (needs_doacross)
    {
        doacross_state = sgpl_alloc_doacross_state(start, doacross_num_sync_ids);
        if (!doacross_state)
        {
            fprintf(stderr, "Runtime Error: failed to allocate doacross sync table\n");
            return;
        }
        doacross_init_state(doacross_state, start);
    }

    sgpl_prepare_doacross_profile(doacross_state, 1, sgpl_compute_trip_count(start, end, step));
    g_tls_doacross_sync_lane = doacross_state ? doacross_state->profile_lanes : NULL;
    g_tls_doacross_state = doacross_state;

    if (step < 0)
    {
        for (i = start; i > end; i += step)
        {
            body(i, env);
        }
    }
    else
    {
        for (i = start; i < end; i += step)
        {
            body(i, env);
        }
    }

    g_tls_doacross_state = NULL;
    g_tls_doacross_sync_lane = saved_sync_lane;
    sgpl_free_doacross_state(doacross_state);
}

static void sgpl_fatal_doacross_state_error(const char *message)
{
    fprintf(stderr, "Runtime Error: %s\n", message);
    exit(1);
}

static sgpl_doacross_state *sgpl_alloc_doacross_state(int64_t start, int32_t num_ids)
{
    sgpl_doacross_state *state = NULL;
    size_t total_slots = 0;
    size_t idx = 0;

    if (num_ids <= 0)
    {
        fprintf(stderr, "Runtime Error: doacross loop missing sync-id metadata\n");
        exit(1);
    }

    total_slots = (size_t)num_ids * (size_t)SYNC_WINDOW;
    state = (sgpl_doacross_state *)calloc(1, sizeof(sgpl_doacross_state));
    if (!state)
        return NULL;

    state->completion = (atomic_long *)calloc(total_slots, sizeof(atomic_long));
    if (!state->completion)
    {
        free(state);
        return NULL;
    }
    state->num_ids = num_ids;
    state->sync_window = SYNC_WINDOW;
    state->loop_start = start;
    for (idx = 0; idx < total_slots; ++idx)
        atomic_init(&state->completion[idx], start - 1000);

    return state;
}

static void sgpl_prepare_doacross_profile(sgpl_doacross_state *state,
                                          int32_t threads, int64_t trips)
{
    if (!state || threads < 1)
        return;
    state->profile_threads = threads;
    state->profile_trip_bucket = sgpl_trip_count_bucket(trips);
    state->profile_lanes = calloc((size_t)threads, sizeof(*state->profile_lanes));
}

static void sgpl_free_doacross_state(sgpl_doacross_state *state)
{
    if (!state)
        return;
    sgpl_commit_doacross_profile(state);
    free(state->profile_lanes);
    free(state->completion);
    free(state);
}

static void doacross_init_state(sgpl_doacross_state *state, int64_t start)
{
    int32_t id = 0;
    int slot = 0;
    long init_val = start - 1000;

    if (!state)
        sgpl_fatal_doacross_state_error("missing doacross state for active worker");

    state->loop_start = start;

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] doacross-init start=%lld ids=%d window=%d\n",
                (long long)start,
                state->num_ids,
                state->sync_window);
    }

    for (id = 0; id < state->num_ids; ++id)
    {
        for (slot = 0; slot < state->sync_window; ++slot)
        {
            atomic_store(sgpl_doacross_slot(state, id, slot), init_val);
        }
    }
}

static void doacross_wait_state(sgpl_doacross_state *state, int64_t iter, int64_t dist, int32_t id)
{
    int64_t dep_iter;
    int slot;
    uint64_t start_ns = 0;

    if (!state)
        sgpl_fatal_doacross_state_error("missing doacross state for active worker");
    if (g_tls_doacross_profile_desc)
        start_ns = sgpl_now_ns();

    if (id < 0 || id >= state->num_ids)
    {
        fprintf(stderr,
                "Runtime Error: compact doacross ID %d out of range [0, %d)\n",
                id,
                state->num_ids);
        exit(1);
    }

    dep_iter = iter - dist;
    if (dep_iter < state->loop_start)
        return;

    slot = (int)(dep_iter % state->sync_window);
    while (atomic_load(sgpl_doacross_slot(state, id, slot)) < dep_iter)
    {
#if defined(__x86_64__) || defined(__i386__)
        __builtin_ia32_pause();
#endif
    }

    if (g_tls_doacross_profile_desc)
    {
        g_tls_doacross_wait_total_ns += sgpl_now_ns() - start_ns;
        g_tls_doacross_wait_count += 1;
    }
}

static void doacross_post_state(sgpl_doacross_state *state, int64_t iter, int32_t id)
{
    int slot;
    uint64_t start_ns = 0;
    if (!state)
        sgpl_fatal_doacross_state_error("missing doacross state for active worker");
    if (g_tls_doacross_profile_desc)
        start_ns = sgpl_now_ns();
    if (id < 0 || id >= state->num_ids)
    {
        fprintf(stderr,
                "Runtime Error: compact doacross ID %d out of range [0, %d)\n",
                id,
                state->num_ids);
        exit(1);
    }

    slot = (int)(iter % state->sync_window);
    atomic_store(sgpl_doacross_slot(state, id, slot), iter);
    if (g_tls_doacross_profile_desc)
    {
        g_tls_doacross_post_total_ns += sgpl_now_ns() - start_ns;
        g_tls_doacross_post_count += 1;
    }
}

void doacross_init(int64_t start)
{
    doacross_init_state(g_tls_doacross_state, start);
}

void doacross_wait(int64_t iter, int64_t dist, int32_t id)
{
    doacross_wait_state(g_tls_doacross_state, iter, dist, id);
}

void doacross_post(int64_t iter, int32_t id)
{
    doacross_post_state(g_tls_doacross_state, iter, id);
}

/* Iterations per block-cyclic chunk.  The floor that matters is one cache line
 * (8 doubles / 16 int32s); the default is larger so a chunk also amortises the
 * per-iteration indirect call into the outlined body, while staying small
 * enough that P chunks still balance a skewed loop. */
static int64_t sgpl_loop_chunk_iterations(void)
{
    static int64_t cached = -1;
    if (cached < 0)
    {
        const char *v = getenv("SGPL_LOOP_CHUNK");
        cached = 256;
        if (v && *v)
        {
            long long parsed = atoll(v);
            if (parsed >= 0)
                cached = (int64_t)parsed;
        }
    }
    return cached;
}

static void *worker_main(void *_arg)
{
    workers_args_t *a = (workers_args_t *)_arg;
    int tid = a->tid;
    int nthreads = a->nthreads;
    int64_t start = a->start;
    int64_t end = a->end;
    int64_t step = a->step;
    int64_t i = 0;
    int32_t previous_worker_index = g_tls_worker_index;
    int32_t previous_scope = 0;
    sgpl_doacross_sync_lane_sample *saved_sync_lane = g_tls_doacross_sync_lane;

    if (step == 0)
        return NULL;

    /* Publish this region's width as the budget scope divisor: nested
     * parallelism inside the loop body then takes a share of the region's
     * grant (cap / nthreads) instead of drawing from the machine width.
     * SGPL_NO_REGION_SCOPE=1 restores the previous behaviour. */
    previous_scope = g_tls_thread_budget_scope_reserved;
    if (!region_scope_disabled() && nthreads > 1)
        g_tls_thread_budget_scope_reserved = nthreads;

    g_tls_doacross_state = a->doacross_state;
    g_tls_doacross_sync_lane = (a->doacross_state && a->doacross_state->profile_lanes)
                                  ? &a->doacross_state->profile_lanes[tid] : NULL;
    g_tls_worker_index = tid;
    if (a->override_originals && a->override_replacements && a->num_override_targets > 0)
    {
        roaring_bitmap_set_thread_local_overrides(a->override_originals,
                                                  a->override_replacements,
                                                  a->num_override_targets);
    }

    /* Iteration distribution.
     *
     * The original schedule was pure cyclic: thread t ran i = t, t+P, t+2P, ...
     * For a DOALL loop over a vertex array that is the worst possible layout.
     * Eight doubles share a 64-byte line, so with P=4 every line is written by
     * every thread -- false sharing on *every* line -- and no thread sees a
     * contiguous run, so hardware prefetch and any residual vectorisation are
     * both defeated.  Measured on a 1e6-vertex loop with 24 flops/iteration
     * this ran at 0.72x of serial while burning 2.8 cores, and the ratio did
     * not move when the loop count went 20 -> 200, confirming a per-iteration
     * effect rather than dispatch overhead.
     *
     * Block-cyclic with a chunk of at least one cache line fixes both: each
     * thread walks contiguous runs (prefetch works, lines are privately owned)
     * while still interleaving chunks, so a skewed body -- the common case in
     * graph work, where per-vertex cost tracks degree -- stays balanced.  A
     * pure block split would lose that balance.
     *
     * DOACROSS is excluded: its wait/post protocol is written against the
     * cyclic order, and reordering iterations there would change synchronisation
     * semantics, not just locality.
     * SGPL_LOOP_CHUNK=0 restores the cyclic schedule; >0 sets the chunk.
     *
     * The chunk is clamped adaptively: block-cyclic hands thread t the
     * iterations [t*chunk, (t+1)*chunk), so a loop whose whole trip count fits
     * in one chunk runs entirely on thread 0.  The CleanCut executor's
     * partition loop (workers*4 trips, e.g. 48) hit exactly that and executed
     * on a single worker.  Shrink the chunk so every worker owns at least one
     * block whenever there are enough iterations; below that, fall back to the
     * cyclic schedule. */
    {
        int64_t chunk = sgpl_loop_chunk_iterations();
        int use_chunked = (chunk > 1) && !a->doacross_state;

        if (use_chunked)
        {
            int64_t total = step > 0
                                ? (end - start + step - 1) / step
                                : (start - end + (-step) - 1) / (-step);
            int64_t max_chunk = total > 0
                                    ? (total + nthreads - 1) / nthreads
                                    : 1;
            if (max_chunk < 1)
                max_chunk = 1;
            if (chunk > max_chunk)
            {
                chunk = max_chunk;
                use_chunked = chunk > 1;
            }
        }

        if (!use_chunked)
        {
            if (step < 0)
            {
                for (i = start + (int64_t)tid * step; i > end; i += (int64_t)nthreads * step)
                {
                    if (a->body)
                        a->body(i, a->env);
                }
            }
            else
            {
                for (i = start + (int64_t)tid * step; i < end; i += (int64_t)nthreads * step)
                {
                    if (runtime_iter_debug_enabled())
                        printf("Iteration %lld by thread %d\n", (long long)i, tid);
                    if (a->body)
                        a->body(i, a->env);
                }
            }
        }
        else
        {
            /* Chunk c of the loop is iterations [c*chunk, (c+1)*chunk) counted
             * in units of `step`; thread tid takes chunks tid, tid+P, ...
             *
             * When the compiler supplied a range entry point, hand it the whole
             * chunk in one call.  That is what lets the parallel path keep up
             * with the serial one: the per-index form is reached through a
             * function pointer, so the body cannot be inlined and the loop
             * around it cannot be vectorised, while the serial original sits in
             * main() and O3 vectorises it 4-wide.  Four scalar threads against
             * one vector thread is a loss, which is exactly what was measured
             * (0.72x at 24 flops/iteration).  The range entry point puts the
             * loop back on the compiler's side of the pointer. */
            int64_t block = chunk * step;              /* signed: follows step */
            int64_t stride = (int64_t)nthreads * block;
            int64_t base = start + (int64_t)tid * block;
            int64_t c = 0;

            if (step < 0)
            {
                for (; base > end; base += stride)
                {
                    if (a->range_body)
                    {
                        int64_t remaining = (base - end + (-step) - 1) / (-step);
                        int64_t count = remaining < chunk ? remaining : chunk;
                        if (count > 0)
                            a->range_body(base, count, step, a->env);
                        continue;
                    }
                    for (c = 0, i = base; c < chunk && i > end; ++c, i += step)
                    {
                        if (a->body)
                            a->body(i, a->env);
                    }
                }
            }
            else
            {
                for (; base < end; base += stride)
                {
                    if (a->range_body && !runtime_iter_debug_enabled())
                    {
                        int64_t remaining = (end - base + step - 1) / step;
                        int64_t count = remaining < chunk ? remaining : chunk;
                        if (count > 0)
                            a->range_body(base, count, step, a->env);
                        continue;
                    }
                    for (c = 0, i = base; c < chunk && i < end; ++c, i += step)
                    {
                        if (runtime_iter_debug_enabled())
                            printf("Iteration %lld by thread %d\n", (long long)i, tid);
                        if (a->body)
                            a->body(i, a->env);
                    }
                }
            }
        }
    }

    g_tls_thread_budget_scope_reserved = previous_scope;
    g_tls_doacross_state = NULL;
    g_tls_doacross_sync_lane = saved_sync_lane;
    if (a->override_originals && a->override_replacements && a->num_override_targets > 0)
        roaring_bitmap_clear_thread_local_overrides();
    g_tls_worker_index = previous_worker_index;
    return NULL;
}

static void *sgpl_thread_pool_worker_entry(void *arg)
{
    int tid = (int)(intptr_t)arg;

    g_tls_is_pool_worker = 1;
    for (;;)
    {
        pthread_barrier_wait(&g_sgpl_thread_pool_barrier);
        if (atomic_load(&g_sgpl_thread_pool_shutdown))
            break;

        if (tid < g_sgpl_thread_pool_job_nthreads && g_sgpl_thread_pool_job_args)
            worker_main(&g_sgpl_thread_pool_job_args[tid]);

        pthread_barrier_wait(&g_sgpl_thread_pool_barrier);
    }
    g_tls_is_pool_worker = 0;
    return NULL;
}

static void sgpl_thread_pool_init_once(void)
{
    int i = 0;

    sgpl_tdg_init_runtime_config();
    g_sgpl_thread_pool_size = sgpl_runtime_thread_count();
    if (g_sgpl_thread_pool_size <= 0)
        g_sgpl_thread_pool_size = 1;

    if (pthread_barrier_init(&g_sgpl_thread_pool_barrier, NULL, (unsigned)(g_sgpl_thread_pool_size + 1)) != 0)
    {
        fprintf(stderr, "Runtime Error: failed to init thread pool barrier\n");
        g_sgpl_thread_pool_size = 0;
        return;
    }

    g_sgpl_thread_pool_threads = (pthread_t *)calloc((size_t)g_sgpl_thread_pool_size, sizeof(pthread_t));
    if (!g_sgpl_thread_pool_threads)
    {
        fprintf(stderr, "Runtime Error: failed to allocate thread pool workers\n");
        pthread_barrier_destroy(&g_sgpl_thread_pool_barrier);
        g_sgpl_thread_pool_size = 0;
        return;
    }

    for (i = 0; i < g_sgpl_thread_pool_size; ++i)
    {
        if (pthread_create(&g_sgpl_thread_pool_threads[i], NULL, sgpl_thread_pool_worker_entry, (void *)(intptr_t)i) != 0)
        {
            fprintf(stderr, "Runtime Error: failed to create thread pool worker %d\n", i);
            atomic_store(&g_sgpl_thread_pool_shutdown, 1);
            g_sgpl_thread_pool_size = i;
            break;
        }
    }
}

static void sgpl_thread_pool_ensure_init(void)
{
    pthread_once(&g_sgpl_thread_pool_once, sgpl_thread_pool_init_once);
}

static int sgpl_thread_pool_available(void)
{
    sgpl_thread_pool_ensure_init();
    return g_sgpl_thread_pool_size > 0 && !atomic_load(&g_sgpl_thread_pool_shutdown);
}

static void sgpl_thread_pool_run(workers_args_t *args, int nthreads)
{
    if (!args || nthreads <= 0)
        return;

    if (nthreads > g_sgpl_thread_pool_size)
        nthreads = g_sgpl_thread_pool_size;

    pthread_mutex_lock(&g_sgpl_thread_pool_run_lock);
    g_sgpl_thread_pool_job_args = args;
    g_sgpl_thread_pool_job_nthreads = nthreads;
    pthread_barrier_wait(&g_sgpl_thread_pool_barrier);
    pthread_barrier_wait(&g_sgpl_thread_pool_barrier);
    g_sgpl_thread_pool_job_args = NULL;
    g_sgpl_thread_pool_job_nthreads = 0;
    pthread_mutex_unlock(&g_sgpl_thread_pool_run_lock);
}

static void sgpl_append_copy_lane(int64_t lane, void *env)
{
    sgpl_append_copy_context *copy = (sgpl_append_copy_context *)env;
    int64_t count = 0;

    if (!copy || lane < 0 || !copy->sources || !copy->offsets || !copy->destination)
        return;

    count = copy->offsets[lane + 1] - copy->offsets[lane];
    if (count <= 0 || !copy->sources[lane])
        return;

    memcpy(copy->destination + copy->offsets[lane],
           copy->sources[lane],
           (size_t)count * sizeof(int32_t));
}

static int32_t sgpl_priv_target_kind(const int32_t *priv_kinds, int32_t target)
{
    if (!priv_kinds)
        return SGPL_PRIV_ROARING;
    return priv_kinds[target];
}

static int32_t sgpl_read_i32_env_field(void *env, int64_t offset, int is_pointer_slot)
{
    if (!env || offset < 0)
        return 0;

    if (is_pointer_slot)
    {
        int32_t **slot = (int32_t **)((char *)env + offset);
        if (!slot || !*slot)
            return 0;
        return **slot;
    }

    return *(int32_t *)((char *)env + offset);
}

static void sgpl_write_i32_env_field(void *env, int64_t offset, int is_pointer_slot, int32_t value)
{
    if (!env || offset < 0)
        return;

    if (is_pointer_slot)
    {
        int32_t **slot = (int32_t **)((char *)env + offset);
        if (!slot || !*slot)
            return;
        **slot = value;
        return;
    }

    *(int32_t *)((char *)env + offset) = value;
}

static int32_t *sgpl_read_i32_ptr_env_field(void *env, int64_t offset)
{
    if (!env || offset < 0)
        return NULL;
    return *(int32_t **)((char *)env + offset);
}

static void sgpl_write_i32_ptr_env_field(void *env, int64_t offset, int32_t *value)
{
    if (!env || offset < 0)
        return;
    *(int32_t **)((char *)env + offset) = value;
}

static void *sgpl_tdg_worker_main(void *arg)
{
    sgpl_tdg_worker_arg *worker = (sgpl_tdg_worker_arg *)arg;
    sgpl_tdg_level_state *state = worker->state;
    int32_t loop_threads = worker->loop_threads;
    int32_t scope_threads = worker->scope_threads;
    int32_t prior_scope = 0;
    int32_t old_depth = 0;
    sgpl_level_loop_pool *saved_pool = NULL;
    int32_t saved_pending_loop = -1;
    int32_t saved_task_slot = -1;

    int idx = worker->first_index;
    for (;; idx = atomic_fetch_add(&state->next_index, 1))
    {
        if (idx >= state->task_count)
            break;

        if (state->tasks[idx].fn)
        {
            sgpl_bind_level_loop_pool((sgpl_level_loop_pool *)state->loop_pool, &saved_pool, &saved_pending_loop);
            sgpl_tdg_enter_budget_scope(scope_threads, loop_threads, &prior_scope, &old_depth);
            saved_task_slot = g_tls_current_tdg_task_slot;
            g_tls_current_tdg_task_slot = idx;

            int active = atomic_fetch_add(&state->active_count, 1) + 1;
            int peak = atomic_load(&state->max_active);
            while (active > peak &&
                   !atomic_compare_exchange_weak(&state->max_active, &peak, active))
                ;

            uint64_t t0 = sgpl_now_ns();
            SGPL_TDG_INVOKE(&state->tasks[idx]);

            atomic_fetch_sub(&state->active_count, 1);

            g_tls_current_tdg_task_slot = saved_task_slot;
            sgpl_tdg_exit_budget_scope(prior_scope, old_depth);
            sgpl_unbind_level_loop_pool(saved_pool, saved_pending_loop);

            sgpl_record_tdg_profile_sample(&state->tasks[idx],
                                           idx,
                                           (double)(sgpl_now_ns() - t0),
                                           "parallel-worker");
        }
    }

    return NULL;
}

static int64_t sgpl_tdg_profile_units_from_ns(double elapsed_ns)
{
    double units = 0.0;

    if (elapsed_ns <= 0.0 || sgpl_tdg_unit_cost_ns <= 0.0)
        return 1;

    units = elapsed_ns / sgpl_tdg_unit_cost_ns;
    if (units < 1.0)
        units = 1.0;
    if (units > 2147483647.0)
        units = 2147483647.0;
    return (int64_t)(units + 0.5);
}

static void sgpl_record_tdg_profile_sample(const sgpl_tdg_task_desc *task,
                                           int32_t idx,
                                           double elapsed_ns,
                                           const char *path)
{
    sgpl_tdg_profile_state *slot = NULL;
    double old_ewma = 0.0;
    double new_ewma = 0.0;
    uint32_t old_samples = 0;
    uint32_t new_samples = 0;

    if (!task)
        return;
    if (task->profile_id < 0 || task->profile_id >= SGPL_MAX_TDG_PROFILE_SLOTS)
        return;

    slot = &g_tdg_profiles[task->profile_id];

    pthread_mutex_lock(&g_tdg_profile_lock);
    old_ewma = slot->ewma_ns;
    old_samples = slot->sample_count;
    if (slot->sample_count == 0)
        slot->ewma_ns = elapsed_ns;
    else
        slot->ewma_ns = (1.0 - SGPL_TDG_PROFILE_ALPHA) * slot->ewma_ns +
                        SGPL_TDG_PROFILE_ALPHA * elapsed_ns;
    if (slot->sample_count < UINT32_MAX)
        slot->sample_count++;
    new_ewma = slot->ewma_ns;
    new_samples = slot->sample_count;
    pthread_mutex_unlock(&g_tdg_profile_lock);

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.profile] idx=%d profile_id=%d path=%s elapsed_ns=%.2f old_samples=%u new_samples=%u old_ewma_ns=%.2f new_ewma_ns=%.2f refined_units=%lld\n",
                idx,
                task->profile_id,
                path ? path : "unknown",
                elapsed_ns,
                old_samples,
                new_samples,
                old_ewma,
                new_ewma,
                (long long)sgpl_tdg_profile_units_from_ns(new_ewma));
    }
}

static int64_t sgpl_tdg_effective_task_units(const sgpl_tdg_task_desc *task,
                                             uint32_t *samples_out,
                                             int *used_profile_out)
{
    int64_t static_units = 1;
    uint32_t samples = 0;
    double ewma_ns = 0.0;
    int64_t profiled_units = 0;
    int64_t min_units = 1;
    int64_t max_units = 0;

    if (!task)
        return 1;

    if (task->static_work_units > 0)
        static_units = (int64_t)task->static_work_units;

    if (samples_out)
        *samples_out = 0;
    if (used_profile_out)
        *used_profile_out = 0;

    if (task->profile_id < 0 || task->profile_id >= SGPL_MAX_TDG_PROFILE_SLOTS)
        return static_units;

    pthread_mutex_lock(&g_tdg_profile_lock);
    samples = g_tdg_profiles[task->profile_id].sample_count;
    ewma_ns = g_tdg_profiles[task->profile_id].ewma_ns;
    pthread_mutex_unlock(&g_tdg_profile_lock);

    if (samples_out)
        *samples_out = samples;

    if (samples < 2 || ewma_ns <= 0.0)
        return static_units;

    profiled_units = sgpl_tdg_profile_units_from_ns(ewma_ns);
    min_units = static_units / 2;
    if (min_units < 1)
        min_units = 1;
    max_units = static_units * 16;
    if (max_units < min_units)
        max_units = min_units;

    if (profiled_units < min_units)
        profiled_units = min_units;
    if (profiled_units > max_units)
        profiled_units = max_units;

    if (used_profile_out)
        *used_profile_out = 1;
    return profiled_units;
}

static void sgpl_parallel_launch_plain_ephemeral(int64_t start,
                                                 int64_t end,
                                                 int64_t step,
                                                 loop_body_fn body,
                                                 void *env,
                                                 int32_t needs_doacross,
                                                 int32_t doacross_num_sync_ids,
                                                 int nthreads)
{
    pthread_t *threads = NULL;
    workers_args_t *args = NULL;
    sgpl_doacross_state *doacross_state = NULL;
    loop_range_fn range_body = sgpl_take_pending_range_body();
    int i = 0;

    if (step == 0 || !body)
        return;

    if (needs_doacross)
    {
        doacross_state = sgpl_alloc_doacross_state(start, doacross_num_sync_ids);
        if (!doacross_state)
        {
            fprintf(stderr, "Runtime Error: failed to allocate doacross sync table\n");
            return;
        }
        doacross_init_state(doacross_state, start);
    }

    sgpl_prepare_doacross_profile(doacross_state, nthreads, sgpl_compute_trip_count(start, end, step));
    threads = (pthread_t *)malloc((size_t)nthreads * sizeof(pthread_t));
    args = (workers_args_t *)malloc((size_t)nthreads * sizeof(workers_args_t));
    if (!threads || !args)
    {
        sgpl_free_doacross_state(doacross_state);
        free(threads);
        free(args);
        return;
    }

    for (i = 0; i < nthreads; ++i)
    {
        args[i].start = start;
        args[i].end = end;
        args[i].step = step;
        args[i].body = body;
        args[i].range_body = range_body;
        args[i].env = env;
        args[i].tid = i;
        args[i].nthreads = nthreads;
        args[i].ncpus = nthreads;
        args[i].doacross_state = doacross_state;
        args[i].override_originals = NULL;
        args[i].override_replacements = NULL;
        args[i].num_override_targets = 0;
        pthread_create(&threads[i], NULL, worker_main, &args[i]);
    }

    for (i = 0; i < nthreads; ++i)
        pthread_join(threads[i], NULL);

    free(threads);
    free(args);
    sgpl_free_doacross_state(doacross_state);
}

static void sgpl_parallel_launch_plain_raw(int64_t start,
                                           int64_t end,
                                           int64_t step,
                                           loop_body_fn body,
                                           void *env,
                                           int32_t needs_doacross,
                                           int32_t doacross_num_sync_ids,
                                           int nthreads)
{
    workers_args_t *args = NULL;
    sgpl_doacross_state *doacross_state = NULL;
    loop_range_fn range_body = sgpl_take_pending_range_body();
    int i = 0;
    int use_pool = 0;

    if (step == 0 || !body || nthreads <= 0)
        return;

    if (needs_doacross)
    {
        doacross_state = sgpl_alloc_doacross_state(start, doacross_num_sync_ids);
        if (!doacross_state)
        {
            fprintf(stderr, "Runtime Error: failed to allocate doacross sync table\n");
            return;
        }
        doacross_init_state(doacross_state, start);
    }

    use_pool = !g_tls_is_pool_worker && !needs_doacross && sgpl_thread_pool_available();
    if (!use_pool)
    {
        /* Hand the range entry point on to the ephemeral path, which takes the
         * slot itself. */
        g_tls_pending_range_body = range_body;
        sgpl_parallel_launch_plain_ephemeral(start,
                                             end,
                                             step,
                                             body,
                                             env,
                                             needs_doacross,
                                             doacross_num_sync_ids,
                                             nthreads);
        return;
    }

    args = (workers_args_t *)malloc((size_t)nthreads * sizeof(workers_args_t));
    if (!args)
    {
        sgpl_free_doacross_state(doacross_state);
        return;
    }

    for (i = 0; i < nthreads; ++i)
    {
        args[i].start = start;
        args[i].end = end;
        args[i].step = step;
        args[i].body = body;
        args[i].range_body = range_body;
        args[i].env = env;
        args[i].tid = i;
        args[i].nthreads = nthreads;
        args[i].ncpus = nthreads;
        args[i].doacross_state = doacross_state;
        args[i].override_originals = NULL;
        args[i].override_replacements = NULL;
        args[i].num_override_targets = 0;
    }

    sgpl_thread_pool_run(args, nthreads);
    free(args);
    sgpl_free_doacross_state(doacross_state);
}

static void sgpl_parallel_launch_priv_raw(int64_t start,
                                          int64_t end,
                                          int64_t step,
                                          loop_body_fn body,
                                          void *env,
                                          int64_t env_size,
                                          const int64_t *priv_offsets,
                                          const int32_t *priv_kinds,
                                          const int64_t *priv_aux,
                                          int32_t num_priv_targets,
                                          int32_t needs_doacross,
                                          int32_t doacross_num_sync_ids,
                                          int nthreads)
{
    pthread_t *threads = NULL;
    unsigned char *thread_started = NULL;
    workers_args_t *args = NULL;
    void **thread_envs = NULL;
    RoaringBitmap ***priv_bitmaps = NULL;
    RoaringBitmap **priv_originals = NULL;
    RoaringBitmap ***thread_override_replacements = NULL;
    int32_t ***append_local_bufs = NULL;
    int32_t ***append_local_sizes = NULL;
    sgpl_doacross_state *doacross_state = NULL;
    int target = 0;
    int tid = 0;
    int use_pool = 0;
    int setup_failed = 0;
    int32_t num_roaring_targets = 0;
    int32_t roaring_target = 0;

    if (step == 0 || !body)
        return;

    if (!env || env_size <= 0 || !priv_offsets || num_priv_targets <= 0)
    {
        sgpl_parallel_launch_plain_raw(start, end, step, body, env, needs_doacross, doacross_num_sync_ids, nthreads);
        return;
    }

    if (needs_doacross)
    {
        doacross_state = sgpl_alloc_doacross_state(start, doacross_num_sync_ids);
        if (!doacross_state)
        {
            fprintf(stderr, "Runtime Error: failed to allocate doacross sync table\n");
            return;
        }
        doacross_init_state(doacross_state, start);
    }

    sgpl_prepare_doacross_profile(doacross_state, nthreads, sgpl_compute_trip_count(start, end, step));
    use_pool = !g_tls_is_pool_worker && !needs_doacross && sgpl_thread_pool_available();

    for (target = 0; target < num_priv_targets; ++target)
    {
        if (sgpl_priv_target_kind(priv_kinds, target) == SGPL_PRIV_ROARING)
            ++num_roaring_targets;
    }

    if (num_roaring_targets > 0)
        priv_originals = (RoaringBitmap **)calloc((size_t)num_roaring_targets, sizeof(RoaringBitmap *));
    args = (workers_args_t *)malloc((size_t)nthreads * sizeof(workers_args_t));
    thread_envs = (void **)calloc((size_t)nthreads, sizeof(void *));
    priv_bitmaps = (RoaringBitmap ***)calloc((size_t)num_priv_targets, sizeof(RoaringBitmap **));
    append_local_bufs = (int32_t ***)calloc((size_t)num_priv_targets, sizeof(int32_t **));
    append_local_sizes = (int32_t ***)calloc((size_t)num_priv_targets, sizeof(int32_t **));
    thread_override_replacements = (RoaringBitmap ***)calloc((size_t)nthreads, sizeof(RoaringBitmap **));
    if (!use_pool)
    {
        threads = (pthread_t *)malloc((size_t)nthreads * sizeof(pthread_t));
        thread_started = (unsigned char *)calloc((size_t)nthreads, sizeof(unsigned char));
    }

    if (!args || !thread_envs || !priv_bitmaps ||
        (num_roaring_targets > 0 && !priv_originals) || !append_local_bufs ||
        !append_local_sizes || !thread_override_replacements ||
        (!use_pool && (!threads || !thread_started)))
    {
        sgpl_free_doacross_state(doacross_state);
        free(threads);
        free(thread_started);
        free(args);
        free(thread_envs);
        free(priv_bitmaps);
        free(priv_originals);
        free(append_local_bufs);
        free(append_local_sizes);
        free(thread_override_replacements);
        return;
    }

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] allocating privatized envs targets=%d threads=%d env_size=%lld pool=%d\n",
                num_priv_targets,
                nthreads,
                (long long)env_size,
                use_pool);
    }

    for (target = 0; target < num_priv_targets; ++target)
    {
        if (sgpl_priv_target_kind(priv_kinds, target) == SGPL_PRIV_INT_APPEND)
        {
            append_local_bufs[target] = (int32_t **)calloc((size_t)nthreads, sizeof(int32_t *));
            append_local_sizes[target] = (int32_t **)calloc((size_t)nthreads, sizeof(int32_t *));
            if (!append_local_bufs[target] || !append_local_sizes[target])
            {
                setup_failed = 1;
                break;
            }
            continue;
        }

        priv_bitmaps[target] = (RoaringBitmap **)calloc((size_t)nthreads, sizeof(RoaringBitmap *));
        if (!priv_bitmaps[target])
        {
            setup_failed = 1;
            break;
        }
    }

    for (tid = 0; tid < nthreads && !setup_failed; ++tid)
    {
        void *env_copy = malloc((size_t)env_size);
        if (!env_copy)
        {
            setup_failed = 1;
            break;
        }
        if (num_roaring_targets > 0)
            thread_override_replacements[tid] =
                (RoaringBitmap **)calloc((size_t)num_roaring_targets, sizeof(RoaringBitmap *));
        if (num_roaring_targets > 0 && !thread_override_replacements[tid])
        {
            free(env_copy);
            setup_failed = 1;
            break;
        }

        memcpy(env_copy, env, (size_t)env_size);
        thread_envs[tid] = env_copy;
        roaring_target = 0;

        for (target = 0; target < num_priv_targets; ++target)
        {
            int64_t offset = priv_offsets[target];
            int32_t kind = sgpl_priv_target_kind(priv_kinds, target);

            if (kind == SGPL_PRIV_INT_APPEND)
            {
                int64_t size_offset = priv_aux ? priv_aux[target * 2] : -1;
                int64_t cap_offset = priv_aux ? priv_aux[target * 2 + 1] : -1;
                int32_t cap = sgpl_read_i32_env_field(env, cap_offset, 0);
                int32_t *local_buf = NULL;
                int32_t *local_size = NULL;

                if (cap <= 0)
                    cap = 1;

                local_buf = (int32_t *)calloc((size_t)cap, sizeof(int32_t));
                local_size = (int32_t *)calloc(1, sizeof(int32_t));
                if (!local_buf || !local_size)
                {
                    free(local_buf);
                    free(local_size);
                    setup_failed = 1;
                    break;
                }

                append_local_bufs[target][tid] = local_buf;
                append_local_sizes[target][tid] = local_size;
                *local_size = 0;
                sgpl_write_i32_ptr_env_field(env_copy, offset, local_buf);
                sgpl_write_i32_ptr_env_field(env_copy, size_offset, local_size);
                continue;
            }

            {
                RoaringBitmap **slot = (RoaringBitmap **)((char *)env_copy + offset);
                RoaringBitmap *original = *(RoaringBitmap **)((char *)env + offset);
                RoaringBitmap *local = roaring_bitmap_create_like(original);
                if (!local)
                {
                    setup_failed = 1;
                    break;
                }
                priv_originals[roaring_target] = original;
                priv_bitmaps[target][tid] = local;
                thread_override_replacements[tid][roaring_target] = local;
                *slot = local;
                ++roaring_target;
            }
        }

        if (setup_failed)
            break;

        args[tid].start = start;
        args[tid].end = end;
        args[tid].step = step;
        args[tid].body = body;
        args[tid].range_body = NULL;
        args[tid].env = env_copy;
        args[tid].tid = tid;
        args[tid].nthreads = nthreads;
        args[tid].ncpus = nthreads;
        args[tid].doacross_state = doacross_state;
        args[tid].override_originals = priv_originals;
        args[tid].override_replacements = thread_override_replacements[tid];
        args[tid].num_override_targets = num_roaring_targets;
    }

    if (setup_failed)
    {
        /* Runtime diagnostic intentionally disabled. */
        goto cleanup;
    }

    if (use_pool)
        sgpl_thread_pool_run(args, nthreads);
    else
    {
        for (tid = 0; tid < nthreads; ++tid)
        {
            if (pthread_create(&threads[tid], NULL, worker_main, &args[tid]) == 0)
                thread_started[tid] = 1;
            else
                worker_main(&args[tid]);
        }
        for (tid = 0; tid < nthreads; ++tid)
        {
            if (thread_started[tid])
                pthread_join(threads[tid], NULL);
        }
    }

    for (target = 0; target < num_priv_targets; ++target)
    {
        int64_t offset = priv_offsets[target];
        int32_t kind = sgpl_priv_target_kind(priv_kinds, target);

        if (kind == SGPL_PRIV_INT_APPEND)
        {
            int64_t size_offset = priv_aux ? priv_aux[target * 2] : -1;
            int64_t cap_offset = priv_aux ? priv_aux[target * 2 + 1] : -1;
            int32_t *shared_arr = sgpl_read_i32_ptr_env_field(env, offset);
            int32_t capacity = sgpl_read_i32_env_field(env, cap_offset, 0);
            int32_t initial_size = sgpl_read_i32_env_field(env, size_offset, 1);
            int64_t *prefix_offsets = NULL;
            int64_t total_size = initial_size;
            int append_valid = shared_arr != NULL && capacity >= 0 &&
                               initial_size >= 0 && initial_size <= capacity;

            if (!append_valid)
                continue;

            /* Merge diagnostics intentionally disabled. */

            prefix_offsets = (int64_t *)calloc((size_t)nthreads + 1, sizeof(int64_t));
            if (!prefix_offsets)
            {
                continue;
            }
            prefix_offsets[0] = initial_size;

            for (tid = 0; tid < nthreads; ++tid)
            {
                int32_t *local = append_local_bufs[target] ? append_local_bufs[target][tid] : NULL;
                int32_t *local_size_ptr = append_local_sizes[target] ? append_local_sizes[target][tid] : NULL;
                int64_t local_size = local_size_ptr ? (int64_t)*local_size_ptr : 0;

                if (local_size < 0 || local_size > capacity || (local_size > 0 && !local) ||
                    total_size > (int64_t)capacity - local_size)
                {
                    append_valid = 0;
                    break;
                }
                total_size += local_size;
                prefix_offsets[tid + 1] = total_size;
            }

            if (!append_valid)
            {
                free(prefix_offsets);
                continue;
            }

            if (total_size >= SGPL_APPEND_PARALLEL_COPY_THRESHOLD &&
                nthreads > 1 && !g_tls_is_pool_worker)
            {
                sgpl_append_copy_context copy_context;

                copy_context.sources = append_local_bufs[target];
                copy_context.offsets = prefix_offsets;
                copy_context.destination = shared_arr;

                for (tid = 0; tid < nthreads; ++tid)
                {
                    args[tid].start = 0;
                    args[tid].end = nthreads;
                    args[tid].step = 1;
                    args[tid].body = sgpl_append_copy_lane;
                    args[tid].env = &copy_context;
                    args[tid].tid = tid;
                    args[tid].nthreads = nthreads;
                    args[tid].ncpus = nthreads;
                    args[tid].doacross_state = NULL;
                    args[tid].override_originals = NULL;
                    args[tid].override_replacements = NULL;
                    args[tid].num_override_targets = 0;
                }

                if (use_pool)
                    sgpl_thread_pool_run(args, nthreads);
                else
                {
                    memset(thread_started, 0, (size_t)nthreads);
                    for (tid = 0; tid < nthreads; ++tid)
                    {
                        if (pthread_create(&threads[tid], NULL, worker_main, &args[tid]) == 0)
                            thread_started[tid] = 1;
                        else
                            worker_main(&args[tid]);
                    }
                    for (tid = 0; tid < nthreads; ++tid)
                    {
                        if (thread_started[tid])
                            pthread_join(threads[tid], NULL);
                    }
                }
            }
            else
            {
                for (tid = 0; tid < nthreads; ++tid)
                    sgpl_append_copy_lane(tid,
                                          &(sgpl_append_copy_context){
                                              append_local_bufs[target],
                                              prefix_offsets,
                                              shared_arr});
            }

            sgpl_write_i32_env_field(env, size_offset, 1, (int32_t)total_size);
            free(prefix_offsets);
            continue;
        }

        {
            RoaringBitmap *original = *(RoaringBitmap **)((char *)env + offset);

            /* Merge diagnostics intentionally disabled. */

            for (tid = 0; tid < nthreads; ++tid)
            {
                RoaringBitmap *local = priv_bitmaps[target] ? priv_bitmaps[target][tid] : NULL;
                if (!local)
                    continue;
                roaring_bitmap_or_inplace(original, local);
                roaring_bitmap_free(local);
                priv_bitmaps[target][tid] = NULL;
            }
        }
    }

cleanup:
    for (tid = 0; tid < nthreads; ++tid)
    {
        free(thread_envs[tid]);
        free(thread_override_replacements[tid]);
    }

    for (target = 0; target < num_priv_targets; ++target)
    {
        if (priv_bitmaps[target])
        {
            for (tid = 0; tid < nthreads; ++tid)
            {
                if (priv_bitmaps[target][tid])
                    roaring_bitmap_free(priv_bitmaps[target][tid]);
            }
        }
        if (append_local_bufs[target])
        {
            for (tid = 0; tid < nthreads; ++tid)
                free(append_local_bufs[target][tid]);
        }
        if (append_local_sizes[target])
        {
            for (tid = 0; tid < nthreads; ++tid)
                free(append_local_sizes[target][tid]);
        }
        free(append_local_bufs[target]);
        free(append_local_sizes[target]);
        free(priv_bitmaps[target]);
    }

    free(threads);
    free(thread_started);
    free(args);
    free(thread_envs);
    free(priv_bitmaps);
    free(priv_originals);
    free(append_local_bufs);
    free(append_local_sizes);
    free(thread_override_replacements);
    sgpl_free_doacross_state(doacross_state);
}

static void sgpl_noop_body(int64_t i, void *env)
{
    (void)i;
    (void)env;
}

static sgpl_loop_runtime_state *sgpl_get_loop_state(const sgpl_loop_profile_desc *desc)
{
    if (!desc)
        return NULL;
    if (desc->loop_id < 0 || desc->loop_id >= SGPL_MAX_PROFILED_LOOPS)
        return NULL;
    return &g_loop_states[desc->loop_id];
}

static const char *sgpl_loop_debug_name(const sgpl_loop_profile_desc *desc)
{
    if (!desc || !desc->debug_name || desc->debug_name[0] == '\0')
        return "<unnamed-loop>";
    return desc->debug_name;
}

static int32_t sgpl_trip_count_bucket(int64_t trip_count)
{
    if (trip_count <= 8)
        return 0;
    if (trip_count <= 32)
        return 1;
    if (trip_count <= 128)
        return 2;
    if (trip_count <= 512)
        return 3;
    if (trip_count <= 2048)
        return 4;
    return 5;
}

static int64_t sgpl_trip_count_bucket_representative(int32_t bucket)
{
    switch (bucket)
    {
    case 0:
        return 8;
    case 1:
        return 32;
    case 2:
        return 128;
    case 3:
        return 512;
    case 4:
        return 2048;
    default:
        return 4096;
    }
}

static const char *sgpl_c_sampling_state_name(int32_t state)
{
    switch (state)
    {
    case SGPL_C_SAMPLING_WARMING:
        return "warming";
    case SGPL_C_SAMPLING_STABLE:
        return "stable";
    case SGPL_C_SAMPLING_RECALIBRATING:
        return "recalibrating";
    default:
        return "unknown";
    }
}

static int sgpl_loop_regime_matches(const sgpl_loop_runtime_state *state,
                                    const sgpl_loop_profile_desc *desc,
                                    int64_t trip_count)
{
    if (!state || !desc || !state->regime_valid)
        return 0;

    return state->regime_mode == desc->mode &&
           state->regime_runtime_kind == desc->runtime_kind &&
           state->regime_trip_bucket == sgpl_trip_count_bucket(trip_count) &&
           state->regime_env_size == desc->env_size &&
           state->regime_num_priv_targets == desc->num_priv_targets &&
           state->regime_waits_per_iter == desc->doacross_waits_per_iter &&
           state->regime_posts_per_iter == desc->doacross_posts_per_iter &&
           state->regime_doacross_num_sync_ids == desc->doacross_num_sync_ids;
}

static void sgpl_loop_set_regime(sgpl_loop_runtime_state *state,
                                 const sgpl_loop_profile_desc *desc,
                                 int64_t trip_count,
                                 int32_t sampling_state)
{
    if (!state || !desc)
        return;

    state->regime_valid = 1;
    state->regime_mode = desc->mode;
    state->regime_runtime_kind = desc->runtime_kind;
    state->regime_trip_bucket = sgpl_trip_count_bucket(trip_count);
    state->regime_env_size = desc->env_size;
    state->regime_num_priv_targets = desc->num_priv_targets;
    state->regime_waits_per_iter = desc->doacross_waits_per_iter;
    state->regime_posts_per_iter = desc->doacross_posts_per_iter;
    state->regime_doacross_num_sync_ids = desc->doacross_num_sync_ids;
    state->c_sampling_state = sampling_state;
    state->c_batch_samples = 0;
    state->c_probe_skip_counter = 0;
    state->c_consecutive_drift = 0;
    state->c_sampling_epoch += 1;
    state->decision_cache_valid = 0;
}

static void sgpl_loop_invalidate_decision_cache(sgpl_loop_runtime_state *state,
                                                const sgpl_loop_profile_desc *desc,
                                                const char *reason)
{
    if (!state)
        return;

    if (state->decision_cache_valid && runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] decision-cache-invalidate loop=%s loop_id=%d reason=%s bucket=%d c_state=%s cached_threads=%d cached_budget=%d cached_choose=%s epoch=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                reason ? reason : "unknown",
                state->regime_trip_bucket,
                sgpl_c_sampling_state_name(state->c_sampling_state),
                state->cached_threads,
                state->cached_budget_bucket,
                state->cached_choose_parallel ? "parallel" : "serial",
                state->cached_sampling_epoch);
    }
    state->decision_cache_valid = 0;
}

static int32_t sgpl_loop_pool_lookup_assigned_threads(int32_t loop_id)
{
    sgpl_level_loop_pool *pool = g_tls_active_loop_pool;
    int32_t task_slot = g_tls_current_tdg_task_slot;
    int i = 0;

    if (!pool || !pool->active)
        return 0;
    if (loop_id < 0)
        return 0; /* unregistered caller: no assigned width; the decision layer
                   * falls through to the pool-share (or ledger) path */

    for (i = 0; i < pool->entry_count; ++i)
    {
        if (pool->entries[i].loop_id != loop_id)
            continue;
        if (task_slot >= 0 && pool->entries[i].task_slot != task_slot)
            continue;
        return pool->entries[i].assigned_threads;
    }

    // Loop not in this level's plan (e.g. DOALL/DOACROSS nested under a TDG
    // task that only scheduled the outer graph-init task). Returning 1 here
    // forced sgpl_should_parallelize_* into permanent single-thread / serial
    // mode. Return 0 so the caller falls back to the live thread budget.
    return 0;
}

/* Validation knob: SGPL_FORCE_WIDTHS="<loop_id>:<width>[,<loop_id>:<width>...]"
 * pins the width of named loop sites so a caller can enumerate allocations
 * (e.g. sweep every (w1, w2) split between two task loops in one TDG level).
 * Default off: with the variable unset the planner decides, unchanged. */
static int32_t sgpl_forced_width_for_loop_id(int32_t loop_id)
{
    const char *env = getenv("SGPL_FORCE_WIDTHS");
    const char *p = env;
    if (!env || !*env || loop_id < 0)
        return -1;
    while (*p)
    {
        char *end = NULL;
        long id = strtol(p, &end, 10);
        if (end == p || !end || *end != ':')
            break;
        long w = strtol(end + 1, &end, 10);
        if (end == p)
            break;
        p = end;
        if (id == loop_id && w > 0)
            return (int32_t)w;
        while (*p && *p != ',')
            ++p;
        if (*p == ',')
            ++p;
    }
    return -1;
}

static int32_t sgpl_loop_effective_decision_threads_for_loop_id(int32_t loop_id)
{
    int32_t threads = sgpl_runtime_thread_count();
    int32_t forced = sgpl_forced_width_for_loop_id(loop_id);
    if (forced > 0)
        return forced;
    int32_t assigned = sgpl_loop_pool_lookup_assigned_threads(loop_id);

    if (assigned > 0)
        return assigned;

    /* No assigned entry for this caller -- unregistered dispatch, or a site the
     * plan did not enable (engine step without a profiled site): share the
     * level's remaining budget.  The ledger path would return ~1 thread because
     * the level reserved its whole budget already. */
    if (!unregistered_pool_share_disabled())
    {
        sgpl_level_loop_pool *pool = g_tls_active_loop_pool;
        if (pool && pool->active)
        {
            int32_t cap = sgpl_thread_budget_cap();
            int32_t remaining = atomic_load(&pool->remaining_threads);

            threads = cap;
            if (remaining > 0 && remaining < threads)
                threads = remaining;
            if (threads < 1)
                threads = 1;
            return threads;
        }
    }

    {
        int32_t available = sgpl_budget_available_threads();
        if (available < threads)
            threads = available;
    }
    if (threads < 1)
        threads = 1;
    return threads;
}

static int32_t sgpl_loop_effective_decision_threads(void)
{
    return sgpl_loop_effective_decision_threads_for_loop_id(g_tls_pending_loop_id);
}

/* Requested width for one dispatch: never more threads than work items, never
 * more than the annotated desired maximum, and at least the annotated minimum
 * when the trip count and the machine allow it.  The budget/pool layer still
 * has the final say on the grant. */
static int32_t sgpl_dispatch_thread_request(int32_t nthreads, int64_t trip_count)
{
    if (trip_count > 0 && nthreads > (int32_t)trip_count)
        nthreads = (int32_t)trip_count;

    if (g_tls_desired_threads_max > 0 && nthreads > g_tls_desired_threads_max)
        nthreads = g_tls_desired_threads_max;

    if (g_tls_desired_threads_min > 1 && nthreads < g_tls_desired_threads_min)
    {
        int32_t want = g_tls_desired_threads_min;
        int32_t cap = sgpl_runtime_thread_count();

        if (want > cap)
            want = cap;
        if (trip_count > 0 && want > (int32_t)trip_count)
            want = (int32_t)trip_count;
        if (want > nthreads)
            nthreads = want;
    }

    if (nthreads < 1)
        nthreads = 1;
    return nthreads;
}

static void sgpl_bind_level_loop_pool(sgpl_level_loop_pool *pool,
                                      sgpl_level_loop_pool **saved_pool,
                                      int32_t *saved_pending_loop)
{
    if (saved_pool)
        *saved_pool = g_tls_active_loop_pool;
    if (saved_pending_loop)
        *saved_pending_loop = g_tls_pending_loop_id;

    g_tls_active_loop_pool = pool;
    g_tls_pending_loop_id = -1;
}

static void sgpl_unbind_level_loop_pool(sgpl_level_loop_pool *saved_pool,
                                        int32_t saved_pending_loop)
{
    g_tls_active_loop_pool = saved_pool;
    g_tls_pending_loop_id = saved_pending_loop;
}

static void sgpl_loop_pool_init(sgpl_level_loop_pool *pool, const sgpl_level_loop_plan *plan)
{
    int i = 0;

    if (!pool)
        return;

    memset(pool, 0, sizeof(*pool));
    pthread_mutex_init(&pool->lock, NULL);

    if (!plan)
        return;

    pool->active = 1;
    pool->loop_budget = plan->loop_budget;
    atomic_init(&pool->remaining_threads, plan->loop_budget);
    pool->entry_count = plan->entry_count;
    pool->dominant_loop_id = plan->dominant_loop_id;
    pool->dominant_model_ns = plan->dominant_model_ns;
    for (i = 0; i < plan->entry_count; ++i)
        pool->entries[i] = plan->entries[i];
}

static void sgpl_loop_pool_destroy(sgpl_level_loop_pool *pool)
{
    if (!pool)
        return;
    pthread_mutex_destroy(&pool->lock);
}

static int32_t sgpl_loop_pool_try_acquire(int32_t loop_id, int32_t requested)
{
    sgpl_level_loop_pool *pool = g_tls_active_loop_pool;
    int32_t task_slot = g_tls_current_tdg_task_slot;
    int32_t granted = 1;
    int matched = 0;
    int i = 0;

    if (!pool || !pool->active)
        return sgpl_budget_try_reserve(requested);

    if (loop_id >= 0)
    {
        pthread_mutex_lock(&pool->lock);
        for (i = 0; i < pool->entry_count; ++i)
        {
            int remaining = atomic_load(&pool->remaining_threads);

            if (pool->entries[i].loop_id != loop_id)
                continue;
            if (task_slot >= 0 && pool->entries[i].task_slot != task_slot)
                continue;

            matched = 1;
            if (requested > pool->entries[i].assigned_threads)
                requested = pool->entries[i].assigned_threads;

            while (pool->entries[i].active_threads == 0 && requested >= 2 && remaining >= requested)
            {
                /* Unprofiled callers share this counter without the plan
                 * mutex; the reservation must use their same atomic CAS. */
                if (atomic_compare_exchange_weak(&pool->remaining_threads, &remaining,
                                                 remaining - requested))
                {
                    pool->entries[i].active_threads = requested;
                    granted = requested;
                    break;
                }
            }
            break;
        }
        pthread_mutex_unlock(&pool->lock);
    }

    /* No plan entry for this caller -- an unregistered dispatch, an engine step
     * without a profiled site, or a site the plan did not enable.  Share the
     * level's remaining budget instead of serialising: the grant stays inside
     * the level's reservation and is returned by sgpl_loop_pool_release. */
    if (!matched && !unregistered_pool_share_disabled())
    {
        for (;;)
        {
            int remaining = atomic_load(&pool->remaining_threads);
            int want = requested;
            if (want > remaining)
                want = remaining;
            if (want <= 1)
                return 1;
            if (atomic_compare_exchange_weak(&pool->remaining_threads, &remaining,
                                             remaining - want))
            {
                granted = want;
                break;
            }
        }
    }

    if (budget_debug_enabled())
    {
        fprintf(stderr,
                "[budget.loop-pool.reserve] task_slot=%d loop_id=%d requested=%d granted=%d remaining=%d budget=%d\n",
                task_slot,
                loop_id,
                requested,
                granted,
                atomic_load(&pool->remaining_threads),
                pool->loop_budget);
    }

    return granted;
}

static void sgpl_loop_pool_release(int32_t loop_id, int32_t granted)
{
    sgpl_level_loop_pool *pool = g_tls_active_loop_pool;
    int32_t task_slot = g_tls_current_tdg_task_slot;

    if (!pool || !pool->active)
    {
        sgpl_budget_release(granted);
        return;
    }
    if (loop_id < 0)
    {
        /* Unregistered caller: the grant came from the pool (see the matching
         * branch in sgpl_loop_pool_try_acquire), so return it to the pool. */
        if (!unregistered_pool_share_disabled() && granted > 1)
        {
            atomic_fetch_add(&pool->remaining_threads, granted);
            if (budget_debug_enabled())
            {
                fprintf(stderr,
                        "[budget.loop-pool.release] task_slot=%d loop_id=%d released=%d remaining=%d budget=%d\n",
                        task_slot,
                        loop_id,
                        granted,
                        atomic_load(&pool->remaining_threads),
                        pool->loop_budget);
            }
        }
        else
        {
            sgpl_budget_release(granted);
        }
        return;
    }
    if (granted <= 1)
        return;

    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < pool->entry_count; ++i)
    {
        if (pool->entries[i].loop_id != loop_id)
            continue;
        if (task_slot >= 0 && pool->entries[i].task_slot != task_slot)
            continue;
        pool->entries[i].active_threads = 0;
        break;
    }
    atomic_fetch_add(&pool->remaining_threads, granted);
    pthread_mutex_unlock(&pool->lock);

    if (budget_debug_enabled())
    {
        fprintf(stderr,
                "[budget.loop-pool.release] task_slot=%d loop_id=%d released=%d remaining=%d budget=%d\n",
                task_slot,
                loop_id,
                granted,
                atomic_load(&pool->remaining_threads),
                pool->loop_budget);
    }
}

static int sgpl_build_runtime_loop_desc(int32_t loop_id,
                                        const sgpl_loop_runtime_state *state,
                                        sgpl_loop_profile_desc *desc)
{
    if (!state || !desc || loop_id < 0 || !state->regime_valid)
        return 0;

    memset(desc, 0, sizeof(*desc));
    desc->loop_id = loop_id;
    desc->mode = state->regime_mode;
    desc->runtime_kind = state->regime_runtime_kind;
    desc->env_size = state->regime_env_size;
    desc->num_priv_targets = state->regime_num_priv_targets;
    desc->doacross_waits_per_iter = state->regime_waits_per_iter;
    desc->doacross_posts_per_iter = state->regime_posts_per_iter;
    desc->has_doacross_profile = state->regime_mode == SGPL_LOOP_DOACROSS ? 1 : 0;
    desc->doacross_num_sync_ids = state->regime_doacross_num_sync_ids;
    return 1;
}

static double sgpl_loop_serial_model_time_ns(const sgpl_loop_profile_desc *desc,
                                             const sgpl_loop_runtime_state *state,
                                             int64_t trip_count)
{
    if (!state || !desc || trip_count <= 0)
        return 0.0;
    if (desc->mode == SGPL_LOOP_DOACROSS)
        return (double)trip_count * (state->c_dep_ns_per_iter_ewma + state->c_ind_ns_per_iter_ewma);
    return (double)trip_count * state->c_ns_per_iter_ewma;
}

/* Largest lane's work under the existing cyclic/block-cyclic schedule.
 * Fractional N/P invents balance the dispatcher cannot deliver near a chunk
 * boundary. This mirrors worker_main; it does not change the schedule. */
static double sgpl_loop_max_lane_iterations(int64_t trips, int32_t threads, int doacross)
{
    int64_t chunk = doacross ? 1 : sgpl_loop_chunk_iterations();
    int64_t max_chunk;
    int64_t stride;
    int64_t remainder;
    if (trips <= 0)
        return 0.0;
    if (threads <= 1)
        return (double)trips;
    max_chunk = trips / threads + (trips % threads != 0);
    if (chunk > max_chunk)
        chunk = max_chunk;
    if (chunk < 1)
        chunk = 1;
    /* Avoid signed overflow for extreme trip counts. */
    if (chunk > INT64_MAX / threads)
        return (double)chunk;
    stride = chunk * threads;
    remainder = trips % stride;
    return (double)(trips / stride) * (double)chunk +
           (double)(remainder < chunk ? remainder : chunk);
}

typedef struct sgpl_doacross_sync_costs
{
    double wait_ns, post_ns;
    uint32_t samples;
    int seeded_wait, seeded_post;
} sgpl_doacross_sync_costs;

static int sgpl_doacross_width_matches(const sgpl_doacross_width_profile *profile,
                                      const sgpl_loop_profile_desc *desc,
                                      int32_t threads, int32_t trip_bucket)
{
    return profile->samples && profile->threads == threads && profile->trip_bucket == trip_bucket &&
           profile->runtime_kind == desc->runtime_kind && profile->env_size == desc->env_size &&
           profile->priv_targets == desc->num_priv_targets && profile->waits == desc->doacross_waits_per_iter &&
           profile->posts == desc->doacross_posts_per_iter && profile->sync_ids == desc->doacross_num_sync_ids;
}

static sgpl_doacross_sync_costs sgpl_get_doacross_sync_costs(const sgpl_loop_profile_desc *desc,
                                                           const sgpl_loop_runtime_state *state,
                                                           int64_t trips, int32_t threads)
{
    sgpl_doacross_sync_costs costs = {0};
    if (!desc)
        return costs;
    costs.seeded_wait = desc->doacross_waits_per_iter > 0;
    costs.seeded_post = desc->doacross_posts_per_iter > 0;
    costs.wait_ns = costs.seeded_wait ? SGPL_DOACROSS_SIGMA_WAIT_SEED_NS : 0.0;
    costs.post_ns = costs.seeded_post ? SGPL_DOACROSS_SIGMA_POST_SEED_NS : 0.0;
    if (!state)
        return costs;
    pthread_mutex_lock(&g_doacross_profile_lock);
    for (int i = 0; i < SGPL_DOACROSS_WIDTH_PROFILES; ++i)
    {
        const sgpl_doacross_width_profile *profile = &state->sync_widths[i];
        if (!sgpl_doacross_width_matches(profile, desc, threads, sgpl_trip_count_bucket(trips)))
            continue;
        costs.samples = profile->samples;
        if (costs.seeded_wait && profile->wait_ns > 0.0)
        {
            costs.wait_ns = profile->wait_ns;
            costs.seeded_wait = 0;
        }
        if (costs.seeded_post && profile->post_ns > 0.0)
        {
            costs.post_ns = profile->post_ns;
            costs.seeded_post = 0;
        }
        break;
    }
    pthread_mutex_unlock(&g_doacross_profile_lock);
    return costs;
}

static double sgpl_loop_parallel_model_time_ns(const sgpl_loop_profile_desc *desc,
                                               const sgpl_loop_runtime_state *state,
                                               int64_t trip_count,
                                               int32_t threads)
{
    sgpl_launch_overhead_detail launch;

    if (!state || !desc || trip_count <= 0)
        return 0.0;
    if (threads <= 1)
        return sgpl_loop_serial_model_time_ns(desc, state, trip_count);

    launch = sgpl_get_launch_overhead_detail(desc, threads);
    if (desc->mode == SGPL_LOOP_DOACROSS)
    {
        sgpl_doacross_sync_costs costs = sgpl_get_doacross_sync_costs(desc, state, trip_count, threads);
        double sync_per_iter = (double)desc->doacross_waits_per_iter * costs.wait_ns +
                               (double)desc->doacross_posts_per_iter * costs.post_ns;

        return launch.total_ns +
               (double)trip_count * state->c_dep_ns_per_iter_ewma +
               sgpl_loop_max_lane_iterations(trip_count, threads, 1) *
                   (state->c_ind_ns_per_iter_ewma + sync_per_iter);
    }

    return launch.total_ns +
           sgpl_loop_max_lane_iterations(trip_count, threads, 0) * state->c_ns_per_iter_ewma;
}

static int32_t sgpl_loop_min_profitable_threads(const sgpl_level_loop_candidate *candidate,
                                                int32_t loop_budget)
{
    const sgpl_loop_runtime_state *state = NULL;
    int32_t threads = 0;

    if (!candidate || loop_budget < 2 || candidate->loop_id < 0 || candidate->loop_id >= SGPL_MAX_PROFILED_LOOPS)
        return 0;

    state = &g_loop_states[candidate->loop_id];
    for (threads = 2; threads <= loop_budget; ++threads)
    {
        double model = sgpl_loop_parallel_model_time_ns(&candidate->desc,
                                                        state,
                                                        candidate->representative_trip_count,
                                                        threads);
        if (model + 1e-9 < candidate->serial_model_ns)
            return threads;
    }
    return 0;
}

static int32_t sgpl_loop_best_threads(const sgpl_level_loop_candidate *candidate,
                                      int32_t loop_budget)
{
    const sgpl_loop_runtime_state *state = NULL;
    int32_t threads = 0;
    int32_t best_threads = 0;
    double best_model = 0.0;

    if (!candidate || loop_budget < 2 || candidate->loop_id < 0 || candidate->loop_id >= SGPL_MAX_PROFILED_LOOPS)
        return 0;

    state = &g_loop_states[candidate->loop_id];
    for (threads = 2; threads <= loop_budget; ++threads)
    {
        double model = sgpl_loop_parallel_model_time_ns(&candidate->desc,
                                                        state,
                                                        candidate->representative_trip_count,
                                                        threads);
        if (model + 1e-9 >= candidate->serial_model_ns)
            continue;
        if (best_threads == 0 || model < best_model)
        {
            best_threads = threads;
            best_model = model;
        }
    }

    return best_threads;
}

static double sgpl_loop_candidate_model_time_ns(const sgpl_level_loop_candidate *candidate,
                                                int32_t threads)
{
    const sgpl_loop_runtime_state *state = NULL;

    if (!candidate || candidate->loop_id < 0 || candidate->loop_id >= SGPL_MAX_PROFILED_LOOPS)
        return 0.0;

    state = &g_loop_states[candidate->loop_id];
    if (threads <= 1)
        return sgpl_loop_serial_model_time_ns(&candidate->desc,
                                              state,
                                              candidate->representative_trip_count);

    return sgpl_loop_parallel_model_time_ns(&candidate->desc,
                                            state,
                                            candidate->representative_trip_count,
                                            threads);
}

typedef struct
{
    int32_t candidate_count;
    int32_t loop_budget;
    const double *cost_table;
    const sgpl_level_loop_candidate *candidates;
    int32_t parent_workers;
    int32_t group_index[SGPL_MAX_TDG_LEVEL_LOOPS];
} sgpl_nlopt_problem;

/* Serial sites on different task workers overlap. Parallel DOALL sites use
 * the existing exclusive pool, so their service times add. Sites within one
 * task are serial. This is a resource lower bound, not a release-time model. */
static double sgpl_level_loop_cost_ns(const sgpl_level_loop_candidate *candidates,
                                      int32_t count, const double *costs,
                                      const double *pooled_fraction, int32_t workers,
                                      const int32_t *group_index)
{
    double task_cost[SGPL_MAX_TDG_LEVEL_LOOPS];
    double pool_cost = 0.0, total = 0.0, longest = 0.0;
    for (int32_t i = 0; i < count; ++i) task_cost[i] = 0.0;
    for (int32_t i = 0; i < count; ++i)
    {
        int32_t group = group_index[i];
        task_cost[group] += costs[i];
        total += costs[i];
        if (candidates[i].desc.mode != SGPL_LOOP_DOACROSS)
            pool_cost += pooled_fraction[i] * costs[i];
    }
    for (int32_t i = 0; i < count; ++i)
        if (task_cost[i] > longest)
            longest = task_cost[i];
    return fmax(pool_cost, fmax(longest, total / (workers > 0 ? workers : 1)));
}

static double sgpl_loop_relaxed_cost_from_samples(const double *samples,
                                                  int32_t loop_budget,
                                                  double consumed_threads)
{
    int32_t lo = 0;
    int32_t hi = 0;
    double alpha = 0.0;

    if (!samples || loop_budget < 0)
        return 0.0;
    if (consumed_threads <= 0.0)
        return samples[0];
    if (consumed_threads >= (double)loop_budget)
        return samples[loop_budget];

    lo = (int32_t)floor(consumed_threads);
    hi = lo + 1;
    if (hi > loop_budget)
        hi = loop_budget;
    alpha = consumed_threads - (double)lo;
    return samples[lo] * (1.0 - alpha) + samples[hi] * alpha;
}

static double sgpl_nlopt_loop_objective(unsigned n, const double *x, double *grad, void *data)
{
    sgpl_nlopt_problem *problem = (sgpl_nlopt_problem *)data;
    double costs[SGPL_MAX_TDG_LEVEL_LOOPS];
    double pooled_fraction[SGPL_MAX_TDG_LEVEL_LOOPS];
    unsigned i = 0;

    (void)n;
    if (grad)
    {
        for (i = 0; i < n; ++i)
            grad[i] = 0.0;
    }
    if (!problem || !x)
        return 0.0;

    for (i = 0; i < (unsigned)problem->candidate_count; ++i)
    {
        const double *samples = problem->cost_table + (size_t)i * (size_t)(problem->loop_budget + 1);
        costs[i] = sgpl_loop_relaxed_cost_from_samples(samples, problem->loop_budget, x[i]);
        pooled_fraction[i] = fmin(1.0, fmax(0.0, x[i] - 1.0));
    }
    return sgpl_level_loop_cost_ns(problem->candidates, problem->candidate_count,
                                   costs, pooled_fraction, problem->parent_workers, problem->group_index);
}

static double sgpl_integer_loop_objective(const sgpl_nlopt_problem *problem, const int32_t *consumed)
{
    double x[SGPL_MAX_TDG_LEVEL_LOOPS + 1];
    for (int32_t i = 0; i < problem->candidate_count; ++i)
        x[i] = consumed[i];
    x[problem->candidate_count] = 0.0;
    return sgpl_nlopt_loop_objective((unsigned)(problem->candidate_count + 1), x, NULL, (void *)problem);
}

/* Sites in one callback execute sequentially and return their grant after
 * each dispatch. Reserve the largest team per task, rather than summing all
 * of that task's sequential sites. Different task workers can overlap. */
static double sgpl_loop_consumed_budget(const int32_t *group_index,
                                        int32_t count, const double *consumed)
{
    double peak[SGPL_MAX_TDG_LEVEL_LOOPS];
    double total = 0.0;
    for (int32_t i = 0; i < count; ++i) peak[i] = 0.0;
    for (int32_t i = 0; i < count; ++i)
    {
        int32_t group = group_index[i];
        if (consumed[i] > peak[group])
        {
            total += consumed[i] - peak[group];
            peak[group] = consumed[i];
        }
    }
    return total;
}

static int32_t sgpl_integer_consumed_budget(const sgpl_nlopt_problem *problem, const int32_t *consumed)
{
    double x[SGPL_MAX_TDG_LEVEL_LOOPS];
    for (int32_t i = 0; i < problem->candidate_count; ++i)
        x[i] = consumed[i];
    return (int32_t)sgpl_loop_consumed_budget(problem->group_index, problem->candidate_count, x);
}

static double sgpl_nlopt_budget_constraint(unsigned n, const double *x, double *grad, void *data)
{
    sgpl_nlopt_problem *problem = (sgpl_nlopt_problem *)data;
    double sum = 0.0;
    unsigned i = 0;

    if (grad)
    {
        for (i = 0; i < n; ++i)
            grad[i] = 0.0;
        if (problem && x)
        {
            for (i = 0; i < (unsigned)problem->candidate_count; ++i)
            {
                unsigned winner = i, j;
                for (j = 0; j < i; ++j)
                    if (problem->candidates[j].task_slot == problem->candidates[i].task_slot)
                        break;
                if (j < i) continue;
                for (j = i + 1; j < (unsigned)problem->candidate_count; ++j)
                    if (problem->candidates[j].task_slot == problem->candidates[i].task_slot && x[j] > x[winner])
                        winner = j;
                grad[winner] = 1.0;
            }
            grad[problem->candidate_count] = 1.0;
        }
    }
    if (!problem || !x)
        return 0.0;

    sum = sgpl_loop_consumed_budget(problem->group_index, problem->candidate_count, x);
    sum += x[problem->candidate_count];
    return sum - (double)problem->loop_budget;
}

static void sgpl_initialize_nlopt_guess(const sgpl_level_loop_candidate *candidates,
                                        int32_t candidate_count,
                                        int32_t loop_budget,
                                        double *x)
{
    int32_t remaining = loop_budget;
    int32_t i = 0;

    if (!x)
        return;

    for (i = 0; i <= candidate_count; ++i)
        x[i] = 0.0;

    for (i = 0; i < candidate_count; ++i)
    {
        int32_t min_threads = sgpl_loop_min_profitable_threads(&candidates[i], loop_budget);
        if (min_threads >= 2 && remaining >= min_threads)
        {
            x[i] = (double)min_threads;
            remaining -= min_threads;
        }
    }

    for (i = 0; i < candidate_count; ++i)
    {
        int32_t best_threads = sgpl_loop_best_threads(&candidates[i], loop_budget);
        int32_t current = (int32_t)(x[i] + 0.5);
        int32_t extra = 0;

        if (current < 2 || best_threads <= current || remaining <= 0)
            continue;

        extra = best_threads - current;
        if (extra > remaining)
            extra = remaining;
        x[i] += (double)extra;
        remaining -= extra;
        if (remaining <= 0)
            break;
    }

    x[candidate_count] = (double)remaining;
}

static int32_t sgpl_collect_level_loop_candidates(const sgpl_tdg_task_desc *tasks,
                                                  int32_t task_count,
                                                  sgpl_level_loop_candidate *out_candidates,
                                                  int32_t max_candidates)
{
    int32_t count = 0;
    int i = 0;

    if (!tasks || task_count <= 0 || !out_candidates || max_candidates <= 0)
        return 0;

    for (i = 0; i < task_count; ++i)
    {
        int32_t num_loop_sites = tasks[i].num_loop_sites;
        const int32_t *loop_site_ids = tasks[i].loop_site_ids;
        int32_t j = 0;

        if (num_loop_sites <= 0 || !loop_site_ids)
            continue;

        for (j = 0; j < num_loop_sites; ++j)
        {
            int32_t loop_id = loop_site_ids[j];
            const sgpl_loop_runtime_state *state = NULL;
            sgpl_level_loop_candidate candidate;

            if (loop_id < 0 || loop_id >= SGPL_MAX_PROFILED_LOOPS)
                continue;
            if (count >= max_candidates)
                continue;

            state = &g_loop_states[loop_id];
            if (!state->regime_valid || state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
                continue;
            if (!sgpl_build_runtime_loop_desc(loop_id, state, &candidate.desc))
                continue;

            candidate.task_slot = i;
            candidate.loop_id = loop_id;
            candidate.representative_trip_count = state->last_observed_trip_count > 0
                                                     ? state->last_observed_trip_count
                                                     : sgpl_trip_count_bucket_representative(state->regime_trip_bucket);
            if (state->observed_trip_count_ewma > 0.0)
                candidate.representative_trip_count = (int64_t)(state->observed_trip_count_ewma + 0.5);
            candidate.c_rank_ns = candidate.desc.mode == SGPL_LOOP_DOACROSS
                                      ? state->c_dep_ns_per_iter_ewma + state->c_ind_ns_per_iter_ewma
                                      : state->c_ns_per_iter_ewma;
            if (candidate.c_rank_ns <= 0.0)
                continue;
            candidate.serial_model_ns = sgpl_loop_serial_model_time_ns(&candidate.desc,
                                                                       state,
                                                                       candidate.representative_trip_count);
            out_candidates[count++] = candidate;
        }
    }

    for (i = 0; i < count; ++i)
    {
        int j = i + 1;
        for (; j < count; ++j)
        {
            if (out_candidates[j].c_rank_ns > out_candidates[i].c_rank_ns)
            {
                sgpl_level_loop_candidate tmp = out_candidates[i];
                out_candidates[i] = out_candidates[j];
                out_candidates[j] = tmp;
            }
        }
    }

    return count;
}

static void sgpl_evaluate_level_loop_plan(const sgpl_level_loop_candidate *candidates,
                                          int32_t candidate_count,
                                          int32_t loop_budget,
                                          int32_t parent_workers,
                                          sgpl_level_loop_plan *plan)
{
    int32_t assigned[SGPL_MAX_TDG_LEVEL_LOOPS] = {0};
    int32_t i = 0;
    double *cost_table = NULL;
    double *x_relaxed = NULL;
    double *lower_bounds = NULL;
    double *upper_bounds = NULL;
    int32_t consumed[SGPL_MAX_TDG_LEVEL_LOOPS] = {0};
    int32_t rounded_idle = 0;
    int32_t total_consumed = 0;
    double best_cost = 0.0;
    nlopt_opt opt = NULL;
    nlopt_result opt_result = NLOPT_FAILURE;
    sgpl_nlopt_problem problem;

    if (!plan)
        return;

    memset(plan, 0, sizeof(*plan));
    plan->loop_budget = loop_budget;
    plan->remaining_threads = loop_budget;
    plan->idle_threads = loop_budget;
    plan->dominant_loop_id = -1;
    if (!candidates || candidate_count <= 0)
    {
        plan->total_model_ns = 0.0;
        plan->slack_threads = loop_budget;
        return;
    }

    /* Cache task grouping once; optimizer evaluations need only linear work. */
    for (i = 0; i < candidate_count; ++i)
    {
        problem.group_index[i] = i;
        for (int32_t j = 0; j < i; ++j)
            if (candidates[j].task_slot == candidates[i].task_slot)
            {
                problem.group_index[i] = j;
                break;
            }
    }

    if (loop_budget <= 1)
    {
        double costs[SGPL_MAX_TDG_LEVEL_LOOPS], pooled_fraction[SGPL_MAX_TDG_LEVEL_LOOPS] = {0};
        for (i = 0; i < candidate_count; ++i)
        {
            sgpl_level_loop_plan_entry *entry = &plan->entries[plan->entry_count++];
            entry->task_slot = candidates[i].task_slot;
            entry->loop_id = candidates[i].loop_id;
            entry->assigned_threads = 1;
            entry->c_rank_ns = candidates[i].c_rank_ns;
            entry->model_ns = sgpl_loop_candidate_model_time_ns(&candidates[i], 1);
            costs[i] = entry->model_ns;
            if (entry->model_ns > plan->dominant_model_ns)
            {
                plan->dominant_model_ns = entry->model_ns;
                plan->dominant_loop_id = entry->loop_id;
            }
        }
        plan->total_model_ns = sgpl_level_loop_cost_ns(candidates, candidate_count, costs,
                                                      pooled_fraction, parent_workers, problem.group_index);
        plan->slack_threads = loop_budget;
        return;
    }

    cost_table = (double *)malloc((size_t)candidate_count * (size_t)(loop_budget + 1) * sizeof(double));
    x_relaxed = (double *)malloc((size_t)(candidate_count + 1) * sizeof(double));
    lower_bounds = (double *)malloc((size_t)(candidate_count + 1) * sizeof(double));
    upper_bounds = (double *)malloc((size_t)(candidate_count + 1) * sizeof(double));
    if (!cost_table || !x_relaxed || !lower_bounds || !upper_bounds)
    {
        free(cost_table);
        free(x_relaxed);
        free(lower_bounds);
        free(upper_bounds);
        plan->slack_threads = loop_budget;
        return;
    }

    for (i = 0; i < candidate_count; ++i)
    {
        int32_t threads = 0;
        double serial_model = sgpl_loop_candidate_model_time_ns(&candidates[i], 1);
        cost_table[(size_t)i * (size_t)(loop_budget + 1)] = serial_model;
        if (loop_budget >= 1)
            cost_table[(size_t)i * (size_t)(loop_budget + 1) + 1] = serial_model;
        for (threads = 1; threads <= loop_budget; ++threads)
        {
            if (threads >= 2)
                cost_table[(size_t)i * (size_t)(loop_budget + 1) + (size_t)threads] =
                    sgpl_loop_candidate_model_time_ns(&candidates[i], threads);
        }
    }

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.opt-input] loop_budget=%d loop_sites=%d objective=level-resources solver=nlopt serial-consumes=0\n",
                loop_budget,
                candidate_count);
    }

    problem.candidate_count = candidate_count;
    problem.loop_budget = loop_budget;
    problem.cost_table = cost_table;
    problem.candidates = candidates;
    problem.parent_workers = parent_workers;

    for (i = 0; i <= candidate_count; ++i)
    {
        lower_bounds[i] = 0.0;
        upper_bounds[i] = (double)loop_budget;
    }

    sgpl_initialize_nlopt_guess(candidates, candidate_count, loop_budget, x_relaxed);
    x_relaxed[candidate_count] = loop_budget - sgpl_loop_consumed_budget(problem.group_index, candidate_count, x_relaxed);
    opt = nlopt_create(NLOPT_LN_COBYLA, (unsigned)(candidate_count + 1));
    if (opt)
    {
        nlopt_set_lower_bounds(opt, lower_bounds);
        nlopt_set_upper_bounds(opt, upper_bounds);
        nlopt_set_min_objective(opt, sgpl_nlopt_loop_objective, &problem);
        nlopt_add_equality_constraint(opt, sgpl_nlopt_budget_constraint, &problem, 1e-7);
        nlopt_set_xtol_rel(opt, 1e-4);
        nlopt_set_ftol_rel(opt, 1e-6);
        nlopt_set_maxeval(opt, 500 * (candidate_count + 1));
        best_cost = sgpl_nlopt_loop_objective((unsigned)(candidate_count + 1), x_relaxed, NULL, &problem);
        opt_result = nlopt_optimize(opt, x_relaxed, &best_cost);
    }

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.opt-relaxed] status=%d objective_ns=%.2f",
                (int)opt_result,
                best_cost);
        for (i = 0; i < candidate_count; ++i)
            fprintf(stderr, " x%d=%.4f", i, x_relaxed[i]);
        fprintf(stderr, " idle=%.4f\n", x_relaxed[candidate_count]);
    }

    total_consumed = 0;
    for (i = 0; i < candidate_count; ++i)
    {
        int32_t rounded = (int32_t)floor(x_relaxed[i] + 0.5);
        if (rounded < 0)
            rounded = 0;
        if (rounded > loop_budget)
            rounded = loop_budget;
        if (rounded == 1)
            rounded = 0;
        consumed[i] = rounded;
    }
    total_consumed = sgpl_integer_consumed_budget(&problem, consumed);
    rounded_idle = (int32_t)floor(x_relaxed[candidate_count] + 0.5);
    if (rounded_idle < 0)
        rounded_idle = 0;
    if (rounded_idle > loop_budget)
        rounded_idle = loop_budget;

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.opt-rounded] total_consumed=%d idle=%d",
                total_consumed,
                rounded_idle);
        for (i = 0; i < candidate_count; ++i)
            fprintf(stderr, " c%d=%d", i, consumed[i]);
        fprintf(stderr, "\n");
    }

    while (total_consumed + rounded_idle < loop_budget)
    {
        int32_t best_index = -1;
        int32_t best_new_value = 0;
        int32_t best_delta_budget = 1;
        double best_delta_cost = 0.0;
        int found = 0;

        for (i = 0; i < candidate_count; ++i)
        {
            int32_t cur = consumed[i];
            int32_t next_value = 0;
            int32_t delta_budget = 0;
            double old_cost = 0.0;
            double new_cost = 0.0;
            double delta_cost = 0.0;

            if (cur == 0)
            {
                next_value = 2;
                delta_budget = 2;
            }
            else if (cur >= 2 && cur < loop_budget)
            {
                next_value = cur + 1;
                delta_budget = 1;
            }
            else
                continue;

            consumed[i] = next_value;
            delta_budget = sgpl_integer_consumed_budget(&problem, consumed) - total_consumed;
            consumed[i] = cur;
            if (total_consumed + rounded_idle + delta_budget > loop_budget)
                continue;

            old_cost = sgpl_integer_loop_objective(&problem, consumed);
            consumed[i] = next_value;
            new_cost = sgpl_integer_loop_objective(&problem, consumed);
            consumed[i] = cur;
            delta_cost = new_cost - old_cost;
            if (!found || delta_cost + 1e-9 < best_delta_cost)
            {
                found = 1;
                best_index = i;
                best_new_value = next_value;
                best_delta_budget = delta_budget;
                best_delta_cost = delta_cost;
            }
        }

        if (!found || best_delta_cost >= 0.0)
        {
            rounded_idle += 1;
            if (tdg_debug_enabled())
                fprintf(stderr, "[tdg.opt-repair] action=idle+1 total=%d idle=%d\n", total_consumed + rounded_idle, rounded_idle);
            continue;
        }

        consumed[best_index] = best_new_value;
        total_consumed += best_delta_budget;
        if (tdg_debug_enabled())
        {
            fprintf(stderr,
                    "[tdg.opt-repair] action=loop+ task_slot=%d loop_id=%d new_consumed=%d total=%d delta_cost_ns=%.2f\n",
                    candidates[best_index].task_slot,
                    candidates[best_index].loop_id,
                    best_new_value,
                    total_consumed + rounded_idle,
                    best_delta_cost);
        }
    }

    while (total_consumed + rounded_idle > loop_budget)
    {
        int32_t best_kind = -1;
        int32_t best_index = -1;
        int32_t best_new_value = 0;
        int32_t best_delta_budget = 1;
        double best_delta_cost = 0.0;
        int found = 0;

        if (rounded_idle > 0)
        {
            best_kind = 0;
            best_index = -1;
            best_new_value = rounded_idle - 1;
            best_delta_budget = 1;
            best_delta_cost = 0.0;
            found = 1;
        }

        for (i = 0; i < candidate_count; ++i)
        {
            int32_t cur = consumed[i];
            int32_t next_value = 0;
            int32_t delta_budget = 0;
            double old_cost = 0.0;
            double new_cost = 0.0;
            double delta_cost = 0.0;

            if (cur == 2)
            {
                next_value = 0;
                delta_budget = 2;
            }
            else if (cur > 2)
            {
                next_value = cur - 1;
                delta_budget = 1;
            }
            else
                continue;

            consumed[i] = next_value;
            delta_budget = total_consumed - sgpl_integer_consumed_budget(&problem, consumed);
            consumed[i] = cur;
            old_cost = sgpl_integer_loop_objective(&problem, consumed);
            consumed[i] = next_value;
            new_cost = sgpl_integer_loop_objective(&problem, consumed);
            consumed[i] = cur;
            delta_cost = new_cost - old_cost;
            if (!found || delta_cost + 1e-9 < best_delta_cost)
            {
                found = 1;
                best_kind = 1;
                best_index = i;
                best_new_value = next_value;
                best_delta_budget = delta_budget;
                best_delta_cost = delta_cost;
            }
        }

        if (!found)
            break;

        if (best_kind == 0)
        {
            rounded_idle -= 1;
            if (tdg_debug_enabled())
                fprintf(stderr, "[tdg.opt-repair] action=idle-1 total=%d idle=%d\n", total_consumed + rounded_idle, rounded_idle);
        }
        else
        {
            consumed[best_index] = best_new_value;
            total_consumed -= best_delta_budget;
            if (tdg_debug_enabled())
            {
                fprintf(stderr,
                        "[tdg.opt-repair] action=loop- task_slot=%d loop_id=%d new_consumed=%d total=%d delta_cost_ns=%.2f\n",
                        candidates[best_index].task_slot,
                        candidates[best_index].loop_id,
                        best_new_value,
                        total_consumed + rounded_idle,
                        best_delta_cost);
            }
        }
    }

    /* Rounding a relaxed solution can cross the serial/parallel discontinuity.
     * Always compare with all-serial. Small one/two-site plans can be repaired
     * exactly; bound this work by the number of trials, not by machine type. */
    {
        int32_t trial[SGPL_MAX_TDG_LEVEL_LOOPS] = {0};
        double incumbent = sgpl_integer_loop_objective(&problem, consumed);
        double serial = sgpl_integer_loop_objective(&problem, trial);
        if (serial <= incumbent)
        {
            memset(consumed, 0, sizeof(consumed));
            total_consumed = 0;
            incumbent = serial;
        }
        if (candidate_count <= 2 && (int64_t)(loop_budget + 1) * (loop_budget + 1) <= 65536)
        {
            for (int32_t a = 0; a <= loop_budget; ++a)
            {
                if (a == 1) continue;
                trial[0] = a;
                for (int32_t b = 0; b <= (candidate_count == 2 ? loop_budget : 0); ++b)
                {
                    double cost;
                    int32_t used;
                    if (b == 1) continue;
                    trial[1] = b;
                    used = sgpl_integer_consumed_budget(&problem, trial);
                    if (used > loop_budget) continue;
                    cost = sgpl_integer_loop_objective(&problem, trial);
                    if (cost < incumbent - 1e-9 || (fabs(cost - incumbent) <= 1e-9 && used < total_consumed))
                    {
                        consumed[0] = a; consumed[1] = b;
                        total_consumed = used;
                        incumbent = cost;
                    }
                }
            }
        }
        /* On larger plans, fill profitable sequential sites within their
         * task's already allocated peak team. A relaxed solver can stop on
         * the flat serial interval without finding this free reuse. */
        if (candidate_count > 2)
        {
            int32_t trials = 0;
            for (int pass = 0; pass < 2 && trials < 65536; ++pass)
            {
                int changed = 0;
                for (i = 0; i < candidate_count && trials < 65536; ++i)
                {
                    int32_t cap = 0, cur = consumed[i], best = cur;
                    double best_cost = incumbent;
                    for (int32_t j = 0; j < candidate_count; ++j)
                        if (problem.group_index[i] == problem.group_index[j] && consumed[j] > cap)
                            cap = consumed[j];
                    for (int32_t width = 0; width <= cap && trials < 65536; ++width)
                    {
                        double cost;
                        if (width == 1) continue;
                        consumed[i] = width;
                        ++trials;
                        cost = sgpl_integer_loop_objective(&problem, consumed);
                        if (cost < best_cost - 1e-9)
                        {
                            best = width;
                            best_cost = cost;
                        }
                    }
                    consumed[i] = best;
                    if (best != cur) changed = 1;
                    incumbent = best_cost;
                }
                if (!changed) break;
            }
            total_consumed = sgpl_integer_consumed_budget(&problem, consumed);
        }
        rounded_idle = loop_budget - total_consumed;
        plan->total_model_ns = incumbent;
    }

    for (i = 0; i < candidate_count; ++i)
    {
        int32_t actual_threads = consumed[i] >= 2 ? consumed[i] : 1;
        double model = cost_table[(size_t)i * (size_t)(loop_budget + 1) + (size_t)actual_threads];

        assigned[i] = actual_threads;

        if (model > plan->dominant_model_ns)
        {
            plan->dominant_model_ns = model;
            plan->dominant_loop_id = candidates[i].loop_id;
        }

        if (plan->entry_count < SGPL_MAX_TDG_LEVEL_LOOPS)
        {
            plan->entries[plan->entry_count].task_slot = candidates[i].task_slot;
            plan->entries[plan->entry_count].loop_id = candidates[i].loop_id;
            plan->entries[plan->entry_count].assigned_threads = assigned[i];
            plan->entries[plan->entry_count].c_rank_ns = candidates[i].c_rank_ns;
            plan->entries[plan->entry_count].model_ns = model;
            plan->entry_count += 1;
        }

        if (tdg_debug_enabled())
        {
            fprintf(stderr,
                    "[tdg.opt-final] task_slot=%d loop_id=%d assigned_threads=%d consumed=%d c_ns=%.2f model_ns=%.2f trip_count=%lld\n",
                    candidates[i].task_slot,
                    candidates[i].loop_id,
                    assigned[i],
                    assigned[i] <= 1 ? 0 : assigned[i],
                    candidates[i].c_rank_ns,
                    model,
                    (long long)candidates[i].representative_trip_count);
        }
    }

    plan->idle_threads = rounded_idle;
    plan->remaining_threads = rounded_idle;
    plan->slack_threads = rounded_idle;

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.opt-summary] loop_budget=%d used=%d idle=%d total_model_ns=%.2f dominant_loop=%d dominant_model_ns=%.2f parallel_loops=%d\n",
                loop_budget,
                total_consumed,
                plan->idle_threads,
                plan->total_model_ns,
                plan->dominant_loop_id,
                plan->dominant_model_ns,
                plan->entry_count);
    }

    if (opt)
        nlopt_destroy(opt);
    free(cost_table);
    free(x_relaxed);
    free(lower_bounds);
    free(upper_bounds);
}

/* Validation: print the level's plan (the model's chosen allocation) and let
 * SGPL_FORCE_WIDTHS pin it.  Gated on the budget debug flag, or on the sweep
 * variable being present so a sweep always carries the model's own choice. */
static void sgpl_tdg_print_level_plan(const sgpl_level_loop_plan *plan,
                                      int64_t work_units,
                                      int64_t span_units,
                                      int32_t level_budget)
{
    const char *force = getenv("SGPL_FORCE_WIDTHS");
    const char *dump = getenv("SGPL_TDG_PLAN_DUMP");
    int32_t i;
    if (!plan)
        return;
    if (!budget_debug_enabled() && !(force && *force) && !(dump && *dump))
        return;
    fprintf(stderr,
            "[tdg.plan] level entries=%d budget=%d loop_budget=%d idle=%d dominant=%d total_model_ns=%.1f work=%lld span=%lld\n",
            plan->entry_count,
            level_budget,
            plan->loop_budget,
            plan->idle_threads,
            plan->dominant_loop_id,
            plan->total_model_ns,
            (long long)work_units,
            (long long)span_units);
    for (i = 0; i < plan->entry_count; ++i)
    {
        fprintf(stderr,
                "[tdg.plan]   slot=%d loop_id=%d assigned_threads=%d c_rank_ns=%.3f model_ns=%.1f\n",
                plan->entries[i].task_slot,
                plan->entries[i].loop_id,
                plan->entries[i].assigned_threads,
                plan->entries[i].c_rank_ns,
                plan->entries[i].model_ns);
    }
    fflush(stderr);
}

static void sgpl_tdg_apply_forced_widths(sgpl_level_loop_plan *plan)
{
    const char *env = getenv("SGPL_FORCE_WIDTHS");
    int32_t i;
    int32_t sum = 0;
    int32_t forced_any = 0;
    if (!plan || plan->entry_count <= 0)
        return;
    for (i = 0; i < plan->entry_count; ++i)
    {
        int32_t forced = -1;
        const char *q = env;
        while (q && *q)
        {
            char *end = NULL;
            long id = strtol(q, &end, 10);
            if (end == q || !end || *end != ':')
                break;
            long w = strtol(end + 1, &end, 10);
            if (end == q)
                break;
            q = end;
            if (id == plan->entries[i].loop_id && w > 0)
            {
                forced = (int32_t)w;
                break;
            }
            while (*q && *q != ',')
                ++q;
            if (*q == ',')
                ++q;
        }
        if (forced > 0)
        {
            plan->entries[i].assigned_threads = forced;
            forced_any = 1;
        }
    }
    if (forced_any)
    {
        for (i = 0; i < plan->entry_count; ++i)
        {
            int32_t peak = 0, j;
            for (j = 0; j < i; ++j)
                if (plan->entries[j].task_slot == plan->entries[i].task_slot)
                    break;
            if (j < i) continue;
            for (j = i; j < plan->entry_count; ++j)
                if (plan->entries[j].task_slot == plan->entries[i].task_slot && plan->entries[j].assigned_threads > peak)
                    peak = plan->entries[j].assigned_threads;
            if (peak >= 2) sum += peak;
        }
        plan->idle_threads = sum < plan->loop_budget ? plan->loop_budget - sum : 0;
        plan->remaining_threads = plan->idle_threads;
        plan->slack_threads = plan->idle_threads;
    }
}

/* A TDG task is a serial callback. Reserve one parent worker per task;
 * only its declared loop sites can spend the remaining level capacity. Task
 * duration profiles must not trade parent workers for loop workers. */
static int32_t sgpl_choose_tdg_threads_with_loop_budget(int64_t work_units,
                                                        int64_t span_units,
                                                        const sgpl_tdg_task_desc *tasks,
                                                        int32_t task_count,
                                                        int32_t level_budget_threads,
                                                        sgpl_level_loop_plan *selected_plan)
{
    sgpl_level_loop_candidate candidates[SGPL_MAX_TDG_LEVEL_LOOPS];
    int32_t workers = task_count < level_budget_threads ? task_count : level_budget_threads;
    int32_t count;
    (void)work_units;
    (void)span_units;
    if (workers < 1)
        workers = 1;
    count = sgpl_collect_level_loop_candidates(tasks, task_count, candidates,
                                               SGPL_MAX_TDG_LEVEL_LOOPS);
    sgpl_evaluate_level_loop_plan(candidates, count,
                                  level_budget_threads > workers ? level_budget_threads - workers : 0,
                                  workers,
                                  selected_plan);
    if (tdg_debug_enabled())
        fprintf(stderr, "[tdg.univ] workers=%d tasks=%d level_budget=%d loop_budget=%d reason=one-worker-per-task\n",
                workers, task_count, level_budget_threads,
                selected_plan ? selected_plan->loop_budget : 0);
    return workers;
}

static int sgpl_loop_try_cached_decision(sgpl_loop_runtime_state *state,
                                         const sgpl_loop_profile_desc *desc,
                                         int32_t effective_threads,
                                         int32_t *cached_choice_out)
{
    if (cached_choice_out)
        *cached_choice_out = 0;

    if (!state || !desc || !state->decision_cache_valid)
        return 0;
    if (state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
        return 0;

    if (state->cached_threads != effective_threads ||
        state->cached_budget_bucket != effective_threads ||
        state->cached_sampling_epoch != state->c_sampling_epoch)
        return 0;

    if (desc->mode == SGPL_LOOP_DOACROSS &&
        state->cached_sync_sampling_epoch != state->sync_sampling_epoch)
        return 0;

    if (cached_choice_out)
        *cached_choice_out = state->cached_choose_parallel;
    return 1;
}

static void sgpl_loop_store_cached_decision(sgpl_loop_runtime_state *state,
                                            const sgpl_loop_profile_desc *desc,
                                            int32_t effective_threads,
                                            int choose_parallel)
{
    if (!state || !desc)
        return;

    state->decision_cache_valid = 1;
    state->cached_choose_parallel = choose_parallel ? 1 : 0;
    state->cached_threads = effective_threads;
    state->cached_budget_bucket = effective_threads;
    state->cached_trip_bucket = state->regime_trip_bucket;
    state->cached_sampling_epoch = state->c_sampling_epoch;
    state->cached_sync_sampling_epoch = state->sync_sampling_epoch;
    state->cached_runtime_kind = desc->runtime_kind;
    state->cached_mode = desc->mode;
    state->cached_env_size = desc->env_size;
    state->cached_num_priv_targets = desc->num_priv_targets;
    state->cached_waits_per_iter = desc->doacross_waits_per_iter;
    state->cached_posts_per_iter = desc->doacross_posts_per_iter;
    state->cached_doacross_num_sync_ids = desc->doacross_num_sync_ids;
}

static int sgpl_loop_prepare_c_sampling(sgpl_loop_runtime_state *state,
                                        const sgpl_loop_profile_desc *desc,
                                        int64_t trip_count,
                                        const char **reason_out)
{
    if (reason_out)
        *reason_out = NULL;

    if (!state)
    {
        if (reason_out)
            *reason_out = "missing-state";
        return 0;
    }
    if (!desc)
    {
        if (reason_out)
            *reason_out = "missing-desc";
        return 0;
    }

    if (!state->regime_valid)
    {
        sgpl_loop_set_regime(state, desc, trip_count, SGPL_C_SAMPLING_WARMING);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=initial trip_bucket=%d state=%s\n",
                    sgpl_loop_debug_name(desc),
                    desc->loop_id,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state));
        }
    }
    else if (!sgpl_loop_regime_matches(state, desc, trip_count))
    {
        sgpl_loop_set_regime(state, desc, trip_count, SGPL_C_SAMPLING_RECALIBRATING);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=signature-shift trip_bucket=%d state=%s mode=%d runtime_kind=%d env=%lld targets=%d waits=%d posts=%d sync_ids=%d\n",
                    sgpl_loop_debug_name(desc),
                    desc->loop_id,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    desc->mode,
                    desc->runtime_kind,
                    (long long)desc->env_size,
                    desc->num_priv_targets,
                    desc->doacross_waits_per_iter,
                    desc->doacross_posts_per_iter,
                    desc->doacross_num_sync_ids);
        }
    }

    if (state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
    {
        if (reason_out)
            *reason_out = state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING ? "recalibrating" : "warmup";
        return 0;
    }

    return 1;
}

int32_t sgpl_choose_tdg_threads(int64_t work_units, int64_t span_units, int32_t task_count)
{
    int32_t available;
    (void)work_units;
    (void)span_units;
    if (task_count <= 0)
        return 0;
    available = sgpl_budget_available_threads();
    return task_count < available ? task_count : available;
}

static void sgpl_run_tdg_fifo_reserved(const sgpl_tdg_task_desc *tasks,
                        int32_t task_count,
                        int64_t work_units,
                        int64_t span_units,
                        int32_t level_budget_threads)
{
    int32_t chosen_threads = 0;
    int32_t granted_threads = level_budget_threads;
    int i = 0;
    int64_t effective_work_units = 0;
    int64_t effective_span_units = 0;
    int32_t loop_threads_total = 0;
    sgpl_level_loop_plan loop_plan;
    sgpl_level_loop_pool loop_pool;

    if (!tasks || task_count <= 0)
        return;

    memset(&loop_plan, 0, sizeof(loop_plan));
    memset(&loop_pool, 0, sizeof(loop_pool));

    for (i = 0; i < task_count; ++i)
    {
        uint32_t samples = 0;
        int used_profile = 0;
        int64_t task_units = sgpl_tdg_effective_task_units(&tasks[i], &samples, &used_profile);

        if (task_units <= 0)
            task_units = 1;
        effective_work_units += task_units;
        if (task_units > effective_span_units)
            effective_span_units = task_units;

        if (tdg_debug_enabled())
        {
            fprintf(stderr,
                    "[tdg.weight] idx=%d profile_id=%d static=%d effective=%lld samples=%u source=%s\n",
                    i,
                    tasks[i].profile_id,
                    tasks[i].static_work_units,
                    (long long)task_units,
                    samples,
                    used_profile ? "profile" : "static");
        }
    }

    if (effective_work_units <= 0 || effective_span_units <= 0)
    {
        effective_work_units = work_units;
        effective_span_units = span_units;
    }

    if (tdg_debug_enabled())
    {
        fprintf(stderr,
                "[tdg.aggregate] compiler_work=%lld compiler_span=%lld effective_work=%lld effective_span=%lld tasks=%d\n",
                (long long)work_units,
                (long long)span_units,
                (long long)effective_work_units,
                (long long)effective_span_units,
                task_count);
    }

    if (granted_threads <= 1)
    {
        int32_t prior_scope = 0;
        int32_t old_depth = 0;
        sgpl_level_loop_pool *saved_pool = NULL;
        int32_t saved_pending_loop = -1;
        int32_t saved_task_slot = -1;

        sgpl_loop_pool_init(&loop_pool, NULL);
        loop_pool.active = 1;
        loop_pool.task_count = task_count;
        sgpl_tdg_enter_budget_scope(1, 0, &prior_scope, &old_depth);
        sgpl_bind_level_loop_pool(&loop_pool, &saved_pool, &saved_pending_loop);
        if (tdg_debug_enabled())
        {
            fprintf(stderr,
                    "[tdg.launch] decision=serial chosen_threads=%d launched_threads=0 tasks=%d work=%lld span=%lld reason=budget-fallback\n",
                    chosen_threads,
                    task_count,
                    (long long)effective_work_units,
                    (long long)effective_span_units);
        }
        for (i = 0; i < task_count; ++i)
        {
            if (tasks[i].fn)
            {
                saved_task_slot = g_tls_current_tdg_task_slot;
                g_tls_current_tdg_task_slot = i;
                uint64_t t0 = sgpl_now_ns();
                SGPL_TDG_INVOKE(&tasks[i]);
                g_tls_current_tdg_task_slot = saved_task_slot;
                sgpl_record_tdg_profile_sample(&tasks[i],
                                               i,
                                               (double)(sgpl_now_ns() - t0),
                                               "serial-budget-fallback");
            }
        }
        sgpl_unbind_level_loop_pool(saved_pool, saved_pending_loop);
        sgpl_tdg_exit_budget_scope(prior_scope, old_depth);
        sgpl_loop_pool_destroy(&loop_pool);
        return;
    }

    level_budget_threads = granted_threads;
    chosen_threads = sgpl_choose_tdg_threads_with_loop_budget(effective_work_units,
                                                              effective_span_units,
                                                              tasks, task_count,
                                                              level_budget_threads,
                                                              &loop_plan);
    loop_threads_total = level_budget_threads - chosen_threads;
    sgpl_tdg_print_level_plan(&loop_plan, effective_work_units, effective_span_units, level_budget_threads);
    sgpl_tdg_apply_forced_widths(&loop_plan);

    sgpl_loop_pool_init(&loop_pool, &loop_plan);
    loop_pool.task_count = task_count;

    if (chosen_threads <= 1)
    {
        int32_t prior_scope = 0;
        int32_t old_depth = 0;
        sgpl_level_loop_pool *saved_pool = NULL;
        int32_t saved_pending_loop = -1;
        int32_t saved_task_slot = -1;

        sgpl_tdg_enter_budget_scope(1, loop_threads_total, &prior_scope, &old_depth);
        sgpl_bind_level_loop_pool(&loop_pool, &saved_pool, &saved_pending_loop);

        for (i = 0; i < task_count; ++i)
        {
            if (tasks[i].fn)
            {
                saved_task_slot = g_tls_current_tdg_task_slot;
                g_tls_current_tdg_task_slot = i;
                uint64_t t0 = sgpl_now_ns();
                SGPL_TDG_INVOKE(&tasks[i]);
                g_tls_current_tdg_task_slot = saved_task_slot;
                sgpl_record_tdg_profile_sample(&tasks[i],
                                               i,
                                               (double)(sgpl_now_ns() - t0),
                                               "serial-budget-scope");
            }
        }

        sgpl_unbind_level_loop_pool(saved_pool, saved_pending_loop);
        sgpl_tdg_exit_budget_scope(prior_scope, old_depth);
        sgpl_loop_pool_destroy(&loop_pool);
        return;
    }

    {
        pthread_t *threads = (pthread_t *)malloc((size_t)chosen_threads * sizeof(pthread_t));
        sgpl_tdg_worker_arg *args = (sgpl_tdg_worker_arg *)malloc((size_t)chosen_threads * sizeof(sgpl_tdg_worker_arg));
        sgpl_tdg_level_state state;
        int32_t started = 0;

        /* Deterministic fault injection exercises the serial allocation
         * fallback and its parent budget binding in runtime validation. */
        if (!threads || !args || sgpl_tdg_env_enabled("SGPL_TDG_TEST_ALLOC_FAIL", 0))
        {
            sgpl_level_loop_pool *saved_pool = NULL;
            int32_t saved_pending_loop = -1;
            int32_t saved_task_slot = -1;
            int32_t prior_scope = 0;
            int32_t old_depth = 0;

            free(threads);
            free(args);
            /* Allocation fallback runs callbacks serially, with no loop team.
             * The public level executor still owns the reservation. */
            atomic_store(&loop_pool.remaining_threads, 0);
            if (tdg_debug_enabled())
            {
                fprintf(stderr,
                        "[tdg.launch] decision=serial chosen_threads=%d launched_threads=0 tasks=%d work=%lld span=%lld reason=allocation-fallback\n",
                        chosen_threads,
                        task_count,
                        (long long)effective_work_units,
                        (long long)effective_span_units);
            }
            sgpl_tdg_enter_budget_scope(1, 0, &prior_scope, &old_depth);
            sgpl_bind_level_loop_pool(&loop_pool, &saved_pool, &saved_pending_loop);
            for (i = 0; i < task_count; ++i)
            {
                if (tasks[i].fn)
                {
                    saved_task_slot = g_tls_current_tdg_task_slot;
                    g_tls_current_tdg_task_slot = i;
                    uint64_t t0 = sgpl_now_ns();
                    SGPL_TDG_INVOKE(&tasks[i]);
                    g_tls_current_tdg_task_slot = saved_task_slot;
                    sgpl_record_tdg_profile_sample(&tasks[i],
                                                   i,
                                                   (double)(sgpl_now_ns() - t0),
                                                   "serial-allocation-fallback");
                }
            }
            sgpl_unbind_level_loop_pool(saved_pool, saved_pending_loop);
            sgpl_tdg_exit_budget_scope(prior_scope, old_depth);
            sgpl_loop_pool_destroy(&loop_pool);
            return;
        }

        state.tasks = tasks;
        state.task_count = task_count;
        state.loop_pool = &loop_pool;
        atomic_init(&state.next_index, chosen_threads);
        atomic_init(&state.active_count, 0);
        atomic_init(&state.max_active, 0);

        if (tdg_debug_enabled())
        {
            fprintf(stderr,
                    "[tdg.launch] decision=parallel chosen_threads=%d launched_threads=%d loop_threads=%d tasks=%d work=%lld span=%lld reserved=%d dominant_loop=%d planned_sites=%d\n",
                    chosen_threads,
                    chosen_threads,
                    loop_threads_total,
                    task_count,
                    (long long)effective_work_units,
                    (long long)effective_span_units,
                    atomic_load(&g_sgpl_reserved_threads),
                    loop_plan.dominant_loop_id,
                    loop_plan.entry_count);
            for (i = 0; i < loop_plan.entry_count; ++i)
            {
                fprintf(stderr,
                        "[tdg.loop-plan] task_slot=%d loop_id=%d assigned_threads=%d c_ns=%.2f selection_model_ns=%.2f\n",
                        loop_plan.entries[i].task_slot,
                        loop_plan.entries[i].loop_id,
                        loop_plan.entries[i].assigned_threads,
                        loop_plan.entries[i].c_rank_ns,
                        loop_plan.entries[i].model_ns);
            }
        }

        for (i = 0; i < chosen_threads; ++i)
        {
            args[i].state = &state;
            args[i].loop_threads = loop_threads_total;
            args[i].scope_threads = chosen_threads;
            args[i].first_index = i;
            if (pthread_create(&threads[started], NULL, sgpl_tdg_worker_main, &args[i]) == 0)
                ++started;
            else
                sgpl_tdg_worker_main(&args[i]);
        }

        for (i = 0; i < started; ++i)
            pthread_join(threads[i], NULL);

        if (tdg_debug_enabled())
            fprintf(stderr, "[tdg.overlap] tasks=%d max_active=%d\n",
                    task_count, atomic_load(&state.max_active));

        free(threads);
        free(args);
        sgpl_loop_pool_destroy(&loop_pool);
    }
}

/* Cold/unsupported/nested levels retain one bounded task queue in source
 * order. This executor never partitions ordinary callbacks from loop tasks. */
void sgpl_run_tdg_level_fifo(const sgpl_tdg_task_desc *tasks,
                        int32_t task_count,
                        int64_t work_units,
                        int64_t span_units)
{
    if (!tasks || task_count <= 0)
        return;
    int32_t budget = sgpl_budget_try_reserve(sgpl_budget_available_threads());
    sgpl_run_tdg_fifo_reserved(tasks, task_count, work_units, span_units, budget);
    sgpl_budget_release(budget);
}

#include "tdg_scheduler.inc"

void sgpl_run_tdg_level(const sgpl_tdg_task_desc *tasks, int32_t count,
                        int64_t work_units, int64_t span_units)
{
    sgpl_run_tdg_level_joint(tasks, count, work_units, span_units);
}

/* Ignore the cold first dispatch and use a median: scheduler interruptions
 * during one calibration should not become a permanent launch threshold. */
static double sgpl_launch_sample_median(double samples[5])
{
    for (int i = 0; i < 5; ++i)
        for (int j = i + 1; j < 5; ++j)
            if (samples[j] < samples[i])
            {
                double tmp = samples[i];
                samples[i] = samples[j];
                samples[j] = tmp;
            }
    return samples[2];
}

static double sgpl_calibrate_thread_launch_overhead_ns(int threads)
{
    int sample = 0;
    double samples[5];

    for (sample = 0; sample < 6; ++sample)
    {
        uint64_t t0 = sgpl_now_ns();
        sgpl_parallel_launch_plain_raw(0,
                                       threads,
                                       1,
                                       sgpl_noop_body,
                                       NULL,
                                       0,
                                       0,
                                       threads);

        {
            double sample_ns = (double)(sgpl_now_ns() - t0);
            if (sample > 0)
                samples[sample - 1] = sample_ns;
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] launch-thread-calibration-sample sample=%d threads=%d sample_L_thread_ns=%.2f\n",
                        sample + 1,
                        threads,
                        sample_ns);
            }
        }
    }

    return sgpl_launch_sample_median(samples);
}

static double sgpl_calibrate_total_launch_overhead_ns(const sgpl_loop_profile_desc *desc, int threads)
{
    int sample = 0;
    double samples[5];

    for (sample = 0; sample < 6; ++sample)
    {
        uint64_t t0 = sgpl_now_ns();

        if (desc && desc->runtime_kind == SGPL_RUNTIME_PRIVATIZED &&
            desc->env_size > 0 && desc->priv_offsets && desc->num_priv_targets > 0)
        {
            void *env = calloc(1, (size_t)desc->env_size);
            RoaringBitmap **originals = (RoaringBitmap **)calloc((size_t)desc->num_priv_targets, sizeof(RoaringBitmap *));
            int target = 0;

            if (!env || !originals)
            {
                free(env);
                free(originals);
                return 0.0;
            }

            for (target = 0; target < desc->num_priv_targets; ++target)
            {
                int64_t offset = desc->priv_offsets[target];
                if (offset < 0 || offset + (int64_t)sizeof(RoaringBitmap *) > desc->env_size)
                    continue;
                originals[target] = roaring_bitmap_create(64 * 1024, 8);
                *(RoaringBitmap **)((char *)env + offset) = originals[target];
            }

            sgpl_parallel_launch_priv_raw(0,
                                          threads,
                                          1,
                                          sgpl_noop_body,
                                          env,
                                          desc->env_size,
                                          desc->priv_offsets,
                                          NULL,
                                          NULL,
                                          desc->num_priv_targets,
                                          desc && desc->mode == SGPL_LOOP_DOACROSS ? 1 : 0,
                                          desc ? desc->doacross_num_sync_ids : 0,
                                          threads);

            for (target = 0; target < desc->num_priv_targets; ++target)
                roaring_bitmap_free(originals[target]);

            free(originals);
            free(env);
        }
        else
        {
            sgpl_parallel_launch_plain_raw(0,
                                           threads,
                                           1,
                                           sgpl_noop_body,
                                           NULL,
                                           desc && desc->mode == SGPL_LOOP_DOACROSS ? 1 : 0,
                                           desc ? desc->doacross_num_sync_ids : 0,
                                           threads);
        }

        {
            double sample_ns = (double)(sgpl_now_ns() - t0);
            if (sample > 0)
                samples[sample - 1] = sample_ns;
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] launch-total-calibration-sample loop=%s kind=%d mode=%d sample=%d threads=%d env=%lld targets=%d sync_ids=%d sample_L_total_ns=%.2f\n",
                        sgpl_loop_debug_name(desc),
                        desc ? desc->runtime_kind : SGPL_RUNTIME_PLAIN,
                        desc ? desc->mode : SGPL_LOOP_DOALL,
                        sample + 1,
                        threads,
                        (long long)(desc ? desc->env_size : 0),
                        desc ? desc->num_priv_targets : 0,
                        desc ? desc->doacross_num_sync_ids : 0,
                        sample_ns);
            }
        }
    }

    return sgpl_launch_sample_median(samples);
}

static double sgpl_get_thread_launch_overhead_ns(int threads)
{
    double l_thread_ns = 0.0;
    int ephemeral = g_tls_is_pool_worker || !sgpl_thread_pool_available();

    if (threads <= 0)
        return 0.0;

    pthread_mutex_lock(&g_launch_model_lock);
    if (threads < SGPL_TDG_MAX_REPORTED_THREADS && g_launch_thread_cost_valid[ephemeral][threads])
    {
        l_thread_ns = g_launch_thread_costs[ephemeral][threads];
        pthread_mutex_unlock(&g_launch_model_lock);
        return l_thread_ns;
    }
    pthread_mutex_unlock(&g_launch_model_lock);

    l_thread_ns = sgpl_calibrate_thread_launch_overhead_ns(threads);
    pthread_mutex_lock(&g_launch_model_lock);
    if (threads < SGPL_TDG_MAX_REPORTED_THREADS)
    {
        g_launch_thread_costs[ephemeral][threads] = l_thread_ns;
        g_launch_thread_cost_valid[ephemeral][threads] = 1;
    }
    pthread_mutex_unlock(&g_launch_model_lock);

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] launch-thread-calibration threads=%d L_thread_ns(median)=%.2f\n",
                threads,
                l_thread_ns);
    }

    return l_thread_ns;
}

static double sgpl_get_path_launch_overhead_ns(const sgpl_loop_profile_desc *desc, int threads)
{
    int i = 0;
    double total_ns = 0.0;
    double path_ns = 0.0;
    int runtime_kind = desc ? desc->runtime_kind : SGPL_RUNTIME_PLAIN;
    int mode = desc ? desc->mode : SGPL_LOOP_DOALL;
    int64_t env_size = desc ? desc->env_size : 0;
    int32_t num_priv_targets = desc ? desc->num_priv_targets : 0;
    int32_t doacross_num_sync_ids = desc ? desc->doacross_num_sync_ids : 0;
    int ephemeral = g_tls_is_pool_worker || !sgpl_thread_pool_available();

    if (runtime_kind == SGPL_RUNTIME_PLAIN && mode == SGPL_LOOP_DOALL)
        return 0.0;

    pthread_mutex_lock(&g_launch_model_lock);
    for (i = 0; i < SGPL_MAX_LAUNCH_KEYS; ++i)
    {
        if (!g_launch_path_costs[i].in_use)
            continue;

        if (g_launch_path_costs[i].ephemeral == ephemeral &&
            g_launch_path_costs[i].runtime_kind == runtime_kind &&
            g_launch_path_costs[i].mode == mode &&
            g_launch_path_costs[i].threads == threads &&
            g_launch_path_costs[i].env_size == env_size &&
            g_launch_path_costs[i].num_priv_targets == num_priv_targets &&
            g_launch_path_costs[i].doacross_num_sync_ids == doacross_num_sync_ids)
        {
            path_ns = g_launch_path_costs[i].path_ns;
            pthread_mutex_unlock(&g_launch_model_lock);
            return path_ns;
        }
    }
    pthread_mutex_unlock(&g_launch_model_lock);

    total_ns = sgpl_calibrate_total_launch_overhead_ns(desc, threads);
    path_ns = total_ns - sgpl_get_thread_launch_overhead_ns(threads);
    if (path_ns < 0.0)
        path_ns = 0.0;

    pthread_mutex_lock(&g_launch_model_lock);
    for (i = 0; i < SGPL_MAX_LAUNCH_KEYS; ++i)
    {
        if (!g_launch_path_costs[i].in_use)
        {
            g_launch_path_costs[i].in_use = 1;
            g_launch_path_costs[i].ephemeral = ephemeral;
            g_launch_path_costs[i].runtime_kind = runtime_kind;
            g_launch_path_costs[i].mode = mode;
            g_launch_path_costs[i].threads = threads;
            g_launch_path_costs[i].env_size = env_size;
            g_launch_path_costs[i].num_priv_targets = num_priv_targets;
            g_launch_path_costs[i].doacross_num_sync_ids = doacross_num_sync_ids;
            g_launch_path_costs[i].path_ns = path_ns;
            pthread_mutex_unlock(&g_launch_model_lock);

            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] launch-path-calibration loop=%s kind=%d mode=%d threads=%d env=%lld targets=%d sync_ids=%d L_path_ns(median)=%.2f\n",
                        sgpl_loop_debug_name(desc),
                        runtime_kind,
                        mode,
                        threads,
                        (long long)env_size,
                        num_priv_targets,
                        doacross_num_sync_ids,
                        path_ns);
            }
            return path_ns;
        }
    }
    pthread_mutex_unlock(&g_launch_model_lock);

    return path_ns;
}

static sgpl_launch_overhead_detail sgpl_get_launch_overhead_detail(const sgpl_loop_profile_desc *desc, int threads)
{
    sgpl_launch_overhead_detail detail;
    /* Price the launch path that the runtime actually uses. The pool includes
     * two barriers over ALL configured workers, including idle lanes; DOACROSS
     * bypasses it and creates/join temporary threads. A fixed microsecond seed
     * cannot represent either path on different hosts.
     * Calibration must not consume the range entry point of the real launch. */
    loop_range_fn saved_range_body = g_tls_pending_range_body;
    g_tls_pending_range_body = NULL;
    detail.thread_ns = sgpl_get_thread_launch_overhead_ns(threads);
    detail.path_ns = sgpl_get_path_launch_overhead_ns(desc, threads);
    detail.total_ns = detail.thread_ns + detail.path_ns;
    g_tls_pending_range_body = saved_range_body;
    return detail;
}


// Minimum trip count at which a DOALL loop takes the parallel path before the
// cost model has calibrated.
//
// The model refuses to parallelise until it has a stable per-iteration cost
// estimate, which it builds from serial samples.  A loop that runs once never
// gets there, and one that runs a few times may not either: measured on
// PageRank, 22 invocations produced 22 serial choices, cycling
// warmup-uninitialized -> warmup -> missing-c-dep-or-c-ind.  So the autotuner
// was permanently in warmup on exactly the workloads it exists to speed up.
//
// Dispatch overhead is a few microseconds; at 1M iterations even a 1 ns body is
// a millisecond of work, so spreading is safe well below that.  The default is
// deliberately conservative.  Sampling continues, so the model still calibrates
// and can override this on later invocations.
static int64_t sgpl_warmup_parallel_min_trips(void)
{
    static int64_t cached = -1;
    if (cached < 0)
    {
        const char *v = getenv("SGPL_WARMUP_PARALLEL_MIN_TRIPS");
        cached = 65536;
        if (v && *v)
        {
            long long parsed = atoll(v);
            if (parsed > 0)
                cached = (int64_t)parsed;
        }
    }
    return cached;
}

static double sgpl_min_parallel_c_ns(void)
{
    static double cached = -1.0;
    if (cached < 0.0)
    {
        const char *v = getenv("SGPL_MIN_PARALLEL_C_NS");
        cached = 3.0;
        if (v && *v)
        {
            double parsed = atof(v);
            if (parsed >= 0.0)
                cached = parsed;
        }
    }
    return cached;
}

static int sgpl_warmup_calibration_disabled(void)
{
    static int cached = -1;
    if (cached < 0)
    {
        const char *v = getenv("SGPL_NO_WARMUP_CALIBRATION");
        cached = (v && *v && strcmp(v, "0") != 0) ? 1 : 0;
    }
    return cached;
}

/* A decision taken before the sampler is STABLE must not be cached: the
 * DOACROSS path would otherwise freeze its very first (warming, serial)
 * verdict for the life of the process and never calibrate.  Validation knob,
 * default off (SGPL_NO_WARMUP_DECISION_CACHE=1 enables). */
static int sgpl_should_cache_decision(const sgpl_loop_runtime_state *state)
{
    static int no_warm_cache = -1;
    if (no_warm_cache < 0)
    {
        const char *v = getenv("SGPL_NO_WARMUP_DECISION_CACHE");
        no_warm_cache = v && *v && strcmp(v, "0") != 0;
    }
    if (no_warm_cache && state && state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
        return 0;
    return 1;
}

int32_t sgpl_should_parallelize_doall(const sgpl_loop_profile_desc *desc,
                                      int64_t start,
                                      int64_t end,
                                      int64_t step)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    int64_t trip_count = sgpl_compute_trip_count(start, end, step);
    double c_ns = 0.0;
    double l_thread_ns = 0.0;
    double l_path_ns = 0.0;
    double l_ns = 0.0;
    double threshold = 0.0;
    double speedup_factor = 0.0;
    double denominator = 0.0;
    int choose_parallel = 0;
    int32_t effective_threads = 0;
    int32_t cached_choice = 0;

    if (state)
    {
        state->invocation_count++;
        state->last_observed_trip_count = trip_count;
        if (trip_count > 0)
        {
            if (state->observed_trip_count_ewma <= 0.0)
                state->observed_trip_count_ewma = (double)trip_count;
            else
                state->observed_trip_count_ewma = 0.25 * (double)trip_count + 0.75 * state->observed_trip_count_ewma;
        }
    }

    effective_threads = sgpl_loop_effective_decision_threads_for_loop_id(desc ? desc->loop_id : -1);

    if (force_doall_parallel_enabled() && effective_threads > 1 && trip_count > 0)
    {
        // Explicit test override: bypass cost-model warmup for ablation benches.
        g_tls_pending_loop_id = desc ? desc->loop_id : -1;
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] cost-doall loop=%s loop_id=%d choose=parallel reason=force-env P=%d N=%lld\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    effective_threads,
                    (long long)trip_count);
        }
        return 1;
    }

    if (effective_threads <= 1 || trip_count <= 0 || !state)
    {
        if (runtime_debug_enabled())
        {
            long decision_index = atomic_fetch_add(&g_doall_decision_calls, 1) + 1;
            fprintf(stderr,
                    "[parallel-runtime] cost-doall loop=%s loop_id=%d invocation=%ld choose=serial reason=%s N=%lld P=%d bucket=%d c_state=%s c_ns=%.2f L_thread_ns=%.2f L_path_ns=%.2f L_total_ns=%.2f samples=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    effective_threads <= 1 ? "single-thread" : trip_count <= 0 ? "empty-trip-count" : "missing-state",
                    (long long)trip_count,
                    effective_threads,
                    state ? state->regime_trip_bucket : -1,
                    state ? sgpl_c_sampling_state_name(state->c_sampling_state) : "none",
                    state ? state->c_ns_per_iter_ewma : 0.0,
                    0.0,
                    0.0,
                    0.0,
                    state ? state->serial_samples : 0);
            fprintf(stderr,
                    "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=serial-original\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index);
        }
        g_tls_pending_loop_id = -1;
        return 0;
    }

    if (sgpl_loop_try_cached_decision(state, desc, effective_threads, &cached_choice))
    {
        if (runtime_debug_enabled())
        {
            long decision_index = atomic_fetch_add(&g_doall_decision_calls, 1) + 1;
            fprintf(stderr,
                    "[parallel-runtime] decision-cache-hit loop=%s loop_id=%d invocation=%ld bucket=%d c_state=%s threads=%d choose=%s epoch=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    effective_threads,
                    cached_choice ? "parallel" : "serial",
                    state->c_sampling_epoch);
            fprintf(stderr,
                    "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=%s\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    cached_choice ? "parallel-clone" : "serial-original");
        }
        g_tls_pending_loop_id = (cached_choice && desc) ? desc->loop_id : -1;
        return cached_choice ? 1 : 0;
    }

    if (!state->regime_valid || state->c_sampling_state != SGPL_C_SAMPLING_STABLE || state->c_ns_per_iter_ewma <= 0.0)
    {
        // Uncalibrated, but the loop is big enough that parallelism pays for
        // itself regardless of the per-iteration cost.
        //
        // ... except that taking this exit *is* what keeps the loop
        // uncalibrated.  c_ns_per_iter_ewma is fed only by
        // sgpl_record_doall_serial_sample(), which runs on the serial path, so a
        // loop that clears the trip threshold on its first invocation goes
        // parallel forever and the cost model never gets a single sample.  The
        // shortcut is a floor for loops that genuinely cannot afford a serial
        // pass, not a permanent answer.
        //
        // So spend a bounded number of invocations on calibration: invocation 1
        // still goes parallel (a loop called exactly once must not be made to
        // pay for a measurement it will never use), and the next
        // SGPL_C_INITIAL_BATCH_SAMPLES invocations fall through to serial to
        // fill the batch.  From then on the model is STABLE and decides on the
        // real per-iteration cost -- which is the difference between "spread a
        // memory-bound copy over 4 threads for nothing" and "spread a loop with
        // real work per element".  If the batch still has not filled (the loop
        // is genuinely rare), the shortcut resumes.
        // SGPL_NO_WARMUP_CALIBRATION=1 restores the unconditional shortcut.
        int calibrating =
            !sgpl_warmup_calibration_disabled() &&
            state->c_sampling_state != SGPL_C_SAMPLING_STABLE &&
            state->invocation_count > 1 &&
            state->invocation_count <= (uint64_t)(1 + SGPL_C_INITIAL_BATCH_SAMPLES);

        if (!calibrating && trip_count >= sgpl_warmup_parallel_min_trips() && effective_threads > 1)
        {
            g_tls_pending_loop_id = desc ? desc->loop_id : -1;
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] cost-doall loop=%s loop_id=%d choose=parallel "
                        "reason=warmup-large-trip N=%lld P=%d\n",
                        sgpl_loop_debug_name(desc),
                        desc ? desc->loop_id : -1,
                        (long long)trip_count,
                        effective_threads);
            }
            return 1;
        }
        const char *reason = !state->regime_valid ? "warmup-uninitialized"
                             : state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING ? "recalibrating"
                             : state->c_sampling_state == SGPL_C_SAMPLING_WARMING ? "warmup"
                             : "missing-c";
        if (runtime_debug_enabled())
        {
            long decision_index = atomic_fetch_add(&g_doall_decision_calls, 1) + 1;
            fprintf(stderr,
                    "[parallel-runtime] cost-doall loop=%s loop_id=%d invocation=%ld choose=serial reason=%s N=%lld P=%d bucket=%d c_state=%s c_ns=%.2f L_thread_ns=%.2f L_path_ns=%.2f L_total_ns=%.2f samples=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    reason,
                    (long long)trip_count,
                    effective_threads,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    state->c_ns_per_iter_ewma,
                    0.0,
                    0.0,
                    0.0,
                    state->serial_samples);
            fprintf(stderr,
                    "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=serial-original\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index);
        }
        g_tls_pending_loop_id = -1;
        return 0;
    }

    c_ns = state->c_ns_per_iter_ewma;

    /* Memory-bandwidth floor.
     *
     * The model below assumes a parallel loop runs (1 - 1/P) faster, i.e. that
     * adding threads adds throughput.  That holds while the loop is limited by
     * compute.  It does not hold once the loop is limited by memory bandwidth:
     * a second thread streaming the same arrays shares one bus and adds
     * nothing, so the model predicts a win and delivers a small loss.
     *
     * The per-iteration cost the model already measures separates the two
     * cleanly.  Measured on a 4-core box, 1e6-element vertex loops, best of 3,
     * against the serial path (see the range entry point in worker_main --
     * these numbers are only meaningful with it):
     *
     *   flops/iter   c_ns    speedup
     *      0          ~1.0    0.88x
     *      2          ~1.4    0.96x
     *      8          ~5.6    1.72x
     *     24         ~31.4    2.80x
     *
     * The crossover sits between 1.4 and 5.6 ns/iteration; below it the loop is
     * moving bytes, not doing arithmetic.  Loops under the floor stay serial.
     * SGPL_MIN_PARALLEL_C_NS overrides it; 0 disables the floor. */
    if (c_ns > 0.0 && c_ns < sgpl_min_parallel_c_ns())
    {
        sgpl_loop_store_cached_decision(state, desc, effective_threads, 0);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] cost-doall loop=%s loop_id=%d choose=serial "
                    "reason=memory-bound c_ns=%.2f floor=%.2f N=%lld P=%d\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    c_ns,
                    sgpl_min_parallel_c_ns(),
                    (long long)trip_count,
                    effective_threads);
        }
        g_tls_pending_loop_id = -1;
        return 0;
    }

    {
        sgpl_launch_overhead_detail launch = sgpl_get_launch_overhead_detail(desc, effective_threads);
        l_thread_ns = launch.thread_ns;
        l_path_ns = launch.path_ns;
        l_ns = launch.total_ns;
    }
    speedup_factor = 1.0 - sgpl_loop_max_lane_iterations(trip_count, effective_threads, 0) / (double)trip_count;
    denominator = c_ns * speedup_factor;
    if (denominator > 0.0)
        threshold = l_ns / denominator;

    choose_parallel = denominator > 0.0 && (double)trip_count > threshold;
    if (sgpl_should_cache_decision(state))
        sgpl_loop_store_cached_decision(state, desc, effective_threads, choose_parallel);

    if (runtime_debug_enabled())
    {
        long decision_index = atomic_fetch_add(&g_doall_decision_calls, 1) + 1;
        fprintf(stderr,
                "[parallel-runtime] decision-cache-miss loop=%s loop_id=%d invocation=%ld bucket=%d c_state=%s threads=%d choose=%s epoch=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                state->regime_trip_bucket,
                sgpl_c_sampling_state_name(state->c_sampling_state),
                effective_threads,
                choose_parallel ? "parallel" : "serial",
                state->c_sampling_epoch);
        fprintf(stderr,
                "[parallel-runtime] cost-doall loop=%s loop_id=%d invocation=%ld choose=%s N=%lld P=%d c_ns=%.2f L_thread_ns=%.2f L_path_ns=%.2f L_total_ns=%.2f speedup=(1-max_lane/N)=%.6f denominator=c*speedup=%.6f threshold=L/denominator=%.2f samples=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel" : "serial",
                (long long)trip_count,
                effective_threads,
                c_ns,
                l_thread_ns,
                l_path_ns,
                l_ns,
                speedup_factor,
                denominator,
                threshold,
                state->serial_samples);
        fprintf(stderr,
                "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=%s\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel-clone" : "serial-original");
    }

    g_tls_pending_loop_id = (choose_parallel && desc) ? desc->loop_id : -1;
    return choose_parallel ? 1 : 0;
}

void sgpl_record_doall_serial_sample(const sgpl_loop_profile_desc *desc,
                                     int64_t start,
                                     int64_t end,
                                     int64_t step,
                                     uint64_t elapsed_ns)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    int64_t trip_count = sgpl_compute_trip_count(start, end, step);
    double c_sample = 0.0;
    const char *sample_type = NULL;
    double drift = 0.0;
    double denom = 0.0;

    sgpl_joint_trace_serial(desc ? desc->loop_id : -1, trip_count, elapsed_ns);

    if (!state || trip_count <= 0 || elapsed_ns == 0)
        return;

    c_sample = (double)elapsed_ns / (double)trip_count;
    if (!state->regime_valid)
    {
        sgpl_loop_set_regime(state,
                             desc,
                             trip_count,
                             SGPL_C_SAMPLING_WARMING);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=initial trip_bucket=%d state=%s\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state));
        }
    }

    if (state->c_sampling_state == SGPL_C_SAMPLING_STABLE)
    {
        state->c_probe_skip_counter++;
        if (state->c_probe_skip_counter < SGPL_C_PROBE_INTERVAL)
            return;

        state->c_probe_skip_counter = 0;
        if (!sgpl_loop_regime_matches(state, desc, trip_count))
        {
            sgpl_loop_set_regime(state, desc, trip_count, SGPL_C_SAMPLING_RECALIBRATING);
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=signature-shift trip_bucket=%d state=%s mode=%d runtime_kind=%d env=%lld targets=%d waits=%d posts=%d sync_ids=%d\n",
                        sgpl_loop_debug_name(desc),
                        desc ? desc->loop_id : -1,
                        state->regime_trip_bucket,
                        sgpl_c_sampling_state_name(state->c_sampling_state),
                        desc->mode,
                        desc->runtime_kind,
                        (long long)desc->env_size,
                        desc->num_priv_targets,
                        desc->doacross_waits_per_iter,
                        desc->doacross_posts_per_iter,
                        desc->doacross_num_sync_ids);
            }
        }

        if (state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
            goto update_doall_batch;

        denom = state->c_ns_per_iter_ewma > 1e-9 ? state->c_ns_per_iter_ewma : 1e-9;
        drift = fabs(c_sample - state->c_ns_per_iter_ewma) / denom;
        if (drift > SGPL_C_DRIFT_THRESHOLD)
            state->c_consecutive_drift++;
        else
            state->c_consecutive_drift = 0;

        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-probe loop=%s loop_id=%d bucket=%d state=%s c_sample=%.2f c_ewma=%.2f drift=%.6f consecutive=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    c_sample,
                    state->c_ns_per_iter_ewma,
                    drift,
                    state->c_consecutive_drift);
        }

        if (state->c_consecutive_drift < SGPL_C_DRIFT_CONFIRMATIONS)
            return;

        state->c_sampling_state = SGPL_C_SAMPLING_RECALIBRATING;
        state->c_batch_samples = 0;
        state->c_consecutive_drift = 0;
        state->c_sampling_epoch += 1;
        sgpl_loop_invalidate_decision_cache(state, desc, "drift-recalibration");
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=drift trip_bucket=%d state=%s drift=%.6f\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    drift);
        }
    }
    else if (!sgpl_loop_regime_matches(state, desc, trip_count))
    {
        sgpl_loop_set_regime(state,
                             desc,
                             trip_count,
                             SGPL_C_SAMPLING_RECALIBRATING);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=signature-shift trip_bucket=%d state=%s mode=%d runtime_kind=%d env=%lld targets=%d waits=%d posts=%d sync_ids=%d\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    desc->mode,
                    desc->runtime_kind,
                    (long long)desc->env_size,
                    desc->num_priv_targets,
                    desc->doacross_waits_per_iter,
                    desc->doacross_posts_per_iter,
                    desc->doacross_num_sync_ids);
        }
    }

update_doall_batch:
    sample_type = state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING ? "recalibration" : "warmup";

    if (state->c_batch_samples == 0)
        state->c_ns_per_iter_ewma = c_sample;
    else
        state->c_ns_per_iter_ewma = 0.25 * c_sample + 0.75 * state->c_ns_per_iter_ewma;

    state->serial_samples += 1;
    state->c_batch_samples += 1;

    if ((state->c_sampling_state == SGPL_C_SAMPLING_WARMING &&
         state->c_batch_samples >= SGPL_C_INITIAL_BATCH_SAMPLES) ||
        (state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING &&
         state->c_batch_samples >= SGPL_C_RECALIBRATION_BATCH_SAMPLES))
    {
        state->c_sampling_epoch += 1;
        sgpl_loop_invalidate_decision_cache(state, desc, "new-c-batch-ready");
        state->c_sampling_state = SGPL_C_SAMPLING_STABLE;
        state->c_batch_samples = 0;
        state->c_probe_skip_counter = 0;
        state->c_consecutive_drift = 0;
    }

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] serial-sample loop=%s loop_id=%d N=%lld bucket=%d state=%s sample_type=%s elapsed_ns=%llu c_sample=elapsed/N=%.2f c_ewma=%.2f samples=%u batch_progress=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                (long long)trip_count,
                state->regime_trip_bucket,
                sgpl_c_sampling_state_name(state->c_sampling_state),
                sample_type,
                (unsigned long long)elapsed_ns,
                c_sample,
                state->c_ns_per_iter_ewma,
                state->serial_samples,
                state->c_batch_samples);
    }
}

void sgpl_record_doacross_serial_sample(const sgpl_loop_profile_desc *desc,
                                        int64_t start,
                                        int64_t end,
                                        int64_t step,
                                        uint64_t dep_elapsed_ns,
                                        uint64_t ind_elapsed_ns)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    int64_t trip_count = sgpl_compute_trip_count(start, end, step);
    double c_dep_sample = 0.0;
    double c_ind_sample = 0.0;
    double drift_dep = 0.0;
    double drift_ind = 0.0;
    double drift = 0.0;
    double denom_dep = 0.0;
    double denom_ind = 0.0;
    const char *sample_type = NULL;

    sgpl_joint_trace_serial(desc ? desc->loop_id : -1, trip_count,
                            dep_elapsed_ns + ind_elapsed_ns);

    if (!state || trip_count <= 0)
        return;

    c_dep_sample = (double)dep_elapsed_ns / (double)trip_count;
    c_ind_sample = (double)ind_elapsed_ns / (double)trip_count;

    if (!state->regime_valid)
    {
        sgpl_loop_set_regime(state,
                             desc,
                             trip_count,
                             SGPL_C_SAMPLING_WARMING);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=initial trip_bucket=%d state=%s\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state));
        }
    }

    if (state->c_sampling_state == SGPL_C_SAMPLING_STABLE)
    {
        state->c_probe_skip_counter++;
        if (state->c_probe_skip_counter < SGPL_C_PROBE_INTERVAL)
            return;

        state->c_probe_skip_counter = 0;
        if (!sgpl_loop_regime_matches(state, desc, trip_count))
        {
            sgpl_loop_set_regime(state, desc, trip_count, SGPL_C_SAMPLING_RECALIBRATING);
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=signature-shift trip_bucket=%d state=%s mode=%d runtime_kind=%d env=%lld targets=%d waits=%d posts=%d sync_ids=%d\n",
                        sgpl_loop_debug_name(desc),
                        desc ? desc->loop_id : -1,
                        state->regime_trip_bucket,
                        sgpl_c_sampling_state_name(state->c_sampling_state),
                        desc->mode,
                        desc->runtime_kind,
                        (long long)desc->env_size,
                        desc->num_priv_targets,
                        desc->doacross_waits_per_iter,
                        desc->doacross_posts_per_iter,
                        desc->doacross_num_sync_ids);
            }
        }

        if (state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
            goto update_doacross_batch;

        denom_dep = state->c_dep_ns_per_iter_ewma > 1e-9 ? state->c_dep_ns_per_iter_ewma : 1e-9;
        denom_ind = state->c_ind_ns_per_iter_ewma > 1e-9 ? state->c_ind_ns_per_iter_ewma : 1e-9;
        drift_dep = fabs(c_dep_sample - state->c_dep_ns_per_iter_ewma) / denom_dep;
        drift_ind = fabs(c_ind_sample - state->c_ind_ns_per_iter_ewma) / denom_ind;
        drift = drift_dep > drift_ind ? drift_dep : drift_ind;

        if (drift > SGPL_C_DRIFT_THRESHOLD)
            state->c_consecutive_drift++;
        else
            state->c_consecutive_drift = 0;

        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-probe-doacross loop=%s loop_id=%d bucket=%d state=%s c_dep_sample=%.2f c_dep_ewma=%.2f c_ind_sample=%.2f c_ind_ewma=%.2f drift=%.6f consecutive=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    c_dep_sample,
                    state->c_dep_ns_per_iter_ewma,
                    c_ind_sample,
                    state->c_ind_ns_per_iter_ewma,
                    drift,
                    state->c_consecutive_drift);
        }

        if (state->c_consecutive_drift < SGPL_C_DRIFT_CONFIRMATIONS)
            return;

        state->c_sampling_state = SGPL_C_SAMPLING_RECALIBRATING;
        state->c_batch_samples = 0;
        state->c_consecutive_drift = 0;
        state->c_sampling_epoch += 1;
        sgpl_loop_invalidate_decision_cache(state, desc, "drift-recalibration");
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=drift-doacross trip_bucket=%d state=%s drift=%.6f\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    drift);
        }
    }
    else if (!sgpl_loop_regime_matches(state, desc, trip_count))
    {
        sgpl_loop_set_regime(state,
                             desc,
                             trip_count,
                             SGPL_C_SAMPLING_RECALIBRATING);
        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] c-regime-change loop=%s loop_id=%d reason=signature-shift trip_bucket=%d state=%s mode=%d runtime_kind=%d env=%lld targets=%d waits=%d posts=%d sync_ids=%d\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    desc->mode,
                    desc->runtime_kind,
                    (long long)desc->env_size,
                    desc->num_priv_targets,
                    desc->doacross_waits_per_iter,
                    desc->doacross_posts_per_iter,
                    desc->doacross_num_sync_ids);
        }
    }

update_doacross_batch:
    sample_type = state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING ? "recalibration" : "warmup";

    if (state->c_batch_samples == 0)
    {
        state->c_dep_ns_per_iter_ewma = c_dep_sample;
        state->c_ind_ns_per_iter_ewma = c_ind_sample;
    }
    else
    {
        state->c_dep_ns_per_iter_ewma = 0.25 * c_dep_sample + 0.75 * state->c_dep_ns_per_iter_ewma;
        state->c_ind_ns_per_iter_ewma = 0.25 * c_ind_sample + 0.75 * state->c_ind_ns_per_iter_ewma;
    }

    state->doacross_serial_samples += 1;
    state->c_batch_samples += 1;

    if ((state->c_sampling_state == SGPL_C_SAMPLING_WARMING &&
         state->c_batch_samples >= SGPL_C_INITIAL_BATCH_SAMPLES) ||
        (state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING &&
         state->c_batch_samples >= SGPL_C_RECALIBRATION_BATCH_SAMPLES))
    {
        state->c_sampling_epoch += 1;
        sgpl_loop_invalidate_decision_cache(state, desc, "new-c-batch-ready");
        state->c_sampling_state = SGPL_C_SAMPLING_STABLE;
        state->c_batch_samples = 0;
        state->c_probe_skip_counter = 0;
        state->c_consecutive_drift = 0;
    }

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] doacross-serial-sample loop=%s loop_id=%d N=%lld bucket=%d state=%s sample_type=%s dep_total_ns=%llu ind_total_ns=%llu c_dep_sample=%.2f c_ind_sample=%.2f c_dep_ewma=%.2f c_ind_ewma=%.2f samples=%u batch_progress=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                (long long)trip_count,
                state->regime_trip_bucket,
                sgpl_c_sampling_state_name(state->c_sampling_state),
                sample_type,
                (unsigned long long)dep_elapsed_ns,
                (unsigned long long)ind_elapsed_ns,
                c_dep_sample,
                c_ind_sample,
                state->c_dep_ns_per_iter_ewma,
                state->c_ind_ns_per_iter_ewma,
                state->doacross_serial_samples,
                state->c_batch_samples);
    }
}

static void sgpl_record_doacross_sync_batch(const sgpl_loop_profile_desc *desc,
                                           int32_t threads, int32_t trip_bucket,
                                           uint64_t wait_elapsed_ns_total,
                                           uint64_t post_elapsed_ns_total,
                                           uint64_t wait_count, uint64_t post_count)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    if (!state || (!wait_count && !post_count))
        return;
    pthread_mutex_lock(&g_doacross_profile_lock);
    sgpl_doacross_width_profile *profile = NULL;
    sgpl_doacross_width_profile *oldest = &state->sync_widths[0];
    for (int i = 0; i < SGPL_DOACROSS_WIDTH_PROFILES; ++i)
    {
        sgpl_doacross_width_profile *candidate = &state->sync_widths[i];
        if (sgpl_doacross_width_matches(candidate, desc, threads, trip_bucket))
        {
            profile = candidate;
            break;
        }
        if (candidate->stamp < oldest->stamp)
            oldest = candidate;
    }
    if (!profile)
    {
        profile = oldest;
        *profile = (sgpl_doacross_width_profile){.threads=threads, .trip_bucket=trip_bucket,
            .runtime_kind=desc->runtime_kind, .env_size=desc->env_size,
            .priv_targets=desc->num_priv_targets, .waits=desc->doacross_waits_per_iter,
            .posts=desc->doacross_posts_per_iter, .sync_ids=desc->doacross_num_sync_ids};
    }
    if (wait_count > 0)
    {
        double sigma_wait = (double)wait_elapsed_ns_total / (double)wait_count;
        if (profile->samples == 0 || profile->wait_ns <= 0.0)
            profile->wait_ns = sigma_wait;
        else
            profile->wait_ns = 0.25 * sigma_wait + 0.75 * profile->wait_ns;
    }
    if (post_count > 0)
    {
        double sigma_post = (double)post_elapsed_ns_total / (double)post_count;
        if (profile->samples == 0 || profile->post_ns <= 0.0)
            profile->post_ns = sigma_post;
        else
            profile->post_ns = 0.25 * sigma_post + 0.75 * profile->post_ns;
    }
    profile->samples += 1;
    profile->stamp = ++state->sync_width_clock;
    state->doacross_sync_samples += 1;
    /* The atomic epoch invalidates decisions without worker writes to cache fields. */
    state->sync_sampling_epoch += 1;
    double wait_ns = profile->wait_ns, post_ns = profile->post_ns;
    pthread_mutex_unlock(&g_doacross_profile_lock);
    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] doacross-sync-sample loop=%s loop_id=%d threads=%d trip_bucket=%d wait_total_ns=%llu post_total_ns=%llu wait_count=%llu post_count=%llu sigma_wait=%.2f sigma_post=%.2f sync_batches=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                threads,
                trip_bucket,
                (unsigned long long)wait_elapsed_ns_total,
                (unsigned long long)post_elapsed_ns_total,
                (unsigned long long)wait_count,
                (unsigned long long)post_count,
                wait_ns,
                post_ns,
                state->doacross_sync_samples);
    }
}

void sgpl_record_doacross_sync_sample(const sgpl_loop_profile_desc *desc,
                                      uint64_t wait_elapsed_ns_total,
                                      uint64_t post_elapsed_ns_total,
                                      uint64_t wait_count, uint64_t post_count)
{
    sgpl_doacross_sync_lane_sample *lane = g_tls_doacross_sync_lane;
    if (desc && lane && (!lane->desc || lane->desc == desc))
    {
        lane->desc = desc;
        lane->wait_ns += wait_elapsed_ns_total;
        lane->post_ns += post_elapsed_ns_total;
        lane->waits += wait_count;
        lane->posts += post_count;
        lane->samples += 1;
        return;
    }
    /* Standalone callers have no known parallel grant; never label them with
     * configured machine width. A rare second descriptor uses the actual team. */
    sgpl_doacross_state *region = g_tls_doacross_state;
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    sgpl_record_doacross_sync_batch(desc, region ? region->profile_threads : 1,
        region ? region->profile_trip_bucket : (state ? state->regime_trip_bucket : 0),
        wait_elapsed_ns_total, post_elapsed_ns_total, wait_count, post_count);
}

static void sgpl_commit_doacross_profile(sgpl_doacross_state *region)
{
    if (!region->profile_lanes)
        return;
    /* Called after all workers join, including the serial fallback. */
    for (int t = 0; t < region->profile_threads; ++t)
    {
        const sgpl_loop_profile_desc *desc = region->profile_lanes[t].desc;
        if (!desc)
            continue;
        int already_merged = 0;
        for (int earlier = 0; earlier < t; ++earlier)
            if (region->profile_lanes[earlier].desc == desc)
            {
                already_merged = 1;
                break;
            }
        if (already_merged)
            continue;
        uint64_t waits = 0, posts = 0, wait_ns = 0, post_ns = 0;
        for (int u = t; u < region->profile_threads; ++u)
        {
            const sgpl_doacross_sync_lane_sample *lane = &region->profile_lanes[u];
            if (lane->desc != desc)
                continue;
            waits += lane->waits; posts += lane->posts;
            wait_ns += lane->wait_ns; post_ns += lane->post_ns;
        }
        sgpl_record_doacross_sync_batch(desc, region->profile_threads, region->profile_trip_bucket,
                                        wait_ns, post_ns, waits, posts);
    }
}

void sgpl_doacross_profile_enter(const sgpl_loop_profile_desc *desc)
{
    g_tls_doacross_profile_desc = desc;
    g_tls_doacross_wait_total_ns = 0;
    g_tls_doacross_post_total_ns = 0;
    g_tls_doacross_wait_count = 0;
    g_tls_doacross_post_count = 0;
}

void sgpl_doacross_profile_exit(const sgpl_loop_profile_desc *desc)
{
    if (!g_tls_doacross_profile_desc || (desc && g_tls_doacross_profile_desc != desc))
    {
        g_tls_doacross_profile_desc = NULL;
        g_tls_doacross_wait_total_ns = 0;
        g_tls_doacross_post_total_ns = 0;
        g_tls_doacross_wait_count = 0;
        g_tls_doacross_post_count = 0;
        return;
    }

    sgpl_record_doacross_sync_sample(g_tls_doacross_profile_desc,
                                     g_tls_doacross_wait_total_ns,
                                     g_tls_doacross_post_total_ns,
                                     g_tls_doacross_wait_count,
                                     g_tls_doacross_post_count);
    g_tls_doacross_profile_desc = NULL;
    g_tls_doacross_wait_total_ns = 0;
    g_tls_doacross_post_total_ns = 0;
    g_tls_doacross_wait_count = 0;
    g_tls_doacross_post_count = 0;
}

int32_t sgpl_should_parallelize_doacross(const sgpl_loop_profile_desc *desc,
                                         int64_t start,
                                         int64_t end,
                                         int64_t step)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    int64_t trip_count = sgpl_compute_trip_count(start, end, step);
    double c_dep = 0.0;
    double c_ind = 0.0;
    double sigma_wait = 0.0;
    double sigma_post = 0.0;
    double l_thread_ns = 0.0;
    double l_path_ns = 0.0;
    double l_ns = 0.0;
    double critical_path_ns = 0.0;
    double sync_per_iter = 0.0;
    double serial_lhs = 0.0;
    double parallel_rhs = 0.0;
    int choose_parallel = 0;
    const char *reason = NULL;
    int using_seeded_sigma_wait = 0;
    int using_seeded_sigma_post = 0;
    int32_t effective_threads = 0;
    int32_t cached_choice = 0;
    uint32_t width_sync_samples = 0;

    if (state)
    {
        state->invocation_count++;
        state->last_observed_trip_count = trip_count;
        if (trip_count > 0)
        {
            if (state->observed_trip_count_ewma <= 0.0)
                state->observed_trip_count_ewma = (double)trip_count;
            else
                state->observed_trip_count_ewma = 0.25 * (double)trip_count + 0.75 * state->observed_trip_count_ewma;
        }
    }

    effective_threads = sgpl_loop_effective_decision_threads_for_loop_id(desc ? desc->loop_id : -1);

    if (effective_threads <= 1)
        reason = "single-thread";
    else if (trip_count <= 0)
        reason = "empty-trip-count";
    else if (!state || !desc || !desc->has_doacross_profile)
        reason = "missing-profile-desc";

    if (force_doacross_parallel_enabled())
    {
        // Explicit test override: bypass sampling/model readiness entirely.
        g_tls_pending_loop_id = desc ? desc->loop_id : -1;
        return 1;
    }

    if (!reason && sgpl_loop_try_cached_decision(state, desc, effective_threads, &cached_choice))
    {
        choose_parallel = cached_choice ? 1 : 0;
        if (runtime_debug_enabled())
        {
            long decision_index = atomic_fetch_add(&g_doacross_decision_calls, 1) + 1;
            fprintf(stderr,
                    "[parallel-runtime] decision-cache-hit loop=%s loop_id=%d invocation=%ld bucket=%d c_state=%s threads=%d choose=%s epoch=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    state->regime_trip_bucket,
                    sgpl_c_sampling_state_name(state->c_sampling_state),
                    effective_threads,
                    choose_parallel ? "parallel" : "serial",
                    state->c_sampling_epoch);
            fprintf(stderr,
                    "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=%s\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    choose_parallel ? "parallel-clone" : "serial-original");
        }
        g_tls_pending_loop_id = (choose_parallel && desc) ? desc->loop_id : -1;
        return choose_parallel ? 1 : 0;
    }

    if (!reason)
    {
        if (!state->regime_valid)
            reason = "warmup-uninitialized";
        else if (state->c_sampling_state != SGPL_C_SAMPLING_STABLE)
            reason = state->c_sampling_state == SGPL_C_SAMPLING_RECALIBRATING ? "recalibrating" : "warmup";
        else if (state->c_dep_ns_per_iter_ewma < 0.0 || state->c_ind_ns_per_iter_ewma < 0.0 ||
                 state->c_dep_ns_per_iter_ewma + state->c_ind_ns_per_iter_ewma <= 0.0)
            reason = "missing-c-dep-or-c-ind";
    }

    if (!reason)
    {
        c_dep = state->c_dep_ns_per_iter_ewma;
        c_ind = state->c_ind_ns_per_iter_ewma;
        sgpl_doacross_sync_costs costs = sgpl_get_doacross_sync_costs(desc, state, trip_count, effective_threads);
        sigma_wait = costs.wait_ns;
        sigma_post = costs.post_ns;
        using_seeded_sigma_wait = costs.seeded_wait;
        using_seeded_sigma_post = costs.seeded_post;
        width_sync_samples = costs.samples;
        {
            sgpl_launch_overhead_detail launch = sgpl_get_launch_overhead_detail(desc, effective_threads);
            l_thread_ns = launch.thread_ns;
            l_path_ns = launch.path_ns;
            l_ns = launch.total_ns;
        }
        critical_path_ns = (double)trip_count * c_dep;
        sync_per_iter = (double)desc->doacross_waits_per_iter * sigma_wait +
                        (double)desc->doacross_posts_per_iter * sigma_post;
        serial_lhs = (double)trip_count * (c_dep + c_ind);
        parallel_rhs = l_ns + critical_path_ns +
                       sgpl_loop_max_lane_iterations(trip_count, effective_threads, 1) *
                           (c_ind + sync_per_iter);
        choose_parallel = serial_lhs > parallel_rhs;
        if (sgpl_should_cache_decision(state))
            sgpl_loop_store_cached_decision(state, desc, effective_threads, choose_parallel);
    }

    if (runtime_debug_enabled())
    {
        long decision_index = atomic_fetch_add(&g_doacross_decision_calls, 1) + 1;
        if (state)
        {
            c_dep = state->c_dep_ns_per_iter_ewma;
            c_ind = state->c_ind_ns_per_iter_ewma;
            sgpl_doacross_sync_costs costs = sgpl_get_doacross_sync_costs(desc, state, trip_count, effective_threads);
            sigma_wait = costs.wait_ns;
            sigma_post = costs.post_ns;
            using_seeded_sigma_wait = costs.seeded_wait;
            using_seeded_sigma_post = costs.seeded_post;
            width_sync_samples = costs.samples;
        }
        if (!reason)
        {
            fprintf(stderr,
                    "[parallel-runtime] decision-cache-miss loop=%s loop_id=%d invocation=%ld bucket=%d c_state=%s threads=%d choose=%s epoch=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    state ? state->regime_trip_bucket : -1,
                    state ? sgpl_c_sampling_state_name(state->c_sampling_state) : "none",
                    effective_threads,
                    choose_parallel ? "parallel" : "serial",
                    state ? state->c_sampling_epoch : 0);
        }
        fprintf(stderr,
                "[parallel-runtime] cost-doacross loop=%s loop_id=%d invocation=%ld choose=%s N=%lld P=%d bucket=%d c_state=%s c_dep_ns=%.2f c_ind_ns=%.2f L_thread_ns=%.2f L_path_ns=%.2f L_total_ns=%.2f C_ns=N*c_dep=%.2f f_w=%d sigma_wait_ns=%.2f%s f_p=%d sigma_post_ns=%.2f%s sync_term_per_iter=%.2f serial_lhs=N*(c_dep+c_ind)=%.2f parallel_rhs=L+C+max_lane*(c_ind+sync_term)=%.2f serial_samples=%u sync_batches_at_width=%u%s%s\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel" : "serial",
                (long long)trip_count,
                effective_threads,
                state ? state->regime_trip_bucket : -1,
                state ? sgpl_c_sampling_state_name(state->c_sampling_state) : "none",
                c_dep,
                c_ind,
                l_thread_ns,
                l_path_ns,
                l_ns,
                critical_path_ns,
                desc ? desc->doacross_waits_per_iter : 0,
                sigma_wait,
                using_seeded_sigma_wait ? "(seed)" : "",
                desc ? desc->doacross_posts_per_iter : 0,
                sigma_post,
                using_seeded_sigma_post ? "(seed)" : "",
                sync_per_iter,
                serial_lhs,
                parallel_rhs,
                state ? state->doacross_serial_samples : 0,
                width_sync_samples,
                reason ? " reason=" : "",
                reason ? reason : "");
        fprintf(stderr,
                "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=%s\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel-clone" : "serial-original");
    }

    g_tls_pending_loop_id = (choose_parallel && desc) ? desc->loop_id : -1;
    return choose_parallel ? 1 : 0;
}

void parallel_for_runtime(int64_t start,
                          int64_t end,
                          int64_t step,
                          loop_body_fn body,
                          void *env,
                          int32_t needs_doacross,
                          int32_t doacross_num_sync_ids)
{
    int nthreads = 0;
    int granted_threads = 0;
    int32_t loop_id = g_tls_pending_loop_id;
    int64_t trip_count = 0;
    long call_index = 0;
    long total_trips = 0;

    if (step == 0 || !body)
        return;

    uint64_t trace_begin = g_tls_joint_trace ? sgpl_now_ns() : 0;

    ++g_tls_dispatch_depth;
    nthreads = sgpl_loop_effective_decision_threads();
    trip_count = sgpl_compute_trip_count(start, end, step);
    nthreads = sgpl_dispatch_thread_request(nthreads, trip_count);
    call_index = atomic_fetch_add(&g_parallel_plain_calls, 1) + 1;
    total_trips = atomic_fetch_add(&g_parallel_plain_trip_total, trip_count) + trip_count;

    if (runtime_debug_enabled() &&
        (call_index <= 8 || call_index % 100 == 0 || trip_count < nthreads))
    {
        fprintf(stderr,
                "[parallel-runtime] plain-call=%ld trips=%lld total_trips=%ld range=[%lld,%lld) step=%lld threads=%d\n",
                call_index,
                (long long)trip_count,
                total_trips,
                (long long)start,
                (long long)end,
                (long long)step,
                nthreads);
    }

    granted_threads = sgpl_loop_pool_try_acquire(loop_id, nthreads);
    if (g_tls_dispatch_depth > 1 && granted_threads > 1)
        atomic_fetch_add(&g_debug_nested_parallel_calls, 1);
    if (budget_debug_enabled())
    {
        fprintf(stderr,
                "[budget.loop] kind=plain task_slot=%d loop_id=%d requested=%d granted=%d trips=%lld reserved_now=%d\n",
                g_tls_current_tdg_task_slot,
                loop_id,
                nthreads,
                granted_threads,
                (long long)trip_count,
                atomic_load(&g_sgpl_reserved_threads));
    }
    {
        int force_parallel = force_doall_parallel_enabled() ||
                             (needs_doacross && force_doacross_parallel_enabled());
        int borrowed = 0;
        if (granted_threads <= 1 && force_parallel)
        {
            // Parent TDG task often reserves the whole machine; nested outlined
            // loops then see granted=1. Force path borrows hardware threads
            // without touching the budget counters (ablation / peak speedup).
            granted_threads = sgpl_runtime_thread_count();
            borrowed = 1;
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] plain-call=%ld force-borrow threads=%d (budget was exhausted)\n",
                        call_index,
                        granted_threads);
            }
        }
        if (granted_threads <= 1)
        {
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] plain-call=%ld budget-fallback=serial trips=%lld reserved=%d\n",
                        call_index,
                        (long long)trip_count,
                        atomic_load(&g_sgpl_reserved_threads));
            }
            g_tls_pending_loop_id = -1;
            sgpl_take_pending_range_body();
            sgpl_run_loop_serial(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
            if (g_tls_joint_trace)
                sgpl_joint_trace_loop(loop_id, trip_count, trace_begin, sgpl_now_ns());
            g_tls_dispatch_depth--;
            return;
        }

        int32_t prior_scope = 0;
        int32_t old_depth = 0;

        if (!region_scope_disabled())
            sgpl_tdg_enter_budget_scope(granted_threads, granted_threads,
                                        &prior_scope, &old_depth);
        g_tls_pending_loop_id = -1;
        sgpl_parallel_launch_plain_raw(start,
                                       end,
                                       step,
                                       body,
                                       env,
                                       needs_doacross,
                                       doacross_num_sync_ids,
                                       granted_threads);
        if (!region_scope_disabled())
            sgpl_tdg_exit_budget_scope(prior_scope, old_depth);
        if (!borrowed)
            sgpl_loop_pool_release(loop_id, granted_threads);
        if (g_tls_joint_trace)
            sgpl_joint_trace_loop(loop_id, trip_count, trace_begin, sgpl_now_ns());
        g_tls_dispatch_depth--;
    }
}

void parallel_for_runtime_ex(int64_t start,
                             int64_t end,
                             int64_t step,
                             loop_body_fn body,
                             void *env,
                             int64_t env_size,
                             const int64_t *priv_offsets,
                             const int32_t *priv_kinds,
                             const int64_t *priv_aux,
                             int32_t num_priv_targets,
                             int32_t needs_doacross,
                             int32_t doacross_num_sync_ids)
{
    int nthreads = 0;
    int granted_threads = 0;
    int32_t loop_id = g_tls_pending_loop_id;
    int64_t trip_count = 0;
    long call_index = 0;
    long total_trips = 0;

    if (step == 0 || !body)
        return;

    if (!env || env_size <= 0 || !priv_offsets || num_priv_targets <= 0)
    {
        parallel_for_runtime(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
        return;
    }

    uint64_t trace_begin = g_tls_joint_trace ? sgpl_now_ns() : 0;

    nthreads = sgpl_loop_effective_decision_threads();
    trip_count = sgpl_compute_trip_count(start, end, step);
    nthreads = sgpl_dispatch_thread_request(nthreads, trip_count);
    call_index = atomic_fetch_add(&g_parallel_ex_calls, 1) + 1;
    total_trips = atomic_fetch_add(&g_parallel_ex_trip_total, trip_count) + trip_count;

    ++g_tls_dispatch_depth;

    if (runtime_debug_enabled() &&
        (call_index <= 8 || call_index % 100 == 0 || trip_count < nthreads))
    {
        fprintf(stderr,
                "[parallel-runtime] ex-call=%ld trips=%lld total_trips=%ld range=[%lld,%lld) step=%lld threads=%d priv_targets=%d env_size=%lld\n",
                call_index,
                (long long)trip_count,
                total_trips,
                (long long)start,
                (long long)end,
                (long long)step,
                nthreads,
                num_priv_targets,
                (long long)env_size);
    }

    granted_threads = sgpl_loop_pool_try_acquire(loop_id, nthreads);
    if (g_tls_dispatch_depth > 1 && granted_threads > 1)
        atomic_fetch_add(&g_debug_nested_parallel_calls, 1);
    if (budget_debug_enabled())
    {
        fprintf(stderr,
                "[budget.loop] kind=priv task_slot=%d loop_id=%d requested=%d granted=%d trips=%lld reserved_now=%d env_size=%lld priv_targets=%d\n",
                g_tls_current_tdg_task_slot,
                loop_id,
                nthreads,
                granted_threads,
                (long long)trip_count,
                atomic_load(&g_sgpl_reserved_threads),
                (long long)env_size,
                num_priv_targets);
    }
    {
        int force_parallel = force_doall_parallel_enabled() ||
                             (needs_doacross && force_doacross_parallel_enabled());
        int borrowed = 0;
        if (granted_threads <= 1 && force_parallel)
        {
            granted_threads = sgpl_runtime_thread_count();
            borrowed = 1;
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] ex-call=%ld force-borrow threads=%d (budget was exhausted)\n",
                        call_index,
                        granted_threads);
            }
        }
        if (granted_threads <= 1)
        {
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] ex-call=%ld budget-fallback=serial trips=%lld reserved=%d\n",
                        call_index,
                        (long long)trip_count,
                        atomic_load(&g_sgpl_reserved_threads));
            }
            g_tls_pending_loop_id = -1;
            sgpl_take_pending_range_body();
            sgpl_run_loop_serial(start, end, step, body, env, needs_doacross, doacross_num_sync_ids);
            if (g_tls_joint_trace)
                sgpl_joint_trace_loop(loop_id, trip_count, trace_begin, sgpl_now_ns());
            g_tls_dispatch_depth--;
            return;
        }

        int32_t prior_scope = 0;
        int32_t old_depth = 0;

        if (!region_scope_disabled())
            sgpl_tdg_enter_budget_scope(granted_threads, granted_threads,
                                        &prior_scope, &old_depth);
        g_tls_pending_loop_id = -1;
        sgpl_parallel_launch_priv_raw(start,
                                      end,
                                      step,
                                      body,
                                      env,
                                      env_size,
                                      priv_offsets,
                                      priv_kinds,
                                      priv_aux,
                                      num_priv_targets,
                                      needs_doacross,
                                      doacross_num_sync_ids,
                                      granted_threads);
        if (!region_scope_disabled())
            sgpl_tdg_exit_budget_scope(prior_scope, old_depth);
        if (!borrowed)
            sgpl_loop_pool_release(loop_id, granted_threads);
        if (g_tls_joint_trace)
            sgpl_joint_trace_loop(loop_id, trip_count, trace_begin, sgpl_now_ns());
        g_tls_dispatch_depth--;
    }
}
