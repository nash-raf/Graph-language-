#include <cstdint>
#include <cstdio>

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
};

extern "C" Graph *load_graph_from_file(const char *filename);

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s GRAPH\n", argv[0]);
        return 2;
    }
    Graph *graph = load_graph_from_file(argv[1]);
    if (!graph)
        return 3;
    std::printf("%lld\n", static_cast<long long>(graph->n));
    return 0;
}
