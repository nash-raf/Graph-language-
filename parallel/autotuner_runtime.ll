; ModuleID = 'autotuner_runtime.c'
source_filename = "autotuner_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.AutoGraphMeta = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, i64, i64, ptr, ptr, i64, ptr, ptr, i32, i32 }
%struct.EdgePair = type { i64, i64 }

@g_meta_count = internal unnamed_addr global i32 0, align 4
@g_meta = internal global [64 x %struct.AutoGraphMeta] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @build_csr_from_set(i64 noundef %0, ptr noundef %1, ptr nocapture noundef readonly %2, ptr nocapture noundef writeonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5) local_unnamed_addr #0 {
  %7 = add i64 %0, 1
  %8 = tail call noalias ptr @calloc(i64 noundef %7, i64 noundef 8) #13
  %9 = mul nsw i64 %0, 10
  %10 = tail call i64 @llvm.smax.i64(i64 %9, i64 1000)
  %11 = getelementptr i8, ptr %8, i64 8
  %12 = and i64 %10, 4294967294
  %13 = icmp eq i64 %12, 0
  br i1 %13, label %16, label %14

14:                                               ; preds = %6
  %15 = and i64 %10, 4294967294
  br label %28

16:                                               ; preds = %44, %6
  %17 = phi i64 [ 0, %6 ], [ %45, %44 ]
  %18 = icmp slt i64 %0, 1
  br i1 %18, label %62, label %19

19:                                               ; preds = %16
  %20 = load i64, ptr %8, align 8
  %21 = and i64 %0, 3
  %22 = icmp ult i64 %0, 4
  br i1 %22, label %48, label %23

23:                                               ; preds = %19
  %24 = and i64 %0, 9223372036854775804
  %25 = getelementptr i8, ptr %8, i64 8
  %26 = getelementptr i8, ptr %8, i64 16
  %27 = getelementptr i8, ptr %8, i64 24
  br label %69

28:                                               ; preds = %14, %44
  %29 = phi i64 [ 0, %14 ], [ %46, %44 ]
  %30 = phi i64 [ 0, %14 ], [ %45, %44 ]
  %31 = trunc nuw i64 %29 to i32
  %32 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %1, i32 noundef %31) #14
  br i1 %32, label %33, label %44

33:                                               ; preds = %28
  %34 = getelementptr inbounds nuw %struct.EdgePair, ptr %2, i64 %29
  %35 = load i64, ptr %34, align 8, !tbaa !3
  %36 = icmp sgt i64 %35, -1
  %37 = icmp slt i64 %35, %0
  %38 = and i1 %36, %37
  br i1 %38, label %39, label %44

39:                                               ; preds = %33
  %40 = getelementptr i64, ptr %11, i64 %35
  %41 = load i64, ptr %40, align 8, !tbaa !8
  %42 = add nsw i64 %41, 1
  store i64 %42, ptr %40, align 8, !tbaa !8
  %43 = add nsw i64 %30, 1
  br label %44

44:                                               ; preds = %33, %39, %28
  %45 = phi i64 [ %30, %28 ], [ %43, %39 ], [ %30, %33 ]
  %46 = add nuw nsw i64 %29, 1
  %47 = icmp eq i64 %46, %15
  br i1 %47, label %16, label %28, !llvm.loop !9

48:                                               ; preds = %69, %19
  %49 = phi i64 [ %20, %19 ], [ %84, %69 ]
  %50 = phi i64 [ 1, %19 ], [ %85, %69 ]
  %51 = icmp eq i64 %21, 0
  br i1 %51, label %62, label %52

52:                                               ; preds = %48, %52
  %53 = phi i64 [ %58, %52 ], [ %49, %48 ]
  %54 = phi i64 [ %59, %52 ], [ %50, %48 ]
  %55 = phi i64 [ %60, %52 ], [ 0, %48 ]
  %56 = getelementptr i64, ptr %8, i64 %54
  %57 = load i64, ptr %56, align 8, !tbaa !8
  %58 = add nsw i64 %57, %53
  store i64 %58, ptr %56, align 8, !tbaa !8
  %59 = add nuw i64 %54, 1
  %60 = add i64 %55, 1
  %61 = icmp eq i64 %60, %21
  br i1 %61, label %62, label %52, !llvm.loop !11

62:                                               ; preds = %48, %52, %16
  %63 = shl i64 %17, 2
  %64 = tail call noalias ptr @malloc(i64 noundef %63) #15
  %65 = shl i64 %7, 3
  %66 = tail call noalias ptr @malloc(i64 noundef %65) #15
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %66, ptr align 8 %8, i64 %65, i1 false)
  br i1 %13, label %88, label %67

67:                                               ; preds = %62
  %68 = and i64 %10, 4294967294
  br label %89

69:                                               ; preds = %69, %23
  %70 = phi i64 [ %20, %23 ], [ %84, %69 ]
  %71 = phi i64 [ 1, %23 ], [ %85, %69 ]
  %72 = phi i64 [ 0, %23 ], [ %86, %69 ]
  %73 = getelementptr i64, ptr %8, i64 %71
  %74 = load i64, ptr %73, align 8, !tbaa !8
  %75 = add nsw i64 %74, %70
  store i64 %75, ptr %73, align 8, !tbaa !8
  %76 = getelementptr i64, ptr %25, i64 %71
  %77 = load i64, ptr %76, align 8, !tbaa !8
  %78 = add nsw i64 %77, %75
  store i64 %78, ptr %76, align 8, !tbaa !8
  %79 = getelementptr i64, ptr %26, i64 %71
  %80 = load i64, ptr %79, align 8, !tbaa !8
  %81 = add nsw i64 %80, %78
  store i64 %81, ptr %79, align 8, !tbaa !8
  %82 = getelementptr i64, ptr %27, i64 %71
  %83 = load i64, ptr %82, align 8, !tbaa !8
  %84 = add nsw i64 %83, %81
  store i64 %84, ptr %82, align 8, !tbaa !8
  %85 = add nuw i64 %71, 4
  %86 = add i64 %72, 4
  %87 = icmp eq i64 %86, %24
  br i1 %87, label %48, label %69, !llvm.loop !13

88:                                               ; preds = %111, %62
  tail call void @free(ptr noundef %66) #14
  store ptr %8, ptr %3, align 8, !tbaa !14
  store ptr %64, ptr %4, align 8, !tbaa !17
  store i64 %17, ptr %5, align 8, !tbaa !8
  ret void

89:                                               ; preds = %67, %111
  %90 = phi i64 [ 0, %67 ], [ %112, %111 ]
  %91 = trunc nuw i64 %90 to i32
  %92 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %1, i32 noundef %91) #14
  br i1 %92, label %93, label %111

93:                                               ; preds = %89
  %94 = getelementptr inbounds nuw %struct.EdgePair, ptr %2, i64 %90
  %95 = load i64, ptr %94, align 8, !tbaa !3
  %96 = getelementptr inbounds nuw i8, ptr %94, i64 8
  %97 = load i64, ptr %96, align 8, !tbaa !19
  %98 = icmp sgt i64 %95, -1
  br i1 %98, label %99, label %111

99:                                               ; preds = %93
  %100 = icmp slt i64 %95, %0
  %101 = icmp sgt i64 %97, -1
  %102 = icmp slt i64 %97, %0
  %103 = and i1 %101, %102
  %104 = select i1 %100, i1 %103, i1 false
  br i1 %104, label %105, label %111

105:                                              ; preds = %99
  %106 = trunc i64 %97 to i32
  %107 = getelementptr inbounds nuw i64, ptr %66, i64 %95
  %108 = load i64, ptr %107, align 8, !tbaa !8
  %109 = add nsw i64 %108, 1
  store i64 %109, ptr %107, align 8, !tbaa !8
  %110 = getelementptr inbounds i32, ptr %64, i64 %108
  store i32 %106, ptr %110, align 4, !tbaa !20
  br label %111

111:                                              ; preds = %93, %99, %105, %89
  %112 = add nuw nsw i64 %90, 1
  %113 = icmp eq i64 %112, %68
  br i1 %113, label %88, label %89, !llvm.loop !22
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #2

declare dso_local zeroext i1 @roaring_bitmap_contains(ptr noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local void @free(ptr allocptr nocapture noundef) local_unnamed_addr #6

; Function Attrs: nounwind uwtable
define dso_local void @rebuild_sets_from_csr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr noundef %4, ptr noundef %5, ptr nocapture noundef readonly %6) local_unnamed_addr #0 {
  tail call void @roaring_bitmap_clear(ptr noundef %4) #14
  tail call void @roaring_bitmap_clear(ptr noundef %5) #14
  %8 = icmp slt i64 %0, 1
  br i1 %8, label %14, label %9

9:                                                ; preds = %7
  %10 = trunc i64 %0 to i32
  %11 = mul i32 %10, 10
  %12 = icmp eq i32 %11, 0
  %13 = zext i32 %11 to i64
  br label %15

14:                                               ; preds = %24, %7
  ret void

15:                                               ; preds = %9, %24
  %16 = phi i64 [ 1, %9 ], [ %25, %24 ]
  %17 = getelementptr i64, ptr %2, i64 %16
  %18 = getelementptr i8, ptr %17, i64 -8
  %19 = load i64, ptr %18, align 8, !tbaa !8
  %20 = load i64, ptr %17, align 8, !tbaa !8
  %21 = icmp sgt i64 %20, %19
  br i1 %21, label %22, label %24

22:                                               ; preds = %15
  %23 = trunc i64 %16 to i32
  tail call void @roaring_bitmap_add(ptr noundef %4, i32 noundef %23) #14
  br label %27

24:                                               ; preds = %57, %15
  %25 = add nuw i64 %16, 1
  %26 = icmp eq i64 %16, %0
  br i1 %26, label %14, label %15, !llvm.loop !23

