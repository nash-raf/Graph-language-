#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct RoaringBitmap RoaringBitmap;

extern RoaringBitmap *roaring_bitmap_create(size_t arena_size, size_t initial_containers);
extern void roaring_bitmap_free(RoaringBitmap *bm);
extern void roaring_bitmap_add(RoaringBitmap *bm, uint32_t value);
extern RoaringBitmap *roaring_bitmap_union(RoaringBitmap **bitmaps, size_t count);
extern RoaringBitmap *roaring_bitmap_intersect(RoaringBitmap *bm1, RoaringBitmap *bm2);
extern int roaring_bitmap_contains(RoaringBitmap *bm, uint32_t value);
extern uint32_t roaring_bitmap_get_at_index(RoaringBitmap *bm, uint32_t index);
extern uint64_t roaring_bitmap_get_cardinality(RoaringBitmap *bm);

void *roaring_bitmap_clone_runtime(void *raw)
{
    RoaringBitmap *bm = (RoaringBitmap *)raw;
    if (!bm)
        return roaring_bitmap_create(64 * 1024, 8);
    RoaringBitmap *inputs[1] = {bm};
    return roaring_bitmap_union(inputs, 1);
}

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
    GC_OR = 5,
    GC_EDGE_HAS = 6
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

static bool draw_has_suffix(const char *value, const char *suffix)
{
    size_t value_len = strlen(value);
    size_t suffix_len = strlen(suffix);
    return value_len >= suffix_len &&
           strcmp(value + value_len - suffix_len, suffix) == 0;
}

static int draw_compare_i32(const void *lhs, const void *rhs)
{
    int32_t a = *(const int32_t *)lhs;
    int32_t b = *(const int32_t *)rhs;
    return (a > b) - (a < b);
}

static double draw_numeric_value(const void *values, int32_t kind, int64_t index)
{
    if (kind == 2 || kind == 3)
        return ((const double *)values)[index];
    return (double)((const int32_t *)values)[index];
}

static void draw_interpolate_color(double t, char output[8])
{
    static const int stops[3][3] = {
        {68, 1, 84},
        {33, 145, 140},
        {253, 231, 37}
    };
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    int left = t < 0.5 ? 0 : 1;
    double local = t < 0.5 ? t * 2.0 : (t - 0.5) * 2.0;
    int r = (int)(stops[left][0] + local * (stops[left + 1][0] - stops[left][0]));
    int g = (int)(stops[left][1] + local * (stops[left + 1][1] - stops[left][1]));
    int b = (int)(stops[left][2] + local * (stops[left + 1][2] - stops[left][2]));
    snprintf(output, 8, "#%02x%02x%02x", r, g, b);
}

static void draw_categorical_color(int64_t category_index, char output[8])
{
    static const char *palette[] = {
        "#0072b2", "#e69f00", "#009e73", "#cc79a7",
        "#d55e00", "#56b4e9", "#f0e442", "#332288",
        "#88ccee", "#44aa99", "#aa4499", "#999933"
    };
    const int64_t palette_size = (int64_t)(sizeof(palette) / sizeof(palette[0]));
    if (category_index < palette_size)
    {
        memcpy(output, palette[category_index], 8);
        return;
    }

    uint32_t hash = (uint32_t)category_index * 2654435761u;
    int r = 48 + (int)(hash & 0xaf);
    int g = 48 + (int)((hash >> 8) & 0xaf);
    int b = 48 + (int)((hash >> 16) & 0xaf);
    snprintf(output, 8, "#%02x%02x%02x", r, g, b);
}

static int64_t draw_category_index(const int32_t *categories, int64_t count, int32_t value)
{
    int64_t low = 0;
    int64_t high = count;
    while (low < high)
    {
        int64_t middle = low + (high - low) / 2;
        if (categories[middle] < value)
            low = middle + 1;
        else
            high = middle;
    }
    return low;
}

static const char *draw_layout_engine(int32_t layout, bool directed, int64_t n)
{
    switch (layout)
    {
    case 1: return "dot";
    case 2: return "sfdp";
    case 3: return "twopi";
    case 4: return "circo";
    case 5: return "osage";
    default:
        if (directed && n <= 200)
            return "dot";
        return n <= 50 ? "neato" : "sfdp";
    }
}

