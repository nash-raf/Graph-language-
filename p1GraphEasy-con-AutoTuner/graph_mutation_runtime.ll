; ModuleID = 'graph_mutation_runtime.c'
source_filename = "graph_mutation_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_node(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %20, label %6

6:                                                ; preds = %4
  %7 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %8 = icmp eq i32 %7, 3
  br i1 %8, label %18, label %9

9:                                                ; preds = %6
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  %10 = tail call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #4
  %11 = load i64, ptr %0, align 8, !tbaa !3
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %13 = load i64, ptr %12, align 8, !tbaa !11
  %14 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %15 = load ptr, ptr %14, align 8, !tbaa !12
  %16 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %17 = load ptr, ptr %16, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %11, i64 noundef %13, ptr noundef %15, ptr noundef %17, ptr noundef %1, ptr noundef null, ptr noundef %2, i32 noundef %7) #4
  br label %20

18:                                               ; preds = %6
  %19 = tail call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #4
  br label %20

20:                                               ; preds = %9, %18, %4
  ret void
}

declare dso_local i32 @autograph_get_layout(ptr noundef) local_unnamed_addr #1

declare dso_local void @autograph_ensure_layout_set(ptr noundef) local_unnamed_addr #1

declare dso_local i32 @autograph_canonical_add_node(ptr noundef, i32 noundef) local_unnamed_addr #1

declare dso_local void @autograph_ensure_layout(ptr noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_node(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %20, label %6

6:                                                ; preds = %4
  %7 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %8 = icmp eq i32 %7, 3
  br i1 %8, label %18, label %9

9:                                                ; preds = %6
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  %10 = tail call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #4
  %11 = load i64, ptr %0, align 8, !tbaa !3
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %13 = load i64, ptr %12, align 8, !tbaa !11
  %14 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %15 = load ptr, ptr %14, align 8, !tbaa !12
  %16 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %17 = load ptr, ptr %16, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %11, i64 noundef %13, ptr noundef %15, ptr noundef %17, ptr noundef %1, ptr noundef null, ptr noundef %2, i32 noundef %7) #4
  br label %20

18:                                               ; preds = %6
  %19 = tail call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #4
  br label %20

20:                                               ; preds = %9, %18, %4
  ret void
}

declare dso_local i32 @autograph_canonical_remove_node(ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_edge(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %111, label %7

7:                                                ; preds = %5
  %8 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %9 = icmp sgt i32 %2, -1
  br i1 %9, label %10, label %19

10:                                               ; preds = %7
  %11 = zext nneg i32 %2 to i64
  %12 = load i64, ptr %0, align 8, !tbaa !3
  %13 = icmp sgt i64 %12, %11
  %14 = icmp sgt i32 %3, -1
  %15 = and i1 %14, %13
  br i1 %15, label %16, label %19

16:                                               ; preds = %10
  %17 = zext nneg i32 %3 to i64
  %18 = icmp samesign ugt i64 %12, %17
  br label %19

19:                                               ; preds = %16, %10, %7
  %20 = phi i1 [ false, %10 ], [ false, %7 ], [ %18, %16 ]
  switch i32 %8, label %108 [
    i32 0, label %21
    i32 1, label %32
  ]

21:                                               ; preds = %19
  tail call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %2, i32 noundef %3)
  tail call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %3, i32 noundef %2)
  br i1 %20, label %111, label %22

22:                                               ; preds = %21
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  %23 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %24 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  %25 = load i64, ptr %0, align 8, !tbaa !3
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %27 = load i64, ptr %26, align 8, !tbaa !11
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %29 = load ptr, ptr %28, align 8, !tbaa !12
  %30 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %31 = load ptr, ptr %30, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %25, i64 noundef %27, ptr noundef %29, ptr noundef %31, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 0) #4
  br label %111

32:                                               ; preds = %19
  %33 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %34 = load ptr, ptr %33, align 8, !tbaa !12
  %35 = icmp eq ptr %34, null
  br i1 %35, label %98, label %36

36:                                               ; preds = %32
  %37 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %38 = load ptr, ptr %37, align 8, !tbaa !13
  %39 = icmp eq ptr %38, null
  %40 = icmp slt i32 %2, 0
  %41 = or i1 %40, %39
  br i1 %41, label %69, label %42

42:                                               ; preds = %36
  %43 = zext nneg i32 %2 to i64
  %44 = load i64, ptr %0, align 8, !tbaa !3
  %45 = icmp sgt i64 %44, %43
  br i1 %45, label %46, label %69

