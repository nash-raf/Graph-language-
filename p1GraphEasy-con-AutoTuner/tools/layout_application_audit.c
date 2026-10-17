/* Separate diagnostics. Timed executables never contain these wrappers. */
#include "autotuner_runtime.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static int prepared,fixed=-1,full=1;
static uint64_t stream_ops,window_ops,operations,adds,removes,sampled,native_ops,epoch,cached_epoch=UINT64_MAX;
static uint64_t arc_count,digest;
static void *last_graph;
static const char *names[]={"CSR","PCSR","BCSR","SET"};
static void finish(void) {
  fprintf(stderr,"AUDIT counts adds=%llu removes=%llu samples=%llu native=%llu\n",
    (unsigned long long)adds,(unsigned long long)removes,(unsigned long long)sampled,(unsigned long long)native_ops);
}
static void prepare(void) {
  if(prepared)return;prepared=1;atexit(finish);
  const char *p=getenv("BENCH_FIXED_LAYOUT");if(p)fixed=atoi(p);
  p=getenv("AUDIT_STREAM_OPERATIONS");if(p)stream_ops=strtoull(p,0,10);
  p=getenv("AUDIT_WINDOW_OPERATIONS");if(p)window_ops=strtoull(p,0,10);
  p=getenv("AUDIT_FULL_NATIVE");if(p)full=atoi(p);
}
static void check(void *g) {
  prepare();if(fixed>=0 && autograph_get_layout(g)!=fixed) {
    fprintf(stderr,"AUDIT wrong fixed layout\n");exit(90);
  }
}
static int physical(void) {
  return full || operations<=2*window_ops || operations>stream_ops-2*window_ops;
}
static int sample(void) {
  return operations<=2*window_ops || operations>stream_ops-2*window_ops;
}
static int has(void *g,int32_t u,int32_t v) {
  AutoNeighborIter it;int32_t x;autograph_neighbor_iter_init(g,u,&it);
  while(autograph_neighbor_iter_next(&it,&x))if(x==v)return 1;return 0;
}
static uint64_t mix(uint64_t x) {
  x^=x>>30;x*=UINT64_C(0xbf58476d1ce4e5b9);x^=x>>27;x*=UINT64_C(0x94d049bb133111eb);return x^(x>>31);
}
static void snapshot(void *g,const char *kind,int32_t source) {
  check(g);
  if(epoch!=cached_epoch || g!=last_graph) {
    int64_t n=*(int64_t *)g;arc_count=digest=0;
    for(int64_t u=0;u<n;u++) {
      AutoNeighborIter it;int32_t v;autograph_neighbor_iter_init(g,u,&it);
      while(autograph_neighbor_iter_next(&it,&v)) {arc_count++;digest+=mix(((uint64_t)(uint32_t)u<<32)|(uint32_t)v);}
    }
    cached_epoch=epoch;last_graph=g;
  }
  fprintf(stderr,"AUDIT snapshot kind=%s source=%d layout=%s arcs=%llu digest=%llu\n",
    kind,source,names[autograph_get_layout(g)],(unsigned long long)arc_count,(unsigned long long)digest);
}
extern void __real_autograph_ensure_layout(void *,int64_t,int64_t,int64_t *,int32_t *,void *,void *,void *,int32_t);
void __wrap_autograph_ensure_layout(void *g,int64_t n,int64_t m,int64_t *rp,int32_t *ci,void *nb,void *eb,void *ep,int32_t target) {
  int before=autograph_get_layout(g);__real_autograph_ensure_layout(g,n,m,rp,ci,nb,eb,ep,target);
  int after=autograph_get_layout(g);if(before!=after) fprintf(stderr,"AUDIT layout %s -> %s\n",names[before],names[after]);
}
extern void __real_graph_add_edge(void *,void *,int32_t,int32_t,int32_t);
void __wrap_graph_add_edge(void *g,void *bm,int32_t u,int32_t v,int32_t id) {
  check(g);operations++;adds++;
  if(physical()) {
    int probe=sample();if(probe && has(g,u,v))exit(91);
    __real_graph_add_edge(g,bm,u,v,id);native_ops++;epoch++;
    if(probe) {sampled++;if(!has(g,u,v)||!has(g,v,u))exit(92);}
  }
  check(g);
}
extern void __real_graph_remove_edge(void *,void *,int32_t,int32_t,int32_t);
void __wrap_graph_remove_edge(void *g,void *bm,int32_t u,int32_t v,int32_t id) {
  check(g);operations++;removes++;
  if(physical()) {
    int probe=sample();if(probe && !has(g,u,v))exit(93);
    __real_graph_remove_edge(g,bm,u,v,id);native_ops++;epoch++;
    if(probe) {sampled++;if(has(g,u,v)||has(g,v,u))exit(94);}
  }
  check(g);
}
extern int __real_bfs_runtime_src(void *,int32_t,int32_t **,int32_t *);
int __wrap_bfs_runtime_src(void *g,int32_t src,int32_t **array,int32_t *size) {
  snapshot(g,"bfs",src);int r=__real_bfs_runtime_src(g,src,array,size);check(g);return r;
}
extern int __real_dfs_runtime_src(void *,int32_t,int32_t **,int32_t *);
int __wrap_dfs_runtime_src(void *g,int32_t src,int32_t **array,int32_t *size) {
  snapshot(g,"dfs",src);int r=__real_dfs_runtime_src(g,src,array,size);check(g);return r;
}
extern int32_t __real_autograph_edgemap(void *,int32_t,int32_t *,int32_t,int32_t *,int32_t,int32_t *,int32_t *,int32_t,int32_t);
int32_t __wrap_autograph_edgemap(void *g,int32_t mode,int32_t *f,int32_t fs,int32_t *next,int32_t ns,int32_t *p0,int32_t *p1,int32_t s0,int32_t s1) {
  snapshot(g,"edgemap",mode);int32_t r=__real_autograph_edgemap(g,mode,f,fs,next,ns,p0,p1,s0,s1);check(g);return r;
}
