#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
static atomic_int occurrence_calls;
static int skip_appends;
static int begins_by_occurrence[16],ends_by_occurrence[16];
static void array_pair(void *unused,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)unused;(void)u;atomic_fetch_add(&occurrence_calls,1);
  if(skip_appends)return;
  autograph_frontier_activate(ctx,v);autograph_frontier_activate(ctx,v);
}
static void array_begin(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;(void)u;++begins_by_occurrence[autograph_exec_ctx_occurrence(ctx)];
}
static void array_end(sgpl_runtime_op *op,sgpl_exec_ctx *ctx,int32_t u) {
  (void)op;(void)u;++ends_by_occurrence[autograph_exec_ctx_occurrence(ctx)];
}
int main(void) {
  int64_t rp[8]={0,2,4,5,5,6,7,7};int32_t ci[7]={1,2,2,2,3,1,6};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);
  int32_t input[]={4,1,3,0,1,6};
  int32_t expected_output[]={1,1,2,2,2,2,1,1,2,2,2,2,2,2};
  for(int layout=0;layout<4;++layout)for(int source=0;source<2;++source) {
    autograph_ensure_layout(&g,7,7,rp,ci,&fixture_nodes,&fixture_edges,pairs,layout);
    check(autograph_build_clean_cut(&g,3)==3,"prepare array CleanCut topology");
    int32_t initial[2]={99,88},*out=initial,head=0;int64_t cap=2;
    atomic_store(&occurrence_calls,0);memset(begins_by_occurrence,0,sizeof begins_by_occurrence);
    skip_appends=0;
    memset(ends_by_occurrence,0,sizeof ends_by_occurrence);
    sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR|SGPL_OP_SOURCE_BEGIN|SGPL_OP_SOURCE_END,
        NULL,array_pair,NULL,array_begin,array_end,NULL,NULL,0);
    sgpl_runtime_op *ops[]={op};
    sgpl_exec_ctx *ctx=autograph_exec_ctx_create(&g,source?SGPL_TRAVERSE_OWNER_U:SGPL_TRAVERSE_OWNER_V,
        SGPL_DOMAIN_FRONTIER,NULL,NULL,out,0,&head,NULL,0,ops,1,-1);
    check(!autograph_exec_ctx_bind_array_frontier(ctx,input,6,&out,&cap,2),"bind array frontier");
    int key=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
    check(!autograph_exec_ctx_prepare_spatial(ctx,key,key,1),"reserve ordered occurrence log");
    check(autograph_frontier_execute(&g,ctx)==14,"retain duplicate append cardinality");
    check(atomic_load(&occurrence_calls)==7,"duplicate sources and edges execute exactly once per occurrence");
    check(cap>=14&&!memcmp(out,expected_output,sizeof expected_output),"compact in original source and neighbor order");
    for(int i=0;i<6;++i)check(begins_by_occurrence[i]==1&&ends_by_occurrence[i]==1,"source hooks include zero-neighbor occurrences");
    skip_appends=1;
    check(autograph_frontier_execute(&g,ctx)==0&&atomic_load(&occurrence_calls)==14,
        "reused context clears prior append validity without replaying a dispatch");
    autograph_exec_ctx_destroy(ctx);sgpl_frontier_release_buffers();
  }
  int32_t old[]={7,8},*data=old;int64_t capacity=2;
  check(!sgpl_frontier_ensure_i32(&data,&capacity,2)&&data!=old&&data[0]==7&&data[1]==8,
      "fallback grows and copies existing stack buffer");
  setenv("SGPL_FRONTIER_FAIL","grow",1);
  int32_t *before=data;int64_t before_capacity=capacity;
  check(sgpl_frontier_ensure_i32(&data,&capacity,capacity)==SGPL_DAG_ERR_NOMEM&&data==before&&capacity==before_capacity,
      "failed growth preserves the old allocation and its capacity");
  unsetenv("SGPL_FRONTIER_FAIL");sgpl_frontier_release_buffers();
  data=old;capacity=2;
  check(!sgpl_frontier_reserve_i32(&g,input,6,&data,&capacity,2,1)&&capacity==15&&data[0]==7,
      "symbolic traversal bound includes duplicate source occurrences and the existing prefix");
  sgpl_frontier_release_buffers();data=old;capacity=2;
  setenv("SGPL_FRONTIER_FAIL","alloc",1);
  check(sgpl_frontier_reserve_i32(&g,input,6,&data,&capacity,2,1)==SGPL_DAG_ERR_NOMEM&&data==old,
      "reservation failure leaves the original buffer before any effect");
  unsetenv("SGPL_FRONTIER_FAIL");
  printf("ordered array frontier: %d failures\n",failures);return failures?1:0;
}