46:                                               ; preds = %42
  %47 = icmp sgt i32 %3, -1
  %48 = zext nneg i32 %3 to i64
  %49 = icmp samesign ugt i64 %44, %48
  %50 = select i1 %47, i1 %49, i1 false
  br i1 %50, label %51, label %69

51:                                               ; preds = %46
  %52 = getelementptr inbounds nuw i64, ptr %34, i64 %43
  %53 = load i64, ptr %52, align 8, !tbaa !14
  %54 = getelementptr inbounds nuw i8, ptr %52, i64 8
  %55 = load i64, ptr %54, align 8, !tbaa !14
  %56 = icmp slt i64 %53, %55
  br i1 %56, label %57, label %69

57:                                               ; preds = %51, %66
  %58 = phi i64 [ %67, %66 ], [ %53, %51 ]
  %59 = getelementptr inbounds i32, ptr %38, i64 %58
  %60 = load i32, ptr %59, align 4, !tbaa !15
  %61 = icmp eq i32 %60, %3
  br i1 %61, label %69, label %62

62:                                               ; preds = %57
  %63 = icmp eq i32 %60, -1
  br i1 %63, label %64, label %66

64:                                               ; preds = %62
  %65 = getelementptr inbounds i32, ptr %38, i64 %58
  store i32 %3, ptr %65, align 4, !tbaa !15
  br label %69

66:                                               ; preds = %62
  %67 = add i64 %58, 1
  %68 = icmp eq i64 %67, %55
  br i1 %68, label %69, label %57, !llvm.loop !17

69:                                               ; preds = %66, %57, %64, %51, %46, %42, %36
  %70 = icmp slt i32 %3, 0
  %71 = or i1 %70, %39
  br i1 %71, label %98, label %72

72:                                               ; preds = %69
  %73 = zext nneg i32 %3 to i64
  %74 = load i64, ptr %0, align 8, !tbaa !3
  %75 = icmp sgt i64 %74, %73
  %76 = zext nneg i32 %2 to i64
  %77 = icmp samesign ugt i64 %74, %76
  %78 = and i1 %9, %75
  %79 = select i1 %78, i1 %77, i1 false
  br i1 %79, label %80, label %98

80:                                               ; preds = %72
  %81 = getelementptr inbounds nuw i64, ptr %34, i64 %73
  %82 = load i64, ptr %81, align 8, !tbaa !14
  %83 = getelementptr inbounds nuw i8, ptr %81, i64 8
  %84 = load i64, ptr %83, align 8, !tbaa !14
  %85 = icmp slt i64 %82, %84
  br i1 %85, label %86, label %98

86:                                               ; preds = %80, %95
  %87 = phi i64 [ %96, %95 ], [ %82, %80 ]
  %88 = getelementptr inbounds i32, ptr %38, i64 %87
  %89 = load i32, ptr %88, align 4, !tbaa !15
  %90 = icmp eq i32 %89, %2
  br i1 %90, label %98, label %91

91:                                               ; preds = %86
  %92 = icmp eq i32 %89, -1
  br i1 %92, label %93, label %95

93:                                               ; preds = %91
  %94 = getelementptr inbounds i32, ptr %38, i64 %87
  store i32 %2, ptr %94, align 4, !tbaa !15
  br label %98

95:                                               ; preds = %91
  %96 = add i64 %87, 1
  %97 = icmp eq i64 %96, %84
  br i1 %97, label %98, label %86, !llvm.loop !17

98:                                               ; preds = %86, %95, %32, %69, %72, %80, %93
  br i1 %20, label %111, label %99

99:                                               ; preds = %98
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  %100 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %101 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  %102 = load i64, ptr %0, align 8, !tbaa !3
  %103 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %104 = load i64, ptr %103, align 8, !tbaa !11
  %105 = load ptr, ptr %33, align 8, !tbaa !12
  %106 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %107 = load ptr, ptr %106, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %102, i64 noundef %104, ptr noundef %105, ptr noundef %107, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 1) #4
  br label %111

108:                                              ; preds = %19
  %109 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %110 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  br label %111

111:                                              ; preds = %108, %22, %21, %99, %98, %5
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @csr_add_directed(ptr nocapture noundef nonnull %0, ptr noundef nonnull %1, i32 noundef %2, i32 noundef %3) unnamed_addr #0 {
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !12
  %7 = icmp eq ptr %6, null
  br i1 %7, label %57, label %8

