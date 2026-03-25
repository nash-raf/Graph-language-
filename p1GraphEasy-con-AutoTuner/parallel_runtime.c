#define _GNU_SOURCE
#include "parallel_runtime.h"

#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

static int runtime_debug_enabled(void);
static int runtime_iter_debug_enabled(void);

typedef struct
{
    atomic_long *completion;
    int32_t num_ids;
    int32_t sync_window;
    int64_t loop_start;
} sgpl_doacross_state;

typedef void (*loop_body_fn)(int64_t i, void *env);

typedef struct
{
    int64_t start;
    int64_t end;
    int64_t step;
    loop_body_fn body;
    void *env;
    int tid;
    int nthreads;
    int ncpus;
    sgpl_doacross_state *doacross_state;
    RoaringBitmap **override_originals;
    RoaringBitmap **override_replacements;
    int32_t num_override_targets;
} workers_args_t;

typedef struct
{
    uint32_t serial_samples;
    double c_ns_per_iter_ewma;
    uint64_t invocation_count;
    double c_dep_ns_per_iter_ewma;
    double c_ind_ns_per_iter_ewma;
    double sigma_wait_ns_ewma;
    double sigma_post_ns_ewma;
    uint32_t doacross_serial_samples;
    uint32_t doacross_sync_samples;
} sgpl_loop_runtime_state;

typedef struct
{
    int in_use;
    int runtime_kind;
    int threads;
    int64_t env_size;
    int32_t num_priv_targets;
    double l_ns;
} sgpl_launch_calibration_entry;

static sgpl_loop_runtime_state g_loop_states[SGPL_MAX_PROFILED_LOOPS];
static sgpl_launch_calibration_entry g_launch_costs[SGPL_MAX_LAUNCH_KEYS];

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

static const double SGPL_DOACROSS_SIGMA_WAIT_SEED_NS = 250.0;
static const double SGPL_DOACROSS_SIGMA_POST_SEED_NS = 80.0;

static inline atomic_long *sgpl_doacross_slot(sgpl_doacross_state *state, int32_t id, int slot)
{
    return &state->completion[(size_t)id * (size_t)state->sync_window + (size_t)slot];
}

static int runtime_debug_enabled(void)
{
    /* Debug logging disabled. Restore getenv("GRAPH_PARALLEL_DEBUG") here to re-enable. */
    return 0;
}

static int runtime_iter_debug_enabled(void)
{
    /* Iteration logging disabled. Restore getenv("GRAPH_PARALLEL_ITER_DEBUG") here to re-enable. */
    return 0;
}

static int force_doacross_parallel_enabled(void)
{
    /* Debug override disabled. Restore getenv("SGPL_FORCE_DOACROSS_PARALLEL") here to re-enable. */
    return 0;
}

