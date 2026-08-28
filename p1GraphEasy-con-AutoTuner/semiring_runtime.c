/* Dense semiring-closure kernel behind the AST closure motif.
 *
 * IRGenVisitor recognizes a Kleene-closure triple nest written in ordinary
 * GraphEasy and rewrites it into one call to autograph_closure, tagged with
 * which closed semiring the body implemented.  This file is the engine those
 * calls land in -- the dense counterpart of autograph_edgemap.
 *
 * Why a runtime kernel rather than leaving the loops to the optimizer: the
 * blocked schedule below is a SOURCE-LEVEL restructuring whose legality rests
 * on the semiring's ADD being idempotent (min(a,a)=a, max(a,a)=a), which lets
 * phase 3 re-relax cells that are already final.  Dependence analysis on the
 * lowered IR cannot know that, so a polyhedral optimizer must serialize the
 * k loop.  Knowing the semiring at the AST level is exactly what buys the
 * freedom to reassociate.
 *
 * Engine strategy (win by being faster, not by declining):
 *   1. MaxTimes on a 0/1 matrix → bit-parallel Warshall (64-wide word OR).
 *      That is the boolean transitive-closure algorithm, not cubic FW over
 *      int32 multiplies — typically 10–50× vs a Polly nest on the same D.
 *   2. Otherwise → tiled FW (Venkataraman) with ONE OpenMP team for the
 *      whole n-loop.  Older builds forked a team per corner rectangle and
 *      lost badly to fork/join until n≈768; a persistent team scales with
 *      threads from much smaller n.
 *
 * Lives in its own gcc-compiled object rather than in autotuner_runtime.c:
 * that file is lowered to autotuner_runtime.ll by clang and linked into the
 * user module as IR, so an OpenMP pragma there would emit libomp (__kmpc_*)
 * calls into a program that links libgomp.  Keeping the pragmas here keeps
 * both halves on one OpenMP runtime.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Must agree with SemiringKind in IRGenVisitor.cpp. */
#define SGPL_SEMIRING_MIN_PLUS 0
#define SGPL_SEMIRING_MAX_PLUS 1
#define SGPL_SEMIRING_MAX_TIMES 2
#define SGPL_SEMIRING_MIN_TIMES 3

/* Block edge for the tiled schedule.  64 int32 columns = 256 B, so a block row
 * spans four cache lines and a 64x64 tile is 16 KiB -- three of them (the
 * diagonal, the row panel and the column panel a phase touches) sit inside a
 * typical 32-48 KiB L1. */
#define SGPL_CLOSURE_BLOCK 64

/* Blocked FW only pays for a thread team once cubic work is large; below this
 * stay serial-blocked (still better cache than a naïve nest at mid n). */
#define SGPL_CLOSURE_PARALLEL_MIN 512

#define SGPL_RELAX(mul, better)                                                \
    do {                                                                       \
        int32_t cand = mul;                                                    \
        if (better)                                                            \
            row_i[j] = cand;                                                   \
    } while (0)

/* Serial rectangle update (k outermost — required when the rectangle overlaps
 * its own reads/writes). */
#define SGPL_DEFINE_PHASE_SERIAL(NAME, MUL, BETTER)                            \
    static void NAME(int32_t *D, int64_t n, int64_t i0, int64_t i1,            \
                     int64_t j0, int64_t j1, int64_t k0, int64_t k1)           \
    {                                                                          \
        for (int64_t k = k0; k < k1; k++)                                      \
        {                                                                      \
            const int32_t *row_k = D + k * n;                                  \
            for (int64_t i = i0; i < i1; i++)                                  \
            {                                                                  \
                int32_t *row_i = D + i * n;                                    \
                int32_t a = row_i[k];                                          \
                int64_t j;                                                     \
                _Pragma("omp simd")                                            \
                for (j = j0; j < j1; j++)                                      \
                    SGPL_RELAX(MUL, BETTER);                                   \
            }                                                                  \
        }                                                                      \
    }

/* Phase-3 style: rows independent, k is an inner reduction.  Designed to run
 * INSIDE an already-open OpenMP parallel region (`omp for` / no new team). */
