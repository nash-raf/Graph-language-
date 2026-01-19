; ModuleID = 'my_module'
source_filename = "my_module"

@s1_blob = private constant [25 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\03\00\00\00\00\00\00\00\01\00\02\00\03\00"
@0 = private unnamed_addr constant [44 x i8] c"[DEBUG] Calling roaring_bitmap_add(%p, %d)\0A\00", align 1
@1 = private unnamed_addr constant [37 x i8] c"[DEBUG] After add, printing bitmap:\0A\00", align 1
@2 = private unnamed_addr constant [47 x i8] c"[DEBUG] Calling roaring_bitmap_remove(%p, %d)\0A\00", align 1
@3 = private unnamed_addr constant [40 x i8] c"[DEBUG] After remove, printing bitmap:\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %s1.bitmap = call ptr @roaring_from_serialized(ptr @s1_blob, i64 25)
  %0 = call i32 (ptr, ...) @printf(ptr @0, ptr %s1.bitmap, i32 400)
  call void @roaring_bitmap_add(ptr %s1.bitmap, i32 400)
  %1 = call i32 (ptr, ...) @printf(ptr @1)
  call void @roaring_print(ptr %s1.bitmap)
  call void @roaring_print(ptr %s1.bitmap)
  %2 = call i32 (ptr, ...) @printf(ptr @2, ptr %s1.bitmap, i32 2)
  call void @roaring_bitmap_remove(ptr %s1.bitmap, i32 2)
  %3 = call i32 (ptr, ...) @printf(ptr @3)
  call void @roaring_print(ptr %s1.bitmap)
  call void @roaring_print(ptr %s1.bitmap)
  %contains.result = call i32 @roaring_bitmap_contains(ptr %s1.bitmap, i32 1)
  %contains.bool = icmp ne i32 %contains.result, 0
  br i1 %contains.bool, label %then, label %ifcont

then:                                             ; preds = %entry
  %4 = call i32 (ptr, ...) @printf(ptr @4, i32 12)
  br label %ifcont

ifcont:                                           ; preds = %then, %entry
  ret i32 0
}

declare ptr @roaring_bitmap_create(i64, i64)

declare void @roaring_bitmap_add(ptr, i32)

declare ptr @roaring_bitmap_union(ptr, i64)

declare ptr @roaring_bitmap_intersect(ptr, ptr)

declare void @roaring_print(ptr)

declare void @roaring_bitmap_remove(ptr, i32)

declare i32 @roaring_bitmap_contains(ptr, i32)

declare i32 @roaring_bitmap_get_at_index(ptr, i32)

declare i64 @roaring_bitmap_get_cardinality(ptr)

declare ptr @roaring_from_serialized(ptr, i64)

declare i32 @printf(ptr, ...)
