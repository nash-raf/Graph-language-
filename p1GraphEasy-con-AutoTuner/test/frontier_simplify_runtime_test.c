/* Exercise additive binding, identity checks and dispatch using the existing
 * ordered append fixture. Actual generated loops are tested by the LLVM JIT. */
#define SGPL_SPATIAL_FIXTURE_ONLY
#define SGPL_FIXTURE_EDGE_COUNT(g) (((ExecTestGraph *)(g))->m)
#include "spatial_exec_test.c"
#include "append_journal_plan.h"
#include "frontier_simplify_plan.h"
extern void sgpl_exec_bind_append_journal_v1(sgpl_exec_ctx *,const sgpl_append_journal_plan_v1 *);
extern int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *,int32_t);
extern void sgpl_exec_bind_simplified_frontier_v1(sgpl_exec_ctx *,const sgpl_frontier_simplify_plan_v1 *);
static _Atomic unsigned kernel_calls,pair_calls;
static int fail_binding;
static void reference_pair(void *state,sgpl_exec_ctx *c,int32_t u,int32_t v) {
  (void)state;atomic_fetch_add(&pair_calls,1);
  if(c->array_work_ordinal%3){autograph_frontier_activate(c,v);autograph_frontier_activate(c,u);}
}
static void journal_pair(void *state,sgpl_exec_ctx *c,int32_t u,int32_t v) {
  (void)state;atomic_fetch_add(&pair_calls,1);
  if(c->array_work_ordinal%3){sgpl_frontier_journal_activate_v1(c,v);sgpl_frontier_journal_activate_v1(c,u);}
}
static int64_t protocol_kernel(const sgpl_frontier_simplify_args_v1 *a,int stride,int journal) {
  atomic_fetch_add(&kernel_calls,1);sgpl_exec_ctx *c=a->context;int64_t groups=0;
  for(int64_t i=a->first;i<a->last;++i){
    if(a->error&&__atomic_load_n(a->error,__ATOMIC_ACQUIRE)<0)return -1;
    int32_t u=a->records[stride*i],v=a->records[stride*i+1];
    c->array_work_ordinal=a->records[stride*i+2];c->array_append_seq=0;
    c->array_edge_index=stride==4?a->records[stride*i+3]:(a->edges?a->edges[i]:-1);
    if(a->seen){if(a->tags){if(a->tags[v]!=a->epoch){a->tags[v]=a->epoch;a->seen[v]=1;++groups;}}
      else if(a->sparse){if(!a->seen[v]){a->seen[v]=1;++groups;}}else a->seen[v]=1;}
    (journal?journal_pair:reference_pair)(a->state,c,u,v);
  }
  return groups;
}
static int64_t plain(const sgpl_frontier_simplify_args_v1 *a){return protocol_kernel(a,3,0);}
static int64_t weighted(const sgpl_frontier_simplify_args_v1 *a){return protocol_kernel(a,4,0);}
static int64_t journal_plain(const sgpl_frontier_simplify_args_v1 *a){return protocol_kernel(a,3,1);}
static int64_t journal_weighted(const sgpl_frontier_simplify_args_v1 *a){return protocol_kernel(a,4,1);}
static sgpl_exec_ctx *simplify_context(ExecTestGraph *g,int32_t *in,int count,int32_t **out,int64_t *capacity,int32_t *head,
    const sgpl_frontier_simplify_plan_v1 *p,int journal) {
  sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL,NULL,reference_pair,NULL,NULL,NULL,NULL,NULL,0);
  sgpl_runtime_op *ops[]={op};sgpl_exec_ctx *c=autograph_exec_ctx_create(g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,*out,2,head,NULL,0,ops,1,-1);
  check(c!=NULL,"create frontier simplification context");autograph_exec_ctx_set_dag_axes(c,1,-1);
  check(!autograph_exec_ctx_bind_array_frontier(c,in,count,out,capacity,2),"bind ordered array");
  if(p){if(fail_binding&&c->execution_scratch)atomic_store(&fail_calloc,1);sgpl_exec_bind_simplified_frontier_v1(c,p);}
  static const sgpl_append_journal_plan_v1 jp={SGPL_APPEND_JOURNAL_VERSION,sizeof(jp),5|SGPL_JOURNAL_PROVEN_APPEND,2,reference_pair,journal_pair,NULL,NULL,NULL};
  if(journal)sgpl_exec_bind_append_journal_v1(c,&jp);
  check(!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),"prepare existing records");
  return c;
}
int main(void) {
  int64_t rows[]={0,4,8,10,10,12,13,13};int32_t cols[]={1,1,0,6,2,2,0,1,2,3,0,4,6},ep[26];
  for(int u=0;u<7;++u)for(int64_t e=rows[u];e<rows[u+1];++e){ep[2*e]=u;ep[2*e+1]=cols[e];}
  native_pairs=ep;ExecTestGraph g={.n=7,.m=13,.row_ptr=rows,.col_idx=cols,.directed=1};
  autograph_init(&g,7,13,NULL,NULL,ep);autograph_ensure_layout(&g,7,13,rows,cols,&fixture_nodes,&fixture_edges,ep,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
  int32_t input[]={0,0,3,1,4,6,2,0};
  sgpl_frontier_simplify_plan_v1 good={1,sizeof(good),8|16,0,sizeof(sgpl_exec_ctx),sizeof(sgpl_runtime_op),sizeof(sgpl_frontier_simplify_args_v1),reference_pair,journal_pair,plain,weighted,journal_plain,journal_weighted};
  unsigned activations=0;
  for(int journal=0;journal<2;++journal)for(int mode=0;mode<15;++mode) {
    sgpl_frontier_simplify_plan_v1 p=good;
    if(mode==1)++p.version;if(mode==2)--p.bytes;if(mode==3)p.rules=32;if(mode==4)p.reserved=1;
    if(mode==5)--p.context_bytes;if(mode==6)--p.operation_bytes;if(mode==7)--p.argument_bytes;
    if(mode==8)p.pair=journal_pair;if(mode==9)p.plain=NULL;if(mode==10)p.journal_plain=NULL;
    int32_t a[256],b[256],*pa=a,*pb=b,ha=0,hb=0;int64_t ca=256,cb=256;
    for(int i=0;i<256;++i)a[i]=b[i]=1000+i;
    fail_binding=mode==14;
    sgpl_exec_ctx *x=simplify_context(&g,input,8,&pa,&ca,&ha,NULL,journal),*y=simplify_context(&g,input,8,&pb,&cb,&hb,&p,journal);
    fail_binding=0;
    for(int round=0;round<4;++round) {
      if(mode==11)setenv("SGPL_NO_EXEC_SIMPLIFY","1",1);else unsetenv("SGPL_NO_EXEC_SIMPLIFY");
      if(mode==12)setenv("SGPL_NO_FUSION_C","1",1);else unsetenv("SGPL_NO_FUSION_C");
      if(mode==13&&round==1)autograph_update_csr_pointers(&g,rows,cols);
      int ra=autograph_frontier_execute(&g,x);atomic_store(&kernel_calls,0);atomic_store(&pair_calls,0);
      int rb=autograph_frontier_execute(&g,y);unsigned called=atomic_load(&kernel_calls);activations+=!!called;
      check(ra==rb&&!memcmp(pa,pb,sizeof a),"original/journal/simplified stable duplicate append output including unused tail");
      check(x->spatial_occupied_groups==y->spatial_occupied_groups,"occupied group accounting");
      check(atomic_load(&pair_calls)==20,"no skipped or replayed pair callbacks");
      int supported=y->execution_scratch&&(y->array_compact_records==3||y->array_compact_records==4);
      check(!!called==(supported&&(mode==0||mode==12||mode==13)),"descriptor refusal and pre-effect runtime selection");
    }
    autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
    /* An unrelated context may borrow the lease but must never inherit a plan. */
    y=simplify_context(&g,input,8,&pb,&cb,&hb,NULL,0);atomic_store(&kernel_calls,0);
    check(autograph_frontier_execute(&g,y)>=0&&!atomic_load(&kernel_calls),"lease reuse discards prior simplification binding");autograph_exec_ctx_destroy(y);
  }
  unsetenv("SGPL_NO_EXEC_SIMPLIFY");unsetenv("SGPL_NO_FUSION_C");
  if(getenv("SGPL_TEST_EXPECT_SIMPLIFY")&&*getenv("SGPL_TEST_EXPECT_SIMPLIFY"))check(activations>0,"representative runtime activation");
  printf("frontier simplification runtime: %u activations, %d failures\n",activations,failures);return failures?1:0;
}
