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

  /* OWNER_V source coverage: partition-local scan compacts each partition's
   * membership-gated source range into scratch_coverage_members at the
   * partition's slice (partition_start[p] .. partition_start[p+1]) and records
   * the member count; a deterministic ascending walk then emits the
   * source_begin/source_end callbacks.  Lazily grown to csr_n / partition_count
   * and kept for the process lifetime, like the other scratch buffers. */
  int32_t *scratch_coverage_members;
  int64_t scratch_coverage_members_cap;
  int32_t *scratch_coverage_counts;
  int32_t scratch_coverage_counts_cap;

  /* Round-separation shadow buffers (in-place relax / round-separated loops).
   * One byte buffer per slot, lazily sized to the largest request (the round
   * preheader asks for csr_n * element-bytes); the compiler's emitted shadow
   * construction memcpy's the live array into these each round so the pair
   * work function reads a frozen round-start snapshot. */
  uint8_t *scratch_shadow[4];
  int64_t scratch_shadow_bytes[4];

  /* Composition R3: per-partition private copies for *privatized* in-loop
   * updates.  When a base cannot be given a single owner region -- a
   * data-valued subscript (`cnt[deg[u]] += 1`) or one base written through
   * both endpoint regions (`arr[u] += 1; arr[v] += 1`) -- but every store to
   * it is a recognized U_⊕ update, each partition accumulates into its own
   * copy and the emitted combine folds the copies with the operator's own
   * combine.  partition p's copy starts at
   * priv_buf[s] + p * priv_stride[s], `priv_stride[s]` being
   * elems * elem_bytes.  Slot-indexed (a step may privatize several bases);
   * lazily sized and re-initialized to the operator identity on every bind,
   * so a round in a driver loop does not pay an allocation. */
  uint8_t *priv_buf[4];
  int64_t priv_bytes[4];
  int64_t priv_stride[4];

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

  /* Per-graph CleanCut cache identity.  The cache lock protects rebuilding
   * and these fields; independent graphs must not evict one another. */
  int32_t cc_cache_valid;
  int32_t cc_cache_partitions;
  int32_t cc_cache_layout_epoch;
  int64_t cc_cache_live_edges;
  int64_t cc_cache_extra_edges;
  int64_t cc_ranges_n; /* vertex-span identity for range-only array cuts */
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

typedef void (*sgpl_frontier_pair_fn)(int32_t source, int32_t destination,
                                      int64_t destination_index, void *env);

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
/* Compiler-emitted stage scope: pin the selected partitions through setup,
 * executor, and commit.  The calls must be paired on the same task thread. */
int32_t autograph_pin_clean_cut(void *graph_ptr, int32_t partitions);
void autograph_unpin_clean_cut(void);

int32_t autograph_home_partition_of(void *graph_ptr, int32_t destination);

/* Debug: dump the built CleanCut partition structures to stderr. */
void autograph_debug_dump_clean_cut(void *graph_ptr);




/* Pair-wise folder used by reduction-style callbacks (partial, out). */
typedef void (*sgpl_frontier_combine_fn)(const void *partial, void *out);

/* Frontier envelope helpers for CleanCut (array- and set-based BFS/SSSP).
 * dest_seen is zeroed each prepare; membership is filled from the current
 * frontier.  The activation primitive claims dest_seen and packs the
 * transitioned destinations into next_frontier. */
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

/* -- Composable runtime execution (R1) ------------------------------------
 *
 * The compiler-side effect expression is interpreted into a flat execution
 * context plus an array of executable operation descriptors.  The runtime
 * contains no expression tree: the executor traverses the selected domain and
 * dispatches lifecycle events to the operations that declare the matching
 * capability.  Activation, reduction, source finalization, snapshots and
 * claims are operations, never engine behavior. */

/* Traversal mechanisms (execution mechanisms, not strategy classes). */
enum {
  SGPL_TRAVERSE_OWNER_U = 0, /* source-owned flat slices (src_pairs) */
  SGPL_TRAVERSE_OWNER_V = 1  /* destination-owned rows (push_rp/ci/indir) */
};

/* Domain kinds (metadata; the Frontier gate itself is `membership`). */
enum {
  SGPL_DOMAIN_ALL_VERTICES = 0,
  SGPL_DOMAIN_FRONTIER = 1,
  SGPL_DOMAIN_NEIGHBORS = 2
};

