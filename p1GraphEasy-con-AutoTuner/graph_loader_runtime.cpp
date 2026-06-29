#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <unordered_set>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _OPENMP
#include <omp.h>
#else
static int omp_get_max_threads() { return 1; }
static int omp_get_thread_num() { return 0; }
#endif

#include "roaring_bitmap.h"

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
    int32_t directed;
    int64_t *in_row_ptr;
    int32_t *in_col_idx;
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

struct MappedFile {
    int fd = -1;
    const char *data = nullptr;
    size_t size = 0;
};

static std::vector<GraphExtra> g_extras;
static std::vector<Graph *> g_graph_registry;

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

static GraphExtra build_extra_from_csr(const Graph *g) {
    GraphExtra extra;
    if (!g)
        return extra;

    std::vector<ParsedEdge> logical_edges;
    logical_edges.reserve(static_cast<size_t>(g->directed ? g->m : (g->m > 0 ? g->m / 2 : 0)));

    std::unordered_set<uint64_t> seen;
    seen.reserve(static_cast<size_t>(g->directed ? g->m : (g->m > 0 ? g->m / 2 : 0)));

    for (int32_t u = 0; u < g->n; ++u) {
        int64_t start = g->row_ptr[u];
        int64_t end = g->row_ptr[u + 1];
        for (int64_t i = start; i < end; ++i) {
            int32_t v = g->col_idx[i];
            int32_t weight = g->weights ? g->weights[i] : 1;
            if (g->directed) {
                logical_edges.push_back({u, v, weight});
            } else {
                uint64_t key = pack_undirected_edge(u, v);
                if (!seen.insert(key).second)
                    continue;
                logical_edges.push_back({std::min(u, v), std::max(u, v), weight});
            }
        }
    }

    return build_extra_from_edge_list(logical_edges, g->n);
}

static MappedFile map_file_read_only(const char *filename) {
    MappedFile mf;
    mf.fd = open(filename, O_RDONLY);
    if (mf.fd < 0) return mf;

    struct stat st;
    if (fstat(mf.fd, &st) != 0) {
        close(mf.fd);
        mf.fd = -1;
        return mf;
    }

    mf.size = static_cast<size_t>(st.st_size);
    if (mf.size == 0) return mf;

    void *mapped = mmap(nullptr, mf.size, PROT_READ, MAP_PRIVATE, mf.fd, 0);
    if (mapped == MAP_FAILED) {
        close(mf.fd);
        mf.fd = -1;
        mf.size = 0;
        return mf;
    }

    madvise(mapped, mf.size, MADV_WILLNEED);
    mf.data = static_cast<const char *>(mapped);
    return mf;
}

static void unmap_file(MappedFile &mf) {
    if (mf.data)
        munmap(const_cast<char *>(mf.data), mf.size);
    if (mf.fd >= 0)
        close(mf.fd);
    mf.fd = -1;
    mf.data = nullptr;
    mf.size = 0;
}

static const char *skip_line(const char *p, const char *end) {
    while (p < end && *p != '\n') ++p;
    if (p < end) ++p;
    return p;
}

static const char *skip_inline_space(const char *p, const char *end) {
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\r')) ++p;
    return p;
}

static bool parse_int_token(const char *&p, const char *end, int32_t &value) {
    p = skip_inline_space(p, end);
    if (p >= end) return false;

    int sign = 1;
    if (*p == '+' || *p == '-') {
        sign = (*p == '-') ? -1 : 1;
        ++p;
    }
    if (p >= end || !std::isdigit(static_cast<unsigned char>(*p)))
        return false;

    int64_t parsed = 0;
    while (p < end && std::isdigit(static_cast<unsigned char>(*p))) {
        parsed = parsed * 10 + (*p - '0');
        ++p;
    }
    value = static_cast<int32_t>(sign * parsed);
    return true;
}

static void compute_block_bounds(
    const char *data,
    size_t size,
    size_t block_start,
    size_t block_bytes,
    const char *&begin,
    const char *&end)
{
    const char *file_begin = data;
    const char *file_end = data + size;
    begin = file_begin + std::min(block_start, size);
    end = file_begin + std::min(block_start + block_bytes, size);

    if (begin > file_begin && *(begin - 1) != '\n')
        begin = skip_line(begin, file_end);
    if (end < file_end && end > file_begin && *(end - 1) != '\n')
        end = skip_line(end, file_end);
    if (begin > end)
        begin = end;
}

static void parse_edge_block(
    const char *begin,
    const char *end,
    std::vector<ParsedEdge> &out_edges,
    int32_t &local_max_id,
    bool weighted)
{
    const char *line = begin;
    while (line < end) {
        const char *line_end = line;
        while (line_end < end && *line_end != '\n') ++line_end;

        const char *p = skip_inline_space(line, line_end);
        if (p < line_end && *p != '#' && *p != '%') {
            int32_t u = 0;
            int32_t v = 0;
            int32_t w = 1;
            const char *cursor = p;
            bool ok = parse_int_token(cursor, line_end, u) &&
                      parse_int_token(cursor, line_end, v);
            if (ok && weighted)
                ok = parse_int_token(cursor, line_end, w);

            if (ok) {
                out_edges.push_back({u, v, w});
                if (u > local_max_id) local_max_id = u;
                if (v > local_max_id) local_max_id = v;
            }
        }

        line = line_end;
        if (line < end && *line == '\n') ++line;
    }
}

