#define SGPL_SPATIAL_FIXTURE_ONLY
#define SGPL_FIXTURE_EDGE_COUNT(g) (((ExecTestGraph *)(g))->m)
#include "spatial_exec_test.c"
#include "append_journal_plan.h"
#include "execution_code_layout.inc"
extern void sgpl_exec_bind_append_journal_v1(sgpl_exec_ctx *,const sgpl_append_journal_plan_v1 *);
extern int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *,int32_t);
#if __has_include("frontier_fusion_plan.h")
#include "frontier_fusion_plan.h"
extern void sgpl_exec_bind_frontier_fusion_v1(sgpl_exec_ctx *,const sgpl_frontier_fusion_plan_v1 *);
#endif
static atomic_int journal_calls;
static int no_output;
static void journal_test_body(void *unused,sgpl_exec_ctx *c,int32_t u,int32_t v,int sparse) {
  (void)unused;(void)u;atomic_fetch_add(&journal_calls,1);
  if(no_output||c->array_work_ordinal%3==0)return;
  int32_t first=(int32_t)c->array_work_ordinal;
  if(sparse){sgpl_frontier_journal_activate_v1(c,first);sgpl_frontier_journal_activate_v1(c,v);}
  else {autograph_frontier_activate(c,first);autograph_frontier_activate(c,v);}
}
static void journal_test_pair(void *s,sgpl_exec_ctx *c,int32_t u,int32_t v){journal_test_body(s,c,u,v,0);}
static void journal_test_sparse_pair(void *s,sgpl_exec_ctx *c,int32_t u,int32_t v){journal_test_body(s,c,u,v,1);}
#ifdef SGPL_FRONTIER_FUSION_PLAN_H
static void journal_test_owner(void *state,sgpl_exec_ctx *c,int32_t lo,int32_t hi,const int64_t *opaque,
    const int32_t *unused,const void *unused2,int journal) {
  const sgpl_frontier_owner_view_v1 *view=(const void *)opaque;(void)unused;(void)unused2;
  for(int32_t d=lo;d<hi;++d) {
    sgpl_frontier_descriptor_v1 x=view->descriptors[d];c->array_source_ordinal=x.occurrence;
    if(c->spatial_source_gate&&!c->spatial_source_gate[x.occurrence])continue;
    for(int64_t e=view->rows[x.row];e<view->rows[x.row+1];++e) {
      if(c->execution_error&&__atomic_load_n(c->execution_error,__ATOMIC_ACQUIRE)<0)return;
      int32_t v=view->columns[e];c->array_work_ordinal=(int64_t)x.work+view->ordinals[e];c->array_append_seq=0;
      if(c->spatial_groups_seen){c->spatial_groups_seen[v]=1;if(view->group_tags&&view->group_tags[v]!=view->group_epoch){view->group_tags[v]=view->group_epoch;++c->spatial_occupied_groups;}}
      journal_test_body(state,c,x.source,v,journal);
    }
  }
}
static void journal_test_dense_owner(void *s,sgpl_exec_ctx *c,int32_t l,int32_t h,const int64_t *v,const int32_t *u,const void *u2){journal_test_owner(s,c,l,h,v,u,u2,0);}
static void journal_test_sparse_owner(void *s,sgpl_exec_ctx *c,int32_t l,int32_t h,const int64_t *v,const int32_t *u,const void *u2){journal_test_owner(s,c,l,h,v,u,u2,1);}
#endif
static sgpl_exec_ctx *journal_test_context(ExecTestGraph *g,int32_t *input,int count,int32_t **out,int64_t *capacity,
    int32_t *head,int enabled,int direct,const sgpl_append_journal_plan_v1 *plan) {
  sgpl_runtime_op *pair=autograph_exec_op_create(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL,NULL,journal_test_pair,NULL,NULL,NULL,NULL,NULL,0);
  sgpl_runtime_op *ops[]={pair};sgpl_exec_ctx *c=autograph_exec_ctx_create(g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,
      NULL,NULL,*out,2,head,NULL,0,ops,1,-1);
  check(c!=NULL,"create bounded journal operation");autograph_exec_ctx_set_dag_axes(c,1,-1);
  check(!autograph_exec_ctx_bind_array_frontier(c,input,count,out,capacity,2),"bind bounded ordered append");
#ifdef SGPL_FRONTIER_FUSION_PLAN_H
  static const sgpl_frontier_fusion_plan_v1 frontier={SGPL_FRONTIER_FUSION_VERSION,sizeof(frontier),5,0,
      journal_test_pair,journal_test_dense_owner,journal_test_dense_owner,NULL};
  if(direct)sgpl_exec_bind_frontier_fusion_v1(c,&frontier);
#else
  (void)direct;
#endif
  if(enabled==2&&c->execution_scratch)atomic_store(&fail_calloc,1);
  if(enabled)sgpl_exec_bind_append_journal_v1(c,plan);
  check(!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),"reserve logical slots before user effects");
  return c;
}
int main(void) {
  setenv("SGPL_FUSION_B_FORCE_COLD","1",1);
  int64_t rows[]={0,16,32,48,48,52,56,56};int32_t columns[56],edge_pairs[112];
  for(int32_t u=0;u<7;++u)for(int64_t e=rows[u];e<rows[u+1];++e){columns[e]=(e*3+1)%7;edge_pairs[2*e]=u;edge_pairs[2*e+1]=columns[e];}
  native_pairs=edge_pairs;ExecTestGraph g={.n=7,.m=56,.row_ptr=rows,.col_idx=columns,.directed=1};
  autograph_init(&g,7,56,NULL,NULL,edge_pairs);autograph_ensure_layout(&g,7,56,rows,columns,&fixture_nodes,&fixture_edges,edge_pairs,LAYOUT_CSR);autograph_build_clean_cut(&g,3);
  int32_t input[]={4,1,3,0,1,6,4,1,3,0,1,6,4,1,3,0,1,6,4,1,3,0,1,6};
  sgpl_append_journal_plan_v1 plan={SGPL_APPEND_JOURNAL_VERSION,sizeof(plan),5,0,journal_test_pair,journal_test_sparse_pair,NULL,NULL,NULL};
#ifdef SGPL_FRONTIER_FUSION_PLAN_H
  plan.direct_dense=plan.direct_sparse=journal_test_sparse_owner;
#endif
  check(((uintptr_t)(void *)autograph_exec_ctx_set_source_gate%SGPL_SOURCE_GATE_ALIGNMENT)==0,"execution source-gate alignment contract");
  for(int proof=0;proof<2;++proof)for(int layout=0;layout<4;++layout)for(int direct=0;direct<2;++direct) {
    plan.obligations=5|(proof?SGPL_JOURNAL_PROVEN_APPEND:0);
    plan.append_sites=proof?2:0;
    autograph_ensure_layout(&g,7,56,rows,columns,&fixture_nodes,&fixture_edges,edge_pairs,layout);
    autograph_build_clean_cut(&g,3);
    int32_t a[600],b[600],*pa=a,*pb=b,ha=0,hb=0;int64_t ca=600,cb=600;
    for(int i=0;i<600;++i)a[i]=b[i]=700+i;
    sgpl_exec_ctx *x=journal_test_context(&g,input,24,&pa,&ca,&ha,0,direct,&plan);
    sgpl_exec_ctx *y=journal_test_context(&g,input,24,&pb,&cb,&hb,1,direct,&plan);
#ifdef SGPL_FRONTIER_FUSION_PLAN_H
    if(direct&&layout==LAYOUT_CSR&&y->execution_scratch)
      check(y->array_compact_records==6,"journal contract also exercises the native descriptor executor");
#endif
    for(int round=0;round<10;++round) {
      if(round%3==1)setenv("SGPL_NO_FUSION_C","1",1);else unsetenv("SGPL_NO_FUSION_C");
      no_output=round%4==3;
      int32_t na=autograph_frontier_execute(&g,x);
      if(round==5&&y->execution_scratch)atomic_store(&fail_malloc,1);
#ifdef SGPL_JOURNAL_BITMAP_PROTOCOL
      if(round==6&&y->execution_scratch)atomic_store(&fail_malloc,2);
#endif
      atomic_store(&journal_calls,0);int32_t nb=autograph_frontier_execute(&g,y);
      check(na==nb&&!memcmp(pa,pb,600*4),"stable sparse merge preserves prefix, duplicates, work/append order and entire unused tail");
      check(atomic_load(&journal_calls)==208,"every original pair executes once; no callback replay on fallback");
      check(x->spatial_occupied_groups==y->spatial_occupied_groups,"journal mode retains group accounting");
#ifdef SGPL_JOURNAL_BITMAP_PROTOCOL
      if((round==5||round==6)&&y->execution_scratch)
        check((void *)y->ops[0].pair==plan.pair,"owner heap and ordinal bitmap failures both select dense execution before effects");
#endif
      if(y->execution_scratch&&round==0)check((void *)y->ops[0].pair==plan.journal_pair,"bounded journal activates before effects");
    }
    autograph_exec_ctx_destroy(y);autograph_exec_ctx_destroy(x);
    /* A different context reusing this thread's lease has no journal plan.
     * Old ordinals must not become dense-log validity tags. */
    y=journal_test_context(&g,input,24,&pb,&cb,&hb,0,direct,&plan);no_output=1;
    check(autograph_frontier_execute(&g,y)==2,"journal-to-generic lease transition clears validity without phantom appends");
    autograph_exec_ctx_destroy(y);
  }
  unsetenv("SGPL_NO_FUSION_C");
  /* Descriptor rejection and insufficient pre-effect reservation must leave
   * the original callback installed. No-output work makes the conservative
   * capacity check observable without asking the old path to grow stack data. */
  for(int mode=0;mode<6;++mode) {
    sgpl_append_journal_plan_v1 bad=plan;
    if(mode==0)++bad.version;
    if(mode==1)--bad.bytes;
    if(mode==2)bad.append_sites=UINT32_MAX;
    if(mode==5)bad.version=2;
    int32_t a[600],b[600],*pa=a,*pb=b,ha=0,hb=0;int64_t ca=600,cb=600;
    for(int i=0;i<600;++i)a[i]=b[i]=800+i;
    sgpl_exec_ctx *x=journal_test_context(&g,input,24,&pa,&ca,&ha,0,0,&plan);
    sgpl_exec_ctx *y=journal_test_context(&g,input,24,&pb,&cb,&hb,1,0,&bad);
    if(mode==3)ca=cb=2;
    if(mode==4)y->array_append_sites=1; /* Recheck the bound at execution, not only binding. */
    no_output=1;int32_t na=autograph_frontier_execute(&g,x);atomic_store(&journal_calls,0);
    int32_t nb=autograph_frontier_execute(&g,y);
    check(na==2&&nb==na&&!memcmp(pa,pb,600*4)&&(void *)y->ops[0].pair==plan.pair,
      "unsupported descriptor and insufficient capacity refuse journals before effects");
    check(atomic_load(&journal_calls)==208,"conservative refusal never skips or replays user effects");
    autograph_exec_ctx_destroy(y);autograph_exec_ctx_destroy(x);
  }
  {
    int32_t a[600],b[600],*pa=a,*pb=b,ha=0,hb=0;int64_t ca=600,cb=600;
    for(int i=0;i<600;++i)a[i]=b[i]=700+i;
    sgpl_exec_ctx *x=journal_test_context(&g,input,24,&pa,&ca,&ha,0,0,&plan);
    sgpl_exec_ctx *y=journal_test_context(&g,input,24,&pb,&cb,&hb,2,0,&plan);no_output=0;
    int32_t na=autograph_frontier_execute(&g,x);atomic_store(&journal_calls,0);
    int32_t nb=autograph_frontier_execute(&g,y);
    check(na==nb&&!memcmp(pa,pb,600*4)&&(void *)y->ops[0].pair==plan.pair,
      "optional execution-lease allocation failure falls back before effects");
    check(atomic_load(&journal_calls)==208,"optional lease failure never replays callbacks");
    autograph_exec_ctx_destroy(y);autograph_exec_ctx_destroy(x);
  }
  /* Different logical domains reuse the lease. Exercise bitmap word tails,
   * a zero-work invocation, and the changing boundary between values/keys. */
  const int counts[]={1,6,24,0,24,1};
  for(unsigned k=0;k<sizeof(counts)/sizeof(counts[0]);++k) {
    int32_t a[600],b[600],*pa=a,*pb=b,ha=0,hb=0;int64_t ca=600,cb=600;
    for(int i=0;i<600;++i)a[i]=b[i]=900+i;
    sgpl_exec_ctx *x=journal_test_context(&g,input,counts[k],&pa,&ca,&ha,0,0,&plan);
    sgpl_exec_ctx *y=journal_test_context(&g,input,counts[k],&pb,&cb,&hb,1,0,&plan);no_output=0;
    int expected=0;for(int i=0;i<counts[k];++i)expected+=(int)(rows[input[i]+1]-rows[input[i]]);
    int32_t na=autograph_frontier_execute(&g,x);atomic_store(&journal_calls,0);
    int32_t nb=autograph_frontier_execute(&g,y);
    check(na==nb&&!memcmp(pa,pb,sizeof a),"changing domains, empty work and partial bitmap words preserve complete output");
    check(atomic_load(&journal_calls)==expected,"domain transitions execute each occurrence exactly once");
    autograph_exec_ctx_destroy(y);autograph_exec_ctx_destroy(x);
  }
  printf("append journal contracts: %d failures\n",failures);return failures?1:0;
}
