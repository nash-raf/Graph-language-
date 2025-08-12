; ModuleID = 'my_module'
source_filename = "my_module"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %j = alloca i32, align 4
  %arr = alloca [5 x i32], align 4
  %arr_elem = getelementptr [5 x i32], ptr %arr, i32 0, i32 0
  store i32 0, ptr %arr_elem, align 4
  %arr_elem1 = getelementptr [5 x i32], ptr %arr, i32 0, i32 1
  store i32 0, ptr %arr_elem1, align 4
  %arr_elem2 = getelementptr [5 x i32], ptr %arr, i32 0, i32 2
  store i32 0, ptr %arr_elem2, align 4
  %arr_elem3 = getelementptr [5 x i32], ptr %arr, i32 0, i32 3
  store i32 0, ptr %arr_elem3, align 4
  %arr_elem4 = getelementptr [5 x i32], ptr %arr, i32 0, i32 4
  store i32 0, ptr %arr_elem4, align 4
  %arr_elemptr = getelementptr [5 x i32], ptr %arr, i32 0, i32 3
  store i32 1, ptr %arr_elemptr, align 4
  %arr_elemptr5 = getelementptr [5 x i32], ptr %arr, i32 0, i32 2
  %arr_loadelem = load i32, ptr %arr_elemptr5, align 4
  store i32 %arr_loadelem, ptr %j, align 4
  %j6 = load i32, ptr %j, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt_int, i32 %j6)
  ret i32 0
}

declare i32 @printf(ptr, ...)
