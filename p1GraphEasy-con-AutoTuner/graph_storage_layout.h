#ifndef SGPL_GRAPH_STORAGE_LAYOUT_H
#define SGPL_GRAPH_STORAGE_LAYOUT_H
#include <stddef.h>
#include <stdint.h>

/* Shared native ABI. Keep the original forward-CSR offsets stable. Runtime
 * consumers also read the nullable transpose slots, including on fresh loads. */
typedef struct SgplGraphStorage {
    int64_t n, m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
    int32_t directed;
    int64_t *in_row_ptr;
    int32_t *in_col_idx;
} SgplGraphStorage;
#ifdef __cplusplus
#define SGPL_GRAPH_ASSERT static_assert
#else
#define SGPL_GRAPH_ASSERT _Static_assert
#endif
SGPL_GRAPH_ASSERT(sizeof(SgplGraphStorage) == 64, "Graph ABI requires 64 bytes");
SGPL_GRAPH_ASSERT(offsetof(SgplGraphStorage, row_ptr) == 16, "Graph row offset");
SGPL_GRAPH_ASSERT(offsetof(SgplGraphStorage, col_idx) == 24, "Graph column offset");
SGPL_GRAPH_ASSERT(offsetof(SgplGraphStorage, weights) == 32, "Graph weight offset");
SGPL_GRAPH_ASSERT(offsetof(SgplGraphStorage, directed) == 40, "Graph direction offset");
SGPL_GRAPH_ASSERT(offsetof(SgplGraphStorage, in_row_ptr) == 48, "Graph transpose rows");
SGPL_GRAPH_ASSERT(offsetof(SgplGraphStorage, in_col_idx) == 56, "Graph transpose columns");
#undef SGPL_GRAPH_ASSERT
#endif
