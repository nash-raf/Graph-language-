#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
  int32_t *weights;
} Graph;

extern void graph_register_csr_metadata(Graph *g);

static int contains_neighbor(const Graph *g, int32_t from, int32_t to) {
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

static int ensure_node_capacity(Graph *g, int32_t node_id) {
  if (!g || node_id < 0)
    return 0;
  if ((int64_t)node_id < g->n)
    return 1;

  int64_t old_n = g->n;
  int64_t new_n = (int64_t)node_id + 1;
  int64_t *new_rp = (int64_t *)realloc(g->row_ptr, (size_t)(new_n + 1) * sizeof(int64_t));
  if (!new_rp)
    return 0;
  g->row_ptr = new_rp;
  for (int64_t i = old_n + 1; i <= new_n; i++) {
    g->row_ptr[i] = g->m;
  }
  g->n = new_n;
  return 1;
}

static int insert_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !ensure_node_capacity(g, from) || !ensure_node_capacity(g, to))
    return 0;
  if (contains_neighbor(g, from, to))
    return 1;

  int64_t pos = g->row_ptr[from + 1];
  int64_t old_m = g->m;
  int32_t *new_ci = (int32_t *)realloc(g->col_idx, (size_t)(old_m + 1) * sizeof(int32_t));
  if (!new_ci)
    return 0;
  g->col_idx = new_ci;

  if (g->weights) {
    int32_t *new_w = (int32_t *)realloc(g->weights, (size_t)(old_m + 1) * sizeof(int32_t));
    if (!new_w)
      return 0;
    g->weights = new_w;
  }

  if (pos < old_m) {
    memmove(&g->col_idx[pos + 1], &g->col_idx[pos], (size_t)(old_m - pos) * sizeof(int32_t));
    if (g->weights) {
      memmove(&g->weights[pos + 1], &g->weights[pos], (size_t)(old_m - pos) * sizeof(int32_t));
    }
  }

  g->col_idx[pos] = to;
  if (g->weights)
    g->weights[pos] = 1;
  g->m = old_m + 1;
  for (int64_t i = (int64_t)from + 1; i <= g->n; i++) {
    g->row_ptr[i]++;
  }
  return 1;
}

static void remove_directed(Graph *g, int32_t from, int32_t to) {
  if (!g || !g->row_ptr || !g->col_idx)
    return;
  if (from < 0 || (int64_t)from >= g->n || to < 0 || (int64_t)to >= g->n)
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
    memmove(&g->col_idx[pos], &g->col_idx[pos + 1], (size_t)(old_m - pos - 1) * sizeof(int32_t));
    if (g->weights) {
      memmove(&g->weights[pos], &g->weights[pos + 1], (size_t)(old_m - pos - 1) * sizeof(int32_t));
    }
  }
  g->m = old_m - 1;
  for (int64_t i = (int64_t)from + 1; i <= g->n; i++) {
    g->row_ptr[i]--;
  }

  if (g->m == 0) {
    free(g->col_idx);
    g->col_idx = NULL;
    free(g->weights);
    g->weights = NULL;
    return;
  }

  int32_t *shrunk_ci = (int32_t *)realloc(g->col_idx, (size_t)g->m * sizeof(int32_t));
  if (shrunk_ci)
    g->col_idx = shrunk_ci;
  if (g->weights) {
    int32_t *shrunk_w = (int32_t *)realloc(g->weights, (size_t)g->m * sizeof(int32_t));
    if (shrunk_w)
      g->weights = shrunk_w;
  }
}

void graph_add_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs, int32_t node_id) {
  (void)nodes_bmp;
  (void)edge_pairs;
  Graph *g = (Graph *)graph_ptr;
  if (!ensure_node_capacity(g, node_id))
    return;
  graph_register_csr_metadata(g);
}

void graph_remove_node(void *graph_ptr, void *nodes_bmp, void *edge_pairs, int32_t node_id) {
  (void)nodes_bmp;
  (void)edge_pairs;
  Graph *g = (Graph *)graph_ptr;
  if (!g || node_id < 0 || (int64_t)node_id >= g->n)
    return;

  for (int64_t from = 0; from < g->n; from++) {
    if ((int32_t)from == node_id)
      continue;
    remove_directed(g, (int32_t)from, node_id);
  }

  while (g->row_ptr[node_id] < g->row_ptr[node_id + 1]) {
    int32_t dst = g->col_idx[g->row_ptr[node_id]];
    remove_directed(g, node_id, dst);
    if (dst >= 0 && (int64_t)dst < g->n)
      remove_directed(g, dst, node_id);
  }

  graph_register_csr_metadata(g);
}

void graph_add_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to, int32_t edge_id) {
  (void)edges_bmp;
  (void)edge_id;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;
  if (!insert_directed(g, from, to))
    return;
  insert_directed(g, to, from);
  graph_register_csr_metadata(g);
}

void graph_remove_edge(void *graph_ptr, void *edges_bmp, int32_t from, int32_t to, int32_t edge_id) {
  (void)edges_bmp;
  (void)edge_id;
  Graph *g = (Graph *)graph_ptr;
  if (!g)
    return;
  remove_directed(g, from, to);
  remove_directed(g, to, from);
  graph_register_csr_metadata(g);
}
