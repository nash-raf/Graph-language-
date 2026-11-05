/* Private preparation storage in the existing thread-owned owner-map cache.
 * Templates are copied into each context: live contexts never share arrays. */
typedef struct {
  AutoGraphMeta *meta;uint64_t generation;
  int32_t layout_epoch,layout,partitions,traversal,group,begins,ends,parallel,local;
  int64_t n,m,records,groups;
  int64_t *bounds,*record_offsets,*group_offsets,*partition_groups;
  int32_t *records_data;
  size_t template_bytes,capacity;
  uint64_t hits;
  int64_t *counts,*positions;int32_t *keys;
} SgplGpuPreparation;
static int sgpl_gpu_records_reuse_enabled(void) {
  const char *v=getenv("SGPL_GPU_REUSE_RECORDS");return !v||strcmp(v,"0");
}
static void sgpl_gpu_template_clear(SgplGpuPreparation *s) {
  free(s->bounds);free(s->record_offsets);free(s->group_offsets);free(s->partition_groups);free(s->records_data);
  s->bounds=NULL;s->record_offsets=NULL;s->group_offsets=NULL;s->partition_groups=NULL;s->records_data=NULL;
  s->meta=NULL;s->template_bytes=0;
}
static void sgpl_gpu_preparation_free(void *opaque) {
  SgplGpuPreparation *s=opaque;if(!s)return;
  sgpl_gpu_template_clear(s);free(s->counts);free(s->positions);free(s->keys);free(s);
}
static SgplGpuPreparation *sgpl_gpu_preparation_cache(void) {
  if(!sgpl_gpu_records_reuse_enabled())return NULL;
  pthread_once(&sgpl_owner_map_once,sgpl_owner_map_key_init);
  if(sgpl_owner_map_key_status)return NULL;
  SgplOwnerMap *owner=pthread_getspecific(sgpl_owner_map_key);
  if(!owner) {
    owner=calloc(1,sizeof(*owner));if(!owner)return NULL;
    if(pthread_setspecific(sgpl_owner_map_key,owner)){free(owner);return NULL;}
  }
  if(!owner->gpu_preparation) {
    owner->gpu_preparation=calloc(1,sizeof(SgplGpuPreparation));
    owner->gpu_preparation_free=sgpl_gpu_preparation_free;
  }
  return owner->gpu_preparation;
}
static int sgpl_gpu_template_eligible(const sgpl_exec_ctx *c,const AutoGraphMeta *m) {
  return c&&!c->array_append_sites&&c->domain_kind==SGPL_DOMAIN_ALL_VERTICES&&!c->membership&&
    m&&m->partition_count>0&&m->partition_start&&m->cc_cache_valid==1&&!getenv("SGPL_GPU_GROUP_TRACE");
}
static int sgpl_gpu_template_load(SgplGpuPreparation *s,sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {
  if(!s||!sgpl_gpu_template_eligible(c,m)||c->gpu_records||c->gpu_record_offsets||
      s->meta!=m||s->generation!=atomic_load_explicit(&g_cc_topology_generation,memory_order_relaxed)||
      s->layout_epoch!=m->layout_epoch||s->layout!=m->current_layout||s->n!=m->csr_n||s->m!=m->csr_m||
      s->partitions!=m->partition_count||s->traversal!=c->traversal_kind||s->group!=c->spatial_group||
      s->begins!=begins||s->ends!=ends||s->parallel!=c->gpu_parallel_groups||s->local!=c->gpu_local_partials||
      memcmp(s->bounds,m->partition_start,((size_t)m->partition_count+1)*8))return 0;
  size_t parts=((size_t)s->partitions+1)*8,records=(size_t)s->records*12,groups=((size_t)s->groups+1)*8;
  int32_t *r=malloc(records?records:4);int64_t *ro=malloc(parts),*go=malloc(groups),*pg=malloc(parts);
  if(!r||!ro||!go||!pg){free(r);free(ro);free(go);free(pg);return 0;}
  if(records)memcpy(r,s->records_data,records);memcpy(ro,s->record_offsets,parts);
  memcpy(go,s->group_offsets,groups);memcpy(pg,s->partition_groups,parts);
  c->gpu_records=r;c->gpu_record_offsets=ro;c->gpu_group_offsets=go;c->gpu_partition_groups=pg;
  c->gpu_record_count=s->records;c->gpu_group_count=s->groups;
  c->gpu_partition_count=s->partitions;c->gpu_vertex_count=s->n;
  ++s->hits;
  if(getenv("SGPL_GPU_PREPARATION_STATS"))fprintf(stderr,"[gpu-record-reuse] hit records=%lld\n",(long long)s->records);
  return 1;
}
static void sgpl_gpu_template_store(SgplGpuPreparation *s,const sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {
  if(!s||!sgpl_gpu_template_eligible(c,m)||c->gpu_record_count<0||c->gpu_group_count<0)return;
  uint64_t records=(uint64_t)c->gpu_record_count,groups=(uint64_t)c->gpu_group_count,parts=(uint64_t)m->partition_count+1;
  if(records>64u*1024*1024/12||groups>64u*1024*1024/8||parts>64u*1024*1024/24)return;
  size_t bytes=records*12+(groups+1)*8+parts*24;
  if(bytes>64u*1024*1024||s->capacity>(64u*1024*1024-bytes)/20)return;
  int32_t *r=malloc(records?records*12:4);int64_t *ro=malloc(parts*8),*go=malloc((groups+1)*8),*pg=malloc(parts*8),*bounds=malloc(parts*8);
  if(!r||!ro||!go||!pg||!bounds){free(r);free(ro);free(go);free(pg);free(bounds);return;}
  if(records)memcpy(r,c->gpu_records,records*12);memcpy(ro,c->gpu_record_offsets,parts*8);
  memcpy(go,c->gpu_group_offsets,(groups+1)*8);memcpy(pg,c->gpu_partition_groups,parts*8);memcpy(bounds,m->partition_start,parts*8);
  sgpl_gpu_template_clear(s);s->records_data=r;s->record_offsets=ro;s->group_offsets=go;s->partition_groups=pg;s->bounds=bounds;
  s->meta=m;s->generation=atomic_load_explicit(&g_cc_topology_generation,memory_order_relaxed);
  s->layout_epoch=m->layout_epoch;s->layout=m->current_layout;s->n=m->csr_n;s->m=m->csr_m;s->partitions=m->partition_count;
  s->traversal=c->traversal_kind;s->group=c->spatial_group;s->begins=begins;s->ends=ends;
  s->parallel=c->gpu_parallel_groups;s->local=c->gpu_local_partials;s->records=records;s->groups=groups;s->template_bytes=bytes;
}
static int sgpl_gpu_group_scratch(SgplGpuPreparation *s,size_t n) {
  if(!s||n>64u*1024*1024/20)return 0;
  if(n>s->capacity) {
    int64_t *counts=calloc(n?n:1,8),*positions=malloc((n?n:1)*8);int32_t *keys=malloc((n?n:1)*4);
    if(!counts||!positions||!keys){free(counts);free(positions);free(keys);return 0;}
    if(s->template_bytes>64u*1024*1024-n*20)sgpl_gpu_template_clear(s);
    free(s->counts);free(s->positions);free(s->keys);s->counts=counts;s->positions=positions;s->keys=keys;s->capacity=n;
  }
  if(n)memset(s->counts,0,n*8);return 1;
}
