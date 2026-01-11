; ModuleID = 'my_module'
source_filename = "my_module"

@h_bitmap = global ptr null
@h_blob = private constant [25 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\03\00\00\00\00\00\00\00\01\00\02\00\04\00"

define i32 @main() {
entry:
  %h.bitmap = call ptr @roaring_from_serialized(ptr @h_blob, i64 25)
  store ptr %h.bitmap, ptr @h_bitmap, align 8
  ret i32 0
}

declare ptr @roaring_from_serialized(ptr, i64)
