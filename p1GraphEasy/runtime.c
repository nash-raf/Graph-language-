#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

typedef struct RoaringBitmap RoaringBitmap;

extern RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_containers);
extern void roaring_bitmap_free(RoaringBitmap *bm);
extern void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value);
extern RoaringBitmap *roaring_bitmap_union(RoaringBitmap **bitmaps, size_t count);
extern RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2);
extern int roaring_bitmap_contains(RoaringBitmap *bm, uint32_t value);
extern uint32_t roaring_bitmap_get_at_index(RoaringBitmap *bm, uint32_t index);
extern uint64_t roaring_bitmap_get_cardinality(RoaringBitmap *bm);

char* concat_strings(const char* a, const char* b)
{
    char* result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

static int show_counter = 0;

enum
{
    GC_CONNECTED = 1,
    GC_CYCLE = 2,
    GC_DEGREE = 3,
    GC_AND = 4,
    GC_OR = 5
};


void show_graph_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx)
{
    if (!row_ptr || n < 0) return;
    
    int id = show_counter++;

    char dot_name[64];
    char png_name[64];
    snprintf(dot_name, sizeof(dot_name), "graph_%d.dot", id);
    snprintf(png_name, sizeof(png_name), "graph_%d.png", id);

    FILE *f = fopen(dot_name, "w");
    if (!f) return;

    fprintf(f, "graph G%d {\n", id);

    for (int64_t v = 0; v < n; ++v)
        fprintf(f, "  %ld;\n", v);

    if (col_idx) {
        for (int64_t u = 0; u < n; ++u) {
            int64_t start = row_ptr[u];
            int64_t end   = row_ptr[u + 1];
            for (int64_t idx = start; idx < end; ++idx) {
                int32_t v = col_idx[idx];
                if (u < v) {
                    fprintf(f, "  %ld -- %d;\n", u, v);
                }
            }
        }
    }

    fprintf(f, "}\n");
    fclose(f);

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "dot -Tpng \"%s\" -o \"%s\" && xdg-open \"%s\" >/dev/null 2>&1 &",
             dot_name, png_name, png_name);
    system(cmd);
}

// Helper: BFS to collect all nodes reachable from seed set
static void collect_connected_nodes(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                     bool *seeds, bool *visited, bool *result)
{
    // Simple BFS: start from all seed nodes, mark all reachable nodes
    int64_t *queue = malloc(n * sizeof(int64_t));
    int front = 0, rear = 0;

    for (int64_t i = 0; i < n; ++i) {
        if (seeds[i]) {
            queue[rear++] = i;
            visited[i] = true;
            result[i] = true;
        }
    }

    while (front < rear) {
        int64_t u = queue[front++];
        int64_t start = row_ptr[u];
        int64_t end = row_ptr[u + 1];
        for (int64_t idx = start; idx < end; ++idx) {
            int32_t v = col_idx[idx];
            if (!visited[v]) {
                visited[v] = true;
                result[v] = true;
                queue[rear++] = v;
            }
        }
    }
    free(queue);
}

static RoaringBitmap *collect_connected_bitmap(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                               int32_t seed)
{
    if (seed < 0 || seed >= n)
        return roaring_bitmap_create(64 * 1024, 8);

    bool *seeds = calloc(n, sizeof(bool));
    bool *visited = calloc(n, sizeof(bool));
    bool *result = calloc(n, sizeof(bool));
    if (!seeds || !visited || !result)
    {
        free(seeds);
        free(visited);
        free(result);
        return roaring_bitmap_create(64 * 1024, 8);
    }

    seeds[seed] = true;
    collect_connected_nodes(n, row_ptr, col_idx, seeds, visited, result);

    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; ++i)
    {
        if (result[i])
            roaring_bitmap_add(bm, (uint32_t)i);
    }

    free(seeds);
    free(visited);
    free(result);
    return bm;
}