27:                                               ; preds = %22, %57
  %28 = phi i64 [ %58, %57 ], [ %19, %22 ]
  %29 = getelementptr inbounds i32, ptr %3, i64 %28
  %30 = load i32, ptr %29, align 4, !tbaa !20
  tail call void @roaring_bitmap_add(ptr noundef %4, i32 noundef %30) #14
  br i1 %12, label %57, label %31

31:                                               ; preds = %27
  %32 = sext i32 %30 to i64
  br label %33

33:                                               ; preds = %54, %31
  %34 = phi i64 [ 0, %31 ], [ %55, %54 ]
  %35 = getelementptr inbounds nuw %struct.EdgePair, ptr %6, i64 %34
  %36 = load i64, ptr %35, align 8, !tbaa !3
  %37 = icmp eq i64 %36, %16
  br i1 %37, label %38, label %45

38:                                               ; preds = %33
  %39 = getelementptr inbounds nuw i8, ptr %35, i64 8
  %40 = load i64, ptr %39, align 8, !tbaa !19
  %41 = icmp eq i64 %40, %32
  br i1 %41, label %42, label %45

42:                                               ; preds = %50, %38
  %43 = phi i64 [ %34, %38 ], [ %46, %50 ]
  %44 = trunc nuw i64 %43 to i32
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %44) #14
  br label %57

45:                                               ; preds = %33, %38
  %46 = or disjoint i64 %34, 1
  %47 = getelementptr inbounds nuw %struct.EdgePair, ptr %6, i64 %46
  %48 = load i64, ptr %47, align 8, !tbaa !3
  %49 = icmp eq i64 %48, %16
  br i1 %49, label %50, label %54

50:                                               ; preds = %45
  %51 = getelementptr inbounds nuw i8, ptr %47, i64 8
  %52 = load i64, ptr %51, align 8, !tbaa !19
  %53 = icmp eq i64 %52, %32
  br i1 %53, label %42, label %54

54:                                               ; preds = %50, %45
  %55 = add nuw nsw i64 %34, 2
  %56 = icmp eq i64 %55, %13
  br i1 %56, label %57, label %33, !llvm.loop !24

57:                                               ; preds = %54, %27, %42
  %58 = add i64 %28, 1
  %59 = icmp eq i64 %58, %20
  br i1 %59, label %24, label %27, !llvm.loop !25
}

declare dso_local void @roaring_bitmap_clear(ptr noundef) local_unnamed_addr #3

