#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"
#include "../parallel_runtime.h"

#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Roaring / loader stubs. */
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
} FjGraph;

/* ── ops ────────────────────────────────────────────────────────── */

static _Atomic int g_pairs_u;
static _Atomic int g_pairs_v;
static _Atomic int g_round_begin_calls;
static _Atomic int g_round_end_calls;

typedef struct {
  int activate;   /* call sgpl_activate for every walked destination */
  int count_slot; /* 0 -> g_pairs_u, 1 -> g_pairs_v */
} PairOpState;

static void fj_pair(void *state, sgpl_exec_ctx *ctx, int32_t u, int32_t v) {
  PairOpState *s = (PairOpState *)state;
  (void)u;
  if (s->count_slot)
    atomic_fetch_add(&g_pairs_v, 1);
  else
    atomic_fetch_add(&g_pairs_u, 1);
  if (s->activate)
    (void)autograph_frontier_activate(ctx, v);
}

static void fj_round_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx) {
  (void)op;
  (void)ctx;
  atomic_fetch_add(&g_round_begin_calls, 1);
}
static void fj_round_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx) {
  (void)op;
  (void)ctx;
  atomic_fetch_add(&g_round_end_calls, 1);
}

/* ── watchdog ───────────────────────────────────────────────────── */

static void on_alarm(int sig) {
  (void)sig;
  const char *msg = "  FAIL fork/join watchdog timeout (deadlock)\n";
  ssize_t r = write(2, msg, strlen(msg));
  (void)r;
  _exit(2);
}

/* ── helpers ────────────────────────────────────────────────────── */

static int failures = 0;
static void check(int cond, const char *what) {
  if (cond)
    printf("  ok   %s\n", what);
  else {
    printf("  FAIL %s\n", what);
    failures++;
  }
}

/* ── nested fork/join (R7) ─────────────────────────────────────────
 * A Par subtree: the top fork/join runs child a inline and child b on a host
 * thread; b's pair callback forks an inner Par (c, d) from inside the
 * traversal.  Round lifecycle must run exactly once (top owner); the inner
 * owner shares the top owner's round resources with run_round flags false. */

static _Atomic int g_pairs_c;
static _Atomic int g_pairs_d;
static _Atomic int g_inner_forked;
static sgpl_exec_ctx *g_inner_owner;
static sgpl_exec_ctx *g_inner_c;
static sgpl_exec_ctx *g_inner_d;
static void *g_inner_graph;

static void fj_pair_c(void *state, sgpl_exec_ctx *ctx, int32_t u, int32_t v) {
  (void)state;
  (void)u;
  atomic_fetch_add(&g_pairs_c, 1);
  (void)autograph_frontier_activate(ctx, v);
}

static void fj_pair_d(void *state, sgpl_exec_ctx *ctx, int32_t u, int32_t v) {
  (void)state;
  (void)u;
  atomic_fetch_add(&g_pairs_d, 1);
  (void)autograph_frontier_activate(ctx, v);
}

static void fj_pair_nested(void *state, sgpl_exec_ctx *ctx, int32_t u,
                           int32_t v) {
  PairOpState *s = (PairOpState *)state;
  (void)u;
  (void)ctx;
  (void)v;
  if (s->count_slot)
    atomic_fetch_add(&g_pairs_v, 1);
  else
    atomic_fetch_add(&g_pairs_u, 1);
  /* Exactly one pair occurrence triggers the inner Par subtree. */
  if (!atomic_exchange_explicit(&g_inner_forked, 1, memory_order_relaxed))
    (void)autograph_frontier_fork_join(g_inner_graph, g_inner_owner, g_inner_c,
                                       g_inner_d);
}

static int32_t *g_dest_seen;
static int32_t *g_next_frontier;
static int32_t g_append_head;
static int32_t g_initial_next_size;

static void reset_activation(void) {
  memset(g_dest_seen, 0, sizeof(int32_t) * 16);
  memset(g_next_frontier, 0, sizeof(int32_t) * 16);
  g_append_head = 0;
  g_initial_next_size = 0;
}

