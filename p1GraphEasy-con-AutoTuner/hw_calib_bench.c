/*
 * hw_calib_bench.c — Offline hardware calibration for the autotuner cost model.
 *
 * Measures hardware parameters and writes them as JSON to stdout:
 *   L   : cache-line size in bytes (from sysconf, fallback 64)
 *   t   : random-access (pointer-chasing) latency, in nanoseconds
 *   T   : sequential cache-line transfer cost, in nanoseconds per cache line
 *   LLC : last-level-cache capacity in bytes (sysfs / sysconf, fallback 8MB)
 *   Tm  : per-line cost of a cache-resident overlapping memmove (read +
 *         write per line — the CSR/BCSR insert shift pattern), in ns.
 *         Distinct from T, which is a read-only prefetched stream.
 *   P   : DRAM-bound memmove penalty — per-line cost ratio of an overlapping
 *         memmove whose working set is 4x the LLC vs one that fits in it
 *   ramp_lo : working-set size (bytes) at which the memmove penalty FIRST
 *         leaves the cache-resident regime (per-line cost within 5% of Tm).
 *         Measured by sweeping working sets across the LLC.  The cost model
 *         ramps log-linearly between ramp_lo and ramp_hi; a machine with a
 *         sharp cache cliff yields a tight band, a gradual one yields a wide
 *         band.  Replaces the old hardcoded LLC/2 lower bound.
 *   ramp_hi : working-set size (bytes) at which the memmove penalty
 *         saturates at the DRAM rate (per-line cost within 5% of P*Tm).
 *         Replaces the old hardcoded 2*LLC upper bound.
 *   h   : ns per edge_hash_insert during a bulk hash build (replica of the
 *         open-addressing linear-probe EdgeHashMap in autotuner_runtime.c).
 *         Reported as the legacy scalar for backward compatibility, measured
 *         at 1M pairs so the table is DRAM-resident — the regime where the
 *         SET setup cost actually matters.
 *   h_cache : ns per edge_hash_insert when the hash table fits in the LLC
 *         (measured at ~LLC/4 table bytes).  Together with h_dram this lets
 *         the cost model pick h by the actual table size vs LLC, the same
 *         pattern as Tm/P for memmove.
 *   h_dram  : ns per edge_hash_insert when the table is DRAM-resident
 *         (measured at ~4*LLC table bytes).  Equals the legacy h on
 *         DRAM-heavy machines; the model ramps log-linearly between
 *         h_cache and h_dram across the LLC/2 .. 2*LLC band.
 *   c   : ns per sequential-id bitmap contains check (replica of the
 *         find_container + bitset-test path in roaring_bitmap.cpp; static
 *         edge ids are dense, so containers are full bitsets)
 *
 * Usage:
 *   ./hw_calib_bench > ~/.config/sgpl/hw_calib.json
 *
 * The autotuner pass and cost_model.py read this JSON via $AUTOTUNER_HW_CALIB
 * (or the default path above).  Missing keys fall back to conservative
 * defaults, so older JSON files keep working.
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
#include <math.h>

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

/* ── LLC size detection ──────────────────────────────────────────────── */

/* Largest cache reported under /sys/devices/system/cpu/cpu0/cache/.
 * Falls back to sysconf(_SC_LEVEL3_CACHE_SIZE), then 8MB. */
