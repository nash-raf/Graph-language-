/* Out-of-line half of the parallel_runtime shim.  See parallel_runtime.h. */
#include "parallel_runtime.h"

void parallel_for_runtime(int64_t start,
                          int64_t end,
                          int64_t step,
                          void (*body)(int64_t, void *),
                          void *env,
                          int32_t needs_doacross,
                          int32_t doacross_num_sync_ids)
{
    (void)needs_doacross;          /* the EdgeMap engine never requests ordering */
    (void)doacross_num_sync_ids;
    if (step == 0 || !body)
        return;

#ifdef _OPENMP
    /* Only the unit-stride ascending case is worth handing to OpenMP; the engine
     * emits nothing else.  Anything unusual falls through to the serial loop
     * below rather than risking a mis-split iteration space. */
    /* p1 dispatches onto a persistent pool and declines to spread a loop whose
     * trip count is smaller than the thread count.  An OpenMP parallel region
     * costs a fork/join every call, and the EdgeMap engine calls this several
     * times per frontier round, so a small round must stay serial or the
     * overhead swamps the work it is meant to spread. */
    if (step == 1 && end - start >= 4096 && omp_get_max_threads() > 1)
    {
        int64_t i;
#pragma omp parallel for schedule(static)
        for (i = start; i < end; i++)
            body(i, env);
        return;
    }
#endif

    if (step > 0)
        for (int64_t i = start; i < end; i += step)
            body(i, env);
    else
        for (int64_t i = start; i > end; i += step)
            body(i, env);
}
