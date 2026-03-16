; ModuleID = 'autotuner_runtime.c'
source_filename = "autotuner_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.AutoGraphMeta = type { ptr, i32, ptr, ptr, ptr, i64, ptr, ptr, i64, i64, ptr, ptr, i64, i64, i32, ptr, ptr, i64, ptr, ptr, i32, i32 }
%struct.EdgePair = type { i32, i32 }
%struct.EdgeHashEntry_s = type { i64, i64, i8 }

@g_meta_count = internal unnamed_addr global i32 0, align 4
@g_meta = internal global [64 x %struct.AutoGraphMeta] zeroinitializer, align 16
@g_static_edge_hash = internal unnamed_addr global [64 x ptr] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @build_csr_from_set(i64 noundef %0, i64 noundef %1, ptr noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = icmp sgt i64 %1, 0
  br i1 %8, label %15, label %9

9:                                                ; preds = %7
  %10 = tail call noalias dereferenceable_or_null(8) ptr @calloc(i64 noundef 1, i64 noundef 8) #15
  br label %104

11:                                               ; preds = %29
  %12 = add nsw i64 %30, 2
  %13 = tail call noalias ptr @calloc(i64 noundef %12, i64 noundef 8) #15
  %14 = getelementptr inbounds nuw i8, ptr %13, i64 8
  br label %45

15:                                               ; preds = %7, %29
  %16 = phi i64 [ %30, %29 ], [ -1, %7 ]
  %17 = phi i64 [ %31, %29 ], [ 0, %7 ]
  %18 = trunc i64 %17 to i32
  %19 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %18) #16
  br i1 %19, label %20, label %29

20:                                               ; preds = %15
  %21 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %17
  %22 = load i32, ptr %21, align 4, !tbaa !5
  %23 = sext i32 %22 to i64
  %24 = tail call i64 @llvm.smax.i64(i64 %16, i64 %23)
  %25 = getelementptr inbounds nuw i8, ptr %21, i64 4
  %26 = load i32, ptr %25, align 4, !tbaa !10
  %27 = sext i32 %26 to i64
  %28 = tail call i64 @llvm.smax.i64(i64 %24, i64 %27)
  br label %29

29:                                               ; preds = %20, %15
  %30 = phi i64 [ %16, %15 ], [ %28, %20 ]
  %31 = add nuw nsw i64 %17, 1
  %32 = icmp eq i64 %31, %1
  br i1 %32, label %11, label %15, !llvm.loop !11

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
  %49 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %48) #16
  br i1 %49, label %50, label %62

50:                                               ; preds = %45
  %51 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %46
  %52 = load i32, ptr %51, align 4, !tbaa !5
  %53 = icmp slt i32 %52, 0
  %54 = zext nneg i32 %52 to i64
  %55 = icmp slt i64 %30, %54
  %56 = select i1 %53, i1 true, i1 %55
  br i1 %56, label %62, label %57

57:                                               ; preds = %50
  %58 = getelementptr inbounds nuw i64, ptr %14, i64 %54
  %59 = load i64, ptr %58, align 8, !tbaa !13
  %60 = add nsw i64 %59, 1
  store i64 %60, ptr %58, align 8, !tbaa !13
  %61 = add nsw i64 %47, 1
  br label %62

62:                                               ; preds = %45, %50, %57
  %63 = phi i64 [ %61, %57 ], [ %47, %50 ], [ %47, %45 ]
  %64 = add nuw nsw i64 %46, 1
  %65 = icmp eq i64 %64, %1
  br i1 %65, label %33, label %45, !llvm.loop !15

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
  %75 = load i64, ptr %74, align 8, !tbaa !13
  %76 = add nsw i64 %75, %71
  store i64 %76, ptr %74, align 8, !tbaa !13
  %77 = add nuw i64 %72, 1
  %78 = add i64 %73, 1
  %79 = icmp eq i64 %78, %38
  br i1 %79, label %80, label %70, !llvm.loop !16

80:                                               ; preds = %66, %70, %33
  %81 = icmp sgt i64 %63, 0
  br i1 %81, label %101, label %104

82:                                               ; preds = %82, %40
  %83 = phi i64 [ %36, %40 ], [ %97, %82 ]
  %84 = phi i64 [ 1, %40 ], [ %98, %82 ]
  %85 = phi i64 [ 0, %40 ], [ %99, %82 ]
  %86 = getelementptr i64, ptr %13, i64 %84
  %87 = load i64, ptr %86, align 8, !tbaa !13
  %88 = add nsw i64 %87, %83
  store i64 %88, ptr %86, align 8, !tbaa !13
  %89 = getelementptr i64, ptr %42, i64 %84
  %90 = load i64, ptr %89, align 8, !tbaa !13
  %91 = add nsw i64 %90, %88
  store i64 %91, ptr %89, align 8, !tbaa !13
  %92 = getelementptr i64, ptr %43, i64 %84
  %93 = load i64, ptr %92, align 8, !tbaa !13
  %94 = add nsw i64 %93, %91
  store i64 %94, ptr %92, align 8, !tbaa !13
  %95 = getelementptr i64, ptr %44, i64 %84
  %96 = load i64, ptr %95, align 8, !tbaa !13
  %97 = add nsw i64 %96, %94
  store i64 %97, ptr %95, align 8, !tbaa !13
  %98 = add nuw i64 %84, 4
  %99 = add i64 %85, 4
  %100 = icmp eq i64 %99, %41
  br i1 %100, label %66, label %82, !llvm.loop !18

101:                                              ; preds = %80
  %102 = shl i64 %63, 2
  %103 = tail call noalias ptr @malloc(i64 noundef %102) #17
  br label %104

104:                                              ; preds = %9, %80, %101
  %105 = phi ptr [ %13, %101 ], [ %13, %80 ], [ %10, %9 ]
  %106 = phi i64 [ %12, %101 ], [ %12, %80 ], [ 1, %9 ]
  %107 = phi i64 [ %30, %101 ], [ %30, %80 ], [ -1, %9 ]
  %108 = phi i64 [ %63, %101 ], [ %63, %80 ], [ 0, %9 ]
  %109 = phi ptr [ %103, %101 ], [ null, %80 ], [ null, %9 ]
  %110 = shl nuw nsw i64 %106, 3
  %111 = tail call noalias ptr @malloc(i64 noundef %110) #17
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %111, ptr align 8 %105, i64 %110, i1 false)
  br i1 %8, label %113, label %112

112:                                              ; preds = %131, %104
  tail call void @free(ptr noundef %111) #16
  store ptr %105, ptr %4, align 8, !tbaa !19
  store ptr %109, ptr %5, align 8, !tbaa !22
  store i64 %108, ptr %6, align 8, !tbaa !13
  ret void

113:                                              ; preds = %104, %131
  %114 = phi i64 [ %132, %131 ], [ 0, %104 ]
  %115 = trunc i64 %114 to i32
  %116 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %115) #16
  br i1 %116, label %117, label %131

117:                                              ; preds = %113
  %118 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %114
  %119 = load i32, ptr %118, align 4, !tbaa !5
  %120 = icmp sgt i32 %119, -1
  br i1 %120, label %121, label %131

121:                                              ; preds = %117
  %122 = zext nneg i32 %119 to i64
  %123 = icmp slt i64 %107, %122
  br i1 %123, label %131, label %124

124:                                              ; preds = %121
  %125 = getelementptr inbounds nuw i8, ptr %118, i64 4
  %126 = load i32, ptr %125, align 4, !tbaa !10
  %127 = getelementptr inbounds nuw i64, ptr %111, i64 %122
  %128 = load i64, ptr %127, align 8, !tbaa !13
  %129 = add nsw i64 %128, 1
  store i64 %129, ptr %127, align 8, !tbaa !13
  %130 = getelementptr inbounds i32, ptr %109, i64 %128
  store i32 %126, ptr %130, align 4, !tbaa !24
  br label %131

131:                                              ; preds = %113, %117, %121, %124
  %132 = add nuw nsw i64 %114, 1
  %133 = icmp eq i64 %132, %1
  br i1 %133, label %112, label %113, !llvm.loop !25
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare zeroext i1 @roaring_bitmap_contains(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #6

; Function Attrs: nounwind uwtable
define dso_local void @rebuild_sets_from_csr(i64 noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef readonly %4, ptr noundef %5, ptr noundef %6, ptr nocapture noundef readonly %7) local_unnamed_addr #0 {
  tail call void @roaring_bitmap_clear(ptr noundef %5) #16
  tail call void @roaring_bitmap_clear(ptr noundef %6) #16
  %9 = trunc i64 %1 to i32
  %10 = shl i32 %9, 1
  %11 = add i32 %10, 32
  br label %12

12:                                               ; preds = %12, %8
  %13 = phi i32 [ 1, %8 ], [ %15, %12 ]
  %14 = icmp sgt i32 %13, %11
  %15 = shl i32 %13, 1
  br i1 %14, label %16, label %12, !llvm.loop !26

16:                                               ; preds = %12
  %17 = sext i32 %13 to i64
  %18 = tail call noalias ptr @calloc(i64 noundef %17, i64 noundef 24) #15
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
  %32 = load i32, ptr %31, align 4, !tbaa !5
  %33 = getelementptr inbounds nuw i8, ptr %31, i64 4
  %34 = load i32, ptr %33, align 4, !tbaa !10
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
  %52 = load i8, ptr %51, align 8, !tbaa !27
  %53 = icmp eq i8 %52, 0
  br i1 %53, label %68, label %54

54:                                               ; preds = %29, %61
  %55 = phi ptr [ %64, %61 ], [ %50, %29 ]
  %56 = phi i64 [ %63, %61 ], [ %49, %29 ]
  %57 = load i64, ptr %55, align 8, !tbaa !29
  %58 = icmp eq i64 %57, %40
  br i1 %58, label %59, label %61

59:                                               ; preds = %54
  %60 = getelementptr inbounds nuw i8, ptr %55, i64 8
  store i64 %30, ptr %60, align 8, !tbaa !30
  br label %73

61:                                               ; preds = %54
  %62 = add i64 %56, 1
  %63 = and i64 %62, %21
  %64 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %63
  %65 = getelementptr inbounds nuw i8, ptr %64, i64 16
  %66 = load i8, ptr %65, align 8, !tbaa !27
  %67 = icmp eq i8 %66, 0
  br i1 %67, label %68, label %54, !llvm.loop !31

68:                                               ; preds = %61, %29
  %69 = phi i64 [ %49, %29 ], [ %63, %61 ]
  %70 = phi ptr [ %50, %29 ], [ %64, %61 ]
  store i64 %40, ptr %70, align 8, !tbaa !29
  %71 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %69, i32 1
  store i64 %30, ptr %71, align 8, !tbaa !30
  %72 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %69, i32 2
  store i8 1, ptr %72, align 8, !tbaa !27
  br label %73

73:                                               ; preds = %59, %68
  %74 = add nuw nsw i64 %30, 1
  %75 = icmp eq i64 %74, %1
  br i1 %75, label %22, label %29, !llvm.loop !32

76:                                               ; preds = %132, %79
  %77 = icmp eq i64 %83, %0
  br i1 %77, label %78, label %79, !llvm.loop !33

78:                                               ; preds = %76, %22
  tail call void @free(ptr noundef %18) #16
  ret void

79:                                               ; preds = %24, %76
  %80 = phi i64 [ 0, %24 ], [ %83, %76 ]
  %81 = getelementptr inbounds nuw i64, ptr %3, i64 %80
  %82 = load i64, ptr %81, align 8, !tbaa !13
  %83 = add nuw nsw i64 %80, 1
  %84 = getelementptr inbounds nuw i64, ptr %3, i64 %83
  %85 = load i64, ptr %84, align 8, !tbaa !13
  %86 = icmp sgt i64 %85, %82
  br i1 %86, label %87, label %76

87:                                               ; preds = %79
  %88 = trunc i64 %80 to i32
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %88) #16
  %89 = trunc i64 %80 to i32
  br label %90

90:                                               ; preds = %87, %132
  %91 = phi i64 [ %82, %87 ], [ %133, %132 ]
  %92 = getelementptr inbounds i32, ptr %4, i64 %91
  %93 = load i32, ptr %92, align 4, !tbaa !24
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %93) #16
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
  %112 = load i8, ptr %111, align 8, !tbaa !27
  %113 = icmp eq i8 %112, 0
  br i1 %113, label %132, label %121

114:                                              ; preds = %121
  %115 = add i64 %123, 1
  %116 = and i64 %115, %28
  %117 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %116
  %118 = getelementptr inbounds nuw i8, ptr %117, i64 16
  %119 = load i8, ptr %118, align 8, !tbaa !27
  %120 = icmp eq i8 %119, 0
  br i1 %120, label %132, label %121, !llvm.loop !34

121:                                              ; preds = %94, %114
  %122 = phi ptr [ %117, %114 ], [ %110, %94 ]
  %123 = phi i64 [ %116, %114 ], [ %109, %94 ]
  %124 = load i64, ptr %122, align 8, !tbaa !29
  %125 = icmp eq i64 %124, %100
  br i1 %125, label %126, label %114

126:                                              ; preds = %121
  %127 = getelementptr inbounds nuw i8, ptr %122, i64 8
  %128 = load i64, ptr %127, align 8, !tbaa !30
  %129 = icmp sgt i64 %128, -1
  br i1 %129, label %130, label %132

130:                                              ; preds = %126
  %131 = trunc i64 %128 to i32
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %131) #16
  br label %132

132:                                              ; preds = %114, %94, %90, %130, %126
  %133 = add i64 %91, 1
  %134 = icmp eq i64 %133, %85
  br i1 %134, label %76, label %90, !llvm.loop !35
}

declare void @roaring_bitmap_clear(ptr noundef) local_unnamed_addr #2

declare void @roaring_bitmap_add(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_update_csr_pointers(ptr noundef readnone %0, ptr noundef %1, ptr noundef %2) local_unnamed_addr #7 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %19

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %19, label %11, !llvm.loop !36

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !37
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 80
  store ptr %1, ptr %17, align 8, !tbaa !40
  %18 = getelementptr inbounds nuw i8, ptr %13, i64 88
  store ptr %2, ptr %18, align 8, !tbaa !41
  br label %19

19:                                               ; preds = %8, %3, %16
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout_set(ptr noundef writeonly %0) local_unnamed_addr #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %167

7:                                                ; preds = %1
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %167, label %12, !llvm.loop !36

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 8, !tbaa !37
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 16
  %19 = load ptr, ptr %18, align 8, !tbaa !42
  %20 = icmp eq ptr %19, null
  br i1 %20, label %167, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %14, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !43
  %24 = icmp eq ptr %23, null
  br i1 %24, label %167, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %14, i64 32
  %27 = load ptr, ptr %26, align 8, !tbaa !44
  %28 = icmp eq ptr %27, null
  br i1 %28, label %167, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %14, i64 8
  %31 = load i32, ptr %30, align 8, !tbaa !45
  %32 = icmp eq i32 %31, 3
  br i1 %32, label %167, label %33

33:                                               ; preds = %29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #16
  store ptr null, ptr %2, align 8, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #16
  store ptr null, ptr %3, align 8, !tbaa !22
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %4) #16
  store i64 0, ptr %4, align 8, !tbaa !13
  switch i32 %31, label %70 [
    i32 0, label %34
    i32 1, label %46
    i32 2, label %56
  ]

34:                                               ; preds = %33
  %35 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %36 = load ptr, ptr %35, align 8, !tbaa !40
  store ptr %36, ptr %2, align 8, !tbaa !19
  %37 = getelementptr inbounds nuw i8, ptr %14, i64 88
  %38 = load ptr, ptr %37, align 8, !tbaa !41
  store ptr %38, ptr %3, align 8, !tbaa !22
  %39 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %40 = load i64, ptr %39, align 8, !tbaa !46
  %41 = getelementptr inbounds nuw i8, ptr %14, i64 104
  %42 = load i64, ptr %41, align 8, !tbaa !47
  store i64 %42, ptr %4, align 8, !tbaa !13
  %43 = getelementptr inbounds nuw i8, ptr %14, i64 112
  %44 = load i32, ptr %43, align 8, !tbaa !48
  %45 = icmp ne i32 %44, 0
  br label %70