void draw_graph_runtime(int64_t n, int64_t m,
                        int64_t *row_ptr, int32_t *col_idx, int32_t *weights,
                        int32_t directed, const char *output_path,
                        int32_t layout, int32_t vertex_labels,
                        void *color_values, int32_t color_kind, int64_t color_count,
                        void *size_values, int32_t size_kind, int64_t size_count,
                        int32_t edge_weight_labels)
{
    if (!output_path || !row_ptr || n < 0 || m < 0)
    {
        fprintf(stderr, "draw: invalid graph or output path\n");
        return;
    }
    if (n > 10000 || m > 100000)
    {
        fprintf(stderr,
                "draw: graph is too large (%ld vertices, %ld adjacency entries); "
                "draw a filtered subgraph with at most 10000 vertices and 100000 entries\n",
                (long)n, (long)m);
        return;
    }
    if (color_kind != 0 && (!color_values || color_count < n))
    {
        fprintf(stderr, "draw: color array has %ld values but graph has %ld vertices\n",
                (long)color_count, (long)n);
        return;
    }
    if (size_kind != 0 && (!size_values || size_count < n))
    {
        fprintf(stderr, "draw: size array has %ld values but graph has %ld vertices\n",
                (long)size_count, (long)n);
        return;
    }
    if (edge_weight_labels && !weights)
    {
        fprintf(stderr, "draw: weighted edge labels requested for an unweighted graph\n");
        return;
    }

    const char *format = NULL;
    if (draw_has_suffix(output_path, ".svg")) format = "svg";
    else if (draw_has_suffix(output_path, ".png")) format = "png";
    else if (draw_has_suffix(output_path, ".pdf")) format = "pdf";
    else if (!draw_has_suffix(output_path, ".dot"))
    {
        fprintf(stderr, "draw: unsupported output extension: %s\n", output_path);
        return;
    }

    char *dot_path = NULL;
    if (!format)
    {
        dot_path = strdup(output_path);
    }
    else
    {
        size_t path_len = strlen(output_path) + 16;
        dot_path = malloc(path_len);
        if (dot_path)
            snprintf(dot_path, path_len, "%s.grapheasy.dot", output_path);
    }
    if (!dot_path)
    {
        fprintf(stderr, "draw: could not allocate output path\n");
        return;
    }

    int32_t *categories = NULL;
    int64_t category_count = 0;
    if (color_kind == 1 && n > 0)
    {
        categories = malloc((size_t)n * sizeof(int32_t));
        if (!categories)
        {
            free(dot_path);
            fprintf(stderr, "draw: could not allocate categorical color map\n");
            return;
        }
        const int32_t *values = (const int32_t *)color_values;
        for (int64_t v = 0; v < n; ++v)
            if (values[v] >= 0)
                categories[category_count++] = values[v];
        qsort(categories, (size_t)category_count, sizeof(int32_t), draw_compare_i32);
        int64_t unique = 0;
        for (int64_t i = 0; i < category_count; ++i)
            if (unique == 0 || categories[i] != categories[unique - 1])
                categories[unique++] = categories[i];
        category_count = unique;
    }

    double color_min = 0.0;
    double color_max = 0.0;
    if ((color_kind == 2 || color_kind == 3) && n > 0)
    {
        color_min = color_max = draw_numeric_value(color_values, color_kind, 0);
        for (int64_t v = 1; v < n; ++v)
        {
            double value = draw_numeric_value(color_values, color_kind, v);
            if (value < color_min) color_min = value;
            if (value > color_max) color_max = value;
        }
    }

    double size_min = 0.0;
    double size_max = 0.0;
    if (size_kind != 0 && n > 0)
    {
        size_min = size_max = draw_numeric_value(size_values, size_kind == 2 ? 3 : 1, 0);
        for (int64_t v = 1; v < n; ++v)
        {
            double value = draw_numeric_value(size_values, size_kind == 2 ? 3 : 1, v);
            if (value < size_min) size_min = value;
            if (value > size_max) size_max = value;
        }
    }

    FILE *dot = fopen(dot_path, "w");
    if (!dot)
    {
        fprintf(stderr, "draw: cannot open %s\n", dot_path);
        free(categories);
        free(dot_path);
        return;
    }

    const bool is_directed = directed != 0;
    fprintf(dot, "%s G {\n", is_directed ? "digraph" : "graph");
    fprintf(dot, "  graph [overlap=false, splines=true, outputorder=edgesfirst, "
                 "bgcolor=\"#ffffff\", pad=0.25];\n");
    fprintf(dot, "  node [shape=circle, style=filled, fontname=\"DejaVu Sans\", "
                 "fontsize=11, color=\"#334155\", penwidth=1.2, fillcolor=\"#dbeafe\"];\n");
    fprintf(dot, "  edge [color=\"#94a3b8\", penwidth=1.2, arrowsize=0.75, "
                 "fontname=\"DejaVu Sans\", fontsize=9, fontcolor=\"#475569\"];\n");

    for (int64_t v = 0; v < n; ++v)
    {
        char fill[8] = "#dbeafe";
        if (color_kind == 1)
        {
            int32_t value = ((const int32_t *)color_values)[v];
            if (value < 0)
                memcpy(fill, "#d1d5db", 8);
            else
                draw_categorical_color(
                    draw_category_index(categories, category_count, value), fill);
        }
        else if (color_kind == 2 || color_kind == 3)
        {
            double value = draw_numeric_value(color_values, color_kind, v);
            double range = color_max - color_min;
            draw_interpolate_color(range == 0.0 ? 0.5 : (value - color_min) / range, fill);
        }

        double width = 0.52;
        if (size_kind != 0)
        {
            double value = draw_numeric_value(size_values, size_kind == 2 ? 3 : 1, v);
            double range = size_max - size_min;
            double scaled = range == 0.0 ? 0.5 : (value - size_min) / range;
            width = 0.42 + 0.68 * scaled;
        }

        fprintf(dot, "  %ld [label=\"", (long)v);
        if (vertex_labels)
            fprintf(dot, "%ld", (long)v);
        fprintf(dot, "\", fillcolor=\"%s\", width=%.3f, height=%.3f];\n",
                fill, width, width);
    }

    const char *connector = is_directed ? "->" : "--";
    for (int64_t u = 0; u < n; ++u)
    {
        for (int64_t index = row_ptr[u]; index < row_ptr[u + 1]; ++index)
        {
            int32_t v = col_idx[index];
            if (!is_directed && u > v)
                continue;
            fprintf(dot, "  %ld %s %d", (long)u, connector, v);
            if (is_directed && weights)
            {
                fprintf(dot, " [arrowhead=%s, color=\"%s\"",
                        weights[index] < 0 ? "tee" : "normal",
                        weights[index] < 0 ? "#dc2626" : "#16a34a");
                if (edge_weight_labels)
                    fprintf(dot, ", label=\"%d\"", weights[index]);
                fprintf(dot, "]");
            }
            else if (edge_weight_labels)
                fprintf(dot, " [label=\"%d\"]", weights[index]);
            fprintf(dot, ";\n");
        }
    }
    fprintf(dot, "}\n");
    fclose(dot);
    free(categories);

    if (!format)
    {
        fprintf(stderr, "draw: wrote %s\n", output_path);
        free(dot_path);
        return;
    }

    const char *engine = draw_layout_engine(layout, is_directed, n);
    char format_arg[16];
    snprintf(format_arg, sizeof(format_arg), "-T%s", format);

    pid_t child = fork();
    if (child == 0)
    {
        execlp(engine, engine, format_arg, dot_path, "-o", output_path, (char *)NULL);
        _exit(127);
    }
    if (child < 0)
    {
        fprintf(stderr, "draw: failed to start Graphviz engine %s\n", engine);
        unlink(dot_path);
        free(dot_path);
        return;
    }

    int status = 0;
    if (waitpid(child, &status, 0) < 0 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        fprintf(stderr, "draw: Graphviz engine %s failed\n", engine);
        unlink(dot_path);
        free(dot_path);
        return;
    }

    unlink(dot_path);
    fprintf(stderr, "draw: wrote %s\n", output_path);
    free(dot_path);
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

static RoaringBitmap *compute_edge_has_bitmap(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                              int32_t vertex)
{
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
    if (vertex < 0 || vertex >= n)
        return bm;

    roaring_bitmap_add(bm, (uint32_t)vertex);

    for (int64_t idx = row_ptr[vertex]; idx < row_ptr[vertex + 1]; ++idx)
    {
        int32_t nbr = col_idx[idx];
        if (nbr >= 0 && nbr < n)
            roaring_bitmap_add(bm, (uint32_t)nbr);
    }

    for (int64_t u = 0; u < n; ++u)
    {
        for (int64_t idx = row_ptr[u]; idx < row_ptr[u + 1]; ++idx)
        {
            if (col_idx[idx] == vertex)
            {
                roaring_bitmap_add(bm, (uint32_t)u);
                break;
            }
        }
    }

    return bm;
}

void *graph_edge_has_nodes_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx, int32_t vertex)
{
    return compute_edge_has_bitmap(n, row_ptr, col_idx, vertex);
}

