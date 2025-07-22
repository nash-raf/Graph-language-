; ModuleID = 'my_module'
source_filename = "my_module"

define i32 @main() {
entry:
  %c = alloca i32, align 4
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 5, ptr %x, align 4
  %x1 = load i32, ptr %x, align 4
  %multmp = mul i32 5, %x1
  store i32 %multmp, ptr %y, align 4
  store i32 0, ptr %c, align 4
  %x2 = load i32, ptr %x, align 4
  %lttmp = icmp slt i32 %x2, 5
  br i1 %lttmp, label %then, label %ifcont

then:                                             ; preds = %entry
  %x3 = load i32, ptr %x, align 4
  %subtmp = sub i32 %x3, 1
  store i32 %subtmp, ptr %c, align 4
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  br label %loopcond

loopcond:                                         ; preds = %loopbody, %ifcont
  %c4 = load i32, ptr %c, align 4
  %lttmp5 = icmp slt i32 %c4, 4
  br i1 %lttmp5, label %loopbody, label %loopmerge

loopbody:                                         ; preds = %loopcond
  %y6 = load i32, ptr %y, align 4
  %subtmp7 = sub i32 %y6, 1
  store i32 %subtmp7, ptr %y, align 4
  %c8 = load i32, ptr %c, align 4
  %addtmp = add i32 %c8, 1
  store i32 %addtmp, ptr %c, align 4
  br label %loopcond

loopmerge:                                        ; preds = %loopcond
  ret i32 0
}
