Captured Inputs:
  - 0x56b78c5ba6b8
  - 0x56b78c5b26f0
Captured Inputs:
  -   %j.0 = phi i32 [ 0, %entry ], [ %addtmp14, %loopbody ]
  -   %arr = alloca [10 x i32], align 4
Captured Outputs:
  -   %addtmp14 = add i32 %j.0, 1
; ModuleID = 'my_module'
source_filename = "my_module"

%env.struct = type { ptr, ptr }

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %addtmp14.loc = alloca i32, align 4
  %arr = alloca [10 x i32], align 4
  %arr_elem = getelementptr [10 x i32], ptr %arr, i32 0, i32 0
  store i32 1, ptr %arr_elem, align 4
  %arr_elem1 = getelementptr [10 x i32], ptr %arr, i32 0, i32 1
  store i32 2, ptr %arr_elem1, align 4
  %arr_elem2 = getelementptr [10 x i32], ptr %arr, i32 0, i32 2
  store i32 3, ptr %arr_elem2, align 4
  %arr_elem3 = getelementptr [10 x i32], ptr %arr, i32 0, i32 3
  store i32 4, ptr %arr_elem3, align 4
  %arr_elem4 = getelementptr [10 x i32], ptr %arr, i32 0, i32 4
  store i32 5, ptr %arr_elem4, align 4
  %arr_elem5 = getelementptr [10 x i32], ptr %arr, i32 0, i32 5
  store i32 6, ptr %arr_elem5, align 4
  %arr_elem6 = getelementptr [10 x i32], ptr %arr, i32 0, i32 6
  store i32 7, ptr %arr_elem6, align 4
  %arr_elem7 = getelementptr [10 x i32], ptr %arr, i32 0, i32 7
  store i32 8, ptr %arr_elem7, align 4
  %arr_elem8 = getelementptr [10 x i32], ptr %arr, i32 0, i32 8
  store i32 9, ptr %arr_elem8, align 4
  %arr_elem9 = getelementptr [10 x i32], ptr %arr, i32 0, i32 9
  store i32 10, ptr %arr_elem9, align 4
  %env_raw = call ptr @malloc(i64 16)
  %env_gep = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 0
  store ptr %arr, ptr %env_gep, align 8
  %env_gep25 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 1
  store ptr %addtmp14.loc, ptr %env_gep25, align 8
  call void @parallel_for_runtime(i64 0, i64 10, i64 1, ptr @wrapper, ptr %env_raw)
  br label %loopcond15

loopcond15:                                       ; preds = %entry, %loopbody16
  %lttmp19 = icmp slt i32 %addtmp23, 10
  br i1 %lttmp19, label %loopbody16, label %loopmerge17, !my.loop.parallel !0

loopbody16:                                       ; preds = %loopcond15
  %arr_elemptr21 = getelementptr [10 x i32], ptr %arr, i32 0, i32 %addtmp23
  %arr_loadelem = load i32, ptr %arr_elemptr21, align 4
  %addtmp23 = add i32 %addtmp23, 1
  br label %loopcond15

loopmerge17:                                      ; preds = %loopcond15
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %arr_loadelem)
  ret i32 0
}

declare i32 @printf(ptr, ...)

define dso_local void @outlined_main_loopbody(i32 %j.0, ptr %arr, ptr %addtmp14.out) #0 {
newFuncRoot:
  %addtmp = add i32 %j.0, 1
  %arr_elemptr = getelementptr [10 x i32], ptr %arr, i32 0, i32 %j.0
  store i32 %addtmp, ptr %arr_elemptr, align 4
  %addtmp14 = add i32 %j.0, 1
  store i32 %addtmp14, ptr %addtmp14.out, align 4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare ptr @malloc(i64)

define void @wrapper(i64 %idx, ptr %env) {
entry:
  %fgep = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 0
  %fload = load ptr, ptr %fgep, align 8
  %fgep1 = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 1
  %fload2 = load ptr, ptr %fgep1, align 8
  %idxcast = trunc i64 %idx to i32
  call void @outlined_main_loopbody(i32 %idxcast, ptr %fload, ptr %fload2)
  ret void
}

declare void @parallel_for_runtime(i64, i64, i64, ptr, ptr)

attributes #0 = { "outlined-loop" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!0 = !{!"parallel.type=SEQUENTIAL"}
