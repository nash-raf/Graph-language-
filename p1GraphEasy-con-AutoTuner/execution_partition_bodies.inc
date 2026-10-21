/* Existing execution bodies extracted from frozen daa63293 autotuner_runtime.c.
 * Only shared-context snapshot reads differ. Legacy definitions stay private
 * and unused so the protected translation unit remains byte-for-byte frozen. */
static void sgpl_exec_partition_body(int64_t index, void *opaque) {
  sgpl_exec_ctx *ctx = (sgpl_exec_ctx *)opaque;
  sgpl_exec_ctx local;sgpl_exec_context_copy(&local,ctx); /* worker-local state */
  AutoGraphMeta *meta = find_meta(ctx->graph);
  int32_t p = (int32_t)index;
  local.spatial_partition = p;
  if (local.spatial_lane_seen && meta)
    local.dest_seen = local.spatial_lane_seen + (int64_t)p * meta->csr_n;
  if (local.execution_error && __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0)
    return;
  /* Hoist the single pair operation's callback and state: compiler-emitted
   * contexts carry exactly one pair op, and holding them in locals keeps the
   * per-pair path at one indirect call with no descriptor reloads. */
  void (*PairFn)(void *, sgpl_exec_ctx *, int32_t, int32_t) = NULL;
  void *PairState = NULL;

  local.partition_state =
      (local.partition_base && local.partition_stride > 0)
          ? (char *)local.partition_base + (int64_t)p * local.partition_stride
          : local.partition_base;

  sgpl_exec_partition_begin_ops(&local, p);

  if (local.op_count == 1 && (local.ops[0].capabilities & SGPL_OP_PAIR)) {
    PairFn = local.ops[0].pair;
    PairState = local.ops[0].state;
  }
#define SGPL_EXEC_EMIT_PAIR(U, V)                                             \
  do {                                                                        \
    if (local.execution_error &&                                              \
        __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0)          \
      return;                                                                 \
    if (local.spatial_group == SGPL_SPATIAL_PAIR_OCCURRENCE)                   \
      __atomic_fetch_add(&ctx->spatial_occupied_groups, 1, __ATOMIC_RELAXED);   \
    else if (local.spatial_groups_seen)                                       \
      local.spatial_groups_seen[local.spatial_group == SGPL_SPATIAL_SOURCE    \
          ? (U) : (V)] = 1;                                                  \
    if (PairFn)                                                               \
      PairFn(PairState, &local, (U), (V));                                    \
    else                                                                      \
      sgpl_exec_pair_ops(&local, (U), (V));                                   \
  } while (0)

  if (local.array_append_sites && SGPL_EXEC_PAIR_BATCH && local.array_pair_records && PairFn) {
    /* Preparation certifies that this batch contains pairs only. Select the
     * grouping action once, retain the occurrence gate and cancellation at
     * each callback, and leave lifecycle-bearing batches on the general path. */
    const int32_t *records=ctx->gpu_records;
    const int64_t *sources=ctx->gpu_record_sources,*work=ctx->gpu_record_work;
    const int64_t *edges=ctx->gpu_group_offsets?NULL:ctx->array_record_edges;
    const uint8_t *gate=local.spatial_source_gate;
    uint8_t *seen=local.spatial_groups_seen;
    int sparse_groups=sgpl_sparse_group_tracking(ctx);uint64_t groups_local=0;
    SgplExecutionScratch *group_scratch=ctx->execution_scratch;
    uint32_t *group_tags=sparse_groups&&group_scratch&&group_scratch->group_capacity>=(size_t)meta->csr_n?group_scratch->group_tags:NULL;
    uint32_t group_epoch=group_tags?group_scratch->group_epoch:0;
    int64_t first=ctx->gpu_record_offsets[p],last=ctx->gpu_record_offsets[p+1];
#define SGPL_PAIR_BATCH(MARK) do {                                           \
    for(int64_t i=first;i<last;++i) {                                        \
      if(sgpl_exec_failed(&local))return;                                   \
      int stride=ctx->array_compact_records?4:3;                            \
      int32_t u=records[stride*i],v=records[stride*i+1];                     \
      local.array_source_ordinal=ctx->array_compact_records?records[4*i+2]:sources[i]; \
      local.array_work_ordinal=ctx->array_compact_records?records[4*i+3]:work[i];local.array_append_seq=0; \
      local.array_edge_index=edges?edges[i]:-1;                              \
      if(gate&&!gate[local.array_source_ordinal])continue;                   \
      MARK;                                                                \
      PairFn(PairState,&local,u,v);                                         \
    }                                                                      \
  } while(0)
    if(local.spatial_group==SGPL_SPATIAL_PAIR_OCCURRENCE)
      SGPL_PAIR_BATCH(__atomic_fetch_add(&ctx->spatial_occupied_groups,1,__ATOMIC_RELAXED));
    else if(seen&&local.spatial_group==SGPL_SPATIAL_SOURCE)
      SGPL_PAIR_BATCH(if(group_tags){if(group_tags[u]!=group_epoch){group_tags[u]=group_epoch;seen[u]=1;++groups_local;}}else if(sparse_groups){if(!seen[u]){seen[u]=1;++groups_local;}}else seen[u]=1);
    else if(seen)
      SGPL_PAIR_BATCH(if(group_tags){if(group_tags[v]!=group_epoch){group_tags[v]=group_epoch;seen[v]=1;++groups_local;}}else if(sparse_groups){if(!seen[v]){seen[v]=1;++groups_local;}}else seen[v]=1);
    else
      SGPL_PAIR_BATCH((void)0);
#undef SGPL_PAIR_BATCH
    if(groups_local)__atomic_fetch_add(&ctx->spatial_occupied_groups,groups_local,__ATOMIC_RELAXED);
    if(sgpl_exec_failed(&local))return;
  } else if (local.array_append_sites) {
    for(int64_t i=ctx->gpu_record_offsets[p];i<ctx->gpu_record_offsets[p+1];++i) {
      int32_t u=ctx->gpu_records[3*i],v=ctx->gpu_records[3*i+1],kind=ctx->gpu_records[3*i+2];
      local.array_source_ordinal=ctx->gpu_record_sources[i];
      local.array_work_ordinal=ctx->gpu_record_work[i];local.array_append_seq=0;
      /* Device grouping may reorder records; its CPU fallback keeps the
       * original language query rather than using an unreordered CSR hint. */
      local.array_edge_index=ctx->array_record_edges&&!ctx->gpu_group_offsets?ctx->array_record_edges[i]:-1;
      if(kind==1) {local.source_state=NULL;sgpl_exec_source_begin_ops(&local,u);}
      else if(kind==2)sgpl_exec_source_end_ops(&local,u);
      else if(!local.spatial_source_gate||local.spatial_source_gate[local.array_source_ordinal])
        SGPL_EXEC_EMIT_PAIR(u,v);
      if(sgpl_exec_failed(&local))return;
    }
  } else if (local.traversal_kind == SGPL_TRAVERSE_OWNER_V) {
    int64_t rows = meta->push_row_count ? meta->push_row_count[p] : 0;
    int64_t *rp = meta->push_rp ? meta->push_rp[p] : NULL;
    int32_t *ci = meta->push_ci ? meta->push_ci[p] : NULL;
    int32_t *indir = meta->push_indir ? meta->push_indir[p] : NULL;
    for (int64_t r = 0; r < rows; ++r) {
      int32_t u = indir[r];
      if (local.membership && !local.membership[u])
        continue;
      if (local.spatial_source_gate && !local.spatial_source_gate[u]) continue;
      for (int64_t j = rp[r]; j < rp[r + 1]; ++j)
        SGPL_EXEC_EMIT_PAIR(u, ci[j]);
    }
  } else if (local.spatial_group != SGPL_SPATIAL_NONE) {
    /* Visit the complete source range in original order. Empty rows must not
     * move after later nonempty rows when source lifecycle is observable. */
    int32_t *pairs = meta->src_pairs ? meta->src_pairs[p] : NULL;
    int64_t count = meta->src_pair_count ? meta->src_pair_count[p] : 0, e = 0;
    int32_t lo = (int32_t)meta->partition_start[p];
    int32_t hi = (int32_t)meta->partition_start[p + 1];
    for (int32_t u = lo; u < hi; ++u) {
      if (local.membership && !local.membership[u]) continue;
      if (local.execution_error && __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0) return;
      local.source_state = NULL;
      if (local.spatial_group == SGPL_SPATIAL_SOURCE && local.spatial_groups_seen &&
          sgpl_exec_has_cap(&local, SGPL_OP_SOURCE_BEGIN | SGPL_OP_SOURCE_END))
        local.spatial_groups_seen[u] = 1;
      sgpl_exec_source_begin_ops(&local, u);
      while (e < count && pairs[2*e] < u) ++e;
      while (e < count && pairs[2*e] == u) {
        if (!local.spatial_source_gate || local.spatial_source_gate[u])
          SGPL_EXEC_EMIT_PAIR(u, pairs[2*e+1]);
        ++e;
      }
      if (local.execution_error && __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0) return;
      sgpl_exec_source_end_ops(&local, u);
    }
  } else {
    int32_t *pairs = meta->src_pairs ? meta->src_pairs[p] : NULL;
    int64_t cnt = meta->src_pair_count ? meta->src_pair_count[p] : 0;
    int32_t pending = -1;
    int64_t e = 0;
    for (; e < cnt; ++e) {
      int32_t u = pairs[2 * e];
      int32_t v = pairs[2 * e + 1];
      if (local.membership && !local.membership[u])
        continue;
      if (pending != u) {
        if (pending >= 0)
          sgpl_exec_source_end_ops(&local, pending);
        local.source_state = NULL; /* source-scoped claim channel (R7) */
        sgpl_exec_source_begin_ops(&local, u);
        pending = u;
      }
      if (!local.spatial_source_gate || local.spatial_source_gate[u])
        SGPL_EXEC_EMIT_PAIR(u, v);
    }
    if (pending >= 0)
      sgpl_exec_source_end_ops(&local, pending);

    /* Complete source-domain coverage: a source in this partition's range
     * with no pairs in the slice still gets begin/end, so a zero-pair source
     * observes source_begin(u); source_end(u). */
    if (meta->partition_start && pairs &&
        sgpl_exec_has_cap(&local, SGPL_OP_SOURCE_BEGIN | SGPL_OP_SOURCE_END)) {
      int64_t e2 = 0;
      int32_t lo = (int32_t)meta->partition_start[p];
      int32_t hi = (int32_t)meta->partition_start[p + 1];
      for (int32_t u = lo; u < hi; ++u) {
        if (local.membership && !local.membership[u])
          continue;
        while (e2 < cnt && pairs[2 * e2] < u)
          ++e2;
        if (e2 < cnt && pairs[2 * e2] == u)
          continue;
        local.source_state = NULL; /* source-scoped claim channel (R7) */
        sgpl_exec_source_begin_ops(&local, u);
        sgpl_exec_source_end_ops(&local, u);
      }
    }
  }

  sgpl_exec_partition_end_ops(&local, p);