46:                                               ; preds = %33
  %47 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %48 = load i64, ptr %47, align 8, !tbaa !46
  %49 = getelementptr inbounds nuw i8, ptr %14, i64 120
  %50 = load ptr, ptr %49, align 8, !tbaa !49
  %51 = getelementptr inbounds nuw i8, ptr %14, i64 128
  %52 = load ptr, ptr %51, align 8, !tbaa !50
  call void @convert_pcsr_to_csr(i64 noundef %48, i64 poison, ptr noundef %50, ptr noundef %52, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %53 = load i64, ptr %47, align 8, !tbaa !46
  %54 = load ptr, ptr %49, align 8, !tbaa !49
  tail call void @free(ptr noundef %54) #16
  store ptr null, ptr %49, align 8, !tbaa !49
  %55 = load ptr, ptr %51, align 8, !tbaa !50
  tail call void @free(ptr noundef %55) #16
  store ptr null, ptr %51, align 8, !tbaa !50
  br label %70

56:                                               ; preds = %33
  %57 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %58 = load i64, ptr %57, align 8, !tbaa !46
  %59 = getelementptr inbounds nuw i8, ptr %14, i64 164
  %60 = load i32, ptr %59, align 4, !tbaa !51
  %61 = getelementptr inbounds nuw i8, ptr %14, i64 160
  %62 = load i32, ptr %61, align 8, !tbaa !52
  %63 = getelementptr inbounds nuw i8, ptr %14, i64 144
  %64 = load ptr, ptr %63, align 8, !tbaa !53
  %65 = getelementptr inbounds nuw i8, ptr %14, i64 152
  %66 = load ptr, ptr %65, align 8, !tbaa !54
  call void @convert_bcsr_to_csr(i64 noundef %58, i32 noundef %60, i32 noundef %62, ptr noundef %64, ptr noundef %66, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %67 = load i64, ptr %57, align 8, !tbaa !46
  %68 = load ptr, ptr %63, align 8, !tbaa !53
  call void @free(ptr noundef %68) #16
  store ptr null, ptr %63, align 8, !tbaa !53
  %69 = load ptr, ptr %65, align 8, !tbaa !54
  call void @free(ptr noundef %69) #16
  store ptr null, ptr %65, align 8, !tbaa !54
  br label %70

70:                                               ; preds = %33, %46, %56, %34
  %71 = phi i1 [ %45, %34 ], [ true, %46 ], [ true, %56 ], [ false, %33 ]
  %72 = phi i64 [ %40, %34 ], [ %53, %46 ], [ %67, %56 ], [ 0, %33 ]
  %73 = load ptr, ptr %2, align 8, !tbaa !19
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
  call void @free(ptr noundef nonnull %81) #16
  br label %85

85:                                               ; preds = %84, %80
  %86 = load ptr, ptr %3, align 8
  %87 = icmp ne ptr %86, null
  %88 = select i1 %71, i1 %87, i1 false
  br i1 %88, label %89, label %90

89:                                               ; preds = %85
  call void @free(ptr noundef nonnull %86) #16
  br label %90

90:                                               ; preds = %89, %85
  %91 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %92 = getelementptr inbounds nuw i8, ptr %14, i64 112
  store i32 0, ptr %92, align 8, !tbaa !48
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %91, i8 0, i64 16, i1 false)
  %93 = load ptr, ptr %18, align 8, !tbaa !42
  %94 = call i64 @roaring_bitmap_get_cardinality(ptr noundef %93) #16
  %95 = icmp eq i64 %94, 0
  br i1 %95, label %101, label %96

96:                                               ; preds = %90
  %97 = add i64 %94, -1
  %98 = call i32 @roaring_bitmap_get_at_index(ptr noundef %93, i64 noundef %97) #16
  %99 = zext i32 %98 to i64
  %100 = add nuw nsw i64 %99, 1
  br label %101

101:                                              ; preds = %90, %96
  %102 = phi i64 [ %100, %96 ], [ 0, %90 ]
  store i64 0, ptr %4, align 8, !tbaa !13
  %103 = load ptr, ptr %22, align 8, !tbaa !43
  %104 = getelementptr inbounds nuw i8, ptr %14, i64 40
  %105 = load i64, ptr %104, align 8, !tbaa !55
  %106 = icmp sgt i64 %105, 0
  br i1 %106, label %119, label %107

107:                                              ; preds = %126, %101
  %108 = getelementptr inbounds nuw i8, ptr %14, i64 64
  %109 = load i64, ptr %108, align 8, !tbaa !56
  %110 = icmp sgt i64 %109, 0
  %111 = load i64, ptr %4, align 8
  br i1 %110, label %112, label %141

112:                                              ; preds = %107
  %113 = getelementptr inbounds nuw i8, ptr %14, i64 56
  %114 = load ptr, ptr %113, align 8, !tbaa !57
  %115 = and i64 %109, 1
  %116 = icmp eq i64 %109, 1
  br i1 %116, label %130, label %117

117:                                              ; preds = %112
  %118 = and i64 %109, 9223372036854775806
  br label %145

119:                                              ; preds = %101, %126
  %120 = phi i64 [ %127, %126 ], [ 0, %101 ]
  %121 = trunc i64 %120 to i32
  %122 = call zeroext i1 @roaring_bitmap_contains(ptr noundef %103, i32 noundef %121) #16
  br i1 %122, label %123, label %126

123:                                              ; preds = %119
  %124 = load i64, ptr %4, align 8, !tbaa !13
  %125 = add nsw i64 %124, 1
  store i64 %125, ptr %4, align 8, !tbaa !13
  br label %126

126:                                              ; preds = %119, %123
  %127 = add nuw nsw i64 %120, 1
  %128 = load i64, ptr %104, align 8, !tbaa !55
  %129 = icmp slt i64 %127, %128
  br i1 %129, label %119, label %107, !llvm.loop !58

130:                                              ; preds = %162, %112
  %131 = phi i64 [ poison, %112 ], [ %163, %162 ]
  %132 = phi i64 [ 0, %112 ], [ %164, %162 ]
  %133 = phi i64 [ %111, %112 ], [ %163, %162 ]
  %134 = icmp eq i64 %115, 0
  br i1 %134, label %141, label %135

135:                                              ; preds = %130
  %136 = getelementptr inbounds nuw i8, ptr %114, i64 %132
  %137 = load i8, ptr %136, align 1, !tbaa !59
  %138 = icmp eq i8 %137, 0
  br i1 %138, label %141, label %139

139:                                              ; preds = %135
  %140 = add nsw i64 %133, 1
  store i64 %140, ptr %4, align 8, !tbaa !13
  br label %141

141:                                              ; preds = %130, %139, %135, %107
  %142 = phi i64 [ %111, %107 ], [ %131, %130 ], [ %133, %135 ], [ %140, %139 ]
  store i64 %102, ptr %0, align 8, !tbaa !13
  %143 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %142, ptr %143, align 8, !tbaa !13
  %144 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %144, i8 0, i64 16, i1 false)
  store i32 3, ptr %30, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %4) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #16
  br label %167

145:                                              ; preds = %162, %117
  %146 = phi i64 [ 0, %117 ], [ %164, %162 ]
  %147 = phi i64 [ %111, %117 ], [ %163, %162 ]
  %148 = phi i64 [ 0, %117 ], [ %165, %162 ]
  %149 = getelementptr inbounds nuw i8, ptr %114, i64 %146
  %150 = load i8, ptr %149, align 1, !tbaa !59
  %151 = icmp eq i8 %150, 0
  br i1 %151, label %154, label %152

152:                                              ; preds = %145
  %153 = add nsw i64 %147, 1
  store i64 %153, ptr %4, align 8, !tbaa !13
  br label %154

154:                                              ; preds = %145, %152
  %155 = phi i64 [ %147, %145 ], [ %153, %152 ]
  %156 = or disjoint i64 %146, 1
  %157 = getelementptr inbounds nuw i8, ptr %114, i64 %156
  %158 = load i8, ptr %157, align 1, !tbaa !59
  %159 = icmp eq i8 %158, 0
  br i1 %159, label %162, label %160

160:                                              ; preds = %154
  %161 = add nsw i64 %155, 1
  store i64 %161, ptr %4, align 8, !tbaa !13
  br label %162

162:                                              ; preds = %160, %154
  %163 = phi i64 [ %155, %154 ], [ %161, %160 ]
  %164 = add nuw nsw i64 %146, 2
  %165 = add i64 %148, 2
  %166 = icmp eq i64 %165, %118
  br i1 %166, label %130, label %145, !llvm.loop !60

167:                                              ; preds = %9, %1, %29, %17, %21, %25, %141
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_pcsr_to_csr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = add i64 %0, 1
  %9 = tail call noalias ptr @calloc(i64 noundef %8, i64 noundef 8) #15
  %10 = icmp sgt i64 %0, 0
  br i1 %10, label %11, label %89

11:                                               ; preds = %7
  %12 = load i64, ptr %2, align 8, !tbaa !13
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
  %28 = load i64, ptr %27, align 8, !tbaa !13
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
  %43 = load <2 x i32>, ptr %41, align 4, !tbaa !24
  %44 = load <2 x i32>, ptr %42, align 4, !tbaa !24
  %45 = icmp ne <2 x i32> %43, splat (i32 -1)
  %46 = icmp ne <2 x i32> %44, splat (i32 -1)
  %47 = zext <2 x i1> %45 to <2 x i64>
  %48 = zext <2 x i1> %46 to <2 x i64>
  %49 = add <2 x i64> %39, %47
  %50 = add <2 x i64> %40, %48
  %51 = add nuw i64 %38, 4
  %52 = icmp eq i64 %51, %34
  br i1 %52, label %53, label %37, !llvm.loop !61

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
  store i64 %61, ptr %62, align 8, !tbaa !13
  %63 = add nuw nsw i64 %61, %24
  %64 = icmp eq i64 %26, %0
  br i1 %64, label %13, label %22, !llvm.loop !64

65:                                               ; preds = %57, %65
  %66 = phi i64 [ %72, %65 ], [ %58, %57 ]
  %67 = phi i64 [ %73, %65 ], [ %59, %57 ]
  %68 = getelementptr inbounds i32, ptr %3, i64 %67
  %69 = load i32, ptr %68, align 4, !tbaa !24
  %70 = icmp ne i32 %69, -1
  %71 = zext i1 %70 to i64
  %72 = add nuw nsw i64 %66, %71
  %73 = add nsw i64 %67, 1
  %74 = icmp eq i64 %73, %28
  br i1 %74, label %60, label %65, !llvm.loop !65

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
  %84 = load i64, ptr %83, align 8, !tbaa !13
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !13
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %15
  br i1 %88, label %89, label %79, !llvm.loop !66

89:                                               ; preds = %75, %79, %7
  %90 = phi i64 [ 0, %7 ], [ %63, %79 ], [ %63, %75 ]
  %91 = shl i64 %90, 2
  %92 = tail call noalias ptr @malloc(i64 noundef %91) #17
  %93 = shl i64 %8, 3
  %94 = tail call noalias ptr @malloc(i64 noundef %93) #17
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %94, ptr align 8 %9, i64 %93, i1 false)
  br i1 %10, label %95, label %119

95:                                               ; preds = %89
  %96 = load i64, ptr %2, align 8, !tbaa !13
  %97 = getelementptr i8, ptr %3, i64 4
  br label %120

98:                                               ; preds = %98, %17
  %99 = phi i64 [ %14, %17 ], [ %113, %98 ]
  %100 = phi i64 [ 1, %17 ], [ %114, %98 ]
  %101 = phi i64 [ 0, %17 ], [ %115, %98 ]
  %102 = getelementptr i64, ptr %9, i64 %100
  %103 = load i64, ptr %102, align 8, !tbaa !13
  %104 = add nsw i64 %103, %99
  store i64 %104, ptr %102, align 8, !tbaa !13
  %105 = getelementptr i64, ptr %19, i64 %100
  %106 = load i64, ptr %105, align 8, !tbaa !13
  %107 = add nsw i64 %106, %104
  store i64 %107, ptr %105, align 8, !tbaa !13
  %108 = getelementptr i64, ptr %20, i64 %100
  %109 = load i64, ptr %108, align 8, !tbaa !13
  %110 = add nsw i64 %109, %107
  store i64 %110, ptr %108, align 8, !tbaa !13
  %111 = getelementptr i64, ptr %21, i64 %100
  %112 = load i64, ptr %111, align 8, !tbaa !13
  %113 = add nsw i64 %112, %110
  store i64 %113, ptr %111, align 8, !tbaa !13
  %114 = add nuw i64 %100, 4
  %115 = add i64 %101, 4
  %116 = icmp eq i64 %115, %18
  br i1 %116, label %75, label %98, !llvm.loop !67

117:                                              ; preds = %143, %163, %120
  %118 = icmp eq i64 %123, %0
  br i1 %118, label %119, label %120, !llvm.loop !68

119:                                              ; preds = %117, %89
  tail call void @free(ptr noundef %94) #16
  store ptr %9, ptr %4, align 8, !tbaa !19
  store ptr %92, ptr %5, align 8, !tbaa !22
  store i64 %90, ptr %6, align 8, !tbaa !13
  ret void

120:                                              ; preds = %95, %117
  %121 = phi i64 [ %125, %117 ], [ %96, %95 ]
  %122 = phi i64 [ %123, %117 ], [ 0, %95 ]
  %123 = add nuw nsw i64 %122, 1
  %124 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %125 = load i64, ptr %124, align 8, !tbaa !13
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
  %135 = load i32, ptr %134, align 4, !tbaa !24
  %136 = icmp eq i32 %135, -1
  br i1 %136, label %141, label %137

137:                                              ; preds = %133
  %138 = load i64, ptr %128, align 8, !tbaa !13
  %139 = add nsw i64 %138, 1
  store i64 %139, ptr %128, align 8, !tbaa !13
  %140 = getelementptr inbounds i32, ptr %92, i64 %138
  store i32 %135, ptr %140, align 4, !tbaa !24
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
  %149 = load i32, ptr %148, align 4, !tbaa !24
  %150 = icmp eq i32 %149, -1
  br i1 %150, label %155, label %151

151:                                              ; preds = %146
  %152 = load i64, ptr %128, align 8, !tbaa !13
  %153 = add nsw i64 %152, 1
  store i64 %153, ptr %128, align 8, !tbaa !13
  %154 = getelementptr inbounds i32, ptr %92, i64 %152
  store i32 %149, ptr %154, align 4, !tbaa !24
  br label %155

155:                                              ; preds = %146, %151
  %156 = getelementptr i32, ptr %97, i64 %147
  %157 = load i32, ptr %156, align 4, !tbaa !24
  %158 = icmp eq i32 %157, -1
  br i1 %158, label %163, label %159

159:                                              ; preds = %155
  %160 = load i64, ptr %128, align 8, !tbaa !13
  %161 = add nsw i64 %160, 1
  store i64 %161, ptr %128, align 8, !tbaa !13
  %162 = getelementptr inbounds i32, ptr %92, i64 %160
  store i32 %157, ptr %162, align 4, !tbaa !24
  br label %163

163:                                              ; preds = %159, %155
  %164 = add nsw i64 %147, 2
  %165 = icmp eq i64 %164, %125
  br i1 %165, label %117, label %146, !llvm.loop !69
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
  %22 = tail call noalias ptr @calloc(i64 noundef %21, i64 noundef 8) #15
  %23 = icmp eq ptr %22, null
  br i1 %23, label %167, label %24

24:                                               ; preds = %20
  %25 = icmp eq i32 %1, 0
  br i1 %25, label %33, label %26

26:                                               ; preds = %24
  %27 = zext nneg i32 %2 to i64
  %28 = getelementptr i8, ptr %22, i64 8
  %29 = zext nneg i32 %1 to i64
  %30 = load i32, ptr %3, align 4, !tbaa !24
  br label %44

31:                                               ; preds = %71, %44
  %32 = icmp eq i64 %50, %29
  br i1 %32, label %33, label %44, !llvm.loop !70

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
  %52 = load i32, ptr %51, align 4, !tbaa !24
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
  %61 = load i32, ptr %60, align 4, !tbaa !24
  %62 = sext i32 %61 to i64
  %63 = add nsw i64 %47, %62
  %64 = icmp sgt i32 %61, -1
  %65 = icmp slt i64 %63, %49
  %66 = select i1 %64, i1 %65, i1 false
  br i1 %66, label %67, label %71