#define SGPL_DEFINE_PHASE_PARROWS(NAME, MUL, BETTER)                           \
    static void NAME(int32_t *D, int64_t n, int64_t i0, int64_t i1,            \
                     int64_t j0, int64_t j1, int64_t k0, int64_t k1)           \
    {                                                                          \
        int64_t i;                                                             \
        _Pragma("omp for schedule(static) nowait")                             \
        for (i = i0; i < i1; i++)                                              \
        {                                                                      \
            int32_t *row_i = D + i * n;                                        \
            for (int64_t k = k0; k < k1; k++)                                  \
            {                                                                  \
                const int32_t *row_k = D + k * n;                              \
                int32_t a = row_i[k];                                          \
                for (int64_t j = j0; j < j1; j++)                              \
                    SGPL_RELAX(MUL, BETTER);                                   \
            }                                                                  \
        }                                                                      \
    }

SGPL_DEFINE_PHASE_SERIAL(serial_min_plus,  a + row_k[j], cand < row_i[j])
SGPL_DEFINE_PHASE_SERIAL(serial_max_plus,  a + row_k[j], cand > row_i[j])
SGPL_DEFINE_PHASE_SERIAL(serial_max_times, a * row_k[j], cand > row_i[j])
SGPL_DEFINE_PHASE_SERIAL(serial_min_times, a * row_k[j], cand < row_i[j])

SGPL_DEFINE_PHASE_PARROWS(parrows_min_plus,  a + row_k[j], cand < row_i[j])
SGPL_DEFINE_PHASE_PARROWS(parrows_max_plus,  a + row_k[j], cand > row_i[j])
SGPL_DEFINE_PHASE_PARROWS(parrows_max_times, a * row_k[j], cand > row_i[j])
SGPL_DEFINE_PHASE_PARROWS(parrows_min_times, a * row_k[j], cand < row_i[j])

typedef void (*sgpl_rect_fn)(int32_t *, int64_t, int64_t, int64_t, int64_t,
                             int64_t, int64_t, int64_t);

static void sgpl_pick_fns(int32_t semiring, sgpl_rect_fn *serial,
                          sgpl_rect_fn *parrows)
{
    switch (semiring)
    {
    case SGPL_SEMIRING_MIN_PLUS:
        *serial = serial_min_plus;
        *parrows = parrows_min_plus;
        break;
    case SGPL_SEMIRING_MAX_PLUS:
        *serial = serial_max_plus;
        *parrows = parrows_max_plus;
        break;
    case SGPL_SEMIRING_MAX_TIMES:
        *serial = serial_max_times;
        *parrows = parrows_max_times;
        break;
    case SGPL_SEMIRING_MIN_TIMES:
        *serial = serial_min_times;
        *parrows = parrows_min_times;
        break;
    default:
        *serial = NULL;
        *parrows = NULL;
        break;
    }
}

/* ── Bit-parallel boolean transitive closure (MaxTimes on {0,1}) ─────────
 *
 * (max,×) on a 0/1 adjacency matrix is OR-AND: reachability.  Packing each
 * row into uint64_t words turns the inner update into a single OR of a
 * whole word — ~64× less work than int32 FW, and the k-loop body becomes
 * "if bit k set in row i, row_i |= row_k".
 */
static int matrix_is_boolean_01(const int32_t *D, int64_t n)
{
    const int64_t cells = n * n;
    /* Full scan is O(n²) vs O(n³) closure — cheap insurance. */
    for (int64_t i = 0; i < cells; i++)
    {
        int32_t v = D[i];
        if (v != 0 && v != 1)
            return 0;
    }
    return 1;
}

static int closure_bitparallel_reachability(int32_t *D, int64_t n)
{
    const int64_t words = (n + 63) / 64;
    uint64_t *bits = (uint64_t *)calloc((size_t)n * (size_t)words, sizeof(uint64_t));
    if (!bits)
        return 0;

    for (int64_t i = 0; i < n; i++)
    {
        uint64_t *row = bits + i * words;
        const int32_t *src = D + i * n;
        for (int64_t j = 0; j < n; j++)
            if (src[j])
                row[j >> 6] |= (uint64_t)1 << (j & 63);
        /* reflexive — matches demo kernels that set D[i][i]=1 */
        row[i >> 6] |= (uint64_t)1 << (i & 63);
    }

    /* Serial bit-Warshall on purpose.  Each k-step is a few KB of ORs; an
     * OpenMP barrier per k costs more than the work until n is enormous
     * (measured: 4 threads turned 0.2 ms → 500 ms at n=192).  Word-parallel
     * OR already gives ~64× vs int32 FW; threading rarely adds more. */
    for (int64_t k = 0; k < n; k++)
    {
        const uint64_t *row_k = bits + k * words;
        const int64_t kw = k >> 6;
        const uint64_t kb = (uint64_t)1 << (k & 63);
        for (int64_t i = 0; i < n; i++)
        {
            uint64_t *row_i = bits + i * words;
            if (row_i[kw] & kb)
            {
                for (int64_t w = 0; w < words; w++)
                    row_i[w] |= row_k[w];
            }
        }
    }

    for (int64_t i = 0; i < n; i++)
    {
        const uint64_t *row = bits + i * words;
        int32_t *dst = D + i * n;
        for (int64_t j = 0; j < n; j++)
            dst[j] = (row[j >> 6] >> (j & 63)) & 1u ? 1 : 0;
    }
    free(bits);
    return 1;
}