static long detect_llc_bytes(void) {
    long best = 0;
    char path[128], buf[64];
    for (int idx = 0; idx < 10; idx++) {
        snprintf(path, sizeof path,
                 "/sys/devices/system/cpu/cpu0/cache/index%d/size", idx);
        FILE *f = fopen(path, "r");
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
#ifdef _SC_LEVEL3_CACHE_SIZE
    if (best <= 0) best = (long)sysconf(_SC_LEVEL3_CACHE_SIZE);
#endif
    if (best <= 0) best = 8L * 1024 * 1024;
    return best;
}

/* ── DRAM-bound memmove penalty P ────────────────────────────────────── */

/* Per-line cost of an overlapping 8-byte shift (memmove(buf+8, buf, ws)) —
 * exactly the access pattern of the CSR/BCSR insert shift. */
static double measure_memmove_per_line(long ws, long line, int passes) {
    char *buf = (char *)malloc((size_t)ws + 64);
    if (!buf) return 0.0;
    memset(buf, 1, (size_t)ws + 64);
    memmove(buf + 8, buf, (size_t)ws); /* warm-up: page-fault + cache fill */
    double t0 = now_ns();
    for (int p = 0; p < passes; p++)
        memmove(buf + 8, buf, (size_t)ws);
    double t1 = now_ns();
    free(buf);
    return (t1 - t0) / ((double)passes * ((double)ws / (double)line));
}

/* Cache-resident per-line memmove cost Tm and the DRAM penalty ratio P. */
static void measure_Tm_P(long llc, long line, double *out_Tm, double *out_P) {
    long small = llc / 4;
    if (small < (1L << 20)) small = 1L << 20;
    long big = llc * 4;
    double cache_ns = measure_memmove_per_line(small, line, 16);
    double dram_ns = measure_memmove_per_line(big, line, 4);
    *out_Tm = (cache_ns > 0.0) ? cache_ns : 2.5;
    if (cache_ns <= 0.0 || dram_ns <= 0.0) {
        *out_P = 4.0;
        return;
    }
    double P = dram_ns / cache_ns;
    *out_P = (P < 1.0) ? 1.0 : P;
}

/* ── Measured ramp band [ramp_lo, ramp_hi] ────────────────────────────── */

/* Fine log-spaced sweep of memmove per-line cost across working-set sizes.
 * Emitted as the raw curve (ws_bytes, per_line_ns) so cost_model.py can
 * interpolate the penalty from actual measurements instead of assuming a
 * log-linear ramp between two band edges.  Log-spaced because the transition
 * is spread over decades of working-set size (L2 → LLC → DRAM). */
#define CURVE_MIN_MULT 0.03125
#define CURVE_MAX_MULT 16.0
#define CURVE_POINTS   64

static void measure_memmove_curve(long llc, long line,
                                  long *out_ws, double *out_per,
                                  int *out_n) {
    double lo = log(CURVE_MIN_MULT * (double)llc);
    double hi = log(CURVE_MAX_MULT * (double)llc);
    int n = 0;
    for (int i = 0; i < CURVE_POINTS; i++) {
        double ws = exp(lo + (hi - lo) * (double)i / (double)(CURVE_POINTS - 1));
        long wsb = (long)(ws + 0.5);
        if (wsb < line * 8) wsb = line * 8;
        int passes = (wsb < llc / 4) ? 64 : (wsb < llc) ? 32
                   : (wsb < 4 * llc) ? 8 : 4;
        double per = measure_memmove_per_line(wsb, line, passes);
        if (per <= 0.0) continue;
        out_ws[n] = wsb;
        out_per[n] = per;
        n++;
    }
    *out_n = n;
}

/* Sweep memmove per-line cost across working-set sizes spanning the LLC and
 * derive the band where the penalty transitions from cache-resident (1.0) to
 * DRAM-saturated (P).  The cost model ramps log-linearly between ramp_lo and
 * ramp_hi; these are MEASURED band edges (in bytes), not the hardcoded
 * LLC/2 .. 2*LLC guess the model used before.
 *
 *   ramp_lo = largest working set whose per-line cost is still within 5% of
 *             the cache-resident Tm (penalty <= 1.05) — the last point that
 *             is effectively still in-cache.
 *   ramp_hi = smallest working set whose per-line cost is within 5% of the
 *             DRAM-saturated rate (penalty >= 0.95*P) — the first point that
 *             is effectively fully DRAM-bound.
 *
 * Both are read directly off the measured curve, so a machine with a sharp
 * cliff yields a tight band (e.g. 0.75*LLC .. 1.25*LLC) and a machine with a
 * gradual transition yields a wide one (e.g. LLC/2 .. 2*LLC).  No graph-data
 * is consulted. */
static void measure_ramp_bounds(long llc, long line, double Tm, double P,
                                long *out_lo, long *out_hi) {
    /* Fallbacks if the machine shows no real transition (e.g. huge LLC, a
     * noisy loaded machine where P is depressed below ~1.5x, or a
     * measurement failure): keep the original conservative band. */
    if (Tm <= 0.0 || P <= 1.5) {
        *out_lo = llc / 2;
        *out_hi = llc * 2;
        return;
    }
    const double mults[] = {0.125, 0.25, 0.375, 0.5, 0.75, 1.0,
                            1.125, 1.25, 1.375, 1.5, 1.625, 1.75,
                            1.875, 2.0, 2.5, 3.0, 4.0};
    const int n = (int)(sizeof(mults) / sizeof(mults[0]));
    long lo = llc / 2;   /* last cache-resident ws seen */
    long hi = llc * 2;   /* first saturated ws seen */
    int found_hi = 0;
    for (int i = 0; i < n; i++) {
        long ws = (long)(mults[i] * (double)llc);
        if (ws < line * 4) ws = line * 4;
        /* more passes for small (fast) working sets, fewer for big ones */
        int passes = (ws < llc) ? 32 : (ws < 2 * llc) ? 8 : 4;
        double per = measure_memmove_per_line(ws, line, passes);
        if (per <= 0.0) continue;
        double pen = per / Tm;
        if (pen <= 1.05)
            lo = ws; /* still essentially cache-resident */
        if (pen >= 0.95 * P && !found_hi) {
            hi = ws; /* effectively saturated */
            found_hi = 1;
        }
    }
    if (!found_hi)
        hi = (long)(4.0 * (double)llc); /* never saturated in range; use last */
    if (hi <= lo)
        hi = lo + line; /* sharp cliff: keep at least one cache line of ramp */
    *out_lo = lo;
    *out_hi = hi;
}

/* ── Hash-insert cost h ──────────────────────────────────────────────── */

/* Replica of EdgeHashMap in autotuner_runtime.c: open addressing, linear
 * probing, 24-byte entries, capacity = next_pow2(2*expected + 1). */
typedef struct {
    uint64_t key;
    int64_t value;
    uint8_t occupied;
} CalibHashEntry;

static uint64_t mix_u64(uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

static double measure_h(long n_pairs) {
    long cap = 1;
    while (cap < 2 * n_pairs + 1) cap <<= 1;
    CalibHashEntry *table =
        (CalibHashEntry *)calloc((size_t)cap, sizeof(CalibHashEntry));
    if (!table) return 0.0;

    /* Pseudo-random (u, v) pairs via a fixed-seed LCG. */
    uint64_t state = 0x9e3779b97f4a7c15ULL;
    double t0 = now_ns();
    for (long i = 0; i < n_pairs; i++) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        uint32_t u = (uint32_t)(state >> 33);
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        uint32_t v = (uint32_t)(state >> 33);
        uint32_t lo = u < v ? u : v;
        uint32_t hi = u < v ? v : u;
        uint64_t k = ((uint64_t)lo << 32) | (uint64_t)hi;
        uint64_t idx = mix_u64(k) & (uint64_t)(cap - 1);
        while (table[idx].occupied) {
            if (table[idx].key == k) break;
            idx = (idx + 1) & (uint64_t)(cap - 1);
        }
        table[idx].key = k;
        table[idx].value = i;
        table[idx].occupied = 1;
    }
    double t1 = now_ns();
    free(table);
    return (t1 - t0) / (double)n_pairs;
}

/* ── Sequential-id bitmap contains cost c ────────────────────────────── */

/* Replica of the canonical_edge_count scan: roaring_bitmap_contains for
 * ids 0..N-1 against fully-populated bitset containers — binary search over
 * container keys + bitset word test.  noinline to include the function-call
 * overhead the real roaring_bitmap_contains pays. */
typedef struct {
    uint16_t key;
    uint64_t *bits; /* 65536 bits = 8KB */
} CalibContainer;

__attribute__((noinline))
static int calib_contains(const CalibContainer *conts, long k, uint32_t value) {
    uint16_t high = (uint16_t)(value >> 16);
    uint16_t low = (uint16_t)(value & 0xFFFF);
    long lo = 0, hi = k - 1;
    const CalibContainer *found = NULL;
    while (lo <= hi) {
        long mid = (lo + hi) / 2;
        if (conts[mid].key == high) { found = &conts[mid]; break; }
        if (conts[mid].key < high) lo = mid + 1;
        else hi = mid - 1;
    }
    if (!found) return 0;
    return (int)((found->bits[low >> 6] >> (low & 63)) & 1ULL);
}

static double measure_c(long n_ids) {
    long k = (n_ids + 65535) / 65536;
    if (k < 1) k = 1;
    CalibContainer *conts =
        (CalibContainer *)malloc((size_t)k * sizeof(CalibContainer));
    if (!conts) return 0.0;
    for (long i = 0; i < k; i++) {
        conts[i].key = (uint16_t)i;
        conts[i].bits = (uint64_t *)malloc(8192);
        if (!conts[i].bits) return 0.0;
        memset(conts[i].bits, 0xFF, 8192);
    }

    volatile long count = 0;
    double t0 = now_ns();
    for (long e = 0; e < n_ids; e++)
        count += calib_contains(conts, k, (uint32_t)e);
    double t1 = now_ns();

    for (long i = 0; i < k; i++) free(conts[i].bits);
    free(conts);
    return (t1 - t0) / (double)n_ids;
}

int main(void) {
    long L = (long)sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (L <= 0) L = 64;

    /* Use a moderate working set for pointer-chasing (~256K elements = 1MB
     * for uint32_t) to exceed L2 on typical machines. */
    double t = measure_t(256 * 1024);

    /* Stream ~16MB to measure cache-line transfer cost. */
    double T = measure_T(16 * 1024 * 1024, L);

    long llc = detect_llc_bytes();
    double Tm = 0.0, P = 0.0;
    measure_Tm_P(llc, L, &Tm, &P);

    /* Measured ramp band: sweep memmove cost across working-set sizes
     * spanning the LLC and read off the cache-resident -> DRAM-saturated
     * transition edges.  The cost model uses these (ramp_lo, ramp_hi) instead
     * of the old hardcoded LLC/2 .. 2*LLC guess.  Derived from the same
     * memmove measurements as Tm and P — no graph data consulted. */
    long ramp_lo = 0, ramp_hi = 0;
    measure_ramp_bounds(llc, L, Tm, P, &ramp_lo, &ramp_hi);

    /* Fine-grained memmove curve: raw (ws_bytes, per_line_ns) anchors so
     * cost_model.py can interpolate the true penalty instead of the
     * log-linear band approximation above. */
    long curve[64];
    double curve_per[64];
    int n_curve = 0;
    measure_memmove_curve(llc, L, curve, curve_per, &n_curve);

    /* h is size-aware: measure at two table sizes — one cache-resident
     * (~LLC/4 table bytes => ~LLC/(4*24) pairs) and one DRAM-resident
     * (~4*LLC table bytes => ~4*LLC/24 pairs).  The cost model ramps
     * log-linearly between h_cache and h_dram across the [ramp_lo, ramp_hi]
     * band, mirroring the Tm/P ramp for memmove.  We also keep the legacy
     * scalar h (= the DRAM measurement) so older JSON consumers keep working. */
    long h_cache_pairs = (long)(llc / (4.0 * 24.0));
    if (h_cache_pairs < 1024) h_cache_pairs = 1024;
    long h_dram_pairs  = (long)(4.0 * llc / 24.0);
    if (h_dram_pairs < h_cache_pairs * 8) h_dram_pairs = h_cache_pairs * 8;
    double h_cache = measure_h(h_cache_pairs);
    double h_dram  = measure_h(h_dram_pairs);
    double h = h_dram; /* legacy scalar == DRAM rate */

    double c = measure_c(1000 * 1000);

    printf("{ \"L\": %ld, \"t\": %.4f, \"T\": %.4f, \"LLC\": %ld, "
           "\"Tm\": %.4f, \"P\": %.4f, \"ramp_lo\": %ld, \"ramp_hi\": %ld, "
           "\"h\": %.4f, \"h_cache\": %.4f, \"h_dram\": %.4f, \"c\": %.4f, "
           "\"memmove_curve\": [",
           L, t, T, llc, Tm, P, ramp_lo, ramp_hi,
           h, h_cache, h_dram, c);
    for (int i = 0; i < n_curve; i++) {
        printf("%s[%ld, %.4f]", i ? ", " : "",
               curve[i], curve_per[i]);
    }
    printf("] }\n");
    return 0;
}