67:                                               ; preds = %57
  %68 = getelementptr i64, ptr %28, i64 %63
  %69 = load i64, ptr %68, align 8, !tbaa !13
  %70 = add nsw i64 %69, 1
  store i64 %70, ptr %68, align 8, !tbaa !13
  br label %71

71:                                               ; preds = %67, %57
  %72 = add nsw i64 %58, 2
  %73 = add i32 %59, 3
  %74 = icmp slt i32 %73, %52
  br i1 %74, label %57, label %31, !llvm.loop !71

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
  %84 = load i64, ptr %83, align 8, !tbaa !13
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !13
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %37
  br i1 %88, label %89, label %79, !llvm.loop !72

89:                                               ; preds = %75, %79, %33
  %90 = getelementptr inbounds i64, ptr %22, i64 %0
  %91 = load i64, ptr %90, align 8, !tbaa !13
  %92 = icmp sgt i64 %91, 0
  br i1 %92, label %112, label %117

93:                                               ; preds = %93, %39
  %94 = phi i64 [ %36, %39 ], [ %108, %93 ]
  %95 = phi i64 [ 1, %39 ], [ %109, %93 ]
  %96 = phi i64 [ 0, %39 ], [ %110, %93 ]
  %97 = getelementptr i64, ptr %22, i64 %95
  %98 = load i64, ptr %97, align 8, !tbaa !13
  %99 = add nsw i64 %98, %94
  store i64 %99, ptr %97, align 8, !tbaa !13
  %100 = getelementptr i64, ptr %41, i64 %95
  %101 = load i64, ptr %100, align 8, !tbaa !13
  %102 = add nsw i64 %101, %99
  store i64 %102, ptr %100, align 8, !tbaa !13
  %103 = getelementptr i64, ptr %42, i64 %95
  %104 = load i64, ptr %103, align 8, !tbaa !13
  %105 = add nsw i64 %104, %102
  store i64 %105, ptr %103, align 8, !tbaa !13
  %106 = getelementptr i64, ptr %43, i64 %95
  %107 = load i64, ptr %106, align 8, !tbaa !13
  %108 = add nsw i64 %107, %105
  store i64 %108, ptr %106, align 8, !tbaa !13
  %109 = add nuw i64 %95, 4
  %110 = add i64 %96, 4
  %111 = icmp eq i64 %110, %40
  br i1 %111, label %75, label %93, !llvm.loop !73

112:                                              ; preds = %89
  %113 = shl i64 %91, 2
  %114 = tail call noalias ptr @malloc(i64 noundef %113) #17
  %115 = icmp eq ptr %114, null
  br i1 %115, label %116, label %117

116:                                              ; preds = %112
  tail call void @free(ptr noundef nonnull %22) #16
  br label %167

117:                                              ; preds = %112, %89
  %118 = phi ptr [ %114, %112 ], [ null, %89 ]
  %119 = shl i64 %21, 3
  %120 = tail call noalias ptr @malloc(i64 noundef %119) #17
  %121 = icmp eq ptr %120, null
  br i1 %121, label %122, label %123

122:                                              ; preds = %117
  tail call void @free(ptr noundef nonnull %22) #16
  tail call void @free(ptr noundef %118) #16
  br label %167

123:                                              ; preds = %117
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 %120, ptr nonnull align 8 %22, i64 %119, i1 false)
  br i1 %25, label %130, label %124

124:                                              ; preds = %123
  %125 = zext nneg i32 %2 to i64
  %126 = zext nneg i32 %1 to i64
  %127 = load i32, ptr %3, align 4, !tbaa !24
  br label %131

128:                                              ; preds = %162, %131
  %129 = icmp eq i64 %137, %126
  br i1 %129, label %130, label %131, !llvm.loop !74

130:                                              ; preds = %128, %123
  tail call void @free(ptr noundef %120) #16
  store ptr %22, ptr %5, align 8, !tbaa !19
  store ptr %118, ptr %6, align 8, !tbaa !22
  store i64 %91, ptr %7, align 8, !tbaa !13
  br label %167

131:                                              ; preds = %124, %128
  %132 = phi i32 [ %127, %124 ], [ %139, %128 ]
  %133 = phi i64 [ 0, %124 ], [ %137, %128 ]
  %134 = mul nuw nsw i64 %133, %125
  %135 = add nuw nsw i64 %134, %125
  %136 = tail call i64 @llvm.smin.i64(i64 %135, i64 %0)
  %137 = add nuw nsw i64 %133, 1
  %138 = getelementptr inbounds nuw i32, ptr %3, i64 %137
  %139 = load i32, ptr %138, align 4, !tbaa !24
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
  %148 = load i32, ptr %147, align 4, !tbaa !24
  %149 = sext i32 %148 to i64
  %150 = add nsw i64 %134, %149
  %151 = icmp sgt i32 %148, -1
  %152 = icmp slt i64 %150, %136
  %153 = select i1 %151, i1 %152, i1 false
  br i1 %153, label %154, label %162

154:                                              ; preds = %144
  %155 = sext i32 %146 to i64
  %156 = getelementptr inbounds i32, ptr %4, i64 %155
  %157 = load i32, ptr %156, align 4, !tbaa !24
  %158 = getelementptr inbounds nuw i64, ptr %120, i64 %150
  %159 = load i64, ptr %158, align 8, !tbaa !13
  %160 = add nsw i64 %159, 1
  store i64 %160, ptr %158, align 8, !tbaa !13
  %161 = getelementptr inbounds i32, ptr %118, i64 %159
  store i32 %157, ptr %161, align 4, !tbaa !24
  br label %162

162:                                              ; preds = %154, %144
  %163 = add nsw i64 %145, 2
  %164 = trunc nsw i64 %145 to i32
  %165 = add i32 %164, 3
  %166 = icmp slt i32 %165, %139
  br i1 %166, label %144, label %128, !llvm.loop !75

167:                                              ; preds = %20, %122, %130, %116, %8
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @rebuild_sets_from_csr_meta(ptr nocapture noundef nonnull %0, ptr nocapture noundef nonnull readonly %1, ptr nocapture noundef nonnull readonly %2, i64 noundef %3) unnamed_addr #0 {
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !42
  %7 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %8 = load ptr, ptr %7, align 8, !tbaa !43
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %10 = load ptr, ptr %9, align 8, !tbaa !44
  tail call void @roaring_bitmap_clear(ptr noundef %6) #16
  tail call void @roaring_bitmap_clear(ptr noundef %8) #16
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %12 = load i64, ptr %11, align 8, !tbaa !76
  %13 = icmp sgt i64 %12, 0
  br i1 %13, label %14, label %16

14:                                               ; preds = %4
  %15 = getelementptr inbounds nuw i8, ptr %0, i64 56
  br label %33

16:                                               ; preds = %33, %4
  %17 = getelementptr inbounds nuw i8, ptr %0, i64 64
  store i64 0, ptr %17, align 8, !tbaa !56
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %19 = load i64, ptr %18, align 8, !tbaa !55
  %20 = trunc i64 %19 to i32
  %21 = shl i32 %20, 1
  %22 = add i32 %21, 32
  br label %23

23:                                               ; preds = %23, %16
  %24 = phi i32 [ 1, %16 ], [ %26, %23 ]
  %25 = icmp sgt i32 %24, %22
  %26 = shl i32 %24, 1
  br i1 %25, label %27, label %23, !llvm.loop !26

27:                                               ; preds = %23
  %28 = sext i32 %24 to i64
  %29 = tail call noalias ptr @calloc(i64 noundef %28, i64 noundef 24) #15
  %30 = icmp sgt i64 %19, 0
  br i1 %30, label %31, label %40

31:                                               ; preds = %27
  %32 = add nsw i64 %28, -1
  br label %49

33:                                               ; preds = %14, %33
  %34 = phi i64 [ 0, %14 ], [ %37, %33 ]
  %35 = load ptr, ptr %15, align 8, !tbaa !57
  %36 = getelementptr inbounds nuw i8, ptr %35, i64 %34
  store i8 0, ptr %36, align 1, !tbaa !59
  %37 = add nuw nsw i64 %34, 1
  %38 = load i64, ptr %11, align 8, !tbaa !76
  %39 = icmp slt i64 %37, %38
  br i1 %39, label %33, label %16, !llvm.loop !77

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
  %52 = load i32, ptr %51, align 4, !tbaa !5
  %53 = getelementptr inbounds nuw i8, ptr %51, i64 4
  %54 = load i32, ptr %53, align 4, !tbaa !10
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
  %72 = load i8, ptr %71, align 8, !tbaa !27
  %73 = icmp eq i8 %72, 0
  br i1 %73, label %88, label %74

74:                                               ; preds = %49, %81
  %75 = phi ptr [ %84, %81 ], [ %70, %49 ]
  %76 = phi i64 [ %83, %81 ], [ %69, %49 ]
  %77 = load i64, ptr %75, align 8, !tbaa !29
  %78 = icmp eq i64 %77, %60
  br i1 %78, label %79, label %81

79:                                               ; preds = %74
  %80 = getelementptr inbounds nuw i8, ptr %75, i64 8
  store i64 %50, ptr %80, align 8, !tbaa !30
  br label %93

81:                                               ; preds = %74
  %82 = add i64 %76, 1
  %83 = and i64 %82, %32
  %84 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %83
  %85 = getelementptr inbounds nuw i8, ptr %84, i64 16
  %86 = load i8, ptr %85, align 8, !tbaa !27
  %87 = icmp eq i8 %86, 0
  br i1 %87, label %88, label %74, !llvm.loop !31

88:                                               ; preds = %81, %49
  %89 = phi i64 [ %69, %49 ], [ %83, %81 ]
  %90 = phi ptr [ %70, %49 ], [ %84, %81 ]
  store i64 %60, ptr %90, align 8, !tbaa !29
  %91 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %89, i32 1
  store i64 %50, ptr %91, align 8, !tbaa !30
  %92 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %89, i32 2
  store i8 1, ptr %92, align 8, !tbaa !27
  br label %93

93:                                               ; preds = %79, %88
  %94 = add nuw nsw i64 %50, 1
  %95 = icmp eq i64 %94, %19
  br i1 %95, label %40, label %49, !llvm.loop !78

96:                                               ; preds = %218, %99
  %97 = icmp eq i64 %103, %3
  br i1 %97, label %98, label %99, !llvm.loop !79

98:                                               ; preds = %96, %40
  tail call void @free(ptr noundef %29) #16
  ret void

99:                                               ; preds = %42, %96
  %100 = phi i64 [ 0, %42 ], [ %103, %96 ]
  %101 = getelementptr inbounds nuw i64, ptr %1, i64 %100
  %102 = load i64, ptr %101, align 8, !tbaa !13
  %103 = add nuw nsw i64 %100, 1
  %104 = getelementptr inbounds nuw i64, ptr %1, i64 %103
  %105 = load i64, ptr %104, align 8, !tbaa !13
  %106 = icmp sgt i64 %105, %102
  br i1 %106, label %107, label %96

107:                                              ; preds = %99
  %108 = trunc i64 %100 to i32
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %108) #16
  %109 = trunc i64 %100 to i32
  br label %110

110:                                              ; preds = %107, %218
  %111 = phi i64 [ %102, %107 ], [ %219, %218 ]
  %112 = getelementptr inbounds i32, ptr %2, i64 %111
  %113 = load i32, ptr %112, align 4, !tbaa !24
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %113) #16
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
  %132 = load i8, ptr %131, align 8, !tbaa !27
  %133 = icmp eq i8 %132, 0
  br i1 %133, label %152, label %141

134:                                              ; preds = %141
  %135 = add i64 %143, 1
  %136 = and i64 %135, %46
  %137 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %29, i64 %136
  %138 = getelementptr inbounds nuw i8, ptr %137, i64 16
  %139 = load i8, ptr %138, align 8, !tbaa !27
  %140 = icmp eq i8 %139, 0
  br i1 %140, label %152, label %141, !llvm.loop !34

141:                                              ; preds = %114, %134
  %142 = phi ptr [ %137, %134 ], [ %130, %114 ]
  %143 = phi i64 [ %136, %134 ], [ %129, %114 ]
  %144 = load i64, ptr %142, align 8, !tbaa !29
  %145 = icmp eq i64 %144, %120
  br i1 %145, label %146, label %134

146:                                              ; preds = %141
  %147 = getelementptr inbounds nuw i8, ptr %142, i64 8
  %148 = load i64, ptr %147, align 8, !tbaa !30
  %149 = icmp sgt i64 %148, -1
  br i1 %149, label %150, label %152

150:                                              ; preds = %146
  %151 = trunc i64 %148 to i32
  tail call void @roaring_bitmap_add(ptr noundef %8, i32 noundef %151) #16
  br label %218

152:                                              ; preds = %134, %114, %110, %146
  %153 = load i64, ptr %17, align 8, !tbaa !56
  %154 = icmp slt i64 %153, 1
  br i1 %154, label %178, label %155

155:                                              ; preds = %152
  %156 = load ptr, ptr %47, align 8, !tbaa !80
  br label %157

157:                                              ; preds = %172, %155
  %158 = phi i64 [ 0, %155 ], [ %173, %172 ]
  %159 = shl nuw nsw i64 %158, 1
  %160 = getelementptr inbounds nuw i32, ptr %156, i64 %159
  %161 = load i32, ptr %160, align 4, !tbaa !24
  %162 = or disjoint i64 %159, 1
  %163 = getelementptr inbounds nuw i32, ptr %156, i64 %162
  %164 = load i32, ptr %163, align 4, !tbaa !24
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
  br i1 %174, label %178, label %157, !llvm.loop !81

175:                                              ; preds = %168, %157
  %176 = load ptr, ptr %48, align 8, !tbaa !57
  %177 = getelementptr inbounds nuw i8, ptr %176, i64 %158
  store i8 1, ptr %177, align 1, !tbaa !59
  br label %218

178:                                              ; preds = %172, %152
  %179 = load i64, ptr %11, align 8, !tbaa !76
  %180 = icmp slt i64 %153, %179
  br i1 %180, label %181, label %184

181:                                              ; preds = %178
  %182 = load ptr, ptr %47, align 8, !tbaa !80
  %183 = load ptr, ptr %48, align 8, !tbaa !57
  br label %208

184:                                              ; preds = %178
  %185 = icmp sgt i64 %179, 0
  %186 = select i1 %185, i64 %179, i64 16
  br label %187

187:                                              ; preds = %187, %184
  %188 = phi i64 [ %186, %184 ], [ %190, %187 ]
  %189 = icmp sgt i64 %188, %153
  %190 = shl nsw i64 %188, 1
  br i1 %189, label %191, label %187, !llvm.loop !82

191:                                              ; preds = %187
  %192 = load ptr, ptr %47, align 8, !tbaa !80
  %193 = shl i64 %188, 3
  %194 = tail call ptr @realloc(ptr noundef %192, i64 noundef %193) #18
  %195 = load ptr, ptr %48, align 8, !tbaa !57
  %196 = tail call ptr @realloc(ptr noundef %195, i64 noundef %188) #18
  %197 = icmp ne ptr %194, null
  %198 = icmp ne ptr %196, null
  %199 = and i1 %197, %198
  br i1 %199, label %200, label %218

200:                                              ; preds = %191
  %201 = load i64, ptr %11, align 8, !tbaa !76
  %202 = icmp sgt i64 %188, %201
  br i1 %202, label %203, label %206

203:                                              ; preds = %200
  %204 = getelementptr inbounds i8, ptr %196, i64 %201
  %205 = sub nsw i64 %188, %201
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %204, i8 0, i64 %205, i1 false)
  br label %206

206:                                              ; preds = %203, %200
  store ptr %194, ptr %47, align 8, !tbaa !80
  store ptr %196, ptr %48, align 8, !tbaa !57
  store i64 %188, ptr %11, align 8, !tbaa !76
  %207 = load i64, ptr %17, align 8, !tbaa !56
  br label %208