static void build_subgraph_from_bitmap_runtime(int64_t n_src, int64_t *row_ptr_src, int32_t *col_idx_src,
                                               int32_t *weights_src,
                                               RoaringBitmap *node_bitmap,
                                               int64_t *out_n, int64_t *out_m,
                                               int64_t **out_row_ptr, int32_t **out_col_idx,
                                               int32_t **out_weights)
{
    if (out_weights)
        *out_weights = NULL;
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
    int32_t *new_weights = NULL;
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
        if (weights_src && out_weights)
        {
            new_weights = malloc(total_edges * sizeof(int32_t));
            if (!new_weights)
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
        }
    }

    int64_t *write_ptr = malloc(new_n * sizeof(int64_t));
    if (!write_ptr)
    {
        free(old_to_new);
        free(new_row_ptr);
        free(new_col_idx);
        free(new_weights);
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
            {
                int64_t write = write_ptr[new_u]++;
                new_col_idx[write] = new_v;
                if (new_weights)
                    new_weights[write] = weights_src[idx];
            }
        }
    }

    free(old_to_new);
    free(write_ptr);

    *out_n = new_n;
    *out_m = total_edges;
    *out_row_ptr = new_row_ptr;
    *out_col_idx = new_col_idx;
    if (out_weights)
        *out_weights = new_weights;
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

static int graph_directed_edge_kind_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                            int32_t *weights, int32_t u, int32_t v)
{
    if (!row_ptr || !col_idx || u < 0 || v < 0 || u >= n || v >= n)
        return 0;
    for (int64_t idx = row_ptr[u]; idx < row_ptr[u + 1]; ++idx)
    {
        if (col_idx[idx] == v)
        {
            if (!weights || weights[idx] > 0)
                return 1;
            if (weights[idx] < 0)
                return -1;
            return 2;
        }
    }
    return 0;
}

