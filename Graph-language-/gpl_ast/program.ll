; ModuleID = 'my_module'
source_filename = "my_module"

define i32 @lol() {
entry:
  ret i32 5
}

define i32 @main() {
entry:
  %j = alloca i32, align 4
  %calltmp = call i32 @lol()
  store i32 %calltmp, ptr %j, align 4
  ret i32 0
}
