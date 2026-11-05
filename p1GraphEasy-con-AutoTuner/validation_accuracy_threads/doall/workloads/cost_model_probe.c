/* Diagnostic only: exercise the unchanged runtime and its private equations.
 * Include the implementation so predictions use the exact production helpers.
 * Build through cost_model_audit.py; no graph-layout autotuner is linked. */
#include "../parallel_runtime.c"
#include <assert.h>

static double *values[2];
static int chain_length = 12;
static int streaming;
static int distance = 1;
static int pair_ratio = 1;

static void body(int64_t i, void *env)
{
    int site = (int)(intptr_t)env;
    double x = (double)(i + 1) * 0.37;
    if (streaming) {
        values[site][i] = x;
        return;
    }
    double s = x * .5;
    for (int k = 0; k < chain_length * (site ? pair_ratio : 1); ++k) {
        s += x / (1.0 + s * s + k);
        x = x * 1.0000001 + s;
    }
    values[site][i] = s;
}

static void range(int64_t first, int64_t count, int64_t step, void *env)
{
    for (int64_t k = 0; k < count; ++k)
        body(first + k * step, env);
}

static void recurrence(int64_t i, void *env)
{
    (void)env;
    doacross_wait(i, distance, 0);
    values[0][i] = values[0][i - distance] * .5 + 3.0;
    doacross_post(i, 0);
}

static void sync_only(int64_t i, void *env)
{
    (void)env;
    doacross_wait(i, distance, 0);
    doacross_post(i, 0);
}

static double median(double *samples, int n)
{
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (samples[j] < samples[i]) {
                double t = samples[i]; samples[i] = samples[j]; samples[j] = t;
            }
    return samples[n / 2];
}

static double measure_site(int mode, int64_t n, int p, int reps, int site)
{
    double samples[15];
    for (int r = 0; r < reps; ++r) {
        uint64_t t = sgpl_now_ns();
        if (mode == 0)
            range(0, n, 1, (void *)(intptr_t)site);
        else {
            if (mode == 1) sgpl_set_pending_range_body(range);
            sgpl_parallel_launch_plain_raw(mode == 2 ? distance : 0, n, 1,
                mode == 2 ? recurrence : body, (void *)(intptr_t)site, mode == 2, mode == 2, p);
        }
        samples[r] = (double)(sgpl_now_ns() - t);
    }
    return median(samples, reps);
}

static double measure(int mode, int64_t n, int p, int reps)
{
    return measure_site(mode, n, p, reps, 0);
}

struct task { int site; int64_t n; int width; uint64_t begin, end; };
static pthread_barrier_t task_start;
static void *parallel_task(void *arg)
{
    struct task *t = arg;
    pthread_barrier_wait(&task_start);
    t->begin = sgpl_now_ns();
    if (t->width <= 1)
        range(0, t->n, 1, (void *)(intptr_t)t->site);
    else {
        sgpl_set_pending_range_body(range);
        sgpl_parallel_launch_plain_raw(0, t->n, 1, body,
            (void *)(intptr_t)t->site, 0, 0, t->width);
    }
    t->end = sgpl_now_ns();
    return NULL;
}

