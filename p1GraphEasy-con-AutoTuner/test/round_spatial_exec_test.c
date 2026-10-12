#define SGPL_SPATIAL_FIXTURE_ONLY
#include "spatial_exec_test.c"
#include "../parallel_runtime.h"
typedef struct joint_state joint_state;
typedef struct round_frame {
  joint_state *shared;
  spatial_state body;
  sgpl_exec_ctx *ctx;
  int64_t round;
  int32_t seen[7], next[7], head, values[7];
  int started;
  RoaringBitmap input, committed;
} round_frame;
struct joint_state {
  ExecTestGraph *graph;
  int source, frontier, parts, rounds, overlap, reserve_fail, work_fail, dag_alloc_fail;
  atomic_int setups, finals, releases, reserves, active, peak, errors;
  atomic_int completed[32], occurrences[32], active_round[32];
  int result[32];
};
static int32_t reserve_round(void *opaque, void *state, int64_t round) {
  joint_state *s=opaque; round_frame *f=state;
  f->shared=s; f->round=round;
  check(sgpl_round_spatial_current_frame()==f,"reservation frame channel is invocation-local");
  atomic_fetch_add(&s->reserves,1);
  if(s->reserve_fail && round==1)return SGPL_DAG_ERR_NOMEM;
  expected(&f->body,s->source,s->parts); f->body.overlap=0;
  f->ctx=context(s->graph,&f->body,s->source,s->frontier,f->seen,f->next,&f->head,NULL);
  if(!f->ctx)return SGPL_DAG_ERR_NOMEM;
  int status=autograph_exec_ctx_reserve_frontier(f->ctx,1,s->frontier);
  if(status)return status;
  int key=s->source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
  status=autograph_exec_ctx_prepare_spatial(f->ctx,key,key,s->frontier);
  if(status)return status;
  status=autograph_exec_ctx_reserve_snapshot(f->ctx,0,7,sizeof(int32_t));
  int groups=s->rounds<sgpl_configured_worker_count()?s->rounds:sgpl_configured_worker_count();
  if(!status && s->dag_alloc_fail && atomic_load(&s->reserves)==groups)atomic_store(&fail_calloc,s->dag_alloc_fail);
  return status;
}
static int32_t setup_round(void *opaque,int64_t round) {
  round_frame *f=opaque; joint_state *s=f->shared;
  if(sgpl_round_spatial_current_frame()!=f||sgpl_current_thread_budget()!=1)atomic_fetch_add(&s->errors,1);
  for(int i=0;i<7;++i)f->values[i]=100*(int)round+i;
  f->input.bits=127;f->committed.bits=UINT64_MAX;
  int32_t vertices[7]={0,1,2,3,4,5,6};
  int status=round%2 ? autograph_exec_ctx_materialize_frontier_array(f->ctx,vertices,7)
      : autograph_exec_ctx_materialize_frontier_bitmap(f->ctx,&f->input);
  if(status)return status;
  status=autograph_exec_ctx_materialize_snapshot(f->ctx,0,f->values,sizeof(f->values));
  if(status)return status;
  status=sgpl_exec_spatial_begin(f->ctx);
  if(status)return status;
  status=sgpl_exec_spatial_publish(f->ctx);
  if(status)return status;
  f->started=1; atomic_fetch_add(&s->setups,1); return 0;
}
static int32_t execute_partition(void *opaque,int64_t p) {
  round_frame *f=opaque; joint_state *s=f->shared;
  if(!f->started||sgpl_round_spatial_current_frame()!=f||sgpl_current_thread_budget()!=1)atomic_fetch_add(&s->errors,1);
  int *old=autograph_exec_ctx_snapshot(f->ctx,0);
  for(int i=0;i<7;++i)if(old[i]!=100*f->round+i)atomic_fetch_add(&s->errors,1);
  maximum(&s->peak,atomic_fetch_add(&s->active,1)+1);
  maximum(&f->body.peak,atomic_fetch_add(&s->active_round[f->round],1)+1);
  if(s->overlap) {
    double deadline=time_seconds()+2;
    while(atomic_load(&s->active)<4 && time_seconds()<deadline)usleep(100);
  }
  int status=sgpl_exec_spatial_partition(f->ctx,(int32_t)p);
  atomic_fetch_sub(&s->active_round[f->round],1); atomic_fetch_sub(&s->active,1);
  if(s->work_fail && f->round==0)return SGPL_DAG_ERR_NODE;
  return status;
}
static int32_t finalize_round(void *opaque,int64_t round) {
  round_frame *f=opaque; joint_state *s=f->shared;
  if(atomic_load(&s->active_round[round])||!f->started)atomic_fetch_add(&s->errors,1);
  int status=sgpl_exec_spatial_finish(f->ctx);
  if(status<0)return status;
  if(s->frontier) {
    int rc=autograph_exec_ctx_commit_frontier_array(f->ctx,f->next,status);
    if(!rc)rc=autograph_exec_ctx_commit_frontier_bitmap(f->ctx,&f->committed,status);
    if(rc)return rc;
    uint64_t bits=0;for(int i=0;i<status;++i)bits|=UINT64_C(1)<<f->next[i];
    if(bits!=((1<<1)|(1<<2)|(1<<3)|(1<<6))||f->committed.bits!=bits)atomic_fetch_add(&s->errors,1);
  }
  if(status!=(s->frontier?4:0)||atomic_load(&f->body.calls)!=7||atomic_load(&f->body.errors))atomic_fetch_add(&s->errors,1);
  for(int p=0;p<s->parts;++p)
    if(f->body.length[p]!=f->body.expected_length[p]||memcmp(f->body.sequence[p],f->body.expected[p],f->body.length[p]*sizeof(int)))atomic_fetch_add(&s->errors,1);
  if(s->source)for(int u=0;u<7;++u)if(f->body.begin[u]!=1||f->body.end[u]!=1)atomic_fetch_add(&s->errors,1);
  if(s->overlap && atomic_load(&f->body.peak)<2)atomic_fetch_add(&s->errors,1);
  s->result[round]=100*(int)round+atomic_load(&f->body.calls);
  atomic_fetch_add(&s->completed[round],1); atomic_fetch_add(&s->finals,1);return 0;
}
static void release_round(void *opaque) {
  round_frame *f=opaque; joint_state *s=f->shared;
  if(s) { atomic_fetch_add(&s->occurrences[f->round],atomic_load(&f->body.calls));atomic_fetch_add(&s->releases,1); }
  autograph_exec_ctx_destroy(f->ctx);
}
static sgpl_round_spatial_plan plan(joint_state *s,int rounds) {
  s->rounds=rounds;
  return (sgpl_round_spatial_plan){.shared=s,.first_round=0,.round_count=rounds,.frame_bytes=sizeof(round_frame),
    .partitions=s->parts,.reserve=reserve_round,.setup=setup_round,.partition=execute_partition,.finalize=finalize_round,.release=release_round};
}
static void run_cases(ExecTestGraph *g) {
  uint8_t *shared_membership=autograph_scratch_membership(g);
  int32_t *shared_seen=autograph_scratch_dest_seen(g),*shared_next=autograph_scratch_next_frontier(g);
  for(int i=0;i<7;++i){shared_membership[i]=0xA5;shared_seen[i]=1000+i;shared_next[i]=2000+i;}
  for(int layout=0;layout<4;++layout)for(int source=0;source<2;++source) {
    autograph_ensure_layout(g,7,7,g->row_ptr,g->col_idx,&fixture_nodes,&fixture_edges,pairs,layout);
    int parts=autograph_build_clean_cut(g,3);
    joint_state s={.graph=g,.source=source,.frontier=1,.parts=parts};
    sgpl_round_spatial_plan p=plan(&s,13);
    check(sgpl_exec_dag_round_spatial(&p,sgpl_configured_worker_count())==0,"combined native execution succeeds");
    check(!atomic_load(&s.errors)&&!atomic_load(&s.active),"round state, snapshot versions, lifecycle, order and joins are isolated");
    check(atomic_load(&s.finals)==13&&atomic_load(&s.releases)==13,"complete lifecycle exactly once across windows");
    for(int i=0;i<13;++i)check(s.result[i]==100*i+7&&atomic_load(&s.completed[i])==1&&atomic_load(&s.occurrences[i])==7,"outputs and duplicate occurrence counts equal serial execution");
  }
  for(int i=0;i<7;++i)check(shared_membership[i]==0xA5&&shared_seen[i]==1000+i&&shared_next[i]==2000+i,
      "round frontier publication and commit leave graph scratch untouched");
  if(sgpl_configured_worker_count()>=4) {
    int parts=autograph_build_clean_cut(g,2);
    joint_state s={.graph=g,.parts=parts,.overlap=1}; sgpl_round_spatial_plan p=plan(&s,2);
    check(!sgpl_exec_dag_round_spatial(&p,4)&&!atomic_load(&s.errors)&&atomic_load(&s.peak)==4,"two rounds and two batches overlap through one four-worker DAG");
  }
}
static int32_t empty_setup(void *f,int64_t r){(void)r;(*(int*)f)++;return 0;}
static int32_t empty_finish(void *f,int64_t r){(void)r;check(*(int*)f==1,"empty round setup precedes finish");return 0;}
static int32_t empty_reserve(void *s,void *f,int64_t r){(void)s;(void)f;(void)r;return 0;}
static void empty_release(void *f){(void)f;}
static void run_failures(ExecTestGraph *g) {
  int parts=autograph_build_clean_cut(g,3);
  for(int n=1;n<=3;++n) {
    joint_state s={.graph=g,.parts=parts}; sgpl_round_spatial_plan p=plan(&s,2);
    atomic_store(&fail_calloc,n);
    check(sgpl_exec_dag_round_spatial(&p,4)==SGPL_DAG_ERR_NOMEM&&!atomic_load(&s.setups),"DAG/frame allocation fails before user effects");
  }
  for(int n=1;n<=4;++n) {
    joint_state s={.graph=g,.parts=parts,.dag_alloc_fail=n};sgpl_round_spatial_plan p=plan(&s,2);
    check(sgpl_exec_dag_round_spatial(&p,4)==SGPL_DAG_ERR_NOMEM&&!atomic_load(&s.setups)&&atomic_load(&s.reserves)==atomic_load(&s.releases),"scheduler allocation failure releases reservations before user effects");
  }
  sgpl_push_thread_budget(1);
  joint_state limited={.graph=g,.parts=parts};sgpl_round_spatial_plan limited_plan=plan(&limited,3);
  check(!sgpl_exec_dag_round_spatial(&limited_plan,8)&&atomic_load(&limited.peak)==1&&sgpl_current_thread_budget()==1,"parent grant caps combined dispatch and is restored");
  sgpl_pop_thread_budget();
  joint_state s={.graph=g,.parts=parts,.reserve_fail=1};sgpl_round_spatial_plan p=plan(&s,3);
  check(sgpl_exec_dag_round_spatial(&p,1)<0&&atomic_load(&s.completed[0])==1&&atomic_load(&s.occurrences[0])==7&&!atomic_load(&s.completed[1]),"later-window failure returns without replaying completed rounds");
  s=(joint_state){.graph=g,.parts=parts,.work_fail=1};p=plan(&s,2);
  check(sgpl_exec_dag_round_spatial(&p,1)<0&&!atomic_load(&s.completed[0])&&atomic_load(&s.occurrences[0])>0&&atomic_load(&s.occurrences[0])<=7,"batch failure cancels finalization without replay");
  if(sgpl_configured_worker_count()>1) {
    s=(joint_state){.graph=g,.parts=parts};p=plan(&s,2);atomic_store(&fail_thread,1);
    check(!sgpl_exec_dag_round_spatial(&p,4)&&atomic_load(&fail_thread)==1&&atomic_load(&s.reserves)==atomic_load(&s.releases),"combined windows reuse workers and release all frames");
    atomic_store(&fail_thread,0);
  }
  for(int allocation=1;allocation<=3;++allocation) {
    spatial_state body={0};int head=0;
    sgpl_exec_ctx *ctx=context(g,&body,0,1,NULL,NULL,&head,NULL);
    atomic_store(&fail_calloc,allocation);
    check(autograph_exec_ctx_reserve_frontier(ctx,1,1)==SGPL_DAG_ERR_NOMEM,"partial frontier reservation returns allocation failure");
    autograph_exec_ctx_destroy(ctx);
  }
  sgpl_round_spatial_plan empty={.round_count=3,.frame_bytes=sizeof(int),.partitions=0,.reserve=empty_reserve,.setup=empty_setup,.finalize=empty_finish,.release=empty_release};
  check(!sgpl_exec_dag_round_spatial(&empty,4),"empty rounds execute setup and finish without a batch");
  empty.round_count=0;check(!sgpl_exec_dag_round_spatial(&empty,4),"zero rounds execute no callbacks");
  empty.round_count=2;empty.first_round=INT64_MAX;check(sgpl_exec_dag_round_spatial(&empty,4)==SGPL_DAG_ERR_INVALID,"round index overflow refuses before effects");
  check(!sgpl_round_spatial_current_frame()&&!sgpl_current_thread_budget(),"frame channel and budget are restored");
}
int main(void) {
  int64_t rp[8]={0,2,4,5,5,6,7,7};int32_t ci[7]={1,2,2,2,3,1,6};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);run_cases(&g);run_failures(&g);
  printf("round/spatial executor: %d failures\n",failures);return failures?1:0;
}