// Build subgraph: returns new n, m, row_ptr, col_idx arrays
// Caller must free row_ptr and col_idx
static void build_subgraph_runtime(int64_t n_src, int64_t *row_ptr_src, int32_t *col_idx_src,
                                   bool *node_mask,  // which nodes to include
                                   int64_t *out_n, int64_t *out_m,
                                   int64_t **out_row_ptr, int32_t **out_col_idx)
{
    // Count nodes in mask
    int64_t new_n = 0;
    int32_t *old_to_new = malloc(n_src * sizeof(int32_t));
    if (!old_to_new) {
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = NULL;
        *out_col_idx = NULL;
        return;
    }
    
    for (int64_t i = 0; i < n_src; ++i) {
        if (node_mask[i]) {
            old_to_new[i] = new_n++;
        } else {
            old_to_new[i] = -1;
        }
    }

    // Handle empty subgraph
    if (new_n == 0) {
        free(old_to_new);
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = malloc(sizeof(int64_t));  // At least 1 element for row_ptr[0]
        *out_col_idx = NULL;
        if (*out_row_ptr) {
            (*out_row_ptr)[0] = 0;
        }
        return;
    }

    // Count edges
    int64_t new_m = 0;
    for (int64_t u = 0; u < n_src; ++u) {
        if (!node_mask[u]) continue;
        int64_t start = row_ptr_src[u];
        int64_t end = row_ptr_src[u + 1];
        for (int64_t idx = start; idx < end; ++idx) {
            int32_t v = col_idx_src[idx];
            if (node_mask[v] && u < v) {  // undirected: count once
                new_m++;
            }
        }
    }
    new_m *= 2;  // undirected

    // Allocate new CSR - always allocate row_ptr (needed even for empty graph)
    int64_t *new_row_ptr = malloc((new_n + 1) * sizeof(int64_t));
    if (!new_row_ptr) {
        free(old_to_new);
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = NULL;
        *out_col_idx = NULL;
        return;
    }
    memset(new_row_ptr, 0, (new_n + 1) * sizeof(int64_t));
    
    // Only allocate col_idx if we have edges
    int32_t *new_col_idx = NULL;
    if (new_m > 0) {
        new_col_idx = malloc(new_m * sizeof(int32_t));
        if (!new_col_idx) {
            free(new_row_ptr);
            free(old_to_new);
            *out_n = 0;
            *out_m = 0;
            *out_row_ptr = NULL;
            *out_col_idx = NULL;
            return;
        }
    }

    // Build new CSR
    for (int64_t u = 0; u < n_src; ++u) {
        if (!node_mask[u]) continue;
        int64_t u_new = old_to_new[u];
        int64_t start = row_ptr_src[u];
        int64_t end = row_ptr_src[u + 1];
        for (int64_t idx = start; idx < end; ++idx) {
            int32_t v = col_idx_src[idx];
            if (node_mask[v]) {
                new_row_ptr[u_new + 1]++;
            }
        }
    }

    for (int64_t i = 1; i <= new_n; ++i) {
        new_row_ptr[i] += new_row_ptr[i - 1];
    }

    if (new_m > 0 && new_col_idx) {
        int64_t *next = malloc((new_n + 1) * sizeof(int64_t));
        if (!next) {
            free(new_row_ptr);
            free(new_col_idx);
            free(old_to_new);
            *out_n = 0;
            *out_m = 0;
            *out_row_ptr = NULL;
            *out_col_idx = NULL;
            return;
        }
        memcpy(next, new_row_ptr, (new_n + 1) * sizeof(int64_t));

        for (int64_t u = 0; u < n_src; ++u) {
            if (!node_mask[u]) continue;
            int64_t u_new = old_to_new[u];
            int64_t start = row_ptr_src[u];
            int64_t end = row_ptr_src[u + 1];
            for (int64_t idx = start; idx < end; ++idx) {
                int32_t v = col_idx_src[idx];
                if (node_mask[v]) {
                    int32_t v_new = old_to_new[v];
                    new_col_idx[next[u_new]++] = v_new;
                    new_col_idx[next[v_new]++] = u_new;
                }
            }
        }

        free(next);
    }

    free(old_to_new);

    *out_n = new_n;
    *out_m = new_m;
    *out_row_ptr = new_row_ptr;
    *out_col_idx = new_col_idx;
}