/* Operation capabilities: which lifecycle callbacks the operation implements. */
enum {
  SGPL_OP_PAIR = 1u << 0,
  SGPL_OP_SOURCE_BEGIN = 1u << 1,
  SGPL_OP_SOURCE_END = 1u << 2,
  SGPL_OP_PARTITION_BEGIN = 1u << 3,
  SGPL_OP_PARTITION_END = 1u << 4,
  SGPL_OP_ROUND_BEGIN = 1u << 5,
  SGPL_OP_ROUND_END = 1u << 6,
  SGPL_OP_COMBINE = 1u << 7,
  SGPL_OP_SNAPSHOT = 1u << 8, /* RoundBegin -> Snapshot(A) -> CrossRead(A) */
  SGPL_OP_NO_SOURCE_ORDINAL = 1u << 10 /* compiler proves no source-ordinal or source-gate access in pair body */
};

/* Resource classes. */
enum {
  SGPL_RES_MEMBERSHIP = 1u << 0,
  SGPL_RES_DEST_SEEN = 1u << 1,
  SGPL_RES_NEXT_FRONTIER = 1u << 2,
  SGPL_RES_SNAPSHOT = 1u << 3,
  SGPL_RES_PARTIAL = 1u << 4,
  SGPL_RES_PRIVATE = 1u << 5,
  SGPL_RES_CLAIM = 1u << 6
};

/* Resource access modes. */
enum {
  SGPL_ACCESS_READ = 0,
  SGPL_ACCESS_WRITE = 1,
  SGPL_ACCESS_ATOMIC_WRITE = 2,
  SGPL_ACCESS_PRIVATE = 3
};

typedef struct sgpl_res_access {
  uint32_t resource; /* SGPL_RES_* */
  uint8_t mode;      /* SGPL_ACCESS_* */
} sgpl_res_access;

struct sgpl_exec_ctx;

/* One executable operation.  This is an execution ABI object, not an
 * expression: no child pointers, no SEQ/PAR structure.
 *
 * Hot callbacks (`pair`, `combine`) receive the operation's `state` directly;
 * the worker-local environment is derived inside the callback (for compiler-
 * emitted code the callback IS the cloned work function, so this keeps the
 * per-pair call depth at one indirect call).  Lifecycle callbacks receive the
 * descriptor so they can reach both state and capabilities. */
typedef struct sgpl_runtime_op {
  uint64_t capabilities;            /* SGPL_OP_* */
  uint32_t resource_count;
  const sgpl_res_access *resources; /* borrowed from the compiler */
  void *state;

  void (*round_begin)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *);
  /* Snapshot(A): published once per round, after the round-begin ops and before
   * any traversal.  The callback produces the snapshot resource (e.g. via
   * autograph_snapshot_publish) into the operation's state. */
  void (*snapshot)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *);
  void (*round_end)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *);
  void (*partition_begin)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *,
                          int32_t partition_id);
  void (*source_begin)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *,
                       int32_t u);
  void (*pair)(void *state, struct sgpl_exec_ctx *, int32_t u, int32_t v);
  void (*source_end)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *,
                     int32_t u);
  void (*partition_end)(struct sgpl_runtime_op *, struct sgpl_exec_ctx *,
                        int32_t partition_id);
  void (*combine)(void *state, struct sgpl_exec_ctx *);
} sgpl_runtime_op;

enum { SGPL_EXECUTION_ABI_VERSION = 4 };

/* A symbolic effect-derived schedule. The number of owners remains dynamic;
 * the compiler proves the structure and callback context independence. */
#define SGPL_STATIC_SCHEDULE_VERSION 1u
#define SGPL_SCHEDULE_PREPARE_OWNERS_JOIN 1u
#define SGPL_SCHEDULE_EFFECT_PROOF 0x1u
#define SGPL_SCHEDULE_CONTEXT_PROOF 0x2u
typedef struct sgpl_static_schedule_v1 {
  uint32_t version, kind, proofs, obligations;
  void *pair;
  void *source_begin;
} sgpl_static_schedule_v1;
void autograph_execution_abi_check_v1(int32_t version, uint64_t context_bytes, uint64_t operation_bytes);

/* A static witness selects grouping. CleanCut only materializes this rule. */
enum sgpl_spatial_group {
  SGPL_SPATIAL_NONE = 0,
  SGPL_SPATIAL_PAIR_OCCURRENCE = 1,
  SGPL_SPATIAL_DESTINATION = 2,
  SGPL_SPATIAL_SOURCE = 3
};

/* Flat execution context for one stage.  Round resources (membership,
 * dest_seen, next_frontier, append head, round state) are shared by all stages
 * of one round; the compiler owns their lifetime. */