declare dso_local void @roaring_bitmap_add(ptr noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_pcsr(i64 noundef %0, i64 noundef %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = tail call i64 @llvm.smax.i64(i64 %1, i64 16)
  %9 = shl nuw i64 %8, 1
  %10 = shl i64 %8, 3
  %11 = tail call noalias ptr @malloc(i64 noundef %10) #15
  %12 = shl i64 %0, 3
  %13 = add i64 %12, 8
  %14 = tail call noalias ptr @malloc(i64 noundef %13) #15
  tail call void @llvm.memset.p0.i64(ptr align 4 %11, i8 -1, i64 %10, i1 false)
  %15 = icmp sgt i64 %0, 0
  br i1 %15, label %16, label %20

16:                                               ; preds = %7
  %17 = ptrtoint ptr %3 to i64
  br label %25

18:                                               ; preds = %83, %79, %49
  %19 = icmp eq i64 %30, %0
  br i1 %19, label %20, label %25, !llvm.loop !26

20:                                               ; preds = %18, %7
  %21 = phi i64 [ 0, %7 ], [ %38, %18 ]
  %22 = phi ptr [ %11, %7 ], [ %50, %18 ]
  %23 = phi i64 [ %9, %7 ], [ %51, %18 ]
  %24 = getelementptr inbounds i64, ptr %14, i64 %0
  store i64 %21, ptr %24, align 8, !tbaa !8
  store ptr %14, ptr %4, align 8, !tbaa !14
  store ptr %22, ptr %5, align 8, !tbaa !17
  store i64 %23, ptr %6, align 8, !tbaa !8
  ret void

25:                                               ; preds = %16, %18
  %26 = phi i64 [ %51, %18 ], [ %9, %16 ]
  %27 = phi ptr [ %50, %18 ], [ %11, %16 ]
  %28 = phi i64 [ %38, %18 ], [ 0, %16 ]
  %29 = phi i64 [ %30, %18 ], [ 0, %16 ]
  %30 = add nuw nsw i64 %29, 1
  %31 = getelementptr inbounds nuw i64, ptr %2, i64 %30
  %32 = load i64, ptr %31, align 8, !tbaa !8
  %33 = getelementptr inbounds nuw i64, ptr %2, i64 %29
  %34 = load i64, ptr %33, align 8, !tbaa !8
  %35 = sub nsw i64 %32, %34
  %36 = tail call i64 @llvm.smax.i64(i64 %35, i64 2)
  %37 = add i64 %35, %28
  %38 = add i64 %37, %36
  %39 = icmp sgt i64 %38, %26
  br i1 %39, label %40, label %49

40:                                               ; preds = %25
  %41 = shl nsw i64 %38, 1
  %42 = shl i64 %38, 3
  %43 = tail call ptr @realloc(ptr noundef %27, i64 noundef %42) #16
  %44 = icmp sgt i64 %41, %26
  br i1 %44, label %45, label %49

45:                                               ; preds = %40
  %46 = getelementptr inbounds i32, ptr %43, i64 %26
  %47 = sub nsw i64 %41, %26
  %48 = shl i64 %47, 2
  tail call void @llvm.memset.p0.i64(ptr align 4 %46, i8 -1, i64 %48, i1 false)
  br label %49

49:                                               ; preds = %40, %45, %25
  %50 = phi ptr [ %27, %25 ], [ %43, %45 ], [ %43, %40 ]
  %51 = phi i64 [ %26, %25 ], [ %41, %45 ], [ %41, %40 ]
  %52 = ptrtoint ptr %50 to i64
  %53 = getelementptr inbounds nuw i64, ptr %14, i64 %29
  store i64 %28, ptr %53, align 8, !tbaa !8
  %54 = icmp sgt i64 %35, 0
  br i1 %54, label %55, label %18

55:                                               ; preds = %49
  %56 = load i64, ptr %33, align 8, !tbaa !8
  %57 = getelementptr i32, ptr %3, i64 %56
  %58 = getelementptr i32, ptr %50, i64 %28
  %59 = icmp ult i64 %35, 8
  br i1 %59, label %81, label %60

60:                                               ; preds = %55
  %61 = shl i64 %28, 2
  %62 = shl i64 %56, 2
  %63 = add i64 %61, %52
  %64 = add i64 %62, %17
  %65 = sub i64 %63, %64
  %66 = icmp ult i64 %65, 32
  br i1 %66, label %81, label %67

67:                                               ; preds = %60
  %68 = and i64 %35, 9223372036854775800
  br label %69

69:                                               ; preds = %69, %67
  %70 = phi i64 [ 0, %67 ], [ %77, %69 ]
  %71 = getelementptr i32, ptr %57, i64 %70
  %72 = getelementptr i8, ptr %71, i64 16
  %73 = load <4 x i32>, ptr %71, align 4, !tbaa !20
  %74 = load <4 x i32>, ptr %72, align 4, !tbaa !20
  %75 = getelementptr i32, ptr %58, i64 %70
  %76 = getelementptr i8, ptr %75, i64 16
  store <4 x i32> %73, ptr %75, align 4, !tbaa !20
  store <4 x i32> %74, ptr %76, align 4, !tbaa !20
  %77 = add nuw i64 %70, 8
  %78 = icmp eq i64 %77, %68
  br i1 %78, label %79, label %69, !llvm.loop !27

79:                                               ; preds = %69
  %80 = icmp eq i64 %35, %68
  br i1 %80, label %18, label %81

81:                                               ; preds = %60, %55, %79
  %82 = phi i64 [ 0, %55 ], [ 0, %60 ], [ %68, %79 ]
  br label %83

83:                                               ; preds = %81, %83
  %84 = phi i64 [ %88, %83 ], [ %82, %81 ]
  %85 = getelementptr i32, ptr %57, i64 %84
  %86 = load i32, ptr %85, align 4, !tbaa !20
  %87 = getelementptr i32, ptr %58, i64 %84
  store i32 %86, ptr %87, align 4, !tbaa !20
  %88 = add nuw nsw i64 %84, 1
  %89 = icmp eq i64 %88, %35
  br i1 %89, label %18, label %83, !llvm.loop !30
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #8

; Function Attrs: nounwind uwtable
define dso_local void @convert_pcsr_to_csr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = add i64 %0, 1
  %9 = tail call noalias ptr @calloc(i64 noundef %8, i64 noundef 8) #13
  %10 = icmp sgt i64 %0, 0
  br i1 %10, label %11, label %89

11:                                               ; preds = %7
  %12 = load i64, ptr %2, align 8, !tbaa !8
  br label %22

13:                                               ; preds = %60
  %14 = load i64, ptr %9, align 8
  %15 = and i64 %0, 3
  %16 = icmp ult i64 %0, 4
  br i1 %16, label %75, label %17

17:                                               ; preds = %13
  %18 = and i64 %0, 9223372036854775804
  %19 = getelementptr i8, ptr %9, i64 8
  %20 = getelementptr i8, ptr %9, i64 16
  %21 = getelementptr i8, ptr %9, i64 24
  br label %98

22:                                               ; preds = %11, %60
  %23 = phi i64 [ %28, %60 ], [ %12, %11 ]
  %24 = phi i64 [ %63, %60 ], [ 0, %11 ]
  %25 = phi i64 [ %26, %60 ], [ 0, %11 ]
  %26 = add nuw nsw i64 %25, 1
  %27 = getelementptr inbounds nuw i64, ptr %2, i64 %26
  %28 = load i64, ptr %27, align 8, !tbaa !8
  %29 = icmp slt i64 %23, %28
  br i1 %29, label %30, label %60

30:                                               ; preds = %22
  %31 = sub i64 %28, %23
  %32 = icmp ult i64 %31, 4
  br i1 %32, label %57, label %33

33:                                               ; preds = %30
  %34 = and i64 %31, -4
  %35 = add i64 %23, %34
  %36 = getelementptr i32, ptr %3, i64 %23
  br label %37

37:                                               ; preds = %37, %33
  %38 = phi i64 [ 0, %33 ], [ %51, %37 ]
  %39 = phi <2 x i64> [ zeroinitializer, %33 ], [ %49, %37 ]
  %40 = phi <2 x i64> [ zeroinitializer, %33 ], [ %50, %37 ]
  %41 = getelementptr i32, ptr %36, i64 %38
  %42 = getelementptr inbounds nuw i8, ptr %41, i64 8
  %43 = load <2 x i32>, ptr %41, align 4, !tbaa !20
  %44 = load <2 x i32>, ptr %42, align 4, !tbaa !20
  %45 = icmp ne <2 x i32> %43, splat (i32 -1)
  %46 = icmp ne <2 x i32> %44, splat (i32 -1)
  %47 = zext <2 x i1> %45 to <2 x i64>
  %48 = zext <2 x i1> %46 to <2 x i64>
  %49 = add <2 x i64> %39, %47
  %50 = add <2 x i64> %40, %48
  %51 = add nuw i64 %38, 4
  %52 = icmp eq i64 %51, %34
  br i1 %52, label %53, label %37, !llvm.loop !31

53:                                               ; preds = %37
  %54 = add <2 x i64> %50, %49
  %55 = tail call i64 @llvm.vector.reduce.add.v2i64(<2 x i64> %54)
  %56 = icmp eq i64 %31, %34
  br i1 %56, label %60, label %57

57:                                               ; preds = %30, %53
  %58 = phi i64 [ 0, %30 ], [ %55, %53 ]
  %59 = phi i64 [ %23, %30 ], [ %35, %53 ]
  br label %65

60:                                               ; preds = %65, %53, %22
  %61 = phi i64 [ 0, %22 ], [ %55, %53 ], [ %72, %65 ]
  %62 = getelementptr inbounds nuw i64, ptr %9, i64 %26
  store i64 %61, ptr %62, align 8, !tbaa !8
  %63 = add nuw nsw i64 %61, %24
  %64 = icmp eq i64 %26, %0
  br i1 %64, label %13, label %22, !llvm.loop !32

65:                                               ; preds = %57, %65
  %66 = phi i64 [ %72, %65 ], [ %58, %57 ]
  %67 = phi i64 [ %73, %65 ], [ %59, %57 ]
  %68 = getelementptr inbounds i32, ptr %3, i64 %67
  %69 = load i32, ptr %68, align 4, !tbaa !20
  %70 = icmp ne i32 %69, -1
  %71 = zext i1 %70 to i64
  %72 = add nuw nsw i64 %66, %71
  %73 = add nsw i64 %67, 1
  %74 = icmp eq i64 %73, %28
  br i1 %74, label %60, label %65, !llvm.loop !33

75:                                               ; preds = %98, %13
  %76 = phi i64 [ %14, %13 ], [ %113, %98 ]
  %77 = phi i64 [ 1, %13 ], [ %114, %98 ]
  %78 = icmp eq i64 %15, 0
  br i1 %78, label %89, label %79

79:                                               ; preds = %75, %79
  %80 = phi i64 [ %85, %79 ], [ %76, %75 ]
  %81 = phi i64 [ %86, %79 ], [ %77, %75 ]
  %82 = phi i64 [ %87, %79 ], [ 0, %75 ]
  %83 = getelementptr i64, ptr %9, i64 %81
  %84 = load i64, ptr %83, align 8, !tbaa !8
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !8
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %15
  br i1 %88, label %89, label %79, !llvm.loop !34

89:                                               ; preds = %75, %79, %7
  %90 = phi i64 [ 0, %7 ], [ %63, %79 ], [ %63, %75 ]
  %91 = shl i64 %90, 2
  %92 = tail call noalias ptr @malloc(i64 noundef %91) #15
  %93 = shl i64 %8, 3
  %94 = tail call noalias ptr @malloc(i64 noundef %93) #15
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %94, ptr align 8 %9, i64 %93, i1 false)
  br i1 %10, label %95, label %119

95:                                               ; preds = %89
  %96 = load i64, ptr %2, align 8, !tbaa !8
  %97 = getelementptr i8, ptr %3, i64 4
  br label %120

98:                                               ; preds = %98, %17
  %99 = phi i64 [ %14, %17 ], [ %113, %98 ]
  %100 = phi i64 [ 1, %17 ], [ %114, %98 ]
  %101 = phi i64 [ 0, %17 ], [ %115, %98 ]
  %102 = getelementptr i64, ptr %9, i64 %100
  %103 = load i64, ptr %102, align 8, !tbaa !8
  %104 = add nsw i64 %103, %99
  store i64 %104, ptr %102, align 8, !tbaa !8
  %105 = getelementptr i64, ptr %19, i64 %100
  %106 = load i64, ptr %105, align 8, !tbaa !8
  %107 = add nsw i64 %106, %104
  store i64 %107, ptr %105, align 8, !tbaa !8
  %108 = getelementptr i64, ptr %20, i64 %100
  %109 = load i64, ptr %108, align 8, !tbaa !8
  %110 = add nsw i64 %109, %107
  store i64 %110, ptr %108, align 8, !tbaa !8
  %111 = getelementptr i64, ptr %21, i64 %100
  %112 = load i64, ptr %111, align 8, !tbaa !8
  %113 = add nsw i64 %112, %110
  store i64 %113, ptr %111, align 8, !tbaa !8
  %114 = add nuw i64 %100, 4
  %115 = add i64 %101, 4
  %116 = icmp eq i64 %115, %18
  br i1 %116, label %75, label %98, !llvm.loop !35

117:                                              ; preds = %143, %163, %120
  %118 = icmp eq i64 %123, %0
  br i1 %118, label %119, label %120, !llvm.loop !36

119:                                              ; preds = %117, %89
  tail call void @free(ptr noundef %94) #14
  store ptr %9, ptr %4, align 8, !tbaa !14
  store ptr %92, ptr %5, align 8, !tbaa !17
  store i64 %90, ptr %6, align 8, !tbaa !8
  ret void

120:                                              ; preds = %95, %117
  %121 = phi i64 [ %125, %117 ], [ %96, %95 ]
  %122 = phi i64 [ %123, %117 ], [ 0, %95 ]
  %123 = add nuw nsw i64 %122, 1
  %124 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %125 = load i64, ptr %124, align 8, !tbaa !8
  %126 = icmp slt i64 %121, %125
  br i1 %126, label %127, label %117

127:                                              ; preds = %120
  %128 = getelementptr inbounds nuw i64, ptr %94, i64 %122
  %129 = sub i64 %125, %121
  %130 = add i64 %121, 1
  %131 = and i64 %129, 1
  %132 = icmp eq i64 %131, 0
  br i1 %132, label %143, label %133

133:                                              ; preds = %127
  %134 = getelementptr inbounds i32, ptr %3, i64 %121
  %135 = load i32, ptr %134, align 4, !tbaa !20
  %136 = icmp eq i32 %135, -1
  br i1 %136, label %141, label %137

137:                                              ; preds = %133
  %138 = load i64, ptr %128, align 8, !tbaa !8
  %139 = add nsw i64 %138, 1
  store i64 %139, ptr %128, align 8, !tbaa !8
  %140 = getelementptr inbounds i32, ptr %92, i64 %138
  store i32 %135, ptr %140, align 4, !tbaa !20
  br label %141

141:                                              ; preds = %137, %133
  %142 = add nsw i64 %121, 1
  br label %143

143:                                              ; preds = %141, %127
  %144 = phi i64 [ %121, %127 ], [ %142, %141 ]
  %145 = icmp eq i64 %125, %130
  br i1 %145, label %117, label %146

146:                                              ; preds = %143, %163
  %147 = phi i64 [ %164, %163 ], [ %144, %143 ]
  %148 = getelementptr inbounds i32, ptr %3, i64 %147
  %149 = load i32, ptr %148, align 4, !tbaa !20
  %150 = icmp eq i32 %149, -1
  br i1 %150, label %155, label %151

151:                                              ; preds = %146
  %152 = load i64, ptr %128, align 8, !tbaa !8
  %153 = add nsw i64 %152, 1
  store i64 %153, ptr %128, align 8, !tbaa !8
  %154 = getelementptr inbounds i32, ptr %92, i64 %152
  store i32 %149, ptr %154, align 4, !tbaa !20
  br label %155

155:                                              ; preds = %146, %151
  %156 = getelementptr i32, ptr %97, i64 %147
  %157 = load i32, ptr %156, align 4, !tbaa !20
  %158 = icmp eq i32 %157, -1
  br i1 %158, label %163, label %159

159:                                              ; preds = %155
  %160 = load i64, ptr %128, align 8, !tbaa !8
  %161 = add nsw i64 %160, 1
  store i64 %161, ptr %128, align 8, !tbaa !8
  %162 = getelementptr inbounds i32, ptr %92, i64 %160
  store i32 %157, ptr %162, align 4, !tbaa !20
  br label %163

163:                                              ; preds = %159, %155
  %164 = add nsw i64 %147, 2
  %165 = icmp eq i64 %164, %125
  br i1 %165, label %117, label %146, !llvm.loop !37
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_bcsr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr noundef writeonly %4, ptr noundef writeonly %5, i32 noundef %6, ptr noundef writeonly %7) local_unnamed_addr #0 {
  %9 = icmp eq ptr %4, null
  %10 = icmp eq ptr %5, null
  %11 = or i1 %9, %10
  %12 = icmp eq ptr %7, null
  %13 = or i1 %11, %12
  %14 = icmp slt i32 %6, 1
  %15 = or i1 %14, %13
  br i1 %15, label %191, label %16

