; ModuleID = 'graph_mutation_runtime.c'
source_filename = "graph_mutation_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_node(ptr noundef %0, ptr nocapture noundef readnone %1, ptr nocapture noundef readnone %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  %6 = icmp slt i32 %3, 0
  %7 = or i1 %5, %6
  br i1 %7, label %50, label %8

8:                                                ; preds = %4
  %9 = zext nneg i32 %3 to i64
  %10 = load i64, ptr %0, align 8, !tbaa !3
  %11 = icmp sgt i64 %10, %9
  br i1 %11, label %49, label %12

12:                                               ; preds = %8
  %13 = add nuw nsw i64 %9, 1
  %14 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %15 = load ptr, ptr %14, align 8, !tbaa !11
  %16 = shl nuw nsw i64 %9, 3
  %17 = add nuw nsw i64 %16, 16
  %18 = tail call ptr @realloc(ptr noundef %15, i64 noundef %17) #5
  %19 = icmp eq ptr %18, null
  br i1 %19, label %50, label %20

20:                                               ; preds = %12
  store ptr %18, ptr %14, align 8, !tbaa !11
  %21 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %22 = load i64, ptr %21, align 8, !tbaa !12
  %23 = add nuw nsw i64 %9, 1
  %24 = sub i64 %23, %10
  %25 = icmp ult i64 %24, 4
  br i1 %25, label %41, label %26

26:                                               ; preds = %20
  %27 = and i64 %24, -4
  %28 = add i64 %10, %27
  %29 = insertelement <2 x i64> poison, i64 %22, i64 0
  %30 = shufflevector <2 x i64> %29, <2 x i64> poison, <2 x i32> zeroinitializer
  %31 = getelementptr i64, ptr %18, i64 %10
  br label %32

32:                                               ; preds = %32, %26
  %33 = phi i64 [ 0, %26 ], [ %37, %32 ]
  %34 = getelementptr i64, ptr %31, i64 %33
  %35 = getelementptr i8, ptr %34, i64 8
  %36 = getelementptr i8, ptr %34, i64 24
  store <2 x i64> %30, ptr %35, align 8, !tbaa !13
  store <2 x i64> %30, ptr %36, align 8, !tbaa !13
  %37 = add nuw i64 %33, 4
  %38 = icmp eq i64 %37, %27
  br i1 %38, label %39, label %32, !llvm.loop !14

39:                                               ; preds = %32
  %40 = icmp eq i64 %24, %27
  br i1 %40, label %43, label %41

41:                                               ; preds = %20, %39
  %42 = phi i64 [ %10, %20 ], [ %28, %39 ]
  br label %44

43:                                               ; preds = %44, %39
  store i64 %13, ptr %0, align 8, !tbaa !3
  br label %49

44:                                               ; preds = %41, %44
  %45 = phi i64 [ %46, %44 ], [ %42, %41 ]
  %46 = add nsw i64 %45, 1
  %47 = getelementptr inbounds i64, ptr %18, i64 %46
  store i64 %22, ptr %47, align 8, !tbaa !13
  %48 = icmp eq i64 %45, %9
  br i1 %48, label %43, label %44, !llvm.loop !18

49:                                               ; preds = %43, %8
  tail call void @graph_register_csr_metadata(ptr noundef nonnull %0) #6
  br label %50

50:                                               ; preds = %12, %4, %49
  ret void
}