208:                                              ; preds = %181, %206
  %209 = phi ptr [ %183, %181 ], [ %196, %206 ]
  %210 = phi ptr [ %182, %181 ], [ %194, %206 ]
  %211 = phi i64 [ %153, %181 ], [ %207, %206 ]
  %212 = add nsw i64 %211, 1
  store i64 %212, ptr %17, align 8, !tbaa !56
  %213 = shl nsw i64 %211, 1
  %214 = getelementptr inbounds i32, ptr %210, i64 %213
  store i32 %109, ptr %214, align 4, !tbaa !24
  %215 = or disjoint i64 %213, 1
  %216 = getelementptr inbounds i32, ptr %210, i64 %215
  store i32 %113, ptr %216, align 4, !tbaa !24
  %217 = getelementptr inbounds i8, ptr %209, i64 %211
  store i8 1, ptr %217, align 1, !tbaa !59
  br label %218

218:                                              ; preds = %191, %175, %208, %150
  %219 = add i64 %111, 1
  %220 = icmp eq i64 %219, %105
  br i1 %220, label %96, label %110, !llvm.loop !83
}

declare i64 @roaring_bitmap_get_cardinality(ptr noundef) local_unnamed_addr #2

declare i32 @roaring_bitmap_get_at_index(ptr noundef, i64 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_pcsr(i64 noundef %0, i64 noundef %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = tail call i64 @llvm.smax.i64(i64 %1, i64 16)
  %9 = shl nuw i64 %8, 1
  %10 = shl i64 %8, 3
  %11 = tail call noalias ptr @malloc(i64 noundef %10) #17
  %12 = shl i64 %0, 3
  %13 = add i64 %12, 8
  %14 = tail call noalias ptr @malloc(i64 noundef %13) #17
  tail call void @llvm.memset.p0.i64(ptr align 4 %11, i8 -1, i64 %10, i1 false)
  %15 = icmp sgt i64 %0, 0
  br i1 %15, label %16, label %20

16:                                               ; preds = %7
  %17 = ptrtoint ptr %3 to i64
  br label %25

18:                                               ; preds = %84, %80, %50
  %19 = icmp eq i64 %30, %0
  br i1 %19, label %20, label %25, !llvm.loop !84

20:                                               ; preds = %18, %7
  %21 = phi i64 [ 0, %7 ], [ %38, %18 ]
  %22 = phi ptr [ %11, %7 ], [ %51, %18 ]
  %23 = phi i64 [ %9, %7 ], [ %52, %18 ]
  %24 = getelementptr inbounds i64, ptr %14, i64 %0
  store i64 %21, ptr %24, align 8, !tbaa !13
  store ptr %14, ptr %4, align 8, !tbaa !19
  store ptr %22, ptr %5, align 8, !tbaa !22
  store i64 %23, ptr %6, align 8, !tbaa !13
  ret void

25:                                               ; preds = %16, %18
  %26 = phi i64 [ %52, %18 ], [ %9, %16 ]
  %27 = phi ptr [ %51, %18 ], [ %11, %16 ]
  %28 = phi i64 [ %38, %18 ], [ 0, %16 ]
  %29 = phi i64 [ %30, %18 ], [ 0, %16 ]
  %30 = add nuw nsw i64 %29, 1
  %31 = getelementptr inbounds nuw i64, ptr %2, i64 %30
  %32 = load i64, ptr %31, align 8, !tbaa !13
  %33 = getelementptr inbounds nuw i64, ptr %2, i64 %29
  %34 = load i64, ptr %33, align 8, !tbaa !13
  %35 = sub nsw i64 %32, %34
  %36 = tail call i64 @llvm.smax.i64(i64 %35, i64 2)
  %37 = add i64 %35, %28
  %38 = add i64 %37, %36
  %39 = icmp sgt i64 %38, %26
  br i1 %39, label %40, label %50

40:                                               ; preds = %25, %40
  %41 = phi i64 [ %43, %40 ], [ %26, %25 ]
  %42 = icmp slt i64 %41, %38
  %43 = shl nsw i64 %41, 1
  br i1 %42, label %40, label %44, !llvm.loop !85

44:                                               ; preds = %40
  %45 = shl i64 %41, 2
  %46 = tail call ptr @realloc(ptr noundef %27, i64 noundef %45) #18
  %47 = getelementptr inbounds i32, ptr %46, i64 %26
  %48 = sub nsw i64 %41, %26
  %49 = shl i64 %48, 2
  tail call void @llvm.memset.p0.i64(ptr align 4 %47, i8 -1, i64 %49, i1 false)
  br label %50

50:                                               ; preds = %44, %25
  %51 = phi ptr [ %46, %44 ], [ %27, %25 ]
  %52 = phi i64 [ %41, %44 ], [ %26, %25 ]
  %53 = ptrtoint ptr %51 to i64
  %54 = getelementptr inbounds nuw i64, ptr %14, i64 %29
  store i64 %28, ptr %54, align 8, !tbaa !13
  %55 = icmp sgt i64 %35, 0
  br i1 %55, label %56, label %18

56:                                               ; preds = %50
  %57 = load i64, ptr %33, align 8, !tbaa !13
  %58 = getelementptr i32, ptr %3, i64 %57
  %59 = getelementptr i32, ptr %51, i64 %28
  %60 = icmp ult i64 %35, 8
  br i1 %60, label %82, label %61

61:                                               ; preds = %56
  %62 = shl i64 %28, 2
  %63 = shl i64 %57, 2
  %64 = add i64 %62, %53
  %65 = add i64 %63, %17
  %66 = sub i64 %64, %65
  %67 = icmp ult i64 %66, 32
  br i1 %67, label %82, label %68

68:                                               ; preds = %61
  %69 = and i64 %35, 9223372036854775800
  br label %70

70:                                               ; preds = %70, %68
  %71 = phi i64 [ 0, %68 ], [ %78, %70 ]
  %72 = getelementptr i32, ptr %58, i64 %71
  %73 = getelementptr i8, ptr %72, i64 16
  %74 = load <4 x i32>, ptr %72, align 4, !tbaa !24
  %75 = load <4 x i32>, ptr %73, align 4, !tbaa !24
  %76 = getelementptr i32, ptr %59, i64 %71
  %77 = getelementptr i8, ptr %76, i64 16
  store <4 x i32> %74, ptr %76, align 4, !tbaa !24
  store <4 x i32> %75, ptr %77, align 4, !tbaa !24
  %78 = add nuw i64 %71, 8
  %79 = icmp eq i64 %78, %69
  br i1 %79, label %80, label %70, !llvm.loop !86

80:                                               ; preds = %70
  %81 = icmp eq i64 %35, %69
  br i1 %81, label %18, label %82

82:                                               ; preds = %61, %56, %80
  %83 = phi i64 [ 0, %56 ], [ 0, %61 ], [ %69, %80 ]
  br label %84

84:                                               ; preds = %82, %84
  %85 = phi i64 [ %89, %84 ], [ %83, %82 ]
  %86 = getelementptr i32, ptr %58, i64 %85
  %87 = load i32, ptr %86, align 4, !tbaa !24
  %88 = getelementptr i32, ptr %59, i64 %85
  store i32 %87, ptr %88, align 4, !tbaa !24
  %89 = add nuw nsw i64 %85, 1
  %90 = icmp eq i64 %89, %35
  br i1 %90, label %18, label %84, !llvm.loop !87
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #8

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #9

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
  %25 = tail call noalias ptr @calloc(i64 noundef %24, i64 noundef 4) #15
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
  %48 = load i64, ptr %47, align 8, !tbaa !13
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
  %63 = load <2 x i64>, ptr %61, align 8, !tbaa !13
  %64 = load <2 x i64>, ptr %62, align 8, !tbaa !13
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
  br i1 %74, label %75, label %55, !llvm.loop !88

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
  %97 = load i64, ptr %96, align 8, !tbaa !13
  %98 = sub nsw i64 %97, %92
  %99 = tail call i64 @llvm.smax.i64(i64 %98, i64 0)
  %100 = add nuw nsw i64 %99, %94
  %101 = icmp slt i64 %95, %44
  br i1 %101, label %91, label %84, !llvm.loop !89

102:                                              ; preds = %87
  %103 = trunc nuw nsw i64 %89 to i32
  %104 = getelementptr inbounds nuw i32, ptr %28, i64 %33
  store i32 %103, ptr %104, align 4, !tbaa !24
  %105 = add nuw nsw i64 %33, 1
  %106 = icmp eq i64 %105, %31
  br i1 %106, label %108, label %32, !llvm.loop !90

107:                                              ; preds = %87
  tail call void @free(ptr noundef %25) #16
  br label %191

108:                                              ; preds = %102
  %109 = icmp eq i64 %89, 0
  br i1 %109, label %115, label %110

110:                                              ; preds = %108
  %111 = shl nuw nsw i64 %89, 2
  %112 = tail call noalias ptr @malloc(i64 noundef %111) #17
  %113 = icmp eq ptr %112, null
  br i1 %113, label %114, label %115

114:                                              ; preds = %110
  tail call void @free(ptr noundef nonnull %25) #16
  br label %191

115:                                              ; preds = %108, %110
  %116 = phi ptr [ %112, %110 ], [ null, %108 ]
  %117 = and i64 %20, 2147483647
  %118 = getelementptr i8, ptr %116, i64 16
  br label %121

119:                                              ; preds = %138, %27
  %120 = phi ptr [ null, %27 ], [ %116, %138 ]
  store ptr %25, ptr %4, align 8, !tbaa !22
  store ptr %120, ptr %5, align 8, !tbaa !22
  store i32 %21, ptr %7, align 4, !tbaa !24
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
  %129 = load i32, ptr %128, align 4, !tbaa !24
  %130 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %131 = load i64, ptr %130, align 8, !tbaa !13
  br label %141

132:                                              ; preds = %181, %176
  %133 = phi i64 [ %158, %176 ], [ %187, %181 ]
  %134 = trunc nsw i64 %133 to i32
  br label %135

135:                                              ; preds = %132, %141
  %136 = phi i32 [ %144, %141 ], [ %134, %132 ]
  %137 = icmp slt i64 %147, %125
  br i1 %137, label %141, label %138, !llvm.loop !91

138:                                              ; preds = %135, %121
  %139 = add nuw nsw i64 %122, 1
  %140 = icmp eq i64 %139, %117
  br i1 %140, label %119, label %121, !llvm.loop !92

141:                                              ; preds = %127, %135
  %142 = phi i64 [ %149, %135 ], [ %131, %127 ]
  %143 = phi i64 [ %147, %135 ], [ %123, %127 ]
  %144 = phi i32 [ %136, %135 ], [ %129, %127 ]
  %145 = sub nuw nsw i64 %143, %123
  %146 = trunc i64 %145 to i32
  %147 = add nuw nsw i64 %143, 1
  %148 = getelementptr inbounds nuw i64, ptr %2, i64 %147
  %149 = load i64, ptr %148, align 8, !tbaa !13
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
  %170 = load <2 x i32>, ptr %168, align 4, !tbaa !24
  %171 = load <2 x i32>, ptr %169, align 4, !tbaa !24
  %172 = shufflevector <2 x i32> %160, <2 x i32> %170, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %172, ptr %166, align 4, !tbaa !24
  %173 = shufflevector <2 x i32> %160, <2 x i32> %171, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %173, ptr %167, align 4, !tbaa !24
  %174 = add nuw i64 %163, 4
  %175 = icmp eq i64 %174, %156
  br i1 %175, label %176, label %162, !llvm.loop !93

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
  store i32 %146, ptr %184, align 4, !tbaa !24
  %185 = getelementptr inbounds i32, ptr %3, i64 %183
  %186 = load i32, ptr %185, align 4, !tbaa !24
  %187 = add nsw i64 %182, 2
  %188 = getelementptr i8, ptr %184, i64 4
  store i32 %186, ptr %188, align 4, !tbaa !24
  %189 = add nsw i64 %183, 1
  %190 = icmp eq i64 %189, %149
  br i1 %190, label %132, label %181, !llvm.loop !94

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
  %24 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %25 = icmp sgt i32 %24, 0
  br i1 %25, label %26, label %333

26:                                               ; preds = %9
  %27 = zext nneg i32 %24 to i64
  br label %31

28:                                               ; preds = %31
  %29 = add nuw nsw i64 %32, 1
  %30 = icmp eq i64 %29, %27
  br i1 %30, label %333, label %31, !llvm.loop !36

31:                                               ; preds = %28, %26
  %32 = phi i64 [ 0, %26 ], [ %29, %28 ]
  %33 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %32
  %34 = load ptr, ptr %33, align 8, !tbaa !37
  %35 = icmp eq ptr %34, %0
  br i1 %35, label %36, label %28

36:                                               ; preds = %31
  %37 = icmp eq ptr %5, null
  br i1 %37, label %40, label %38

38:                                               ; preds = %36
  %39 = getelementptr inbounds nuw i8, ptr %33, i64 16
  store ptr %5, ptr %39, align 8, !tbaa !42
  br label %40

40:                                               ; preds = %38, %36
  %41 = icmp eq ptr %6, null
  br i1 %41, label %44, label %42

42:                                               ; preds = %40
  %43 = getelementptr inbounds nuw i8, ptr %33, i64 24
  store ptr %6, ptr %43, align 8, !tbaa !43
  br label %44

44:                                               ; preds = %42, %40
  %45 = icmp eq ptr %7, null
  br i1 %45, label %48, label %46

46:                                               ; preds = %44
  %47 = getelementptr inbounds nuw i8, ptr %33, i64 32
  store ptr %7, ptr %47, align 8, !tbaa !44
  br label %48

48:                                               ; preds = %46, %44
  %49 = getelementptr inbounds nuw i8, ptr %33, i64 16
  %50 = load ptr, ptr %49, align 8, !tbaa !42
  %51 = icmp eq ptr %50, null
  br i1 %51, label %333, label %52

52:                                               ; preds = %48
  %53 = getelementptr inbounds nuw i8, ptr %33, i64 24
  %54 = load ptr, ptr %53, align 8, !tbaa !43
  %55 = icmp eq ptr %54, null
  br i1 %55, label %333, label %56

56:                                               ; preds = %52
  %57 = getelementptr inbounds nuw i8, ptr %33, i64 32
  %58 = load ptr, ptr %57, align 8, !tbaa !44
  %59 = icmp eq ptr %58, null
  %60 = icmp ugt i32 %8, 3
  %61 = or i1 %60, %59
  br i1 %61, label %333, label %62

62:                                               ; preds = %56
  %63 = getelementptr inbounds nuw i8, ptr %33, i64 8
  %64 = load i32, ptr %63, align 8, !tbaa !45
  %65 = icmp eq i32 %64, %8
  br i1 %65, label %333, label %66

66:                                               ; preds = %62
  %67 = icmp eq i32 %8, 3
  br i1 %67, label %68, label %202

68:                                               ; preds = %66
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #16
  store ptr null, ptr %10, align 8, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %11) #16
  store ptr null, ptr %11, align 8, !tbaa !22
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %12) #16
  store i64 0, ptr %12, align 8, !tbaa !13
  switch i32 %64, label %105 [
    i32 0, label %69
    i32 1, label %81
    i32 2, label %91
  ]

69:                                               ; preds = %68
  %70 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %71 = load ptr, ptr %70, align 8, !tbaa !40
  store ptr %71, ptr %10, align 8, !tbaa !19
  %72 = getelementptr inbounds nuw i8, ptr %33, i64 88
  %73 = load ptr, ptr %72, align 8, !tbaa !41
  store ptr %73, ptr %11, align 8, !tbaa !22
  %74 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %75 = load i64, ptr %74, align 8, !tbaa !46
  %76 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %77 = load i64, ptr %76, align 8, !tbaa !47
  store i64 %77, ptr %12, align 8, !tbaa !13
  %78 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %79 = load i32, ptr %78, align 8, !tbaa !48
  %80 = icmp ne i32 %79, 0
  br label %105

