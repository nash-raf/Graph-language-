/* Link against an unchanged program.o. Reject conversions during native scans. */
#include "autotuner_runtime.h"
#include <stdio.h>
#include <stdlib.h>

static struct { void *graph; AutoNeighborIter *iter; } active[256];
static int fixed_layout = -2;
static int audit_failed;
static long conversions, initializations;

void __real_autograph_ensure_layout(void *, int64_t, int64_t, int64_t *, int32_t *, void *, void *, void *, int32_t);
void __real_autograph_neighbor_iter_init(void *, int64_t, AutoNeighborIter *);
int32_t __real_autograph_neighbor_iter_next(AutoNeighborIter *, int32_t *);

static void fail(const char *message) {
  audit_failed = 1;
  fprintf(stderr, "ITERATOR AUDIT FAILED: %s\n", message);
  exit(71);
}

void __wrap_autograph_ensure_layout(void *g, int64_t n, int64_t m,
    int64_t *row, int32_t *col, void *nodes, void *edges, void *pairs, int32_t layout) {
  int old = autograph_get_layout(g);
  if (old != layout) {
    for (int i = 0; i < 256; ++i)
      if (active[i].graph == g) fail("layout changed with an active iterator");
    ++conversions;
    fprintf(stderr, "ITERATOR AUDIT layout %d -> %d\n", old, layout);
  }
  __real_autograph_ensure_layout(g, n, m, row, col, nodes, edges, pairs, layout);
}

void __wrap_autograph_neighbor_iter_init(void *g, int64_t u, AutoNeighborIter *iter) {
  if (fixed_layout == -2) {
    const char *s = getenv("AUDIT_FIXED_LAYOUT");
    fixed_layout = s ? atoi(s) : -1;
  }
  if (fixed_layout >= 0 && autograph_get_layout(g) != fixed_layout)
    fail("iterator did not use the fixed layout");
  for (int i = 0; i < 256; ++i)
    if (!active[i].graph || active[i].iter == iter) {
      active[i].graph = g;
      active[i].iter = iter;
      ++initializations;
      __real_autograph_neighbor_iter_init(g, u, iter);
      return;
    }
  fail("too many nested iterators");
}

int32_t __wrap_autograph_neighbor_iter_next(AutoNeighborIter *iter, int32_t *v) {
  AutoGraphMeta *meta = (AutoGraphMeta *)iter->meta;
  if (meta && (iter->layout != meta->current_layout || iter->epoch != meta->layout_epoch))
    fail("iterator state invalidated before next()");
  int32_t result = __real_autograph_neighbor_iter_next(iter, v);
  if (!result)
    for (int i = 0; i < 256; ++i)
      if (active[i].iter == iter) active[i].graph = NULL;
  return result;
}

__attribute__((destructor)) static void report(void) {
  if (!audit_failed)
    fprintf(stderr, "ITERATOR AUDIT passed initializations=%ld conversions=%ld\n", initializations, conversions);
}
