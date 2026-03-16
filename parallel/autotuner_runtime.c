/*
 * autotuner_runtime.c — Phase-Aware Graph Layout AutoTuner Runtime
 *
 * Implements the Set-Based Architecture constraints.
 * LAYOUT_SET (Roaring bitamps) is the absolute base.
 * Transient adjacency layouts (CSR, PCSR, BCSR) are built on-demand
 * for specific execution phases and destroyed after use.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autotuner_runtime.h"

/* Forward declaring external Roaring C API we need */
#ifdef __cplusplus
extern "C" {
#endif
typedef struct roaring_bitmap_s RoaringBitmap;
RoaringBitmap *roaring_bitmap_create(uint64_t arena_size,
                                     uint64_t initial_capacity);
void roaring_bitmap_add(RoaringBitmap *r, uint32_t val);
bool roaring_bitmap_contains(const RoaringBitmap *r, uint32_t val);
void roaring_bitmap_clear(RoaringBitmap *r);
#ifdef __cplusplus
}
#endif

/* Struct defined in IRGenVisitor.h for edge lists */
typedef struct {
  int64_t u;
  int64_t v;
} EdgePair;

/* ══════════════════════════════════════════════════════════════════
 *  Per-graph metadata registry
 * ══════════════════════════════════════════════════════════════════ */
#define MAX_GRAPHS 64
static AutoGraphMeta g_meta[MAX_GRAPHS];
static int g_meta_count = 0;

static AutoGraphMeta *find_meta(void *graph_ptr) {
  for (int i = 0; i < g_meta_count; i++) {
    if (g_meta[i].graph_ptr == graph_ptr)
      return &g_meta[i];
  }
  return NULL;
}

static AutoGraphMeta *find_or_create_meta(void *graph_ptr) {
  AutoGraphMeta *m = find_meta(graph_ptr);
  if (m)
    return m;
  if (g_meta_count >= MAX_GRAPHS)
    return NULL;

  AutoGraphMeta *meta = &g_meta[g_meta_count++];
  memset(meta, 0, sizeof(*meta));
  meta->graph_ptr = graph_ptr;
  meta->current_layout = LAYOUT_SET; /* Absolute Base State */
  return meta;
}

/* ══════════════════════════════════════════════════════════════════
 *  Set Base Construction & Rebuilding
 * ══════════════════════════════════════════════════════════════════ */

/* Build CSR from Set Bitmaps.
 * Iterates through the edge_pairs array, checks if that edge ID is active
 * in edges_bitmap, and populates CSR if so. */
void build_csr_from_set(int64_t n, void *edges_bitmap, void *edge_pairs_raw,
                        int64_t **out_rp, int32_t **out_ci, int64_t *out_m) {
  RoaringBitmap *eb = (RoaringBitmap *)edges_bitmap;
  EdgePair *pairs = (EdgePair *)edge_pairs_raw;

  /* Max possible edges depending on global array size, but we approximate
   * by scanning. For demonstration, we assume edge IDs are 0..MAX_E */

  /* First pass: count degree per vertex */
  int64_t *rp = (int64_t *)calloc((size_t)(n + 1), sizeof(int64_t));
  int64_t m = 0;

  /* In a real Roaring iter, we'd use roaring_iterator.
   * For simplicity, check up to a large bound, or adapt if you have MaxEdgeID
   */
  uint32_t MAX_EDGE_ID = (uint32_t)(n * 10 > 1000 ? n * 10 : 1000);
  for (uint32_t e = 0; e < MAX_EDGE_ID; e++) {
    if (roaring_bitmap_contains(eb, e)) {
      int64_t u = pairs[e].u;
      if (u >= 0 && u < n) {
        rp[u + 1]++;
        m++;
      }
    }
  }

  /* Prefix sum (0-based CSR) */
  for (int64_t i = 1; i <= n; i++) {
    rp[i] += rp[i - 1];
  }

  /* Second pass: copy targets (0-based) */
  int32_t *ci = (int32_t *)malloc((size_t)m * sizeof(int32_t));
  int64_t *next = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));
  memcpy(next, rp, (size_t)(n + 1) * sizeof(int64_t));

  for (uint32_t e = 0; e < MAX_EDGE_ID; e++) {
    if (roaring_bitmap_contains(eb, e)) {
      int64_t u = pairs[e].u;
      int64_t v = pairs[e].v;
      if (u >= 0 && u < n && v >= 0 && v < n) {
        ci[next[u]++] = (int32_t)v;
      }
    }
  }

  free(next);
  *out_rp = rp;
  *out_ci = ci;
  *out_m = m;

}

