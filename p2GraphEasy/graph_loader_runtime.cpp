#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <sys/stat.h>
#include <unordered_set>
#include <vector>

#include "roaring_bitmap.h"

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
};

struct GraphExtra {
    int32_t *edge_pairs = nullptr;
    int32_t *edge_weights = nullptr;
    int64_t num_edge_pairs = 0;
    RoaringBitmap *node_bitmap = nullptr;
    RoaringBitmap *edge_bitmap = nullptr;
};

struct ParsedEdge {
    int32_t u;
    int32_t v;
    int32_t w;
};

struct ParsedEdgeUW {
    int32_t u;
    int32_t v;
};

static std::vector<GraphExtra> g_extras;
static std::vector<Graph *> g_graph_registry;

static long current_rss_kb() {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return -1;

    char line[256];
    long rss_kb = -1;
    while (fgets(line, sizeof line, f)) {
        if (sscanf(line, "VmRSS: %ld kB", &rss_kb) == 1)
            break;
    }
    fclose(f);
    return rss_kb;
}

static void print_checkpoint(const char *phase) {
    long rss_kb = current_rss_kb();
    if (rss_kb < 0) {
        fprintf(stderr, "[loader-stdio] %s | VmRSS: n/a\n", phase);
        return;
    }
    fprintf(stderr, "[loader-stdio] %s | VmRSS: %ld KiB (%.2f MiB)\n",
            phase, rss_kb, rss_kb / 1024.0);
}

static void print_estimates(
    size_t file_bytes,
    size_t parsed_edges_bytes,
    size_t row_ptr_bytes,
    size_t col_idx_bytes,
    size_t weights_bytes,
    size_t edge_pairs_bytes,
    size_t edge_weights_bytes)
{
    size_t csr_total = row_ptr_bytes + col_idx_bytes + weights_bytes;
    size_t extra_total = edge_pairs_bytes + edge_weights_bytes;
    size_t total_known = file_bytes + parsed_edges_bytes + csr_total + extra_total;

    fprintf(stderr,
            "[loader-stdio] estimates | file=%.2f MiB parsed=%.2f MiB csr=%.2f MiB "
            "(row_ptr=%.2f col_idx=%.2f weights=%.2f) extra_no_bitmap=%.2f MiB "
            "(pairs=%.2f eweights=%.2f) known_total=%.2f MiB\n",
            file_bytes / (1024.0 * 1024.0),
            parsed_edges_bytes / (1024.0 * 1024.0),
            csr_total / (1024.0 * 1024.0),
            row_ptr_bytes / (1024.0 * 1024.0),
            col_idx_bytes / (1024.0 * 1024.0),
            weights_bytes / (1024.0 * 1024.0),
            extra_total / (1024.0 * 1024.0),
            edge_pairs_bytes / (1024.0 * 1024.0),
            edge_weights_bytes / (1024.0 * 1024.0),
            total_known / (1024.0 * 1024.0));
}


static uint64_t pack_undirected_edge(int32_t u, int32_t v) {
    uint32_t a = static_cast<uint32_t>(std::min(u, v));
    uint32_t b = static_cast<uint32_t>(std::max(u, v));
    return (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
}

static void free_graph_extra(GraphExtra &extra) {
    free(extra.edge_pairs);
    free(extra.edge_weights);
    if (extra.node_bitmap)
        roaring_bitmap_free(extra.node_bitmap);
    if (extra.edge_bitmap)
        roaring_bitmap_free(extra.edge_bitmap);

    extra.edge_pairs = nullptr;
    extra.edge_weights = nullptr;
    extra.num_edge_pairs = 0;
    extra.node_bitmap = nullptr;
    extra.edge_bitmap = nullptr;
}

static int find_graph_index(Graph *g) {
    for (size_t i = 0; i < g_graph_registry.size(); i++)
        if (g_graph_registry[i] == g) return static_cast<int>(i);
    return -1;
}

static void register_graph_extra(Graph *g, GraphExtra extra) {
    int idx = find_graph_index(g);
    if (idx >= 0) {
        free_graph_extra(g_extras[static_cast<size_t>(idx)]);
        g_extras[static_cast<size_t>(idx)] = extra;
        return;
    }

    g_graph_registry.push_back(g);
    g_extras.push_back(extra);
}

static GraphExtra build_extra_from_edge_list(const std::vector<ParsedEdge> &edges, int64_t n) {
    GraphExtra extra;
    extra.num_edge_pairs = static_cast<int64_t>(edges.size());

    if (!edges.empty()) {
        extra.edge_pairs = static_cast<int32_t *>(malloc(edges.size() * 2 * sizeof(int32_t)));
        extra.edge_weights = static_cast<int32_t *>(malloc(edges.size() * sizeof(int32_t)));
        for (size_t i = 0; i < edges.size(); i++) {
            extra.edge_pairs[i * 2] = edges[i].u;
            extra.edge_pairs[i * 2 + 1] = edges[i].v;
            extra.edge_weights[i] = edges[i].w;
        }
    }

    extra.node_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; i++)
        roaring_bitmap_add(extra.node_bitmap, static_cast<uint32_t>(i));

    extra.edge_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (uint32_t eid = 0; eid < static_cast<uint32_t>(edges.size()); eid++)
        roaring_bitmap_add(extra.edge_bitmap, eid);

    return extra;
}