8:                                                ; preds = %4
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %10 = load ptr, ptr %9, align 8, !tbaa !13
  %11 = icmp eq ptr %10, null
  %12 = icmp slt i32 %2, 0
  %13 = or i1 %12, %11
  br i1 %13, label %57, label %14

14:                                               ; preds = %8
  %15 = zext nneg i32 %2 to i64
  %16 = load i64, ptr %0, align 8, !tbaa !3
  %17 = icmp sgt i64 %16, %15
  %18 = icmp sgt i32 %3, -1
  %19 = and i1 %18, %17
  %20 = zext nneg i32 %3 to i64
  %21 = icmp sgt i64 %16, %20
  %22 = select i1 %19, i1 %21, i1 false
  br i1 %22, label %23, label %57

23:                                               ; preds = %14
  %24 = getelementptr inbounds nuw i64, ptr %6, i64 %15
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  %26 = load i64, ptr %25, align 8, !tbaa !14
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %28 = load i64, ptr %27, align 8, !tbaa !11
  %29 = add nsw i64 %28, 1
  %30 = shl i64 %29, 2
  %31 = tail call ptr @realloc(ptr noundef nonnull %10, i64 noundef %30) #5
  %32 = icmp eq ptr %31, null
  br i1 %32, label %57, label %33

33:                                               ; preds = %23
  store ptr %31, ptr %9, align 8, !tbaa !13
  %34 = load ptr, ptr %5, align 8, !tbaa !12
  tail call void @autograph_update_csr_pointers(ptr noundef nonnull %1, ptr noundef %34, ptr noundef nonnull %31) #4
  %35 = icmp slt i64 %26, %28
  br i1 %35, label %36, label %42

36:                                               ; preds = %33
  %37 = load ptr, ptr %9, align 8, !tbaa !13
  %38 = getelementptr i32, ptr %37, i64 %26
  %39 = getelementptr i8, ptr %38, i64 4
  %40 = sub nsw i64 %28, %26
  %41 = shl i64 %40, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %39, ptr align 4 %38, i64 %41, i1 false)
  br label %42

42:                                               ; preds = %36, %33
  %43 = load ptr, ptr %9, align 8, !tbaa !13
  %44 = getelementptr inbounds i32, ptr %43, i64 %26
  store i32 %3, ptr %44, align 4, !tbaa !15
  store i64 %29, ptr %27, align 8, !tbaa !11
  %45 = load i64, ptr %0, align 8, !tbaa !3
  %46 = icmp sgt i64 %45, %15
  br i1 %46, label %47, label %57

47:                                               ; preds = %42
  %48 = load ptr, ptr %5, align 8, !tbaa !12
  br label %49

49:                                               ; preds = %47, %49
  %50 = phi i64 [ %15, %47 ], [ %51, %49 ]
  %51 = add nuw nsw i64 %50, 1
  %52 = getelementptr inbounds nuw i64, ptr %48, i64 %51
  %53 = load i64, ptr %52, align 8, !tbaa !14
  %54 = add nsw i64 %53, 1
  store i64 %54, ptr %52, align 8, !tbaa !14
  %55 = load i64, ptr %0, align 8, !tbaa !3
  %56 = icmp slt i64 %51, %55
  br i1 %56, label %49, label %57, !llvm.loop !19

57:                                               ; preds = %49, %42, %23, %14, %4, %8
  ret void
}