/* Rebuild Sets from CSR.
 * Iterates through CSR arrays, finds the corresponding edge ID in edge_pairs,
 * and recreates the node and edge Roaring bitmaps. */
void rebuild_sets_from_csr(int64_t n, int64_t m, const int64_t *csr_rp,
                           const int32_t *csr_ci, void *nodes_bitmap,
                           void *edges_bitmap, void *edge_pairs_raw) {
  RoaringBitmap *nb = (RoaringBitmap *)nodes_bitmap;
  RoaringBitmap *eb = (RoaringBitmap *)edges_bitmap;
  EdgePair *pairs = (EdgePair *)edge_pairs_raw;

  /* Clear the existing base sets */
  roaring_bitmap_clear(nb);
  roaring_bitmap_clear(eb);

  uint32_t MAX_EDGE_ID = (uint32_t)n * 10;

  /* Add nodes and edges */
  for (int64_t u = 1; u <= n; u++) {
    int64_t start =
        csr_rp[u - 1]; // Assuming 1-indexed u for graph DSL but 0-indexed CSR
    int64_t end = csr_rp[u];

    if (end > start) {
      roaring_bitmap_add(nb, (uint32_t)u);
    }

    for (int64_t j = start; j < end; j++) {
      int32_t v = csr_ci[j];
      roaring_bitmap_add(nb, (uint32_t)v);

      /* Reverse lookup edge ID. O(E^2) naive, optimize with hash table in prod
       */
      for (uint32_t e = 0; e < MAX_EDGE_ID; e++) {
        if (pairs[e].u == u && pairs[e].v == v) {
          roaring_bitmap_add(eb, e);
          break;
        }
      }
    }
  }

}

/* ══════════════════════════════════════════════════════════════════
 *  CSR <-> PCSR <-> BCSR Transient Conversions
 * ══════════════════════════════════════════════════════════════════ */
void convert_csr_to_pcsr(int64_t n, int64_t m, const int64_t *csr_rp,
                         const int32_t *csr_ci, int64_t **out_rp,
                         int32_t **out_ci, int64_t *out_capacity) {
  int64_t cap = (m < 16) ? 32 : m * 2;
  int32_t *pcol = (int32_t *)malloc((size_t)cap * sizeof(int32_t));
  int64_t *prp = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));

  memset(pcol, 0xFF, (size_t)cap * sizeof(int32_t));
  int64_t write_pos = 0;
  for (int64_t u = 0; u < n; u++) {
    int64_t deg = csr_rp[u + 1] - csr_rp[u];
    int64_t gap_slots = (deg > 2) ? deg : 2;
    int64_t row_cap = deg + gap_slots;

    /* Realloc before writing to avoid buffer overrun */
    if (write_pos + row_cap > cap) {
      int64_t old_cap = cap;
      cap = (write_pos + row_cap) * 2;
      pcol = (int32_t *)realloc(pcol, (size_t)cap * sizeof(int32_t));
      if (cap > old_cap)
        memset(pcol + old_cap, 0xFF,
               (size_t)(cap - old_cap) * sizeof(int32_t));
    }

    prp[u] = write_pos;
    for (int64_t j = 0; j < deg; j++) {
      pcol[write_pos + j] = csr_ci[csr_rp[u] + j];
    }
    write_pos += row_cap;
  }
  prp[n] = write_pos;

  *out_rp = prp;
  *out_ci = pcol;
  *out_capacity = cap;
}

