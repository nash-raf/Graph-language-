/* Default TDG scheduler: bounded integer allocation + event admission, with peak-team
 * reservation for the entire callback. Loop dispatchers are unchanged. */
static double sgpl_launch_sample_median(double samples[5]);
#define SGPL_JOINT_LOW_OVERHEAD 1

typedef struct {
    int id, mode, max_threads, runtime_model;
    int64_t trips;
    double before_ns;
    double costs[SGPL_JOINT_MAX_BUDGET + 1];
} sgpl_joint_event;

typedef struct {
    int count;
    double tail_ns;
    sgpl_joint_event events[SGPL_JOINT_MAX_EVENTS];
} sgpl_joint_job;

typedef struct {
    int count, budget, reference, workers, evaluations, backfill, max_parents;
    int order[SGPL_JOINT_MAX_TASKS];
    int widths[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    int event_ids[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    int event_counts[SGPL_JOINT_MAX_TASKS];
    int peak[SGPL_JOINT_MAX_TASKS];
    double prediction_ns, reference_ns, planning_ns, reference_planning_ns;
} sgpl_joint_plan;

typedef struct {
    int valid, count, budget, overload_only, ordering_only;
    sgpl_tdg_task_fn_t functions[SGPL_JOINT_MAX_TASKS];
    int profile_ids[SGPL_JOINT_MAX_TASKS], units[SGPL_JOINT_MAX_TASKS];
    uint32_t revisions[SGPL_JOINT_MAX_TASKS];
    int calibrated[SGPL_JOINT_MAX_TASKS];
    int counts[SGPL_JOINT_MAX_TASKS];
    uint64_t metadata[SGPL_JOINT_MAX_TASKS];
    sgpl_joint_trace_event traces[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    double rank[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    double sync[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    double dep[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    double ind[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    uint64_t regimes[SGPL_JOINT_MAX_TASKS][SGPL_JOINT_MAX_EVENTS];
    sgpl_joint_plan plan, reference_plan;
} sgpl_joint_cached_plan;

static sgpl_joint_cached_plan g_joint_cache[SGPL_JOINT_CACHE_SIZE];
static unsigned g_joint_cache_next;
static pthread_mutex_t g_joint_cache_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_joint_planning_lock = PTHREAD_MUTEX_INITIALIZER;
static double g_joint_dispatch_ns[SGPL_JOINT_MAX_BUDGET + 1][SGPL_JOINT_MAX_TASKS + 1];
static double g_joint_reference_dispatch_ns[SGPL_JOINT_MAX_BUDGET + 1][SGPL_JOINT_MAX_TASKS + 1];
static int g_joint_measurement_enabled;
static atomic_ulong g_joint_cache_hits, g_joint_plans, g_joint_reference_calls;
static atomic_ulong g_joint_revalidations, g_joint_calibration_runs, g_joint_search_evals, g_joint_learning_ns;
static _Thread_local double g_tls_joint_last_prediction_ns;
static _Thread_local uint64_t g_joint_search_deadline;
static _Thread_local const sgpl_joint_plan *g_joint_warm_start;

// Passive policy feedback belongs to this level/regime only. Verification
// executes useful work once through the FIFO fallback; no duplicate callbacks.
typedef struct {
    uint64_t key, plan_key;
    unsigned calls, references, candidates, slower_streak;
    double reference_ns, candidate_ns;
} sgpl_joint_observation;
static sgpl_joint_observation g_joint_observed[SGPL_JOINT_CACHE_SIZE];
static unsigned g_joint_observed_next;
static pthread_mutex_t g_joint_observed_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_ulong g_joint_observed_fallbacks;

static int sgpl_joint_search_expired(int evaluations)
{
    return evaluations >= SGPL_JOINT_MAX_EVALUATIONS ||
        (g_joint_search_deadline && !(evaluations & 15) && sgpl_now_ns() >= g_joint_search_deadline);
}

static void sgpl_joint_execute(const sgpl_tdg_task_desc *tasks, const sgpl_joint_plan *plan);

static void *sgpl_joint_noop_task(void *argument)
{
    (void)argument;
    return NULL;
}

static double sgpl_joint_measure_dispatch(int h, int count, int reference)
{
    double *cost = reference ? &g_joint_reference_dispatch_ns[h][count] : &g_joint_dispatch_ns[h][count];
    if (*cost != 0.0 || !g_joint_measurement_enabled)
        return *cost;
    sgpl_tdg_task_desc tasks[SGPL_JOINT_MAX_TASKS];
    sgpl_joint_plan plan = {.count=count,.budget=h,.workers=h,.backfill=1};
    for (int j = 0; j < count; ++j)
    {
        tasks[j] = (sgpl_tdg_task_desc){sgpl_joint_noop_task,NULL,-1,1,0,NULL};
        plan.order[j] = j;
    }
    double samples[2];
    for (int sample = 0; sample < 2; ++sample)
    {
        uint64_t begin = sgpl_now_ns();
        if (reference)
            sgpl_run_tdg_fifo_reserved(tasks,count,count,1,h);
        else
            sgpl_joint_execute(tasks,&plan);
        samples[sample] = (double)(sgpl_now_ns() - begin);
        atomic_fetch_add(&g_joint_calibration_runs, 1);
    }
    // Use the larger sample as a conservative launch estimate; uncertain
    // startup timing should not encourage unnecessary parent creation.
    *cost = fmax(samples[0], samples[1]);
    return *cost;
}

static double sgpl_joint_dispatch_cost(int h, int count, int reference)
{
    double (*costs)[SGPL_JOINT_MAX_TASKS + 1] =
        reference ? g_joint_reference_dispatch_ns : g_joint_dispatch_ns;
    if (costs[h][count] || !g_joint_measurement_enabled)
        return costs[h][count];
    // Three anchors per executor, shared across all level shapes: one/two
    // inline callbacks and two parents. Separate fixed, per-callback and
    // parent launch costs; no loop-team proxy or per-task calibration table.
    double one = sgpl_joint_measure_dispatch(1, 1, reference);
    double two = sgpl_joint_measure_dispatch(1, 2, reference);
    double inline_ns = one + (count - 1) * fmax(two - one, 0.0);
    if (h == 1)
        return inline_ns;
    double pair_ns = sgpl_joint_measure_dispatch(2, 2, reference);
    return inline_ns + fmax(pair_ns - two, 0.0) * h / 2.0;
}

static double sgpl_joint_event_cost(const sgpl_joint_event *event, int p)
{
    if (!event->runtime_model || event->costs[p] >= 0.0)
        return event->costs[p];
    sgpl_level_loop_candidate candidate = {.loop_id=event->id,
        .representative_trip_count=event->trips};
    sgpl_build_runtime_loop_desc(event->id, &g_loop_states[event->id], &candidate.desc);
    // Runtime jobs own this memoized curve. Abstract oracle jobs have their
    // own supplied curves and never enter this branch.
    double value = sgpl_loop_candidate_model_time_ns(&candidate, p);
    ((sgpl_joint_event *)event)->costs[p] = value;
    return value;
}

static void sgpl_joint_refresh_peaks(const sgpl_joint_job *jobs, sgpl_joint_plan *plan)
{
    for (int j = 0; j < plan->count; ++j)
    {
        plan->peak[j] = 0;
        for (int i = 0; i < jobs[j].count; ++i)
            if (plan->widths[j][i] >= 2 && plan->widths[j][i] > plan->peak[j])
                plan->peak[j] = plan->widths[j][i];
    }
}

/* FIFO acquisition of the real exclusive DOALL pool. Extra workers remain
 * reserved during pool waits and ordinary segments, as in the executor. */
static double sgpl_joint_simulate(const sgpl_joint_job *jobs, sgpl_joint_plan *plan)
{
    int status[SGPL_JOINT_MAX_TASKS] = {0};
    int next[SGPL_JOINT_MAX_TASKS] = {0};
    double ready[SGPL_JOINT_MAX_TASKS];
    int free_workers = plan->budget, active = 0, finished = 0, peak_active = 0;
    double now = 0.0, pool_free = 0.0;
    int parent_limit = plan->max_parents > 0 ? plan->max_parents : plan->budget;
    sgpl_joint_refresh_peaks(jobs, plan);
    while (finished < plan->count)
    {
        for (int r = 0; r < plan->count; ++r)
        {
            if (active >= parent_limit)
                break;
            int j = plan->order[r], demand = 1 + plan->peak[j];
            if (status[j])
                continue;
            if (demand > free_workers)
            {
                if (!plan->backfill)
                    break; // Deliberate deferral can protect the next team.
                continue;
            }
            status[j] = 1;
            free_workers -= demand;
            if (++active > peak_active)
                peak_active = active;
            ready[j] = now + (jobs[j].count ? jobs[j].events[0].before_ns : jobs[j].tail_ns);
        }
        int chosen = -1;
        for (int r = 0; r < plan->count; ++r)
        {
            int j = plan->order[r];
            if (status[j] == 1 && (chosen < 0 || ready[j] < ready[chosen]))
                chosen = j;
        }
        if (chosen < 0)
            return HUGE_VAL; // An infeasible width cannot deadlock the search.
        int j = chosen, i = next[j];
        now = ready[j];
        if (i == jobs[j].count)
        {
            status[j] = 2;
            --active;
            ++finished;
            free_workers += 1 + plan->peak[j];
        }
        else
        {
            const sgpl_joint_event *event = &jobs[j].events[i];
            int p = plan->widths[j][i];
            if (p < 1 || p > plan->budget - 1)
                return HUGE_VAL;
            double duration = sgpl_joint_event_cost(event, p);
            if (!isfinite(duration))
                return HUGE_VAL;
            double start = now;
            if (p >= 2 && event->mode == SGPL_LOOP_DOALL)
                start = fmax(start, pool_free);
            double end = start + duration;
            if (p >= 2 && event->mode == SGPL_LOOP_DOALL)
                pool_free = end;
            ++next[j];
            ready[j] = end + (next[j] == jobs[j].count ? jobs[j].tail_ns :
                                      jobs[j].events[next[j]].before_ns);
        }
    }
    plan->workers = peak_active;
    double launch = sgpl_joint_dispatch_cost(peak_active, plan->count, plan->reference);
    return now + launch + (plan->reference ? plan->reference_planning_ns : 0.0);
}

static void sgpl_joint_set_width(const sgpl_joint_job *jobs, sgpl_joint_plan *plan,
                                 int j, int i, int p)
{
    // A static site has one dispatcher entry; repeated occurrences reuse it.
    for (int k = 0; k < jobs[j].count; ++k)
        if (jobs[j].events[k].id == jobs[j].events[i].id)
            plan->widths[j][k] = p;
}

static int sgpl_joint_site_first(const sgpl_joint_job *jobs, int j, int i)
{
    for (int k = 0; k < i; ++k)
        if (jobs[j].events[k].id == jobs[j].events[i].id)
            return 0;
    return 1;
}

static void sgpl_joint_sort(const sgpl_joint_job *jobs, sgpl_joint_plan *plan, int kind)
{
    double lengths[SGPL_JOINT_MAX_TASKS];
    for (int j = 0; j < plan->count; ++j)
    {
        plan->order[j] = j;
        lengths[j] = jobs[j].tail_ns;
        for (int i = 0; i < jobs[j].count; ++i)
            lengths[j] += jobs[j].events[i].before_ns +
                sgpl_joint_event_cost(&jobs[j].events[i], plan->widths[j][i]);
    }
    for (int r = 1; r < plan->count; ++r)
    {
        int j = plan->order[r], k = r;
        while (k > 0)
        {
            int prev = plan->order[k-1];
            int move = lengths[j] > lengths[prev];
            if (kind == 1 && (!!jobs[j].count != !!jobs[prev].count))
                move = jobs[j].count == 0;
            if (kind == 2 && (!!jobs[j].count != !!jobs[prev].count))
                move = jobs[j].count != 0;
            if (!move)
                break;
            plan->order[k--] = prev;
        }
        plan->order[k] = j;
    }
}

static void sgpl_joint_seed_reference(const sgpl_tdg_task_desc *tasks,
                                      const sgpl_joint_job *jobs, sgpl_joint_plan *plan)
{
    /* Model the same single queue used by the actual fallback. In particular,
     * overload does not impose a barrier or move ordinary jobs ahead of loops. */
    for (int j = 0; j < plan->count; ++j) plan->order[j] = j;
    plan->max_parents = plan->count < plan->budget ? plan->count : plan->budget;
    sgpl_level_loop_plan loops;
    (void)sgpl_choose_tdg_threads_with_loop_budget(0, 0, tasks, plan->count, plan->budget, &loops);
    for (int e = 0; e < loops.entry_count; ++e)
    {
        int j = loops.entries[e].task_slot;
        for (int i = 0; i < jobs[j].count; ++i)
            if (jobs[j].events[i].id == loops.entries[e].loop_id)
            {
                int p = loops.entries[e].assigned_threads;
                if (jobs[j].events[i].max_threads > 0 && p > jobs[j].events[i].max_threads)
                    p = jobs[j].events[i].max_threads;
                if (p > jobs[j].events[i].trips && jobs[j].events[i].trips > 0)
                    p = (int)jobs[j].events[i].trips;
                plan->widths[j][i] = p;
            }
    }
}

static sgpl_joint_plan sgpl_joint_optimize_with_reference(const sgpl_tdg_task_desc *tasks,
                                          const sgpl_joint_job *jobs, int count,
                                          int budget, int ordering_only, sgpl_joint_plan *reference_out)
{
    sgpl_joint_plan best = {.count=count, .budget=budget, .backfill=1, .reference=1};
    for (int j = 0; j < count; ++j)
    {
        best.event_counts[j] = jobs[j].count;
        for (int i = 0; i < jobs[j].count; ++i)
        {
            best.widths[j][i] = 1;
            best.event_ids[j][i] = jobs[j].events[i].id;
        }
    }
    uint64_t seed_begin = sgpl_now_ns();
    sgpl_joint_seed_reference(tasks, jobs, &best);
    if (g_joint_measurement_enabled)
        best.reference_planning_ns = (double)(sgpl_now_ns() - seed_begin);
    double reference = sgpl_joint_simulate(jobs, &best);
    double score = reference;
    int evaluations = 1;
    // Same widths with different admissions isolates the ordering decision.
    sgpl_joint_plan reference_plan = best;
    if (reference_out)
        *reference_out = reference_plan;
    if (g_joint_warm_start)
    {
        sgpl_joint_plan previous = *g_joint_warm_start;
        previous.reference = 0;
        double value = sgpl_joint_simulate(jobs, &previous);
        ++evaluations;
        if (value < score)
            best = previous, score = value;
    }
    for (int kind = 0; kind < (g_joint_warm_start ? 1 : 3) && !sgpl_joint_search_expired(evaluations); ++kind)
        for (int seed = 0; seed < (ordering_only ? 1 : 3) && !sgpl_joint_search_expired(evaluations); ++seed)
          for (int backfill = 0; backfill < 2 && !sgpl_joint_search_expired(evaluations); ++backfill)
           for (int parents = 1; parents <= count && parents <= budget && !sgpl_joint_search_expired(evaluations); ++parents)
        {
            sgpl_joint_plan candidate = reference_plan;
            candidate.reference = 0;
            candidate.backfill = backfill;
            candidate.max_parents = parents;
            if (seed)
                for (int j = 0; j < count; ++j)
                    for (int i = 0; i < jobs[j].count; ++i)
                    {
                        int selected = 1;
                        if (seed == 2)
                            for (int p = 2; p < budget; ++p)
                            {
                                if (g_joint_search_deadline && sgpl_now_ns() >= g_joint_search_deadline)
                                    break;
                                if (sgpl_joint_event_cost(&jobs[j].events[i], p) <
                                    sgpl_joint_event_cost(&jobs[j].events[i], selected))
                                    selected = p;
                            }
                        sgpl_joint_set_width(jobs, &candidate, j, i, selected);
                    }
            sgpl_joint_sort(jobs, &candidate, kind);
            double value = sgpl_joint_simulate(jobs, &candidate);
            ++evaluations;
            if (value < score)
                best = candidate, score = value;
        }
    for (int pass = 0; pass < 3 && !sgpl_joint_search_expired(evaluations); ++pass)
    {
        double previous = score;
        for (int j = 0; !ordering_only && j < count; ++j)
            for (int i = 0; i < jobs[j].count; ++i)
            {
                if (!sgpl_joint_site_first(jobs, j, i))
                    continue;
                for (int p = 1; p < budget && !sgpl_joint_search_expired(evaluations); ++p)
                {
                    sgpl_joint_plan trial = best;
                    trial.reference = 0;
                    sgpl_joint_set_width(jobs, &trial, j, i, p);
                    double value = sgpl_joint_simulate(jobs, &trial);
                    ++evaluations;
                    if (value < score)
                        best = trial, score = value;
                }
            }
        for (int a = 0; a < count; ++a)
            for (int b = a + 1; b < count && !sgpl_joint_search_expired(evaluations); ++b)
            {
                sgpl_joint_plan trial = best;
                trial.reference = 0;
                int j = trial.order[a];
                trial.order[a] = trial.order[b];
                trial.order[b] = j;
                double value = sgpl_joint_simulate(jobs, &trial);
                ++evaluations;
                if (value < score)
                    best = trial, score = value;
            }
        if (score >= previous)
            break;
    }
    best.prediction_ns = score;
    best.reference_ns = reference;
    best.evaluations = evaluations;
    return best;
}

static sgpl_joint_plan sgpl_joint_optimize(const sgpl_tdg_task_desc *tasks,
                                          const sgpl_joint_job *jobs, int count,
                                          int budget, int ordering_only)
{
    return sgpl_joint_optimize_with_reference(tasks, jobs, count, budget, ordering_only, NULL);
}

static int sgpl_joint_profiles_ready(const sgpl_tdg_task_desc *tasks, int count,
                                    sgpl_joint_task_profile *profiles)
{
    int ready = 1;
    pthread_mutex_lock(&g_joint_profile_lock);
    for (int j = 0; j < count; ++j)
    {
        int id = tasks[j].profile_id;
        if (!tasks[j].fn || id < 0 || id >= SGPL_MAX_TDG_PROFILE_SLOTS)
        {
            ready = 0;
            break;
        }
        profiles[j] = g_joint_profiles[id];
        if (tasks[j].num_loop_sites <= 0 &&
            (profiles[j].fn != tasks[j].fn || !profiles[j].valid || profiles[j].samples < 2))
        {
            // Instruction-weight units are a cold prior, converted using the
            // existing runtime calibration. Never add units to nanoseconds.
            profiles[j].fn = tasks[j].fn;
            profiles[j].valid = 1;
            profiles[j].samples = 0;
            profiles[j].event_count = 0;
            profiles[j].tail_ns = fmax(tasks[j].static_work_units, 1) * sgpl_tdg_unit_cost_ns;
            continue;
        }
        if (profiles[j].fn != tasks[j].fn || !profiles[j].valid || profiles[j].samples < 2)
        {
            ready = 0;
            break;
        }
        if (tasks[j].num_loop_sites > 0 && profiles[j].event_count == 0)
        {
            ready = 0; // No trace is not evidence of a serial, known-cost task.
            break;
        }
    }
    pthread_mutex_unlock(&g_joint_profile_lock);
    for (int j = 0; ready && j < count; ++j)
        for (int i = 0; i < profiles[j].event_count; ++i)
        {
            int id = profiles[j].events[i].loop_id;
            if (id < 0 || id >= SGPL_MAX_PROFILED_LOOPS || !sgpl_step_site_ready(id))
                ready = 0;
            int declared = 0;
            for (int k = 0; tasks[j].loop_site_ids && k < tasks[j].num_loop_sites; ++k)
                declared |= tasks[j].loop_site_ids[k] == id;
            if (!declared)
                ready = 0;
        }
    return ready;
}

static double sgpl_joint_rank(int id)
{
    const sgpl_loop_runtime_state *state = &g_loop_states[id];
    return state->regime_mode == SGPL_LOOP_DOACROSS ?
        state->c_dep_ns_per_iter_ewma + state->c_ind_ns_per_iter_ewma : state->c_ns_per_iter_ewma;
}

static uint64_t sgpl_joint_metadata(const sgpl_tdg_task_desc *task)
{
    uint64_t hash = (uint32_t)task->num_loop_sites;
    for (int i = 0; task->loop_site_ids && i < task->num_loop_sites; ++i)
        hash = (hash ^ (uint32_t)task->loop_site_ids[i]) * UINT64_C(1099511628211);
    return hash;
}

static uint64_t sgpl_joint_regime(int id)
{
    const sgpl_loop_runtime_state *s = &g_loop_states[id];
    const int64_t fields[] = {s->regime_mode, s->regime_runtime_kind,
        s->regime_trip_bucket, s->regime_env_size, s->regime_num_priv_targets,
        s->regime_waits_per_iter, s->regime_posts_per_iter, s->regime_doacross_num_sync_ids};
    uint64_t hash = 0;
    for (unsigned k = 0; k < sizeof(fields) / sizeof(fields[0]); ++k)
        hash = (hash ^ (uint64_t)fields[k]) * UINT64_C(1099511628211);
    return hash;
}

static uint64_t sgpl_joint_level_key(const sgpl_tdg_task_desc *tasks, int count, int budget)
{
    uint64_t hash = ((uint64_t)budget << 32) ^ (uint32_t)count;
    pthread_mutex_lock(&g_joint_profile_lock);
    for (int j = 0; j < count; ++j)
    {
        hash = (hash ^ (uintptr_t)tasks[j].fn) * UINT64_C(1099511628211);
        hash = (hash ^ (uint32_t)tasks[j].profile_id ^ (uint32_t)tasks[j].static_work_units ^
            sgpl_joint_metadata(&tasks[j])) * UINT64_C(1099511628211);
        int profile_id = tasks[j].profile_id;
        if (profile_id >= 0 && profile_id < SGPL_MAX_TDG_PROFILE_SLOTS)
        {
            const sgpl_joint_task_profile *profile = &g_joint_profiles[profile_id];
            hash = (hash ^ (uint32_t)profile->event_count) * UINT64_C(1099511628211);
            for (int i = 0; i < profile->event_count; ++i)
                hash = (hash ^ (uint32_t)profile->events[i].loop_id ^
                    (uint32_t)profile->events[i].max_threads) * UINT64_C(1099511628211);
        }
        for (int k = 0; tasks[j].loop_site_ids && k < tasks[j].num_loop_sites; ++k)
        {
            int id = tasks[j].loop_site_ids[k];
            if (id >= 0 && id < SGPL_MAX_PROFILED_LOOPS)
                hash = (hash ^ sgpl_joint_regime(id) ^ (uint64_t)g_loop_states[id].last_observed_trip_count) *
                    UINT64_C(1099511628211);
        }
    }
    pthread_mutex_unlock(&g_joint_profile_lock);
    return hash ? hash : 1;
}

static uint64_t sgpl_joint_plan_key(const sgpl_joint_plan *plan)
{
    uint64_t hash = plan->workers ^ ((uint64_t)plan->backfill << 32);
    for (int r = 0; r < plan->count; ++r)
    {
        int j = plan->order[r];
        hash = (hash ^ (uint32_t)j) * UINT64_C(1099511628211);
        for (int i = 0; i < plan->event_counts[j]; ++i)
            hash = (hash ^ (uint32_t)plan->widths[j][i]) * UINT64_C(1099511628211);
    }
    return hash;
}

static sgpl_joint_observation *sgpl_joint_find_observation(uint64_t key)
{
    for (int k = 0; k < SGPL_JOINT_CACHE_SIZE; ++k)
        if (g_joint_observed[k].key == key)
            return &g_joint_observed[k];
    sgpl_joint_observation *entry = &g_joint_observed[g_joint_observed_next++ % SGPL_JOINT_CACHE_SIZE];
    *entry = (sgpl_joint_observation){.key=key};
    return entry;
}

static int sgpl_joint_observed_guard(uint64_t key, sgpl_joint_plan *plan, double lookup_ns)
{
    int fallback = 0;
    pthread_mutex_lock(&g_joint_observed_lock);
    sgpl_joint_observation *entry = sgpl_joint_find_observation(key);
    if (entry->references >= 2)
    {
        plan->reference_ns = fmin(plan->reference_ns, entry->reference_ns);
        fallback = plan->prediction_ns + lookup_ns >= plan->reference_ns ||
            (entry->plan_key == sgpl_joint_plan_key(plan) && entry->slower_streak >= 2) ||
            (entry->calls && !(entry->calls & 15));
    }
    pthread_mutex_unlock(&g_joint_observed_lock);
    return fallback;
}

static void sgpl_joint_observe(uint64_t key, const sgpl_joint_plan *plan, int reference, double elapsed)
{
    pthread_mutex_lock(&g_joint_observed_lock);
    sgpl_joint_observation *entry = sgpl_joint_find_observation(key);
    ++entry->calls;
    if (reference)
    {
        // The second useful baseline replaces the cold first dispatch. Later
        // verification/fallback samples track changing execution conditions.
        entry->reference_ns = entry->references < 2 ? elapsed :
            entry->reference_ns + SGPL_TDG_PROFILE_ALPHA * (elapsed - entry->reference_ns);
        ++entry->references;
    }
    else
    {
        uint64_t identity = sgpl_joint_plan_key(plan);
        if (entry->plan_key != identity)
        {
            entry->plan_key = identity;
            entry->candidates = entry->slower_streak = 0;
        }
        entry->candidate_ns = entry->candidates ? entry->candidate_ns +
            SGPL_TDG_PROFILE_ALPHA * (elapsed - entry->candidate_ns) : elapsed;
        ++entry->candidates;
        entry->slower_streak = elapsed > entry->reference_ns ? entry->slower_streak + 1 : 0;
    }
    pthread_mutex_unlock(&g_joint_observed_lock);
}

static int sgpl_joint_cache_matches(const sgpl_joint_cached_plan *cache,
                                    const sgpl_tdg_task_desc *tasks,
                                    const sgpl_joint_task_profile *profiles,
                                    int count, int budget, int overload, int ordering, int costs)
{
    if (!cache->valid || cache->count != count || cache->budget != budget ||
        cache->overload_only != overload || cache->ordering_only != ordering)
        return 0;
    for (int j = 0; j < count; ++j)
    {
        if (cache->functions[j] != tasks[j].fn || cache->profile_ids[j] != tasks[j].profile_id ||
            cache->units[j] != tasks[j].static_work_units ||
            cache->metadata[j] != sgpl_joint_metadata(&tasks[j]) ||
            (costs && (cache->calibrated[j] != (profiles[j].samples >= 2) ||
             cache->revisions[j] != profiles[j].revision)) || cache->counts[j] != profiles[j].event_count)
            return 0;
        for (int i = 0; i < profiles[j].event_count; ++i)
        {
            const sgpl_joint_trace_event *event = &profiles[j].events[i];
            const sgpl_joint_trace_event *old = &cache->traces[j][i];
            int id = event->loop_id;
            double rank = sgpl_joint_rank(id);
            double sync = g_loop_states[id].sigma_wait_ns_ewma + g_loop_states[id].sigma_post_ns_ewma;
            if (id != old->loop_id ||
                event->max_threads != old->max_threads ||
                fabs((double)event->trips - old->trips) > SGPL_C_DRIFT_THRESHOLD * fmax(old->trips, 1) ||
                (costs && (fabs(rank - cache->rank[j][i]) > SGPL_C_DRIFT_THRESHOLD * fmax(cache->rank[j][i], 1e-9) ||
                fabs(sync - cache->sync[j][i]) > SGPL_C_DRIFT_THRESHOLD * fmax(cache->sync[j][i], 1.0) ||
                fabs(g_loop_states[id].c_dep_ns_per_iter_ewma - cache->dep[j][i]) >
                    SGPL_C_DRIFT_THRESHOLD * fmax(cache->dep[j][i], 1e-9) ||
                fabs(g_loop_states[id].c_ind_ns_per_iter_ewma - cache->ind[j][i]) >
                    SGPL_C_DRIFT_THRESHOLD * fmax(cache->ind[j][i], 1e-9))) ||
                sgpl_joint_regime(id) != cache->regimes[j][i])
                return 0;
        }
    }
    return 1;
}

static int sgpl_joint_get_plan(const sgpl_tdg_task_desc *tasks, int count, int budget,
                               sgpl_joint_plan *out)
{
    sgpl_joint_task_profile profiles[SGPL_JOINT_MAX_TASKS];
    uint64_t lookup_begin = sgpl_now_ns();
    int overload = sgpl_tdg_env_enabled("SGPL_JOINT_OVERLOAD_ONLY", 0);
    int ordering = sgpl_tdg_env_enabled("SGPL_JOINT_ORDERING_ONLY", 0);
    if (!sgpl_joint_profiles_ready(tasks, count, profiles))
        return 0;
    sgpl_joint_cached_plan previous;
    int previous_slot = -1;
    pthread_mutex_lock(&g_joint_cache_lock);
    for (int k = 0; k < SGPL_JOINT_CACHE_SIZE; ++k)
        if (sgpl_joint_cache_matches(&g_joint_cache[k], tasks, profiles, count, budget, overload, ordering, 0))
        {
            if (!sgpl_joint_cache_matches(&g_joint_cache[k], tasks, profiles, count, budget, overload, ordering, 1))
            {
                previous = g_joint_cache[k];
                previous_slot = k;
                break;
            }
            *out = g_joint_cache[k].plan;
            if (out->reference_ns - out->prediction_ns <= (double)(sgpl_now_ns() - lookup_begin))
                out->reference = 1;
            atomic_fetch_add(&g_joint_cache_hits, 1);
            pthread_mutex_unlock(&g_joint_cache_lock);
            return 1;
        }
    pthread_mutex_unlock(&g_joint_cache_lock);
    pthread_mutex_lock(&g_joint_planning_lock);
    sgpl_joint_job *jobs = calloc((size_t)count, sizeof(*jobs));
    if (!jobs)
    {
        pthread_mutex_unlock(&g_joint_planning_lock);
        return 0;
    }
    uint64_t begin = sgpl_now_ns();
    double serial_work = 0.0;
    for (int j = 0; j < count; ++j)
    {
        jobs[j].count = profiles[j].event_count;
        jobs[j].tail_ns = profiles[j].tail_ns;
        serial_work += jobs[j].tail_ns;
        for (int i = 0; i < jobs[j].count; ++i)
        {
            sgpl_joint_event *event = &jobs[j].events[i];
            event->id = profiles[j].events[i].loop_id;
            event->trips = profiles[j].events[i].trips;
            event->max_threads = profiles[j].events[i].max_threads;
            event->before_ns = profiles[j].events[i].before_ns;
            event->runtime_model = 1;
            sgpl_level_loop_candidate candidate = {.loop_id=event->id,
                .representative_trip_count=event->trips};
            sgpl_build_runtime_loop_desc(event->id, &g_loop_states[event->id], &candidate.desc);
            event->mode = candidate.desc.mode;
            for (int p = 1; p < budget; ++p)
                event->costs[p] = p > event->trips ||
                    (event->max_threads > 0 && p > event->max_threads) ? HUGE_VAL : -1.0;
            serial_work += event->before_ns + sgpl_joint_event_cost(event, 1);
        }
    }
    // Timing drift need not change the allocation. Rescore the old candidate
    // and its reference with current curves before paying for another search.
    g_joint_measurement_enabled = 1;
    int searched = 1;
    sgpl_joint_plan reference_model;
    if (previous_slot >= 0)
    {
        sgpl_joint_plan candidate = previous.plan;
        sgpl_joint_plan reference = previous.reference_plan;
        double value = sgpl_joint_simulate(jobs, &candidate);
        double baseline = sgpl_joint_simulate(jobs, &reference);
        double drift = fabs(value - previous.plan.prediction_ns);
        if (isfinite(value) && isfinite(baseline) &&
            drift <= SGPL_C_DRIFT_THRESHOLD * fmax(previous.plan.prediction_ns, 1.0))
        {
            *out = candidate;
            out->prediction_ns = value;
            out->reference_ns = baseline;
            out->reference = value >= baseline;
            reference_model = reference;
            searched = 0;
            atomic_fetch_add(&g_joint_revalidations, 1);
        }
    }
    if (searched)
    {
        // Search may use at most 5% of estimated serial work. A legacy loop
        // calibration is atomic; check the deadline again immediately after
        // that evaluation. No timing-derived budget changes resource safety.
        g_joint_search_deadline = begin + (uint64_t)(serial_work * .05);
        g_joint_warm_start = previous_slot >= 0 ? &previous.plan : NULL;
        *out = sgpl_joint_optimize_with_reference(tasks, jobs, count, budget, ordering, &reference_model);
        g_joint_warm_start = NULL;
        g_joint_search_deadline = 0;
        atomic_fetch_add(&g_joint_search_evals, out->evaluations);
        atomic_fetch_add(&g_joint_plans, 1);
    }
    g_joint_measurement_enabled = 0;
    free(jobs);
    out->planning_ns = (double)(sgpl_now_ns() - begin);
    atomic_fetch_add(&g_joint_learning_ns, (unsigned long)out->planning_ns);
    pthread_mutex_unlock(&g_joint_planning_lock);
    int cached_reference = out->reference;
    // A one-shot saving must cover the time spent finding it. Cached reuse
    // is reported separately; do not hide planning cost in the benchmark.
    if (out->reference_ns - out->prediction_ns <= out->planning_ns)
        out->reference = 1;
    pthread_mutex_lock(&g_joint_cache_lock);
    sgpl_joint_cached_plan *cache = &g_joint_cache[previous_slot >= 0 ? previous_slot :
        g_joint_cache_next++ % SGPL_JOINT_CACHE_SIZE];
    memset(cache, 0, sizeof(*cache));
    cache->count = count;
    cache->budget = budget;
    cache->overload_only = overload;
    cache->ordering_only = ordering;
    cache->plan = *out;
    cache->plan.reference = cached_reference;
    cache->reference_plan = reference_model;
    for (int j = 0; j < count; ++j)
    {
        cache->functions[j] = tasks[j].fn;
        cache->profile_ids[j] = tasks[j].profile_id;
        cache->units[j] = tasks[j].static_work_units;
        cache->metadata[j] = sgpl_joint_metadata(&tasks[j]);
        cache->revisions[j] = profiles[j].revision;
        cache->calibrated[j] = profiles[j].samples >= 2;
        cache->counts[j] = profiles[j].event_count;
        for (int i = 0; i < profiles[j].event_count; ++i)
        {
            int id = profiles[j].events[i].loop_id;
            cache->traces[j][i] = profiles[j].events[i];
            cache->rank[j][i] = sgpl_joint_rank(id);
            cache->sync[j][i] = g_loop_states[id].sigma_wait_ns_ewma + g_loop_states[id].sigma_post_ns_ewma;
            cache->dep[j][i] = g_loop_states[id].c_dep_ns_per_iter_ewma;
            cache->ind[j][i] = g_loop_states[id].c_ind_ns_per_iter_ewma;
            cache->regimes[j][i] = sgpl_joint_regime(id);
        }
    }
    cache->valid = 1;
    pthread_mutex_unlock(&g_joint_cache_lock);
    return 1;
}

typedef struct {
    const sgpl_tdg_task_desc *tasks;
    const sgpl_joint_plan *plan;
    int started[SGPL_JOINT_MAX_TASKS];
    int completed, free_workers, gate;
    pthread_mutex_t lock;
    pthread_cond_t change;
} sgpl_joint_execution;

static void *sgpl_joint_worker(void *argument)
{
    sgpl_joint_execution *state = argument;
    const sgpl_joint_plan *plan = state->plan;
    for (;;)
    {
        pthread_mutex_lock(&state->lock);
        int j = -1;
        while (j < 0 && state->completed < plan->count)
        {
            if (state->gate)
                for (int r = 0; r < plan->count; ++r)
                {
                    int task = plan->order[r];
                    if (state->started[task])
                        continue;
                    if (1 + plan->peak[task] > state->free_workers)
                    {
                        if (!plan->backfill)
                            break;
                        continue;
                    }
                    {
                        j = task;
                        state->started[j] = 1;
                        state->free_workers -= 1 + plan->peak[j];
                        break;
                    }
                }
            if (j < 0)
                pthread_cond_wait(&state->change, &state->lock);
        }
        pthread_mutex_unlock(&state->lock);
        if (j < 0)
            return NULL;

        sgpl_level_loop_plan loops = {.loop_budget=plan->peak[j], .dominant_loop_id=-1};
        const sgpl_tdg_task_desc *task = &state->tasks[j];
        // Include every declared site, even serial/branch-inactive ones, so an
        // unexpected occurrence cannot steal another task's reserved workers.
        for (int i = 0; task->loop_site_ids && i < task->num_loop_sites &&
                        loops.entry_count < SGPL_MAX_TDG_LEVEL_LOOPS; ++i)
        {
            sgpl_level_loop_plan_entry *entry = &loops.entries[loops.entry_count++];
            entry->task_slot = j;
            entry->loop_id = task->loop_site_ids[i];
            entry->assigned_threads = 1;
            for (int e = 0; e < plan->event_counts[j]; ++e)
                if (plan->event_ids[j][e] == entry->loop_id)
                    entry->assigned_threads = plan->widths[j][e];
        }
        sgpl_level_loop_pool pool, *saved_pool;
        int32_t pending, slot = g_tls_current_tdg_task_slot, scope, depth;
        sgpl_loop_pool_init(&pool, &loops);
        pool.task_count = plan->count;
        sgpl_bind_level_loop_pool(&pool, &saved_pool, &pending);
        sgpl_tdg_enter_budget_scope(1, plan->peak[j], &scope, &depth);
        g_tls_current_tdg_task_slot = j;
        uint64_t begin = sgpl_now_ns();
        SGPL_TDG_INVOKE(task);
        sgpl_record_tdg_profile_sample(task, j, (double)(sgpl_now_ns() - begin), "joint-worker");
        g_tls_current_tdg_task_slot = slot;
        sgpl_tdg_exit_budget_scope(scope, depth);
        sgpl_unbind_level_loop_pool(saved_pool, pending);
        sgpl_loop_pool_destroy(&pool);

        pthread_mutex_lock(&state->lock);
        state->free_workers += 1 + plan->peak[j];
        ++state->completed;
        pthread_cond_broadcast(&state->change);
        pthread_mutex_unlock(&state->lock);
    }
}

static void sgpl_joint_execute(const sgpl_tdg_task_desc *tasks, const sgpl_joint_plan *plan)
{
    sgpl_joint_execution state = {.tasks=tasks, .plan=plan, .free_workers=plan->budget};
    pthread_t threads[SGPL_JOINT_MAX_BUDGET];
    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.change, NULL);
    int started = 0, workers = plan->workers;
    if (workers < 1)
        workers = 1;
    if (workers > 1)
        for (int h = 0; h < workers; ++h)
        {
            if (sgpl_tdg_env_enabled("SGPL_TDG_TEST_THREAD_FAIL", 0) ||
                pthread_create(&threads[started], NULL, sgpl_joint_worker, &state) != 0)
                break;
            ++started;
        }
    pthread_mutex_lock(&state.lock);
    state.gate = 1;
    pthread_cond_broadcast(&state.change);
    pthread_mutex_unlock(&state.lock);
    if (started == 0)
        sgpl_joint_worker(&state); // Safe inline progress if creation fails.
    for (int h = 0; h < started; ++h)
        pthread_join(threads[h], NULL);
    pthread_cond_destroy(&state.change);
    pthread_mutex_destroy(&state.lock);
}

void sgpl_run_tdg_level_joint(const sgpl_tdg_task_desc *tasks, int32_t count,
                              int64_t work_units, int64_t span_units)
{
    if (!tasks || count <= 0)
        return;
    g_tls_joint_last_prediction_ns = 0.0;
    if (sgpl_tdg_level_active() || g_tls_dispatch_depth > 0)
    {
        // The existing nested scope owns these resources. The model covers a
        // single top-level level and cannot create another independent grant.
        sgpl_run_tdg_level_fifo(tasks, count, work_units, span_units);
        return;
    }
    atomic_store_explicit(&g_joint_collect_enabled, 1, memory_order_relaxed);
    int budget = sgpl_budget_try_reserve(sgpl_budget_available_threads());
    int ordinary = 0;
    for (int j = 0; j < count; ++j)
        ordinary += tasks[j].num_loop_sites <= 0 || !tasks[j].loop_site_ids;
    sgpl_joint_plan plan;
    uint64_t planning_begin = sgpl_now_ns();
    int known = budget > 1 && budget <= SGPL_JOINT_MAX_BUDGET && count <= SGPL_JOINT_MAX_TASKS &&
                !force_doall_parallel_enabled() && !force_doacross_parallel_enabled() &&
                !getenv("SGPL_FORCE_WIDTHS") &&
                !(count <= budget && ordinary == count) &&
                !(count <= budget && sgpl_tdg_env_enabled("SGPL_JOINT_OVERLOAD_ONLY", 0)) &&
                !sgpl_tdg_env_enabled("SGPL_TDG_TEST_ALLOC_FAIL", 0) &&
                sgpl_joint_get_plan(tasks, count, budget, &plan);
    uint64_t observation_key = budget > 1 && budget <= SGPL_JOINT_MAX_BUDGET && count <= SGPL_JOINT_MAX_TASKS ?
        sgpl_joint_level_key(tasks, count, budget) : 0;
    int observed_fallback = known && !plan.reference &&
        sgpl_joint_observed_guard(observation_key, &plan, (double)(sgpl_now_ns() - planning_begin));
    if (observed_fallback)
    {
        plan.reference = 1;
        atomic_fetch_add(&g_joint_observed_fallbacks, 1);
    }
    if (known)
        g_tls_joint_last_prediction_ns = (plan.reference ? plan.reference_ns : plan.prediction_ns) +
            (double)(sgpl_now_ns() - planning_begin);
    uint64_t execution_begin = sgpl_now_ns();
    int reference = !known || plan.reference;
    if (reference)
    {
        atomic_fetch_add(&g_joint_reference_calls, 1);
        if (tdg_debug_enabled())
            fprintf(stderr, "[tdg.joint] policy=fifo reason=%s budget=%d tasks=%d\n",
                    observed_fallback ? "observed-baseline" : known ? "model-or-overhead" : "profile-or-scope", budget, count);
        sgpl_run_tdg_fifo_reserved(tasks, count, work_units, span_units, budget);
    }
    else
    {
        if (tdg_debug_enabled())
        {
            fprintf(stderr, "[tdg.joint] policy=joint budget=%d tasks=%d workers=%d prediction_ns=%.0f reference_ns=%.0f planning_ns=%.0f evaluations=%d\n",
                budget, count, plan.workers, plan.prediction_ns, plan.reference_ns, plan.planning_ns, plan.evaluations);
            for (int r = 0; r < count; ++r)
                fprintf(stderr, "[tdg.joint.task] rank=%d task=%d demand=%d\n",
                        r, plan.order[r], 1 + plan.peak[plan.order[r]]);
        }
        sgpl_joint_execute(tasks, &plan);
    }
    if (observation_key)
        sgpl_joint_observe(sgpl_joint_level_key(tasks, count, budget), &plan, reference,
            (double)(sgpl_now_ns() - execution_begin));
    sgpl_budget_release(budget);
}
