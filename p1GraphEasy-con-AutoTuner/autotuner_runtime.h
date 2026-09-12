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

  /* Reverse (in-)adjacency for directed traversal: in_row_ptr/in_col_idx
   * mirror the Graph struct's transpose cells; built on demand by
   * autograph_ensure_transpose.  For undirected graphs in_row_ptr == NULL
   * (the forward CSR is symmetric). */
  int64_t *in_row_ptr;
  int32_t *in_col_idx;

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
  int32_t *scratch_cur_frontier;  /* dense current-frontier buffer, size csr_n */
  int32_t *scratch_next_frontier; /* dense next-frontier buffer, size csr_n */
  int32_t *scratch_dest_seen;     /* per-round append flags, size csr_n */

  /* Round-separation shadow buffers (in-place relax / round-separated loops).
   * One byte buffer per slot, lazily sized to the largest request (the round
   * preheader asks for csr_n * element-bytes); the compiler's emitted shadow
   * construction memcpy's the live array into these each round so the pair
   * work function reads a frozen round-start snapshot. */
  uint8_t *scratch_shadow[4];
  int64_t scratch_shadow_bytes[4];

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

  /* Graptor CleanCut partitions (owner-computes rule).  When built, every
   * destination v has a home partition p = homePartition[v]; all edges whose
   * destination lies in p are stored in p's structures.  Parallel execution
   * assigns each partition to one worker, which is the guarantee that no two
   * workers ever write the same destination slot — for push AND pull.
   *
   *   partition_start[P+1] : destination range boundaries (contiguous home
   *                          ranges after the owner-computes assignment).
   *   push_rp[p]/push_ci[p] : per-partition CSR over the partition's source
   *                          rows (source ids listed in push_indir[p]).
   *   push_indir[p][i]      : global source id for row i of partition p.
   *   push_row_count[p]     : number of source rows in partition p.
   *
   * 0 < partition_count => built; 0 => fall back to the shared CSR walker.
   */
  int32_t partition_count;
  int64_t *partition_start;
  int64_t **push_rp;
  int32_t **push_ci;
  int32_t **push_indir;
  int64_t *push_row_count;

  /* Source-owned CleanCut slices (layout-native).  Partition p owns the
   * source range [partition_start[p], partition_start[p+1]); src_pairs[p] is
   * the partition's flat [(u,v),(u,v),...] arc list (2 int32 per pair) built
   * by enumerating WHATEVER layout the AutoTuner picked (CSR rows, PCSR rows
   * with GAP slots skipped, BCSR block decode, or the SET base pairs), so the
   * source step never touches the transient layout directly. */
  int32_t **src_pairs;
  int64_t *src_pair_count;
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

/* Build (once, O(E)) the reverse adjacency (transpose) for directed graphs so
 * that pull-style owner-computes traversal can scan in-edges.  For directed
 * layouts the transpose lives in the Graph struct (in_row_ptr/in_col_idx);
 * this mirrors those cells into the meta.  Returns 1 if usable, 0 otherwise. */
int autograph_ensure_transpose(void *graph_ptr);
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

/* ── Generic owner-computes frontier step (Graptor CleanCut model) ──
 *
 * Race-free for any per-pair work: destinations are partitioned into
 * contiguous home ranges (owner-computes rule); each partition is executed
 * serially on one worker, so exactly one thread ever writes a destination's
 * property slots.  The per-pair callback does the actual accumulation or
 * relaxation; it may WRITE destination-indexed slots and READ read-only
 * (previous-round) arrays.  No atomics required; exact, deterministic.
 *
 *   work_fn(source, destination, destination_index, env)
 *       - destination_index: 0-based position of `destination` in the
 *         partition's owned range (not needed by most kernels)
 *   membership: when non-NULL, an in-neighbor `u` is only visited if it is in
 *       the frontier (sparse-round behavior); NULL visits all in-neighbors.
 *   source_out: optional output set; if non-NULL, every (source, dest) pair
 *       visited appends source to the owning lane and appends to next_frontier
 *       exactly once per destination via env->next (an AutoFrontierSet*).
 *
 * Returns the number of destinations appended to next_frontier.
 */
typedef void (*sgpl_frontier_pair_fn)(int32_t source, int32_t destination,
                                      int64_t destination_index, void *env);

