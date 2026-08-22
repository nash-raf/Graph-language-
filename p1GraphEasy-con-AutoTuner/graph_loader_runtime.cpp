#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <limits>
#include <string>
#include <unordered_set>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _OPENMP
#include <omp.h>
#else
static int omp_get_max_threads() { return 1; }
static int omp_get_thread_num() { return 0; }
#endif

#include "roaring_bitmap.h"
#include "autotuner_runtime.h"

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
    /* Appended after the original five fields on purpose: autotuner_runtime.c
     * reads row_ptr/col_idx/weights at hard-coded byte offsets 16/24/32, and
     * appending leaves those offsets unchanged. */
    int32_t directed;
};

struct GraphExtra {
    int32_t *edge_pairs = nullptr;
    int32_t *edge_weights = nullptr;
    int64_t num_edge_pairs = 0;
    RoaringBitmap *node_bitmap = nullptr;
    RoaringBitmap *edge_bitmap = nullptr;
    bool edge_arrays_owned = true;
    /* False when row_ptr/col_idx/weights borrow the .sgplbin mmap. */
    bool csr_arrays_owned = true;
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

enum CacheSection : size_t {
    CACHE_ROW_PTR = 0,
    CACHE_COL_IDX,
    CACHE_CSR_WEIGHTS,
    CACHE_EDGE_PAIRS,
    CACHE_EDGE_WEIGHTS,
    CACHE_NODE_BITMAP,
    CACHE_EDGE_BITMAP,
    CACHE_SECTION_COUNT
};

struct GraphCacheHeader {
    char magic[8];
    uint32_t version;
    uint32_t header_size;
    uint32_t endian_marker;
    uint32_t weighted;
    uint64_t source_size;
    int64_t source_mtime_sec;
    int64_t source_mtime_nsec;
    int64_t n;
    int64_t m;
    int64_t num_edge_pairs;
    uint64_t section_offsets[CACHE_SECTION_COUNT];
    uint64_t section_sizes[CACHE_SECTION_COUNT];
    uint64_t section_hashes[CACHE_SECTION_COUNT];
};

static constexpr char kCacheMagic[8] = {'S', 'G', 'P', 'L', 'C', 'S', 'R', '1'};
static constexpr uint32_t kCacheVersion = 3;
static constexpr uint32_t kEndianMarker = 0x01020304U;
static constexpr uint64_t kFnvOffset = 1469598103934665603ULL;
static constexpr uint64_t kFnvPrime = 1099511628211ULL;

static std::vector<GraphExtra> g_extras;
static std::vector<Graph *> g_graph_registry;

static uint64_t pack_undirected_edge(int32_t u, int32_t v) {
    uint32_t a = static_cast<uint32_t>(std::min(u, v));
    uint32_t b = static_cast<uint32_t>(std::max(u, v));
    return (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
}

static void free_graph_extra(GraphExtra &extra) {
    if (extra.edge_arrays_owned) {
        free(extra.edge_pairs);
        free(extra.edge_weights);
    }
    if (extra.node_bitmap)
        roaring_bitmap_free(extra.node_bitmap);
    if (extra.edge_bitmap)
        roaring_bitmap_free(extra.edge_bitmap);

    extra.edge_pairs = nullptr;
    extra.edge_weights = nullptr;
    extra.num_edge_pairs = 0;
    extra.node_bitmap = nullptr;
    extra.edge_bitmap = nullptr;
    extra.edge_arrays_owned = true;
    extra.csr_arrays_owned = true;
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

static RoaringBitmap *build_dense_id_bitmap(uint64_t count) {
    RoaringBitmap *bitmap = roaring_bitmap_create(64 * 1024, 8);
    for (uint64_t id = 0; id < count; ++id)
        roaring_bitmap_add(bitmap, static_cast<uint32_t>(id));
    return bitmap;
}

static GraphExtra build_extra_from_edge_list(const std::vector<ParsedEdge> &edges, int64_t n) {
    GraphExtra extra;
    extra.num_edge_pairs = static_cast<int64_t>(edges.size());

    if (!edges.empty()) {
        extra.edge_pairs = static_cast<int32_t *>(malloc(edges.size() * 2 * sizeof(int32_t)));
        extra.edge_weights = static_cast<int32_t *>(malloc(edges.size() * sizeof(int32_t)));
        if (!extra.edge_pairs || !extra.edge_weights)
            abort();
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < edges.size(); i++) {
            extra.edge_pairs[i * 2] = edges[i].u;
            extra.edge_pairs[i * 2 + 1] = edges[i].v;
            extra.edge_weights[i] = edges[i].w;
        }
    }

    extra.node_bitmap = build_dense_id_bitmap(static_cast<uint64_t>(n));
    extra.edge_bitmap = build_dense_id_bitmap(edges.size());

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

static uint64_t hash_bytes(uint64_t hash, const void *data, size_t size) {
    const uint8_t *bytes = static_cast<const uint8_t *>(data);
    while (size >= sizeof(uint64_t)) {
        uint64_t word;
        memcpy(&word, bytes, sizeof(word));
        hash ^= word;
        hash *= kFnvPrime;
        bytes += sizeof(word);
        size -= sizeof(word);
    }
    while (size > 0) {
        hash ^= *bytes++;
        hash *= kFnvPrime;
        --size;
    }
    return hash;
}

static bool checked_mul_size(uint64_t count, uint64_t width, uint64_t &result) {
    if (width != 0 && count > std::numeric_limits<uint64_t>::max() / width)
        return false;
    result = count * width;
    return result <= static_cast<uint64_t>(std::numeric_limits<size_t>::max());
}

static bool read_full_at(int fd, void *buffer, size_t size, uint64_t offset) {
    uint8_t *out = static_cast<uint8_t *>(buffer);
    while (size > 0) {
        ssize_t got = pread(fd, out, size, static_cast<off_t>(offset));
        if (got <= 0)
            return false;
        out += got;
        size -= static_cast<size_t>(got);
        offset += static_cast<uint64_t>(got);
    }
    return true;
}

static bool write_full_at(int fd, const void *buffer, size_t size, uint64_t offset) {
    const uint8_t *in = static_cast<const uint8_t *>(buffer);
    while (size > 0) {
        ssize_t wrote = pwrite(fd, in, size, static_cast<off_t>(offset));
        if (wrote <= 0)
            return false;
        in += wrote;
        size -= static_cast<size_t>(wrote);
        offset += static_cast<uint64_t>(wrote);
    }
    return true;
}

static bool source_identity_matches(const struct stat &a, const struct stat &b) {
    return a.st_size == b.st_size &&
           a.st_mtim.tv_sec == b.st_mtim.tv_sec &&
           a.st_mtim.tv_nsec == b.st_mtim.tv_nsec;
}

static std::string cache_path_for(const char *filename, bool weighted,
                                  bool directed) {
    // Directedness is part of the key: the two builds produce different CSRs
    // from the same source file, so they must not share a cache entry.
    return std::string(filename) + (weighted ? ".weighted" : "") +
           (directed ? ".directed" : "") + ".sgplbin";
}

static bool graph_cache_disabled() {
    const char *mode = getenv("SGPL_GRAPH_CACHE");
    return mode && strcmp(mode, "off") == 0;
}

static bool graph_cache_refresh_requested() {
    const char *mode = getenv("SGPL_GRAPH_CACHE");
    return mode && strcmp(mode, "refresh") == 0;
}

static bool validate_cache_layout(const GraphCacheHeader &header,
                                  off_t file_size,
                                  bool weighted) {
    if (memcmp(header.magic, kCacheMagic, sizeof(kCacheMagic)) != 0 ||
        header.version != kCacheVersion ||
        header.header_size != sizeof(GraphCacheHeader) ||
        header.endian_marker != kEndianMarker ||
        header.weighted != static_cast<uint32_t>(weighted) ||
        header.n < 0 || header.n > INT32_MAX ||
        header.m < 0 || header.num_edge_pairs < 0 ||
        header.num_edge_pairs > UINT32_MAX ||
        header.m != header.num_edge_pairs * 2)
        return false;

    uint64_t expected[CACHE_SECTION_COUNT] = {};
    if (!checked_mul_size(static_cast<uint64_t>(header.n) + 1,
                          sizeof(int64_t), expected[CACHE_ROW_PTR]) ||
        !checked_mul_size(static_cast<uint64_t>(header.m),
                          sizeof(int32_t), expected[CACHE_COL_IDX]) ||
        !checked_mul_size(static_cast<uint64_t>(header.m),
                          weighted ? sizeof(int32_t) : 0,
                          expected[CACHE_CSR_WEIGHTS]) ||
        !checked_mul_size(static_cast<uint64_t>(header.num_edge_pairs),
                          2 * sizeof(int32_t), expected[CACHE_EDGE_PAIRS]) ||
        !checked_mul_size(static_cast<uint64_t>(header.num_edge_pairs),
                          sizeof(int32_t), expected[CACHE_EDGE_WEIGHTS]))
        return false;

    for (size_t section = 0; section <= CACHE_EDGE_WEIGHTS; ++section)
        if (header.section_sizes[section] != expected[section])
            return false;
    if (header.section_sizes[CACHE_NODE_BITMAP] == 0 ||
        header.section_sizes[CACHE_EDGE_BITMAP] == 0)
        return false;

    uint64_t expected_offset = sizeof(GraphCacheHeader);
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section) {
        if (header.section_offsets[section] != expected_offset ||
            header.section_sizes[section] >
                std::numeric_limits<uint64_t>::max() - expected_offset)
            return false;
        expected_offset += header.section_sizes[section];
    }
    return expected_offset <= static_cast<uint64_t>(file_size);
}

static Graph *try_load_graph_cache(const std::string &path,
                                   const struct stat &source_stat,
                                   bool weighted,
                                   bool directed) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return nullptr;

    struct stat cache_stat;
    GraphCacheHeader header = {};
    bool valid = fstat(fd, &cache_stat) == 0 &&
                 read_full_at(fd, &header, sizeof(header), 0) &&
                 header.source_size == static_cast<uint64_t>(source_stat.st_size) &&
                 header.source_mtime_sec == source_stat.st_mtim.tv_sec &&
                 header.source_mtime_nsec == source_stat.st_mtim.tv_nsec &&
                 validate_cache_layout(header, cache_stat.st_size, weighted);
    if (!valid) {
        close(fd);
        return nullptr;
    }

    void *mapping =
        mmap(nullptr, static_cast<size_t>(cache_stat.st_size),
             PROT_READ | PROT_WRITE,
             MAP_PRIVATE, fd, 0);
    close(fd);
    if (mapping == MAP_FAILED)
        return nullptr;
    madvise(mapping, static_cast<size_t>(cache_stat.st_size), MADV_WILLNEED);
    const uint8_t *base = static_cast<const uint8_t *>(mapping);

    std::array<uint8_t, CACHE_SECTION_COUNT> valid_hashes = {};
    #pragma omp parallel for schedule(static)
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section) {
        size_t size = static_cast<size_t>(header.section_sizes[section]);
        uint64_t hash =
            hash_bytes(kFnvOffset,
                       base + header.section_offsets[section], size);
        valid_hashes[section] =
            static_cast<uint8_t>(hash == header.section_hashes[section]);
    }
    for (uint8_t hash_valid : valid_hashes)
        if (!hash_valid) {
            munmap(mapping, static_cast<size_t>(cache_stat.st_size));
            return nullptr;
        }

