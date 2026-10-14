/* Execution storage only: leases never share buffers between live contexts.
 * Retain at most one 64 MiB arena per thread, released at thread exit. */
#ifndef SGPL_EXEC_REUSE_BUFFERS
#define SGPL_EXEC_REUSE_BUFFERS 1
#endif
#ifndef SGPL_EXEC_SKIP_FRESH_CLEAR
#define SGPL_EXEC_SKIP_FRESH_CLEAR 1
#endif
#ifndef SGPL_EXEC_CACHE_OWNERS
#define SGPL_EXEC_CACHE_OWNERS 1
#endif
#ifndef SGPL_EXEC_PAIR_BATCH
#define SGPL_EXEC_PAIR_BATCH 1
#endif
enum { SGPL_BUF_APPEND, SGPL_BUF_RECORDS, SGPL_BUF_SOURCES, SGPL_BUF_WORK,
       SGPL_BUF_EDGES, SGPL_BUF_OFFSETS, SGPL_BUF_SNAPSHOT };
typedef struct {
  void **buffers;
  size_t *capacities, count, total;
} SgplExecutionScratch;
static pthread_key_t sgpl_execution_scratch_key;
static pthread_once_t sgpl_execution_scratch_once=PTHREAD_ONCE_INIT;
static int sgpl_execution_scratch_key_status;
static void sgpl_execution_scratch_free(void *opaque) {
  SgplExecutionScratch *s=opaque;
  if(!s)return;
  for(size_t i=0;i<s->count;++i)free(s->buffers[i]);
  free(s->buffers);free(s->capacities);free(s);
}
static void sgpl_execution_scratch_key_init(void) {
  sgpl_execution_scratch_key_status=pthread_key_create(&sgpl_execution_scratch_key,sgpl_execution_scratch_free);
}
static void *sgpl_execution_scratch_acquire(void) {
  if(!SGPL_EXEC_REUSE_BUFFERS)return NULL;
  pthread_once(&sgpl_execution_scratch_once,sgpl_execution_scratch_key_init);
  if(sgpl_execution_scratch_key_status)return NULL;
  void *s=pthread_getspecific(sgpl_execution_scratch_key);
  if(s) {
    if(pthread_setspecific(sgpl_execution_scratch_key,NULL))return NULL;
    return s;
  }
  return calloc(1,sizeof(SgplExecutionScratch));
}
static void sgpl_execution_scratch_release(void *opaque,int cacheable) {
  SgplExecutionScratch *s=opaque;
  if(!s)return;
  if(!cacheable||s->total>64u*1024u*1024u||sgpl_execution_scratch_key_status||
      pthread_getspecific(sgpl_execution_scratch_key)||pthread_setspecific(sgpl_execution_scratch_key,s))
    sgpl_execution_scratch_free(s);
}
static void *sgpl_execution_buffer(sgpl_exec_ctx *c,size_t slot,size_t bytes,int clear) {
  SgplExecutionScratch *s=c->execution_scratch;
  if(!s)return clear?calloc(bytes?bytes:1,1):malloc(bytes?bytes:1);
  if(slot>=s->count) {
    if(slot==SIZE_MAX||slot+1>SIZE_MAX/sizeof(void *)||slot+1>SIZE_MAX/sizeof(size_t))return NULL;
    void **buffers=calloc(slot+1,sizeof(void *));
    size_t *capacities=calloc(slot+1,sizeof(size_t));
    if(!buffers||!capacities) {free(buffers);free(capacities);return NULL;}
    if(s->count) {
      memcpy(buffers,s->buffers,s->count*sizeof(void *));
      memcpy(capacities,s->capacities,s->count*sizeof(size_t));
    }
    free(s->buffers);free(s->capacities);
    s->buffers=buffers;s->capacities=capacities;s->count=slot+1;
  }
  size_t need=bytes?bytes:1;
  if(s->capacities[slot]<need) {
    if(need>SIZE_MAX-(s->total-s->capacities[slot]))return NULL;
    /* Allocate before releasing the previous storage: failure keeps the lease. */
    void *next=malloc(need);
    if(!next)return NULL;
    free(s->buffers[slot]);
    s->total-=s->capacities[slot];s->total+=need;
    s->buffers[slot]=next;s->capacities[slot]=need;
  }
  if(sgpl_gpu_host_access&&bytes)sgpl_gpu_host_access(s->buffers[slot],bytes,1);
  if(clear&&bytes)memset(s->buffers[slot],0,bytes);
  return s->buffers[slot];
}
/* Device grouping owns/reorders/frees record arrays. Transfer the active
 * buffers out of the lease before that original ownership path takes over. */
