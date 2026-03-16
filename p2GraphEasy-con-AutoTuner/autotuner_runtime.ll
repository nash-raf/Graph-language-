; ModuleID = 'autotuner_runtime.c'
source_filename = "autotuner_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.AutoGraphMeta = type { ptr, i32, ptr, ptr, ptr, i64, ptr, ptr, i64, i64, ptr, ptr, i64, i64, i32, ptr, ptr, i64, ptr, ptr, i32, i32 }
%struct.EdgePair = type { i32, i32 }
%struct.EdgeHashEntry_s = type { i64, i64, i8 }
%struct.IdMapEntry = type { i32, i32, i8 }

@g_meta_count = internal unnamed_addr global i32 0, align 4
@g_meta = internal global [64 x %struct.AutoGraphMeta] zeroinitializer, align 16
@g_static_edge_hash = internal unnamed_addr global [64 x ptr] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @build_csr_from_set(i64 noundef %0, i64 noundef %1, ptr noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = icmp sgt i64 %1, 0
  br i1 %8, label %15, label %9

9:                                                ; preds = %7
  %10 = tail call noalias dereferenceable_or_null(8) ptr @calloc(i64 noundef 1, i64 noundef 8) #17
  br label %104

11:                                               ; preds = %29
  %12 = add nsw i64 %30, 2
  %13 = tail call noalias ptr @calloc(i64 noundef %12, i64 noundef 8) #17
  %14 = getelementptr inbounds nuw i8, ptr %13, i64 8
  br label %45

15:                                               ; preds = %7, %29
  %16 = phi i64 [ %30, %29 ], [ -1, %7 ]
  %17 = phi i64 [ %31, %29 ], [ 0, %7 ]
  %18 = trunc i64 %17 to i32
  %19 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %18) #18
  br i1 %19, label %20, label %29

20:                                               ; preds = %15
  %21 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %17
  %22 = load i32, ptr %21, align 4, !tbaa !3
  %23 = sext i32 %22 to i64
  %24 = tail call i64 @llvm.smax.i64(i64 %16, i64 %23)
  %25 = getelementptr inbounds nuw i8, ptr %21, i64 4
  %26 = load i32, ptr %25, align 4, !tbaa !8
  %27 = sext i32 %26 to i64
  %28 = tail call i64 @llvm.smax.i64(i64 %24, i64 %27)
  br label %29

29:                                               ; preds = %20, %15
  %30 = phi i64 [ %16, %15 ], [ %28, %20 ]
  %31 = add nuw nsw i64 %17, 1
  %32 = icmp eq i64 %31, %1
  br i1 %32, label %11, label %15, !llvm.loop !9

33:                                               ; preds = %62
  %34 = icmp slt i64 %30, 0
  br i1 %34, label %80, label %35

35:                                               ; preds = %33
  %36 = load i64, ptr %13, align 8
  %37 = add nuw i64 %30, 1
  %38 = and i64 %37, 3
  %39 = icmp ult i64 %30, 3
  br i1 %39, label %66, label %40

40:                                               ; preds = %35
  %41 = and i64 %37, -4
  %42 = getelementptr i8, ptr %13, i64 8
  %43 = getelementptr i8, ptr %13, i64 16
  %44 = getelementptr i8, ptr %13, i64 24
  br label %82

45:                                               ; preds = %11, %62
  %46 = phi i64 [ %64, %62 ], [ 0, %11 ]
  %47 = phi i64 [ %63, %62 ], [ 0, %11 ]
  %48 = trunc i64 %46 to i32
  %49 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %48) #18
  br i1 %49, label %50, label %62

50:                                               ; preds = %45
  %51 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %46
  %52 = load i32, ptr %51, align 4, !tbaa !3
  %53 = icmp slt i32 %52, 0
  %54 = zext nneg i32 %52 to i64
  %55 = icmp slt i64 %30, %54
  %56 = select i1 %53, i1 true, i1 %55
  br i1 %56, label %62, label %57

57:                                               ; preds = %50
  %58 = getelementptr inbounds nuw i64, ptr %14, i64 %54
  %59 = load i64, ptr %58, align 8, !tbaa !11
  %60 = add nsw i64 %59, 1
  store i64 %60, ptr %58, align 8, !tbaa !11
  %61 = add nsw i64 %47, 1
  br label %62

62:                                               ; preds = %45, %50, %57
  %63 = phi i64 [ %61, %57 ], [ %47, %50 ], [ %47, %45 ]
  %64 = add nuw nsw i64 %46, 1
  %65 = icmp eq i64 %64, %1
  br i1 %65, label %33, label %45, !llvm.loop !13

66:                                               ; preds = %82, %35
  %67 = phi i64 [ %36, %35 ], [ %97, %82 ]
  %68 = phi i64 [ 1, %35 ], [ %98, %82 ]
  %69 = icmp eq i64 %38, 0
  br i1 %69, label %80, label %70

70:                                               ; preds = %66, %70
  %71 = phi i64 [ %76, %70 ], [ %67, %66 ]
  %72 = phi i64 [ %77, %70 ], [ %68, %66 ]
  %73 = phi i64 [ %78, %70 ], [ 0, %66 ]
  %74 = getelementptr i64, ptr %13, i64 %72
  %75 = load i64, ptr %74, align 8, !tbaa !11
  %76 = add nsw i64 %75, %71
  store i64 %76, ptr %74, align 8, !tbaa !11
  %77 = add nuw i64 %72, 1
  %78 = add i64 %73, 1
  %79 = icmp eq i64 %78, %38
  br i1 %79, label %80, label %70, !llvm.loop !14

80:                                               ; preds = %66, %70, %33
  %81 = icmp sgt i64 %63, 0
  br i1 %81, label %101, label %104

82:                                               ; preds = %82, %40
  %83 = phi i64 [ %36, %40 ], [ %97, %82 ]
  %84 = phi i64 [ 1, %40 ], [ %98, %82 ]
  %85 = phi i64 [ 0, %40 ], [ %99, %82 ]
  %86 = getelementptr i64, ptr %13, i64 %84
  %87 = load i64, ptr %86, align 8, !tbaa !11
  %88 = add nsw i64 %87, %83
  store i64 %88, ptr %86, align 8, !tbaa !11
  %89 = getelementptr i64, ptr %42, i64 %84
  %90 = load i64, ptr %89, align 8, !tbaa !11
  %91 = add nsw i64 %90, %88
  store i64 %91, ptr %89, align 8, !tbaa !11
  %92 = getelementptr i64, ptr %43, i64 %84
  %93 = load i64, ptr %92, align 8, !tbaa !11
  %94 = add nsw i64 %93, %91
  store i64 %94, ptr %92, align 8, !tbaa !11
  %95 = getelementptr i64, ptr %44, i64 %84
  %96 = load i64, ptr %95, align 8, !tbaa !11
  %97 = add nsw i64 %96, %94
  store i64 %97, ptr %95, align 8, !tbaa !11
  %98 = add nuw i64 %84, 4
  %99 = add i64 %85, 4
  %100 = icmp eq i64 %99, %41
  br i1 %100, label %66, label %82, !llvm.loop !16

101:                                              ; preds = %80
  %102 = shl i64 %63, 2
  %103 = tail call noalias ptr @malloc(i64 noundef %102) #19
  br label %104

104:                                              ; preds = %9, %80, %101
  %105 = phi ptr [ %13, %101 ], [ %13, %80 ], [ %10, %9 ]
  %106 = phi i64 [ %12, %101 ], [ %12, %80 ], [ 1, %9 ]
  %107 = phi i64 [ %30, %101 ], [ %30, %80 ], [ -1, %9 ]
  %108 = phi i64 [ %63, %101 ], [ %63, %80 ], [ 0, %9 ]
  %109 = phi ptr [ %103, %101 ], [ null, %80 ], [ null, %9 ]
  %110 = shl nuw nsw i64 %106, 3
  %111 = tail call noalias ptr @malloc(i64 noundef %110) #19
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %111, ptr align 8 %105, i64 %110, i1 false)
  br i1 %8, label %113, label %112

112:                                              ; preds = %131, %104
  tail call void @free(ptr noundef %111) #18
  store ptr %105, ptr %4, align 8, !tbaa !17
  store ptr %109, ptr %5, align 8, !tbaa !20
  store i64 %108, ptr %6, align 8, !tbaa !11
  ret void

113:                                              ; preds = %104, %131
  %114 = phi i64 [ %132, %131 ], [ 0, %104 ]
  %115 = trunc i64 %114 to i32
  %116 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %115) #18
  br i1 %116, label %117, label %131

117:                                              ; preds = %113
  %118 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %114
  %119 = load i32, ptr %118, align 4, !tbaa !3
  %120 = icmp sgt i32 %119, -1
  br i1 %120, label %121, label %131

121:                                              ; preds = %117
  %122 = zext nneg i32 %119 to i64
  %123 = icmp slt i64 %107, %122
  br i1 %123, label %131, label %124

124:                                              ; preds = %121
  %125 = getelementptr inbounds nuw i8, ptr %118, i64 4
  %126 = load i32, ptr %125, align 4, !tbaa !8
  %127 = getelementptr inbounds nuw i64, ptr %111, i64 %122
  %128 = load i64, ptr %127, align 8, !tbaa !11
  %129 = add nsw i64 %128, 1
  store i64 %129, ptr %127, align 8, !tbaa !11
  %130 = getelementptr inbounds i32, ptr %109, i64 %128
  store i32 %126, ptr %130, align 4, !tbaa !22
  br label %131

131:                                              ; preds = %113, %117, %121, %124
  %132 = add nuw nsw i64 %114, 1
  %133 = icmp eq i64 %132, %1
  br i1 %133, label %112, label %113, !llvm.loop !23
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local zeroext i1 @roaring_bitmap_contains(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local void @free(ptr allocptr nocapture noundef) local_unnamed_addr #6

; Function Attrs: nounwind uwtable
define dso_local void @rebuild_sets_from_csr(i64 noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef readonly %4, ptr noundef %5, ptr noundef %6, ptr nocapture noundef readonly %7) local_unnamed_addr #0 {
  tail call void @roaring_bitmap_clear(ptr noundef %5) #18
  tail call void @roaring_bitmap_clear(ptr noundef %6) #18
  %9 = trunc i64 %1 to i32
  %10 = shl i32 %9, 1
  %11 = add i32 %10, 32
  br label %12

12:                                               ; preds = %12, %8
  %13 = phi i32 [ 1, %8 ], [ %15, %12 ]
  %14 = icmp sgt i32 %13, %11
  %15 = shl i32 %13, 1
  br i1 %14, label %16, label %12, !llvm.loop !24

16:                                               ; preds = %12
  %17 = sext i32 %13 to i64
  %18 = tail call noalias ptr @calloc(i64 noundef %17, i64 noundef 24) #17
  %19 = icmp sgt i64 %1, 0
  br i1 %19, label %20, label %22

20:                                               ; preds = %16
  %21 = add nsw i64 %17, -1
  br label %29

22:                                               ; preds = %73, %16
  %23 = icmp sgt i64 %0, 0
  br i1 %23, label %24, label %78

24:                                               ; preds = %22
  %25 = icmp eq ptr %18, null
  %26 = icmp slt i32 %13, 1
  %27 = or i1 %26, %25
  %28 = add nsw i64 %17, -1
  br label %79

29:                                               ; preds = %20, %73
  %30 = phi i64 [ 0, %20 ], [ %74, %73 ]
  %31 = getelementptr inbounds nuw %struct.EdgePair, ptr %7, i64 %30
  %32 = load i32, ptr %31, align 4, !tbaa !3
  %33 = getelementptr inbounds nuw i8, ptr %31, i64 4
  %34 = load i32, ptr %33, align 4, !tbaa !8
  %35 = tail call i32 @llvm.smin.i32(i32 %32, i32 %34)
  %36 = tail call i32 @llvm.smax.i32(i32 %32, i32 %34)
  %37 = zext i32 %35 to i64
  %38 = shl nuw i64 %37, 32
  %39 = zext i32 %36 to i64
  %40 = or disjoint i64 %38, %39
  %41 = lshr i64 %40, 30
  %42 = xor i64 %41, %40
  %43 = mul i64 %42, -4658895280553007687
  %44 = lshr i64 %43, 27
  %45 = xor i64 %44, %43
  %46 = mul i64 %45, -7723592293110705685
  %47 = lshr i64 %46, 31
  %48 = xor i64 %47, %46
  %49 = and i64 %48, %21
  %50 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %49
  %51 = getelementptr inbounds nuw i8, ptr %50, i64 16
  %52 = load i8, ptr %51, align 8, !tbaa !25
  %53 = icmp eq i8 %52, 0
  br i1 %53, label %68, label %54

54:                                               ; preds = %29, %61
  %55 = phi ptr [ %64, %61 ], [ %50, %29 ]
  %56 = phi i64 [ %63, %61 ], [ %49, %29 ]
  %57 = load i64, ptr %55, align 8, !tbaa !27
  %58 = icmp eq i64 %57, %40
  br i1 %58, label %59, label %61

59:                                               ; preds = %54
  %60 = getelementptr inbounds nuw i8, ptr %55, i64 8
  store i64 %30, ptr %60, align 8, !tbaa !28
  br label %73

61:                                               ; preds = %54
  %62 = add i64 %56, 1
  %63 = and i64 %62, %21
  %64 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %63
  %65 = getelementptr inbounds nuw i8, ptr %64, i64 16
  %66 = load i8, ptr %65, align 8, !tbaa !25
  %67 = icmp eq i8 %66, 0
  br i1 %67, label %68, label %54, !llvm.loop !29

68:                                               ; preds = %61, %29
  %69 = phi i64 [ %49, %29 ], [ %63, %61 ]
  %70 = phi ptr [ %50, %29 ], [ %64, %61 ]
  store i64 %40, ptr %70, align 8, !tbaa !27
  %71 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %69, i32 1
  store i64 %30, ptr %71, align 8, !tbaa !28
  %72 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %69, i32 2
  store i8 1, ptr %72, align 8, !tbaa !25
  br label %73

73:                                               ; preds = %59, %68
  %74 = add nuw nsw i64 %30, 1
  %75 = icmp eq i64 %74, %1
  br i1 %75, label %22, label %29, !llvm.loop !30

76:                                               ; preds = %132, %79
  %77 = icmp eq i64 %83, %0
  br i1 %77, label %78, label %79, !llvm.loop !31

78:                                               ; preds = %76, %22
  tail call void @free(ptr noundef %18) #18
  ret void

79:                                               ; preds = %24, %76
  %80 = phi i64 [ 0, %24 ], [ %83, %76 ]
  %81 = getelementptr inbounds nuw i64, ptr %3, i64 %80
  %82 = load i64, ptr %81, align 8, !tbaa !11
  %83 = add nuw nsw i64 %80, 1
  %84 = getelementptr inbounds nuw i64, ptr %3, i64 %83
  %85 = load i64, ptr %84, align 8, !tbaa !11
  %86 = icmp sgt i64 %85, %82
  br i1 %86, label %87, label %76

87:                                               ; preds = %79
  %88 = trunc i64 %80 to i32
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %88) #18
  %89 = trunc i64 %80 to i32
  br label %90

90:                                               ; preds = %87, %132
  %91 = phi i64 [ %82, %87 ], [ %133, %132 ]
  %92 = getelementptr inbounds i32, ptr %4, i64 %91
  %93 = load i32, ptr %92, align 4, !tbaa !22
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %93) #18
  br i1 %27, label %132, label %94

94:                                               ; preds = %90
  %95 = tail call i32 @llvm.smin.i32(i32 %89, i32 %93)
  %96 = tail call i32 @llvm.smax.i32(i32 %89, i32 %93)
  %97 = zext i32 %95 to i64
  %98 = shl nuw i64 %97, 32
  %99 = zext i32 %96 to i64
  %100 = or disjoint i64 %98, %99
  %101 = lshr i64 %100, 30
  %102 = xor i64 %101, %100
  %103 = mul i64 %102, -4658895280553007687
  %104 = lshr i64 %103, 27
  %105 = xor i64 %104, %103
  %106 = mul i64 %105, -7723592293110705685
  %107 = lshr i64 %106, 31
  %108 = xor i64 %107, %106
  %109 = and i64 %108, %28
  %110 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %109
  %111 = getelementptr inbounds nuw i8, ptr %110, i64 16
  %112 = load i8, ptr %111, align 8, !tbaa !25
  %113 = icmp eq i8 %112, 0
  br i1 %113, label %132, label %121

114:                                              ; preds = %121
  %115 = add i64 %123, 1
  %116 = and i64 %115, %28
  %117 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %116
  %118 = getelementptr inbounds nuw i8, ptr %117, i64 16
  %119 = load i8, ptr %118, align 8, !tbaa !25
  %120 = icmp eq i8 %119, 0
  br i1 %120, label %132, label %121, !llvm.loop !32

121:                                              ; preds = %94, %114
  %122 = phi ptr [ %117, %114 ], [ %110, %94 ]
  %123 = phi i64 [ %116, %114 ], [ %109, %94 ]
  %124 = load i64, ptr %122, align 8, !tbaa !27
  %125 = icmp eq i64 %124, %100
  br i1 %125, label %126, label %114

126:                                              ; preds = %121
  %127 = getelementptr inbounds nuw i8, ptr %122, i64 8
  %128 = load i64, ptr %127, align 8, !tbaa !28
  %129 = icmp sgt i64 %128, -1
  br i1 %129, label %130, label %132

130:                                              ; preds = %126
  %131 = trunc i64 %128 to i32
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %131) #18
  br label %132

132:                                              ; preds = %114, %94, %90, %130, %126
  %133 = add i64 %91, 1
  %134 = icmp eq i64 %133, %85
  br i1 %134, label %76, label %90, !llvm.loop !33
}

declare dso_local void @roaring_bitmap_clear(ptr noundef) local_unnamed_addr #2

declare dso_local void @roaring_bitmap_add(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_update_csr_pointers(ptr noundef readnone %0, ptr noundef %1, ptr noundef %2) local_unnamed_addr #7 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %19

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %19, label %11, !llvm.loop !34

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !35
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 80
  store ptr %1, ptr %17, align 8, !tbaa !38
  %18 = getelementptr inbounds nuw i8, ptr %13, i64 88
  store ptr %2, ptr %18, align 8, !tbaa !39
  br label %19

19:                                               ; preds = %8, %3, %16
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout_set(ptr noundef writeonly %0) local_unnamed_addr #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %159

7:                                                ; preds = %1
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %159, label %12, !llvm.loop !34

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 8, !tbaa !35
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 16
  %19 = load ptr, ptr %18, align 8, !tbaa !40
  %20 = icmp eq ptr %19, null
  br i1 %20, label %159, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %14, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !41
  %24 = icmp eq ptr %23, null
  br i1 %24, label %159, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %14, i64 32
  %27 = load ptr, ptr %26, align 8, !tbaa !42
  %28 = icmp eq ptr %27, null
  br i1 %28, label %159, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %14, i64 8
  %31 = load i32, ptr %30, align 8, !tbaa !43
  %32 = icmp eq i32 %31, 3
  br i1 %32, label %159, label %33

33:                                               ; preds = %29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #18
  store ptr null, ptr %2, align 8, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #18
  store ptr null, ptr %3, align 8, !tbaa !20
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %4) #18
  store i64 0, ptr %4, align 8, !tbaa !11
  switch i32 %31, label %70 [
    i32 0, label %34
    i32 1, label %46
    i32 2, label %56
  ]

