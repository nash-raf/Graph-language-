#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "autotuner_runtime.h"

extern "C" {
typedef struct Graph Graph;
extern Graph *load_graph_from_file(const char *filename);
extern void autograph_ensure_layout(void *graph_ptr, int64_t n, int64_t m,
                                    int64_t *struct_row_ptr,
                                    int32_t *struct_col_idx, void *nodes_bmp,
                                    void *edges_bmp, void *edge_pairs_table,
                                    int32_t target_layout);
}

typedef void (*bench_pair_fn)(void *, sgpl_exec_ctx *, int32_t, int32_t);

/* One pass over the clean-cut partition metadata through the exec ABI (the
 * legacy owner step entry points were deleted in R5).  The work function is
 * installed as the pair callback directly: same per-pair call depth as the
 * legacy engine (one indirect call). */
static void bench_step(Graph *g, bench_pair_fn work, int32_t traversal) {
    sgpl_runtime_op *desc = autograph_exec_op_create(
        SGPL_OP_PAIR, NULL, work, NULL, NULL, NULL, NULL, NULL, 0);
    sgpl_runtime_op *ops[1] = {desc};
    sgpl_exec_ctx *ctx = autograph_exec_ctx_create(
        g, traversal, 0, NULL, NULL, NULL, 0, NULL, NULL, 0, ops, 1);
    autograph_frontier_execute(g, ctx);
    autograph_exec_ctx_destroy(ctx);
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

void pair_fn(void *state, sgpl_exec_ctx *ctx, int32_t src, int32_t dst) {
    (void)state; (void)ctx;
    double contrib = s_damp * s_cur[src] / (double)s_deg[src];
    s_next[dst] += contrib;
}

void count_fn(void *state, sgpl_exec_ctx *ctx, int32_t src, int32_t dst) {
    (void)state; (void)ctx; (void)dst;
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
    Graph *g = load_graph_from_file(argv[1]);
    int64_t n = g->n;
    int64_t canon_m = g->m;
    printf("n=%lld m=%lld directed=%d rounds=%d layout=%d\n", (long long)n,
           (long long)g->m, g->directed, rounds, layout);

    /* Serial reference FIRST, on the pristine canonical CSR. */
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
    double ref999 = n > 999999 ? cur[999999] : 0.0;

    /* Register + convert layout. */
    autograph_init(g, n, g->m, NULL, NULL, NULL);
    if (layout != 0)
        autograph_ensure_layout(g, n, g->m, NULL, NULL, NULL, NULL, NULL,
                                layout);
    autograph_build_clean_cut(g, 0);

    /* Source-step deg check under the current layout. */
    memset(deg, 0, (size_t)n * sizeof(int32_t));
    s_deg = deg;
    bench_step(g, count_fn, SGPL_TRAVERSE_OWNER_U);
    int deg_match = 1;
    for (int64_t v = 0; v < n; ++v)
        if (deg[v] != cdeg[v]) {
            deg_match = 0;
            break;
        }

    /* Push pagerank under the current layout. */
    for (int64_t v = 0; v < n; ++v) cur[v] = one_over_n;
    s_cur = cur; s_next = next; s_deg = deg;
    double t1 = now_s();
    for (int r = 0; r < rounds; ++r) {
        for (int64_t v = 0; v < n; ++v) next[v] = beta;
        bench_step(g, pair_fn, SGPL_TRAVERSE_OWNER_V);
        double *t = cur; cur = next; next = t;
        s_cur = cur; s_next = next;
    }
    double tcc = now_s() - t1;

    int rank_match = (cur[0] == ref0) && (n > 999999 ? cur[999999] == ref999 : 1);
    printf("cc: %.6fs\n", tcc);
    printf("rank0=%.10g ref0=%.10g rank999999=%.10g ref999999=%.10g\n",
           cur[0], ref0, n > 999999 ? cur[999999] : 0.0, ref999);
    printf("DEG_MATCH=%s RANK_MATCH=%s\n", deg_match ? "YES" : "NO",
           rank_match ? "YES" : "NO");
    return 0;
}