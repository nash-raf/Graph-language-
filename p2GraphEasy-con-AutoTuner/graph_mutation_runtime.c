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

#include "autotuner_runtime.h"

typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
} Graph;

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
      return 1;
    if (g->col_idx[j] == -1) {
      g->col_idx[j] = to;
      return 1;
    }
  }
  return 0;
}

static void pcsr_remove_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return;
  if (from < 0 || (int64_t)from >= g->n)
    return;
  if (to < 0 || (int64_t)to >= g->n)
    return;
  int64_t start = g->row_ptr[from];
  int64_t end = g->row_ptr[from + 1];
  for (int64_t j = start; j < end; j++) {
    if (g->col_idx[j] == to) {
      g->col_idx[j] = -1;
      return;
    }
  }
}

/* ═══════════════════════════════════════════════════════════════
 *  Layout-dispatched public API
 * ═══════════════════════════════════════════════════════════════ */
/* Option B: Bitmaps only updated when layout == SET. Else convert to SET first. */
void graph_add_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                    int32_t node_id) {
  (void)nodes_bmp;
  (void)edge_pairs;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;

  int32_t layout = autograph_get_layout(graph_ptr);
  if (layout != LAYOUT_SET)
    autograph_ensure_layout_set(graph_ptr);
  autograph_canonical_add_node(graph_ptr, node_id);
}

void graph_remove_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                       int32_t node_id) {
  (void)nodes_bmp;
  (void)edge_pairs;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;

  int32_t layout = autograph_get_layout(graph_ptr);
  if (layout != LAYOUT_SET)
    autograph_ensure_layout_set(graph_ptr);
  autograph_canonical_remove_node(graph_ptr, node_id);
}

void graph_add_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to,
                    int32_t edge_id) {
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;

  int32_t layout = autograph_get_layout(graph_ptr);
  int in_bounds = (from >= 0 && (int64_t)from < g->n && to >= 0 && (int64_t)to < g->n);

  // Out-of-bounds edges are ignored in CSR/PCSR mode to preserve traversal safety.
  // Converting to SET here can null graph CSR pointers and crash later traversals
  // when no explicit conversion back to CSR is injected.
  if (!in_bounds && layout != LAYOUT_SET)
    return;

  switch (layout) {
  case LAYOUT_CSR:
    csr_add_directed(g, graph_ptr, from, to);
    csr_add_directed(g, graph_ptr, to, from);
    break;

  case LAYOUT_PCSR:
    pcsr_add_directed(g, from, to);
    pcsr_add_directed(g, to, from);
    break;

  case LAYOUT_SET:
  default:
    autograph_canonical_add_edge(graph_ptr, from, to);
    autograph_canonical_add_edge(graph_ptr, to, from);
    break;
  }
}

void graph_remove_edge(void *graph_ptr, void *edges_bmp, int32_t from,
                       int32_t to, int32_t edge_id) {
  (void)edge_id;
  (void)edges_bmp;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;

  int32_t layout = autograph_get_layout(graph_ptr);
  int in_bounds = (from >= 0 && (int64_t)from < g->n && to >= 0 && (int64_t)to < g->n);

  // Same safety guard as add-edge path.
  if (!in_bounds && layout != LAYOUT_SET)
    return;

  switch (layout) {
  case LAYOUT_CSR:
    csr_remove_directed(g, from, to);
    csr_remove_directed(g, to, from);
    break;

  case LAYOUT_PCSR:
    pcsr_remove_directed(g, from, to);
    pcsr_remove_directed(g, to, from);
    break;

  case LAYOUT_SET:
  default:
    autograph_canonical_remove_edge(graph_ptr, from, to);
    autograph_canonical_remove_edge(graph_ptr, to, from);
    break;
  }
}
