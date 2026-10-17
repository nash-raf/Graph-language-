/* Run the shared array semantics fixture with compiler-proved spatial axes. */
#include "../autotuner_runtime.h"
static sgpl_exec_ctx *declare_spatial_context(sgpl_exec_ctx *ctx) {
  if (ctx) autograph_exec_ctx_set_dag_axes(ctx, 1, -1);
  return ctx;
}
#define autograph_exec_ctx_create(...) \
  declare_spatial_context((autograph_exec_ctx_create)(__VA_ARGS__))
#define SGPL_ARRAY_FRONTIER_TEST_MAIN array_fixture_main
#include "array_frontier_exec_test.c"
#undef autograph_exec_ctx_create

int main(int argc, char **argv) {
  (void)argv;
  int nested = argc > 1;
  if (nested) sgpl_push_thread_budget(2);
  int rc = array_fixture_main();
  check(sgpl_current_thread_budget() == (nested ? 2 : 0),
        "graph execution preserves the enclosing budget");
  if (nested) sgpl_pop_thread_budget();
  check(!sgpl_current_thread_budget(), "parent budget restored after graph work");
  return rc || failures;
}
