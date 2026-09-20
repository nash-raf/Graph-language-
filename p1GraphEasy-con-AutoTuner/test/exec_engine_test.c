#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Roaring stubs: the executor path does not use bitmaps; these satisfy the
 * linker for the motif/graph code referenced by autotuner_runtime.c. */
typedef struct RoaringBitmap RoaringBitmap;

RoaringBitmap *roaring_bitmap_create(uint64_t arena_size,
                                     uint64_t initial_capacity) {
  (void)arena_size;
  (void)initial_capacity;
  return NULL;
}
void roaring_bitmap_add(RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
}
void roaring_bitmap_remove(RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
}
bool roaring_bitmap_contains(const RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
  return false;
}
void roaring_bitmap_clear(RoaringBitmap *bitmap) { (void)bitmap; }
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *bitmap) {
  (void)bitmap;
  return 0;
}
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *bitmap,
                                     uint64_t index) {
  (void)bitmap;
  (void)index;
  return 0;
}
RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *prototype) {
  (void)prototype;
  return NULL;
}
void roaring_bitmap_or_inplace(RoaringBitmap *destination,
                               RoaringBitmap *source) {
  (void)destination;
  (void)source;
}
void roaring_bitmap_free(RoaringBitmap *bitmap) { (void)bitmap; }
void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **originals,
                                               RoaringBitmap **replacements,
                                               int32_t count) {
  (void)originals;
  (void)replacements;
  (void)count;
}
void roaring_bitmap_clear_thread_local_overrides(void) {}

int32_t *graph_get_edge_pairs(void *graph_ptr) {
  (void)graph_ptr;
  return NULL;
}
int64_t graph_get_num_edge_pairs(void *graph_ptr) {
  (void)graph_ptr;
  return 0;
}

/* Graph struct whose field layout matches the offsets autotuner_runtime.c
 * reads (n@0, m@8, row_ptr@16, col_idx@24, weights@32, directed@40,
 * in_row_ptr@48, in_col_idx@56). */
typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
  int32_t *weights;
  int32_t directed;
  int64_t *in_row_ptr;
  int32_t *in_col_idx;
} ExecTestGraph;

/* ── event log ──────────────────────────────────────────────────── */

enum {
  EV_PART_BEGIN,
  EV_PART_END,
  EV_SRC_BEGIN,
  EV_SRC_END,
  EV_PAIR,
  EV_ROUND_BEGIN,
  EV_ROUND_END,
  EV_COMBINE
};

typedef struct {
  int kind;
  int a;
  int b;
  int stage;
} Ev;

static Ev g_log[16384];
static int g_log_n;
static pthread_mutex_t g_log_mu = PTHREAD_MUTEX_INITIALIZER;

static void ev(int stage, int kind, int a, int b) {
  pthread_mutex_lock(&g_log_mu);
  if (g_log_n < (int)(sizeof(g_log) / sizeof(g_log[0]))) {
    g_log[g_log_n].kind = kind;
    g_log[g_log_n].a = a;
    g_log[g_log_n].b = b;
    g_log[g_log_n].stage = stage;
    g_log_n++;
  }
  pthread_mutex_unlock(&g_log_mu);
}

static void log_reset(void) {
  pthread_mutex_lock(&g_log_mu);
  g_log_n = 0;
  pthread_mutex_unlock(&g_log_mu);
}

static int count_kind(int stage, int kind) {
  int i, n = 0;
  for (i = 0; i < g_log_n; ++i)
    if (g_log[i].stage == stage && g_log[i].kind == kind)
      n++;
  return n;
}

/* first/last index of a (stage, kind) event, -1 when absent */
static int first_of(int stage, int kind) {
  int i;
  for (i = 0; i < g_log_n; ++i)
    if (g_log[i].stage == stage && g_log[i].kind == kind)
      return i;
  return -1;
}
static int last_of(int stage, int kind) {
  int i, r = -1;
  for (i = 0; i < g_log_n; ++i)
    if (g_log[i].stage == stage && g_log[i].kind == kind)
      r = i;
  return r;
}

/* ── recording operation ────────────────────────────────────────── */

typedef struct {
  int stage;
  int id; /* op index within the ctx (Par-local order check) */
} RecState;

