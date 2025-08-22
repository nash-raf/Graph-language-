; ModuleID = 'test-meta.ll'
source_filename = "my_module"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  br label %loopbody

loopbody:                                         ; preds = %entry, %loopbody
  %x.01 = phi i32 [ 10, %entry ], [ %subtmp, %loopbody ]
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %x.01)
  %subtmp = sub nuw nsw i32 %x.01, 1
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

!0 = distinct !{!0, !"DOALL", !1, !2}
!1 = !{!"llvm.loop.parallelize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.enable", i1 true}