void convert_pcsr_to_csr(int64_t n, int64_t pcsr_cap __attribute__((unused)),
                         const int64_t *pcsr_rp, const int32_t *pcsr_ci,
                         int64_t **out_rp, int32_t **out_ci, int64_t *out_m) {
  const int32_t GAP = -1;
  int64_t *rp = (int64_t *)calloc((size_t)(n + 1), sizeof(int64_t));
  int64_t total = 0;

  for (int64_t u = 0; u < n; u++) {
    int64_t start = pcsr_rp[u];
    int64_t end = pcsr_rp[u + 1];
    int64_t count = 0;
    for (int64_t j = start; j < end; j++) {
      if (pcsr_ci[j] != GAP)
        count++;
    }
    rp[u + 1] = count;
    total += count;
  }
  for (int64_t u = 1; u <= n; u++)
    rp[u] += rp[u - 1];

  int32_t *ci = (int32_t *)malloc((size_t)total * sizeof(int32_t));
  int64_t *next = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));
  memcpy(next, rp, (size_t)(n + 1) * sizeof(int64_t));

  for (int64_t u = 0; u < n; u++) {
    int64_t start = pcsr_rp[u];
    int64_t end = pcsr_rp[u + 1];
    for (int64_t j = start; j < end; j++) {
      if (pcsr_ci[j] != GAP) {
        ci[next[u]++] = pcsr_ci[j];
      }
    }
  }
  free(next);
  *out_rp = rp;
  *out_ci = ci;
  *out_m = total;
}

/*
 * BCSR payload format used by this runtime:
 *   - brow[b]..brow[b+1)-1 is the slice for block b
 *   - bcol stores (local_row, col) pairs as consecutive i32 values
 *     [r0, c0, r1, c1, ...]
 * where local_row is in [0, block_size).
 */
void convert_csr_to_bcsr(int64_t n, int64_t m, const int64_t *csr_rp,
                         const int32_t *csr_ci, int32_t **out_brow,
                         int32_t **out_bcol, int32_t block_size,
                         int32_t *out_nblocks) {
  if (!out_brow || !out_bcol || !out_nblocks || block_size <= 0) {
    return;
  }

  int32_t nb = (int32_t)((n + block_size - 1) / block_size);
  int32_t *brow = (int32_t *)calloc((size_t)(nb + 1), sizeof(int32_t));
  if (!brow) {
    return;
  }

  /* First pass: count i32 payload per block (2 ints per edge). */
  int64_t total_ints64 = 0;
  for (int32_t b = 0; b < nb; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int64_t block_edges = 0;
    for (int64_t u = start_u; u < end_u; u++) {
      int64_t deg = csr_rp[u + 1] - csr_rp[u];
      if (deg > 0)
        block_edges += deg;
    }

    int64_t block_ints = block_edges * 2;
    total_ints64 += block_ints;
    if (total_ints64 > INT32_MAX) {
      free(brow);
      return;
    }
    brow[b + 1] = (int32_t)total_ints64;
  }

  int32_t *bcol = NULL;
  if (total_ints64 > 0) {
    bcol = (int32_t *)malloc((size_t)total_ints64 * sizeof(int32_t));
    if (!bcol) {
      free(brow);
      return;
    }
  }

  /* Second pass: emit (local_row, col) pairs by block. */
  for (int32_t b = 0; b < nb; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int32_t write = brow[b];
    for (int64_t u = start_u; u < end_u; u++) {
      int32_t local_row = (int32_t)(u - start_u);
      for (int64_t j = csr_rp[u]; j < csr_rp[u + 1]; j++) {
        bcol[write++] = local_row;
        bcol[write++] = csr_ci[j];
      }
    }
  }

  *out_brow = brow;
  *out_bcol = bcol;
  *out_nblocks = nb;
  (void)m; /* m is implicit in payload length; keep signature stable. */
}

