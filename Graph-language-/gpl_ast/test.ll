; ModuleID = 'my_module'
source_filename = "my_module"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 1000000, ptr %x, align 4
  store i32 3, ptr %y, align 4
  br label %loopcond

loopcond:                                         ; preds = %loopbody, %entry
  %x1 = load i32, ptr %x, align 4
  %gttmp = icmp sgt i32 %x1, 0
  br i1 %gttmp, label %loopbody, label %loopmerge

loopbody:                                         ; preds = %loopcond
  %x2 = load i32, ptr %x, align 4
  %subtmp = sub i32 %x2, 1
  store i32 %subtmp, ptr %x, align 4
  %x3 = load i32, ptr %x, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %x3)
  br label %loopcond

loopmerge:                                        ; preds = %loopcond
  %x4 = load i32, ptr %x, align 4
  %y5 = load i32, ptr %y, align 4
  %addtmp = add i32 %x4, %y5
  ret i32 %addtmp
}

define i32 @main.1() {
entry:
  ret i32 0
}

declare i32 @printf(ptr, ...)