16:                                               ; preds = %8
  %17 = zext nneg i32 %6 to i64
  %18 = add i64 %0, -1
  %19 = add i64 %18, %17
  %20 = sdiv i64 %19, %17
  %21 = trunc i64 %20 to i32
  %22 = shl i64 %20, 32
  %23 = add i64 %22, 4294967296
  %24 = ashr exact i64 %23, 32
  %25 = tail call noalias ptr @calloc(i64 noundef %24, i64 noundef 4) #13
  %26 = icmp eq ptr %25, null
  br i1 %26, label %191, label %27

27:                                               ; preds = %16
  %28 = getelementptr inbounds nuw i8, ptr %25, i64 4
  %29 = icmp sgt i32 %21, 0
  br i1 %29, label %30, label %119

30:                                               ; preds = %27
  %31 = and i64 %20, 2147483647
  br label %32

32:                                               ; preds = %30, %102
  %33 = phi i64 [ 0, %30 ], [ %105, %102 ]
  %34 = phi i64 [ 0, %30 ], [ %89, %102 ]
  %35 = mul i64 %33, %17
  %36 = add i64 %35, %17
  %37 = tail call i64 @llvm.smin.i64(i64 %0, i64 %36)
  %38 = add i64 %35, 1
  %39 = tail call i64 @llvm.smax.i64(i64 %37, i64 %38)
  %40 = mul i64 %33, %17
  %41 = sub i64 %39, %40
  %42 = mul nuw nsw i64 %33, %17
  %43 = add nuw nsw i64 %42, %17
  %44 = tail call i64 @llvm.smin.i64(i64 %43, i64 %0)
  %45 = icmp sgt i64 %0, %42
  br i1 %45, label %46, label %87

46:                                               ; preds = %32
  %47 = getelementptr inbounds nuw i64, ptr %2, i64 %42
  %48 = load i64, ptr %47, align 8, !tbaa !8
  %49 = icmp ult i64 %41, 4
  br i1 %49, label %80, label %50

50:                                               ; preds = %46
  %51 = and i64 %41, -4
  %52 = add i64 %42, %51
  %53 = insertelement <2 x i64> poison, i64 %48, i64 1
  %54 = getelementptr i64, ptr %2, i64 %42
  br label %55

55:                                               ; preds = %55, %50
  %56 = phi i64 [ 0, %50 ], [ %73, %55 ]
  %57 = phi <2 x i64> [ %53, %50 ], [ %64, %55 ]
  %58 = phi <2 x i64> [ zeroinitializer, %50 ], [ %71, %55 ]
  %59 = phi <2 x i64> [ zeroinitializer, %50 ], [ %72, %55 ]
  %60 = getelementptr i64, ptr %54, i64 %56
  %61 = getelementptr i8, ptr %60, i64 8
  %62 = getelementptr i8, ptr %60, i64 24
  %63 = load <2 x i64>, ptr %61, align 8, !tbaa !8
  %64 = load <2 x i64>, ptr %62, align 8, !tbaa !8
  %65 = shufflevector <2 x i64> %57, <2 x i64> %63, <2 x i32> <i32 1, i32 2>
  %66 = shufflevector <2 x i64> %63, <2 x i64> %64, <2 x i32> <i32 1, i32 2>
  %67 = sub nsw <2 x i64> %63, %65
  %68 = sub nsw <2 x i64> %64, %66
  %69 = tail call <2 x i64> @llvm.smax.v2i64(<2 x i64> %67, <2 x i64> zeroinitializer)
  %70 = tail call <2 x i64> @llvm.smax.v2i64(<2 x i64> %68, <2 x i64> zeroinitializer)
  %71 = add <2 x i64> %69, %58
  %72 = add <2 x i64> %70, %59
  %73 = add nuw i64 %56, 4
  %74 = icmp eq i64 %73, %51
  br i1 %74, label %75, label %55, !llvm.loop !38

75:                                               ; preds = %55
  %76 = add <2 x i64> %72, %71
  %77 = tail call i64 @llvm.vector.reduce.add.v2i64(<2 x i64> %76)
  %78 = extractelement <2 x i64> %64, i64 1
  %79 = icmp eq i64 %41, %51
  br i1 %79, label %84, label %80

80:                                               ; preds = %46, %75
  %81 = phi i64 [ %48, %46 ], [ %78, %75 ]
  %82 = phi i64 [ %42, %46 ], [ %52, %75 ]
  %83 = phi i64 [ 0, %46 ], [ %77, %75 ]
  br label %91

84:                                               ; preds = %91, %75
  %85 = phi i64 [ %77, %75 ], [ %100, %91 ]
  %86 = shl nuw nsw i64 %85, 1
  br label %87

87:                                               ; preds = %84, %32
  %88 = phi i64 [ 0, %32 ], [ %86, %84 ]
  %89 = add nuw nsw i64 %88, %34
  %90 = icmp samesign ult i64 %89, 2147483648
  br i1 %90, label %102, label %107

91:                                               ; preds = %80, %91
  %92 = phi i64 [ %97, %91 ], [ %81, %80 ]
  %93 = phi i64 [ %95, %91 ], [ %82, %80 ]
  %94 = phi i64 [ %100, %91 ], [ %83, %80 ]
  %95 = add nuw nsw i64 %93, 1
  %96 = getelementptr inbounds nuw i64, ptr %2, i64 %95
  %97 = load i64, ptr %96, align 8, !tbaa !8
  %98 = sub nsw i64 %97, %92
  %99 = tail call i64 @llvm.smax.i64(i64 %98, i64 0)
  %100 = add nuw nsw i64 %99, %94
  %101 = icmp slt i64 %95, %44
  br i1 %101, label %91, label %84, !llvm.loop !39

102:                                              ; preds = %87
  %103 = trunc nuw nsw i64 %89 to i32
  %104 = getelementptr inbounds nuw i32, ptr %28, i64 %33
  store i32 %103, ptr %104, align 4, !tbaa !20
  %105 = add nuw nsw i64 %33, 1
  %106 = icmp eq i64 %105, %31
  br i1 %106, label %108, label %32, !llvm.loop !40

107:                                              ; preds = %87
  tail call void @free(ptr noundef %25) #14
  br label %191

108:                                              ; preds = %102
  %109 = icmp eq i64 %89, 0
  br i1 %109, label %115, label %110

110:                                              ; preds = %108
  %111 = shl nuw nsw i64 %89, 2
  %112 = tail call noalias ptr @malloc(i64 noundef %111) #15
  %113 = icmp eq ptr %112, null
  br i1 %113, label %114, label %115

114:                                              ; preds = %110
  tail call void @free(ptr noundef nonnull %25) #14
  br label %191

115:                                              ; preds = %108, %110
  %116 = phi ptr [ %112, %110 ], [ null, %108 ]
  %117 = and i64 %20, 2147483647
  %118 = getelementptr i8, ptr %116, i64 16
  br label %121

119:                                              ; preds = %138, %27
  %120 = phi ptr [ null, %27 ], [ %116, %138 ]
  store ptr %25, ptr %4, align 8, !tbaa !17
  store ptr %120, ptr %5, align 8, !tbaa !17
  store i32 %21, ptr %7, align 4, !tbaa !20
  br label %191

121:                                              ; preds = %115, %138
  %122 = phi i64 [ 0, %115 ], [ %139, %138 ]
  %123 = mul nuw nsw i64 %122, %17
  %124 = add nuw nsw i64 %123, %17
  %125 = tail call i64 @llvm.smin.i64(i64 %124, i64 %0)
  %126 = icmp sgt i64 %0, %123
  br i1 %126, label %127, label %138

127:                                              ; preds = %121
  %128 = getelementptr inbounds nuw i32, ptr %25, i64 %122
  %129 = load i32, ptr %128, align 4, !tbaa !20
  %130 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %131 = load i64, ptr %130, align 8, !tbaa !8
  br label %141

132:                                              ; preds = %181, %176
  %133 = phi i64 [ %158, %176 ], [ %187, %181 ]
  %134 = trunc nsw i64 %133 to i32
  br label %135

135:                                              ; preds = %132, %141
  %136 = phi i32 [ %144, %141 ], [ %134, %132 ]
  %137 = icmp slt i64 %147, %125
  br i1 %137, label %141, label %138, !llvm.loop !41

138:                                              ; preds = %135, %121
  %139 = add nuw nsw i64 %122, 1
  %140 = icmp eq i64 %139, %117
  br i1 %140, label %119, label %121, !llvm.loop !42

141:                                              ; preds = %127, %135
  %142 = phi i64 [ %149, %135 ], [ %131, %127 ]
  %143 = phi i64 [ %147, %135 ], [ %123, %127 ]
  %144 = phi i32 [ %136, %135 ], [ %129, %127 ]
  %145 = sub nuw nsw i64 %143, %123
  %146 = trunc i64 %145 to i32
  %147 = add nuw nsw i64 %143, 1
  %148 = getelementptr inbounds nuw i64, ptr %2, i64 %147
  %149 = load i64, ptr %148, align 8, !tbaa !8
  %150 = icmp slt i64 %142, %149
  br i1 %150, label %151, label %135

