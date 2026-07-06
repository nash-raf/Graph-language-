/*
 * hw_calib_bench.c — Offline hardware calibration for the autotuner cost model.
 *
 * Measures three hardware parameters and writes them as JSON to stdout:
 *   L : cache-line size in bytes (from sysconf, fallback 64)
 *   t : random-access (pointer-chasing) latency, in nanoseconds
 *   T : sequential cache-line transfer cost, in nanoseconds per cache line
 *
 * Usage:
 *   ./hw_calib_bench > ~/.config/sgpl/hw_calib.json
 *
 * The autotuner pass reads this JSON via $AUTOTUNER_HW_CALIB (or the default
 * path above). If missing, the pass falls back to {L=64, t=1, T=1}.
 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

/* Measure random-access latency t via pointer-chasing through a shuffled
 * array of indices. Each step is a dependent load — models the redirection
 * cost of navigating metadata (row_ptr, brow, etc.). */
static double measure_t(long N) {
    if (N < 1) N = 1;
    uint32_t *next = (uint32_t *)malloc((size_t)N * sizeof(uint32_t));
    if (!next) return 1.0;

    /* Initialize sequential, then Fisher-Yates shuffle. */
    for (long i = 0; i < N; i++) next[i] = (uint32_t)i;
    srand(42);
    for (long i = N - 1; i > 0; i--) {
        long j = (long)((double)rand() / ((double)RAND_MAX + 1.0) * (i + 1));
        uint32_t tmp = next[i];
        next[i] = next[j];
        next[j] = tmp;
    }

    /* Chase pointers to defeat prefetchers. */
    volatile uint32_t idx = 0;
    long iters = N * 10;
    double t0 = now_ns();
    for (long k = 0; k < iters; k++) {
        idx = next[idx];
    }
    double t1 = now_ns();
    free(next);

    double per_access_ns = (t1 - t0) / (double)iters;
    return per_access_ns;
}

/* Measure sequential cache-line transfer cost T by streaming through a
 * large array, touching one byte per cache line. */
static double measure_T(long bytes, long line_size) {
    if (bytes < line_size) bytes = line_size * 1024;
    if (line_size < 1) line_size = 64;

    char *buf = (char *)malloc((size_t)bytes);
    if (!buf) return 1.0;
    memset(buf, 0, (size_t)bytes);

    long num_lines = bytes / line_size;
    volatile char sink = 0;
    double t0 = now_ns();
    for (long pass = 0; pass < 20; pass++) {
        for (long i = 0; i < num_lines; i++) {
            sink ^= buf[i * line_size];
        }
    }
    double t1 = now_ns();
    free(buf);

    double per_line_ns = (t1 - t0) / (double)(num_lines * 20);
    return per_line_ns;
}

int main(void) {
    long L = (long)sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (L <= 0) L = 64;

    /* Use a moderate working set for pointer-chasing (~256K elements = 1MB
     * for uint32_t) to exceed L2 on typical machines. */
    double t = measure_t(256 * 1024);

    /* Stream ~16MB to measure cache-line transfer cost. */
    double T = measure_T(16 * 1024 * 1024, L);

    printf("{ \"L\": %ld, \"t\": %.4f, \"T\": %.4f }\n", L, t, T);
    return 0;
}