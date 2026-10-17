#define SGPL_ARRAY_FRONTIER_TEST_MAIN unused_array_contract_main
#include "array_frontier_exec_test.c"
static int mode;static atomic_int budget_errors;
static void compact_pair(void *state,sgpl_exec_ctx *c,int32_t u,int32_t v) {
 (void)state;(void)u;if(mode>=3&&sgpl_current_thread_budget()>2)atomic_fetch_add(&budget_errors,1);
 if((c->array_work_ordinal+mode)%3){autograph_frontier_activate(c,v);autograph_frontier_activate(c,v);}
}
int main(void) {
 const int64_t edges=300007;int64_t rp[8]={0};int32_t *ci=malloc(edges*4);
 for(int i=1;i<8;i++)rp[i]=edges;for(int64_t i=0;i<edges;i++)ci[i]=(int32_t)((i*37)%7);
 ExecTestGraph g={.n=7,.m=edges,.row_ptr=rp,.col_idx=ci,.directed=1};autograph_init(&g,7,edges,NULL,NULL,pairs);check(autograph_build_array_cut(&g,3)==3,"cut");
 int32_t source=0,head=0;int64_t capacity=2*edges+32;int32_t *out=malloc(capacity*4);
 sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR,NULL,compact_pair,NULL,NULL,NULL,NULL,NULL,0),*ops[]={op};
 sgpl_exec_ctx *c=autograph_exec_ctx_create(&g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,NULL,8,&head,NULL,0,ops,1,-1);
 check(!autograph_exec_ctx_bind_array_frontier(c,&source,1,&out,&capacity,2),"bind");check(!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,1),"prepare");
 for(mode=0;mode<5;mode++) {
  for(int64_t i=0;i<capacity;i++)out[i]=-991;for(int i=0;i<8;i++)out[i]=100+i;
  if(mode>=3)sgpl_push_thread_budget(2);int32_t size=autograph_frontier_execute(&g,c);if(mode>=3)sgpl_pop_thread_budget();check(!atomic_load(&budget_errors),"nested callback budget");check(sgpl_debug_reserved_threads()==0,"balanced budget ledger");int64_t at=8;
  for(int64_t i=0;i<edges;i++)if((i+mode)%3){if(out[at++]!=ci[i]||out[at++]!=ci[i]){check(0,"stable occurrence order");break;}}
  check(size==at,"selected size");check(c->spatial_occupied_groups==7,"exact groups after epoch wrap");for(int i=0;i<8;i++)check(out[i]==100+i,"prefix");
  for(int64_t i=at;i<capacity;i++)if(out[i]!=-991){check(0,"inactive tail");break;}
 }
 autograph_exec_ctx_destroy(c);sgpl_frontier_release_buffers();free(out);free(ci);printf("large stable compaction: %d failures\n",failures);return failures?1:0;
}