81:                                               ; preds = %68
  %82 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %83 = load i64, ptr %82, align 8, !tbaa !46
  %84 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %85 = load ptr, ptr %84, align 8, !tbaa !49
  %86 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %87 = load ptr, ptr %86, align 8, !tbaa !50
  call void @convert_pcsr_to_csr(i64 noundef %83, i64 poison, ptr noundef %85, ptr noundef %87, ptr noundef nonnull %10, ptr noundef nonnull %11, ptr noundef nonnull %12)
  %88 = load i64, ptr %82, align 8, !tbaa !46
  %89 = load ptr, ptr %84, align 8, !tbaa !49
  tail call void @free(ptr noundef %89) #16
  store ptr null, ptr %84, align 8, !tbaa !49
  %90 = load ptr, ptr %86, align 8, !tbaa !50
  tail call void @free(ptr noundef %90) #16
  store ptr null, ptr %86, align 8, !tbaa !50
  br label %105

91:                                               ; preds = %68
  %92 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %93 = load i64, ptr %92, align 8, !tbaa !46
  %94 = getelementptr inbounds nuw i8, ptr %33, i64 164
  %95 = load i32, ptr %94, align 4, !tbaa !51
  %96 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %97 = load i32, ptr %96, align 8, !tbaa !52
  %98 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %99 = load ptr, ptr %98, align 8, !tbaa !53
  %100 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %101 = load ptr, ptr %100, align 8, !tbaa !54
  call void @convert_bcsr_to_csr(i64 noundef %93, i32 noundef %95, i32 noundef %97, ptr noundef %99, ptr noundef %101, ptr noundef nonnull %10, ptr noundef nonnull %11, ptr noundef nonnull %12)
  %102 = load i64, ptr %92, align 8, !tbaa !46
  %103 = load ptr, ptr %98, align 8, !tbaa !53
  call void @free(ptr noundef %103) #16
  store ptr null, ptr %98, align 8, !tbaa !53
  %104 = load ptr, ptr %100, align 8, !tbaa !54
  call void @free(ptr noundef %104) #16
  store ptr null, ptr %100, align 8, !tbaa !54
  br label %105

105:                                              ; preds = %68, %81, %91, %69
  %106 = phi i1 [ %80, %69 ], [ true, %81 ], [ true, %91 ], [ false, %68 ]
  %107 = phi i64 [ %75, %69 ], [ %88, %81 ], [ %102, %91 ], [ 0, %68 ]
  %108 = load ptr, ptr %10, align 8, !tbaa !19
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
  call void @free(ptr noundef nonnull %116) #16
  br label %120

120:                                              ; preds = %119, %115
  %121 = load ptr, ptr %11, align 8
  %122 = icmp ne ptr %121, null
  %123 = select i1 %106, i1 %122, i1 false
  br i1 %123, label %124, label %125

124:                                              ; preds = %120
  call void @free(ptr noundef nonnull %121) #16
  br label %125

125:                                              ; preds = %124, %120
  %126 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %127 = getelementptr inbounds nuw i8, ptr %33, i64 112
  store i32 0, ptr %127, align 8, !tbaa !48
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %126, i8 0, i64 16, i1 false)
  %128 = load ptr, ptr %49, align 8, !tbaa !42
  %129 = call i64 @roaring_bitmap_get_cardinality(ptr noundef %128) #16
  %130 = icmp eq i64 %129, 0
  br i1 %130, label %136, label %131

131:                                              ; preds = %125
  %132 = add i64 %129, -1
  %133 = call i32 @roaring_bitmap_get_at_index(ptr noundef %128, i64 noundef %132) #16
  %134 = zext i32 %133 to i64
  %135 = add nuw nsw i64 %134, 1
  br label %136

136:                                              ; preds = %125, %131
  %137 = phi i64 [ %135, %131 ], [ 0, %125 ]
  store i64 0, ptr %12, align 8, !tbaa !13
  %138 = load ptr, ptr %53, align 8, !tbaa !43
  %139 = getelementptr inbounds nuw i8, ptr %33, i64 40
  %140 = load i64, ptr %139, align 8, !tbaa !55
  %141 = icmp sgt i64 %140, 0
  br i1 %141, label %154, label %142

142:                                              ; preds = %161, %136
  %143 = getelementptr inbounds nuw i8, ptr %33, i64 64
  %144 = load i64, ptr %143, align 8, !tbaa !56
  %145 = icmp sgt i64 %144, 0
  %146 = load i64, ptr %12, align 8
  br i1 %145, label %147, label %176

147:                                              ; preds = %142
  %148 = getelementptr inbounds nuw i8, ptr %33, i64 56
  %149 = load ptr, ptr %148, align 8, !tbaa !57
  %150 = and i64 %144, 1
  %151 = icmp eq i64 %144, 1
  br i1 %151, label %165, label %152

152:                                              ; preds = %147
  %153 = and i64 %144, 9223372036854775806
  br label %180

154:                                              ; preds = %136, %161
  %155 = phi i64 [ %162, %161 ], [ 0, %136 ]
  %156 = trunc i64 %155 to i32
  %157 = call zeroext i1 @roaring_bitmap_contains(ptr noundef %138, i32 noundef %156) #16
  br i1 %157, label %158, label %161

158:                                              ; preds = %154
  %159 = load i64, ptr %12, align 8, !tbaa !13
  %160 = add nsw i64 %159, 1
  store i64 %160, ptr %12, align 8, !tbaa !13
  br label %161

161:                                              ; preds = %154, %158
  %162 = add nuw nsw i64 %155, 1
  %163 = load i64, ptr %139, align 8, !tbaa !55
  %164 = icmp slt i64 %162, %163
  br i1 %164, label %154, label %142, !llvm.loop !95

165:                                              ; preds = %197, %147
  %166 = phi i64 [ poison, %147 ], [ %198, %197 ]
  %167 = phi i64 [ 0, %147 ], [ %199, %197 ]
  %168 = phi i64 [ %146, %147 ], [ %198, %197 ]
  %169 = icmp eq i64 %150, 0
  br i1 %169, label %176, label %170

170:                                              ; preds = %165
  %171 = getelementptr inbounds nuw i8, ptr %149, i64 %167
  %172 = load i8, ptr %171, align 1, !tbaa !59
  %173 = icmp eq i8 %172, 0
  br i1 %173, label %176, label %174

174:                                              ; preds = %170
  %175 = add nsw i64 %168, 1
  store i64 %175, ptr %12, align 8, !tbaa !13
  br label %176

176:                                              ; preds = %165, %174, %170, %142
  %177 = phi i64 [ %146, %142 ], [ %166, %165 ], [ %168, %170 ], [ %175, %174 ]
  store i64 %137, ptr %0, align 8, !tbaa !13
  %178 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %177, ptr %178, align 8, !tbaa !13
  %179 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %179, i8 0, i64 16, i1 false)
  store i32 3, ptr %63, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %12) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %11) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #16
  br label %333

180:                                              ; preds = %197, %152
  %181 = phi i64 [ 0, %152 ], [ %199, %197 ]
  %182 = phi i64 [ %146, %152 ], [ %198, %197 ]
  %183 = phi i64 [ 0, %152 ], [ %200, %197 ]
  %184 = getelementptr inbounds nuw i8, ptr %149, i64 %181
  %185 = load i8, ptr %184, align 1, !tbaa !59
  %186 = icmp eq i8 %185, 0
  br i1 %186, label %189, label %187

187:                                              ; preds = %180
  %188 = add nsw i64 %182, 1
  store i64 %188, ptr %12, align 8, !tbaa !13
  br label %189

189:                                              ; preds = %180, %187
  %190 = phi i64 [ %182, %180 ], [ %188, %187 ]
  %191 = or disjoint i64 %181, 1
  %192 = getelementptr inbounds nuw i8, ptr %149, i64 %191
  %193 = load i8, ptr %192, align 1, !tbaa !59
  %194 = icmp eq i8 %193, 0
  br i1 %194, label %197, label %195

195:                                              ; preds = %189
  %196 = add nsw i64 %190, 1
  store i64 %196, ptr %12, align 8, !tbaa !13
  br label %197

197:                                              ; preds = %195, %189
  %198 = phi i64 [ %190, %189 ], [ %196, %195 ]
  %199 = add nuw nsw i64 %181, 2
  %200 = add i64 %183, 2
  %201 = icmp eq i64 %200, %153
  br i1 %201, label %165, label %180, !llvm.loop !96

202:                                              ; preds = %66
  %203 = icmp eq i32 %64, 3
  br i1 %203, label %204, label %227

204:                                              ; preds = %202
  %205 = getelementptr inbounds nuw i8, ptr %33, i64 96
  switch i32 %8, label %219 [
    i32 0, label %206
    i32 1, label %211
  ]

206:                                              ; preds = %204
  %207 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %208 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %209 = getelementptr inbounds nuw i8, ptr %33, i64 88
  %210 = getelementptr inbounds nuw i8, ptr %33, i64 104
  tail call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %207, ptr noundef %208, ptr noundef %209, ptr noundef %210)
  store i32 0, ptr %63, align 8, !tbaa !45
  br label %308

211:                                              ; preds = %204
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %13) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %14) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %15) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %16) #16
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %14, ptr noundef %15, ptr noundef %16, ptr noundef %13)
  %212 = load i64, ptr %14, align 8, !tbaa !13
  store i64 %212, ptr %205, align 8, !tbaa !46
  %213 = load i64, ptr %13, align 8, !tbaa !13
  %214 = load ptr, ptr %15, align 8, !tbaa !19
  %215 = load ptr, ptr %16, align 8, !tbaa !22
  %216 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %217 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %218 = getelementptr inbounds nuw i8, ptr %33, i64 136
  tail call void @convert_csr_to_pcsr(i64 noundef %212, i64 noundef %213, ptr noundef %214, ptr noundef %215, ptr noundef nonnull %216, ptr noundef nonnull %217, ptr noundef nonnull %218)
  tail call void @free(ptr noundef %214) #16
  tail call void @free(ptr noundef %215) #16
  store i32 1, ptr %63, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %16) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %15) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %14) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %13) #16
  br label %317

219:                                              ; preds = %204
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %17) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %18) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %19) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %20) #16
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %18, ptr noundef %19, ptr noundef %20, ptr noundef %17)
  %220 = load i64, ptr %18, align 8, !tbaa !13
  store i64 %220, ptr %205, align 8, !tbaa !46
  %221 = getelementptr inbounds nuw i8, ptr %33, i64 160
  store i32 64, ptr %221, align 8, !tbaa !52
  %222 = load ptr, ptr %19, align 8, !tbaa !19
  %223 = load ptr, ptr %20, align 8, !tbaa !22
  %224 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %225 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %226 = getelementptr inbounds nuw i8, ptr %33, i64 164
  tail call void @convert_csr_to_bcsr(i64 noundef %220, i64 poison, ptr noundef %222, ptr noundef %223, ptr noundef nonnull %224, ptr noundef nonnull %225, i32 noundef 64, ptr noundef nonnull %226)
  tail call void @free(ptr noundef %222) #16
  tail call void @free(ptr noundef %223) #16
  store i32 2, ptr %63, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %20) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %19) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %18) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %17) #16
  br label %326

227:                                              ; preds = %202
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %21) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %22) #16
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %23) #16
  switch i32 %64, label %250 [
    i32 1, label %228
    i32 2, label %237
  ]

228:                                              ; preds = %227
  %229 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %230 = load i64, ptr %229, align 8, !tbaa !46
  %231 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %232 = load ptr, ptr %231, align 8, !tbaa !49
  %233 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %234 = load ptr, ptr %233, align 8, !tbaa !50
  call void @convert_pcsr_to_csr(i64 noundef %230, i64 poison, ptr noundef %232, ptr noundef %234, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %235 = load ptr, ptr %231, align 8, !tbaa !49
  tail call void @free(ptr noundef %235) #16
  store ptr null, ptr %231, align 8, !tbaa !49
  %236 = load ptr, ptr %233, align 8, !tbaa !50
  tail call void @free(ptr noundef %236) #16
  store ptr null, ptr %233, align 8, !tbaa !50
  br label %259

237:                                              ; preds = %227
  %238 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %239 = load i64, ptr %238, align 8, !tbaa !46
  %240 = getelementptr inbounds nuw i8, ptr %33, i64 164
  %241 = load i32, ptr %240, align 4, !tbaa !51
  %242 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %243 = load i32, ptr %242, align 8, !tbaa !52
  %244 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %245 = load ptr, ptr %244, align 8, !tbaa !53
  %246 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %247 = load ptr, ptr %246, align 8, !tbaa !54
  call void @convert_bcsr_to_csr(i64 noundef %239, i32 noundef %241, i32 noundef %243, ptr noundef %245, ptr noundef %247, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %248 = load ptr, ptr %244, align 8, !tbaa !53
  call void @free(ptr noundef %248) #16
  store ptr null, ptr %244, align 8, !tbaa !53
  %249 = load ptr, ptr %246, align 8, !tbaa !54
  call void @free(ptr noundef %249) #16
  store ptr null, ptr %246, align 8, !tbaa !54
  br label %259

250:                                              ; preds = %227
  %251 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %252 = load i64, ptr %251, align 8, !tbaa !47
  store i64 %252, ptr %21, align 8, !tbaa !13
  %253 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %254 = load ptr, ptr %253, align 8, !tbaa !40
  store ptr %254, ptr %22, align 8, !tbaa !19
  %255 = getelementptr inbounds nuw i8, ptr %33, i64 88
  %256 = load ptr, ptr %255, align 8, !tbaa !41
  store ptr %256, ptr %23, align 8, !tbaa !22
  %257 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %258 = load i32, ptr %257, align 8, !tbaa !48
  br label %259

259:                                              ; preds = %237, %250, %228
  %260 = phi i32 [ 1, %228 ], [ 1, %237 ], [ %258, %250 ]
  switch i32 %8, label %279 [
    i32 0, label %299
    i32 1, label %261
  ]

261:                                              ; preds = %259
  %262 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %263 = load i64, ptr %262, align 8, !tbaa !46
  %264 = load i64, ptr %21, align 8, !tbaa !13
  %265 = load ptr, ptr %22, align 8, !tbaa !19
  %266 = load ptr, ptr %23, align 8, !tbaa !22
  %267 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %268 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %269 = getelementptr inbounds nuw i8, ptr %33, i64 136
  call void @convert_csr_to_pcsr(i64 noundef %263, i64 noundef %264, ptr noundef %265, ptr noundef %266, ptr noundef nonnull %267, ptr noundef nonnull %268, ptr noundef nonnull %269)
  %270 = icmp eq i32 %260, 0
  br i1 %270, label %274, label %271

271:                                              ; preds = %261
  %272 = load ptr, ptr %22, align 8, !tbaa !19
  call void @free(ptr noundef %272) #16
  %273 = load ptr, ptr %23, align 8, !tbaa !22
  call void @free(ptr noundef %273) #16
  br label %274

274:                                              ; preds = %271, %261
  %275 = load i32, ptr %63, align 8, !tbaa !45
  %276 = icmp eq i32 %275, 0
  br i1 %276, label %277, label %298

277:                                              ; preds = %274
  %278 = getelementptr inbounds nuw i8, ptr %33, i64 80
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %278, i8 0, i64 16, i1 false)
  br label %298

279:                                              ; preds = %259
  %280 = getelementptr inbounds nuw i8, ptr %33, i64 160
  store i32 64, ptr %280, align 8, !tbaa !52
  %281 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %282 = load i64, ptr %281, align 8, !tbaa !46
  %283 = load ptr, ptr %22, align 8, !tbaa !19
  %284 = load ptr, ptr %23, align 8, !tbaa !22
  %285 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %286 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %287 = getelementptr inbounds nuw i8, ptr %33, i64 164
  call void @convert_csr_to_bcsr(i64 noundef %282, i64 poison, ptr noundef %283, ptr noundef %284, ptr noundef nonnull %285, ptr noundef nonnull %286, i32 noundef 64, ptr noundef nonnull %287)
  %288 = icmp eq i32 %260, 0
  br i1 %288, label %292, label %289

289:                                              ; preds = %279
  %290 = load ptr, ptr %22, align 8, !tbaa !19
  call void @free(ptr noundef %290) #16
  %291 = load ptr, ptr %23, align 8, !tbaa !22
  call void @free(ptr noundef %291) #16
  br label %292

292:                                              ; preds = %289, %279
  %293 = load i32, ptr %63, align 8, !tbaa !45
  %294 = icmp eq i32 %293, 0
  br i1 %294, label %295, label %297

295:                                              ; preds = %292
  %296 = getelementptr inbounds nuw i8, ptr %33, i64 80
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %296, i8 0, i64 16, i1 false)
  br label %297

