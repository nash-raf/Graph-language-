Captured Inputs:
  - 0x2c004a10
  - 0x2c0113b8
  - 0x2c00a910
  - 0x2c00b130
Captured Inputs:
  -   %a = alloca [5 x i32], align 4
  -   %i.0 = phi i32 [ 0, %entry ], [ %addtmp20, %loopbody ]
  -   %b = alloca [5 x i32], align 4
  -   %c = alloca [5 x i32], align 4
; ModuleID = 'my_module'
source_filename = "my_module"

%env.struct = type { ptr, ptr, ptr, ptr }

define i32 @main() {
entry:
  %addtmp20.loc = alloca i32, align 4
  %c = alloca [5 x i32], align 4
  %b = alloca [5 x i32], align 4
  %a = alloca [5 x i32], align 4
  %a_elem = getelementptr [5 x i32], ptr %a, i32 0, i32 0
  store i32 5, ptr %a_elem, align 4
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
  %c_elem = getelementptr [5 x i32], ptr %c, i32 0, i32 0
  store i32 1, ptr %c_elem, align 4
  %c_elem9 = getelementptr [5 x i32], ptr %c, i32 0, i32 1
  store i32 2, ptr %c_elem9, align 4
  %c_elem10 = getelementptr [5 x i32], ptr %c, i32 0, i32 2
  store i32 3, ptr %c_elem10, align 4
  %c_elem11 = getelementptr [5 x i32], ptr %c, i32 0, i32 3
  store i32 4, ptr %c_elem11, align 4
  %c_elem12 = getelementptr [5 x i32], ptr %c, i32 0, i32 4
  store i32 5, ptr %c_elem12, align 4
  %env_raw = call ptr @malloc(i64 32)
  %env_gep = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 0
  store ptr %a, ptr %env_gep, align 8
  %env_gep21 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 1
  store ptr %b, ptr %env_gep21, align 8
  %env_gep22 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 2
  store ptr %c, ptr %env_gep22, align 8
  %env_gep23 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 3
  store ptr null, ptr %env_gep23, align 8
  call void @parallel_for_runtime(i64 0, i64 5, i64 1, ptr @wrapper, ptr %env_raw)
  br label %loopmerge

loopcond:                                         ; preds = %codeRepl
  %i.0 = phi i32 [ 0, %entry ], [ %addtmp20.reload, %codeRepl ]
  %lttmp = icmp slt i32 %i.0, 5
  br i1 %lttmp, label %codeRepl, label %loopmerge, !my.loop.parallel !0

codeRepl:                                         ; preds = %loopcond
  call void @llvm.lifetime.start.p0(i64 -1, ptr %addtmp20.loc)
  call void @outlined_main_loopbody(ptr %a, i32 %i.0, ptr %b, ptr %c, ptr %addtmp20.loc)
  %addtmp20.reload = load i32, ptr %addtmp20.loc, align 4
  call void @llvm.lifetime.end.p0(i64 -1, ptr %addtmp20.loc)
  br label %loopcond

loopmerge:                                        ; preds = %entry, %loopcond
  ret i32 0
}

define dso_local void @outlined_main_loopbody(ptr %a, i32 %i.0, ptr %b, ptr %c, ptr %addtmp20.out) #0 {
newFuncRoot:
  br label %loopbody

loopbody:                                         ; preds = %newFuncRoot
  %a_elemptr = getelementptr [5 x i32], ptr %a, i32 0, i32 %i.0
  %a_loadelem = load i32, ptr %a_elemptr, align 4
  %b_elemptr = getelementptr [5 x i32], ptr %b, i32 0, i32 %i.0
  %b_loadelem = load i32, ptr %b_elemptr, align 4
  %addtmp = add i32 %a_loadelem, %b_loadelem
  %c_elemptr = getelementptr [5 x i32], ptr %c, i32 0, i32 %i.0
  store i32 %addtmp, ptr %c_elemptr, align 4
  %c_elemptr18 = getelementptr [5 x i32], ptr %c, i32 0, i32 %i.0
  %c_loadelem = load i32, ptr %c_elemptr18, align 4
  %addtmp20 = add i32 %i.0, 1
  store i32 %addtmp20, ptr %addtmp20.out, align 4
  br label %loopcond.exitStub

loopcond.exitStub:                                ; preds = %loopbody
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
  %idxcast = trunc i64 %idx to i32
  call void @outlined_main_loopbody(ptr %fload, i32 %idxcast, ptr %fload2, ptr %fload4, ptr %fload6)
  ret void
}

declare void @parallel_for_runtime(i64, i64, i64, ptr, ptr)

attributes #0 = { "outlined-loop" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!0 = !{!"parallel.type=DOALL"}
