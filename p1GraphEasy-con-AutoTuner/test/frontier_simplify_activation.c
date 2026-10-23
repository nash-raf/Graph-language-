/* Correctness-only link wrapper. Count selection of generated kernels in real
 * algorithm programs without changing their arguments or observing time. */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include "autotuner_runtime.h"
#include "frontier_simplify_plan.h"
static sgpl_frontier_simplify_plan_v1 original, wrapped;
static _Atomic unsigned long calls[4], bindings;
static int64_t plain(const sgpl_frontier_simplify_args_v1 *a){atomic_fetch_add(&calls[0],1);return original.plain(a);}
static int64_t weighted(const sgpl_frontier_simplify_args_v1 *a){atomic_fetch_add(&calls[1],1);return original.weighted(a);}
static int64_t journal_plain(const sgpl_frontier_simplify_args_v1 *a){atomic_fetch_add(&calls[2],1);return original.journal_plain(a);}
static int64_t journal_weighted(const sgpl_frontier_simplify_args_v1 *a){atomic_fetch_add(&calls[3],1);return original.journal_weighted(a);}
extern void __real_sgpl_exec_bind_simplified_frontier_v1(sgpl_exec_ctx *,const sgpl_frontier_simplify_plan_v1 *);
void __wrap_sgpl_exec_bind_simplified_frontier_v1(sgpl_exec_ctx *c,const sgpl_frontier_simplify_plan_v1 *p) {
  if(!p||p->version!=1||p->bytes!=sizeof(*p))abort();
  if(atomic_load(&bindings)&&
     (original.plain!=p->plain||original.weighted!=p->weighted||original.journal_plain!=p->journal_plain||original.journal_weighted!=p->journal_weighted))abort();
  original=*p;wrapped=*p;wrapped.plain=plain;wrapped.weighted=weighted;
  wrapped.journal_plain=p->journal_plain?journal_plain:NULL;
  wrapped.journal_weighted=p->journal_weighted?journal_weighted:NULL;
  atomic_fetch_add(&bindings,1);__real_sgpl_exec_bind_simplified_frontier_v1(c,&wrapped);
}
__attribute__((destructor)) static void result(void) {
  const char *path=getenv("SGPL_FRONTIER_ACTIVATION_FILE");if(!path)return;
  FILE *f=fopen(path,"w");if(!f)abort();
  fprintf(f,"{\"bindings\":%u,\"plain\":%lu,\"weighted\":%lu,\"journal_plain\":%lu,\"journal_weighted\":%lu}\n",
      atomic_load(&bindings),atomic_load(&calls[0]),atomic_load(&calls[1]),atomic_load(&calls[2]),atomic_load(&calls[3]));
  fclose(f);
}
