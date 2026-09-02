#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <string>
#include <unordered_set>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__linux__) && !defined(MAP_POPULATE)
#define MAP_POPULATE 0x80000
#endif

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
    /* False when edge_pairs/edge_weights and the graph's row_ptr/col_idx/weights
       borrow the .sgplbin mmap instead of owning heap storage. free()/realloc()
       must never be applied to those. */
    bool arrays_owned = true;
};

/* ------------------------------------------------------------------------
   Binary CSR cache (.sgplbin)

   Parsing a text edge list dominates load time, and the parse result is a pure
   function of the file plus the (directed, weight_mode) pair the program asked
   for. So the first load writes a snapshot of everything the runtime needs and
   later loads mmap it directly.

   The snapshot deliberately does NOT include the transpose CSR: that is built
   on demand by graph_ensure_in_csr(), so caching it would store bytes most
   programs never touch.

   Cache identity is (source size, source mtime ns, directed, weight_mode).
   The last two matter because the SAME text file yields different CSRs
   depending on them -- a directed load stores each edge once, an undirected
   load twice, and weight modes 2/3/4 synthesize +1/0/-1 while mode 1 reads a
   third column. Keying only on "weighted" would let a mode-2 cache satisfy a
   mode-4 request and silently invert every edge sign.
   ------------------------------------------------------------------------ */

enum CacheSection : size_t {
    CACHE_ROW_PTR = 0,
    CACHE_COL_IDX,
    CACHE_CSR_WEIGHTS,
    CACHE_EDGE_PAIRS,
    CACHE_NODE_BITMAP,
    CACHE_EDGE_BITMAP,
    CACHE_SECTION_COUNT
};

