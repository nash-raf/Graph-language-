#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* No roaring usage on this path — stubs keep the test independent. */
typedef struct RoaringBitmap RoaringBitmap;
RoaringBitmap *roaring_bitmap_create(uint64_t a, uint64_t c) {
  (void)a; (void)c; return NULL;
}
void roaring_bitmap_add(RoaringBitmap *b, uint32_t v) { (void)b; (void)v; }
void roaring_bitmap_remove(RoaringBitmap *b, uint32_t v) { (void)b; (void)v; }
bool roaring_bitmap_contains(const RoaringBitmap *b, uint32_t v) {
  (void)b; (void)v; return false;
}
void roaring_bitmap_clear(RoaringBitmap *b) { (void)b; }
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *b) {
  (void)b; return 0;
}
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *b, uint64_t i) {
  (void)b; (void)i; return 0;
}
RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *p) {
  (void)p; return NULL;
}
void roaring_bitmap_or_inplace(RoaringBitmap *d, RoaringBitmap *s) {
  (void)d; (void)s;
}
void roaring_bitmap_free(RoaringBitmap *b) { (void)b; }
void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **o,
                                               RoaringBitmap **r, int32_t c) {
  (void)o; (void)r; (void)c;
}
void roaring_bitmap_clear_thread_local_overrides(void) {}
int32_t *graph_get_edge_pairs(void *g) { (void)g; return NULL; }
int64_t graph_get_num_edge_pairs(void *g) { (void)g; return 0; }

typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
  int32_t *weights;
  int32_t directed;
  int64_t *in_row_ptr;
  int32_t *in_col_idx;
} SnTestGraph;

/* ── scenario state ─────────────────────────────────────────────── */

static int32_t g_live[8];
static void *g_snap_slot;          /* pointer slot published by the Snapshot op */
static int32_t g_frozen_obs[16];   /* values the pairs observed from the snapshot */
static int g_frozen_n;
static _Atomic int g_snap_calls;
static _Atomic int64_t g_source_state_sum;
static _Atomic int g_source_state_mismatch;
static int32_t g_ended[8];         /* SourceEnd per source (exactly-once) */

/* ── Snapshot operation (R7) ────────────────────────────────────── */

static void snap_op(sgpl_runtime_op *op, sgpl_exec_ctx *ctx) {
  void **slot = (void **)op->state;
  atomic_fetch_add(&g_snap_calls, 1);
  *slot = autograph_snapshot_publish(ctx->graph, g_live, sizeof(int32_t), 0);
}

static void pair_read_snap(void *state, sgpl_exec_ctx *ctx, int32_t u,
                           int32_t v) {
  int32_t *snap = (int32_t *)g_snap_slot; /* published by the Snapshot op */
  (void)state;
  (void)ctx;
  (void)v;
  g_frozen_obs[g_frozen_n++] = snap ? snap[u] : -999;
}

/* ── source-state channel (R7) ──────────────────────────────────── */

static void src_begin_set(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  (void)op;
  ctx->source_state = (void *)(intptr_t)(u + 1); /* the per-source claim */
}

static void pair_read_src(void *state, sgpl_exec_ctx *ctx, int32_t u,
                          int32_t v) {
  (void)state;
  (void)v;
  atomic_fetch_add(&g_source_state_sum, (intptr_t)ctx->source_state);
  if ((intptr_t)ctx->source_state != (intptr_t)(u + 1))
    atomic_fetch_add(&g_source_state_mismatch, 1);
}

static void src_end_rec(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  (void)ctx;
  ((int32_t *)op->state)[u]++;
}

static int failures = 0;
static void check(int cond, const char *what) {
  if (cond)
    printf("  ok   %s\n", what);
  else {
    printf("  FAIL %s\n", what);
    failures++;
  }
}

static int cmp_i32(const void *a, const void *b) {
  int32_t x = *(const int32_t *)a;
  int32_t y = *(const int32_t *)b;
  return (x > y) - (x < y);
}

/* Snapshot(A): RoundBegin -> Snapshot -> pairs read the frozen copy; mutating
 * the live array after the round does not affect what pairs observed. */
