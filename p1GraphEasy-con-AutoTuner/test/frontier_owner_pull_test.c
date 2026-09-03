#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

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
} TestGraph;

/* The exercised CSR frontier paths do not use Roaring bitmaps.  These stubs
 * keep this focused runtime test independent of the bitmap implementation. */
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

void roaring_bitmap_clear(RoaringBitmap *bitmap) {
  (void)bitmap;
}

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

void roaring_bitmap_free(RoaringBitmap *bitmap) {
  (void)bitmap;
}

void roaring_bitmap_set_thread_local_overrides(
    RoaringBitmap **originals, RoaringBitmap **replacements, int32_t count) {
  (void)originals;
  (void)replacements;
  (void)count;
}

void roaring_bitmap_clear_thread_local_overrides(void) {
}

static int is_neighbor(const TestGraph *graph, int32_t source,
                       int32_t destination) {
  for (int64_t edge = graph->row_ptr[source];
       edge < graph->row_ptr[source + 1]; ++edge)
    if (graph->col_idx[edge] == destination)
      return 1;
  return 0;
}

static int verify_step(const TestGraph *graph, const int32_t *frontier,
                       int32_t frontier_size, const int32_t *next,
                       int32_t next_size, const int32_t *claim,
                       const int32_t *parent, int destination_four_preclaimed) {
  int32_t expected[7] = {0};
  int32_t observed[7] = {0};
  expected[2] = 1;
  expected[3] = 1;
  expected[4] = destination_four_preclaimed ? 0 : 1;

  for (int32_t index = 0; index < next_size; ++index) {
    int32_t destination = next[index];
    if (destination < 0 || destination >= graph->n ||
        observed[destination]++)
      return 0;
  }

  for (int32_t destination = 0; destination < graph->n; ++destination) {
    if (observed[destination] != expected[destination])
      return 0;
    if (!expected[destination])
      continue;
    if (claim[destination] != 1 ||
        !is_neighbor(graph, parent[destination], destination))
      return 0;
    int parent_is_frontier = 0;
    for (int32_t index = 0; index < frontier_size; ++index)
      parent_is_frontier |= parent[destination] == frontier[index];
    if (!parent_is_frontier)
      return 0;
  }

  if (destination_four_preclaimed &&
      (claim[4] != 1 || parent[4] != 99 || observed[4] != 0))
    return 0;
  return 1;
}

static int run_mode(TestGraph *graph, const char *mode,
                    int destination_four_preclaimed) {
  int32_t frontier[2] = {0, 1};
  int32_t next[7] = {-1, -1, -1, -1, -1, -1, -1};
  int32_t claim[7] = {1, 1, 0, 0, 0, 0, 0};
  int32_t parent[7] = {0, 1, -1, -1, -1, -1, -1};
  if (destination_four_preclaimed) {
    claim[4] = 1;
    parent[4] = 99;
  }

  if (setenv("SGPL_FRONTIER_MODE", mode, 1) != 0)
    return 0;
  int32_t next_size =
      autograph_frontier_step(graph, frontier, 2, next, 0, claim, 0, 1,
                              parent);
  int32_t expected_size = destination_four_preclaimed ? 2 : 3;
  return next_size == expected_size &&
         verify_step(graph, frontier, 2, next, next_size, claim, parent,
                     destination_four_preclaimed);
}

/* Multi-step BFS under pull reuses persistent scratch across steps. */
static int run_multi_step_pull(TestGraph *graph) {
  if (setenv("SGPL_FRONTIER_MODE", "pull", 1) != 0)
    return 0;

  int32_t claim[7] = {1, 0, 0, 0, 0, 0, 0};
  int32_t parent[7] = {0, -1, -1, -1, -1, -1, -1};
  int32_t frontier[7] = {0};
  int32_t next[7];
  int32_t frontier_size = 1;
  int32_t visited = 1;

  for (int step = 0; step < 7 && frontier_size > 0; ++step) {
    int32_t next_size = autograph_frontier_step(
        graph, frontier, frontier_size, next, 0, claim, 0, 1, parent);
    if (next_size < 0 || next_size > graph->n)
      return 0;
    for (int32_t i = 0; i < next_size; ++i) {
      int32_t v = next[i];
      if (v < 0 || v >= graph->n || claim[v] != 1)
        return 0;
      if (!is_neighbor(graph, parent[v], v))
        return 0;
    }
    visited += next_size;
    frontier_size = next_size;
    memcpy(frontier, next, (size_t)next_size * sizeof(int32_t));
  }

  return visited == graph->n && frontier_size == 0;
}

int main(void) {
  int64_t row_ptr[8] = {0, 2, 4, 6, 9, 11, 13, 14};
  int32_t col_idx[14] = {
      2, 3, 3, 4, 0, 5, 0, 1, 5, 1, 6, 2, 3, 4,
  };
  TestGraph graph = {
      .n = 7,
      .m = 14,
      .row_ptr = row_ptr,
      .col_idx = col_idx,
  };

  autograph_init(&graph, graph.n, graph.m, NULL, NULL, NULL);
  if (!run_mode(&graph, "pull", 0) ||
      !run_mode(&graph, "pull", 1) ||
      !run_mode(&graph, "push", 0) ||
      !run_multi_step_pull(&graph)) {
    fprintf(stderr, "frontier owner-computes test failed\n");
    return 1;
  }

  puts("frontier owner-computes tests passed");
  return 0;
}