/* Shared round resources are provided by the owner context. */
static void init_resource_ctx(sgpl_exec_ctx *ctx, sgpl_runtime_op *ops,
                              uint32_t n) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->round_id = 1;
  ctx->dest_seen = g_dest_seen;
  ctx->next_frontier = g_next_frontier;
  ctx->initial_next_size = g_initial_next_size;
  ctx->append_head = &g_append_head;
  ctx->ops = ops;
  ctx->op_count = n;
}

static int next_frontier_set_size(void) {
  int seen[16] = {0};
  int i, n = 0;
  int32_t total = g_append_head;
  for (i = 0; i < total; ++i) {
    int v = g_next_frontier[i];
    if (v >= 0 && v < 16 && !seen[v]) {
      seen[v] = 1;
      n++;
    }
  }
  return n;
}

/* ── tests ──────────────────────────────────────────────────────── */

/* Concurrent activation from two incompatible traversals: OWNER_U with
 * membership {0} contributes dests {1,5}; OWNER_V with membership {2}
 * contributes {2,6}; the union is the frontier set. */
static void test_fork_join_activation(const FjGraph *g, int32_t partitions) {
  uint8_t m0[8] = {1, 0, 0, 0, 0, 0, 0, 0};
  uint8_t m2[8] = {0, 0, 1, 0, 0, 0, 0, 0};
  sgpl_runtime_op owner_ops[1];
  sgpl_runtime_op a_ops[1], b_ops[1];
  PairOpState sa = {.activate = 1, .count_slot = 0};
  PairOpState sb = {.activate = 1, .count_slot = 1};
  sgpl_exec_ctx owner, a, b;
  int before_rb, before_re;

  autograph_build_clean_cut((void *)g, partitions);
  reset_activation();
  atomic_store(&g_pairs_u, 0);
  atomic_store(&g_pairs_v, 0);
  before_rb = atomic_load(&g_round_begin_calls);
  before_re = atomic_load(&g_round_end_calls);

  memset(owner_ops, 0, sizeof(owner_ops));
  owner_ops[0].capabilities = SGPL_OP_ROUND_BEGIN | SGPL_OP_ROUND_END;
  owner_ops[0].round_begin = fj_round_begin;
  owner_ops[0].round_end = fj_round_end;

  memset(a_ops, 0, sizeof(a_ops));
  a_ops[0].capabilities = SGPL_OP_PAIR;
  a_ops[0].state = &sa;
  a_ops[0].pair = fj_pair;

  memset(b_ops, 0, sizeof(b_ops));
  b_ops[0].capabilities = SGPL_OP_PAIR;
  b_ops[0].state = &sb;
  b_ops[0].pair = fj_pair;

  init_resource_ctx(&owner, owner_ops, 1);
  owner.run_round_begin = 1;
  owner.run_round_end = 1;
  owner.traversal_kind = SGPL_TRAVERSE_OWNER_U; /* unused by fork/join */

  init_resource_ctx(&a, a_ops, 1);
  a.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  a.domain_kind = SGPL_DOMAIN_FRONTIER;
  a.membership = m0;

  init_resource_ctx(&b, b_ops, 1);
  b.traversal_kind = SGPL_TRAVERSE_OWNER_V;
  b.domain_kind = SGPL_DOMAIN_FRONTIER;
  b.membership = m2;

  (void)autograph_frontier_fork_join((void *)g, &owner, &a, &b);

  check(atomic_load(&g_round_begin_calls) == before_rb + 1 &&
            atomic_load(&g_round_end_calls) == before_re + 1,
        "owner ran round lifecycle exactly once");
  check(atomic_load(&g_pairs_u) == 2 && atomic_load(&g_pairs_v) == 2,
        "both children executed their traversals");
  check(owner.next_size == 4 && next_frontier_set_size() == 4,
        "concurrent activation unioned the frontier set");
  {
    int ok = g_dest_seen[1] && g_dest_seen[2] && g_dest_seen[5] && g_dest_seen[6];
    check(ok, "dest_seen membership matches the activated set");
  }
}

/* The worst case for the pool: the parent execution holds the whole worker
 * budget while it forks two child executions.  Children must still complete
 * (budget denial degrades to a serial run on the child's own host thread; the
 * runtime never blocks a child waiting for a worker).  The watchdog turns a
 * regression into a test failure instead of a hang. */
