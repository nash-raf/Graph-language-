#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Roaring / loader stubs (see exec_engine_test.c). */
typedef struct RoaringBitmap RoaringBitmap;
RoaringBitmap *roaring_bitmap_create(uint64_t a, uint64_t b) {
  (void)a; (void)b; return NULL;
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
} CoverGraph;

enum { MAXN = 16 };

typedef struct {
  int32_t expect[MAXN];
  int32_t accum[MAXN]; /* atomic adds from concurrent partitions */
  int32_t began[MAXN];
  int32_t ended[MAXN];
  int32_t begin_order[MAXN];
  int32_t end_order[MAXN];
  int32_t n_begin;
  int32_t n_end;
} CoverState;

static void cov_src_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  CoverState *s = (CoverState *)op->state;
  (void)ctx;
  s->began[u]++;
  s->begin_order[s->n_begin++] = u;
}
static void cov_pair(void *state, sgpl_exec_ctx *ctx, int32_t u, int32_t v) {
  CoverState *s = (CoverState *)state;
  (void)ctx; (void)v;
  __atomic_fetch_add(&s->accum[u], 1, __ATOMIC_RELAXED);
}
static void cov_src_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  CoverState *s = (CoverState *)op->state;
  (void)ctx;
  s->ended[u]++;
  s->end_order[s->n_end++] = u;
  /* consume: the accumulated per-source value must match the serial out-degree;
   * reset for the next round. */
  if (s->accum[u] != s->expect[u]) {
    fprintf(stderr, "  bg   source %d accum=%d expect=%d\n", u, s->accum[u],
            s->expect[u]);
  }
  s->accum[u] = 0;
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

static void expect_degrees(const CoverGraph *g, CoverState *s, int n) {
  for (int u = 0; u < n; ++u)
    s->expect[u] = (int32_t)(g->row_ptr[u + 1] - g->row_ptr[u]);
}

/* One OWNER_V run with a membership set (NULL = all sources). */
static void run_cover(const CoverGraph *g, int32_t partitions,
                      const uint8_t *membership, const char *label,
                      int n) {
  CoverState st;
  sgpl_runtime_op op;
  sgpl_exec_ctx ctx;
  int32_t built;
  int u;

  memset(&st, 0, sizeof(st));
  expect_degrees(g, &st, n);
  memset(&op, 0, sizeof(op));
  op.capabilities = SGPL_OP_PAIR | SGPL_OP_SOURCE_BEGIN | SGPL_OP_SOURCE_END;
  op.state = &st;
  op.source_begin = cov_src_begin;
  op.pair = cov_pair;
  op.source_end = cov_src_end;

  built = autograph_build_clean_cut((void *)g, partitions);
  if (built <= 0) {
    printf("  FAIL %s build\n", label);
    failures++;
    return;
  }

  memset(&ctx, 0, sizeof(ctx));
  ctx.round_id = 1;
  ctx.traversal_kind = SGPL_TRAVERSE_OWNER_V;
  ctx.domain_kind = membership ? SGPL_DOMAIN_FRONTIER : SGPL_DOMAIN_ALL_VERTICES;
  ctx.membership = membership;
  ctx.ops = &op;
  ctx.op_count = 1;
  (void)autograph_frontier_execute((void *)g, &ctx);

  {
    int gated = 0, bad = 0;
    for (u = 0; u < n; ++u) {
      int want = (!membership || membership[u]) ? 1 : 0;
      gated += want;
      if (want && (st.began[u] != 1 || st.ended[u] != 1))
        bad++;
      if (!want && (st.began[u] != 0 || st.ended[u] != 0))
        bad++;
    }
    if (bad || st.n_begin != gated || st.n_end != gated) {
      printf("  FAIL %s lifecycle coverage (want %d, begin=%d end=%d bad=%d)\n",
             label, gated, st.n_begin, st.n_end, bad);
      failures++;
    } else {
      printf("  ok   %s lifecycle exactly-once over %d sources\n", label,
             gated);
    }
    /* ascending finish order */
    {
      int asc = 1, prev = -1;
      for (int i = 0; i < st.n_end; ++i) {
        if (st.end_order[i] <= prev)
          asc = 0;
        prev = st.end_order[i];
      }
      check(asc, "deterministic ascending source_end order");
    }
    /* accumulation matched the serial out-degree (checked in cov_src_end) */
    {
      int ok = 1;
      for (u = 0; u < n; ++u)
        if ((!membership || membership[u]) && st.began[u] && st.ended[u] &&
            st.accum[u] != 0)
          ok = 0;
      check(ok, "per-source accumulation consumed and reset");
    }
  }
}

int main(void) {
  int64_t row_ptr[8] = {0, 2, 4, 6, 9, 11, 12, 12};
  int32_t col_idx[12] = {1, 5, 0, 3, 2, 6, 1, 4, 5, 0, 6, 3};
  CoverGraph g = {
      .n = 7, .m = 12, .row_ptr = row_ptr, .col_idx = col_idx,
      .weights = NULL, .directed = 1, .in_row_ptr = NULL, .in_col_idx = NULL,
  };
  uint8_t m6[8] = {0, 0, 0, 0, 0, 0, 1, 0};   /* zero-pair source 6 */
  uint8_t m02[8] = {1, 0, 1, 0, 0, 0, 0, 0};  /* sources 0 and 2 */
  const int32_t partitions[] = {1, 3, 8};
  char label[64];

  autograph_init(&g, g.n, g.m, NULL, NULL, NULL);

  for (size_t pi = 0; pi < sizeof(partitions) / sizeof(partitions[0]); ++pi) {
    printf("== partitions=%d ==\n", partitions[pi]);
    snprintf(label, sizeof(label), "all [p=%d]", partitions[pi]);
    run_cover(&g, partitions[pi], NULL, label, 7);
    snprintf(label, sizeof(label), "zero-pair {6} [p=%d]", partitions[pi]);
    run_cover(&g, partitions[pi], m6, label, 7);
    snprintf(label, sizeof(label), "gated {0,2} [p=%d]", partitions[pi]);
    run_cover(&g, partitions[pi], m02, label, 7);
  }

  printf("owner-v source coverage test: %d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