static GraphExtra build_extra_from_edge_list_unweighted(const std::vector<ParsedEdgeUW> &edges, int64_t n) {
    GraphExtra extra;
    extra.num_edge_pairs = static_cast<int64_t>(edges.size());

    if (!edges.empty()) {
        extra.edge_pairs = static_cast<int32_t *>(malloc(edges.size() * 2 * sizeof(int32_t)));
        extra.edge_weights = nullptr;
        for (size_t i = 0; i < edges.size(); i++) {
            extra.edge_pairs[i * 2] = edges[i].u;
            extra.edge_pairs[i * 2 + 1] = edges[i].v;
        }
    }

    extra.node_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; i++)
        roaring_bitmap_add(extra.node_bitmap, static_cast<uint32_t>(i));

    extra.edge_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (uint32_t eid = 0; eid < static_cast<uint32_t>(edges.size()); eid++)
        roaring_bitmap_add(extra.edge_bitmap, eid);

    return extra;
}

static GraphExtra build_extra_from_csr(const Graph *g) {
    GraphExtra extra;
    if (!g)
        return extra;

    std::vector<ParsedEdge> logical_edges;
    logical_edges.reserve(static_cast<size_t>(g->m > 0 ? g->m / 2 : 0));

    std::unordered_set<uint64_t> seen;
    seen.reserve(static_cast<size_t>(g->m > 0 ? g->m / 2 : 0));

    for (int32_t u = 0; u < g->n; ++u) {
        int64_t start = g->row_ptr[u];
        int64_t end = g->row_ptr[u + 1];
        for (int64_t i = start; i < end; ++i) {
            int32_t v = g->col_idx[i];
            uint64_t key = pack_undirected_edge(u, v);
            if (!seen.insert(key).second)
                continue;

            int32_t weight = g->weights ? g->weights[i] : 1;
            logical_edges.push_back({std::min(u, v), std::max(u, v), weight});
        }
    }

    return build_extra_from_edge_list(logical_edges, g->n);
}

static Graph *load_graph_from_file_impl(const char *filename, bool weighted) {
    struct stat st;
    size_t file_bytes = (stat(filename, &st) == 0 && st.st_size > 0)
                        ? static_cast<size_t>(st.st_size)
                        : 0;

    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open graph file '%s'\n", filename);
        exit(1);
    }
    print_checkpoint("1/5 after fopen");

    std::vector<ParsedEdge> edges_w;
    std::vector<ParsedEdgeUW> edges_uw;
    char line[4096];
    int32_t max_id = -1;

    while (fgets(line, sizeof line, f)) {
        const char *p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r') ++p;
        if (*p == '\0' || *p == '\n' || *p == '#' || *p == '%')
            continue;

        int u = 0;
        int v = 0;
        if (weighted) {
            int w = 1;
            int nread = sscanf(p, "%d %d %d", &u, &v, &w);
            if (nread == 3) {
                edges_w.push_back({static_cast<int32_t>(u), static_cast<int32_t>(v), static_cast<int32_t>(w)});
                if (u > max_id) max_id = u;
                if (v > max_id) max_id = v;
            }
        } else {
            int nread = sscanf(p, "%d %d", &u, &v);
            if (nread == 2) {
                edges_uw.push_back({static_cast<int32_t>(u), static_cast<int32_t>(v)});
                if (u > max_id) max_id = u;
                if (v > max_id) max_id = v;
            }
        }
    }
    fclose(f);

    const size_t edge_count = weighted ? edges_w.size() : edges_uw.size();
    size_t parsed_edges_bytes =
        weighted ? (edges_w.size() * sizeof(ParsedEdge)) : (edges_uw.size() * sizeof(ParsedEdgeUW));
    print_checkpoint("2/5 after parse+fclose");

    int64_t n = (max_id >= 0) ? static_cast<int64_t>(max_id) + 1 : 0;
    int64_t m = 2 * static_cast<int64_t>(edge_count);

    int64_t *row_ptr = static_cast<int64_t *>(calloc(n + 1, sizeof(int64_t)));
    if (weighted) {
        for (const ParsedEdge &e : edges_w) {
            row_ptr[e.u + 1]++;
            row_ptr[e.v + 1]++;
        }
    } else {
        for (const ParsedEdgeUW &e : edges_uw) {
            row_ptr[e.u + 1]++;
            row_ptr[e.v + 1]++;
        }
    }
    for (int64_t i = 1; i <= n; i++)
        row_ptr[i] += row_ptr[i - 1];

    int32_t *col_idx = static_cast<int32_t *>(malloc(m * sizeof(int32_t)));
    int32_t *weights = weighted ? static_cast<int32_t *>(malloc(m * sizeof(int32_t))) : nullptr;
    int64_t *next = static_cast<int64_t *>(malloc((n + 1) * sizeof(int64_t)));
    memcpy(next, row_ptr, (n + 1) * sizeof(int64_t));
    if (weighted) {
        for (const ParsedEdge &e : edges_w) {
            int64_t pos_u = next[e.u]++;
            int64_t pos_v = next[e.v]++;
            col_idx[pos_u] = e.v;
            col_idx[pos_v] = e.u;
            weights[pos_u] = e.w;
            weights[pos_v] = e.w;
        }
    } else {
        for (const ParsedEdgeUW &e : edges_uw) {
            int64_t pos_u = next[e.u]++;
            int64_t pos_v = next[e.v]++;
            col_idx[pos_u] = e.v;
            col_idx[pos_v] = e.u;
        }
    }
    free(next);

    size_t row_ptr_bytes = static_cast<size_t>(n + 1) * sizeof(int64_t);
    size_t col_idx_bytes = static_cast<size_t>(m) * sizeof(int32_t);
    size_t weights_bytes = weighted ? static_cast<size_t>(m) * sizeof(int32_t) : 0;
    print_checkpoint("3/5 after csr arrays");

    Graph *g = static_cast<Graph *>(malloc(sizeof(Graph)));
    g->n = n;
    g->m = m;
    g->row_ptr = row_ptr;
    g->col_idx = col_idx;
    g->weights = weights;

    GraphExtra extra = weighted ? build_extra_from_edge_list(edges_w, n)
                                : build_extra_from_edge_list_unweighted(edges_uw, n);
    register_graph_extra(g, extra);

    size_t edge_pairs_bytes = edge_count * 2 * sizeof(int32_t);
    size_t edge_weights_bytes = weighted ? (edge_count * sizeof(int32_t)) : 0;
    print_checkpoint("4/5 after GraphExtra+register");
    print_estimates(file_bytes, parsed_edges_bytes, row_ptr_bytes, col_idx_bytes,
                    weights_bytes, edge_pairs_bytes, edge_weights_bytes);

    print_checkpoint("5/5 final before return");
    return g;
}

