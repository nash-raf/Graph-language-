#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* concat_strings(const char* a, const char* b)
{
    char* result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

static int show_counter = 0;


void show_graph_runtime(int64_t n, int64_t *row_ptr, int32_t *col_idx)
{
    int id = show_counter++;

    char dot_name[64];
    char png_name[64];
    snprintf(dot_name, sizeof(dot_name), "graph_%d.dot", id);
    snprintf(png_name, sizeof(png_name), "graph_%d.png", id);

    FILE *f = fopen(dot_name, "w");
    if (!f) return;

    fprintf(f, "graph G%d {\n", id);

    for (int64_t v = 0; v < n; ++v)
        fprintf(f, "  %ld;\n", v);

    for (int64_t u = 0; u < n; ++u) {
        int64_t start = row_ptr[u];
        int64_t end   = row_ptr[u + 1];
        for (int64_t idx = start; idx < end; ++idx) {
            int32_t v = col_idx[idx];
            if (u < v) {
                fprintf(f, "  %ld -- %d;\n", u, v);
            }
        }
    }

    fprintf(f, "}\n");
    fclose(f);

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "dot -Tpng \"%s\" -o \"%s\" && xdg-open \"%s\" >/dev/null 2>&1 &",
             dot_name, png_name, png_name);
    system(cmd);
}