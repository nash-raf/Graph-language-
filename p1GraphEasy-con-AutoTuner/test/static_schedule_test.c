/* Hand-built descriptors exercise runtime validation. Compiler certificates
 * are tested separately; instrumentation here deliberately observes grants. */
#define SGPL_FULLGRAPH_TEST_MAIN fullgraph_contract_main
#include "fullgraph_test.c"
static int preparation_calls,preparation_grant;
static void schedule_test_gate(sgpl_runtime_op *op,sgpl_exec_ctx *c,int32_t u) {
  ++preparation_calls;preparation_grant=sgpl_current_thread_budget();
  full_test_gate(op,c,u);
}
int main(void) {
  if(fullgraph_contract_main())return 1;
  int64_t rows[]={0,2,4,5,6,7,8,8};int32_t columns[]={2,1,1,1,1,3,2,2};
  int32_t pairs[]={0,2,0,1,1,1,1,1,2,1,3,3,4,2,5,2};
  ExecTestGraph g={.n=7,.m=8,.row_ptr=rows,.col_idx=columns,.directed=1};native_pairs=pairs;
  autograph_init(&g,7,8,NULL,NULL,pairs);
  autograph_ensure_layout(&g,7,8,rows,columns,&fixture_nodes,&fixture_edges,pairs,LAYOUT_CSR);
  autograph_build_clean_cut(&g,6);
  sgpl_static_schedule_v1 plan={SGPL_STATIC_SCHEDULE_VERSION,SGPL_SCHEDULE_PREPARE_OWNERS_JOIN,
    SGPL_SCHEDULE_EFFECT_PROOF|SGPL_SCHEDULE_CONTEXT_PROOF,4,full_test_pair,schedule_test_gate};
  for(int specialized=0;specialized<2;++specialized)for(int disabled=0;disabled<2;++disabled) {
    FullTest a,b;sgpl_exec_ctx *x=full_test_context(&g,&a,specialized,0),*y=full_test_context(&g,&b,specialized,0);
    y->ops[0].source_begin=schedule_test_gate;
    autograph_exec_ctx_bind_static_schedule_v1(y,&plan,SGPL_EXECUTION_ABI_VERSION,sizeof(*y),sizeof(sgpl_runtime_op));
    check(y->static_schedule==&plan,"bind matching effect-derived schedule");
    if(disabled)setenv("SGPL_NO_STATIC_SCHEDULE","1",1);else unsetenv("SGPL_NO_STATIC_SCHEDULE");
    preparation_calls=0;preparation_grant=0;
    int outer=sgpl_current_thread_budget();sgpl_push_thread_budget(2);
    check(autograph_frontier_execute(&g,x)==0 && autograph_frontier_execute(&g,y)==0,"execute static schedule and generic reference");
    check(preparation_calls==7,"preparation callbacks once per source including isolated source");
    check(preparation_grant>=1&&preparation_grant<=2,"preparation respects nested grant");
    sgpl_pop_thread_budget();check(sgpl_current_thread_budget()==outer,"restore enclosing thread budget");
    full_test_equal(&a,&b);
    autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
  }
  unsetenv("SGPL_NO_STATIC_SCHEDULE");
  FullTest s;sgpl_exec_ctx *c=full_test_context(&g,&s,0,0);
  sgpl_static_schedule_v1 bad=plan;bad.version++;
  autograph_exec_ctx_bind_static_schedule_v1(c,&bad,SGPL_EXECUTION_ABI_VERSION,sizeof(*c),sizeof(sgpl_runtime_op));
  check(!c->static_schedule,"refuse unknown descriptor version");
  autograph_exec_ctx_bind_static_schedule_v1(c,&plan,SGPL_EXECUTION_ABI_VERSION,sizeof(*c),sizeof(sgpl_runtime_op));
  check(!c->static_schedule,"refuse callback identity mismatch");
  c->ops[0].source_begin=schedule_test_gate;
  c->ops[0].capabilities|=SGPL_OP_SOURCE_END;
  autograph_exec_ctx_bind_static_schedule_v1(c,&plan,SGPL_EXECUTION_ABI_VERSION,sizeof(*c),sizeof(sgpl_runtime_op));
  check(!c->static_schedule,"refuse additional lifecycle effects");
  c->ops[0].capabilities=SGPL_OP_SOURCE_BEGIN;
  autograph_exec_ctx_bind_static_schedule_v1(c,&plan,SGPL_EXECUTION_ABI_VERSION,sizeof(*c),sizeof(sgpl_runtime_op));
  c->ops[0].source_begin=full_test_gate;
  check(autograph_frontier_execute(&g,c)==0,"descriptor mutation falls back before source effects");
  autograph_exec_ctx_destroy(c);
  printf("static schedule contracts: %d failures\n",failures);return failures?1:0;
}