uint64_t sgpl_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static int sgpl_runtime_thread_count(void)
{
    int nthreads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (nthreads <= 0)
        nthreads = 1;
    return nthreads;
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

static void sgpl_free_doacross_state(sgpl_doacross_state *state)
{
    if (!state)
        return;
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

static void *worker_main(void *_arg)
{
    workers_args_t *a = (workers_args_t *)_arg;
    int tid = a->tid;
    int nthreads = a->nthreads;
    int64_t start = a->start;
    int64_t end = a->end;
    int64_t step = a->step;
    int64_t i = 0;

    if (step == 0)
        return NULL;

    g_tls_doacross_state = a->doacross_state;
    if (a->override_originals && a->override_replacements && a->num_override_targets > 0)
    {
        roaring_bitmap_set_thread_local_overrides(a->override_originals,
                                                  a->override_replacements,
                                                  a->num_override_targets);
    }

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

    g_tls_doacross_state = NULL;
    if (a->override_originals && a->override_replacements && a->num_override_targets > 0)
        roaring_bitmap_clear_thread_local_overrides();
    return NULL;
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
    pthread_t *threads = NULL;
    workers_args_t *args = NULL;
    sgpl_doacross_state *doacross_state = NULL;
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
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
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
                                          int32_t num_priv_targets,
                                          int32_t needs_doacross,
                                          int32_t doacross_num_sync_ids,
                                          int nthreads)
{
    pthread_t *threads = NULL;
    workers_args_t *args = NULL;
    void **thread_envs = NULL;
    RoaringBitmap ***priv_bitmaps = NULL;
    RoaringBitmap **priv_originals = NULL;
    RoaringBitmap ***thread_override_replacements = NULL;
    sgpl_doacross_state *doacross_state = NULL;
    int target = 0;
    int tid = 0;

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

    threads = (pthread_t *)malloc((size_t)nthreads * sizeof(pthread_t));
    args = (workers_args_t *)malloc((size_t)nthreads * sizeof(workers_args_t));
    thread_envs = (void **)calloc((size_t)nthreads, sizeof(void *));
    priv_bitmaps = (RoaringBitmap ***)calloc((size_t)num_priv_targets, sizeof(RoaringBitmap **));
    priv_originals = (RoaringBitmap **)calloc((size_t)num_priv_targets, sizeof(RoaringBitmap *));
    thread_override_replacements = (RoaringBitmap ***)calloc((size_t)nthreads, sizeof(RoaringBitmap **));
    if (!threads || !args || !thread_envs || !priv_bitmaps || !priv_originals || !thread_override_replacements)
    {
        sgpl_free_doacross_state(doacross_state);
        free(threads);
        free(args);
        free(thread_envs);
        free(priv_bitmaps);
        free(priv_originals);
        free(thread_override_replacements);
        return;
    }

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] allocating privatized envs targets=%d threads=%d env_size=%lld\n",
                num_priv_targets,
                nthreads,
                (long long)env_size);
    }

    for (target = 0; target < num_priv_targets; ++target)
    {
        priv_bitmaps[target] = (RoaringBitmap **)calloc((size_t)nthreads, sizeof(RoaringBitmap *));
        if (!priv_bitmaps[target])
        {
            num_priv_targets = target;
            nthreads = 0;
            break;
        }
    }

    for (tid = 0; tid < nthreads; ++tid)
    {
        void *env_copy = malloc((size_t)env_size);
        if (!env_copy)
            break;
        thread_override_replacements[tid] = (RoaringBitmap **)calloc((size_t)num_priv_targets, sizeof(RoaringBitmap *));
        if (!thread_override_replacements[tid])
        {
            free(env_copy);
            break;
        }

        memcpy(env_copy, env, (size_t)env_size);
        thread_envs[tid] = env_copy;

        for (target = 0; target < num_priv_targets; ++target)
        {
            int64_t offset = priv_offsets[target];
            RoaringBitmap **slot = (RoaringBitmap **)((char *)env_copy + offset);
            RoaringBitmap *original = *(RoaringBitmap **)((char *)env + offset);
            RoaringBitmap *local = roaring_bitmap_create_like(original);
            priv_originals[target] = original;
            priv_bitmaps[target][tid] = local;
            thread_override_replacements[tid][target] = local;
            *slot = local;
        }

        args[tid].start = start;
        args[tid].end = end;
        args[tid].step = step;
        args[tid].body = body;
        args[tid].env = env_copy;
        args[tid].tid = tid;
        args[tid].nthreads = nthreads;
        args[tid].ncpus = nthreads;
        args[tid].doacross_state = doacross_state;
        args[tid].override_originals = priv_originals;
        args[tid].override_replacements = thread_override_replacements[tid];
        args[tid].num_override_targets = num_priv_targets;
        pthread_create(&threads[tid], NULL, worker_main, &args[tid]);
    }

    for (tid = 0; tid < nthreads; ++tid)
    {
        if (thread_envs[tid])
            pthread_join(threads[tid], NULL);
    }

    for (target = 0; target < num_priv_targets; ++target)
    {
        int64_t offset = priv_offsets[target];
        RoaringBitmap *original = *(RoaringBitmap **)((char *)env + offset);

        if (runtime_debug_enabled())
        {
            fprintf(stderr,
                    "[parallel-runtime] merging privatized target=%d threads=%d\n",
                    target,
                    nthreads);
        }

        for (tid = 0; tid < nthreads; ++tid)
        {
            RoaringBitmap *local = priv_bitmaps[target] ? priv_bitmaps[target][tid] : NULL;
            if (!local)
                continue;
            roaring_bitmap_or_inplace(original, local);
            roaring_bitmap_free(local);
        }
    }

    for (tid = 0; tid < nthreads; ++tid)
    {
        free(thread_envs[tid]);
        free(thread_override_replacements[tid]);
    }

    for (target = 0; target < num_priv_targets; ++target)
    {
        free(priv_bitmaps[target]);
    }

    free(threads);
    free(args);
    free(thread_envs);
    free(priv_bitmaps);
    free(priv_originals);
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

