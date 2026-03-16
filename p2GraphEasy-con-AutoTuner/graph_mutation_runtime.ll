; ModuleID = 'graph_mutation_runtime.c'
source_filename = "graph_mutation_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_node(ptr noundef %0, ptr nocapture noundef readnone %1, ptr nocapture noundef readnone %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %12, label %6

6:                                                ; preds = %4
  %7 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %8 = icmp eq i32 %7, 3
  br i1 %8, label %10, label %9

9:                                                ; preds = %6
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  br label %10

10:                                               ; preds = %9, %6
  %11 = tail call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #4
  br label %12

12:                                               ; preds = %4, %10
  ret void
}

declare i32 @autograph_get_layout(ptr noundef) local_unnamed_addr #1

declare void @autograph_ensure_layout_set(ptr noundef) local_unnamed_addr #1

declare i32 @autograph_canonical_add_node(ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_node(ptr noundef %0, ptr nocapture noundef readnone %1, ptr nocapture noundef readnone %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %12, label %6

6:                                                ; preds = %4
  %7 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %8 = icmp eq i32 %7, 3
  br i1 %8, label %10, label %9

9:                                                ; preds = %6
  tail call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #4
  br label %10

10:                                               ; preds = %9, %6
  %11 = tail call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #4
  br label %12

12:                                               ; preds = %4, %10
  ret void
}

declare i32 @autograph_canonical_remove_node(ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_edge(ptr noundef %0, ptr nocapture noundef readnone %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %94, label %7

7:                                                ; preds = %5
  %8 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %9 = icmp sgt i32 %2, -1
  br i1 %9, label %10, label %19

10:                                               ; preds = %7
  %11 = zext nneg i32 %2 to i64
  %12 = load i64, ptr %0, align 8, !tbaa !5
  %13 = icmp sgt i64 %12, %11
  %14 = icmp sgt i32 %3, -1
  %15 = and i1 %14, %13
  br i1 %15, label %16, label %19

16:                                               ; preds = %10
  %17 = zext nneg i32 %3 to i64
  %18 = icmp samesign ule i64 %12, %17
  br label %19

19:                                               ; preds = %16, %10, %7
  %20 = phi i1 [ true, %10 ], [ true, %7 ], [ %18, %16 ]
  %21 = icmp ne i32 %8, 3
  %22 = select i1 %20, i1 %21, i1 false
  br i1 %22, label %94, label %23

23:                                               ; preds = %19
  switch i32 %8, label %91 [
    i32 0, label %24
    i32 1, label %25
  ]

24:                                               ; preds = %23
  tail call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %2, i32 noundef %3)
  tail call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %3, i32 noundef %2)
  br label %94

25:                                               ; preds = %23
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %27 = load ptr, ptr %26, align 8, !tbaa !13
  %28 = icmp eq ptr %27, null
  br i1 %28, label %94, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %31 = load ptr, ptr %30, align 8, !tbaa !14
  %32 = icmp eq ptr %31, null
  %33 = icmp slt i32 %2, 0
  %34 = or i1 %33, %32
  br i1 %34, label %62, label %35

35:                                               ; preds = %29
  %36 = zext nneg i32 %2 to i64
  %37 = load i64, ptr %0, align 8, !tbaa !5
  %38 = icmp sgt i64 %37, %36
  br i1 %38, label %39, label %62

39:                                               ; preds = %35
  %40 = icmp sgt i32 %3, -1
  %41 = zext nneg i32 %3 to i64
  %42 = icmp samesign ugt i64 %37, %41
  %43 = select i1 %40, i1 %42, i1 false
  br i1 %43, label %44, label %62

44:                                               ; preds = %39
  %45 = getelementptr inbounds nuw i64, ptr %27, i64 %36
  %46 = load i64, ptr %45, align 8, !tbaa !15
  %47 = getelementptr inbounds nuw i8, ptr %45, i64 8
  %48 = load i64, ptr %47, align 8, !tbaa !15
  %49 = icmp slt i64 %46, %48
  br i1 %49, label %50, label %62

50:                                               ; preds = %44, %59
  %51 = phi i64 [ %60, %59 ], [ %46, %44 ]
  %52 = getelementptr inbounds i32, ptr %31, i64 %51
  %53 = load i32, ptr %52, align 4, !tbaa !16
  %54 = icmp eq i32 %53, %3
  br i1 %54, label %62, label %55

55:                                               ; preds = %50
  %56 = icmp eq i32 %53, -1
  br i1 %56, label %57, label %59

57:                                               ; preds = %55
  %58 = getelementptr inbounds i32, ptr %31, i64 %51
  store i32 %3, ptr %58, align 4, !tbaa !16
  br label %62

59:                                               ; preds = %55
  %60 = add i64 %51, 1
  %61 = icmp eq i64 %60, %48
  br i1 %61, label %62, label %50, !llvm.loop !18

62:                                               ; preds = %59, %50, %57, %44, %39, %35, %29
  %63 = icmp slt i32 %3, 0
  %64 = or i1 %63, %32
  br i1 %64, label %94, label %65

65:                                               ; preds = %62
  %66 = zext nneg i32 %3 to i64
  %67 = load i64, ptr %0, align 8, !tbaa !5
  %68 = icmp sgt i64 %67, %66
  %69 = zext nneg i32 %2 to i64
  %70 = icmp samesign ugt i64 %67, %69
  %71 = and i1 %9, %68
  %72 = select i1 %71, i1 %70, i1 false
  br i1 %72, label %73, label %94

73:                                               ; preds = %65
  %74 = getelementptr inbounds nuw i64, ptr %27, i64 %66
  %75 = load i64, ptr %74, align 8, !tbaa !15
  %76 = getelementptr inbounds nuw i8, ptr %74, i64 8
  %77 = load i64, ptr %76, align 8, !tbaa !15
  %78 = icmp slt i64 %75, %77
  br i1 %78, label %79, label %94

79:                                               ; preds = %73, %88
  %80 = phi i64 [ %89, %88 ], [ %75, %73 ]
  %81 = getelementptr inbounds i32, ptr %31, i64 %80
  %82 = load i32, ptr %81, align 4, !tbaa !16
  %83 = icmp eq i32 %82, %2
  br i1 %83, label %94, label %84

84:                                               ; preds = %79
  %85 = icmp eq i32 %82, -1
  br i1 %85, label %86, label %88

86:                                               ; preds = %84
  %87 = getelementptr inbounds i32, ptr %31, i64 %80
  store i32 %2, ptr %87, align 4, !tbaa !16
  br label %94

88:                                               ; preds = %84
  %89 = add i64 %80, 1
  %90 = icmp eq i64 %89, %77
  br i1 %90, label %94, label %79, !llvm.loop !18

91:                                               ; preds = %23
  %92 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %93 = tail call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  br label %94

94:                                               ; preds = %88, %79, %25, %86, %73, %65, %62, %19, %91, %24, %5
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @csr_add_directed(ptr nocapture noundef nonnull %0, ptr noundef nonnull %1, i32 noundef %2, i32 noundef %3) unnamed_addr #0 {
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !13
  %7 = icmp eq ptr %6, null
  br i1 %7, label %57, label %8

8:                                                ; preds = %4
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %10 = load ptr, ptr %9, align 8, !tbaa !14
  %11 = icmp eq ptr %10, null
  %12 = icmp slt i32 %2, 0
  %13 = or i1 %12, %11
  br i1 %13, label %57, label %14

14:                                               ; preds = %8
  %15 = zext nneg i32 %2 to i64
  %16 = load i64, ptr %0, align 8, !tbaa !5
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
  %26 = load i64, ptr %25, align 8, !tbaa !15
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %28 = load i64, ptr %27, align 8, !tbaa !20
  %29 = add nsw i64 %28, 1
  %30 = shl i64 %29, 2
  %31 = tail call ptr @realloc(ptr noundef nonnull %10, i64 noundef %30) #5
  %32 = icmp eq ptr %31, null
  br i1 %32, label %57, label %33

33:                                               ; preds = %23
  store ptr %31, ptr %9, align 8, !tbaa !14
  %34 = load ptr, ptr %5, align 8, !tbaa !13
  tail call void @autograph_update_csr_pointers(ptr noundef nonnull %1, ptr noundef %34, ptr noundef nonnull %31) #4
  %35 = icmp slt i64 %26, %28
  br i1 %35, label %36, label %42

36:                                               ; preds = %33
  %37 = load ptr, ptr %9, align 8, !tbaa !14
  %38 = getelementptr i32, ptr %37, i64 %26
  %39 = getelementptr i8, ptr %38, i64 4
  %40 = sub nsw i64 %28, %26
  %41 = shl i64 %40, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %39, ptr align 4 %38, i64 %41, i1 false)
  br label %42

