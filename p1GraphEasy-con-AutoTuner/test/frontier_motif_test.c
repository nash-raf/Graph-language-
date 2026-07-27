#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
  int32_t *weights; /* offset +32; matches Graph layout for weighted RelaxMin */
} TestGraph;

typedef struct RoaringBitmap RoaringBitmap;

RoaringBitmap *roaring_bitmap_create(uint64_t arena_size,
                                     uint64_t initial_capacity) {
  (void)arena_size;
  (void)initial_capacity;
  return NULL;
}
void roaring_bitmap_add(RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
}
void roaring_bitmap_remove(RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
}
bool roaring_bitmap_contains(const RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
  return false;
}
void roaring_bitmap_clear(RoaringBitmap *bitmap) { (void)bitmap; }
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *bitmap) {
  (void)bitmap;
  return 0;
}
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *bitmap,
                                     uint64_t index) {
  (void)bitmap;
  (void)index;
  return 0;
}
RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *prototype) {
  (void)prototype;
  return NULL;
}
void roaring_bitmap_or_inplace(RoaringBitmap *destination,
                               RoaringBitmap *source) {
  (void)destination;
  (void)source;
}
void roaring_bitmap_free(RoaringBitmap *bitmap) { (void)bitmap; }
void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **originals,
                                               RoaringBitmap **replacements,
                                               int32_t count) {
  (void)originals;
  (void)replacements;
  (void)count;
}
void roaring_bitmap_clear_thread_local_overrides(void) {}

static int contains(const int32_t *arr, int32_t n, int32_t v) {
  for (int32_t i = 0; i < n; ++i)
    if (arr[i] == v)
      return 1;
  return 0;
}

static int test_write_min(TestGraph *graph, const char *mode) {
  if (setenv("SGPL_FRONTIER_MODE", mode, 1) != 0)
    return 0;

  /* Line 0-1-2-3; labels start as vertex ids. Push label 0 from vertex 0. */
  int32_t labels[4] = {0, 1, 2, 3};
  int32_t frontier[1] = {0};
  int32_t next[4] = {-1, -1, -1, -1};
  int32_t next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_WRITE_MIN, frontier, 1, next, 0, labels, NULL, 0);
  if (next_size != 1 || !contains(next, next_size, 1) || labels[1] != 0)
    return 0;

  frontier[0] = 1;
  next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_WRITE_MIN, frontier, 1, next, 0, labels, NULL, 0);
  if (next_size != 1 || !contains(next, next_size, 2) || labels[2] != 0)
    return 0;
  return 1;
}

static int test_peel_k2(TestGraph *graph, const char *mode) {
  if (setenv("SGPL_FRONTIER_MODE", mode, 1) != 0)
    return 0;

  /* Path 0-1-2-3: degrees 1,2,2,1. k=2 peels endpoints then the rest. */
  int32_t deg[4] = {1, 2, 2, 1};
  int32_t alive[4] = {1, 1, 1, 1};
  int32_t frontier[4] = {0, 3};
  int32_t next[4] = {-1, -1, -1, -1};
  int32_t next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_PEEL_K, frontier, 2, next, 0, alive, deg, 2);
  if (alive[0] != 0 || alive[3] != 0)
    return 0;
  if (deg[1] != 1 || deg[2] != 1)
    return 0;
  if (next_size != 2 || !contains(next, next_size, 1) ||
      !contains(next, next_size, 2))
    return 0;

  next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_PEEL_K, next, next_size, frontier, 0, alive, deg, 2);
  if (alive[1] != 0 || alive[2] != 0)
    return 0;
  return 1;
}

/* ≥2 WriteMin steps under the forced mode; labels should flood to the end. */
static int test_write_min_multi(TestGraph *graph, const char *mode) {
  if (setenv("SGPL_FRONTIER_MODE", mode, 1) != 0)
    return 0;

  int32_t labels[4] = {0, 1, 2, 3};
  int32_t frontier[4] = {0};
  int32_t next[4];
  int32_t frontier_size = 1;
  for (int step = 0; step < 4 && frontier_size > 0; ++step) {
    int32_t next_size = autograph_motif_frontier_step(
        graph, SGPL_MOTIF_WRITE_MIN, frontier, frontier_size, next, 0, labels,
        NULL, 0);
    if (next_size < 0 || next_size > graph->n)
      return 0;
    frontier_size = next_size;
    memcpy(frontier, next, (size_t)next_size * sizeof(int32_t));
  }
  return labels[0] == 0 && labels[1] == 0 && labels[2] == 0 && labels[3] == 0 &&
         frontier_size == 0;
}