34:                                               ; preds = %33
  %35 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %36 = load ptr, ptr %35, align 8, !tbaa !38
  store ptr %36, ptr %2, align 8, !tbaa !17
  %37 = getelementptr inbounds nuw i8, ptr %14, i64 88
  %38 = load ptr, ptr %37, align 8, !tbaa !39
  store ptr %38, ptr %3, align 8, !tbaa !20
  %39 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %40 = load i64, ptr %39, align 8, !tbaa !44
  %41 = getelementptr inbounds nuw i8, ptr %14, i64 104
  %42 = load i64, ptr %41, align 8, !tbaa !45
  store i64 %42, ptr %4, align 8, !tbaa !11
  %43 = getelementptr inbounds nuw i8, ptr %14, i64 112
  %44 = load i32, ptr %43, align 8, !tbaa !46
  %45 = icmp ne i32 %44, 0
  br label %70

46:                                               ; preds = %33
  %47 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %48 = load i64, ptr %47, align 8, !tbaa !44
  %49 = getelementptr inbounds nuw i8, ptr %14, i64 120
  %50 = load ptr, ptr %49, align 8, !tbaa !47
  %51 = getelementptr inbounds nuw i8, ptr %14, i64 128
  %52 = load ptr, ptr %51, align 8, !tbaa !48
  call void @convert_pcsr_to_csr(i64 noundef %48, i64 poison, ptr noundef %50, ptr noundef %52, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %53 = load i64, ptr %47, align 8, !tbaa !44
  %54 = load ptr, ptr %49, align 8, !tbaa !47
  tail call void @free(ptr noundef %54) #18
  store ptr null, ptr %49, align 8, !tbaa !47
  %55 = load ptr, ptr %51, align 8, !tbaa !48
  tail call void @free(ptr noundef %55) #18
  store ptr null, ptr %51, align 8, !tbaa !48
  br label %70

56:                                               ; preds = %33
  %57 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %58 = load i64, ptr %57, align 8, !tbaa !44
  %59 = getelementptr inbounds nuw i8, ptr %14, i64 164
  %60 = load i32, ptr %59, align 4, !tbaa !49
  %61 = getelementptr inbounds nuw i8, ptr %14, i64 160
  %62 = load i32, ptr %61, align 8, !tbaa !50
  %63 = getelementptr inbounds nuw i8, ptr %14, i64 144
  %64 = load ptr, ptr %63, align 8, !tbaa !51
  %65 = getelementptr inbounds nuw i8, ptr %14, i64 152
  %66 = load ptr, ptr %65, align 8, !tbaa !52
  call void @convert_bcsr_to_csr(i64 noundef %58, i32 noundef %60, i32 noundef %62, ptr noundef %64, ptr noundef %66, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %67 = load i64, ptr %57, align 8, !tbaa !44
  %68 = load ptr, ptr %63, align 8, !tbaa !51
  call void @free(ptr noundef %68) #18
  store ptr null, ptr %63, align 8, !tbaa !51
  %69 = load ptr, ptr %65, align 8, !tbaa !52
  call void @free(ptr noundef %69) #18
  store ptr null, ptr %65, align 8, !tbaa !52
  br label %70

70:                                               ; preds = %33, %46, %56, %34
  %71 = phi i1 [ %45, %34 ], [ true, %46 ], [ true, %56 ], [ false, %33 ]
  %72 = phi i64 [ %40, %34 ], [ %53, %46 ], [ %67, %56 ], [ 0, %33 ]
  %73 = load ptr, ptr %2, align 8, !tbaa !17
  %74 = icmp ne ptr %73, null
  %75 = load ptr, ptr %3, align 8
  %76 = icmp ne ptr %75, null
  %77 = select i1 %74, i1 %76, i1 false
  br i1 %77, label %78, label %80

78:                                               ; preds = %70
  call fastcc void @rebuild_sets_from_csr_meta(ptr noundef %14, ptr noundef %73, ptr noundef %75, i64 noundef %72)
  %79 = load ptr, ptr %2, align 8
  br label %80

80:                                               ; preds = %78, %70
  %81 = phi ptr [ %79, %78 ], [ %73, %70 ]
  %82 = icmp ne ptr %81, null
  %83 = select i1 %71, i1 %82, i1 false
  br i1 %83, label %84, label %85

84:                                               ; preds = %80
  call void @free(ptr noundef nonnull %81) #18
  br label %85

85:                                               ; preds = %84, %80
  %86 = load ptr, ptr %3, align 8
  %87 = icmp ne ptr %86, null
  %88 = select i1 %71, i1 %87, i1 false
  br i1 %88, label %89, label %90

89:                                               ; preds = %85
  call void @free(ptr noundef nonnull %86) #18
  br label %90

90:                                               ; preds = %89, %85
  %91 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %92 = getelementptr inbounds nuw i8, ptr %14, i64 112
  store i32 0, ptr %92, align 8, !tbaa !46
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %91, i8 0, i64 16, i1 false)
  %93 = load ptr, ptr %18, align 8, !tbaa !40
  %94 = call i64 @roaring_bitmap_get_cardinality(ptr noundef %93) #18
  store i64 0, ptr %4, align 8, !tbaa !11
  %95 = load ptr, ptr %22, align 8, !tbaa !41
  %96 = getelementptr inbounds nuw i8, ptr %14, i64 40
  %97 = load i64, ptr %96, align 8, !tbaa !53
  %98 = icmp sgt i64 %97, 0
  br i1 %98, label %111, label %99

99:                                               ; preds = %118, %90
  %100 = getelementptr inbounds nuw i8, ptr %14, i64 64
  %101 = load i64, ptr %100, align 8, !tbaa !54
  %102 = icmp sgt i64 %101, 0
  %103 = load i64, ptr %4, align 8
  br i1 %102, label %104, label %133

104:                                              ; preds = %99
  %105 = getelementptr inbounds nuw i8, ptr %14, i64 56
  %106 = load ptr, ptr %105, align 8, !tbaa !55
  %107 = and i64 %101, 1
  %108 = icmp eq i64 %101, 1
  br i1 %108, label %122, label %109

109:                                              ; preds = %104
  %110 = and i64 %101, 9223372036854775806
  br label %137

111:                                              ; preds = %90, %118
  %112 = phi i64 [ %119, %118 ], [ 0, %90 ]
  %113 = trunc i64 %112 to i32
  %114 = call zeroext i1 @roaring_bitmap_contains(ptr noundef %95, i32 noundef %113) #18
  br i1 %114, label %115, label %118

115:                                              ; preds = %111
  %116 = load i64, ptr %4, align 8, !tbaa !11
  %117 = add nsw i64 %116, 1
  store i64 %117, ptr %4, align 8, !tbaa !11
  br label %118

118:                                              ; preds = %111, %115
  %119 = add nuw nsw i64 %112, 1
  %120 = load i64, ptr %96, align 8, !tbaa !53
  %121 = icmp slt i64 %119, %120
  br i1 %121, label %111, label %99, !llvm.loop !56

122:                                              ; preds = %154, %104
  %123 = phi i64 [ poison, %104 ], [ %155, %154 ]
  %124 = phi i64 [ 0, %104 ], [ %156, %154 ]
  %125 = phi i64 [ %103, %104 ], [ %155, %154 ]
  %126 = icmp eq i64 %107, 0
  br i1 %126, label %133, label %127

127:                                              ; preds = %122
  %128 = getelementptr inbounds nuw i8, ptr %106, i64 %124
  %129 = load i8, ptr %128, align 1, !tbaa !57
  %130 = icmp eq i8 %129, 0
  br i1 %130, label %133, label %131

131:                                              ; preds = %127
  %132 = add nsw i64 %125, 1
  store i64 %132, ptr %4, align 8, !tbaa !11
  br label %133

133:                                              ; preds = %122, %131, %127, %99
  %134 = phi i64 [ %103, %99 ], [ %123, %122 ], [ %125, %127 ], [ %132, %131 ]
  store i64 %94, ptr %0, align 8, !tbaa !11
  %135 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %134, ptr %135, align 8, !tbaa !11
  %136 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %136, i8 0, i64 16, i1 false)
  store i32 3, ptr %30, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %4) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #18
  br label %159

137:                                              ; preds = %154, %109
  %138 = phi i64 [ 0, %109 ], [ %156, %154 ]
  %139 = phi i64 [ %103, %109 ], [ %155, %154 ]
  %140 = phi i64 [ 0, %109 ], [ %157, %154 ]
  %141 = getelementptr inbounds nuw i8, ptr %106, i64 %138
  %142 = load i8, ptr %141, align 1, !tbaa !57
  %143 = icmp eq i8 %142, 0
  br i1 %143, label %146, label %144

144:                                              ; preds = %137
  %145 = add nsw i64 %139, 1
  store i64 %145, ptr %4, align 8, !tbaa !11
  br label %146

146:                                              ; preds = %137, %144
  %147 = phi i64 [ %139, %137 ], [ %145, %144 ]
  %148 = or disjoint i64 %138, 1
  %149 = getelementptr inbounds nuw i8, ptr %106, i64 %148
  %150 = load i8, ptr %149, align 1, !tbaa !57
  %151 = icmp eq i8 %150, 0
  br i1 %151, label %154, label %152

152:                                              ; preds = %146
  %153 = add nsw i64 %147, 1
  store i64 %153, ptr %4, align 8, !tbaa !11
  br label %154

154:                                              ; preds = %152, %146
  %155 = phi i64 [ %147, %146 ], [ %153, %152 ]
  %156 = add nuw nsw i64 %138, 2
  %157 = add i64 %140, 2
  %158 = icmp eq i64 %157, %110
  br i1 %158, label %122, label %137, !llvm.loop !58

159:                                              ; preds = %9, %1, %29, %17, %21, %25, %133
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_pcsr_to_csr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = add i64 %0, 1
  %9 = tail call noalias ptr @calloc(i64 noundef %8, i64 noundef 8) #17
  %10 = icmp sgt i64 %0, 0
  br i1 %10, label %11, label %89

11:                                               ; preds = %7
  %12 = load i64, ptr %2, align 8, !tbaa !11
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
  %28 = load i64, ptr %27, align 8, !tbaa !11
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
  %43 = load <2 x i32>, ptr %41, align 4, !tbaa !22
  %44 = load <2 x i32>, ptr %42, align 4, !tbaa !22
  %45 = icmp ne <2 x i32> %43, splat (i32 -1)
  %46 = icmp ne <2 x i32> %44, splat (i32 -1)
  %47 = zext <2 x i1> %45 to <2 x i64>
  %48 = zext <2 x i1> %46 to <2 x i64>
  %49 = add <2 x i64> %39, %47
  %50 = add <2 x i64> %40, %48
  %51 = add nuw i64 %38, 4
  %52 = icmp eq i64 %51, %34
  br i1 %52, label %53, label %37, !llvm.loop !59

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
  store i64 %61, ptr %62, align 8, !tbaa !11
  %63 = add nuw nsw i64 %61, %24
  %64 = icmp eq i64 %26, %0
  br i1 %64, label %13, label %22, !llvm.loop !62

65:                                               ; preds = %57, %65
  %66 = phi i64 [ %72, %65 ], [ %58, %57 ]
  %67 = phi i64 [ %73, %65 ], [ %59, %57 ]
  %68 = getelementptr inbounds i32, ptr %3, i64 %67
  %69 = load i32, ptr %68, align 4, !tbaa !22
  %70 = icmp ne i32 %69, -1
  %71 = zext i1 %70 to i64
  %72 = add nuw nsw i64 %66, %71
  %73 = add nsw i64 %67, 1
  %74 = icmp eq i64 %73, %28
  br i1 %74, label %60, label %65, !llvm.loop !63

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
  %84 = load i64, ptr %83, align 8, !tbaa !11
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !11
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %15
  br i1 %88, label %89, label %79, !llvm.loop !64

89:                                               ; preds = %75, %79, %7
  %90 = phi i64 [ 0, %7 ], [ %63, %79 ], [ %63, %75 ]
  %91 = shl i64 %90, 2
  %92 = tail call noalias ptr @malloc(i64 noundef %91) #19
  %93 = shl i64 %8, 3
  %94 = tail call noalias ptr @malloc(i64 noundef %93) #19
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %94, ptr align 8 %9, i64 %93, i1 false)
  br i1 %10, label %95, label %119

95:                                               ; preds = %89
  %96 = load i64, ptr %2, align 8, !tbaa !11
  %97 = getelementptr i8, ptr %3, i64 4
  br label %120

98:                                               ; preds = %98, %17
  %99 = phi i64 [ %14, %17 ], [ %113, %98 ]
  %100 = phi i64 [ 1, %17 ], [ %114, %98 ]
  %101 = phi i64 [ 0, %17 ], [ %115, %98 ]
  %102 = getelementptr i64, ptr %9, i64 %100
  %103 = load i64, ptr %102, align 8, !tbaa !11
  %104 = add nsw i64 %103, %99
  store i64 %104, ptr %102, align 8, !tbaa !11
  %105 = getelementptr i64, ptr %19, i64 %100
  %106 = load i64, ptr %105, align 8, !tbaa !11
  %107 = add nsw i64 %106, %104
  store i64 %107, ptr %105, align 8, !tbaa !11
  %108 = getelementptr i64, ptr %20, i64 %100
  %109 = load i64, ptr %108, align 8, !tbaa !11
  %110 = add nsw i64 %109, %107
  store i64 %110, ptr %108, align 8, !tbaa !11
  %111 = getelementptr i64, ptr %21, i64 %100
  %112 = load i64, ptr %111, align 8, !tbaa !11
  %113 = add nsw i64 %112, %110
  store i64 %113, ptr %111, align 8, !tbaa !11
  %114 = add nuw i64 %100, 4
  %115 = add i64 %101, 4
  %116 = icmp eq i64 %115, %18
  br i1 %116, label %75, label %98, !llvm.loop !65

117:                                              ; preds = %143, %163, %120
  %118 = icmp eq i64 %123, %0
  br i1 %118, label %119, label %120, !llvm.loop !66

119:                                              ; preds = %117, %89
  tail call void @free(ptr noundef %94) #18
  store ptr %9, ptr %4, align 8, !tbaa !17
  store ptr %92, ptr %5, align 8, !tbaa !20
  store i64 %90, ptr %6, align 8, !tbaa !11
  ret void

120:                                              ; preds = %95, %117
  %121 = phi i64 [ %125, %117 ], [ %96, %95 ]
  %122 = phi i64 [ %123, %117 ], [ 0, %95 ]
  %123 = add nuw nsw i64 %122, 1
  %124 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %125 = load i64, ptr %124, align 8, !tbaa !11
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
  %135 = load i32, ptr %134, align 4, !tbaa !22
  %136 = icmp eq i32 %135, -1
  br i1 %136, label %141, label %137

137:                                              ; preds = %133
  %138 = load i64, ptr %128, align 8, !tbaa !11
  %139 = add nsw i64 %138, 1
  store i64 %139, ptr %128, align 8, !tbaa !11
  %140 = getelementptr inbounds i32, ptr %92, i64 %138
  store i32 %135, ptr %140, align 4, !tbaa !22
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
  %149 = load i32, ptr %148, align 4, !tbaa !22
  %150 = icmp eq i32 %149, -1
  br i1 %150, label %155, label %151

151:                                              ; preds = %146
  %152 = load i64, ptr %128, align 8, !tbaa !11
  %153 = add nsw i64 %152, 1
  store i64 %153, ptr %128, align 8, !tbaa !11
  %154 = getelementptr inbounds i32, ptr %92, i64 %152
  store i32 %149, ptr %154, align 4, !tbaa !22
  br label %155

155:                                              ; preds = %146, %151
  %156 = getelementptr i32, ptr %97, i64 %147
  %157 = load i32, ptr %156, align 4, !tbaa !22
  %158 = icmp eq i32 %157, -1
  br i1 %158, label %163, label %159

159:                                              ; preds = %155
  %160 = load i64, ptr %128, align 8, !tbaa !11
  %161 = add nsw i64 %160, 1
  store i64 %161, ptr %128, align 8, !tbaa !11
  %162 = getelementptr inbounds i32, ptr %92, i64 %160
  store i32 %157, ptr %162, align 4, !tbaa !22
  br label %163

163:                                              ; preds = %159, %155
  %164 = add nsw i64 %147, 2
  %165 = icmp eq i64 %164, %125
  br i1 %165, label %117, label %146, !llvm.loop !67
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
  %22 = tail call noalias ptr @calloc(i64 noundef %21, i64 noundef 8) #17
  %23 = icmp eq ptr %22, null
  br i1 %23, label %167, label %24

24:                                               ; preds = %20
  %25 = icmp eq i32 %1, 0
  br i1 %25, label %33, label %26

26:                                               ; preds = %24
  %27 = zext nneg i32 %2 to i64
  %28 = getelementptr i8, ptr %22, i64 8
  %29 = zext nneg i32 %1 to i64
  %30 = load i32, ptr %3, align 4, !tbaa !22
  br label %44

31:                                               ; preds = %71, %44
  %32 = icmp eq i64 %50, %29
  br i1 %32, label %33, label %44, !llvm.loop !68

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
  %52 = load i32, ptr %51, align 4, !tbaa !22
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
  %61 = load i32, ptr %60, align 4, !tbaa !22
  %62 = sext i32 %61 to i64
  %63 = add nsw i64 %47, %62
  %64 = icmp sgt i32 %61, -1
  %65 = icmp slt i64 %63, %49
  %66 = select i1 %64, i1 %65, i1 false
  br i1 %66, label %67, label %71

67:                                               ; preds = %57
  %68 = getelementptr i64, ptr %28, i64 %63
  %69 = load i64, ptr %68, align 8, !tbaa !11
  %70 = add nsw i64 %69, 1
  store i64 %70, ptr %68, align 8, !tbaa !11
  br label %71

71:                                               ; preds = %67, %57
  %72 = add nsw i64 %58, 2
  %73 = add i32 %59, 3
  %74 = icmp slt i32 %73, %52
  br i1 %74, label %57, label %31, !llvm.loop !69

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
  %84 = load i64, ptr %83, align 8, !tbaa !11
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !11
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %37
  br i1 %88, label %89, label %79, !llvm.loop !70

89:                                               ; preds = %75, %79, %33
  %90 = getelementptr inbounds i64, ptr %22, i64 %0
  %91 = load i64, ptr %90, align 8, !tbaa !11
  %92 = icmp sgt i64 %91, 0
  br i1 %92, label %112, label %117

93:                                               ; preds = %93, %39
  %94 = phi i64 [ %36, %39 ], [ %108, %93 ]
  %95 = phi i64 [ 1, %39 ], [ %109, %93 ]
  %96 = phi i64 [ 0, %39 ], [ %110, %93 ]
  %97 = getelementptr i64, ptr %22, i64 %95
  %98 = load i64, ptr %97, align 8, !tbaa !11
  %99 = add nsw i64 %98, %94
  store i64 %99, ptr %97, align 8, !tbaa !11
  %100 = getelementptr i64, ptr %41, i64 %95
  %101 = load i64, ptr %100, align 8, !tbaa !11
  %102 = add nsw i64 %101, %99
  store i64 %102, ptr %100, align 8, !tbaa !11
  %103 = getelementptr i64, ptr %42, i64 %95
  %104 = load i64, ptr %103, align 8, !tbaa !11
  %105 = add nsw i64 %104, %102
  store i64 %105, ptr %103, align 8, !tbaa !11
  %106 = getelementptr i64, ptr %43, i64 %95
  %107 = load i64, ptr %106, align 8, !tbaa !11
  %108 = add nsw i64 %107, %105
  store i64 %108, ptr %106, align 8, !tbaa !11
  %109 = add nuw i64 %95, 4
  %110 = add i64 %96, 4
  %111 = icmp eq i64 %110, %40
  br i1 %111, label %75, label %93, !llvm.loop !71

