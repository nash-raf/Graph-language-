#include "roaring_bitmap.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

// The Graph struct must match the LLVM IR layout exactly:
//   { i64 n, i64 m, i64* row_ptr, i32* col_idx, i32* weights,
//     i32 directed, i64* in_row_ptr, i32* in_col_idx }
struct GraphCSR {
    int64_t n;           // number of vertices
    int64_t m;           // number of directed edges (2x for undirected)
    int64_t *row_ptr;    // size n+1
    int32_t *col_idx;    // size m
    int32_t *weights;    // size m (may be null for unweighted)
    int32_t directed;
    int64_t *in_row_ptr;
    int32_t *in_col_idx;
};

// Builds the transpose (incoming) CSR. Only three things in the language read
// it -- `for each in neighbor`, inDegree() and degree() on a directed graph --
// so it is built on demand rather than for every directed graph at load time.
// A null in_row_ptr means "not built yet", which is why nothing may alias it to
// the forward CSR: that would be indistinguishable from a real transpose and
// would silently iterate out-neighbours instead of in-neighbours.
static void build_incoming_csr(GraphCSR *G)
{
    G->in_row_ptr = reinterpret_cast<int64_t *>(calloc(G->n + 1, sizeof(int64_t)));
    G->in_col_idx = G->m > 0
        ? reinterpret_cast<int32_t *>(malloc(sizeof(int32_t) * G->m))
        : nullptr;

    for (int64_t u = 0; u < G->n; ++u)
    {
        for (int64_t i = G->row_ptr[u]; i < G->row_ptr[u + 1]; ++i)
        {
            int32_t v = G->col_idx[i];
            if (v >= 0 && v < G->n)
                G->in_row_ptr[v + 1]++;
        }
    }
    for (int64_t i = 1; i <= G->n; ++i)
        G->in_row_ptr[i] += G->in_row_ptr[i - 1];

    std::vector<int64_t> next(G->in_row_ptr, G->in_row_ptr + G->n + 1);
    for (int64_t u = 0; u < G->n; ++u)
    {
        for (int64_t i = G->row_ptr[u]; i < G->row_ptr[u + 1]; ++i)
        {
            int32_t v = G->col_idx[i];
            if (v >= 0 && v < G->n)
                G->in_col_idx[next[v]++] = static_cast<int32_t>(u);
        }
    }
}

// Defers the transpose. Undirected graphs never need one (in-neighbours ==
// out-neighbours), so they stay null forever.
static void init_incoming_csr(GraphCSR *G)
{
    if (!G)
        return;
    G->in_row_ptr = nullptr;
    G->in_col_idx = nullptr;
}

// Called by emitted IR immediately before any read of GraphTy fields 6/7.
// Idempotent: a graph whose transpose was embedded at compile time, or already
// built by an earlier call, is left untouched.
extern "C" void graph_ensure_in_csr(void *graphRaw)
{
    auto *G = reinterpret_cast<GraphCSR *>(graphRaw);
    if (!G || !G->directed || G->in_row_ptr)
        return;
    build_incoming_csr(G);
}

// ---------------------------------------------------------------
// neighbors(G, v) -> roaring bitmap of v's neighbors
// ---------------------------------------------------------------
extern "C" uint8_t *graph_neighbors(void *graphRaw, int32_t v)
{
    auto *G = reinterpret_cast<GraphCSR *>(graphRaw);
    if (!G || v < 0 || v >= (int32_t)G->n)
    {
        // Return empty bitmap
        RoaringBitmap *bm = roaring_bitmap_create(256, 8);
        return reinterpret_cast<uint8_t *>(bm);
    }

    int64_t start = G->row_ptr[v];
    int64_t end = G->row_ptr[v + 1];

    RoaringBitmap *bm = roaring_bitmap_create(256, 8);
    for (int64_t i = start; i < end; i++)
    {
        roaring_bitmap_add(bm, static_cast<uint32_t>(G->col_idx[i]));
    }

    return reinterpret_cast<uint8_t *>(bm);
}

