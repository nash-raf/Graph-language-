#ifndef SGPL_EXECUTION_CONTEXT_COPY_INC
#define SGPL_EXECUTION_CONTEXT_COPY_INC
#include <stddef.h>
#include <string.h>
/* Workers publish these two fields atomically while other workers start.
 * An aggregate structure copy would read them nonatomically. All remaining
 * context fields are immutable between publication and the required join. */
static __attribute__((always_inline)) inline void sgpl_exec_context_copy(
    sgpl_exec_ctx *out,const sgpl_exec_ctx *in) {
  enum { GROUP=offsetof(sgpl_exec_ctx,spatial_occupied_groups),
         AFTER_GROUP=GROUP+sizeof(in->spatial_occupied_groups),
         STATUS=offsetof(sgpl_exec_ctx,execution_status),
         AFTER_STATUS=STATUS+sizeof(in->execution_status) };
  _Static_assert(AFTER_GROUP<=STATUS&&AFTER_STATUS<=sizeof(*in),"context snapshot field order");
  memcpy(out,in,GROUP);
  out->spatial_occupied_groups=__atomic_load_n(&in->spatial_occupied_groups,__ATOMIC_RELAXED);
  memcpy((char *)out+AFTER_GROUP,(const char *)in+AFTER_GROUP,STATUS-AFTER_GROUP);
  out->execution_status=__atomic_load_n(&in->execution_status,__ATOMIC_ACQUIRE);
  memcpy((char *)out+AFTER_STATUS,(const char *)in+AFTER_STATUS,sizeof(*in)-AFTER_STATUS);
}
#endif