151:                                              ; preds = %141
  %152 = sext i32 %144 to i64
  %153 = sub i64 %149, %142
  %154 = icmp ult i64 %153, 4
  br i1 %154, label %178, label %155

155:                                              ; preds = %151
  %156 = and i64 %153, -4
  %157 = shl i64 %156, 1
  %158 = add i64 %157, %152
  %159 = add i64 %142, %156
  %160 = insertelement <2 x i32> poison, i32 %146, i64 0
  %161 = getelementptr i32, ptr %3, i64 %142
  br label %162

162:                                              ; preds = %162, %155
  %163 = phi i64 [ 0, %155 ], [ %174, %162 ]
  %164 = shl i64 %163, 1
  %165 = add i64 %164, %152
  %166 = getelementptr inbounds i32, ptr %116, i64 %165
  %167 = getelementptr i32, ptr %118, i64 %165
  %168 = getelementptr i32, ptr %161, i64 %163
  %169 = getelementptr inbounds nuw i8, ptr %168, i64 8
  %170 = load <2 x i32>, ptr %168, align 4, !tbaa !20
  %171 = load <2 x i32>, ptr %169, align 4, !tbaa !20
  %172 = shufflevector <2 x i32> %160, <2 x i32> %170, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %172, ptr %166, align 4, !tbaa !20
  %173 = shufflevector <2 x i32> %160, <2 x i32> %171, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %173, ptr %167, align 4, !tbaa !20
  %174 = add nuw i64 %163, 4
  %175 = icmp eq i64 %174, %156
  br i1 %175, label %176, label %162, !llvm.loop !43

176:                                              ; preds = %162
  %177 = icmp eq i64 %153, %156
  br i1 %177, label %132, label %178

178:                                              ; preds = %151, %176
  %179 = phi i64 [ %152, %151 ], [ %158, %176 ]
  %180 = phi i64 [ %142, %151 ], [ %159, %176 ]
  br label %181

181:                                              ; preds = %178, %181
  %182 = phi i64 [ %187, %181 ], [ %179, %178 ]
  %183 = phi i64 [ %189, %181 ], [ %180, %178 ]
  %184 = getelementptr inbounds i32, ptr %116, i64 %182
  store i32 %146, ptr %184, align 4, !tbaa !20
  %185 = getelementptr inbounds i32, ptr %3, i64 %183
  %186 = load i32, ptr %185, align 4, !tbaa !20
  %187 = add nsw i64 %182, 2
  %188 = getelementptr i8, ptr %184, i64 4
  store i32 %186, ptr %188, align 4, !tbaa !20
  %189 = add nsw i64 %183, 1
  %190 = icmp eq i64 %189, %149
  br i1 %190, label %132, label %181, !llvm.loop !44

191:                                              ; preds = %107, %16, %114, %119, %8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_bcsr_to_csr(i64 noundef %0, i32 noundef %1, i32 noundef %2, ptr noundef readonly %3, ptr nocapture noundef readonly %4, ptr noundef writeonly %5, ptr noundef writeonly %6, ptr noundef writeonly %7) local_unnamed_addr #0 {
  %9 = insertelement <4 x ptr> poison, ptr %5, i64 0
  %10 = insertelement <4 x ptr> %9, ptr %6, i64 1
  %11 = insertelement <4 x ptr> %10, ptr %7, i64 2
  %12 = insertelement <4 x ptr> %11, ptr %3, i64 3
  %13 = icmp eq <4 x ptr> %12, zeroinitializer
  %14 = icmp slt i32 %2, 1
  %15 = icmp slt i32 %1, 0
  %16 = bitcast <4 x i1> %13 to i4
  %17 = icmp ne i4 %16, 0
  %18 = or i1 %17, %14
  %19 = or i1 %18, %15
  br i1 %19, label %167, label %20

20:                                               ; preds = %8
  %21 = add i64 %0, 1
  %22 = tail call noalias ptr @calloc(i64 noundef %21, i64 noundef 8) #13
  %23 = icmp eq ptr %22, null
  br i1 %23, label %167, label %24

24:                                               ; preds = %20
  %25 = icmp eq i32 %1, 0
  br i1 %25, label %33, label %26

26:                                               ; preds = %24
  %27 = zext nneg i32 %2 to i64
  %28 = getelementptr i8, ptr %22, i64 8
  %29 = zext nneg i32 %1 to i64
  %30 = load i32, ptr %3, align 4, !tbaa !20
  br label %44

31:                                               ; preds = %71, %44
  %32 = icmp eq i64 %50, %29
  br i1 %32, label %33, label %44, !llvm.loop !45

33:                                               ; preds = %31, %24
  %34 = icmp slt i64 %0, 1
  br i1 %34, label %89, label %35

35:                                               ; preds = %33
  %36 = load i64, ptr %22, align 8
  %37 = and i64 %0, 3
  %38 = icmp ult i64 %0, 4
  br i1 %38, label %75, label %39

39:                                               ; preds = %35
  %40 = and i64 %0, 9223372036854775804
  %41 = getelementptr i8, ptr %22, i64 8
  %42 = getelementptr i8, ptr %22, i64 16
  %43 = getelementptr i8, ptr %22, i64 24
  br label %93

44:                                               ; preds = %26, %31
  %45 = phi i32 [ %30, %26 ], [ %52, %31 ]
  %46 = phi i64 [ 0, %26 ], [ %50, %31 ]
  %47 = mul nuw nsw i64 %46, %27
  %48 = add nuw nsw i64 %47, %27
  %49 = tail call i64 @llvm.smin.i64(i64 %48, i64 %0)
  %50 = add nuw nsw i64 %46, 1
  %51 = getelementptr inbounds nuw i32, ptr %3, i64 %50
  %52 = load i32, ptr %51, align 4, !tbaa !20
  %53 = add nsw i32 %45, 1
  %54 = icmp slt i32 %53, %52
  br i1 %54, label %55, label %31

55:                                               ; preds = %44
  %56 = sext i32 %45 to i64
  br label %57

57:                                               ; preds = %55, %71
  %58 = phi i64 [ %56, %55 ], [ %72, %71 ]
  %59 = trunc i64 %58 to i32
  %60 = getelementptr inbounds i32, ptr %4, i64 %58
  %61 = load i32, ptr %60, align 4, !tbaa !20
  %62 = sext i32 %61 to i64
  %63 = add nsw i64 %47, %62
  %64 = icmp sgt i32 %61, -1
  %65 = icmp slt i64 %63, %49
  %66 = select i1 %64, i1 %65, i1 false
  br i1 %66, label %67, label %71

67:                                               ; preds = %57
  %68 = getelementptr i64, ptr %28, i64 %63
  %69 = load i64, ptr %68, align 8, !tbaa !8
  %70 = add nsw i64 %69, 1
  store i64 %70, ptr %68, align 8, !tbaa !8
  br label %71

71:                                               ; preds = %67, %57
  %72 = add nsw i64 %58, 2
  %73 = add i32 %59, 3
  %74 = icmp slt i32 %73, %52
  br i1 %74, label %57, label %31, !llvm.loop !46

75:                                               ; preds = %93, %35
  %76 = phi i64 [ %36, %35 ], [ %108, %93 ]
  %77 = phi i64 [ 1, %35 ], [ %109, %93 ]
  %78 = icmp eq i64 %37, 0
  br i1 %78, label %89, label %79

79:                                               ; preds = %75, %79
  %80 = phi i64 [ %85, %79 ], [ %76, %75 ]
  %81 = phi i64 [ %86, %79 ], [ %77, %75 ]
  %82 = phi i64 [ %87, %79 ], [ 0, %75 ]
  %83 = getelementptr i64, ptr %22, i64 %81
  %84 = load i64, ptr %83, align 8, !tbaa !8
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !8
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %37
  br i1 %88, label %89, label %79, !llvm.loop !47

89:                                               ; preds = %75, %79, %33
  %90 = getelementptr inbounds i64, ptr %22, i64 %0
  %91 = load i64, ptr %90, align 8, !tbaa !8
  %92 = icmp sgt i64 %91, 0
  br i1 %92, label %112, label %117

93:                                               ; preds = %93, %39
  %94 = phi i64 [ %36, %39 ], [ %108, %93 ]
  %95 = phi i64 [ 1, %39 ], [ %109, %93 ]
  %96 = phi i64 [ 0, %39 ], [ %110, %93 ]
  %97 = getelementptr i64, ptr %22, i64 %95
  %98 = load i64, ptr %97, align 8, !tbaa !8
  %99 = add nsw i64 %98, %94
  store i64 %99, ptr %97, align 8, !tbaa !8
  %100 = getelementptr i64, ptr %41, i64 %95
  %101 = load i64, ptr %100, align 8, !tbaa !8
  %102 = add nsw i64 %101, %99
  store i64 %102, ptr %100, align 8, !tbaa !8
  %103 = getelementptr i64, ptr %42, i64 %95
  %104 = load i64, ptr %103, align 8, !tbaa !8
  %105 = add nsw i64 %104, %102
  store i64 %105, ptr %103, align 8, !tbaa !8
  %106 = getelementptr i64, ptr %43, i64 %95
  %107 = load i64, ptr %106, align 8, !tbaa !8
  %108 = add nsw i64 %107, %105
  store i64 %108, ptr %106, align 8, !tbaa !8
  %109 = add nuw i64 %95, 4
  %110 = add i64 %96, 4
  %111 = icmp eq i64 %110, %40
  br i1 %111, label %75, label %93, !llvm.loop !48

112:                                              ; preds = %89
  %113 = shl i64 %91, 2
  %114 = tail call noalias ptr @malloc(i64 noundef %113) #15
  %115 = icmp eq ptr %114, null
  br i1 %115, label %116, label %117