static double sgpl_calibrate_launch_overhead_ns(const sgpl_loop_profile_desc *desc, int threads)
{
    int sample = 0;
    double total_ns = 0.0;

    for (sample = 0; sample < 3; ++sample)
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
                                          desc->num_priv_targets,
                                          desc && desc->mode == SGPL_LOOP_DOACROSS ? 1 : 0,
                                          desc ? desc->doacross_num_sync_ids : 0,
                                          threads);

            for (target = 0; target < desc->num_priv_targets; ++target)
            {
                roaring_bitmap_free(originals[target]);
            }

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
            total_ns += sample_ns;
            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] launch-calibration-sample loop=%s kind=%d sample=%d threads=%d env=%lld targets=%d sample_L_ns=%.2f\n",
                        sgpl_loop_debug_name(desc),
                        desc ? desc->runtime_kind : SGPL_RUNTIME_PLAIN,
                        sample + 1,
                        threads,
                        (long long)(desc ? desc->env_size : 0),
                        desc ? desc->num_priv_targets : 0,
                        sample_ns);
            }
        }
    }

    return total_ns / 3.0;
}

static double sgpl_get_launch_overhead_ns(const sgpl_loop_profile_desc *desc, int threads)
{
    int i = 0;

    for (i = 0; i < SGPL_MAX_LAUNCH_KEYS; ++i)
    {
        if (!g_launch_costs[i].in_use)
            continue;

        if (g_launch_costs[i].runtime_kind == (desc ? desc->runtime_kind : SGPL_RUNTIME_PLAIN) &&
            g_launch_costs[i].threads == threads &&
            g_launch_costs[i].env_size == (desc ? desc->env_size : 0) &&
            g_launch_costs[i].num_priv_targets == (desc ? desc->num_priv_targets : 0))
        {
            return g_launch_costs[i].l_ns;
        }
    }

    for (i = 0; i < SGPL_MAX_LAUNCH_KEYS; ++i)
    {
        if (!g_launch_costs[i].in_use)
        {
            double l_ns = sgpl_calibrate_launch_overhead_ns(desc, threads);
            g_launch_costs[i].in_use = 1;
            g_launch_costs[i].runtime_kind = desc ? desc->runtime_kind : SGPL_RUNTIME_PLAIN;
            g_launch_costs[i].threads = threads;
            g_launch_costs[i].env_size = desc ? desc->env_size : 0;
            g_launch_costs[i].num_priv_targets = desc ? desc->num_priv_targets : 0;
            g_launch_costs[i].l_ns = l_ns;

            if (runtime_debug_enabled())
            {
                fprintf(stderr,
                        "[parallel-runtime] launch-calibration loop=%s kind=%d threads=%d env=%lld targets=%d L_ns(avg)=%.2f\n",
                        sgpl_loop_debug_name(desc),
                        g_launch_costs[i].runtime_kind,
                        threads,
                        (long long)g_launch_costs[i].env_size,
                        g_launch_costs[i].num_priv_targets,
                        g_launch_costs[i].l_ns);
            }

            return g_launch_costs[i].l_ns;
        }
    }

    return sgpl_calibrate_launch_overhead_ns(desc, threads);
}

