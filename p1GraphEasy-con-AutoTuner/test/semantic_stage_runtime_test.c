#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
#include "semantic_stage_plan.h"
#include <math.h>
static int operation;
static void *active;
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
    sgpl_semantic_stage_plan_v1 p={1,sizeof(p),rules,0,sizeof(sgpl_exec_ctx),pair,row,operation<6?4:8,0,0,0};
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
    sgpl_semantic_stage_plan_v1 p={1,sizeof(p),2048,(unsigned)operation+1,sizeof(sgpl_exec_ctx),pair,NULL,4,0,0,0};
    int got=sgpl_try_semantic_stage_v1(&graph,&p,sources[order],counts[order],&out,&cap,b,5);
    check(got==0&&!memcmp(a,b,sizeof a),"whole fixed point matches round-separated extrema/bitwise joins");free(out);++checks;
  }
  operation=4;int32_t values[5]={5,4,3,2,1},copy[5],*out=NULL;int64_t cap=0;
  sgpl_semantic_stage_plan_v1 plan={1,sizeof(plan),512,0,sizeof(sgpl_exec_ctx),pair,row,4,0,0,0};active=values;
  for(int fault=0;fault<12;++fault) {
    memcpy(copy,values,sizeof copy);sgpl_semantic_stage_plan_v1 p=plan;
    if(fault==0)++p.version;if(fault==1)--p.bytes;if(fault==2)--p.context_bytes;if(fault==3)p.rules=4096;
    if(fault==4)p.element_bytes=3;if(fault==5)p.snapshot_slot=64;if(fault==6)p.reserved=1;
    if(fault==7){p.rules=2048;p.join=1;p.weighted=1;}
    if(fault==8)setenv("SGPL_NO_EXEC_SIMPLIFY","1",1);
    if(fault==9)atomic_store(&fail_malloc,1);if(fault==10)atomic_store(&fail_calloc,1);
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
    check(sgpl_try_semantic_stage_v1(&graph,&plan,sources[0],5,&out,&cap,data,5)<0&&!memcmp(data,before,sizeof data),"invalid destination rejected before effects");columns[0]=saved;
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
  printf("semantic runtime: %u cases, %d failures\n",checks,failures);return failures?1:0;
}
