; ModuleID = 'test.ll'
source_filename = "my_module"

%struct.Graph = type { i64, i64, ptr, ptr }

@g_csr_row = internal constant [8 x i64] [i64 0, i64 2, i64 5, i64 8, i64 10, i64 10, i64 10, i64 10]
@g_csr_col = internal constant [10 x i32] [i32 1, i32 2, i32 0, i32 2, i32 3, i32 0, i32 1, i32 3, i32 1, i32 2]

define i32 @main() {
entry:
  %head = alloca i32, align 4
  %tail = alloca i32, align 4
  %i = alloca i64, align 8
  %rp_raw = call ptr @malloc(i64 64)
  %ci_raw = call ptr @malloc(i64 40)
  call void @llvm.memcpy.p0.p0.i64(ptr %rp_raw, ptr @g_csr_row, i64 64, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr %ci_raw, ptr @g_csr_col, i64 40, i1 false)
  %graph_raw = call ptr @malloc(i64 32)
  %g_n_ptr = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 0
  store i64 7, ptr %g_n_ptr, align 4
  %g_m_ptr = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 1
  store i64 10, ptr %g_m_ptr, align 4
  %g_rp_ptr = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 2
  store ptr %rp_raw, ptr %g_rp_ptr, align 8
  %g_ci_ptr = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 3
  store ptr %ci_raw, ptr %g_ci_ptr, align 8
  %g_n_ptr1 = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 0
  %g_rp_ptr2 = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 2
  %g_ci_ptr3 = getelementptr inbounds %struct.Graph, ptr %graph_raw, i32 0, i32 3
  %n_val = load i64, ptr %g_n_ptr1, align 4
  %row_ptr = load ptr, ptr %g_rp_ptr2, align 8
  %col_idx = load ptr, ptr %g_ci_ptr3, align 8
  %0 = mul i64 %n_val, 1
  %rawVisited = call ptr @malloc(i64 %0)
  call void @llvm.memset.p0.i64(ptr align 1 %rawVisited, i8 0, i64 %0, i1 false)
  %1 = mul i64 %n_val, 4
  %rawQueue = call ptr @malloc(i64 %1)
  store i32 0, ptr %head, align 4
  store i32 0, ptr %tail, align 4
  %2 = getelementptr i32, ptr %rawQueue, i32 0
  store i32 0, ptr %2, align 4
  %3 = getelementptr ptr, ptr %rawVisited, i32 0
  store i1 true, ptr %3, align 1
  store i32 1, ptr %tail, align 4
  br label %bfs.cond

bfs.cond:                                         ; preds = %bfs.for.exit, %entry
  %h = load i32, ptr %head, align 4
  %t = load i32, ptr %tail, align 4
  %cond = icmp slt i32 %h, %t
  br i1 %cond, label %bfs.body, label %bfs.exit, !llvm.loop !0

bfs.body:                                         ; preds = %bfs.cond
  %uqptr = getelementptr i32, ptr %rawQueue, i32 %h
  %u = load i32, ptr %uqptr, align 4
  %4 = add i32 %h, 1
  store i32 %4, ptr %head, align 4
  %5 = sext i32 %u to i64
  %6 = getelementptr ptr, ptr %row_ptr, i64 %5
  %rp_u = load i64, ptr %6, align 4
  %7 = add i64 %5, 1
  %8 = getelementptr ptr, ptr %row_ptr, i64 %7
  %rp_u1 = load i64, ptr %8, align 4
  br label %bfs.for.init

bfs.exit:                                         ; preds = %bfs.cond
  call void @free(ptr %rawVisited)
  call void @free(ptr %rawQueue)
  ret i32 0

bfs.for.init:                                     ; preds = %bfs.body
  store i64 %rp_u, ptr %i, align 4
  br label %bfs.for.body

bfs.for.body:                                     ; preds = %bfs.for.cont, %bfs.for.init
  %iVal = load i64, ptr %i, align 4
  %innerCond = icmp slt i64 %iVal, %rp_u1
  br i1 %innerCond, label %bfs.enqueue, label %bfs.for.exit, !llvm.loop !3

bfs.enqueue:                                      ; preds = %bfs.for.body
  %9 = trunc i64 %iVal to i32
  %10 = getelementptr ptr, ptr %col_idx, i32 %9
  %nbr = load i32, ptr %10, align 4
  %vptr = getelementptr ptr, ptr %rawVisited, i32 %nbr
  %wasVisited = load i1, ptr %vptr, align 1
  %11 = icmp eq i1 %wasVisited, false
  br i1 %11, label %bfs.doEnqueue, label %bfs.for.cont

bfs.doEnqueue:                                    ; preds = %bfs.enqueue
  %tailVal = load i32, ptr %tail, align 4
  %12 = getelementptr i32, ptr %rawQueue, i32 %tailVal
  store i32 %nbr, ptr %12, align 4
  store i1 true, ptr %vptr, align 1
  %13 = add i32 %tailVal, 1
  store i32 %13, ptr %tail, align 4
  br label %bfs.for.cont

bfs.for.cont:                                     ; preds = %bfs.doEnqueue, %bfs.enqueue
  %14 = add i64 %iVal, 1
  store i64 %14, ptr %i, align 4
  br label %bfs.for.body

bfs.for.exit:                                     ; preds = %bfs.for.body
  br label %bfs.cond
}

declare ptr @malloc(i64)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

declare void @free(ptr)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!0 = distinct !{!1, !2}
!1 = !{!"llvm.loop.parallelize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.enable", i1 true}
!3 = distinct !{!1, !2}