112:                                              ; preds = %89
  %113 = shl i64 %91, 2
  %114 = tail call noalias ptr @malloc(i64 noundef %113) #19
  %115 = icmp eq ptr %114, null
  br i1 %115, label %116, label %117

116:                                              ; preds = %112
  tail call void @free(ptr noundef nonnull %22) #18
  br label %167

117:                                              ; preds = %112, %89
  %118 = phi ptr [ %114, %112 ], [ null, %89 ]
  %119 = shl i64 %21, 3
  %120 = tail call noalias ptr @malloc(i64 noundef %119) #19
  %121 = icmp eq ptr %120, null
  br i1 %121, label %122, label %123

122:                                              ; preds = %117
  tail call void @free(ptr noundef nonnull %22) #18
  tail call void @free(ptr noundef %118) #18
  br label %167

123:                                              ; preds = %117
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 %120, ptr nonnull align 8 %22, i64 %119, i1 false)
  br i1 %25, label %130, label %124

124:                                              ; preds = %123
  %125 = zext nneg i32 %2 to i64
  %126 = zext nneg i32 %1 to i64
  %127 = load i32, ptr %3, align 4, !tbaa !22
  br label %131

128:                                              ; preds = %162, %131
  %129 = icmp eq i64 %137, %126
  br i1 %129, label %130, label %131, !llvm.loop !72

130:                                              ; preds = %128, %123
  tail call void @free(ptr noundef %120) #18
  store ptr %22, ptr %5, align 8, !tbaa !17
  store ptr %118, ptr %6, align 8, !tbaa !20
  store i64 %91, ptr %7, align 8, !tbaa !11
  br label %167

131:                                              ; preds = %124, %128
  %132 = phi i32 [ %127, %124 ], [ %139, %128 ]
  %133 = phi i64 [ 0, %124 ], [ %137, %128 ]
  %134 = mul nuw nsw i64 %133, %125
  %135 = add nuw nsw i64 %134, %125
  %136 = tail call i64 @llvm.smin.i64(i64 %135, i64 %0)
  %137 = add nuw nsw i64 %133, 1
  %138 = getelementptr inbounds nuw i32, ptr %3, i64 %137
  %139 = load i32, ptr %138, align 4, !tbaa !22
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
  %148 = load i32, ptr %147, align 4, !tbaa !22
  %149 = sext i32 %148 to i64
  %150 = add nsw i64 %134, %149
  %151 = icmp sgt i32 %148, -1
  %152 = icmp slt i64 %150, %136
  %153 = select i1 %151, i1 %152, i1 false
  br i1 %153, label %154, label %162

154:                                              ; preds = %144
  %155 = sext i32 %146 to i64
  %156 = getelementptr inbounds i32, ptr %4, i64 %155
  %157 = load i32, ptr %156, align 4, !tbaa !22
  %158 = getelementptr inbounds nuw i64, ptr %120, i64 %150
  %159 = load i64, ptr %158, align 8, !tbaa !11
  %160 = add nsw i64 %159, 1
  store i64 %160, ptr %158, align 8, !tbaa !11
  %161 = getelementptr inbounds i32, ptr %118, i64 %159
  store i32 %157, ptr %161, align 4, !tbaa !22
  br label %162

162:                                              ; preds = %154, %144
  %163 = add nsw i64 %145, 2
  %164 = trunc nsw i64 %145 to i32
  %165 = add i32 %164, 3
  %166 = icmp slt i32 %165, %139
  br i1 %166, label %144, label %128, !llvm.loop !73

167:                                              ; preds = %20, %122, %130, %116, %8
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @rebuild_sets_from_csr_meta(ptr nocapture noundef nonnull %0, ptr nocapture noundef nonnull readonly %1, ptr nocapture noundef nonnull readonly %2, i64 noundef %3) unnamed_addr #0 {
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !40
  %7 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %8 = load ptr, ptr %7, align 8, !tbaa !41
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %10 = load ptr, ptr %9, align 8, !tbaa !42
  tail call void @roaring_bitmap_clear(ptr noundef %6) #18
  tail call void @roaring_bitmap_clear(ptr noundef %8) #18
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %12 = load i64, ptr %11, align 8, !tbaa !74
  %13 = icmp sgt i64 %12, 0
  br i1 %13, label %14, label %16

14:                                               ; preds = %4
  %15 = getelementptr inbounds nuw i8, ptr %0, i64 56
  br label %33

16:                                               ; preds = %33, %4
  %17 = getelementptr inbounds nuw i8, ptr %0, i64 64
  store i64 0, ptr %17, align 8, !tbaa !54
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %19 = load i64, ptr %18, align 8, !tbaa !53
  %20 = trunc i64 %19 to i32
  %21 = shl i32 %20, 1
  %22 = add i32 %21, 32
  br label %23

23:                                               ; preds = %23, %16
  %24 = phi i32 [ 1, %16 ], [ %26, %23 ]
  %25 = icmp sgt i32 %24, %22
  %26 = shl i32 %24, 1
  br i1 %25, label %27, label %23, !llvm.loop !24

27:                                               ; preds = %23
  %28 = sext i32 %24 to i64
  %29 = tail call noalias ptr @calloc(i64 noundef %28, i64 noundef 24) #17
  %30 = icmp sgt i64 %19, 0
  br i1 %30, label %31, label %40

31:                                               ; preds = %27
  %32 = add nsw i64 %28, -1
  br label %49

33:                                               ; preds = %14, %33
  %34 = phi i64 [ 0, %14 ], [ %37, %33 ]
  %35 = load ptr, ptr %15, align 8, !tbaa !55
  %36 = getelementptr inbounds nuw i8, ptr %35, i64 %34
  store i8 0, ptr %36, align 1, !tbaa !57
  %37 = add nuw nsw i64 %34, 1
  %38 = load i64, ptr %11, align 8, !tbaa !74
  %39 = icmp slt i64 %37, %38
  br i1 %39, label %33, label %16, !llvm.loop !75

40:                                               ; preds = %93, %27
  %41 = icmp sgt i64 %3, 0
  br i1 %41, label %42, label %98

42:                                               ; preds = %40
  %43 = icmp eq ptr %29, null
  %44 = icmp slt i32 %24, 1
  %45 = or i1 %44, %43
  %46 = add nsw i64 %28, -1
  %47 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %48 = getelementptr inbounds nuw i8, ptr %0, i64 56
  br label %99

49:                                               ; preds = %31, %93
  %50 = phi i64 [ 0, %31 ], [ %94, %93 ]
  %51 = getelementptr inbounds nuw %struct.EdgePair, ptr %10, i64 %50
  %52 = load i32, ptr %51, align 4, !tbaa !3
  %53 = getelementptr inbounds nuw i8, ptr %51, i64 4
  %54 = load i32, ptr %53, align 4, !tbaa !8
  %55 = tail call i32 @llvm.smin.i32(i32 %52, i32 %54)
  %56 = tail call i32 @llvm.smax.i32(i32 %52, i32 %54)
  %57 = zext i32 %55 to i64
  %58 = shl nuw i64 %57, 32
  %59 = zext i32 %56 to i64
  %60 = or disjoint i64 %58, %59
  %61 = lshr i64 %60, 30
  %62 = xor i64 %61, %60
  %63 = mul i64 %62, -4658895280553007687
  %64 = lshr i64 %63, 27
  %65 = xor i64 %64, %63
  %66 = mul i64 %65, -7723592293110705685
  %67 = lshr i64 %66, 31
  %68 = xor i64 %67, %66
  %69 = and i64 %68, %32
  %70 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %69
  %71 = getelementptr inbounds nuw i8, ptr %70, i64 16
  %72 = load i8, ptr %71, align 8, !tbaa !25
  %73 = icmp eq i8 %72, 0
  br i1 %73, label %88, label %74

74:                                               ; preds = %49, %81
  %75 = phi ptr [ %84, %81 ], [ %70, %49 ]
  %76 = phi i64 [ %83, %81 ], [ %69, %49 ]
  %77 = load i64, ptr %75, align 8, !tbaa !27
  %78 = icmp eq i64 %77, %60
  br i1 %78, label %79, label %81

79:                                               ; preds = %74
  %80 = getelementptr inbounds nuw i8, ptr %75, i64 8
  store i64 %50, ptr %80, align 8, !tbaa !28
  br label %93

81:                                               ; preds = %74
  %82 = add i64 %76, 1
  %83 = and i64 %82, %32
  %84 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %83
  %85 = getelementptr inbounds nuw i8, ptr %84, i64 16
  %86 = load i8, ptr %85, align 8, !tbaa !25
  %87 = icmp eq i8 %86, 0
  br i1 %87, label %88, label %74, !llvm.loop !29

88:                                               ; preds = %81, %49
  %89 = phi i64 [ %69, %49 ], [ %83, %81 ]
  %90 = phi ptr [ %70, %49 ], [ %84, %81 ]
  store i64 %60, ptr %90, align 8, !tbaa !27
  %91 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %89, i32 1
  store i64 %50, ptr %91, align 8, !tbaa !28
  %92 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %89, i32 2
  store i8 1, ptr %92, align 8, !tbaa !25
  br label %93

93:                                               ; preds = %79, %88
  %94 = add nuw nsw i64 %50, 1
  %95 = icmp eq i64 %94, %19
  br i1 %95, label %40, label %49, !llvm.loop !76

96:                                               ; preds = %218, %99
  %97 = icmp eq i64 %103, %3
  br i1 %97, label %98, label %99, !llvm.loop !77

98:                                               ; preds = %96, %40
  tail call void @free(ptr noundef %29) #18
  ret void

99:                                               ; preds = %42, %96
  %100 = phi i64 [ 0, %42 ], [ %103, %96 ]
  %101 = getelementptr inbounds nuw i64, ptr %1, i64 %100
  %102 = load i64, ptr %101, align 8, !tbaa !11
  %103 = add nuw nsw i64 %100, 1
  %104 = getelementptr inbounds nuw i64, ptr %1, i64 %103
  %105 = load i64, ptr %104, align 8, !tbaa !11
  %106 = icmp sgt i64 %105, %102
  br i1 %106, label %107, label %96

107:                                              ; preds = %99
  %108 = trunc i64 %100 to i32
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %108) #18
  %109 = trunc i64 %100 to i32
  br label %110

110:                                              ; preds = %107, %218
  %111 = phi i64 [ %102, %107 ], [ %219, %218 ]
  %112 = getelementptr inbounds i32, ptr %2, i64 %111
  %113 = load i32, ptr %112, align 4, !tbaa !22
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %113) #18
  br i1 %45, label %152, label %114

114:                                              ; preds = %110
  %115 = tail call i32 @llvm.smin.i32(i32 %109, i32 %113)
  %116 = tail call i32 @llvm.smax.i32(i32 %109, i32 %113)
  %117 = zext i32 %115 to i64
  %118 = shl nuw i64 %117, 32
  %119 = zext i32 %116 to i64
  %120 = or disjoint i64 %118, %119
  %121 = lshr i64 %120, 30
  %122 = xor i64 %121, %120
  %123 = mul i64 %122, -4658895280553007687
  %124 = lshr i64 %123, 27
  %125 = xor i64 %124, %123
  %126 = mul i64 %125, -7723592293110705685
  %127 = lshr i64 %126, 31
  %128 = xor i64 %127, %126
  %129 = and i64 %128, %46
  %130 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %129
  %131 = getelementptr inbounds nuw i8, ptr %130, i64 16
  %132 = load i8, ptr %131, align 8, !tbaa !25
  %133 = icmp eq i8 %132, 0
  br i1 %133, label %152, label %141

134:                                              ; preds = %141
  %135 = add i64 %143, 1
  %136 = and i64 %135, %46
  %137 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %136
  %138 = getelementptr inbounds nuw i8, ptr %137, i64 16
  %139 = load i8, ptr %138, align 8, !tbaa !25
  %140 = icmp eq i8 %139, 0
  br i1 %140, label %152, label %141, !llvm.loop !32

141:                                              ; preds = %114, %134
  %142 = phi ptr [ %137, %134 ], [ %130, %114 ]
  %143 = phi i64 [ %136, %134 ], [ %129, %114 ]
  %144 = load i64, ptr %142, align 8, !tbaa !27
  %145 = icmp eq i64 %144, %120
  br i1 %145, label %146, label %134

146:                                              ; preds = %141
  %147 = getelementptr inbounds nuw i8, ptr %142, i64 8
  %148 = load i64, ptr %147, align 8, !tbaa !28
  %149 = icmp sgt i64 %148, -1
  br i1 %149, label %150, label %152

150:                                              ; preds = %146
  %151 = trunc i64 %148 to i32
  tail call void @roaring_bitmap_add(ptr noundef %8, i32 noundef %151) #18
  br label %218

152:                                              ; preds = %134, %114, %110, %146
  %153 = load i64, ptr %17, align 8, !tbaa !54
  %154 = icmp slt i64 %153, 1
  br i1 %154, label %178, label %155

155:                                              ; preds = %152
  %156 = load ptr, ptr %47, align 8, !tbaa !78
  br label %157

157:                                              ; preds = %172, %155
  %158 = phi i64 [ 0, %155 ], [ %173, %172 ]
  %159 = shl nuw nsw i64 %158, 1
  %160 = getelementptr inbounds nuw i32, ptr %156, i64 %159
  %161 = load i32, ptr %160, align 4, !tbaa !22
  %162 = or disjoint i64 %159, 1
  %163 = getelementptr inbounds nuw i32, ptr %156, i64 %162
  %164 = load i32, ptr %163, align 4, !tbaa !22
  %165 = icmp eq i32 %161, %109
  %166 = icmp eq i32 %164, %113
  %167 = select i1 %165, i1 %166, i1 false
  br i1 %167, label %175, label %168

168:                                              ; preds = %157
  %169 = icmp ne i32 %161, %113
  %170 = icmp ne i32 %164, %109
  %171 = select i1 %169, i1 true, i1 %170
  br i1 %171, label %172, label %175

172:                                              ; preds = %168
  %173 = add nuw nsw i64 %158, 1
  %174 = icmp eq i64 %173, %153
  br i1 %174, label %178, label %157, !llvm.loop !79

175:                                              ; preds = %168, %157
  %176 = load ptr, ptr %48, align 8, !tbaa !55
  %177 = getelementptr inbounds nuw i8, ptr %176, i64 %158
  store i8 1, ptr %177, align 1, !tbaa !57
  br label %218

178:                                              ; preds = %172, %152
  %179 = load i64, ptr %11, align 8, !tbaa !74
  %180 = icmp slt i64 %153, %179
  br i1 %180, label %181, label %184

181:                                              ; preds = %178
  %182 = load ptr, ptr %47, align 8, !tbaa !78
  %183 = load ptr, ptr %48, align 8, !tbaa !55
  br label %208

184:                                              ; preds = %178
  %185 = icmp sgt i64 %179, 0
  %186 = select i1 %185, i64 %179, i64 16
  br label %187

187:                                              ; preds = %187, %184
  %188 = phi i64 [ %186, %184 ], [ %190, %187 ]
  %189 = icmp sgt i64 %188, %153
  %190 = shl nsw i64 %188, 1
  br i1 %189, label %191, label %187, !llvm.loop !80

191:                                              ; preds = %187
  %192 = load ptr, ptr %47, align 8, !tbaa !78
  %193 = shl i64 %188, 3
  %194 = tail call ptr @realloc(ptr noundef %192, i64 noundef %193) #20
  %195 = load ptr, ptr %48, align 8, !tbaa !55
  %196 = tail call ptr @realloc(ptr noundef %195, i64 noundef %188) #20
  %197 = icmp ne ptr %194, null
  %198 = icmp ne ptr %196, null
  %199 = and i1 %197, %198
  br i1 %199, label %200, label %218

200:                                              ; preds = %191
  %201 = load i64, ptr %11, align 8, !tbaa !74
  %202 = icmp sgt i64 %188, %201
  br i1 %202, label %203, label %206

203:                                              ; preds = %200
  %204 = getelementptr inbounds i8, ptr %196, i64 %201
  %205 = sub nsw i64 %188, %201
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %204, i8 0, i64 %205, i1 false)
  br label %206

206:                                              ; preds = %203, %200
  store ptr %194, ptr %47, align 8, !tbaa !78
  store ptr %196, ptr %48, align 8, !tbaa !55
  store i64 %188, ptr %11, align 8, !tbaa !74
  %207 = load i64, ptr %17, align 8, !tbaa !54
  br label %208

208:                                              ; preds = %181, %206
  %209 = phi ptr [ %183, %181 ], [ %196, %206 ]
  %210 = phi ptr [ %182, %181 ], [ %194, %206 ]
  %211 = phi i64 [ %153, %181 ], [ %207, %206 ]
  %212 = add nsw i64 %211, 1
  store i64 %212, ptr %17, align 8, !tbaa !54
  %213 = shl nsw i64 %211, 1
  %214 = getelementptr inbounds i32, ptr %210, i64 %213
  store i32 %109, ptr %214, align 4, !tbaa !22
  %215 = or disjoint i64 %213, 1
  %216 = getelementptr inbounds i32, ptr %210, i64 %215
  store i32 %113, ptr %216, align 4, !tbaa !22
  %217 = getelementptr inbounds i8, ptr %209, i64 %211
  store i8 1, ptr %217, align 1, !tbaa !57
  br label %218

218:                                              ; preds = %191, %175, %208, %150
  %219 = add i64 %111, 1
  %220 = icmp eq i64 %219, %105
  br i1 %220, label %96, label %110, !llvm.loop !81
}

