#define SGPL_ARRAY_FRONTIER_TEST_MAIN unused_array_contract_main
#include "array_frontier_exec_test.c"
static sgpl_exec_ctx *make_epoch_context(ExecTestGraph *g,const int32_t *input,int count,int32_t **out,int64_t *capacity,int32_t *head) {
 sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,array_pair,NULL,NULL,NULL,NULL,NULL,0),*ops[]={op};
 sgpl_exec_ctx *c=autograph_exec_ctx_create(g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,NULL,0,head,NULL,0,ops,1,-1);
 check(c&&!autograph_exec_ctx_bind_array_frontier(c,input,count,out,capacity,2)&&!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,1),"epoch context preparation");return c;
}
int main(void) {
 int64_t rp[]={0,2,4,5,5,6,7,7};int32_t ci[]={1,2,2,2,3,1,6};ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
 autograph_init(&g,7,7,NULL,NULL,pairs);check(autograph_build_array_cut(&g,3)==3,"array cut");
 int32_t full[]={4,1,3,0,1,6},empty[]={3},*out=NULL,head=0;int64_t capacity=0;
 sgpl_exec_ctx *a=make_epoch_context(&g,full,6,&out,&capacity,&head);skip_appends=0;
 check(autograph_frontier_execute(&g,a)==14&&autograph_frontier_execute(&g,a)==14,"two executions write distinct validity epochs");
 check(((uint32_t *)a->array_append_log)[0]==2,"second invocation carries epoch two");autograph_exec_ctx_destroy(a);
 for(int i=0;i<2;++i) {
  sgpl_exec_ctx *b=make_epoch_context(&g,empty,1,&out,&capacity,&head);
  check(autograph_frontier_execute(&g,b)==0,"empty live view retains larger backing storage");
#if SGPL_EXEC_APPEND_EPOCH_MAX == 3
  if(i==1)check(((uint32_t *)b->array_append_log)[0]==0,"epoch wrap clears full retained capacity, including inactive tail");
#endif
  autograph_exec_ctx_destroy(b);
 }
 sgpl_exec_ctx *d=make_epoch_context(&g,full,6,&out,&capacity,&head);skip_appends=1;
 check(autograph_frontier_execute(&g,d)==0,"old validity tags cannot revive after wrap and view growth");
 skip_appends=0;check(autograph_frontier_execute(&g,d)==14,"duplicates and multiple append sites survive tag reuse");
 check(!sgpl_gpu_prepare_spatial_records(d),"device conversion owns legacy boolean flags");
 skip_appends=1;check(autograph_frontier_execute(&g,d)==0,"CPU fallback clears stale stamped log after device transfer");
 autograph_exec_ctx_destroy(d);sgpl_frontier_release_buffers();printf("append epoch contract: %d failures\n",failures);return failures?1:0;
}
