/* Complete native CleanCut storage certifies exclusive destination owners.
 * Scan each owner's source rows in their original order. Counting and filling
 * different owners touch disjoint destination cells, so no atomic edge counts
 * or worker-sized copies of the vertex array are needed. */
typedef struct {AutoGraphMeta *meta;SgplIncomingIndex *index;int64_t *cursor;_Atomic int error;} SgplIncomingBuild;
static SGPL_FULL_FN void sgpl_incoming_count_owned(void *opaque,int32_t lane,int32_t width) {
  SgplIncomingBuild *j=opaque;AutoGraphMeta *m=j->meta;SgplIncomingIndex *s=j->index;int32_t parts=m->partition_count;
  for(int64_t owner=lane;owner<parts;owner+=width) {
    int32_t p=(int32_t)owner;
    int64_t count=m->push_row_count[p];if(!count)continue;
    const int64_t *rp=m->push_rp[p];const int32_t *ci=m->push_ci[p],*sources=m->push_indir[p];
    int64_t lo=((int64_t)p*s->n+parts-1)/parts,hi=((int64_t)(p+1)*s->n+parts-1)/parts;
    int32_t previous=-1;int64_t edges=rp[count];
    for(int64_t r=0;r<count;++r) {
      int32_t u=sources[r];if(u<0||u>=s->n||u<=previous||rp[r]<0||rp[r+1]<rp[r]||rp[r+1]>edges){atomic_store(&j->error,1);return;}previous=u;
      for(int64_t e=rp[r];e<rp[r+1];++e) {
        int32_t v=ci[e];if(v<lo||v>=hi){atomic_store(&j->error,1);return;}
        ++s->incoming_rows[(int64_t)v+1];
      }
    }
  }
}
static SGPL_FULL_FN void sgpl_incoming_fill_owned(void *opaque,int32_t lane,int32_t width) {
  SgplIncomingBuild *j=opaque;AutoGraphMeta *m=j->meta;
  for(int64_t owner=lane;owner<m->partition_count;owner+=width) {
    int32_t p=(int32_t)owner;
    int64_t count=m->push_row_count[p];if(!count)continue;
    const int64_t *rp=m->push_rp[p];const int32_t *ci=m->push_ci[p],*sources=m->push_indir[p];
    for(int64_t r=0;r<count;++r)for(int64_t e=rp[r];e<rp[r+1];++e)
      j->index->sources[j->cursor[ci[e]]++]=sources[r];
  }
}
/* 0 retains the original constructor, -1 requests generic execution fallback. */
static SGPL_FULL_FN int sgpl_incoming_partitioned(AutoGraphMeta *m,SgplIncomingIndex *s,int64_t *cursor) {
  if(getenv("SGPL_FULL_INDEX_LEGACY")||__atomic_load_n(&m->cc_cache_valid,__ATOMIC_ACQUIRE)!=1||
     m->partition_count<1||!m->push_row_count||!m->push_rp||!m->push_ci||!m->push_indir)return 0;
  int64_t total=0;
  for(int32_t p=0;p<m->partition_count;++p) {
    int64_t count=m->push_row_count[p];if(count<0||count>s->n)return -1;
    const int64_t *rp=m->push_rp[p];
    if(!count){if(rp&&rp[0])return -1;continue;}
    if(!rp||!m->push_indir[p]||rp[0]||rp[count]<0||rp[count]>s->m-total||
       (rp[count]&&!m->push_ci[p]))return -1;
    total+=rp[count];
  }
  if(total!=s->m)return -1;
  int32_t width=sgpl_configured_worker_count(),budget=sgpl_current_thread_budget();
  if(budget>0&&width>budget)width=budget;
  if(sgpl_is_worker_pool_thread()||getenv("SGPL_FULL_INDEX_SERIAL"))width=1;
  int64_t useful=s->m/SGPL_CUT_PARALLEL_GRAIN;if(width>useful)width=(int32_t)useful;
  if(width>m->partition_count)width=m->partition_count;if(width<1)width=1;
  SgplIncomingBuild job={.meta=m,.index=s,.cursor=cursor};
  if(width>1)sgpl_run_preparation(sgpl_incoming_count_owned,&job,width);else sgpl_incoming_count_owned(&job,0,1);
  if(atomic_load(&job.error))return -1;
  for(int64_t v=0;v<s->n;++v){s->incoming_rows[v+1]+=s->incoming_rows[v];cursor[v]=s->incoming_rows[v];}
  if(s->incoming_rows[s->n]!=s->m)return -1;
  if(width>1)sgpl_run_preparation(sgpl_incoming_fill_owned,&job,width);else sgpl_incoming_fill_owned(&job,0,1);
  return 1;
}