declare dso_local i64 @roaring_bitmap_get_cardinality(ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_pcsr(i64 noundef %0, i64 noundef %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = tail call i64 @llvm.smax.i64(i64 %1, i64 16)
  %9 = shl nuw i64 %8, 1
  %10 = shl i64 %8, 3
  %11 = tail call noalias ptr @malloc(i64 noundef %10) #19
  %12 = shl i64 %0, 3
  %13 = add i64 %12, 8
  %14 = tail call noalias ptr @malloc(i64 noundef %13) #19
  tail call void @llvm.memset.p0.i64(ptr align 4 %11, i8 -1, i64 %10, i1 false)
  %15 = icmp sgt i64 %0, 0
  br i1 %15, label %16, label %18

16:                                               ; preds = %7
  %17 = ptrtoint ptr %3 to i64
  br label %23

18:                                               ; preds = %82, %7
  %19 = phi i64 [ 0, %7 ], [ %67, %82 ]
  %20 = phi ptr [ %11, %7 ], [ %83, %82 ]
  %21 = phi i64 [ %9, %7 ], [ %84, %82 ]
  %22 = getelementptr inbounds i64, ptr %14, i64 %0
  store i64 %19, ptr %22, align 8, !tbaa !11
  store ptr %14, ptr %4, align 8, !tbaa !17
  store ptr %20, ptr %5, align 8, !tbaa !20
  store i64 %21, ptr %6, align 8, !tbaa !11
  ret void

23:                                               ; preds = %16, %82
  %24 = phi i64 [ %84, %82 ], [ %9, %16 ]
  %25 = phi ptr [ %83, %82 ], [ %11, %16 ]
  %26 = phi i64 [ %67, %82 ], [ 0, %16 ]
  %27 = phi i64 [ %29, %82 ], [ 0, %16 ]
  %28 = ptrtoint ptr %25 to i64
  %29 = add nuw nsw i64 %27, 1
  %30 = getelementptr inbounds nuw i64, ptr %2, i64 %29
  %31 = load i64, ptr %30, align 8, !tbaa !11
  %32 = getelementptr inbounds nuw i64, ptr %2, i64 %27
  %33 = load i64, ptr %32, align 8, !tbaa !11
  %34 = sub nsw i64 %31, %33
  %35 = tail call i64 @llvm.smax.i64(i64 %34, i64 2)
  %36 = getelementptr inbounds nuw i64, ptr %14, i64 %27
  store i64 %26, ptr %36, align 8, !tbaa !11
  %37 = icmp sgt i64 %34, 0
  br i1 %37, label %38, label %65

38:                                               ; preds = %23
  %39 = getelementptr i32, ptr %3, i64 %33
  %40 = getelementptr i32, ptr %25, i64 %26
  %41 = icmp ult i64 %34, 8
  br i1 %41, label %63, label %42

42:                                               ; preds = %38
  %43 = shl i64 %26, 2
  %44 = shl i64 %33, 2
  %45 = add i64 %43, %28
  %46 = add i64 %44, %17
  %47 = sub i64 %45, %46
  %48 = icmp ult i64 %47, 32
  br i1 %48, label %63, label %49

49:                                               ; preds = %42
  %50 = and i64 %34, 9223372036854775800
  br label %51

51:                                               ; preds = %51, %49
  %52 = phi i64 [ 0, %49 ], [ %59, %51 ]
  %53 = getelementptr i32, ptr %39, i64 %52
  %54 = getelementptr i8, ptr %53, i64 16
  %55 = load <4 x i32>, ptr %53, align 4, !tbaa !22
  %56 = load <4 x i32>, ptr %54, align 4, !tbaa !22
  %57 = getelementptr i32, ptr %40, i64 %52
  %58 = getelementptr i8, ptr %57, i64 16
  store <4 x i32> %55, ptr %57, align 4, !tbaa !22
  store <4 x i32> %56, ptr %58, align 4, !tbaa !22
  %59 = add nuw i64 %52, 8
  %60 = icmp eq i64 %59, %50
  br i1 %60, label %61, label %51, !llvm.loop !82

61:                                               ; preds = %51
  %62 = icmp eq i64 %34, %50
  br i1 %62, label %65, label %63

63:                                               ; preds = %42, %38, %61
  %64 = phi i64 [ 0, %38 ], [ 0, %42 ], [ %50, %61 ]
  br label %69

65:                                               ; preds = %69, %61, %23
  %66 = add i64 %34, %26
  %67 = add i64 %66, %35
  %68 = icmp sgt i64 %67, %24
  br i1 %68, label %76, label %82

69:                                               ; preds = %63, %69
  %70 = phi i64 [ %74, %69 ], [ %64, %63 ]
  %71 = getelementptr i32, ptr %39, i64 %70
  %72 = load i32, ptr %71, align 4, !tbaa !22
  %73 = getelementptr i32, ptr %40, i64 %70
  store i32 %72, ptr %73, align 4, !tbaa !22
  %74 = add nuw nsw i64 %70, 1
  %75 = icmp eq i64 %74, %34
  br i1 %75, label %65, label %69, !llvm.loop !83

76:                                               ; preds = %65
  %77 = shl nsw i64 %67, 1
  %78 = shl i64 %67, 3
  %79 = tail call ptr @realloc(ptr noundef %25, i64 noundef %78) #20
  %80 = getelementptr inbounds i32, ptr %79, i64 %67
  %81 = shl i64 %67, 2
  tail call void @llvm.memset.p0.i64(ptr align 4 %80, i8 -1, i64 %81, i1 false)
  br label %82

82:                                               ; preds = %76, %65
  %83 = phi ptr [ %79, %76 ], [ %25, %65 ]
  %84 = phi i64 [ %77, %76 ], [ %24, %65 ]
  %85 = icmp eq i64 %29, %0
  br i1 %85, label %18, label %23, !llvm.loop !84
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #8

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #9

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
  %25 = tail call noalias ptr @calloc(i64 noundef %24, i64 noundef 4) #17
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
  %48 = load i64, ptr %47, align 8, !tbaa !11
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
  %63 = load <2 x i64>, ptr %61, align 8, !tbaa !11
  %64 = load <2 x i64>, ptr %62, align 8, !tbaa !11
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
  br i1 %74, label %75, label %55, !llvm.loop !85

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
  %97 = load i64, ptr %96, align 8, !tbaa !11
  %98 = sub nsw i64 %97, %92
  %99 = tail call i64 @llvm.smax.i64(i64 %98, i64 0)
  %100 = add nuw nsw i64 %99, %94
  %101 = icmp slt i64 %95, %44
  br i1 %101, label %91, label %84, !llvm.loop !86

102:                                              ; preds = %87
  %103 = trunc nuw nsw i64 %89 to i32
  %104 = getelementptr inbounds nuw i32, ptr %28, i64 %33
  store i32 %103, ptr %104, align 4, !tbaa !22
  %105 = add nuw nsw i64 %33, 1
  %106 = icmp eq i64 %105, %31
  br i1 %106, label %108, label %32, !llvm.loop !87

107:                                              ; preds = %87
  tail call void @free(ptr noundef %25) #18
  br label %191

108:                                              ; preds = %102
  %109 = icmp eq i64 %89, 0
  br i1 %109, label %115, label %110

110:                                              ; preds = %108
  %111 = shl nuw nsw i64 %89, 2
  %112 = tail call noalias ptr @malloc(i64 noundef %111) #19
  %113 = icmp eq ptr %112, null
  br i1 %113, label %114, label %115

114:                                              ; preds = %110
  tail call void @free(ptr noundef nonnull %25) #18
  br label %191

115:                                              ; preds = %108, %110
  %116 = phi ptr [ %112, %110 ], [ null, %108 ]
  %117 = and i64 %20, 2147483647
  %118 = getelementptr i8, ptr %116, i64 16
  br label %121

119:                                              ; preds = %138, %27
  %120 = phi ptr [ null, %27 ], [ %116, %138 ]
  store ptr %25, ptr %4, align 8, !tbaa !20
  store ptr %120, ptr %5, align 8, !tbaa !20
  store i32 %21, ptr %7, align 4, !tbaa !22
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
  %129 = load i32, ptr %128, align 4, !tbaa !22
  %130 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %131 = load i64, ptr %130, align 8, !tbaa !11
  br label %141

132:                                              ; preds = %181, %176
  %133 = phi i64 [ %158, %176 ], [ %187, %181 ]
  %134 = trunc nsw i64 %133 to i32
  br label %135

135:                                              ; preds = %132, %141
  %136 = phi i32 [ %144, %141 ], [ %134, %132 ]
  %137 = icmp slt i64 %147, %125
  br i1 %137, label %141, label %138, !llvm.loop !88

138:                                              ; preds = %135, %121
  %139 = add nuw nsw i64 %122, 1
  %140 = icmp eq i64 %139, %117
  br i1 %140, label %119, label %121, !llvm.loop !89

141:                                              ; preds = %127, %135
  %142 = phi i64 [ %149, %135 ], [ %131, %127 ]
  %143 = phi i64 [ %147, %135 ], [ %123, %127 ]
  %144 = phi i32 [ %136, %135 ], [ %129, %127 ]
  %145 = sub nuw nsw i64 %143, %123
  %146 = trunc i64 %145 to i32
  %147 = add nuw nsw i64 %143, 1
  %148 = getelementptr inbounds nuw i64, ptr %2, i64 %147
  %149 = load i64, ptr %148, align 8, !tbaa !11
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
  %170 = load <2 x i32>, ptr %168, align 4, !tbaa !22
  %171 = load <2 x i32>, ptr %169, align 4, !tbaa !22
  %172 = shufflevector <2 x i32> %160, <2 x i32> %170, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %172, ptr %166, align 4, !tbaa !22
  %173 = shufflevector <2 x i32> %160, <2 x i32> %171, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %173, ptr %167, align 4, !tbaa !22
  %174 = add nuw i64 %163, 4
  %175 = icmp eq i64 %174, %156
  br i1 %175, label %176, label %162, !llvm.loop !90

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
  store i32 %146, ptr %184, align 4, !tbaa !22
  %185 = getelementptr inbounds i32, ptr %3, i64 %183
  %186 = load i32, ptr %185, align 4, !tbaa !22
  %187 = add nsw i64 %182, 2
  %188 = getelementptr i8, ptr %184, i64 4
  store i32 %186, ptr %188, align 4, !tbaa !22
  %189 = add nsw i64 %183, 1
  %190 = icmp eq i64 %189, %149
  br i1 %190, label %132, label %181, !llvm.loop !91

191:                                              ; preds = %107, %16, %114, %119, %8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout(ptr noundef writeonly %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readnone %3, ptr nocapture noundef readnone %4, ptr noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8) local_unnamed_addr #0 {
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  %14 = alloca i64, align 8
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  %17 = alloca i64, align 8
  %18 = alloca i64, align 8
  %19 = alloca ptr, align 8
  %20 = alloca ptr, align 8
  %21 = alloca i64, align 8
  %22 = alloca ptr, align 8
  %23 = alloca ptr, align 8
  %24 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %25 = icmp sgt i32 %24, 0
  br i1 %25, label %26, label %325

26:                                               ; preds = %9
  %27 = zext nneg i32 %24 to i64
  br label %31

28:                                               ; preds = %31
  %29 = add nuw nsw i64 %32, 1
  %30 = icmp eq i64 %29, %27
  br i1 %30, label %325, label %31, !llvm.loop !34

31:                                               ; preds = %28, %26
  %32 = phi i64 [ 0, %26 ], [ %29, %28 ]
  %33 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %32
  %34 = load ptr, ptr %33, align 8, !tbaa !35
  %35 = icmp eq ptr %34, %0
  br i1 %35, label %36, label %28

36:                                               ; preds = %31
  %37 = icmp eq ptr %5, null
  br i1 %37, label %40, label %38

38:                                               ; preds = %36
  %39 = getelementptr inbounds nuw i8, ptr %33, i64 16
  store ptr %5, ptr %39, align 8, !tbaa !40
  br label %40

40:                                               ; preds = %38, %36
  %41 = icmp eq ptr %6, null
  br i1 %41, label %44, label %42

42:                                               ; preds = %40
  %43 = getelementptr inbounds nuw i8, ptr %33, i64 24
  store ptr %6, ptr %43, align 8, !tbaa !41
  br label %44

44:                                               ; preds = %42, %40
  %45 = icmp eq ptr %7, null
  br i1 %45, label %48, label %46

46:                                               ; preds = %44
  %47 = getelementptr inbounds nuw i8, ptr %33, i64 32
  store ptr %7, ptr %47, align 8, !tbaa !42
  br label %48

48:                                               ; preds = %46, %44
  %49 = getelementptr inbounds nuw i8, ptr %33, i64 16
  %50 = load ptr, ptr %49, align 8, !tbaa !40
  %51 = icmp eq ptr %50, null
  br i1 %51, label %325, label %52

52:                                               ; preds = %48
  %53 = getelementptr inbounds nuw i8, ptr %33, i64 24
  %54 = load ptr, ptr %53, align 8, !tbaa !41
  %55 = icmp eq ptr %54, null
  br i1 %55, label %325, label %56

56:                                               ; preds = %52
  %57 = getelementptr inbounds nuw i8, ptr %33, i64 32
  %58 = load ptr, ptr %57, align 8, !tbaa !42
  %59 = icmp eq ptr %58, null
  %60 = icmp ugt i32 %8, 3
  %61 = or i1 %60, %59
  br i1 %61, label %325, label %62

62:                                               ; preds = %56
  %63 = getelementptr inbounds nuw i8, ptr %33, i64 8
  %64 = load i32, ptr %63, align 8, !tbaa !43
  %65 = icmp eq i32 %64, %8
  br i1 %65, label %325, label %66

66:                                               ; preds = %62
  %67 = icmp eq i32 %8, 3
  br i1 %67, label %68, label %194

68:                                               ; preds = %66
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #18
  store ptr null, ptr %10, align 8, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %11) #18
  store ptr null, ptr %11, align 8, !tbaa !20
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %12) #18
  store i64 0, ptr %12, align 8, !tbaa !11
  switch i32 %64, label %105 [
    i32 0, label %69
    i32 1, label %81
    i32 2, label %91
  ]

69:                                               ; preds = %68
  %70 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %71 = load ptr, ptr %70, align 8, !tbaa !38
  store ptr %71, ptr %10, align 8, !tbaa !17
  %72 = getelementptr inbounds nuw i8, ptr %33, i64 88
  %73 = load ptr, ptr %72, align 8, !tbaa !39
  store ptr %73, ptr %11, align 8, !tbaa !20
  %74 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %75 = load i64, ptr %74, align 8, !tbaa !44
  %76 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %77 = load i64, ptr %76, align 8, !tbaa !45
  store i64 %77, ptr %12, align 8, !tbaa !11
  %78 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %79 = load i32, ptr %78, align 8, !tbaa !46
  %80 = icmp ne i32 %79, 0
  br label %105

81:                                               ; preds = %68
  %82 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %83 = load i64, ptr %82, align 8, !tbaa !44
  %84 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %85 = load ptr, ptr %84, align 8, !tbaa !47
  %86 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %87 = load ptr, ptr %86, align 8, !tbaa !48
  call void @convert_pcsr_to_csr(i64 noundef %83, i64 poison, ptr noundef %85, ptr noundef %87, ptr noundef nonnull %10, ptr noundef nonnull %11, ptr noundef nonnull %12)
  %88 = load i64, ptr %82, align 8, !tbaa !44
  %89 = load ptr, ptr %84, align 8, !tbaa !47
  tail call void @free(ptr noundef %89) #18
  store ptr null, ptr %84, align 8, !tbaa !47
  %90 = load ptr, ptr %86, align 8, !tbaa !48
  tail call void @free(ptr noundef %90) #18
  store ptr null, ptr %86, align 8, !tbaa !48
  br label %105

91:                                               ; preds = %68
  %92 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %93 = load i64, ptr %92, align 8, !tbaa !44
  %94 = getelementptr inbounds nuw i8, ptr %33, i64 164
  %95 = load i32, ptr %94, align 4, !tbaa !49
  %96 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %97 = load i32, ptr %96, align 8, !tbaa !50
  %98 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %99 = load ptr, ptr %98, align 8, !tbaa !51
  %100 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %101 = load ptr, ptr %100, align 8, !tbaa !52
  call void @convert_bcsr_to_csr(i64 noundef %93, i32 noundef %95, i32 noundef %97, ptr noundef %99, ptr noundef %101, ptr noundef nonnull %10, ptr noundef nonnull %11, ptr noundef nonnull %12)
  %102 = load i64, ptr %92, align 8, !tbaa !44
  %103 = load ptr, ptr %98, align 8, !tbaa !51
  call void @free(ptr noundef %103) #18
  store ptr null, ptr %98, align 8, !tbaa !51
  %104 = load ptr, ptr %100, align 8, !tbaa !52
  call void @free(ptr noundef %104) #18
  store ptr null, ptr %100, align 8, !tbaa !52
  br label %105

105:                                              ; preds = %68, %81, %91, %69
  %106 = phi i1 [ %80, %69 ], [ true, %81 ], [ true, %91 ], [ false, %68 ]
  %107 = phi i64 [ %75, %69 ], [ %88, %81 ], [ %102, %91 ], [ 0, %68 ]
  %108 = load ptr, ptr %10, align 8, !tbaa !17
  %109 = icmp ne ptr %108, null
  %110 = load ptr, ptr %11, align 8
  %111 = icmp ne ptr %110, null
  %112 = select i1 %109, i1 %111, i1 false
  br i1 %112, label %113, label %115

113:                                              ; preds = %105
  call fastcc void @rebuild_sets_from_csr_meta(ptr noundef %33, ptr noundef %108, ptr noundef %110, i64 noundef %107)
  %114 = load ptr, ptr %10, align 8
  br label %115

115:                                              ; preds = %113, %105
  %116 = phi ptr [ %114, %113 ], [ %108, %105 ]
  %117 = icmp ne ptr %116, null
  %118 = select i1 %106, i1 %117, i1 false
  br i1 %118, label %119, label %120

119:                                              ; preds = %115
  call void @free(ptr noundef nonnull %116) #18
  br label %120

120:                                              ; preds = %119, %115
  %121 = load ptr, ptr %11, align 8
  %122 = icmp ne ptr %121, null
  %123 = select i1 %106, i1 %122, i1 false
  br i1 %123, label %124, label %125

124:                                              ; preds = %120
  call void @free(ptr noundef nonnull %121) #18
  br label %125

125:                                              ; preds = %124, %120
  %126 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %127 = getelementptr inbounds nuw i8, ptr %33, i64 112
  store i32 0, ptr %127, align 8, !tbaa !46
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %126, i8 0, i64 16, i1 false)
  %128 = load ptr, ptr %49, align 8, !tbaa !40
  %129 = call i64 @roaring_bitmap_get_cardinality(ptr noundef %128) #18
  store i64 0, ptr %12, align 8, !tbaa !11
  %130 = load ptr, ptr %53, align 8, !tbaa !41
  %131 = getelementptr inbounds nuw i8, ptr %33, i64 40
  %132 = load i64, ptr %131, align 8, !tbaa !53
  %133 = icmp sgt i64 %132, 0
  br i1 %133, label %146, label %134

134:                                              ; preds = %153, %125
  %135 = getelementptr inbounds nuw i8, ptr %33, i64 64
  %136 = load i64, ptr %135, align 8, !tbaa !54
  %137 = icmp sgt i64 %136, 0
  %138 = load i64, ptr %12, align 8
  br i1 %137, label %139, label %168

139:                                              ; preds = %134
  %140 = getelementptr inbounds nuw i8, ptr %33, i64 56
  %141 = load ptr, ptr %140, align 8, !tbaa !55
  %142 = and i64 %136, 1
  %143 = icmp eq i64 %136, 1
  br i1 %143, label %157, label %144

144:                                              ; preds = %139
  %145 = and i64 %136, 9223372036854775806
  br label %172

146:                                              ; preds = %125, %153
  %147 = phi i64 [ %154, %153 ], [ 0, %125 ]
  %148 = trunc i64 %147 to i32
  %149 = call zeroext i1 @roaring_bitmap_contains(ptr noundef %130, i32 noundef %148) #18
  br i1 %149, label %150, label %153

150:                                              ; preds = %146
  %151 = load i64, ptr %12, align 8, !tbaa !11
  %152 = add nsw i64 %151, 1
  store i64 %152, ptr %12, align 8, !tbaa !11
  br label %153

153:                                              ; preds = %146, %150
  %154 = add nuw nsw i64 %147, 1
  %155 = load i64, ptr %131, align 8, !tbaa !53
  %156 = icmp slt i64 %154, %155
  br i1 %156, label %146, label %134, !llvm.loop !92

157:                                              ; preds = %189, %139
  %158 = phi i64 [ poison, %139 ], [ %190, %189 ]
  %159 = phi i64 [ 0, %139 ], [ %191, %189 ]
  %160 = phi i64 [ %138, %139 ], [ %190, %189 ]
  %161 = icmp eq i64 %142, 0
  br i1 %161, label %168, label %162

162:                                              ; preds = %157
  %163 = getelementptr inbounds nuw i8, ptr %141, i64 %159
  %164 = load i8, ptr %163, align 1, !tbaa !57
  %165 = icmp eq i8 %164, 0
  br i1 %165, label %168, label %166

166:                                              ; preds = %162
  %167 = add nsw i64 %160, 1
  store i64 %167, ptr %12, align 8, !tbaa !11
  br label %168

168:                                              ; preds = %157, %166, %162, %134
  %169 = phi i64 [ %138, %134 ], [ %158, %157 ], [ %160, %162 ], [ %167, %166 ]
  store i64 %129, ptr %0, align 8, !tbaa !11
  %170 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %169, ptr %170, align 8, !tbaa !11
  %171 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %171, i8 0, i64 16, i1 false)
  store i32 3, ptr %63, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %12) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %11) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #18
  br label %325