void convert_bcsr_to_csr(int64_t n, int32_t nblocks, int32_t block_size,
                         const int32_t *bcsr_brow, const int32_t *bcsr_bcol,
                         int64_t **out_rp, int32_t **out_ci, int64_t *out_m) {
  if (!out_rp || !out_ci || !out_m || !bcsr_brow || block_size <= 0 ||
      nblocks < 0) {
    return;
  }

  int64_t *rp = (int64_t *)calloc((size_t)(n + 1), sizeof(int64_t));
  if (!rp) {
    return;
  }

  /* First pass: count per-row degree from BCSR payload pairs. */
  for (int32_t b = 0; b < nblocks; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int32_t start = bcsr_brow[b];
    int32_t end = bcsr_brow[b + 1];
    for (int32_t k = start; k + 1 < end; k += 2) {
      int32_t local_row = bcsr_bcol[k];
      int64_t u = start_u + (int64_t)local_row;
      if (u >= start_u && u < end_u) {
        rp[u + 1]++;
      }
    }
  }

  for (int64_t u = 1; u <= n; u++) {
    rp[u] += rp[u - 1];
  }

  int64_t total_edges = rp[n];
  int32_t *ci = NULL;
  if (total_edges > 0) {
    ci = (int32_t *)malloc((size_t)total_edges * sizeof(int32_t));
    if (!ci) {
      free(rp);
      return;
    }
  }

  int64_t *next = (int64_t *)malloc((size_t)(n + 1) * sizeof(int64_t));
  if (!next) {
    free(rp);
    free(ci);
    return;
  }
  memcpy(next, rp, (size_t)(n + 1) * sizeof(int64_t));

  /* Second pass: restore CSR col_idx stream. */
  for (int32_t b = 0; b < nblocks; b++) {
    int64_t start_u = (int64_t)b * block_size;
    int64_t end_u = start_u + block_size;
    if (end_u > n)
      end_u = n;

    int32_t start = bcsr_brow[b];
    int32_t end = bcsr_brow[b + 1];
    for (int32_t k = start; k + 1 < end; k += 2) {
      int32_t local_row = bcsr_bcol[k];
      int32_t col = bcsr_bcol[k + 1];
      int64_t u = start_u + (int64_t)local_row;
      if (u >= start_u && u < end_u) {
        ci[next[u]++] = col;
      }
    }
  }

  free(next);
  *out_rp = rp;
  *out_ci = ci;
  *out_m = total_edges;
}

/* ══════════════════════════════════════════════════════════════════
 *  Main Layout Transition Entry
 * ══════════════════════════════════════════════════════════════════ */