struct GraphCacheHeader {
    char magic[8];
    uint32_t version;
    uint32_t header_size;
    uint32_t endian_marker;
    uint32_t weight_mode;   /* 0 none, 1 from-file, 2 positive, 3 zero, 4 negative */
    uint32_t directed;
    uint32_t reserved;
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

static constexpr char kCacheMagic[8] = {'S', 'G', 'P', 'L', 'C', 'S', 'R', '2'};
static constexpr uint32_t kCacheVersion = 2;
static constexpr uint32_t kEndianMarker = 0x01020304U;
static constexpr uint64_t kFnvOffset = 1469598103934665603ULL;
static constexpr uint64_t kFnvPrime = 1099511628211ULL;

struct ParsedEdge {
    int32_t u;
    int32_t v;
    int32_t w;
};

struct ParsedEdgeUW {
    int32_t u;
    int32_t v;
};

struct MappedFile {
    int fd = -1;
    const char *data = nullptr;
    /// Bytes of real file content (from `fstat`); used for accounting and default parse bound.
    size_t size = 0;
    /// Length passed to `mmap`/`munmap` (may be larger than `size` if padded via env).
    size_t mapped_len = 0;
};

/// Parse bound for OpenMP blocks: real file size, or full mapping if `GRAPH_MMAP_PARSE_USE_MAP_LEN=1`.
static size_t mmap_parse_limit_bytes(const MappedFile &mf) {
    const char *e = std::getenv("GRAPH_MMAP_PARSE_USE_MAP_LEN");
    if (e && e[0] == '1' && e[1] == '\0')
        return mf.mapped_len;
    return mf.size;
}

enum class ChunkMode {
    kSplitByThread = 0,
    kFixedBlocks = 1,
};

static ChunkMode mmap_chunk_mode() {
    const char *e = std::getenv("GRAPH_MMAP_CHUNK_MODE");
    if (!e || e[0] == '\0')
        return ChunkMode::kSplitByThread;
    if (std::strcmp(e, "block") == 0 || std::strcmp(e, "blocks") == 0)
        return ChunkMode::kFixedBlocks;
    return ChunkMode::kSplitByThread;
}

static size_t mmap_block_bytes() {
    const char *e = std::getenv("GRAPH_MMAP_BLOCK_BYTES");
    if (!e || e[0] == '\0')
        return 256 * 1024;
    unsigned long v = std::strtoul(e, nullptr, 10);
    return v > 0 ? static_cast<size_t>(v) : static_cast<size_t>(256 * 1024);
}

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
        fprintf(stderr, "[loader-mmap] %s | VmRSS: n/a\n", phase);
        return;
    }
    fprintf(stderr, "[loader-mmap] %s | VmRSS: %ld KiB (%.2f MiB)\n",
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
            "[loader-mmap] estimates | file=%.2f MiB parsed=%.2f MiB csr=%.2f MiB "
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
    if (extra.arrays_owned) {
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
        // For unweighted graphs, we omit edge_weights; API returns weight=1 when absent.
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

    const size_t data_len = static_cast<size_t>(st.st_size);
    mf.size = data_len;
    mf.mapped_len = 0;
    if (data_len == 0) return mf;

    size_t map_len = data_len;
    if (const char *env_min = std::getenv("GRAPH_MMAP_MIN_MAP_BYTES")) {
        if (env_min[0] != '\0') {
            unsigned long v = std::strtoul(env_min, nullptr, 10);
            if (v > map_len)
                map_len = static_cast<size_t>(v);
        }
    }

    int mmap_flags = MAP_PRIVATE;
#ifdef MAP_POPULATE
    if (const char *pop = std::getenv("GRAPH_MMAP_MAP_POPULATE")) {
        if (pop[0] == '1' && pop[1] == '\0')
            mmap_flags |= MAP_POPULATE;
    }
#endif
    void *mapped = mmap(nullptr, map_len, PROT_READ, mmap_flags, mf.fd, 0);
    if (mapped == MAP_FAILED) {
        close(mf.fd);
        mf.fd = -1;
        mf.size = 0;
        return mf;
    }

    mf.mapped_len = map_len;
    mf.data = static_cast<const char *>(mapped);

    bool advise_full = false;
    if (const char *adv = std::getenv("GRAPH_MMAP_MADVISE_FULL")) {
        if (adv[0] == '1' && adv[1] == '\0')
            advise_full = true;
    }
    const size_t advise_len = advise_full ? map_len : std::min(map_len, data_len);
    if (advise_len > 0)
        madvise(mapped, advise_len, MADV_WILLNEED);

    if (map_len > data_len) {
        fprintf(stderr,
                "[loader-mmap] padded mmap: real_file_bytes=%zu mapped_bytes=%zu "
                "(GRAPH_MMAP_MIN_MAP_BYTES)\n",
                data_len, map_len);
    }
#ifdef MAP_POPULATE
    if ((mmap_flags & MAP_POPULATE) != 0 && map_len > data_len) {
        fprintf(stderr,
                "[loader-mmap] MAP_POPULATE set — kernel prefaults entire mapping; "
                "expect higher RSS.\n");
    }
#endif
    return mf;
}

static void unmap_file(MappedFile &mf) {
    if (mf.data && mf.mapped_len > 0)
        munmap(const_cast<char *>(mf.data), mf.mapped_len);
    if (mf.fd >= 0)
        close(mf.fd);
    mf.fd = -1;
    mf.data = nullptr;
    mf.size = 0;
    mf.mapped_len = 0;
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
    int weight_mode)
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
            if (ok && weight_mode == 1)
                ok = parse_int_token(cursor, line_end, w);

            if (ok && weight_mode == 2) w = 1;
            else if (ok && weight_mode == 3) w = 0;
            else if (ok && weight_mode == 4) w = -1;

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

static void parse_edge_block_unweighted(
    const char *begin,
    const char *end,
    std::vector<ParsedEdgeUW> &out_edges,
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
            bool ok = parse_int_token(cursor, line_end, u) &&
                      parse_int_token(cursor, line_end, v);
            if (ok) {
                out_edges.push_back({u, v});
                if (u > local_max_id) local_max_id = u;
                if (v > local_max_id) local_max_id = v;
            }
        }

        line = line_end;
        if (line < end && *line == '\n') ++line;
    }
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
    if (width != 0 && count > UINT64_MAX / width)
        return false;
    result = count * width;
    return true;
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

