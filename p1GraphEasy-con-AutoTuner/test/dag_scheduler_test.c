/* dag_scheduler_test.c -- V1 ready-work DAG scheduler unit test.
 * Proves: dependency order (multi- and single-worker), cancellation on node
 * failure, cycle detection, semantic-edge/witness validation, realization
 * edges.  Run: SGPL_DAG_TEST=1 ./dag_scheduler_test  (no env needed). */
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
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
static _Atomic int g_order_n;
static _Atomic int64_t g_seen[8];
static int g_fail_on = -1;

/* §19 partial-DAG overlap: when enabled, the two independent nodes (0 and 1)
 * rendezvous, so the test observes real overlap rather than a lucky order. */
static atomic_int g_arrived;
static int g_barrier_on;
static atomic_int g_overlap01;
static atomic_int g_pred_ready; /* node 2 saw both predecessors completed */

/* §15 grants observed *inside* the nodes (the pushed sibling budget).  Every
 * node writes its own slot; the whole set is read after the run's join. */
static int g_weights_on;
static int g_share_by_node[8];
static atomic_int g_light_granted;

/* §15 nested grant: a dispatch made *inside* a node observes the node's own
 * ledger grant, never the whole machine. */
static int g_nested_on;
static int g_nested_share;
static int32_t nested_probe_fn(void *state, int64_t node) {
  (void)state;
  (void)node;
  g_nested_share = sgpl_current_thread_budget();
  return 0;
}

static int32_t node_fn(void *state, int64_t node) {
  (void)state;
  if ((int)node == g_fail_on)
    return 1;
  if (g_nested_on && (int)node == 0) {
    sgpl_dag_template NT = {1, 0, NULL, nested_probe_fn, NULL};
    (void)autograph_execute_dag(&NT, NULL, 64);
  }
  if (g_weights_on) {
    int32_t b = sgpl_current_thread_budget();
    g_share_by_node[(int)node] = b;
    if ((int)node == 0)
      atomic_store(&g_light_granted, 1);
    else if ((int)node == 1)
      while (!atomic_load(&g_light_granted)) {
        /* bounded: never blocks the run when the light node was not dispatched */
        static _Atomic int spins;
        if (atomic_fetch_add(&spins, 1) > 1000000000)
          break;
      }
  }
  if (g_barrier_on && (node == 0 || node == 1)) {
    atomic_fetch_add(&g_arrived, 1);
    for (long spin = 0; spin < 1000000000L; spin++)
      if (atomic_load(&g_arrived) >= 2)
        break;
    if (atomic_load(&g_arrived) >= 2)
      atomic_store(&g_overlap01, 1);
  }
  if (node == 2 && atomic_load(&g_seen[0]) && atomic_load(&g_seen[1]))
    atomic_store(&g_pred_ready, 1);
  {
    int slot = atomic_fetch_add(&g_order_n, 1);
    if (slot < 8)
      g_order[slot] = (int)node;
  }
  if (node >= 0 && node < 8)
    atomic_store(&g_seen[node], 1);
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
  atomic_store(&g_order_n, 0);
  g_fail_on = -1;
  atomic_store(&g_arrived, 0);
  g_barrier_on = 0;
  atomic_store(&g_overlap01, 0);
  atomic_store(&g_pred_ready, 0);
  g_weights_on = 0;
  memset(g_share_by_node, 0, sizeof(g_share_by_node));
  atomic_store(&g_light_granted, 0);
  g_nested_on = 0;
  g_nested_share = 0;
}

