#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
#include "semantic_stage_plan.h"
#include <math.h>
static int operation;
static void *active;
static atomic_int weight_fallbacks;
static atomic_uint worker_mask;
static atomic_int minimum_calls;
typedef struct {
  ExecTestGraph *graph;sgpl_semantic_stage_plan_v1 plan;
  const int32_t *input;int count;int32_t **output;int64_t *capacity;int32_t *values;int result;
} SetFailureTask;
static void *set_failure_thread(void *opaque) {
  SetFailureTask *t=opaque;t->result=sgpl_try_semantic_stage_v1(t->graph,&t->plan,t->input,t->count,t->output,t->capacity,t->values,5);return NULL;
}
static void minimum_pair(void *state,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)state;atomic_fetch_add(&minimum_calls,1);
  int32_t *a=active,*s=ctx->spatial_snapshots[0];
  int32_t candidate=(int32_t)((uint32_t)s[u]+(uint32_t)autograph_exec_ctx_edge_weight(ctx,ctx->graph,u,v));
  if(candidate<a[v]){a[v]=candidate;sgpl_semantic_activate_v1(ctx,v);}
}
static void minimum_row(sgpl_exec_ctx *ctx,int32_t v,const int32_t *sources,const int64_t *edges,int64_t first,int64_t last) {
  for(int64_t e=first;e<last;++e){ctx->array_work_ordinal=edges[e];ctx->array_edge_index=ctx->array_record_edges[edges[e]];minimum_pair(NULL,ctx,sources[e],v);}
}
int32_t graph_get_edge_weight(void *opaque,int32_t u,int32_t v) {
  ExecTestGraph *g=opaque;atomic_fetch_add(&weight_fallbacks,1);
  for(int64_t e=g->row_ptr[u];e<g->row_ptr[u+1];++e)if(g->col_idx[e]==v)return g->weights?g->weights[e]:1;
  return INT32_MAX;
}
static void weighted_pair(void *state,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)state;int32_t *a=active,*s=ctx->spatial_snapshots[0];
  atomic_fetch_or(&worker_mask,1u<<sgpl_current_worker_index());
  int32_t w=autograph_exec_ctx_edge_weight(ctx,ctx->graph,u,v);
  a[v]=(int32_t)((uint32_t)a[v]+(uint32_t)s[u]+(uint32_t)w);
  sgpl_semantic_activate_v1(ctx,v);
}
static void weighted_row(sgpl_exec_ctx *ctx,int32_t v,const int32_t *sources,const int64_t *edges,int64_t first,int64_t last) {
  for(int64_t e=first;e<last;++e) {
    ctx->array_work_ordinal=edges[e];ctx->array_edge_index=ctx->array_record_edges[edges[e]];
    weighted_pair(NULL,ctx,sources[e],v);
  }
}
static int update(void *array,const void *snapshot,int u,int v) {
  if(operation<6) {
    int32_t *a=array;const int32_t *s=snapshot;int32_t old=a[v],next=old;
    if(operation==0)next=old<s[u]?old:s[u];
    if(operation==1)next=old>s[u]?old:s[u];
    if(operation==2)next=(int32_t)((uint32_t)old|(uint32_t)s[u]);
    if(operation==3)next=(int32_t)((uint32_t)old&(uint32_t)s[u]);
    if(operation==4)next=(int32_t)((uint32_t)old-(uint32_t)s[u]);
    if(operation==5)next=(int32_t)((uint32_t)old+(uint32_t)s[u]);
    if(operation>=4||next!=old){a[v]=next;return 1;}return 0;
  }
  double *a=array;const double *s=snapshot;
  if(operation==6)a[v]=a[v]+s[u];else a[v]=a[v]-s[u];return 1;
}
static void pair(void *state,sgpl_exec_ctx *ctx,int32_t u,int32_t v) {
  (void)state;
  if(update(active,ctx->spatial_snapshots[0],u,v))sgpl_semantic_activate_v1(ctx,v);
}
static void row(sgpl_exec_ctx *ctx,int32_t v,const int32_t *sources,const int64_t *edges,int64_t first,int64_t last) {
  for(int64_t e=first;e<last;++e){ctx->array_work_ordinal=edges[e];pair(NULL,ctx,sources[e],v);}
}
static int reference(ExecTestGraph *g,void *array,const int32_t *input,int count,int32_t *out) {
  unsigned char snapshot[64];memcpy(snapshot,array,5*(operation<6?4:8));int head=0;
  for(int i=0;i<count;++i)for(int64_t e=g->row_ptr[input[i]];e<g->row_ptr[input[i]+1];++e)
    if(update(array,snapshot,input[i],g->col_idx[e]))out[head++]=g->col_idx[e];
  return head;
}
static void frontier_removal_cases(void) {
  static ExecTestGraph graph;
  int64_t rows[]={0,3,5,7,8,8};int32_t cols[]={0,1,2,2,3,3,4,4};
  int32_t weights[]={INT32_MAX,4,7,-3,5,2,9,0},ep[16];
  for(int u=0;u<5;++u)for(int64_t e=rows[u];e<rows[u+1];++e){ep[2*e]=u;ep[2*e+1]=cols[e];}
  graph=(ExecTestGraph){.n=5,.m=8,.row_ptr=rows,.col_idx=cols,.weights=weights,.directed=1};
  native_pairs=ep;autograph_init(&graph,5,8,NULL,NULL,ep);autograph_ensure_layout(&graph,5,8,rows,cols,&fixture_nodes,&fixture_edges,ep,LAYOUT_CSR);
  {
    int32_t values[]={1,2,3,4,5},isolated=4,*out=NULL;int64_t cap=0;
    sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),512|128|SGPL_EXEC_R14,0,sizeof(sgpl_exec_ctx),minimum_pair,NULL,4,0,0,0};
    check(sgpl_try_semantic_stage_v1(&graph,&p,&isolated,1,&out,&cap,values,5)==0&&!out&&!cap&&values[4]==5,
        "R14 intersects exact bounds: no contribution needs no whole-graph frontier reservation");
    free(out);
  }
  for(int duplicate=0;duplicate<2;++duplicate)for(int subset=0;subset<4;++subset)for(int disabled=0;disabled<2;++disabled) {
    cols[1]=duplicate?0:1;autograph_record_adjacency_state(&graph,5,8,rows,cols);
    sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),512|128|(subset&1?SGPL_EXEC_R14:0)|(subset&2?SGPL_EXEC_R15:0),0,sizeof(sgpl_exec_ctx),minimum_pair,NULL,4,0,1,0};
    int32_t a[]={0,INT32_MAX,INT32_MAX,INT32_MAX,INT32_MAX},b[5],inputbuf[]={0,0,1,2,3,4};memcpy(b,a,sizeof a);
    int32_t *input=inputbuf,*out=NULL;int64_t cap=0;int count=6;char proof_key;
    sgpl_semantic_frontier_reset_v1(&proof_key);
    if(disabled)setenv("SGPL_NO_PROOF_REMOVALS","1",1);
    for(int round=0;round<4;++round) {
      int32_t snapshot[5];memcpy(snapshot,a,sizeof a);unsigned expected=0;
      for(int i=0;i<count;++i)for(int64_t e=rows[input[i]];e<rows[input[i]+1];++e) {
        int32_t v=cols[e],candidate=(int32_t)((uint32_t)snapshot[input[i]]+(uint32_t)graph_get_edge_weight(&graph,input[i],v));
        if(candidate<a[v]){a[v]=candidate;expected|=1u<<v;}
      }
      active=b;int got=sgpl_try_semantic_stage_facts_v1(&graph,&p,input,count,&out,&cap,b,5,&proof_key);
      unsigned actual=0;if(got>=0)for(int i=0;i<got;++i){check(out[i]>=0&&out[i]<5,"set output IDs valid");actual|=1u<<out[i];}
      check(got==__builtin_popcount(expected)&&expected==actual&&!memcmp(a,b,sizeof a),"R14/R15 independent/composed exact modular updates and unique set");
      if(round>0)free(input);input=out;out=NULL;cap=0;count=got;
      if(round==1){weights[2]=-11;autograph_update_csr_pointers(&graph,rows,cols);} /* live weight and topology invalidation */
    }
    free(input);unsetenv("SGPL_NO_PROOF_REMOVALS");
    int32_t bad[]={5},before[5];memcpy(before,b,sizeof b);out=NULL;cap=0;
    check(sgpl_try_semantic_stage_v1(&graph,&p,bad,1,&out,&cap,b,5)<0&&!memcmp(before,b,sizeof b),"R14 descriptor alone never licenses an invalid initial input");
    sgpl_semantic_frontier_reset_v1(&proof_key);
    check(sgpl_try_semantic_stage_facts_v1(&graph,&p,bad,1,&out,&cap,b,5,&proof_key)<0&&!memcmp(before,b,sizeof b),"fresh/reset induction never trusts arbitrary input");
    free(out);
  }
}
int main(void) {
  setenv("SGPL_GPU_BACKEND","0",1);
  int64_t rows[]={0,4,6,9,10,10};int32_t columns[]={1,1,0,2,2,3,3,0,2,3},ep[20];
  for(int u=0;u<5;++u)for(int64_t e=rows[u];e<rows[u+1];++e){ep[2*e]=u;ep[2*e+1]=columns[e];}
  native_pairs=ep;static ExecTestGraph graph;graph=(ExecTestGraph){.n=5,.m=10,.row_ptr=rows,.col_idx=columns,.directed=1};
  autograph_init(&graph,5,10,NULL,NULL,ep);autograph_ensure_layout(&graph,5,10,rows,columns,&fixture_nodes,&fixture_edges,ep,LAYOUT_CSR);
  int32_t sources[3][8]={{0,1,2,3,4},{2,0,0,4,1,3},{4}},counts[]={5,6,1};
  unsigned checks=0;
  for(operation=0;operation<8;++operation)for(unsigned rules=512;rules<2048;rules+=512)
  for(int order=0;order<3;++order)for(int data=0;data<5;++data) {
    union {double d[8];int32_t i[16];} a,b;
    double fd[5][5]={{1e16,-1e16,1,0,-0.0},{INFINITY,-INFINITY,0,1,NAN},{NAN,1,2,3,4},{0,-0.0,-0.0,0,0},{1,2,3,4,5}};
    int32_t id[5][5]={{0,1,2,3,4},{INT32_MIN,INT32_MAX,-1,0,7},{7,6,5,4,3},{0,0,0,0,0},{1,2,4,8,16}};
    size_t bytes=operation<6?20:40;memcpy(&a,operation<6?(void *)id[data]:(void *)fd[data],bytes);memcpy(&b,&a,bytes);
    int32_t expected[128],*out=NULL;int64_t cap=0;
    int wanted=reference(&graph,&a,sources[order],counts[order],expected);active=&b;
    sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),rules,0,sizeof(sgpl_exec_ctx),pair,row,operation<6?4:8,0,0,0};
    int got=sgpl_try_semantic_stage_v1(&graph,&p,sources[order],counts[order],&out,&cap,&b,5);
    if(rules==1024&&order){check(got==-1&&!memcmp(&b,operation<6?(void *)id[data]:(void *)fd[data],bytes),"ordered CSR refuses non-full or permuted occurrence stream");}
    else check(got==wanted&&!memcmp(&a,&b,bytes)&&!memcmp(out,expected,(size_t)wanted*4),"generic/nonfoldable and exact floating bits/duplicate notification order");
    free(out);++checks;
  }
  for(operation=0;operation<4;++operation)for(int order=0;order<3;++order)for(int data=0;data<20;++data) {
    int32_t a[5],b[5],in[128],outref[128];for(int i=0;i<5;++i)a[i]=b[i]=(int32_t)((unsigned)data*2654435761u+(unsigned)i*2246822519u);
    int count=counts[order];memcpy(in,sources[order],count*4);int rounds=0;
    while(count&&rounds++<1000){count=reference(&graph,a,in,count,outref);check(count<128,"reference bounded frontier");memcpy(in,outref,count*4);}
    check(rounds<1000,"finite lattice converges");
    int32_t *out=NULL;int64_t cap=0;active=b;
    sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),2048,(unsigned)operation+1,sizeof(sgpl_exec_ctx),pair,NULL,4,0,0,0};
    int got=sgpl_try_semantic_stage_v1(&graph,&p,sources[order],counts[order],&out,&cap,b,5);
    check(got==0&&!memcmp(a,b,sizeof a),"whole fixed point matches round-separated extrema/bitwise joins");free(out);++checks;
  }
  operation=4;int32_t values[5]={5,4,3,2,1},copy[5],*out=NULL;int64_t cap=0;
  sgpl_semantic_stage_plan_v1 plan={SGPL_SEMANTIC_STAGE_VERSION,sizeof(plan),512,0,sizeof(sgpl_exec_ctx),pair,row,4,0,0,0};active=values;
  for(int fault=0;fault<16;++fault) {
    memcpy(copy,values,sizeof copy);sgpl_semantic_stage_plan_v1 p=plan;
    if(fault==0)++p.version;if(fault==1)--p.bytes;if(fault==2)--p.context_bytes;if(fault==3)p.rules=4096;
    if(fault==4)p.element_bytes=3;if(fault==5)p.snapshot_slot=64;if(fault==6)p.reserved=1;
    if(fault==7){p.rules=2048;p.join=1;p.weighted=1;}
    if(fault==8)setenv("SGPL_NO_EXEC_SIMPLIFY","1",1);
    if(fault==9)atomic_store(&fail_malloc,1);if(fault==10)atomic_store(&fail_calloc,1);
    if(fault==12)p.version=1; /* Old runtime cannot execute the new row ABI contract. */
    if(fault==13)p.version=2;if(fault==14)p.rules=96;
    if(fault==15){p.rules=608;p.element_bytes=8;}
    int r=sgpl_try_semantic_stage_v1(&graph,&p,sources[0],5,&out,&cap,values,fault==11?4:5);
    check(r==-1&&!memcmp(values,copy,sizeof copy)&&!out&&cap==0,"descriptor/bounds/disable/allocation refusal before effects");
    atomic_store(&fail_malloc,0);atomic_store(&fail_calloc,0);unsetenv("SGPL_NO_EXEC_SIMPLIFY");++checks;
  }
  for(unsigned rule=512;rule<=2048;rule*=2)for(int allocator=0;allocator<2;++allocator)for(int nth=1;nth<=4;++nth) {
    int32_t before[5]={5,4,3,2,1},data[5];memcpy(data,before,sizeof data);active=data;
    sgpl_semantic_stage_plan_v1 p=plan;p.rules=rule;p.join=rule==2048?SGPL_JOIN_SMIN:0;operation=rule==2048?0:4;
    out=NULL;cap=0;atomic_store(allocator?&fail_calloc:&fail_malloc,nth);
    int r=sgpl_try_semantic_stage_v1(&graph,&p,sources[0],5,&out,&cap,data,5);
    atomic_store(&fail_malloc,0);atomic_store(&fail_calloc,0);
    if(r<0)check(!memcmp(data,before,sizeof data)&&out==NULL&&cap==0,"all optional preparation allocation refusals are transactional");
    else check(r>=0,"nonapplicable allocation injection permits completion");free(out);out=NULL;cap=0;++checks;
  }
  {
    int32_t before[5]={5,4,3,2,1},data[5];int32_t bad[]={0,5};memcpy(data,before,sizeof data);active=data;
    check(sgpl_try_semantic_stage_v1(&graph,&plan,bad,2,&out,&cap,data,5)<0&&!memcmp(data,before,sizeof data),"invalid source rejected before effects");
    int saved=columns[0];columns[0]=5;
    autograph_record_adjacency_state(&graph,5,10,rows,columns);
    check(sgpl_try_semantic_stage_v1(&graph,&plan,sources[0],5,&out,&cap,data,5)<0&&!memcmp(data,before,sizeof data),"invalid destination rejected before effects");columns[0]=saved;
    autograph_record_adjacency_state(&graph,5,10,rows,columns);
    int32_t *aliased=data;int64_t aliascap=5;
    check(sgpl_try_semantic_stage_v1(&graph,&plan,sources[0],5,&aliased,&aliascap,data,5)<0&&!memcmp(data,before,sizeof data),"output alias rejected before effects");
    check(sgpl_try_semantic_stage_v1(&graph,&plan,sources[0],5,&out,&cap,rows,5)<0,"CSR/target alias rejected");checks+=4;
  }
  {
    static ExecTestGraph empty,isolated;int64_t er[]={0},ir[]={0,0,0,0};int32_t iv[]={1,2,3},in[]={0,1,2};
    empty=(ExecTestGraph){.n=0,.m=0,.row_ptr=er,.col_idx=NULL,.directed=1};
    isolated=(ExecTestGraph){.n=3,.m=0,.row_ptr=ir,.col_idx=NULL,.directed=1};
    autograph_init(&empty,0,0,NULL,NULL,NULL);autograph_ensure_layout(&empty,0,0,er,NULL,&fixture_nodes,&fixture_edges,NULL,LAYOUT_CSR);
    autograph_init(&isolated,3,0,NULL,NULL,NULL);autograph_ensure_layout(&isolated,3,0,ir,NULL,&fixture_nodes,&fixture_edges,NULL,LAYOUT_CSR);
    for(unsigned rule=512;rule<=2048;rule*=2) {
      sgpl_semantic_stage_plan_v1 p=plan;p.rules=rule;p.join=rule==2048?SGPL_JOIN_SMIN:0;active=iv;
      check(sgpl_try_semantic_stage_v1(&empty,&p,NULL,0,&out,&cap,iv,0)==0,"empty graph executes no contribution");
      check(sgpl_try_semantic_stage_v1(&isolated,&p,in,3,&out,&cap,iv,3)==0&&iv[0]==1&&iv[1]==2&&iv[2]==3,"isolated vertices do not read/write an accumulator contribution");checks+=2;
    }
    free(out);
  }
  {
    /* Unequal duplicate weights, repeated/permuted inputs, wraparound, repeat
     * use, and in-place topology invalidation must retain first-match meaning.
     * The accessor fallback counter proves adjacency searches disappear. */
    static ExecTestGraph weighted;
    int64_t wr[]={0,4,6,9,10,10};int32_t wc[]={1,1,0,2,2,3,3,0,2,3};
    int32_t ww[]={3,999,INT32_MAX,7,-4,9,2,INT32_MIN,5,6};
    weighted=(ExecTestGraph){.n=5,.m=10,.row_ptr=wr,.col_idx=wc,.weights=ww,.directed=1};
    autograph_init(&weighted,5,10,NULL,NULL,ep);
    autograph_ensure_layout(&weighted,5,10,wr,wc,&fixture_nodes,&fixture_edges,ep,LAYOUT_CSR);
    for(int mutation=0;mutation<3;++mutation) {
      if(mutation==1){wc[0]=2;autograph_record_adjacency_state(&weighted,5,10,wr,wc);}
      if(mutation==2){ww[0]=-9;ww[1]=17;} /* Values change; cached topology reads live weights. */
      for(unsigned rules=512;rules<2048;rules+=512)for(int order=0;order<3;++order) {
        if(rules==1024&&order)continue;
        for(int repeat=0;repeat<3;++repeat) {
          int32_t a[]={INT32_MAX,2,-3,INT32_MIN,5},b[5],snapshot[5],expected[128];
          memcpy(b,a,sizeof a);memcpy(snapshot,a,sizeof a);int head=0;
          for(int i=0;i<counts[order];++i)for(int64_t e=wr[sources[order][i]];e<wr[sources[order][i]+1];++e) {
            int32_t u=sources[order][i],v=wc[e],w=graph_get_edge_weight(&weighted,u,v);
            a[v]=(int32_t)((uint32_t)a[v]+(uint32_t)snapshot[u]+(uint32_t)w);expected[head++]=v;
          }
          active=b;out=NULL;cap=0;atomic_store(&weight_fallbacks,0);atomic_store(&worker_mask,0);
          sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),rules,0,sizeof(sgpl_exec_ctx),weighted_pair,weighted_row,4,0,1,0};
          int got=sgpl_try_semantic_stage_v1(&weighted,&p,sources[order],counts[order],&out,&cap,b,5);
          check(got==head&&!memcmp(a,b,sizeof a)&&!memcmp(out,expected,(size_t)head*4),"weighted order/duplicates/wraparound and cache invalidation");
          check(atomic_load(&weight_fallbacks)==0,"weighted R10/R11 use first-match indices without adjacency searches");
          if(head&&sgpl_configured_worker_count()>1&&!sgpl_is_worker_pool_thread())
            check((atomic_load(&worker_mask)&(atomic_load(&worker_mask)-1))!=0,"destination-owned callbacks execute on multiple workers");
          free(out);++checks;
        }
      }
    }
    for(unsigned rules=512;rules<2048;rules+=512)for(unsigned pruning=32;pruning<=96;pruning+=32)
    for(int order=0;order<3;++order)for(int data=0;data<4;++data)for(int disabled=0;disabled<2;++disabled) {
      if(rules==1024&&order)continue;
      int32_t a[5],b[5],snapshot[5],expected[128];int head=0;
      for(int i=0;i<5;++i)a[i]=data==0?INT32_MIN:(int32_t)((uint32_t)data*2654435761u+(uint32_t)i*2246822519u);
      memcpy(b,a,sizeof a);memcpy(snapshot,a,sizeof a);
      for(int i=0;i<counts[order];++i)for(int64_t e=wr[sources[order][i]];e<wr[sources[order][i]+1];++e) {
        int32_t u=sources[order][i],v=wc[e];
        int32_t candidate=(int32_t)((uint32_t)snapshot[u]+(uint32_t)graph_get_edge_weight(&weighted,u,v));
        if(candidate<a[v]){a[v]=candidate;expected[head++]=v;}
      }
      active=b;out=NULL;cap=0;atomic_store(&minimum_calls,0);atomic_store(&weight_fallbacks,0);
      if(disabled)setenv("SGPL_NO_EXEC_COMPOSITION","1",1);
      sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),rules|pruning,0,sizeof(sgpl_exec_ctx),minimum_pair,minimum_row,4,0,1,0};
      int got=sgpl_try_semantic_stage_v1(&weighted,&p,sources[order],counts[order],&out,&cap,b,5);
      unsetenv("SGPL_NO_EXEC_COMPOSITION");
      check(got==head&&!memcmp(a,b,sizeof a)&&!memcmp(out,expected,(size_t)head*4),"composed R6/R7 preserve exact weighted minimum notifications and modular overflow");
      check(!atomic_load(&weight_fallbacks),"composed callbacks retain direct first-match weight access");
      if(!data&&!disabled&&(pruning&64))check(!atomic_load(&minimum_calls),"R7 eliminates all ineffective contributions before callback execution");
      if(!data&&!disabled&&pruning==32&&order==1)check(atomic_load(&minimum_calls)==10,"R6 removes repeated source traversal before key preparation");
      if(!data&&disabled) {
        int original=0;for(int i=0;i<counts[order];++i)original+=(int)(wr[sources[order][i]+1]-wr[sources[order][i]]);
        check(atomic_load(&minimum_calls)==original,"composition disablement retains the original contribution stream");
      }
      free(out);++checks;
    }
    for(int mutation=0;mutation<2;++mutation)for(int order=0;order<3;++order)
    for(int data=0;data<12;++data)for(int disabled=0;disabled<2;++disabled) {
      if(mutation){wc[0]=1;autograph_record_adjacency_state(&weighted,5,10,wr,wc);}
      int32_t a[5],b[5],snapshot[5];unsigned expected=0;int head=0;
      for(int i=0;i<5;++i)a[i]=data==0?INT32_MIN:(int32_t)((uint32_t)data*2654435761u+(uint32_t)i*2246822519u);
      memcpy(b,a,sizeof a);memcpy(snapshot,a,sizeof a);
      for(int i=0;i<counts[order];++i)for(int64_t e=wr[sources[order][i]];e<wr[sources[order][i]+1];++e) {
        int32_t u=sources[order][i],v=wc[e],candidate=(int32_t)((uint32_t)snapshot[u]+(uint32_t)graph_get_edge_weight(&weighted,u,v));
        if(candidate<a[v]){a[v]=candidate;expected|=1u<<v;++head;}
      }
      active=b;out=NULL;cap=0;atomic_store(&minimum_calls,0);atomic_store(&weight_fallbacks,0);
      if(disabled)setenv("SGPL_NO_ITERATION_MEMORY","1",1);
      sgpl_semantic_stage_plan_v1 p={SGPL_SEMANTIC_STAGE_VERSION,sizeof(p),512|128,0,sizeof(sgpl_exec_ctx),minimum_pair,NULL,4,0,1,0};
      int got=sgpl_try_semantic_stage_v1(&weighted,&p,sources[order],counts[order],&out,&cap,b,5);
      unsetenv("SGPL_NO_ITERATION_MEMORY");unsigned actual=0;int unique=1;
      if(got>=0)for(int i=0;i<got;++i){if(out[i]<0||out[i]>=5){unique=0;break;}unsigned bit=1u<<out[i];if(actual&bit)unique=0;actual|=bit;}
      check(got>=0&&!memcmp(a,b,sizeof a)&&actual==expected,"R8 set round preserves signed modular minima and changed set");
      check(disabled?got==head:unique&&got==__builtin_popcount(expected),"only R8 may collapse unobserved duplicate notifications");
      check(disabled||!atomic_load(&minimum_calls),"R8 folds before one destination write, no callback replay");
      check(!atomic_load(&weight_fallbacks),"R8 retains cached first-match duplicate weights");
      free(out);++checks;
    }
  }
  operation=0;
  for(int allocator=0;allocator<2;++allocator)for(int nth=1;nth<=12;++nth) {
    int32_t a[]={INT32_MAX,2,-3,0,5},b[5],expected[128];memcpy(b,a,sizeof a);
    int wanted=reference(&graph,a,sources[1],counts[1],expected);unsigned wanted_set=0;
    for(int i=0;i<wanted;++i)wanted_set|=1u<<expected[i];
    active=b;out=NULL;cap=0;sgpl_semantic_stage_plan_v1 p=plan;p.rules=512|128;
    SetFailureTask task={&graph,p,sources[1],counts[1],&out,&cap,b,-1};pthread_t thread;
    atomic_store(&fail_thread,0);atomic_store(allocator?&fail_calloc:&fail_malloc,nth);
    // Each fresh thread has an empty exclusive workspace, so allocation
    // injection exercises preparation buffers rather than a warmed capacity.
    int started=__real_pthread_create(&thread,NULL,set_failure_thread,&task);
    if(!started)pthread_join(thread,NULL);
    atomic_store(&fail_malloc,0);atomic_store(&fail_calloc,0);
    check(!started,"fresh R8 workspace fixture starts");int got=task.result;
    int32_t initial[]={INT32_MAX,2,-3,0,5};
    if(got<0)check(!memcmp(b,initial,sizeof b)&&!out&&!cap,"R8 workspace allocation refusal precedes all destination effects");
    else {unsigned actual=0;for(int i=0;i<got;++i)actual|=1u<<out[i];check(got==__builtin_popcount(wanted_set)&&actual==wanted_set&&!memcmp(a,b,sizeof a),"R8 allocation recovery retains minimum and changed set");}
    free(out);++checks;
  }
  for(int allocator=0;allocator<2;++allocator)for(int nth=1;nth<=12;++nth) {
    int32_t a[]={INT32_MAX,2,-3,0,5},b[5],expected[128];memcpy(b,a,sizeof a);
    int wanted=reference(&graph,a,sources[1],counts[1],expected);active=b;out=NULL;cap=0;
    sgpl_semantic_stage_plan_v1 p=plan;p.rules=608;
    atomic_store(allocator?&fail_calloc:&fail_malloc,nth);
    int got=sgpl_try_semantic_stage_v1(&graph,&p,sources[1],counts[1],&out,&cap,b,5);
    atomic_store(&fail_malloc,0);atomic_store(&fail_calloc,0);
    int32_t initial[]={INT32_MAX,2,-3,0,5};
    if(got<0)check(!memcmp(b,initial,sizeof b)&&!out&&!cap,"composed preparation allocation refusal is before effects");
    else check(got==wanted&&!memcmp(a,b,sizeof a)&&!memcmp(out,expected,(size_t)wanted*4),"composed allocation and worker-pool recovery preserve exact results");
    free(out);++checks;
  }
  frontier_removal_cases();
  printf("semantic runtime: %u cases, %d failures\n",checks,failures);return failures?1:0;
}
