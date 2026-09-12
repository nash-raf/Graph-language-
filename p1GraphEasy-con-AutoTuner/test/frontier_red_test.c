#define _POSIX_C_SOURCE 200809L

#include "../autotuner_runtime.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The exercised reduction path does not use Roaring bitmaps.  These stubs
 * keep this focused runtime test independent of the bitmap implementation. */
typedef struct RoaringBitmap RoaringBitmap;

RoaringBitmap *roaring_bitmap_create(uint64_t arena_size,
                                     uint64_t initial_capacity) {
  (void)arena_size;
  (void)initial_capacity;
  return NULL;
}

void roaring_bitmap_add(RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
}

void roaring_bitmap_remove(RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
}

bool roaring_bitmap_contains(const RoaringBitmap *bitmap, uint32_t value) {
  (void)bitmap;
  (void)value;
  return false;
}

void roaring_bitmap_clear(RoaringBitmap *bitmap) {
  (void)bitmap;
}

uint64_t roaring_bitmap_get_cardinality(const RoaringBitmap *bitmap) {
  (void)bitmap;
  return 0;
}

uint32_t roaring_bitmap_get_at_index(const RoaringBitmap *bitmap,
                                     uint64_t index) {
  (void)bitmap;
  (void)index;
  return 0;
}

RoaringBitmap *roaring_bitmap_create_like(const RoaringBitmap *prototype) {
  (void)prototype;
  return NULL;
}

void roaring_bitmap_or_inplace(RoaringBitmap *destination,
                               RoaringBitmap *source) {
  (void)destination;
  (void)source;
}

void roaring_bitmap_free(RoaringBitmap *bitmap) {
  (void)bitmap;
}

void roaring_bitmap_set_thread_local_overrides(
    RoaringBitmap **originals, RoaringBitmap **replacements, int32_t count) {
  (void)originals;
  (void)replacements;
  (void)count;
}

void roaring_bitmap_clear_thread_local_overrides(void) {
}

/* cc_arcs_init only reaches these on the SET layout; the CSR harness never
 * calls them, but the linker needs the symbols defined. */
int32_t *graph_get_edge_pairs(void *graph_ptr) {
  (void)graph_ptr;
  return NULL;
}

int64_t graph_get_num_edge_pairs(void *graph_ptr) {
  (void)graph_ptr;
  return 0;
}

/* Graph struct whose field layout matches the offsets autotuner_runtime.c
 * reads (n@0, m@8, row_ptr@16, col_idx@24, weights@32, directed@40,
 * in_row_ptr@48, in_col_idx@56). */
typedef struct {
  int64_t n;
  int64_t m;
  int64_t *row_ptr;
  int32_t *col_idx;
  int32_t *weights;
  int32_t directed;
  int64_t *in_row_ptr;
  int32_t *in_col_idx;
} RedTestGraph;

/* Ops exercised: Add, Sub, Mul, Min, Max, And, Or, Xor.  The work_fn /
 * combine_fn pair below mirrors the compiler's emitted sgpl_combine_red
 * semantics per op (see identityFor / emitRedCombiner in
 * graph_frontier_lowering.cpp). */
enum Op {
  OpAdd,
  OpSub,
  OpMul,
  OpMin,
  OpMax,
  OpAnd,
  OpOr,
  OpXor,
  OpCount
};

static const char *op_name(enum Op op) {
  switch (op) {
  case OpAdd: return "add";
  case OpSub: return "sub";
  case OpMul: return "mul";
  case OpMin: return "min";
  case OpMax: return "max";
  case OpAnd: return "and";
  case OpOr: return "or";
  case OpXor: return "xor";
  default: return "?";
  }
}

/* identity used to pre-zero each partition partial. */
static int32_t identity_for(enum Op op) {
  switch (op) {
  case OpMul: return 1;
  case OpAnd: return -1;
  case OpMin: return INT32_MAX;
  case OpMax: return INT32_MIN;
  default: return 0; /* Add, Sub, Or, Xor */
  }
}

/* The red step dispatches work_fn(u, v, v, partial); we install per-op
 * functions so the op is statically known inside each callback. */
#define DEF_WORK(opname, code)                                                 \
  static void work_##opname(int32_t source, int32_t destination,               \
                            int64_t destination_index, void *env) {            \
    (void)source;                                                              \
    (void)destination_index;                                                   \
    int32_t *partial = (int32_t *)env;                                         \
    int32_t v = destination;                                                   \
    code;                                                                      \
  }

#define DEF_COMBINE(opname, code)                                              \
  static void combine_##opname(const void *partial, void *out) {               \
    const int32_t *p = (const int32_t *)partial;                               \
    int32_t *o = (int32_t *)out;                                               \
    code;                                                                      \
  }

DEF_WORK(add, *partial = *partial + v)
DEF_WORK(sub, *partial = *partial - v)
DEF_WORK(mul, *partial = *partial * v)
DEF_WORK(min, if (v < *partial) *partial = v)
DEF_WORK(max, if (v > *partial) *partial = v)
DEF_WORK(and, *partial = *partial & v)
DEF_WORK(or, *partial = *partial | v)
DEF_WORK(xor, *partial = *partial ^ v)