    int64_t *row_ptr = reinterpret_cast<int64_t *>(
        const_cast<uint8_t *>(base + header.section_offsets[CACHE_ROW_PTR]));
    int32_t *col_idx = reinterpret_cast<int32_t *>(
        const_cast<uint8_t *>(base + header.section_offsets[CACHE_COL_IDX]));
    int32_t *csr_weights =
        weighted
            ? reinterpret_cast<int32_t *>(const_cast<uint8_t *>(
                  base + header.section_offsets[CACHE_CSR_WEIGHTS]))
            : nullptr;
    valid = row_ptr[0] == 0 && row_ptr[header.n] == header.m;
    for (int64_t vertex = 0; valid && vertex < header.n; ++vertex)
        valid = row_ptr[vertex] <= row_ptr[vertex + 1];
    #pragma omp parallel for reduction(&&:valid) schedule(static)
    for (int64_t edge = 0; edge < header.m; ++edge) {
        bool edge_valid = col_idx[edge] >= 0 && col_idx[edge] < header.n;
        valid = valid && edge_valid;
    }
    if (!valid) {
        munmap(mapping, static_cast<size_t>(cache_stat.st_size));
        return nullptr;
    }

    GraphExtra extra;
    extra.num_edge_pairs = header.num_edge_pairs;
    extra.edge_pairs = reinterpret_cast<int32_t *>(const_cast<uint8_t *>(
        base + header.section_offsets[CACHE_EDGE_PAIRS]));
    extra.edge_weights = reinterpret_cast<int32_t *>(const_cast<uint8_t *>(
        base + header.section_offsets[CACHE_EDGE_WEIGHTS]));
    extra.edge_arrays_owned = false;
    extra.csr_arrays_owned = false;
    extra.node_bitmap = roaring_bitmap_portable_deserialize(
        base + header.section_offsets[CACHE_NODE_BITMAP],
        static_cast<size_t>(header.section_sizes[CACHE_NODE_BITMAP]));
    extra.edge_bitmap = roaring_bitmap_portable_deserialize(
        base + header.section_offsets[CACHE_EDGE_BITMAP],
        static_cast<size_t>(header.section_sizes[CACHE_EDGE_BITMAP]));
    valid = extra.node_bitmap && extra.edge_bitmap &&
            roaring_bitmap_get_cardinality(extra.node_bitmap) ==
                static_cast<uint64_t>(header.n) &&
            roaring_bitmap_get_cardinality(extra.edge_bitmap) ==
                static_cast<uint64_t>(header.num_edge_pairs);
    if (!valid) {
        if (extra.node_bitmap)
            roaring_bitmap_free(extra.node_bitmap);
        if (extra.edge_bitmap)
            roaring_bitmap_free(extra.edge_bitmap);
        munmap(mapping, static_cast<size_t>(cache_stat.st_size));
        return nullptr;
    }