static std::string cache_path_for(const char *filename, int weight_mode, bool directed) {
    char suffix[32];
    snprintf(suffix, sizeof(suffix), ".%c%d.sgplbin", directed ? 'd' : 'u', weight_mode);
    return std::string(filename) + suffix;
}

static bool graph_cache_disabled() {
    const char *mode = getenv("SGPL_GRAPH_CACHE");
    return mode && strcmp(mode, "off") == 0;
}

static bool graph_cache_refresh_requested() {
    const char *mode = getenv("SGPL_GRAPH_CACHE");
    return mode && strcmp(mode, "refresh") == 0;
}

static bool validate_cache_layout(const GraphCacheHeader &header, off_t file_size,
                                  int weight_mode, bool directed) {
    if (memcmp(header.magic, kCacheMagic, sizeof(kCacheMagic)) != 0 ||
        header.version != kCacheVersion ||
        header.header_size != sizeof(GraphCacheHeader) ||
        header.endian_marker != kEndianMarker ||
        header.weight_mode != static_cast<uint32_t>(weight_mode) ||
        header.directed != static_cast<uint32_t>(directed ? 1 : 0) ||
        header.n < 0 || header.n > INT32_MAX ||
        header.m < 0 || header.num_edge_pairs < 0)
        return false;

    // Directed graphs store one arc per input edge, undirected store two.
    const int64_t expected_m = (directed ? 1 : 2) * header.num_edge_pairs;
    if (header.m != expected_m)
        return false;

    const bool weighted = weight_mode != 0;
    uint64_t expected[CACHE_SECTION_COUNT] = {};
    if (!checked_mul_size(static_cast<uint64_t>(header.n) + 1, sizeof(int64_t),
                          expected[CACHE_ROW_PTR]) ||
        !checked_mul_size(static_cast<uint64_t>(header.m), sizeof(int32_t),
                          expected[CACHE_COL_IDX]) ||
        !checked_mul_size(static_cast<uint64_t>(header.m),
                          weighted ? sizeof(int32_t) : 0,
                          expected[CACHE_CSR_WEIGHTS]) ||
        !checked_mul_size(static_cast<uint64_t>(header.num_edge_pairs),
                          2 * sizeof(int32_t), expected[CACHE_EDGE_PAIRS]))
        return false;

    for (size_t section = 0; section <= CACHE_EDGE_PAIRS; ++section)
        if (header.section_sizes[section] != expected[section])
            return false;
    if (header.section_sizes[CACHE_NODE_BITMAP] == 0 ||
        header.section_sizes[CACHE_EDGE_BITMAP] == 0)
        return false;

    uint64_t expected_offset = sizeof(GraphCacheHeader);
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section) {
        if (header.section_offsets[section] != expected_offset ||
            header.section_sizes[section] > UINT64_MAX - expected_offset)
            return false;
        expected_offset += header.section_sizes[section];
    }
    return expected_offset <= static_cast<uint64_t>(file_size);
}