DEF_COMBINE(add, *o = *o + *p)
DEF_COMBINE(sub, *o = *o + *p) /* partials are negated; combine by addition */
DEF_COMBINE(mul, *o = *o * *p)
DEF_COMBINE(min, if (*p < *o) *o = *p)
DEF_COMBINE(max, if (*p > *o) *o = *p)
DEF_COMBINE(and, *o = *o & *p)
DEF_COMBINE(or, *o = *o | *p)
DEF_COMBINE(xor, *o = *o ^ *p)

/* Serial ground truth: fold over every edge (u, v) in vertex-ascending order
 * with the same operator semantics, starting from init. */
static int32_t serial_fold(const RedTestGraph *g, enum Op op, int32_t init) {
  int32_t acc = init;
  for (int64_t u = 0; u < g->n; ++u) {
    for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j) {
      int32_t v = g->col_idx[j];
      switch (op) {
      case OpAdd: acc = acc + v; break;
      case OpSub: acc = acc - v; break;
      case OpMul: acc = acc * v; break;
      case OpMin: if (v < acc) acc = v; break;
      case OpMax: if (v > acc) acc = v; break;
      case OpAnd: acc = acc & v; break;
      case OpOr: acc = acc | v; break;
      case OpXor: acc = acc ^ v; break;
      default: break;
      }
    }
  }
  return acc;
}

static int run_one(const RedTestGraph *g, enum Op op, int32_t partitions,
                   int32_t init) {
  sgpl_frontier_pair_fn work;
  sgpl_frontier_combine_fn combine;
  switch (op) {
  case OpAdd: work = work_add; combine = combine_add; break;
  case OpSub: work = work_sub; combine = combine_sub; break;
  case OpMul: work = work_mul; combine = combine_mul; break;
  case OpMin: work = work_min; combine = combine_min; break;
  case OpMax: work = work_max; combine = combine_max; break;
  case OpAnd: work = work_and; combine = combine_and; break;
  case OpOr: work = work_or; combine = combine_or; break;
  default: work = work_xor; combine = combine_xor; break;
  }

  int32_t built = autograph_build_clean_cut((void *)g, partitions);
  if (built <= 0) {
    fprintf(stderr, "  FAIL clean-cut built %d partitions\n", built);
    return 0;
  }

  int32_t *partials =
      (int32_t *)calloc((size_t)built, sizeof(int32_t));
  int32_t out = init;
  if (!partials)
    return 0;
  for (int32_t p = 0; p < built; ++p)
    partials[p] = identity_for(op);

  int32_t next = autograph_frontier_step_owner_red(
      (void *)g, NULL, 0, work, partials, sizeof(int32_t), combine, &out,
      NULL, NULL, 0, NULL);

  int32_t expected = serial_fold(g, op, init);
  free(partials);
  (void)next;

  if (out != expected) {
    fprintf(stderr,
            "  FAIL op=%s partitions=%d init=%d out=%d expected=%d\n",
            op_name(op), (int)built, init, out, expected);
    return 0;
  }
  printf("  ok   op=%-3s partitions=%-2d init=%d -> %d\n", op_name(op),
         (int)built, init, out);
  return 1;
}

int main(void) {
  /* Directed CSR: n=7, m=12.  Rows deliberately vary in length and vertex 0
   * is a source with two destinations so multiple rows feed each partition. */
  int64_t row_ptr[8] = {0, 2, 4, 6, 9, 11, 12, 12};
  int32_t col_idx[12] = {1, 5, 0, 3, 2, 6, 1, 4, 5, 0, 6, 3};
  RedTestGraph g = {
      .n = 7,
      .m = 12,
      .row_ptr = row_ptr,
      .col_idx = col_idx,
      .weights = NULL,
      .directed = 1,
      .in_row_ptr = NULL,
      .in_col_idx = NULL,
  };

  autograph_init(&g, g.n, g.m, NULL, NULL, NULL);

  const int32_t partitions[] = {1, 3, 8};
  const int32_t inits[] = {0, 100, -1, 7}; /* exercise out=identity and non-identity */
  int failures = 0;
  int runs = 0;

  for (size_t pi = 0; pi < sizeof(partitions) / sizeof(partitions[0]); ++pi) {
    for (int op = 0; op < OpCount; ++op) {
      /* init chosen per op: identity, plus a non-identity probe for ops that
       * combine into out (Add/Sub/Mul/Or/Xor start from the init value). */
      int32_t init;
      switch (op) {
      case OpAdd: init = 100; break;
      case OpSub: init = 100; break;
      case OpMul: init = 2; break;
      case OpMin: init = INT32_MAX; break;
      case OpMax: init = INT32_MIN; break;
      case OpAnd: init = -1; break;
      case OpOr: init = 0; break;
      default: init = 0; break;
      }
      runs++;
      if (!run_one(&g, (enum Op)op, partitions[pi], init))
        failures++;
    }
  }

  printf("red step matrix: %d configs, %d failures\n", runs, failures);
  return failures == 0 ? 0 : 1;
}