116:                                              ; preds = %112
  tail call void @free(ptr noundef nonnull %22) #14
  br label %167

117:                                              ; preds = %112, %89
  %118 = phi ptr [ %114, %112 ], [ null, %89 ]
  %119 = shl i64 %21, 3
  %120 = tail call noalias ptr @malloc(i64 noundef %119) #15
  %121 = icmp eq ptr %120, null
  br i1 %121, label %122, label %123

122:                                              ; preds = %117
  tail call void @free(ptr noundef nonnull %22) #14
  tail call void @free(ptr noundef %118) #14
  br label %167

123:                                              ; preds = %117
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 %120, ptr nonnull align 8 %22, i64 %119, i1 false)
  br i1 %25, label %130, label %124

124:                                              ; preds = %123
  %125 = zext nneg i32 %2 to i64
  %126 = zext nneg i32 %1 to i64
  %127 = load i32, ptr %3, align 4, !tbaa !20
  br label %131

128:                                              ; preds = %162, %131
  %129 = icmp eq i64 %137, %126
  br i1 %129, label %130, label %131, !llvm.loop !49

130:                                              ; preds = %128, %123
  tail call void @free(ptr noundef %120) #14
  store ptr %22, ptr %5, align 8, !tbaa !14
  store ptr %118, ptr %6, align 8, !tbaa !17
  store i64 %91, ptr %7, align 8, !tbaa !8
  br label %167

131:                                              ; preds = %124, %128
  %132 = phi i32 [ %127, %124 ], [ %139, %128 ]
  %133 = phi i64 [ 0, %124 ], [ %137, %128 ]
  %134 = mul nuw nsw i64 %133, %125
  %135 = add nuw nsw i64 %134, %125
  %136 = tail call i64 @llvm.smin.i64(i64 %135, i64 %0)
  %137 = add nuw nsw i64 %133, 1
  %138 = getelementptr inbounds nuw i32, ptr %3, i64 %137
  %139 = load i32, ptr %138, align 4, !tbaa !20
  %140 = add nsw i32 %132, 1
  %141 = icmp slt i32 %140, %139
  br i1 %141, label %142, label %128

142:                                              ; preds = %131
  %143 = sext i32 %132 to i64
  br label %144

144:                                              ; preds = %142, %162
  %145 = phi i64 [ %143, %142 ], [ %163, %162 ]
  %146 = phi i32 [ %140, %142 ], [ %165, %162 ]
  %147 = getelementptr inbounds i32, ptr %4, i64 %145
  %148 = load i32, ptr %147, align 4, !tbaa !20
  %149 = sext i32 %148 to i64
  %150 = add nsw i64 %134, %149
  %151 = icmp sgt i32 %148, -1
  %152 = icmp slt i64 %150, %136
  %153 = select i1 %151, i1 %152, i1 false
  br i1 %153, label %154, label %162

154:                                              ; preds = %144
  %155 = sext i32 %146 to i64
  %156 = getelementptr inbounds i32, ptr %4, i64 %155
  %157 = load i32, ptr %156, align 4, !tbaa !20
  %158 = getelementptr inbounds nuw i64, ptr %120, i64 %150
  %159 = load i64, ptr %158, align 8, !tbaa !8
  %160 = add nsw i64 %159, 1
  store i64 %160, ptr %158, align 8, !tbaa !8
  %161 = getelementptr inbounds i32, ptr %118, i64 %159
  store i32 %157, ptr %161, align 4, !tbaa !20
  br label %162

162:                                              ; preds = %154, %144
  %163 = add nsw i64 %145, 2
  %164 = trunc nsw i64 %145 to i32
  %165 = add i32 %164, 3
  %166 = icmp slt i32 %165, %139
  br i1 %166, label %144, label %128, !llvm.loop !50

167:                                              ; preds = %20, %122, %130, %116, %8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readnone %3, ptr nocapture noundef readnone %4, ptr noundef %5, ptr noundef %6, ptr nocapture noundef readonly %7, i32 noundef %8) local_unnamed_addr #0 {
  %10 = alloca i64, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca i64, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca i64, align 8
  %17 = alloca ptr, align 8
  %18 = alloca ptr, align 8
  %19 = alloca i64, align 8
  %20 = alloca ptr, align 8
  %21 = alloca ptr, align 8
  %22 = alloca i64, align 8
  %23 = alloca ptr, align 8
  %24 = alloca ptr, align 8
  %25 = load i32, ptr @g_meta_count, align 4, !tbaa !20
  %26 = icmp sgt i32 %25, 0
  br i1 %26, label %27, label %39

27:                                               ; preds = %9
  %28 = zext nneg i32 %25 to i64
  br label %32

29:                                               ; preds = %32
  %30 = add nuw nsw i64 %33, 1
  %31 = icmp eq i64 %30, %28
  br i1 %31, label %37, label %32, !llvm.loop !51

32:                                               ; preds = %29, %27
  %33 = phi i64 [ 0, %27 ], [ %30, %29 ]
  %34 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %33
  %35 = load ptr, ptr %34, align 8, !tbaa !52
  %36 = icmp eq ptr %35, %0
  br i1 %36, label %44, label %29

37:                                               ; preds = %29
  %38 = icmp sgt i32 %25, 63
  br i1 %38, label %185, label %39

39:                                               ; preds = %37, %9
  %40 = add nsw i32 %25, 1
  store i32 %40, ptr @g_meta_count, align 4, !tbaa !20
  %41 = sext i32 %25 to i64
  %42 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %41
  %43 = getelementptr inbounds nuw i8, ptr %42, i64 8
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(120) %43, i8 0, i64 112, i1 false)
  store ptr %0, ptr %42, align 8, !tbaa !52
  store i32 3, ptr %43, align 8, !tbaa !54
  br label %47

44:                                               ; preds = %32
  %45 = getelementptr inbounds nuw i8, ptr %34, i64 8
  %46 = load i32, ptr %45, align 8, !tbaa !54
  br label %47

47:                                               ; preds = %44, %39
  %48 = phi i32 [ 3, %39 ], [ %46, %44 ]
  %49 = phi ptr [ %42, %39 ], [ %34, %44 ]
  %50 = getelementptr inbounds nuw i8, ptr %49, i64 8
  %51 = icmp eq i32 %48, %8
  br i1 %51, label %185, label %52

52:                                               ; preds = %47
  %53 = icmp eq i32 %8, 3
  br i1 %53, label %54, label %88

54:                                               ; preds = %52
  switch i32 %48, label %86 [
    i32 0, label %55
    i32 1, label %62
    i32 2, label %71
  ]

55:                                               ; preds = %54
  %56 = getelementptr inbounds nuw i8, ptr %49, i64 40
  %57 = load ptr, ptr %56, align 8, !tbaa !55
  %58 = getelementptr inbounds nuw i8, ptr %49, i64 48
  %59 = load ptr, ptr %58, align 8, !tbaa !56
  tail call void @rebuild_sets_from_csr(i64 noundef %1, i64 poison, ptr noundef %57, ptr noundef %59, ptr noundef %5, ptr noundef %6, ptr noundef %7)
  %60 = load ptr, ptr %56, align 8, !tbaa !55
  tail call void @free(ptr noundef %60) #14
  store ptr null, ptr %56, align 8, !tbaa !55
  %61 = load ptr, ptr %58, align 8, !tbaa !56
  tail call void @free(ptr noundef %61) #14
  store ptr null, ptr %58, align 8, !tbaa !56
  br label %86

62:                                               ; preds = %54
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %11) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %12) #14
  %63 = getelementptr inbounds nuw i8, ptr %49, i64 72
  %64 = load ptr, ptr %63, align 8, !tbaa !57
  %65 = getelementptr inbounds nuw i8, ptr %49, i64 80
  %66 = load ptr, ptr %65, align 8, !tbaa !58
  call void @convert_pcsr_to_csr(i64 noundef %1, i64 poison, ptr noundef %64, ptr noundef %66, ptr noundef nonnull %11, ptr noundef nonnull %12, ptr noundef nonnull %10)
  %67 = load ptr, ptr %11, align 8, !tbaa !14
  %68 = load ptr, ptr %12, align 8, !tbaa !17
  tail call void @rebuild_sets_from_csr(i64 noundef %1, i64 poison, ptr noundef %67, ptr noundef %68, ptr noundef %5, ptr noundef %6, ptr noundef %7)
  tail call void @free(ptr noundef %67) #14
  tail call void @free(ptr noundef %68) #14
  %69 = load ptr, ptr %63, align 8, !tbaa !57
  tail call void @free(ptr noundef %69) #14
  store ptr null, ptr %63, align 8, !tbaa !57
  %70 = load ptr, ptr %65, align 8, !tbaa !58
  tail call void @free(ptr noundef %70) #14
  store ptr null, ptr %65, align 8, !tbaa !58
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %12) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %11) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #14
  br label %86