    Graph *graph = static_cast<Graph *>(malloc(sizeof(Graph)));
    if (!graph) {
        roaring_bitmap_free(extra.node_bitmap);
        roaring_bitmap_free(extra.edge_bitmap);
        munmap(mapping, static_cast<size_t>(cache_stat.st_size));
        return nullptr;
    }
    graph->n = header.n;
    graph->m = header.m;
    graph->row_ptr = row_ptr;
    graph->col_idx = col_idx;
    graph->weights = csr_weights;
    graph->directed = directed ? 1 : 0;

    // Graphs have process lifetime in the current runtime. Keep the private
    // mapping alive so Graph and GraphExtra can borrow cache sections directly.
    // Mutations must call graph_ensure_owned_storage first so realloc/free are
    // never applied to mmap pointers.
    register_graph_extra(graph, extra);
    return graph;
}

static void fsync_parent_directory(const std::string &path) {
    size_t slash = path.find_last_of('/');
    std::string directory =
        slash == std::string::npos ? "." : path.substr(0, slash);
    int fd = open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd >= 0) {
        fsync(fd);
        close(fd);
    }
}

static void publish_graph_cache(const std::string &path,
                                const char *source_filename,
                                const struct stat &source_stat,
                                bool weighted,
                                const Graph *graph,
                                const GraphExtra &extra) {
    if (!graph || !extra.node_bitmap || !extra.edge_bitmap)
        return;

    size_t node_bitmap_size =
        roaring_bitmap_portable_size_in_bytes(extra.node_bitmap);
    size_t edge_bitmap_size =
        roaring_bitmap_portable_size_in_bytes(extra.edge_bitmap);
    std::vector<uint8_t> node_bitmap(node_bitmap_size);
    std::vector<uint8_t> edge_bitmap(edge_bitmap_size);
    roaring_bitmap_portable_serialize(extra.node_bitmap, node_bitmap.data());
    roaring_bitmap_portable_serialize(extra.edge_bitmap, edge_bitmap.data());

    GraphCacheHeader header = {};
    memcpy(header.magic, kCacheMagic, sizeof(kCacheMagic));
    header.version = kCacheVersion;
    header.header_size = sizeof(GraphCacheHeader);
    header.endian_marker = kEndianMarker;
    header.weighted = static_cast<uint32_t>(weighted);
    header.source_size = static_cast<uint64_t>(source_stat.st_size);
    header.source_mtime_sec = source_stat.st_mtim.tv_sec;
    header.source_mtime_nsec = source_stat.st_mtim.tv_nsec;
    header.n = graph->n;
    header.m = graph->m;
    header.num_edge_pairs = extra.num_edge_pairs;
    checked_mul_size(static_cast<uint64_t>(graph->n) + 1, sizeof(int64_t),
                     header.section_sizes[CACHE_ROW_PTR]);
    checked_mul_size(static_cast<uint64_t>(graph->m), sizeof(int32_t),
                     header.section_sizes[CACHE_COL_IDX]);
    checked_mul_size(static_cast<uint64_t>(graph->m),
                     weighted ? sizeof(int32_t) : 0,
                     header.section_sizes[CACHE_CSR_WEIGHTS]);
    checked_mul_size(static_cast<uint64_t>(extra.num_edge_pairs),
                     2 * sizeof(int32_t),
                     header.section_sizes[CACHE_EDGE_PAIRS]);
    checked_mul_size(static_cast<uint64_t>(extra.num_edge_pairs),
                     sizeof(int32_t),
                     header.section_sizes[CACHE_EDGE_WEIGHTS]);
    header.section_sizes[CACHE_NODE_BITMAP] = node_bitmap.size();
    header.section_sizes[CACHE_EDGE_BITMAP] = edge_bitmap.size();
    uint64_t offset = sizeof(GraphCacheHeader);
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section) {
        header.section_offsets[section] = offset;
        offset += header.section_sizes[section];
    }

    std::array<const void *, CACHE_SECTION_COUNT> sources = {
        graph->row_ptr, graph->col_idx, graph->weights, extra.edge_pairs,
        extra.edge_weights, node_bitmap.data(), edge_bitmap.data()};
    #pragma omp parallel for schedule(static)
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section)
        header.section_hashes[section] = hash_bytes(
            kFnvOffset, sources[section],
            static_cast<size_t>(header.section_sizes[section]));

    std::string temporary =
        path + ".tmp." + std::to_string(static_cast<long long>(getpid()));
    int fd = open(temporary.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0)
        return;
    bool ok = write_full_at(fd, &header, sizeof(header), 0);
    for (size_t section = 0; ok && section < CACHE_SECTION_COUNT; ++section) {
        size_t size = static_cast<size_t>(header.section_sizes[section]);
        if (size > 0)
            ok = write_full_at(fd, sources[section], size,
                               header.section_offsets[section]);
    }
    if (ok)
        ok = fsync(fd) == 0;
    close(fd);

    struct stat current_source;
    if (ok && stat(source_filename, &current_source) == 0)
        ok = source_identity_matches(source_stat, current_source);
    else
        ok = false;

    if (ok && rename(temporary.c_str(), path.c_str()) == 0)
        fsync_parent_directory(path);
    else
        unlink(temporary.c_str());
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