// ---------------------------------------------------------------
// subgraph(G, vertexSet) -> induced subgraph as a new GraphCSR*
//
// Given a graph G and a roaring bitmap of vertex IDs, build the
// induced subgraph containing only those vertices and edges
// between them. Vertices are renumbered 0..k-1 in sorted order.
// ---------------------------------------------------------------
extern "C" void *graph_induced_subgraph(void *graphRaw, void *vertexSetRaw)
{
    auto *G = reinterpret_cast<GraphCSR *>(graphRaw);
    auto *vset = reinterpret_cast<RoaringBitmap *>(vertexSetRaw);

    if (!G || !vset)
        return nullptr;

    uint64_t k = roaring_bitmap_get_cardinality(vset);
    if (k == 0)
    {
        // Empty subgraph
        auto *sub = reinterpret_cast<GraphCSR *>(malloc(sizeof(GraphCSR)));
        sub->n = 0;
        sub->m = 0;
        sub->row_ptr = reinterpret_cast<int64_t *>(malloc(sizeof(int64_t)));
        sub->row_ptr[0] = 0;
        sub->col_idx = nullptr;
        sub->weights = nullptr;
        sub->directed = G->directed;
        init_incoming_csr(sub);
        return sub;
    }

    // Build sorted vertex list and a map: original_id -> new_id
    // Using a simple array since vertex IDs are small (graph vertices)
    std::vector<int32_t> vertices(k);
    for (uint32_t i = 0; i < k; i++)
    {
        vertices[i] = static_cast<int32_t>(roaring_bitmap_get_at_index(vset, i));
    }

    // Find max vertex ID for the map
    int32_t maxId = 0;
    for (auto v : vertices)
        if (v > maxId) maxId = v;

    // Create reverse map: original_id -> new_id (-1 = not in set)
    std::vector<int32_t> newId(maxId + 1, -1);
    for (uint32_t i = 0; i < k; i++)
    {
        newId[vertices[i]] = static_cast<int32_t>(i);
    }

    // First pass: count edges for each new vertex
    std::vector<int64_t> degree(k, 0);
    int64_t totalEdges = 0;

    for (uint32_t i = 0; i < k; i++)
    {
        int32_t u = vertices[i];
        if (u >= G->n) continue;
        int64_t start = G->row_ptr[u];
        int64_t end = G->row_ptr[u + 1];
        for (int64_t j = start; j < end; j++)
        {
            int32_t v = G->col_idx[j];
            if (v <= maxId && newId[v] >= 0)
            {
                degree[i]++;
                totalEdges++;
            }
        }
    }

    // Build CSR
    auto *sub = reinterpret_cast<GraphCSR *>(malloc(sizeof(GraphCSR)));
    sub->n = static_cast<int64_t>(k);
    sub->m = totalEdges;
    sub->row_ptr = reinterpret_cast<int64_t *>(malloc(sizeof(int64_t) * (k + 1)));
    sub->col_idx = totalEdges > 0
        ? reinterpret_cast<int32_t *>(malloc(sizeof(int32_t) * totalEdges))
        : nullptr;
    sub->weights = (G->weights && totalEdges > 0)
        ? reinterpret_cast<int32_t *>(malloc(sizeof(int32_t) * totalEdges))
        : nullptr;
    sub->directed = G->directed;

    // Build row_ptr via prefix sum
    sub->row_ptr[0] = 0;
    for (uint32_t i = 0; i < k; i++)
    {
        sub->row_ptr[i + 1] = sub->row_ptr[i] + degree[i];
    }

    // Second pass: fill col_idx with remapped IDs
    std::vector<int64_t> offset(k, 0);
    for (uint32_t i = 0; i < k; i++)
    {
        int32_t u = vertices[i];
        if (u >= G->n) continue;
        int64_t start = G->row_ptr[u];
        int64_t end = G->row_ptr[u + 1];
        for (int64_t j = start; j < end; j++)
        {
            int32_t v = G->col_idx[j];
            if (v <= maxId && newId[v] >= 0)
            {
                int64_t pos = sub->row_ptr[i] + offset[i];
                sub->col_idx[pos] = newId[v];
                if (sub->weights)
                    sub->weights[pos] = G->weights[j];
                offset[i]++;
            }
        }
    }

    init_incoming_csr(sub);
    return sub;
}