declare dso_local void @graph_register_csr_metadata(ptr noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_node(ptr noundef %0, ptr nocapture noundef readnone %1, ptr nocapture noundef readnone %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  %6 = icmp slt i32 %3, 0
  %7 = or i1 %5, %6
  br i1 %7, label %52, label %8

8:                                                ; preds = %4
  %9 = zext nneg i32 %3 to i64
  %10 = load i64, ptr %0, align 8, !tbaa !3
  %11 = icmp sgt i64 %10, %9
  br i1 %11, label %22, label %52

12:                                               ; preds = %29
  %13 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %14 = load ptr, ptr %13, align 8, !tbaa !11
  %15 = getelementptr inbounds nuw i64, ptr %14, i64 %9
  %16 = load i64, ptr %15, align 8, !tbaa !13
  %17 = getelementptr inbounds nuw i8, ptr %15, i64 8
  %18 = load i64, ptr %17, align 8, !tbaa !13
  %19 = icmp slt i64 %16, %18
  br i1 %19, label %20, label %51

20:                                               ; preds = %12
  %21 = getelementptr inbounds nuw i8, ptr %0, i64 24
  br label %33

22:                                               ; preds = %8, %29
  %23 = phi i64 [ %30, %29 ], [ %10, %8 ]
  %24 = phi i64 [ %31, %29 ], [ 0, %8 ]
  %25 = trunc i64 %24 to i32
  %26 = icmp eq i32 %3, %25
  br i1 %26, label %29, label %27

27:                                               ; preds = %22
  tail call fastcc void @remove_directed(ptr noundef %0, i32 noundef %25, i32 noundef %3)
  %28 = load i64, ptr %0, align 8, !tbaa !3
  br label %29

29:                                               ; preds = %22, %27
  %30 = phi i64 [ %23, %22 ], [ %28, %27 ]
  %31 = add nuw nsw i64 %24, 1
  %32 = icmp slt i64 %31, %30
  br i1 %32, label %22, label %12, !llvm.loop !19

33:                                               ; preds = %20, %44
  %34 = phi i64 [ %16, %20 ], [ %47, %44 ]
  %35 = load ptr, ptr %21, align 8, !tbaa !20
  %36 = getelementptr inbounds i32, ptr %35, i64 %34
  %37 = load i32, ptr %36, align 4, !tbaa !21
  tail call fastcc void @remove_directed(ptr noundef %0, i32 noundef %3, i32 noundef %37)
  %38 = icmp sgt i32 %37, -1
  br i1 %38, label %39, label %44

39:                                               ; preds = %33
  %40 = zext nneg i32 %37 to i64
  %41 = load i64, ptr %0, align 8, !tbaa !3
  %42 = icmp sgt i64 %41, %40
  br i1 %42, label %43, label %44

43:                                               ; preds = %39
  tail call fastcc void @remove_directed(ptr noundef %0, i32 noundef %37, i32 noundef %3)
  br label %44

44:                                               ; preds = %43, %39, %33
  %45 = load ptr, ptr %13, align 8, !tbaa !11
  %46 = getelementptr inbounds nuw i64, ptr %45, i64 %9
  %47 = load i64, ptr %46, align 8, !tbaa !13
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 8
  %49 = load i64, ptr %48, align 8, !tbaa !13
  %50 = icmp slt i64 %47, %49
  br i1 %50, label %33, label %51, !llvm.loop !23

51:                                               ; preds = %44, %12
  tail call void @graph_register_csr_metadata(ptr noundef nonnull %0) #6
  br label %52

52:                                               ; preds = %4, %8, %51
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @remove_directed(ptr nocapture noundef nonnull %0, i32 noundef %1, i32 noundef %2) unnamed_addr #0 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %5 = load ptr, ptr %4, align 8, !tbaa !11
  %6 = icmp eq ptr %5, null
  br i1 %6, label %93, label %7

7:                                                ; preds = %3
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %9 = load ptr, ptr %8, align 8, !tbaa !20
  %10 = icmp eq ptr %9, null
  %11 = icmp slt i32 %1, 0
  %12 = or i1 %11, %10
  br i1 %12, label %93, label %13

13:                                               ; preds = %7
  %14 = zext nneg i32 %1 to i64
  %15 = load i64, ptr %0, align 8, !tbaa !3
  %16 = icmp sgt i64 %15, %14
  %17 = icmp sgt i32 %2, -1
  %18 = and i1 %17, %16
  %19 = zext nneg i32 %2 to i64
  %20 = icmp sgt i64 %15, %19
  %21 = select i1 %18, i1 %20, i1 false
  br i1 %21, label %22, label %93

22:                                               ; preds = %13
  %23 = getelementptr inbounds nuw i64, ptr %5, i64 %14
  %24 = load i64, ptr %23, align 8, !tbaa !13
  %25 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %26 = load i64, ptr %25, align 8, !tbaa !13
  %27 = icmp slt i64 %24, %26
  br i1 %27, label %28, label %93

28:                                               ; preds = %22, %33
  %29 = phi i64 [ %34, %33 ], [ %24, %22 ]
  %30 = getelementptr inbounds i32, ptr %9, i64 %29
  %31 = load i32, ptr %30, align 4, !tbaa !21
  %32 = icmp eq i32 %31, %2
  br i1 %32, label %36, label %33

33:                                               ; preds = %28
  %34 = add i64 %29, 1
  %35 = icmp eq i64 %34, %26
  br i1 %35, label %93, label %28, !llvm.loop !24

36:                                               ; preds = %28
  %37 = getelementptr inbounds i32, ptr %9, i64 %29
  %38 = icmp slt i64 %29, 0
  br i1 %38, label %93, label %39

39:                                               ; preds = %36
  %40 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %41 = load i64, ptr %40, align 8, !tbaa !12
  %42 = add nuw nsw i64 %29, 1
  %43 = icmp slt i64 %42, %41
  br i1 %43, label %44, label %55

44:                                               ; preds = %39
  %45 = getelementptr inbounds nuw i32, ptr %9, i64 %42
  %46 = xor i64 %29, -1
  %47 = add nsw i64 %41, %46
  %48 = shl i64 %47, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %37, ptr nonnull align 4 %45, i64 %48, i1 false)
  %49 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %50 = load ptr, ptr %49, align 8, !tbaa !25
  %51 = icmp eq ptr %50, null
  br i1 %51, label %55, label %52

52:                                               ; preds = %44
  %53 = getelementptr inbounds nuw i32, ptr %50, i64 %29
  %54 = getelementptr inbounds nuw i32, ptr %50, i64 %42
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %53, ptr nonnull align 4 %54, i64 %48, i1 false)
  br label %55