42:                                               ; preds = %36, %33
  %43 = load ptr, ptr %9, align 8, !tbaa !14
  %44 = getelementptr inbounds i32, ptr %43, i64 %26
  store i32 %3, ptr %44, align 4, !tbaa !16
  store i64 %29, ptr %27, align 8, !tbaa !20
  %45 = load i64, ptr %0, align 8, !tbaa !5
  %46 = icmp sgt i64 %45, %15
  br i1 %46, label %47, label %57

47:                                               ; preds = %42
  %48 = load ptr, ptr %5, align 8, !tbaa !13
  br label %49

49:                                               ; preds = %47, %49
  %50 = phi i64 [ %15, %47 ], [ %51, %49 ]
  %51 = add nuw nsw i64 %50, 1
  %52 = getelementptr inbounds nuw i64, ptr %48, i64 %51
  %53 = load i64, ptr %52, align 8, !tbaa !15
  %54 = add nsw i64 %53, 1
  store i64 %54, ptr %52, align 8, !tbaa !15
  %55 = load i64, ptr %0, align 8, !tbaa !5
  %56 = icmp slt i64 %51, %55
  br i1 %56, label %49, label %57, !llvm.loop !21

57:                                               ; preds = %49, %42, %23, %14, %4, %8
  ret void
}

