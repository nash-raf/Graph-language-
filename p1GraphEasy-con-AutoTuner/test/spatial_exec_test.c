/* Reuse the executor fixture's link stubs and graph ABI. */
#define main legacy_executor_main
#define graph_get_edge_pairs unused_stub_pairs
#define graph_get_num_edge_pairs unused_stub_pair_count
#define roaring_bitmap_add unused_bitmap_add
#define roaring_bitmap_clear unused_bitmap_clear
#define roaring_bitmap_contains unused_bitmap_contains
#define roaring_bitmap_get_cardinality unused_bitmap_get_cardinality
#define roaring_bitmap_get_at_index unused_bitmap_get_at_index
#include "exec_engine_test.c"
#undef main
#undef graph_get_edge_pairs
#undef graph_get_num_edge_pairs
#undef roaring_bitmap_add
#undef roaring_bitmap_clear
#undef roaring_bitmap_contains
#undef roaring_bitmap_get_cardinality
#undef roaring_bitmap_get_at_index
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>

/* Small exact bitmap fixture for native layout conversions. */
struct RoaringBitmap { uint64_t bits; };
static RoaringBitmap fixture_nodes={127},fixture_edges={127};
void roaring_bitmap_add(RoaringBitmap *b,uint32_t v) { if(b && v<64)b->bits|=UINT64_C(1)<<v; }
void roaring_bitmap_clear(RoaringBitmap *b) { if(b)b->bits=0; }
bool roaring_bitmap_contains(const RoaringBitmap *b,uint32_t v) { return b && v<64 && (b->bits&(UINT64_C(1)<<v)); }
uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *b) { return b?__builtin_popcountll(b->bits):0; }
uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *b,uint64_t i) {
  for(uint32_t v=0;v<64;++v)if(roaring_bitmap_contains(b,v) && i--==0)return v;
  return 0;
}
static int32_t pairs[] = {0,1,0,2,1,2,1,2,2,3,4,1,5,6};
static int32_t shuffled[] = {5,6,1,2,0,1,4,1,1,2,0,2,2,3};
static int32_t *native_pairs = pairs;
int32_t *graph_get_edge_pairs(void *g) { (void)g; return native_pairs; }
#ifndef SGPL_FIXTURE_EDGE_COUNT
#define SGPL_FIXTURE_EDGE_COUNT(g) (((ExecTestGraph *)(g))->m?7:0)
#endif
int64_t graph_get_num_edge_pairs(void *g) { return SGPL_FIXTURE_EDGE_COUNT(g); }
static atomic_int fail_malloc,fail_calloc,fail_thread;
/* Optional shape-scoped failures exercise a builder's parallel and serial
 * allocation recovery without poisoning the reference executor's buffers. */