/* Blocked FW with a single OpenMP team for all of phase 3 across every
 * diagonal block.  Phases 1/2 stay sequential (dependences); phase 3 rows
 * are independent and use `omp for` inside the open region. */
static void closure_blocked(int32_t *D, int64_t n, sgpl_rect_fn serial,
                            sgpl_rect_fn parrows, int use_threads)
{
    if (!use_threads)
    {
        for (int64_t lo = 0; lo < n; lo += SGPL_CLOSURE_BLOCK)
        {
            int64_t hi = lo + SGPL_CLOSURE_BLOCK;
            if (hi > n)
                hi = n;
            serial(D, n, lo, hi, lo, hi, lo, hi);
            serial(D, n, lo, hi, 0, lo, lo, hi);
            serial(D, n, lo, hi, hi, n, lo, hi);
            serial(D, n, 0, lo, lo, hi, lo, hi);
            serial(D, n, hi, n, lo, hi, lo, hi);
            serial(D, n, 0, lo, 0, lo, lo, hi);
            serial(D, n, 0, lo, hi, n, lo, hi);
            serial(D, n, hi, n, 0, lo, lo, hi);
            serial(D, n, hi, n, hi, n, lo, hi);
        }
        return;
    }

#ifdef _OPENMP
#pragma omp parallel
#endif
    {
        for (int64_t lo = 0; lo < n; lo += SGPL_CLOSURE_BLOCK)
        {
            int64_t hi = lo + SGPL_CLOSURE_BLOCK;
            if (hi > n)
                hi = n;

            /* One thread runs the dependent diagonal + panel updates. */
#ifdef _OPENMP
#pragma omp single
#endif
            {
                serial(D, n, lo, hi, lo, hi, lo, hi);
                serial(D, n, lo, hi, 0, lo, lo, hi);
                serial(D, n, lo, hi, hi, n, lo, hi);
                serial(D, n, 0, lo, lo, hi, lo, hi);
                serial(D, n, hi, n, lo, hi, lo, hi);
            }
#ifdef _OPENMP
#pragma omp barrier
#endif
            /* Four corner rectangles — row-parallel, same team, no re-fork. */
            parrows(D, n, 0, lo, 0, lo, lo, hi);
            parrows(D, n, 0, lo, hi, n, lo, hi);
            parrows(D, n, hi, n, 0, lo, lo, hi);
            parrows(D, n, hi, n, hi, n, lo, hi);
#ifdef _OPENMP
#pragma omp barrier
#endif
        }
    }
}

/* Entry point emitted by IRGenVisitor.
 *
 * Returns 0 only on invalid args.  Always computes the closure when args are
 * valid (TDG may have outlined away any IR fallback nest). */
int32_t autograph_closure(int32_t *D, int32_t n_in, int32_t semiring)
{
    int64_t n = (int64_t)n_in;
    if (!D || n <= 0)
        return 0;

    /* Boolean reachability: bit-parallel TC beats int32 (max,×) FW by a lot. */
    if (semiring == SGPL_SEMIRING_MAX_TIMES && matrix_is_boolean_01(D, n))
    {
        if (closure_bitparallel_reachability(D, n))
            return 1;
        /* OOM fallback → general path */
    }

    sgpl_rect_fn serial = NULL, parrows = NULL;
    sgpl_pick_fns(semiring, &serial, &parrows);
    if (!serial || !parrows)
        return 0;

    int threads = 1;
#ifdef _OPENMP
    threads = omp_get_max_threads();
#endif
    int use_threads = (n >= SGPL_CLOSURE_PARALLEL_MIN && threads > 1);

    /* Tiny n: plain triple (serial fn over full extent) — lowest overhead. */
    if (n < 48)
    {
        serial(D, n, 0, n, 0, n, 0, n);
        return 1;
    }

    closure_blocked(D, n, serial, parrows, use_threads);
    return 1;
}