static void motif_backtrack_runtime(int32_t pos, int32_t var_count,
                                    int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                    int32_t *weights, int8_t *required, int32_t *assignment,
                                    uint8_t *used, RoaringBitmap *matched)
{
    if (pos == var_count)
    {
        for (int32_t i = 0; i < var_count; ++i)
            roaring_bitmap_add(matched, (uint32_t)assignment[i]);
        return;
    }

    for (int64_t candidate = 0; candidate < n; ++candidate)
    {
        if (used[candidate])
            continue;

        int ok = 1;
        for (int32_t prev = 0; prev < pos && ok; ++prev)
        {
            int32_t prev_vertex = assignment[prev];
            int actual_forward = graph_directed_edge_kind_runtime(
                n, row_ptr, col_idx, weights, (int32_t)candidate, prev_vertex);
            int actual_backward = graph_directed_edge_kind_runtime(
                n, row_ptr, col_idx, weights, prev_vertex, (int32_t)candidate);
            int required_forward = required[pos * var_count + prev];
            int required_backward = required[prev * var_count + pos];
            if (actual_forward != required_forward || actual_backward != required_backward)
                ok = 0;
        }
        if (!ok)
            continue;

        assignment[pos] = (int32_t)candidate;
        used[candidate] = 1;
        motif_backtrack_runtime(pos + 1, var_count, n, row_ptr, col_idx,
                                weights, required, assignment, used, matched);
        used[candidate] = 0;
        assignment[pos] = -1;
    }
}

void graph_motif_filter_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                int32_t *weights,
                                int32_t *edge_src_vars, int32_t *edge_dst_vars,
                                int32_t *edge_signs,
                                int32_t edge_count, int32_t var_count,
                                int64_t *out_n, int64_t *out_m,
                                int64_t **out_row_ptr, int32_t **out_col_idx,
                                int32_t **out_weights)
{
    if (out_n) *out_n = 0;
    if (out_m) *out_m = 0;
    if (out_row_ptr) *out_row_ptr = NULL;
    if (out_col_idx) *out_col_idx = NULL;
    if (out_weights) *out_weights = NULL;

    if (!row_ptr || !col_idx || !edge_src_vars || !edge_dst_vars || !edge_signs || n < 0 ||
        edge_count <= 0 || var_count <= 1 || var_count > 6 ||
        !out_n || !out_m || !out_row_ptr || !out_col_idx || !out_weights)
        return;

    int8_t *required = calloc((size_t)var_count * (size_t)var_count, sizeof(int8_t));
    int32_t *assignment = malloc((size_t)var_count * sizeof(int32_t));
    uint8_t *used = calloc((size_t)n, sizeof(uint8_t));
    RoaringBitmap *matched = roaring_bitmap_create(64 * 1024, 8);
    if (!required || !assignment || !used || !matched)
    {
        free(required);
        free(assignment);
        free(used);
        if (matched)
            roaring_bitmap_free(matched);
        return;
    }

    for (int32_t i = 0; i < var_count; ++i)
        assignment[i] = -1;

    for (int32_t i = 0; i < edge_count; ++i)
    {
        int32_t src = edge_src_vars[i];
        int32_t dst = edge_dst_vars[i];
        if (src >= 0 && src < var_count && dst >= 0 && dst < var_count && src != dst)
            required[src * var_count + dst] = edge_signs[i] < 0 ? -1 : 1;
    }

    motif_backtrack_runtime(0, var_count, n, row_ptr, col_idx,
                            weights, required, assignment, used, matched);
    build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, weights, matched,
                                       out_n, out_m, out_row_ptr, out_col_idx,
                                       out_weights);

    roaring_bitmap_free(matched);
    free(required);
    free(assignment);
    free(used);
}

typedef struct MotifMatchesRuntime
{
    int32_t count;
    int32_t var_count;
    int32_t *bindings;
    int32_t capacity;
} MotifMatchesRuntime;

typedef struct GraphRuntimeObject
{
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
    int32_t directed;
    int64_t *in_row_ptr;
    int32_t *in_col_idx;
} GraphRuntimeObject;

void *graph_from_match_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                               int32_t *weights, int32_t directed,
                               int32_t *binding, int32_t var_count)
{
    if (!row_ptr || !col_idx || !binding || var_count <= 0 || var_count > 6)
        return NULL;

    RoaringBitmap *nodes = roaring_bitmap_create(64 * 1024, 8);
    if (!nodes)
        return NULL;
    for (int32_t i = 0; i < var_count; ++i)
    {
        if (binding[i] >= 0)
            roaring_bitmap_add(nodes, (uint32_t)binding[i]);
    }

    GraphRuntimeObject *graph = calloc(1, sizeof(GraphRuntimeObject));
    if (!graph)
    {
        roaring_bitmap_free(nodes);
        return NULL;
    }

    build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, weights, nodes,
                                       &graph->n, &graph->m,
                                       &graph->row_ptr, &graph->col_idx,
                                       &graph->weights);
    graph->directed = directed;
    /* Previously aliased to the forward CSR, which made reverse traversal on a
       per-match graph silently wrong for directed graphs. NULL means "transpose
       not built"; graph_ensure_in_csr() builds it on first use. */
    graph->in_row_ptr = NULL;
    graph->in_col_idx = NULL;
    roaring_bitmap_free(nodes);
    return graph;
}

