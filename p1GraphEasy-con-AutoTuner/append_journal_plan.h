#ifndef SGPL_APPEND_JOURNAL_PLAN_H
#define SGPL_APPEND_JOURNAL_PLAN_H
#include <stdint.h>
/* Protocol v3 retains the v1 descriptor shape and v2 private storage. The
 * additional certificate states that native record induction and the acyclic
 * append proof already imply the producer's ordinal/capacity/order checks.
 * The fourth word is now the certified append bound, not a reserved word. */
#define SGPL_JOURNAL_BITMAP_PROTOCOL 1
#define SGPL_JOURNAL_PROVEN_APPEND 256u
enum {SGPL_APPEND_JOURNAL_VERSION=3};
typedef struct sgpl_append_journal_plan_v1 {
  uint32_t version,bytes,obligations,append_sites;
  void *pair,*journal_pair,*source_gate,*direct_dense,*direct_sparse;
} sgpl_append_journal_plan_v1;
#endif
