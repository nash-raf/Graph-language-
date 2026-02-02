// chromacity_runtime.c
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>

extern "C"
{

    struct Graph
    {
        int64_t n;        // number of vertices
        int64_t m;        // number of edges
        int64_t *row_ptr; // CSR row pointer (size n+1)
        int32_t *col_idx; // CSR column indices (size m)
    };

    // Greedy graph coloring on CSR
    int chromacity_runtime(Graph *g, int **out_color)
    {
        if (!g) return -1;

        int64_t N = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        if (N == 0) return 0;

        // Allocate color array
        int *color = (int*)malloc(N * sizeof(int));
        for (int64_t i = 0; i < N; i++) color[i] = -1;

        // Find max degree (size of C array)
        int maxDeg = 0;
        for (int64_t i = 0; i < N; i++) {
            int deg = (int)(row[i+1] - row[i]);
            if (deg > maxDeg) maxDeg = deg;
        }

        int *C = (int*)malloc((maxDeg + 1) * sizeof(int));
        memset(C, 0, (maxDeg + 1) * sizeof(int));

        // First vertex
        color[0] = 0;
        int colorNum = 0;

        // Loop over vertices
        for (int64_t u = 1; u < N; u++)
        {
            int deg = (int)(row[u+1] - row[u]);

            // mark neighbor colors
            for (int64_t j = row[u]; j < row[u+1]; j++)
            {
                int v = col[j];
                if (color[v] != -1)
                    C[color[v]] = 1;
            }

            // assign smallest available color
            for (int c = 0; c <= maxDeg; c++)
            {
                if (C[c] == 0)
                {
                    color[u] = c;
                    if (c > colorNum) colorNum = c;
                    break;
                }
            }

            // unmark neighbor colors
            for (int64_t j = row[u]; j < row[u+1]; j++)
            {
                int v = col[j];
                if (color[v] != -1)
                    C[color[v]] = 0;
            }
        }

        printf("[Chromacity] color number = %d\n", colorNum);

        free(C);

        // return full color array if caller wants it
        if (out_color) {
            *out_color = color;
        } else {
            free(color);
        }

        return colorNum;
    }

} // extern "C"