static int motif_matches_append_runtime(MotifMatchesRuntime *matches, int32_t *assignment)
{
    if (matches->count == matches->capacity)
    {
        int32_t next_capacity = matches->capacity == 0 ? 16 : matches->capacity * 2;
        size_t values = (size_t)next_capacity * (size_t)matches->var_count;
        int32_t *next = realloc(matches->bindings, values * sizeof(int32_t));
        if (!next)
            return 0;
        matches->bindings = next;
        matches->capacity = next_capacity;
    }

    memcpy(matches->bindings + (size_t)matches->count * matches->var_count,
           assignment, (size_t)matches->var_count * sizeof(int32_t));
    matches->count++;
    return 1;
}

static int motif_permutation_preserves_runtime(const int8_t *required,
                                               const int32_t *permutation,
                                               int32_t var_count)
{
    for (int32_t i = 0; i < var_count; ++i)
        for (int32_t j = 0; j < var_count; ++j)
            if (required[i * var_count + j] !=
                required[permutation[i] * var_count + permutation[j]])
                return 0;
    return 1;
}

static int motif_permutation_is_smaller_runtime(const int32_t *assignment,
                                                const int32_t *permutation,
                                                int32_t var_count)
{
    for (int32_t i = 0; i < var_count; ++i)
    {
        int32_t permuted = assignment[permutation[i]];
        if (permuted < assignment[i])
            return 1;
        if (permuted > assignment[i])
            return 0;
    }
    return 0;
}

static int motif_has_smaller_automorphism_runtime(int32_t pos, int32_t var_count,
                                                  const int8_t *required,
                                                  const int32_t *assignment,
                                                  int32_t *permutation, uint8_t *used)
{
    if (pos == var_count)
        return motif_permutation_preserves_runtime(required, permutation, var_count) &&
               motif_permutation_is_smaller_runtime(assignment, permutation, var_count);

    for (int32_t candidate = 0; candidate < var_count; ++candidate)
    {
        if (used[candidate])
            continue;
        permutation[pos] = candidate;
        used[candidate] = 1;
        if (motif_has_smaller_automorphism_runtime(pos + 1, var_count, required,
                                                   assignment, permutation, used))
            return 1;
        used[candidate] = 0;
    }
    return 0;
}

static int motif_assignment_is_canonical_runtime(int32_t var_count,
                                                 const int8_t *required,
                                                 const int32_t *assignment)
{
    int32_t permutation[6] = {0};
    uint8_t used[6] = {0};
    return !motif_has_smaller_automorphism_runtime(0, var_count, required,
                                                   assignment, permutation, used);
}

static int motif_collect_backtrack_runtime(int32_t pos, int32_t var_count,
                                           int64_t n, int64_t *row_ptr,
                                           int32_t *col_idx, int32_t *weights,
                                           int8_t *required, int32_t *assignment,
                                           uint8_t *used, MotifMatchesRuntime *matches)
{
    if (pos == var_count)
    {
        if (!motif_assignment_is_canonical_runtime(var_count, required, assignment))
            return 1;
        return motif_matches_append_runtime(matches, assignment);
    }

    for (int64_t candidate = 0; candidate < n; ++candidate)
    {
        if (used[candidate])
            continue;

        int ok = 1;
        for (int32_t prev = 0; prev < pos && ok; ++prev)
        {
            int32_t prev_vertex = assignment[prev];
            int actual_forward = graph_directed_edge_kind_runtime(
                n, row_ptr, col_idx, weights, (int32_t)candidate, prev_vertex);
            int actual_backward = graph_directed_edge_kind_runtime(
                n, row_ptr, col_idx, weights, prev_vertex, (int32_t)candidate);
            if (actual_forward != required[pos * var_count + prev] ||
                actual_backward != required[prev * var_count + pos])
                ok = 0;
        }
        if (!ok)
            continue;

        assignment[pos] = (int32_t)candidate;
        used[candidate] = 1;
        if (!motif_collect_backtrack_runtime(pos + 1, var_count, n, row_ptr,
                                             col_idx, weights, required, assignment,
                                             used, matches))
            return 0;
        used[candidate] = 0;
        assignment[pos] = -1;
    }
    return 1;
}

void *motif_find_matches_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                 int32_t *weights, int32_t *edge_src_vars,
                                 int32_t *edge_dst_vars, int32_t *edge_signs,
                                 int32_t edge_count, int32_t var_count)
{
    MotifMatchesRuntime *matches = calloc(1, sizeof(MotifMatchesRuntime));
    if (!matches)
        return NULL;
    matches->var_count = var_count;

    if (!row_ptr || !col_idx || !edge_src_vars || !edge_dst_vars || !edge_signs ||
        n < 0 || edge_count <= 0 || var_count <= 1 || var_count > 6)
        return matches;

    int8_t *required = calloc((size_t)var_count * var_count, sizeof(int8_t));
    int32_t *assignment = malloc((size_t)var_count * sizeof(int32_t));
    uint8_t *used = calloc((size_t)n, sizeof(uint8_t));
    if (!required || !assignment || !used)
    {
        free(required);
        free(assignment);
        free(used);
        return matches;
    }

    for (int32_t i = 0; i < var_count; ++i)
        assignment[i] = -1;
    for (int32_t i = 0; i < edge_count; ++i)
    {
        int32_t src = edge_src_vars[i];
        int32_t dst = edge_dst_vars[i];
        if (src >= 0 && src < var_count && dst >= 0 && dst < var_count && src != dst)
            required[src * var_count + dst] = edge_signs[i] < 0 ? -1 : 1;
    }

    motif_collect_backtrack_runtime(0, var_count, n, row_ptr, col_idx, weights,
                                    required, assignment, used, matches);
    free(required);
    free(assignment);
    free(used);
    return matches;
}

