#define SGPL_FULLGRAPH_TEST_MAIN baseline_fullgraph_contracts
#include "fullgraph_test.c"
#include "execution_simplify_plan.h"
#include "iteration_memory_plan.h"
#include <math.h>
static void *iteration_source,*iteration_destination;
static void iteration_gate(void *state,sgpl_exec_ctx *c,int32_t u) {
  (void)state;autograph_exec_ctx_set_source_gate(c,u,u%3!=1);
}
static void iteration_map(void *state,sgpl_exec_ctx *c,int64_t hi,const int64_t *rows,const uint8_t *gate,void *storage,int64_t lo) {
  (void)state;(void)c;double *out=storage,*source=iteration_source;
  for(int64_t u=lo;u<hi;++u)if(rows[u+1]>rows[u]&&gate[u])out[u]=source[u]/(double)(rows[u+1]-rows[u]);
}
static void iteration_kernel(void *state,sgpl_exec_ctx *c,int32_t lo,int32_t hi,const int64_t *rows,const int32_t *sources,const void *storage,double seed) {
  (void)state;const double *map=storage;double *destination=iteration_destination;
  for(int32_t v=lo;v<hi;++v){double acc=seed;for(int64_t e=rows[v];e<rows[v+1];++e)if(c->spatial_source_gate[sources[e]])acc=map[sources[e]]+acc;destination[v]=acc;}
}
static void iteration_cases(ExecTestGraph *g) {
  double initial[]={1e16,-1e16,1.0,-0.0,0.0,INFINITY,NAN};
  for(int data=0;data<4;++data)for(int rounds=1;rounds<=20;rounds+=rounds<3?1:17)for(int failure=0;failure<9;++failure) {
    double a[7],b[7],want[7],next[7];memcpy(a,initial,sizeof a);if(data==1)a[5]=0.0;if(data==2)a[0]=NAN;
    memcpy(want,a,sizeof a);for(int i=0;i<7;++i)b[i]=next[i]=123.0;
    double seed=data==3?-0.0:0.15;
    for(int r=0;r<rounds;++r){for(int v=0;v<7;++v)next[v]=seed;for(int u=0;u<7;++u)if(u%3!=1&&g->row_ptr[u+1]>g->row_ptr[u]){double contribution=want[u]/(double)(g->row_ptr[u+1]-g->row_ptr[u]);for(int64_t e=g->row_ptr[u];e<g->row_ptr[u+1];++e)next[g->col_idx[e]]=contribution+next[g->col_idx[e]];}memcpy(want,next,sizeof want);}
    iteration_source=a;iteration_destination=b;
    sgpl_iteration_memory_plan_v1 p={1,sizeof(p),(void *)iteration_kernel,iteration_gate,&iteration_source,&iteration_destination,iteration_kernel,iteration_map,8};
    if(failure==1)++p.version;if(failure==2)--p.bytes;if(failure==3)p.kernel=NULL;
    if(failure>=4&&failure<8)atomic_store(&fail_malloc,failure-3);
    if(failure==8)--g->n;
    int result=sgpl_try_iteration_memory_v1(g,&p,seed,rounds);atomic_store(&fail_malloc,0);
    if(failure==8){++g->n;check(!result,"graph header and certified native extent mismatch refuses before effects");}
    check(iteration_source==a&&iteration_destination==b,"iteration buffer identities are restored");
    if(result)check(!memcmp(a,want,sizeof a)&&!memcmp(b,want,sizeof b),"seed/map/rotation preserve exact bits, empty rows and both final arrays");
    else {double original[7];memcpy(original,initial,sizeof original);if(data==1)original[5]=0.0;if(data==2)original[0]=NAN;check(!memcmp(a,original,sizeof a)&&b[0]==123.0,"iteration descriptor/allocation refusal precedes user effects");}
  }
}
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
  iteration_cases(&g);
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