declare dso_local i32 @autograph_canonical_add_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_edge(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %217, label %7

7:                                                ; preds = %5
  %8 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %9 = icmp sgt i32 %2, -1
  br i1 %9, label %10, label %19

10:                                               ; preds = %7
  %11 = zext nneg i32 %2 to i64
  %12 = load i64, ptr %0, align 8, !tbaa !3
  %13 = icmp sgt i64 %12, %11
  %14 = icmp sgt i32 %3, -1
  %15 = and i1 %14, %13
  br i1 %15, label %16, label %19

16:                                               ; preds = %10
  %17 = zext nneg i32 %3 to i64
  %18 = icmp samesign ugt i64 %12, %17
  br label %19

19:                                               ; preds = %16, %10, %7
  %20 = phi i1 [ false, %10 ], [ false, %7 ], [ %18, %16 ]
  switch i32 %8, label %214 [
    i32 0, label %21
    i32 1, label %142
  ]

21:                                               ; preds = %19
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %23 = load ptr, ptr %22, align 8, !tbaa !12
  %24 = icmp eq ptr %23, null
  br i1 %24, label %132, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %27 = load ptr, ptr %26, align 8, !tbaa !13
  %28 = icmp eq ptr %27, null
  %29 = icmp slt i32 %2, 0
  %30 = or i1 %29, %28
  br i1 %30, label %80, label %31

31:                                               ; preds = %25
  %32 = zext nneg i32 %2 to i64
  %33 = load i64, ptr %0, align 8, !tbaa !3
  %34 = icmp sgt i64 %33, %32
  br i1 %34, label %35, label %80

35:                                               ; preds = %31
  %36 = getelementptr inbounds nuw i64, ptr %23, i64 %32
  %37 = load i64, ptr %36, align 8, !tbaa !14
  %38 = getelementptr inbounds nuw i8, ptr %36, i64 8
  %39 = load i64, ptr %38, align 8, !tbaa !14
  %40 = icmp slt i64 %37, %39
  br i1 %40, label %41, label %80

41:                                               ; preds = %35, %46
  %42 = phi i64 [ %47, %46 ], [ %37, %35 ]
  %43 = getelementptr inbounds i32, ptr %27, i64 %42
  %44 = load i32, ptr %43, align 4, !tbaa !15
  %45 = icmp eq i32 %44, %3
  br i1 %45, label %49, label %46

46:                                               ; preds = %41
  %47 = add i64 %42, 1
  %48 = icmp eq i64 %47, %39
  br i1 %48, label %77, label %41, !llvm.loop !20

49:                                               ; preds = %41
  %50 = getelementptr inbounds i32, ptr %27, i64 %42
  %51 = icmp slt i64 %42, 0
  br i1 %51, label %77, label %52

52:                                               ; preds = %49
  %53 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %54 = load i64, ptr %53, align 8, !tbaa !11
  %55 = add nuw nsw i64 %42, 1
  %56 = icmp slt i64 %55, %54
  br i1 %56, label %57, label %64

57:                                               ; preds = %52
  %58 = getelementptr inbounds nuw i32, ptr %27, i64 %55
  %59 = xor i64 %42, -1
  %60 = add nsw i64 %54, %59
  %61 = shl i64 %60, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %50, ptr nonnull align 4 %58, i64 %61, i1 false)
  %62 = load i64, ptr %0, align 8, !tbaa !3
  %63 = load ptr, ptr %22, align 8, !tbaa !12
  br label %64

64:                                               ; preds = %57, %52
  %65 = phi ptr [ %63, %57 ], [ %23, %52 ]
  %66 = phi i64 [ %62, %57 ], [ %33, %52 ]
  %67 = add nsw i64 %54, -1
  store i64 %67, ptr %53, align 8, !tbaa !11
  %68 = icmp sgt i64 %66, %32
  br i1 %68, label %69, label %77

69:                                               ; preds = %64, %69
  %70 = phi i64 [ %71, %69 ], [ %32, %64 ]
  %71 = add nuw nsw i64 %70, 1
  %72 = getelementptr inbounds nuw i64, ptr %65, i64 %71
  %73 = load i64, ptr %72, align 8, !tbaa !14
  %74 = add nsw i64 %73, -1
  store i64 %74, ptr %72, align 8, !tbaa !14
  %75 = load i64, ptr %0, align 8, !tbaa !3
  %76 = icmp slt i64 %71, %75
  br i1 %76, label %69, label %80, !llvm.loop !21

77:                                               ; preds = %46, %49, %64
  %78 = phi ptr [ %23, %49 ], [ %65, %64 ], [ %23, %46 ]
  %79 = icmp eq ptr %78, null
  br i1 %79, label %132, label %80

80:                                               ; preds = %69, %35, %31, %25, %77
  %81 = phi ptr [ %78, %77 ], [ %23, %35 ], [ %23, %31 ], [ %23, %25 ], [ %65, %69 ]
  %82 = load ptr, ptr %26, align 8, !tbaa !13
  %83 = icmp eq ptr %82, null
  %84 = icmp slt i32 %3, 0
  %85 = or i1 %84, %83
  br i1 %85, label %132, label %86

86:                                               ; preds = %80
  %87 = zext nneg i32 %3 to i64
  %88 = load i64, ptr %0, align 8, !tbaa !3
  %89 = icmp sgt i64 %88, %87
  br i1 %89, label %90, label %132

