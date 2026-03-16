; ModuleID = 'my_module'
source_filename = "my_module"

%struct.Graph = type { i64, i64, ptr, ptr }

@__edge_pairs = private constant [4 x i32] [i32 2, i32 3, i32 2, i32 4]
@g1_filename = private unnamed_addr constant [13 x i8] c"edgelist.txt\00", align 1
@g1 = internal global ptr null
@g2_csr_row = internal constant [4 x i64] [i64 0, i64 2, i64 3, i64 4]
@g2_csr_col = internal constant [4 x i32] [i32 1, i32 2, i32 0, i32 0]
@g2 = internal global ptr null
@g2_nodes_blob = private constant [25 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\03\00\00\00\00\00\00\00\02\00\03\00\04\00"
@g2_edges_blob = private constant [23 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\02\00\00\00\00\00\00\00\00\00\01\00"
@g2_edge_pairs_flat = private constant [4 x i32] [i32 2, i32 3, i32 2, i32 4]
@s1 = internal global ptr null
@k = internal global i32 0
@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %graph_ptr = call ptr @load_graph_from_file(ptr @g1_filename)
  store ptr %graph_ptr, ptr @g1, align 8
  %g1_nodes_bm = call ptr @graph_get_node_bitmap(ptr %graph_ptr)
  %g1_edges_bm = call ptr @graph_get_edge_bitmap(ptr %graph_ptr)
  %edge_pairs_ptr = call ptr @graph_get_edge_pairs(ptr %graph_ptr)
  %edge_pairs_count = call i64 @graph_get_num_edge_pairs(ptr %graph_ptr)
  %0 = getelementptr inbounds nuw %struct.Graph, ptr %graph_ptr, i32 0, i32 0
  %1 = load i64, ptr %0, align 4
  %2 = getelementptr inbounds nuw %struct.Graph, ptr %graph_ptr, i32 0, i32 1
  %3 = load i64, ptr %2, align 4
  call void @autograph_init(ptr %graph_ptr, i64 %1, i64 %3, ptr %g1_nodes_bm, ptr %g1_edges_bm, ptr %edge_pairs_ptr)
  %rp_raw = call ptr @malloc(i64 32)
  %ci_raw = call ptr @malloc(i64 16)
  call void @llvm.memcpy.p0.p0.i64(ptr %rp_raw, ptr @g2_csr_row, i64 32, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr %ci_raw, ptr @g2_csr_col, i64 16, i1 false)
  %graph_raw = call ptr @malloc(i64 32)
  %g_n_ptr = getelementptr inbounds nuw %struct.Graph, ptr %graph_raw, i32 0, i32 0
  store i64 3, ptr %g_n_ptr, align 4
  %g_m_ptr = getelementptr inbounds nuw %struct.Graph, ptr %graph_raw, i32 0, i32 1
  store i64 4, ptr %g_m_ptr, align 4
  %g_rp_ptr = getelementptr inbounds nuw %struct.Graph, ptr %graph_raw, i32 0, i32 2
  store ptr %rp_raw, ptr %g_rp_ptr, align 8
  %g_ci_ptr = getelementptr inbounds nuw %struct.Graph, ptr %graph_raw, i32 0, i32 3
  store ptr %ci_raw, ptr %g_ci_ptr, align 8
  store ptr %graph_raw, ptr @g2, align 8
  %g2_nodes_blob.bitmap = call ptr @roaring_from_serialized(ptr @g2_nodes_blob, i64 25)
  %g2_edges_blob.bitmap = call ptr @roaring_from_serialized(ptr @g2_edges_blob, i64 23)
  call void @autograph_init(ptr %graph_raw, i64 3, i64 4, ptr %g2_nodes_blob.bitmap, ptr %g2_edges_blob.bitmap, ptr @g2_edge_pairs_flat)
  %g1.graph = load ptr, ptr @g1, align 8
  %bfs1_arr = alloca ptr, align 8
  %bfs1_size = alloca i32, align 4
  %bfs1_bfs_ret = call i32 @bfs_runtime(ptr %g1.graph, ptr %bfs1_arr, ptr %bfs1_size)
  %bitmap_array = alloca ptr, i64 2, align 8
  %array_elem_0 = getelementptr ptr, ptr %bitmap_array, i32 0
  store ptr %g1_nodes_bm, ptr %array_elem_0, align 8
  %array_elem_1 = getelementptr ptr, ptr %bitmap_array, i32 1
  store ptr %g2_nodes_blob.bitmap, ptr %array_elem_1, align 8
  %set.union.result = call ptr @roaring_bitmap_union(ptr %bitmap_array, i64 2)
  call void @roaring_print(ptr %set.union.result)
  %set.intersect.result = call ptr @roaring_bitmap_intersect(ptr %g1_nodes_bm, ptr %g2_nodes_blob.bitmap)
  call void @roaring_print(ptr %set.intersect.result)
  %set.intersect.result1 = call ptr @roaring_bitmap_intersect(ptr %g1_edges_bm, ptr %g2_edges_blob.bitmap)
  call void @roaring_print_edges(ptr %set.intersect.result1, ptr @g2_edge_pairs_flat, i64 2)
  %set.lit = call ptr @roaring_bitmap_create(i64 65536, i64 8)
  call void @roaring_bitmap_add(ptr %set.lit, i32 1)
  call void @roaring_bitmap_add(ptr %set.lit, i32 2)
  call void @roaring_bitmap_add(ptr %set.lit, i32 3)
  store ptr %set.lit, ptr @s1, align 8
  %s1.load = load ptr, ptr @s1, align 8
  call void @roaring_bitmap_add(ptr %s1.load, i32 400)
  %s1.load2 = load ptr, ptr @s1, align 8
  call void @roaring_bitmap_add(ptr %s1.load2, i32 500)
  %s1.load3 = load ptr, ptr @s1, align 8
  call void @roaring_bitmap_add(ptr %s1.load3, i32 600)
  %s1.load4 = load ptr, ptr @s1, align 8
  call void @roaring_bitmap_add(ptr %s1.load4, i32 700)
  %s1.load5 = load ptr, ptr @s1, align 8
  call void @roaring_bitmap_remove(ptr %s1.load5, i32 2)
  %s1.bitmap = load ptr, ptr @s1, align 8
  call void @roaring_print(ptr %s1.bitmap)
  %g1.graph6 = load ptr, ptr @g1, align 8
  %bfs2_arr = alloca ptr, align 8
  %bfs2_size = alloca i32, align 4
  %bfs2_bfs_ret = call i32 @bfs_runtime(ptr %g1.graph6, ptr %bfs2_arr, ptr %bfs2_size)
  store i32 12, ptr @k, align 4
  %s1.load7 = load ptr, ptr @s1, align 8
  %contains.result = call i32 @roaring_bitmap_contains(ptr %s1.load7, i32 1)
  %contains.bool = icmp ne i32 %contains.result, 0
  br i1 %contains.bool, label %codeRepl, label %ifcont

codeRepl:                                         ; preds = %entry
  call void @task_2()
  br label %ifcont

ifcont:                                           ; preds = %codeRepl, %entry
  ret i32 0
}

declare ptr @load_graph_from_file(ptr)

declare ptr @graph_get_node_bitmap(ptr)

declare ptr @graph_get_edge_bitmap(ptr)

declare ptr @graph_get_edge_pairs(ptr)

declare i64 @graph_get_num_edge_pairs(ptr)

declare void @autograph_init(ptr, i64, i64, ptr, ptr, ptr)

declare ptr @malloc(i64)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

declare ptr @roaring_from_serialized(ptr, i64)

declare i32 @bfs_runtime(ptr, ptr, ptr)

declare ptr @roaring_bitmap_union(ptr, i64)

declare void @roaring_bitmap_free(ptr)

declare i32 @printf(ptr, ...)

declare void @roaring_print(ptr)

declare ptr @roaring_bitmap_intersect(ptr, ptr)

declare void @roaring_print_edges(ptr, ptr, i64)

declare ptr @roaring_bitmap_create(i64, i64)

declare void @roaring_bitmap_add(ptr, i32)

declare void @roaring_bitmap_remove(ptr, i32)

declare i32 @roaring_bitmap_contains(ptr, i32)

define private void @_control_barrier_ctrl_then_73() {
entry:
  unreachable
}

define private void @_control_barrier_ctrl_else_74() {
entry:
  unreachable
}

define internal void @task_2() {
newFuncRoot:
  %k8 = load i32, ptr @k, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 %k8)
  ret void
}

declare i32 @pthread_create(ptr, ptr, ptr, ptr)

declare i32 @pthread_join(ptr, ptr)

declare i64 @pthread_self()

define internal ptr @task_2_wrapper(ptr %0) {
entry:
  call void @task_2()
  ret ptr null
}

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!graph.ir.backend = !{!0}

!0 = !{!"cpu"}