typedef struct sgpl_exec_ctx {
  void *graph;
  uint64_t round_id;
  int32_t domain_kind;    /* SGPL_DOMAIN_* */
  int32_t traversal_kind; /* SGPL_TRAVERSE_* */
  const uint8_t *membership;
  int32_t *dest_seen;
  int32_t *next_frontier;
  int32_t initial_next_size;
  int32_t *append_head; /* atomic counter written by activation ops */
  void *round_state;    /* snapshot/envelope state owned by the round */
  struct sgpl_runtime_op *ops;
  uint32_t op_count;
  void *partition_base; /* per-partition partial array base (combine) */
  int64_t partition_stride;
  void *partition_state; /* current partition slot, set by the executor */
  /* Source-scoped claim channel (R7): the executor zeroes this at every source
   * change before dispatching source_begin; a SourceBegin op (claim) stores its
   * per-source result here and pair callbacks observe it. */
  void *source_state;
  void *env;
  int32_t next_size;       /* out: initial_next_size + appended */
  int32_t run_round_begin; /* owned by the first stage of a round */
  int32_t run_round_end;   /* owned by the last stage of a round */
  /* TDG site id for this stage (compiler-assigned, shared id space with the
   * outlined loops).  -1 disables the TDG path: the stage dispatches exactly
   * as before.  When >= 0 the executor plans the stage like any other parallel
   * site: a bounded calibration of honest serial passes, then a single-site
   * level whose plan sets the dispatch width. */
  int32_t step_id;
  /* Per-step schedule (theorem section 17), set by the compiler through
   * autograph_exec_ctx_set_dag_axes: which axis this stage may run
   * concurrently (1) and which is held in order (0).  The dispatch sites read
   * it instead of a global env switch; SGPL_DAG_SPATIAL=0 stays a kill
   * switch.  Legacy callers leave this 0 = no schedule declaration. */
  int32_t dag_spatial;
  int32_t dag_temporal;
  int32_t temporal_scope; /* sgpl_temporal_scope. A flag does not select scope. */
  int32_t spatial_group; /* sgpl_spatial_group: static proof witness */
  int32_t spatial_runtime_group; /* CleanCut coarsening of the witness */
  int32_t spatial_partition; /* worker-local batch id */
  uint64_t spatial_occupied_groups;
  int32_t spatial_nontrivial; /* set only after counting actual work */
  int32_t spatial_coverage_pending; /* source preparation node before batches */
  uint8_t *spatial_groups_seen; /* endpoint groups; each has one batch owner */
  int32_t execution_status;
  int32_t *execution_error; /* shared cancellation/failure slot */
  void *spatial_frontier_lanes; /* invocation-owned AutoFrontierLane[] */
  int32_t *spatial_frontier_storage;
  int64_t *spatial_frontier_offsets;
  uint8_t *spatial_membership;
  uint8_t *spatial_source_gate;
  int32_t *spatial_seen;
  int32_t *spatial_lane_seen; /* source-owned frontier deduplication per batch */
  void **spatial_snapshots;
  int64_t *spatial_snapshot_bytes;
  int32_t spatial_snapshot_count;
  uint8_t *round_membership; /* reserved before setup, filled by this round */
  int32_t *round_dest_seen;
  int32_t *round_next_frontier;
  void *spatial_claims; /* borrowed source-frame storage */
  /* Immutable execution records. Built before dispatch, owned by this context. */
  int32_t *gpu_records; /* triples: source, destination, lifecycle kind */
  int64_t *gpu_record_offsets;
  int64_t gpu_record_count;
  int32_t gpu_partition_count;
  int64_t gpu_vertex_count;
  uint32_t gpu_private_scalars, gpu_private_arrays;
  void *gpu_private_targets;
  const uint64_t *gpu_private_element_bytes;
  uint64_t gpu_claim_bytes;
  /* Whole proof groups, in stable occurrence order within each group. */
  int64_t *gpu_group_offsets;
  int64_t *gpu_partition_groups;
  int64_t gpu_group_count;
  int32_t gpu_parallel_groups;
  uint64_t *gpu_group_times; /* optional: begin, end, lane for each proof group */
  int32_t gpu_local_partials; /* source accumulator storage belongs to a lane */
  int32_t gpu_parallel_fold; /* device updates honor the accepted private fold */
  /* Ordered array frontiers retain each source and pair occurrence. */
  const int32_t *array_sources;
  int64_t array_source_count;
  int32_t **array_output_slot;
  int64_t *array_output_capacity;
  int32_t array_append_sites;
  int32_t array_append_seq;
  int64_t array_source_ordinal, array_work_ordinal;
  int64_t array_work_count;
  int32_t *array_append_log; /* validity, payload per occurrence and site */
  int64_t *gpu_record_sources, *gpu_record_work;
  /* CPU-only CSR hint, indexed to the first matching neighbor even when a row
   * is unsorted or has duplicates. Contexts own their copies of these hints. */
  int64_t *array_record_edges;
  int64_t array_edge_index;
  /* Exclusive execution-buffer lease; no live context shares its storage. */
  void *execution_scratch;
  int32_t array_log_pristine;
  int32_t array_pair_records;
  /* CPU pair batches: mode 1 is four i32s (u,v,source ordinal,work ordinal).
   * Mode 2 adds an i32 first-match hint after a proven CSR-index range bound.
   * Mode 0 keeps triples plus i64 side arrays. CPU compact modes expand
   * transactionally to the legacy representation before device grouping. */
  int32_t array_compact_records;
  const sgpl_static_schedule_v1 *static_schedule;
} sgpl_exec_ctx;
void autograph_exec_ctx_bind_static_schedule_v1(sgpl_exec_ctx *,
    const sgpl_static_schedule_v1 *, int32_t, uint64_t, uint64_t);
