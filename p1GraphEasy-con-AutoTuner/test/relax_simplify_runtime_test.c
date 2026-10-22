#define SGPL_SPATIAL_FIXTURE_ONLY
#define SGPL_FIXTURE_EDGE_COUNT(g) (((ExecTestGraph *)(g))->m)
#include "spatial_exec_test.c"
#include "relax_simplify_plan.h"
#include "append_journal_plan.h"
extern void sgpl_exec_bind_relax_simplification_v1(sgpl_exec_ctx *,const sgpl_relax_simplify_plan_v1 *,int32_t *);
extern void sgpl_exec_bind_append_journal_v1(sgpl_exec_ctx *,const sgpl_append_journal_plan_v1 *);
extern int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *,int32_t);
static _Atomic unsigned calls;
static int weighted;
int32_t graph_get_edge_weight(void *opaque,int32_t u,int32_t v) {
  ExecTestGraph *g=opaque;
  for(int64_t e=g->row_ptr[u];e<g->row_ptr[u+1];++e)if(g->col_idx[e]==v)return g->weights?g->weights[e]:1;
  return INT32_MAX;
}
static void body(void *state,sgpl_exec_ctx *c,int32_t u,int32_t v,int journal) {
  atomic_fetch_add(&calls,1);
  int32_t *a=state,*snapshot=autograph_exec_ctx_snapshot(c,0),candidate=snapshot[u];
  if(weighted)candidate=(int32_t)((uint32_t)candidate+(uint32_t)autograph_exec_ctx_edge_weight(c,c->graph,u,v));
  if(candidate<a[v]){a[v]=candidate;if(journal)sgpl_frontier_journal_activate_v1(c,v);else autograph_frontier_activate(c,v);}
}
static void pair(void *s,sgpl_exec_ctx *c,int32_t u,int32_t v){body(s,c,u,v,0);}
static void journal_pair(void *s,sgpl_exec_ctx *c,int32_t u,int32_t v){body(s,c,u,v,1);}
static sgpl_exec_ctx *relax_context(ExecTestGraph *g,int32_t *input,int count,int32_t *values,
    int32_t **out,int64_t *capacity,int32_t *head,sgpl_relax_simplify_plan_v1 *p,int journal,int fault) {
  sgpl_runtime_op *op=autograph_exec_op_create(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL,values,pair,NULL,NULL,NULL,NULL,NULL,0);
  sgpl_runtime_op *ops[]={op};
  sgpl_exec_ctx *c=autograph_exec_ctx_create(g,SGPL_TRAVERSE_OWNER_V,SGPL_DOMAIN_FRONTIER,NULL,NULL,*out,0,head,NULL,0,ops,1,-1);
  check(c!=NULL,"create relaxation context");autograph_exec_ctx_set_dag_axes(c,1,-1);
  check(!autograph_exec_ctx_bind_array_frontier(c,input,count,out,capacity,1),"bind relaxation frontier");
  if(p){if(fault==1)atomic_store(&fail_calloc,1);sgpl_exec_bind_relax_simplification_v1(c,p,values);atomic_store(&fail_calloc,0);}
  static const sgpl_append_journal_plan_v1 jp={SGPL_APPEND_JOURNAL_VERSION,sizeof(jp),5|SGPL_JOURNAL_PROVEN_APPEND,1,pair,journal_pair,NULL,NULL,NULL};
  if(journal)sgpl_exec_bind_append_journal_v1(c,&jp);
  if(fault==2&&p&&p->rules&&c->execution_scratch)atomic_store(&fail_malloc,3);
  check(!autograph_exec_ctx_prepare_spatial(c,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),"prepare relaxation records");
  atomic_store(&fail_malloc,0);
  check(!autograph_exec_ctx_bind_snapshot_range(c,0,values,g->n,4),"capture immutable round source");
  return c;
}
int main(void) {
  setenv("SGPL_GPU_BACKEND","0",1);
  int64_t rows[]={0,4,8,10,10,12,13,13};int32_t cols[]={1,1,0,6,2,2,0,1,2,3,0,4,6},ep[26];
  int32_t weights[]={1,99,-3,INT32_MAX,-1,100,3,0,INT32_MIN,0,INT32_MAX,2,-7};
  for(int u=0;u<7;++u)for(int64_t e=rows[u];e<rows[u+1];++e){ep[2*e]=u;ep[2*e+1]=cols[e];}
  native_pairs=ep;ExecTestGraph g={.n=7,.m=13,.row_ptr=rows,.col_idx=cols,.directed=1};
  autograph_init(&g,7,13,NULL,NULL,ep);autograph_ensure_layout(&g,7,13,rows,cols,&fixture_nodes,&fixture_edges,ep,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
  int32_t input[]={0,0,3,1,4,6,2,0},large_input[8192],initial[3][7]={{7,6,5,4,3,2,1},{0,INT32_MAX,INT32_MIN,0,-1,1,INT32_MAX},{5,5,5,5,5,5,5}};
  for(int i=0;i<8192;++i)large_input[i]=i%7;
  unsigned checks=0,reductions=0;
  for(weighted=0;weighted<2;++weighted) {
    g.weights=weighted?weights:NULL;
    autograph_init(&g,7,13,NULL,NULL,ep);autograph_ensure_layout(&g,7,13,rows,cols,&fixture_nodes,&fixture_edges,ep,LAYOUT_CSR);autograph_build_clean_cut(&g,6);
    for(unsigned subset=0;subset<8;++subset)for(int journal=0;journal<2;++journal)
      for(int data=0;data<3;++data)for(int countCase=0;countCase<4;++countCase)for(int mode=0;mode<13;++mode) {
      if(countCase==3&&mode)continue;
      int count=countCase==0?0:countCase==1?1:countCase==2?8:8192;
      int32_t *sources=countCase==3?large_input:input;
      int previous_failures=failures;
      sgpl_relax_simplify_plan_v1 p={1,sizeof(p),subset<<5,(uint32_t)weighted,sizeof(sgpl_exec_ctx),sizeof(sgpl_runtime_op),pair};
      if(mode==1)++p.version;if(mode==2)--p.bytes;if(mode==3)--p.context_bytes;if(mode==4)--p.operation_bytes;
      if(mode==5)p.pair=journal_pair;if(mode==6)p.weighted=2;if(mode==7)p.rules=256;
      int32_t a[7],b[7],oa[256],ob[256],*pa=oa,*pb=ob,ha=0,hb=0;int64_t ca=256,cb=256;
      memcpy(a,initial[data],sizeof a);memcpy(b,a,sizeof a);
      for(int i=0;i<256;++i)oa[i]=ob[i]=1000+i;
      sgpl_exec_ctx *x=relax_context(&g,sources,count,a,&pa,&ca,&ha,NULL,journal,0);
      sgpl_exec_ctx *y=relax_context(&g,sources,count,b,&pb,&cb,&hb,&p,journal,mode==8?1:mode==12?2:0);
      int64_t before=x->gpu_record_count,after=y->gpu_record_count;
      if(!mode&&y->execution_scratch&&subset) {
        int first[7]={0},changed[7]={0};int64_t expected=0;
        for(int i=0;i<count;++i) {
          int u=sources[i];if((p.rules&32)&&first[u]++)continue;
          for(int64_t e=rows[u];e<rows[u+1];++e) {
            int v=cols[e];int32_t candidate=initial[data][u];
            if(weighted)candidate=(int32_t)((uint32_t)candidate+(uint32_t)graph_get_edge_weight(&g,u,v));
            if(p.rules&128){if(candidate<initial[data][v])changed[v]=1;continue;}
            if(!(p.rules&64)||candidate<initial[data][v])++expected;
          }
        }
        if(p.rules&128)for(int v=0;v<7;++v)expected+=changed[v];
        check(after==expected&&y->array_work_count==after,"structural record and empty append-slot removal");
        unsigned char *keys=calloc((size_t)(after?after:1),1);int stride=y->array_compact_records==4?4:3;
        int dense=1;
        for(int64_t i=0;i<after;++i){int key=y->gpu_records[stride*i+2];if(key<0||key>=after||keys[key]){dense=0;break;}keys[key]=1;}
        check(dense,"compacted append keys are unique and cover the complete physical domain");free(keys);
      }
      if(mode==9)setenv("SGPL_NO_EXEC_SIMPLIFY","1",1);
      if(mode==10)autograph_update_csr_pointers(&g,rows,cols);
      for(int round=0;round<(mode==11?2:1);++round) {
        int ra=autograph_frontier_execute(&g,x);atomic_store(&calls,0);int rb=autograph_frontier_execute(&g,y);
        check(ra>=0&&rb>=0&&!memcmp(a,b,sizeof a),"minimum arrays exact including signed wrap/negative weights");
        check(x->spatial_occupied_groups==y->spatial_occupied_groups,"occupied destinations retained after filtering");
        if(!(p.rules&128)||(mode&&mode!=11)||!subset||!y->execution_scratch)check(ra==rb&&!memcmp(pa,pb,(size_t)ra*4),"ordered frontier notifications preserved");
        else {
          int sa[7]={0},sb[7]={0};for(int i=0;i<ra;++i)sa[pa[i]]=1;for(int i=0;i<rb;++i)++sb[pb[i]];
          for(int i=0;i<7;++i)check(sa[i]==sb[i],"R8 one activation per changed destination");
          check((ra==0)==(rb==0),"R8 preserves termination");
        }
        ++checks;
      }
      if(!mode&&after<before)++reductions;
      if(failures!=previous_failures)fprintf(stderr,"bad weighted=%d subset=%u journal=%d data=%d count=%d mode=%d records=%lld/%lld\n",weighted,subset,journal,data,count,mode,(long long)before,(long long)after);
      unsetenv("SGPL_NO_EXEC_SIMPLIFY");autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
    }
  }
  weighted=0;
  {
    int32_t a[7]={7,6,5,4,3,2,1},b[7],*pa=calloc(1,4),*pb=calloc(1,4),ha=0,hb=0;int64_t ca=1,cb=1;
    memcpy(b,a,sizeof a);pa[0]=pb[0]=777;
    sgpl_relax_simplify_plan_v1 p={1,sizeof(p),32|64,0,sizeof(sgpl_exec_ctx),sizeof(sgpl_runtime_op),pair};
    sgpl_exec_ctx *x=relax_context(&g,input,8,a,&pa,&ca,&ha,NULL,0,0);
    sgpl_exec_ctx *y=relax_context(&g,input,8,b,&pb,&cb,&hb,&p,0,0);
    check(!autograph_exec_ctx_reserve_prepared_output(x)&&!autograph_exec_ctx_reserve_prepared_output(y)&&ca==cb&&pa[0]==pb[0],"original output reservation preserved after private key compaction");
    check(autograph_frontier_execute(&g,x)==autograph_frontier_execute(&g,y)&&!memcmp(a,b,sizeof a),"reserved output retains exact execution result");
    autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);free(pa);free(pb);++checks;
  }
  for(int ncase=0;ncase<2;++ncase) {
    int64_t empty_rows[8]={0};int32_t empty_cols[1]={0};
    ExecTestGraph empty={.n=ncase?7:0,.m=0,.row_ptr=empty_rows,.col_idx=empty_cols,.directed=1};
    autograph_init(&empty,empty.n,0,NULL,NULL,ep);
    if(empty.n)check(autograph_build_clean_cut(&empty,6)>0,"empty topology partitions");
    for(unsigned subset=1;subset<8;++subset) {
      int32_t a[7]={0},b[7]={0},oa[1]={0},ob[1]={0},*pa=oa,*pb=ob,ha=0,hb=0;int64_t ca=1,cb=1;
      sgpl_relax_simplify_plan_v1 p={1,sizeof(p),subset<<5,0,sizeof(sgpl_exec_ctx),sizeof(sgpl_runtime_op),pair};
      sgpl_exec_ctx *x=relax_context(&empty,input,ncase?8:0,a,&pa,&ca,&ha,NULL,0,0);
      sgpl_exec_ctx *y=relax_context(&empty,input,ncase?8:0,b,&pb,&cb,&hb,&p,0,0);
      check(!autograph_frontier_execute(&empty,x)&&!autograph_frontier_execute(&empty,y)&&!memcmp(a,b,sizeof a),"empty graph and empty partitions");
      autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);++checks;
    }
  }
  if(getenv("SGPL_TEST_EXPECT_SIMPLIFY"))check(reductions>0,"rules remove prepared records");
  printf("relaxation runtime: %u comparisons, %u reduced preparations, %d failures\n",checks,reductions,failures);
  return failures?1:0;
}