71:                                               ; preds = %54
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %13) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %14) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %15) #14
  %72 = getelementptr inbounds nuw i8, ptr %49, i64 116
  %73 = load i32, ptr %72, align 4, !tbaa !59
  %74 = getelementptr inbounds nuw i8, ptr %49, i64 112
  %75 = load i32, ptr %74, align 8, !tbaa !60
  %76 = getelementptr inbounds nuw i8, ptr %49, i64 96
  %77 = load ptr, ptr %76, align 8, !tbaa !61
  %78 = getelementptr inbounds nuw i8, ptr %49, i64 104
  %79 = load ptr, ptr %78, align 8, !tbaa !62
  call void @convert_bcsr_to_csr(i64 noundef %1, i32 noundef %73, i32 noundef %75, ptr noundef %77, ptr noundef %79, ptr noundef nonnull %14, ptr noundef nonnull %15, ptr noundef nonnull %13)
  %80 = load ptr, ptr %14, align 8, !tbaa !14
  %81 = load ptr, ptr %15, align 8, !tbaa !17
  call void @rebuild_sets_from_csr(i64 noundef %1, i64 poison, ptr noundef %80, ptr noundef %81, ptr noundef %5, ptr noundef %6, ptr noundef %7)
  %82 = load ptr, ptr %14, align 8, !tbaa !14
  call void @free(ptr noundef %82) #14
  %83 = load ptr, ptr %15, align 8, !tbaa !17
  call void @free(ptr noundef %83) #14
  %84 = load ptr, ptr %76, align 8, !tbaa !61
  call void @free(ptr noundef %84) #14
  store ptr null, ptr %76, align 8, !tbaa !61
  %85 = load ptr, ptr %78, align 8, !tbaa !62
  call void @free(ptr noundef %85) #14
  store ptr null, ptr %78, align 8, !tbaa !62
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %15) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %14) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %13) #14
  br label %86

86:                                               ; preds = %54, %62, %71, %55
  %87 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %87, i8 0, i64 16, i1 false)
  store i32 3, ptr %50, align 8, !tbaa !54
  br label %185

88:                                               ; preds = %52
  %89 = icmp eq i32 %48, 3
  br i1 %89, label %90, label %109

90:                                               ; preds = %88
  switch i32 %8, label %185 [
    i32 0, label %91
    i32 1, label %95
    i32 2, label %102
  ]

91:                                               ; preds = %90
  %92 = getelementptr inbounds nuw i8, ptr %49, i64 40
  %93 = getelementptr inbounds nuw i8, ptr %49, i64 48
  %94 = getelementptr inbounds nuw i8, ptr %49, i64 64
  tail call void @build_csr_from_set(i64 noundef %1, ptr noundef %6, ptr noundef %7, ptr noundef nonnull %92, ptr noundef nonnull %93, ptr noundef nonnull %94)
  store i32 0, ptr %50, align 8, !tbaa !54
  br label %172

95:                                               ; preds = %90
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %16) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %17) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %18) #14
  call void @build_csr_from_set(i64 noundef %1, ptr noundef %6, ptr noundef %7, ptr noundef nonnull %17, ptr noundef nonnull %18, ptr noundef nonnull %16)
  %96 = load i64, ptr %16, align 8, !tbaa !8
  %97 = load ptr, ptr %17, align 8, !tbaa !14
  %98 = load ptr, ptr %18, align 8, !tbaa !17
  %99 = getelementptr inbounds nuw i8, ptr %49, i64 72
  %100 = getelementptr inbounds nuw i8, ptr %49, i64 80
  %101 = getelementptr inbounds nuw i8, ptr %49, i64 88
  tail call void @convert_csr_to_pcsr(i64 noundef %1, i64 noundef %96, ptr noundef %97, ptr noundef %98, ptr noundef nonnull %99, ptr noundef nonnull %100, ptr noundef nonnull %101)
  tail call void @free(ptr noundef %97) #14
  tail call void @free(ptr noundef %98) #14
  store i32 1, ptr %50, align 8, !tbaa !54
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %18) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %17) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %16) #14
  br label %179

102:                                              ; preds = %90
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %19) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %20) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %21) #14
  call void @build_csr_from_set(i64 noundef %1, ptr noundef %6, ptr noundef %7, ptr noundef nonnull %20, ptr noundef nonnull %21, ptr noundef nonnull %19)
  %103 = getelementptr inbounds nuw i8, ptr %49, i64 112
  store i32 64, ptr %103, align 8, !tbaa !60
  %104 = load ptr, ptr %20, align 8, !tbaa !14
  %105 = load ptr, ptr %21, align 8, !tbaa !17
  %106 = getelementptr inbounds nuw i8, ptr %49, i64 96
  %107 = getelementptr inbounds nuw i8, ptr %49, i64 104
  %108 = getelementptr inbounds nuw i8, ptr %49, i64 116
  tail call void @convert_csr_to_bcsr(i64 noundef %1, i64 poison, ptr noundef %104, ptr noundef %105, ptr noundef nonnull %106, ptr noundef nonnull %107, i32 noundef 64, ptr noundef nonnull %108)
  tail call void @free(ptr noundef %104) #14
  tail call void @free(ptr noundef %105) #14
  store i32 2, ptr %50, align 8, !tbaa !54
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %20) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %19) #14
  br label %183

109:                                              ; preds = %88
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %22) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %23) #14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %24) #14
  switch i32 %48, label %128 [
    i32 1, label %110
    i32 2, label %117
  ]

110:                                              ; preds = %109
  %111 = getelementptr inbounds nuw i8, ptr %49, i64 72
  %112 = load ptr, ptr %111, align 8, !tbaa !57
  %113 = getelementptr inbounds nuw i8, ptr %49, i64 80
  %114 = load ptr, ptr %113, align 8, !tbaa !58
  call void @convert_pcsr_to_csr(i64 noundef %1, i64 poison, ptr noundef %112, ptr noundef %114, ptr noundef nonnull %23, ptr noundef nonnull %24, ptr noundef nonnull %22)
  %115 = load ptr, ptr %111, align 8, !tbaa !57
  tail call void @free(ptr noundef %115) #14
  store ptr null, ptr %111, align 8, !tbaa !57
  %116 = load ptr, ptr %113, align 8, !tbaa !58
  tail call void @free(ptr noundef %116) #14
  store ptr null, ptr %113, align 8, !tbaa !58
  br label %135

117:                                              ; preds = %109
  %118 = getelementptr inbounds nuw i8, ptr %49, i64 116
  %119 = load i32, ptr %118, align 4, !tbaa !59
  %120 = getelementptr inbounds nuw i8, ptr %49, i64 112
  %121 = load i32, ptr %120, align 8, !tbaa !60
  %122 = getelementptr inbounds nuw i8, ptr %49, i64 96
  %123 = load ptr, ptr %122, align 8, !tbaa !61
  %124 = getelementptr inbounds nuw i8, ptr %49, i64 104
  %125 = load ptr, ptr %124, align 8, !tbaa !62
  call void @convert_bcsr_to_csr(i64 noundef %1, i32 noundef %119, i32 noundef %121, ptr noundef %123, ptr noundef %125, ptr noundef nonnull %23, ptr noundef nonnull %24, ptr noundef nonnull %22)
  %126 = load ptr, ptr %122, align 8, !tbaa !61
  call void @free(ptr noundef %126) #14
  store ptr null, ptr %122, align 8, !tbaa !61
  %127 = load ptr, ptr %124, align 8, !tbaa !62
  call void @free(ptr noundef %127) #14
  store ptr null, ptr %124, align 8, !tbaa !62
  br label %135

128:                                              ; preds = %109
  %129 = getelementptr inbounds nuw i8, ptr %49, i64 64
  %130 = load i64, ptr %129, align 8, !tbaa !63
  store i64 %130, ptr %22, align 8, !tbaa !8
  %131 = getelementptr inbounds nuw i8, ptr %49, i64 40
  %132 = load ptr, ptr %131, align 8, !tbaa !55
  store ptr %132, ptr %23, align 8, !tbaa !14
  %133 = getelementptr inbounds nuw i8, ptr %49, i64 48
  %134 = load ptr, ptr %133, align 8, !tbaa !56
  store ptr %134, ptr %24, align 8, !tbaa !17
  br label %135

135:                                              ; preds = %117, %128, %110
  switch i32 %8, label %171 [
    i32 0, label %136
    i32 1, label %143
    i32 2, label %156
  ]

136:                                              ; preds = %135
  %137 = load i64, ptr %22, align 8, !tbaa !8
  %138 = getelementptr inbounds nuw i8, ptr %49, i64 64
  store i64 %137, ptr %138, align 8, !tbaa !63
  %139 = load ptr, ptr %23, align 8, !tbaa !14
  %140 = getelementptr inbounds nuw i8, ptr %49, i64 40
  store ptr %139, ptr %140, align 8, !tbaa !55
  %141 = load ptr, ptr %24, align 8, !tbaa !17
  %142 = getelementptr inbounds nuw i8, ptr %49, i64 48
  store ptr %141, ptr %142, align 8, !tbaa !56
  store i32 %8, ptr %50, align 8, !tbaa !54
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %24) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #14
  br label %172

143:                                              ; preds = %135
  %144 = load i64, ptr %22, align 8, !tbaa !8
  %145 = load ptr, ptr %23, align 8, !tbaa !14
  %146 = load ptr, ptr %24, align 8, !tbaa !17
  %147 = getelementptr inbounds nuw i8, ptr %49, i64 72
  %148 = getelementptr inbounds nuw i8, ptr %49, i64 80
  %149 = getelementptr inbounds nuw i8, ptr %49, i64 88
  call void @convert_csr_to_pcsr(i64 noundef %1, i64 noundef %144, ptr noundef %145, ptr noundef %146, ptr noundef nonnull %147, ptr noundef nonnull %148, ptr noundef nonnull %149)
  %150 = load ptr, ptr %23, align 8, !tbaa !14
  call void @free(ptr noundef %150) #14
  %151 = load ptr, ptr %24, align 8, !tbaa !17
  call void @free(ptr noundef %151) #14
  %152 = load i32, ptr %50, align 8, !tbaa !54
  %153 = icmp eq i32 %152, 0
  br i1 %153, label %154, label %169

154:                                              ; preds = %143
  %155 = getelementptr inbounds nuw i8, ptr %49, i64 40
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %155, i8 0, i64 16, i1 false)
  br label %169

