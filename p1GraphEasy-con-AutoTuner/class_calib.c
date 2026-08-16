/* class_calib.c — Step 6: class-specific per-line costs by cache residency.
 *
 * For each access class (sequential read, overlapping R-M-W shift, random
 * pointer-chase) measure the per-cache-line cost at working-set sizes that
 * pin the array to a specific residency level:
 *     L2-resident   : ws <= L2 bytes
 *     L3-resident   : L2 < ws <= LLC bytes
 *     DRAM-resident : ws > LLC bytes
 *
 * Output (JSON on stdout):
 *     { l2_bytes, llc_bytes, levels: {class: {L2, L3, DRAM: ns/line}},
 *       curves: {class: [{ws_bytes, ns_per_line, tier}]} }
 *
 * Reuses the timing/memmove/chase patterns of hw_calib_bench.c.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

/* Cache size at a given level (level 2 = L2, 3 = LLC). */
static long detect_level_bytes(int level) {
    long best = 0;
    char path[128], buf[64];
    for (int idx = 0; idx < 16; idx++) {
        snprintf(path, sizeof path,
                 "/sys/devices/system/cpu/cpu0/cache/index%d/level", idx);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        long lv = 0;
        if (fscanf(f, "%ld", &lv) != 1) lv = 0;
        fclose(f);
        if (lv != level) continue;
        snprintf(path, sizeof path,
                 "/sys/devices/system/cpu/cpu0/cache/index%d/size", idx);
        f = fopen(path, "r");
        if (!f) continue;
        if (fgets(buf, sizeof buf, f)) {
            char *end = NULL;
            long val = strtol(buf, &end, 10);
            if (end) {
                if (*end == 'K') val *= 1024L;
                else if (*end == 'M') val *= 1024L * 1024L;
                else if (*end == 'G') val *= 1024L * 1024L * 1024L;
            }
            if (val > best) best = val;
        }
        fclose(f);
    }
    return best;
}

/* Sequential read: one byte per cache line.  per_line = ns/line. */
static double seq_read_per_line(long bytes, long line, int passes) {
    if (bytes < line * 4) bytes = line * 4;
    char *buf = (char *)malloc((size_t)bytes);
    if (!buf) return 0.0;
    memset(buf, 0, (size_t)bytes);
    long num_lines = bytes / line;
    volatile char sink = 0;
    double t0 = now_ns();
    for (int pass = 0; pass < passes; pass++) {
        for (long i = 0; i < num_lines; i++)
            sink ^= buf[i * line];
    }
    double t1 = now_ns();
    free(buf);
    return (t1 - t0) / ((double)num_lines * (double)passes);
}