static void rec_round_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx) {
  RecState *s = (RecState *)op->state;
  ev(s->stage, EV_ROUND_BEGIN, (int)ctx->round_id, 0);
}
static void rec_round_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx) {
  RecState *s = (RecState *)op->state;
  ev(s->stage, EV_ROUND_END, (int)ctx->round_id, 0);
}
static void rec_part_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t p) {
  RecState *s = (RecState *)op->state;
  (void)ctx;
  ev(s->stage, EV_PART_BEGIN, p, s->id);
}
static void rec_part_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t p) {
  RecState *s = (RecState *)op->state;
  (void)ctx;
  ev(s->stage, EV_PART_END, p, s->id);
}
static void rec_src_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  RecState *s = (RecState *)op->state;
  (void)ctx;
  ev(s->stage, EV_SRC_BEGIN, u, s->id);
}
static void rec_src_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  RecState *s = (RecState *)op->state;
  (void)ctx;
  ev(s->stage, EV_SRC_END, u, s->id);
}
static void rec_pair(void *state, sgpl_exec_ctx *ctx, int32_t u, int32_t v) {
  RecState *s = (RecState *)state;
  (void)ctx;
  ev(s->stage, EV_PAIR, u, v);
}

/* combine records (partition slot offset / stride) and the call ordinal */
static void rec_combine(void *state, sgpl_exec_ctx *ctx) {
  RecState *s = (RecState *)state;
  int p = -1;
  if (ctx->partition_base && ctx->partition_stride > 0)
    p = (int)(((char *)ctx->partition_state - (char *)ctx->partition_base) /
              ctx->partition_stride);
  ev(s->stage, EV_COMBINE, p, s->id);
}

static const uint64_t kAllCaps = SGPL_OP_PAIR | SGPL_OP_SOURCE_BEGIN |
                                 SGPL_OP_SOURCE_END | SGPL_OP_PARTITION_BEGIN |
                                 SGPL_OP_PARTITION_END | SGPL_OP_ROUND_BEGIN |
                                 SGPL_OP_ROUND_END;

static void make_rec_op(sgpl_runtime_op *op, RecState *st, int stage, int id) {
  memset(op, 0, sizeof(*op));
  st->stage = stage;
  st->id = id;
  op->capabilities = kAllCaps;
  op->state = st;
  op->round_begin = rec_round_begin;
  op->round_end = rec_round_end;
  op->partition_begin = rec_part_begin;
  op->partition_end = rec_part_end;
  op->source_begin = rec_src_begin;
  op->source_end = rec_src_end;
  op->pair = rec_pair;
}

/* ── expected edge multiset ─────────────────────────────────────── */

static int edge_present(const ExecTestGraph *g, int u, int v) {
  for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j)
    if (g->col_idx[j] == v)
      return 1;
  return 0;
}

static int pair_event_count(int stage, int u, int v) {
  int i, n = 0;
  for (i = 0; i < g_log_n; ++i)
    if (g_log[i].stage == stage && g_log[i].kind == EV_PAIR &&
        g_log[i].a == u && g_log[i].b == v)
      n++;
  return n;
}

/* ── tests ──────────────────────────────────────────────────────── */

static int failures = 0;
static void check(int cond, const char *what) {
  if (cond) {
    printf("  ok   %s\n", what);
  } else {
    printf("  FAIL %s\n", what);
    failures++;
  }
}

/* OWNER_U: every edge exactly once, per-source begin/end exactly once, and a
 * zero-pair source still observes begin+end. */
static void test_owner_u_lifecycle(const ExecTestGraph *g, int32_t partitions) {
  sgpl_runtime_op op;
  RecState st;
  sgpl_exec_ctx ctx;
  int32_t built, u, v;

  autograph_build_clean_cut((void *)g, partitions);
  built = autograph_build_clean_cut((void *)g, partitions);
  if (built <= 0) {
    printf("  FAIL clean-cut build\n");
    failures++;
    return;
  }

  log_reset();
  make_rec_op(&op, &st, 1, 0);
  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 1;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx.domain_kind = SGPL_DOMAIN_ALL_VERTICES;
  ctx.ops = &op;
  ctx.op_count = 1;
  (void)autograph_frontier_execute((void *)g, &ctx);

  for (u = 0; u < (int)g->n; ++u) {
    for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j) {
      v = g->col_idx[j];
      if (pair_event_count(1, u, v) != 1) {
        printf("  FAIL edge (%d,%d) events=%d\n", u, v, pair_event_count(1, u, v));
        failures++;
      }
    }
    if (count_kind(1, EV_SRC_BEGIN) != (int)g->n ||
        count_kind(1, EV_SRC_END) != (int)g->n) {
      printf("  FAIL source begin/end counts begin=%d end=%d (n=%d)\n",
             count_kind(1, EV_SRC_BEGIN), count_kind(1, EV_SRC_END), (int)g->n);
      failures++;
      break;
    }
  }

  /* partition begin/end: one per partition */
  if (count_kind(1, EV_PART_BEGIN) != built ||
      count_kind(1, EV_PART_END) != built) {
    printf("  FAIL partition events begin=%d end=%d built=%d\n",
           count_kind(1, EV_PART_BEGIN), count_kind(1, EV_PART_END), built);
    failures++;
  } else {
    printf("  ok   owner-u lifecycle partitions=%d\n", built);
  }

  /* zero-pair source (vertex 6) must have begin immediately followed by end
   * with no pair between */
  {
    int bi = -1, ei = -1, i;
    for (i = 0; i < g_log_n; ++i) {
      if (g_log[i].stage != 1)
        continue;
      if (g_log[i].kind == EV_SRC_BEGIN && g_log[i].a == 6 && bi < 0)
        bi = i;
      if (g_log[i].kind == EV_SRC_END && g_log[i].a == 6 && ei < 0)
        ei = i;
    }
    int no_pair_between = 1;
    for (i = bi + 1; i >= 0 && i < ei; ++i)
      if (g_log[i].kind == EV_PAIR && g_log[i].a == 6)
        no_pair_between = 0;
    check(bi >= 0 && ei > bi && no_pair_between,
          "zero-pair source got begin+end with no pairs");
  }
}

