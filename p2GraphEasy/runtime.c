#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

char* concat_strings(const char* a, const char* b)
{
    char* result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

static int show_counter = 0;


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
void graph_comprehension_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                 int32_t *condition_nodes, int32_t condition_count,
                                 int32_t is_and,  // 1 for AND, 0 for OR
                                 int64_t *out_n, int64_t *out_m,
                                 int64_t **out_row_ptr, int32_t **out_col_idx)
{
    // Initialize output parameters
    if (out_n) *out_n = 0;
    if (out_m) *out_m = 0;
    if (out_row_ptr) *out_row_ptr = NULL;
    if (out_col_idx) *out_col_idx = NULL;
    
    if (!row_ptr || !col_idx || !condition_nodes || condition_count <= 0 || n <= 0) {
        return;
    }
    
    bool *seeds = calloc(n, sizeof(bool));
    bool *visited = calloc(n, sizeof(bool));
    bool *result = calloc(n, sizeof(bool));
    
    if (!seeds || !visited || !result) {
        if (seeds) free(seeds);
        if (visited) free(visited);
        if (result) free(result);
        return;
    }

    if (is_and) {
        // AND: intersection - start with all nodes, then remove those not connected to each seed
        for (int64_t i = 0; i < n; ++i) {
            result[i] = true;
        }
        
        for (int32_t i = 0; i < condition_count; ++i) {
            int32_t seed = condition_nodes[i];
            if (seed >= 0 && seed < n) {
                bool *component = calloc(n, sizeof(bool));
                bool *comp_visited = calloc(n, sizeof(bool));
                bool *seed_array = calloc(n, sizeof(bool));
                seed_array[seed] = true;
                collect_connected_nodes(n, row_ptr, col_idx, seed_array, comp_visited, component);
                
                // Intersection: keep only nodes in both result and component
                for (int64_t j = 0; j < n; ++j) {
                    result[j] = result[j] && component[j];
                }
                
                free(component);
                free(comp_visited);
                free(seed_array);
            }
        }
    } else {
        // OR: union - collect all nodes connected to any seed
        for (int32_t i = 0; i < condition_count; ++i) {
            int32_t seed = condition_nodes[i];
            if (seed >= 0 && seed < n) {
                seeds[seed] = true;
            }
        }
        
        // BFS from all seeds
        collect_connected_nodes(n, row_ptr, col_idx, seeds, visited, result);
    }

    // Build subgraph
    build_subgraph_runtime(n, row_ptr, col_idx, result, out_n, out_m, out_row_ptr, out_col_idx);

    free(seeds);
    free(visited);
    free(result);
}