172:                                              ; preds = %189, %144
  %173 = phi i64 [ 0, %144 ], [ %191, %189 ]
  %174 = phi i64 [ %138, %144 ], [ %190, %189 ]
  %175 = phi i64 [ 0, %144 ], [ %192, %189 ]
  %176 = getelementptr inbounds nuw i8, ptr %141, i64 %173
  %177 = load i8, ptr %176, align 1, !tbaa !57
  %178 = icmp eq i8 %177, 0
  br i1 %178, label %181, label %179

179:                                              ; preds = %172
  %180 = add nsw i64 %174, 1
  store i64 %180, ptr %12, align 8, !tbaa !11
  br label %181

181:                                              ; preds = %172, %179
  %182 = phi i64 [ %174, %172 ], [ %180, %179 ]
  %183 = or disjoint i64 %173, 1
  %184 = getelementptr inbounds nuw i8, ptr %141, i64 %183
  %185 = load i8, ptr %184, align 1, !tbaa !57
  %186 = icmp eq i8 %185, 0
  br i1 %186, label %189, label %187

187:                                              ; preds = %181
  %188 = add nsw i64 %182, 1
  store i64 %188, ptr %12, align 8, !tbaa !11
  br label %189

189:                                              ; preds = %187, %181
  %190 = phi i64 [ %182, %181 ], [ %188, %187 ]
  %191 = add nuw nsw i64 %173, 2
  %192 = add i64 %175, 2
  %193 = icmp eq i64 %192, %145
  br i1 %193, label %157, label %172, !llvm.loop !93

194:                                              ; preds = %66
  %195 = icmp eq i32 %64, 3
  br i1 %195, label %196, label %219

196:                                              ; preds = %194
  %197 = getelementptr inbounds nuw i8, ptr %33, i64 96
  switch i32 %8, label %211 [
    i32 0, label %198
    i32 1, label %203
  ]

198:                                              ; preds = %196
  %199 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %200 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %201 = getelementptr inbounds nuw i8, ptr %33, i64 88
  %202 = getelementptr inbounds nuw i8, ptr %33, i64 104
  tail call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %199, ptr noundef %200, ptr noundef %201, ptr noundef %202)
  store i32 0, ptr %63, align 8, !tbaa !43
  br label %300

203:                                              ; preds = %196
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %13) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %14) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %15) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %16) #18
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %14, ptr noundef %15, ptr noundef %16, ptr noundef %13)
  %204 = load i64, ptr %14, align 8, !tbaa !11
  store i64 %204, ptr %197, align 8, !tbaa !44
  %205 = load i64, ptr %13, align 8, !tbaa !11
  %206 = load ptr, ptr %15, align 8, !tbaa !17
  %207 = load ptr, ptr %16, align 8, !tbaa !20
  %208 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %209 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %210 = getelementptr inbounds nuw i8, ptr %33, i64 136
  tail call void @convert_csr_to_pcsr(i64 noundef %204, i64 noundef %205, ptr noundef %206, ptr noundef %207, ptr noundef nonnull %208, ptr noundef nonnull %209, ptr noundef nonnull %210)
  tail call void @free(ptr noundef %206) #18
  tail call void @free(ptr noundef %207) #18
  store i32 1, ptr %63, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %16) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %15) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %14) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %13) #18
  br label %309

211:                                              ; preds = %196
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %17) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %18) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %19) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %20) #18
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %18, ptr noundef %19, ptr noundef %20, ptr noundef %17)
  %212 = load i64, ptr %18, align 8, !tbaa !11
  store i64 %212, ptr %197, align 8, !tbaa !44
  %213 = getelementptr inbounds nuw i8, ptr %33, i64 160
  store i32 64, ptr %213, align 8, !tbaa !50
  %214 = load ptr, ptr %19, align 8, !tbaa !17
  %215 = load ptr, ptr %20, align 8, !tbaa !20
  %216 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %217 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %218 = getelementptr inbounds nuw i8, ptr %33, i64 164
  tail call void @convert_csr_to_bcsr(i64 noundef %212, i64 poison, ptr noundef %214, ptr noundef %215, ptr noundef nonnull %216, ptr noundef nonnull %217, i32 noundef 64, ptr noundef nonnull %218)
  tail call void @free(ptr noundef %214) #18
  tail call void @free(ptr noundef %215) #18
  store i32 2, ptr %63, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %20) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %19) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %18) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %17) #18
  br label %318

219:                                              ; preds = %194
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %21) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %22) #18
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %23) #18
  switch i32 %64, label %242 [
    i32 1, label %220
    i32 2, label %229
  ]

220:                                              ; preds = %219
  %221 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %222 = load i64, ptr %221, align 8, !tbaa !44
  %223 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %224 = load ptr, ptr %223, align 8, !tbaa !47
  %225 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %226 = load ptr, ptr %225, align 8, !tbaa !48
  call void @convert_pcsr_to_csr(i64 noundef %222, i64 poison, ptr noundef %224, ptr noundef %226, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %227 = load ptr, ptr %223, align 8, !tbaa !47
  tail call void @free(ptr noundef %227) #18
  store ptr null, ptr %223, align 8, !tbaa !47
  %228 = load ptr, ptr %225, align 8, !tbaa !48
  tail call void @free(ptr noundef %228) #18
  store ptr null, ptr %225, align 8, !tbaa !48
  br label %251

229:                                              ; preds = %219
  %230 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %231 = load i64, ptr %230, align 8, !tbaa !44
  %232 = getelementptr inbounds nuw i8, ptr %33, i64 164
  %233 = load i32, ptr %232, align 4, !tbaa !49
  %234 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %235 = load i32, ptr %234, align 8, !tbaa !50
  %236 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %237 = load ptr, ptr %236, align 8, !tbaa !51
  %238 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %239 = load ptr, ptr %238, align 8, !tbaa !52
  call void @convert_bcsr_to_csr(i64 noundef %231, i32 noundef %233, i32 noundef %235, ptr noundef %237, ptr noundef %239, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %240 = load ptr, ptr %236, align 8, !tbaa !51
  call void @free(ptr noundef %240) #18
  store ptr null, ptr %236, align 8, !tbaa !51
  %241 = load ptr, ptr %238, align 8, !tbaa !52
  call void @free(ptr noundef %241) #18
  store ptr null, ptr %238, align 8, !tbaa !52
  br label %251

242:                                              ; preds = %219
  %243 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %244 = load i64, ptr %243, align 8, !tbaa !45
  store i64 %244, ptr %21, align 8, !tbaa !11
  %245 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %246 = load ptr, ptr %245, align 8, !tbaa !38
  store ptr %246, ptr %22, align 8, !tbaa !17
  %247 = getelementptr inbounds nuw i8, ptr %33, i64 88
  %248 = load ptr, ptr %247, align 8, !tbaa !39
  store ptr %248, ptr %23, align 8, !tbaa !20
  %249 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %250 = load i32, ptr %249, align 8, !tbaa !46
  br label %251

251:                                              ; preds = %229, %242, %220
  %252 = phi i32 [ 1, %220 ], [ 1, %229 ], [ %250, %242 ]
  switch i32 %8, label %271 [
    i32 0, label %291
    i32 1, label %253
  ]

253:                                              ; preds = %251
  %254 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %255 = load i64, ptr %254, align 8, !tbaa !44
  %256 = load i64, ptr %21, align 8, !tbaa !11
  %257 = load ptr, ptr %22, align 8, !tbaa !17
  %258 = load ptr, ptr %23, align 8, !tbaa !20
  %259 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %260 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %261 = getelementptr inbounds nuw i8, ptr %33, i64 136
  call void @convert_csr_to_pcsr(i64 noundef %255, i64 noundef %256, ptr noundef %257, ptr noundef %258, ptr noundef nonnull %259, ptr noundef nonnull %260, ptr noundef nonnull %261)
  %262 = icmp eq i32 %252, 0
  br i1 %262, label %266, label %263

263:                                              ; preds = %253
  %264 = load ptr, ptr %22, align 8, !tbaa !17
  call void @free(ptr noundef %264) #18
  %265 = load ptr, ptr %23, align 8, !tbaa !20
  call void @free(ptr noundef %265) #18
  br label %266

266:                                              ; preds = %263, %253
  %267 = load i32, ptr %63, align 8, !tbaa !43
  %268 = icmp eq i32 %267, 0
  br i1 %268, label %269, label %290

269:                                              ; preds = %266
  %270 = getelementptr inbounds nuw i8, ptr %33, i64 80
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %270, i8 0, i64 16, i1 false)
  br label %290

271:                                              ; preds = %251
  %272 = getelementptr inbounds nuw i8, ptr %33, i64 160
  store i32 64, ptr %272, align 8, !tbaa !50
  %273 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %274 = load i64, ptr %273, align 8, !tbaa !44
  %275 = load ptr, ptr %22, align 8, !tbaa !17
  %276 = load ptr, ptr %23, align 8, !tbaa !20
  %277 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %278 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %279 = getelementptr inbounds nuw i8, ptr %33, i64 164
  call void @convert_csr_to_bcsr(i64 noundef %274, i64 poison, ptr noundef %275, ptr noundef %276, ptr noundef nonnull %277, ptr noundef nonnull %278, i32 noundef 64, ptr noundef nonnull %279)
  %280 = icmp eq i32 %252, 0
  br i1 %280, label %284, label %281

281:                                              ; preds = %271
  %282 = load ptr, ptr %22, align 8, !tbaa !17
  call void @free(ptr noundef %282) #18
  %283 = load ptr, ptr %23, align 8, !tbaa !20
  call void @free(ptr noundef %283) #18
  br label %284

284:                                              ; preds = %281, %271
  %285 = load i32, ptr %63, align 8, !tbaa !43
  %286 = icmp eq i32 %285, 0
  br i1 %286, label %287, label %289

287:                                              ; preds = %284
  %288 = getelementptr inbounds nuw i8, ptr %33, i64 80
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %288, i8 0, i64 16, i1 false)
  br label %289

289:                                              ; preds = %287, %284
  store i32 %8, ptr %63, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #18
  br label %318

290:                                              ; preds = %266, %269
  store i32 %8, ptr %63, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #18
  br label %309

291:                                              ; preds = %251
  %292 = load i64, ptr %21, align 8, !tbaa !11
  %293 = getelementptr inbounds nuw i8, ptr %33, i64 104
  store i64 %292, ptr %293, align 8, !tbaa !45
  %294 = load ptr, ptr %22, align 8, !tbaa !17
  %295 = getelementptr inbounds nuw i8, ptr %33, i64 80
  store ptr %294, ptr %295, align 8, !tbaa !38
  %296 = load ptr, ptr %23, align 8, !tbaa !20
  %297 = getelementptr inbounds nuw i8, ptr %33, i64 88
  store ptr %296, ptr %297, align 8, !tbaa !39
  %298 = getelementptr inbounds nuw i8, ptr %33, i64 112
  store i32 %252, ptr %298, align 8, !tbaa !46
  store i32 %8, ptr %63, align 8, !tbaa !43
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #18
  %299 = getelementptr inbounds nuw i8, ptr %33, i64 96
  br label %300

300:                                              ; preds = %291, %198
  %301 = phi ptr [ %299, %291 ], [ %197, %198 ]
  %302 = load i64, ptr %301, align 8, !tbaa !44
  store i64 %302, ptr %0, align 8, !tbaa !11
  %303 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %304 = load i64, ptr %303, align 8, !tbaa !45
  %305 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %304, ptr %305, align 8, !tbaa !11
  %306 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %307 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %308 = load <2 x ptr>, ptr %306, align 8, !tbaa !94
  store <2 x ptr> %308, ptr %307, align 8, !tbaa !94
  br label %325

309:                                              ; preds = %290, %203
  %310 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %311 = load i64, ptr %310, align 8, !tbaa !44
  store i64 %311, ptr %0, align 8, !tbaa !11
  %312 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %313 = load i64, ptr %312, align 8, !tbaa !45
  %314 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %313, ptr %314, align 8, !tbaa !11
  %315 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %316 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %317 = load <2 x ptr>, ptr %315, align 8, !tbaa !94
  store <2 x ptr> %317, ptr %316, align 8, !tbaa !94
  br label %325

318:                                              ; preds = %289, %211
  %319 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %320 = load i64, ptr %319, align 8, !tbaa !44
  store i64 %320, ptr %0, align 8, !tbaa !11
  %321 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %322 = load i64, ptr %321, align 8, !tbaa !45
  %323 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %322, ptr %323, align 8, !tbaa !11
  %324 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %324, i8 0, i64 16, i1 false)
  br label %325

325:                                              ; preds = %28, %9, %300, %318, %309, %62, %48, %52, %56, %168
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @build_csr_from_meta(ptr nocapture noundef nonnull %0, ptr nocapture noundef nonnull writeonly %1, ptr nocapture noundef nonnull writeonly %2, ptr nocapture noundef nonnull writeonly %3, ptr nocapture noundef nonnull writeonly %4) unnamed_addr #0 {
  %6 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %7 = load ptr, ptr %6, align 8, !tbaa !40
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %9 = load ptr, ptr %8, align 8, !tbaa !41
  %10 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %11 = load ptr, ptr %10, align 8, !tbaa !42
  %12 = tail call i64 @roaring_bitmap_get_cardinality(ptr noundef %7) #18
  %13 = tail call i64 @llvm.smax.i64(i64 %12, i64 0)
  %14 = icmp slt i64 %12, 1
  br i1 %14, label %25, label %15

15:                                               ; preds = %5
  %16 = shl i64 %13, 2
  %17 = tail call noalias ptr @malloc(i64 noundef %16) #19
  br label %19

18:                                               ; preds = %19
  tail call void @qsort(ptr noundef nonnull %17, i64 noundef %13, i64 noundef 4, ptr noundef nonnull @cmp_i32) #18
  br label %25

19:                                               ; preds = %15, %19
  %20 = phi i64 [ 0, %15 ], [ %23, %19 ]
  %21 = tail call i32 @roaring_bitmap_get_at_index(ptr noundef %7, i64 noundef %20) #18
  %22 = getelementptr inbounds nuw i32, ptr %17, i64 %20
  store i32 %21, ptr %22, align 4, !tbaa !22
  %23 = add nuw nsw i64 %20, 1
  %24 = icmp eq i64 %23, %12
  br i1 %24, label %18, label %19, !llvm.loop !95

25:                                               ; preds = %18, %5
  %26 = phi ptr [ %17, %18 ], [ null, %5 ]
  %27 = trunc i64 %13 to i32
  %28 = shl i32 %27, 1
  br label %29

29:                                               ; preds = %29, %25
  %30 = phi i32 [ 1, %25 ], [ %32, %29 ]
  %31 = icmp sgt i32 %30, %28
  %32 = shl i32 %30, 1
  br i1 %31, label %33, label %29, !llvm.loop !24

33:                                               ; preds = %29
  %34 = sext i32 %30 to i64
  %35 = tail call noalias ptr @calloc(i64 noundef %34, i64 noundef 12) #17
  %36 = icmp sgt i64 %12, 0
  br i1 %36, label %37, label %39

37:                                               ; preds = %33
  %38 = add nsw i32 %30, -1
  br label %49

39:                                               ; preds = %70, %33
  %40 = add nuw nsw i64 %13, 1
  %41 = tail call noalias ptr @calloc(i64 noundef %40, i64 noundef 8) #17
  %42 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %43 = getelementptr inbounds nuw i8, ptr %41, i64 8
  %44 = load i64, ptr %42, align 8, !tbaa !53
  %45 = icmp sgt i64 %44, 0
  br i1 %45, label %46, label %78

46:                                               ; preds = %39
  %47 = icmp sgt i32 %30, 0
  %48 = add nsw i32 %30, -1
  br label %90

49:                                               ; preds = %37, %70
  %50 = phi i64 [ 0, %37 ], [ %74, %70 ]
  %51 = getelementptr inbounds nuw i32, ptr %26, i64 %50
  %52 = load i32, ptr %51, align 4, !tbaa !22
  %53 = lshr i32 %52, 16
  %54 = xor i32 %53, %52
  %55 = mul i32 %54, 2146121005
  %56 = lshr i32 %55, 15
  %57 = xor i32 %56, %55
  %58 = mul i32 %57, -2073254261
  %59 = lshr i32 %58, 16
  %60 = xor i32 %59, %58
  br label %61

61:                                               ; preds = %61, %49
  %62 = phi i32 [ %60, %49 ], [ %69, %61 ]
  %63 = and i32 %62, %38
  %64 = zext i32 %63 to i64
  %65 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %64
  %66 = getelementptr inbounds nuw i8, ptr %65, i64 8
  %67 = load i8, ptr %66, align 4, !tbaa !96
  %68 = icmp eq i8 %67, 0
  %69 = add i32 %63, 1
  br i1 %68, label %70, label %61, !llvm.loop !98

70:                                               ; preds = %61
  %71 = getelementptr inbounds nuw i8, ptr %65, i64 8
  store i8 1, ptr %71, align 4, !tbaa !96
  store i32 %52, ptr %65, align 4, !tbaa !99
  %72 = trunc i64 %50 to i32
  %73 = getelementptr inbounds nuw i8, ptr %65, i64 4
  store i32 %72, ptr %73, align 4, !tbaa !100
  %74 = add nuw nsw i64 %50, 1
  %75 = icmp eq i64 %74, %12
  br i1 %75, label %39, label %49, !llvm.loop !101

76:                                               ; preds = %172
  %77 = icmp sgt i64 %175, 0
  br label %78

78:                                               ; preds = %76, %39
  %79 = phi i1 [ false, %39 ], [ %77, %76 ]
  %80 = phi i64 [ 0, %39 ], [ %173, %76 ]
  %81 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %82 = load i64, ptr %81, align 8, !tbaa !54
  %83 = icmp sgt i64 %82, 0
  br i1 %83, label %84, label %177

84:                                               ; preds = %78
  %85 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %86 = load ptr, ptr %85, align 8, !tbaa !55
  %87 = icmp sgt i32 %30, 0
  %88 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %89 = add nsw i32 %30, -1
  br label %188

90:                                               ; preds = %46, %172
  %91 = phi i64 [ 0, %46 ], [ %174, %172 ]
  %92 = phi i64 [ 0, %46 ], [ %173, %172 ]
  %93 = trunc i64 %91 to i32
  %94 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %93) #18
  %95 = and i1 %47, %94
  br i1 %95, label %96, label %172

96:                                               ; preds = %90
  %97 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %91
  %98 = load i32, ptr %97, align 4, !tbaa !3
  %99 = lshr i32 %98, 16
  %100 = xor i32 %99, %98
  %101 = mul i32 %100, 2146121005
  %102 = lshr i32 %101, 15
  %103 = xor i32 %102, %101
  %104 = mul i32 %103, -2073254261
  %105 = lshr i32 %104, 16
  %106 = xor i32 %105, %104
  %107 = and i32 %106, %48
  %108 = zext nneg i32 %107 to i64
  %109 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %108
  %110 = getelementptr inbounds nuw i8, ptr %109, i64 8
  %111 = load i8, ptr %110, align 4, !tbaa !96
  %112 = icmp eq i8 %111, 0
  br i1 %112, label %129, label %121

113:                                              ; preds = %121
  %114 = add nuw nsw i32 %123, 1
  %115 = and i32 %114, %48
  %116 = zext nneg i32 %115 to i64
  %117 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %116
  %118 = getelementptr inbounds nuw i8, ptr %117, i64 8
  %119 = load i8, ptr %118, align 4, !tbaa !96
  %120 = icmp eq i8 %119, 0
  br i1 %120, label %129, label %121, !llvm.loop !102