/* Membership restriction: only sources with membership[u] set are visited. */
static void test_membership(const ExecTestGraph *g, int32_t partitions) {
  uint8_t membership[8] = {0};
  sgpl_runtime_op op;
  RecState st;
  sgpl_exec_ctx ctx;
  int u;

  autograph_build_clean_cut((void *)g, partitions);
  membership[0] = 1;
  membership[2] = 1;

  log_reset();
  make_rec_op(&op, &st, 2, 0);
  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 2;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx.domain_kind = SGPL_DOMAIN_FRONTIER;
  ctx.membership = membership;
  ctx.ops = &op;
  ctx.op_count = 1;
  (void)autograph_frontier_execute((void *)g, &ctx);

  for (u = 0; u < (int)g->n; ++u) {
    if (u != 0 && u != 2 && count_kind(2, EV_SRC_BEGIN) != 2) {
      printf("  FAIL unexpected source visited (u=%d)\n", u);
      failures++;
      return;
    }
  }
  check(count_kind(2, EV_SRC_BEGIN) == 2 && count_kind(2, EV_SRC_END) == 2,
        "membership gate visits exactly {0,2}");
  for (u = 0; u < (int)g->n; ++u)
    if (u != 0 && u != 2)
      for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j)
        if (pair_event_count(2, u, g->col_idx[j]) != 0) {
          printf("  FAIL gated pair (u=%d) visited\n", u);
          failures++;
        }
}

/* Round flags: round_begin first, round_end last, combine between. */
static void test_round_flags_and_combine(const ExecTestGraph *g,
                                         int32_t partitions) {
  sgpl_runtime_op ops[2];
  RecState st0, st1;
  sgpl_exec_ctx ctx;
  int32_t base[8] = {0};
  int32_t built;

  built = autograph_build_clean_cut((void *)g, partitions);
  log_reset();
  make_rec_op(&ops[0], &st0, 3, 0);
  make_rec_op(&ops[1], &st1, 3, 1);
  ops[1].capabilities = SGPL_OP_COMBINE; /* second op combines only */
  ops[1].combine = rec_combine;

  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 3;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx.partition_base = base;
  ctx.partition_stride = (int64_t)sizeof(base[0]);
  ctx.ops = ops;
  ctx.op_count = 2;
  ctx.run_round_begin = 1;
  ctx.run_round_end = 1;
  (void)autograph_frontier_execute((void *)g, &ctx);

  check(first_of(3, EV_ROUND_BEGIN) == 0, "round_begin first");
  check(last_of(3, EV_ROUND_END) == g_log_n - 1, "round_end last");
  check(count_kind(3, EV_COMBINE) == built, "combine per partition");
  {
    int i, expected_p = 0, ascending = 1;
    for (i = 0; i < g_log_n; ++i)
      if (g_log[i].kind == EV_COMBINE) {
        if (g_log[i].a != expected_p)
          ascending = 0;
        expected_p++;
      }
    check(ascending, "combine ascending partition order with correct slots");
  }
  {
    int rb = first_of(3, EV_ROUND_BEGIN), re = last_of(3, EV_ROUND_END);
    int first_pair = first_of(3, EV_PAIR), first_comb = first_of(3, EV_COMBINE);
    check(rb < first_pair && first_pair < first_comb && first_comb < re,
          "round_begin < pairs < combine < round_end");
  }
}

/* Sequential stages share the round context and are separated by a barrier:
 * all stage-1 events precede all stage-2 events. */
