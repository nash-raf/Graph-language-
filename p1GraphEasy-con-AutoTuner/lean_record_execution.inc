#undef sgpl_exec_partition_body
#undef sgpl_exec_partition_body_packed
#include "execution_partition_bodies.inc"

/* Compiler-certified pair-only layouts. Select formats before entering loops;
 * retain the original failure, group accounting, append and budget contracts. */
#if defined(__GNUC__) && defined(__ELF__)
#define SGPL_LEAN_FN __attribute__((noinline,section(".sgpl_lean_records")))
#else
#define SGPL_LEAN_FN
#endif
#define SGPL_LEAN_LOOP(MARK) do { \
  for(int64_t i=first;i<last;++i) { \
    if(sgpl_exec_failed(&local))return; \
    int32_t u=records[STRIDE*i],v=records[STRIDE*i+1]; \
    local.array_work_ordinal=records[STRIDE*i+2];local.array_append_seq=0; \
    local.array_edge_index=WEIGHTED?records[STRIDE*i+3]:(edges?edges[i]:-1); \
    MARK;PairFn(PairState,&local,u,v); \
  } \
}while(0)
#define SGPL_DEFINE_LEAN(NAME,STRIDE_ARG,WEIGHTED_ARG) \
static SGPL_LEAN_FN void NAME(int64_t index,void *opaque) { \
  enum {STRIDE=STRIDE_ARG,WEIGHTED=WEIGHTED_ARG}; \
  sgpl_exec_ctx *ctx=opaque,local;sgpl_exec_context_copy(&local,ctx);AutoGraphMeta *meta=find_meta(ctx->graph); \
  if(!meta||sgpl_exec_failed(&local))return; \
  int32_t p=(int32_t)index;local.spatial_partition=p; \
  if(local.spatial_lane_seen)local.dest_seen=local.spatial_lane_seen+(int64_t)p*meta->csr_n; \
  local.partition_state=local.partition_base&&local.partition_stride>0? \
    (char *)local.partition_base+(int64_t)p*local.partition_stride:local.partition_base; \
  void (*PairFn)(void *,sgpl_exec_ctx *,int32_t,int32_t)=local.ops[0].pair; \
  void *PairState=local.ops[0].state;const int32_t *records=ctx->gpu_records; \
  const int64_t *edges=ctx->gpu_group_offsets?NULL:ctx->array_record_edges; \
  uint8_t *seen=local.spatial_groups_seen;int sparse_groups=sgpl_sparse_group_tracking(ctx); \
  uint64_t groups_local=0;SgplExecutionScratch *group_scratch=ctx->execution_scratch; \
  uint32_t *group_tags=sparse_groups&&group_scratch&&group_scratch->group_capacity>=(size_t)meta->csr_n?group_scratch->group_tags:NULL; \
  uint32_t group_epoch=group_tags?group_scratch->group_epoch:0; \
  int64_t first=ctx->gpu_record_offsets[p],last=ctx->gpu_record_offsets[p+1]; \
  if(local.spatial_group==SGPL_SPATIAL_PAIR_OCCURRENCE) \
    SGPL_LEAN_LOOP(__atomic_fetch_add(&ctx->spatial_occupied_groups,1,__ATOMIC_RELAXED)); \
  else if(seen&&local.spatial_group==SGPL_SPATIAL_SOURCE) \
    SGPL_LEAN_LOOP(if(group_tags){if(group_tags[u]!=group_epoch){group_tags[u]=group_epoch;seen[u]=1;++groups_local;}}else if(sparse_groups){if(!seen[u]){seen[u]=1;++groups_local;}}else seen[u]=1); \
  else if(seen) \
    SGPL_LEAN_LOOP(if(group_tags){if(group_tags[v]!=group_epoch){group_tags[v]=group_epoch;seen[v]=1;++groups_local;}}else if(sparse_groups){if(!seen[v]){seen[v]=1;++groups_local;}}else seen[v]=1); \
  else SGPL_LEAN_LOOP((void)0); \
  if(groups_local)__atomic_fetch_add(&ctx->spatial_occupied_groups,groups_local,__ATOMIC_RELAXED); \
}
SGPL_DEFINE_LEAN(sgpl_exec_partition_body_lean,3,0)
SGPL_DEFINE_LEAN(sgpl_exec_partition_body_lean_weighted,4,1)
#undef SGPL_DEFINE_LEAN
#undef SGPL_LEAN_LOOP
