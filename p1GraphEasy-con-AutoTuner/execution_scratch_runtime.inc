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
       SGPL_BUF_EDGES, SGPL_BUF_OFFSETS, SGPL_BUF_SOURCE_GATE, SGPL_BUF_GROUPS,
       SGPL_BUF_PREP_POSITIONS, SGPL_BUF_PREP_WORK, SGPL_BUF_SNAPSHOT };
typedef struct {
  void **buffers;
  size_t *capacities, count, total;
  uint32_t append_epoch;
  uint32_t *group_tags;
  size_t group_capacity;
  uint32_t group_epoch;
} SgplExecutionScratch;
static pthread_key_t sgpl_execution_scratch_key;
static pthread_once_t sgpl_execution_scratch_once=PTHREAD_ONCE_INIT;
static int sgpl_execution_scratch_key_status;
static void sgpl_execution_scratch_free(void *opaque) {
  SgplExecutionScratch *s=opaque;
  if(!s)return;
  for(size_t i=0;i<s->count;++i)free(s->buffers[i]);
  free(s->group_tags);free(s->buffers);free(s->capacities);free(s);
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
    /* Fresh validity tags must be initialized once; calloc can keep unused
     * pages demand-zero. Later invocations use an exclusive lease epoch. */
    void *next=slot==SGPL_BUF_APPEND?calloc(need,1):malloc(need);
    if(!next)return NULL;
    free(s->buffers[slot]);
    s->total-=s->capacities[slot];s->total+=need;
    s->buffers[slot]=next;s->capacities[slot]=need;
    /* Consumers initialize their live domain: sparse CPU batches clear
     * referenced endpoints; full-domain and device paths clear the full view. */
  }
  if(sgpl_gpu_host_access&&bytes)sgpl_gpu_host_access(s->buffers[slot],bytes,1);
  if(clear&&bytes&&slot!=SGPL_BUF_APPEND)memset(s->buffers[slot],0,bytes);
  return s->buffers[slot];
}
/* Validity is per invocation, never per worker. Each live context owns its
 * lease, and the dispatch joins before its epoch can advance. On wrap, clear
 * the full retained capacity so a later larger view cannot revive old tags. */
#ifndef SGPL_EXEC_APPEND_EPOCH_MAX
#define SGPL_EXEC_APPEND_EPOCH_MAX UINT32_MAX
#endif
static uint32_t sgpl_array_append_epoch(const sgpl_exec_ctx *c) {
  SgplExecutionScratch *s=c->execution_scratch;
  return s?s->append_epoch:1;
}
static void sgpl_array_begin_append(sgpl_exec_ctx *c) {
  SgplExecutionScratch *s=c->execution_scratch;
  if(s) {
    if(s->append_epoch>=SGPL_EXEC_APPEND_EPOCH_MAX) {
      size_t capacity=s->capacities[SGPL_BUF_APPEND];
      if(sgpl_gpu_host_access)sgpl_gpu_host_access(c->array_append_log,capacity,1);
      memset(c->array_append_log,0,capacity);s->append_epoch=0;
    }
    ++s->append_epoch;
    if(!SGPL_EXEC_SKIP_FRESH_CLEAR)
      memset(c->array_append_log,0,(size_t)c->array_work_count*c->array_append_sites*8);
  } else if(!SGPL_EXEC_SKIP_FRESH_CLEAR||!c->array_log_pristine)
    memset(c->array_append_log,0,(size_t)c->array_work_count*c->array_append_sites*8);
  c->array_log_pristine=0;
}
/* Device grouping owns/reorders/frees record arrays. Transfer the active
 * buffers out of the lease before that original ownership path takes over. */
static void sgpl_execution_scratch_detach(sgpl_exec_ctx *c) {
  SgplExecutionScratch *s=c->execution_scratch;if(!s)return;
  void *active[]={c->array_append_log,c->gpu_records,c->gpu_record_sources,
                 c->gpu_record_work,c->array_record_edges,c->gpu_record_offsets,c->spatial_source_gate,
                 c->spatial_groups_seen,NULL,NULL};
  for(size_t i=0;i<s->count;++i) {
    void *owned=i<SGPL_BUF_SNAPSHOT?active[i]:
      (i-SGPL_BUF_SNAPSHOT<(size_t)c->spatial_snapshot_count?
       c->spatial_snapshots[i-SGPL_BUF_SNAPSHOT]:NULL);
    if(s->buffers[i]==owned)s->buffers[i]=NULL;
  }
  /* Device kernels retain the original 0/1 flag ABI. Force a clean log at
   * the joined begin boundary before any device or CPU-fallback dispatch. */
  c->array_log_pristine=0;
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

#ifndef SGPL_GROUP_EPOCH_MAX
#define SGPL_GROUP_EPOCH_MAX UINT32_MAX
#endif
/* The exclusive lease survives sequential contexts. Group ownership ensures
 * each endpoint is marked by exactly one partition on the certified CPU path. */
static int sgpl_group_epoch_begin(sgpl_exec_ctx *c,AutoGraphMeta *m) {
  SgplExecutionScratch *s=c->execution_scratch;
  if(!s||!m||m->csr_n<0||(uint64_t)m->csr_n>SIZE_MAX/4)return 0;
  size_t need=(size_t)m->csr_n;
  if(s->group_capacity<need) {
    uint32_t *next=calloc(need?need:1,sizeof(*next));
    if(!next)return 0;
    free(s->group_tags);s->total-=s->group_capacity*4;
    s->group_tags=next;s->group_capacity=need;s->total+=need*4;s->group_epoch=0;
  }
  if(s->group_epoch>=SGPL_GROUP_EPOCH_MAX) {
    memset(s->group_tags,0,s->group_capacity*4);s->group_epoch=0;
  }
  ++s->group_epoch;return 1;
}