static atomic_int fail_calloc_bytes,fail_calloc_matches;
static int fail_countdown(atomic_int *counter) {
  int old=atomic_load(counter);
  while(old>0)if(atomic_compare_exchange_weak(counter,&old,old-1))return old==1;
  return 0;
}
int __real_pthread_create(pthread_t *,const pthread_attr_t *,void *(*)(void *),void *);
int __wrap_pthread_create(pthread_t *t,const pthread_attr_t *a,void *(*fn)(void *),void *state) {
  if(fail_countdown(&fail_thread))return 11; /* EAGAIN */
  return __real_pthread_create(t,a,fn,state);
}
void *__real_calloc(size_t,size_t);
void *__wrap_calloc(size_t n,size_t sz) {
  if(n*sz==(size_t)atomic_load(&fail_calloc_bytes)&&
      atomic_load(&fail_calloc_matches)>0) {
    atomic_fetch_sub(&fail_calloc_matches,1);return NULL;
  }
  if(fail_countdown(&fail_calloc))return NULL;
  return __real_calloc(n,sz);
}
void *__real_malloc(size_t);
void __sanitizer_print_stack_trace(void) __attribute__((weak));
void *__wrap_malloc(size_t n) {
  if (fail_countdown(&fail_malloc)) {
    if(getenv("SGPL_TEST_ALLOC_STACK")) {
      fprintf(stderr,"injected malloc failure bytes=%zu\n",n);
      if(__sanitizer_print_stack_trace)__sanitizer_print_stack_trace();
    }
    return NULL;
  }
  return __real_malloc(n);
}
typedef struct {
  int parts, source, frontier, fail, overlap;
  atomic_int active, arrived, peak, calls, errors;
  atomic_int owners[7];
  int begin[7], end[7], claims[7], counts[7][7];
  int source_events[7][32], source_event_length[7];
  int length[7], sequence[7][32];
  int expected_length[7], expected[7][32];
} spatial_state;
static void maximum(atomic_int *p, int n) {
  int old = atomic_load(p);
  while (old < n && !atomic_compare_exchange_weak(p, &old, n)) {}
}
static double time_seconds(void) {
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}
static void batch_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t p) {
  spatial_state *s = op->state;
  if (!s->expected_length[p]) return;
  maximum(&s->peak, atomic_fetch_add(&s->active, 1) + 1);
  atomic_fetch_add(&s->arrived, 1);
  if (s->overlap) {
    double end = time_seconds() + 3;
    while (atomic_load(&s->arrived) < 2 && time_seconds() < end) usleep(100);
    if (atomic_load(&s->arrived) < 2) atomic_fetch_add(&s->errors, 1);
  }
  (void)ctx;
}
static void batch_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t p) {
  spatial_state *s = op->state;
  if (s->expected_length[p]) atomic_fetch_sub(&s->active, 1);
  (void)ctx;
}
static void source_begin(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  spatial_state *s = op->state; ++s->begin[u]; s->claims[u] = u + 1;
  ctx->source_state = &s->claims[u];
  s->source_events[ctx->spatial_partition][s->source_event_length[ctx->spatial_partition]++] = 2*u;
}
static void source_end(sgpl_runtime_op *op, sgpl_exec_ctx *ctx, int32_t u) {
  spatial_state *s = op->state; ++s->end[u];
  s->source_events[ctx->spatial_partition][s->source_event_length[ctx->spatial_partition]++] = 2*u+1;
  if (ctx->source_state != &s->claims[u]) atomic_fetch_add(&s->errors, 1);
}
static void spatial_pair(void *state, sgpl_exec_ctx *ctx, int32_t u, int32_t v) {
  spatial_state *s = state;
  int owner = s->source ? u : v, p = ctx->spatial_partition;
  if (atomic_fetch_add(&s->owners[owner], 1)) atomic_fetch_add(&s->errors, 1);
  if (s->source && (ctx->source_state != &s->claims[u] || s->claims[u] != u + 1))
    atomic_fetch_add(&s->errors, 1);
  int slot = s->length[p];
  s->sequence[p][slot] = 10*u+v; ++s->length[p];
  ++s->counts[u][v]; atomic_fetch_add(&s->calls, 1);
  if (s->frontier) autograph_frontier_activate(ctx, v);
  if (s->fail) __atomic_store_n(ctx->execution_error, SGPL_DAG_ERR_NODE, __ATOMIC_RELEASE);
  usleep(500);
  atomic_fetch_sub(&s->owners[owner], 1);
}
static sgpl_exec_ctx *context(ExecTestGraph *g, spatial_state *s, int source,
                              int frontier, int32_t *seen, int32_t *next,
                              int32_t *head, const uint8_t *membership) {
  sgpl_runtime_op *op = calloc(1, sizeof(*op));
  op->state = s; op->capabilities = SGPL_OP_PAIR | SGPL_OP_PARTITION_BEGIN | SGPL_OP_PARTITION_END;
  op->pair = spatial_pair; op->partition_begin = batch_begin; op->partition_end = batch_end;
  if (source) {
    op->capabilities |= SGPL_OP_SOURCE_BEGIN | SGPL_OP_SOURCE_END;
    op->source_begin = source_begin; op->source_end = source_end;
  }
  sgpl_runtime_op *ops[] = {op};
  sgpl_exec_ctx *ctx = autograph_exec_ctx_create(g, source ? SGPL_TRAVERSE_OWNER_U : SGPL_TRAVERSE_OWNER_V,
      SGPL_DOMAIN_ALL_VERTICES, membership, seen, next, 0, head, NULL, 0, ops, 1, -1);
  autograph_exec_ctx_set_dag_axes(ctx, 1, -1);
  s->source = source; s->frontier = frontier;
  (void)frontier;
  return ctx;
}
static void expected(spatial_state *s, int source, int parts) {
  s->parts = parts;
  for (unsigned e = 0; e < sizeof(pairs)/sizeof(pairs[0]); e += 2) {
    int u=pairs[e],v=pairs[e+1];
    int p = source ? ((u+1)*parts-1)/7 : v*parts/7;
    int n=s->expected_length[p]++;
    s->expected[p][n] = 10*u+v;
  }
  int live=0;
  for (int p=0;p<parts;++p) live += s->expected_length[p]>0;
  s->overlap = live>1 && sgpl_configured_worker_count()>1;
}
static void test_layout(ExecTestGraph *g, int layout, int parts, int source, int frontier) {
  autograph_ensure_layout(g, 7, 7, g->row_ptr, g->col_idx, &fixture_nodes, &fixture_edges, native_pairs, layout);
  check(autograph_get_layout(g)==layout, "requested native layout is active");
  int built=autograph_build_clean_cut(g,parts);
  check(built>0, "CleanCut materialized selected layout");
  spatial_state s={0}; expected(&s,source,built);
  int32_t seen[7]={0}, next[7]={0}, head=0;
  sgpl_exec_ctx *ctx=context(g,&s,source,frontier,seen,next,&head,NULL);
  int group=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
  check(autograph_exec_ctx_prepare_spatial(ctx,group,group,frontier)==0, "spatial preparation succeeded");
  check(autograph_frontier_execute(g,ctx)==(frontier?4:0), "joined execution and frontier cardinality");
  check(atomic_load(&s.calls)==7, "all seven occurrences, including duplicate edge, executed once");
  check(atomic_load(&s.errors)==0, "dependent owners never overlap and claim state is source-local");
  check(atomic_load(&s.active)==0, "every batch joined");
  check(ctx->spatial_occupied_groups==(uint64_t)(source?7:4) && ctx->spatial_nontrivial,
        "runtime cardinality is based on nonempty proof groups");
  check(!s.overlap || atomic_load(&s.peak)>=2, "independent nonempty batches overlap");
  for(int p=0;p<built;++p)
    check(s.length[p]==s.expected_length[p] && !memcmp(s.sequence[p],s.expected[p],s.length[p]*sizeof(int)),
          "batch preserves original source and neighbor order");
  if(source) for(int u=0;u<7;++u) check(s.begin[u]==1&&s.end[u]==1,"source lifecycle occurs once, including zero-neighbor sources");
  if(source) for(int p=0;p<built;++p) {
    int lo=p*7/built, hi=(p+1)*7/built;
    check(s.source_event_length[p]==2*(hi-lo), "source hooks cover full batch range");
    for(int i=0;i<2*(hi-lo);++i)
      check(s.source_events[p][i]==2*lo+i, "source begin/end order includes empty rows");
  }
  if(frontier) {
    int bits=0; for(int i=0;i<4;++i) bits|=1<<next[i];
    check(bits==((1<<1)|(1<<2)|(1<<3)|(1<<6)), "private frontier lanes merge exact destination set");
    check(ctx->spatial_seen!=seen, "claims use invocation-owned storage");
  }
  autograph_exec_ctx_destroy(ctx);
}
static void test_failures(ExecTestGraph *g) {
  autograph_build_clean_cut(g,3);
  sgpl_runtime_op *missing[] = {NULL};
  check(!autograph_exec_ctx_create(g, SGPL_TRAVERSE_OWNER_V, SGPL_DOMAIN_ALL_VERTICES,
        NULL, NULL, NULL, 0, NULL, NULL, 0, missing, 1, -1), "missing descriptor cannot silently omit effects");
  spatial_state s={0}; expected(&s,0,3); s.overlap=0;
  uint8_t membership[7]={1,1,1,1,1,1,1};
  sgpl_exec_ctx *ctx=context(g,&s,0,0,NULL,NULL,NULL,membership);
  atomic_store(&fail_malloc,1);
  check(autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0)==SGPL_DAG_ERR_NOMEM,
        "allocation failure occurs before any user effect");
  check(atomic_load(&s.calls)==0,"preparation failure executes no callbacks");
  autograph_exec_ctx_destroy(ctx);
  for(int allocation=1;allocation<=5;++allocation) {
    ctx=context(g,&s,0,0,NULL,NULL,NULL,NULL);
    ctx->ops[0].capabilities |= SGPL_OP_SOURCE_BEGIN;
    ctx->ops[0].source_begin = source_begin;
    check(!autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0),"prepare ordered source publication fixture");
    atomic_store(&fail_calloc,allocation);
    check(autograph_frontier_execute(g,ctx)==SGPL_DAG_ERR_NOMEM,"scheduler allocation fails before source preparation");
    for(int u=0;u<7;++u)check(!s.begin[u],"failed dispatch executes no source claim or guard");
    check(!atomic_load(&s.calls),"failed scheduler preparation executes no pair effect");
    autograph_exec_ctx_destroy(ctx);
  }
  if(sgpl_configured_worker_count()>1) {
    /* Cold creation failures are covered by worker_pool_test. A warm
     * spatial dispatch must not attempt another pthread_create. */
    spatial_state warm={0}; expected(&warm,0,3);warm.overlap=0;
    ctx=context(g,&warm,0,0,NULL,NULL,NULL,NULL);
    atomic_store(&fail_thread,1);
    check(autograph_frontier_execute(g,ctx)>=0&&atomic_load(&fail_thread)==1,
          "spatial dispatch reuses workers without thread creation");
    atomic_store(&fail_thread,0);
    check(atomic_load(&warm.calls)==7&&!atomic_load(&warm.errors),
          "reused spatial workers execute every occurrence once");
    autograph_exec_ctx_destroy(ctx);
  }
  ctx=context(g,&s,0,0,NULL,NULL,NULL,NULL);
  check(autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_SOURCE,SGPL_SPATIAL_DESTINATION,0)==SGPL_DAG_ERR_INVALID,
        "runtime cannot split a proof block under another ownership key");
  check(autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0)==0,"prepare cancellation fixture");
  s.fail=1;
  check(autograph_frontier_execute(g,ctx)<0,"started execution returns failure");
  for(int u=0;u<7;++u)for(int v=0;v<7;++v)check(s.counts[u][v]<= (u==1&&v==2?2:1),"failure never replays completed occurrences");
  autograph_exec_ctx_destroy(ctx);
}
static void test_snapshots(ExecTestGraph *g) {
  spatial_state a={0},b={0};
  sgpl_exec_ctx *x=context(g,&a,0,0,NULL,NULL,NULL,NULL),*y=context(g,&b,0,0,NULL,NULL,NULL,NULL);
  int32_t first[7]={1,2,3,4,5,6,7},second[7]={8,9,10,11,12,13,14};
  check(!autograph_exec_ctx_bind_snapshot(x,0,first,4)&&!autograph_exec_ctx_bind_snapshot(y,0,second,4),"bind two snapshots on the same graph");
  first[0]=99;second[0]=99;
  check(autograph_exec_ctx_snapshot(x,0)!=autograph_exec_ctx_snapshot(y,0)&&
        ((int32_t*)autograph_exec_ctx_snapshot(x,0))[0]==1&&((int32_t*)autograph_exec_ctx_snapshot(y,0))[0]==8,
        "snapshot storage and values are invocation-local");
  atomic_store(&fail_malloc,1);
  check(autograph_exec_ctx_bind_snapshot(x,1,first,4)==SGPL_DAG_ERR_NOMEM,"snapshot allocation failure is reported");
  check(!autograph_exec_ctx_snapshot(x,1),"failed snapshot cannot publish a live pointer");
  autograph_exec_ctx_destroy(x);autograph_exec_ctx_destroy(y);
}
static void test_cardinality(ExecTestGraph *g) {
  autograph_build_clean_cut(g,3);
  for(int pair=0;pair<2;++pair) {
    spatial_state s={0}; s.parts=3;
    uint8_t membership[7]={0,0,0,0,0,1,0};
    sgpl_exec_ctx *ctx=context(g,&s,0,0,NULL,NULL,NULL,membership);
    check(!autograph_exec_ctx_prepare_spatial(ctx,pair?SGPL_SPATIAL_PAIR_OCCURRENCE:SGPL_SPATIAL_DESTINATION,
          SGPL_SPATIAL_DESTINATION,0), "prepare one occupied group");
    check(autograph_frontier_execute(g,ctx)==0 && atomic_load(&s.calls)==1 &&
          ctx->spatial_occupied_groups==1 && !ctx->spatial_nontrivial,
          "one occurrence does not establish a non-trivial partition");
    memset(ctx->spatial_membership,0,7);
    check(autograph_frontier_execute(g,ctx)==0 && ctx->spatial_occupied_groups==0 && !ctx->spatial_nontrivial,
          "empty source domain has no occupied work group");
    autograph_exec_ctx_destroy(ctx);
  }
}
static void test_live_set(ExecTestGraph *g) {
  check(autograph_canonical_remove_edge(g,1,2),"remove one static SET occurrence");
  check(autograph_canonical_add_edge(g,6,0),"add a live SET extra occurrence");
  setenv("SGPL_NO_CLEANCUT_CACHE","1",1);
  int parts=autograph_build_clean_cut(g,3);
  for(int source=0;source<2;++source) {
    spatial_state s={0}; s.parts=parts;
    int total=0;
    for(int u=0;u<7;++u) {
      AutoNeighborIter iterator; int32_t v;
      autograph_neighbor_iter_init(g,u,&iterator);
      while(autograph_neighbor_iter_next(&iterator,&v)) {
        int p=source?((u+1)*parts-1)/7:v*parts/7;
        s.expected[p][s.expected_length[p]++]=10*u+v; ++total;
      }
    }
    sgpl_exec_ctx *ctx=context(g,&s,source,0,NULL,NULL,NULL,NULL);
    int key=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
    check(!autograph_exec_ctx_prepare_spatial(ctx,key,key,0),"prepare live SET groups");
    check(!autograph_frontier_execute(g,ctx) && atomic_load(&s.calls)==total,"live SET batches match serial iterator occurrence count");
    for(int p=0;p<parts;++p)check(s.length[p]==s.expected_length[p] &&
          !memcmp(s.sequence[p],s.expected[p],s.length[p]*sizeof(int)),"live SET batches match serial iterator order, including extra edges");
    autograph_exec_ctx_destroy(ctx);
  }
  unsetenv("SGPL_NO_CLEANCUT_CACHE");
}
static void test_undirected_duplicates(void) {
  int64_t rp[4]={0,1,6,6}; int32_t ci[6]={1,0,1,1,1,1};
  int32_t logical_pairs[6]={0,1,1,1,1,1};
  RoaringBitmap nodes={7},edges={7};
  ExecTestGraph g={.n=3,.m=6,.row_ptr=rp,.col_idx=ci,.directed=0};
  autograph_init(&g,3,6,&nodes,&edges,logical_pairs);
  int layouts[]={LAYOUT_CSR,LAYOUT_PCSR,LAYOUT_BCSR,LAYOUT_SET,LAYOUT_CSR};
  for(unsigned l=0;l<sizeof(layouts)/sizeof(layouts[0]);++l) {
    autograph_ensure_layout(&g,3,6,g.row_ptr,g.col_idx,&nodes,&edges,logical_pairs,layouts[l]);
    check(autograph_get_layout(&g)==layouts[l],"undirected native layout is active");
    check(autograph_build_clean_cut(&g,2)==2,"undirected duplicate batches prepared");
    for(int source=0;source<2;++source) {
      spatial_state state={0}; state.parts=2;
      for(int u=0;u<3;++u)for(int64_t e=rp[u];e<rp[u+1];++e) {
        int v=ci[e],p=source?((u+1)*2-1)/3:v*2/3;
        state.expected[p][state.expected_length[p]++]=10*u+v;
      }
      sgpl_exec_ctx *ctx=context(&g,&state,source,0,NULL,NULL,NULL,NULL);
      int key=source?SGPL_SPATIAL_SOURCE:SGPL_SPATIAL_DESTINATION;
      check(!autograph_exec_ctx_prepare_spatial(ctx,key,key,0),"prepare undirected groups");
      check(!autograph_frontier_execute(&g,ctx) && atomic_load(&state.calls)==6,"duplicate undirected self-loops preserve all six body occurrences");
      for(int p=0;p<2;++p)check(state.length[p]==state.expected_length[p] &&
        !memcmp(state.sequence[p],state.expected[p],state.length[p]*sizeof(int)),"undirected native batches preserve source and neighbor order");
      check(!atomic_load(&state.errors),"undirected dependencies stay within one batch");
      autograph_exec_ctx_destroy(ctx);
    }
  }
}
static void test_empty(void) {
  int64_t rows[8]={0}; int32_t columns[1]={0};
  ExecTestGraph empty={.n=7,.m=0,.row_ptr=rows,.col_idx=columns,.directed=1};
  autograph_init(&empty,7,0,NULL,NULL,pairs);
  check(autograph_build_clean_cut(&empty,3)>0, "zero-edge graph still has source coverage batches");
  spatial_state s={0}; s.parts=3;
  sgpl_exec_ctx *ctx=context(&empty,&s,1,0,NULL,NULL,NULL,NULL);
  check(!autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_SOURCE,SGPL_SPATIAL_SOURCE,0), "prepare empty topology");
  check(!autograph_frontier_execute(&empty,ctx) && !atomic_load(&s.calls), "empty topology executes no pair occurrence");
  for(int u=0;u<7;++u) check(s.begin[u]==1&&s.end[u]==1, "empty topology preserves each source hook");
  autograph_exec_ctx_destroy(ctx);
  ExecTestGraph zero={.n=0,.m=0,.row_ptr=rows,.col_idx=columns,.directed=1};
  autograph_init(&zero,0,0,NULL,NULL,pairs);
  s=(spatial_state){0};
  ctx=context(&zero,&s,0,0,NULL,NULL,NULL,NULL);
  check(!autograph_exec_ctx_prepare_spatial(ctx,SGPL_SPATIAL_DESTINATION,SGPL_SPATIAL_DESTINATION,0) &&
        !autograph_frontier_execute(&zero,ctx) && !ctx->spatial_nontrivial, "zero vertices have no non-trivial partition");
  autograph_exec_ctx_destroy(ctx);
}
#ifndef SGPL_SPATIAL_FIXTURE_ONLY
int main(void) {
  int64_t rp[8]={0,2,4,5,5,6,7,7};
  int32_t ci[7]={1,2,2,2,3,1,6};
  ExecTestGraph g={.n=7,.m=7,.row_ptr=rp,.col_idx=ci,.directed=1};
  autograph_init(&g,7,7,NULL,NULL,pairs);
  for(int layout=0;layout<4;++layout) for(int p=1;p<=3;p+=2) {
    test_layout(&g,layout,p,0,1);test_layout(&g,layout,p,1,0);test_layout(&g,layout,p,1,1);
  }
  /* Round-trip through SET retains directed duplicate occurrences. */
  test_layout(&g,LAYOUT_CSR,3,0,1);
  test_layout(&g,LAYOUT_SET,3,1,1);
  test_failures(&g);test_snapshots(&g);test_cardinality(&g);
  native_pairs=shuffled;
  setenv("SGPL_NO_CLEANCUT_CACHE", "1", 1);
  test_layout(&g,LAYOUT_SET,3,0,1); test_layout(&g,LAYOUT_SET,3,1,1);
  native_pairs=pairs;
  unsetenv("SGPL_NO_CLEANCUT_CACHE");
  test_live_set(&g);
  test_undirected_duplicates();
  test_empty();
  printf("spatial executor: %d failures\n",failures);return failures?1:0;
}

#endif
