Captured Inputs:
  - 0x2dd265a8
  - 0x2dd19690
  - 0x2dd21d60
Captured Inputs:
  -   %i.0 = phi i32 [ 1, %entry ], [ %addtmp20, %loopbody ]
  -   %a = alloca [11 x i32], align 4
  - @.fmt_int = private constant [4 x i8] c"%d\0A\00"
Captured Outputs:
  -   %addtmp20 = add i32 %i.0, 1
; ModuleID = 'my_module'
source_filename = "my_module"

%env.struct = type { ptr, ptr }

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %addtmp20.loc = alloca i32, align 4
  %a = alloca [11 x i32], align 4
  %a_elem = getelementptr [11 x i32], ptr %a, i32 0, i32 0
  store i32 1, ptr %a_elem, align 4
  %a_elem1 = getelementptr [11 x i32], ptr %a, i32 0, i32 1
  store i32 2, ptr %a_elem1, align 4
  %a_elem2 = getelementptr [11 x i32], ptr %a, i32 0, i32 2
  store i32 3, ptr %a_elem2, align 4
  %a_elem3 = getelementptr [11 x i32], ptr %a, i32 0, i32 3
  store i32 4, ptr %a_elem3, align 4
  %a_elem4 = getelementptr [11 x i32], ptr %a, i32 0, i32 4
  store i32 5, ptr %a_elem4, align 4
  %a_elem5 = getelementptr [11 x i32], ptr %a, i32 0, i32 5
  store i32 6, ptr %a_elem5, align 4
  %a_elem6 = getelementptr [11 x i32], ptr %a, i32 0, i32 6
  store i32 7, ptr %a_elem6, align 4
  %a_elem7 = getelementptr [11 x i32], ptr %a, i32 0, i32 7
  store i32 8, ptr %a_elem7, align 4
  %a_elem8 = getelementptr [11 x i32], ptr %a, i32 0, i32 8
  store i32 9, ptr %a_elem8, align 4
  %a_elem9 = getelementptr [11 x i32], ptr %a, i32 0, i32 9
  store i32 10, ptr %a_elem9, align 4
  %a_elem10 = getelementptr [11 x i32], ptr %a, i32 0, i32 10
  store i32 11, ptr %a_elem10, align 4
  %env_raw = call ptr @malloc(i64 16)
  %env_gep = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 0
  store ptr %a, ptr %env_gep, align 8
  %env_gep22 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 1
  store ptr %addtmp20.loc, ptr %env_gep22, align 8
  call void @parallel_for_runtime(i64 1, i64 10, i64 1, ptr @wrapper, ptr %env_raw)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare void @doacross_wait(i64, i64, i32)

declare void @doacross_post(i64, i32)

define dso_local void @outlined_main_loopbody(i32 %i.0, ptr %a, ptr %addtmp20.out) #0 {
newFuncRoot:
  %addtmp = add i32 %i.0, 1
  %a_elemptr = getelementptr [11 x i32], ptr %a, i32 0, i32 %addtmp
  %indvar.i64 = sext i32 %i.0 to i64
  %a_loadelem = load i32, ptr %a_elemptr, align 4, !doacross.post !0, !doacross.id !1
  call void @doacross_post(i64 %indvar.i64, i32 29)
  %a_elemptr14 = getelementptr [11 x i32], ptr %a, i32 0, i32 %i.0
  %indvar.i6421 = sext i32 %i.0 to i64
  call void @doacross_wait(i64 %indvar.i6421, i64 1, i32 29)
  store i32 %a_loadelem, ptr %a_elemptr14, align 4, !doacross.wait !2, !doacross.dist !3, !doacross.src !1
  %a_elemptr16 = getelementptr [11 x i32], ptr %a, i32 0, i32 %i.0
  %a_loadelem17 = load i32, ptr %a_elemptr16, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %a_loadelem17)
  %addtmp20 = add i32 %i.0, 1
  store i32 %addtmp20, ptr %addtmp20.out, align 4
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

!0 = !{!"doacross.post"}
!1 = !{i32 29}
!2 = !{!"doacross.wait"}
!3 = !{i64 1}