static bool parse_int_token(const char *&p, const char *end, int32_t &value,
                            bool &out_of_range) {
    p = skip_inline_space(p, end);
    if (p >= end) return false;

    int sign = 1;
    if (*p == '+' || *p == '-') {
        sign = (*p == '-') ? -1 : 1;
        ++p;
    }
    if (p >= end || !std::isdigit(static_cast<unsigned char>(*p)))
        return false;

    uint64_t parsed = 0;
    uint64_t limit = sign < 0
                         ? static_cast<uint64_t>(INT32_MAX) + 1
                         : static_cast<uint64_t>(INT32_MAX);
    while (p < end && std::isdigit(static_cast<unsigned char>(*p))) {
        uint32_t digit = static_cast<uint32_t>(*p - '0');
        if (parsed > (limit - digit) / 10)
            out_of_range = true;
        else if (!out_of_range)
            parsed = parsed * 10 + digit;
        ++p;
    }
    if (out_of_range)
        return false;
    if (sign < 0 && parsed == static_cast<uint64_t>(INT32_MAX) + 1)
        value = INT32_MIN;
    else
        value = static_cast<int32_t>(sign * static_cast<int64_t>(parsed));
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
    bool weighted,
    bool &invalid_vertex_id)
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
            bool out_of_range = false;
            bool ok = parse_int_token(cursor, line_end, u, out_of_range) &&
                      parse_int_token(cursor, line_end, v, out_of_range);
            if (ok && weighted)
                ok = parse_int_token(cursor, line_end, w, out_of_range);

            if (out_of_range || (ok && (u < 0 || v < 0 ||
                                        u == INT32_MAX || v == INT32_MAX))) {
                invalid_vertex_id = true;
            } else if (ok) {
                out_edges.push_back({u, v, w});
                if (u > local_max_id) local_max_id = u;
                if (v > local_max_id) local_max_id = v;
            }
        }

        line = line_end;
        if (line < end && *line == '\n') ++line;
    }
}

