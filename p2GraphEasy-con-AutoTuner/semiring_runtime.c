/* Dense semiring-closure kernel behind the AST closure motif.
 *
 * IRGenVisitor recognizes a Kleene-closure triple nest written in ordinary
 * GraphEasy and rewrites it into one call to autograph_closure, tagged with
 * which semiring the loop body implemented.  This file is the engine those
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
 * Lives in its own gcc-compiled object rather than in autotuner_runtime.c:
 * that file is lowered to autotuner_runtime.ll by clang and linked into the
 * user module as IR, so an OpenMP pragma there would emit libomp (__kmpc_*)
 * calls into a program that links libgomp.  Keeping the pragmas here keeps
 * both halves on one OpenMP runtime.
 */
#include <stdint.h>
#include <stdlib.h>

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

/* Below this the O(n^3) work does not repay a fork/join, let alone blocking. */
#define SGPL_CLOSURE_PARALLEL_MIN 192

/* One relaxation, specialized per semiring so the comparison and the extension
 * both fold into the caller.  `mul` is the semiring MULTIPLY (path extension),
 * `better` its ADD expressed as a comparison (the ADD is idempotent, which is
 * what makes the blocked schedule legal). */
#define SGPL_RELAX(mul, better)                                                \
    do {                                                                       \
        int32_t cand = mul;                                                    \
        if (better)                                                            \
            row_i[j] = cand;                                                   \
    } while (0)

/* Rectangle update: i in [i0,i1), j in [j0,j1), k in [k0,k1).
 *
 * `parallel_i` says the caller has proved this rectangle's rows independent.
 * That holds only for phase 3: its reads (D[i][k] in the column panel, D[k][j]
 * in the row panel) lie outside its writes (the corner rectangle), so nothing
 * it reads is anything it writes.  When that is true the k loop can move
 * INSIDE i -- it becomes a plain reduction -- and the parallel region is
 * entered once for the whole rectangle instead of once per k.
 *
 * In the sequential form k must stay outermost: phases 1, 2a and 2b each read
 * cells they are also writing, so relaxation k must complete everywhere before
 * relaxation k+1 begins.
 */
#define SGPL_DEFINE_PHASE(NAME, MUL, BETTER)                                   \
    static void NAME(int32_t *D, int64_t n, int64_t i0, int64_t i1,            \
                     int64_t j0, int64_t j1, int64_t k0, int64_t k1,           \
                     int parallel_i)                                           \
    {                                                                          \
        if (parallel_i)                                                        \
        {                                                                      \
            int64_t i;                                                         \
            _Pragma("omp parallel for schedule(static)")                       \
            for (i = i0; i < i1; i++)                                          \
            {                                                                  \
                int32_t *row_i = D + i * n;                                    \
                for (int64_t k = k0; k < k1; k++)                              \
                {                                                              \
                    const int32_t *row_k = D + k * n;                          \
                    int32_t a = row_i[k];                                      \
                    for (int64_t j = j0; j < j1; j++)                          \
                        SGPL_RELAX(MUL, BETTER);                               \
                }                                                              \
            }                                                                  \
            return;                                                            \
        }                                                                      \
        for (int64_t k = k0; k < k1; k++)                                      \
        {                                                                      \
            const int32_t *row_k = D + k * n;                                  \
            for (int64_t i = i0; i < i1; i++)                                  \
            {                                                                  \
                int32_t *row_i = D + i * n;                                    \
                int32_t a = row_i[k];                                          \
                for (int64_t j = j0; j < j1; j++)                              \
                    SGPL_RELAX(MUL, BETTER);                                   \
            }                                                                  \
        }                                                                      \
    }

SGPL_DEFINE_PHASE(phase_min_plus,  a + row_k[j], cand < row_i[j])
SGPL_DEFINE_PHASE(phase_max_plus,  a + row_k[j], cand > row_i[j])
SGPL_DEFINE_PHASE(phase_max_times, a * row_k[j], cand > row_i[j])
SGPL_DEFINE_PHASE(phase_min_times, a * row_k[j], cand < row_i[j])

typedef void (*sgpl_phase_fn)(int32_t *, int64_t, int64_t, int64_t, int64_t,
                              int64_t, int64_t, int64_t, int);

static sgpl_phase_fn sgpl_phase_for(int32_t semiring)
{
    switch (semiring)
    {
    case SGPL_SEMIRING_MIN_PLUS:  return phase_min_plus;
    case SGPL_SEMIRING_MAX_PLUS:  return phase_max_plus;
    case SGPL_SEMIRING_MAX_TIMES: return phase_max_times;
    case SGPL_SEMIRING_MIN_TIMES: return phase_min_times;
    default:                      return NULL;
    }
}

/* Blocked closure, the 4-phase schedule (Venkataraman et al. 2003):
 *
 *   1.  close the diagonal block                     k sequential, B^3
 *   2a. row panel    i in block, j outside           k sequential over block
 *   2b. column panel i outside, j in block           k sequential over block
 *   3.  the four corner rectangles                   k an inner reduction,
 *                                                     i rows independent
 *
 * Phase 3 dominates ((n-B)^2 * B per round) and is the only phase run in
 * parallel: its writes are disjoint from the panels phases 2a/2b just wrote,
 * so no two threads touch the same cell.
 */
static void closure_blocked(int32_t *D, int64_t n, sgpl_phase_fn phase)
{
    for (int64_t lo = 0; lo < n; lo += SGPL_CLOSURE_BLOCK)
    {
        int64_t hi = lo + SGPL_CLOSURE_BLOCK;
        if (hi > n)
            hi = n;

        phase(D, n, lo, hi, lo, hi, lo, hi, 0);          /* 1  */
        phase(D, n, lo, hi, 0,  lo, lo, hi, 0);          /* 2a left  */
        phase(D, n, lo, hi, hi, n,  lo, hi, 0);          /* 2a right */
        phase(D, n, 0,  lo, lo, hi, lo, hi, 0);          /* 2b above */
        phase(D, n, hi, n,  lo, hi, lo, hi, 0);          /* 2b below */

        phase(D, n, 0,  lo, 0,  lo, lo, hi, 1);          /* 3 corners */
        phase(D, n, 0,  lo, hi, n,  lo, hi, 1);
        phase(D, n, hi, n,  0,  lo, lo, hi, 1);
        phase(D, n, hi, n,  hi, n,  lo, hi, 1);
    }
}

/* Entry point emitted by IRGenVisitor.
 *
 * Returns 0 if the request was not something this engine handles, in which case
 * the compiler's fallback loop has already run -- see the guard it emits. */
int32_t autograph_closure(int32_t *D, int32_t n_in, int32_t semiring)
{
    sgpl_phase_fn phase = sgpl_phase_for(semiring);
    int64_t n = (int64_t)n_in;
    if (!D || n <= 0 || !phase)
        return 0;

    if (n < SGPL_CLOSURE_PARALLEL_MIN)
    {
        /* small: plain triple loop, no blocking, no threads */
        phase(D, n, 0, n, 0, n, 0, n, 0);
        return 1;
    }

    closure_blocked(D, n, phase);
    return 1;
}