static void test_sequential_barrier(const ExecTestGraph *g, int32_t partitions) {
  sgpl_runtime_op op1, op2;
  RecState st1, st2;
  sgpl_exec_ctx ctx1, ctx2;

  autograph_build_clean_cut((void *)g, partitions);
  log_reset();
  make_rec_op(&op1, &st1, 10, 0);
  make_rec_op(&op2, &st2, 20, 0);

  memset(&ctx1, 0, sizeof(ctx1));
  ctx1.round_id = 4;
  ctx1.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx1.ops = &op1;
  ctx1.op_count = 1;
  ctx1.run_round_begin = 1;
  (void)autograph_frontier_execute((void *)g, &ctx1);

  memset(&ctx2, 0, sizeof(ctx2));
  ctx2.round_id = 4;
  ctx2.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx2.ops = &op2;
  ctx2.op_count = 1;
  ctx2.run_round_end = 1;
  (void)autograph_frontier_execute((void *)g, &ctx2);

  check(last_of(10, EV_PAIR) >= 0 && first_of(20, EV_PAIR) > last_of(10, EV_PAIR),
        "stage 2 starts after stage 1 finished (barrier)");
  check(first_of(10, EV_ROUND_BEGIN) >= 0 && last_of(20, EV_ROUND_END) >= 0,
        "one round context across both stages");
}

/* No implicit activation: an operation without A+ leaves dest_seen and
 * next_frontier untouched, and next_size is unchanged. */
static void test_no_implicit_activation(const ExecTestGraph *g,
                                        int32_t partitions) {
  sgpl_runtime_op op;
  RecState st;
  sgpl_exec_ctx ctx;
  int32_t dest_seen[8] = {0};
  int32_t next_frontier[8] = {0};
  int32_t head = 0;
  int32_t rc;

  autograph_build_clean_cut((void *)g, partitions);
  log_reset();
  make_rec_op(&op, &st, 30, 0);
  op.capabilities = SGPL_OP_PAIR; /* pair callback only: no activation */
  op.pair = rec_pair;

  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 5;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  ctx.dest_seen = dest_seen;
  ctx.next_frontier = next_frontier;
  ctx.initial_next_size = 0;
  ctx.append_head = &head;
  ctx.ops = &op;
  ctx.op_count = 1;
  rc = autograph_frontier_execute((void *)g, &ctx);

  {
    int untouched = 1;
    for (int i = 0; i < 8; ++i)
      if (dest_seen[i] != 0 || next_frontier[i] != 0)
        untouched = 0;
    check(untouched && rc == 0, "no implicit activation (dest_seen/frontier untouched)");
  }
}

/* OWNER_V enumerates the same edge multiset (dest-owned rows). */
static void test_owner_v_pairs(const ExecTestGraph *g, int32_t partitions) {
  sgpl_runtime_op op;
  RecState st;
  sgpl_exec_ctx ctx;
  int32_t built;

  built = autograph_build_clean_cut((void *)g, partitions);
  log_reset();
  make_rec_op(&op, &st, 40, 0);
  op.capabilities = SGPL_OP_PAIR; /* no source lifecycle under OWNER_V in R1 */
  op.pair = rec_pair;

  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 6;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_V;
  ctx.ops = &op;
  ctx.op_count = 1;
  (void)autograph_frontier_execute((void *)g, &ctx);

  {
    int bad = 0;
    for (int u = 0; u < (int)g->n; ++u)
      for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j)
        if (pair_event_count(40, u, g->col_idx[j]) != 1)
          bad++;
    check(bad == 0 && count_kind(40, EV_PAIR) == (int)g->m,
          "owner-v enumerates every edge exactly once");
  }
  (void)built;
}

int main(void) {
  /* n=7, m=12; vertex 6 has no out-edges (zero-pair source). */
  int64_t row_ptr[8] = {0, 2, 4, 6, 9, 11, 12, 12};
  int32_t col_idx[12] = {1, 5, 0, 3, 2, 6, 1, 4, 5, 0, 6, 3};
  ExecTestGraph g = {
      .n = 7, .m = 12, .row_ptr = row_ptr, .col_idx = col_idx,
      .weights = NULL, .directed = 1, .in_row_ptr = NULL, .in_col_idx = NULL,
  };
  const int32_t partitions[] = {1, 3, 8};

  autograph_init(&g, g.n, g.m, NULL, NULL, NULL);

  for (size_t pi = 0; pi < sizeof(partitions) / sizeof(partitions[0]); ++pi) {
    printf("== partitions=%d ==\n", partitions[pi]);
    test_owner_u_lifecycle(&g, partitions[pi]);
    test_membership(&g, partitions[pi]);
    test_round_flags_and_combine(&g, partitions[pi]);
    test_sequential_barrier(&g, partitions[pi]);
    test_no_implicit_activation(&g, partitions[pi]);
    test_owner_v_pairs(&g, partitions[pi]);
  }

  printf("exec engine test: %d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