int32_t autograph_exec_ctx_edge_weight(sgpl_exec_ctx *,void *,int32_t,int32_t);
int32_t autograph_build_array_cut(void *,int32_t);
int32_t autograph_exec_ctx_bind_array_frontier(sgpl_exec_ctx *,const int32_t *,int32_t,
                                               int32_t **,int64_t *,int32_t);
int64_t autograph_exec_ctx_occurrence(sgpl_exec_ctx *);
int32_t sgpl_gpu_prepare_spatial_records(sgpl_exec_ctx *ctx);
int32_t sgpl_frontier_ensure_i32(int32_t **slot, int64_t *capacity, int64_t index);
int32_t sgpl_frontier_reserve_i32(void *graph, const int32_t *input, int32_t count,
                                 int32_t **slot, int64_t *capacity, int32_t sites,int32_t prefix);
void sgpl_frontier_release_buffers(void);
void autograph_exec_ctx_gpu_storage(sgpl_exec_ctx *, uint32_t, uint32_t,
                                   void *, const uint64_t *, uint64_t);

/* Prepare all invocation-owned state before user effects. These functions
 * return negative scheduler error codes. Execution never replays effects. */
int32_t autograph_exec_ctx_prepare_spatial(sgpl_exec_ctx *ctx, int32_t group,
                                          int32_t runtime_group,
                                          int32_t private_frontier);
int32_t autograph_exec_ctx_bind_snapshot(sgpl_exec_ctx *ctx, int32_t slot,
                                         const void *base, int64_t elem_bytes);
int32_t autograph_exec_ctx_bind_snapshot_range(sgpl_exec_ctx *ctx, int32_t slot,
                                               const void *base, int64_t count,
                                               int64_t elem_bytes);
void *autograph_exec_ctx_snapshot(sgpl_exec_ctx *ctx, int32_t slot);
void autograph_exec_ctx_set_claims(sgpl_exec_ctx *ctx, void *claims);
void *autograph_exec_ctx_claims(sgpl_exec_ctx *ctx);
void autograph_exec_ctx_set_source_gate(sgpl_exec_ctx *ctx, int32_t source, int32_t allowed);
void sgpl_exec_require_status(int32_t status);
int32_t autograph_exec_ctx_last_source(const sgpl_exec_ctx *ctx);
int32_t autograph_priv_bind_instance(void *graph, void *records, int64_t stride,
                                     int64_t offset, int64_t elements,
                                     int64_t element_bytes, int64_t identity,
                                     int32_t slot);
void autograph_priv_release_instance(void *records, int64_t offset,
                                      int32_t partitions);

/* Declare the stage's schedule (see the field comment above). */
void autograph_exec_ctx_set_dag_axes(sgpl_exec_ctx *ctx, int32_t spatial,
                                     int32_t temporal);
/* Test/debug accessor: 0 = not set, 1 = concurrent, -1 = ordered. */
int32_t sgpl_ctx_dag_spatial(const sgpl_exec_ctx *ctx);
int32_t sgpl_ctx_dag_temporal(const sgpl_exec_ctx *ctx);

/* Execute one stage: traverse the selected domain and dispatch the declared
 * lifecycle events to the operations.  Returns the next frontier size. */
int32_t autograph_frontier_execute(void *graph_ptr, sgpl_exec_ctx *ctx);
int32_t sgpl_exec_spatial_gpu(sgpl_exec_ctx *,const char *,const void *,uint32_t,
                             void *const *,uint32_t,int32_t);

/* Activation primitive: claim `dest_seen[v]` (0 -> 1, atomic) and, on the
 * transition, append v to next_frontier under the atomic head.  Returns 1 when
 * v was appended.  This is what an A+ operation's pair callback calls; the
 * executor itself never activates and never appends. */
int32_t autograph_frontier_activate(sgpl_exec_ctx *ctx, int32_t v);