static Graph *load_graph_from_file_impl(const char *filename, bool weighted, bool directed) {
    MappedFile mf = map_file_read_only(filename);
    if (mf.fd < 0) {
        fprintf(stderr, "Error: cannot open graph file '%s'\n", filename);
        exit(1);
    }

    const size_t kBlockBytes = 256 * 1024;
    const int thread_count = std::max(1, omp_get_max_threads());
    std::vector<std::vector<ParsedEdge>> thread_edges(thread_count);
    std::vector<int32_t> thread_max_ids(thread_count, -1);

    if (mf.size > 0) {
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            std::vector<ParsedEdge> &local_edges = thread_edges[tid];
            int32_t local_max_id = -1;

            #pragma omp for schedule(dynamic)
            for (size_t block_start = 0; block_start < mf.size; block_start += kBlockBytes) {
                const char *begin = nullptr;
                const char *end = nullptr;
                compute_block_bounds(mf.data, mf.size, block_start, kBlockBytes, begin, end);
                if (begin < end)
                    parse_edge_block(begin, end, local_edges, local_max_id, weighted);
            }

            thread_max_ids[tid] = local_max_id;
        }
    }

    int32_t max_id = -1;
    size_t total_edges = 0;
    for (int t = 0; t < thread_count; ++t) {
        total_edges += thread_edges[t].size();
        if (thread_max_ids[t] > max_id) max_id = thread_max_ids[t];
    }

    std::vector<ParsedEdge> edges;
    edges.reserve(total_edges);
    for (int t = 0; t < thread_count; ++t)
        edges.insert(edges.end(), thread_edges[t].begin(), thread_edges[t].end());

    unmap_file(mf);

    int64_t n = (max_id >= 0) ? static_cast<int64_t>(max_id) + 1 : 0;
    int64_t m = (directed ? 1 : 2) * static_cast<int64_t>(edges.size());

    int64_t *row_ptr = static_cast<int64_t *>(calloc(n + 1, sizeof(int64_t)));
    int64_t *in_row_ptr = directed ? static_cast<int64_t *>(calloc(n + 1, sizeof(int64_t))) : nullptr;
    for (const ParsedEdge &e : edges) {
        row_ptr[e.u + 1]++;
        if (directed) {
            in_row_ptr[e.v + 1]++;
        } else {
            row_ptr[e.v + 1]++;
        }
    }
    for (int64_t i = 1; i <= n; i++) {
        row_ptr[i] += row_ptr[i - 1];
        if (directed)
            in_row_ptr[i] += in_row_ptr[i - 1];
    }

    int32_t *col_idx = static_cast<int32_t *>(malloc(m * sizeof(int32_t)));
    int32_t *in_col_idx = directed ? static_cast<int32_t *>(malloc(m * sizeof(int32_t))) : nullptr;
    int32_t *weights = weighted ? static_cast<int32_t *>(malloc(m * sizeof(int32_t))) : nullptr;
    int64_t *next = static_cast<int64_t *>(malloc((n + 1) * sizeof(int64_t)));
    int64_t *in_next = directed ? static_cast<int64_t *>(malloc((n + 1) * sizeof(int64_t))) : nullptr;
    memcpy(next, row_ptr, (n + 1) * sizeof(int64_t));
    if (directed)
        memcpy(in_next, in_row_ptr, (n + 1) * sizeof(int64_t));
    for (const ParsedEdge &e : edges) {
        int64_t pos_u = next[e.u]++;
        col_idx[pos_u] = e.v;
        if (weights)
            weights[pos_u] = e.w;
        if (directed) {
            int64_t in_pos_v = in_next[e.v]++;
            in_col_idx[in_pos_v] = e.u;
        } else {
            int64_t pos_v = next[e.v]++;
            col_idx[pos_v] = e.u;
            if (weights)
                weights[pos_v] = e.w;
        }
    }
    free(next);
    free(in_next);

    Graph *g = static_cast<Graph *>(malloc(sizeof(Graph)));
    g->n = n;
    g->m = m;
    g->row_ptr = row_ptr;
    g->col_idx = col_idx;
    g->weights = weights;
    g->directed = directed ? 1 : 0;
    g->in_row_ptr = in_row_ptr;
    g->in_col_idx = in_col_idx;

    register_graph_extra(g, build_extra_from_edge_list(edges, n));
    return g;
}

extern "C" void graph_register_csr_metadata(Graph *g) {
    if (!g)
        return;
    register_graph_extra(g, build_extra_from_csr(g));
}

extern "C" Graph *load_graph_from_file(const char *filename) {
    return load_graph_from_file_impl(filename, false, false);
}

extern "C" Graph *load_weighted_graph_from_file(const char *filename) {
    return load_graph_from_file_impl(filename, true, false);
}

extern "C" Graph *load_graph_from_file_directed(const char *filename) {
    return load_graph_from_file_impl(filename, false, true);
}

extern "C" Graph *load_weighted_graph_from_file_directed(const char *filename) {
    return load_graph_from_file_impl(filename, true, true);
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
        return INT32_MAX;
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
