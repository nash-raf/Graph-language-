/* Reuse the graph-state audit and bypass iterator tracking for its own probes. */
#define autograph_neighbor_iter_init __real_autograph_neighbor_iter_init
#define autograph_neighbor_iter_next __real_autograph_neighbor_iter_next
#define __wrap_autograph_ensure_layout original_audit_ensure_layout
#include "layout_application_audit.c"
#undef autograph_neighbor_iter_init
#undef autograph_neighbor_iter_next
#undef __wrap_autograph_ensure_layout

static struct { void *graph; AutoNeighborIter *iter; } active_iterators[256];
static uint64_t iterator_initializations;

void __wrap_autograph_ensure_layout(void *g,int64_t n,int64_t m,int64_t *rp,
    int32_t *ci,void *nb,void *eb,void *ep,int32_t target) {
  if(autograph_get_layout(g)!=target)
    for(int i=0;i<256;i++) if(active_iterators[i].graph==g) {
      fprintf(stderr,"AUDIT conversion during active iterator\n");exit(96);
    }
  original_audit_ensure_layout(g,n,m,rp,ci,nb,eb,ep,target);
}

void __wrap_autograph_neighbor_iter_init(void *g,int64_t u,AutoNeighborIter *iter) {
  check(g);
  for(int i=0;i<256;i++) if(!active_iterators[i].graph || active_iterators[i].iter==iter) {
    active_iterators[i].graph=g;active_iterators[i].iter=iter;
    iterator_initializations++;
    __real_autograph_neighbor_iter_init(g,u,iter);return;
  }
  fprintf(stderr,"AUDIT too many active iterators\n");exit(97);
}

int __wrap_autograph_neighbor_iter_next(AutoNeighborIter *iter,int32_t *v) {
  AutoGraphMeta *meta=(AutoGraphMeta *)iter->meta;
  if(meta && (iter->layout!=meta->current_layout || iter->epoch!=meta->layout_epoch)) {
    fprintf(stderr,"AUDIT iterator epoch invalidated\n");exit(98);
  }
  int result=__real_autograph_neighbor_iter_next(iter,v);
  if(!result)for(int i=0;i<256;i++)if(active_iterators[i].iter==iter)active_iterators[i].graph=NULL;
  return result;
}

extern int32_t __real_autograph_frontier_execute(void *,void *);
int32_t __wrap_autograph_frontier_execute(void *g,void *ctx) {
  snapshot(g,"frontier_execute",0);
  int32_t result=__real_autograph_frontier_execute(g,ctx);check(g);return result;
}
extern int32_t __real_autograph_frontier_fork_join(void *,void *,void *,void *);
int32_t __wrap_autograph_frontier_fork_join(void *g,void *ctx,void *fork,void *join) {
  snapshot(g,"frontier_fork_join",0);
  int32_t result=__real_autograph_frontier_fork_join(g,ctx,fork,join);check(g);return result;
}
__attribute__((destructor)) static void finish_iterators(void) {
  fprintf(stderr,"AUDIT iterator epochs stable initializations=%llu\n",(unsigned long long)iterator_initializations);
}