static void parallel_prefix_row_ptr(int64_t *row_ptr, int64_t n) {
    int workers = std::max(1, omp_get_max_threads());
    std::vector<int64_t> partial(static_cast<size_t>(workers) + 1, 0);
    #pragma omp parallel num_threads(workers)
    {
        int tid = omp_get_thread_num();
        int actual_workers = 1;
#ifdef _OPENMP
        actual_workers = omp_get_num_threads();
#endif
        int64_t begin = n * tid / actual_workers;
        int64_t end = n * (tid + 1) / actual_workers;
        int64_t sum = 0;
        for (int64_t vertex = begin; vertex < end; ++vertex) {
            sum += row_ptr[vertex + 1];
            row_ptr[vertex + 1] = sum;
        }
        partial[static_cast<size_t>(tid) + 1] = sum;
        #pragma omp barrier
        #pragma omp single
        {
            for (int worker = 1; worker <= actual_workers; ++worker)
                partial[worker] += partial[worker - 1];
        }
        int64_t offset = partial[tid];
        for (int64_t vertex = begin; vertex < end; ++vertex)
            row_ptr[vertex + 1] += offset;
    }
}

static uint64_t csr_histogram_budget_bytes() {
    const char *value = getenv("SGPL_CSR_HISTOGRAM_MB");
    if (!value || !*value)
        return 64ULL * 1024 * 1024;
    char *end = nullptr;
    unsigned long long mb = strtoull(value, &end, 10);
    if (!end || *end != '\0' || mb == 0)
        return 64ULL * 1024 * 1024;
    if (mb > std::numeric_limits<uint64_t>::max() / (1024ULL * 1024))
        return std::numeric_limits<uint64_t>::max();
    return static_cast<uint64_t>(mb) * 1024 * 1024;
}