#undef SGPL_EXEC_EMIT_PAIR
}

/* Record-format versioning preserves the established reader verbatim.
 * Only prepared bounded-hint records enter this fixed-stride reader. */
#if defined(__GNUC__) && defined(__ELF__)
__attribute__((noinline,section(".sgpl_packed_records")))
#endif
static void sgpl_exec_partition_body_packed(int64_t index, void *opaque) {
  sgpl_exec_ctx *ctx = (sgpl_exec_ctx *)opaque;
  sgpl_exec_ctx local;sgpl_exec_context_copy(&local,ctx); /* worker-local state */
  AutoGraphMeta *meta = find_meta(ctx->graph);
  int32_t p = (int32_t)index;
  local.spatial_partition = p;
  if (local.spatial_lane_seen && meta)
    local.dest_seen = local.spatial_lane_seen + (int64_t)p * meta->csr_n;
  if (local.execution_error && __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0)
    return;
  /* Hoist the single pair operation's callback and state: compiler-emitted
   * contexts carry exactly one pair op, and holding them in locals keeps the
   * per-pair path at one indirect call with no descriptor reloads. */
  void (*PairFn)(void *, sgpl_exec_ctx *, int32_t, int32_t) = NULL;
  void *PairState = NULL;

  local.partition_state =
      (local.partition_base && local.partition_stride > 0)
          ? (char *)local.partition_base + (int64_t)p * local.partition_stride
          : local.partition_base;

  sgpl_exec_partition_begin_ops(&local, p);

  if (local.op_count == 1 && (local.ops[0].capabilities & SGPL_OP_PAIR)) {
    PairFn = local.ops[0].pair;
    PairState = local.ops[0].state;
  }
#define SGPL_EXEC_EMIT_PAIR(U, V)                                             \
  do {                                                                        \
    if (local.execution_error &&                                              \
        __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0)          \
      return;                                                                 \
    if (local.spatial_group == SGPL_SPATIAL_PAIR_OCCURRENCE)                   \
      __atomic_fetch_add(&ctx->spatial_occupied_groups, 1, __ATOMIC_RELAXED);   \
    else if (local.spatial_groups_seen)                                       \
      local.spatial_groups_seen[local.spatial_group == SGPL_SPATIAL_SOURCE    \
          ? (U) : (V)] = 1;                                                  \
    if (PairFn)                                                               \
      PairFn(PairState, &local, (U), (V));                                    \
    else                                                                      \
      sgpl_exec_pair_ops(&local, (U), (V));                                   \
  } while (0)

  if (local.array_append_sites && SGPL_EXEC_PAIR_BATCH && local.array_pair_records && PairFn) {
    /* Preparation certifies that this batch contains pairs only. Select the
     * grouping action once, retain the occurrence gate and cancellation at
     * each callback, and leave lifecycle-bearing batches on the general path. */
    const int32_t *records=ctx->gpu_records;
    const int64_t *sources=ctx->gpu_record_sources,*work=ctx->gpu_record_work;
    const int64_t *edges=ctx->gpu_group_offsets?NULL:ctx->array_record_edges;
    const uint8_t *gate=local.spatial_source_gate;
    uint8_t *seen=local.spatial_groups_seen;
    int sparse_groups=sgpl_sparse_group_tracking(ctx);uint64_t groups_local=0;
    SgplExecutionScratch *group_scratch=ctx->execution_scratch;
    uint32_t *group_tags=sparse_groups&&group_scratch&&group_scratch->group_capacity>=(size_t)meta->csr_n?group_scratch->group_tags:NULL;
    uint32_t group_epoch=group_tags?group_scratch->group_epoch:0;
    int64_t first=ctx->gpu_record_offsets[p],last=ctx->gpu_record_offsets[p+1];
#define SGPL_PAIR_BATCH(MARK) do {                                           \
    for(int64_t i=first;i<last;++i) {                                        \
      if(sgpl_exec_failed(&local))return;                                   \
      int stride=5;                            \
      int32_t u=records[stride*i],v=records[stride*i+1];                     \
      local.array_source_ordinal=records[5*i+2]; \
      local.array_work_ordinal=records[5*i+3];local.array_append_seq=0; \
      local.array_edge_index=records[5*i+4];                              \
      if(gate&&!gate[local.array_source_ordinal])continue;                   \
      MARK;                                                                \
      PairFn(PairState,&local,u,v);                                         \
    }                                                                      \
  } while(0)
    if(local.spatial_group==SGPL_SPATIAL_PAIR_OCCURRENCE)
      SGPL_PAIR_BATCH(__atomic_fetch_add(&ctx->spatial_occupied_groups,1,__ATOMIC_RELAXED));
    else if(seen&&local.spatial_group==SGPL_SPATIAL_SOURCE)
      SGPL_PAIR_BATCH(if(group_tags){if(group_tags[u]!=group_epoch){group_tags[u]=group_epoch;seen[u]=1;++groups_local;}}else if(sparse_groups){if(!seen[u]){seen[u]=1;++groups_local;}}else seen[u]=1);
    else if(seen)
      SGPL_PAIR_BATCH(if(group_tags){if(group_tags[v]!=group_epoch){group_tags[v]=group_epoch;seen[v]=1;++groups_local;}}else if(sparse_groups){if(!seen[v]){seen[v]=1;++groups_local;}}else seen[v]=1);
    else
      SGPL_PAIR_BATCH((void)0);
#undef SGPL_PAIR_BATCH
    if(groups_local)__atomic_fetch_add(&ctx->spatial_occupied_groups,groups_local,__ATOMIC_RELAXED);
    if(sgpl_exec_failed(&local))return;
  } else if (local.array_append_sites) {
    for(int64_t i=ctx->gpu_record_offsets[p];i<ctx->gpu_record_offsets[p+1];++i) {
      int32_t u=ctx->gpu_records[3*i],v=ctx->gpu_records[3*i+1],kind=ctx->gpu_records[3*i+2];
      local.array_source_ordinal=ctx->gpu_record_sources[i];
      local.array_work_ordinal=ctx->gpu_record_work[i];local.array_append_seq=0;
      /* Device grouping may reorder records; its CPU fallback keeps the
       * original language query rather than using an unreordered CSR hint. */
      local.array_edge_index=ctx->array_record_edges&&!ctx->gpu_group_offsets?ctx->array_record_edges[i]:-1;
      if(kind==1) {local.source_state=NULL;sgpl_exec_source_begin_ops(&local,u);}
      else if(kind==2)sgpl_exec_source_end_ops(&local,u);
      else if(!local.spatial_source_gate||local.spatial_source_gate[local.array_source_ordinal])
        SGPL_EXEC_EMIT_PAIR(u,v);
      if(sgpl_exec_failed(&local))return;
    }
  } else if (local.traversal_kind == SGPL_TRAVERSE_OWNER_V) {
    int64_t rows = meta->push_row_count ? meta->push_row_count[p] : 0;
    int64_t *rp = meta->push_rp ? meta->push_rp[p] : NULL;
    int32_t *ci = meta->push_ci ? meta->push_ci[p] : NULL;
    int32_t *indir = meta->push_indir ? meta->push_indir[p] : NULL;
    for (int64_t r = 0; r < rows; ++r) {
      int32_t u = indir[r];
      if (local.membership && !local.membership[u])
        continue;
      if (local.spatial_source_gate && !local.spatial_source_gate[u]) continue;
      for (int64_t j = rp[r]; j < rp[r + 1]; ++j)
        SGPL_EXEC_EMIT_PAIR(u, ci[j]);
    }
  } else if (local.spatial_group != SGPL_SPATIAL_NONE) {
    /* Visit the complete source range in original order. Empty rows must not
     * move after later nonempty rows when source lifecycle is observable. */
    int32_t *pairs = meta->src_pairs ? meta->src_pairs[p] : NULL;
    int64_t count = meta->src_pair_count ? meta->src_pair_count[p] : 0, e = 0;
    int32_t lo = (int32_t)meta->partition_start[p];
    int32_t hi = (int32_t)meta->partition_start[p + 1];
    for (int32_t u = lo; u < hi; ++u) {
      if (local.membership && !local.membership[u]) continue;
      if (local.execution_error && __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0) return;
      local.source_state = NULL;
      if (local.spatial_group == SGPL_SPATIAL_SOURCE && local.spatial_groups_seen &&
          sgpl_exec_has_cap(&local, SGPL_OP_SOURCE_BEGIN | SGPL_OP_SOURCE_END))
        local.spatial_groups_seen[u] = 1;
      sgpl_exec_source_begin_ops(&local, u);
      while (e < count && pairs[2*e] < u) ++e;
      while (e < count && pairs[2*e] == u) {
        if (!local.spatial_source_gate || local.spatial_source_gate[u])
          SGPL_EXEC_EMIT_PAIR(u, pairs[2*e+1]);
        ++e;
      }
      if (local.execution_error && __atomic_load_n(local.execution_error, __ATOMIC_ACQUIRE) < 0) return;
      sgpl_exec_source_end_ops(&local, u);
    }
  } else {
    int32_t *pairs = meta->src_pairs ? meta->src_pairs[p] : NULL;
    int64_t cnt = meta->src_pair_count ? meta->src_pair_count[p] : 0;
    int32_t pending = -1;
    int64_t e = 0;
    for (; e < cnt; ++e) {
      int32_t u = pairs[2 * e];
      int32_t v = pairs[2 * e + 1];
      if (local.membership && !local.membership[u])
        continue;
      if (pending != u) {
        if (pending >= 0)
          sgpl_exec_source_end_ops(&local, pending);
        local.source_state = NULL; /* source-scoped claim channel (R7) */
        sgpl_exec_source_begin_ops(&local, u);
        pending = u;
      }
      if (!local.spatial_source_gate || local.spatial_source_gate[u])
        SGPL_EXEC_EMIT_PAIR(u, v);
    }
    if (pending >= 0)
      sgpl_exec_source_end_ops(&local, pending);

    /* Complete source-domain coverage: a source in this partition's range
     * with no pairs in the slice still gets begin/end, so a zero-pair source
     * observes source_begin(u); source_end(u). */
    if (meta->partition_start && pairs &&
        sgpl_exec_has_cap(&local, SGPL_OP_SOURCE_BEGIN | SGPL_OP_SOURCE_END)) {
      int64_t e2 = 0;
      int32_t lo = (int32_t)meta->partition_start[p];
      int32_t hi = (int32_t)meta->partition_start[p + 1];
      for (int32_t u = lo; u < hi; ++u) {
        if (local.membership && !local.membership[u])
          continue;
        while (e2 < cnt && pairs[2 * e2] < u)
          ++e2;
        if (e2 < cnt && pairs[2 * e2] == u)
          continue;
        local.source_state = NULL; /* source-scoped claim channel (R7) */
        sgpl_exec_source_begin_ops(&local, u);
        sgpl_exec_source_end_ops(&local, u);
      }
    }
  }

  sgpl_exec_partition_end_ops(&local, p);
#undef SGPL_EXEC_EMIT_PAIR
}