55:                                               ; preds = %44, %52, %39
  %56 = add nsw i64 %41, -1
  store i64 %56, ptr %40, align 8, !tbaa !12
  %57 = load i64, ptr %0, align 8, !tbaa !3
  %58 = icmp sgt i64 %57, %14
  br i1 %58, label %59, label %63

59:                                               ; preds = %55
  %60 = load ptr, ptr %4, align 8, !tbaa !11
  br label %67

61:                                               ; preds = %67
  %62 = load i64, ptr %40, align 8, !tbaa !12
  br label %63

63:                                               ; preds = %61, %55
  %64 = phi i64 [ %62, %61 ], [ %56, %55 ]
  %65 = icmp eq i64 %64, 0
  %66 = load ptr, ptr %8, align 8, !tbaa !20
  br i1 %65, label %75, label %78

67:                                               ; preds = %59, %67
  %68 = phi i64 [ %14, %59 ], [ %69, %67 ]
  %69 = add nuw nsw i64 %68, 1
  %70 = getelementptr inbounds nuw i64, ptr %60, i64 %69
  %71 = load i64, ptr %70, align 8, !tbaa !13
  %72 = add nsw i64 %71, -1
  store i64 %72, ptr %70, align 8, !tbaa !13
  %73 = load i64, ptr %0, align 8, !tbaa !3
  %74 = icmp slt i64 %69, %73
  br i1 %74, label %67, label %61, !llvm.loop !26

75:                                               ; preds = %63
  tail call void @free(ptr noundef %66) #6
  store ptr null, ptr %8, align 8, !tbaa !20
  %76 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %77 = load ptr, ptr %76, align 8, !tbaa !25
  tail call void @free(ptr noundef %77) #6
  store ptr null, ptr %76, align 8, !tbaa !25
  br label %93

78:                                               ; preds = %63
  %79 = shl i64 %64, 2
  %80 = tail call ptr @realloc(ptr noundef %66, i64 noundef %79) #5
  %81 = icmp eq ptr %80, null
  br i1 %81, label %83, label %82