static void draw_motif_parse_names(const char *csv, int32_t var_count, char names[6][32])
{
    for (int32_t i = 0; i < var_count && i < 6; ++i)
        snprintf(names[i], 32, "v%d", i);
    if (!csv)
        return;

    int32_t idx = 0;
    int32_t pos = 0;
    for (const char *p = csv; *p && idx < var_count && idx < 6; ++p)
    {
        if (*p == ',')
        {
            names[idx][pos] = '\0';
            idx++;
            pos = 0;
            continue;
        }
        if (pos < 31)
            names[idx][pos++] = *p;
    }
    if (idx < var_count && idx < 6)
        names[idx][pos] = '\0';
}

static void draw_motif_base_and_format(const char *output_prefix,
                                       char *base, size_t base_size,
                                       char *format, size_t format_size)
{
    const char *input = (output_prefix && output_prefix[0]) ? output_prefix : "motif_match";
    snprintf(base, base_size, "%s", input);
    snprintf(format, format_size, "png");

    char *dot = strrchr(base, '.');
    if (!dot)
        return;
    if (strcmp(dot, ".png") == 0 || strcmp(dot, ".svg") == 0 ||
        strcmp(dot, ".pdf") == 0)
    {
        snprintf(format, format_size, "%s", dot + 1);
        *dot = '\0';
    }
}

static void draw_single_motif_match_runtime(const char *base, const char *format,
                                            const char *engine, int32_t match_index,
                                            int32_t *assignment, int32_t var_count,
                                            int32_t *edge_src_vars, int32_t *edge_dst_vars,
                                            int32_t *edge_signs, int32_t edge_count, int32_t vertex_labels,
                                            int32_t edge_labels, char names[6][32])
{
    char dot_path[1024];
    char output_path[1024];
    snprintf(dot_path, sizeof(dot_path), "%s_%03d.dot", base, match_index);
    snprintf(output_path, sizeof(output_path), "%s_%03d.%s", base, match_index, format);

    FILE *dot = fopen(dot_path, "w");
    if (!dot)
    {
        fprintf(stderr, "draw motif: cannot open %s\n", dot_path);
        return;
    }

    fprintf(dot, "digraph G {\n");
    fprintf(dot, "  graph [overlap=false, splines=true, outputorder=edgesfirst, bgcolor=\"white\"];\n");
    fprintf(dot, "  node [shape=circle, style=filled, fontname=\"DejaVu Sans\", fontsize=12, fillcolor=\"#dbeafe\", color=\"#1e3a5f\", penwidth=1.5];\n");
    fprintf(dot, "  edge [color=\"#64748b\", penwidth=1.4, arrowsize=0.75, fontname=\"DejaVu Sans\", fontsize=10];\n");

    for (int32_t i = 0; i < var_count; ++i)
    {
        fprintf(dot, "  %d [label=\"", i);
        if (vertex_labels)
            fprintf(dot, "%s=%d", names[i], assignment[i]);
        fprintf(dot, "\"];\n");
    }

    for (int32_t e = 0; e < edge_count; ++e)
    {
        int32_t src = edge_src_vars[e];
        int32_t dst = edge_dst_vars[e];
        fprintf(dot, "  %d -> %d", src, dst);
        fprintf(dot, " [");
        if (edge_signs[e] < 0)
            fprintf(dot, "arrowhead=tee, color=\"#dc2626\"");
        else
            fprintf(dot, "arrowhead=normal, color=\"#16a34a\"");
        if (edge_labels)
            fprintf(dot, ", label=\"%s%s%s\"", names[src],
                    edge_signs[e] < 0 ? "-|" : "->", names[dst]);
        fprintf(dot, "]");
        fprintf(dot, ";\n");
    }

    fprintf(dot, "}\n");
    fclose(dot);

    char format_arg[32];
    snprintf(format_arg, sizeof(format_arg), "-T%s", format);
    pid_t pid = fork();
    if (pid == 0)
    {
        execlp(engine, engine, format_arg, dot_path, "-o", output_path, (char *)NULL);
        _exit(127);
    }
    if (pid < 0)
    {
        fprintf(stderr, "draw motif: failed to start Graphviz engine %s\n", engine);
        unlink(dot_path);
        return;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        fprintf(stderr, "draw motif: Graphviz engine %s failed for match %d\n", engine, match_index);
        unlink(dot_path);
        return;
    }

    unlink(dot_path);
    printf("draw motif: wrote %s\n", output_path);
}

