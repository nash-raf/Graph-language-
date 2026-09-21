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
  SGPL_OP_SNAPSHOT = 1u << 8 /* RoundBegin -> Snapshot(A) -> CrossRead(A) */
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
} sgpl_exec_ctx;

/* Execute one stage: traverse the selected domain and dispatch the declared
 * lifecycle events to the operations.  Returns the next frontier size. */
int32_t autograph_frontier_execute(void *graph_ptr, sgpl_exec_ctx *ctx);

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
    int64_t partition_stride, sgpl_runtime_op **ops, uint32_t op_count);

void autograph_exec_ctx_destroy(sgpl_exec_ctx *ctx);

/* Snapshot primitive (R7): copy `live_base` (n * elem_bytes, n from the graph)
 * into the per-graph scratch snapshot slot and return the frozen buffer.  A
 * Snapshot op calls this in its round-phase callback and publishes the pointer
 * (its operation state) for the round's cross-reads.  The buffer lifecycle is
 * owned by the runtime. */
void *autograph_snapshot_publish(void *graph_ptr, const void *live_base,
                                 int64_t elem_bytes, int32_t slot);

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

#ifdef __cplusplus
}
#endif

#endif /* AUTOTUNER_RUNTIME_H */
