#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <tuple>
#include <vector>

#include "../roaring_bitmap.h"

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
};

extern "C" Graph *load_graph_from_file(const char *filename);
extern "C" Graph *load_weighted_graph_from_file(const char *filename);
extern "C" void *graph_get_node_bitmap(Graph *graph);
extern "C" void *graph_get_edge_bitmap(Graph *graph);
extern "C" int32_t *graph_get_edge_pairs(Graph *graph);
extern "C" int32_t *graph_get_edge_weights(Graph *graph);
extern "C" int64_t graph_get_num_edge_pairs(Graph *graph);

static constexpr uint64_t kFnvOffset = 1469598103934665603ULL;
static constexpr uint64_t kFnvPrime = 1099511628211ULL;

static uint64_t hash_bytes(uint64_t hash, const void *data, size_t size) {
    const uint8_t *bytes = static_cast<const uint8_t *>(data);
    for (size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= kFnvPrime;
    }
    return hash;
}

template <class T>
static uint64_t hash_value(uint64_t hash, const T &value) {
    return hash_bytes(hash, &value, sizeof(value));
}

int main(int argc, char **argv) {
    if (argc != 3 ||
        (strcmp(argv[1], "unweighted") != 0 &&
         strcmp(argv[1], "weighted") != 0)) {
        std::fprintf(stderr, "usage: %s unweighted|weighted GRAPH\n", argv[0]);
        return 2;
    }

    bool weighted = strcmp(argv[1], "weighted") == 0;
    Graph *graph = weighted ? load_weighted_graph_from_file(argv[2])
                            : load_graph_from_file(argv[2]);
    if (!graph)
        return 3;

    uint64_t raw_hash = kFnvOffset;
    raw_hash = hash_value(raw_hash, graph->n);
    raw_hash = hash_value(raw_hash, graph->m);
    raw_hash = hash_bytes(raw_hash, graph->row_ptr,
                          static_cast<size_t>(graph->n + 1) * sizeof(int64_t));
    raw_hash = hash_bytes(raw_hash, graph->col_idx,
                          static_cast<size_t>(graph->m) * sizeof(int32_t));
    if (graph->weights)
        raw_hash = hash_bytes(raw_hash, graph->weights,
                              static_cast<size_t>(graph->m) * sizeof(int32_t));

    uint64_t canonical_hash = kFnvOffset;
    for (int64_t vertex = 0; vertex < graph->n; ++vertex) {
        std::vector<std::pair<int32_t, int32_t>> row;
        for (int64_t edge = graph->row_ptr[vertex];
             edge < graph->row_ptr[vertex + 1]; ++edge)
            row.emplace_back(graph->col_idx[edge],
                             graph->weights ? graph->weights[edge] : 1);
        std::sort(row.begin(), row.end());
        canonical_hash = hash_value(canonical_hash, vertex);
        for (const auto &entry : row) {
            canonical_hash = hash_value(canonical_hash, entry.first);
            canonical_hash = hash_value(canonical_hash, entry.second);
        }
    }

    int64_t pair_count = graph_get_num_edge_pairs(graph);
    int32_t *pairs = graph_get_edge_pairs(graph);
    int32_t *pair_weights = graph_get_edge_weights(graph);
    std::vector<std::tuple<int32_t, int32_t, int32_t>> logical;
    logical.reserve(static_cast<size_t>(pair_count));
    for (int64_t edge = 0; edge < pair_count; ++edge)
        logical.emplace_back(pairs[edge * 2], pairs[edge * 2 + 1],
                             pair_weights[edge]);
    std::sort(logical.begin(), logical.end());
    uint64_t logical_hash = kFnvOffset;
    for (const auto &edge : logical) {
        logical_hash = hash_value(logical_hash, std::get<0>(edge));
        logical_hash = hash_value(logical_hash, std::get<1>(edge));
        logical_hash = hash_value(logical_hash, std::get<2>(edge));
    }

    uint64_t node_cardinality = roaring_bitmap_get_cardinality(
        static_cast<RoaringBitmap *>(graph_get_node_bitmap(graph)));
    uint64_t edge_cardinality = roaring_bitmap_get_cardinality(
        static_cast<RoaringBitmap *>(graph_get_edge_bitmap(graph)));
    std::printf("%lld %lld %lld %llu %llu %llu %llu %llu\n",
                static_cast<long long>(graph->n),
                static_cast<long long>(graph->m),
                static_cast<long long>(pair_count),
                static_cast<unsigned long long>(canonical_hash),
                static_cast<unsigned long long>(logical_hash),
                static_cast<unsigned long long>(node_cardinality),
                static_cast<unsigned long long>(edge_cardinality),
                static_cast<unsigned long long>(raw_hash));
    return 0;
}