static void draw_combined_motif_match_cluster_runtime(FILE *dot, int32_t match_index,
                                                      int32_t *assignment, int32_t var_count,
                                                      int32_t *edge_src_vars, int32_t *edge_dst_vars,
                                                      int32_t *edge_signs, int32_t edge_count, int32_t vertex_labels,
                                                      int32_t edge_labels, char names[6][32])
{
    fprintf(dot, "  subgraph cluster_%d {\n", match_index);
    fprintf(dot, "    label=\"match %d\";\n", match_index);
    fprintf(dot, "    color=\"#cbd5e1\";\n");
    fprintf(dot, "    penwidth=1.2;\n");
    fprintf(dot, "    style=\"rounded\";\n");

    for (int32_t i = 0; i < var_count; ++i)
    {
        fprintf(dot, "    m%d_%d [label=\"", match_index, i);
        if (vertex_labels)
            fprintf(dot, "%s=%d", names[i], assignment[i]);
        fprintf(dot, "\"];\n");
    }

    for (int32_t e = 0; e < edge_count; ++e)
    {
        int32_t src = edge_src_vars[e];
        int32_t dst = edge_dst_vars[e];
        fprintf(dot, "    m%d_%d -> m%d_%d", match_index, src, match_index, dst);
        fprintf(dot, " [");
        if (edge_signs[e] < 0)
            fprintf(dot, "arrowhead=tee, color=\"#dc2626\"");
        else
            fprintf(dot, "arrowhead=normal, color=\"#16a34a\"");
        if (edge_labels)
            fprintf(dot, ", label=\"%s%s%s\"", names[src],
                    edge_signs[e] < 0 ? "-|" : "->", names[dst]);
        fprintf(dot, "]");
        fprintf(dot, ";\n");
    }

    fprintf(dot, "  }\n");
}

static void draw_motif_backtrack_runtime(int32_t pos, int32_t var_count,
                                         int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                         int32_t *weights, int8_t *required, int32_t *assignment,
                                         uint8_t *used, int32_t *match_count,
                                         FILE *manifest, const char *base,
                                         const char *format, const char *engine,
                                         int32_t *edge_src_vars, int32_t *edge_dst_vars,
                                         int32_t *edge_signs, int32_t edge_count, int32_t vertex_labels,
                                         int32_t edge_labels, char names[6][32],
                                         FILE *combined_dot)
{
    if (pos == var_count)
    {
        if (!motif_assignment_is_canonical_runtime(var_count, required, assignment))
            return;
        int32_t match_index = *match_count;
        fprintf(manifest, "match %d:", match_index);
        for (int32_t i = 0; i < var_count; ++i)
            fprintf(manifest, " %s=%d", names[i], assignment[i]);
        fprintf(manifest, "\n");

        if (combined_dot)
            draw_combined_motif_match_cluster_runtime(combined_dot, match_index,
                                                     assignment, var_count,
                                                     edge_src_vars, edge_dst_vars, edge_signs, edge_count,
                                                     vertex_labels, edge_labels, names);
        else
            draw_single_motif_match_runtime(base, format, engine, match_index,
                                            assignment, var_count,
                                            edge_src_vars, edge_dst_vars, edge_signs, edge_count,
                                            vertex_labels, edge_labels, names);
        (*match_count)++;
        return;
    }

    for (int64_t candidate = 0; candidate < n; ++candidate)
    {
        if (used[candidate])
            continue;

        int ok = 1;
        for (int32_t prev = 0; prev < pos && ok; ++prev)
        {
            int32_t prev_vertex = assignment[prev];
            int actual_forward = graph_directed_edge_kind_runtime(
                n, row_ptr, col_idx, weights, (int32_t)candidate, prev_vertex);
            int actual_backward = graph_directed_edge_kind_runtime(
                n, row_ptr, col_idx, weights, prev_vertex, (int32_t)candidate);
            int required_forward = required[pos * var_count + prev];
            int required_backward = required[prev * var_count + pos];
            if (actual_forward != required_forward || actual_backward != required_backward)
                ok = 0;
        }
        if (!ok)
            continue;

        assignment[pos] = (int32_t)candidate;
        used[candidate] = 1;
        draw_motif_backtrack_runtime(pos + 1, var_count, n, row_ptr, col_idx,
                                     weights, required, assignment, used, match_count,
                                     manifest, base, format, engine,
                                     edge_src_vars, edge_dst_vars, edge_signs, edge_count,
                                     vertex_labels, edge_labels, names,
                                     combined_dot);
        used[candidate] = 0;
        assignment[pos] = -1;
    }
}

