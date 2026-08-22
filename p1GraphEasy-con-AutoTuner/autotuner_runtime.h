/*
 * autotuner_runtime.h — Phase-Aware Graph Layout AutoTuner Runtime
 *
 * Provides layout conversion functions for the Set-Based Architecture.
 * LAYOUT_SET (Roaring bitamps) is the absolute base.
 * CSR, PCSR, and BCSR are transient layouts built on-demand and 
 * destroyed after use.
 */

#ifndef AUTOTUNER_RUNTIME_H
#define AUTOTUNER_RUNTIME_H

#include <stdint.h>

/* ── Layout Tags ─────────────────────────────────────────────────── */
#define LAYOUT_CSR  0
#define LAYOUT_PCSR 1
#define LAYOUT_BCSR 2
#define LAYOUT_SET  3

/* ── Per-graph layout metadata (maintained by the runtime) ─────── */
typedef struct {
  void *graph_ptr;        /* pointer to the original struct.Graph   */
  int32_t current_layout; /* one of LAYOUT_*                        */

  /* Base Representation (always valid when LAYOUT_SET, or snapshotted) */
  void *nodes_bitmap;
  void *edges_bitmap;
  void *edge_pairs_table; /* Pairs of (u, v) indexed by edge ID     */
  int64_t static_pair_count;

  /* Extra canonical edges beyond static table (label pairs). */
  int32_t *extra_edge_pairs; /* flat [u0,v0,u1,v1,...] */
  uint8_t *extra_edge_live;  /* 1 if active */
  int64_t extra_edge_count;
  int64_t extra_edge_capacity;
  int32_t canonical_dirty;

  /* Incrementally tracked live undirected edge count (avoids O(m) scan). */
  int64_t live_edge_count;

  /* Transient CSR arrays (allocated on demand) */
  int64_t *csr_row_ptr;
  int32_t *csr_col_idx;
  int32_t *csr_weights;   /* optional; aligned with csr_col_idx when weighted */
  int64_t csr_n;
  int64_t csr_m;
  int32_t csr_owned;      /* 0 = borrowed from graph loader, 1 = autotuner-allocated */

  /* Transient PCSR arrays (allocated on demand) */
  int64_t *pcsr_row_ptr;
  int32_t *pcsr_col_idx;
  int64_t pcsr_capacity; 

  /* Transient BCSR arrays (allocated on demand) */
  int32_t *bcsr_brow_ptr; 
  int32_t *bcsr_bcol_idx; 
  int32_t bcsr_block_size;
  int32_t bcsr_nblocks;

  /* Bumped on every layout convert so live neighbor iterators can detect
   * that their snapshot of CSR/PCSR/BCSR arrays is no longer valid. */
  int32_t layout_epoch;

  /* Persistent frontier scratch (BFS + motif steps). Owned by runtime. */
  void *scratch_lanes; /* AutoFrontierLane* */
  int32_t scratch_lane_count;
  int64_t scratch_n;
  uint8_t *scratch_membership;
  uint8_t *scratch_round_member;
  int64_t *scratch_offsets;
  int32_t scratch_offsets_cap;

  /* Analytic per-op-class RD tiers (Phase 3): per directed insert
   * (N, h2, h3) for {scan, move, brow, struct}, computed by the compiler
   * from the edge file (mirrors analytic_rd.py).  Used to keep runtime-side
   * profiles/predictions consistent with the compile-time cost model. */
  double class_tiers[12];
  uint8_t has_class_tiers;

  /* CSR analytic per-op-class RD tiers (Phase 3 extension): same payload for
   * the CSR structure (move/brow/struct; scan unused), mirrors AnalyticCSR. */
  double csr_class_tiers[12];
  uint8_t has_csr_class_tiers;
} AutoGraphMeta;