static void test_snapshot_op(const SnTestGraph *g, int32_t partitions) {
  sgpl_runtime_op ops[2];
  sgpl_exec_ctx ctx;
  int32_t expected[16];
  int i;

  autograph_build_clean_cut((void *)g, partitions);
  for (i = 0; i < 8; ++i)
    g_live[i] = 100 + i; /* round-start values */
  memset(g_frozen_obs, 0, sizeof(g_frozen_obs));
  g_frozen_n = 0;
  atomic_store(&g_snap_calls, 0);
  g_snap_slot = NULL;

  memset(ops, 0, sizeof(ops));
  ops[0].capabilities = SGPL_OP_SNAPSHOT;
  ops[0].state = &g_snap_slot;
  ops[0].snapshot = snap_op;
  ops[1].capabilities = SGPL_OP_PAIR;
  ops[1].state = &g_snap_slot;
  ops[1].pair = pair_read_snap;

  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 1;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx.ops = ops;
  ctx.op_count = 2;
  ctx.run_round_begin = 1; /* Snapshot runs under the round owner */
  ctx.run_round_end = 0;

  autograph_frontier_execute((void *)g, &ctx);

  for (i = 0; i < g_frozen_n; ++i)
    expected[i] = -1;
  /* Reconstruct expected: pair (u,v) observed live[u] at round start. */
  {
    int k = 0;
    for (int64_t u = 0; u < g->n; ++u)
      for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j) {
        (void)g->col_idx[j];
        expected[k++] = g_live[u];
      }
  }

  /* Mutate live: observed values must be frozen, not these. */
  for (i = 0; i < 8; ++i)
    g_live[i] = 9000 + i;

  check(atomic_load(&g_snap_calls) == 1, "snapshot op ran exactly once");
  check(g_frozen_n == (int)g->m, "every pair observed the frozen snapshot");
  {
    /* Partition bodies run concurrently, so the order in which pairs append
     * their observations is schedule-dependent; compare as a multiset. */
    int32_t obs[16];
    int32_t exp[16];
    int ok = 1;
    memcpy(obs, g_frozen_obs, (size_t)g_frozen_n * sizeof(int32_t));
    memcpy(exp, expected, (size_t)g_frozen_n * sizeof(int32_t));
    qsort(obs, (size_t)g_frozen_n, sizeof(int32_t), cmp_i32);
    qsort(exp, (size_t)g_frozen_n, sizeof(int32_t), cmp_i32);
    for (i = 0; i < g_frozen_n; ++i)
      if (obs[i] != exp[i])
        ok = 0;
    check(ok, "pairs read the round-start snapshot, not the mutated live");
  }
}

/* source_begin -> source_state -> pair: the executor zeroes the channel per
 * source; the begin op produces it; pairs observe exactly the current source's
 * claim.  Zero-pair source 6 still observes begin/end exactly once. */
static void test_source_state_channel(const SnTestGraph *g,
                                      int32_t partitions) {
  sgpl_runtime_op ops[3];
  sgpl_exec_ctx ctx;
  int64_t expected_sum = 0;
  int i;

  autograph_build_clean_cut((void *)g, partitions);
  atomic_store(&g_source_state_sum, 0);
  atomic_store(&g_source_state_mismatch, 0);
  memset(g_ended, 0, sizeof(g_ended));

  memset(ops, 0, sizeof(ops));
  ops[0].capabilities = SGPL_OP_SOURCE_BEGIN;
  ops[0].source_begin = src_begin_set;
  ops[1].capabilities = SGPL_OP_PAIR;
  ops[1].pair = pair_read_src;
  ops[2].capabilities = SGPL_OP_SOURCE_END;
  ops[2].state = g_ended;
  ops[2].source_end = src_end_rec;

  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 1;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx.ops = ops;
  ctx.op_count = 3;

  autograph_frontier_execute((void *)g, &ctx);

  for (int64_t u = 0; u < g->n; ++u)
    expected_sum += (int64_t)(g->row_ptr[u + 1] - g->row_ptr[u]) * (u + 1);

  check(atomic_load(&g_source_state_sum) == expected_sum,
        "pairs observed the per-source claim channel");
  check(atomic_load(&g_source_state_mismatch) == 0,
        "source_state always matched the current source");
  {
    int ok = 1;
    for (i = 0; i < g->n; ++i)
      if (g_ended[i] != 1)
        ok = 0;
    check(ok, "source_begin/source_end exactly once per source (zero-pair incl.)");
  }
}

int main(void) {
  int64_t row_ptr[8] = {0, 2, 4, 6, 9, 11, 12, 12};
  int32_t col_idx[12] = {1, 5, 0, 3, 2, 6, 1, 4, 5, 0, 6, 3};
  SnTestGraph g = {
      .n = 7, .m = 12, .row_ptr = row_ptr, .col_idx = col_idx,
      .weights = NULL, .directed = 1, .in_row_ptr = NULL, .in_col_idx = NULL,
  };
  const int32_t partitions[] = {1, 8};

  autograph_init(&g, g.n, g.m, NULL, NULL, NULL);

  for (size_t pi = 0; pi < sizeof(partitions) / sizeof(partitions[0]); ++pi) {
    printf("== partitions=%d ==\n", partitions[pi]);
    test_snapshot_op(&g, partitions[pi]);
    test_source_state_channel(&g, partitions[pi]);
  }

  printf("snapshot + source-state channel test: %d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