156:                                              ; preds = %135
  %157 = getelementptr inbounds nuw i8, ptr %49, i64 112
  store i32 64, ptr %157, align 8, !tbaa !60
  %158 = load ptr, ptr %23, align 8, !tbaa !14
  %159 = load ptr, ptr %24, align 8, !tbaa !17
  %160 = getelementptr inbounds nuw i8, ptr %49, i64 96
  %161 = getelementptr inbounds nuw i8, ptr %49, i64 104
  %162 = getelementptr inbounds nuw i8, ptr %49, i64 116
  call void @convert_csr_to_bcsr(i64 noundef %1, i64 poison, ptr noundef %158, ptr noundef %159, ptr noundef nonnull %160, ptr noundef nonnull %161, i32 noundef 64, ptr noundef nonnull %162)
  %163 = load ptr, ptr %23, align 8, !tbaa !14
  call void @free(ptr noundef %163) #14
  %164 = load ptr, ptr %24, align 8, !tbaa !17
  call void @free(ptr noundef %164) #14
  %165 = load i32, ptr %50, align 8, !tbaa !54
  %166 = icmp eq i32 %165, 0
  br i1 %166, label %167, label %170

167:                                              ; preds = %156
  %168 = getelementptr inbounds nuw i8, ptr %49, i64 40
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %168, i8 0, i64 16, i1 false)
  br label %170

169:                                              ; preds = %143, %154
  store i32 %8, ptr %50, align 8, !tbaa !54
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %24) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #14
  br label %179

170:                                              ; preds = %167, %156
  store i32 %8, ptr %50, align 8, !tbaa !54
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %24) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #14
  br label %183

171:                                              ; preds = %135
  store i32 %8, ptr %50, align 8, !tbaa !54
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %24) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #14
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #14
  br label %185

172:                                              ; preds = %136, %91
  %173 = getelementptr inbounds nuw i8, ptr %49, i64 40
  %174 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %175 = load <2 x ptr>, ptr %173, align 8, !tbaa !64
  store <2 x ptr> %175, ptr %174, align 8, !tbaa !64
  %176 = getelementptr inbounds nuw i8, ptr %49, i64 64
  %177 = load i64, ptr %176, align 8, !tbaa !63
  %178 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %177, ptr %178, align 8, !tbaa !8
  br label %185

179:                                              ; preds = %169, %95
  %180 = getelementptr inbounds nuw i8, ptr %49, i64 72
  %181 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %182 = load <2 x ptr>, ptr %180, align 8, !tbaa !64
  store <2 x ptr> %182, ptr %181, align 8, !tbaa !64
  br label %185

183:                                              ; preds = %170, %102
  %184 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %184, i8 0, i64 16, i1 false)
  br label %185

185:                                              ; preds = %171, %90, %37, %172, %183, %179, %47, %86
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @autograph_get_layout(ptr noundef readnone %0) local_unnamed_addr #9 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !20
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %17

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %17, label %9, !llvm.loop !65

9:                                                ; preds = %4, %6
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 8, !tbaa !52
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !54
  br label %17

17:                                               ; preds = %6, %1, %14
  %18 = phi i32 [ %16, %14 ], [ 3, %1 ], [ 3, %6 ]
  ret i32 %18
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_debug_print(ptr nocapture noundef readnone %0) local_unnamed_addr #10 {
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_update_csr_pointers(ptr noundef readnone %0, ptr noundef %1, ptr noundef %2) local_unnamed_addr #11 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !20
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %19

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %19, label %11, !llvm.loop !51

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !52
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 40
  store ptr %1, ptr %17, align 8, !tbaa !55
  %18 = getelementptr inbounds nuw i8, ptr %13, i64 48
  store ptr %2, ptr %18, align 8, !tbaa !56
  br label %19

19:                                               ; preds = %8, %3, %16
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_init(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4, ptr noundef %5) local_unnamed_addr #0 {
  %7 = load i32, ptr @g_meta_count, align 4, !tbaa !20
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %9, label %21

9:                                                ; preds = %6
  %10 = zext nneg i32 %7 to i64
  br label %14

11:                                               ; preds = %14
  %12 = add nuw nsw i64 %15, 1
  %13 = icmp eq i64 %12, %10
  br i1 %13, label %19, label %14, !llvm.loop !51

14:                                               ; preds = %11, %9
  %15 = phi i64 [ 0, %9 ], [ %12, %11 ]
  %16 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %15
  %17 = load ptr, ptr %16, align 8, !tbaa !52
  %18 = icmp eq ptr %17, %0
  br i1 %18, label %26, label %11

19:                                               ; preds = %11
  %20 = icmp sgt i32 %7, 63
  br i1 %20, label %40, label %21

21:                                               ; preds = %19, %6
  %22 = add nsw i32 %7, 1
  store i32 %22, ptr @g_meta_count, align 4, !tbaa !20
  %23 = sext i32 %7 to i64
  %24 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %23
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(120) %25, i8 0, i64 112, i1 false)
  store ptr %0, ptr %24, align 8, !tbaa !52
  store i32 3, ptr %25, align 8, !tbaa !54
  br label %26

26:                                               ; preds = %14, %21
  %27 = phi ptr [ %24, %21 ], [ %16, %14 ]
  %28 = getelementptr inbounds nuw i8, ptr %27, i64 16
  store ptr %3, ptr %28, align 8, !tbaa !66
  %29 = getelementptr inbounds nuw i8, ptr %27, i64 24
  store ptr %4, ptr %29, align 8, !tbaa !67
  %30 = getelementptr inbounds nuw i8, ptr %27, i64 32
  store ptr %5, ptr %30, align 8, !tbaa !68
  %31 = getelementptr inbounds nuw i8, ptr %27, i64 56
  store i64 %1, ptr %31, align 8, !tbaa !69
  %32 = getelementptr inbounds nuw i8, ptr %27, i64 64
  store i64 %2, ptr %32, align 8, !tbaa !63
  %33 = getelementptr inbounds nuw i8, ptr %27, i64 8
  store i32 3, ptr %33, align 8, !tbaa !54
  %34 = getelementptr inbounds nuw i8, ptr %27, i64 40
  %35 = getelementptr inbounds nuw i8, ptr %27, i64 48
  tail call void @build_csr_from_set(i64 noundef %1, ptr noundef %4, ptr noundef %5, ptr noundef nonnull %34, ptr noundef nonnull %35, ptr noundef nonnull %32)
  store i32 0, ptr %33, align 8, !tbaa !54
  %36 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %37 = load <2 x ptr>, ptr %34, align 8, !tbaa !64
  store <2 x ptr> %37, ptr %36, align 8, !tbaa !64
  %38 = load i64, ptr %32, align 8, !tbaa !63
  %39 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %38, ptr %39, align 8, !tbaa !8
  br label %40

40:                                               ; preds = %19, %26
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x i64> @llvm.smax.v2i64(<2 x i64>, <2 x i64>) #12

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #6 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #12 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #13 = { nounwind allocsize(0,1) }
attributes #14 = { nounwind }
attributes #15 = { nounwind allocsize(0) }
attributes #16 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"", !5, i64 0, !5, i64 8}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!5, !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.unroll.disable"}
!13 = distinct !{!13, !10}
!14 = !{!15, !15, i64 0}
!15 = !{!"p1 long", !16, i64 0}
!16 = !{!"any pointer", !6, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"p1 int", !16, i64 0}
!19 = !{!4, !5, i64 8}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !6, i64 0}
!22 = distinct !{!22, !10}
!23 = distinct !{!23, !10}
!24 = distinct !{!24, !10}
!25 = distinct !{!25, !10}
!26 = distinct !{!26, !10}
!27 = distinct !{!27, !10, !28, !29}
!28 = !{!"llvm.loop.isvectorized", i32 1}
!29 = !{!"llvm.loop.unroll.runtime.disable"}
!30 = distinct !{!30, !10, !28}
!31 = distinct !{!31, !10, !28, !29}
!32 = distinct !{!32, !10}
!33 = distinct !{!33, !10, !29, !28}
!34 = distinct !{!34, !12}
!35 = distinct !{!35, !10}
!36 = distinct !{!36, !10}
!37 = distinct !{!37, !10}
!38 = distinct !{!38, !10, !28, !29}
!39 = distinct !{!39, !10, !29, !28}
!40 = distinct !{!40, !10}
!41 = distinct !{!41, !10}
!42 = distinct !{!42, !10}
!43 = distinct !{!43, !10, !28, !29}
!44 = distinct !{!44, !10, !29, !28}
!45 = distinct !{!45, !10}
!46 = distinct !{!46, !10}
!47 = distinct !{!47, !12}
!48 = distinct !{!48, !10}
!49 = distinct !{!49, !10}
!50 = distinct !{!50, !10}
!51 = distinct !{!51, !10}
!52 = !{!53, !16, i64 0}
!53 = !{!"", !16, i64 0, !21, i64 8, !16, i64 16, !16, i64 24, !16, i64 32, !15, i64 40, !18, i64 48, !5, i64 56, !5, i64 64, !15, i64 72, !18, i64 80, !5, i64 88, !18, i64 96, !18, i64 104, !21, i64 112, !21, i64 116}
!54 = !{!53, !21, i64 8}
!55 = !{!53, !15, i64 40}
!56 = !{!53, !18, i64 48}
!57 = !{!53, !15, i64 72}
!58 = !{!53, !18, i64 80}
!59 = !{!53, !21, i64 116}
!60 = !{!53, !21, i64 112}
!61 = !{!53, !18, i64 96}
!62 = !{!53, !18, i64 104}
!63 = !{!53, !5, i64 64}
!64 = !{!16, !16, i64 0}
!65 = distinct !{!65, !10}
!66 = !{!53, !16, i64 16}
!67 = !{!53, !16, i64 24}
!68 = !{!53, !16, i64 32}
!69 = !{!53, !5, i64 56}
