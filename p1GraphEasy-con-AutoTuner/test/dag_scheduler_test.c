/* dag_scheduler_test.c -- V1 ready-work DAG scheduler unit test.
 * Proves: dependency order (multi- and single-worker), cancellation on node
 * failure, cycle detection, semantic-edge/witness validation, realization
 * edges.  Run: SGPL_DAG_TEST=1 ./dag_scheduler_test  (no env needed). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "autotuner_runtime.h"

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

  printf("%s (%d failure%s)\n", failures ? "FAILURES" : "ALL PASS", failures,
         failures == 1 ? "" : "s");
  return failures ? 1 : 0;
}