declare i32 @autograph_canonical_add_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_edge(ptr noundef %0, ptr nocapture noundef readnone %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %200, label %7

7:                                                ; preds = %5
  %8 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %9 = icmp sgt i32 %2, -1
  br i1 %9, label %10, label %19

10:                                               ; preds = %7
  %11 = zext nneg i32 %2 to i64
  %12 = load i64, ptr %0, align 8, !tbaa !5
  %13 = icmp sgt i64 %12, %11
  %14 = icmp sgt i32 %3, -1
  %15 = and i1 %14, %13
  br i1 %15, label %16, label %19

16:                                               ; preds = %10
  %17 = zext nneg i32 %3 to i64
  %18 = icmp samesign ule i64 %12, %17
  br label %19

19:                                               ; preds = %16, %10, %7
  %20 = phi i1 [ true, %10 ], [ true, %7 ], [ %18, %16 ]
  %21 = icmp ne i32 %8, 3
  %22 = select i1 %20, i1 %21, i1 false
  br i1 %22, label %200, label %23

23:                                               ; preds = %19
  switch i32 %8, label %197 [
    i32 0, label %24
    i32 1, label %135
  ]

24:                                               ; preds = %23
  %25 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %26 = load ptr, ptr %25, align 8, !tbaa !13
  %27 = icmp eq ptr %26, null
  br i1 %27, label %200, label %28

28:                                               ; preds = %24
  %29 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %30 = load ptr, ptr %29, align 8, !tbaa !14
  %31 = icmp eq ptr %30, null
  %32 = icmp slt i32 %2, 0
  %33 = or i1 %32, %31
  br i1 %33, label %83, label %34

34:                                               ; preds = %28
  %35 = zext nneg i32 %2 to i64
  %36 = load i64, ptr %0, align 8, !tbaa !5
  %37 = icmp sgt i64 %36, %35
  br i1 %37, label %38, label %83

38:                                               ; preds = %34
  %39 = getelementptr inbounds nuw i64, ptr %26, i64 %35
  %40 = load i64, ptr %39, align 8, !tbaa !15
  %41 = getelementptr inbounds nuw i8, ptr %39, i64 8
  %42 = load i64, ptr %41, align 8, !tbaa !15
  %43 = icmp slt i64 %40, %42
  br i1 %43, label %44, label %83

44:                                               ; preds = %38, %49
  %45 = phi i64 [ %50, %49 ], [ %40, %38 ]
  %46 = getelementptr inbounds i32, ptr %30, i64 %45
  %47 = load i32, ptr %46, align 4, !tbaa !16
  %48 = icmp eq i32 %47, %3
  br i1 %48, label %52, label %49

49:                                               ; preds = %44
  %50 = add i64 %45, 1
  %51 = icmp eq i64 %50, %42
  br i1 %51, label %80, label %44, !llvm.loop !22

52:                                               ; preds = %44
  %53 = getelementptr inbounds i32, ptr %30, i64 %45
  %54 = icmp slt i64 %45, 0
  br i1 %54, label %80, label %55

55:                                               ; preds = %52
  %56 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %57 = load i64, ptr %56, align 8, !tbaa !20
  %58 = add nuw nsw i64 %45, 1
  %59 = icmp slt i64 %58, %57
  br i1 %59, label %60, label %67

60:                                               ; preds = %55
  %61 = getelementptr inbounds nuw i32, ptr %30, i64 %58
  %62 = xor i64 %45, -1
  %63 = add nsw i64 %57, %62
  %64 = shl i64 %63, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %53, ptr nonnull align 4 %61, i64 %64, i1 false)
  %65 = load i64, ptr %0, align 8, !tbaa !5
  %66 = load ptr, ptr %25, align 8, !tbaa !13
  br label %67

