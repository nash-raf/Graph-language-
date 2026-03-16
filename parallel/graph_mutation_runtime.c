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

static void ensure_csr(Graph *g) {
  if (!g)
    return;
  int32_t cur = autograph_get_layout((void *)g);
  if (cur != LAYOUT_CSR) {
    autograph_ensure_layout((void *)g, g->n, g->m, g->row_ptr, g->col_idx, NULL,
                            NULL, NULL, LAYOUT_CSR);
  }
}

static int csr_has_directed(const Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return 0;
  if (from < 0 || (int64_t)from >= g->n)
    return 0;
  int64_t start = g->row_ptr[from];
  int64_t end = g->row_ptr[from + 1];
  for (int64_t i = start; i < end; i++) {
    if (g->col_idx[i] == to)
      return 1;
  }
  return 0;
}

static void csr_add_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return;
  if (from < 0 || (int64_t)from >= g->n)
    return;
  if (to < 0 || (int64_t)to >= g->n)
    return;
  if (csr_has_directed(g, from, to))
    return;

  int64_t pos = g->row_ptr[from + 1];
  int64_t old_m = g->m;
  int32_t *new_ci = (int32_t *)realloc(g->col_idx, (size_t)(old_m + 1) * sizeof(int32_t));
  if (!new_ci)
    return;
  g->col_idx = new_ci;

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

void graph_add_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                    int32_t node_id) {
  (void)nodes_bmp;
  (void)edge_pairs;
  (void)node_id;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;
  ensure_csr(g);
  if (!g->row_ptr)
    return;

  int64_t old_n = g->n;
  int64_t *new_rp =
      (int64_t *)realloc(g->row_ptr, (size_t)(old_n + 2) * sizeof(int64_t));
  if (!new_rp)
    return;
  g->row_ptr = new_rp;
  g->row_ptr[old_n + 1] = g->m;
  g->n = old_n + 1;
  autograph_update_csr_pointers(graph_ptr, g->row_ptr, g->col_idx);
}

void graph_remove_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs,
                       int32_t node_id) {
  (void)nodes_bmp;
  (void)edge_pairs;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;
  ensure_csr(g);
  if (!g->row_ptr || !g->col_idx)
    return;
  if (node_id < 0 || (int64_t)node_id >= g->n)
    return;

  /* Remove outgoing edges of node_id. */
  while (g->row_ptr[node_id] < g->row_ptr[node_id + 1]) {
    int32_t v = g->col_idx[g->row_ptr[node_id]];
    csr_remove_directed(g, node_id, v);
  }
  /* Remove incoming edges to node_id. */
  for (int64_t u = 0; u < g->n; u++) {
    if ((int32_t)u == node_id)
      continue;
    while (1) {
      int64_t start = g->row_ptr[u];
      int64_t end = g->row_ptr[u + 1];
      int found = 0;
      for (int64_t j = start; j < end; j++) {
        if (g->col_idx[j] == node_id) {
          csr_remove_directed(g, (int32_t)u, node_id);
          found = 1;
          break;
        }
      }
      if (!found)
        break;
    }
  }
}

void graph_add_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to,
                    int32_t edge_id) {
  (void)edges_bmp;
  (void)edge_id;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;

  if (from < 0 || (int64_t)from >= g->n)
    return;
  if (to < 0 || (int64_t)to >= g->n)
    return;

  ensure_csr(g);
  csr_add_directed(g, from, to);
  csr_add_directed(g, to, from);
  autograph_update_csr_pointers(graph_ptr, g->row_ptr, g->col_idx);
}

void graph_remove_edge(void *graph_ptr, void *edges_bmp, int32_t from,
                       int32_t to, int32_t edge_id) {
  (void)edges_bmp;
  (void)edge_id;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;

  if (from < 0 || (int64_t)from >= g->n)
    return;
  if (to < 0 || (int64_t)to >= g->n)
    return;

  ensure_csr(g);
  csr_remove_directed(g, from, to);
  csr_remove_directed(g, to, from);
}