90:                                               ; preds = %86
  %91 = getelementptr inbounds nuw i64, ptr %81, i64 %87
  %92 = load i64, ptr %91, align 8, !tbaa !14
  %93 = getelementptr inbounds nuw i8, ptr %91, i64 8
  %94 = load i64, ptr %93, align 8, !tbaa !14
  %95 = icmp slt i64 %92, %94
  br i1 %95, label %96, label %132

96:                                               ; preds = %90, %101
  %97 = phi i64 [ %102, %101 ], [ %92, %90 ]
  %98 = getelementptr inbounds i32, ptr %82, i64 %97
  %99 = load i32, ptr %98, align 4, !tbaa !15
  %100 = icmp eq i32 %99, %2
  br i1 %100, label %104, label %101

101:                                              ; preds = %96
  %102 = add i64 %97, 1
  %103 = icmp eq i64 %102, %94
  br i1 %103, label %132, label %96, !llvm.loop !20

104:                                              ; preds = %96
  %105 = getelementptr inbounds i32, ptr %82, i64 %97
  %106 = icmp slt i64 %97, 0
  br i1 %106, label %132, label %107

107:                                              ; preds = %104
  %108 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %109 = load i64, ptr %108, align 8, !tbaa !11
  %110 = add nuw nsw i64 %97, 1
  %111 = icmp slt i64 %110, %109
  br i1 %111, label %112, label %118

112:                                              ; preds = %107
  %113 = getelementptr inbounds nuw i32, ptr %82, i64 %110
  %114 = xor i64 %97, -1
  %115 = add nsw i64 %109, %114
  %116 = shl i64 %115, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %105, ptr nonnull align 4 %113, i64 %116, i1 false)
  %117 = load i64, ptr %0, align 8, !tbaa !3
  br label %118

118:                                              ; preds = %112, %107
  %119 = phi i64 [ %117, %112 ], [ %88, %107 ]
  %120 = add nsw i64 %109, -1
  store i64 %120, ptr %108, align 8, !tbaa !11
  %121 = icmp sgt i64 %119, %87
  br i1 %121, label %122, label %132

122:                                              ; preds = %118
  %123 = load ptr, ptr %22, align 8, !tbaa !12
  br label %124

124:                                              ; preds = %124, %122
  %125 = phi i64 [ %87, %122 ], [ %126, %124 ]
  %126 = add nuw nsw i64 %125, 1
  %127 = getelementptr inbounds nuw i64, ptr %123, i64 %126
  %128 = load i64, ptr %127, align 8, !tbaa !14
  %129 = add nsw i64 %128, -1
  store i64 %129, ptr %127, align 8, !tbaa !14
  %130 = load i64, ptr %0, align 8, !tbaa !3
  %131 = icmp slt i64 %126, %130
  br i1 %131, label %124, label %132, !llvm.loop !21

132:                                              ; preds = %101, %124, %21, %77, %80, %86, %90, %104, %118
  br i1 %20, label %217, label %133

133:                                              ; preds = %132
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  %134 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %135 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  %136 = load i64, ptr %0, align 8, !tbaa !3
  %137 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %138 = load i64, ptr %137, align 8, !tbaa !11
  %139 = load ptr, ptr %22, align 8, !tbaa !12
  %140 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %141 = load ptr, ptr %140, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %136, i64 noundef %138, ptr noundef %139, ptr noundef %141, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 0) #4
  br label %217

142:                                              ; preds = %19
  %143 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %144 = load ptr, ptr %143, align 8, !tbaa !12
  %145 = icmp eq ptr %144, null
  br i1 %145, label %204, label %146

146:                                              ; preds = %142
  %147 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %148 = load ptr, ptr %147, align 8, !tbaa !13
  %149 = icmp eq ptr %148, null
  %150 = icmp slt i32 %2, 0
  %151 = or i1 %150, %149
  br i1 %151, label %177, label %152

152:                                              ; preds = %146
  %153 = zext nneg i32 %2 to i64
  %154 = load i64, ptr %0, align 8, !tbaa !3
  %155 = icmp sgt i64 %154, %153
  %156 = icmp sgt i32 %3, -1
  %157 = and i1 %156, %155
  %158 = zext nneg i32 %3 to i64
  %159 = icmp sgt i64 %154, %158
  %160 = select i1 %157, i1 %159, i1 false
  br i1 %160, label %161, label %177