67:                                               ; preds = %60, %55
  %68 = phi ptr [ %66, %60 ], [ %26, %55 ]
  %69 = phi i64 [ %65, %60 ], [ %36, %55 ]
  %70 = add nsw i64 %57, -1
  store i64 %70, ptr %56, align 8, !tbaa !20
  %71 = icmp sgt i64 %69, %35
  br i1 %71, label %72, label %80

72:                                               ; preds = %67, %72
  %73 = phi i64 [ %74, %72 ], [ %35, %67 ]
  %74 = add nuw nsw i64 %73, 1
  %75 = getelementptr inbounds nuw i64, ptr %68, i64 %74
  %76 = load i64, ptr %75, align 8, !tbaa !15
  %77 = add nsw i64 %76, -1
  store i64 %77, ptr %75, align 8, !tbaa !15
  %78 = load i64, ptr %0, align 8, !tbaa !5
  %79 = icmp slt i64 %74, %78
  br i1 %79, label %72, label %83, !llvm.loop !23

80:                                               ; preds = %49, %52, %67
  %81 = phi ptr [ %26, %52 ], [ %68, %67 ], [ %26, %49 ]
  %82 = icmp eq ptr %81, null
  br i1 %82, label %200, label %83

83:                                               ; preds = %72, %38, %34, %28, %80
  %84 = phi ptr [ %81, %80 ], [ %26, %38 ], [ %26, %34 ], [ %26, %28 ], [ %68, %72 ]
  %85 = load ptr, ptr %29, align 8, !tbaa !14
  %86 = icmp eq ptr %85, null
  %87 = icmp slt i32 %3, 0
  %88 = or i1 %87, %86
  br i1 %88, label %200, label %89

89:                                               ; preds = %83
  %90 = zext nneg i32 %3 to i64
  %91 = load i64, ptr %0, align 8, !tbaa !5
  %92 = icmp sgt i64 %91, %90
  br i1 %92, label %93, label %200