// Evaluate condition tree and build subgraph
// For minimal version: condition_nodes is array of node IDs, condition_count is size
// For AND: intersection of connected components
// For OR: union of connected components
static bool *compute_cycle_nodes(int64_t n, int64_t *row_ptr, int32_t *col_idx)
{
    int64_t *deg = calloc(n, sizeof(int64_t));
    bool *removed = calloc(n, sizeof(bool));
    bool *in_cycle = calloc(n, sizeof(bool));
    int64_t *queue = calloc(n, sizeof(int64_t));
    if (!deg || !removed || !in_cycle || !queue)
    {
        if (deg) free(deg);
        if (removed) free(removed);
        if (in_cycle) free(in_cycle);
        if (queue) free(queue);
        return NULL;
    }

    for (int64_t i = 0; i < n; ++i)
        deg[i] = row_ptr[i + 1] - row_ptr[i];

    int64_t head = 0, tail = 0;
    for (int64_t i = 0; i < n; ++i)
    {
        if (deg[i] <= 1)
            queue[tail++] = i;
    }

    while (head < tail)
    {
        int64_t v = queue[head++];
        if (removed[v]) continue;
        removed[v] = true;
        for (int64_t p = row_ptr[v]; p < row_ptr[v + 1]; ++p)
        {
            int32_t u = col_idx[p];
            if (u < 0 || u >= n || removed[u]) continue;
            deg[u]--;
            if (deg[u] == 1)
                queue[tail++] = u;
        }
    }

    for (int64_t i = 0; i < n; ++i)
        in_cycle[i] = !removed[i];

    free(deg);
    free(removed);
    free(queue);
    return in_cycle;
}

static bool degree_match(int64_t deg, int32_t op, int32_t value)
{
    switch (op)
    {
        case 1: return deg == value;
        case 2: return deg != value;
        case 3: return deg <= value;
        case 4: return deg >= value;
        case 5: return deg < value;
        case 6: return deg > value;
        default: return true;
    }
}

static RoaringBitmap *compute_cycle_bitmap(int64_t n, int64_t *row_ptr, int32_t *col_idx)
{
    bool *cycle_nodes = compute_cycle_nodes(n, row_ptr, col_idx);
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    if (!cycle_nodes)
        return bm;

    for (int64_t i = 0; i < n; ++i)
    {
        if (cycle_nodes[i])
            roaring_bitmap_add(bm, (uint32_t)i);
    }
    free(cycle_nodes);
    return bm;
}

static RoaringBitmap *compute_degree_bitmap(int64_t n, int64_t *row_ptr,
                                            int32_t degree_op, int32_t degree_value)
{
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; ++i)
    {
        int64_t deg = row_ptr[i + 1] - row_ptr[i];
        if (degree_match(deg, degree_op, degree_value))
            roaring_bitmap_add(bm, (uint32_t)i);
    }
    return bm;
}

