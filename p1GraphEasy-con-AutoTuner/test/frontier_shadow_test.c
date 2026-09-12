#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* No roaring usage on this path — stubs keep the test independent. */
typedef struct RoaringBitmap RoaringBitmap;

RoaringBitmap *roaring_bitmap_create(uint64_t a, uint64_t c) {
  (void)a; (void)c; return NULL;
}
void roaring_bitmap_add(RoaringBitmap *b, uint32_t v) { (void)b; (void)v; }
void roaring_bitmap_remove(RoaringBitmap *b, uint32_t v) { (void)b; (void)v; }
bool roaring_bitmap_contains(const RoaringBitmap *b, uint32_t v) {
  (void)b; (void)v; return false;
}
void roaring_bitmap_clear(RoaringBitmap *b) { (void)b; }
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *b) {
  (void)b; return 0;
}
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *b, uint64_t i) {
  (void)b; (void)i; return 0;
}
RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *p) {
  (void)p; return NULL;
}
void roaring_bitmap_or_inplace(RoaringBitmap *d, RoaringBitmap *s) {
  (void)d; (void)s;
}
void roaring_bitmap_free(RoaringBitmap *b) { (void)b; }
void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **o,
                                               RoaringBitmap **r, int32_t c) {
  (void)o; (void)r; (void)c;
}
void roaring_bitmap_clear_thread_local_overrides(void) {}

/* cc_arcs_init reaches these only on the SET layout; the CSR harness never
 * calls them, but the linker needs the symbols. */
int32_t *graph_get_edge_pairs(void *graph_ptr) {
  (void)graph_ptr; return NULL;
}
int64_t graph_get_num_edge_pairs(void *graph_ptr) {
  (void)graph_ptr; return 0;
}

/* Graph layout matching autotuner_runtime.c offsets (n@0, m@8, row_ptr@16,
 * col_idx@24, weights@32, directed@40, in_row_ptr@48, in_col_idx@56). */
typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
  int32_t *weights;
  int32_t directed;
  int64_t *in_row_ptr;
  int32_t *in_col_idx;
} ShadowTestGraph;

/* The compiler-emitted pair fn reads the frozen shadow and writes the live
 * array; here the env carries both (mirrors the emitted construction). */
typedef struct {
  int32_t *live;
  const int32_t *shadow;
} ShadowEnv;

static void shadow_min_work(int32_t source, int32_t destination,
                            int64_t destination_index, void *env) {
  (void)source;
  (void)destination_index;
  ShadowEnv *E = (ShadowEnv *)env;
  int32_t nd = E->shadow[source] + 1;
  if (nd < E->live[destination])
    E->live[destination] = nd;
}

/* Serial round-separated reference: reads the frozen snapshot only. */
static void serial_min_sweep(const ShadowTestGraph *g, int32_t *out,
                             const int32_t *snapshot) {
  for (int64_t u = 0; u < g->n; ++u)
    for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j) {
      int32_t v = g->col_idx[j];
      int32_t nd = snapshot[u] + 1;
      if (nd < out[v])
        out[v] = nd;
    }
}

static int run_sweep(const ShadowTestGraph *g, int32_t partitions) {
  int32_t live[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  int32_t snapshot[8];
  memcpy(snapshot, live, sizeof(snapshot));

  /* Grab the runtime shadow scratch (the buffer the emitted construction
   * memcpy's into) and verify allocation semantics. */
  int32_t *shadow =
      (int32_t *)autograph_scratch_shadow((void *)g, 8 * sizeof(int32_t), 0);
  if (!shadow)
    return 0;
  if (autograph_scratch_shadow((void *)g, 4, 0) != shadow)
    return 0; /* no shrink */
  void *other = autograph_scratch_shadow((void *)g, 8 * sizeof(int32_t), 1);
  if (!other || other == (void *)shadow)
    return 0; /* slot isolation */

  memcpy(shadow, snapshot, sizeof(snapshot));
  if (memcmp(shadow, snapshot, sizeof(snapshot)) != 0)
    return 0;

  int32_t built = autograph_build_clean_cut((void *)g, partitions);
  if (built <= 0)
    return 0;

  ShadowEnv env = {.live = live, .shadow = shadow};
  autograph_frontier_step_owner_push((void *)g, NULL, 0, shadow_min_work,
                                     &env, NULL, NULL, 0, NULL);

  int32_t expected[8];
  memcpy(expected, snapshot, sizeof(expected));
  serial_min_sweep(g, expected, snapshot);
  return memcmp(live, expected, sizeof(live)) == 0;
}

int main(void) {
  /* Directed CSR, n=8: rows vary in length so multiple partitions share work. */
  int64_t row_ptr[9] = {0, 2, 3, 5, 6, 7, 8, 9, 10};
  int32_t col_idx[10] = {1, 2, 3, 3, 4, 5, 6, 7, 0, 7};
  ShadowTestGraph g = {
      .n = 8, .m = 10, .row_ptr = row_ptr, .col_idx = col_idx,
      .weights = NULL, .directed = 1, .in_row_ptr = NULL, .in_col_idx = NULL,
  };

  autograph_init(&g, g.n, g.m, NULL, NULL, NULL);

  /* Scratch growth: request larger, content must survive realloc. */
  int32_t *s0 = (int32_t *)autograph_scratch_shadow(&g, 16, 0);
  if (!s0)
    return 1;
  s0[0] = 42;
  s0[1] = 43;
  int32_t *s0b = (int32_t *)autograph_scratch_shadow(&g, 4096, 0);
  if (!s0b || s0b[0] != 42 || s0b[1] != 43)
    return 1;

  int failures = 0;
  int runs = 0;
  for (int p = 0; p < 3; ++p) {
    int32_t partitions[] = {1, 3, 8};
    runs++;
    if (!run_sweep(&g, partitions[p]))
      failures++;
  }
  printf("shadow scratch + frozen-read owner step: %d configs, %d failures\n",
         runs, failures);
  return failures == 0 ? 0 : 1;
}