static Graph *try_load_graph_cache(const std::string &path,
                                   const struct stat &source_stat,
                                   int weight_mode, bool directed) {
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
                 validate_cache_layout(header, cache_stat.st_size, weight_mode, directed);
    if (!valid) {
        close(fd);
        return nullptr;
    }

    const size_t map_size = static_cast<size_t>(cache_stat.st_size);
    void *mapping = mmap(nullptr, map_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    if (mapping == MAP_FAILED)
        return nullptr;
    madvise(mapping, map_size, MADV_WILLNEED);
    const uint8_t *base = static_cast<const uint8_t *>(mapping);

    // Per-section FNV check: a truncated or corrupted cache must be rebuilt,
    // not handed to the runtime as valid CSR.
    bool hashes_ok = true;
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section) {
        uint64_t hash = hash_bytes(kFnvOffset, base + header.section_offsets[section],
                                   static_cast<size_t>(header.section_sizes[section]));
        if (hash != header.section_hashes[section]) {
            hashes_ok = false;
            break;
        }
    }
    if (!hashes_ok) {
        munmap(mapping, map_size);
        return nullptr;
    }

    const bool weighted = weight_mode != 0;
    int64_t *row_ptr = reinterpret_cast<int64_t *>(
        const_cast<uint8_t *>(base + header.section_offsets[CACHE_ROW_PTR]));
    int32_t *col_idx = reinterpret_cast<int32_t *>(
        const_cast<uint8_t *>(base + header.section_offsets[CACHE_COL_IDX]));
    int32_t *csr_weights =
        weighted ? reinterpret_cast<int32_t *>(const_cast<uint8_t *>(
                       base + header.section_offsets[CACHE_CSR_WEIGHTS]))
                 : nullptr;

    // Structural sanity: monotone row_ptr and in-range column indices. Cheap
    // relative to parsing, and it stops a well-hashed but wrong file from
    // producing out-of-bounds reads deep inside a traversal.
    valid = row_ptr[0] == 0 && row_ptr[header.n] == header.m;
    for (int64_t vertex = 0; valid && vertex < header.n; ++vertex)
        valid = row_ptr[vertex] <= row_ptr[vertex + 1];
    if (valid) {
        #pragma omp parallel for reduction(&& : valid) schedule(static)
        for (int64_t edge = 0; edge < header.m; ++edge) {
            bool edge_valid = col_idx[edge] >= 0 && col_idx[edge] < header.n;
            valid = valid && edge_valid;
        }
    }
    if (!valid) {
        munmap(mapping, map_size);
        return nullptr;
    }

    GraphExtra extra;
    extra.num_edge_pairs = header.num_edge_pairs;
    extra.edge_pairs = reinterpret_cast<int32_t *>(
        const_cast<uint8_t *>(base + header.section_offsets[CACHE_EDGE_PAIRS]));
    extra.edge_weights = nullptr;  // derived from edge_pairs + CSR weights
    extra.arrays_owned = false;
    extra.node_bitmap = roaring_bitmap_portable_deserialize(
        base + header.section_offsets[CACHE_NODE_BITMAP],
        static_cast<size_t>(header.section_sizes[CACHE_NODE_BITMAP]));
    extra.edge_bitmap = roaring_bitmap_portable_deserialize(
        base + header.section_offsets[CACHE_EDGE_BITMAP],
        static_cast<size_t>(header.section_sizes[CACHE_EDGE_BITMAP]));
    if (!extra.node_bitmap || !extra.edge_bitmap) {
        if (extra.node_bitmap) roaring_bitmap_free(extra.node_bitmap);
        if (extra.edge_bitmap) roaring_bitmap_free(extra.edge_bitmap);
        munmap(mapping, map_size);
        return nullptr;
    }

    Graph *graph = static_cast<Graph *>(malloc(sizeof(Graph)));
    if (!graph) {
        roaring_bitmap_free(extra.node_bitmap);
        roaring_bitmap_free(extra.edge_bitmap);
        munmap(mapping, map_size);
        return nullptr;
    }
    graph->n = header.n;
    graph->m = header.m;
    graph->row_ptr = row_ptr;
    graph->col_idx = col_idx;
    graph->weights = csr_weights;
    graph->directed = directed ? 1 : 0;
    graph->in_row_ptr = nullptr;   /* built on demand, never cached */
    graph->in_col_idx = nullptr;

    // Graphs live for the whole process, so the private mapping is intentionally
    // left mapped and the CSR borrows it directly.
    register_graph_extra(graph, extra);
    return graph;
}

static void fsync_parent_directory(const std::string &path) {
    size_t slash = path.find_last_of('/');
    std::string directory = slash == std::string::npos ? "." : path.substr(0, slash);
    int fd = open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd >= 0) {
        fsync(fd);
        close(fd);
    }
}

