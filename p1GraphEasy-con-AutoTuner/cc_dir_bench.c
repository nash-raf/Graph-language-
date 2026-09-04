#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

extern "C" {
typedef struct Graph Graph;
typedef void (*sgpl_frontier_pair_fn)(int32_t, int32_t, int64_t, void *);
extern Graph *load_graph_from_file_directed(const char *filename);
extern void autograph_init(void *graph_ptr, int64_t n, int64_t m,
                           void *nodes_bmp, void *edges_bmp,
                           void *edge_pairs_table);
extern int32_t autograph_build_clean_cut(void *graph_ptr, int32_t partitions);
extern int32_t autograph_frontier_step_owner_push(
    void *graph_ptr, const int32_t *frontier, int32_t frontier_size,
    sgpl_frontier_pair_fn work_fn, void *work_env, const uint8_t *membership,
    int32_t *next_frontier, int32_t initial_next_size, int32_t *dest_seen);
extern int32_t autograph_frontier_step_owner_source(
    void *graph_ptr, const int32_t *frontier, int32_t frontier_size,
    sgpl_frontier_pair_fn work_fn, void *work_env, const uint8_t *membership,
    int32_t *next_frontier, int32_t initial_next_size, int32_t *dest_seen);
extern void autograph_ensure_layout(void *graph_ptr, int64_t n, int64_t m,
                                    int64_t *struct_row_ptr,
                                    int32_t *struct_col_idx, void *nodes_bmp,
                                    void *edges_bmp, void *edge_pairs_table,
                                    int32_t target_layout);
}

struct Graph {
    int64_t n;
    int64_t m;
    int64_t *row_ptr;
    int32_t *col_idx;
    int32_t *weights;
    int32_t directed;
};

static double *s_cur, *s_next;
static int32_t *s_deg;
static double s_damp;

void pair_fn(int32_t src, int32_t dst, int64_t idx, void *env) {
    double contrib = s_damp * s_cur[src] / (double)s_deg[src];
    s_next[dst] += contrib;
}

void count_fn(int32_t src, int32_t dst, int64_t idx, void *env) {
    (void)env;
    s_deg[src]++;
}

static double now_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    int rounds = argc > 2 ? atoi(argv[2]) : 20;
    int layout = argc > 3 ? atoi(argv[3]) : 0;
    Graph *g = load_graph_from_file_directed(argv[1]);
    int64_t n = g->n;
    printf("n=%lld m=%lld directed=%d rounds=%d layout=%d\n", (long long)n,
           (long long)g->m, g->directed, rounds, layout);

    double *cur = (double *)malloc(sizeof(double) * n);
    double *next = (double *)malloc(sizeof(double) * n);
    int32_t *deg = (int32_t *)calloc((size_t)n, sizeof(int32_t));
    int32_t *cdeg = (int32_t *)calloc((size_t)n, sizeof(int32_t));
    s_cur = cur; s_next = next; s_deg = cdeg;
    s_damp = 0.85;
    for (int64_t u = 0; u < n; ++u)
        for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j)
            cdeg[u]++;
    double one_over_n = 1.0 / (double)n;
    double beta = (1.0 - s_damp) / (double)n;
    for (int64_t v = 0; v < n; ++v) cur[v] = one_over_n;
    for (int r = 0; r < rounds; ++r) {
        for (int64_t v = 0; v < n; ++v) next[v] = beta;
        for (int64_t u = 0; u < n; ++u) {
            if (!cdeg[u]) continue;
            double c = s_damp * cur[u] / (double)cdeg[u];
            for (int64_t j = g->row_ptr[u]; j < g->row_ptr[u + 1]; ++j)
                next[g->col_idx[j]] += c;
        }
        double *t = cur; cur = next; next = t;
        s_cur = cur; s_next = next;
    }
    double ref0 = cur[0];

    autograph_init(g, n, g->m, NULL, NULL, NULL);
    if (layout != 0)
        autograph_ensure_layout(g, n, g->m, NULL, NULL, NULL, NULL, NULL,
                                layout);
    autograph_build_clean_cut(g, 0);

    memset(deg, 0, (size_t)n * sizeof(int32_t));
    s_deg = deg;
    autograph_frontier_step_owner_source(g, NULL, 0, count_fn, NULL, NULL,
                                         NULL, 0, NULL);
    int deg_match = 1;
    for (int64_t v = 0; v < n; ++v)
        if (deg[v] != cdeg[v]) {
            deg_match = 0;
            break;
        }

    for (int64_t v = 0; v < n; ++v) cur[v] = one_over_n;
    s_cur = cur; s_next = next; s_deg = deg;
    double t1 = now_s();
    for (int r = 0; r < rounds; ++r) {
        for (int64_t v = 0; v < n; ++v) next[v] = beta;
        autograph_frontier_step_owner_push(g, NULL, 0, pair_fn, NULL, NULL,
                                           NULL, 0, NULL);
        double *t = cur; cur = next; next = t;
        s_cur = cur; s_next = next;
    }
    double tcc = now_s() - t1;
    int rank_match = (cur[0] == ref0);
    printf("cc: %.6fs\n", tcc);
    printf("rank0=%.10g ref0=%.10g\n", cur[0], ref0);
    printf("DEG_MATCH=%s RANK_MATCH=%s\n", deg_match ? "YES" : "NO",
           rank_match ? "YES" : "NO");
    return 0;
}