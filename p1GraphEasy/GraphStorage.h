#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <algorithm>

#include <roaring/roaring.hh>
#include <roaring/roaring64map.hh>

namespace gpl {

static inline uint64_t pack_edge_u32(uint32_t u, uint32_t v) {
    return (uint64_t(u) << 32) | uint64_t(v);
}
static inline std::pair<uint32_t,uint32_t> unpack_edge_u32(uint64_t e) {
    return { uint32_t(e >> 32), uint32_t(e & 0xFFFFFFFFu) };
}

enum class GraphLayoutKind {
    PackedCSR,
    BlockedCSR,
};

struct PackedCSR {
    std::vector<uint64_t> row_ptr;
    std::vector<uint32_t> col_idx;   // dense neighbor ids
    std::vector<uint32_t> nodes;     // dense -> original id
};

struct BlockedCSR {
    uint32_t blockSize = 32;
    std::vector<uint64_t> row_ptr;
    std::vector<uint32_t> col_idx;
    std::vector<uint32_t> nodes;     // dense -> original id
};

struct GraphSet {
    roaring::Roaring nodes;       // original node IDs
    roaring::Roaring64Map edges;  // packed (u,v) in original IDs
    bool directed = true;

    void addNode(uint32_t id) { nodes.add(id); }

    void addEdge(uint32_t u, uint32_t v) {
        nodes.add(u);
        nodes.add(v);
        edges.add(pack_edge_u32(u, v));
    }

    uint64_t numNodesApprox() const { return (uint64_t)nodes.cardinality(); }
    uint64_t numEdgesApprox() const { return (uint64_t)edges.cardinality(); }
};

struct GraphMaterialized {
    GraphLayoutKind kind = GraphLayoutKind::PackedCSR;
    PackedCSR packed;
    BlockedCSR blocked;
};

struct GraphAutotuneHints {
    GraphLayoutKind preferred = GraphLayoutKind::PackedCSR;
    uint32_t blockedBlockSize = 32;
};

class GraphAutotuner {
public:
    GraphAutotuneHints choose(const GraphSet& G, const std::string&, const std::string&) const {
        GraphAutotuneHints h;
        const uint64_t n = G.numNodesApprox();
        const uint64_t m = G.numEdgesApprox();
        if (n >= 50000 && m >= 500000) {
            h.preferred = GraphLayoutKind::BlockedCSR;
            h.blockedBlockSize = 64;
        }
        return h;
    }
};

static inline PackedCSR build_packed_csr(const GraphSet& G) {
    PackedCSR out;

    // nodes_orig sorted unique
    out.nodes.reserve(G.nodes.cardinality());
    for (auto it = G.nodes.begin(); it != G.nodes.end(); ++it) out.nodes.push_back(*it);
    std::sort(out.nodes.begin(), out.nodes.end());

    const size_t n = out.nodes.size();
    auto id_to_idx = [&](uint32_t id) -> uint32_t {
        auto it = std::lower_bound(out.nodes.begin(), out.nodes.end(), id);
        if (it == out.nodes.end() || *it != id) throw std::runtime_error("CSR build: node id missing");
        return (uint32_t)(it - out.nodes.begin());
    };

    std::vector<uint64_t> deg(n, 0);
    for (auto it = G.edges.begin(); it != G.edges.end(); ++it) {
        auto [u, v] = unpack_edge_u32(*it);
        (void)v;
        deg[id_to_idx(u)]++;
    }

    out.row_ptr.resize(n + 1, 0);
    for (size_t i = 0; i < n; ++i) out.row_ptr[i + 1] = out.row_ptr[i] + deg[i];

    const uint64_t m = out.row_ptr[n];
    out.col_idx.resize((size_t)m);

    std::vector<uint64_t> cursor = out.row_ptr;
    for (auto it = G.edges.begin(); it != G.edges.end(); ++it) {
        auto [u, v] = unpack_edge_u32(*it);
        uint32_t ui = id_to_idx(u);
        uint32_t vi = id_to_idx(v);
        out.col_idx[(size_t)cursor[ui]++] = vi;
    }

    for (size_t i = 0; i < n; ++i) {
        uint64_t b = out.row_ptr[i], e = out.row_ptr[i + 1];
        std::sort(out.col_idx.begin() + (size_t)b, out.col_idx.begin() + (size_t)e);
    }

    return out;
}

static inline BlockedCSR build_blocked_csr(const GraphSet& G, uint32_t blockSize) {
    BlockedCSR out;
    out.blockSize = blockSize;
    auto packed = build_packed_csr(G);
    out.row_ptr = std::move(packed.row_ptr);
    out.col_idx = std::move(packed.col_idx);
    out.nodes = std::move(packed.nodes);
    return out;
}

} // namespace gpl