/* Diagnostic-only linker wrappers. Never linked into timed executables. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "autotuner_runtime.h"

static uint64_t edges, digest;
static void *active_graph;
static int required = -1;
static uint64_t adds, removes;
static int report_registered;
static const char *names[] = {"CSR", "PCSR", "BCSR", "SET"};
static uint64_t mix(uint64_t x) {
  x ^= x >> 30; x *= UINT64_C(0xbf58476d1ce4e5b9);
  x ^= x >> 27; x *= UINT64_C(0x94d049bb133111eb);
  return x ^ (x >> 31);
}
static void report(void) {
  fprintf(stderr,"AUDIT mutations adds=%llu removes=%llu\n",(unsigned long long)adds,(unsigned long long)removes);
}
static void check(void *g) {
  if (!report_registered) { atexit(report); report_registered=1; }
  const char *value = getenv("BENCH_FIXED_LAYOUT");
  if (value) required = atoi(value);
  int current = autograph_get_layout(g);
  if (required >= 0 && current != required) {
    fprintf(stderr, "AUDIT ERROR expected=%s observed=%s\n",names[required],names[current]);
    exit(90);
  }
}
static int has_edge(void *g,int32_t u,int32_t v) {
  AutoNeighborIter iter;
  int32_t neighbor;
  autograph_neighbor_iter_init(g,u,&iter);
  while(autograph_neighbor_iter_next(&iter,&neighbor))
    if(neighbor==v) return 1;
  return 0;
}
extern void __real_autograph_ensure_layout(void *,int64_t,int64_t,void *,void *,void *,void *,void *,int32_t);
void __wrap_autograph_ensure_layout(void *g,int64_t n,int64_t m,void *rp,void *ci,void *nb,void *eb,void *ep,int32_t target) {
  int before = autograph_get_layout(g);
  __real_autograph_ensure_layout(g,n,m,rp,ci,nb,eb,ep,target);
  int after = autograph_get_layout(g);
  if(before != after)
    fprintf(stderr,"AUDIT layout %s -> %s target=%s\n",names[before],names[after],names[target]);
}
extern void __real_graph_add_edge(void *,void *,int32_t,int32_t,int32_t);
void __wrap_graph_add_edge(void *g,void *bm,int32_t u,int32_t v,int32_t id) {
  check(g);
  ++adds;
  if(has_edge(g,u,v)) exit(93);
  __real_graph_add_edge(g,bm,u,v,id);
  check(g);
  if(!has_edge(g,u,v) || !has_edge(g,v,u)) exit(94);
}
extern void __real_graph_remove_edge(void *,void *,int32_t,int32_t,int32_t);
void __wrap_graph_remove_edge(void *g,void *bm,int32_t u,int32_t v,int32_t id) {
  check(g); ++removes;
  if(!has_edge(g,u,v) || !has_edge(g,v,u)) exit(95);
  __real_graph_remove_edge(g,bm,u,v,id);
  check(g);
  if(has_edge(g,u,v) || has_edge(g,v,u)) exit(96);
}
extern void __real_autograph_neighbor_iter_init(void *,int64_t,AutoNeighborIter *);
void __wrap_autograph_neighbor_iter_init(void *g,int64_t u,AutoNeighborIter *it) {
  check(g);
  __real_autograph_neighbor_iter_init(g,u,it);
}
extern int32_t __real_autograph_neighbor_iter_next(AutoNeighborIter *,int32_t *);
int32_t __wrap_autograph_neighbor_iter_next(AutoNeighborIter *it,int32_t *v) {
  int32_t r = __real_autograph_neighbor_iter_next(it,v);
  if (r && active_graph) {
    ++edges;
    digest += mix(((uint64_t)(uint32_t)it->u << 32) | (uint32_t)*v);
  }
  return r;
}
extern int __real_bfs_runtime_src(void *,int32_t,int32_t **,int32_t *);
int __wrap_bfs_runtime_src(void *g,int32_t src,int32_t **array,int32_t *size) {
  check(g);
  active_graph=g; edges=0; digest=0;
  int result=__real_bfs_runtime_src(g,src,array,size);
  active_graph=NULL;
  int64_t n=*((int64_t *)g);
  unsigned char *seen=calloc((size_t)n,1);
  if(result || *size!=n || !seen) exit(91);
  for(int i=0;i<*size;i++) {
    int v=(*array)[i];
    if(v<0 || v>=n || seen[v]) exit(92);
    seen[v]=1;
  }
  free(seen);
  fprintf(stderr,"AUDIT bfs layout=%s visited=%d arcs=%llu digest=%llu\n",
          names[autograph_get_layout(g)],*size,(unsigned long long)edges,(unsigned long long)digest);
  return result;
}