297:                                              ; preds = %295, %292
  store i32 %8, ptr %63, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #16
  br label %326

298:                                              ; preds = %274, %277
  store i32 %8, ptr %63, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #16
  br label %317

299:                                              ; preds = %259
  %300 = load i64, ptr %21, align 8, !tbaa !13
  %301 = getelementptr inbounds nuw i8, ptr %33, i64 104
  store i64 %300, ptr %301, align 8, !tbaa !47
  %302 = load ptr, ptr %22, align 8, !tbaa !19
  %303 = getelementptr inbounds nuw i8, ptr %33, i64 80
  store ptr %302, ptr %303, align 8, !tbaa !40
  %304 = load ptr, ptr %23, align 8, !tbaa !22
  %305 = getelementptr inbounds nuw i8, ptr %33, i64 88
  store ptr %304, ptr %305, align 8, !tbaa !41
  %306 = getelementptr inbounds nuw i8, ptr %33, i64 112
  store i32 %260, ptr %306, align 8, !tbaa !48
  store i32 %8, ptr %63, align 8, !tbaa !45
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #16
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #16
  %307 = getelementptr inbounds nuw i8, ptr %33, i64 96
  br label %308

308:                                              ; preds = %299, %206
  %309 = phi ptr [ %307, %299 ], [ %205, %206 ]
  %310 = load i64, ptr %309, align 8, !tbaa !46
  store i64 %310, ptr %0, align 8, !tbaa !13
  %311 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %312 = load i64, ptr %311, align 8, !tbaa !47
  %313 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %312, ptr %313, align 8, !tbaa !13
  %314 = getelementptr inbounds nuw i8, ptr %33, i64 80
  %315 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %316 = load <2 x ptr>, ptr %314, align 8, !tbaa !97
  store <2 x ptr> %316, ptr %315, align 8, !tbaa !97
  br label %333

317:                                              ; preds = %298, %211
  %318 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %319 = load i64, ptr %318, align 8, !tbaa !46
  store i64 %319, ptr %0, align 8, !tbaa !13
  %320 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %321 = load i64, ptr %320, align 8, !tbaa !47
  %322 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %321, ptr %322, align 8, !tbaa !13
  %323 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %324 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %325 = load <2 x ptr>, ptr %323, align 8, !tbaa !97
  store <2 x ptr> %325, ptr %324, align 8, !tbaa !97
  br label %333

326:                                              ; preds = %297, %219
  %327 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %328 = load i64, ptr %327, align 8, !tbaa !46
  store i64 %328, ptr %0, align 8, !tbaa !13
  %329 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %330 = load i64, ptr %329, align 8, !tbaa !47
  %331 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %330, ptr %331, align 8, !tbaa !13
  %332 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %332, i8 0, i64 16, i1 false)
  br label %333

333:                                              ; preds = %28, %9, %308, %326, %317, %62, %48, %52, %56, %176
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @build_csr_from_meta(ptr nocapture noundef nonnull %0, ptr nocapture noundef nonnull writeonly %1, ptr nocapture noundef nonnull writeonly %2, ptr nocapture noundef nonnull writeonly %3, ptr nocapture noundef nonnull writeonly %4) unnamed_addr #0 {
  %6 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %7 = load ptr, ptr %6, align 8, !tbaa !42
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %9 = load ptr, ptr %8, align 8, !tbaa !43
  %10 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %11 = load ptr, ptr %10, align 8, !tbaa !44
  %12 = tail call i64 @roaring_bitmap_get_cardinality(ptr noundef %7) #16
  %13 = icmp slt i64 %12, 1
  br i1 %13, label %20, label %14

14:                                               ; preds = %5
  %15 = add nsw i64 %12, -1
  %16 = tail call i32 @roaring_bitmap_get_at_index(ptr noundef %7, i64 noundef %15) #16
  %17 = tail call i32 @llvm.smax.i32(i32 %16, i32 -1)
  %18 = add i32 %17, 1
  %19 = zext i32 %18 to i64
  br label %20

20:                                               ; preds = %14, %5
  %21 = phi i64 [ %19, %14 ], [ 0, %5 ]
  %22 = add nuw nsw i64 %21, 1
  %23 = tail call noalias ptr @calloc(i64 noundef %22, i64 noundef 8) #15
  %24 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %25 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %26 = load i64, ptr %24, align 8, !tbaa !55
  %27 = icmp sgt i64 %26, 0
  br i1 %27, label %36, label %28

28:                                               ; preds = %64, %20
  %29 = phi i64 [ 0, %20 ], [ %65, %64 ]
  %30 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %31 = load i64, ptr %30, align 8, !tbaa !56
  %32 = icmp sgt i64 %31, 0
  br i1 %32, label %33, label %69

33:                                               ; preds = %28
  %34 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %35 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %82

36:                                               ; preds = %20, %64
  %37 = phi i64 [ %65, %64 ], [ 0, %20 ]
  %38 = phi i64 [ %66, %64 ], [ 0, %20 ]
  %39 = trunc i64 %38 to i32
  %40 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %39) #16
  br i1 %40, label %41, label %64

41:                                               ; preds = %36
  %42 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %38
  %43 = load i32, ptr %42, align 4, !tbaa !5
  %44 = getelementptr inbounds nuw i8, ptr %42, i64 4
  %45 = load i32, ptr %44, align 4, !tbaa !10
  %46 = icmp sgt i32 %43, -1
  %47 = icmp sgt i32 %45, -1
  %48 = select i1 %46, i1 %47, i1 false
  br i1 %48, label %49, label %64

49:                                               ; preds = %41
  %50 = zext nneg i32 %43 to i64
  %51 = icmp samesign ugt i64 %21, %50
  %52 = zext nneg i32 %45 to i64
  %53 = icmp samesign ugt i64 %21, %52
  %54 = select i1 %51, i1 %53, i1 false
  br i1 %54, label %55, label %64

55:                                               ; preds = %49
  %56 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %43) #16
  br i1 %56, label %57, label %64

57:                                               ; preds = %55
  %58 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %45) #16
  br i1 %58, label %59, label %64

59:                                               ; preds = %57
  %60 = getelementptr inbounds nuw i64, ptr %25, i64 %50
  %61 = load i64, ptr %60, align 8, !tbaa !13
  %62 = add nsw i64 %61, 1
  store i64 %62, ptr %60, align 8, !tbaa !13
  %63 = add nsw i64 %37, 1
  br label %64

64:                                               ; preds = %41, %49, %55, %57, %59, %36
  %65 = phi i64 [ %37, %36 ], [ %63, %59 ], [ %37, %57 ], [ %37, %55 ], [ %37, %49 ], [ %37, %41 ]
  %66 = add nuw nsw i64 %38, 1
  %67 = load i64, ptr %24, align 8, !tbaa !55
  %68 = icmp slt i64 %66, %67
  br i1 %68, label %36, label %28, !llvm.loop !98

69:                                               ; preds = %115, %28
  %70 = phi i64 [ %31, %28 ], [ %118, %115 ]
  %71 = phi i64 [ %29, %28 ], [ %116, %115 ]
  %72 = icmp eq i64 %21, 0
  br i1 %72, label %134, label %73

73:                                               ; preds = %69
  %74 = load i64, ptr %23, align 8
  %75 = and i64 %21, 3
  %76 = icmp samesign ult i64 %21, 4
  br i1 %76, label %120, label %77

77:                                               ; preds = %73
  %78 = and i64 %21, 4294967292
  %79 = getelementptr i8, ptr %23, i64 8
  %80 = getelementptr i8, ptr %23, i64 16
  %81 = getelementptr i8, ptr %23, i64 24
  br label %136

82:                                               ; preds = %33, %115
  %83 = phi i64 [ %29, %33 ], [ %116, %115 ]
  %84 = phi i64 [ 0, %33 ], [ %117, %115 ]
  %85 = load ptr, ptr %34, align 8, !tbaa !57
  %86 = getelementptr inbounds nuw i8, ptr %85, i64 %84
  %87 = load i8, ptr %86, align 1, !tbaa !59
  %88 = icmp eq i8 %87, 0
  br i1 %88, label %115, label %89

89:                                               ; preds = %82
  %90 = load ptr, ptr %35, align 8, !tbaa !80
  %91 = shl nuw nsw i64 %84, 1
  %92 = getelementptr inbounds nuw i32, ptr %90, i64 %91
  %93 = load i32, ptr %92, align 4, !tbaa !24
  %94 = or disjoint i64 %91, 1
  %95 = getelementptr inbounds nuw i32, ptr %90, i64 %94
  %96 = load i32, ptr %95, align 4, !tbaa !24
  %97 = icmp sgt i32 %93, -1
  %98 = icmp sgt i32 %96, -1
  %99 = select i1 %97, i1 %98, i1 false
  br i1 %99, label %100, label %115

100:                                              ; preds = %89
  %101 = zext nneg i32 %93 to i64
  %102 = icmp samesign ugt i64 %21, %101
  %103 = zext nneg i32 %96 to i64
  %104 = icmp samesign ugt i64 %21, %103
  %105 = select i1 %102, i1 %104, i1 false
  br i1 %105, label %106, label %115

106:                                              ; preds = %100
  %107 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %93) #16
  br i1 %107, label %108, label %115

108:                                              ; preds = %106
  %109 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %96) #16
  br i1 %109, label %110, label %115

110:                                              ; preds = %108
  %111 = getelementptr inbounds nuw i64, ptr %25, i64 %101
  %112 = load i64, ptr %111, align 8, !tbaa !13
  %113 = add nsw i64 %112, 1
  store i64 %113, ptr %111, align 8, !tbaa !13
  %114 = add nsw i64 %83, 1
  br label %115

115:                                              ; preds = %89, %100, %106, %108, %110, %82
  %116 = phi i64 [ %83, %82 ], [ %114, %110 ], [ %83, %108 ], [ %83, %106 ], [ %83, %100 ], [ %83, %89 ]
  %117 = add nuw nsw i64 %84, 1
  %118 = load i64, ptr %30, align 8, !tbaa !56
  %119 = icmp slt i64 %117, %118
  br i1 %119, label %82, label %69, !llvm.loop !99

120:                                              ; preds = %136, %73
  %121 = phi i64 [ %74, %73 ], [ %151, %136 ]
  %122 = phi i64 [ 1, %73 ], [ %152, %136 ]
  %123 = icmp eq i64 %75, 0
  br i1 %123, label %134, label %124

124:                                              ; preds = %120, %124
  %125 = phi i64 [ %130, %124 ], [ %121, %120 ]
  %126 = phi i64 [ %131, %124 ], [ %122, %120 ]
  %127 = phi i64 [ %132, %124 ], [ 0, %120 ]
  %128 = getelementptr i64, ptr %23, i64 %126
  %129 = load i64, ptr %128, align 8, !tbaa !13
  %130 = add nsw i64 %129, %125
  store i64 %130, ptr %128, align 8, !tbaa !13
  %131 = add nuw nsw i64 %126, 1
  %132 = add i64 %127, 1
  %133 = icmp eq i64 %132, %75
  br i1 %133, label %134, label %124, !llvm.loop !100

134:                                              ; preds = %120, %124, %69
  %135 = icmp sgt i64 %71, 0
  br i1 %135, label %155, label %158

136:                                              ; preds = %136, %77
  %137 = phi i64 [ %74, %77 ], [ %151, %136 ]
  %138 = phi i64 [ 1, %77 ], [ %152, %136 ]
  %139 = phi i64 [ 0, %77 ], [ %153, %136 ]
  %140 = getelementptr i64, ptr %23, i64 %138
  %141 = load i64, ptr %140, align 8, !tbaa !13
  %142 = add nsw i64 %141, %137
  store i64 %142, ptr %140, align 8, !tbaa !13
  %143 = getelementptr i64, ptr %79, i64 %138
  %144 = load i64, ptr %143, align 8, !tbaa !13
  %145 = add nsw i64 %144, %142
  store i64 %145, ptr %143, align 8, !tbaa !13
  %146 = getelementptr i64, ptr %80, i64 %138
  %147 = load i64, ptr %146, align 8, !tbaa !13
  %148 = add nsw i64 %147, %145
  store i64 %148, ptr %146, align 8, !tbaa !13
  %149 = getelementptr i64, ptr %81, i64 %138
  %150 = load i64, ptr %149, align 8, !tbaa !13
  %151 = add nsw i64 %150, %148
  store i64 %151, ptr %149, align 8, !tbaa !13
  %152 = add nuw nsw i64 %138, 4
  %153 = add i64 %139, 4
  %154 = icmp eq i64 %153, %78
  br i1 %154, label %120, label %136, !llvm.loop !101

155:                                              ; preds = %134
  %156 = shl i64 %71, 2
  %157 = tail call noalias ptr @malloc(i64 noundef %156) #17
  br label %158

158:                                              ; preds = %134, %155
  %159 = phi ptr [ %157, %155 ], [ null, %134 ]
  %160 = shl nuw nsw i64 %22, 3
  %161 = tail call noalias ptr @malloc(i64 noundef %160) #17
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %161, ptr noundef nonnull align 8 dereferenceable(1) %23, i64 %160, i1 false)
  %162 = load i64, ptr %24, align 8, !tbaa !55
  %163 = icmp sgt i64 %162, 0
  br i1 %163, label %172, label %166

164:                                              ; preds = %199
  %165 = load i64, ptr %30, align 8, !tbaa !56
  br label %166

166:                                              ; preds = %164, %158
  %167 = phi i64 [ %165, %164 ], [ %70, %158 ]
  %168 = icmp sgt i64 %167, 0
  br i1 %168, label %169, label %203

169:                                              ; preds = %166
  %170 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %171 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %205

172:                                              ; preds = %158, %199
  %173 = phi i64 [ %200, %199 ], [ 0, %158 ]
  %174 = trunc i64 %173 to i32
  %175 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %174) #16
  br i1 %175, label %176, label %199

176:                                              ; preds = %172
  %177 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %173
  %178 = load i32, ptr %177, align 4, !tbaa !5
  %179 = getelementptr inbounds nuw i8, ptr %177, i64 4
  %180 = load i32, ptr %179, align 4, !tbaa !10
  %181 = icmp sgt i32 %178, -1
  %182 = icmp sgt i32 %180, -1
  %183 = select i1 %181, i1 %182, i1 false
  br i1 %183, label %184, label %199

184:                                              ; preds = %176
  %185 = zext nneg i32 %178 to i64
  %186 = icmp samesign ugt i64 %21, %185
  %187 = zext nneg i32 %180 to i64
  %188 = icmp samesign ugt i64 %21, %187
  %189 = select i1 %186, i1 %188, i1 false
  br i1 %189, label %190, label %199

190:                                              ; preds = %184
  %191 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %178) #16
  br i1 %191, label %192, label %199

192:                                              ; preds = %190
  %193 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %180) #16
  br i1 %193, label %194, label %199

194:                                              ; preds = %192
  %195 = getelementptr inbounds nuw i64, ptr %161, i64 %185
  %196 = load i64, ptr %195, align 8, !tbaa !13
  %197 = add nsw i64 %196, 1
  store i64 %197, ptr %195, align 8, !tbaa !13
  %198 = getelementptr inbounds i32, ptr %159, i64 %196
  store i32 %180, ptr %198, align 4, !tbaa !24
  br label %199

199:                                              ; preds = %176, %184, %190, %192, %194, %172
  %200 = add nuw nsw i64 %173, 1
  %201 = load i64, ptr %24, align 8, !tbaa !55
  %202 = icmp slt i64 %200, %201
  br i1 %202, label %172, label %164, !llvm.loop !102

203:                                              ; preds = %237, %166
  tail call void @free(ptr noundef %161) #16
  store i64 %21, ptr %1, align 8, !tbaa !13
  store ptr %23, ptr %2, align 8, !tbaa !19
  store ptr %159, ptr %3, align 8, !tbaa !22
  store i64 %71, ptr %4, align 8, !tbaa !13
  %204 = getelementptr inbounds nuw i8, ptr %0, i64 112
  store i32 1, ptr %204, align 8, !tbaa !48
  ret void

