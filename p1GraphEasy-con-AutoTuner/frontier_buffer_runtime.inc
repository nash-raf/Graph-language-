/* Capacity is infrastructure. Payload order and duplicate occurrences stay
 * unchanged. A buffer can start on the stack and later move to owned storage. */
typedef struct SgplFrontierAllocation {
  int32_t *data;
  int owned;
  struct SgplFrontierAllocation *next;
} SgplFrontierAllocation;
static _Thread_local SgplFrontierAllocation *sgpl_frontier_allocations;
extern void sgpl_gpu_host_access(void *, uint64_t, int32_t) __attribute__((weak));
extern void sgpl_gpu_unregister_buffer(void *) __attribute__((weak));
extern void sgpl_gpu_register_resident_buffer(void *, uint64_t) __attribute__((weak));

static int sgpl_frontier_allocation_owned(void *data) {
  for (SgplFrontierAllocation *a=sgpl_frontier_allocations;a;a=a->next)
    if(a->data==data) return a->owned;
  return 0;
}
static int32_t sgpl_frontier_resize(int32_t **slot,int64_t *capacity,int64_t need,int fallback) {
  if(!slot||!capacity||*capacity<0||need<0||need>INT32_MAX||
     (uint64_t)need>SIZE_MAX/sizeof(int32_t)) return SGPL_DAG_ERR_INVALID;
  if(need<=*capacity) return 0;
  int64_t grown=need;
  if(fallback && *capacity>0 && *capacity<=INT32_MAX/2 && grown<*capacity*2) grown=*capacity*2;
  const char *fault=getenv("SGPL_FRONTIER_FAIL");
  if(fault && (!strcmp(fault,"alloc")||(!strcmp(fault,"grow")&&fallback))) return SGPL_DAG_ERR_NOMEM;
  int32_t *next=calloc((size_t)grown,sizeof(int32_t));
  SgplFrontierAllocation *record=malloc(sizeof(*record));
  if(!next||!record) {free(next);free(record);return SGPL_DAG_ERR_NOMEM;}
  int32_t *old=*slot;
  if(old && *capacity) {
    if(sgpl_gpu_host_access) sgpl_gpu_host_access(old,(uint64_t)*capacity*4,1);
    memcpy(next,old,(size_t)*capacity*4);
  }
  if(old && sgpl_gpu_unregister_buffer) sgpl_gpu_unregister_buffer(old);
  if(sgpl_frontier_allocation_owned(old)) {
    for(SgplFrontierAllocation *a=sgpl_frontier_allocations;a;a=a->next)
      if(a->data==old) {a->data=NULL;break;}
    free(old);
  }
  *record=(SgplFrontierAllocation){next,1,sgpl_frontier_allocations};
  sgpl_frontier_allocations=record;*slot=next;*capacity=grown;
  if(sgpl_gpu_register_resident_buffer) sgpl_gpu_register_resident_buffer(next,(uint64_t)grown*4);
  if(getenv("SGPL_FRONTIER_DEBUG")) fprintf(stderr,"[frontier-buffer] %s capacity=%lld required=%lld\n",
      fallback?"fallback-grow":"reserve",(long long)grown,(long long)need);
  return 0;
}
int32_t sgpl_frontier_ensure_i32(int32_t **slot,int64_t *capacity,int64_t index) {
  if(index<0||index>=INT32_MAX) return SGPL_DAG_ERR_INVALID;
  return sgpl_frontier_resize(slot,capacity,index+1,1);
}
int32_t sgpl_frontier_reserve_i32(void *graph,const int32_t *input,int32_t count,
                                 int32_t **slot,int64_t *capacity,int32_t sites,int32_t prefix) {
  if(!graph||count<0||sites<1||prefix<0||!capacity||prefix>*capacity||(!input&&count)) return SGPL_DAG_ERR_INVALID;
  int64_t n=*(int64_t*)graph,work=prefix;
  for(int32_t i=0;i<count;i++) {
    if(input[i]<0||input[i]>=n) return SGPL_DAG_ERR_INVALID;
    AutoNeighborIter iter;int32_t v;
    autograph_neighbor_iter_init(graph,input[i],&iter);
    while(autograph_neighbor_iter_next(&iter,&v)) {
      if(work>INT32_MAX-sites) return SGPL_DAG_ERR_INVALID;
      work+=sites;
    }
  }
  /* Fault injection changes reservation only. Append guards remain enabled. */
  if(getenv("SGPL_FRONTIER_TEST_UNDERRESERVE")) work=0;
  return sgpl_frontier_resize(slot,capacity,work,0);
}
void sgpl_frontier_release_buffers(void) {
  while(sgpl_frontier_allocations) {
    SgplFrontierAllocation *a=sgpl_frontier_allocations;
    sgpl_frontier_allocations=a->next;
    if(a->data) {
      if(sgpl_gpu_unregister_buffer) sgpl_gpu_unregister_buffer(a->data);
      if(a->owned) free(a->data);
    }
    free(a);
  }
}