static void build_subgraph_from_bitmap_runtime(int64_t n_src, int64_t *row_ptr_src, int32_t *col_idx_src,
                                               RoaringBitmap *node_bitmap,
                                               int64_t *out_n, int64_t *out_m,
                                               int64_t **out_row_ptr, int32_t **out_col_idx)
{
    int64_t new_n = 0;
    int32_t *old_to_new = malloc(n_src * sizeof(int32_t));
    if (!old_to_new)
    {
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = NULL;
        *out_col_idx = NULL;
        return;
    }

    for (int64_t i = 0; i < n_src; ++i)
    {
        if (roaring_bitmap_contains(node_bitmap, (uint32_t)i))
            old_to_new[i] = new_n++;
        else
            old_to_new[i] = -1;
    }

    if (new_n == 0)
    {
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = calloc(1, sizeof(int64_t));
        *out_col_idx = NULL;
        free(old_to_new);
        return;
    }

    int64_t *new_row_ptr = calloc(new_n + 1, sizeof(int64_t));
    if (!new_row_ptr)
    {
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = NULL;
        *out_col_idx = NULL;
        free(old_to_new);
        return;
    }

    for (int64_t old_u = 0; old_u < n_src; ++old_u)
    {
        int32_t new_u = old_to_new[old_u];
        if (new_u == -1)
            continue;

        for (int64_t idx = row_ptr_src[old_u]; idx < row_ptr_src[old_u + 1]; ++idx)
        {
            int32_t old_v = col_idx_src[idx];
            if (old_v >= 0 && old_v < n_src && old_to_new[old_v] != -1)
                new_row_ptr[new_u + 1]++;
        }
    }

    for (int64_t i = 1; i <= new_n; ++i)
        new_row_ptr[i] += new_row_ptr[i - 1];

    int64_t total_edges = new_row_ptr[new_n];
    int32_t *new_col_idx = NULL;
    if (total_edges > 0)
    {
        new_col_idx = malloc(total_edges * sizeof(int32_t));
        if (!new_col_idx)
        {
            free(old_to_new);
            free(new_row_ptr);
            *out_n = 0;
            *out_m = 0;
            *out_row_ptr = NULL;
            *out_col_idx = NULL;
            return;
        }
    }

    int64_t *write_ptr = malloc(new_n * sizeof(int64_t));
    if (!write_ptr)
    {
        free(old_to_new);
        free(new_row_ptr);
        free(new_col_idx);
        *out_n = 0;
        *out_m = 0;
        *out_row_ptr = NULL;
        *out_col_idx = NULL;
        return;
    }

    for (int64_t i = 0; i < new_n; ++i)
        write_ptr[i] = new_row_ptr[i];

    for (int64_t old_u = 0; old_u < n_src; ++old_u)
    {
        int32_t new_u = old_to_new[old_u];
        if (new_u == -1)
            continue;

        for (int64_t idx = row_ptr_src[old_u]; idx < row_ptr_src[old_u + 1]; ++idx)
        {
            int32_t old_v = col_idx_src[idx];
            int32_t new_v = (old_v >= 0 && old_v < n_src) ? old_to_new[old_v] : -1;
            if (new_v != -1)
                new_col_idx[write_ptr[new_u]++] = new_v;
        }
    }

    free(old_to_new);
    free(write_ptr);

    *out_n = new_n;
    *out_m = total_edges;
    *out_row_ptr = new_row_ptr;
    *out_col_idx = new_col_idx;
}

static RoaringBitmap *build_neighbor_bitmap(int64_t start, int64_t end, int32_t *col_idx)
{
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t idx = start; idx < end; ++idx)
    {
        int32_t v = col_idx[idx];
        if (v >= 0)
            roaring_bitmap_add(bm, (uint32_t)v);
    }
    return bm;
}

