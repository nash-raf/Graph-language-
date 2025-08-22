; ModuleID = 'test.ll'
source_filename = "my_module"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  br label %loopbody

loopbody:                                         ; preds = %entry, %loopmerge4
  %y.04 = phi i32 [ 3, %entry ], [ %y.1.lcssa, %loopmerge4 ]
  %x.03 = phi i32 [ 10, %entry ], [ %subtmp, %loopmerge4 ]
  %y.045 = zext i32 %y.04 to i64
  %lttmp1 = icmp ult i32 %y.04, 30
  br i1 %lttmp1, label %loopbody3, label %loopmerge4, !llvm.loop !0

loopmerge:                                        ; preds = %loopmerge4
  %y.0.lcssa = phi i32 [ %y.1.lcssa, %loopmerge4 ]
  %addtmp11 = add i32 0, %y.0.lcssa
  ret i32 %addtmp11

loopbody3:                                        ; preds = %loopbody, %loopbody3
  %y.12 = phi i32 [ %addtmp, %loopbody3 ], [ %y.04, %loopbody ]
  %0 = sub i32 29, %y.04
  %1 = zext i32 %0 to i64
  %trip = add i64 %1, 1
  %step_mul_trip = mul i64 1, %trip
  %loop_end = add i64 %y.045, %step_mul_trip
  %addtmp = add nsw i32 %y.12, 1
  %2 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %addtmp)
  %lttmp = icmp slt i32 %addtmp, 30
  br i1 %lttmp, label %loopbody3, label %loopmerge4, !llvm.loop !0

loopmerge4:                                       ; preds = %loopbody3, %loopbody
  %y.1.lcssa = phi i32 [ %y.04, %loopbody ], [ 30, %loopbody3 ]
  %subtmp = sub nuw nsw i32 %x.03, 1
  %gttmp = icmp ugt i32 %subtmp, 0
  br i1 %gttmp, label %loopbody, label %loopmerge, !llvm.loop !3
}

define i32 @main.1() {
entry:
  ret i32 0
}

declare i32 @printf(ptr, ...)

!0 = distinct !{!0, !"DOALL", !1, !2}
!1 = !{!"llvm.loop.parallelize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.enable", i1 true}
!3 = distinct !{!3, !"DOALL", !1, !2}