121:                                              ; preds = %96, %113
  %122 = phi ptr [ %117, %113 ], [ %109, %96 ]
  %123 = phi i32 [ %115, %113 ], [ %107, %96 ]
  %124 = load i32, ptr %122, align 4, !tbaa !99
  %125 = icmp eq i32 %124, %98
  br i1 %125, label %126, label %113

126:                                              ; preds = %121
  %127 = getelementptr inbounds nuw i8, ptr %122, i64 4
  %128 = load i32, ptr %127, align 4, !tbaa !100
  br label %129

129:                                              ; preds = %113, %96, %126
  %130 = phi i32 [ %128, %126 ], [ -1, %96 ], [ -1, %113 ]
  %131 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %91, i32 1
  %132 = load i32, ptr %131, align 4, !tbaa !8
  %133 = lshr i32 %132, 16
  %134 = xor i32 %133, %132
  %135 = mul i32 %134, 2146121005
  %136 = lshr i32 %135, 15
  %137 = xor i32 %136, %135
  %138 = mul i32 %137, -2073254261
  %139 = lshr i32 %138, 16
  %140 = xor i32 %139, %138
  %141 = and i32 %140, %48
  %142 = zext nneg i32 %141 to i64
  %143 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %142
  %144 = getelementptr inbounds nuw i8, ptr %143, i64 8
  %145 = load i8, ptr %144, align 4, !tbaa !96
  %146 = icmp eq i8 %145, 0
  br i1 %146, label %172, label %155

147:                                              ; preds = %155
  %148 = add nuw nsw i32 %157, 1
  %149 = and i32 %148, %48
  %150 = zext nneg i32 %149 to i64
  %151 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %150
  %152 = getelementptr inbounds nuw i8, ptr %151, i64 8
  %153 = load i8, ptr %152, align 4, !tbaa !96
  %154 = icmp eq i8 %153, 0
  br i1 %154, label %172, label %155, !llvm.loop !103

155:                                              ; preds = %129, %147
  %156 = phi ptr [ %151, %147 ], [ %143, %129 ]
  %157 = phi i32 [ %149, %147 ], [ %141, %129 ]
  %158 = load i32, ptr %156, align 4, !tbaa !99
  %159 = icmp eq i32 %158, %132
  br i1 %159, label %160, label %147

160:                                              ; preds = %155
  %161 = getelementptr inbounds nuw i8, ptr %156, i64 4
  %162 = load i32, ptr %161, align 4, !tbaa !100
  %163 = icmp sgt i32 %162, -1
  %164 = icmp sgt i32 %130, -1
  %165 = select i1 %164, i1 %163, i1 false
  br i1 %165, label %166, label %172

166:                                              ; preds = %160
  %167 = zext nneg i32 %130 to i64
  %168 = getelementptr inbounds nuw i64, ptr %43, i64 %167
  %169 = load i64, ptr %168, align 8, !tbaa !11
  %170 = add nsw i64 %169, 1
  store i64 %170, ptr %168, align 8, !tbaa !11
  %171 = add nsw i64 %92, 1
  br label %172

172:                                              ; preds = %147, %129, %160, %166, %90
  %173 = phi i64 [ %92, %90 ], [ %171, %166 ], [ %92, %160 ], [ %92, %129 ], [ %92, %147 ]
  %174 = add nuw nsw i64 %91, 1
  %175 = load i64, ptr %42, align 8, !tbaa !53
  %176 = icmp slt i64 %174, %175
  br i1 %176, label %90, label %76, !llvm.loop !104

177:                                              ; preds = %275, %78
  %178 = phi i64 [ %80, %78 ], [ %276, %275 ]
  br i1 %14, label %293, label %179

179:                                              ; preds = %177
  %180 = load i64, ptr %41, align 8
  %181 = and i64 %12, 3
  %182 = icmp ult i64 %12, 4
  br i1 %182, label %279, label %183

183:                                              ; preds = %179
  %184 = and i64 %12, 9223372036854775804
  %185 = getelementptr i8, ptr %41, i64 8
  %186 = getelementptr i8, ptr %41, i64 16
  %187 = getelementptr i8, ptr %41, i64 24
  br label %295

188:                                              ; preds = %84, %275
  %189 = phi i64 [ 0, %84 ], [ %277, %275 ]
  %190 = phi i64 [ %80, %84 ], [ %276, %275 ]
  %191 = getelementptr inbounds nuw i8, ptr %86, i64 %189
  %192 = load i8, ptr %191, align 1, !tbaa !57
  %193 = icmp ne i8 %192, 0
  %194 = and i1 %87, %193
  br i1 %194, label %195, label %275

195:                                              ; preds = %188
  %196 = load ptr, ptr %88, align 8, !tbaa !78
  %197 = shl nuw nsw i64 %189, 3
  %198 = getelementptr inbounds nuw i8, ptr %196, i64 %197
  %199 = load i32, ptr %198, align 4, !tbaa !22
  %200 = lshr i32 %199, 16
  %201 = xor i32 %200, %199
  %202 = mul i32 %201, 2146121005
  %203 = lshr i32 %202, 15
  %204 = xor i32 %203, %202
  %205 = mul i32 %204, -2073254261
  %206 = lshr i32 %205, 16
  %207 = xor i32 %206, %205
  %208 = and i32 %207, %89
  %209 = zext nneg i32 %208 to i64
  %210 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %209
  %211 = getelementptr inbounds nuw i8, ptr %210, i64 8
  %212 = load i8, ptr %211, align 4, !tbaa !96
  %213 = icmp eq i8 %212, 0
  br i1 %213, label %230, label %222

214:                                              ; preds = %222
  %215 = add nuw nsw i32 %224, 1
  %216 = and i32 %215, %89
  %217 = zext nneg i32 %216 to i64
  %218 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %217
  %219 = getelementptr inbounds nuw i8, ptr %218, i64 8
  %220 = load i8, ptr %219, align 4, !tbaa !96
  %221 = icmp eq i8 %220, 0
  br i1 %221, label %230, label %222, !llvm.loop !105

222:                                              ; preds = %195, %214
  %223 = phi ptr [ %218, %214 ], [ %210, %195 ]
  %224 = phi i32 [ %216, %214 ], [ %208, %195 ]
  %225 = load i32, ptr %223, align 4, !tbaa !99
  %226 = icmp eq i32 %225, %199
  br i1 %226, label %227, label %214

227:                                              ; preds = %222
  %228 = getelementptr inbounds nuw i8, ptr %223, i64 4
  %229 = load i32, ptr %228, align 4, !tbaa !100
  br label %230

230:                                              ; preds = %214, %195, %227
  %231 = phi i32 [ %229, %227 ], [ -1, %195 ], [ -1, %214 ]
  %232 = shl nuw nsw i64 %189, 1
  %233 = or disjoint i64 %232, 1
  %234 = getelementptr inbounds nuw i32, ptr %196, i64 %233
  %235 = load i32, ptr %234, align 4, !tbaa !22
  %236 = lshr i32 %235, 16
  %237 = xor i32 %236, %235
  %238 = mul i32 %237, 2146121005
  %239 = lshr i32 %238, 15
  %240 = xor i32 %239, %238
  %241 = mul i32 %240, -2073254261
  %242 = lshr i32 %241, 16
  %243 = xor i32 %242, %241
  %244 = and i32 %243, %89
  %245 = zext nneg i32 %244 to i64
  %246 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %245
  %247 = getelementptr inbounds nuw i8, ptr %246, i64 8
  %248 = load i8, ptr %247, align 4, !tbaa !96
  %249 = icmp eq i8 %248, 0
  br i1 %249, label %275, label %258

250:                                              ; preds = %258
  %251 = add nuw nsw i32 %260, 1
  %252 = and i32 %251, %89
  %253 = zext nneg i32 %252 to i64
  %254 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %253
  %255 = getelementptr inbounds nuw i8, ptr %254, i64 8
  %256 = load i8, ptr %255, align 4, !tbaa !96
  %257 = icmp eq i8 %256, 0
  br i1 %257, label %275, label %258, !llvm.loop !106

258:                                              ; preds = %230, %250
  %259 = phi ptr [ %254, %250 ], [ %246, %230 ]
  %260 = phi i32 [ %252, %250 ], [ %244, %230 ]
  %261 = load i32, ptr %259, align 4, !tbaa !99
  %262 = icmp eq i32 %261, %235
  br i1 %262, label %263, label %250

263:                                              ; preds = %258
  %264 = getelementptr inbounds nuw i8, ptr %259, i64 4
  %265 = load i32, ptr %264, align 4, !tbaa !100
  %266 = icmp sgt i32 %265, -1
  %267 = icmp sgt i32 %231, -1
  %268 = select i1 %267, i1 %266, i1 false
  br i1 %268, label %269, label %275

269:                                              ; preds = %263
  %270 = zext nneg i32 %231 to i64
  %271 = getelementptr inbounds nuw i64, ptr %43, i64 %270
  %272 = load i64, ptr %271, align 8, !tbaa !11
  %273 = add nsw i64 %272, 1
  store i64 %273, ptr %271, align 8, !tbaa !11
  %274 = add nsw i64 %190, 1
  br label %275

275:                                              ; preds = %250, %230, %263, %269, %188
  %276 = phi i64 [ %190, %188 ], [ %274, %269 ], [ %190, %263 ], [ %190, %230 ], [ %190, %250 ]
  %277 = add nuw nsw i64 %189, 1
  %278 = icmp eq i64 %277, %82
  br i1 %278, label %177, label %188, !llvm.loop !107

279:                                              ; preds = %295, %179
  %280 = phi i64 [ %180, %179 ], [ %310, %295 ]
  %281 = phi i64 [ 1, %179 ], [ %311, %295 ]
  %282 = icmp eq i64 %181, 0
  br i1 %282, label %293, label %283

283:                                              ; preds = %279, %283
  %284 = phi i64 [ %289, %283 ], [ %280, %279 ]
  %285 = phi i64 [ %290, %283 ], [ %281, %279 ]
  %286 = phi i64 [ %291, %283 ], [ 0, %279 ]
  %287 = getelementptr i64, ptr %41, i64 %285
  %288 = load i64, ptr %287, align 8, !tbaa !11
  %289 = add nsw i64 %288, %284
  store i64 %289, ptr %287, align 8, !tbaa !11
  %290 = add nuw i64 %285, 1
  %291 = add i64 %286, 1
  %292 = icmp eq i64 %291, %181
  br i1 %292, label %293, label %283, !llvm.loop !108

293:                                              ; preds = %279, %283, %177
  %294 = icmp sgt i64 %178, 0
  br i1 %294, label %314, label %317

295:                                              ; preds = %295, %183
  %296 = phi i64 [ %180, %183 ], [ %310, %295 ]
  %297 = phi i64 [ 1, %183 ], [ %311, %295 ]
  %298 = phi i64 [ 0, %183 ], [ %312, %295 ]
  %299 = getelementptr i64, ptr %41, i64 %297
  %300 = load i64, ptr %299, align 8, !tbaa !11
  %301 = add nsw i64 %300, %296
  store i64 %301, ptr %299, align 8, !tbaa !11
  %302 = getelementptr i64, ptr %185, i64 %297
  %303 = load i64, ptr %302, align 8, !tbaa !11
  %304 = add nsw i64 %303, %301
  store i64 %304, ptr %302, align 8, !tbaa !11
  %305 = getelementptr i64, ptr %186, i64 %297
  %306 = load i64, ptr %305, align 8, !tbaa !11
  %307 = add nsw i64 %306, %304
  store i64 %307, ptr %305, align 8, !tbaa !11
  %308 = getelementptr i64, ptr %187, i64 %297
  %309 = load i64, ptr %308, align 8, !tbaa !11
  %310 = add nsw i64 %309, %307
  store i64 %310, ptr %308, align 8, !tbaa !11
  %311 = add nuw i64 %297, 4
  %312 = add i64 %298, 4
  %313 = icmp eq i64 %312, %184
  br i1 %313, label %279, label %295, !llvm.loop !109

314:                                              ; preds = %293
  %315 = shl i64 %178, 2
  %316 = tail call noalias ptr @malloc(i64 noundef %315) #19
  br label %317

317:                                              ; preds = %293, %314
  %318 = phi ptr [ %316, %314 ], [ null, %293 ]
  %319 = shl i64 %40, 3
  %320 = tail call noalias ptr @malloc(i64 noundef %319) #19
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %320, ptr align 8 %41, i64 %319, i1 false)
  br i1 %79, label %321, label %326

321:                                              ; preds = %317
  %322 = icmp sgt i32 %30, 0
  %323 = add nsw i32 %30, -1
  br label %335

324:                                              ; preds = %416
  %325 = load i64, ptr %81, align 8, !tbaa !54
  br label %326

326:                                              ; preds = %324, %317
  %327 = phi i64 [ %325, %324 ], [ %82, %317 ]
  %328 = icmp sgt i64 %327, 0
  br i1 %328, label %329, label %420

329:                                              ; preds = %326
  %330 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %331 = load ptr, ptr %330, align 8, !tbaa !55
  %332 = icmp sgt i32 %30, 0
  %333 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %334 = add nsw i32 %30, -1
  br label %422

335:                                              ; preds = %321, %416
  %336 = phi i64 [ 0, %321 ], [ %417, %416 ]
  %337 = trunc i64 %336 to i32
  %338 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %337) #18
  %339 = and i1 %322, %338
  br i1 %339, label %340, label %416

340:                                              ; preds = %335
  %341 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %336
  %342 = load i32, ptr %341, align 4, !tbaa !3
  %343 = lshr i32 %342, 16
  %344 = xor i32 %343, %342
  %345 = mul i32 %344, 2146121005
  %346 = lshr i32 %345, 15
  %347 = xor i32 %346, %345
  %348 = mul i32 %347, -2073254261
  %349 = lshr i32 %348, 16
  %350 = xor i32 %349, %348
  %351 = and i32 %350, %323
  %352 = zext nneg i32 %351 to i64
  %353 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %352
  %354 = getelementptr inbounds nuw i8, ptr %353, i64 8
  %355 = load i8, ptr %354, align 4, !tbaa !96
  %356 = icmp eq i8 %355, 0
  br i1 %356, label %373, label %365

357:                                              ; preds = %365
  %358 = add nuw nsw i32 %367, 1
  %359 = and i32 %358, %323
  %360 = zext nneg i32 %359 to i64
  %361 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %360
  %362 = getelementptr inbounds nuw i8, ptr %361, i64 8
  %363 = load i8, ptr %362, align 4, !tbaa !96
  %364 = icmp eq i8 %363, 0
  br i1 %364, label %373, label %365, !llvm.loop !110

365:                                              ; preds = %340, %357
  %366 = phi ptr [ %361, %357 ], [ %353, %340 ]
  %367 = phi i32 [ %359, %357 ], [ %351, %340 ]
  %368 = load i32, ptr %366, align 4, !tbaa !99
  %369 = icmp eq i32 %368, %342
  br i1 %369, label %370, label %357

370:                                              ; preds = %365
  %371 = getelementptr inbounds nuw i8, ptr %366, i64 4
  %372 = load i32, ptr %371, align 4, !tbaa !100
  br label %373

373:                                              ; preds = %357, %340, %370
  %374 = phi i32 [ %372, %370 ], [ -1, %340 ], [ -1, %357 ]
  %375 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %336, i32 1
  %376 = load i32, ptr %375, align 4, !tbaa !8
  %377 = lshr i32 %376, 16
  %378 = xor i32 %377, %376
  %379 = mul i32 %378, 2146121005
  %380 = lshr i32 %379, 15
  %381 = xor i32 %380, %379
  %382 = mul i32 %381, -2073254261
  %383 = lshr i32 %382, 16
  %384 = xor i32 %383, %382
  %385 = and i32 %384, %323
  %386 = zext nneg i32 %385 to i64
  %387 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %386
  %388 = getelementptr inbounds nuw i8, ptr %387, i64 8
  %389 = load i8, ptr %388, align 4, !tbaa !96
  %390 = icmp eq i8 %389, 0
  br i1 %390, label %416, label %399

391:                                              ; preds = %399
  %392 = add nuw nsw i32 %401, 1
  %393 = and i32 %392, %323
  %394 = zext nneg i32 %393 to i64
  %395 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %394
  %396 = getelementptr inbounds nuw i8, ptr %395, i64 8
  %397 = load i8, ptr %396, align 4, !tbaa !96
  %398 = icmp eq i8 %397, 0
  br i1 %398, label %416, label %399, !llvm.loop !111

399:                                              ; preds = %373, %391
  %400 = phi ptr [ %395, %391 ], [ %387, %373 ]
  %401 = phi i32 [ %393, %391 ], [ %385, %373 ]
  %402 = load i32, ptr %400, align 4, !tbaa !99
  %403 = icmp eq i32 %402, %376
  br i1 %403, label %404, label %391

404:                                              ; preds = %399
  %405 = getelementptr inbounds nuw i8, ptr %400, i64 4
  %406 = load i32, ptr %405, align 4, !tbaa !100
  %407 = icmp sgt i32 %374, -1
  %408 = icmp sgt i32 %406, -1
  %409 = select i1 %407, i1 %408, i1 false
  br i1 %409, label %410, label %416

410:                                              ; preds = %404
  %411 = zext nneg i32 %374 to i64
  %412 = getelementptr inbounds nuw i64, ptr %320, i64 %411
  %413 = load i64, ptr %412, align 8, !tbaa !11
  %414 = add nsw i64 %413, 1
  store i64 %414, ptr %412, align 8, !tbaa !11
  %415 = getelementptr inbounds i32, ptr %318, i64 %413
  store i32 %406, ptr %415, align 4, !tbaa !22
  br label %416

416:                                              ; preds = %391, %373, %404, %410, %335
  %417 = add nuw nsw i64 %336, 1
  %418 = load i64, ptr %42, align 8, !tbaa !53
  %419 = icmp slt i64 %417, %418
  br i1 %419, label %335, label %324, !llvm.loop !112

420:                                              ; preds = %508, %326
  tail call void @free(ptr noundef %320) #18
  tail call void @free(ptr noundef %35) #18
  tail call void @free(ptr noundef %26) #18
  store i64 %13, ptr %1, align 8, !tbaa !11
  store ptr %41, ptr %2, align 8, !tbaa !17
  store ptr %318, ptr %3, align 8, !tbaa !20
  store i64 %178, ptr %4, align 8, !tbaa !11
  %421 = getelementptr inbounds nuw i8, ptr %0, i64 112
  store i32 1, ptr %421, align 8, !tbaa !46
  ret void

422:                                              ; preds = %329, %508
  %423 = phi i64 [ 0, %329 ], [ %509, %508 ]
  %424 = getelementptr inbounds nuw i8, ptr %331, i64 %423
  %425 = load i8, ptr %424, align 1, !tbaa !57
  %426 = icmp ne i8 %425, 0
  %427 = and i1 %332, %426
  br i1 %427, label %428, label %508

428:                                              ; preds = %422
  %429 = load ptr, ptr %333, align 8, !tbaa !78
  %430 = shl nuw nsw i64 %423, 3
  %431 = getelementptr inbounds nuw i8, ptr %429, i64 %430
  %432 = load i32, ptr %431, align 4, !tbaa !22
  %433 = lshr i32 %432, 16
  %434 = xor i32 %433, %432
  %435 = mul i32 %434, 2146121005
  %436 = lshr i32 %435, 15
  %437 = xor i32 %436, %435
  %438 = mul i32 %437, -2073254261
  %439 = lshr i32 %438, 16
  %440 = xor i32 %439, %438
  %441 = and i32 %440, %334
  %442 = zext nneg i32 %441 to i64
  %443 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %442
  %444 = getelementptr inbounds nuw i8, ptr %443, i64 8
  %445 = load i8, ptr %444, align 4, !tbaa !96
  %446 = icmp eq i8 %445, 0
  br i1 %446, label %463, label %455

