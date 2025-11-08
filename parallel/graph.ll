Captured Inputs:
  - 0xbf8450
  - 0xc066f8
  - 0xbfe280
  - 0xbfeae0
  - 0xbff370
  - 0xc02290
Captured Inputs:
  -   %a = alloca [5 x i32], align 4
  -   %i.0 = phi i32 [ 0, %entry ], [ %addtmp26, %loopbody ]
  -   %b = alloca [5 x i32], align 4
  -   %d = alloca [5 x i32], align 4
  -   %c = alloca [5 x i32], align 4
  - @.fmt_int = private constant [4 x i8] c"%d\0A\00"
Captured Outputs:
  -   %addtmp26 = add i32 %i.0, 1
; ModuleID = 'my_module'
source_filename = "my_module"

%env.struct = type { ptr, ptr, ptr, ptr, ptr }

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %addtmp26.loc = alloca i32, align 4
  %c = alloca [5 x i32], align 4
  %d = alloca [5 x i32], align 4
  %b = alloca [5 x i32], align 4
  %a = alloca [5 x i32], align 4
  %a_elem = getelementptr [5 x i32], ptr %a, i32 0, i32 0
  store i32 1, ptr %a_elem, align 4
  %a_elem1 = getelementptr [5 x i32], ptr %a, i32 0, i32 1
  store i32 2, ptr %a_elem1, align 4
  %a_elem2 = getelementptr [5 x i32], ptr %a, i32 0, i32 2
  store i32 3, ptr %a_elem2, align 4
  %a_elem3 = getelementptr [5 x i32], ptr %a, i32 0, i32 3
  store i32 4, ptr %a_elem3, align 4
  %a_elem4 = getelementptr [5 x i32], ptr %a, i32 0, i32 4
  store i32 5, ptr %a_elem4, align 4
  %b_elem = getelementptr [5 x i32], ptr %b, i32 0, i32 0
  store i32 1, ptr %b_elem, align 4
  %b_elem5 = getelementptr [5 x i32], ptr %b, i32 0, i32 1
  store i32 2, ptr %b_elem5, align 4
  %b_elem6 = getelementptr [5 x i32], ptr %b, i32 0, i32 2
  store i32 3, ptr %b_elem6, align 4
  %b_elem7 = getelementptr [5 x i32], ptr %b, i32 0, i32 3
  store i32 4, ptr %b_elem7, align 4
  %b_elem8 = getelementptr [5 x i32], ptr %b, i32 0, i32 4
  store i32 5, ptr %b_elem8, align 4
  %d_elem = getelementptr [5 x i32], ptr %d, i32 0, i32 0
  store i32 2, ptr %d_elem, align 4
  %d_elem9 = getelementptr [5 x i32], ptr %d, i32 0, i32 1
  store i32 2, ptr %d_elem9, align 4
  %d_elem10 = getelementptr [5 x i32], ptr %d, i32 0, i32 2
  store i32 2, ptr %d_elem10, align 4
  %d_elem11 = getelementptr [5 x i32], ptr %d, i32 0, i32 3
  store i32 2, ptr %d_elem11, align 4
  %d_elem12 = getelementptr [5 x i32], ptr %d, i32 0, i32 4
  store i32 2, ptr %d_elem12, align 4
  %c_elem = getelementptr [5 x i32], ptr %c, i32 0, i32 0
  store i32 0, ptr %c_elem, align 4
  %c_elem13 = getelementptr [5 x i32], ptr %c, i32 0, i32 1
  store i32 0, ptr %c_elem13, align 4
  %c_elem14 = getelementptr [5 x i32], ptr %c, i32 0, i32 2
  store i32 0, ptr %c_elem14, align 4
  %c_elem15 = getelementptr [5 x i32], ptr %c, i32 0, i32 3
  store i32 0, ptr %c_elem15, align 4
  %c_elem16 = getelementptr [5 x i32], ptr %c, i32 0, i32 4
  store i32 0, ptr %c_elem16, align 4
  %env_raw = call ptr @malloc(i64 40)
  %env_gep = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 0
  store ptr %a, ptr %env_gep, align 8
  %env_gep27 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 1
  store ptr %b, ptr %env_gep27, align 8
  %env_gep28 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 2
  store ptr %d, ptr %env_gep28, align 8
  %env_gep29 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 3
  store ptr %c, ptr %env_gep29, align 8
  %env_gep30 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 4
  store ptr %addtmp26.loc, ptr %env_gep30, align 8
  call void @parallel_for_runtime(i64 0, i64 5, i64 1, ptr @wrapper, ptr %env_raw)
  ret i32 0
}

declare i32 @printf(ptr, ...)

define dso_local void @outlined_main_loopbody(ptr %a, i32 %i.0, ptr %b, ptr %d, ptr %c, ptr %addtmp26.out) #0 {
newFuncRoot:
  %a_elemptr = getelementptr [5 x i32], ptr %a, i32 0, i32 %i.0
  %a_loadelem = load i32, ptr %a_elemptr, align 4
  %b_elemptr = getelementptr [5 x i32], ptr %b, i32 0, i32 %i.0
  %b_loadelem = load i32, ptr %b_elemptr, align 4
  %d_elemptr = getelementptr [5 x i32], ptr %d, i32 0, i32 %i.0
  %d_loadelem = load i32, ptr %d_elemptr, align 4
  %multmp = mul i32 %b_loadelem, %d_loadelem
  %addtmp = add i32 %a_loadelem, %multmp
  %c_elemptr = getelementptr [5 x i32], ptr %c, i32 0, i32 %i.0
  store i32 %addtmp, ptr %c_elemptr, align 4
  %c_elemptr23 = getelementptr [5 x i32], ptr %c, i32 0, i32 %i.0
  %c_loadelem = load i32, ptr %c_elemptr23, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %c_loadelem)
  %addtmp26 = add i32 %i.0, 1
  store i32 %addtmp26, ptr %addtmp26.out, align 4
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
  %fgep3 = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 2
  %fload4 = load ptr, ptr %fgep3, align 8
  %fgep5 = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 3
  %fload6 = load ptr, ptr %fgep5, align 8
  %fgep7 = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 4
  %fload8 = load ptr, ptr %fgep7, align 8
  %idxcast = trunc i64 %idx to i32
  call void @outlined_main_loopbody(ptr %fload, i32 %idxcast, ptr %fload2, ptr %fload4, ptr %fload6, ptr %fload8)
  ret void
}

declare void @parallel_for_runtime(i64, i64, i64, ptr, ptr)

attributes #0 = { "outlined-loop" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
