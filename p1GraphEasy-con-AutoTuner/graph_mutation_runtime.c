/*
 * graph_mutation_runtime.c — Layout-aware graph mutation dispatch
 *
 * Each mutation (add/remove node/edge) dispatches to the implementation
 * appropriate for the CURRENT layout of the graph:
 *   LAYOUT_CSR  → realloc + memmove (expensive O(E))
 *   LAYOUT_PCSR → gap-slot fill/clear (cheap O(deg))
 *   LAYOUT_SET  → roaring bitmap canonical ops (O(1) amortised)
 *
 * The autotuner's DP scheduler picks the layout per region so that
 * insert-heavy regions use PCSR or SET for cheap mutations while
 * traversal-heavy regions use CSR for sequential access.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "autotuner_runtime.h"

typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
} Graph;

static uint64_t mutation_now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/* ═══════════════════════════════════════════════════════════════
 *  CSR-native mutations — O(E) due to realloc + memmove
 * ═══════════════════════════════════════════════════════════════ */
static void csr_add_directed(Graph *g, void *graph_ptr, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return;
  if (from < 0 || (int64_t)from >= g->n)
    return;
  if (to < 0 || (int64_t)to >= g->n)
    return;

  int64_t pos = g->row_ptr[from + 1];
  int64_t old_m = g->m;
  int32_t *new_ci = (int32_t *)realloc(g->col_idx, (size_t)(old_m + 1) * sizeof(int32_t));
  if (!new_ci)
    return;
  g->col_idx = new_ci;
  if (graph_ptr)
    autograph_update_csr_pointers(graph_ptr, g->row_ptr, g->col_idx);

  if (pos < old_m) {
    memmove(&g->col_idx[pos + 1], &g->col_idx[pos],
            (size_t)(old_m - pos) * sizeof(int32_t));
  }
  g->col_idx[pos] = to;
  g->m = old_m + 1;
  for (int64_t i = (int64_t)from + 1; i <= g->n; i++) {
    g->row_ptr[i]++;
  }
}

static void csr_remove_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return;
  if (from < 0 || (int64_t)from >= g->n)
    return;

  int64_t start = g->row_ptr[from];
  int64_t end = g->row_ptr[from + 1];
  int64_t pos = -1;
  for (int64_t i = start; i < end; i++) {
    if (g->col_idx[i] == to) {
      pos = i;
      break;
    }
  }
  if (pos < 0)
    return;

  int64_t old_m = g->m;
  if (pos + 1 < old_m) {
    memmove(&g->col_idx[pos], &g->col_idx[pos + 1],
            (size_t)(old_m - pos - 1) * sizeof(int32_t));
  }
  g->m = old_m - 1;
  for (int64_t i = (int64_t)from + 1; i <= g->n; i++) {
    g->row_ptr[i]--;
  }
}

/* ═══════════════════════════════════════════════════════════════
 *  PCSR-native mutations — O(deg) gap-slot scan
 * ═══════════════════════════════════════════════════════════════ */
static int pcsr_add_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return 0;
  if (from < 0 || (int64_t)from >= g->n)
    return 0;
  if (to < 0 || (int64_t)to >= g->n)
    return 0;
  int64_t start = g->row_ptr[from];
  int64_t end = g->row_ptr[from + 1];
  for (int64_t j = start; j < end; j++) {
    if (g->col_idx[j] == to)
      return 0;
    if (g->col_idx[j] == -1) {
      g->col_idx[j] = to;
      return 1;
    }
  }
  return -1;
}

static int pcsr_remove_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return 0;
  if (from < 0 || (int64_t)from >= g->n)
    return 0;
  if (to < 0 || (int64_t)to >= g->n)
    return 0;
  int64_t start = g->row_ptr[from];
  int64_t end = g->row_ptr[from + 1];
  for (int64_t j = start; j < end; j++) {
    if (g->col_idx[j] == to) {
      g->col_idx[j] = -1;
      return 1;
    }
  }
  return 0;
}

/* ═══════════════════════════════════════════════════════════════
 *  Layout-dispatched public API
 * ═══════════════════════════════════════════════════════════════ */
/* Option B: Bitmaps only updated when layout == SET. Else convert to SET first. */
void graph_add_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                    int32_t node_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)nodes_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  int32_t layout_before = autograph_get_layout(graph_ptr);
  if (layout_before != LAYOUT_SET)
    autograph_ensure_layout_set(graph_ptr);
  autograph_canonical_add_node(graph_ptr, node_id);

  if (layout_before != LAYOUT_SET) {
    autograph_ensure_layout(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx,
                            nodes_bmp, NULL, edge_pairs, layout_before);
  }
  autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
}

void graph_remove_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                       int32_t node_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)nodes_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  int32_t layout_before = autograph_get_layout(graph_ptr);
  if (layout_before != LAYOUT_SET)
    autograph_ensure_layout_set(graph_ptr);
  autograph_canonical_remove_node(graph_ptr, node_id);

  if (layout_before != LAYOUT_SET) {
    autograph_ensure_layout(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx,
                            nodes_bmp, NULL, edge_pairs, layout_before);
  }
  autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
}

