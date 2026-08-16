/*
 * shift_state_bench.c — Decompose the BCSR insert cMove slowdown.
 *
 * The standalone memmove curve used by the cost model (hw_calib_bench.c)
 * predicts ~1.16 ns/line for a cached shift, while the real BCSR insert
 * kernel (autotuner_runtime.c autograph_bcsr_add_edge) behaves ~3.3 ns/line
 * at the same footprint.  This benchmark adds the kernel's preceding
 * operations one at a time — at IDENTICAL array size, shift offset and
 * move length — timing ONLY the memmove, so we can attribute the gap:
 *
 *   A  standalone overlapping memmove            (clean baseline)
 *   B  realloc(+2 ints)         -> memmove       (realloc cache-state effect)
 *   C  dup-scan + realloc(+2)   -> memmove       (scan + realloc effect)
 *   D  dup-scan + insert-scan + realloc(+2) -> memmove (full add path)
 *
 * Every measured pass replicates the real kernel's step sequence:
 * realloc(bcol, (total_ints+2)*4), then
 * memmove(&new_bcol[insert_pos+2], &new_bcol[insert_pos], tail*4)
 * (the same 8-byte-overlapping tail shift at :1462), and (D only) the
 * brow[i] += 2 prefix sweep after the timer.
 *
 * Realloc outcomes (in-place vs migrated) are recorded per pass and
 * separated into t_inplace / t_migrated.
 *
 * Usage:
 *   ./shift_state_bench [scan_elems_csv]   # default "256,4096"
 * Output: JSON array of points to stdout.
 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LINE_BYTES 64L

/* Magic pair that never occurs in the sentinel-filled array: the dup-scan
 * and the insert-position scan (all sentinel values are <= 97) always walk
 * the whole segment, matching a real failed lookup & a late insertion. */
#define MAGIC_ROW 200
#define MAGIC_COL 4095

static volatile int64_t g_sink;

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

typedef struct {
    int variant;              /* 0=A, 1=B, 2=C, 3=D */
    long ws;                  /* array size in bytes */
    double frac;              /* insert position = frac * total_ints */
    long scan_elems;          /* dup-scan segment length (ints) */
    long passes;
    double per_line;          /* ns/line median */
    double t_inplace;         /* ns/line, in-place realloc passes (-1 if none) */
    double t_migrated;        /* ns/line, migrated passes (-1 if none) */
    long n_inplace, n_migrated;
} PointOut;

