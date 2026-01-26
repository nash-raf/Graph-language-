; ModuleID = 'my_module'
source_filename = "my_module"

%struct.Graph = type { i64, i64, ptr, ptr, ptr }

@g.rowptr.rodata = private constant [6 x i64] [i64 0, i64 3, i64 5, i64 5, i64 5, i64 5]
@g.colidx.rodata = private constant [5 x i32] [i32 1, i32 2, i32 3, i32 2, i32 4]
@g.nodesorig.rodata = private constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 5]

define i32 @main() {
entry:
  %g.graph = alloca %struct.Graph, align 8
  %rp_raw = call ptr @malloc(i64 48)
  %ci_raw = call ptr @malloc(i64 20)
  %nodes_raw = call ptr @malloc(i64 20)
  call void @llvm.memcpy.p0.p0.i64(ptr %rp_raw, ptr @g.rowptr.rodata, i64 48, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr %ci_raw, ptr @g.colidx.rodata, i64 20, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr %nodes_raw, ptr @g.nodesorig.rodata, i64 20, i1 false)
  %0 = getelementptr inbounds %struct.Graph, ptr %g.graph, i32 0, i32 0
  store i64 5, ptr %0, align 4
  %1 = getelementptr inbounds %struct.Graph, ptr %g.graph, i32 0, i32 1
  store i64 5, ptr %1, align 4
  %2 = getelementptr inbounds %struct.Graph, ptr %g.graph, i32 0, i32 2
  store ptr %rp_raw, ptr %2, align 8
  %3 = getelementptr inbounds %struct.Graph, ptr %g.graph, i32 0, i32 3
  store ptr %ci_raw, ptr %3, align 8
  %4 = getelementptr inbounds %struct.Graph, ptr %g.graph, i32 0, i32 4
  store ptr %nodes_raw, ptr %4, align 8
  ret i32 0
}

declare ptr @malloc(i64)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
