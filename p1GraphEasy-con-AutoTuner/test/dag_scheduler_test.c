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
#include "../parallel_runtime.h"

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

/* §11.4 product cells: every (unit, partition) cell runs exactly once; the
 * callback receives the packed instance `unit * partitions + partition`. */
static _Atomic int g_cell_seen[16];
static _Atomic int g_cell_count;
static int32_t product_cell_fn(void *state, int64_t instance) {
  (void)state;
  if (instance >= 0 && instance < 16)
    atomic_store(&g_cell_seen[instance], 1);
  atomic_fetch_add(&g_cell_count, 1);
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

typedef struct async_fixture {
  int submitted[4], done[4], polls[4], active, peak, fail, errors;
} async_fixture;
static int32_t async_submit(void *opaque,int64_t node) {
  async_fixture *s=opaque;s->submitted[node]++;
  if(node>=2&&(!s->done[0]||!s->done[1]))s->errors++;
  if(node==3) {s->done[node]=1;return 0;} /* synchronous host finalization */
  if(++s->active>s->peak)s->peak=s->active;
  return SGPL_DAG_PENDING;
}
static int32_t async_poll(void *opaque,int64_t node) {
  async_fixture *s=opaque;
  if(++s->polls[node]<(node==1?4:2))return SGPL_DAG_PENDING;
  s->done[node]=1;s->active--;
  return s->fail&&node==0?-1:0;
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
  for (int i = 0; i < 16; ++i)
    atomic_store(&g_cell_seen[i], 0);
  atomic_store(&g_cell_count, 0);
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

static int32_t temporal_fail_fn(void *state, int64_t node) {
  (void)state;
  atomic_fetch_add(&g_seen[node], 1);
  return SGPL_DAG_ERR_NODE;
}

int main(void) {
  reset();
  check("temporal components accept an empty domain",
        sgpl_exec_temporal_components(node_fn, NULL, 0, 4) == SGPL_DAG_OK && g_order_n == 0);
  check("temporal components reject a missing callback",
        sgpl_exec_temporal_components(NULL, NULL, 2, 4) == SGPL_DAG_ERR_INVALID);
  check("temporal components execute each member once",
        sgpl_exec_temporal_components(node_fn, NULL, 3, 1) == SGPL_DAG_OK &&
        g_seen[0] == 1 && g_seen[1] == 1 && g_seen[2] == 1);
  reset();
  check("temporal component failure is returned without replay",
        sgpl_exec_temporal_components(temporal_fail_fn, NULL, 3, 1) == SGPL_DAG_ERR_NODE &&
        g_seen[0] + g_seen[1] + g_seen[2] == 1);
  reset();
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

  /* ── §11 proof-derived graph execution ────────────────────────────────
   * §11.1: proof metadata on an edge (axis mask, relation kind, access kind,
   * phase/round/partition projection) is accepted; a relation a transformation
   * removed must never be scheduled; a unary obligation has no DAG; a mutex
   * edge must be tagged as a realization order. */
  {
    sgpl_dag_relation_desc meta[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 1, 7, SGPL_DAG_REL_FLAG_SEMANTIC,
         SGPL_DAG_AXIS_SPATIAL, SGPL_DAG_RELKIND_MEMORY, 0 /* RAW */, 0,
         1, 2, 0, 0, 1},
    };
    sgpl_dag_template TM = {2, 1, meta, node_fn, NULL};
    reset();
    check("proof metadata accepted (axis/relation/phase/round/partition)",
          autograph_execute_dag(&TM, NULL, 2) == SGPL_DAG_OK && g_order_n == 2);
    sgpl_dag_relation_desc dis[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 1, 7, SGPL_DAG_REL_FLAG_SEMANTIC,
         SGPL_DAG_AXIS_SPATIAL, SGPL_DAG_RELKIND_MEMORY, 0, 3 /* discharged */,
         0, 0, 0, 0, 0},
    };
    sgpl_dag_template TD = {2, 1, dis, node_fn, NULL};
    check("a discharged relation is never scheduled",
          autograph_execute_dag(&TD, NULL, 2) == SGPL_DAG_ERR_INVALID);

    sgpl_dag_relation_desc uni[1] = {
        {SGPL_DAG_KIND_SELF, 0, 0, 0, 0, SGPL_DAG_REL_FLAG_REALIZATION,
         SGPL_DAG_AXIS_SPATIAL, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    sgpl_dag_template TU = {1, 1, uni, node_fn, NULL};
    check("a unary obligation cannot be scheduled",
          autograph_execute_dag(&TU, NULL, 2) == SGPL_DAG_ERR_INVALID);

    sgpl_dag_relation_desc mx[1] = {
        {SGPL_DAG_KIND_MUTEX, 0, 0, 1, 0, 0 /* no realization tag */,
         SGPL_DAG_AXIS_SPATIAL, 0, 2 /* WAW */, 0, 0, 0, 0, 0, 0},
    };
    sgpl_dag_template TX = {2, 1, mx, node_fn, NULL};
    check("mutex edge without a realization tag rejected",
          autograph_execute_dag(&TX, NULL, 2) == SGPL_DAG_ERR_INVALID);
    mx[0].flags = SGPL_DAG_REL_FLAG_REALIZATION;
    reset();
    check("mutex edge with a realization tag scheduled as an order",
          autograph_execute_dag(&TX, NULL, 2) == SGPL_DAG_OK && g_order_n == 2);

    sgpl_dag_relation_desc contra[2] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC,
         0, 0, 0, 0, 0, 0, 0, 0, 0},
        {SGPL_DAG_KIND_MUTEX, 0, 0, 1, 0, SGPL_DAG_REL_FLAG_REALIZATION,
         0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    sgpl_dag_template TC2 = {2, 2, contra, node_fn, NULL};
    check("duplicate contradictory edges rejected",
          autograph_execute_dag(&TC2, NULL, 2) == SGPL_DAG_ERR_INVALID);

    sgpl_dag_relation_desc badaxis[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC,
         0x8u /* unknown axis bit */, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    sgpl_dag_template TA = {2, 1, badaxis, node_fn, NULL};
    check("unknown axis bit rejected",
          autograph_execute_dag(&TA, NULL, 2) == SGPL_DAG_ERR_INVALID);
    reset();
  }

  /* §11.2: the spatial graph entry point validates its axis and shape before
   * any dispatch (the partition body itself needs a real context and is
   * exercised by the corpus). */
  {
    sgpl_exec_ctx bad;
    sgpl_dag_relation_desc wrongaxis[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC,
         SGPL_DAG_AXIS_TEMPORAL, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    memset(&bad, 0, sizeof(bad));
    check("spatial graph rejects a temporal-axis edge",
          sgpl_exec_dag_spatial_graph(&bad, 2, wrongaxis, 1, 2) ==
              SGPL_DAG_ERR_INVALID);
    check("spatial graph rejects zero partitions",
          sgpl_exec_dag_spatial_graph(&bad, 0, NULL, 0, 2) ==
              SGPL_DAG_ERR_INVALID);
    check("spatial graph rejects a null relation array",
          sgpl_exec_dag_spatial_graph(&bad, 2, NULL, 1, 2) ==
              SGPL_DAG_ERR_INVALID);
  }

  /* §11.3: temporal units as a partial DAG (independent units overlap) */
  {
    sgpl_exec_ctx c1, c2;
    sgpl_temporal_unit two[2];
    char g1[64] = {0}, g2[64] = {0};
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));
    two[0].graph = g1; two[0].ctx = &c1;
    two[1].graph = g2; two[1].ctx = &c2;
    sgpl_dag_relation_desc carried[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 1, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC,
         SGPL_DAG_AXIS_TEMPORAL, SGPL_DAG_RELKIND_MEMORY, 2 /* WAW */, 0,
         1, 1, 1, 0, 0},
    };
    check("temporal partial DAG with a round-carried edge runs",
          sgpl_exec_dag_temporal_units(two, 2, carried, 1, 2) == SGPL_DAG_OK);
    carried[0].axis_mask = SGPL_DAG_AXIS_SPATIAL;
    check("temporal units reject a spatial-axis edge",
          sgpl_exec_dag_temporal_units(two, 2, carried, 1, 2) ==
              SGPL_DAG_ERR_INVALID);
  }

  /* §11.4: product execution over (unit x partition) */
  {
    sgpl_exec_ctx c1, c2;
    sgpl_temporal_unit two[2];
    char g1[64] = {0}, g2[64] = {0};
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));
    two[0].graph = g1; two[0].ctx = &c1;
    two[1].graph = g2; two[1].ctx = &c2;
    sgpl_dag_relation_desc sp[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 2, 5, SGPL_DAG_REL_FLAG_SEMANTIC,
         SGPL_DAG_AXIS_SPATIAL, SGPL_DAG_RELKIND_MEMORY, 1 /* WAR */, 0,
         1, 1, 0, 0, 2},
    };
    sgpl_dag_relation_desc tp[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 1, 0, 1, 1, SGPL_DAG_REL_FLAG_SEMANTIC,
         SGPL_DAG_AXIS_TEMPORAL, SGPL_DAG_RELKIND_MEMORY, 2, 0, 1, 1, 1, 0, 0},
    };
    reset();
    rc = sgpl_exec_dag_product(two, 2, 3, product_cell_fn, sp, 1, tp, 1, 4);
    {
      int all = 1;
      for (i = 0; i < 6; ++i)
        if (!atomic_load(&g_cell_seen[i]))
          all = 0;
      check("product runs every (unit, partition) cell exactly once",
            rc == SGPL_DAG_OK && atomic_load(&g_cell_count) == 6 && all);
    }
    sgpl_dag_relation_desc oob[1] = {
        {SGPL_DAG_KIND_PRECEDENCE, 0, 0, 5 /* >= partitions */, 5,
         SGPL_DAG_REL_FLAG_SEMANTIC, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    check("product rejects an out-of-range partition edge",
          sgpl_exec_dag_product(two, 2, 3, product_cell_fn, oob, 1, NULL, 0, 4) ==
              SGPL_DAG_ERR_INVALID);
    check("product rejects a null cell callback",
          sgpl_exec_dag_product(two, 2, 3, NULL, NULL, 0, NULL, 0, 4) ==
              SGPL_DAG_ERR_INVALID);
    reset();
  }

  {
    sgpl_dag_relation_desc edges[3]={
      {.source_node=0,.sink_node=2,.flags=SGPL_DAG_REL_FLAG_REALIZATION},
      {.source_node=1,.sink_node=2,.flags=SGPL_DAG_REL_FLAG_REALIZATION},
      {.source_node=2,.sink_node=3,.flags=SGPL_DAG_REL_FLAG_REALIZATION}};
    int32_t weights[4]={1,1000,1,1};
    sgpl_dag_template device={.node_count=4,.relation_count=3,.relations=edges,
      .node_fn=node_fn,.node_weights=weights};
    async_fixture s={0};sgpl_push_thread_budget(1);
    check("async DAG overlaps independent work under a serial CPU grant",
      autograph_execute_dag_async(&device,&s,async_submit,async_poll)==0&&s.peak==2&&
      !s.errors&&s.done[3]&&sgpl_current_thread_budget()==1&&!sgpl_debug_reserved_threads());
    for(int i=0;i<4;++i)check("async DAG submits each node once",s.submitted[i]==1);
    sgpl_pop_thread_budget();
    s=(async_fixture){.fail=1};
    check("async failure cancels successors and joins submitted work",
      autograph_execute_dag_async(&device,&s,async_submit,async_poll)==SGPL_DAG_ERR_NODE&&
      !s.active&&s.done[0]&&s.done[1]&&!s.submitted[2]&&!s.submitted[3]);
    reset();
    check("validation checks topology without running callbacks",
      autograph_validate_dag(&device)==0&&atomic_load(&g_order_n)==0);
    edges[2].source_node=2;edges[2].sink_node=0;
    check("validation rejects a cycle before execution",autograph_validate_dag(&device)==SGPL_DAG_ERR_CYCLE);
  }
  printf("%s (%d failure%s)\n", failures ? "FAILURES" : "ALL PASS", failures,
         failures == 1 ? "" : "s");
  return failures ? 1 : 0;
}