447:                                              ; preds = %455
  %448 = add nuw nsw i32 %457, 1
  %449 = and i32 %448, %334
  %450 = zext nneg i32 %449 to i64
  %451 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %450
  %452 = getelementptr inbounds nuw i8, ptr %451, i64 8
  %453 = load i8, ptr %452, align 4, !tbaa !96
  %454 = icmp eq i8 %453, 0
  br i1 %454, label %463, label %455, !llvm.loop !113

455:                                              ; preds = %428, %447
  %456 = phi ptr [ %451, %447 ], [ %443, %428 ]
  %457 = phi i32 [ %449, %447 ], [ %441, %428 ]
  %458 = load i32, ptr %456, align 4, !tbaa !99
  %459 = icmp eq i32 %458, %432
  br i1 %459, label %460, label %447

460:                                              ; preds = %455
  %461 = getelementptr inbounds nuw i8, ptr %456, i64 4
  %462 = load i32, ptr %461, align 4, !tbaa !100
  br label %463

463:                                              ; preds = %447, %428, %460
  %464 = phi i32 [ %462, %460 ], [ -1, %428 ], [ -1, %447 ]
  %465 = shl nuw nsw i64 %423, 1
  %466 = or disjoint i64 %465, 1
  %467 = getelementptr inbounds nuw i32, ptr %429, i64 %466
  %468 = load i32, ptr %467, align 4, !tbaa !22
  %469 = lshr i32 %468, 16
  %470 = xor i32 %469, %468
  %471 = mul i32 %470, 2146121005
  %472 = lshr i32 %471, 15
  %473 = xor i32 %472, %471
  %474 = mul i32 %473, -2073254261
  %475 = lshr i32 %474, 16
  %476 = xor i32 %475, %474
  %477 = and i32 %476, %334
  %478 = zext nneg i32 %477 to i64
  %479 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %478
  %480 = getelementptr inbounds nuw i8, ptr %479, i64 8
  %481 = load i8, ptr %480, align 4, !tbaa !96
  %482 = icmp eq i8 %481, 0
  br i1 %482, label %508, label %491

483:                                              ; preds = %491
  %484 = add nuw nsw i32 %493, 1
  %485 = and i32 %484, %334
  %486 = zext nneg i32 %485 to i64
  %487 = getelementptr inbounds nuw %struct.IdMapEntry, ptr %35, i64 %486
  %488 = getelementptr inbounds nuw i8, ptr %487, i64 8
  %489 = load i8, ptr %488, align 4, !tbaa !96
  %490 = icmp eq i8 %489, 0
  br i1 %490, label %508, label %491, !llvm.loop !114

491:                                              ; preds = %463, %483
  %492 = phi ptr [ %487, %483 ], [ %479, %463 ]
  %493 = phi i32 [ %485, %483 ], [ %477, %463 ]
  %494 = load i32, ptr %492, align 4, !tbaa !99
  %495 = icmp eq i32 %494, %468
  br i1 %495, label %496, label %483

496:                                              ; preds = %491
  %497 = getelementptr inbounds nuw i8, ptr %492, i64 4
  %498 = load i32, ptr %497, align 4, !tbaa !100
  %499 = icmp sgt i32 %464, -1
  %500 = icmp sgt i32 %498, -1
  %501 = select i1 %499, i1 %500, i1 false
  br i1 %501, label %502, label %508

502:                                              ; preds = %496
  %503 = zext nneg i32 %464 to i64
  %504 = getelementptr inbounds nuw i64, ptr %320, i64 %503
  %505 = load i64, ptr %504, align 8, !tbaa !11
  %506 = add nsw i64 %505, 1
  store i64 %506, ptr %504, align 8, !tbaa !11
  %507 = getelementptr inbounds i32, ptr %318, i64 %505
  store i32 %498, ptr %507, align 4, !tbaa !22
  br label %508

508:                                              ; preds = %483, %463, %496, %502, %422
  %509 = add nuw nsw i64 %423, 1
  %510 = icmp eq i64 %509, %327
  br i1 %510, label %420, label %422, !llvm.loop !115
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @autograph_get_layout(ptr noundef readnone %0) local_unnamed_addr #10 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %17

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %17, label %9, !llvm.loop !34

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 8, !tbaa !35
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !43
  br label %17

17:                                               ; preds = %6, %1, %14
  %18 = phi i32 [ %16, %14 ], [ 3, %1 ], [ 3, %6 ]
  ret i32 %18
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_debug_print(ptr nocapture noundef readnone %0) local_unnamed_addr #11 {
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: write, inaccessiblemem: none) uwtable
define dso_local range(i32 0, 2) i32 @autograph_get_meta_handles(ptr noundef readnone %0, ptr noundef writeonly %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #12 {
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %32

7:                                                ; preds = %4
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %32, label %12, !llvm.loop !34

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 8, !tbaa !35
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = icmp eq ptr %1, null
  br i1 %18, label %22, label %19

19:                                               ; preds = %17
  %20 = getelementptr inbounds nuw i8, ptr %14, i64 16
  %21 = load ptr, ptr %20, align 8, !tbaa !40
  store ptr %21, ptr %1, align 8, !tbaa !94
  br label %22

22:                                               ; preds = %19, %17
  %23 = icmp eq ptr %2, null
  br i1 %23, label %27, label %24

24:                                               ; preds = %22
  %25 = getelementptr inbounds nuw i8, ptr %14, i64 24
  %26 = load ptr, ptr %25, align 8, !tbaa !41
  store ptr %26, ptr %2, align 8, !tbaa !94
  br label %27

27:                                               ; preds = %24, %22
  %28 = icmp eq ptr %3, null
  br i1 %28, label %32, label %29

29:                                               ; preds = %27
  %30 = getelementptr inbounds nuw i8, ptr %14, i64 32
  %31 = load ptr, ptr %30, align 8, !tbaa !42
  store ptr %31, ptr %3, align 8, !tbaa !94
  br label %32

32:                                               ; preds = %9, %4, %27, %29
  %33 = phi i32 [ 1, %29 ], [ 1, %27 ], [ 0, %4 ], [ 0, %9 ]
  ret i32 %33
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %20

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %20, label %10, !llvm.loop !34

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 8, !tbaa !35
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !40
  %18 = icmp eq ptr %17, null
  br i1 %18, label %20, label %19

19:                                               ; preds = %15
  tail call void @roaring_bitmap_add(ptr noundef nonnull %17, i32 noundef %1) #18
  br label %20

20:                                               ; preds = %7, %2, %15, %19
  %21 = phi i32 [ 1, %19 ], [ 0, %15 ], [ 0, %2 ], [ 0, %7 ]
  ret i32 %21
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %78

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %78, label %10, !llvm.loop !34

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 8, !tbaa !35
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !40
  %18 = icmp eq ptr %17, null
  br i1 %18, label %78, label %19

19:                                               ; preds = %15
  %20 = getelementptr inbounds nuw i8, ptr %12, i64 24
  %21 = load ptr, ptr %20, align 8, !tbaa !41
  %22 = icmp eq ptr %21, null
  br i1 %22, label %78, label %23

23:                                               ; preds = %19
  tail call void @roaring_bitmap_remove(ptr noundef nonnull %17, i32 noundef %1) #18
  %24 = getelementptr inbounds nuw i8, ptr %12, i64 32
  %25 = load ptr, ptr %24, align 8, !tbaa !42
  %26 = load ptr, ptr %20, align 8, !tbaa !41
  %27 = getelementptr inbounds nuw i8, ptr %12, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !53
  %29 = icmp sgt i64 %28, 0
  br i1 %29, label %37, label %30

30:                                               ; preds = %50, %23
  %31 = getelementptr inbounds nuw i8, ptr %12, i64 64
  %32 = load i64, ptr %31, align 8, !tbaa !54
  %33 = icmp sgt i64 %32, 0
  br i1 %33, label %34, label %78

34:                                               ; preds = %30
  %35 = getelementptr inbounds nuw i8, ptr %12, i64 56
  %36 = getelementptr inbounds nuw i8, ptr %12, i64 48
  br label %54

37:                                               ; preds = %23, %50
  %38 = phi i64 [ %51, %50 ], [ 0, %23 ]
  %39 = trunc i64 %38 to i32
  %40 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %26, i32 noundef %39) #18
  br i1 %40, label %41, label %50

41:                                               ; preds = %37
  %42 = getelementptr inbounds nuw %struct.EdgePair, ptr %25, i64 %38
  %43 = load i32, ptr %42, align 4, !tbaa !3
  %44 = icmp eq i32 %43, %1
  br i1 %44, label %49, label %45

45:                                               ; preds = %41
  %46 = getelementptr inbounds nuw i8, ptr %42, i64 4
  %47 = load i32, ptr %46, align 4, !tbaa !8
  %48 = icmp eq i32 %47, %1
  br i1 %48, label %49, label %50

49:                                               ; preds = %45, %41
  tail call void @roaring_bitmap_remove(ptr noundef %26, i32 noundef %39) #18
  br label %50

50:                                               ; preds = %45, %49, %37
  %51 = add nuw nsw i64 %38, 1
  %52 = load i64, ptr %27, align 8, !tbaa !53
  %53 = icmp slt i64 %51, %52
  br i1 %53, label %37, label %30, !llvm.loop !116

54:                                               ; preds = %34, %74
  %55 = phi i64 [ %32, %34 ], [ %75, %74 ]
  %56 = phi i64 [ 0, %34 ], [ %76, %74 ]
  %57 = load ptr, ptr %35, align 8, !tbaa !55
  %58 = getelementptr inbounds nuw i8, ptr %57, i64 %56
  %59 = load i8, ptr %58, align 1, !tbaa !57
  %60 = icmp eq i8 %59, 0
  br i1 %60, label %74, label %61

61:                                               ; preds = %54
  %62 = load ptr, ptr %36, align 8, !tbaa !78
  %63 = shl nuw nsw i64 %56, 1
  %64 = getelementptr inbounds nuw i32, ptr %62, i64 %63
  %65 = load i32, ptr %64, align 4, !tbaa !22
  %66 = icmp eq i32 %65, %1
  br i1 %66, label %72, label %67

67:                                               ; preds = %61
  %68 = or disjoint i64 %63, 1
  %69 = getelementptr inbounds nuw i32, ptr %62, i64 %68
  %70 = load i32, ptr %69, align 4, !tbaa !22
  %71 = icmp eq i32 %70, %1
  br i1 %71, label %72, label %74

72:                                               ; preds = %67, %61
  store i8 0, ptr %58, align 1, !tbaa !57
  %73 = load i64, ptr %31, align 8, !tbaa !54
  br label %74

74:                                               ; preds = %67, %72, %54
  %75 = phi i64 [ %55, %67 ], [ %73, %72 ], [ %55, %54 ]
  %76 = add nuw nsw i64 %56, 1
  %77 = icmp slt i64 %76, %75
  br i1 %77, label %54, label %78, !llvm.loop !117

78:                                               ; preds = %7, %74, %30, %2, %15, %19
  %79 = phi i32 [ 0, %19 ], [ 0, %15 ], [ 0, %2 ], [ 1, %30 ], [ 1, %74 ], [ 0, %7 ]
  ret i32 %79
}

declare dso_local void @roaring_bitmap_remove(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %104

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %104, label %11, !llvm.loop !34

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !35
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 16
  %18 = load ptr, ptr %17, align 8, !tbaa !40
  %19 = icmp eq ptr %18, null
  br i1 %19, label %104, label %20

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %22 = load ptr, ptr %21, align 8, !tbaa !41
  %23 = icmp eq ptr %22, null
  br i1 %23, label %104, label %24

24:                                               ; preds = %20
  tail call void @roaring_bitmap_add(ptr noundef nonnull %18, i32 noundef %1) #18
  %25 = load ptr, ptr %17, align 8, !tbaa !40
  tail call void @roaring_bitmap_add(ptr noundef %25, i32 noundef %2) #18
  %26 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %27 = icmp sgt i32 %26, -1
  br i1 %27, label %28, label %30

28:                                               ; preds = %24
  %29 = load ptr, ptr %21, align 8, !tbaa !41
  tail call void @roaring_bitmap_add(ptr noundef %29, i32 noundef %26) #18
  br label %104

30:                                               ; preds = %24
  %31 = getelementptr inbounds nuw i8, ptr %13, i64 64
  %32 = load i64, ptr %31, align 8, !tbaa !54
  %33 = icmp slt i64 %32, 1
  br i1 %33, label %59, label %34

34:                                               ; preds = %30
  %35 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %36 = load ptr, ptr %35, align 8, !tbaa !78
  br label %37

37:                                               ; preds = %52, %34
  %38 = phi i64 [ 0, %34 ], [ %53, %52 ]
  %39 = shl nuw nsw i64 %38, 1
  %40 = getelementptr inbounds nuw i32, ptr %36, i64 %39
  %41 = load i32, ptr %40, align 4, !tbaa !22
  %42 = or disjoint i64 %39, 1
  %43 = getelementptr inbounds nuw i32, ptr %36, i64 %42
  %44 = load i32, ptr %43, align 4, !tbaa !22
  %45 = icmp eq i32 %41, %1
  %46 = icmp eq i32 %44, %2
  %47 = select i1 %45, i1 %46, i1 false
  br i1 %47, label %55, label %48

48:                                               ; preds = %37
  %49 = icmp ne i32 %41, %2
  %50 = icmp ne i32 %44, %1
  %51 = select i1 %49, i1 true, i1 %50
  br i1 %51, label %52, label %55

52:                                               ; preds = %48
  %53 = add nuw nsw i64 %38, 1
  %54 = icmp eq i64 %53, %32
  br i1 %54, label %59, label %37, !llvm.loop !79

55:                                               ; preds = %48, %37
  %56 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %57 = load ptr, ptr %56, align 8, !tbaa !55
  %58 = getelementptr inbounds nuw i8, ptr %57, i64 %38
  store i8 1, ptr %58, align 1, !tbaa !57
  br label %104

59:                                               ; preds = %52, %30
  %60 = getelementptr inbounds nuw i8, ptr %13, i64 72
  %61 = load i64, ptr %60, align 8, !tbaa !74
  %62 = icmp slt i64 %32, %61
  br i1 %62, label %63, label %68

63:                                               ; preds = %59
  %64 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %65 = load ptr, ptr %64, align 8, !tbaa !78
  %66 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %67 = load ptr, ptr %66, align 8, !tbaa !55
  br label %94

68:                                               ; preds = %59
  %69 = icmp sgt i64 %61, 0
  %70 = select i1 %69, i64 %61, i64 16
  br label %71

71:                                               ; preds = %71, %68
  %72 = phi i64 [ %70, %68 ], [ %74, %71 ]
  %73 = icmp sgt i64 %72, %32
  %74 = shl nsw i64 %72, 1
  br i1 %73, label %75, label %71, !llvm.loop !80

75:                                               ; preds = %71
  %76 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %77 = load ptr, ptr %76, align 8, !tbaa !78
  %78 = shl i64 %72, 3
  %79 = tail call ptr @realloc(ptr noundef %77, i64 noundef %78) #20
  %80 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %81 = load ptr, ptr %80, align 8, !tbaa !55
  %82 = tail call ptr @realloc(ptr noundef %81, i64 noundef %72) #20
  %83 = icmp ne ptr %79, null
  %84 = icmp ne ptr %82, null
  %85 = and i1 %83, %84
  br i1 %85, label %86, label %104

86:                                               ; preds = %75
  %87 = load i64, ptr %60, align 8, !tbaa !74
  %88 = icmp sgt i64 %72, %87
  br i1 %88, label %89, label %92

89:                                               ; preds = %86
  %90 = getelementptr inbounds i8, ptr %82, i64 %87
  %91 = sub nsw i64 %72, %87
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %90, i8 0, i64 %91, i1 false)
  br label %92

92:                                               ; preds = %89, %86
  store ptr %79, ptr %76, align 8, !tbaa !78
  store ptr %82, ptr %80, align 8, !tbaa !55
  store i64 %72, ptr %60, align 8, !tbaa !74
  %93 = load i64, ptr %31, align 8, !tbaa !54
  br label %94

94:                                               ; preds = %63, %92
  %95 = phi ptr [ %67, %63 ], [ %82, %92 ]
  %96 = phi ptr [ %65, %63 ], [ %79, %92 ]
  %97 = phi i64 [ %32, %63 ], [ %93, %92 ]
  %98 = add nsw i64 %97, 1
  store i64 %98, ptr %31, align 8, !tbaa !54
  %99 = shl nsw i64 %97, 1
  %100 = getelementptr inbounds i32, ptr %96, i64 %99
  store i32 %1, ptr %100, align 4, !tbaa !22
  %101 = or disjoint i64 %99, 1
  %102 = getelementptr inbounds i32, ptr %96, i64 %101
  store i32 %2, ptr %102, align 4, !tbaa !22
  %103 = getelementptr inbounds i8, ptr %95, i64 %97
  store i8 1, ptr %103, align 1, !tbaa !57
  br label %104

104:                                              ; preds = %8, %3, %75, %28, %94, %55, %16, %20
  %105 = phi i32 [ 0, %20 ], [ 0, %16 ], [ 1, %28 ], [ 1, %55 ], [ 1, %94 ], [ 0, %75 ], [ 0, %3 ], [ 0, %8 ]
  ret i32 %105
}

; Function Attrs: nofree nounwind memory(readwrite, argmem: read) uwtable
define internal fastcc i32 @canonical_pair_find_static(ptr noundef nonnull readonly %0, i32 noundef %1, i32 noundef %2) unnamed_addr #13 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %5 = load ptr, ptr %4, align 8, !tbaa !42
  %6 = icmp eq ptr %5, null
  br i1 %6, label %136, label %7

7:                                                ; preds = %3
  %8 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %9 = icmp sgt i32 %8, 0
  br i1 %9, label %10, label %136

10:                                               ; preds = %7
  %11 = zext nneg i32 %8 to i64
  br label %12

