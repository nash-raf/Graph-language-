#ifndef SGPL_PARALLEL_RUNTIME_H
#define SGPL_PARALLEL_RUNTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SGPL_LOOP_DOALL = 0,
    SGPL_LOOP_DOACROSS = 1,
} sgpl_loop_mode_t;

typedef enum
{
    SGPL_RUNTIME_PLAIN = 0,
    SGPL_RUNTIME_PRIVATIZED = 1,
} sgpl_runtime_kind_t;

typedef enum
{
    SGPL_PRIV_ROARING = 0,
    SGPL_PRIV_INT_APPEND = 1,
} sgpl_priv_kind_t;

typedef struct
{
    int32_t loop_id;
    int32_t mode;
    int32_t runtime_kind;
    int32_t reserved;
    int64_t env_size;
    const int64_t *priv_offsets;
    int32_t num_priv_targets;
    int32_t reserved2;
    const char *debug_name;
    int32_t doacross_waits_per_iter;
    int32_t doacross_posts_per_iter;
    int32_t has_doacross_profile;
    int32_t reserved3;
    int32_t doacross_num_sync_ids;
    int32_t reserved4;
    int64_t reserved5;
} sgpl_loop_profile_desc;

typedef void *(*sgpl_tdg_task_fn_t)(void *);

typedef struct
{
    sgpl_tdg_task_fn_t fn;
    void *arg;
    int32_t profile_id;
    int32_t static_work_units;
    int32_t num_loop_sites;
    const int32_t *loop_site_ids;
} sgpl_tdg_task_desc;

int32_t sgpl_current_thread_budget(void);
void sgpl_push_thread_budget(int32_t max_threads);
void sgpl_pop_thread_budget(void);
int32_t sgpl_configured_worker_count(void);
int32_t sgpl_current_worker_index(void);

/* Debug introspection: current global budget reservations.  Used by the
 * fork/join resources-and-budget tests to verify the ledger is balanced. */
int32_t sgpl_debug_reserved_threads(void);

/* Debug introspection: how many nested dispatches (inside an already running
 * parallel region) were granted more than one thread.  A bounded budget model
 * keeps this at zero. */
long sgpl_debug_nested_parallel_calls(void);

/* TDG profile readiness for a site id (an outlined loop or an engine step):
 * 1 once the calibration batch is complete, i.e. once the cost model may plan
 * the site.  Engines use this to run a bounded number of honest serial passes
 * first; no per-site constants are involved (the batch size is the runtime's). */
int32_t sgpl_step_site_ready(int32_t loop_id);

uint64_t sgpl_now_ns(void);

int32_t sgpl_should_parallelize_doall(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step);

/* Registers the range entry point for the next parallel_for_runtime() call on
 * this thread.  Optional: without it the runtime drives the per-index body. */
void sgpl_set_pending_range_body(void *fn);

/* Loop identity for the next parallel_for_runtime() call on this thread.
 * Mirrors sgpl_set_pending_range_body: the compiler (or an engine shim) sets
 * the loop id immediately before the dispatch so the runtime can look up the
 * TDG level pool entry for that loop.  Passing -1 marks the caller as
 * unregistered (engine steps without their own level entry), which shares the
 * active level's remaining budget instead of being forced serial.
 * Disable the sharing with SGPL_NO_UNREGISTERED_POOL_SHARE=1. */
void sgpl_set_pending_loop_id(int32_t loop_id);

/* True while this thread is executing a callback owned by a TDG level.
 * Engine steps use the level's loop pool instead of creating a nested level. */
int32_t sgpl_tdg_level_active(void);
int32_t sgpl_tdg_level_task_count(void);

/* Desired width for the next dispatch on this thread (annotated by the
 * compiler or by an engine).  max<=0 means "no upper bound"; min<=0 means
 * "no lower bound".  The runtime still clamps by the budget, the trip count
 * and the machine width. */
void sgpl_set_desired_threads(int32_t min_threads, int32_t max_threads);

void sgpl_record_doall_serial_sample(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step,
    uint64_t elapsed_ns);

void sgpl_record_doacross_serial_sample(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step,
    uint64_t dep_elapsed_ns,
    uint64_t ind_elapsed_ns);

void sgpl_record_doacross_sync_sample(
    const sgpl_loop_profile_desc *desc,
    uint64_t wait_elapsed_ns_total,
    uint64_t post_elapsed_ns_total,
    uint64_t wait_count,
    uint64_t post_count);

int32_t sgpl_should_parallelize_doacross(
    const sgpl_loop_profile_desc *desc,
    int64_t start,
    int64_t end,
    int64_t step);

/* Parent workers = min(task_count, available budget). Work/span are retained
 * for API compatibility; task durations do not affect worker allocation. */
int32_t sgpl_choose_tdg_threads(
    int64_t work_units,
    int64_t span_units,
    int32_t task_count);

/* Default: jointly choose loop widths and task admission within this level.
 * Each callback uses one parent; only declared loop dispatches get extra
 * workers. Cold/unsupported/nested levels use the bounded FIFO executor. */
void sgpl_run_tdg_level(
    const sgpl_tdg_task_desc *tasks,
    int32_t task_count,
    int64_t work_units,
    int64_t span_units);

/* Compatibility entry for callers that explicitly selected dynamic scheduling.
 * Dynamic scheduling is included in every normal runtime build. */
void sgpl_run_tdg_level_joint(
    const sgpl_tdg_task_desc *tasks,
    int32_t task_count,
    int64_t work_units,
    int64_t span_units);

/* Conservative one-queue executor for compiler-excluded graph/GPU levels,
 * nested scopes, and unavailable profiles. No ordinary-first phase rule. */
void sgpl_run_tdg_level_fifo(const sgpl_tdg_task_desc *tasks, int32_t task_count,
                            int64_t work_units, int64_t span_units);

void sgpl_doacross_profile_enter(const sgpl_loop_profile_desc *desc);
void sgpl_doacross_profile_exit(const sgpl_loop_profile_desc *desc);

void parallel_for_runtime(
    int64_t start,
    int64_t end,
    int64_t step,
    void (*body)(int64_t, void *),
    void *env,
    int32_t needs_doacross,
    int32_t doacross_num_sync_ids);

void parallel_for_runtime_ex(
    int64_t start,
    int64_t end,
    int64_t step,
    void (*body)(int64_t, void *),
    void *env,
    int64_t env_size,
    const int64_t *priv_offsets,
    const int32_t *priv_kinds,
    const int64_t *priv_aux,
    int32_t num_priv_targets,
    int32_t needs_doacross,
    int32_t doacross_num_sync_ids);

#ifdef __cplusplus
}
#endif

#endif