93:                                               ; preds = %89
  %94 = getelementptr inbounds nuw i64, ptr %84, i64 %90
  %95 = load i64, ptr %94, align 8, !tbaa !15
  %96 = getelementptr inbounds nuw i8, ptr %94, i64 8
  %97 = load i64, ptr %96, align 8, !tbaa !15
  %98 = icmp slt i64 %95, %97
  br i1 %98, label %99, label %200

99:                                               ; preds = %93, %104
  %100 = phi i64 [ %105, %104 ], [ %95, %93 ]
  %101 = getelementptr inbounds i32, ptr %85, i64 %100
  %102 = load i32, ptr %101, align 4, !tbaa !16
  %103 = icmp eq i32 %102, %2
  br i1 %103, label %107, label %104

104:                                              ; preds = %99
  %105 = add i64 %100, 1
  %106 = icmp eq i64 %105, %97
  br i1 %106, label %200, label %99, !llvm.loop !22

107:                                              ; preds = %99
  %108 = getelementptr inbounds i32, ptr %85, i64 %100
  %109 = icmp slt i64 %100, 0
  br i1 %109, label %200, label %110

110:                                              ; preds = %107
  %111 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %112 = load i64, ptr %111, align 8, !tbaa !20
  %113 = add nuw nsw i64 %100, 1
  %114 = icmp slt i64 %113, %112
  br i1 %114, label %115, label %121

115:                                              ; preds = %110
  %116 = getelementptr inbounds nuw i32, ptr %85, i64 %113
  %117 = xor i64 %100, -1
  %118 = add nsw i64 %112, %117
  %119 = shl i64 %118, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %108, ptr nonnull align 4 %116, i64 %119, i1 false)
  %120 = load i64, ptr %0, align 8, !tbaa !5
  br label %121

121:                                              ; preds = %115, %110
  %122 = phi i64 [ %120, %115 ], [ %91, %110 ]
  %123 = add nsw i64 %112, -1
  store i64 %123, ptr %111, align 8, !tbaa !20
  %124 = icmp sgt i64 %122, %90
  br i1 %124, label %125, label %200

125:                                              ; preds = %121
  %126 = load ptr, ptr %25, align 8, !tbaa !13
  br label %127

127:                                              ; preds = %127, %125
  %128 = phi i64 [ %90, %125 ], [ %129, %127 ]
  %129 = add nuw nsw i64 %128, 1
  %130 = getelementptr inbounds nuw i64, ptr %126, i64 %129
  %131 = load i64, ptr %130, align 8, !tbaa !15
  %132 = add nsw i64 %131, -1
  store i64 %132, ptr %130, align 8, !tbaa !15
  %133 = load i64, ptr %0, align 8, !tbaa !5
  %134 = icmp slt i64 %129, %133
  br i1 %134, label %127, label %200, !llvm.loop !23

135:                                              ; preds = %23
  %136 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %137 = load ptr, ptr %136, align 8, !tbaa !13
  %138 = icmp eq ptr %137, null
  br i1 %138, label %200, label %139

139:                                              ; preds = %135
  %140 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %141 = load ptr, ptr %140, align 8, !tbaa !14
  %142 = icmp eq ptr %141, null
  %143 = icmp slt i32 %2, 0
  %144 = or i1 %143, %142
  br i1 %144, label %170, label %145

145:                                              ; preds = %139
  %146 = zext nneg i32 %2 to i64
  %147 = load i64, ptr %0, align 8, !tbaa !5
  %148 = icmp sgt i64 %147, %146
  %149 = icmp sgt i32 %3, -1
  %150 = and i1 %149, %148
  %151 = zext nneg i32 %3 to i64
  %152 = icmp sgt i64 %147, %151
  %153 = select i1 %150, i1 %152, i1 false
  br i1 %153, label %154, label %170

