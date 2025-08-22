; ModuleID = 'my_module'
source_filename = "my_module"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 10, ptr %x, align 4
  store i32 3, ptr %y, align 4
  br label %loopcond

loopcond:                                         ; preds = %loopmerge4, %entry
  %x1 = load i32, ptr %x, align 4
  %gttmp = icmp sgt i32 %x1, 0
  br i1 %gttmp, label %loopbody, label %loopmerge

loopbody:                                         ; preds = %loopcond
  br label %loopcond2

loopmerge:                                        ; preds = %loopcond
  %x9 = load i32, ptr %x, align 4
  %y10 = load i32, ptr %y, align 4
  %addtmp11 = add i32 %x9, %y10
  ret i32 %addtmp11

loopcond2:                                        ; preds = %loopbody3, %loopbody
  %y5 = load i32, ptr %y, align 4
  %lttmp = icmp slt i32 %y5, 30
  br i1 %lttmp, label %loopbody3, label %loopmerge4

loopbody3:                                        ; preds = %loopcond2
  %y6 = load i32, ptr %y, align 4
  %addtmp = add i32 %y6, 1
  store i32 %addtmp, ptr %y, align 4
  %y7 = load i32, ptr %y, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %y7)
  br label %loopcond2

loopmerge4:                                       ; preds = %loopcond2
  %x8 = load i32, ptr %x, align 4
  %subtmp = sub i32 %x8, 1
  store i32 %subtmp, ptr %x, align 4
  br label %loopcond
}

define i32 @main.1() {
entry:
  ret i32 0
}

declare i32 @printf(ptr, ...)
