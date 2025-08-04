; ModuleID = 'my_module'
source_filename = "my_module"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %i = alloca i32, align 4
  %arr3 = alloca [0 x i32], align 4
  %arr2 = alloca [0 x i32], align 4
  %arr = alloca [0 x i32], align 4
  %it = alloca i32, align 4
  store i32 0, ptr %it, align 4
  br label %loopcond

loopcond:                                         ; preds = %loopbody, %entry
  %it1 = load i32, ptr %it, align 4
  %lttmp = icmp slt i32 %it1, 1000000000
  br i1 %lttmp, label %loopbody, label %loopmerge

loopbody:                                         ; preds = %loopcond
  %it2 = load i32, ptr %it, align 4
  %addtmp = add i32 %it2, 1
  store i32 %addtmp, ptr %it, align 4
  br label %loopcond

loopmerge:                                        ; preds = %loopcond
  %it3 = load i32, ptr %it, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %it3)
  store i32 0, ptr %i, align 4
  br label %loopcond4

loopcond4:                                        ; preds = %loopbody5, %loopmerge
  %i7 = load i32, ptr %i, align 4
  %lttmp8 = icmp slt i32 %i7, 1000000000
  br i1 %lttmp8, label %loopbody5, label %loopmerge6

loopbody5:                                        ; preds = %loopcond4
  %i9 = load i32, ptr %i, align 4
  %addtmp10 = add i32 %i9, 1
  store i32 %addtmp10, ptr %i, align 4
  br label %loopcond4

loopmerge6:                                       ; preds = %loopcond4
  ret i32 0
}

declare i32 @printf(ptr, ...)