/* Fork/join for incompatible parallel children.  `owner` is the enclosing
 * (owning) round context carrying the round lifecycle ops and flags; `a` and
 * `b` are non-owning child contexts executed concurrently on two host threads
 * with a join barrier.  Children must not carry run_round_begin/run_round_end
 * (abort otherwise).  Recursion: a nested Par subtree is realized by calling
 * fork/join again (from a helper the compiler emits); the nested call passes an
 * owner copy whose run_round_begin/run_round_end are false but which shares the
 * top owner's round resources (membership/dest_seen/next_frontier/append_head),
 * so round lifecycle runs exactly once per round.  Children whose traversal is
 * denied budget still make progress serially on their own thread (the budget
 * allocator never blocks), at any depth. */
int32_t autograph_frontier_fork_join(void *graph_ptr, sgpl_exec_ctx *owner,
                                     sgpl_exec_ctx *a, sgpl_exec_ctx *b);

/* Staged (ordered) realization of the same two-child structure: the owner's
 * round lifecycle and source coverage run exactly as in fork/join, but child
 * `a` completes before child `b` starts.  This is the realization for a dual
 * owner whose phases write the same base (R6): "all U before all V" is a sound
 * superset of the required per-element order, so the children keep their
 * internal parallelism while the cross-child order is preserved. */
int32_t autograph_frontier_staged(void *graph_ptr, sgpl_exec_ctx *owner,
                                  sgpl_exec_ctx *a, sgpl_exec_ctx *b);

/* -- layout-free construction ABI for compiler-emitted expressions --------
 * The compiler builds operation descriptors and execution contexts through
 * these helpers without knowing the C struct layouts.  Ownership:
 * autograph_exec_op_create allocates one descriptor; autograph_exec_ctx_create
 * consumes the descriptors (copies them into a contiguous array and frees the
 * originals) and allocates the context; autograph_exec_ctx_destroy frees both.
 * autograph_exec_op_state / autograph_exec_partition_state are the callback-
 * side accessors (avoiding layout knowledge in emitted code). */
void *autograph_exec_op_state(sgpl_runtime_op *op);
void *autograph_exec_partition_state(sgpl_exec_ctx *ctx);
void *autograph_exec_ctx_graph(sgpl_exec_ctx *ctx);
void autograph_exec_ctx_own_round(sgpl_exec_ctx *ctx, int32_t begin, int32_t end);

sgpl_runtime_op *autograph_exec_op_create(
    uint64_t capabilities, void *state,
    void (*pair_fn)(void *state, sgpl_exec_ctx *, int32_t, int32_t),
    void (*combine_fn)(void *state, sgpl_exec_ctx *),
    void (*source_begin_fn)(sgpl_runtime_op *, sgpl_exec_ctx *, int32_t),
    void (*source_end_fn)(sgpl_runtime_op *, sgpl_exec_ctx *, int32_t),
    void (*snapshot_fn)(sgpl_runtime_op *, sgpl_exec_ctx *),
    const sgpl_res_access *resources, uint32_t resource_count);

sgpl_exec_ctx *autograph_exec_ctx_create(
    void *graph, int32_t traversal_kind, int32_t domain_kind,
    const uint8_t *membership, int32_t *dest_seen, int32_t *next_frontier,
    int32_t initial_next_size, int32_t *append_head, void *partition_base,
    int64_t partition_stride, sgpl_runtime_op **ops, uint32_t op_count,
    int32_t step_id);

void autograph_exec_ctx_destroy(sgpl_exec_ctx *ctx);

/* Snapshot primitive (R7): copy `live_base` (n * elem_bytes, n from the graph)
 * into the invocation scratch snapshot slot and return the frozen buffer.  A
 * Snapshot op calls this in its round-phase callback and publishes the pointer
 * (its operation state) for the round's cross-reads.  The buffer lifecycle is
 * owned by the runtime. */
void *autograph_snapshot_publish(void *graph_ptr, const void *live_base,
                                 int64_t elem_bytes, int32_t slot,
                                 const char *name);

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

/* ── V1 generic ready-work DAG scheduler (two-axis spec, section 11) ──────
 * A template is a compact runtime-neutral descriptor: node count, relation
 * list, one node function.  A run allocates predecessor counters and a ready
 * ring for the actual runtime instances, dispatches ready nodes within the
 * worker budget, and releases formula-derived successors on completion.
 * Invalid descriptors, self edges and cycles fail closed. */
#define SGPL_DAG_OK 0
#define SGPL_DAG_ERR_INVALID (-1)
#define SGPL_DAG_ERR_CYCLE (-2)
#define SGPL_DAG_ERR_NODE (-3)
#define SGPL_DAG_ERR_NOMEM (-4)