82:                                               ; preds = %78
  store ptr %80, ptr %8, align 8, !tbaa !20
  br label %83

83:                                               ; preds = %82, %78
  %84 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %85 = load ptr, ptr %84, align 8, !tbaa !25
  %86 = icmp eq ptr %85, null
  br i1 %86, label %93, label %87

87:                                               ; preds = %83
  %88 = load i64, ptr %40, align 8, !tbaa !12
  %89 = shl i64 %88, 2
  %90 = tail call ptr @realloc(ptr noundef nonnull %85, i64 noundef %89) #5
  %91 = icmp eq ptr %90, null
  br i1 %91, label %93, label %92

92:                                               ; preds = %87
  store ptr %90, ptr %84, align 8, !tbaa !25
  br label %93

93:                                               ; preds = %33, %22, %36, %83, %92, %87, %75, %13, %3, %7
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_edge(ptr noundef %0, ptr nocapture noundef readnone %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %12, label %7

7:                                                ; preds = %5
  %8 = tail call fastcc i32 @insert_directed(ptr noundef %0, i32 noundef %2, i32 noundef %3)
  %9 = icmp eq i32 %8, 0
  br i1 %9, label %12, label %10

10:                                               ; preds = %7
  %11 = tail call fastcc i32 @insert_directed(ptr noundef %0, i32 noundef %3, i32 noundef %2)
  tail call void @graph_register_csr_metadata(ptr noundef nonnull %0) #6
  br label %12

12:                                               ; preds = %7, %5, %10
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc range(i32 0, 2) i32 @insert_directed(ptr nocapture noundef nonnull %0, i32 noundef %1, i32 noundef %2) unnamed_addr #0 {
  %4 = icmp slt i32 %1, 0
  br i1 %4, label %177, label %5

5:                                                ; preds = %3
  %6 = zext nneg i32 %1 to i64
  %7 = load i64, ptr %0, align 8, !tbaa !3
  %8 = icmp sgt i64 %7, %6
  br i1 %8, label %46, label %9

9:                                                ; preds = %5
  %10 = add nuw nsw i64 %6, 1
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %12 = load ptr, ptr %11, align 8, !tbaa !11
  %13 = shl nuw nsw i64 %6, 3
  %14 = add nuw nsw i64 %13, 16
  %15 = tail call ptr @realloc(ptr noundef %12, i64 noundef %14) #5
  %16 = icmp eq ptr %15, null
  br i1 %16, label %177, label %17

17:                                               ; preds = %9
  store ptr %15, ptr %11, align 8, !tbaa !11
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %19 = load i64, ptr %18, align 8, !tbaa !12
  %20 = add nuw nsw i64 %6, 1
  %21 = sub i64 %20, %7
  %22 = icmp ult i64 %21, 4
  br i1 %22, label %38, label %23

23:                                               ; preds = %17
  %24 = and i64 %21, -4
  %25 = add i64 %7, %24
  %26 = insertelement <2 x i64> poison, i64 %19, i64 0
  %27 = shufflevector <2 x i64> %26, <2 x i64> poison, <2 x i32> zeroinitializer
  %28 = getelementptr i64, ptr %15, i64 %7
  br label %29

29:                                               ; preds = %29, %23
  %30 = phi i64 [ 0, %23 ], [ %34, %29 ]
  %31 = getelementptr i64, ptr %28, i64 %30
  %32 = getelementptr i8, ptr %31, i64 8
  %33 = getelementptr i8, ptr %31, i64 24
  store <2 x i64> %27, ptr %32, align 8, !tbaa !13
  store <2 x i64> %27, ptr %33, align 8, !tbaa !13
  %34 = add nuw i64 %30, 4
  %35 = icmp eq i64 %34, %24
  br i1 %35, label %36, label %29, !llvm.loop !27

36:                                               ; preds = %29
  %37 = icmp eq i64 %21, %24
  br i1 %37, label %40, label %38

38:                                               ; preds = %17, %36
  %39 = phi i64 [ %7, %17 ], [ %25, %36 ]
  br label %41

40:                                               ; preds = %41, %36
  store i64 %10, ptr %0, align 8, !tbaa !3
  br label %46

41:                                               ; preds = %38, %41
  %42 = phi i64 [ %43, %41 ], [ %39, %38 ]
  %43 = add nsw i64 %42, 1
  %44 = getelementptr inbounds i64, ptr %15, i64 %43
  store i64 %19, ptr %44, align 8, !tbaa !13
  %45 = icmp eq i64 %42, %6
  br i1 %45, label %40, label %41, !llvm.loop !28

46:                                               ; preds = %40, %5
  %47 = phi i64 [ %10, %40 ], [ %7, %5 ]
  %48 = icmp slt i32 %2, 0
  br i1 %48, label %177, label %49

49:                                               ; preds = %46
  %50 = zext nneg i32 %2 to i64
  %51 = icmp sgt i64 %47, %50
  br i1 %51, label %89, label %52

52:                                               ; preds = %49
  %53 = add nuw nsw i64 %50, 1
  %54 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %55 = load ptr, ptr %54, align 8, !tbaa !11
  %56 = shl nuw nsw i64 %50, 3
  %57 = add nuw nsw i64 %56, 16
  %58 = tail call ptr @realloc(ptr noundef %55, i64 noundef %57) #5
  %59 = icmp eq ptr %58, null
  br i1 %59, label %177, label %60

60:                                               ; preds = %52
  store ptr %58, ptr %54, align 8, !tbaa !11
  %61 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %62 = load i64, ptr %61, align 8, !tbaa !12
  %63 = add nuw nsw i64 %50, 1
  %64 = sub i64 %63, %47
  %65 = icmp ult i64 %64, 4
  br i1 %65, label %81, label %66

66:                                               ; preds = %60
  %67 = and i64 %64, -4
  %68 = add i64 %47, %67
  %69 = insertelement <2 x i64> poison, i64 %62, i64 0
  %70 = shufflevector <2 x i64> %69, <2 x i64> poison, <2 x i32> zeroinitializer
  %71 = getelementptr i64, ptr %58, i64 %47
  br label %72

72:                                               ; preds = %72, %66
  %73 = phi i64 [ 0, %66 ], [ %77, %72 ]
  %74 = getelementptr i64, ptr %71, i64 %73
  %75 = getelementptr i8, ptr %74, i64 8
  %76 = getelementptr i8, ptr %74, i64 24
  store <2 x i64> %70, ptr %75, align 8, !tbaa !13
  store <2 x i64> %70, ptr %76, align 8, !tbaa !13
  %77 = add nuw i64 %73, 4
  %78 = icmp eq i64 %77, %67
  br i1 %78, label %79, label %72, !llvm.loop !29

79:                                               ; preds = %72
  %80 = icmp eq i64 %64, %67
  br i1 %80, label %83, label %81

81:                                               ; preds = %60, %79
  %82 = phi i64 [ %47, %60 ], [ %68, %79 ]
  br label %84

83:                                               ; preds = %84, %79
  store i64 %53, ptr %0, align 8, !tbaa !3
  br label %92

84:                                               ; preds = %81, %84
  %85 = phi i64 [ %86, %84 ], [ %82, %81 ]
  %86 = add nsw i64 %85, 1
  %87 = getelementptr inbounds i64, ptr %58, i64 %86
  store i64 %62, ptr %87, align 8, !tbaa !13
  %88 = icmp eq i64 %85, %50
  br i1 %88, label %83, label %84, !llvm.loop !30

89:                                               ; preds = %49
  %90 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %91 = load ptr, ptr %90, align 8, !tbaa !11, !nonnull !31, !noundef !31
  br label %92

92:                                               ; preds = %89, %83
  %93 = phi ptr [ %58, %83 ], [ %91, %89 ]
  %94 = phi i64 [ %53, %83 ], [ %47, %89 ]
  %95 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %96 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %97 = load ptr, ptr %96, align 8, !tbaa !20
  %98 = icmp eq ptr %97, null
  br i1 %98, label %115, label %99

99:                                               ; preds = %92
  %100 = icmp sgt i64 %94, %6
  br i1 %100, label %101, label %115

101:                                              ; preds = %99
  %102 = getelementptr inbounds nuw i64, ptr %93, i64 %6
  %103 = load i64, ptr %102, align 8, !tbaa !13
  %104 = getelementptr inbounds nuw i8, ptr %102, i64 8
  %105 = load i64, ptr %104, align 8, !tbaa !13
  %106 = icmp slt i64 %103, %105
  br i1 %106, label %110, label %115

107:                                              ; preds = %110
  %108 = add i64 %111, 1
  %109 = icmp eq i64 %108, %105
  br i1 %109, label %115, label %110, !llvm.loop !32

110:                                              ; preds = %101, %107
  %111 = phi i64 [ %108, %107 ], [ %103, %101 ]
  %112 = getelementptr inbounds i32, ptr %97, i64 %111
  %113 = load i32, ptr %112, align 4, !tbaa !21
  %114 = icmp eq i32 %113, %2
  br i1 %114, label %177, label %107

115:                                              ; preds = %107, %92, %99, %101
  %116 = phi ptr [ null, %92 ], [ %97, %99 ], [ %97, %101 ], [ %97, %107 ]
  %117 = getelementptr i64, ptr %93, i64 %6
  %118 = getelementptr i8, ptr %117, i64 8
  %119 = load i64, ptr %118, align 8, !tbaa !13
  %120 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %121 = load i64, ptr %120, align 8, !tbaa !12
  %122 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %123 = add nsw i64 %121, 1
  %124 = shl i64 %123, 2
  %125 = tail call ptr @realloc(ptr noundef %116, i64 noundef %124) #5
  %126 = icmp eq ptr %125, null
  br i1 %126, label %177, label %127

127:                                              ; preds = %115
  store ptr %125, ptr %122, align 8, !tbaa !20
  %128 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %129 = load ptr, ptr %128, align 8, !tbaa !25
  %130 = icmp eq ptr %129, null
  br i1 %130, label %136, label %131

131:                                              ; preds = %127
  %132 = tail call ptr @realloc(ptr noundef nonnull %129, i64 noundef %124) #5
  %133 = icmp eq ptr %132, null
  br i1 %133, label %177, label %134

134:                                              ; preds = %131
  store ptr %132, ptr %128, align 8, !tbaa !25
  %135 = load ptr, ptr %122, align 8, !tbaa !20
  br label %136

136:                                              ; preds = %134, %127
  %137 = phi ptr [ %135, %134 ], [ %125, %127 ]
  %138 = phi ptr [ %132, %134 ], [ null, %127 ]
  %139 = icmp slt i64 %119, %121
  br i1 %139, label %140, label %155

140:                                              ; preds = %136
  %141 = add nsw i64 %119, 1
  %142 = getelementptr inbounds i32, ptr %137, i64 %141
  %143 = getelementptr inbounds i32, ptr %137, i64 %119
  %144 = sub nsw i64 %121, %119
  %145 = shl i64 %144, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %142, ptr align 4 %143, i64 %145, i1 false)
  %146 = load ptr, ptr %128, align 8, !tbaa !25
  %147 = icmp eq ptr %146, null
  br i1 %147, label %148, label %150

