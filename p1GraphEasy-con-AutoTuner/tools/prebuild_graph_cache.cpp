#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
};

extern "C" Graph *load_graph_from_file(const char *filename);
extern "C" Graph *load_weighted_graph_from_file(const char *filename);

static std::string cache_path_for(const char *filename, bool weighted) {
    return std::string(filename) + (weighted ? ".weighted.sgplbin" : ".sgplbin");
}

static void usage(const char *argv0) {
    std::fprintf(stderr,
                 "usage: %s [--weighted] [--refresh] <edge_list_path>\n"
                 "  Builds the SGPL .sgplbin sidecar for a text edge list so\n"
                 "  the first program launch hits the warm mmap path.\n",
                 argv0);
}

int main(int argc, char **argv) {
    bool weighted = false;
    bool refresh = false;
    const char *path = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--weighted") == 0) {
            weighted = true;
        } else if (std::strcmp(argv[i], "--refresh") == 0) {
            refresh = true;
        } else if (std::strcmp(argv[i], "-h") == 0 ||
                   std::strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            std::fprintf(stderr, "unknown option: %s\n", argv[i]);
            usage(argv[0]);
            return 2;
        } else if (path == nullptr) {
            path = argv[i];
        } else {
            usage(argv[0]);
            return 2;
        }
    }

    if (path == nullptr) {
        usage(argv[0]);
        return 2;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        std::perror(path);
        return 1;
    }

    // Force a rebuild when requested so compile-time prebuild always refreshes
    // a stale or corrupt sidecar instead of silently reusing it.
    if (refresh)
        setenv("SGPL_GRAPH_CACHE", "refresh", 1);

    Graph *graph = weighted ? load_weighted_graph_from_file(path)
                            : load_graph_from_file(path);
    if (!graph) {
        std::fprintf(stderr, "failed to load graph: %s\n", path);
        return 1;
    }

    const std::string cache_path = cache_path_for(path, weighted);
    if (stat(cache_path.c_str(), &st) != 0) {
        std::fprintf(stderr, "cache was not created: %s\n", cache_path.c_str());
        return 1;
    }

    std::printf("%s\n", cache_path.c_str());
    std::printf("n=%lld m=%lld\n", static_cast<long long>(graph->n),
                static_cast<long long>(graph->m));
    return 0;
}
