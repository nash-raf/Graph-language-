#ifndef SGPL_FRONTIER_SIMPLIFY_PLAN_H
#define SGPL_FRONTIER_SIMPLIFY_PLAN_H
#include <stdint.h>
/* Private compiler/runtime ABI. Public execution contexts are unchanged. */
#define SGPL_FRONTIER_SIMPLIFY_VERSION 1u
#define SGPL_EXEC_SIMPLIFY_R4 8u
#define SGPL_EXEC_SIMPLIFY_R5 16u
typedef struct {
  void *state, *context;
  const int32_t *records;
  const int64_t *edges;
  int64_t first, last;
  int32_t *error;
  uint8_t *seen;
  uint32_t *tags;
  uint32_t epoch;
  int32_t sparse;
} sgpl_frontier_simplify_args_v1;
/* Return local occupied groups, or -1 at the original cancellation boundary.
 * As in the reference executor, an interrupted owner does not publish its
 * local group counter. Already executed pair effects are never replayed. */
typedef int64_t (*sgpl_frontier_simplify_kernel_v1)(const sgpl_frontier_simplify_args_v1 *);
typedef struct {
  uint32_t version, bytes, rules, reserved;
  uint64_t context_bytes, operation_bytes, argument_bytes;
  void *pair, *journal_pair;
  sgpl_frontier_simplify_kernel_v1 plain, weighted, journal_plain, journal_weighted;
} sgpl_frontier_simplify_plan_v1;
#endif