148:                                              ; preds = %140
  %149 = load ptr, ptr %122, align 8, !tbaa !20
  br label %160

150:                                              ; preds = %140
  %151 = getelementptr inbounds i32, ptr %146, i64 %141
  %152 = getelementptr inbounds i32, ptr %146, i64 %119
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %151, ptr nonnull align 4 %152, i64 %145, i1 false)
  %153 = load ptr, ptr %128, align 8, !tbaa !25
  %154 = load ptr, ptr %122, align 8, !tbaa !20
  br label %155

155:                                              ; preds = %150, %136
  %156 = phi ptr [ %154, %150 ], [ %137, %136 ]
  %157 = phi ptr [ %153, %150 ], [ %138, %136 ]
  %158 = getelementptr inbounds i32, ptr %156, i64 %119
  store i32 %2, ptr %158, align 4, !tbaa !21
  %159 = icmp eq ptr %157, null
  br i1 %159, label %164, label %160

160:                                              ; preds = %155, %148
  %161 = phi ptr [ %149, %148 ], [ %157, %155 ]
  %162 = phi i32 [ %2, %148 ], [ 1, %155 ]
  %163 = getelementptr inbounds i32, ptr %161, i64 %119
  store i32 %162, ptr %163, align 4, !tbaa !21
  br label %164

