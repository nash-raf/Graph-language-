; ModuleID = 'my_module'
source_filename = "my_module"

%struct.Graph.0 = type { i64, i64, ptr, ptr, ptr }

@g_csr_row = internal constant [7 x i64] [i64 0, i64 1, i64 3, i64 4, i64 6, i64 7, i64 8]
@g_csr_col = internal constant [8 x i32] [i32 1, i32 0, i32 4, i32 3, i32 2, i32 5, i32 1, i32 3]
@g_w_col = internal constant [8 x i32] [i32 1, i32 1, i32 3, i32 2, i32 2, i32 4, i32 3, i32 4]
@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %v.idx64 = alloca i64, align 8
  %v = alloca i32, align 4
  %g = alloca i32, align 4
  %rp_raw = call ptr @malloc(i64 56)
  %ci_raw = call ptr @malloc(i64 32)
  %w_raw = call ptr @malloc(i64 32)
  call void @llvm.memcpy.p0.p0.i64(ptr %rp_raw, ptr @g_csr_row, i64 56, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr %ci_raw, ptr @g_csr_col, i64 32, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr %w_raw, ptr @g_w_col, i64 32, i1 false)
  %graph_raw = call ptr @malloc(i64 40)
  %g_n_ptr = getelementptr inbounds nuw %struct.Graph.0, ptr %graph_raw, i32 0, i32 0
  store i64 6, ptr %g_n_ptr, align 4
  %g_m_ptr = getelementptr inbounds nuw %struct.Graph.0, ptr %graph_raw, i32 0, i32 1
  store i64 8, ptr %g_m_ptr, align 4
  %g_rp_ptr = getelementptr inbounds nuw %struct.Graph.0, ptr %graph_raw, i32 0, i32 2
  store ptr %rp_raw, ptr %g_rp_ptr, align 8
  %g_ci_ptr = getelementptr inbounds nuw %struct.Graph.0, ptr %graph_raw, i32 0, i32 3
  store ptr %ci_raw, ptr %g_ci_ptr, align 8
  %g_w_ptr = getelementptr inbounds nuw %struct.Graph.0, ptr %graph_raw, i32 0, i32 4
  store ptr %w_raw, ptr %g_w_ptr, align 8
  store ptr %graph_raw, ptr %g, align 8
  %g_n_ptr1 = getelementptr inbounds nuw %struct.Graph.0, ptr %graph_raw, i32 0, i32 0
  %n_val = load i64, ptr %g_n_ptr1, align 4
  store i64 0, ptr %v.idx64, align 4
  br label %foreach.cond

foreach.cond:                                     ; preds = %foreach.inc, %entry
  %idx_val = load i64, ptr %v.idx64, align 4
  %loop_cond = icmp slt i64 %idx_val, %n_val
  br i1 %loop_cond, label %foreach.body, label %foreach.merge

foreach.body:                                     ; preds = %foreach.cond
  %idx32 = trunc i64 %idx_val to i32
  store i32 %idx32, ptr %v, align 4
  %v2 = load i32, ptr %v, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %v2)
  br label %foreach.inc

foreach.inc:                                      ; preds = %foreach.body
  %idx_next = add i64 %idx_val, 1
  store i64 %idx_next, ptr %v.idx64, align 4
  br label %foreach.cond

foreach.merge:                                    ; preds = %foreach.cond
  ret i32 0
}

declare ptr @malloc(i64)

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

declare i32 @printf(ptr, ...)

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
