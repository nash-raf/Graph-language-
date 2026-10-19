#include "graph_storage_layout.h"
#include "roaring_bitmap.h"
#include <cassert>
#include <cstdio>
extern "C" SgplGraphStorage *load_weighted_graph_from_file_directed(const char *);
extern "C" SgplGraphStorage *load_weighted_graph_from_file(const char *);
extern "C" void *graph_induced_subgraph(void *,void *);
extern "C" void *graph_orient(void *,int32_t *);
extern "C" int32_t graph_get_edge_weight(SgplGraphStorage *,int32_t,int32_t);
static void fresh(SgplGraphStorage *g) {
    assert(g && !g->in_row_ptr && !g->in_col_idx);
    assert(g->row_ptr && g->row_ptr[g->n]==g->m);
}
int main(int argc,char **argv) {
    assert(argc==2);
    auto *g=load_weighted_graph_from_file_directed(argv[1]);fresh(g);
    assert(g->n==3 && g->m==4 && g->directed==1);
    assert(graph_get_edge_weight(g,0,1)==7);
    auto *undirected=load_weighted_graph_from_file(argv[1]);fresh(undirected);
    assert(undirected->n==3 && undirected->m==8 && !undirected->directed);
    auto *selected=roaring_bitmap_create(256,8);
    fresh((SgplGraphStorage *)graph_induced_subgraph(g,selected));
    roaring_bitmap_add(selected,0);roaring_bitmap_add(selected,1);
    auto *sub=(SgplGraphStorage *)graph_induced_subgraph(g,selected);fresh(sub);
    assert(sub->n==2 && sub->m==2 && sub->directed==1);
    int32_t order[]={0,1,2};auto *dag=(SgplGraphStorage *)graph_orient(g,order);fresh(dag);
    assert(dag->n==3 && dag->m==3 && dag->directed==1);
    roaring_bitmap_free(selected);
    puts("PASS Graph ABI, duplicate weights, empty/induced subgraphs and orientation");
}
