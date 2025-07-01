#include "def.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

// Hash function for Edge_t
size_t h(const Edge_t& e) {
    int s_ = e.s < e.t ? e.s : e.t;
    int t_ = e.s < e.t ? e.t : e.s;
    size_t hash = 1;
    std::hash<int> H;

    hash ^= H(s_) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= H(t_) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

// Edge_t constructors
Edge_t::Edge_t() = default;

Edge_t::Edge_t(int s, int t, bool directed) {
    if (directed) {
        this->s = s;
        this->t = t;
    } else {
        this->s = s < t ? s : t;
        this->t = s < t ? t : s;
    }
}

Edge_t::Edge_t(int s, int t, float w, bool directed) : weight(w) {
    if (directed) {
        this->s = s;
        this->t = t;
    } else {
        this->s = s < t ? s : t;
        this->t = s < t ? t : s;
    }
}

// Comparison operators for Edge_t
bool Edge_t::operator<(const Edge_t& e) const {
    return (this->s < e.s) || (this->s == e.s && this->t < e.t);
}

bool Edge_t::operator==(const Edge_t& e) const {
    return this->s == e.s && this->t == e.t;
}

// Hash function operator for Edge_t
size_t Edge_t::Hash_Edge_t::operator()(const Edge_t& e) const {
    return h(e);
}

// HashMap_t constructor and destructor
HashMap_t::HashMap_t() {
    table = new HashItem_t*[TAB_SIZE];
    table_size = new int[TAB_SIZE]();
}

HashMap_t::~HashMap_t() {
    for (size_t i = 0; i < TAB_SIZE; ++i) {
        if (table_size[i] > 0) delete[] table[i];
    }
    delete[] table;
    delete[] table_size;
}

// Check existence of an edge key in the hashmap
int HashMap_t::exist(const Edge_t& key) {
    size_t pos = h(key) % TAB_SIZE;
    if (table_size[pos] > 0) {
        for (int i = 0; i < table_size[pos]; i++) {
            const Edge_t& e = table[pos][i].key;
            if ((e.s == key.s && e.t == key.t) || (e.s == key.t && e.t == key.s))
                return table[pos][i].val;
        }
    }
    return -1;
}

// Insert edge key and value into hashmap
void HashMap_t::insert(const Edge_t& key, int val) {
    size_t pos = h(key) % TAB_SIZE;

    if (table_size[pos] == 0)
        table[pos] = new HashItem_t[MAX_COLL];

    assert(table_size[pos] < MAX_COLL);
    table[pos][table_size[pos]].key = key;
    table[pos][table_size[pos]++].val = val;
}

// Remove an edge key from hashmap
void HashMap_t::remove(const Edge_t& key) {
    size_t pos = h(key) % TAB_SIZE;
    if (table_size[pos] > 0) {
        for (int i = 0; i < table_size[pos]; i++) {
            if ((table[pos][i].key == key)) {
                table[pos][i] = table[pos][--table_size[pos]];
                if (table_size[pos] == 0) delete[] table[pos];
                return;
            }
        }
    }
    exit(-1); // key not found
}

// EBBkC_Graph_t constructor
EBBkC_Graph_t::EBBkC_Graph_t(int v, int e, bool dir, bool w, bool stoch)
    : v_size(v), e_size(e), directed(dir), weighted(w), stochastic(stoch) {
    G_deg = new int[v_size]();
    cd = new int[v_size + 1]();
    adj = new int[2 * e_size]();
    weights = nullptr;
    stochastic_weights = nullptr;
    if (weighted) weights = new float[2 * e_size]();
    if (stochastic) stochastic_weights = new float[2 * e_size]();
}

// EBBkC_Graph_t destructor
EBBkC_Graph_t::~EBBkC_Graph_t() {
    delete[] G_deg;
    delete[] cd;
    delete[] adj;
    if (weights) delete[] weights;
    if (stochastic_weights) delete[] stochastic_weights;
}

// Build graph from edge list
void EBBkC_Graph_t::buildGraph(const std::vector<Edge_t>& edges) {
    int* deg = new int[v_size]();

    // Compute degrees
    for (const auto& e : edges) {
        deg[e.s]++;
        deg[e.t]++;
    }

    cd[0] = 0;
    for (int i = 1; i <= v_size; ++i) {
        cd[i] = cd[i - 1] + deg[i - 1];
        G_deg[i - 1] = deg[i - 1];
        deg[i - 1] = 0;
    }

    // Fill adjacency lists and weights
    for (int i = 0; i < (int)edges.size(); ++i) {
        const Edge_t& e = edges[i];

        int pos1 = cd[e.s] + deg[e.s]++;
        int pos2 = cd[e.t] + deg[e.t]++;

        adj[pos1] = e.t;
        adj[pos2] = e.s;

        if (weighted) {
            weights[pos1] = e.weight;
            weights[pos2] = e.weight;
        }
    }

    delete[] deg;

    if (stochastic) ensureStochastic();
}

// Normalize weights to stochastic weights if needed
bool EBBkC_Graph_t::ensureStochastic() {
    if (!weighted) return false;
    for (int i = 0; i < v_size; ++i) {
        float sum = 0.0f;
        for (int j = cd[i]; j < cd[i + 1]; ++j) sum += weights[j];
        if (sum == 0) continue;
        for (int j = cd[i]; j < cd[i + 1]; ++j) stochastic_weights[j] = weights[j] / sum;
    }
    return true;
}
