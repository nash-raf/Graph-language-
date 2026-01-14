; ModuleID = 'my_module'
source_filename = "my_module"

@h_bitmap = global ptr null
@h_blob = private constant [25 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\03\00\00\00\00\00\00\00\01\00\02\00\04\00"
@v_bitmap = global ptr null
@v_blob = private constant [25 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\03\00\00\00\00\00\00\00\01\00\02\00\03\00"
@s_bitmap = global ptr null

define i32 @main() {
entry:
  %h.bitmap = call ptr @roaring_from_serialized(ptr @h_blob, i64 25)
  store ptr %h.bitmap, ptr @h_bitmap, align 8
  %v.bitmap = call ptr @roaring_from_serialized(ptr @v_blob, i64 25)
  store ptr %v.bitmap, ptr @v_bitmap, align 8
  %set.intersect.result = call ptr @roaring_bitmap_intersect(ptr %h.bitmap, ptr %v.bitmap)
  store ptr %set.intersect.result, ptr @s_bitmap, align 8
  call void @roaring_print(ptr %set.intersect.result)
  ret i32 0
}

declare ptr @roaring_bitmap_create(i64, i64)

declare void @roaring_bitmap_add(ptr, i32)

declare ptr @roaring_bitmap_union(ptr, ptr)

declare ptr @roaring_bitmap_intersect(ptr, ptr)

declare void @roaring_print(ptr)

declare ptr @roaring_from_serialized(ptr, i64)