// ---------------------------------------------------------------
// orient(G, order) -> DAG where edges point from lower to higher rank
//
// order[v] = rank of vertex v. Edge u->v is kept only if order[u] < order[v].
// Returns a new GraphCSR* (same vertex numbering, just fewer edges).
// ---------------------------------------------------------------
extern "C" void *graph_orient(void *graphRaw, int32_t *order)
{
    auto *G = reinterpret_cast<GraphCSR *>(graphRaw);
    if (!G || !order) return nullptr;

    int64_t n = G->n;

    // First pass: count edges per vertex in DAG
    std::vector<int64_t> degree(n, 0);
    int64_t totalEdges = 0;

    for (int64_t u = 0; u < n; u++)
    {
        int64_t start = G->row_ptr[u];
        int64_t end = G->row_ptr[u + 1];
        for (int64_t j = start; j < end; j++)
        {
            int32_t v = G->col_idx[j];
            if (order[u] < order[v])
            {
                degree[u]++;
                totalEdges++;
            }
        }
    }

    // Build CSR
    auto *dag = reinterpret_cast<GraphCSR *>(malloc(sizeof(GraphCSR)));
    dag->n = n;
    dag->m = totalEdges;
    dag->row_ptr = reinterpret_cast<int64_t *>(malloc(sizeof(int64_t) * (n + 1)));
    dag->col_idx = totalEdges > 0
        ? reinterpret_cast<int32_t *>(malloc(sizeof(int32_t) * totalEdges))
        : nullptr;
    dag->weights = (G->weights && totalEdges > 0)
        ? reinterpret_cast<int32_t *>(malloc(sizeof(int32_t) * totalEdges))
        : nullptr;
    dag->directed = 1;

    // Build row_ptr
    dag->row_ptr[0] = 0;
    for (int64_t i = 0; i < n; i++)
    {
        dag->row_ptr[i + 1] = dag->row_ptr[i] + degree[i];
    }

    // Fill col_idx
    std::vector<int64_t> offset(n, 0);
    for (int64_t u = 0; u < n; u++)
    {
        int64_t start = G->row_ptr[u];
        int64_t end = G->row_ptr[u + 1];
        for (int64_t j = start; j < end; j++)
        {
            int32_t v = G->col_idx[j];
            if (order[u] < order[v])
            {
                int64_t pos = dag->row_ptr[u] + offset[u];
                dag->col_idx[pos] = v;
                if (dag->weights)
                    dag->weights[pos] = G->weights[j];
                offset[u]++;
            }
        }
    }

    init_incoming_csr(dag);
    return dag;
}

// ---------------------------------------------------------------
// degreeOrder(G) -> int32_t* array where order[v] = rank by degree
// Lower degree gets lower rank. Ties broken by vertex ID.
// ---------------------------------------------------------------
extern "C" int32_t *graph_degree_order(void *graphRaw)
{
    auto *G = reinterpret_cast<GraphCSR *>(graphRaw);
    if (!G) return nullptr;

    int64_t n = G->n;

    // Build (degree, vertex) pairs
    struct DegVtx { int64_t degree; int32_t vertex; };
    std::vector<DegVtx> pairs(n);
    for (int64_t i = 0; i < n; i++)
    {
        pairs[i].degree = G->row_ptr[i + 1] - G->row_ptr[i];
        pairs[i].vertex = static_cast<int32_t>(i);
    }

    // Sort by degree ascending, then vertex ID ascending
    std::sort(pairs.begin(), pairs.end(), [](const DegVtx &a, const DegVtx &b) {
        if (a.degree != b.degree) return a.degree < b.degree;
        return a.vertex < b.vertex;
    });

    // Build order array: order[vertex] = rank
    auto *order = reinterpret_cast<int32_t *>(malloc(sizeof(int32_t) * n));
    for (int64_t rank = 0; rank < n; rank++)
    {
        order[pairs[rank].vertex] = static_cast<int32_t>(rank);
    }

    return order;
}