/* Weighted path 0-1-2-3 with edge weights 2,3,5 (symmetric). */
static int test_relax_min_weighted(TestGraph *graph, const char *mode) {
  if (setenv("SGPL_FRONTIER_MODE", mode, 1) != 0)
    return 0;

  const int32_t INF = INT32_MAX / 4;
  int32_t dist[4] = {0, INF, INF, INF};
  int32_t frontier[1] = {0};
  int32_t next[4] = {-1, -1, -1, -1};

  int32_t next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_RELAX_MIN_WEIGHTED, frontier, 1, next, 0, dist, NULL,
      0);
  if (next_size != 1 || !contains(next, next_size, 1) || dist[1] != 2)
    return 0;

  frontier[0] = 1;
  next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_RELAX_MIN_WEIGHTED, frontier, 1, next, 0, dist, NULL,
      0);
  if (next_size != 1 || !contains(next, next_size, 2) || dist[2] != 5)
    return 0;

  frontier[0] = 2;
  next_size = autograph_motif_frontier_step(
      graph, SGPL_MOTIF_RELAX_MIN_WEIGHTED, frontier, 1, next, 0, dist, NULL,
      0);
  if (next_size != 1 || !contains(next, next_size, 3) || dist[3] != 10)
    return 0;

  return dist[0] == 0 && dist[1] == 2 && dist[2] == 5 && dist[3] == 10;
}

static int test_relax_min_weighted_multi(TestGraph *graph, const char *mode) {
  if (setenv("SGPL_FRONTIER_MODE", mode, 1) != 0)
    return 0;

  const int32_t INF = INT32_MAX / 4;
  int32_t dist[4] = {0, INF, INF, INF};
  int32_t frontier[4] = {0};
  int32_t next[4];
  int32_t frontier_size = 1;
  for (int step = 0; step < 4 && frontier_size > 0; ++step) {
    int32_t next_size = autograph_motif_frontier_step(
        graph, SGPL_MOTIF_RELAX_MIN_WEIGHTED, frontier, frontier_size, next, 0,
        dist, NULL, 0);
    if (next_size < 0 || next_size > graph->n)
      return 0;
    frontier_size = next_size;
    memcpy(frontier, next, (size_t)next_size * sizeof(int32_t));
  }
  return dist[0] == 0 && dist[1] == 2 && dist[2] == 5 && dist[3] == 10 &&
         frontier_size == 0;
}

int main(void) {
  /* Undirected path 0-1-2-3 stored as symmetric CSR.
   * Edge weights: 0-1:2, 1-2:3, 2-3:5 (and reverse). */
  int64_t row_ptr[5] = {0, 1, 3, 5, 6};
  int32_t col_idx[6] = {1, 0, 2, 1, 3, 2};
  int32_t weights[6] = {2, 2, 3, 3, 5, 5};
  TestGraph graph = {
      .n = 4,
      .m = 6,
      .row_ptr = row_ptr,
      .col_idx = col_idx,
      .weights = weights,
  };

  autograph_init(&graph, graph.n, graph.m, NULL, NULL, NULL);
  if (!test_write_min(&graph, "push") || !test_write_min(&graph, "pull") ||
      !test_write_min_multi(&graph, "push") ||
      !test_write_min_multi(&graph, "pull") || !test_peel_k2(&graph, "push") ||
      !test_peel_k2(&graph, "pull") ||
      !test_relax_min_weighted(&graph, "push") ||
      !test_relax_min_weighted(&graph, "pull") ||
      !test_relax_min_weighted_multi(&graph, "push") ||
      !test_relax_min_weighted_multi(&graph, "pull")) {
    fprintf(stderr, "frontier motif test failed\n");
    return 1;
  }

  puts("frontier motif tests passed");
  return 0;
}
