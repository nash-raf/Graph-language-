Captured Inputs:
  - 0x1caeeb50
  - 0x1caeefd0
  - 0x1caef5f8
Captured Inputs:
  -   %b.bitmap = call ptr @roaring_from_serialized(ptr @b_blob, i64 57)
  -   %c.bitmap = call ptr @roaring_from_serialized(ptr @c_blob, i64 59)
  -   %i.0 = phi i32 [ 10000, %entry ], [ %subtmp, %loopbody ]
Captured Outputs:
  -   %subtmp = sub i32 %i.0, 1
; ModuleID = 'my_module'
source_filename = "my_module"

%env.struct = type { ptr, ptr, ptr }

@s_blob = private constant [51 x i8] c"\03\00\00\00\00\00\00\00\00\00\00\03\00\00\00\00\00\00\00\03\00\04\00\05\00\01\00\00\01\00\00\00\00\00\00\00\A0\86\EB\0B\00\01\00\00\00\00\00\00\00\00\C2"
@b_blob = private constant [57 x i8] c"\03\00\00\00\00\00\00\00\00\00\00\06\00\00\00\00\00\00\00\03\00\04\00\05\00\06\00\07\00\08\00\01\00\00\01\00\00\00\00\00\00\00\A0\86\EB\0B\00\01\00\00\00\00\00\00\00\00\C2"
@c_blob = private constant [59 x i8] c"\03\00\00\00\00\00\00\00\00\00\00\07\00\00\00\00\00\00\00\03\00\04\00\06\00\07\00\08\00\09\00\0A\00\01\00\00\01\00\00\00\00\00\00\00\A0\86\EB\0B\00\01\00\00\00\00\00\00\00\00\C2"
@d_blob = private constant [70 x i8] c"\04\00\00\00\00\00\00\00\00\00\00\06\00\00\00\00\00\00\00\03\00\04\00\06\00\07\00\08\00\09\00\01\00\00\01\00\00\00\00\00\00\00\A0\86\EB\0B\00\01\00\00\00\00\00\00\00\00\C2\9A;\00\01\00\00\00\00\00\00\00\00\CA"
@e_blob = private constant [27 x i8] c"\01\00\00\00\00\00\00\00\00\00\00\04\00\00\00\00\00\00\00\14\00\15\00\16\00\17\00"

define i32 @main() {
entry:
  %subtmp.loc = alloca i32, align 4
  %s.bitmap = call ptr @roaring_from_serialized(ptr @s_blob, i64 51)
  %b.bitmap = call ptr @roaring_from_serialized(ptr @b_blob, i64 57)
  %c.bitmap = call ptr @roaring_from_serialized(ptr @c_blob, i64 59)
  %d.bitmap = call ptr @roaring_from_serialized(ptr @d_blob, i64 70)
  %e.bitmap = call ptr @roaring_from_serialized(ptr @e_blob, i64 27)
  %env_raw = call ptr @malloc(i64 24)
  %env_gep = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 0
  store ptr %b.bitmap, ptr %env_gep, align 8
  %env_gep3 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 1
  store ptr %c.bitmap, ptr %env_gep3, align 8
  %env_gep4 = getelementptr inbounds nuw %env.struct, ptr %env_raw, i32 0, i32 2
  store ptr %subtmp.loc, ptr %env_gep4, align 8
  call void @parallel_for_runtime(i64 10000, i64 0, i64 -1, ptr @wrapper, ptr %env_raw)
  ret i32 0
}

declare ptr @roaring_bitmap_create(i64, i64)

declare void @roaring_bitmap_add(ptr, i32)

declare ptr @roaring_bitmap_union(ptr, i64)

declare ptr @roaring_bitmap_intersect(ptr, ptr)

declare void @roaring_print(ptr)

declare ptr @roaring_from_serialized(ptr, i64)

define dso_local void @outlined_main_loopbody(ptr %b.bitmap, ptr %c.bitmap, i32 %i.0, ptr %subtmp.out) #0 {
newFuncRoot:
  %set.intersect.result = call ptr @roaring_bitmap_intersect(ptr %b.bitmap, ptr %c.bitmap)
  %subtmp = sub i32 %i.0, 1
  store i32 %subtmp, ptr %subtmp.out, align 4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare ptr @malloc(i64)

define void @wrapper(i64 %idx, ptr %env) {
entry:
  %fgep = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 0
  %fload = load ptr, ptr %fgep, align 8
  %fgep1 = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 1
  %fload2 = load ptr, ptr %fgep1, align 8
  %fgep3 = getelementptr inbounds nuw %env.struct, ptr %env, i32 0, i32 2
  %fload4 = load ptr, ptr %fgep3, align 8
  %idxcast = trunc i64 %idx to i32
  call void @outlined_main_loopbody(ptr %fload, ptr %fload2, i32 %idxcast, ptr %fload4)
  ret void
}

declare void @parallel_for_runtime(i64, i64, i64, ptr, ptr)

attributes #0 = { "outlined-loop" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