static Graph *build_parallel_csr(const std::vector<ParsedEdge> &edges,
                                 const std::vector<size_t> &thread_offsets,
                                 int64_t n,
                                 bool weighted,
                                 bool directed) {
    int thread_count = static_cast<int>(thread_offsets.size()) - 1;
    // An undirected graph stores every edge in both rows; a directed one stores
    // it only in the source row, so it needs half the space.
    int64_t m = (directed ? 1 : 2) * static_cast<int64_t>(edges.size());
    int64_t *row_ptr =
        static_cast<int64_t *>(calloc(static_cast<size_t>(n) + 1,
                                     sizeof(int64_t)));
    int32_t *col_idx = m > 0
                           ? static_cast<int32_t *>(
                                 malloc(static_cast<size_t>(m) * sizeof(int32_t)))
                           : nullptr;
    int32_t *weights =
        weighted && m > 0
            ? static_cast<int32_t *>(
                  malloc(static_cast<size_t>(m) * sizeof(int32_t)))
            : nullptr;
    if (!row_ptr || (m > 0 && !col_idx) || (weighted && m > 0 && !weights))
        abort();

    uint64_t histogram_slots = 0;
    bool use_histograms =
        n > 0 &&
        (static_cast<uint64_t>(n) <=
             std::numeric_limits<uint64_t>::max() /
                 static_cast<uint64_t>(std::max(1, thread_count)) &&
         (histogram_slots = static_cast<uint64_t>(n) *
                            static_cast<uint64_t>(std::max(1, thread_count))) <=
             csr_histogram_budget_bytes() / sizeof(uint64_t));

    std::vector<uint64_t> histograms;
    if (use_histograms && histogram_slots > 0) {
        try {
            histograms.assign(static_cast<size_t>(histogram_slots), 0);
        } catch (...) {
            use_histograms = false;
        }
    }

    if (use_histograms) {
        #pragma omp parallel for schedule(static)
        for (int thread = 0; thread < thread_count; ++thread) {
            uint64_t *local =
                histograms.data() + static_cast<size_t>(thread) * n;
            for (size_t i = thread_offsets[thread];
                 i < thread_offsets[thread + 1]; ++i) {
                const ParsedEdge &edge = edges[i];
                local[edge.u]++;
                if (!directed)
                    local[edge.v]++;
            }
        }
        #pragma omp parallel for schedule(static)
        for (int64_t vertex = 0; vertex < n; ++vertex) {
            int64_t degree = 0;
            for (int thread = 0; thread < thread_count; ++thread)
                degree += static_cast<int64_t>(
                    histograms[static_cast<size_t>(thread) * n + vertex]);
            row_ptr[vertex + 1] = degree;
        }
        parallel_prefix_row_ptr(row_ptr, n);

        #pragma omp parallel for schedule(static)
        for (int64_t vertex = 0; vertex < n; ++vertex) {
            uint64_t cursor = static_cast<uint64_t>(row_ptr[vertex]);
            for (int thread = 0; thread < thread_count; ++thread) {
                size_t index = static_cast<size_t>(thread) * n + vertex;
                uint64_t count = histograms[index];
                histograms[index] = cursor;
                cursor += count;
            }
        }
        #pragma omp parallel for schedule(static)
        for (int thread = 0; thread < thread_count; ++thread) {
            uint64_t *cursors =
                histograms.data() + static_cast<size_t>(thread) * n;
            for (size_t i = thread_offsets[thread];
                 i < thread_offsets[thread + 1]; ++i) {
                const ParsedEdge &edge = edges[i];
                uint64_t pos_u = cursors[edge.u]++;
                col_idx[pos_u] = edge.v;
                if (weights)
                    weights[pos_u] = edge.w;
                if (!directed) {
                    uint64_t pos_v = cursors[edge.v]++;
                    col_idx[pos_v] = edge.u;
                    if (weights)
                        weights[pos_v] = edge.w;
                }
            }
        }
    } else {
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < edges.size(); ++i) {
            const ParsedEdge &edge = edges[i];
            #pragma omp atomic update
            row_ptr[edge.u + 1]++;
            if (!directed) {
                #pragma omp atomic update
                row_ptr[edge.v + 1]++;
            }
        }
        parallel_prefix_row_ptr(row_ptr, n);
        std::vector<int64_t> next(static_cast<size_t>(n));
        #pragma omp parallel for schedule(static)
        for (int64_t vertex = 0; vertex < n; ++vertex)
            next[vertex] = row_ptr[vertex];
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < edges.size(); ++i) {
            const ParsedEdge &edge = edges[i];
            int64_t pos_u;
            #pragma omp atomic capture
            pos_u = next[edge.u]++;
            col_idx[pos_u] = edge.v;
            if (weights)
                weights[pos_u] = edge.w;
            if (!directed) {
                int64_t pos_v;
                #pragma omp atomic capture
                pos_v = next[edge.v]++;
                col_idx[pos_v] = edge.u;
                if (weights)
                    weights[pos_v] = edge.w;
            }
        }
    }

    Graph *graph = static_cast<Graph *>(malloc(sizeof(Graph)));
    if (!graph)
        abort();
    graph->n = n;
    graph->m = m;
    graph->row_ptr = row_ptr;
    graph->col_idx = col_idx;
    graph->weights = weights;
    graph->directed = directed ? 1 : 0;
    return graph;
}