static void publish_graph_cache(const std::string &path, const char *source_filename,
                                const struct stat &source_stat, int weight_mode,
                                bool directed, const Graph *graph,
                                const GraphExtra &extra) {
    if (!graph || !extra.node_bitmap || !extra.edge_bitmap)
        return;

    const bool weighted = weight_mode != 0;
    size_t node_bitmap_size = roaring_bitmap_portable_size_in_bytes(extra.node_bitmap);
    size_t edge_bitmap_size = roaring_bitmap_portable_size_in_bytes(extra.edge_bitmap);
    std::vector<uint8_t> node_bitmap(node_bitmap_size);
    std::vector<uint8_t> edge_bitmap(edge_bitmap_size);
    roaring_bitmap_portable_serialize(extra.node_bitmap, node_bitmap.data());
    roaring_bitmap_portable_serialize(extra.edge_bitmap, edge_bitmap.data());

    GraphCacheHeader header = {};
    memcpy(header.magic, kCacheMagic, sizeof(kCacheMagic));
    header.version = kCacheVersion;
    header.header_size = sizeof(GraphCacheHeader);
    header.endian_marker = kEndianMarker;
    header.weight_mode = static_cast<uint32_t>(weight_mode);
    header.directed = directed ? 1u : 0u;
    header.source_size = static_cast<uint64_t>(source_stat.st_size);
    header.source_mtime_sec = source_stat.st_mtim.tv_sec;
    header.source_mtime_nsec = source_stat.st_mtim.tv_nsec;
    header.n = graph->n;
    header.m = graph->m;
    header.num_edge_pairs = extra.num_edge_pairs;

    if (!checked_mul_size(static_cast<uint64_t>(graph->n) + 1, sizeof(int64_t),
                          header.section_sizes[CACHE_ROW_PTR]) ||
        !checked_mul_size(static_cast<uint64_t>(graph->m), sizeof(int32_t),
                          header.section_sizes[CACHE_COL_IDX]) ||
        !checked_mul_size(static_cast<uint64_t>(graph->m), weighted ? sizeof(int32_t) : 0,
                          header.section_sizes[CACHE_CSR_WEIGHTS]) ||
        !checked_mul_size(static_cast<uint64_t>(extra.num_edge_pairs), 2 * sizeof(int32_t),
                          header.section_sizes[CACHE_EDGE_PAIRS]))
        return;
    header.section_sizes[CACHE_NODE_BITMAP] = node_bitmap.size();
    header.section_sizes[CACHE_EDGE_BITMAP] = edge_bitmap.size();

    uint64_t offset = sizeof(GraphCacheHeader);
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section) {
        header.section_offsets[section] = offset;
        offset += header.section_sizes[section];
    }

    const void *sources[CACHE_SECTION_COUNT] = {
        graph->row_ptr, graph->col_idx, graph->weights, extra.edge_pairs,
        node_bitmap.data(), edge_bitmap.data()};
    for (size_t section = 0; section < CACHE_SECTION_COUNT; ++section)
        header.section_hashes[section] =
            hash_bytes(kFnvOffset, sources[section],
                       static_cast<size_t>(header.section_sizes[section]));

    // Write to a pid-suffixed temporary and rename, so a crash or a concurrent
    // run can never leave a half-written cache that later passes validation.
    std::string temporary = path + ".tmp." + std::to_string(static_cast<long long>(getpid()));
    int fd = open(temporary.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0)
        return;
    bool ok = write_full_at(fd, &header, sizeof(header), 0);
    for (size_t section = 0; ok && section < CACHE_SECTION_COUNT; ++section) {
        size_t size = static_cast<size_t>(header.section_sizes[section]);
        if (size > 0)
            ok = write_full_at(fd, sources[section], size, header.section_offsets[section]);
    }
    if (ok)
        ok = fsync(fd) == 0;
    close(fd);

    // Re-stat: if the source changed while we were building, the snapshot is
    // already stale and must not be published.
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