int sgpl_gpu_engine_step_verdict(int64_t arcs, int64_t min_pairs) {
  (void)arcs; (void)min_pairs;
  return 1; /* SGPL_GPU_SMALL_TRIPS: tests stay on the CPU path */
}
int sgpl_gpu_step_schedule_ok(int32_t spatial, int32_t temporal) {
  (void)spatial; (void)temporal;
  return 1; /* the schedule gate itself is exercised by tdg_budget_test T11 */
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

  sgpl_dag_relation_desc unk[1] = {
      {99 /* relation kind the runtime does not implement */, 0, 0, 1, 1,
       SGPL_DAG_REL_FLAG_SEMANTIC},
  };
  sgpl_dag_template TU = {2, 1, unk, node_fn};
  check("unsupported relation kind rejected",
        autograph_execute_dag(&TU, NULL, 2) == SGPL_DAG_ERR_INVALID);

  sgpl_dag_relation_desc noflag[1] = {
      {SGPL_DAG_REL_PRECEDENCE, 0, 0, 1, 1, 0 /* no semantic/realization flag */},
  };
  sgpl_dag_template TN = {2, 1, noflag, node_fn};
  check("edge with no semantic/realization flag rejected",
        autograph_execute_dag(&TN, NULL, 2) == SGPL_DAG_ERR_INVALID);

  /* Section 19 partial DAGs: one edge 0 -> 2 with node 1 independent.  The
   * runtime treats the temporal-unit case (semantic edge, witness 1) and the
   * spatial-partition case (realization edge, witness 0) identically: the
   * dependent node waits for its predecessor, the independent node overlaps it
   * (observed through the rendezvous, not a lucky order), and every node runs.
   * With one worker there is no overlap but the order stays legal. */
  {
    sgpl_dag_relation_desc sem[1] = {
        {SGPL_DAG_REL_PRECEDENCE, 0, 0, 2, 1, SGPL_DAG_REL_FLAG_SEMANTIC},
    };
    sgpl_dag_relation_desc rea[1] = {
        {SGPL_DAG_REL_PRECEDENCE, 0, 0, 2, 0, SGPL_DAG_REL_FLAG_REALIZATION},
    };
    sgpl_dag_relation_desc *edges[2] = {sem, rea};
    const char *what[2] = {"temporal partial DAG (U1->U3, U2 independent)",
                           "spatial partial DAG (P1->P3, P2 independent)"};
    char label[160];
    for (int k = 0; k < 2; k++) {
      sgpl_dag_template TP = {3, 1, edges[k], node_fn, NULL};
      int pos0 = -1, pos2 = -1;
      reset();
      g_barrier_on = 1;
      rc = autograph_execute_dag(&TP, NULL, 4);
      for (i = 0; i < g_order_n; i++) {
        if (g_order[i] == 0) pos0 = i;
        if (g_order[i] == 2) pos2 = i;
      }
      snprintf(label, sizeof(label), "%s: overlap + dependent waits", what[k]);
      check(label, rc == SGPL_DAG_OK && g_order_n == 3 &&
                       atomic_load(&g_overlap01) && atomic_load(&g_pred_ready) &&
                       pos0 >= 0 && pos2 > pos0);
      reset();
      rc = autograph_execute_dag(&TP, NULL, 1);
      snprintf(label, sizeof(label), "%s: single worker legal order", what[k]);
      check(label, rc == SGPL_DAG_OK && g_order_n == 3 &&
                       !atomic_load(&g_overlap01) && g_order[2] == 2);
    }
    reset();
  }

  /* Section 15: the grant handed to a node inside the scheduler is the
   * ready-set share (never the whole machine), the node's work-estimate weight
   * caps it (an estimate error can only under-grant), no node observes less
   * than one thread, and a dispatch made *inside* a node can never exceed the
   * node's own grant. */
  {
    int32_t w[2] = {1, 100};
    sgpl_dag_template TW = {2, 0, NULL, node_fn, w};
    reset();
    g_weights_on = 1;
    rc = autograph_execute_dag(&TW, NULL, 8);
    check("budget: weight ceiling caps the light node to a serial grant",
          rc == SGPL_DAG_OK && g_share_by_node[0] == 1);
    check("budget: heavy node never granted less than the light one",
          g_share_by_node[1] >= g_share_by_node[0] && g_share_by_node[1] <= 4);
    {
      int mn = 0, mx = 0;
      for (i = 0; i < TW.node_count; i++) {
        if (g_share_by_node[i] == 0)
          continue;
        if (mn == 0 || g_share_by_node[i] < mn)
          mn = g_share_by_node[i];
        if (g_share_by_node[i] > mx)
          mx = g_share_by_node[i];
      }
      check("budget: every grant within [1, run budget]", mn >= 1 && mx <= 8);
    }
    reset();
    g_weights_on = 1;
    g_nested_on = 1;
    rc = autograph_execute_dag(&TW, NULL, 8);
    check("budget: nested dispatch capped by the node's grant",
          rc == SGPL_DAG_OK && g_nested_share >= 1 &&
              g_nested_share <= g_share_by_node[0]);
    reset();
  }

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
