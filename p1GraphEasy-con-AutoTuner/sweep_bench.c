#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <float.h>

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static long detect_llc(void) {
    long best = 0;
    for (int idx = 0; idx < 10; idx++) {
        char path[128];
        snprintf(path, sizeof path,
                 "/sys/devices/system/cpu/cpu0/cache/index%d/size", idx);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        char buf[64];
        if (fgets(buf, sizeof buf, f)) {
            char *end = NULL;
            long val = strtol(buf, &end, 10);
            if (end) {
                if (*end == 'K' || *end == 'k') val *= 1024L;
                else if (*end == 'M') val *= 1024L * 1024L;
                else if (*end == 'G') val *= 1024L * 1024L * 1024L;
            }
            if (val > best) best = val;
        }
        fclose(f);
    }
#ifdef _SC_LEVEL3_CACHE_SIZE
    if (best <= 0) best = (long)sysconf(_SC_LEVEL3_CACHE_SIZE);
#endif
    if (best <= 0) best = 8L * 1024 * 1024;
    return best;
}

/* Run passes memmove iterations, return ns per cache line. */
static double run_bench(long ws, long line, int passes) {
    char *buf = (char *)malloc((size_t)ws + 64);
    if (!buf) return 0.0;
    memset(buf, 1, (size_t)ws + 64);
    memmove(buf + 8, buf, (size_t)ws);
    double t0 = now_ns();
    for (int p = 0; p < passes; p++)
        memmove(buf + 8, buf, (size_t)ws);
    double t1 = now_ns();
    free(buf);
    return (t1 - t0) / ((double)passes * ((double)ws / (double)line));
}

static double measure_memmove_per_line(long ws, long line, int passes, int trials) {
    double best = DBL_MAX;
    for (int t = 0; t < trials; t++) {
        double per = run_bench(ws, line, passes);
        if (per > 0.0 && per < best)
            best = per;
    }
    return best;
}

int main(void) {
    fprintf(stderr, "Usage: taskset -c 0 chrt -f 99 ./sweep_bench | python3 plot_sweep.py\n");
    fprintf(stderr, "       (mlockall skipped due to RLIMIT_MEMLOCK; taskset + min-of-7 sufficient)\n\n");

    long llc = detect_llc();
    long line = 64;
    int trials = 7;

    fprintf(stderr, "LLC = %ld bytes (%.2f MB)\n", llc, llc / (1024.0*1024.0));

    double mults[] = {0.125, 0.25, 0.375, 0.5, 0.75, 1.0,
                      1.125, 1.25, 1.375, 1.5, 1.625, 1.75,
                      1.875, 2.0, 2.5, 3.0, 4.0};
    int n = sizeof(mults) / sizeof(mults[0]);

    long small = llc / 4;
    if (small < (1L << 20)) small = 1L << 20;
    double Tm = measure_memmove_per_line(small, line, 128, trials);
    fprintf(stderr, "Tm (cache-resident per-line cost, 128 passes x %d trials) = %.4f ns\n", trials, Tm);

    printf("ws_bytes,ws_llc_ratio,ns_per_line,penalty\n");
    for (int i = 0; i < n; i++) {
        long ws = (long)(mults[i] * (double)llc);
        if (ws < line * 4) ws = line * 4;
        int passes = (ws < llc) ? 256 : (ws < 2 * llc) ? 64 : 32;
        double per = measure_memmove_per_line(ws, line, passes, trials);
        if (per <= 0.0) continue;
        double pen = per / Tm;
        printf("%ld,%.3f,%.4f,%.4f\n", ws, mults[i], per, pen);
    }
    return 0;
}
