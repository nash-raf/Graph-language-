#ifndef DEF_H
#define DEF_H

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

#define N_EDGES 200000000
#define N_NODES 10000000

class Edge_t {
public:
    int s = 0;
    int t = 0;
    float weight = 1.0f;

    Edge_t();
    Edge_t(int s, int t, bool directed);
    Edge_t(int s, int t, float weight, bool directed);

    bool operator<(const Edge_t& e) const;
    bool operator==(const Edge_t& e) const;

    struct Hash_Edge_t {
        size_t operator()(const Edge_t& e) const;
    };
};

class HashMap_t {
    struct HashItem_t {
        Edge_t key;
        int val = -1;
    };

private:
    const size_t TAB_SIZE = 0x7fffff;
    const size_t MAX_COLL = 100;
    int* table_size;
    HashItem_t** table;

public:
    HashMap_t();
    ~HashMap_t();

    int exist(const Edge_t& key);
    void insert(const Edge_t& key, int val);
    void remove(const Edge_t& key);
};

class EBBkC_Graph_t {
public:
    int v_size = 0;
    int e_size = 0;
    bool directed = false;
    bool weighted = false;
    bool stochastic = false;

    int* G_deg = nullptr;
    int* cd = nullptr;
    int* adj = nullptr;
    float* weights = nullptr;
    float* stochastic_weights = nullptr;

    EBBkC_Graph_t(int v, int e, bool dir, bool w, bool stoch);
    ~EBBkC_Graph_t();

    void buildGraph(const std::vector<Edge_t>& edges);
    bool ensureStochastic();
};

#endif // DEF_H