#ifdef __cplusplus
extern "C" {
#endif

/* ── Primary API (called by injected IR) ─────────────────────────
 *
 *  graph_ptr  : pointer to existing struct.Graph (unchanged layout)
 *  n, m       : vertex / edge counts 
 *  row_ptr    : pointer to the row_ptr field *inside* struct.Graph
 *  col_idx    : pointer to the col_idx field *inside* struct.Graph
 *  nodes_bmp  : pointer to RoaringBitmap of nodes
 *  edges_bmp  : pointer to RoaringBitmap of edges
 *  edge_pairs : pointer to array of Edge pair structs
 *  target     : LAYOUT_*
 */
void autograph_ensure_layout(void *graph_ptr, int64_t n, int64_t m,
                             int64_t *row_ptr, int32_t *col_idx,
                             void *nodes_bmp, void *edges_bmp, 
                             void *edge_pairs,
                             int32_t target_layout);

int32_t autograph_get_layout(void *graph_ptr);
void autograph_debug_print(void *graph_ptr);
int autograph_get_meta_handles(void *graph_ptr, void **nodes_bmp, void **edges_bmp,
                               void **edge_pairs);
int autograph_canonical_add_node(void *graph_ptr, int32_t node_label);
int autograph_canonical_remove_node(void *graph_ptr, int32_t node_label);
int autograph_canonical_add_edge(void *graph_ptr, int32_t u, int32_t v);
int autograph_canonical_remove_edge(void *graph_ptr, int32_t u, int32_t v);
void autograph_mark_canonical_dirty(void *graph_ptr);
void autograph_record_adjacency_state(void *graph_ptr, int64_t n, int64_t m,
                                      int64_t *row_ptr, int32_t *col_idx);
void autograph_sync_canonical_if_dirty(void *graph_ptr);
void autograph_profile_region_enter(int32_t region_id, int32_t kind,
                                    int32_t layout, double predicted_ns);
void autograph_profile_region_exit(int32_t region_id);
void autograph_profile_record_kernel_ns(int32_t kind, uint64_t elapsed_ns);

/* Notify runtime after CSR realloc (e.g. from csr_add_directed) */
void autograph_update_csr_pointers(void *graph_ptr, int64_t *row_ptr, int32_t *col_idx);

/* Option B: Convert to SET from current layout (rebuild bitmaps from adjacency).
 * Call from mutation path when an out-of-bounds op would require bitmaps. */
void autograph_ensure_layout_set(void *graph_ptr);

/* ── Representation-agnostic neighbor iterator ────────────────────
 *
 * Iterates over neighbors of vertex u under the current layout
 * (CSR/PCSR/BCSR/SET) without materializing an intermediate buffer.
 * Usage:
 *   AutoNeighborIter iter;
 *   autograph_neighbor_iter_init(graph_ptr, u, &iter);
 *   int32_t v;
 *   while (autograph_neighbor_iter_next(&iter, &v)) { ... }
 */
typedef struct {
  void *meta;     /* AutoGraphMeta* */
  int64_t u;      /* source vertex */
  int64_t pos;    /* current iteration position */
  int64_t end;    /* end position */
  int32_t state;  /* layout-specific (BCSR local_row, SET phase) */
  int32_t layout; /* LAYOUT_* snapshotted at init */
  int32_t epoch;  /* meta->layout_epoch at init */
  int32_t pad;
} AutoNeighborIter;

void autograph_neighbor_iter_init(void *graph_ptr, int64_t u,
                                  AutoNeighborIter *iter);
int  autograph_neighbor_iter_next(AutoNeighborIter *iter, int32_t *out_v);

/* Execute one structurally-verified first-wins frontier step.  The runtime
 * automatically chooses sparse push or dense pull and traverses the active
 * graph layout directly (without materializing neighbor arrays). */
int32_t autograph_frontier_step(void *graph_ptr,
                                const int32_t *frontier,
                                int32_t frontier_size,
                                int32_t *next_frontier,
                                int32_t initial_next_size,
                                int32_t *claim,
                                int32_t expected,
                                int32_t desired,
                                int32_t *parent);

/* Compositional frontier operators (internal combine kinds). */
enum {
  SGPL_COMBINE_CAS_FIRST = 0,
  SGPL_COMBINE_MIN_COPY = 1,
  SGPL_COMBINE_PEEL_K = 2,
  SGPL_COMBINE_MIN_WEIGHTED = 3
};

/* Unified EdgeMap: sparse push or owner-computes pull over array frontier.
 *   CAS_FIRST:     prop0=claim, prop1=parent, scalar0=expected, scalar1=desired
 *   MIN_COPY:      prop0=labels (min copy prop[u] into prop[v])
 *   MIN_WEIGHTED:  prop0=dist (min prop[u]+w into prop[v]); CSR weights required
 *   PEEL_K:        prop0=alive, prop1=deg, scalar0=k */
int32_t autograph_edgemap(void *graph_ptr,
                          int32_t combine,
                          const int32_t *frontier,
                          int32_t frontier_size,
                          int32_t *next_frontier,
                          int32_t initial_next_size,
                          int32_t *prop0,
                          int32_t *prop1,
                          int32_t scalar0,
                          int32_t scalar1);

/* Generalized array-frontier motif step (non-BFS). Modes:
 *   SGPL_MOTIF_WRITE_MIN           — RelaxMin: prop[v] = min(prop[v], prop[u])
 *   SGPL_MOTIF_PEEL_K              — prop0 = alive, prop1 = deg, scalar = k
 *   SGPL_MOTIF_RELAX_MIN_WEIGHTED  — RelaxMin: prop[v] = min(prop[v], prop[u]+w)
 * Thin wrappers over autograph_edgemap for ABI compatibility. */
enum {
  SGPL_MOTIF_WRITE_MIN = 1,
  SGPL_MOTIF_PEEL_K = 2,
  SGPL_MOTIF_RELAX_MIN_WEIGHTED = 3
};

int32_t autograph_motif_frontier_step(void *graph_ptr,
                                      int32_t mode,
                                      const int32_t *frontier,
                                      int32_t frontier_size,
                                      int32_t *next_frontier,
                                      int32_t initial_next_size,
                                      int32_t *prop0,
                                      int32_t *prop1,
                                      int32_t scalar);

/* BCSR-native edge mutation. Returns 1 on success, 0 on failure/skip. */
int autograph_bcsr_add_edge(void *graph_ptr, int32_t from, int32_t to);
int autograph_bcsr_remove_edge(void *graph_ptr, int32_t from, int32_t to);

/* ── Low-level conversion helpers ── */
void build_csr_from_set(int64_t n, int64_t pair_count, void *edges_bitmap, void *edge_pairs,
                        int64_t **out_rp, int32_t **out_ci, int64_t *out_m);

void rebuild_sets_from_csr(int64_t n, int64_t pair_count, int64_t m, const int64_t *csr_rp,
                           const int32_t *csr_ci, void *nodes_bitmap, 
                           void *edges_bitmap, void *edge_pairs);

void convert_csr_to_pcsr(int64_t n, int64_t m, const int64_t *csr_rp,
                         const int32_t *csr_ci, int64_t **out_rp,
                         int32_t **out_ci, int64_t *out_capacity);

void convert_csr_to_bcsr(int64_t n, int64_t m, const int64_t *csr_rp,
                         const int32_t *csr_ci, int32_t **out_brow,
                         int32_t **out_bcol, int32_t block_size,
                         int32_t *out_nblocks);

void convert_pcsr_to_csr(int64_t n, int64_t pcsr_cap, const int64_t *pcsr_rp,
                         const int32_t *pcsr_ci, int64_t **out_rp,
                         int32_t **out_ci, int64_t *out_m);

void convert_bcsr_to_csr(int64_t n, int32_t nblocks, int32_t block_size,
                         const int32_t *bcsr_brow, const int32_t *bcsr_bcol,
                         int64_t **out_rp, int32_t **out_ci, int64_t *out_m);

#ifdef __cplusplus
}
#endif

#endif /* AUTOTUNER_RUNTIME_H */

/* Register a new graph with the Set-Base Architecture */
void autograph_init(void *graph_ptr, int64_t n, int64_t m, 
                    void *nodes_bmp, void *edges_bmp, 
                    void *edge_pairs_table);

/* Phase 3: attach analytic per-op-class RD tiers computed at compile time
 * (mirrors analytic_rd.py).  tiers must point to 12 doubles in
 * (N, h2, h3) × {scan, move, brow, struct} order; NULL clears. */
void autograph_set_class_tiers(void *graph_ptr, const double *tiers);

/* Phase 3 extension: attach the CSR analytic per-op-class RD tiers
 * (mirrors analytic_rd.py AnalyticCSR; scan unused). */
void autograph_set_class_tiers_csr(void *graph_ptr, const double *tiers);
