#define SGPL_FULLGRAPH_TEST_MAIN baseline_fullgraph_contracts
#include "fullgraph_test.c"
#include "execution_simplify_plan.h"
extern void autograph_exec_ctx_bind_simplified_fullgraph_v1(sgpl_exec_ctx *,const sgpl_execution_simplify_plan_v1 *);
static _Atomic unsigned optimized_calls,optimized_maps;
static void optimized_kernel(void *state,sgpl_exec_ctx *ctx,int32_t lo,int32_t hi,const int64_t *rp,const int32_t *sources,const void *map) {
  atomic_fetch_add(&optimized_calls,1);full_test_kernel(state,ctx,lo,hi,rp,sources,map);
}
static void optimized_map(void *state,sgpl_exec_ctx *ctx,int64_t n,const int64_t *rp,const uint8_t *gate,void *map) {
  atomic_fetch_add(&optimized_maps,1);full_test_map(state,ctx,n,rp,gate,map);
}
int main(void) {
  if(baseline_fullgraph_contracts())return 1;
  int64_t rows[]={0,2,4,5,6,7,8,8};int32_t columns[]={2,1,1,1,1,3,2,2};
  int32_t pairs[]={0,2,0,1,1,1,1,1,2,1,3,3,4,2,5,2};
  ExecTestGraph g={.n=7,.m=8,.row_ptr=rows,.col_idx=columns,.directed=1};native_pairs=pairs;
  autograph_init(&g,7,8,NULL,NULL,pairs);autograph_ensure_layout(&g,7,8,rows,columns,&fixture_nodes,&fixture_edges,pairs,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
  for(int mode=0;mode<10;++mode) {
    FullTest a,b;sgpl_exec_ctx *x=full_test_context(&g,&a,1,0),*y=full_test_context(&g,&b,1,0);
    sgpl_execution_simplify_plan_v1 plan={1,sizeof(plan),7,0,(void *)full_test_kernel,(void *)full_test_map,(void *)optimized_kernel,(void *)optimized_map};
    if(mode==1)plan.version=2;if(mode==2)--plan.bytes;if(mode==3)plan.rules=8;if(mode==4)plan.baseline_kernel=NULL;
    if(mode==5)plan.baseline_map=NULL;if(mode==6)plan.kernel=NULL;if(mode==7)plan.map=NULL;if(mode==8)plan.reserved=1;
    atomic_store(&optimized_calls,0);atomic_store(&optimized_maps,0);
    autograph_exec_ctx_bind_simplified_fullgraph_v1(y,&plan);
    if(mode==9)setenv("SGPL_NO_EXEC_SIMPLIFY","1",1);
    check(!autograph_frontier_execute(&g,x)&&!autograph_frontier_execute(&g,y),"extension baseline/candidate execution");full_test_equal(&a,&b);
    check((atomic_load(&optimized_calls)>0)==(mode==0),"descriptor activation/refusal/runtime disable");
    check(atomic_load(&optimized_maps)==(mode==0),"map selection before effects");unsetenv("SGPL_NO_EXEC_SIMPLIFY");
    autograph_update_csr_pointers(&g,rows,columns);atomic_store(&optimized_calls,0);atomic_store(&optimized_maps,0);
    check(!autograph_frontier_execute(&g,y),"invalidation keeps generic execution");check(!atomic_load(&optimized_calls)&&!atomic_load(&optimized_maps),"invalidation discards extension");
    autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
  }
  printf("execution simplification runtime: %d failures\n",failures);return failures?1:0;
}