static int cmp_d(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

static double median5(double v[5]) {
    double c[5];
    memcpy(c, v, sizeof(c));
    qsort(c, 5, sizeof(double), cmp_d);
    return c[2];
}

/* One repeat of one point: returns per-line (acc/ns), in-place/migrated
 * split.  probe==1 => fixed 24-pass probe used to pick the main pass count. */
static void repeat_once(long total_ints, long ip, long move_ints,
                        int variant, long scan_elems, long brow_elems,
                        int passes, int probe,
                        double *per_line, double *t_in, double *t_mi,
                        long *n_in, long *n_mi) {
    int32_t *bcol = (int32_t *)malloc((size_t)total_ints * 4);
    if (!bcol) {
        *per_line = 0.0;
        return;
    }
    for (long i = 0; i < total_ints; i++)
        bcol[i] = (int32_t)(1 + (i % 97));

    int32_t *brow = (int32_t *)malloc((size_t)brow_elems * 4);
    if (brow)
        for (long i = 0; i < brow_elems; i++)
            brow[i] = (int32_t)(2 * i);

    long seg_end = ip;                              /* insert_pos */
    long seg_start = seg_end - (scan_elems - 2);
    if (seg_start < 0) seg_start = 0;
    if (seg_start > seg_end) seg_start = seg_end;

    int warmup_passes = (probe || passes == 0) ? 1 : 1;
    (void)warmup_passes;

    double acc = 0.0;
    double acc_in = 0.0, acc_mi = 0.0;
    long cnt_in = 0, cnt_mi = 0;
    long cur_total = total_ints;
    int32_t *cur = bcol;

    for (int p = 0; p < passes + 1; p++) {
        int is_warmup = (p == 0);
        int moved_flag = 0;
        if (variant >= 1) {
            int32_t *old = cur;
            cur = (int32_t *)realloc(cur, (size_t)(cur_total + 2) * 4);
            if (!cur) cur = old;
            moved_flag = (cur != old);
        }
        if (variant >= 2) {
            for (long k = seg_start; k < seg_end; k += 2)
                g_sink += cur[k] + cur[k + 1];
        }
        if (variant >= 3) {
            for (long k = seg_start; k < seg_end; k += 2)
                if (cur[k] > MAGIC_ROW) break;
        }

        size_t mv = (size_t)move_ints * 4;
        double t0 = now_ns();
        memmove(cur + ip + 2, cur + ip, mv);
        double t1 = now_ns();

        if (variant >= 3 && brow) {
            for (long i = 2; i < brow_elems; i++)
                brow[i] += 2;
            g_sink += brow[brow_elems - 1];
        }

        if (!is_warmup) {
            double per = t1 - t0;
            acc += per;
            if (variant >= 1) {
                if (moved_flag) { acc_mi += per; cnt_mi++; }
                else            { acc_in += per; cnt_in++; }
            }
        }
        cur_total += 2;
    }

    if (g_sink == 0x7fffffffffffffffLL) per_line[0] = 0.0; /* never: keep sink live */

    double lines = (double)move_ints * 4.0 / (double)LINE_BYTES;
    *per_line = acc / ((double)passes * lines);
    *t_in = (cnt_in > 0) ? acc_in / ((double)cnt_in * lines) : -1.0;
    *t_mi = (cnt_mi > 0) ? acc_mi / ((double)cnt_mi * lines) : -1.0;
    *n_in = cnt_in;
    *n_mi = cnt_mi;

    free(brow);
    free(cur);
}

static void run_point(int variant, long ws, double frac, long scan_elems,
                      long brow_elems, PointOut *out) {
    long total_ints = (long)((double)ws / 4.0);
    long ip = (long)((double)total_ints * frac);
    if (ip > total_ints - 4) ip = total_ints - 4;
    if (ip < 0) ip = 0;
    long move_ints = total_ints - ip;
    if (move_ints < 64) move_ints = 64;
    double move_lines = (double)move_ints * 4.0 / (double)LINE_BYTES;

    /* Probe to pick a pass count targeting ~3 ms of timed memmove. */
    double pl, ti, tm;
    long ni, nm;
    repeat_once(total_ints, ip, move_ints, variant, scan_elems, brow_elems,
                24, 1, &pl, &ti, &tm, &ni, &nm);
    if (pl <= 0.0) pl = 2.0;
    long passes = (long)(3e6 / (pl * move_lines));
    if (passes < 8) passes = 8;
    if (passes > 600) passes = 600;

    double per[5] = {0}, t_in[5] = {0}, t_mi[5] = {0};
    long n_in[5] = {0}, n_mi[5] = {0};
    for (int r = 0; r < 5; r++)
        repeat_once(total_ints, ip, move_ints, variant, scan_elems, brow_elems,
                    (int)passes, 0, &per[r], &t_in[r], &t_mi[r], &n_in[r], &n_mi[r]);

    out->variant = variant;
    out->ws = ws;
    out->frac = frac;
    out->scan_elems = scan_elems;
    out->passes = passes;
    out->per_line = median5(per);
    out->t_inplace = median5(t_in);
    out->t_migrated = median5(t_mi);
    out->n_inplace = n_in[0] + n_in[1] + n_in[2] + n_in[3] + n_in[4];
    out->n_migrated = n_mi[0] + n_mi[1] + n_mi[2] + n_mi[3] + n_mi[4];
}

static const char *vname(int v) {
    return v == 0 ? "A" : v == 1 ? "B" : v == 2 ? "C" : "D";
}

int main(int argc, char **argv) {
    static const long ws_list[] = {
        512L * 1024, 1024L * 1024, 2L * 1024 * 1024, 4L * 1024 * 1024,
        8L * 1024 * 1024, 16L * 1024 * 1024, 32L * 1024 * 1024,
        64L * 1024 * 1024
    };
    static const double frac_list[] = {0.25, 0.5, 0.75};
    long scan_list[4];
    int n_scan = 2;
    scan_list[0] = 256;
    scan_list[1] = 4096;
    if (argc > 1) {
        n_scan = 0;
        char *tok = strtok(argv[1], ",");
        while (tok && n_scan < 4) {
            scan_list[n_scan++] = atol(tok);
            tok = strtok(NULL, ",");
        }
    }
    const long brow_elems = 8192;

    int cap = (int)(sizeof(ws_list) / sizeof(ws_list[0])) *
              (int)(sizeof(frac_list) / sizeof(frac_list[0])) *
              n_scan * 4 + 16;
    PointOut *outs = (PointOut *)malloc((size_t)cap * sizeof(PointOut));
    int n_out = 0;

    for (size_t wi = 0; wi < sizeof(ws_list) / sizeof(ws_list[0]); wi++)
        for (size_t fi = 0; fi < sizeof(frac_list) / sizeof(frac_list[0]); fi++)
            for (int si = 0; si < n_scan; si++)
                for (int v = 0; v < 4; v++)
                    run_point(v, ws_list[wi], frac_list[fi], scan_list[si],
                              brow_elems, &outs[n_out++]);

    printf("{\n  \"points\": [\n");
    for (int i = 0; i < n_out; i++) {
        PointOut *o = &outs[i];
        printf("    {\"v\": \"%s\", \"ws\": %ld, \"frac\": %.2f, "
               "\"scan\": %ld, \"passes\": %ld, \"per_line\": %.4f, "
               "\"t_inplace\": %.4f, \"t_migrated\": %.4f, "
               "\"n_inplace\": %ld, \"n_migrated\": %ld}%s\n",
               vname(o->variant), o->ws, o->frac, o->scan_elems, o->passes,
               o->per_line, o->t_inplace, o->t_migrated,
               o->n_inplace, o->n_migrated,
               (i + 1 < n_out) ? "," : "");
    }
    printf("  ]\n}\n");
    free(outs);
    return 0;
}