static Graph *load_graph_from_file_impl(const char *filename, bool weighted,
                                       bool directed) {
    struct stat source_stat;
    if (stat(filename, &source_stat) != 0) {
        fprintf(stderr, "Error: cannot open graph file '%s'\n", filename);
        exit(1);
    }

    std::string cache_path = cache_path_for(filename, weighted, directed);
    bool cache_disabled = graph_cache_disabled();
    if (!cache_disabled && !graph_cache_refresh_requested()) {
        Graph *cached =
            try_load_graph_cache(cache_path, source_stat, weighted, directed);
        if (cached)
            return cached;
    }

    MappedFile mf = map_file_read_only(filename);
    if (mf.fd < 0) {
        fprintf(stderr, "Error: cannot open graph file '%s'\n", filename);
        exit(1);
    }

    const size_t kBlockBytes = 256 * 1024;
    const int thread_count = std::max(1, omp_get_max_threads());
    std::vector<std::vector<ParsedEdge>> thread_edges(thread_count);
    std::vector<int32_t> thread_max_ids(thread_count, -1);
    std::vector<uint8_t> thread_invalid_ids(thread_count, 0);

    if (mf.size > 0) {
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            std::vector<ParsedEdge> &local_edges = thread_edges[tid];
            int32_t local_max_id = -1;
            bool invalid_vertex_id = false;

            #pragma omp for schedule(dynamic)
            for (size_t block_start = 0; block_start < mf.size; block_start += kBlockBytes) {
                const char *begin = nullptr;
                const char *end = nullptr;
                compute_block_bounds(mf.data, mf.size, block_start, kBlockBytes, begin, end);
                if (begin < end)
                    parse_edge_block(begin, end, local_edges, local_max_id,
                                     weighted, invalid_vertex_id);
            }

            thread_max_ids[tid] = local_max_id;
            thread_invalid_ids[tid] =
                static_cast<uint8_t>(invalid_vertex_id);
        }
    }

    int32_t max_id = -1;
    size_t total_edges = 0;
    bool invalid_vertex_id = false;
    std::vector<size_t> thread_offsets(static_cast<size_t>(thread_count) + 1,
                                       0);
    for (int t = 0; t < thread_count; ++t) {
        total_edges += thread_edges[t].size();
        thread_offsets[static_cast<size_t>(t) + 1] = total_edges;
        if (thread_max_ids[t] > max_id) max_id = thread_max_ids[t];
        invalid_vertex_id =
            invalid_vertex_id || thread_invalid_ids[t] != 0;
    }
    if (invalid_vertex_id || total_edges > UINT32_MAX) {
        unmap_file(mf);
        fprintf(stderr, "Error: graph '%s' contains an invalid vertex ID or "
                        "too many logical edges\n", filename);
        exit(1);
    }

    std::vector<ParsedEdge> edges(total_edges);
    #pragma omp parallel for schedule(static)
    for (int t = 0; t < thread_count; ++t)
        std::copy(thread_edges[t].begin(), thread_edges[t].end(),
                  edges.begin() + static_cast<ptrdiff_t>(
                                      thread_offsets[t]));

    unmap_file(mf);

    int64_t n = (max_id >= 0) ? static_cast<int64_t>(max_id) + 1 : 0;
    Graph *graph =
        build_parallel_csr(edges, thread_offsets, n, weighted, directed);
    GraphExtra extra = build_extra_from_edge_list(edges, n);
    if (!cache_disabled)
        publish_graph_cache(cache_path, filename, source_stat, weighted,
                            graph, extra);
    register_graph_extra(graph, extra);
    return graph;
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

