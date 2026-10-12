/* Link with --wrap for the two functions below. Bound every rendezvous.
 * The probe checks dispatch overlap, source joins, and shared-graph scans. */
#include "autotuner_runtime.h"
#include "parallel_runtime.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  sgpl_dag_node_fn fn;
  void *state;
  atomic_int arrived, active, peak;
  int rendezvous;
} probe;
static atomic_int dispatch_active, dispatch_count, peak_callbacks, peak_scans;
static atomic_int active_scans;
static _Thread_local int in_component;

static void maximum(atomic_int *slot, int value) {
  int old = atomic_load(slot);
  while (old < value && !atomic_compare_exchange_weak(slot, &old, value)) {}
}
static double seconds(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec + 1e-9 * t.tv_nsec;
}
static void fail(const char *reason) {
  fprintf(stderr, "temporal probe failed: %s\n", reason);
  exit(90);
}
static int32_t component(void *state, int64_t id) {
  probe *p = state;
  int active = atomic_fetch_add(&p->active, 1) + 1;
  maximum(&p->peak, active);
  atomic_fetch_add(&p->arrived, 1);
  double deadline = seconds() + 3.0;
  while (atomic_load(&p->arrived) < p->rendezvous) {
    if (seconds() > deadline) fail("independent callbacks did not overlap");
    usleep(100);
  }
  in_component = 1;
  int32_t rc = p->fn(p->state, id);
  in_component = 0;
  atomic_fetch_sub(&p->active, 1);
  return rc;
}
int32_t __real_sgpl_exec_temporal_components(sgpl_dag_node_fn, void *, int32_t, int32_t);
int32_t __wrap_sgpl_exec_temporal_components(sgpl_dag_node_fn fn, void *state,
                                             int32_t count, int32_t budget) {
  if (atomic_exchange(&dispatch_active, 1)) fail("next source started before join");
  const char *live = getenv("SGPL_TEST_LIVE_COMPONENTS");
  if (live) {
    int source = atomic_load(&dispatch_count);
    int divisor = atoi(live);
    if (count != (source % divisor == 0 ? 3 : 2))
      fail("empty or guarded Pair was counted as active");
  }
  probe p = {.fn = fn, .state = state, .rendezvous = 1};
  if (count > 1 && budget > 1) p.rendezvous = 2;
  int32_t rc = __real_sgpl_exec_temporal_components(component, &p, count, budget);
  if (atomic_load(&p.active)) fail("dispatch returned before join");
  maximum(&peak_callbacks, atomic_load(&p.peak));
  atomic_fetch_add(&dispatch_count, 1);
  atomic_store(&dispatch_active, 0);
  return rc;
}
int __real_autograph_neighbor_iter_next(AutoNeighborIter *, int32_t *);
int __wrap_autograph_neighbor_iter_next(AutoNeighborIter *iter, int32_t *v) {
  if (!in_component || !getenv("SGPL_TEST_ROUNDS"))
    return __real_autograph_neighbor_iter_next(iter, v);
  maximum(&peak_scans, atomic_fetch_add(&active_scans, 1) + 1);
  usleep(1000);
  int rc = __real_autograph_neighbor_iter_next(iter, v);
  atomic_fetch_sub(&active_scans, 1);
  return rc;
}
__attribute__((destructor)) static void report(void) {
  const char *expected = getenv("SGPL_TEST_DISPATCHES");
  if (expected && atomic_load(&dispatch_count) != atoi(expected)) fail("wrong dispatch count");
  if (expected && sgpl_configured_worker_count() > 1 && atomic_load(&peak_callbacks) < 2)
    fail("no component overlap");
  if (sgpl_configured_worker_count() == 1 && atomic_load(&peak_callbacks) > 1)
    fail("one worker overlapped component callbacks");
  if (getenv("SGPL_TEST_ROUNDS") && sgpl_configured_worker_count() > 1 && atomic_load(&peak_scans) < 2)
    fail("rounds did not scan the same graph concurrently");
  fprintf(stderr, "[temporal-probe] dispatches=%d callbacks=%d scans=%d\n",
          atomic_load(&dispatch_count), atomic_load(&peak_callbacks), atomic_load(&peak_scans));
}
