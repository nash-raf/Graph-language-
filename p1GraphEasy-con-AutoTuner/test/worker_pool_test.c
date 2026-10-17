/* Persistent DAG/loop pool regression. Linker wrappers count actual thread
 * creation and joins, and inject failures before the first user effect. */
#include "../parallel_runtime.h"
#define main legacy_dag_test_main
#include "dag_scheduler_test.c"
#undef main
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

static atomic_int created, joined, fail_create, fail_allocation;
static int countdown(atomic_int *counter) {
  int old=atomic_load(counter);
  while(old>0)
    if(atomic_compare_exchange_weak(counter,&old,old-1))return old==1;
  return 0;
}
int __real_pthread_create(pthread_t *,const pthread_attr_t *,void *(*)(void *),void *);
int __real_pthread_join(pthread_t,void **);
void *__real_calloc(size_t,size_t);
int __wrap_pthread_create(pthread_t *t,const pthread_attr_t *a,void *(*f)(void *),void *s) {
  if(countdown(&fail_create))return EAGAIN;
  int rc=__real_pthread_create(t,a,f,s);
  if(!rc)atomic_fetch_add(&created,1);
  return rc;
}
int __wrap_pthread_join(pthread_t t,void **result) {
  int rc=__real_pthread_join(t,result);
  if(!rc)atomic_fetch_add(&joined,1);
  return rc;
}
void *__wrap_calloc(size_t n,size_t size) {
  if(countdown(&fail_allocation))return NULL;
  return __real_calloc(n,size);
}