extern "C" void graph_register_csr_metadata(Graph *g) {
    if (!g)
        return;
    register_graph_extra(g, build_extra_from_csr(g));
}

extern "C" Graph *load_graph_from_file(const char *filename) {
    return load_graph_from_file_impl(filename, false);
}

extern "C" Graph *load_weighted_graph_from_file(const char *filename) {
    return load_graph_from_file_impl(filename, true);
}

extern "C" void *graph_get_node_bitmap(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return static_cast<void *>(g_extras[static_cast<size_t>(idx)].node_bitmap);
}

extern "C" void *graph_get_edge_bitmap(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return static_cast<void *>(g_extras[static_cast<size_t>(idx)].edge_bitmap);
}

extern "C" int32_t *graph_get_edge_pairs(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return g_extras[static_cast<size_t>(idx)].edge_pairs;
}

extern "C" int64_t graph_get_num_edge_pairs(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return 0;
    return g_extras[static_cast<size_t>(idx)].num_edge_pairs;
}

extern "C" int32_t *graph_get_edge_weights(Graph *g) {
    int idx = find_graph_index(g);
    if (idx < 0) return nullptr;
    return g_extras[static_cast<size_t>(idx)].edge_weights;
}

extern "C" int32_t graph_get_edge_src_by_id(Graph *g, int32_t eid) {
    int idx = find_graph_index(g);
    if (idx < 0) return -1;
    const GraphExtra &extra = g_extras[static_cast<size_t>(idx)];
    if (eid < 0 || eid >= extra.num_edge_pairs || !extra.edge_pairs)
        return -1;
    return extra.edge_pairs[static_cast<size_t>(eid) * 2];
}

extern "C" int32_t graph_get_edge_dst_by_id(Graph *g, int32_t eid) {
    int idx = find_graph_index(g);
    if (idx < 0) return -1;
    const GraphExtra &extra = g_extras[static_cast<size_t>(idx)];
    if (eid < 0 || eid >= extra.num_edge_pairs || !extra.edge_pairs)
        return -1;
    return extra.edge_pairs[static_cast<size_t>(eid) * 2 + 1];
}

extern "C" int32_t graph_get_edge_weight_by_id(Graph *g, int32_t eid) {
    int idx = find_graph_index(g);
    if (idx < 0) return INT32_MAX;
    const GraphExtra &extra = g_extras[static_cast<size_t>(idx)];
    if (eid < 0 || eid >= extra.num_edge_pairs || !extra.edge_weights)
        return 1;
    return extra.edge_weights[eid];
}

extern "C" int32_t graph_get_edge_weight(Graph *g, int32_t u, int32_t v) {
    if (!g || u < 0 || v < 0 || u >= g->n || v >= g->n)
        return INT32_MAX;

    int64_t start = g->row_ptr[u];
    int64_t end = g->row_ptr[u + 1];
    for (int64_t i = start; i < end; ++i) {
        if (g->col_idx[i] != v)
            continue;
        return g->weights ? g->weights[i] : 1;
    }

    return INT32_MAX;
}