int32_t sgpl_should_parallelize_doall(const sgpl_loop_profile_desc *desc,
                                      int64_t start,
                                      int64_t end,
                                      int64_t step)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    int threads = sgpl_runtime_thread_count();
    int64_t trip_count = sgpl_compute_trip_count(start, end, step);
    double c_ns = 0.0;
    double l_ns = 0.0;
    double threshold = 0.0;
    double speedup_factor = 0.0;
    double denominator = 0.0;
    int choose_parallel = 0;

    if (state)
        state->invocation_count++;

    if (threads <= 1 || trip_count <= 0 || !state || state->serial_samples < 3 || state->c_ns_per_iter_ewma <= 0.0)
    {
        if (runtime_debug_enabled())
        {
            long decision_index = atomic_fetch_add(&g_doall_decision_calls, 1) + 1;
            fprintf(stderr,
                    "[parallel-runtime] cost-doall loop=%s loop_id=%d invocation=%ld choose=serial reason=warmup N=%lld P=%d c_ns=%.2f L_ns=%.2f samples=%u\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index,
                    (long long)trip_count,
                    threads,
                    state ? state->c_ns_per_iter_ewma : 0.0,
                    0.0,
                    0.0,
                    state ? state->serial_samples : 0);
            fprintf(stderr,
                    "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=serial-original\n",
                    sgpl_loop_debug_name(desc),
                    desc ? desc->loop_id : -1,
                    decision_index);
        }
        return 0;
    }

    c_ns = state->c_ns_per_iter_ewma;
    l_ns = sgpl_get_launch_overhead_ns(desc, threads);
    speedup_factor = 1.0 - (1.0 / (double)threads);
    denominator = c_ns * speedup_factor;
    if (denominator > 0.0)
        threshold = l_ns / denominator;

    choose_parallel = denominator > 0.0 && (double)trip_count > threshold;

    if (runtime_debug_enabled())
    {
        long decision_index = atomic_fetch_add(&g_doall_decision_calls, 1) + 1;
        fprintf(stderr,
                "[parallel-runtime] cost-doall loop=%s loop_id=%d invocation=%ld choose=%s N=%lld P=%d c_ns=%.2f L_ns=%.2f speedup=(1-1/P)=%.6f denominator=c*speedup=%.6f threshold=L/denominator=%.2f samples=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel" : "serial",
                (long long)trip_count,
                threads,
                c_ns,
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

    if (!state || trip_count <= 0 || elapsed_ns == 0)
        return;

    c_sample = (double)elapsed_ns / (double)trip_count;
    if (state->serial_samples == 0)
        state->c_ns_per_iter_ewma = c_sample;
    else
        state->c_ns_per_iter_ewma = 0.25 * c_sample + 0.75 * state->c_ns_per_iter_ewma;

    state->serial_samples += 1;

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] serial-sample loop=%s loop_id=%d N=%lld elapsed_ns=%llu c_sample=elapsed/N=%.2f c_ewma=%.2f samples=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                (long long)trip_count,
                (unsigned long long)elapsed_ns,
                c_sample,
                state->c_ns_per_iter_ewma,
                state->serial_samples);
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

    if (!state || trip_count <= 0)
        return;

    c_dep_sample = (double)dep_elapsed_ns / (double)trip_count;
    c_ind_sample = (double)ind_elapsed_ns / (double)trip_count;

    if (state->doacross_serial_samples == 0)
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

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] doacross-serial-sample loop=%s loop_id=%d N=%lld dep_total_ns=%llu ind_total_ns=%llu c_dep_sample=%.2f c_ind_sample=%.2f c_dep_ewma=%.2f c_ind_ewma=%.2f samples=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                (long long)trip_count,
                (unsigned long long)dep_elapsed_ns,
                (unsigned long long)ind_elapsed_ns,
                c_dep_sample,
                c_ind_sample,
                state->c_dep_ns_per_iter_ewma,
                state->c_ind_ns_per_iter_ewma,
                state->doacross_serial_samples);
    }
}

