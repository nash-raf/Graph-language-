/* Minimal shim standing in for p1's parallel_runtime worker pool.
 *
 * autotuner_runtime.c (ported from p1GraphEasy-con-AutoTuner) needs exactly two
 * things from that 5000-line module: how many workers exist, and which one is
 * running. p1 answers those from its own persistent worker pool; p2-AT has no
 * such pool, so we answer from OpenMP, which is what its parallel regions
 * actually use. Pulling in the full module (and its nlopt dependency) for two
 * accessors would be disproportionate.
 *
 * Semantics required by the EdgeMap engine: worker_count >= 1, and
 * current_worker_index in [0, worker_count) so per-lane scratch indexing is
 * in range. Both hold for OpenMP inside and outside a parallel region.
 */
#ifndef SGPL_PARALLEL_RUNTIME_SHIM_H
#define SGPL_PARALLEL_RUNTIME_SHIM_H

#include <stdint.h>

#ifdef _OPENMP
/* Declared directly rather than via <omp.h>: this header is also fed to clang
 * when autotuner_runtime.c is lowered to autotuner_runtime.ll, and GCC's omp.h
 * uses attribute spellings clang rejects.  Both prototypes are ABI-fixed by the
 * OpenMP spec, so declaring them costs nothing and keeps the two compilers
 * agreeing on this header. */
extern int omp_get_max_threads(void);
extern int omp_get_thread_num(void);

static inline int32_t sgpl_configured_worker_count(void) {
    int32_t n = (int32_t)omp_get_max_threads();
    return n > 0 ? n : 1;
}
static inline int32_t sgpl_current_worker_index(void) {
    int32_t i = (int32_t)omp_get_thread_num();
    return i >= 0 ? i : 0;
}
#else
static inline int32_t sgpl_configured_worker_count(void) { return 1; }
static inline int32_t sgpl_current_worker_index(void) { return 0; }
#endif


/* Parallel loop driver.  p1 dispatches these onto a persistent, budgeted worker
 * pool with optional doacross ordering; every call the EdgeMap engine makes uses
 * step=1 and needs_doacross=0, so an OpenMP parallel-for is a faithful stand-in.
 * Thread numbers run 0..omp_get_num_threads()-1, which is exactly the range
 * sgpl_current_worker_index() reports, so per-lane scratch indexing stays valid.
 */
void parallel_for_runtime(int64_t start,
                          int64_t end,
                          int64_t step,
                          void (*body)(int64_t, void *),
                          void *env,
                          int32_t needs_doacross,
                          int32_t doacross_num_sync_ids);

#endif /* SGPL_PARALLEL_RUNTIME_SHIM_H */
