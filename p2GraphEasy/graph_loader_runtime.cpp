#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
};

struct GraphExtra {
    int32_t *edge_pairs;
    int64_t num_edge_pairs;
    RoaringBitmap *node_bitmap;
    RoaringBitmap *edge_bitmap;
};

struct ParsedEdge {
    int32_t u;
    int32_t v;
};

struct MappedFile {
    int fd = -1;
    const char *data = nullptr;
    size_t size = 0;
};

static std::vector<GraphExtra> g_extras;
static std::vector<Graph *> g_graph_registry;

static int find_graph_index(Graph *g) {
    for (size_t i = 0; i < g_graph_registry.size(); i++)
        if (g_graph_registry[i] == g) return (int)i;
    return -1;
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
    int32_t &local_max_id)
{
    const char *line = begin;
    while (line < end) {
        const char *line_end = line;
        while (line_end < end && *line_end != '\n') ++line_end;

        const char *p = skip_inline_space(line, line_end);
        if (p < line_end && *p != '#' && *p != '%') {
            int32_t u = 0;
            int32_t v = 0;
            const char *cursor = p;
            if (parse_int_token(cursor, line_end, u) &&
                parse_int_token(cursor, line_end, v)) {
                out_edges.push_back({u, v});
                if (u > local_max_id) local_max_id = u;
                if (v > local_max_id) local_max_id = v;
            }
        }

        line = line_end;
        if (line < end && *line == '\n') ++line;
    }
}

extern "C" Graph *load_graph_from_file(const char *filename) {
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
                    parse_edge_block(begin, end, local_edges, local_max_id);
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
    int64_t m = 2 * static_cast<int64_t>(edges.size());

    int64_t *row_ptr = static_cast<int64_t *>(calloc(n + 1, sizeof(int64_t)));
    for (const ParsedEdge &e : edges) {
        row_ptr[e.u + 1]++;
        row_ptr[e.v + 1]++;
    }
    for (int64_t i = 1; i <= n; i++)
        row_ptr[i] += row_ptr[i - 1];

    int32_t *col_idx = static_cast<int32_t *>(malloc(m * sizeof(int32_t)));
    int64_t *next = static_cast<int64_t *>(malloc((n + 1) * sizeof(int64_t)));
    memcpy(next, row_ptr, (n + 1) * sizeof(int64_t));
    for (const ParsedEdge &e : edges) {
        col_idx[next[e.u]++] = e.v;
        col_idx[next[e.v]++] = e.u;
    }
    free(next);

    Graph *g = static_cast<Graph *>(malloc(sizeof(Graph)));
    g->n = n;
    g->m = m;
    g->row_ptr = row_ptr;
    g->col_idx = col_idx;

    GraphExtra extra;
    extra.num_edge_pairs = static_cast<int64_t>(edges.size());
    extra.edge_pairs = static_cast<int32_t *>(malloc(edges.size() * 2 * sizeof(int32_t)));
    for (size_t i = 0; i < edges.size(); i++) {
        extra.edge_pairs[i * 2] = edges[i].u;
        extra.edge_pairs[i * 2 + 1] = edges[i].v;
    }

    extra.node_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (int64_t i = 0; i < n; i++)
        roaring_bitmap_add(extra.node_bitmap, static_cast<uint32_t>(i));

    extra.edge_bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (uint32_t eid = 0; eid < static_cast<uint32_t>(edges.size()); eid++)
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