12:                                               ; preds = %16, %10
  %13 = phi i64 [ 0, %10 ], [ %17, %16 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %19, label %16

16:                                               ; preds = %12
  %17 = add nuw nsw i64 %13, 1
  %18 = icmp eq i64 %17, %11
  br i1 %18, label %136, label %12, !llvm.loop !118

19:                                               ; preds = %12
  %20 = and i64 %13, 4294967295
  %21 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %20
  %22 = load ptr, ptr %21, align 8, !tbaa !119
  %23 = icmp eq ptr %22, null
  br i1 %23, label %26, label %24

24:                                               ; preds = %19
  %25 = load ptr, ptr %22, align 8, !tbaa !121
  br label %91

26:                                               ; preds = %19
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !53
  %29 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #19
  %30 = trunc i64 %28 to i32
  %31 = shl i32 %30, 1
  %32 = add i32 %31, 32
  br label %33

33:                                               ; preds = %33, %26
  %34 = phi i32 [ 1, %26 ], [ %36, %33 ]
  %35 = icmp sgt i32 %34, %32
  %36 = shl i32 %34, 1
  br i1 %35, label %37, label %33, !llvm.loop !24

37:                                               ; preds = %33
  %38 = sext i32 %34 to i64
  %39 = getelementptr inbounds nuw i8, ptr %29, i64 8
  store i64 %38, ptr %39, align 8, !tbaa !124
  %40 = tail call noalias ptr @calloc(i64 noundef %38, i64 noundef 24) #17
  store ptr %40, ptr %29, align 8, !tbaa !121
  store ptr %29, ptr %21, align 8, !tbaa !119
  %41 = icmp sgt i64 %28, 0
  br i1 %41, label %42, label %91

42:                                               ; preds = %37
  %43 = add nsw i64 %38, -1
  br label %44

44:                                               ; preds = %88, %42
  %45 = phi i64 [ 0, %42 ], [ %89, %88 ]
  %46 = getelementptr inbounds nuw %struct.EdgePair, ptr %5, i64 %45
  %47 = load i32, ptr %46, align 4, !tbaa !3
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !8
  %50 = tail call i32 @llvm.smin.i32(i32 %47, i32 %49)
  %51 = tail call i32 @llvm.smax.i32(i32 %47, i32 %49)
  %52 = zext i32 %50 to i64
  %53 = shl nuw i64 %52, 32
  %54 = zext i32 %51 to i64
  %55 = or disjoint i64 %53, %54
  %56 = lshr i64 %55, 30
  %57 = xor i64 %56, %55
  %58 = mul i64 %57, -4658895280553007687
  %59 = lshr i64 %58, 27
  %60 = xor i64 %59, %58
  %61 = mul i64 %60, -7723592293110705685
  %62 = lshr i64 %61, 31
  %63 = xor i64 %62, %61
  %64 = and i64 %63, %43
  %65 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %64
  %66 = getelementptr inbounds nuw i8, ptr %65, i64 16
  %67 = load i8, ptr %66, align 8, !tbaa !25
  %68 = icmp eq i8 %67, 0
  br i1 %68, label %83, label %69

69:                                               ; preds = %44, %76
  %70 = phi ptr [ %79, %76 ], [ %65, %44 ]
  %71 = phi i64 [ %78, %76 ], [ %64, %44 ]
  %72 = load i64, ptr %70, align 8, !tbaa !27
  %73 = icmp eq i64 %72, %55
  br i1 %73, label %74, label %76

74:                                               ; preds = %69
  %75 = getelementptr inbounds nuw i8, ptr %70, i64 8
  store i64 %45, ptr %75, align 8, !tbaa !28
  br label %88

76:                                               ; preds = %69
  %77 = add i64 %71, 1
  %78 = and i64 %77, %43
  %79 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %78
  %80 = getelementptr inbounds nuw i8, ptr %79, i64 16
  %81 = load i8, ptr %80, align 8, !tbaa !25
  %82 = icmp eq i8 %81, 0
  br i1 %82, label %83, label %69, !llvm.loop !29

83:                                               ; preds = %76, %44
  %84 = phi i64 [ %64, %44 ], [ %78, %76 ]
  %85 = phi ptr [ %65, %44 ], [ %79, %76 ]
  store i64 %55, ptr %85, align 8, !tbaa !27
  %86 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %84, i32 1
  store i64 %45, ptr %86, align 8, !tbaa !28
  %87 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %84, i32 2
  store i8 1, ptr %87, align 8, !tbaa !25
  br label %88

88:                                               ; preds = %83, %74
  %89 = add nuw nsw i64 %45, 1
  %90 = icmp eq i64 %89, %28
  br i1 %90, label %91, label %44, !llvm.loop !125

91:                                               ; preds = %88, %24, %37
  %92 = phi ptr [ %40, %37 ], [ %25, %24 ], [ %40, %88 ]
  %93 = phi ptr [ %29, %37 ], [ %22, %24 ], [ %29, %88 ]
  %94 = icmp eq ptr %92, null
  br i1 %94, label %136, label %95

95:                                               ; preds = %91
  %96 = getelementptr inbounds nuw i8, ptr %93, i64 8
  %97 = load i64, ptr %96, align 8, !tbaa !124
  %98 = icmp slt i64 %97, 1
  br i1 %98, label %136, label %99

99:                                               ; preds = %95
  %100 = tail call i32 @llvm.smin.i32(i32 %1, i32 %2)
  %101 = tail call i32 @llvm.smax.i32(i32 %1, i32 %2)
  %102 = zext i32 %100 to i64
  %103 = shl nuw i64 %102, 32
  %104 = zext i32 %101 to i64
  %105 = or disjoint i64 %103, %104
  %106 = lshr i64 %105, 30
  %107 = xor i64 %106, %105
  %108 = mul i64 %107, -4658895280553007687
  %109 = lshr i64 %108, 27
  %110 = xor i64 %109, %108
  %111 = mul i64 %110, -7723592293110705685
  %112 = lshr i64 %111, 31
  %113 = xor i64 %112, %111
  %114 = add nsw i64 %97, -1
  %115 = and i64 %114, %113
  %116 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %92, i64 %115
  %117 = getelementptr inbounds nuw i8, ptr %116, i64 16
  %118 = load i8, ptr %117, align 8, !tbaa !25
  %119 = icmp eq i8 %118, 0
  br i1 %119, label %136, label %127

120:                                              ; preds = %127
  %121 = add i64 %129, 1
  %122 = and i64 %121, %114
  %123 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %92, i64 %122
  %124 = getelementptr inbounds nuw i8, ptr %123, i64 16
  %125 = load i8, ptr %124, align 8, !tbaa !25
  %126 = icmp eq i8 %125, 0
  br i1 %126, label %136, label %127, !llvm.loop !32

127:                                              ; preds = %99, %120
  %128 = phi ptr [ %123, %120 ], [ %116, %99 ]
  %129 = phi i64 [ %122, %120 ], [ %115, %99 ]
  %130 = load i64, ptr %128, align 8, !tbaa !27
  %131 = icmp eq i64 %130, %105
  br i1 %131, label %132, label %120

132:                                              ; preds = %127
  %133 = getelementptr inbounds nuw i8, ptr %128, i64 8
  %134 = load i64, ptr %133, align 8, !tbaa !28
  %135 = trunc i64 %134 to i32
  br label %136

136:                                              ; preds = %16, %120, %132, %99, %95, %91, %7, %3
  %137 = phi i32 [ -1, %3 ], [ -1, %95 ], [ -1, %91 ], [ %135, %132 ], [ -1, %99 ], [ -1, %7 ], [ -1, %120 ], [ -1, %16 ]
  ret i32 %137
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %54

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %54, label %11, !llvm.loop !34

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !35
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %18 = load ptr, ptr %17, align 8, !tbaa !41
  %19 = icmp eq ptr %18, null
  br i1 %19, label %54, label %20

20:                                               ; preds = %16
  %21 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %22 = icmp sgt i32 %21, -1
  br i1 %22, label %23, label %25

23:                                               ; preds = %20
  %24 = load ptr, ptr %17, align 8, !tbaa !41
  tail call void @roaring_bitmap_remove(ptr noundef %24, i32 noundef %21) #18
  br label %54

25:                                               ; preds = %20
  %26 = getelementptr inbounds nuw i8, ptr %13, i64 64
  %27 = load i64, ptr %26, align 8, !tbaa !54
  %28 = icmp slt i64 %27, 1
  br i1 %28, label %54, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %31 = load ptr, ptr %30, align 8, !tbaa !78
  br label %32

32:                                               ; preds = %47, %29
  %33 = phi i64 [ 0, %29 ], [ %48, %47 ]
  %34 = shl nuw nsw i64 %33, 1
  %35 = getelementptr inbounds nuw i32, ptr %31, i64 %34
  %36 = load i32, ptr %35, align 4, !tbaa !22
  %37 = or disjoint i64 %34, 1
  %38 = getelementptr inbounds nuw i32, ptr %31, i64 %37
  %39 = load i32, ptr %38, align 4, !tbaa !22
  %40 = icmp eq i32 %36, %1
  %41 = icmp eq i32 %39, %2
  %42 = select i1 %40, i1 %41, i1 false
  br i1 %42, label %50, label %43

43:                                               ; preds = %32
  %44 = icmp ne i32 %36, %2
  %45 = icmp ne i32 %39, %1
  %46 = select i1 %44, i1 true, i1 %45
  br i1 %46, label %47, label %50

47:                                               ; preds = %43
  %48 = add nuw nsw i64 %33, 1
  %49 = icmp eq i64 %48, %27
  br i1 %49, label %54, label %32, !llvm.loop !79

50:                                               ; preds = %43, %32
  %51 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %52 = load ptr, ptr %51, align 8, !tbaa !55
  %53 = getelementptr inbounds nuw i8, ptr %52, i64 %33
  store i8 0, ptr %53, align 1, !tbaa !57
  br label %54

54:                                               ; preds = %8, %47, %25, %3, %23, %50, %16
  %55 = phi i32 [ 0, %16 ], [ 1, %50 ], [ 1, %23 ], [ 0, %3 ], [ 1, %25 ], [ 1, %47 ], [ 0, %8 ]
  ret i32 %55
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_init(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4, ptr noundef %5) local_unnamed_addr #0 {
  %7 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %9, label %21

9:                                                ; preds = %6
  %10 = zext nneg i32 %7 to i64
  br label %14

11:                                               ; preds = %14
  %12 = add nuw nsw i64 %15, 1
  %13 = icmp eq i64 %12, %10
  br i1 %13, label %19, label %14, !llvm.loop !34

14:                                               ; preds = %11, %9
  %15 = phi i64 [ 0, %9 ], [ %12, %11 ]
  %16 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %15
  %17 = load ptr, ptr %16, align 8, !tbaa !35
  %18 = icmp eq ptr %17, %0
  br i1 %18, label %26, label %11

19:                                               ; preds = %11
  %20 = icmp sgt i32 %7, 63
  br i1 %20, label %66, label %21

21:                                               ; preds = %19, %6
  %22 = add nsw i32 %7, 1
  store i32 %22, ptr @g_meta_count, align 4, !tbaa !22
  %23 = sext i32 %7 to i64
  %24 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %23
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(168) %25, i8 0, i64 160, i1 false)
  store ptr %0, ptr %24, align 8, !tbaa !35
  store i32 3, ptr %25, align 8, !tbaa !43
  br label %26

26:                                               ; preds = %14, %21
  %27 = phi i32 [ %22, %21 ], [ %7, %14 ]
  %28 = phi ptr [ %24, %21 ], [ %16, %14 ]
  %29 = icmp sgt i32 %27, 0
  br i1 %29, label %30, label %46

30:                                               ; preds = %26
  %31 = zext nneg i32 %27 to i64
  br label %32

32:                                               ; preds = %36, %30
  %33 = phi i64 [ 0, %30 ], [ %37, %36 ]
  %34 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %33
  %35 = icmp eq ptr %34, %28
  br i1 %35, label %39, label %36

36:                                               ; preds = %32
  %37 = add nuw nsw i64 %33, 1
  %38 = icmp eq i64 %37, %31
  br i1 %38, label %46, label %32, !llvm.loop !118

39:                                               ; preds = %32
  %40 = and i64 %33, 4294967295
  %41 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %40
  %42 = load ptr, ptr %41, align 8, !tbaa !119
  %43 = icmp eq ptr %42, null
  br i1 %43, label %46, label %44

44:                                               ; preds = %39
  %45 = load ptr, ptr %42, align 8, !tbaa !121
  tail call void @free(ptr noundef %45) #18
  tail call void @free(ptr noundef nonnull %42) #18
  store ptr null, ptr %41, align 8, !tbaa !119
  br label %46

46:                                               ; preds = %36, %26, %44, %39
  %47 = getelementptr inbounds nuw i8, ptr %28, i64 16
  store ptr %3, ptr %47, align 8, !tbaa !40
  %48 = getelementptr inbounds nuw i8, ptr %28, i64 24
  store ptr %4, ptr %48, align 8, !tbaa !41
  %49 = getelementptr inbounds nuw i8, ptr %28, i64 32
  store ptr %5, ptr %49, align 8, !tbaa !42
  %50 = icmp sgt i64 %2, 1
  %51 = zext i1 %50 to i64
  %52 = lshr i64 %2, %51
  %53 = getelementptr inbounds nuw i8, ptr %28, i64 40
  store i64 %52, ptr %53, align 8, !tbaa !53
  %54 = getelementptr inbounds nuw i8, ptr %28, i64 64
  %55 = getelementptr inbounds nuw i8, ptr %28, i64 48
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %54, i8 0, i64 16, i1 false)
  %56 = load ptr, ptr %55, align 8, !tbaa !78
  tail call void @free(ptr noundef %56) #18
  %57 = getelementptr inbounds nuw i8, ptr %28, i64 56
  %58 = load ptr, ptr %57, align 8, !tbaa !55
  tail call void @free(ptr noundef %58) #18
  %59 = getelementptr inbounds nuw i8, ptr %28, i64 96
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %55, i8 0, i64 16, i1 false)
  store i64 %1, ptr %59, align 8, !tbaa !44
  %60 = getelementptr inbounds nuw i8, ptr %28, i64 104
  store i64 %2, ptr %60, align 8, !tbaa !45
  %61 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %62 = getelementptr inbounds nuw i8, ptr %28, i64 80
  %63 = load <2 x ptr>, ptr %61, align 8, !tbaa !94
  store <2 x ptr> %63, ptr %62, align 8, !tbaa !94
  %64 = getelementptr inbounds nuw i8, ptr %28, i64 112
  store i32 0, ptr %64, align 8, !tbaa !46
  %65 = getelementptr inbounds nuw i8, ptr %28, i64 8
  store i32 0, ptr %65, align 8, !tbaa !43
  br label %66

66:                                               ; preds = %19, %46
  ret void
}

declare dso_local i32 @roaring_bitmap_get_at_index(ptr noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local void @qsort(ptr noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #14

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal range(i32 -1, 2) i32 @cmp_i32(ptr nocapture noundef readonly %0, ptr nocapture noundef readonly %1) #15 {
  %3 = load i32, ptr %0, align 4, !tbaa !22
  %4 = load i32, ptr %1, align 4, !tbaa !22
  %5 = tail call i32 @llvm.scmp.i32.i32(i32 %3, i32 %4)
  ret i32 %5
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #16

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #16

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #16

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #16

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.scmp.i32.i32(i32, i32) #16

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>) #16

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x i64> @llvm.smax.v2i64(<2 x i64>, <2 x i64>) #16

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #6 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #9 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #12 = { nofree norecurse nosync nounwind memory(read, argmem: write, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #13 = { nofree nounwind memory(readwrite, argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #14 = { nofree "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #15 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #16 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #17 = { nounwind allocsize(0,1) }
attributes #18 = { nounwind }
attributes #19 = { nounwind allocsize(0) }
attributes #20 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"", !5, i64 0, !5, i64 4}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!4, !5, i64 4}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!12, !12, i64 0}
!12 = !{!"long", !6, i64 0}
!13 = distinct !{!13, !10}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.unroll.disable"}
!16 = distinct !{!16, !10}
!17 = !{!18, !18, i64 0}
!18 = !{!"p1 long", !19, i64 0}
!19 = !{!"any pointer", !6, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"p1 int", !19, i64 0}
!22 = !{!5, !5, i64 0}
!23 = distinct !{!23, !10}
!24 = distinct !{!24, !10}
!25 = !{!26, !6, i64 16}
!26 = !{!"EdgeHashEntry_s", !12, i64 0, !12, i64 8, !6, i64 16}
!27 = !{!26, !12, i64 0}
!28 = !{!26, !12, i64 8}
!29 = distinct !{!29, !10}
!30 = distinct !{!30, !10}
!31 = distinct !{!31, !10}
!32 = distinct !{!32, !10}
!33 = distinct !{!33, !10}
!34 = distinct !{!34, !10}
!35 = !{!36, !19, i64 0}
!36 = !{!"", !19, i64 0, !5, i64 8, !19, i64 16, !19, i64 24, !19, i64 32, !12, i64 40, !21, i64 48, !37, i64 56, !12, i64 64, !12, i64 72, !18, i64 80, !21, i64 88, !12, i64 96, !12, i64 104, !5, i64 112, !18, i64 120, !21, i64 128, !12, i64 136, !21, i64 144, !21, i64 152, !5, i64 160, !5, i64 164}
!37 = !{!"p1 omnipotent char", !19, i64 0}
!38 = !{!36, !18, i64 80}
!39 = !{!36, !21, i64 88}
!40 = !{!36, !19, i64 16}
!41 = !{!36, !19, i64 24}
!42 = !{!36, !19, i64 32}
!43 = !{!36, !5, i64 8}
!44 = !{!36, !12, i64 96}
!45 = !{!36, !12, i64 104}
!46 = !{!36, !5, i64 112}
!47 = !{!36, !18, i64 120}
!48 = !{!36, !21, i64 128}
!49 = !{!36, !5, i64 164}
!50 = !{!36, !5, i64 160}
!51 = !{!36, !21, i64 144}
!52 = !{!36, !21, i64 152}
!53 = !{!36, !12, i64 40}
!54 = !{!36, !12, i64 64}
!55 = !{!36, !37, i64 56}
!56 = distinct !{!56, !10}
!57 = !{!6, !6, i64 0}
!58 = distinct !{!58, !10}
!59 = distinct !{!59, !10, !60, !61}
!60 = !{!"llvm.loop.isvectorized", i32 1}
!61 = !{!"llvm.loop.unroll.runtime.disable"}
!62 = distinct !{!62, !10}
!63 = distinct !{!63, !10, !61, !60}
!64 = distinct !{!64, !15}
!65 = distinct !{!65, !10}
!66 = distinct !{!66, !10}
!67 = distinct !{!67, !10}
!68 = distinct !{!68, !10}
!69 = distinct !{!69, !10}
!70 = distinct !{!70, !15}
!71 = distinct !{!71, !10}
!72 = distinct !{!72, !10}
!73 = distinct !{!73, !10}
!74 = !{!36, !12, i64 72}
!75 = distinct !{!75, !10}
!76 = distinct !{!76, !10}
!77 = distinct !{!77, !10}
!78 = !{!36, !21, i64 48}
!79 = distinct !{!79, !10}
!80 = distinct !{!80, !10}
!81 = distinct !{!81, !10}
!82 = distinct !{!82, !10, !60, !61}
!83 = distinct !{!83, !10, !60}
!84 = distinct !{!84, !10}
!85 = distinct !{!85, !10, !60, !61}
!86 = distinct !{!86, !10, !61, !60}
!87 = distinct !{!87, !10}
!88 = distinct !{!88, !10}
!89 = distinct !{!89, !10}
!90 = distinct !{!90, !10, !60, !61}
!91 = distinct !{!91, !10, !61, !60}
!92 = distinct !{!92, !10}
!93 = distinct !{!93, !10}
!94 = !{!19, !19, i64 0}
!95 = distinct !{!95, !10}
!96 = !{!97, !6, i64 8}
!97 = !{!"", !5, i64 0, !5, i64 4, !6, i64 8}
!98 = distinct !{!98, !10}
!99 = !{!97, !5, i64 0}
!100 = !{!97, !5, i64 4}
!101 = distinct !{!101, !10}
!102 = distinct !{!102, !10}
!103 = distinct !{!103, !10}
!104 = distinct !{!104, !10}
!105 = distinct !{!105, !10}
!106 = distinct !{!106, !10}
!107 = distinct !{!107, !10}
!108 = distinct !{!108, !15}
!109 = distinct !{!109, !10}
!110 = distinct !{!110, !10}
!111 = distinct !{!111, !10}
!112 = distinct !{!112, !10}
!113 = distinct !{!113, !10}
!114 = distinct !{!114, !10}
!115 = distinct !{!115, !10}
!116 = distinct !{!116, !10}
!117 = distinct !{!117, !10}
!118 = distinct !{!118, !10}
!119 = !{!120, !120, i64 0}
!120 = !{!"p1 _ZTS13EdgeHashMap_s", !19, i64 0}
!121 = !{!122, !123, i64 0}
!122 = !{!"EdgeHashMap_s", !123, i64 0, !12, i64 8}
!123 = !{!"p1 _ZTS15EdgeHashEntry_s", !19, i64 0}
!124 = !{!122, !12, i64 8}
!125 = distinct !{!125, !10}