typedef struct {
  atomic_int calls, errors, active, peak;
  atomic_int visits[16];
  int width, nested;
} pool_fixture;
static atomic_int all_active, all_peak;
static void raise_peak(atomic_int *peak,int value) {
  int old=atomic_load(peak);
  while(value>old&&!atomic_compare_exchange_weak(peak,&old,value)){}
}
static int32_t child_node(void *state,int64_t node) {
  pool_fixture *s=state;
  if(sgpl_current_thread_budget()!=1||!sgpl_is_worker_pool_thread())
    atomic_fetch_add(&s->errors,1);
  atomic_fetch_add(&s->visits[node],1);
  return 0;
}
static void nested_loop(int64_t i,void *state) {
  pool_fixture *s=state;
  if(sgpl_current_thread_budget()!=1)atomic_fetch_add(&s->errors,1);
  atomic_fetch_add(&s->visits[i],1);
}
static void root_loop(int64_t i,void *state) {
  pool_fixture *s=state;
  int lane=sgpl_current_worker_index();
  if(!sgpl_is_worker_pool_thread()||lane<0||lane>=s->width)
    atomic_fetch_add(&s->errors,1);
  atomic_fetch_add(&s->visits[i],1);
}
static int32_t pool_node(void *state,int64_t node) {
  pool_fixture *s=state;
  int lane=sgpl_current_worker_index();
  if(lane<0||lane>=s->width||sgpl_current_thread_budget()<1)
    atomic_fetch_add(&s->errors,1);
  int active=atomic_fetch_add(&s->active,1)+1;
  raise_peak(&s->peak,active);
  active=atomic_fetch_add(&all_active,1)+1;
  raise_peak(&all_peak,active);
  if(s->nested) {
    sgpl_dag_template child={.node_count=3,.node_fn=child_node};
    int parent=sgpl_current_thread_budget(), parent_lane=lane;
    if(autograph_execute_dag(&child,s,64)!=SGPL_DAG_OK||
       sgpl_current_thread_budget()!=parent||sgpl_current_worker_index()!=parent_lane)
      atomic_fetch_add(&s->errors,1);
    /* Even the force-parallel setting cannot create a nested team. */
    sgpl_push_thread_budget(1);
    parallel_for_runtime(0,3,1,nested_loop,s,0,0);
    sgpl_pop_thread_budget();
  } else {
    struct timespec delay={0,100000};
    nanosleep(&delay,NULL);
  }
  atomic_fetch_add(&s->calls,1);
  atomic_fetch_sub(&s->active,1);
  atomic_fetch_sub(&all_active,1);
  (void)node;
  return 0;
}
static sgpl_dag_template independent(int nodes) {
  sgpl_dag_template t={.node_count=nodes,.node_fn=pool_node};return t;
}
static pthread_t lanes[64];
static atomic_int lane_errors;
static int record_lanes;
static void lane_probe(void *unused,int32_t lane) {
  (void)unused;
  if(lane!=sgpl_current_worker_index()||!sgpl_is_worker_pool_thread())
    atomic_fetch_add(&lane_errors,1);
  if(record_lanes)lanes[lane]=pthread_self();
  else if(!pthread_equal(lanes[lane],pthread_self()))atomic_fetch_add(&lane_errors,1);
}
static void cleanup_check(void) {
  /* Registered before pool initialization. The pool's exit handler runs first. */
  if(atomic_load(&created)!=atomic_load(&joined)) {
    fprintf(stderr,"FAIL persistent workers were not joined at shutdown\n");
    _Exit(1);
  }
  puts("PASS shutdown joins every persistent worker");
}
static void *host_caller(void *state) {
  pool_fixture *s=state;sgpl_dag_template t=independent(8);
  for(int i=0;i<30;++i)
    if(autograph_execute_dag(&t,s,s->width)!=SGPL_DAG_OK)
      atomic_fetch_add(&s->errors,1);
  return NULL;
}
static int32_t cancellation_node(void *state,int64_t node) {
  pool_fixture *s=state;atomic_fetch_add(&s->visits[node],1);
  return node==0?1:0;
}
int main(int argc,char **argv) {
  int width=sgpl_configured_worker_count();
  if(width>64)return 2;
  atexit(cleanup_check);
  pool_fixture s={.width=width};sgpl_dag_template t=independent(8);
  if(argc>1) {
    if(!strcmp(argv[1],"allocation"))atomic_store(&fail_allocation,5);
    else atomic_store(&fail_create,!strcmp(argv[1],"last")?width-1:atoi(argv[1]));
    int rc=autograph_execute_dag(&t,&s,width);
    check("cold pool failure starts no user effects",rc<0&&atomic_load(&s.calls)==0);
    check("partial pool creation joins all created workers",atomic_load(&created)==atomic_load(&joined));
    atomic_store(&fail_allocation,0);atomic_store(&fail_create,0);
    check("later dispatch can reserve after a preparation failure",
          autograph_execute_dag(&t,&s,width)==0&&atomic_load(&s.calls)==8);
    return failures?1:0;
  }
  /* Serial calls require no team, even when thread creation would fail. */
  atomic_store(&fail_create,1);
  check("one worker runs without creating threads",autograph_execute_dag(&t,&s,1)==0&&
        atomic_load(&created)==0&&atomic_load(&fail_create)==1);
  atomic_store(&fail_create,0);
  if(width==1)return failures?1:0;
  check("configured team includes the caller",autograph_execute_dag(&t,&s,width)==0&&atomic_load(&created)==width-1);
  int created_before=atomic_load(&created);
  for(int i=0;i<250;++i)
    if(autograph_execute_dag(&t,&s,width)!=0)atomic_fetch_add(&s.errors,1);
  check("250 DAG dispatches reuse threads and execute every node once",
        atomic_load(&created)==created_before&&atomic_load(&joined)==0&&
        atomic_load(&s.calls)==252*8&&!atomic_load(&s.errors));
  record_lanes=1;sgpl_run_on_worker_pool(lane_probe,NULL,width);record_lanes=0;
  for(int i=0;i<20;++i)sgpl_run_on_worker_pool(lane_probe,NULL,width);
  check("worker threads retain stable lane identities",!atomic_load(&lane_errors));
  pool_fixture callers[4]={0};pthread_t hosts[4];atomic_store(&all_peak,0);
  for(int i=0;i<4;++i) {
    callers[i].width=width;
    if(__real_pthread_create(&hosts[i],NULL,host_caller,&callers[i]))return 2;
  }
  for(int i=0;i<4;++i) {
    __real_pthread_join(hosts[i],NULL);
    check("concurrent caller retains its state and exact node counts",
          atomic_load(&callers[i].calls)==240&&!atomic_load(&callers[i].errors));
  }
  check("concurrent callers share the configured worker limit",
        atomic_load(&all_peak)<=width&&atomic_load(&all_peak)>1&&
        atomic_load(&created)==created_before);
  setenv("SGPL_FORCE_DOALL_PARALLEL","1",1);
  pool_fixture loop={.width=width};
  parallel_for_runtime(0,16,1,root_loop,&loop,0,0);
  int complete_loop=1;
  for(int i=0;i<16;++i)if(atomic_load(&loop.visits[i])!=1)complete_loop=0;
  check("root loop and DAG execution share the same persistent threads",
        complete_loop&&!atomic_load(&loop.errors)&&atomic_load(&created)==created_before);
  pool_fixture nested={.width=width,.nested=1};
  check("nested DAG and loop dispatches complete without a second team",
        autograph_execute_dag(&t,&nested,width)==0&&!atomic_load(&nested.errors)&&
        atomic_load(&nested.calls)==8&&atomic_load(&created)==created_before);
  for(int i=0;i<3;++i)check("nested child and loop execute every member once",atomic_load(&nested.visits[i])==16);
  unsetenv("SGPL_FORCE_DOALL_PARALLEL");
  sgpl_dag_relation_desc edges[2]={
    {.source_node=0,.sink_node=1,.flags=SGPL_DAG_REL_FLAG_REALIZATION},
    {.source_node=1,.sink_node=2,.flags=SGPL_DAG_REL_FLAG_REALIZATION}};
  sgpl_dag_template failing={.node_count=3,.relation_count=2,.relations=edges,.node_fn=cancellation_node};
  pool_fixture failed={.width=width};
  check("failed run cancels successors without replay",
        autograph_execute_dag(&failing,&failed,width)==SGPL_DAG_ERR_NODE&&
        atomic_load(&failed.visits[0])==1&&!atomic_load(&failed.visits[1])&&!atomic_load(&failed.visits[2]));
  pool_fixture recovered={.width=width};
  check("pool accepts a fresh run after callback failure",
        autograph_execute_dag(&t,&recovered,width)==0&&atomic_load(&recovered.calls)==8);
  sgpl_push_thread_budget(1);
  pool_fixture limited={.width=width};
  check("parent grant limits a dispatch and remains balanced",
        autograph_execute_dag(&t,&limited,width)==0&&atomic_load(&limited.peak)==1&&sgpl_current_thread_budget()==1);
  sgpl_pop_thread_budget();
  check("caller budget and worker ledger are restored",!sgpl_current_thread_budget()&&!sgpl_debug_reserved_threads());
  printf("persistent worker pool: %d failures\n",failures);
  return failures?1:0;
}
