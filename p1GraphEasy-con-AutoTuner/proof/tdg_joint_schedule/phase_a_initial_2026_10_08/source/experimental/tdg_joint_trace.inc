/* Boundary profiling for the Phase A experiment. Included by parallel_runtime.c.
 * No timing operations are added to loop bodies. The normal build excludes it. */
#define SGPL_JOINT_MAX_TASKS 64
#define SGPL_JOINT_MAX_EVENTS 16
#define SGPL_JOINT_MAX_BUDGET 64
#define SGPL_JOINT_CACHE_SIZE 8
#define SGPL_JOINT_MAX_EVALUATIONS 2048

typedef struct {
    int32_t loop_id;
    int32_t max_threads;
    int64_t trips;
    double before_ns;
} sgpl_joint_trace_event;

typedef struct {
    sgpl_tdg_task_fn_t fn;
    uint32_t samples, revision;
    int32_t event_count, valid;
    double tail_ns, anchor_ns;
    sgpl_joint_trace_event events[SGPL_JOINT_MAX_EVENTS];
} sgpl_joint_task_profile;

typedef struct {
    uint64_t begin, last_end;
    int count, invalid;
    sgpl_joint_trace_event events[SGPL_JOINT_MAX_EVENTS];
} sgpl_joint_trace;

static sgpl_joint_task_profile g_joint_profiles[SGPL_MAX_TDG_PROFILE_SLOTS];
static pthread_mutex_t g_joint_profile_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int g_joint_collect_enabled;
static _Thread_local sgpl_joint_trace *g_tls_joint_trace;

static void sgpl_joint_trace_loop(int32_t loop_id, int64_t trips,
                                  uint64_t begin, uint64_t end)
{
    sgpl_joint_trace *trace = g_tls_joint_trace;
    if (!trace)
        return;
    if (loop_id < 0 || end < begin || begin < trace->last_end ||
        trace->count == SGPL_JOINT_MAX_EVENTS)
    {
        trace->invalid = 1;
        return;
    }
    trace->events[trace->count++] = (sgpl_joint_trace_event){
        .loop_id=loop_id, .max_threads=g_tls_desired_threads_max,
        .trips=trips, .before_ns=(double)(begin - trace->last_end)};
    trace->last_end = end;
}

static void sgpl_joint_trace_serial(int32_t loop_id, int64_t trips, uint64_t elapsed)
{
    if (g_tls_joint_trace)
    {
        uint64_t end = sgpl_now_ns();
        sgpl_joint_trace_loop(loop_id, trips, end >= elapsed ? end - elapsed : 0, end);
    }
}

static void sgpl_joint_invoke_task(const sgpl_tdg_task_desc *task)
{
    sgpl_joint_trace trace;
    sgpl_joint_trace *saved;
    int id = task->profile_id;
    if (!atomic_load_explicit(&g_joint_collect_enabled, memory_order_relaxed) ||
        id < 0 || id >= SGPL_MAX_TDG_PROFILE_SLOTS)
    {
        task->fn(task->arg);
        return;
    }
    memset(&trace, 0, sizeof(trace));
    trace.begin = trace.last_end = sgpl_now_ns();
    saved = g_tls_joint_trace;
    g_tls_joint_trace = &trace;
    task->fn(task->arg);
    uint64_t end = sgpl_now_ns();
    g_tls_joint_trace = saved;

    // Reject undeclared/overlapping occurrences; opaque graph work is not
    // silently promoted to an ordinary, known-resource task.
    for (int i = 0; i < trace.count; ++i)
    {
        int declared = 0;
        for (int j = 0; task->loop_site_ids && j < task->num_loop_sites; ++j)
            if (task->loop_site_ids[j] == trace.events[i].loop_id)
                declared = 1;
        if (!declared)
            trace.invalid = 1;
    }
    double residual = (double)(end - trace.last_end);
    for (int i = 0; i < trace.count; ++i)
        residual += trace.events[i].before_ns;
    pthread_mutex_lock(&g_joint_profile_lock);
    sgpl_joint_task_profile *profile = &g_joint_profiles[id];
    int same = profile->fn == task->fn && profile->event_count == trace.count &&
               profile->valid == !trace.invalid;
    for (int i = 0; same && i < trace.count; ++i)
        same = profile->events[i].loop_id == trace.events[i].loop_id &&
               profile->events[i].max_threads == trace.events[i].max_threads &&
               sgpl_trip_count_bucket(profile->events[i].trips) ==
               sgpl_trip_count_bucket(trace.events[i].trips);
    if (!same)
    {
        profile->samples = 0;
        ++profile->revision;
        profile->anchor_ns = residual;
    }
    double alpha = profile->samples ? SGPL_TDG_PROFILE_ALPHA : 1.0;
    profile->fn = task->fn;
    profile->event_count = trace.count;
    profile->valid = !trace.invalid;
    profile->tail_ns += alpha * ((double)(end - trace.last_end) - profile->tail_ns);
    for (int i = 0; i < trace.count; ++i)
    {
        profile->events[i].loop_id = trace.events[i].loop_id;
        profile->events[i].max_threads = trace.events[i].max_threads;
        profile->events[i].trips = trace.events[i].trips;
        profile->events[i].before_ns += alpha *
            (trace.events[i].before_ns - profile->events[i].before_ns);
    }
    double total = profile->tail_ns;
    for (int i = 0; i < trace.count; ++i)
        total += profile->events[i].before_ns;
    if (fabs(total - profile->anchor_ns) >
        SGPL_C_DRIFT_THRESHOLD * fmax(profile->anchor_ns, 1.0))
    {
        ++profile->revision;
        profile->anchor_ns = total;
    }
    if (profile->samples < UINT32_MAX)
        ++profile->samples;
    pthread_mutex_unlock(&g_joint_profile_lock);
}