205:                                              ; preds = %169, %237
  %206 = phi i64 [ 0, %169 ], [ %238, %237 ]
  %207 = load ptr, ptr %170, align 8, !tbaa !57
  %208 = getelementptr inbounds nuw i8, ptr %207, i64 %206
  %209 = load i8, ptr %208, align 1, !tbaa !59
  %210 = icmp eq i8 %209, 0
  br i1 %210, label %237, label %211

211:                                              ; preds = %205
  %212 = load ptr, ptr %171, align 8, !tbaa !80
  %213 = shl nuw nsw i64 %206, 1
  %214 = getelementptr inbounds nuw i32, ptr %212, i64 %213
  %215 = load i32, ptr %214, align 4, !tbaa !24
  %216 = or disjoint i64 %213, 1
  %217 = getelementptr inbounds nuw i32, ptr %212, i64 %216
  %218 = load i32, ptr %217, align 4, !tbaa !24
  %219 = icmp sgt i32 %215, -1
  %220 = icmp sgt i32 %218, -1
  %221 = select i1 %219, i1 %220, i1 false
  br i1 %221, label %222, label %237

222:                                              ; preds = %211
  %223 = zext nneg i32 %215 to i64
  %224 = icmp samesign ugt i64 %21, %223
  %225 = zext nneg i32 %218 to i64
  %226 = icmp samesign ugt i64 %21, %225
  %227 = select i1 %224, i1 %226, i1 false
  br i1 %227, label %228, label %237

228:                                              ; preds = %222
  %229 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %215) #16
  br i1 %229, label %230, label %237

230:                                              ; preds = %228
  %231 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %7, i32 noundef %218) #16
  br i1 %231, label %232, label %237

232:                                              ; preds = %230
  %233 = getelementptr inbounds nuw i64, ptr %161, i64 %223
  %234 = load i64, ptr %233, align 8, !tbaa !13
  %235 = add nsw i64 %234, 1
  store i64 %235, ptr %233, align 8, !tbaa !13
  %236 = getelementptr inbounds i32, ptr %159, i64 %234
  store i32 %218, ptr %236, align 4, !tbaa !24
  br label %237

237:                                              ; preds = %211, %222, %228, %230, %232, %205
  %238 = add nuw nsw i64 %206, 1
  %239 = load i64, ptr %30, align 8, !tbaa !56
  %240 = icmp slt i64 %238, %239
  br i1 %240, label %205, label %203, !llvm.loop !103
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @autograph_get_layout(ptr noundef readnone %0) local_unnamed_addr #10 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %17

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %17, label %9, !llvm.loop !36

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 8, !tbaa !37
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !45
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
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %32

7:                                                ; preds = %4
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %32, label %12, !llvm.loop !36

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 8, !tbaa !37
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = icmp eq ptr %1, null
  br i1 %18, label %22, label %19

19:                                               ; preds = %17
  %20 = getelementptr inbounds nuw i8, ptr %14, i64 16
  %21 = load ptr, ptr %20, align 8, !tbaa !42
  store ptr %21, ptr %1, align 8, !tbaa !97
  br label %22

22:                                               ; preds = %19, %17
  %23 = icmp eq ptr %2, null
  br i1 %23, label %27, label %24

24:                                               ; preds = %22
  %25 = getelementptr inbounds nuw i8, ptr %14, i64 24
  %26 = load ptr, ptr %25, align 8, !tbaa !43
  store ptr %26, ptr %2, align 8, !tbaa !97
  br label %27

27:                                               ; preds = %24, %22
  %28 = icmp eq ptr %3, null
  br i1 %28, label %32, label %29

29:                                               ; preds = %27
  %30 = getelementptr inbounds nuw i8, ptr %14, i64 32
  %31 = load ptr, ptr %30, align 8, !tbaa !44
  store ptr %31, ptr %3, align 8, !tbaa !97
  br label %32

32:                                               ; preds = %9, %4, %27, %29
  %33 = phi i32 [ 1, %29 ], [ 1, %27 ], [ 0, %4 ], [ 0, %9 ]
  ret i32 %33
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %20

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %20, label %10, !llvm.loop !36

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 8, !tbaa !37
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !42
  %18 = icmp eq ptr %17, null
  br i1 %18, label %20, label %19

19:                                               ; preds = %15
  tail call void @roaring_bitmap_add(ptr noundef nonnull %17, i32 noundef %1) #16
  br label %20

20:                                               ; preds = %7, %2, %15, %19
  %21 = phi i32 [ 1, %19 ], [ 0, %15 ], [ 0, %2 ], [ 0, %7 ]
  ret i32 %21
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %78

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %78, label %10, !llvm.loop !36

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 8, !tbaa !37
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !42
  %18 = icmp eq ptr %17, null
  br i1 %18, label %78, label %19

19:                                               ; preds = %15
  %20 = getelementptr inbounds nuw i8, ptr %12, i64 24
  %21 = load ptr, ptr %20, align 8, !tbaa !43
  %22 = icmp eq ptr %21, null
  br i1 %22, label %78, label %23

23:                                               ; preds = %19
  tail call void @roaring_bitmap_remove(ptr noundef nonnull %17, i32 noundef %1) #16
  %24 = getelementptr inbounds nuw i8, ptr %12, i64 32
  %25 = load ptr, ptr %24, align 8, !tbaa !44
  %26 = load ptr, ptr %20, align 8, !tbaa !43
  %27 = getelementptr inbounds nuw i8, ptr %12, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !55
  %29 = icmp sgt i64 %28, 0
  br i1 %29, label %37, label %30

30:                                               ; preds = %50, %23
  %31 = getelementptr inbounds nuw i8, ptr %12, i64 64
  %32 = load i64, ptr %31, align 8, !tbaa !56
  %33 = icmp sgt i64 %32, 0
  br i1 %33, label %34, label %78

34:                                               ; preds = %30
  %35 = getelementptr inbounds nuw i8, ptr %12, i64 56
  %36 = getelementptr inbounds nuw i8, ptr %12, i64 48
  br label %54

37:                                               ; preds = %23, %50
  %38 = phi i64 [ %51, %50 ], [ 0, %23 ]
  %39 = trunc i64 %38 to i32
  %40 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %26, i32 noundef %39) #16
  br i1 %40, label %41, label %50

41:                                               ; preds = %37
  %42 = getelementptr inbounds nuw %struct.EdgePair, ptr %25, i64 %38
  %43 = load i32, ptr %42, align 4, !tbaa !5
  %44 = icmp eq i32 %43, %1
  br i1 %44, label %49, label %45

45:                                               ; preds = %41
  %46 = getelementptr inbounds nuw i8, ptr %42, i64 4
  %47 = load i32, ptr %46, align 4, !tbaa !10
  %48 = icmp eq i32 %47, %1
  br i1 %48, label %49, label %50

49:                                               ; preds = %45, %41
  tail call void @roaring_bitmap_remove(ptr noundef %26, i32 noundef %39) #16
  br label %50

50:                                               ; preds = %45, %49, %37
  %51 = add nuw nsw i64 %38, 1
  %52 = load i64, ptr %27, align 8, !tbaa !55
  %53 = icmp slt i64 %51, %52
  br i1 %53, label %37, label %30, !llvm.loop !104

54:                                               ; preds = %34, %74
  %55 = phi i64 [ %32, %34 ], [ %75, %74 ]
  %56 = phi i64 [ 0, %34 ], [ %76, %74 ]
  %57 = load ptr, ptr %35, align 8, !tbaa !57
  %58 = getelementptr inbounds nuw i8, ptr %57, i64 %56
  %59 = load i8, ptr %58, align 1, !tbaa !59
  %60 = icmp eq i8 %59, 0
  br i1 %60, label %74, label %61

61:                                               ; preds = %54
  %62 = load ptr, ptr %36, align 8, !tbaa !80
  %63 = shl nuw nsw i64 %56, 1
  %64 = getelementptr inbounds nuw i32, ptr %62, i64 %63
  %65 = load i32, ptr %64, align 4, !tbaa !24
  %66 = icmp eq i32 %65, %1
  br i1 %66, label %72, label %67

67:                                               ; preds = %61
  %68 = or disjoint i64 %63, 1
  %69 = getelementptr inbounds nuw i32, ptr %62, i64 %68
  %70 = load i32, ptr %69, align 4, !tbaa !24
  %71 = icmp eq i32 %70, %1
  br i1 %71, label %72, label %74

72:                                               ; preds = %67, %61
  store i8 0, ptr %58, align 1, !tbaa !59
  %73 = load i64, ptr %31, align 8, !tbaa !56
  br label %74

74:                                               ; preds = %67, %72, %54
  %75 = phi i64 [ %55, %67 ], [ %73, %72 ], [ %55, %54 ]
  %76 = add nuw nsw i64 %56, 1
  %77 = icmp slt i64 %76, %75
  br i1 %77, label %54, label %78, !llvm.loop !105

78:                                               ; preds = %7, %74, %30, %2, %15, %19
  %79 = phi i32 [ 0, %19 ], [ 0, %15 ], [ 0, %2 ], [ 1, %30 ], [ 1, %74 ], [ 0, %7 ]
  ret i32 %79
}

declare void @roaring_bitmap_remove(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %104

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %104, label %11, !llvm.loop !36

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !37
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 16
  %18 = load ptr, ptr %17, align 8, !tbaa !42
  %19 = icmp eq ptr %18, null
  br i1 %19, label %104, label %20

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %22 = load ptr, ptr %21, align 8, !tbaa !43
  %23 = icmp eq ptr %22, null
  br i1 %23, label %104, label %24

24:                                               ; preds = %20
  tail call void @roaring_bitmap_add(ptr noundef nonnull %18, i32 noundef %1) #16
  %25 = load ptr, ptr %17, align 8, !tbaa !42
  tail call void @roaring_bitmap_add(ptr noundef %25, i32 noundef %2) #16
  %26 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %27 = icmp sgt i32 %26, -1
  br i1 %27, label %28, label %30

28:                                               ; preds = %24
  %29 = load ptr, ptr %21, align 8, !tbaa !43
  tail call void @roaring_bitmap_add(ptr noundef %29, i32 noundef %26) #16
  br label %104

30:                                               ; preds = %24
  %31 = getelementptr inbounds nuw i8, ptr %13, i64 64
  %32 = load i64, ptr %31, align 8, !tbaa !56
  %33 = icmp slt i64 %32, 1
  br i1 %33, label %59, label %34

34:                                               ; preds = %30
  %35 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %36 = load ptr, ptr %35, align 8, !tbaa !80
  br label %37

37:                                               ; preds = %52, %34
  %38 = phi i64 [ 0, %34 ], [ %53, %52 ]
  %39 = shl nuw nsw i64 %38, 1
  %40 = getelementptr inbounds nuw i32, ptr %36, i64 %39
  %41 = load i32, ptr %40, align 4, !tbaa !24
  %42 = or disjoint i64 %39, 1
  %43 = getelementptr inbounds nuw i32, ptr %36, i64 %42
  %44 = load i32, ptr %43, align 4, !tbaa !24
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
  br i1 %54, label %59, label %37, !llvm.loop !81

55:                                               ; preds = %48, %37
  %56 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %57 = load ptr, ptr %56, align 8, !tbaa !57
  %58 = getelementptr inbounds nuw i8, ptr %57, i64 %38
  store i8 1, ptr %58, align 1, !tbaa !59
  br label %104

59:                                               ; preds = %52, %30
  %60 = getelementptr inbounds nuw i8, ptr %13, i64 72
  %61 = load i64, ptr %60, align 8, !tbaa !76
  %62 = icmp slt i64 %32, %61
  br i1 %62, label %63, label %68

63:                                               ; preds = %59
  %64 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %65 = load ptr, ptr %64, align 8, !tbaa !80
  %66 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %67 = load ptr, ptr %66, align 8, !tbaa !57
  br label %94

68:                                               ; preds = %59
  %69 = icmp sgt i64 %61, 0
  %70 = select i1 %69, i64 %61, i64 16
  br label %71

71:                                               ; preds = %71, %68
  %72 = phi i64 [ %70, %68 ], [ %74, %71 ]
  %73 = icmp sgt i64 %72, %32
  %74 = shl nsw i64 %72, 1
  br i1 %73, label %75, label %71, !llvm.loop !82

75:                                               ; preds = %71
  %76 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %77 = load ptr, ptr %76, align 8, !tbaa !80
  %78 = shl i64 %72, 3
  %79 = tail call ptr @realloc(ptr noundef %77, i64 noundef %78) #18
  %80 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %81 = load ptr, ptr %80, align 8, !tbaa !57
  %82 = tail call ptr @realloc(ptr noundef %81, i64 noundef %72) #18
  %83 = icmp ne ptr %79, null
  %84 = icmp ne ptr %82, null
  %85 = and i1 %83, %84
  br i1 %85, label %86, label %104

86:                                               ; preds = %75
  %87 = load i64, ptr %60, align 8, !tbaa !76
  %88 = icmp sgt i64 %72, %87
  br i1 %88, label %89, label %92

89:                                               ; preds = %86
  %90 = getelementptr inbounds i8, ptr %82, i64 %87
  %91 = sub nsw i64 %72, %87
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %90, i8 0, i64 %91, i1 false)
  br label %92

92:                                               ; preds = %89, %86
  store ptr %79, ptr %76, align 8, !tbaa !80
  store ptr %82, ptr %80, align 8, !tbaa !57
  store i64 %72, ptr %60, align 8, !tbaa !76
  %93 = load i64, ptr %31, align 8, !tbaa !56
  br label %94

94:                                               ; preds = %63, %92
  %95 = phi ptr [ %67, %63 ], [ %82, %92 ]
  %96 = phi ptr [ %65, %63 ], [ %79, %92 ]
  %97 = phi i64 [ %32, %63 ], [ %93, %92 ]
  %98 = add nsw i64 %97, 1
  store i64 %98, ptr %31, align 8, !tbaa !56
  %99 = shl nsw i64 %97, 1
  %100 = getelementptr inbounds i32, ptr %96, i64 %99
  store i32 %1, ptr %100, align 4, !tbaa !24
  %101 = or disjoint i64 %99, 1
  %102 = getelementptr inbounds i32, ptr %96, i64 %101
  store i32 %2, ptr %102, align 4, !tbaa !24
  %103 = getelementptr inbounds i8, ptr %95, i64 %97
  store i8 1, ptr %103, align 1, !tbaa !59
  br label %104

104:                                              ; preds = %8, %3, %75, %28, %94, %55, %16, %20
  %105 = phi i32 [ 0, %20 ], [ 0, %16 ], [ 1, %28 ], [ 1, %55 ], [ 1, %94 ], [ 0, %75 ], [ 0, %3 ], [ 0, %8 ]
  ret i32 %105
}

; Function Attrs: nofree nounwind memory(readwrite, argmem: read) uwtable
define internal fastcc i32 @canonical_pair_find_static(ptr noundef nonnull readonly %0, i32 noundef %1, i32 noundef %2) unnamed_addr #13 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %5 = load ptr, ptr %4, align 8, !tbaa !44
  %6 = icmp eq ptr %5, null
  br i1 %6, label %136, label %7

7:                                                ; preds = %3
  %8 = load i32, ptr @g_meta_count, align 4, !tbaa !24
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
  br i1 %18, label %136, label %12, !llvm.loop !106

19:                                               ; preds = %12
  %20 = and i64 %13, 4294967295
  %21 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %20
  %22 = load ptr, ptr %21, align 8, !tbaa !107
  %23 = icmp eq ptr %22, null
  br i1 %23, label %26, label %24

24:                                               ; preds = %19
  %25 = load ptr, ptr %22, align 8, !tbaa !109
  br label %91

26:                                               ; preds = %19
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !55
  %29 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #17
  %30 = trunc i64 %28 to i32
  %31 = shl i32 %30, 1
  %32 = add i32 %31, 32
  br label %33

33:                                               ; preds = %33, %26
  %34 = phi i32 [ 1, %26 ], [ %36, %33 ]
  %35 = icmp sgt i32 %34, %32
  %36 = shl i32 %34, 1
  br i1 %35, label %37, label %33, !llvm.loop !26