void graph_add_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to,
                    int32_t edge_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  int32_t layout = autograph_get_layout(graph_ptr);
  int in_bounds = (from >= 0 && (int64_t)from < g->n && to >= 0 && (int64_t)to < g->n);
  int recorded_kernel_time = 0;

  switch (layout) {
  case LAYOUT_CSR:
  {
    int64_t old_m = g->m;
    csr_add_directed(g, graph_ptr, from, to);
    csr_add_directed(g, graph_ptr, to, from);
    autograph_record_adjacency_state(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx);
    if (g->m != old_m)
      autograph_mark_canonical_dirty(graph_ptr);
    if (!in_bounds) {
      autograph_ensure_layout_set(graph_ptr);
      autograph_canonical_add_edge(graph_ptr, from, to);
      autograph_ensure_layout(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx,
                              NULL, edges_bmp, NULL, LAYOUT_CSR);
    }
    break;
  }

  case LAYOUT_PCSR:
  {
    int added_ab = pcsr_add_directed(g, from, to);
    int added_ba = pcsr_add_directed(g, to, from);
    if (added_ab > 0 && added_ba > 0)
      g->m += 2;
    autograph_record_adjacency_state(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx);
    if (added_ab > 0 && added_ba > 0)
      autograph_mark_canonical_dirty(graph_ptr);
    if (!in_bounds) {
      autograph_ensure_layout_set(graph_ptr);
      autograph_canonical_add_edge(graph_ptr, from, to);
      autograph_ensure_layout(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx,
                              NULL, edges_bmp, NULL, LAYOUT_PCSR);
    }
    break;
  }

  case LAYOUT_BCSR:
  {
    int added_ab = autograph_bcsr_add_edge(graph_ptr, from, to);
    int added_ba = autograph_bcsr_add_edge(graph_ptr, to, from);
    if (added_ab > 0 && added_ba > 0)
      autograph_mark_canonical_dirty(graph_ptr);
    if (!in_bounds) {
      autograph_ensure_layout_set(graph_ptr);
      autograph_canonical_add_edge(graph_ptr, from, to);
      autograph_canonical_add_edge(graph_ptr, to, from);
    }
    break;
  }

  case LAYOUT_SET:
  default:
    autograph_canonical_add_edge(graph_ptr, from, to);
    break;
  }
  if (!recorded_kernel_time)
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
}

void graph_remove_edge(void *graph_ptr, void *edges_bmp, int32_t from,
                       int32_t to, int32_t edge_id) {
  uint64_t start_ns = mutation_now_ns();
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g) {
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
    return;
  }

  int32_t layout = autograph_get_layout(graph_ptr);
  int in_bounds = (from >= 0 && (int64_t)from < g->n && to >= 0 && (int64_t)to < g->n);
  int recorded_kernel_time = 0;

  switch (layout) {
  case LAYOUT_CSR:
  {
    int64_t old_m = g->m;
    csr_remove_directed(g, from, to);
    csr_remove_directed(g, to, from);
    autograph_record_adjacency_state(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx);
    if (g->m != old_m)
      autograph_mark_canonical_dirty(graph_ptr);
    if (!in_bounds) {
      autograph_ensure_layout_set(graph_ptr);
      autograph_canonical_remove_edge(graph_ptr, from, to);
      autograph_ensure_layout(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx,
                              NULL, edges_bmp, NULL, LAYOUT_CSR);
    }
    break;
  }

  case LAYOUT_PCSR:
  {
    int removed_ab = pcsr_remove_directed(g, from, to);
    int removed_ba = pcsr_remove_directed(g, to, from);
    if (removed_ab > 0 && removed_ba > 0)
      g->m -= 2;
    autograph_record_adjacency_state(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx);
    if (removed_ab > 0 && removed_ba > 0)
      autograph_mark_canonical_dirty(graph_ptr);
    if (!in_bounds) {
      autograph_ensure_layout_set(graph_ptr);
      autograph_canonical_remove_edge(graph_ptr, from, to);
      autograph_ensure_layout(graph_ptr, g->n, g->m, g->row_ptr, g->col_idx,
                              NULL, edges_bmp, NULL, LAYOUT_PCSR);
    }
    break;
  }

  case LAYOUT_BCSR:
  {
    int removed_ab = autograph_bcsr_remove_edge(graph_ptr, from, to);
    int removed_ba = autograph_bcsr_remove_edge(graph_ptr, to, from);
    if (removed_ab > 0 && removed_ba > 0)
      autograph_mark_canonical_dirty(graph_ptr);
    if (!in_bounds) {
      autograph_ensure_layout_set(graph_ptr);
      autograph_canonical_remove_edge(graph_ptr, from, to);
      autograph_canonical_remove_edge(graph_ptr, to, from);
    }
    break;
  }

  case LAYOUT_SET:
  default:
    autograph_canonical_remove_edge(graph_ptr, from, to);
    break;
  }
  if (!recorded_kernel_time)
    autograph_profile_record_kernel_ns(1, mutation_now_ns() - start_ns);
}