typedef struct {
  void *graph;
  sgpl_exec_ctx *owner, *a, *b;
  int32_t rc;
} OuterEnv;

static void outer_body(int64_t index, void *opaque) {
  OuterEnv *e = (OuterEnv *)opaque;
  (void)index;
  e->rc = autograph_frontier_fork_join(e->graph, e->owner, e->a, e->b);
}

static void test_fork_join_parent_holds_workers(const FjGraph *g,
                                                int32_t partitions) {
  sgpl_runtime_op a_ops[1], b_ops[1];
  PairOpState sa = {.activate = 0, .count_slot = 0};
  PairOpState sb = {.activate = 0, .count_slot = 1};
  sgpl_exec_ctx owner, a, b;
  OuterEnv oe;
  int32_t ledger_before, ledger_after;

  autograph_build_clean_cut((void *)g, partitions);
  atomic_store(&g_pairs_u, 0);
  atomic_store(&g_pairs_v, 0);

  memset(a_ops, 0, sizeof(a_ops));
  a_ops[0].capabilities = SGPL_OP_PAIR;
  a_ops[0].state = &sa;
  a_ops[0].pair = fj_pair;

  memset(b_ops, 0, sizeof(b_ops));
  b_ops[0].capabilities = SGPL_OP_PAIR;
  b_ops[0].state = &sb;
  b_ops[0].pair = fj_pair;

  init_resource_ctx(&owner, NULL, 0);
  owner.dest_seen = NULL;
  owner.next_frontier = NULL;
  owner.append_head = NULL;
  owner.run_round_begin = 0;
  owner.run_round_end = 0;

  init_resource_ctx(&a, a_ops, 1);
  a.dest_seen = NULL;
  a.next_frontier = NULL;
  a.append_head = NULL;
  a.traversal_kind = SGPL_TRAVERSE_OWNER_U;

  init_resource_ctx(&b, b_ops, 1);
  b.dest_seen = NULL;
  b.next_frontier = NULL;
  b.append_head = NULL;
  b.traversal_kind = SGPL_TRAVERSE_OWNER_V;

  oe.graph = (void *)g;
  oe.owner = &owner;
  oe.a = &a;
  oe.b = &b;
  oe.rc = 0;

  ledger_before = sgpl_debug_reserved_threads();

  /* Outer parallel loop reserves the pool; its body forks while the budget is
   * held. */
  parallel_for_runtime(0, 1, 1, outer_body, &oe, 0, 0);

  ledger_after = sgpl_debug_reserved_threads();

  check(atomic_load(&g_pairs_u) == (int)g->m &&
            atomic_load(&g_pairs_v) == (int)g->m,
        "children ran under a fully reserved pool (no deadlock)");
  check(ledger_before == ledger_after, "worker budget ledger balanced");
}