void autograph_ensure_layout(void *graph_ptr, int64_t n, int64_t m,
                             int64_t *struct_row_ptr, int32_t *struct_col_idx,
                             void *nodes_bmp, void *edges_bmp,
                             void *edge_pairs_table, int32_t target_layout) {
  AutoGraphMeta *meta = find_or_create_meta(graph_ptr);
  if (!meta)
    return;

  if (meta->current_layout == target_layout) {
    return; // nothing to do
  }

  /*
   * CASE 1: Transitioning FROM a transient layout BACK TO the BASE SET
   * Action: Rebuild the set to capture mutations, then completely DESTROY the
   * transient layout.
   */
  if (target_layout == LAYOUT_SET) {
    if (meta->current_layout == LAYOUT_CSR) {
      rebuild_sets_from_csr(n, meta->csr_m, meta->csr_row_ptr,
                            meta->csr_col_idx, nodes_bmp, edges_bmp,
                            edge_pairs_table);
      free(meta->csr_row_ptr);
      meta->csr_row_ptr = NULL;
      free(meta->csr_col_idx);
      meta->csr_col_idx = NULL;
    } else if (meta->current_layout == LAYOUT_PCSR) {
      int64_t tmp_m;
      int64_t *tmp_rp;
      int32_t *tmp_ci;
      convert_pcsr_to_csr(n, meta->pcsr_capacity, meta->pcsr_row_ptr,
                          meta->pcsr_col_idx, &tmp_rp, &tmp_ci, &tmp_m);
      rebuild_sets_from_csr(n, tmp_m, tmp_rp, tmp_ci, nodes_bmp, edges_bmp,
                            edge_pairs_table);
      free(tmp_rp);
      free(tmp_ci);

      free(meta->pcsr_row_ptr);
      meta->pcsr_row_ptr = NULL;
      free(meta->pcsr_col_idx);
      meta->pcsr_col_idx = NULL;
    } else if (meta->current_layout == LAYOUT_BCSR) {
      int64_t tmp_m;
      int64_t *tmp_rp;
      int32_t *tmp_ci;
      convert_bcsr_to_csr(n, meta->bcsr_nblocks, meta->bcsr_block_size,
                          meta->bcsr_brow_ptr, meta->bcsr_bcol_idx, &tmp_rp,
                          &tmp_ci, &tmp_m);
      rebuild_sets_from_csr(n, tmp_m, tmp_rp, tmp_ci, nodes_bmp, edges_bmp,
                            edge_pairs_table);
      free(tmp_rp);
      free(tmp_ci);

      free(meta->bcsr_brow_ptr);
      meta->bcsr_brow_ptr = NULL;
      free(meta->bcsr_bcol_idx);
      meta->bcsr_bcol_idx = NULL;
    }

    /* Update struct.Graph arrays to NULL (they shouldn't be used while in SET
     * layout) */
    char *base = (char *)graph_ptr;
    *((int64_t **)(base + 16)) = NULL;
    *((int32_t **)(base + 24)) = NULL;

    meta->current_layout = LAYOUT_SET;
    return;
  }

  /*
   * CASE 2: Transitioning FROM BASE SET to a TRANSIENT LAYOUT
   * Action: Build the layout directly from the Sets.
   */
  if (meta->current_layout == LAYOUT_SET) {
    if (target_layout == LAYOUT_CSR) {
      build_csr_from_set(n, edges_bmp, edge_pairs_table, &meta->csr_row_ptr,
                         &meta->csr_col_idx, &meta->csr_m);
      meta->current_layout = LAYOUT_CSR;
    } else if (target_layout == LAYOUT_PCSR) {
      /* Build CSR first, then PCSR, then destroy CSR */
      int64_t tmp_m;
      int64_t *tmp_rp;
      int32_t *tmp_ci;
      build_csr_from_set(n, edges_bmp, edge_pairs_table, &tmp_rp, &tmp_ci,
                         &tmp_m);
      convert_csr_to_pcsr(n, tmp_m, tmp_rp, tmp_ci, &meta->pcsr_row_ptr,
                          &meta->pcsr_col_idx, &meta->pcsr_capacity);
      free(tmp_rp);
      free(tmp_ci);
      meta->current_layout = LAYOUT_PCSR;
    } else if (target_layout == LAYOUT_BCSR) {
      /* Build CSR first, then BCSR, then destroy CSR */
      int64_t tmp_m;
      int64_t *tmp_rp;
      int32_t *tmp_ci;
      build_csr_from_set(n, edges_bmp, edge_pairs_table, &tmp_rp, &tmp_ci,
                         &tmp_m);
      meta->bcsr_block_size = 64;
      convert_csr_to_bcsr(n, tmp_m, tmp_rp, tmp_ci, &meta->bcsr_brow_ptr,
                          &meta->bcsr_bcol_idx, 64, &meta->bcsr_nblocks);
      free(tmp_rp);
      free(tmp_ci);
      meta->current_layout = LAYOUT_BCSR;
    }
  }

  /*
   * CASE 3: Transient to Transient
   * Action: Convert, then immediately destroy the old layout arrays.
   */
  else {
    // ... (We route everything through CSR internally for simplicity)
    int64_t tmp_m;
    int64_t *tmp_rp;
    int32_t *tmp_ci;

    // Convert CURRENT to CSR
    if (meta->current_layout == LAYOUT_PCSR) {
      convert_pcsr_to_csr(n, meta->pcsr_capacity, meta->pcsr_row_ptr,
                          meta->pcsr_col_idx, &tmp_rp, &tmp_ci, &tmp_m);
      free(meta->pcsr_row_ptr);
      meta->pcsr_row_ptr = NULL;
      free(meta->pcsr_col_idx);
      meta->pcsr_col_idx = NULL;
    } else if (meta->current_layout == LAYOUT_BCSR) {
      convert_bcsr_to_csr(n, meta->bcsr_nblocks, meta->bcsr_block_size,
                          meta->bcsr_brow_ptr, meta->bcsr_bcol_idx, &tmp_rp,
                          &tmp_ci, &tmp_m);
      free(meta->bcsr_brow_ptr);
      meta->bcsr_brow_ptr = NULL;
      free(meta->bcsr_bcol_idx);
      meta->bcsr_bcol_idx = NULL;
    } else {
      tmp_m = meta->csr_m;
      tmp_rp = meta->csr_row_ptr;
      tmp_ci = meta->csr_col_idx;
    }

    // Convert CSR to TARGET
    if (target_layout == LAYOUT_CSR) {
      meta->csr_m = tmp_m;
      meta->csr_row_ptr = tmp_rp;
      meta->csr_col_idx = tmp_ci;
    } else if (target_layout == LAYOUT_PCSR) {
      convert_csr_to_pcsr(n, tmp_m, tmp_rp, tmp_ci, &meta->pcsr_row_ptr,
                          &meta->pcsr_col_idx, &meta->pcsr_capacity);
      free(tmp_rp);
      free(tmp_ci);
      if (meta->current_layout == LAYOUT_CSR) {
        meta->csr_row_ptr = NULL;
        meta->csr_col_idx = NULL;
      }
    } else if (target_layout == LAYOUT_BCSR) {
      meta->bcsr_block_size = 64;
      convert_csr_to_bcsr(n, tmp_m, tmp_rp, tmp_ci, &meta->bcsr_brow_ptr,
                          &meta->bcsr_bcol_idx, 64, &meta->bcsr_nblocks);
      free(tmp_rp);
      free(tmp_ci);
      if (meta->current_layout == LAYOUT_CSR) {
        meta->csr_row_ptr = NULL;
        meta->csr_col_idx = NULL;
      }
    }
    meta->current_layout = target_layout;
  }

  /* Final Step: write active array view into struct.Graph. */
  char *base = (char *)graph_ptr;

  if (meta->current_layout == LAYOUT_CSR) {
    *((int64_t **)(base + 16)) = meta->csr_row_ptr;
    *((int32_t **)(base + 24)) = meta->csr_col_idx;
    *((int64_t *)(base + 8)) = meta->csr_m;
  } else if (meta->current_layout == LAYOUT_PCSR) {
    *((int64_t **)(base + 16)) = meta->pcsr_row_ptr;
    *((int32_t **)(base + 24)) = meta->pcsr_col_idx;
  } else if (meta->current_layout == LAYOUT_BCSR) {
    /* Traversal/compute are constrained to CSR, so expose no CSR arrays here. */
    *((int64_t **)(base + 16)) = NULL;
    *((int32_t **)(base + 24)) = NULL;
  }
}