void sgpl_record_doacross_sync_sample(const sgpl_loop_profile_desc *desc,
                                      uint64_t wait_elapsed_ns_total,
                                      uint64_t post_elapsed_ns_total,
                                      uint64_t wait_count,
                                      uint64_t post_count)
{
    sgpl_loop_runtime_state *state = sgpl_get_loop_state(desc);
    if (!state)
        return;

    if (wait_count > 0)
    {
        double sigma_wait = (double)wait_elapsed_ns_total / (double)wait_count;
        if (state->doacross_sync_samples == 0 || state->sigma_wait_ns_ewma <= 0.0)
            state->sigma_wait_ns_ewma = sigma_wait;
        else
            state->sigma_wait_ns_ewma = 0.25 * sigma_wait + 0.75 * state->sigma_wait_ns_ewma;
    }

    if (post_count > 0)
    {
        double sigma_post = (double)post_elapsed_ns_total / (double)post_count;
        if (state->doacross_sync_samples == 0 || state->sigma_post_ns_ewma <= 0.0)
            state->sigma_post_ns_ewma = sigma_post;
        else
            state->sigma_post_ns_ewma = 0.25 * sigma_post + 0.75 * state->sigma_post_ns_ewma;
    }

    state->doacross_sync_samples += 1;

    if (runtime_debug_enabled())
    {
        fprintf(stderr,
                "[parallel-runtime] doacross-sync-sample loop=%s loop_id=%d wait_total_ns=%llu post_total_ns=%llu wait_count=%llu post_count=%llu sigma_wait=%.2f sigma_post=%.2f sync_samples=%u\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                (unsigned long long)wait_elapsed_ns_total,
                (unsigned long long)post_elapsed_ns_total,
                (unsigned long long)wait_count,
                (unsigned long long)post_count,
                state->sigma_wait_ns_ewma,
                state->sigma_post_ns_ewma,
                state->doacross_sync_samples);
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
    int threads = sgpl_runtime_thread_count();
    int64_t trip_count = sgpl_compute_trip_count(start, end, step);
    double c_dep = 0.0;
    double c_ind = 0.0;
    double sigma_wait = 0.0;
    double sigma_post = 0.0;
    double l_ns = 0.0;
    double critical_path_ns = 0.0;
    double sync_per_iter = 0.0;
    double serial_lhs = 0.0;
    double parallel_rhs = 0.0;
    int choose_parallel = 0;
    const char *reason = NULL;
    int using_seeded_sigma_wait = 0;
    int using_seeded_sigma_post = 0;

    if (state)
        state->invocation_count++;

    if (threads <= 1)
        reason = "single-thread";
    else if (trip_count <= 0)
        reason = "empty-trip-count";
    else if (!state || !desc || !desc->has_doacross_profile)
        reason = "missing-profile-desc";
    else if (state->doacross_serial_samples < 3)
        reason = "warmup";
    else if (state->c_dep_ns_per_iter_ewma <= 0.0 || state->c_ind_ns_per_iter_ewma <= 0.0)
        reason = "missing-c-dep-or-c-ind";

    if (!reason)
    {
        c_dep = state->c_dep_ns_per_iter_ewma;
        c_ind = state->c_ind_ns_per_iter_ewma;
        if (desc->doacross_waits_per_iter > 0)
        {
            if (state->doacross_sync_samples == 0 || state->sigma_wait_ns_ewma <= 0.0)
            {
                sigma_wait = SGPL_DOACROSS_SIGMA_WAIT_SEED_NS;
                using_seeded_sigma_wait = 1;
            }
            else
                sigma_wait = state->sigma_wait_ns_ewma;
        }
        if (desc->doacross_posts_per_iter > 0)
        {
            if (state->doacross_sync_samples == 0 || state->sigma_post_ns_ewma <= 0.0)
            {
                sigma_post = SGPL_DOACROSS_SIGMA_POST_SEED_NS;
                using_seeded_sigma_post = 1;
            }
            else
                sigma_post = state->sigma_post_ns_ewma;
        }
        l_ns = sgpl_get_launch_overhead_ns(desc, threads);
        critical_path_ns = (double)trip_count * c_dep;
        sync_per_iter = (double)desc->doacross_waits_per_iter * sigma_wait +
                        (double)desc->doacross_posts_per_iter * sigma_post;
        serial_lhs = (double)trip_count * (c_dep + c_ind);
        parallel_rhs = l_ns + critical_path_ns +
                       ((double)trip_count * c_ind / (double)threads) +
                       ((double)trip_count * sync_per_iter);
        choose_parallel = serial_lhs > parallel_rhs;
    }

    if (force_doacross_parallel_enabled() && !reason)
    {
        choose_parallel = 1;
        reason = "forced-test";
    }

    if (runtime_debug_enabled())
    {
        long decision_index = atomic_fetch_add(&g_doacross_decision_calls, 1) + 1;
        if (state)
        {
            c_dep = state->c_dep_ns_per_iter_ewma;
            c_ind = state->c_ind_ns_per_iter_ewma;
            sigma_wait = (state->doacross_sync_samples == 0 || state->sigma_wait_ns_ewma <= 0.0)
                             ? SGPL_DOACROSS_SIGMA_WAIT_SEED_NS
                             : state->sigma_wait_ns_ewma;
            sigma_post = (state->doacross_sync_samples == 0 || state->sigma_post_ns_ewma <= 0.0)
                             ? SGPL_DOACROSS_SIGMA_POST_SEED_NS
                             : state->sigma_post_ns_ewma;
        }
        fprintf(stderr,
                "[parallel-runtime] cost-doacross loop=%s loop_id=%d invocation=%ld choose=%s N=%lld P=%d c_dep_ns=%.2f c_ind_ns=%.2f L_ns=%.2f C_ns=N*c_dep=%.2f f_w=%d sigma_wait_ns=%.2f%s f_p=%d sigma_post_ns=%.2f%s sync_term_per_iter=%.2f serial_lhs=N*(c_dep+c_ind)=%.2f parallel_rhs=L+C+N*c_ind/P+N*sync_term=%.2f serial_samples=%u sync_samples=%u%s%s\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel" : "serial",
                (long long)trip_count,
                threads,
                c_dep,
                c_ind,
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
                state ? state->doacross_sync_samples : 0,
                reason ? " reason=" : "",
                reason ? reason : "");
        fprintf(stderr,
                "[parallel-runtime] branch-taken loop=%s loop_id=%d invocation=%ld branch=%s\n",
                sgpl_loop_debug_name(desc),
                desc ? desc->loop_id : -1,
                decision_index,
                choose_parallel ? "parallel-clone" : "serial-original");
    }

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
    int64_t trip_count = 0;
    long call_index = 0;
    long total_trips = 0;

    if (step == 0 || !body)
        return;

    nthreads = sgpl_runtime_thread_count();
    trip_count = sgpl_compute_trip_count(start, end, step);
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

    sgpl_parallel_launch_plain_raw(start,
                                   end,
                                   step,
                                   body,
                                   env,
                                   needs_doacross,
                                   doacross_num_sync_ids,
                                   nthreads);
}

void parallel_for_runtime_ex(int64_t start,
                             int64_t end,
                             int64_t step,
                             loop_body_fn body,
                             void *env,
                             int64_t env_size,
                             const int64_t *priv_offsets,
                             int32_t num_priv_targets,
                             int32_t needs_doacross,
                             int32_t doacross_num_sync_ids)
{
    int nthreads = 0;
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

    nthreads = sgpl_runtime_thread_count();
    trip_count = sgpl_compute_trip_count(start, end, step);
    call_index = atomic_fetch_add(&g_parallel_ex_calls, 1) + 1;
    total_trips = atomic_fetch_add(&g_parallel_ex_trip_total, trip_count) + trip_count;

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

    sgpl_parallel_launch_priv_raw(start,
                                  end,
                                  step,
                                  body,
                                  env,
                                  env_size,
                                  priv_offsets,
                                  num_priv_targets,
                                  needs_doacross,
                                  doacross_num_sync_ids,
                                  nthreads);
}