static void test_nested_fork_join(const FjGraph *g, int32_t partitions) {
  uint8_t m0[8] = {1, 0, 0, 0, 0, 0, 0, 0};
  uint8_t m2[8] = {0, 0, 1, 0, 0, 0, 0, 0};
  sgpl_runtime_op owner_ops[1], a_ops[1], b_ops[1], c_ops[1], d_ops[1];
  PairOpState sa = {.activate = 0, .count_slot = 0};
  PairOpState sb = {.activate = 0, .count_slot = 1};
  sgpl_exec_ctx owner, a, b, inner_owner, c, d;
  int before_rb, before_re;
  int32_t ledger_before, ledger_after;

  autograph_build_clean_cut((void *)g, partitions);
  reset_activation();
  atomic_store(&g_pairs_u, 0);
  atomic_store(&g_pairs_v, 0);
  atomic_store(&g_pairs_c, 0);
  atomic_store(&g_pairs_d, 0);
  atomic_store(&g_inner_forked, 0);
  before_rb = atomic_load(&g_round_begin_calls);
  before_re = atomic_load(&g_round_end_calls);

  memset(owner_ops, 0, sizeof(owner_ops));
  owner_ops[0].capabilities = SGPL_OP_ROUND_BEGIN | SGPL_OP_ROUND_END;
  owner_ops[0].round_begin = fj_round_begin;
  owner_ops[0].round_end = fj_round_end;

  memset(a_ops, 0, sizeof(a_ops));
  a_ops[0].capabilities = SGPL_OP_PAIR;
  a_ops[0].state = &sa;
  a_ops[0].pair = fj_pair;

  memset(b_ops, 0, sizeof(b_ops));
  b_ops[0].capabilities = SGPL_OP_PAIR;
  b_ops[0].state = &sb;
  b_ops[0].pair = fj_pair_nested;

  memset(c_ops, 0, sizeof(c_ops));
  c_ops[0].capabilities = SGPL_OP_PAIR;
  c_ops[0].pair = fj_pair_c;

  memset(d_ops, 0, sizeof(d_ops));
  d_ops[0].capabilities = SGPL_OP_PAIR;
  d_ops[0].pair = fj_pair_d;

  init_resource_ctx(&owner, owner_ops, 1);
  owner.run_round_begin = 1;
  owner.run_round_end = 1;
  owner.traversal_kind = SGPL_TRAVERSE_OWNER_U;

  init_resource_ctx(&a, a_ops, 1);
  a.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  a.membership = m0;

  init_resource_ctx(&b, b_ops, 1);
  b.traversal_kind = SGPL_TRAVERSE_OWNER_V;

  /* Inner owner: non-owning shallow share of the top round resources. */
  init_resource_ctx(&inner_owner, NULL, 0);
  inner_owner.run_round_begin = 0;
  inner_owner.run_round_end = 0;
  inner_owner.traversal_kind = SGPL_TRAVERSE_OWNER_U;

  init_resource_ctx(&c, c_ops, 1);
  c.traversal_kind = SGPL_TRAVERSE_OWNER_U;
  c.membership = m0;

  init_resource_ctx(&d, d_ops, 1);
  d.traversal_kind = SGPL_TRAVERSE_OWNER_V;
  d.membership = m2;

  g_inner_owner = &inner_owner;
  g_inner_c = &c;
  g_inner_d = &d;
  g_inner_graph = (void *)g;

  ledger_before = sgpl_debug_reserved_threads();
  (void)autograph_frontier_fork_join((void *)g, &owner, &a, &b);
  ledger_after = sgpl_debug_reserved_threads();

  check(atomic_load(&g_round_begin_calls) == before_rb + 1 &&
            atomic_load(&g_round_end_calls) == before_re + 1,
        "nested: round lifecycle ran exactly once");
  check(atomic_load(&g_pairs_u) == 2 && atomic_load(&g_pairs_v) == (int)g->m &&
            atomic_load(&g_pairs_c) == 2 && atomic_load(&g_pairs_d) == 2,
        "nested: outer children and inner subtree completed");
  check(owner.next_size == 4 && next_frontier_set_size() == 4,
        "nested: inner activation unioned the frontier set");
  check(g_dest_seen[1] && g_dest_seen[2] && g_dest_seen[5] && g_dest_seen[6],
        "nested: dest_seen matches the inner activated set");
  check(ledger_before == ledger_after, "nested: worker budget ledger balanced");
}

int main(void) {
  int64_t row_ptr[8] = {0, 2, 4, 6, 9, 11, 12, 12};
  int32_t col_idx[12] = {1, 5, 0, 3, 2, 6, 1, 4, 5, 0, 6, 3};
  FjGraph g = {
      .n = 7, .m = 12, .row_ptr = row_ptr, .col_idx = col_idx,
      .weights = NULL, .directed = 1, .in_row_ptr = NULL, .in_col_idx = NULL,
  };
  int32_t dest_seen[16];
  int32_t next_frontier[16];
  const int32_t partitions[] = {1, 3, 8};

  g_dest_seen = dest_seen;
  g_next_frontier = next_frontier;
  autograph_init(&g, g.n, g.m, NULL, NULL, NULL);

  signal(SIGALRM, on_alarm);
  alarm(30); /* watchdog: a deadlock fails instead of hanging */

  for (size_t pi = 0; pi < sizeof(partitions) / sizeof(partitions[0]); ++pi) {
    printf("== partitions=%d ==\n", partitions[pi]);
    test_fork_join_activation(&g, partitions[pi]);
    test_fork_join_parent_holds_workers(&g, partitions[pi]);
    test_nested_fork_join(&g, partitions[pi]);
  }

  alarm(0);
  printf("fork/join + budget test: %d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