int32_t autograph_frontier_step_owner(void *graph_ptr,
                                      const int32_t *frontier,
                                      int32_t frontier_size,
                                      sgpl_frontier_pair_fn work_fn,
                                      void *work_env,
                                      const uint8_t *membership,
                                      int32_t *next_frontier,
                                      int32_t initial_next_size,
                                      int32_t *dest_seen);

/* Graptor CleanCut support (owner-computes rule).
 *
 * Build (idempotent, O(E)) the per-partition structures used by the race-free
 * push traversal: each partition owns a contiguous destination range, and all
 * edges whose destination falls in that range are grouped by source in
 * push_rp/ci/indir so that a worker scanning its own partition never writes a
 * foreign destination.  partitions <= 0 picks a default (a small multiple of
 * the worker count).  Returns the partition count built.
 */
int32_t autograph_build_clean_cut(void *graph_ptr, int32_t partitions);

int32_t autograph_home_partition_of(void *graph_ptr, int32_t destination);

/* Debug: dump the built CleanCut partition structures to stderr. */
void autograph_debug_dump_clean_cut(void *graph_ptr);

/* Race-free push traversal under CleanCut partitions: each partition is
 * executed serially by one worker; for each source row, if the source is in
 * the frontier (membership non-NULL) then work_fn(source, dest, ...) is called
 * for every dest in the partition.  All dests are home-owned by this worker,
 * hence no races without atomics. */
int32_t autograph_frontier_step_owner_push(void *graph_ptr,
                                           const int32_t *frontier,
                                           int32_t frontier_size,
                                           sgpl_frontier_pair_fn work_fn,
                                           void *work_env,
                                           const uint8_t *membership,
                                           int32_t *next_frontier,
                                           int32_t initial_next_size,
                                           int32_t *dest_seen);

/* Source-owned variant: partition p owns the SOURCE range [p*n/P,(p+1)*n/P),
 * so work_fn pairs are dispatched with each source's own CSR row.  Used for
 * loops whose write target is indexed by the source vertex (owner-computes on
 * the source side).  Same ABI and frontier behaviour as the push variant. */
int32_t autograph_frontier_step_owner_source(void *graph_ptr,
                                             const int32_t *frontier,
                                             int32_t frontier_size,
                                             sgpl_frontier_pair_fn work_fn,
                                             void *work_env,
                                             const uint8_t *membership,
                                             int32_t *next_frontier,
                                             int32_t initial_next_size,
                                             int32_t *dest_seen);

/* Reduction step with per-partition partials (owner-computes, destination-
 * owned): every partition accumulates its pair work into its own partial at
 * work_env + p * partial_bytes; once all partitions finish, combine_fn folds
 * each partial into `out` in ascending partition order.  The zeroing of the
 * partials and the allocation are the caller's (compiler pass) job. */
typedef void (*sgpl_frontier_combine_fn)(const void *partial, void *out);
int32_t autograph_frontier_step_owner_red(void *graph_ptr,
                                          const int32_t *frontier,
                                          int32_t frontier_size,
                                          sgpl_frontier_pair_fn work_fn,
                                          void *work_env,
                                          int64_t partial_bytes,
                                          sgpl_frontier_combine_fn combine_fn,
                                          void *out,
                                          const uint8_t *membership,
                                          int32_t *next_frontier,
                                          int32_t initial_next_size,
                                          int32_t *dest_seen);

/* Frontier envelope helpers for CleanCut (array- and set-based BFS/SSSP).
 * dest_seen is zeroed each prepare; membership is filled from the current
 * frontier.  work_fn requests an append by storing 1 into dest_seen[v]; the
 * step then packs those destinations into next_frontier. */
int32_t *autograph_scratch_dest_seen(void *graph_ptr);
int32_t *autograph_scratch_next_frontier(void *graph_ptr);
uint8_t *autograph_scratch_membership(void *graph_ptr);
void *autograph_scratch_shadow(void *graph_ptr, int64_t bytes, int32_t slot);
int32_t autograph_prepare_frontier_array(void *graph_ptr,
                                         const int32_t *frontier,
                                         int32_t frontier_size);
int32_t autograph_prepare_frontier_bitmap(void *graph_ptr, void *frontier_bitmap);
void autograph_commit_frontier_bitmap(void *graph_ptr, void *next_bitmap,
                                      int32_t new_size);

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