static void build_union_or_intersection(int64_t n,
                                        int64_t *row_ptr1, int32_t *col_idx1,
                                        int64_t *row_ptr2, int32_t *col_idx2,
                                        bool is_union,
                                        int64_t *out_n, int64_t *out_m,
                                        int64_t **out_row_ptr, int32_t **out_col_idx)
{
    if (!out_n || !out_m || !out_row_ptr || !out_col_idx)
        return;
    *out_n = 0;
    *out_m = 0;
    *out_row_ptr = NULL;
    *out_col_idx = NULL;

    if (!row_ptr1 || !col_idx1 || !row_ptr2 || !col_idx2 || n <= 0)
        return;

    int64_t *degrees = calloc(n, sizeof(int64_t));
    if (!degrees)
        return;

    for (int64_t u = 0; u < n; ++u)
    {
        RoaringBitmap *lhs = build_neighbor_bitmap(row_ptr1[u], row_ptr1[u + 1], col_idx1);
        RoaringBitmap *rhs = build_neighbor_bitmap(row_ptr2[u], row_ptr2[u + 1], col_idx2);
        RoaringBitmap *result = NULL;

        if (is_union)
        {
            RoaringBitmap *inputs[2] = {lhs, rhs};
            result = roaring_bitmap_union(inputs, 2);
        }
        else
        {
            result = roaring_bitmap_intersect(lhs, rhs);
        }

        degrees[u] = (int64_t)roaring_bitmap_get_cardinality(result);
        roaring_bitmap_free(lhs);
        roaring_bitmap_free(rhs);
        roaring_bitmap_free(result);
    }

    int64_t *new_row_ptr = calloc(n + 1, sizeof(int64_t));
    if (!new_row_ptr)
    {
        free(degrees);
        return;
    }

    for (int64_t i = 0; i < n; ++i)
        new_row_ptr[i + 1] = new_row_ptr[i] + degrees[i];

    int64_t total = new_row_ptr[n];
    int32_t *new_col_idx = NULL;
    if (total > 0)
    {
        new_col_idx = malloc(sizeof(int32_t) * total);
        if (!new_col_idx)
        {
            free(degrees);
            free(new_row_ptr);
            return;
        }
    }

    for (int64_t u = 0; u < n; ++u)
    {
        RoaringBitmap *lhs = build_neighbor_bitmap(row_ptr1[u], row_ptr1[u + 1], col_idx1);
        RoaringBitmap *rhs = build_neighbor_bitmap(row_ptr2[u], row_ptr2[u + 1], col_idx2);
        RoaringBitmap *result = NULL;

        if (is_union)
        {
            RoaringBitmap *inputs[2] = {lhs, rhs};
            result = roaring_bitmap_union(inputs, 2);
        }
        else
        {
            result = roaring_bitmap_intersect(lhs, rhs);
        }

        uint64_t card = roaring_bitmap_get_cardinality(result);
        int64_t write = new_row_ptr[u];
        for (uint64_t idx = 0; idx < card; ++idx)
            new_col_idx[write++] = (int32_t)roaring_bitmap_get_at_index(result, (uint32_t)idx);

        roaring_bitmap_free(lhs);
        roaring_bitmap_free(rhs);
        roaring_bitmap_free(result);
    }

    free(degrees);
    *out_n = n;
    *out_m = total;
    *out_row_ptr = new_row_ptr;
    *out_col_idx = new_col_idx;
}

