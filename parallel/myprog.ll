; ModuleID = 'my_module'
source_filename = "my_module"

define i32 @main() {
entry:
  %ar = alloca [10 x i32], align 4
  %ar_elem = getelementptr [10 x i32], ptr %ar, i32 0, i32 0
  store i32 1, ptr %ar_elem, align 4
  %ar_elem1 = getelementptr [10 x i32], ptr %ar, i32 0, i32 1
  store i32 2, ptr %ar_elem1, align 4
  %ar_elem2 = getelementptr [10 x i32], ptr %ar, i32 0, i32 2
  store i32 3, ptr %ar_elem2, align 4
  %ar_elem3 = getelementptr [10 x i32], ptr %ar, i32 0, i32 3
  store i32 4, ptr %ar_elem3, align 4
  %ar_elem4 = getelementptr [10 x i32], ptr %ar, i32 0, i32 4
  store i32 5, ptr %ar_elem4, align 4
  %ar_elem5 = getelementptr [10 x i32], ptr %ar, i32 0, i32 5
  store i32 6, ptr %ar_elem5, align 4
  %ar_elem6 = getelementptr [10 x i32], ptr %ar, i32 0, i32 6
  store i32 7, ptr %ar_elem6, align 4
  %ar_elem7 = getelementptr [10 x i32], ptr %ar, i32 0, i32 7
  store i32 8, ptr %ar_elem7, align 4
  %ar_elem8 = getelementptr [10 x i32], ptr %ar, i32 0, i32 8
  store i32 9, ptr %ar_elem8, align 4
  %ar_elem9 = getelementptr [10 x i32], ptr %ar, i32 0, i32 9
  store i32 10, ptr %ar_elem9, align 4
  br label %loopcond

loopcond:                                         ; preds = %loopbody, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %addtmp16, %loopbody ]
  %sum.0 = phi i32 [ 0, %entry ], [ %addtmp, %loopbody ]
  %lttmp = icmp slt i32 %i.0, 10
  br i1 %lttmp, label %loopbody, label %loopmerge

loopbody:                                         ; preds = %loopcond
  %addtmp = add i32 %sum.0, %i.0
  %ar_elemptr = getelementptr [10 x i32], ptr %ar, i32 0, i32 %addtmp
  store i32 %i.0, ptr %ar_elemptr, align 4
  %addtmp16 = add i32 %i.0, 1
  br label %loopcond

loopmerge:                                        ; preds = %loopcond
  ret i32 0
}
