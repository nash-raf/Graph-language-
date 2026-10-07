/* dag_scheduler_test.c -- V1 ready-work DAG scheduler unit test.
 * Proves: dependency order (multi- and single-worker), cancellation on node
 * failure, cycle detection, semantic-edge/witness validation, realization
 * edges.  Run: SGPL_DAG_TEST=1 ./dag_scheduler_test  (no env needed). */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "autotuner_runtime.h"

/* Linker stubs: this unit test never enters the roaring-backed motif paths
 * (the fake graph is unregistered, so the engine early-returns). */
typedef struct RoaringBitmap RoaringBitmap;
RoaringBitmap *roaring_bitmap_create(uint64_t a, uint64_t b) {
  (void)a; (void)b; return NULL;
}
void roaring_bitmap_add(RoaringBitmap *r, uint32_t v) { (void)r; (void)v; }
void roaring_bitmap_remove(RoaringBitmap *r, uint32_t v) { (void)r; (void)v; }
bool roaring_bitmap_contains(const RoaringBitmap *r, uint32_t v) {
  (void)r; (void)v; return false;
}
void roaring_bitmap_clear(RoaringBitmap *r) { (void)r; }
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *r) {
  (void)r; return 0;
}
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *r, uint64_t i) {
  (void)r; (void)i; return 0;
}
RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *p) {
  (void)p; return NULL;
}
void roaring_bitmap_or_inplace(RoaringBitmap *d, RoaringBitmap *s) {
  (void)d; (void)s;
}
void roaring_bitmap_free(RoaringBitmap *r) { (void)r; }
void roaring_bitmap_set_thread_local_overrides(RoaringBitmap **o,
                                               RoaringBitmap **n, int32_t c) {
  (void)o; (void)n; (void)c;
}
void roaring_bitmap_clear_thread_local_overrides(void) {}
int32_t *graph_get_edge_pairs(void *g) { (void)g; return NULL; }
int64_t graph_get_num_edge_pairs(void *g) { (void)g; return 0; }

static int g_order[8];
static int g_order_n;
static int64_t g_seen[8];
static int g_fail_on = -1;

static int32_t node_fn(void *state, int64_t node) {
  (void)state;
  if ((int)node == g_fail_on)
    return 1;
  if (g_order_n < 8)
    g_order[g_order_n++] = (int)node;
  if (node >= 0 && node < 8)
    g_seen[node] = 1;
  return 0;
}

static int failures;
static void check(const char *name, int ok) {
  printf("%s %s\n", ok ? "PASS" : "FAIL", name);
  if (!ok)
    failures++;
}

static void reset(void) {
  memset(g_order, 0, sizeof(g_order));
  memset(g_seen, 0, sizeof(g_seen));
  g_order_n = 0;
  g_fail_on = -1;
}

int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs) {
  (void)arcs; (void)min_pairs;
  return 1; /* SGPL_GPU_SMALL_TRIPS: tests stay on the CPU path */
}
int gpup_step_try(const char *kernel_name, const int32_t *pairs, int64_t npairs) {
  (void)kernel_name; (void)pairs; (void)npairs;
  return 0;
}
int gpup_step_v_try(const char *kernel_name, const void *layout_sig, int32_t npart,
                    int64_t *const *rp, const int32_t *const *ci,
                    const int32_t *const *indir, const int64_t *row_counts,
                    const uint8_t *mem, int64_t nmem, const uint8_t **claimed_out) {
  (void)kernel_name; (void)layout_sig; (void)npart; (void)rp; (void)ci;
  (void)indir; (void)row_counts; (void)mem; (void)nmem; (void)claimed_out;
  return 0;
}

/* GPU runtime hook stubs: these tests never take the device path. */
const char *autograph_gpu_step_name_for(int32_t step_id) { (void)step_id; return NULL; }