164:                                              ; preds = %160, %155
  store i64 %123, ptr %120, align 8, !tbaa !12
  %165 = load i64, ptr %0, align 8, !tbaa !3
  %166 = icmp sgt i64 %165, %6
  br i1 %166, label %167, label %177

167:                                              ; preds = %164
  %168 = load ptr, ptr %95, align 8, !tbaa !11
  br label %169

169:                                              ; preds = %167, %169
  %170 = phi i64 [ %6, %167 ], [ %171, %169 ]
  %171 = add nuw nsw i64 %170, 1
  %172 = getelementptr inbounds nuw i64, ptr %168, i64 %171
  %173 = load i64, ptr %172, align 8, !tbaa !13
  %174 = add nsw i64 %173, 1
  store i64 %174, ptr %172, align 8, !tbaa !13
  %175 = load i64, ptr %0, align 8, !tbaa !3
  %176 = icmp slt i64 %171, %175
  br i1 %176, label %169, label %177, !llvm.loop !33

177:                                              ; preds = %110, %169, %164, %52, %46, %9, %3, %115, %131
  %178 = phi i32 [ 0, %115 ], [ 0, %131 ], [ 0, %3 ], [ 0, %9 ], [ 0, %46 ], [ 0, %52 ], [ 1, %164 ], [ 1, %169 ], [ 1, %110 ]
  ret i32 %178
}

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_edge(ptr noundef %0, ptr nocapture noundef readnone %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %8, label %7

7:                                                ; preds = %5
  tail call fastcc void @remove_directed(ptr noundef %0, i32 noundef %2, i32 noundef %3)
  tail call fastcc void @remove_directed(ptr noundef %0, i32 noundef %3, i32 noundef %2)
  tail call void @graph_register_csr_metadata(ptr noundef nonnull %0) #6
  br label %8

8:                                                ; preds = %5, %7
  ret void
}

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #3

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local void @free(ptr allocptr nocapture noundef) local_unnamed_addr #4

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind allocsize(1) }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"", !5, i64 0, !5, i64 8, !8, i64 16, !10, i64 24, !10, i64 32}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"p1 long", !9, i64 0}
!9 = !{!"any pointer", !6, i64 0}
!10 = !{!"p1 int", !9, i64 0}
!11 = !{!4, !8, i64 16}
!12 = !{!4, !5, i64 8}
!13 = !{!5, !5, i64 0}
!14 = distinct !{!14, !15, !16, !17}
!15 = !{!"llvm.loop.mustprogress"}
!16 = !{!"llvm.loop.isvectorized", i32 1}
!17 = !{!"llvm.loop.unroll.runtime.disable"}
!18 = distinct !{!18, !15, !17, !16}
!19 = distinct !{!19, !15}
!20 = !{!4, !10, i64 24}
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !6, i64 0}
!23 = distinct !{!23, !15}
!24 = distinct !{!24, !15}
!25 = !{!4, !10, i64 32}
!26 = distinct !{!26, !15}
!27 = distinct !{!27, !15, !16, !17}
!28 = distinct !{!28, !15, !17, !16}
!29 = distinct !{!29, !15, !16, !17}
!30 = distinct !{!30, !15, !17, !16}
!31 = !{}
!32 = distinct !{!32, !15}
!33 = distinct !{!33, !15}