static void sgpl_execution_scratch_detach(sgpl_exec_ctx *c) {
  SgplExecutionScratch *s=c->execution_scratch;if(!s)return;
  void *active[]={c->array_append_log,c->gpu_records,c->gpu_record_sources,
                 c->gpu_record_work,c->array_record_edges,c->gpu_record_offsets};
  for(size_t i=0;i<s->count;++i) {
    void *owned=i<SGPL_BUF_SNAPSHOT?active[i]:
      (i-SGPL_BUF_SNAPSHOT<(size_t)c->spatial_snapshot_count?
       c->spatial_snapshots[i-SGPL_BUF_SNAPSHOT]:NULL);
    if(s->buffers[i]==owned)s->buffers[i]=NULL;
  }
  c->execution_scratch=NULL;sgpl_execution_scratch_free(s);
}

/* Partition ownership depends on the complete boundary values, not the
 * pointer's identity. This cache is read only during preparation. */
typedef struct {
  AutoGraphMeta *meta;
  int64_t n;
  int32_t count;
  int64_t *boundaries;
  int32_t *owners;
} SgplOwnerMap;
static pthread_key_t sgpl_owner_map_key;
static pthread_once_t sgpl_owner_map_once=PTHREAD_ONCE_INIT;
static int sgpl_owner_map_key_status;
static void sgpl_owner_map_free(void *opaque) {
  SgplOwnerMap *s=opaque;if(!s)return;
  free(s->boundaries);free(s->owners);free(s);
}
static void sgpl_owner_map_key_init(void) {
  sgpl_owner_map_key_status=pthread_key_create(&sgpl_owner_map_key,sgpl_owner_map_free);
}
static int sgpl_array_owner_map(AutoGraphMeta *m,const int32_t **out) {
  if(m->csr_n==0&&m->partition_count==0) {
    static const int32_t empty_owner=0;*out=&empty_owner;return 0;
  }
  if(m->csr_n<0||m->partition_count<=0||!m->partition_start||
      (uint64_t)m->csr_n>SIZE_MAX/sizeof(int32_t))return SGPL_DAG_ERR_INVALID;
  pthread_once(&sgpl_owner_map_once,sgpl_owner_map_key_init);
  if(sgpl_owner_map_key_status)return SGPL_DAG_ERR_NOMEM;
  SgplOwnerMap *s=pthread_getspecific(sgpl_owner_map_key);
  if(!s) {
    s=calloc(1,sizeof(*s));if(!s)return SGPL_DAG_ERR_NOMEM;
    if(pthread_setspecific(sgpl_owner_map_key,s)) {free(s);return SGPL_DAG_ERR_NOMEM;}
  }
  if(s->meta==m&&s->n==m->csr_n&&s->count==m->partition_count&&
      !memcmp(s->boundaries,m->partition_start,((size_t)m->partition_count+1)*8)) {
    *out=s->owners;return 0;
  }
  int32_t *owners=malloc((size_t)(m->csr_n?m->csr_n:1)*sizeof(int32_t));
  int64_t *bounds=malloc(((size_t)m->partition_count+1)*8);
  if(!owners||!bounds) {free(owners);free(bounds);return SGPL_DAG_ERR_NOMEM;}
  memcpy(bounds,m->partition_start,((size_t)m->partition_count+1)*8);
  if(bounds[0]!=0||bounds[m->partition_count]!=m->csr_n)goto invalid;
  for(int p=0;p<m->partition_count;++p) {
    if(bounds[p]<0||bounds[p+1]<bounds[p]||bounds[p+1]>m->csr_n)goto invalid;
    for(int64_t u=bounds[p];u<bounds[p+1];++u)owners[u]=p;
  }
  free(s->owners);free(s->boundaries);
  *s=(SgplOwnerMap){m,m->csr_n,m->partition_count,bounds,owners};
  *out=owners;return 0;
invalid:
  free(owners);free(bounds);return SGPL_DAG_ERR_INVALID;
}