154:                                              ; preds = %145
  %155 = getelementptr inbounds nuw i64, ptr %137, i64 %146
  %156 = load i64, ptr %155, align 8, !tbaa !15
  %157 = getelementptr inbounds nuw i8, ptr %155, i64 8
  %158 = load i64, ptr %157, align 8, !tbaa !15
  %159 = icmp slt i64 %156, %158
  br i1 %159, label %160, label %170

160:                                              ; preds = %154, %167
  %161 = phi i64 [ %168, %167 ], [ %156, %154 ]
  %162 = getelementptr inbounds i32, ptr %141, i64 %161
  %163 = load i32, ptr %162, align 4, !tbaa !16
  %164 = icmp eq i32 %163, %3
  br i1 %164, label %165, label %167

165:                                              ; preds = %160
  %166 = getelementptr inbounds i32, ptr %141, i64 %161
  store i32 -1, ptr %166, align 4, !tbaa !16
  br label %170

167:                                              ; preds = %160
  %168 = add i64 %161, 1
  %169 = icmp eq i64 %168, %158
  br i1 %169, label %170, label %160, !llvm.loop !24

170:                                              ; preds = %167, %165, %154, %145, %139
  %171 = icmp slt i32 %3, 0
  %172 = or i1 %171, %142
  br i1 %172, label %200, label %173

173:                                              ; preds = %170
  %174 = zext nneg i32 %3 to i64
  %175 = load i64, ptr %0, align 8, !tbaa !5
  %176 = icmp sgt i64 %175, %174
  %177 = and i1 %9, %176
  %178 = zext nneg i32 %2 to i64
  %179 = icmp sgt i64 %175, %178
  %180 = select i1 %177, i1 %179, i1 false
  br i1 %180, label %181, label %200

181:                                              ; preds = %173
  %182 = getelementptr inbounds nuw i64, ptr %137, i64 %174
  %183 = load i64, ptr %182, align 8, !tbaa !15
  %184 = getelementptr inbounds nuw i8, ptr %182, i64 8
  %185 = load i64, ptr %184, align 8, !tbaa !15
  %186 = icmp slt i64 %183, %185
  br i1 %186, label %187, label %200

187:                                              ; preds = %181, %194
  %188 = phi i64 [ %195, %194 ], [ %183, %181 ]
  %189 = getelementptr inbounds i32, ptr %141, i64 %188
  %190 = load i32, ptr %189, align 4, !tbaa !16
  %191 = icmp eq i32 %190, %2
  br i1 %191, label %192, label %194

192:                                              ; preds = %187
  %193 = getelementptr inbounds i32, ptr %141, i64 %188
  store i32 -1, ptr %193, align 4, !tbaa !16
  br label %200

194:                                              ; preds = %187
  %195 = add i64 %188, 1
  %196 = icmp eq i64 %195, %185
  br i1 %196, label %200, label %187, !llvm.loop !24

197:                                              ; preds = %23
  %198 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #4
  %199 = tail call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #4
  br label %200

200:                                              ; preds = %194, %104, %127, %135, %24, %192, %181, %173, %170, %121, %107, %93, %89, %83, %80, %19, %197, %5
  ret void
}

declare i32 @autograph_canonical_remove_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #2

declare void @autograph_update_csr_pointers(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nounwind }
attributes #5 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.1.8 (https://github.com/llvm/llvm-project.git 87f0227cb60147a26a1eeb4fb06e3b505e9c7261)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"", !7, i64 0, !7, i64 8, !10, i64 16, !12, i64 24}
!7 = !{!"long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"p1 long", !11, i64 0}
!11 = !{!"any pointer", !8, i64 0}
!12 = !{!"p1 int", !11, i64 0}
!13 = !{!6, !10, i64 16}
!14 = !{!6, !12, i64 24}
!15 = !{!7, !7, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !8, i64 0}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.mustprogress"}
!20 = !{!6, !7, i64 8}
!21 = distinct !{!21, !19}
!22 = distinct !{!22, !19}
!23 = distinct !{!23, !19}
!24 = distinct !{!24, !19}
