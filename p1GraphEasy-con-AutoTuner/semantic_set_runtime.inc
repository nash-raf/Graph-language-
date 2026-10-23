/* Per-round set equivalence, not weighted whole-loop convergence. Candidate
 * addition is modular and comparisons are signed, exactly as in the callback.
 * Captures precede every destination write; weight lookup retains first-match
 * duplicate-edge semantics. Only the independently proved R8 consumer permits
 * replacing the ordered notification trace by a changed-destination set. */
typedef struct {
  int busy,parts;uint32_t epoch;
  AutoGraphMeta *meta;const int32_t *input;const int64_t *weights;
  int32_t count;int64_t n;const sgpl_semantic_stage_plan_v1 *plan;
  int32_t *capture,*acc,*changed;uint32_t *tags;uint64_t *keys;
  size_t capture_bytes,acc_bytes,changed_bytes,tag_bytes,key_bytes;
  int64_t cells[SGPL_SEMANTIC_LANES*SGPL_SEMANTIC_LANES];
  int64_t offsets[SGPL_SEMANTIC_LANES+1],used[SGPL_SEMANTIC_LANES];
  int32_t *values;
} SgplSemanticSet;
static pthread_key_t sgpl_semantic_set_key;
static pthread_once_t sgpl_semantic_set_once=PTHREAD_ONCE_INIT;
static int sgpl_semantic_set_status;
static void sgpl_semantic_set_destroy(void *opaque) {
  SgplSemanticSet *s=opaque;if(!s)return;
  free(s->capture);free(s->acc);free(s->changed);free(s->tags);free(s->keys);free(s);
}
static void sgpl_semantic_set_init(void) {
  sgpl_semantic_set_status=pthread_key_create(&sgpl_semantic_set_key,sgpl_semantic_set_destroy);
}
static int sgpl_semantic_grow(void **p,size_t *capacity,size_t need) {
  if(need<=*capacity)return 1;
  void *q=malloc(need?need:1);if(!q)return 0;
  free(*p);*p=q;*capacity=need;return 1;
}
static inline __attribute__((always_inline)) void sgpl_semantic_set_prepare_impl(void *opaque,int32_t worker,int32_t width,int direct) {
  SgplSemanticSet *s=opaque;
  for(int part=worker;part<s->parts;part+=width) {
    int64_t *cells=s->cells+(size_t)part*s->parts;
    for(int64_t i=(int64_t)s->count*part/s->parts;i<(int64_t)s->count*(part+1)/s->parts;++i) {
      int32_t u=s->input[i],source=s->capture[i];
      for(int64_t e=s->meta->csr_row_ptr[u];e<s->meta->csr_row_ptr[u+1];++e) {
        int32_t v=s->meta->csr_col_idx[e],candidate=source;
        if(s->plan->weighted)candidate=(int32_t)((uint32_t)source+
            (uint32_t)(s->meta->csr_weights?s->meta->csr_weights[direct?e:s->weights[e]]:1));
        if(candidate<s->values[v])++cells[v%s->parts];
      }
    }
  }
}
static inline __attribute__((always_inline)) void sgpl_semantic_set_fill_impl(void *opaque,int32_t worker,int32_t width,int direct) {
  SgplSemanticSet *s=opaque;
  for(int part=worker;part<s->parts;part+=width) {
    int64_t *cells=s->cells+(size_t)part*s->parts;
    for(int64_t i=(int64_t)s->count*part/s->parts;i<(int64_t)s->count*(part+1)/s->parts;++i) {
      int32_t u=s->input[i],source=s->capture[i];
      for(int64_t e=s->meta->csr_row_ptr[u];e<s->meta->csr_row_ptr[u+1];++e) {
        int32_t v=s->meta->csr_col_idx[e],candidate=source;
        if(s->plan->weighted)candidate=(int32_t)((uint32_t)source+
            (uint32_t)(s->meta->csr_weights?s->meta->csr_weights[direct?e:s->weights[e]]:1));
        if(candidate<s->values[v])s->keys[cells[v%s->parts]++]=((uint64_t)(uint32_t)v<<32)|(uint32_t)candidate;
      }
    }
  }
}
static void sgpl_semantic_set_prepare(void *s,int32_t w,int32_t n){sgpl_semantic_set_prepare_impl(s,w,n,0);}
static void sgpl_semantic_set_prepare_direct(void *s,int32_t w,int32_t n){sgpl_semantic_set_prepare_impl(s,w,n,1);}
static void sgpl_semantic_set_fill(void *s,int32_t w,int32_t n){sgpl_semantic_set_fill_impl(s,w,n,0);}
static void sgpl_semantic_set_fill_direct(void *s,int32_t w,int32_t n){sgpl_semantic_set_fill_impl(s,w,n,1);}
static void sgpl_semantic_set_execute(void *opaque,int32_t worker,int32_t width) {
  SgplSemanticSet *s=opaque;int64_t stride=s->n/s->parts+1;
  for(int part=worker;part<s->parts;part+=width) {
    int64_t count=0;int32_t *changed=s->changed+part*stride;
    for(int64_t i=s->offsets[part];i<s->offsets[part+1];++i) {
      uint64_t key=s->keys[i];int32_t v=(int32_t)(key>>32),candidate=(int32_t)(uint32_t)key;
      if(s->tags[v]!=s->epoch) {s->tags[v]=s->epoch;s->acc[v]=candidate;changed[count++]=v;}
      else if(candidate<s->acc[v])s->acc[v]=candidate;
    }
    // All retained candidates improve the round-start target. One owned store
    // and one activation replace the unobserved intermediate minima.
    for(int64_t i=0;i<count;++i){int32_t v=changed[i];s->values[v]=s->acc[v];}
    s->used[part]=count;
  }
}
static int32_t sgpl_semantic_set_round(AutoGraphMeta *m,const sgpl_semantic_stage_plan_v1 *p,
    const int32_t *input,int32_t count,int32_t **output,int64_t *capacity,int32_t *values,int64_t pairs) {
  pthread_once(&sgpl_semantic_set_once,sgpl_semantic_set_init);
  if(sgpl_semantic_set_status)return -1;
  SgplSemanticSet *s=pthread_getspecific(sgpl_semantic_set_key);
  if(!s){s=calloc(1,sizeof(*s));if(!s)return -1;if(pthread_setspecific(sgpl_semantic_set_key,s)){free(s);return -1;}}
  if(s->busy)return -1;s->busy=1;int32_t result=-1;
  int parts=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
  if(budget>0&&parts>budget)parts=budget;if(parts<1||sgpl_is_worker_pool_thread())parts=1;
  if(parts>SGPL_SEMANTIC_LANES)parts=SGPL_SEMANTIC_LANES;
  const int64_t *weights=NULL;
  int allow_identity=(p->rules&SGPL_EXEC_R15)&&!getenv("SGPL_NO_PROOF_REMOVALS");
  if(p->weighted&&sgpl_array_weight_view(m,&weights,allow_identity)<0)goto done;
  int direct=p->weighted&&!weights;
  if((uint64_t)count>SIZE_MAX/4||(uint64_t)m->csr_n>SIZE_MAX/4-(unsigned)parts)goto done;
  size_t old_tags=s->tag_bytes;
  if(!sgpl_semantic_grow((void **)&s->capture,&s->capture_bytes,(size_t)count*4)||
     !sgpl_semantic_grow((void **)&s->acc,&s->acc_bytes,(size_t)m->csr_n*4)||
     !sgpl_semantic_grow((void **)&s->changed,&s->changed_bytes,((size_t)m->csr_n+parts)*4)||
     !sgpl_semantic_grow((void **)&s->tags,&s->tag_bytes,(size_t)m->csr_n*4))goto done;
  if(old_tags!=s->tag_bytes||++s->epoch==0){memset(s->tags,0,s->tag_bytes);s->epoch=1;}
  s->parts=parts;s->meta=m;s->input=input;s->weights=weights;s->count=count;s->n=m->csr_n;s->plan=p;s->values=values;
  memset(s->cells,0,sizeof(s->cells));memset(s->used,0,sizeof(s->used));
  for(int32_t i=0;i<count;++i)s->capture[i]=values[input[i]];
  sgpl_run_preparation(direct?sgpl_semantic_set_prepare_direct:sgpl_semantic_set_prepare,s,parts);
  int64_t at=0;
  for(int part=0;part<parts;++part){s->offsets[part]=at;for(int chunk=0;chunk<parts;++chunk){int64_t *c=&s->cells[chunk*parts+part],size=*c;*c=at;at+=size;}}
  s->offsets[parts]=at;
  if((uint64_t)at>SIZE_MAX/8||!sgpl_semantic_grow((void **)&s->keys,&s->key_bytes,(size_t)at*8))goto done;
  // R14 supplies the vertex bound instead of a degree sum. The retained
  // candidate count is already available, and is an independent upper bound
  // on emitted IDs. Intersect the bounds; sparse rounds need no n-sized reserve.
  int bounded=(p->rules&SGPL_EXEC_R14)&&!getenv("SGPL_NO_PROOF_REMOVALS");
  int64_t reserve_bound=bounded&&at<pairs?at:pairs;
  if(sgpl_frontier_resize(output,capacity,reserve_bound,0)<0)goto done;
  sgpl_run_preparation(direct?sgpl_semantic_set_fill_direct:sgpl_semantic_set_fill,s,parts);
  sgpl_run_preparation(sgpl_semantic_set_execute,s,parts);
  result=0;int64_t stride=s->n/parts+1;
  for(int part=0;part<parts;++part)if(s->used[part]){memcpy(*output+result,s->changed+part*stride,(size_t)s->used[part]*4);result+=(int32_t)s->used[part];}
  if(getenv("SGPL_SEMANTIC_STAGE_STATS"))fprintf(stderr,"[iteration-memory] R8 set reserve_bound=%lld candidates=%lld changed=%d capture=%d\n",(long long)reserve_bound,(long long)at,result,count);
  if(getenv("SGPL_SEMANTIC_STAGE_STATS")&&allow_identity)fprintf(stderr,"[proof-removals] R15 %s: %s\n",direct?"activate":"refuse",direct?"first-match identity; direct weight workers":"duplicate neighbors; retain first-match map");
done:s->busy=0;return result;
}
