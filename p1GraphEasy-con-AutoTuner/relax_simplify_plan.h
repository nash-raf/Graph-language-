#ifndef SGPL_RELAX_SIMPLIFY_PLAN_H
#define SGPL_RELAX_SIMPLIFY_PLAN_H
#include <stdint.h>
#define SGPL_EXEC_SIMPLIFY_R6 32u
#define SGPL_EXEC_SIMPLIFY_R7 64u
#define SGPL_EXEC_SIMPLIFY_R8 128u
#define SGPL_EXEC_SIMPLIFY_R9 256u
/* A checked recipe for i32 signed minimum, with exact modular candidate add.
 * The compiler proves the complete callback, snapshot origin and private
 * invocation scope. These bits are witnesses, not caller-selected promises. */
typedef struct {
  uint32_t version, bytes, rules, weighted;
  uint64_t context_bytes, operation_bytes;
  void *pair;
} sgpl_relax_simplify_plan_v1;
/* Version 2 binds the actual snapshot extent as a separate scalar. Legacy
 * version-1 bindings retain baseline execution rather than assume an extent. */
typedef sgpl_relax_simplify_plan_v1 sgpl_relax_simplify_plan_v2;
#endif
