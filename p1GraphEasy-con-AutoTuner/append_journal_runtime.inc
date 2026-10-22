/* Protocol v3 splits the original eight-byte append allocation into values
 * indexed by logical key and private owner journals of successful keys.
 * An additional ordinal bitmap is reserved before lifecycle/user effects.
 * Included after the private full-graph/frontier executor declarations. */
#include "append_journal_plan.h"
#if defined(__GNUC__) && defined(__ELF__)
#define SGPL_JOURNAL_FN __attribute__((noinline,section(".sgpl_append_journal")))
#else
#define SGPL_JOURNAL_FN
#endif
typedef struct {
  int64_t first,capacity,used,read;
  char independent_counter_padding[32];
} SgplJournalOwner;
_Static_assert(sizeof(SgplJournalOwner)==64,"journal owner counters occupy one private cache line");
typedef struct {
  const sgpl_append_journal_plan_v1 *plan;
  SgplJournalOwner *owners;
  void *owner_allocation;
  int32_t *heap;
  uint64_t *bitmap;
  int64_t slots,words;
  int32_t parts,pair_slot,active,heap_merge,proven_append;
} SgplAppendJournal;
static void sgpl_journal_begin(sgpl_exec_ctx *);
static SGPL_JOURNAL_FN void sgpl_journal_release(void *opaque) {
  SgplAppendJournal *s=opaque;if(!s)return;free(s->owner_allocation);free(s->heap);free(s->bitmap);free(s);
}
SGPL_JOURNAL_FN void sgpl_exec_bind_append_journal_v1(sgpl_exec_ctx *c,const sgpl_append_journal_plan_v1 *p) {
  if(!c||!c->execution_scratch||!p||p->version!=SGPL_APPEND_JOURNAL_VERSION||p->bytes!=sizeof(*p)||
      (p->obligations!=5&&p->obligations!=(5|SGPL_JOURNAL_PROVEN_APPEND))||
      ((p->obligations&SGPL_JOURNAL_PROVEN_APPEND)?p->append_sites!=c->array_append_sites:p->append_sites!=0)||
      !p->pair||!p->journal_pair||!c->array_append_sites||
      c->spatial_group!=SGPL_SPATIAL_NONE||c->traversal_kind!=SGPL_TRAVERSE_OWNER_V)return;
  SgplExecutionScratch *scratch=c->execution_scratch;
  if(!scratch->fusion){
    scratch->fusion=calloc(1,sizeof(*scratch->fusion));if(!scratch->fusion)return;
    scratch->total+=sizeof(*scratch->fusion);
  }
  scratch->fusion->journal_plan=p;
  scratch->fusion->journal_begin=sgpl_journal_begin;
  scratch->fusion->journal_release=sgpl_journal_release;
}
static SGPL_JOURNAL_FN void *sgpl_journal_direct_kernel(sgpl_exec_ctx *c,int sparse,void *original) {
  SgplExecutionScratch *scratch=c->execution_scratch;
  SgplAppendJournal *s=scratch&&scratch->fusion?scratch->fusion->journal_state:NULL;
  if(!s||!s->active)return original;
  return sparse?s->plan->direct_sparse:s->plan->direct_dense;
}
static SGPL_JOURNAL_FN void sgpl_journal_begin(sgpl_exec_ctx *c) {
  SgplExecutionScratch *scratch=c->execution_scratch;
  SgplExecutionFusionLease *lease=scratch?scratch->fusion:NULL;if(!lease)return;
  SgplAppendJournal *old=lease->journal_state;
  if(old&&old->pair_slot>=0&&(unsigned)old->pair_slot<c->op_count&&
      (void *)c->ops[old->pair_slot].pair==old->plan->journal_pair)
    c->ops[old->pair_slot].pair=(void (*)(void *,sgpl_exec_ctx *,int32_t,int32_t))old->plan->pair;
  sgpl_journal_release(old);lease->journal_state=NULL;
  const sgpl_append_journal_plan_v1 *p=lease->journal_plan;AutoGraphMeta *m=find_meta(c->graph);
  SgplAppendJournal *s=NULL;
  if(!p||getenv("SGPL_NO_EFFECT_FUSION")||getenv("SGPL_NO_FUSION_C")||!m||m->partition_count<1||
      ((p->obligations&SGPL_JOURNAL_PROVEN_APPEND)&&p->append_sites!=c->array_append_sites)||
      c->spatial_group!=SGPL_SPATIAL_DESTINATION||c->spatial_runtime_group!=SGPL_SPATIAL_DESTINATION||
      !c->array_pair_records||c->gpu_group_offsets||!c->gpu_record_offsets||!c->array_append_log||
      c->array_work_count<=0||c->array_append_sites<1||c->initial_next_size<0||
      c->array_work_count>(INT32_MAX-c->initial_next_size)/c->array_append_sites||
      !c->array_output_slot||!c->array_output_capacity||
      *c->array_output_capacity-c->initial_next_size<c->array_work_count*c->array_append_sites)goto dense;
  int pair_slot=-1,pairs=0,gates=0;
  for(unsigned i=0;i<c->op_count;++i) {
    sgpl_runtime_op *op=&c->ops[i];
    if(op->capabilities&~(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL|SGPL_OP_SOURCE_BEGIN|SGPL_OP_SNAPSHOT)||
        op->round_begin||op->round_end||op->source_end||op->partition_begin||op->partition_end||op->combine)goto dense;
    if(op->pair){if((void *)op->pair!=p->pair)goto dense;pair_slot=(int)i;++pairs;}
    if(op->snapshot&&!(op->capabilities&SGPL_OP_SNAPSHOT))goto dense;
    if(op->source_begin){if((void *)op->source_begin!=p->source_gate)goto dense;++gates;}
  }
  if(pairs!=1||gates!=(p->source_gate?1:0))goto dense;
  s=calloc(1,sizeof(*s));if(!s)goto dense;
  s->plan=p;s->parts=m->partition_count;s->pair_slot=pair_slot;
  s->slots=c->array_work_count*c->array_append_sites;s->words=(s->slots+63)/64;
  s->heap_merge=getenv("SGPL_FUSION_C_HEAP_MERGE")!=NULL;
  /* This flag is meaningful only for compiler-certified bodies executed by
   * the runtime's record builders. Other plans keep every checked accessor. */
  s->proven_append=(p->obligations&SGPL_JOURNAL_PROVEN_APPEND)&&!getenv("SGPL_FUSION_C_CHECKED_APPEND");
  if((size_t)s->parts>(SIZE_MAX-63)/sizeof(SgplJournalOwner))goto dense;
  s->owner_allocation=calloc((size_t)s->parts*sizeof(SgplJournalOwner)+63,1);
  s->heap=malloc((size_t)s->parts*sizeof(*s->heap));
  if((uint64_t)s->words>SIZE_MAX/sizeof(*s->bitmap))goto dense;
  s->bitmap=malloc((size_t)s->words*sizeof(*s->bitmap));
  if(!s->owner_allocation||!s->heap||!s->bitmap)goto dense;
  memset(s->bitmap,0,(size_t)s->words*sizeof(*s->bitmap));
  s->owners=(SgplJournalOwner *)(((uintptr_t)s->owner_allocation+63)&~(uintptr_t)63);
  int64_t prefix=0;
  for(int32_t owner=0;owner<s->parts;++owner) {
    int64_t count=c->gpu_record_offsets[owner+1]-c->gpu_record_offsets[owner];
    if(c->array_compact_records==6) {
#ifdef SGPL_FRONTIER_FUSION_PLAN_H
      SgplFullState *full=sgpl_fullgraph_state(c);
      if(!sgpl_frontier_is_direct(full)||!p->direct_dense||!p->direct_sparse)goto dense;
      count=((SgplFrontierDirect *)full->values)->pair_counts[owner];
#else
      goto dense;
#endif
    }
    if(count<0||count>c->array_work_count||prefix>c->array_work_count*c->array_append_sites-count*c->array_append_sites)goto dense;
    s->owners[owner].first=prefix;s->owners[owner].capacity=count*c->array_append_sites;prefix+=count*c->array_append_sites;
  }
  if(prefix!=s->slots)goto dense;
  s->active=1;lease->journal_state=s;lease->append_was_journal=1;
  c->ops[pair_slot].pair=(void (*)(void *,sgpl_exec_ctx *,int32_t,int32_t))p->journal_pair;
  return;
dense:
  sgpl_journal_release(s);
  /* An ordinal from a previous sparse invocation must never masquerade as a
   * validity epoch when the same allocation returns to the dense executor. */
  if(lease->append_was_journal&&c->array_append_log) {
    size_t capacity=scratch->capacities[SGPL_BUF_APPEND];
    memset(c->array_append_log,0,capacity);lease->append_was_journal=0;
  }
}
SGPL_JOURNAL_FN int32_t sgpl_frontier_journal_activate_v1(sgpl_exec_ctx *c,int32_t v) {
  if(!c||v<0)return 0;
  SgplExecutionScratch *scratch=c->execution_scratch;
  SgplAppendJournal *s=scratch&&scratch->fusion?scratch->fusion->journal_state:NULL;
  if(!s||!s->active)return autograph_frontier_activate(c,v);
  SgplJournalOwner *owner;int64_t ordinal,at;
  if(s->proven_append) {
    /* Native owner loops supply a valid partition and monotonically ordered
     * work keys; they reset append_seq for each occurrence. The complete
     * acyclic use-chain proof bounds all appends by array_append_sites and
     * excludes context/log observers or mutation. The pre-effect reservation
     * therefore bounds owner->used as well. No supported error is removed. */
    owner=&s->owners[c->spatial_partition];
    ordinal=c->array_work_ordinal*c->array_append_sites+c->array_append_seq;
    at=owner->first+owner->used;
  } else {
    if(c->array_work_ordinal<0||c->array_work_ordinal>=c->array_work_count||
        c->array_append_seq>=c->array_append_sites||c->spatial_partition<0||c->spatial_partition>=s->parts)goto invalid;
    owner=&s->owners[c->spatial_partition];
    if(owner->used>=owner->capacity)goto invalid;
    ordinal=c->array_work_ordinal*c->array_append_sites+c->array_append_seq;
    at=owner->first+owner->used;
    if(owner->used&&c->array_append_log[s->slots+at-1]>=ordinal)goto invalid;
  }
  /* Original work keys are unique across owners. The value half therefore
   * has disjoint writes; the key half is disjoint by private owner capacity. */
  ++c->array_append_seq;++owner->used;
  c->array_append_log[ordinal]=v;c->array_append_log[s->slots+at]=(int32_t)ordinal;return 1;
invalid:
  if(c->execution_error)__atomic_store_n(c->execution_error,SGPL_DAG_ERR_INVALID,__ATOMIC_RELEASE);return 0;
}
static __attribute__((always_inline)) inline int sgpl_journal_less(sgpl_exec_ctx *c,SgplAppendJournal *s,int32_t a,int32_t b) {
  SgplJournalOwner *x=&s->owners[a],*y=&s->owners[b];
  int32_t i=c->array_append_log[s->slots+x->first+x->read],j=c->array_append_log[s->slots+y->first+y->read];
  return i<j||(i==j&&a<b);
}
static SGPL_JOURNAL_FN int sgpl_journal_compact(sgpl_exec_ctx *c,int32_t *result) {
  SgplExecutionScratch *scratch=c->execution_scratch;SgplAppendJournal *s=scratch&&scratch->fusion?scratch->fusion->journal_state:NULL;
  if(!s||!s->active)return 0;
  int32_t size=0;int64_t kept=c->initial_next_size;
  for(int32_t i=0;i<s->parts;++i){s->owners[i].read=0;kept+=s->owners[i].used;if(s->owners[i].used)s->heap[size++]=i;}
  if(kept>*c->array_output_capacity||kept>INT32_MAX){*result=SGPL_DAG_ERR_INVALID;return 1;}
  /* Mark only after the original join: no atomic bit updates or shared
   * producer writes are introduced. A bit identifies an append POSITION,
   * never its value. Equal values in distinct occurrences remain distinct.
   * Validate all keys before writing output; a contract failure never replays
   * partially executed effects or falls back after they have begun. */
  for(int32_t owner=0;owner<s->parts;++owner) {
    SgplJournalOwner *o=&s->owners[owner];
    for(int64_t i=0;i<o->used;++i) {
      int32_t key=c->array_append_log[s->slots+o->first+i];
      if(key<0||key>=s->slots){*result=SGPL_DAG_ERR_INVALID;return 1;}
      uint64_t bit=UINT64_C(1)<<((unsigned)key&63u);
      if(s->bitmap[(uint32_t)key>>6]&bit){*result=SGPL_DAG_ERR_INVALID;return 1;}
      s->bitmap[(uint32_t)key>>6]|=bit;
    }
  }
  c->next_frontier=*c->array_output_slot;
  if(sgpl_gpu_host_access&&kept)sgpl_gpu_host_access(c->next_frontier,(uint64_t)kept*4,1);
  int64_t at=c->initial_next_size;
  if(!s->heap_merge) {
    /* At most ceil(potential appends / 64) bitmap words, plus the successful
     * positions, replace per-entry heap selection. No new SIMD is used. */
    if(size)for(int64_t word=0;word<s->words;++word) {
      uint64_t bits=s->bitmap[word];
      while(bits) {
        unsigned bit=(unsigned)__builtin_ctzll(bits);
        c->next_frontier[at++]=c->array_append_log[64*word+bit];bits&=bits-1;
      }
    }
    if(c->append_head)*c->append_head=(int32_t)(at-c->initial_next_size);
    *result=0;return 1;
  }
  /* Same-format heap control isolates selection from production and bitmap
   * marking. It is diagnostic, and never selected by graph identity. */
  for(int32_t start=size/2;start>0;) {
    int32_t at=--start,value=s->heap[at];
    while(at<size/2) {
      int32_t child=2*at+1;if(child+1<size&&sgpl_journal_less(c,s,s->heap[child+1],s->heap[child]))++child;
      if(!sgpl_journal_less(c,s,s->heap[child],value))break;
      s->heap[at]=s->heap[child];at=child;
    }
    s->heap[at]=value;
  }
  while(size) {
    int32_t owner=s->heap[0];SgplJournalOwner *o=&s->owners[owner];
    int32_t key=c->array_append_log[s->slots+o->first+o->read++];
    c->next_frontier[at++]=c->array_append_log[key];
    int32_t value=owner;if(o->read==o->used)value=s->heap[--size];
    if(!size)break;
    int32_t pos=0;
    while(pos<size/2) {
      int32_t child=2*pos+1;if(child+1<size&&sgpl_journal_less(c,s,s->heap[child+1],s->heap[child]))++child;
      if(!sgpl_journal_less(c,s,s->heap[child],value))break;
      s->heap[pos]=s->heap[child];pos=child;
    }
    s->heap[pos]=value;
  }
  if(c->append_head)*c->append_head=(int32_t)(at-c->initial_next_size);
  *result=0;return 1;
}
