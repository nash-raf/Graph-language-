; ModuleID = 'test.ll'
source_filename = "my_module"

%main.env = type {}

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %loop.env = alloca %main.env, align 8
  call void @parallel_for_runtime(i64 1000000, i64 0, i64 -1, ptr @main.outlined, ptr null)
  br label %loopbody

loopbody:                                         ; preds = %entry, %loopbody
  %x.01 = phi i32 [ 1000000, %entry ], [ %subtmp, %loopbody ]
  %subtmp = sub nuw nsw i32 %x.01, 1
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %subtmp)
  %gttmp = icmp ugt i32 %subtmp, 0
  br i1 %gttmp, label %loopbody, label %loopmerge, !llvm.loop !0

loopmerge:                                        ; preds = %loopbody
  %addtmp = add i32 0, 3
  ret i32 %addtmp
}

define i32 @main.1() {
entry:
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare void @parallel_for_runtime(i64, i64, i64, ptr, ptr)

; Function Attrs: nounwind
define internal void @main.outlined(i64 %idx, ptr %env) #0 {
entry:
  %idx_trunc = trunc i64 %idx to i32
  br label %loopbody.out

exit:                                             ; preds = %loopbody.out
  ret void

loopbody.out:                                     ; preds = %entry, %loopbody.out
  %0 = sub nuw nsw i32 %idx_trunc, 1
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %0)
  %2 = icmp ugt i32 %0, 0
  br i1 %2, label %loopbody.out, label %exit
}

attributes #0 = { nounwind }

!0 = distinct !{!0, !"DOALL", !1, !2}
!1 = !{!"llvm.loop.parallelize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.enable", i1 true}