/*
 * Copy mmap-backed CSR / edge-pair arrays into malloc storage so later
 * realloc/free in mutation and layout conversion are legal. No-op when the
 * arrays are already owned. The .sgplbin mapping is left mapped for the
 * process lifetime (same as today's warm-load path).
 */
extern "C" void graph_ensure_owned_storage(Graph *g) {
    if (!g)
        return;
    int idx = find_graph_index(g);
    if (idx < 0)
        return;
    GraphExtra &extra = g_extras[static_cast<size_t>(idx)];

    if (!extra.csr_arrays_owned) {
        if (g->n < 0 || g->m < 0)
            abort();
        /* After convert-to-SET the Graph CSR views can be cleared; skip copy. */
        if (!g->row_ptr) {
            extra.csr_arrays_owned = true;
        } else if (autograph_get_layout(g) != LAYOUT_CSR) {
            /* PCSR/BCSR arrays are heap-allocated by their converters and are
             * capacity-indexed (PCSR row ends reach ~2m). A CSR-sized (g->m)
             * copy would truncate the gap-encoded buffer and cause heap
             * overreads in PCSR gap-slot scans. They are already owned, so no
             * copy is needed. */
            extra.csr_arrays_owned = true;
        } else {
        size_t row_bytes = static_cast<size_t>(g->n + 1) * sizeof(int64_t);
        size_t col_bytes = static_cast<size_t>(g->m) * sizeof(int32_t);
        int64_t *owned_row = static_cast<int64_t *>(malloc(row_bytes));
        int32_t *owned_col =
            g->m > 0 ? static_cast<int32_t *>(malloc(col_bytes)) : nullptr;
        int32_t *owned_weights = nullptr;
        if (!owned_row || (g->m > 0 && !owned_col))
            abort();
        memcpy(owned_row, g->row_ptr, row_bytes);
        if (g->m > 0)
            memcpy(owned_col, g->col_idx, col_bytes);
        if (g->weights) {
            owned_weights = static_cast<int32_t *>(malloc(col_bytes));
            if (!owned_weights)
                abort();
            memcpy(owned_weights, g->weights, col_bytes);
        }
        g->row_ptr = owned_row;
        g->col_idx = owned_col;
        g->weights = owned_weights;
        extra.csr_arrays_owned = true;
        }
    }

    if (!extra.edge_arrays_owned) {
        int64_t pairs = extra.num_edge_pairs;
        if (pairs < 0)
            abort();
        int32_t *owned_pairs = nullptr;
        int32_t *owned_ew = nullptr;
        if (pairs > 0) {
            size_t pair_bytes = static_cast<size_t>(pairs) * 2 * sizeof(int32_t);
            owned_pairs = static_cast<int32_t *>(malloc(pair_bytes));
            if (!owned_pairs)
                abort();
            memcpy(owned_pairs, extra.edge_pairs, pair_bytes);
            owned_ew =
                static_cast<int32_t *>(malloc(static_cast<size_t>(pairs) *
                                              sizeof(int32_t)));
            if (!owned_ew)
                abort();
            /* Unweighted caches store an empty weights section; synthesize 1s. */
            if (g->weights)
                memcpy(owned_ew, extra.edge_weights,
                       static_cast<size_t>(pairs) * sizeof(int32_t));
            else
                for (int64_t i = 0; i < pairs; ++i)
                    owned_ew[i] = 1;
        }
        extra.edge_pairs = owned_pairs;
        extra.edge_weights = owned_ew;
        extra.edge_arrays_owned = true;
    }
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