#define SGPL_DAG_REL_PRECEDENCE 0u
#define SGPL_DAG_REL_FLAG_SEMANTIC 0x1u   /* carries an R1-R7 witness id */
#define SGPL_DAG_REL_FLAG_REALIZATION 0x2u /* deterministic order only */

/* §11.1 relation kinds.  PRECEDENCE is an executable dependence; MUTEX is a
 * mutual exclusion realized by a deterministic order (R6: U-owned before
 * V-owned) and must be tagged REALIZATION; SELF is a validation-only record of
 * a unary obligation (a footprint no partition can own) — a template carrying
 * one is rejected, because a surviving unary obligation means the axis has no
 * realizable DAG at all. */
#define SGPL_DAG_KIND_PRECEDENCE 0u
#define SGPL_DAG_KIND_MUTEX 1u
#define SGPL_DAG_KIND_SELF 2u

/* §11.1 axis mask: which proof graph the edge came from. */
#define SGPL_DAG_AXIS_SPATIAL 0x1u
#define SGPL_DAG_AXIS_TEMPORAL 0x2u
#define SGPL_DAG_AXIS_PRODUCT 0x4u

/* Relation kind of the underlying access relation (diagnostics). */
#define SGPL_DAG_RELKIND_MEMORY 0u
#define SGPL_DAG_RELKIND_RESOURCE 1u

typedef struct sgpl_dag_relation_desc {
  uint32_t kind;       /* SGPL_DAG_KIND_* */
  int64_t offset;      /* affine instance offset */
  int32_t source_node; /* predecessor */
  int32_t sink_node;   /* successor, released when the source completes */
  uint32_t witness_id; /* semantic edges: witness id, else 0 */
  uint32_t flags;      /* semantic / realization order tag */
  /* §11.1 proof metadata.  Appended so a compiler-emitted array from before
   * this extension stays layout-compatible for the fields above. */
  uint32_t axis_mask;     /* SGPL_DAG_AXIS_*; 0 = unspecified (accepted for
                           * descriptors built before this extension) */
  uint32_t relation_kind; /* SGPL_DAG_RELKIND_* (memory vs resource) */
  uint32_t access_kind;   /* 0 RAW, 1 WAR, 2 WAW */
  uint32_t discharge_id;  /* transformation that removed it; 0 = none.  A
                           * executable edge must be 0: a discharged relation
                           * is removed from the graph, not scheduled. */
  int32_t phase_from;     /* PhaseSegment projection (0..3), 0 = unknown */
  int32_t phase_to;
  int32_t round_offset;   /* 0 same round, +1 next round */
  int32_t partition_from; /* partition-domain tag, 0 = unknown */
  int32_t partition_to;
} sgpl_dag_relation_desc;

typedef struct sgpl_dag_run sgpl_dag_run;

typedef int32_t (*sgpl_dag_node_fn)(void *state, int64_t instance);

typedef struct sgpl_dag_template {
  int32_t node_count;
  int32_t relation_count;
  const sgpl_dag_relation_desc *relations;
  sgpl_dag_node_fn node_fn;
  /* Optional per-node work estimate (relative weights, e.g. per-partition pair
   * counts).  NULL = uniform.  Must hold node_count entries when set.  The
   * runtime uses it only as a CEILING on the node's nested-dispatch share,
   * never to raise it, so a wrong estimate can only under-grant (plan
   * section 15). */
  const int32_t *node_weights;
} sgpl_dag_template;

/* Execute a template for one runtime instance set.  Returns SGPL_DAG_OK or a
 * negative SGPL_DAG_ERR_* code; on failure new dispatch is cancelled, active
 * nodes are awaited, and the failure is returned. */
int32_t autograph_execute_dag(const sgpl_dag_template *template_desc,
                              void *state,
                              int32_t worker_budget);

/* Device work uses the same ready queue and dependency bookkeeping without
 * worker grants. Submit/poll return 0 for completion, 1 for pending work, or
 * a negative failure. Failure cancels new submissions and drains pending work.
 * Callbacks run on the caller. Poll must join failed work before returning. */
#define SGPL_DAG_PENDING 1
int32_t autograph_validate_dag(const sgpl_dag_template *template_desc);
int32_t autograph_execute_dag_async(const sgpl_dag_template *template_desc,
    void *state, sgpl_dag_node_fn submit, sgpl_dag_node_fn poll);
/* GPU traversal geometry is independent of CPU worker grants. */
int32_t autograph_gpu_clean_cut_partitions(void);
struct sgpl_gpu_plan;
/* The same node identities and relations must describe both realizations. */
int32_t sgpl_exec_dag_gpu(const struct sgpl_gpu_plan *device,
    sgpl_dag_node_fn cpu_node,void *cpu_state);