void draw_motif_matches_runtime(int64_t n, int64_t m,
                                int64_t *row_ptr, int32_t *col_idx,
                                int32_t *weights, int32_t directed, const char *output_prefix,
                                int32_t layout, int32_t vertex_labels,
                                int32_t edge_labels, int32_t combined_image,
                                int32_t *edge_src_vars, int32_t *edge_dst_vars,
                                int32_t *edge_signs,
                                int32_t edge_count, int32_t var_count,
                                const char *var_names_csv)
{
    (void)m;
    if (!row_ptr || !col_idx || !edge_src_vars || !edge_dst_vars || !edge_signs ||
        edge_count <= 0 || var_count <= 1 || var_count > 6 || n < 0)
        return;

    int8_t *required = calloc((size_t)var_count * (size_t)var_count, sizeof(int8_t));
    int32_t *assignment = malloc((size_t)var_count * sizeof(int32_t));
    uint8_t *used = calloc((size_t)n, sizeof(uint8_t));
    if (!required || !assignment || !used)
    {
        free(required);
        free(assignment);
        free(used);
        return;
    }

    for (int32_t i = 0; i < var_count; ++i)
        assignment[i] = -1;
    for (int32_t i = 0; i < edge_count; ++i)
    {
        int32_t src = edge_src_vars[i];
        int32_t dst = edge_dst_vars[i];
        if (src >= 0 && src < var_count && dst >= 0 && dst < var_count && src != dst)
            required[src * var_count + dst] = edge_signs[i] < 0 ? -1 : 1;
    }

    char names[6][32];
    draw_motif_parse_names(var_names_csv, var_count, names);

    char base[512];
    char format[16];
    draw_motif_base_and_format(output_prefix, base, sizeof(base), format, sizeof(format));
    const char *engine = draw_layout_engine(layout, directed != 0, var_count);

    char manifest_path[1024];
    snprintf(manifest_path, sizeof(manifest_path), "%s_manifest.txt", base);
    FILE *manifest = fopen(manifest_path, "w");
    if (!manifest)
    {
        fprintf(stderr, "draw motif: cannot open %s\n", manifest_path);
        free(required);
        free(assignment);
        free(used);
        return;
    }

    fprintf(manifest, "motif variables:");
    for (int32_t i = 0; i < var_count; ++i)
        fprintf(manifest, " %s", names[i]);
    fprintf(manifest, "\n");
    fprintf(manifest, "motif edges:");
    for (int32_t i = 0; i < edge_count; ++i)
        fprintf(manifest, " %s%s%s", names[edge_src_vars[i]],
                edge_signs[i] < 0 ? "-|" : "->", names[edge_dst_vars[i]]);
    fprintf(manifest, "\n");

    FILE *combined_dot = NULL;
    char combined_dot_path[1024];
    char combined_output_path[1024];
    if (combined_image)
    {
        snprintf(combined_dot_path, sizeof(combined_dot_path), "%s.dot", base);
        snprintf(combined_output_path, sizeof(combined_output_path), "%s.%s", base, format);
        combined_dot = fopen(combined_dot_path, "w");
        if (!combined_dot)
        {
            fprintf(stderr, "draw motif: cannot open %s\n", combined_dot_path);
            fclose(manifest);
            free(required);
            free(assignment);
            free(used);
            return;
        }
        fprintf(combined_dot, "digraph G {\n");
        fprintf(combined_dot, "  graph [compound=true, overlap=false, splines=true, outputorder=edgesfirst, bgcolor=\"white\", rankdir=TB];\n");
        fprintf(combined_dot, "  node [shape=circle, style=filled, fontname=\"DejaVu Sans\", fontsize=12, fillcolor=\"#dbeafe\", color=\"#1e3a5f\", penwidth=1.5];\n");
        fprintf(combined_dot, "  edge [color=\"#64748b\", penwidth=1.4, arrowsize=0.75, fontname=\"DejaVu Sans\", fontsize=10];\n");
    }

    int32_t match_count = 0;
    draw_motif_backtrack_runtime(0, var_count, n, row_ptr, col_idx,
                                 weights, required, assignment, used, &match_count,
                                 manifest, base, format, engine,
                                 edge_src_vars, edge_dst_vars, edge_signs, edge_count,
                                 vertex_labels, edge_labels, names,
                                 combined_dot);
    if (combined_dot)
    {
        fprintf(combined_dot, "}\n");
        fclose(combined_dot);

        char format_arg[32];
        snprintf(format_arg, sizeof(format_arg), "-T%s", format);
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp(engine, engine, format_arg, combined_dot_path, "-o", combined_output_path, (char *)NULL);
            _exit(127);
        }
        if (pid < 0)
        {
            fprintf(stderr, "draw motif: failed to start Graphviz engine %s\n", engine);
        }
        else
        {
            int status = 0;
            if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
                fprintf(stderr, "draw motif: Graphviz engine %s failed for combined image\n", engine);
            else
                printf("draw motifs: wrote %s\n", combined_output_path);
        }
        unlink(combined_dot_path);
    }
    fprintf(manifest, "total_matches: %d\n", match_count);
    fclose(manifest);
    printf("draw motif: wrote %s\n", manifest_path);

    free(required);
    free(assignment);
    free(used);
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
        build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, NULL, all_nodes,
                                           out_n, out_m, out_row_ptr, out_col_idx, NULL);
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
            case GC_EDGE_HAS:
                value = compute_edge_has_bitmap(n, row_ptr, col_idx, token_arg1[i]);
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
        build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, NULL, stack[0],
                                           out_n, out_m, out_row_ptr, out_col_idx, NULL);

cleanup:
    for (int32_t i = 0; i < stack_size; ++i)
        roaring_bitmap_free(stack[i]);
    free(stack);
}

void graph_filter_vertex_set_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx,
                                     void *vertex_set,
                                     int64_t *out_n, int64_t *out_m,
                                     int64_t **out_row_ptr, int32_t **out_col_idx)
{
    build_subgraph_from_bitmap_runtime(n, row_ptr, col_idx, NULL,
                                       (RoaringBitmap *)vertex_set,
                                       out_n, out_m, out_row_ptr, out_col_idx, NULL);
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