static Graph *load_graph_from_file_impl(const char *filename, int weight_mode, bool directed) {
    const bool weighted = weight_mode != 0;

    struct stat source_stat;
    const bool have_source_stat = stat(filename, &source_stat) == 0;
    const std::string cache_path = cache_path_for(filename, weight_mode, directed);
    const bool cache_disabled = graph_cache_disabled();
    if (have_source_stat && !cache_disabled && !graph_cache_refresh_requested()) {
        if (Graph *cached = try_load_graph_cache(cache_path, source_stat, weight_mode, directed)) {
            print_checkpoint("cache hit (.sgplbin)");
            return cached;
        }
    }

    MappedFile mf = map_file_read_only(filename);
    if (mf.fd < 0) {
        fprintf(stderr, "Error: cannot open graph file '%s'\n", filename);
        exit(1);
    }
    print_checkpoint("1/5 after mmap open");

    const size_t parse_limit = mmap_parse_limit_bytes(mf);
    if (parse_limit != mf.size) {
        fprintf(stderr,
                "[loader-mmap] parse_limit=%zu (full map; GRAPH_MMAP_PARSE_USE_MAP_LEN=1) "
                "real_file_bytes=%zu\n",
                parse_limit, mf.size);
    }

    const int thread_count = std::max(1, omp_get_max_threads());
    std::vector<int32_t> thread_max_ids(thread_count, -1);

    // Parse edges: keep temporary parsed representation minimal for unweighted graphs.
    std::vector<ParsedEdge> edges_w;
    std::vector<ParsedEdgeUW> edges_uw;
    size_t parsed_edges_bytes = 0;
    int32_t max_id = -1;

    if (parse_limit > 0) {
        const ChunkMode mode = mmap_chunk_mode();
        const bool use_blocks = (mode == ChunkMode::kFixedBlocks);

        if (weighted) {
            std::vector<std::vector<ParsedEdge>> thread_edges(thread_count);
            if (use_blocks) {
                const size_t block_bytes = mmap_block_bytes();
                #pragma omp parallel
                {
                    const int tid = omp_get_thread_num();
                    std::vector<ParsedEdge> &local_edges = thread_edges[tid];
                    int32_t local_max_id = -1;

                    #pragma omp for schedule(dynamic)
                    for (size_t block_start = 0; block_start < parse_limit; block_start += block_bytes) {
                        const char *begin = nullptr;
                        const char *end = nullptr;
                        compute_block_bounds(mf.data, parse_limit, block_start, block_bytes, begin, end);
                        if (begin < end)
                            parse_edge_block(begin, end, local_edges, local_max_id, weight_mode);
                    }
                    thread_max_ids[tid] = local_max_id;
                }
            } else {
                const size_t chunk_bytes =
                    (parse_limit + static_cast<size_t>(thread_count) - 1) / static_cast<size_t>(thread_count);
                #pragma omp parallel
                {
                    const int tid = omp_get_thread_num();
                    std::vector<ParsedEdge> &local_edges = thread_edges[tid];
                    int32_t local_max_id = -1;

                    const size_t block_start = std::min(static_cast<size_t>(tid) * chunk_bytes, parse_limit);
                    const size_t desired_end =
                        std::min(static_cast<size_t>(tid + 1) * chunk_bytes, parse_limit);
                    const size_t block_bytes = desired_end > block_start ? (desired_end - block_start) : 0;
                    const char *begin = nullptr;
                    const char *end = nullptr;
                    compute_block_bounds(mf.data, parse_limit, block_start, block_bytes, begin, end);
                    if (begin < end)
                        parse_edge_block(begin, end, local_edges, local_max_id, weight_mode);
                    thread_max_ids[tid] = local_max_id;
                }
            }

            size_t total_edges = 0;
            for (int t = 0; t < thread_count; ++t) {
                total_edges += thread_edges[t].size();
                if (thread_max_ids[t] > max_id) max_id = thread_max_ids[t];
            }
            edges_w.reserve(total_edges);
            for (int t = 0; t < thread_count; ++t)
                edges_w.insert(edges_w.end(), thread_edges[t].begin(), thread_edges[t].end());
            for (int t = 0; t < thread_count; ++t)
                std::vector<ParsedEdge>().swap(thread_edges[t]);
            std::vector<std::vector<ParsedEdge>>().swap(thread_edges);

            parsed_edges_bytes = edges_w.size() * sizeof(ParsedEdge);
        } else {
            std::vector<std::vector<ParsedEdgeUW>> thread_edges(thread_count);
            if (use_blocks) {
                const size_t block_bytes = mmap_block_bytes();
                #pragma omp parallel
                {
                    const int tid = omp_get_thread_num();
                    std::vector<ParsedEdgeUW> &local_edges = thread_edges[tid];
                    int32_t local_max_id = -1;

                    #pragma omp for schedule(dynamic)
                    for (size_t block_start = 0; block_start < parse_limit; block_start += block_bytes) {
                        const char *begin = nullptr;
                        const char *end = nullptr;
                        compute_block_bounds(mf.data, parse_limit, block_start, block_bytes, begin, end);
                        if (begin < end)
                            parse_edge_block_unweighted(begin, end, local_edges, local_max_id);
                    }
                    thread_max_ids[tid] = local_max_id;
                }
            } else {
                const size_t chunk_bytes =
                    (parse_limit + static_cast<size_t>(thread_count) - 1) / static_cast<size_t>(thread_count);
                #pragma omp parallel
                {
                    const int tid = omp_get_thread_num();
                    std::vector<ParsedEdgeUW> &local_edges = thread_edges[tid];
                    int32_t local_max_id = -1;

                    const size_t block_start = std::min(static_cast<size_t>(tid) * chunk_bytes, parse_limit);
                    const size_t desired_end =
                        std::min(static_cast<size_t>(tid + 1) * chunk_bytes, parse_limit);
                    const size_t block_bytes = desired_end > block_start ? (desired_end - block_start) : 0;
                    const char *begin = nullptr;
                    const char *end = nullptr;
                    compute_block_bounds(mf.data, parse_limit, block_start, block_bytes, begin, end);
                    if (begin < end)
                        parse_edge_block_unweighted(begin, end, local_edges, local_max_id);
                    thread_max_ids[tid] = local_max_id;
                }
            }

            size_t total_edges = 0;
            for (int t = 0; t < thread_count; ++t) {
                total_edges += thread_edges[t].size();
                if (thread_max_ids[t] > max_id) max_id = thread_max_ids[t];
            }
            edges_uw.reserve(total_edges);
            for (int t = 0; t < thread_count; ++t)
                edges_uw.insert(edges_uw.end(), thread_edges[t].begin(), thread_edges[t].end());
            for (int t = 0; t < thread_count; ++t)
                std::vector<ParsedEdgeUW>().swap(thread_edges[t]);
            std::vector<std::vector<ParsedEdgeUW>>().swap(thread_edges);

            parsed_edges_bytes = edges_uw.size() * sizeof(ParsedEdgeUW);
        }
    }

    print_checkpoint("2/5 after parse before csr");

    int64_t n = (max_id >= 0) ? static_cast<int64_t>(max_id) + 1 : 0;
    const size_t edge_count = weighted ? edges_w.size() : edges_uw.size();
    int64_t m = (directed ? 1 : 2) * static_cast<int64_t>(edge_count);

    // The transpose (in_row_ptr / in_col_idx) is NOT built here. Only three
    // constructs read it -- `for each in neighbor`, inDegree() and degree() on
    // a directed graph -- so emitted IR calls graph_ensure_in_csr() before any
    // such read and the cost is paid only by programs that actually use it.
    // Building it eagerly cost (n+1)*8 + m*4 bytes and a full counting sort on
    // every directed load, for a feature most programs never touch.
    int64_t *row_ptr = static_cast<int64_t *>(calloc(n + 1, sizeof(int64_t)));
    if (weighted) {
        for (const ParsedEdge &e : edges_w) {
            row_ptr[e.u + 1]++;
            if (!directed)
                row_ptr[e.v + 1]++;
        }
    } else {
        for (const ParsedEdgeUW &e : edges_uw) {
            row_ptr[e.u + 1]++;
            if (!directed)
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
            col_idx[pos_u] = e.v;
            weights[pos_u] = e.w;
            if (!directed) {
                int64_t pos_v = next[e.v]++;
                col_idx[pos_v] = e.u;
                weights[pos_v] = e.w;
            }
        }
    } else {
        for (const ParsedEdgeUW &e : edges_uw) {
            int64_t pos_u = next[e.u]++;
            col_idx[pos_u] = e.v;
            if (!directed) {
                int64_t pos_v = next[e.v]++;
                col_idx[pos_v] = e.u;
            }
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
    g->directed = directed ? 1 : 0;
    // null == "transpose not built yet"; graph_ensure_in_csr() fills these in
    // on first use. Never alias them to row_ptr/col_idx -- that is
    // indistinguishable from a real transpose and silently yields
    // out-neighbours where in-neighbours were asked for.
    g->in_row_ptr = nullptr;
    g->in_col_idx = nullptr;

    GraphExtra extra = weighted ? build_extra_from_edge_list(edges_w, n)
                                : build_extra_from_edge_list_unweighted(edges_uw, n);
    if (have_source_stat && !cache_disabled)
        publish_graph_cache(cache_path, filename, source_stat, weight_mode, directed, g,
                            extra);
    register_graph_extra(g, extra);

    size_t edge_pairs_bytes = edge_count * 2 * sizeof(int32_t);
    size_t edge_weights_bytes = weighted ? (edge_count * sizeof(int32_t)) : 0;
    print_checkpoint("4/5 after GraphExtra+register");
    print_estimates(mf.size, parsed_edges_bytes, row_ptr_bytes, col_idx_bytes,
                    weights_bytes, edge_pairs_bytes, edge_weights_bytes);

    // CSR + GraphExtra now own the data needed by the runtime.
    std::vector<ParsedEdge>().swap(edges_w);
    std::vector<ParsedEdgeUW>().swap(edges_uw);

    unmap_file(mf);
    print_checkpoint("5/5 after munmap+close");
    return g;
}

extern "C" void graph_register_csr_metadata(Graph *g) {
    if (!g)
        return;
    register_graph_extra(g, build_extra_from_csr(g));
}

extern "C" Graph *load_graph_from_file(const char *filename) {
    return load_graph_from_file_impl(filename, 0, false);
}

extern "C" Graph *load_weighted_graph_from_file(const char *filename) {
    return load_graph_from_file_impl(filename, 1, false);
}

extern "C" Graph *load_graph_from_file_directed(const char *filename) {
    return load_graph_from_file_impl(filename, 0, true);
}

extern "C" Graph *load_weighted_graph_from_file_directed(const char *filename) {
    return load_graph_from_file_impl(filename, 1, true);
}

extern "C" Graph *load_weighted_graph_from_file_mode(const char *filename, int32_t weight_mode) {
    return load_graph_from_file_impl(filename, weight_mode, false);
}

extern "C" Graph *load_weighted_graph_from_file_mode_directed(const char *filename, int32_t weight_mode) {
    return load_graph_from_file_impl(filename, weight_mode, true);
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

extern "C" int32_t graph_get_edge_weight(Graph *g, int32_t u, int32_t v);

// Derived from edge_pairs + the CSR weights rather than from a second
// pair-ordered weights array. The graph already stores every weight once per
// CSR slot, so keeping a parallel copy in edge order duplicated the data in
// memory and in the .sgplbin cache purely to turn this O(deg) row scan into an
// O(1) index. Only edgeWeight(G, eid) reads it, from two .graph files, whereas
// weight(G, u, v) -- which uses the CSR copy -- is used by twenty-two.
//
// Caveat: with duplicate parallel edges carrying different weights, the row
// scan reports the first match, where the old array reported the specific
// occurrence. graph_get_edge_weight() already had that behaviour, so the two
// accessors now at least agree with each other.
extern "C" int32_t graph_get_edge_weight_by_id(Graph *g, int32_t eid) {
    int idx = find_graph_index(g);
    if (idx < 0) return INT32_MAX;
    const GraphExtra &extra = g_extras[static_cast<size_t>(idx)];
    if (eid < 0 || eid >= extra.num_edge_pairs || !extra.edge_pairs)
        return 1;
    if (!g->weights)
        return 1;  // unweighted: every edge reads as 1, matching weight()
    const int32_t u = extra.edge_pairs[static_cast<size_t>(eid) * 2];
    const int32_t v = extra.edge_pairs[static_cast<size_t>(eid) * 2 + 1];
    return graph_get_edge_weight(g, u, v);
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