typedef enum sgpl_temporal_scope {
  SGPL_TEMPORAL_NONE = 0,
  SGPL_TEMPORAL_ROUND = 1,
  SGPL_TEMPORAL_SOURCE_PHASE = 2
} sgpl_temporal_scope;
void autograph_exec_ctx_set_temporal_scope(sgpl_exec_ctx *ctx,
                                           sgpl_temporal_scope scope);

/* Each callback executes one independent component in its original order.
 * This call joins all callbacks before it returns. A failure cancels pending
 * callbacks and joins active callbacks. The caller must not replay effects. */
int32_t sgpl_exec_temporal_components(sgpl_dag_node_fn component_fn,
                                     void *state, int32_t component_count,
                                     int32_t worker_budget);
int32_t sgpl_exec_temporal_gpu_components(sgpl_dag_node_fn component_fn,
    void *state, int32_t component_count, int32_t worker_budget,
    const char *kernel, uint64_t frame_bytes, const void *fields, uint32_t field_count,
    uint32_t temporal_scope);
/* Compiler dispatch uses the configured budget, including a parent grant. */
int32_t sgpl_temporal_worker_budget(void);
/* Graph construction must finish before concurrent callbacks read topology. */
int32_t sgpl_temporal_prepare_graph(void *graph);
/* Report a generated dispatch failure without replaying completed effects. */
void sgpl_temporal_dispatch_failed(int32_t status);

/* Step 7 (runtime half): run a frontier round's spatial partitions through
 * the ready-work DAG scheduler.  V1 template: one node per partition, no
 * relation edges (partitions independent); the compiler-side spatial template
 * will supply witness-backed edges in the next sub-step.  Each node invokes
 * the existing partition body, so all round lifecycle, snapshots, combines
 * and coverage stay exactly where they are. */
int32_t sgpl_exec_dag_spatial(sgpl_exec_ctx *ctx, int32_t partitions,
                              int32_t worker_budget);

/* §11.2: the same spatial dispatch with a real proof-derived graph.  `relations`
 * are the compiler's surviving spatial edges over partition indices (PRECEDENCE
 * or REALIZATION-tagged MUTEX); independent components run concurrently and a
 * precedence edge delays its sink until the source completed.  An invalid
 * descriptor fails closed (no dispatch). */
int32_t sgpl_exec_dag_spatial_graph(sgpl_exec_ctx *ctx, int32_t partitions,
                                    const sgpl_dag_relation_desc *relations,
                                    int32_t relation_count,
                                    int32_t worker_budget);

/* Step 8: temporal unit wrapper.  A temporal unit is one whole round of a
 * frontier nest (or one temporal stage of a multi-stage round); its wrapper
 * executes that unit's complete lifecycle through the existing
 * autograph_frontier_execute, so round-begin ops, snapshots, source coverage,
 * combines and round-end ops are never duplicated in the scheduler -- the
 * scheduler only orders and dispatches wrappers.
 *
 * V1 realization: a chain.  Unit i+1 starts only after unit i completed,
 * because round i+1's traversal reads round i's output (the temporal-axis
 * witnesses R2/R4/R5).  The chain edges are semantic: they carry round-carried
 * dependence as witness id 1, never a graph-topology heuristic. */
typedef struct sgpl_temporal_unit {
  void *graph;        /* unit's graph (registered with the runtime) */
  sgpl_exec_ctx *ctx; /* unit's context; lifetime owned by the caller */
} sgpl_temporal_unit;

/* Run `unit_count` temporal units as a witness-backed chain through the
 * ready-work DAG scheduler.  Returns SGPL_DAG_OK or a negative SGPL_DAG_ERR_*
 * code; a unit whose graph/context is unusable fails the chain closed. */
int32_t sgpl_exec_dag_temporal_chain(const sgpl_temporal_unit *units,
                                     int32_t unit_count,
                                     int32_t worker_budget);

/* §11.3: temporal units as a partial DAG instead of a chain.  `relations` are
 * the surviving across-round edges over unit indices (PRECEDENCE for a carried
 * dependence, REALIZATION for an order the realization imposes); units without
 * an incoming edge start together, so independent temporal stages overlap.
 * A unit whose graph/context is unusable fails the run closed. */
int32_t sgpl_exec_dag_temporal_units(const sgpl_temporal_unit *units,
                                     int32_t unit_count,
                                     const sgpl_dag_relation_desc *relations,
                                     int32_t relation_count,
                                     int32_t worker_budget);

