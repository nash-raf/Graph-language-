; ModuleID = 'my_module'
source_filename = "my_module"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 10, ptr %x, align 4
  ret i32 0
}
Entering PrintExpr
ASTBuilder Visiting inner expr
ASTBuilder exitinng inner expr