/* Overlapping 8-byte forward shift (the BCSR insert memmove pattern). */
static double rmw_per_line(long ws, long line, int passes) {
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

/* Dependent sequential R-M-W (the CSR row_ptr prefix-loop pattern):
 * for i: a[i] += 1  — every int64 element is loaded, updated, stored; the
 * iteration is sequential so each cache line is loaded once, updated
 * in-line, and written back per pass.  Unlike memmove, the read and write
 * touch the SAME line, serializing the load -> modify -> storeback.
 *
 * no-tree-vectorize keeps the scalar addq $1, mem pattern that the kernel's
 * row_ptr[from+1..n]++ loop actually compiles to (verified on the compiled
 * graph_mutation_runtime.c) — vectorization would hide the per-element
 * dependent R-M-W and measure a cheaper read+write stream. */
__attribute__((optimize("no-tree-vectorize")))
static double dep_per_line(long ws, long line, int passes) {
    long N = ws / 8;
    if (N < 8) N = 8;
    int64_t *a = (int64_t *)malloc((size_t)N * 8);
    if (!a) return 0.0;
    for (long i = 0; i < N; i++) a[i] = 1;
    volatile int64_t sink = 0;
    double t0 = now_ns();
    for (int p = 0; p < passes; p++)
        for (long i = 0; i < N; i++) a[i] += 1;
    double t1 = now_ns();
    (void)sink;
    free(a);
    long lines = (N * 8 + line - 1) / line;
    return (t1 - t0) / ((double)lines * (double)passes);
}

/* CSR col_idx memmove tail (the actual kernel access pattern):
 * csr_add_directed shifts the int32 tail [pos, old_m) one element right via
 * libc memmove(dst = src + 4, src, len) — a 4-byte overlapping forward
 * shift (glibc copies backward for dst > src), then writes the freed slot.
 * Same libc path, same direction/order, same element size as the kernel. */
static double csr_move_per_line(long ws, long line, int passes) {
    int32_t *buf = (int32_t *)malloc((size_t)ws + 64);
    if (!buf) return 0.0;
    for (long i = 0; i < (ws + 64) / 4; i++) buf[i] = 1;
    memmove(buf + 1, buf, (size_t)ws);          /* dst = src + 4 bytes */
    double t0 = now_ns();
    for (int p = 0; p < passes; p++)
        memmove(buf + 1, buf, (size_t)ws);
    double t1 = now_ns();
    free(buf);
    return (t1 - t0) / ((double)passes * ((double)ws / (double)line));
}

/* Random access: dependent pointer chase over shuffled uint32 indices.
 * Array of N = bytes/4 elements spread over the working set. */
static double rand_per_line(long bytes, int passes) {
    long N = bytes / 4;
    if (N < 8) N = 8;
    uint32_t *next = (uint32_t *)malloc((size_t)N * sizeof(uint32_t));
    if (!next) return 0.0;
    for (long i = 0; i < N; i++) next[i] = (uint32_t)i;
    srand(42);
    for (long i = N - 1; i > 0; i--) {
        long j = (long)((double)rand() / ((double)RAND_MAX + 1.0) * (i + 1));
        uint32_t tmp = next[i];
        next[i] = next[j];
        next[j] = tmp;
    }
    volatile uint32_t idx = 0;
    long iters = N * (long)passes;
    double t0 = now_ns();
    for (long k = 0; k < iters; k++)
        idx = next[idx];
    double t1 = now_ns();
    free(next);
    /* N dependent accesses cover N lines => ns/line */
    return (t1 - t0) / (double)N / (double)passes;
}

#define MAX_PTS 40

static long g_ws[MAX_PTS];
static double g_seq[MAX_PTS], g_rmw[MAX_PTS], g_rand[MAX_PTS];
static double g_dep[MAX_PTS], g_csrmove[MAX_PTS];
static int g_n = 0;

/* Log-spaced sweep from L2/8 up to 8*LLC. */
static void sweep(long l2, long llc, long line) {
    double lo = log((double)l2 / 8.0);
    double hi = log((double)llc * 8.0);
    for (int i = 0; i < MAX_PTS; i++) {
        long ws = (long)(exp(lo + (hi - lo) * (double)i / (double)(MAX_PTS - 1)) + 0.5);
        if (ws < line * 8) ws = line * 8;
        /* passes: keep ~64MB of total traffic per class per point */
        int passes = (int)((256L * 1024 * 1024) / ws);
        if (passes < 2) passes = 2;
        if (passes > 4096) passes = 4096;
        g_ws[g_n] = ws;
        g_seq[g_n]  = seq_read_per_line(ws, line, passes);
        g_rmw[g_n]  = rmw_per_line(ws, line, passes);
        g_rand[g_n] = rand_per_line(ws, 8);
        g_dep[g_n]  = dep_per_line(ws, line, passes);
        g_csrmove[g_n] = csr_move_per_line(ws, line, passes);
        g_n++;
    }
}

static const char *tier_of(long l2, long llc, long ws) {
    if (ws <= l2) return "L2";
    if (ws <= llc) return "L3";
    return "DRAM";
}

static double tier_rate(long l2, long llc, long line, const double *v) {
    /* Representative cost at a ws firmly inside each residency band. */
    long l2_ws = l2 / 2, l3_ws = (l2 + llc) / 2, dram_ws = 4 * llc;
    double best = 0.0;
    for (int i = 0; i < g_n; i++) {
        double d = (double)g_ws[i];
        double want = 0.0;
        if (line == 0) want = (double)l2_ws; else if (line == 1) want = (double)l3_ws; else want = (double)dram_ws;
        double dist = fabs(d - want);
        (void)dist;
    }
    /* simpler: nearest measured ws to the representative size */
    long target = line == 0 ? l2 / 2 : line == 1 ? (l2 + llc) / 2 : 4 * llc;
    double bd = 1e300;
    for (int i = 0; i < g_n; i++) {
        double d = fabs((double)g_ws[i] - (double)target);
        if (d < bd) { bd = d; best = v[i]; }
    }
    return best;
}

int main(void) {
    long line = (long)sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (line <= 0) line = 64;
    long l2 = detect_level_bytes(2);
    long llc = detect_level_bytes(3);
#ifdef _SC_LEVEL3_CACHE_SIZE
    if (llc <= 0) llc = (long)sysconf(_SC_LEVEL3_CACHE_SIZE);
#endif
    if (llc <= 0) llc = 12L * 1024 * 1024;
    if (l2 <= 0) l2 = llc / 10;

    sweep(l2, llc, line);

    printf("{\n  \"l2_bytes\": %ld,\n  \"llc_bytes\": %ld,\n  \"levels\": {\n", l2, llc);
    const char *names[5] = {"seq", "rmw", "rand", "dep", "csr_move"};
    const double *arrs[5] = {g_seq, g_rmw, g_rand, g_dep, g_csrmove};
    for (int c = 0; c < 5; c++) {
        printf("    \"%s\": {\"L2\": %.4f, \"L3\": %.4f, \"DRAM\": %.4f}%s\n",
               names[c],
               tier_rate(l2, llc, 0, arrs[c]),
               tier_rate(l2, llc, 1, arrs[c]),
               tier_rate(l2, llc, 2, arrs[c]),
               c < 4 ? "," : "");
    }
    printf("  },\n  \"curves\": {\n");
    for (int c = 0; c < 5; c++) {
        printf("    \"%s\": [", names[c]);
        for (int i = 0; i < g_n; i++) {
            printf("%s{\"ws\": %ld, \"ns_per_line\": %.4f, \"tier\": \"%s\"}",
                   i ? ", " : "", g_ws[i], arrs[c][i],
                   tier_of(l2, llc, g_ws[i]));
        }
        printf("]%s\n", c < 4 ? "," : "");
    }
    printf("  }\n}\n");
    return 0;
}