/* §11.4: product execution over (temporal unit x spatial partition).  The
 * product node index is `unit * partitions + partition`; each cell is
 * dispatched through `cell_fn` (which receives that packed instance), spatial
 * relations are applied inside every unit and temporal relations between
 * units.  Callbacks order existing lifecycle units only; nothing is
 * re-implemented here.  Lifecycle exactly-once: the caller keeps round-begin /
 * snapshot / combine / round-end in the unit wrappers, so the product scheduler
 * must be given cell callbacks that touch only the partition body. */
int32_t sgpl_exec_dag_product(const sgpl_temporal_unit *units,
                              int32_t unit_count, int32_t partitions,
                              sgpl_dag_node_fn cell_fn,
                              const sgpl_dag_relation_desc *spatial_relations,
                              int32_t spatial_count,
                              const sgpl_dag_relation_desc *temporal_relations,
                              int32_t temporal_count,
                              int32_t worker_budget);

/* Complete round lifecycle on one shared DAG. reserve performs no user
 * effects. release accepts partially reserved frames and runs after all tasks
 * join, including failure. Setup/finalize return zero or a negative status. */
typedef int32_t (*sgpl_round_prepare_fn)(void *shared, void *frame, int64_t round);
typedef int32_t (*sgpl_round_work_fn)(void *frame, int64_t index);
typedef void (*sgpl_round_release_fn)(void *frame);
typedef struct sgpl_round_spatial_plan {
  void *shared;
  int64_t first_round;
  int64_t round_count;
  uint64_t frame_bytes;
  int32_t partitions;
  sgpl_round_prepare_fn reserve;
  sgpl_round_work_fn setup;
  sgpl_round_work_fn partition;
  sgpl_round_work_fn finalize;
  sgpl_round_release_fn release;
} sgpl_round_spatial_plan;
int32_t sgpl_exec_dag_round_spatial(const sgpl_round_spatial_plan *plan,
                                    int32_t worker_budget);
int32_t sgpl_exec_dag_round_spatial_gpu(const sgpl_round_spatial_plan *,int32_t,
    const char *,uint64_t,const void *,uint32_t,void *const *,uint32_t);
int32_t sgpl_round_spatial_host_call(sgpl_round_work_fn,void *,int64_t);
void *sgpl_round_spatial_current_frame(void);
void *sgpl_round_spatial_alloc(uint64_t count, uint64_t stride);
int32_t autograph_clean_cut_partition_count(void *graph);
int32_t sgpl_exec_spatial_begin(sgpl_exec_ctx *ctx);
int32_t sgpl_exec_spatial_publish(sgpl_exec_ctx *ctx);
int32_t sgpl_exec_spatial_partition(sgpl_exec_ctx *ctx, int32_t partition);
int32_t sgpl_exec_spatial_finish(sgpl_exec_ctx *ctx);
/* Invocation-owned frontier infrastructure. Materialization and commit never
 * access graph scratch. Call reserve before prepare_spatial, then materialize
 * after round setup and before spatial_begin. */
int32_t autograph_exec_ctx_reserve_frontier(sgpl_exec_ctx *ctx, int32_t membership,
                                           int32_t append);
int32_t autograph_exec_ctx_materialize_frontier_array(sgpl_exec_ctx *ctx,
                                                       const int32_t *vertices, int32_t count);
int32_t autograph_exec_ctx_materialize_frontier_bitmap(sgpl_exec_ctx *ctx, const void *bitmap);
int32_t autograph_exec_ctx_commit_frontier_array(sgpl_exec_ctx *ctx, int32_t *vertices,
                                                 int32_t count);
int32_t autograph_exec_ctx_commit_frontier_bitmap(sgpl_exec_ctx *ctx, void *bitmap,
                                                  int32_t count);
/* Reservation does not read the live version. Materialization occurs at setup. */
int32_t autograph_exec_ctx_reserve_snapshot(sgpl_exec_ctx *ctx, int32_t slot,
                                            int64_t count, int64_t elem_bytes);
int32_t autograph_exec_ctx_materialize_snapshot(sgpl_exec_ctx *ctx, int32_t slot,
                                                const void *base, int64_t bytes);

/* Debug introspection: effective width of the most recent DAG spatial
 * dispatch, after the nested-budget clamp.  Used by the nested-budget test
 * to prove a unit's dispatch shares the run budget instead of the machine. */
int32_t sgpl_debug_last_dag_dispatch_width(void);

/* Caller proves that all redirected reads use input-source indices. */
int32_t autograph_exec_ctx_bind_source_snapshot_range(sgpl_exec_ctx *,int32_t,
    const void *,int64_t,int64_t);

/* Preparation has already validated every source and counted pair occurrences. */
int32_t autograph_exec_ctx_reserve_prepared_output(sgpl_exec_ctx *);

#ifdef __cplusplus
}
#endif

#endif /* AUTOTUNER_RUNTIME_H */