37:                                               ; preds = %33
  %38 = sext i32 %34 to i64
  %39 = getelementptr inbounds nuw i8, ptr %29, i64 8
  store i64 %38, ptr %39, align 8, !tbaa !112
  %40 = tail call noalias ptr @calloc(i64 noundef %38, i64 noundef 24) #15
  store ptr %40, ptr %29, align 8, !tbaa !109
  store ptr %29, ptr %21, align 8, !tbaa !107
  %41 = icmp sgt i64 %28, 0
  br i1 %41, label %42, label %91

42:                                               ; preds = %37
  %43 = add nsw i64 %38, -1
  br label %44

44:                                               ; preds = %88, %42
  %45 = phi i64 [ 0, %42 ], [ %89, %88 ]
  %46 = getelementptr inbounds nuw %struct.EdgePair, ptr %5, i64 %45
  %47 = load i32, ptr %46, align 4, !tbaa !5
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !10
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
  %67 = load i8, ptr %66, align 8, !tbaa !27
  %68 = icmp eq i8 %67, 0
  br i1 %68, label %83, label %69

69:                                               ; preds = %44, %76
  %70 = phi ptr [ %79, %76 ], [ %65, %44 ]
  %71 = phi i64 [ %78, %76 ], [ %64, %44 ]
  %72 = load i64, ptr %70, align 8, !tbaa !29
  %73 = icmp eq i64 %72, %55
  br i1 %73, label %74, label %76

74:                                               ; preds = %69
  %75 = getelementptr inbounds nuw i8, ptr %70, i64 8
  store i64 %45, ptr %75, align 8, !tbaa !30
  br label %88

76:                                               ; preds = %69
  %77 = add i64 %71, 1
  %78 = and i64 %77, %43
  %79 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %78
  %80 = getelementptr inbounds nuw i8, ptr %79, i64 16
  %81 = load i8, ptr %80, align 8, !tbaa !27
  %82 = icmp eq i8 %81, 0
  br i1 %82, label %83, label %69, !llvm.loop !31

83:                                               ; preds = %76, %44
  %84 = phi i64 [ %64, %44 ], [ %78, %76 ]
  %85 = phi ptr [ %65, %44 ], [ %79, %76 ]
  store i64 %55, ptr %85, align 8, !tbaa !29
  %86 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %84, i32 1
  store i64 %45, ptr %86, align 8, !tbaa !30
  %87 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %84, i32 2
  store i8 1, ptr %87, align 8, !tbaa !27
  br label %88

88:                                               ; preds = %83, %74
  %89 = add nuw nsw i64 %45, 1
  %90 = icmp eq i64 %89, %28
  br i1 %90, label %91, label %44, !llvm.loop !113

91:                                               ; preds = %88, %24, %37
  %92 = phi ptr [ %40, %37 ], [ %25, %24 ], [ %40, %88 ]
  %93 = phi ptr [ %29, %37 ], [ %22, %24 ], [ %29, %88 ]
  %94 = icmp eq ptr %92, null
  br i1 %94, label %136, label %95

95:                                               ; preds = %91
  %96 = getelementptr inbounds nuw i8, ptr %93, i64 8
  %97 = load i64, ptr %96, align 8, !tbaa !112
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
  %118 = load i8, ptr %117, align 8, !tbaa !27
  %119 = icmp eq i8 %118, 0
  br i1 %119, label %136, label %127

120:                                              ; preds = %127
  %121 = add i64 %129, 1
  %122 = and i64 %121, %114
  %123 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %92, i64 %122
  %124 = getelementptr inbounds nuw i8, ptr %123, i64 16
  %125 = load i8, ptr %124, align 8, !tbaa !27
  %126 = icmp eq i8 %125, 0
  br i1 %126, label %136, label %127, !llvm.loop !34

127:                                              ; preds = %99, %120
  %128 = phi ptr [ %123, %120 ], [ %116, %99 ]
  %129 = phi i64 [ %122, %120 ], [ %115, %99 ]
  %130 = load i64, ptr %128, align 8, !tbaa !29
  %131 = icmp eq i64 %130, %105
  br i1 %131, label %132, label %120

132:                                              ; preds = %127
  %133 = getelementptr inbounds nuw i8, ptr %128, i64 8
  %134 = load i64, ptr %133, align 8, !tbaa !30
  %135 = trunc i64 %134 to i32
  br label %136

136:                                              ; preds = %16, %120, %132, %99, %95, %91, %7, %3
  %137 = phi i32 [ -1, %3 ], [ -1, %95 ], [ -1, %91 ], [ %135, %132 ], [ -1, %99 ], [ -1, %7 ], [ -1, %120 ], [ -1, %16 ]
  ret i32 %137
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %54

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %54, label %11, !llvm.loop !36

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !37
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %18 = load ptr, ptr %17, align 8, !tbaa !43
  %19 = icmp eq ptr %18, null
  br i1 %19, label %54, label %20

20:                                               ; preds = %16
  %21 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %22 = icmp sgt i32 %21, -1
  br i1 %22, label %23, label %25

23:                                               ; preds = %20
  %24 = load ptr, ptr %17, align 8, !tbaa !43
  tail call void @roaring_bitmap_remove(ptr noundef %24, i32 noundef %21) #16
  br label %54

25:                                               ; preds = %20
  %26 = getelementptr inbounds nuw i8, ptr %13, i64 64
  %27 = load i64, ptr %26, align 8, !tbaa !56
  %28 = icmp slt i64 %27, 1
  br i1 %28, label %54, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %31 = load ptr, ptr %30, align 8, !tbaa !80
  br label %32

32:                                               ; preds = %47, %29
  %33 = phi i64 [ 0, %29 ], [ %48, %47 ]
  %34 = shl nuw nsw i64 %33, 1
  %35 = getelementptr inbounds nuw i32, ptr %31, i64 %34
  %36 = load i32, ptr %35, align 4, !tbaa !24
  %37 = or disjoint i64 %34, 1
  %38 = getelementptr inbounds nuw i32, ptr %31, i64 %37
  %39 = load i32, ptr %38, align 4, !tbaa !24
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
  br i1 %49, label %54, label %32, !llvm.loop !81

50:                                               ; preds = %43, %32
  %51 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %52 = load ptr, ptr %51, align 8, !tbaa !57
  %53 = getelementptr inbounds nuw i8, ptr %52, i64 %33
  store i8 0, ptr %53, align 1, !tbaa !59
  br label %54

54:                                               ; preds = %8, %47, %25, %3, %23, %50, %16
  %55 = phi i32 [ 0, %16 ], [ 1, %50 ], [ 1, %23 ], [ 0, %3 ], [ 1, %25 ], [ 1, %47 ], [ 0, %8 ]
  ret i32 %55
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_init(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4, ptr noundef %5) local_unnamed_addr #0 {
  %7 = load i32, ptr @g_meta_count, align 4, !tbaa !24
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %9, label %21

9:                                                ; preds = %6
  %10 = zext nneg i32 %7 to i64
  br label %14

11:                                               ; preds = %14
  %12 = add nuw nsw i64 %15, 1
  %13 = icmp eq i64 %12, %10
  br i1 %13, label %19, label %14, !llvm.loop !36

14:                                               ; preds = %11, %9
  %15 = phi i64 [ 0, %9 ], [ %12, %11 ]
  %16 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %15
  %17 = load ptr, ptr %16, align 8, !tbaa !37
  %18 = icmp eq ptr %17, %0
  br i1 %18, label %26, label %11

19:                                               ; preds = %11
  %20 = icmp sgt i32 %7, 63
  br i1 %20, label %66, label %21

21:                                               ; preds = %19, %6
  %22 = add nsw i32 %7, 1
  store i32 %22, ptr @g_meta_count, align 4, !tbaa !24
  %23 = sext i32 %7 to i64
  %24 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %23
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(168) %25, i8 0, i64 160, i1 false)
  store ptr %0, ptr %24, align 8, !tbaa !37
  store i32 3, ptr %25, align 8, !tbaa !45
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
  br i1 %38, label %46, label %32, !llvm.loop !106

39:                                               ; preds = %32
  %40 = and i64 %33, 4294967295
  %41 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %40
  %42 = load ptr, ptr %41, align 8, !tbaa !107
  %43 = icmp eq ptr %42, null
  br i1 %43, label %46, label %44

44:                                               ; preds = %39
  %45 = load ptr, ptr %42, align 8, !tbaa !109
  tail call void @free(ptr noundef %45) #16
  tail call void @free(ptr noundef nonnull %42) #16
  store ptr null, ptr %41, align 8, !tbaa !107
  br label %46

46:                                               ; preds = %36, %26, %44, %39
  %47 = getelementptr inbounds nuw i8, ptr %28, i64 16
  store ptr %3, ptr %47, align 8, !tbaa !42
  %48 = getelementptr inbounds nuw i8, ptr %28, i64 24
  store ptr %4, ptr %48, align 8, !tbaa !43
  %49 = getelementptr inbounds nuw i8, ptr %28, i64 32
  store ptr %5, ptr %49, align 8, !tbaa !44
  %50 = icmp sgt i64 %2, 1
  %51 = zext i1 %50 to i64
  %52 = lshr i64 %2, %51
  %53 = getelementptr inbounds nuw i8, ptr %28, i64 40
  store i64 %52, ptr %53, align 8, !tbaa !55
  %54 = getelementptr inbounds nuw i8, ptr %28, i64 64
  %55 = getelementptr inbounds nuw i8, ptr %28, i64 48
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %54, i8 0, i64 16, i1 false)
  %56 = load ptr, ptr %55, align 8, !tbaa !80
  tail call void @free(ptr noundef %56) #16
  %57 = getelementptr inbounds nuw i8, ptr %28, i64 56
  %58 = load ptr, ptr %57, align 8, !tbaa !57
  tail call void @free(ptr noundef %58) #16
  %59 = getelementptr inbounds nuw i8, ptr %28, i64 96
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %55, i8 0, i64 16, i1 false)
  store i64 %1, ptr %59, align 8, !tbaa !46
  %60 = getelementptr inbounds nuw i8, ptr %28, i64 104
  store i64 %2, ptr %60, align 8, !tbaa !47
  %61 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %62 = getelementptr inbounds nuw i8, ptr %28, i64 80
  %63 = load <2 x ptr>, ptr %61, align 8, !tbaa !97
  store <2 x ptr> %63, ptr %62, align 8, !tbaa !97
  %64 = getelementptr inbounds nuw i8, ptr %28, i64 112
  store i32 0, ptr %64, align 8, !tbaa !48
  %65 = getelementptr inbounds nuw i8, ptr %28, i64 8
  store i32 0, ptr %65, align 8, !tbaa !45
  br label %66

66:                                               ; preds = %19, %46
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #14

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #14

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #14

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #14

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>) #14

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x i64> @llvm.smax.v2i64(<2 x i64>, <2 x i64>) #14

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
attributes #14 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #15 = { nounwind allocsize(0,1) }
attributes #16 = { nounwind }
attributes #17 = { nounwind allocsize(0) }
attributes #18 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.1.8 (https://github.com/llvm/llvm-project.git 87f0227cb60147a26a1eeb4fb06e3b505e9c7261)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"", !7, i64 0, !7, i64 4}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!6, !7, i64 4}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
!13 = !{!14, !14, i64 0}
!14 = !{!"long", !8, i64 0}
!15 = distinct !{!15, !12}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.unroll.disable"}
!18 = distinct !{!18, !12}
!19 = !{!20, !20, i64 0}
!20 = !{!"p1 long", !21, i64 0}
!21 = !{!"any pointer", !8, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"p1 int", !21, i64 0}
!24 = !{!7, !7, i64 0}
!25 = distinct !{!25, !12}
!26 = distinct !{!26, !12}
!27 = !{!28, !8, i64 16}
!28 = !{!"EdgeHashEntry_s", !14, i64 0, !14, i64 8, !8, i64 16}
!29 = !{!28, !14, i64 0}
!30 = !{!28, !14, i64 8}
!31 = distinct !{!31, !12}
!32 = distinct !{!32, !12}
!33 = distinct !{!33, !12}
!34 = distinct !{!34, !12}
!35 = distinct !{!35, !12}
!36 = distinct !{!36, !12}
!37 = !{!38, !21, i64 0}
!38 = !{!"", !21, i64 0, !7, i64 8, !21, i64 16, !21, i64 24, !21, i64 32, !14, i64 40, !23, i64 48, !39, i64 56, !14, i64 64, !14, i64 72, !20, i64 80, !23, i64 88, !14, i64 96, !14, i64 104, !7, i64 112, !20, i64 120, !23, i64 128, !14, i64 136, !23, i64 144, !23, i64 152, !7, i64 160, !7, i64 164}
!39 = !{!"p1 omnipotent char", !21, i64 0}
!40 = !{!38, !20, i64 80}
!41 = !{!38, !23, i64 88}
!42 = !{!38, !21, i64 16}
!43 = !{!38, !21, i64 24}
!44 = !{!38, !21, i64 32}
!45 = !{!38, !7, i64 8}
!46 = !{!38, !14, i64 96}
!47 = !{!38, !14, i64 104}
!48 = !{!38, !7, i64 112}
!49 = !{!38, !20, i64 120}
!50 = !{!38, !23, i64 128}
!51 = !{!38, !7, i64 164}
!52 = !{!38, !7, i64 160}
!53 = !{!38, !23, i64 144}
!54 = !{!38, !23, i64 152}
!55 = !{!38, !14, i64 40}
!56 = !{!38, !14, i64 64}
!57 = !{!38, !39, i64 56}
!58 = distinct !{!58, !12}
!59 = !{!8, !8, i64 0}
!60 = distinct !{!60, !12}
!61 = distinct !{!61, !12, !62, !63}
!62 = !{!"llvm.loop.isvectorized", i32 1}
!63 = !{!"llvm.loop.unroll.runtime.disable"}
!64 = distinct !{!64, !12}
!65 = distinct !{!65, !12, !63, !62}
!66 = distinct !{!66, !17}
!67 = distinct !{!67, !12}
!68 = distinct !{!68, !12}
!69 = distinct !{!69, !12}
!70 = distinct !{!70, !12}
!71 = distinct !{!71, !12}
!72 = distinct !{!72, !17}
!73 = distinct !{!73, !12}
!74 = distinct !{!74, !12}
!75 = distinct !{!75, !12}
!76 = !{!38, !14, i64 72}
!77 = distinct !{!77, !12}
!78 = distinct !{!78, !12}
!79 = distinct !{!79, !12}
!80 = !{!38, !23, i64 48}
!81 = distinct !{!81, !12}
!82 = distinct !{!82, !12}
!83 = distinct !{!83, !12}
!84 = distinct !{!84, !12}
!85 = distinct !{!85, !12}
!86 = distinct !{!86, !12, !62, !63}
!87 = distinct !{!87, !12, !62}
!88 = distinct !{!88, !12, !62, !63}
!89 = distinct !{!89, !12, !63, !62}
!90 = distinct !{!90, !12}
!91 = distinct !{!91, !12}
!92 = distinct !{!92, !12}
!93 = distinct !{!93, !12, !62, !63}
!94 = distinct !{!94, !12, !63, !62}
!95 = distinct !{!95, !12}
!96 = distinct !{!96, !12}
!97 = !{!21, !21, i64 0}
!98 = distinct !{!98, !12}
!99 = distinct !{!99, !12}
!100 = distinct !{!100, !17}
!101 = distinct !{!101, !12}
!102 = distinct !{!102, !12}
!103 = distinct !{!103, !12}
!104 = distinct !{!104, !12}
!105 = distinct !{!105, !12}
!106 = distinct !{!106, !12}
!107 = !{!108, !108, i64 0}
!108 = !{!"p1 _ZTS13EdgeHashMap_s", !21, i64 0}
!109 = !{!110, !111, i64 0}
!110 = !{!"EdgeHashMap_s", !111, i64 0, !14, i64 8}
!111 = !{!"p1 _ZTS15EdgeHashEntry_s", !21, i64 0}
!112 = !{!110, !14, i64 8}
!113 = distinct !{!113, !12}
