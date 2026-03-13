#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include "roaring_bitmap.h"

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
};

struct GraphExtra {
    int32_t *edge_pairs;
    int64_t num_edge_pairs;
    RoaringBitmap *node_bitmap;
    RoaringBitmap *edge_bitmap;
};

static std::vector<GraphExtra> g_extras;
static std::vector<Graph *> g_graph_registry;

static int find_graph_index(Graph *g) {
    for (size_t i = 0; i < g_graph_registry.size(); i++)
        if (g_graph_registry[i] == g) return (int)i;
    return -1;
}

extern "C" Graph *load_graph_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open graph file '%s'\n", filename);
        exit(1);
    }

    std::vector<std::pair<int,int>> edges;
    int u, v;
    int max_id = -1;
    while (fscanf(f, "%d %d%*[^\n]", &u, &v) == 2) {
        edges.push_back({u, v});
        if (u > max_id) max_id = u;
        if (v > max_id) max_id = v;
    }
    fclose(f);

    int64_t n = (max_id >= 0) ? (int64_t)(max_id + 1) : 0;
    int64_t m = 2 * (int64_t)edges.size();

    int64_t *row_ptr = (int64_t *)calloc(n + 1, sizeof(int64_t));
    for (auto &e : edges) {
        row_ptr[e.first + 1]++;
        row_ptr[e.second + 1]++;
    }
    for (int64_t i = 1; i <= n; i++)
        row_ptr[i] += row_ptr[i - 1];

    int32_t *col_idx = (int32_t *)malloc(m * sizeof(int32_t));
    int64_t *next = (int64_t *)malloc((n + 1) * sizeof(int64_t));
    memcpy(next, row_ptr, (n + 1) * sizeof(int64_t));
    for (auto &e : edges) {
        col_idx[next[e.first]++]  = (int32_t)e.second;
        col_idx[next[e.second]++] = (int32_t)e.first;
    }
    free(next);

    Graph *g = (Graph *)malloc(sizeof(Graph));
    g->n = n;
    g->m = m;
    g->row_ptr = row_ptr;
    g->col_idx = col_idx;

    GraphExtra extra;
    extra.num_edge_pairs = (int64_t)edges.size();
    extra.edge_pairs = (int32_t *)malloc(edges.size() * 2 * sizeof(int32_t));
    for (size_t i = 0; i < edges.size(); i++) {
        extra.edge_pairs[i * 2]     = (int32_t)edges[i].first;
        extra.edge_pairs[i * 2 + 1] = (int32_t)edges[i].second;
    }

    extra.node_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; i++)
        roaring_bitmap_add(extra.node_bitmap, (uint32_t)i);

    extra.edge_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (uint32_t eid = 0; eid < (uint32_t)edges.size(); eid++)
        roaring_bitmap_add(extra.edge_bitmap, eid);

    g_graph_registry.push_back(g);
    g_extras.push_back(extra);

    return g;
}

extern "C" void *graph_get_node_bitmap(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return (void *)g_extras[idx].node_bitmap;
}

extern "C" void *graph_get_edge_bitmap(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return (void *)g_extras[idx].edge_bitmap;
}

extern "C" int32_t *graph_get_edge_pairs(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return g_extras[idx].edge_pairs;
}

extern "C" int64_t graph_get_num_edge_pairs(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return 0;
    return g_extras[idx].num_edge_pairs;
}