void graph_comprehension_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                 int32_t *token_kinds, int32_t *token_arg1, int32_t *token_arg2,
                                 int32_t token_count,
                                 int64_t *out_n, int64_t *out_m,
                                 int64_t **out_row_ptr, int32_t **out_col_idx)
{
    if (out_n) *out_n = 0;
    if (out_m) *out_m = 0;
    if (out_row_ptr) *out_row_ptr = NULL;
    if (out_col_idx) *out_col_idx = NULL;

    if (!row_ptr || !col_idx || n < 0 || !out_n || !out_m || !out_row_ptr || !out_col_idx)
        return;

    if (!token_kinds || !token_arg1 || !token_arg2 || token_count <= 0)
    {
        RoaringBitmap *all_nodes = roaring_bitmap_create(64 * 1024, 8);
        for (int64_t i = 0; i < n; ++i)
            roaring_bitmap_add(all_nodes, (uint32_t)i);
        build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, all_nodes, out_n, out_m, out_row_ptr, out_col_idx);
        roaring_bitmap_free(all_nodes);
        return;
    }

    RoaringBitmap **stack = calloc((size_t)token_count, sizeof(RoaringBitmap *));
    int32_t stack_size = 0;
    if (!stack)
        return;

    for (int32_t i = 0; i < token_count; ++i)
    {
        RoaringBitmap *value = NULL;
        switch (token_kinds[i])
        {
            case GC_CONNECTED:
                value = collect_connected_bitmap(n, row_ptr, col_idx, token_arg1[i]);
                stack[stack_size++] = value;
                break;
            case GC_CYCLE:
                value = compute_cycle_bitmap(n, row_ptr, col_idx);
                stack[stack_size++] = value;
                break;
            case GC_DEGREE:
                value = compute_degree_bitmap(n, row_ptr, token_arg1[i], token_arg2[i]);
                stack[stack_size++] = value;
                break;
            case GC_AND:
            case GC_OR:
            {
                if (stack_size < 2)
                    goto cleanup;

                RoaringBitmap *rhs = stack[--stack_size];
                RoaringBitmap *lhs = stack[--stack_size];
                if (token_kinds[i] == GC_AND)
                {
                    value = roaring_bitmap_intersect(lhs, rhs);
                }
                else
                {
                    RoaringBitmap *inputs[2] = {lhs, rhs};
                    value = roaring_bitmap_union(inputs, 2);
                }
                roaring_bitmap_free(lhs);
                roaring_bitmap_free(rhs);
                stack[stack_size++] = value;
                break;
            }
            default:
                goto cleanup;
        }
    }

    if (stack_size == 1)
        build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, stack[0], out_n, out_m, out_row_ptr, out_col_idx);

cleanup:
    for (int32_t i = 0; i < stack_size; ++i)
        roaring_bitmap_free(stack[i]);
    free(stack);
}

void graph_union_runtime(int64_t n,
                         int64_t *row_ptr1, int32_t *col_idx1,
                         int64_t *row_ptr2, int32_t *col_idx2,
                         int64_t *out_n, int64_t *out_m,
                         int64_t **out_row_ptr, int32_t **out_col_idx)
{
    if (!row_ptr1 || !col_idx1 || !row_ptr2 || !col_idx2 || n <= 0) return;
    build_union_or_intersection(n, row_ptr1, col_idx1, row_ptr2, col_idx2, true,
                                out_n, out_m, out_row_ptr, out_col_idx);
}

void graph_intersection_runtime(int64_t n,
                                int64_t *row_ptr1, int32_t *col_idx1,
                                int64_t *row_ptr2, int32_t *col_idx2,
                                int64_t *out_n, int64_t *out_m,
                                int64_t **out_row_ptr, int32_t **out_col_idx)
{
    if (!row_ptr1 || !col_idx1 || !row_ptr2 || !col_idx2 || n <= 0) return;
    build_union_or_intersection(n, row_ptr1, col_idx1, row_ptr2, col_idx2, false,
                                out_n, out_m, out_row_ptr, out_col_idx);
}

void *graph_build_node_bitmap_runtime(int64_t n)
{
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; ++i)
        roaring_bitmap_add(bm, (uint32_t)i);
    return bm;
}

void *graph_build_edge_bitmap_runtime(int64_t m)
{
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    int64_t undirected_edges = m / 2;
    for (int64_t i = 0; i < undirected_edges; ++i)
        roaring_bitmap_add(bm, (uint32_t)i);
    return bm;
}

// Sleep function: sleep for specified number of seconds
void sleep_runtime(int32_t seconds)
{
    if (seconds < 0) return;
    // Use nanosleep for more reliable sleep
    struct timespec req, rem;
    req.tv_sec = (time_t)seconds;
    req.tv_nsec = 0;
    // Loop in case of interruption
    while (nanosleep(&req, &rem) == -1) {
        req = rem;
    }
}

// Timer function: returns current time in seconds since epoch as double
double timer_runtime(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    }
    return 0.0;
}