161:                                              ; preds = %152
  %162 = getelementptr inbounds nuw i64, ptr %144, i64 %153
  %163 = load i64, ptr %162, align 8, !tbaa !14
  %164 = getelementptr inbounds nuw i8, ptr %162, i64 8
  %165 = load i64, ptr %164, align 8, !tbaa !14
  %166 = icmp slt i64 %163, %165
  br i1 %166, label %167, label %177

167:                                              ; preds = %161, %174
  %168 = phi i64 [ %175, %174 ], [ %163, %161 ]
  %169 = getelementptr inbounds i32, ptr %148, i64 %168
  %170 = load i32, ptr %169, align 4, !tbaa !15
  %171 = icmp eq i32 %170, %3
  br i1 %171, label %172, label %174

172:                                              ; preds = %167
  %173 = getelementptr inbounds i32, ptr %148, i64 %168
  store i32 -1, ptr %173, align 4, !tbaa !15
  br label %177

174:                                              ; preds = %167
  %175 = add i64 %168, 1
  %176 = icmp eq i64 %175, %165
  br i1 %176, label %177, label %167, !llvm.loop !22

177:                                              ; preds = %174, %172, %161, %152, %146
  %178 = icmp slt i32 %3, 0
  %179 = or i1 %178, %149
  br i1 %179, label %204, label %180

180:                                              ; preds = %177
  %181 = zext nneg i32 %3 to i64
  %182 = load i64, ptr %0, align 8, !tbaa !3
  %183 = icmp sgt i64 %182, %181
  %184 = and i1 %9, %183
  %185 = zext nneg i32 %2 to i64
  %186 = icmp sgt i64 %182, %185
  %187 = select i1 %184, i1 %186, i1 false
  br i1 %187, label %188, label %204

188:                                              ; preds = %180
  %189 = getelementptr inbounds nuw i64, ptr %144, i64 %181
  %190 = load i64, ptr %189, align 8, !tbaa !14
  %191 = getelementptr inbounds nuw i8, ptr %189, i64 8
  %192 = load i64, ptr %191, align 8, !tbaa !14
  %193 = icmp slt i64 %190, %192
  br i1 %193, label %194, label %204

194:                                              ; preds = %188, %201
  %195 = phi i64 [ %202, %201 ], [ %190, %188 ]
  %196 = getelementptr inbounds i32, ptr %148, i64 %195
  %197 = load i32, ptr %196, align 4, !tbaa !15
  %198 = icmp eq i32 %197, %2
  br i1 %198, label %199, label %201

199:                                              ; preds = %194
  %200 = getelementptr inbounds i32, ptr %148, i64 %195
  store i32 -1, ptr %200, align 4, !tbaa !15
  br label %204

201:                                              ; preds = %194
  %202 = add i64 %195, 1
  %203 = icmp eq i64 %202, %192
  br i1 %203, label %204, label %194, !llvm.loop !22

204:                                              ; preds = %201, %142, %177, %180, %188, %199
  br i1 %20, label %217, label %205

205:                                              ; preds = %204
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  %206 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %207 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  %208 = load i64, ptr %0, align 8, !tbaa !3
  %209 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %210 = load i64, ptr %209, align 8, !tbaa !11
  %211 = load ptr, ptr %143, align 8, !tbaa !12
  %212 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %213 = load ptr, ptr %212, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %208, i64 noundef %210, ptr noundef %211, ptr noundef %213, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 1) #4
  br label %217

214:                                              ; preds = %19
  %215 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %216 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  br label %217

217:                                              ; preds = %214, %133, %132, %205, %204, %5
  ret void
}

declare dso_local i32 @autograph_canonical_remove_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #2

declare dso_local void @autograph_update_csr_pointers(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nounwind }
attributes #5 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"", !5, i64 0, !5, i64 8, !8, i64 16, !10, i64 24}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"p1 long", !9, i64 0}
!9 = !{!"any pointer", !6, i64 0}
!10 = !{!"p1 int", !9, i64 0}
!11 = !{!4, !5, i64 8}
!12 = !{!4, !8, i64 16}
!13 = !{!4, !10, i64 24}
!14 = !{!5, !5, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !6, i64 0}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.mustprogress"}
!19 = distinct !{!19, !18}
!20 = distinct !{!20, !18}
!21 = distinct !{!21, !18}
!22 = distinct !{!22, !18}