int main(int argc, char **argv)
{
    int64_t n = argc > 1 ? atoll(argv[1]) : 20000;
    int p = sgpl_configured_worker_count();
    chain_length = argc > 2 ? atoi(argv[2]) : 12;
    streaming = argc > 3 ? atoi(argv[3]) : 0;
    distance = argc > 4 ? atoi(argv[4]) : 1;
    for (int i = 0; i < 2; ++i) {
        values[i] = calloc((size_t)n + 16, sizeof(double));
        if (!values[i]) return 2;
    }
    sgpl_loop_profile_desc all = {.loop_id = 201, .mode = SGPL_LOOP_DOALL};
    sgpl_loop_profile_desc across = {.loop_id = 202, .mode = SGPL_LOOP_DOACROSS,
        .has_doacross_profile = 1, .doacross_waits_per_iter = 1,
        .doacross_posts_per_iter = 1, .doacross_num_sync_ids = 1};
    sgpl_thread_pool_available(); /* exclude first pool creation from warm rows */
    /* Structural checks, independent of timing noise. */
    assert(sgpl_loop_max_lane_iterations(1025, 4, 0) == 257.0);
    assert(sgpl_loop_max_lane_iterations(5, 4, 0) == 2.0);
    assert(sgpl_loop_max_lane_iterations(8191, 4, 1) == 2048.0);
    assert(sgpl_loop_max_lane_iterations(0, 4, 0) == 0.0);
    double serial = measure(0, n, p, 7);
    for (int i = 0; i < 6; ++i)
        sgpl_record_doall_serial_sample(&all, 0, n, 1, (uint64_t)serial);
    sgpl_set_pending_range_body(range);
    sgpl_launch_overhead_detail la = sgpl_get_launch_overhead_detail(&all, p);
    assert(g_tls_pending_range_body == range);
    g_tls_pending_range_body = NULL;
    sgpl_launch_overhead_detail ld = sgpl_get_launch_overhead_detail(&across, p);
    double empty[7], empty_across[7];
    for (int i = 0; i < 7; ++i) {
        uint64_t t = sgpl_now_ns();
        sgpl_parallel_launch_plain_raw(0, p, 1, sgpl_noop_body, NULL, 0, 0, p);
        empty[i] = (double)(sgpl_now_ns() - t);
        t = sgpl_now_ns();
        sgpl_parallel_launch_plain_raw(0, p, 1, sgpl_noop_body, NULL, 1, 1, p);
        empty_across[i] = (double)(sgpl_now_ns() - t);
    }
    double par = measure(1, n, p, 7);
    double across_ns = measure(2, n, p, 7);
    /* Proposed mechanism calibration, outside production: price the SAME
     * cyclic wait/post protocol at a separate 4096-trip calibration size.
     * Distance is a proposed input, currently absent from the descriptor. */
    double sync_calibration[7], useful_serial[7];
    int64_t calibration_trips = 4096;
    for (int r = 0; r < 7; ++r) {
        uint64_t begin = sgpl_now_ns();
        sgpl_parallel_launch_plain_raw(distance, distance + calibration_trips,
            1, sync_only, NULL, 1, 1, p);
        sync_calibration[r] = (double)(sgpl_now_ns() - begin);
        begin = sgpl_now_ns();
        for (int64_t i = distance; i < n; ++i)
            values[0][i] = values[0][i - distance] * .5 + 3.0;
        useful_serial[r] = (double)(sgpl_now_ns() - begin);
    }
    int chains = distance < p ? distance : p;
    double handoff = fmax(0.0, median(sync_calibration, 7) - ld.total_ns) * chains / calibration_trips;
    double useful_c = median(useful_serial, 7) / (n - distance);
    double across_proposal = ld.total_ns + (double)(n - distance) / chains * (handoff + useful_c);
    /* Reproduce the serial basic-block clock intervals: the sampler receives
     * just the interval, while elapsed wall time includes both clock calls
     * and accumulation outside the interval. This is not a fit parameter. */
    uint64_t t = sgpl_now_ns(), dep_total = 0;
    for (int64_t i = distance; i < n; ++i) {
        uint64_t b = sgpl_now_ns();
        values[0][i] = values[0][i - distance] * .5 + 3.0;
        uint64_t e = sgpl_now_ns();
        dep_total += e - b;
    }
    uint64_t serial_profile_wall = sgpl_now_ns() - t;
    double clock_outside = (double)(serial_profile_wall - dep_total) / (n - distance);
    for (int i = 0; i < 6; ++i)
        sgpl_record_doacross_serial_sample(&across, distance, n, 1, dep_total, 0);
    int pure_chain_decision = sgpl_should_parallelize_doacross(&across, distance, n, 1);
    assert(g_loop_states[202].c_sampling_state == SGPL_C_SAMPLING_STABLE);
    assert(g_loop_states[202].decision_cache_valid); /* a real calibrated verdict */
    assert(pure_chain_decision == 0);
    double pred = sgpl_loop_parallel_model_time_ns(&all, &g_loop_states[201], n, p);
    printf("all,%d,%lld,%d,%d,%.0f,%.0f,%.0f,%.0f,%.0f\n",
        p, (long long)n, chain_length, streaming, serial, pred, par,
        la.total_ns, median(empty, 7));
    printf("across,%d,%lld,%d,%.0f,%.0f,%.0f,%.0f,%.0f,%.3f,%d\n",
        p, (long long)n, distance, (double)dep_total, (double)serial_profile_wall,
        across_ns, ld.total_ns, median(empty_across, 7), clock_outside, pure_chain_decision);
    printf("across-proposal,%d,%lld,%d,%.0f,%.0f,%.3f,%.3f\n", p,
        (long long)n, distance, across_proposal, across_ns, handoff, useful_c);
    /* Directly compare simultaneous task arrivals with exclusive pool service. */
    for (pair_ratio = 1; pair_ratio <= (streaming ? 1 : 3); pair_ratio += 2) {
      double serial_heavy = pair_ratio == 1 ? serial : measure_site(0, n, p, 7, 1);
      for (int choice = 0; choice < p; ++choice) {
        int w1 = choice == 0 ? 1 : choice;
        int w2 = choice == 0 ? 1 : p - w1;
        if (choice == 1 && p == 2) continue;
        struct task tasks[2] = {{0, n, w1, 0, 0}, {1, n, w2, 0, 0}};
        double alone1 = w1 <= 1 ? serial : measure(1, n, w1, 5);
        double alone2 = w2 <= 1 ? serial_heavy : measure_site(1, n, w2, 5, 1);
        double paired[5];
        for (int r = 0; r < 5; ++r) {
            pthread_t threads[2];
            pthread_barrier_init(&task_start, NULL, 3);
            for (int i = 0; i < 2; ++i)
                pthread_create(&threads[i], NULL, parallel_task, &tasks[i]);
            uint64_t begin = sgpl_now_ns();
            pthread_barrier_wait(&task_start);
            for (int i = 0; i < 2; ++i) pthread_join(threads[i], NULL);
            paired[r] = (double)(sgpl_now_ns() - begin);
            pthread_barrier_destroy(&task_start);
        }
        double old_sum = alone1 + alone2;
        double resource_model = (w1 > 1 && w2 > 1) ? old_sum : fmax(alone1, alone2);
        printf("pair,%d,%lld,%d,%d,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%.0f\n", p,
            (long long)n, chain_length, streaming, pair_ratio, w1, w2,
            alone1, alone2, old_sum, resource_model, median(paired, 5));
      }
    }
    printf("checksum,%.17g\n", values[0][n - 1] + values[1][n - 1]);
    return 0;
}