int32_t autograph_get_layout(void *graph_ptr) {
  for (int i = 0; i < g_meta_count; i++) {
    if (g_meta[i].graph_ptr == graph_ptr)
      return g_meta[i].current_layout;
  }
  return LAYOUT_SET; /* assumed SET if not tracked yet */
}

void autograph_debug_print(void *graph_ptr) {
  AutoGraphMeta *meta = NULL;
  for (int i = 0; i < g_meta_count; i++) {
    if (g_meta[i].graph_ptr == graph_ptr) {
      meta = &g_meta[i];
      break;
    }
  }
  if (!meta) {
    return;
  }
}

void autograph_update_csr_pointers(void *graph_ptr, int64_t *row_ptr,
                                   int32_t *col_idx) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  if (meta) {
    meta->csr_row_ptr = row_ptr;
    meta->csr_col_idx = col_idx;
  }
}

/* ══════════════════════════════════════════════════════════════════
 *  Initial Registration (Called once by compiled IR)
 * ══════════════════════════════════════════════════════════════════ */
void autograph_init(void *graph_ptr, int64_t n, int64_t m, 
                    void *nodes_bmp, void *edges_bmp, 
                    void *edge_pairs_table) 
{
    AutoGraphMeta *meta = find_or_create_meta(graph_ptr);
    if (!meta) return;
    
    meta->nodes_bitmap = nodes_bmp;
    meta->edges_bitmap = edges_bmp;
    meta->edge_pairs_table = edge_pairs_table;
    meta->csr_n = n;
    meta->csr_m = m;
    meta->current_layout = LAYOUT_SET;
    
    /* Build CSR immediately so struct.Graph has valid row_ptr/col_idx for BFS.
     * (ensure_layout may not run if ADCE removes it or autotuner skips injection.) */
    build_csr_from_set(n, edges_bmp, edge_pairs_table,
                       &meta->csr_row_ptr, &meta->csr_col_idx, &meta->csr_m);
    meta->current_layout = LAYOUT_CSR;
    
    char *base = (char *)graph_ptr;
    *((int64_t **)(base + 16)) = meta->csr_row_ptr;
    *((int32_t **)(base + 24)) = meta->csr_col_idx;
    *((int64_t *)(base + 8)) = meta->csr_m;

}