int main(void) {
  sgpl_dag_relation_desc rels[3] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 2, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
      {SGPL_DAG_REL_PRECEDENCE, 0, 1, 2, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
      {SGPL_DAG_REL_PRECEDENCE, 0, 2, 3, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
  };
  sgpl_dag_template T = {4, 3, rels, node_fn};
  int pos[4], i;
  reset();
  int rc = autograph_execute_dag(&T, NULL, 4);
  for (i = 0; i < 4; i++) pos[i] = -1;
  for (i = 0; i < g_order_n; i++) if (g_order[i] < 4) pos[g_order[i]] = i;
  check("diamond executes all nodes", rc == SGPL_DAG_OK && g_order_n == 4);
  check("diamond respects dependencies", pos[2] > pos[0] && pos[2] > pos[1] && pos[3] > pos[2]);

  reset();
  rc = autograph_execute_dag(&T, NULL, 1);
  for (i = 0; i < 4; i++) pos[i] = -1;
  for (i = 0; i < g_order_n; i++) if (g_order[i] < 4) pos[g_order[i]] = i;
  check("single worker respects dependencies", rc == SGPL_DAG_OK && pos[2] > pos[0] && pos[2] > pos[1] && pos[3] > pos[2]);

  sgpl_dag_relation_desc cyc[2] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
      {SGPL_DAG_REL_PRECEDENCE, 0, 1, 0, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
  };
  sgpl_dag_template TC = {2, 2, cyc, node_fn};
  check("cycle fails closed", autograph_execute_dag(&TC, NULL, 2) == SGPL_DAG_ERR_CYCLE);

  sgpl_dag_relation_desc bad[1] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 1, 0, SGPL_DAG_REL_FLAG_SEMANTIC},
  };
  sgpl_dag_template TB = {2, 1, bad, node_fn};
  check("semantic edge without witness rejected", autograph_execute_dag(&TB, NULL, 2) == SGPL_DAG_ERR_INVALID);

  sgpl_dag_relation_desc self[1] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 0, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
  };
  sgpl_dag_template TS = {1, 1, self, node_fn};
  check("self edge rejected", autograph_execute_dag(&TS, NULL, 2) == SGPL_DAG_ERR_CYCLE);

  sgpl_dag_relation_desc chain[2] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
      {SGPL_DAG_REL_PRECEDENCE, 0, 1, 2, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
  };
  sgpl_dag_template TF = {3, 2, chain, node_fn};
  reset();
  g_fail_on = 0;
  check("node failure fails closed", autograph_execute_dag(&TF, NULL, 2) == SGPL_DAG_ERR_NODE);
  check("failure cancels successors", g_seen[1] == 0 && g_seen[2] == 0);
  reset();

  sgpl_dag_relation_desc real[1] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 1, 0, SGPL_DAG_REL_FLAG_REALIZATION},
  };
  sgpl_dag_template TR = {2, 1, real, node_fn};
  check("realization-order edge accepted", autograph_execute_dag(&TR, NULL, 2) == SGPL_DAG_OK);

  /* Step 8: temporal unit wrappers.  Unusable units fail closed; a
   * valid-but-unregistered graph is an engine early-return, still scheduled
   * through the chain. */
  {
    sgpl_exec_ctx tc;
    sgpl_temporal_unit one[1];
    char fake_graph[64] = {0};
    memset(&tc, 0, sizeof(tc));
    one[0].graph = NULL;
    one[0].ctx = &tc;
    check("temporal null-graph unit fails closed",
          sgpl_exec_dag_temporal_chain(one, 1, 1) == SGPL_DAG_ERR_NODE);
    check("temporal empty set rejected",
          sgpl_exec_dag_temporal_chain(one, 0, 1) == SGPL_DAG_ERR_INVALID);
    check("temporal null set rejected",
          sgpl_exec_dag_temporal_chain(NULL, 3, 1) == SGPL_DAG_ERR_INVALID);
    one[0].graph = fake_graph;
    check("temporal single unit accepted",
          sgpl_exec_dag_temporal_chain(one, 1, 2) == SGPL_DAG_OK);
    one[0].ctx = NULL;
    check("temporal null-ctx unit fails closed",
          sgpl_exec_dag_temporal_chain(one, 1, 2) == SGPL_DAG_ERR_NODE);
  }

  printf("%s (%d failure%s)\n", failures ? "FAILURES" : "ALL PASS", failures,
         failures == 1 ? "" : "s");
  return failures ? 1 : 0;
}
