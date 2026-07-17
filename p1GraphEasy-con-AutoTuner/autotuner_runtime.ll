; ModuleID = 'autotuner_runtime.c'
source_filename = "autotuner_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.AutoGraphMeta = type { ptr, i32, ptr, ptr, ptr, i64, ptr, ptr, i64, i64, i32, i64, ptr, ptr, i64, i64, i32, ptr, ptr, i64, ptr, ptr, i32, i32 }
%struct.EdgePair = type { i32, i32 }
%struct.EdgeHashEntry_s = type { i64, i64, i8 }
%struct.timespec = type { i64, i64 }
%struct.AutoNeighborScratchSlot = type { ptr, i64 }
%struct.AutoFrontierStepEnv = type { ptr, ptr, ptr, i32, i32, ptr, i32, i32, ptr, ptr }

@g_conversion_ns = internal unnamed_addr global i64 0, align 8
@g_conversions_injected = internal unnamed_addr global i32 0, align 4
@g_meta_count = internal unnamed_addr global i32 0, align 4
@g_meta = internal global [64 x %struct.AutoGraphMeta] zeroinitializer, align 16
@g_neighbor_scratch_key_once = internal global i32 0, align 4
@g_neighbor_scratch_key = internal global i32 0, align 4
@g_static_edge_hash = internal unnamed_addr global [64 x ptr] zeroinitializer, align 16
@g_extra_edge_hash = internal unnamed_addr global [64 x ptr] zeroinitializer, align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_profile_region_enter(i32 noundef %0, i32 noundef %1, i32 noundef %2, double noundef %3) local_unnamed_addr #0 {
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_profile_region_exit(i32 noundef %0) local_unnamed_addr #0 {
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_profile_record_kernel_ns(i32 noundef %0, i64 noundef %1) local_unnamed_addr #0 {
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @build_csr_from_set(i64 noundef %0, i64 noundef %1, ptr noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #1 {
  %8 = icmp sgt i64 %1, 0
  br i1 %8, label %15, label %9

9:                                                ; preds = %7
  %10 = tail call noalias dereferenceable_or_null(8) ptr @calloc(i64 noundef 1, i64 noundef 8) #18
  br label %104

11:                                               ; preds = %29
  %12 = add nsw i64 %30, 2
  %13 = tail call noalias ptr @calloc(i64 noundef %12, i64 noundef 8) #18
  %14 = getelementptr inbounds nuw i8, ptr %13, i64 8
  br label %45

15:                                               ; preds = %7, %29
  %16 = phi i64 [ %30, %29 ], [ -1, %7 ]
  %17 = phi i64 [ %31, %29 ], [ 0, %7 ]
  %18 = trunc i64 %17 to i32
  %19 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %18) #19
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
  %49 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %48) #19
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
  %103 = tail call noalias ptr @malloc(i64 noundef %102) #20
  br label %104

104:                                              ; preds = %9, %80, %101
  %105 = phi ptr [ %13, %101 ], [ %13, %80 ], [ %10, %9 ]
  %106 = phi i64 [ %12, %101 ], [ %12, %80 ], [ 1, %9 ]
  %107 = phi i64 [ %30, %101 ], [ %30, %80 ], [ -1, %9 ]
  %108 = phi i64 [ %63, %101 ], [ %63, %80 ], [ 0, %9 ]
  %109 = phi ptr [ %103, %101 ], [ null, %80 ], [ null, %9 ]
  %110 = shl nuw nsw i64 %106, 3
  %111 = tail call noalias ptr @malloc(i64 noundef %110) #20
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %111, ptr align 8 %105, i64 %110, i1 false)
  br i1 %8, label %113, label %112

112:                                              ; preds = %131, %104
  tail call void @free(ptr noundef %111) #19
  store ptr %105, ptr %4, align 8, !tbaa !17
  store ptr %109, ptr %5, align 8, !tbaa !20
  store i64 %108, ptr %6, align 8, !tbaa !11
  ret void

113:                                              ; preds = %104, %131
  %114 = phi i64 [ %132, %131 ], [ 0, %104 ]
  %115 = trunc i64 %114 to i32
  %116 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %115) #19
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
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

declare dso_local zeroext i1 @roaring_bitmap_contains(ptr noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #6

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local void @free(ptr allocptr nocapture noundef) local_unnamed_addr #7

; Function Attrs: nounwind uwtable
define dso_local void @rebuild_sets_from_csr(i64 noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef readonly %4, ptr noundef %5, ptr noundef %6, ptr nocapture noundef readonly %7) local_unnamed_addr #1 {
  tail call void @roaring_bitmap_clear(ptr noundef %5) #19
  tail call void @roaring_bitmap_clear(ptr noundef %6) #19
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
  %18 = tail call noalias ptr @calloc(i64 noundef %17, i64 noundef 24) #18
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
  tail call void @free(ptr noundef %18) #19
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
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %88) #19
  %89 = trunc i64 %80 to i32
  br label %90

90:                                               ; preds = %87, %132
  %91 = phi i64 [ %82, %87 ], [ %133, %132 ]
  %92 = getelementptr inbounds i32, ptr %4, i64 %91
  %93 = load i32, ptr %92, align 4, !tbaa !22
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %93) #19
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
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %131) #19
  br label %132

132:                                              ; preds = %114, %94, %90, %130, %126
  %133 = add i64 %91, 1
  %134 = icmp eq i64 %133, %85
  br i1 %134, label %76, label %90, !llvm.loop !33
}

declare dso_local void @roaring_bitmap_clear(ptr noundef) local_unnamed_addr #3

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define internal fastcc void @edge_hash_insert(ptr nocapture noundef readonly %0, i32 noundef %1, i32 noundef %2, i64 noundef %3) unnamed_addr #8 {
  %5 = tail call i32 @llvm.smin.i32(i32 %1, i32 %2)
  %6 = tail call i32 @llvm.smax.i32(i32 %1, i32 %2)
  %7 = zext i32 %5 to i64
  %8 = shl nuw i64 %7, 32
  %9 = zext i32 %6 to i64
  %10 = or disjoint i64 %8, %9
  %11 = lshr i64 %10, 30
  %12 = xor i64 %11, %10
  %13 = mul i64 %12, -4658895280553007687
  %14 = lshr i64 %13, 27
  %15 = xor i64 %14, %13
  %16 = mul i64 %15, -7723592293110705685
  %17 = lshr i64 %16, 31
  %18 = xor i64 %17, %16
  %19 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %20 = load i64, ptr %19, align 8, !tbaa !34
  %21 = add nsw i64 %20, -1
  %22 = load ptr, ptr %0, align 8, !tbaa !37
  %23 = and i64 %18, %21
  %24 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %23
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 16
  %26 = load i8, ptr %25, align 8, !tbaa !25
  %27 = icmp eq i8 %26, 0
  br i1 %27, label %42, label %28

28:                                               ; preds = %4, %35
  %29 = phi ptr [ %38, %35 ], [ %24, %4 ]
  %30 = phi i64 [ %37, %35 ], [ %23, %4 ]
  %31 = load i64, ptr %29, align 8, !tbaa !27
  %32 = icmp eq i64 %31, %10
  br i1 %32, label %33, label %35

33:                                               ; preds = %28
  %34 = getelementptr inbounds nuw i8, ptr %29, i64 8
  store i64 %3, ptr %34, align 8, !tbaa !28
  br label %47

35:                                               ; preds = %28
  %36 = add i64 %30, 1
  %37 = and i64 %36, %21
  %38 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %37
  %39 = getelementptr inbounds nuw i8, ptr %38, i64 16
  %40 = load i8, ptr %39, align 8, !tbaa !25
  %41 = icmp eq i8 %40, 0
  br i1 %41, label %42, label %28, !llvm.loop !29

42:                                               ; preds = %35, %4
  %43 = phi i64 [ %23, %4 ], [ %37, %35 ]
  %44 = phi ptr [ %24, %4 ], [ %38, %35 ]
  store i64 %10, ptr %44, align 8, !tbaa !27
  %45 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %43, i32 1
  store i64 %3, ptr %45, align 8, !tbaa !28
  %46 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %43, i32 2
  store i8 1, ptr %46, align 8, !tbaa !25
  br label %47

47:                                               ; preds = %42, %33
  ret void
}

declare dso_local void @roaring_bitmap_add(ptr noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_update_csr_pointers(ptr noundef readnone %0, ptr noundef %1, ptr noundef %2) local_unnamed_addr #9 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %19

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %19, label %11, !llvm.loop !38

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !39
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 96
  store ptr %1, ptr %17, align 8, !tbaa !42
  %18 = getelementptr inbounds nuw i8, ptr %13, i64 104
  store ptr %2, ptr %18, align 8, !tbaa !43
  br label %19

19:                                               ; preds = %8, %3, %16
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_record_adjacency_state(ptr noundef readnone %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4) local_unnamed_addr #9 {
  %6 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %7 = icmp sgt i32 %6, 0
  br i1 %7, label %8, label %23

8:                                                ; preds = %5
  %9 = zext nneg i32 %6 to i64
  br label %13

10:                                               ; preds = %13
  %11 = add nuw nsw i64 %14, 1
  %12 = icmp eq i64 %11, %9
  br i1 %12, label %23, label %13, !llvm.loop !38

13:                                               ; preds = %10, %8
  %14 = phi i64 [ 0, %8 ], [ %11, %10 ]
  %15 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %14
  %16 = load ptr, ptr %15, align 8, !tbaa !39
  %17 = icmp eq ptr %16, %0
  br i1 %17, label %18, label %10

18:                                               ; preds = %13
  %19 = getelementptr inbounds nuw i8, ptr %15, i64 112
  store i64 %1, ptr %19, align 8, !tbaa !44
  %20 = getelementptr inbounds nuw i8, ptr %15, i64 120
  store i64 %2, ptr %20, align 8, !tbaa !45
  %21 = getelementptr inbounds nuw i8, ptr %15, i64 96
  store ptr %3, ptr %21, align 8, !tbaa !42
  %22 = getelementptr inbounds nuw i8, ptr %15, i64 104
  store ptr %4, ptr %22, align 8, !tbaa !43
  br label %23

23:                                               ; preds = %10, %5, %18
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_mark_canonical_dirty(ptr noundef readnone %0) local_unnamed_addr #9 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %16

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %16, label %9, !llvm.loop !38

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 8, !tbaa !39
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 80
  store i32 1, ptr %15, align 8, !tbaa !46
  br label %16

16:                                               ; preds = %6, %1, %14
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_sync_canonical_if_dirty(ptr noundef %0) local_unnamed_addr #1 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %19

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %19, label %9, !llvm.loop !38

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 8, !tbaa !39
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 80
  %16 = load i32, ptr %15, align 8, !tbaa !46
  %17 = icmp eq i32 %16, 0
  br i1 %17, label %19, label %18

18:                                               ; preds = %14
  tail call void @autograph_ensure_layout_set(ptr noundef %0)
  br label %19

19:                                               ; preds = %6, %1, %14, %18
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout_set(ptr noundef writeonly %0) local_unnamed_addr #1 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %357

7:                                                ; preds = %1
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %357, label %12, !llvm.loop !38

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 8, !tbaa !39
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 16
  %19 = load ptr, ptr %18, align 8, !tbaa !47
  %20 = icmp eq ptr %19, null
  br i1 %20, label %357, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %14, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !48
  %24 = icmp eq ptr %23, null
  br i1 %24, label %357, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %14, i64 32
  %27 = load ptr, ptr %26, align 8, !tbaa !49
  %28 = icmp eq ptr %27, null
  br i1 %28, label %357, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %14, i64 8
  %31 = load i32, ptr %30, align 8, !tbaa !50
  %32 = icmp eq i32 %31, 3
  br i1 %32, label %33, label %38

33:                                               ; preds = %29
  %34 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %35 = load i32, ptr %34, align 8, !tbaa !46
  %36 = icmp eq i32 %35, 0
  br i1 %36, label %357, label %37

37:                                               ; preds = %33
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #19
  store ptr null, ptr %2, align 8, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #19
  store ptr null, ptr %3, align 8, !tbaa !20
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %4) #19
  store i64 0, ptr %4, align 8, !tbaa !11
  br label %75

38:                                               ; preds = %29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #19
  store ptr null, ptr %2, align 8, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #19
  store ptr null, ptr %3, align 8, !tbaa !20
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %4) #19
  store i64 0, ptr %4, align 8, !tbaa !11
  switch i32 %31, label %75 [
    i32 0, label %39
    i32 1, label %51
    i32 2, label %61
  ]

39:                                               ; preds = %38
  %40 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %41 = load ptr, ptr %40, align 8, !tbaa !42
  store ptr %41, ptr %2, align 8, !tbaa !17
  %42 = getelementptr inbounds nuw i8, ptr %14, i64 104
  %43 = load ptr, ptr %42, align 8, !tbaa !43
  store ptr %43, ptr %3, align 8, !tbaa !20
  %44 = getelementptr inbounds nuw i8, ptr %14, i64 112
  %45 = load i64, ptr %44, align 8, !tbaa !44
  %46 = getelementptr inbounds nuw i8, ptr %14, i64 120
  %47 = load i64, ptr %46, align 8, !tbaa !45
  store i64 %47, ptr %4, align 8, !tbaa !11
  %48 = getelementptr inbounds nuw i8, ptr %14, i64 128
  %49 = load i32, ptr %48, align 8, !tbaa !51
  %50 = icmp ne i32 %49, 0
  br label %75

51:                                               ; preds = %38
  %52 = getelementptr inbounds nuw i8, ptr %14, i64 112
  %53 = load i64, ptr %52, align 8, !tbaa !44
  %54 = getelementptr inbounds nuw i8, ptr %14, i64 136
  %55 = load ptr, ptr %54, align 8, !tbaa !52
  %56 = getelementptr inbounds nuw i8, ptr %14, i64 144
  %57 = load ptr, ptr %56, align 8, !tbaa !53
  call void @convert_pcsr_to_csr(i64 noundef %53, i64 poison, ptr noundef %55, ptr noundef %57, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %58 = load i64, ptr %52, align 8, !tbaa !44
  %59 = load ptr, ptr %54, align 8, !tbaa !52
  tail call void @free(ptr noundef %59) #19
  store ptr null, ptr %54, align 8, !tbaa !52
  %60 = load ptr, ptr %56, align 8, !tbaa !53
  tail call void @free(ptr noundef %60) #19
  store ptr null, ptr %56, align 8, !tbaa !53
  br label %75

61:                                               ; preds = %38
  %62 = getelementptr inbounds nuw i8, ptr %14, i64 112
  %63 = load i64, ptr %62, align 8, !tbaa !44
  %64 = getelementptr inbounds nuw i8, ptr %14, i64 180
  %65 = load i32, ptr %64, align 4, !tbaa !54
  %66 = getelementptr inbounds nuw i8, ptr %14, i64 176
  %67 = load i32, ptr %66, align 8, !tbaa !55
  %68 = getelementptr inbounds nuw i8, ptr %14, i64 160
  %69 = load ptr, ptr %68, align 8, !tbaa !56
  %70 = getelementptr inbounds nuw i8, ptr %14, i64 168
  %71 = load ptr, ptr %70, align 8, !tbaa !57
  call void @convert_bcsr_to_csr(i64 noundef %63, i32 noundef %65, i32 noundef %67, ptr noundef %69, ptr noundef %71, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %72 = load i64, ptr %62, align 8, !tbaa !44
  %73 = load ptr, ptr %68, align 8, !tbaa !56
  call void @free(ptr noundef %73) #19
  store ptr null, ptr %68, align 8, !tbaa !56
  %74 = load ptr, ptr %70, align 8, !tbaa !57
  call void @free(ptr noundef %74) #19
  store ptr null, ptr %70, align 8, !tbaa !57
  br label %75

75:                                               ; preds = %37, %38, %51, %61, %39
  %76 = phi i64 [ %45, %39 ], [ %58, %51 ], [ %72, %61 ], [ 0, %38 ], [ 0, %37 ]
  %77 = phi i1 [ %50, %39 ], [ true, %51 ], [ true, %61 ], [ false, %38 ], [ false, %37 ]
  %78 = load ptr, ptr %2, align 8, !tbaa !17
  %79 = icmp ne ptr %78, null
  %80 = load ptr, ptr %3, align 8
  %81 = icmp ne ptr %80, null
  %82 = select i1 %79, i1 %81, i1 false
  br i1 %82, label %83, label %342

83:                                               ; preds = %75
  %84 = getelementptr inbounds nuw i8, ptr %14, i64 88
  store i64 -1, ptr %84, align 8, !tbaa !58
  %85 = load ptr, ptr %18, align 8, !tbaa !47
  %86 = load ptr, ptr %22, align 8, !tbaa !48
  %87 = load ptr, ptr %26, align 8, !tbaa !49
  call void @roaring_bitmap_clear(ptr noundef %85) #19
  call void @roaring_bitmap_clear(ptr noundef %86) #19
  %88 = getelementptr inbounds nuw i8, ptr %14, i64 72
  %89 = load i64, ptr %88, align 8, !tbaa !59
  %90 = icmp sgt i64 %89, 0
  br i1 %90, label %91, label %93

91:                                               ; preds = %83
  %92 = getelementptr inbounds nuw i8, ptr %14, i64 56
  br label %123

93:                                               ; preds = %123, %83
  %94 = getelementptr inbounds nuw i8, ptr %14, i64 64
  store i64 0, ptr %94, align 8, !tbaa !60
  %95 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %96 = icmp slt i32 %95, 1
  %97 = zext nneg i32 %95 to i64
  %98 = add nsw i64 %97, -1
  %99 = icmp ugt i64 %13, %98
  %100 = select i1 %96, i1 true, i1 %99
  br i1 %100, label %107, label %101

101:                                              ; preds = %93
  %102 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %13
  %103 = load ptr, ptr %102, align 8, !tbaa !61
  %104 = icmp eq ptr %103, null
  br i1 %104, label %107, label %105

105:                                              ; preds = %101
  %106 = load ptr, ptr %103, align 8, !tbaa !37
  call void @free(ptr noundef %106) #19
  call void @free(ptr noundef nonnull %103) #19
  store ptr null, ptr %102, align 8, !tbaa !61
  br label %107

107:                                              ; preds = %105, %101, %93
  %108 = getelementptr inbounds nuw i8, ptr %14, i64 40
  %109 = load i64, ptr %108, align 8, !tbaa !63
  %110 = trunc i64 %109 to i32
  %111 = shl i32 %110, 1
  %112 = add i32 %111, 32
  br label %113

113:                                              ; preds = %113, %107
  %114 = phi i32 [ 1, %107 ], [ %116, %113 ]
  %115 = icmp sgt i32 %114, %112
  %116 = shl i32 %114, 1
  br i1 %115, label %117, label %113, !llvm.loop !24

117:                                              ; preds = %113
  %118 = sext i32 %114 to i64
  %119 = call noalias ptr @calloc(i64 noundef %118, i64 noundef 24) #18
  %120 = icmp sgt i64 %109, 0
  br i1 %120, label %121, label %130

121:                                              ; preds = %117
  %122 = add nsw i64 %118, -1
  br label %140

123:                                              ; preds = %123, %91
  %124 = phi i64 [ 0, %91 ], [ %127, %123 ]
  %125 = load ptr, ptr %92, align 8, !tbaa !64
  %126 = getelementptr inbounds nuw i8, ptr %125, i64 %124
  store i8 0, ptr %126, align 1, !tbaa !65
  %127 = add nuw nsw i64 %124, 1
  %128 = load i64, ptr %88, align 8, !tbaa !59
  %129 = icmp slt i64 %127, %128
  br i1 %129, label %123, label %93, !llvm.loop !66

130:                                              ; preds = %184, %117
  %131 = icmp sgt i64 %76, 0
  br i1 %131, label %132, label %339

132:                                              ; preds = %130
  %133 = icmp eq ptr %119, null
  %134 = icmp slt i32 %114, 1
  %135 = or i1 %134, %133
  %136 = add nsw i64 %118, -1
  %137 = getelementptr inbounds nuw i8, ptr %14, i64 48
  %138 = getelementptr inbounds nuw i8, ptr %14, i64 56
  %139 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %13
  br label %189

140:                                              ; preds = %184, %121
  %141 = phi i64 [ 0, %121 ], [ %185, %184 ]
  %142 = getelementptr inbounds nuw %struct.EdgePair, ptr %87, i64 %141
  %143 = load i32, ptr %142, align 4, !tbaa !3
  %144 = getelementptr inbounds nuw i8, ptr %142, i64 4
  %145 = load i32, ptr %144, align 4, !tbaa !8
  %146 = call i32 @llvm.smin.i32(i32 %143, i32 %145)
  %147 = call i32 @llvm.smax.i32(i32 %143, i32 %145)
  %148 = zext i32 %146 to i64
  %149 = shl nuw i64 %148, 32
  %150 = zext i32 %147 to i64
  %151 = or disjoint i64 %149, %150
  %152 = lshr i64 %151, 30
  %153 = xor i64 %152, %151
  %154 = mul i64 %153, -4658895280553007687
  %155 = lshr i64 %154, 27
  %156 = xor i64 %155, %154
  %157 = mul i64 %156, -7723592293110705685
  %158 = lshr i64 %157, 31
  %159 = xor i64 %158, %157
  %160 = and i64 %159, %122
  %161 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %160
  %162 = getelementptr inbounds nuw i8, ptr %161, i64 16
  %163 = load i8, ptr %162, align 8, !tbaa !25
  %164 = icmp eq i8 %163, 0
  br i1 %164, label %179, label %165

165:                                              ; preds = %140, %172
  %166 = phi ptr [ %175, %172 ], [ %161, %140 ]
  %167 = phi i64 [ %174, %172 ], [ %160, %140 ]
  %168 = load i64, ptr %166, align 8, !tbaa !27
  %169 = icmp eq i64 %168, %151
  br i1 %169, label %170, label %172

170:                                              ; preds = %165
  %171 = getelementptr inbounds nuw i8, ptr %166, i64 8
  store i64 %141, ptr %171, align 8, !tbaa !28
  br label %184

172:                                              ; preds = %165
  %173 = add i64 %167, 1
  %174 = and i64 %173, %122
  %175 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %174
  %176 = getelementptr inbounds nuw i8, ptr %175, i64 16
  %177 = load i8, ptr %176, align 8, !tbaa !25
  %178 = icmp eq i8 %177, 0
  br i1 %178, label %179, label %165, !llvm.loop !29

179:                                              ; preds = %172, %140
  %180 = phi i64 [ %160, %140 ], [ %174, %172 ]
  %181 = phi ptr [ %161, %140 ], [ %175, %172 ]
  store i64 %151, ptr %181, align 8, !tbaa !27
  %182 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %180, i32 1
  store i64 %141, ptr %182, align 8, !tbaa !28
  %183 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %180, i32 2
  store i8 1, ptr %183, align 8, !tbaa !25
  br label %184

184:                                              ; preds = %179, %170
  %185 = add nuw nsw i64 %141, 1
  %186 = icmp eq i64 %185, %109
  br i1 %186, label %130, label %140, !llvm.loop !67

187:                                              ; preds = %336, %189
  %188 = icmp eq i64 %193, %76
  br i1 %188, label %339, label %189, !llvm.loop !68

189:                                              ; preds = %187, %132
  %190 = phi i64 [ 0, %132 ], [ %193, %187 ]
  %191 = getelementptr inbounds nuw i64, ptr %78, i64 %190
  %192 = load i64, ptr %191, align 8, !tbaa !11
  %193 = add nuw nsw i64 %190, 1
  %194 = getelementptr inbounds nuw i64, ptr %78, i64 %193
  %195 = load i64, ptr %194, align 8, !tbaa !11
  %196 = icmp sgt i64 %195, %192
  br i1 %196, label %197, label %187

197:                                              ; preds = %189
  %198 = trunc i64 %190 to i32
  call void @roaring_bitmap_add(ptr noundef %85, i32 noundef %198) #19
  br label %199

199:                                              ; preds = %336, %197
  %200 = phi i64 [ %192, %197 ], [ %337, %336 ]
  %201 = getelementptr inbounds i32, ptr %80, i64 %200
  %202 = load i32, ptr %201, align 4, !tbaa !22
  call void @roaring_bitmap_add(ptr noundef %85, i32 noundef %202) #19
  %203 = sext i32 %202 to i64
  %204 = icmp sgt i64 %190, %203
  br i1 %204, label %336, label %205

205:                                              ; preds = %199
  br i1 %135, label %244, label %206

206:                                              ; preds = %205
  %207 = call i32 @llvm.smin.i32(i32 %198, i32 %202)
  %208 = call i32 @llvm.smax.i32(i32 %198, i32 %202)
  %209 = zext i32 %207 to i64
  %210 = shl nuw i64 %209, 32
  %211 = zext i32 %208 to i64
  %212 = or disjoint i64 %210, %211
  %213 = lshr i64 %212, 30
  %214 = xor i64 %213, %212
  %215 = mul i64 %214, -4658895280553007687
  %216 = lshr i64 %215, 27
  %217 = xor i64 %216, %215
  %218 = mul i64 %217, -7723592293110705685
  %219 = lshr i64 %218, 31
  %220 = xor i64 %219, %218
  %221 = and i64 %220, %136
  %222 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %221
  %223 = getelementptr inbounds nuw i8, ptr %222, i64 16
  %224 = load i8, ptr %223, align 8, !tbaa !25
  %225 = icmp eq i8 %224, 0
  br i1 %225, label %244, label %233

226:                                              ; preds = %233
  %227 = add i64 %235, 1
  %228 = and i64 %227, %136
  %229 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %228
  %230 = getelementptr inbounds nuw i8, ptr %229, i64 16
  %231 = load i8, ptr %230, align 8, !tbaa !25
  %232 = icmp eq i8 %231, 0
  br i1 %232, label %244, label %233, !llvm.loop !32

233:                                              ; preds = %206, %226
  %234 = phi ptr [ %229, %226 ], [ %222, %206 ]
  %235 = phi i64 [ %228, %226 ], [ %221, %206 ]
  %236 = load i64, ptr %234, align 8, !tbaa !27
  %237 = icmp eq i64 %236, %212
  br i1 %237, label %238, label %226

238:                                              ; preds = %233
  %239 = getelementptr inbounds nuw i8, ptr %234, i64 8
  %240 = load i64, ptr %239, align 8, !tbaa !28
  %241 = icmp sgt i64 %240, -1
  br i1 %241, label %242, label %244

242:                                              ; preds = %238
  %243 = trunc i64 %240 to i32
  call void @roaring_bitmap_add(ptr noundef %86, i32 noundef %243) #19
  br label %336

244:                                              ; preds = %226, %238, %206, %205
  %245 = call fastcc i64 @canonical_pair_find_extra(ptr noundef nonnull %14, i32 noundef %198, i32 noundef %202)
  %246 = icmp sgt i64 %245, -1
  br i1 %246, label %247, label %250

247:                                              ; preds = %244
  %248 = load ptr, ptr %138, align 8, !tbaa !64
  %249 = getelementptr inbounds nuw i8, ptr %248, i64 %245
  store i8 1, ptr %249, align 1, !tbaa !65
  br label %336

250:                                              ; preds = %244
  %251 = load i64, ptr %94, align 8, !tbaa !60
  %252 = add nsw i64 %251, 1
  %253 = call fastcc i32 @ensure_extra_capacity(ptr noundef nonnull %14, i64 noundef %252)
  %254 = icmp eq i32 %253, 0
  br i1 %254, label %336, label %255

255:                                              ; preds = %250
  %256 = load i64, ptr %94, align 8, !tbaa !60
  %257 = add nsw i64 %256, 1
  store i64 %257, ptr %94, align 8, !tbaa !60
  %258 = load ptr, ptr %137, align 8, !tbaa !69
  %259 = shl nsw i64 %256, 1
  %260 = getelementptr inbounds i32, ptr %258, i64 %259
  store i32 %198, ptr %260, align 4, !tbaa !22
  %261 = or disjoint i64 %259, 1
  %262 = getelementptr inbounds i32, ptr %258, i64 %261
  store i32 %202, ptr %262, align 4, !tbaa !22
  %263 = load ptr, ptr %138, align 8, !tbaa !64
  %264 = getelementptr inbounds i8, ptr %263, i64 %256
  store i8 1, ptr %264, align 1, !tbaa !65
  %265 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %266 = icmp slt i32 %265, 1
  %267 = zext nneg i32 %265 to i64
  %268 = add nsw i64 %267, -1
  %269 = icmp ugt i64 %13, %268
  %270 = select i1 %266, i1 true, i1 %269
  br i1 %270, label %336, label %271

271:                                              ; preds = %255
  %272 = load ptr, ptr %139, align 8, !tbaa !61
  %273 = icmp eq ptr %272, null
  br i1 %273, label %278, label %274

274:                                              ; preds = %271
  %275 = getelementptr inbounds nuw i8, ptr %272, i64 8
  %276 = load i64, ptr %275, align 8, !tbaa !34
  %277 = load ptr, ptr %272, align 8, !tbaa !37
  br label %294

278:                                              ; preds = %271
  %279 = load i64, ptr %88, align 8, !tbaa !59
  %280 = icmp sgt i64 %279, 0
  %281 = call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #20
  %282 = trunc i64 %279 to i32
  %283 = shl i32 %282, 2
  %284 = add i32 %283, 32
  %285 = select i1 %280, i32 %284, i32 128
  br label %286

286:                                              ; preds = %286, %278
  %287 = phi i32 [ 1, %278 ], [ %289, %286 ]
  %288 = icmp sgt i32 %287, %285
  %289 = shl i32 %287, 1
  br i1 %288, label %290, label %286, !llvm.loop !24

290:                                              ; preds = %286
  %291 = sext i32 %287 to i64
  %292 = getelementptr inbounds nuw i8, ptr %281, i64 8
  store i64 %291, ptr %292, align 8, !tbaa !34
  %293 = call noalias ptr @calloc(i64 noundef %291, i64 noundef 24) #18
  store ptr %293, ptr %281, align 8, !tbaa !37
  store ptr %281, ptr %139, align 8, !tbaa !61
  br label %294

294:                                              ; preds = %290, %274
  %295 = phi ptr [ %293, %290 ], [ %277, %274 ]
  %296 = phi i64 [ %291, %290 ], [ %276, %274 ]
  %297 = call i32 @llvm.smin.i32(i32 %198, i32 %202)
  %298 = call i32 @llvm.smax.i32(i32 %198, i32 %202)
  %299 = zext i32 %297 to i64
  %300 = shl nuw i64 %299, 32
  %301 = zext i32 %298 to i64
  %302 = or disjoint i64 %300, %301
  %303 = lshr i64 %302, 30
  %304 = xor i64 %303, %302
  %305 = mul i64 %304, -4658895280553007687
  %306 = lshr i64 %305, 27
  %307 = xor i64 %306, %305
  %308 = mul i64 %307, -7723592293110705685
  %309 = lshr i64 %308, 31
  %310 = xor i64 %309, %308
  %311 = add nsw i64 %296, -1
  %312 = and i64 %311, %310
  %313 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %312
  %314 = getelementptr inbounds nuw i8, ptr %313, i64 16
  %315 = load i8, ptr %314, align 8, !tbaa !25
  %316 = icmp eq i8 %315, 0
  br i1 %316, label %331, label %317

317:                                              ; preds = %294, %324
  %318 = phi ptr [ %327, %324 ], [ %313, %294 ]
  %319 = phi i64 [ %326, %324 ], [ %312, %294 ]
  %320 = load i64, ptr %318, align 8, !tbaa !27
  %321 = icmp eq i64 %320, %302
  br i1 %321, label %322, label %324

322:                                              ; preds = %317
  %323 = getelementptr inbounds nuw i8, ptr %318, i64 8
  store i64 %256, ptr %323, align 8, !tbaa !28
  br label %336

324:                                              ; preds = %317
  %325 = add i64 %319, 1
  %326 = and i64 %325, %311
  %327 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %326
  %328 = getelementptr inbounds nuw i8, ptr %327, i64 16
  %329 = load i8, ptr %328, align 8, !tbaa !25
  %330 = icmp eq i8 %329, 0
  br i1 %330, label %331, label %317, !llvm.loop !29

331:                                              ; preds = %324, %294
  %332 = phi i64 [ %312, %294 ], [ %326, %324 ]
  %333 = phi ptr [ %313, %294 ], [ %327, %324 ]
  store i64 %302, ptr %333, align 8, !tbaa !27
  %334 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %332, i32 1
  store i64 %256, ptr %334, align 8, !tbaa !28
  %335 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %332, i32 2
  store i8 1, ptr %335, align 8, !tbaa !25
  br label %336

336:                                              ; preds = %331, %322, %255, %250, %247, %242, %199
  %337 = add i64 %200, 1
  %338 = icmp eq i64 %337, %195
  br i1 %338, label %187, label %199, !llvm.loop !70

339:                                              ; preds = %187, %130
  call void @free(ptr noundef %119) #19
  %340 = getelementptr inbounds nuw i8, ptr %14, i64 80
  store i32 0, ptr %340, align 8, !tbaa !46
  call fastcc void @refresh_graph_counts_from_canonical(ptr noundef nonnull %14)
  %341 = load ptr, ptr %2, align 8
  br label %342

342:                                              ; preds = %339, %75
  %343 = phi ptr [ %341, %339 ], [ %78, %75 ]
  %344 = icmp ne ptr %343, null
  %345 = select i1 %77, i1 %344, i1 false
  br i1 %345, label %346, label %347

346:                                              ; preds = %342
  call void @free(ptr noundef nonnull %343) #19
  br label %347

347:                                              ; preds = %346, %342
  %348 = load ptr, ptr %3, align 8
  %349 = icmp ne ptr %348, null
  %350 = select i1 %77, i1 %349, i1 false
  br i1 %350, label %351, label %352

351:                                              ; preds = %347
  call void @free(ptr noundef nonnull %348) #19
  br label %352

352:                                              ; preds = %351, %347
  %353 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %354 = getelementptr inbounds nuw i8, ptr %14, i64 128
  store i32 0, ptr %354, align 8, !tbaa !51
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %353, i8 0, i64 16, i1 false)
  call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %14)
  %355 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %355, i8 0, i64 16, i1 false)
  store i32 3, ptr %30, align 8, !tbaa !50
  %356 = getelementptr inbounds nuw i8, ptr %14, i64 80
  store i32 0, ptr %356, align 8, !tbaa !46
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %4) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #19
  br label %357

357:                                              ; preds = %9, %1, %33, %17, %21, %25, %352
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_pcsr_to_csr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #1 {
  %8 = add i64 %0, 1
  %9 = tail call noalias ptr @calloc(i64 noundef %8, i64 noundef 8) #18
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
  br i1 %52, label %53, label %37, !llvm.loop !71

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
  br i1 %64, label %13, label %22, !llvm.loop !74

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
  br i1 %74, label %60, label %65, !llvm.loop !75

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
  br i1 %88, label %89, label %79, !llvm.loop !76

89:                                               ; preds = %75, %79, %7
  %90 = phi i64 [ 0, %7 ], [ %63, %79 ], [ %63, %75 ]
  %91 = shl i64 %90, 2
  %92 = tail call noalias ptr @malloc(i64 noundef %91) #20
  %93 = shl i64 %8, 3
  %94 = tail call noalias ptr @malloc(i64 noundef %93) #20
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
  br i1 %116, label %75, label %98, !llvm.loop !77

117:                                              ; preds = %143, %163, %120
  %118 = icmp eq i64 %123, %0
  br i1 %118, label %119, label %120, !llvm.loop !78

119:                                              ; preds = %117, %89
  tail call void @free(ptr noundef %94) #19
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
  br i1 %165, label %117, label %146, !llvm.loop !79
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_bcsr_to_csr(i64 noundef %0, i32 noundef %1, i32 noundef %2, ptr noundef readonly %3, ptr nocapture noundef readonly %4, ptr noundef writeonly %5, ptr noundef writeonly %6, ptr noundef writeonly %7) local_unnamed_addr #1 {
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
  %22 = tail call noalias ptr @calloc(i64 noundef %21, i64 noundef 8) #18
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
  br i1 %32, label %33, label %44, !llvm.loop !80

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
  br i1 %74, label %57, label %31, !llvm.loop !81

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
  br i1 %88, label %89, label %79, !llvm.loop !82

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
  br i1 %111, label %75, label %93, !llvm.loop !83

112:                                              ; preds = %89
  %113 = shl i64 %91, 2
  %114 = tail call noalias ptr @malloc(i64 noundef %113) #20
  %115 = icmp eq ptr %114, null
  br i1 %115, label %116, label %117

116:                                              ; preds = %112
  tail call void @free(ptr noundef nonnull %22) #19
  br label %167

117:                                              ; preds = %112, %89
  %118 = phi ptr [ %114, %112 ], [ null, %89 ]
  %119 = shl i64 %21, 3
  %120 = tail call noalias ptr @malloc(i64 noundef %119) #20
  %121 = icmp eq ptr %120, null
  br i1 %121, label %122, label %123

122:                                              ; preds = %117
  tail call void @free(ptr noundef nonnull %22) #19
  tail call void @free(ptr noundef %118) #19
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
  br i1 %129, label %130, label %131, !llvm.loop !84

130:                                              ; preds = %128, %123
  tail call void @free(ptr noundef %120) #19
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
  br i1 %166, label %144, label %128, !llvm.loop !85

167:                                              ; preds = %20, %122, %130, %116, %8
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @refresh_graph_counts_from_canonical(ptr nocapture noundef nonnull %0) unnamed_addr #1 {
  %2 = load ptr, ptr %0, align 8, !tbaa !39
  %3 = icmp eq ptr %2, null
  br i1 %3, label %92, label %4

4:                                                ; preds = %1
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !47
  %7 = icmp eq ptr %6, null
  br i1 %7, label %16, label %8

8:                                                ; preds = %4
  %9 = tail call i64 @roaring_bitmap_get_cardinality(ptr noundef nonnull %6) #19
  %10 = icmp eq i64 %9, 0
  br i1 %10, label %16, label %11

11:                                               ; preds = %8
  %12 = add i64 %9, -1
  %13 = tail call i32 @roaring_bitmap_get_at_index(ptr noundef nonnull %6, i64 noundef %12) #19
  %14 = zext i32 %13 to i64
  %15 = add nuw nsw i64 %14, 1
  br label %16

16:                                               ; preds = %4, %8, %11
  %17 = phi i64 [ 0, %4 ], [ %15, %11 ], [ 0, %8 ]
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 88
  %19 = load i64, ptr %18, align 8, !tbaa !58
  %20 = icmp slt i64 %19, 0
  br i1 %20, label %21, label %86

21:                                               ; preds = %16
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !48
  %24 = icmp eq ptr %23, null
  br i1 %24, label %84, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %27 = load i64, ptr %26, align 8, !tbaa !63
  %28 = icmp sgt i64 %27, 0
  br i1 %28, label %64, label %29

29:                                               ; preds = %64, %25
  %30 = phi i64 [ 0, %25 ], [ %70, %64 ]
  %31 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %32 = load i64, ptr %31, align 8, !tbaa !60
  %33 = icmp sgt i64 %32, 0
  br i1 %33, label %34, label %84

34:                                               ; preds = %29
  %35 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %36 = load ptr, ptr %35, align 8, !tbaa !64
  %37 = icmp ult i64 %32, 4
  br i1 %37, label %61, label %38

38:                                               ; preds = %34
  %39 = and i64 %32, 9223372036854775804
  %40 = insertelement <2 x i64> <i64 poison, i64 0>, i64 %30, i64 0
  br label %41

41:                                               ; preds = %41, %38
  %42 = phi i64 [ 0, %38 ], [ %55, %41 ]
  %43 = phi <2 x i64> [ %40, %38 ], [ %53, %41 ]
  %44 = phi <2 x i64> [ zeroinitializer, %38 ], [ %54, %41 ]
  %45 = getelementptr inbounds nuw i8, ptr %36, i64 %42
  %46 = getelementptr inbounds nuw i8, ptr %45, i64 2
  %47 = load <2 x i8>, ptr %45, align 1, !tbaa !65
  %48 = load <2 x i8>, ptr %46, align 1, !tbaa !65
  %49 = icmp ne <2 x i8> %47, zeroinitializer
  %50 = icmp ne <2 x i8> %48, zeroinitializer
  %51 = zext <2 x i1> %49 to <2 x i64>
  %52 = zext <2 x i1> %50 to <2 x i64>
  %53 = add <2 x i64> %43, %51
  %54 = add <2 x i64> %44, %52
  %55 = add nuw i64 %42, 4
  %56 = icmp eq i64 %55, %39
  br i1 %56, label %57, label %41, !llvm.loop !86

57:                                               ; preds = %41
  %58 = add <2 x i64> %54, %53
  %59 = tail call i64 @llvm.vector.reduce.add.v2i64(<2 x i64> %58)
  %60 = icmp eq i64 %32, %39
  br i1 %60, label %84, label %61

61:                                               ; preds = %34, %57
  %62 = phi i64 [ 0, %34 ], [ %39, %57 ]
  %63 = phi i64 [ %30, %34 ], [ %59, %57 ]
  br label %74

64:                                               ; preds = %25, %64
  %65 = phi i64 [ %71, %64 ], [ 0, %25 ]
  %66 = phi i64 [ %70, %64 ], [ 0, %25 ]
  %67 = trunc i64 %65 to i32
  %68 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef nonnull %23, i32 noundef %67) #19
  %69 = zext i1 %68 to i64
  %70 = add nuw nsw i64 %66, %69
  %71 = add nuw nsw i64 %65, 1
  %72 = load i64, ptr %26, align 8, !tbaa !63
  %73 = icmp slt i64 %71, %72
  br i1 %73, label %64, label %29, !llvm.loop !87

74:                                               ; preds = %61, %74
  %75 = phi i64 [ %82, %74 ], [ %62, %61 ]
  %76 = phi i64 [ %81, %74 ], [ %63, %61 ]
  %77 = getelementptr inbounds nuw i8, ptr %36, i64 %75
  %78 = load i8, ptr %77, align 1, !tbaa !65
  %79 = icmp ne i8 %78, 0
  %80 = zext i1 %79 to i64
  %81 = add nsw i64 %76, %80
  %82 = add nuw nsw i64 %75, 1
  %83 = icmp eq i64 %82, %32
  br i1 %83, label %84, label %74, !llvm.loop !88

84:                                               ; preds = %74, %57, %29, %21
  %85 = phi i64 [ 0, %21 ], [ %30, %29 ], [ %59, %57 ], [ %81, %74 ]
  store i64 %85, ptr %18, align 8, !tbaa !58
  br label %86

86:                                               ; preds = %16, %84
  %87 = phi i64 [ %19, %16 ], [ %85, %84 ]
  %88 = shl nsw i64 %87, 1
  store i64 %17, ptr %2, align 8, !tbaa !11
  %89 = getelementptr inbounds nuw i8, ptr %2, i64 8
  store i64 %88, ptr %89, align 8, !tbaa !11
  %90 = getelementptr inbounds nuw i8, ptr %0, i64 112
  store i64 %17, ptr %90, align 8, !tbaa !44
  %91 = getelementptr inbounds nuw i8, ptr %0, i64 120
  store i64 %88, ptr %91, align 8, !tbaa !45
  br label %92

92:                                               ; preds = %1, %86
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_pcsr(i64 noundef %0, i64 noundef %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #1 {
  %8 = tail call i64 @llvm.smax.i64(i64 %1, i64 16)
  %9 = shl nuw i64 %8, 1
  %10 = shl i64 %8, 3
  %11 = tail call noalias ptr @malloc(i64 noundef %10) #20
  %12 = shl i64 %0, 3
  %13 = add i64 %12, 8
  %14 = tail call noalias ptr @malloc(i64 noundef %13) #20
  tail call void @llvm.memset.p0.i64(ptr align 4 %11, i8 -1, i64 %10, i1 false)
  %15 = icmp sgt i64 %0, 0
  br i1 %15, label %16, label %20

16:                                               ; preds = %7
  %17 = ptrtoint ptr %3 to i64
  br label %25

18:                                               ; preds = %84, %80, %50
  %19 = icmp eq i64 %30, %0
  br i1 %19, label %20, label %25, !llvm.loop !89

20:                                               ; preds = %18, %7
  %21 = phi i64 [ 0, %7 ], [ %38, %18 ]
  %22 = phi ptr [ %11, %7 ], [ %51, %18 ]
  %23 = phi i64 [ %9, %7 ], [ %52, %18 ]
  %24 = getelementptr inbounds i64, ptr %14, i64 %0
  store i64 %21, ptr %24, align 8, !tbaa !11
  store ptr %14, ptr %4, align 8, !tbaa !17
  store ptr %22, ptr %5, align 8, !tbaa !20
  store i64 %23, ptr %6, align 8, !tbaa !11
  ret void

25:                                               ; preds = %16, %18
  %26 = phi i64 [ %52, %18 ], [ %9, %16 ]
  %27 = phi ptr [ %51, %18 ], [ %11, %16 ]
  %28 = phi i64 [ %38, %18 ], [ 0, %16 ]
  %29 = phi i64 [ %30, %18 ], [ 0, %16 ]
  %30 = add nuw nsw i64 %29, 1
  %31 = getelementptr inbounds nuw i64, ptr %2, i64 %30
  %32 = load i64, ptr %31, align 8, !tbaa !11
  %33 = getelementptr inbounds nuw i64, ptr %2, i64 %29
  %34 = load i64, ptr %33, align 8, !tbaa !11
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
  br i1 %42, label %40, label %44, !llvm.loop !90

44:                                               ; preds = %40
  %45 = shl i64 %41, 2
  %46 = tail call ptr @realloc(ptr noundef %27, i64 noundef %45) #21
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
  store i64 %28, ptr %54, align 8, !tbaa !11
  %55 = icmp sgt i64 %35, 0
  br i1 %55, label %56, label %18

56:                                               ; preds = %50
  %57 = load i64, ptr %33, align 8, !tbaa !11
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
  %74 = load <4 x i32>, ptr %72, align 4, !tbaa !22
  %75 = load <4 x i32>, ptr %73, align 4, !tbaa !22
  %76 = getelementptr i32, ptr %59, i64 %71
  %77 = getelementptr i8, ptr %76, i64 16
  store <4 x i32> %74, ptr %76, align 4, !tbaa !22
  store <4 x i32> %75, ptr %77, align 4, !tbaa !22
  %78 = add nuw i64 %71, 8
  %79 = icmp eq i64 %78, %69
  br i1 %79, label %80, label %70, !llvm.loop !91

80:                                               ; preds = %70
  %81 = icmp eq i64 %35, %69
  br i1 %81, label %18, label %82

82:                                               ; preds = %61, %56, %80
  %83 = phi i64 [ 0, %56 ], [ 0, %61 ], [ %69, %80 ]
  br label %84

84:                                               ; preds = %82, %84
  %85 = phi i64 [ %89, %84 ], [ %83, %82 ]
  %86 = getelementptr i32, ptr %58, i64 %85
  %87 = load i32, ptr %86, align 4, !tbaa !22
  %88 = getelementptr i32, ptr %59, i64 %85
  store i32 %87, ptr %88, align 4, !tbaa !22
  %89 = add nuw nsw i64 %85, 1
  %90 = icmp eq i64 %89, %35
  br i1 %90, label %18, label %84, !llvm.loop !92
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #10

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #11

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_bcsr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr noundef writeonly %4, ptr noundef writeonly %5, i32 noundef %6, ptr noundef writeonly %7) local_unnamed_addr #1 {
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
  %25 = tail call noalias ptr @calloc(i64 noundef %24, i64 noundef 4) #18
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
  br i1 %74, label %75, label %55, !llvm.loop !93

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
  br i1 %101, label %91, label %84, !llvm.loop !94

102:                                              ; preds = %87
  %103 = trunc nuw nsw i64 %89 to i32
  %104 = getelementptr inbounds nuw i32, ptr %28, i64 %33
  store i32 %103, ptr %104, align 4, !tbaa !22
  %105 = add nuw nsw i64 %33, 1
  %106 = icmp eq i64 %105, %31
  br i1 %106, label %108, label %32, !llvm.loop !95

107:                                              ; preds = %87
  tail call void @free(ptr noundef %25) #19
  br label %191

108:                                              ; preds = %102
  %109 = icmp eq i64 %89, 0
  br i1 %109, label %115, label %110

110:                                              ; preds = %108
  %111 = shl nuw nsw i64 %89, 2
  %112 = tail call noalias ptr @malloc(i64 noundef %111) #20
  %113 = icmp eq ptr %112, null
  br i1 %113, label %114, label %115

114:                                              ; preds = %110
  tail call void @free(ptr noundef nonnull %25) #19
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
  br i1 %137, label %141, label %138, !llvm.loop !96

138:                                              ; preds = %135, %121
  %139 = add nuw nsw i64 %122, 1
  %140 = icmp eq i64 %139, %117
  br i1 %140, label %119, label %121, !llvm.loop !97

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
  br i1 %175, label %176, label %162, !llvm.loop !98

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
  br i1 %190, label %132, label %181, !llvm.loop !99

191:                                              ; preds = %107, %16, %114, %119, %8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readnone %3, ptr nocapture noundef readnone %4, ptr noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8) local_unnamed_addr #1 {
  %10 = alloca %struct.timespec, align 8
  %11 = alloca %struct.timespec, align 8
  %12 = alloca %struct.timespec, align 8
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
  br i1 %25, label %26, label %226

26:                                               ; preds = %9
  %27 = zext nneg i32 %24 to i64
  br label %31

28:                                               ; preds = %31
  %29 = add nuw nsw i64 %32, 1
  %30 = icmp eq i64 %29, %27
  br i1 %30, label %226, label %31, !llvm.loop !38

31:                                               ; preds = %28, %26
  %32 = phi i64 [ 0, %26 ], [ %29, %28 ]
  %33 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %32
  %34 = load ptr, ptr %33, align 8, !tbaa !39
  %35 = icmp eq ptr %34, %0
  br i1 %35, label %36, label %28

36:                                               ; preds = %31
  %37 = icmp eq ptr %5, null
  br i1 %37, label %40, label %38

38:                                               ; preds = %36
  %39 = getelementptr inbounds nuw i8, ptr %33, i64 16
  store ptr %5, ptr %39, align 8, !tbaa !47
  br label %40

40:                                               ; preds = %38, %36
  %41 = icmp eq ptr %6, null
  br i1 %41, label %44, label %42

42:                                               ; preds = %40
  %43 = getelementptr inbounds nuw i8, ptr %33, i64 24
  store ptr %6, ptr %43, align 8, !tbaa !48
  br label %44

44:                                               ; preds = %42, %40
  %45 = icmp eq ptr %7, null
  br i1 %45, label %48, label %46

46:                                               ; preds = %44
  %47 = getelementptr inbounds nuw i8, ptr %33, i64 32
  store ptr %7, ptr %47, align 8, !tbaa !49
  br label %48

48:                                               ; preds = %46, %44
  %49 = getelementptr inbounds nuw i8, ptr %33, i64 16
  %50 = load ptr, ptr %49, align 8, !tbaa !47
  %51 = icmp eq ptr %50, null
  br i1 %51, label %226, label %52

52:                                               ; preds = %48
  %53 = getelementptr inbounds nuw i8, ptr %33, i64 24
  %54 = load ptr, ptr %53, align 8, !tbaa !48
  %55 = icmp eq ptr %54, null
  br i1 %55, label %226, label %56

56:                                               ; preds = %52
  %57 = getelementptr inbounds nuw i8, ptr %33, i64 32
  %58 = load ptr, ptr %57, align 8, !tbaa !49
  %59 = icmp eq ptr %58, null
  %60 = icmp ugt i32 %8, 3
  %61 = or i1 %60, %59
  br i1 %61, label %226, label %62

62:                                               ; preds = %56
  %63 = getelementptr inbounds nuw i8, ptr %33, i64 8
  %64 = load i32, ptr %63, align 8, !tbaa !50
  %65 = icmp eq i32 %64, %8
  br i1 %65, label %226, label %66

66:                                               ; preds = %62
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %12) #19
  %67 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %12) #19
  %68 = load i64, ptr %12, align 8, !tbaa !100
  %69 = mul i64 %68, -1000000000
  %70 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %71 = load i64, ptr %70, align 8, !tbaa !102
  %72 = sub i64 %69, %71
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %12) #19
  %73 = icmp eq i32 %8, 3
  br i1 %73, label %74, label %79

74:                                               ; preds = %66
  call void @autograph_ensure_layout_set(ptr noundef %0)
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %11) #19
  %75 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %11) #19
  %76 = load i64, ptr %11, align 8, !tbaa !100
  %77 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %78 = load i64, ptr %77, align 8, !tbaa !102
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %11) #19
  br label %216

79:                                               ; preds = %66
  %80 = load i32, ptr %63, align 8, !tbaa !50
  %81 = icmp eq i32 %80, 3
  br i1 %81, label %82, label %105

82:                                               ; preds = %79
  %83 = getelementptr inbounds nuw i8, ptr %33, i64 112
  switch i32 %8, label %97 [
    i32 0, label %84
    i32 1, label %89
  ]

84:                                               ; preds = %82
  %85 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %86 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %87 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %88 = getelementptr inbounds nuw i8, ptr %33, i64 120
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %85, ptr noundef %86, ptr noundef %87, ptr noundef %88)
  store i32 0, ptr %63, align 8, !tbaa !50
  br label %186

89:                                               ; preds = %82
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %13) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %14) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %15) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %16) #19
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %14, ptr noundef %15, ptr noundef %16, ptr noundef %13)
  %90 = load i64, ptr %14, align 8, !tbaa !11
  store i64 %90, ptr %83, align 8, !tbaa !44
  %91 = load i64, ptr %13, align 8, !tbaa !11
  %92 = load ptr, ptr %15, align 8, !tbaa !17
  %93 = load ptr, ptr %16, align 8, !tbaa !20
  %94 = getelementptr inbounds nuw i8, ptr %33, i64 136
  %95 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %96 = getelementptr inbounds nuw i8, ptr %33, i64 152
  call void @convert_csr_to_pcsr(i64 noundef %90, i64 noundef %91, ptr noundef %92, ptr noundef %93, ptr noundef nonnull %94, ptr noundef nonnull %95, ptr noundef nonnull %96)
  call void @free(ptr noundef %92) #19
  call void @free(ptr noundef %93) #19
  store i32 1, ptr %63, align 8, !tbaa !50
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %16) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %15) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %14) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %13) #19
  br label %195

97:                                               ; preds = %82
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %17) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %18) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %19) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %20) #19
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %18, ptr noundef %19, ptr noundef %20, ptr noundef %17)
  %98 = load i64, ptr %18, align 8, !tbaa !11
  store i64 %98, ptr %83, align 8, !tbaa !44
  %99 = getelementptr inbounds nuw i8, ptr %33, i64 176
  store i32 64, ptr %99, align 8, !tbaa !55
  %100 = load ptr, ptr %19, align 8, !tbaa !17
  %101 = load ptr, ptr %20, align 8, !tbaa !20
  %102 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %103 = getelementptr inbounds nuw i8, ptr %33, i64 168
  %104 = getelementptr inbounds nuw i8, ptr %33, i64 180
  call void @convert_csr_to_bcsr(i64 noundef %98, i64 poison, ptr noundef %100, ptr noundef %101, ptr noundef nonnull %102, ptr noundef nonnull %103, i32 noundef 64, ptr noundef nonnull %104)
  call void @free(ptr noundef %100) #19
  call void @free(ptr noundef %101) #19
  store i32 2, ptr %63, align 8, !tbaa !50
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %20) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %19) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %18) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %17) #19
  br label %204

105:                                              ; preds = %79
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %21) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %22) #19
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %23) #19
  switch i32 %80, label %128 [
    i32 1, label %106
    i32 2, label %115
  ]

106:                                              ; preds = %105
  %107 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %108 = load i64, ptr %107, align 8, !tbaa !44
  %109 = getelementptr inbounds nuw i8, ptr %33, i64 136
  %110 = load ptr, ptr %109, align 8, !tbaa !52
  %111 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %112 = load ptr, ptr %111, align 8, !tbaa !53
  call void @convert_pcsr_to_csr(i64 noundef %108, i64 poison, ptr noundef %110, ptr noundef %112, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %113 = load ptr, ptr %109, align 8, !tbaa !52
  call void @free(ptr noundef %113) #19
  store ptr null, ptr %109, align 8, !tbaa !52
  %114 = load ptr, ptr %111, align 8, !tbaa !53
  call void @free(ptr noundef %114) #19
  store ptr null, ptr %111, align 8, !tbaa !53
  br label %137

115:                                              ; preds = %105
  %116 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %117 = load i64, ptr %116, align 8, !tbaa !44
  %118 = getelementptr inbounds nuw i8, ptr %33, i64 180
  %119 = load i32, ptr %118, align 4, !tbaa !54
  %120 = getelementptr inbounds nuw i8, ptr %33, i64 176
  %121 = load i32, ptr %120, align 8, !tbaa !55
  %122 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %123 = load ptr, ptr %122, align 8, !tbaa !56
  %124 = getelementptr inbounds nuw i8, ptr %33, i64 168
  %125 = load ptr, ptr %124, align 8, !tbaa !57
  call void @convert_bcsr_to_csr(i64 noundef %117, i32 noundef %119, i32 noundef %121, ptr noundef %123, ptr noundef %125, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %126 = load ptr, ptr %122, align 8, !tbaa !56
  call void @free(ptr noundef %126) #19
  store ptr null, ptr %122, align 8, !tbaa !56
  %127 = load ptr, ptr %124, align 8, !tbaa !57
  call void @free(ptr noundef %127) #19
  store ptr null, ptr %124, align 8, !tbaa !57
  br label %137

128:                                              ; preds = %105
  %129 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %130 = load i64, ptr %129, align 8, !tbaa !45
  store i64 %130, ptr %21, align 8, !tbaa !11
  %131 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %132 = load ptr, ptr %131, align 8, !tbaa !42
  store ptr %132, ptr %22, align 8, !tbaa !17
  %133 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %134 = load ptr, ptr %133, align 8, !tbaa !43
  store ptr %134, ptr %23, align 8, !tbaa !20
  %135 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %136 = load i32, ptr %135, align 8, !tbaa !51
  br label %137

137:                                              ; preds = %115, %128, %106
  %138 = phi i32 [ 1, %106 ], [ 1, %115 ], [ %136, %128 ]
  switch i32 %8, label %157 [
    i32 0, label %177
    i32 1, label %139
  ]

139:                                              ; preds = %137
  %140 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %141 = load i64, ptr %140, align 8, !tbaa !44
  %142 = load i64, ptr %21, align 8, !tbaa !11
  %143 = load ptr, ptr %22, align 8, !tbaa !17
  %144 = load ptr, ptr %23, align 8, !tbaa !20
  %145 = getelementptr inbounds nuw i8, ptr %33, i64 136
  %146 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %147 = getelementptr inbounds nuw i8, ptr %33, i64 152
  call void @convert_csr_to_pcsr(i64 noundef %141, i64 noundef %142, ptr noundef %143, ptr noundef %144, ptr noundef nonnull %145, ptr noundef nonnull %146, ptr noundef nonnull %147)
  %148 = icmp eq i32 %138, 0
  br i1 %148, label %152, label %149

149:                                              ; preds = %139
  %150 = load ptr, ptr %22, align 8, !tbaa !17
  call void @free(ptr noundef %150) #19
  %151 = load ptr, ptr %23, align 8, !tbaa !20
  call void @free(ptr noundef %151) #19
  br label %152

152:                                              ; preds = %149, %139
  %153 = load i32, ptr %63, align 8, !tbaa !50
  %154 = icmp eq i32 %153, 0
  br i1 %154, label %155, label %176

155:                                              ; preds = %152
  %156 = getelementptr inbounds nuw i8, ptr %33, i64 96
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %156, i8 0, i64 16, i1 false)
  br label %176

157:                                              ; preds = %137
  %158 = getelementptr inbounds nuw i8, ptr %33, i64 176
  store i32 64, ptr %158, align 8, !tbaa !55
  %159 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %160 = load i64, ptr %159, align 8, !tbaa !44
  %161 = load ptr, ptr %22, align 8, !tbaa !17
  %162 = load ptr, ptr %23, align 8, !tbaa !20
  %163 = getelementptr inbounds nuw i8, ptr %33, i64 160
  %164 = getelementptr inbounds nuw i8, ptr %33, i64 168
  %165 = getelementptr inbounds nuw i8, ptr %33, i64 180
  call void @convert_csr_to_bcsr(i64 noundef %160, i64 poison, ptr noundef %161, ptr noundef %162, ptr noundef nonnull %163, ptr noundef nonnull %164, i32 noundef 64, ptr noundef nonnull %165)
  %166 = icmp eq i32 %138, 0
  br i1 %166, label %170, label %167

167:                                              ; preds = %157
  %168 = load ptr, ptr %22, align 8, !tbaa !17
  call void @free(ptr noundef %168) #19
  %169 = load ptr, ptr %23, align 8, !tbaa !20
  call void @free(ptr noundef %169) #19
  br label %170

170:                                              ; preds = %167, %157
  %171 = load i32, ptr %63, align 8, !tbaa !50
  %172 = icmp eq i32 %171, 0
  br i1 %172, label %173, label %175

173:                                              ; preds = %170
  %174 = getelementptr inbounds nuw i8, ptr %33, i64 96
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %174, i8 0, i64 16, i1 false)
  br label %175

175:                                              ; preds = %173, %170
  store i32 %8, ptr %63, align 8, !tbaa !50
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #19
  br label %204

176:                                              ; preds = %152, %155
  store i32 %8, ptr %63, align 8, !tbaa !50
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #19
  br label %195

177:                                              ; preds = %137
  %178 = load i64, ptr %21, align 8, !tbaa !11
  %179 = getelementptr inbounds nuw i8, ptr %33, i64 120
  store i64 %178, ptr %179, align 8, !tbaa !45
  %180 = load ptr, ptr %22, align 8, !tbaa !17
  %181 = getelementptr inbounds nuw i8, ptr %33, i64 96
  store ptr %180, ptr %181, align 8, !tbaa !42
  %182 = load ptr, ptr %23, align 8, !tbaa !20
  %183 = getelementptr inbounds nuw i8, ptr %33, i64 104
  store ptr %182, ptr %183, align 8, !tbaa !43
  %184 = getelementptr inbounds nuw i8, ptr %33, i64 128
  store i32 %138, ptr %184, align 8, !tbaa !51
  store i32 %8, ptr %63, align 8, !tbaa !50
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #19
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #19
  %185 = getelementptr inbounds nuw i8, ptr %33, i64 112
  br label %186

186:                                              ; preds = %177, %84
  %187 = phi ptr [ %185, %177 ], [ %83, %84 ]
  %188 = load i64, ptr %187, align 8, !tbaa !44
  store i64 %188, ptr %0, align 8, !tbaa !11
  %189 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %190 = load i64, ptr %189, align 8, !tbaa !45
  %191 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %190, ptr %191, align 8, !tbaa !11
  %192 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %193 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %194 = load <2 x ptr>, ptr %192, align 8, !tbaa !103
  store <2 x ptr> %194, ptr %193, align 8, !tbaa !103
  br label %211

195:                                              ; preds = %176, %89
  %196 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %197 = load i64, ptr %196, align 8, !tbaa !44
  store i64 %197, ptr %0, align 8, !tbaa !11
  %198 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %199 = load i64, ptr %198, align 8, !tbaa !45
  %200 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %199, ptr %200, align 8, !tbaa !11
  %201 = getelementptr inbounds nuw i8, ptr %33, i64 136
  %202 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %203 = load <2 x ptr>, ptr %201, align 8, !tbaa !103
  store <2 x ptr> %203, ptr %202, align 8, !tbaa !103
  br label %211

204:                                              ; preds = %175, %97
  %205 = getelementptr inbounds nuw i8, ptr %33, i64 112
  %206 = load i64, ptr %205, align 8, !tbaa !44
  store i64 %206, ptr %0, align 8, !tbaa !11
  %207 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %208 = load i64, ptr %207, align 8, !tbaa !45
  %209 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %208, ptr %209, align 8, !tbaa !11
  %210 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %210, i8 0, i64 16, i1 false)
  br label %211

211:                                              ; preds = %195, %204, %186
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %10) #19
  %212 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %10) #19
  %213 = load i64, ptr %10, align 8, !tbaa !100
  %214 = getelementptr inbounds nuw i8, ptr %10, i64 8
  %215 = load i64, ptr %214, align 8, !tbaa !102
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %10) #19
  br label %216

216:                                              ; preds = %211, %74
  %217 = phi i64 [ %215, %211 ], [ %78, %74 ]
  %218 = phi i64 [ %213, %211 ], [ %76, %74 ]
  %219 = mul i64 %218, 1000000000
  %220 = load i64, ptr @g_conversion_ns, align 8, !tbaa !11
  %221 = add i64 %217, %72
  %222 = add i64 %221, %219
  %223 = add i64 %222, %220
  store i64 %223, ptr @g_conversion_ns, align 8, !tbaa !11
  %224 = load i32, ptr @g_conversions_injected, align 4, !tbaa !22
  %225 = add nsw i32 %224, 1
  store i32 %225, ptr @g_conversions_injected, align 4, !tbaa !22
  br label %226

226:                                              ; preds = %28, %9, %62, %48, %52, %56, %216
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @build_csr_from_meta(ptr nocapture noundef nonnull %0, ptr nocapture noundef nonnull writeonly %1, ptr nocapture noundef nonnull writeonly %2, ptr nocapture noundef nonnull writeonly %3, ptr nocapture noundef nonnull writeonly %4) unnamed_addr #1 {
  %6 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %7 = load ptr, ptr %6, align 8, !tbaa !47
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %9 = load ptr, ptr %8, align 8, !tbaa !48
  %10 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %11 = load ptr, ptr %10, align 8, !tbaa !49
  %12 = tail call i64 @roaring_bitmap_get_cardinality(ptr noundef %7) #19
  %13 = icmp slt i64 %12, 1
  br i1 %13, label %20, label %14

14:                                               ; preds = %5
  %15 = add nsw i64 %12, -1
  %16 = tail call i32 @roaring_bitmap_get_at_index(ptr noundef %7, i64 noundef %15) #19
  %17 = tail call i32 @llvm.smax.i32(i32 %16, i32 -1)
  %18 = add i32 %17, 1
  %19 = zext i32 %18 to i64
  br label %20

20:                                               ; preds = %14, %5
  %21 = phi i64 [ %19, %14 ], [ 0, %5 ]
  %22 = add nuw nsw i64 %21, 1
  %23 = tail call noalias ptr @calloc(i64 noundef %22, i64 noundef 8) #18
  %24 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %25 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %26 = load i64, ptr %24, align 8, !tbaa !63
  %27 = icmp sgt i64 %26, 0
  br i1 %27, label %40, label %30

28:                                               ; preds = %67
  %29 = icmp sgt i64 %70, 0
  br label %30

30:                                               ; preds = %28, %20
  %31 = phi i1 [ false, %20 ], [ %29, %28 ]
  %32 = phi i64 [ 0, %20 ], [ %68, %28 ]
  %33 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %34 = load i64, ptr %33, align 8, !tbaa !60
  %35 = icmp sgt i64 %34, 0
  br i1 %35, label %36, label %72

36:                                               ; preds = %30
  %37 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %38 = load ptr, ptr %37, align 8, !tbaa !64
  %39 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %84

40:                                               ; preds = %20, %67
  %41 = phi i64 [ %68, %67 ], [ 0, %20 ]
  %42 = phi i64 [ %69, %67 ], [ 0, %20 ]
  %43 = trunc i64 %42 to i32
  %44 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %43) #19
  br i1 %44, label %45, label %67

45:                                               ; preds = %40
  %46 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %42
  %47 = load i32, ptr %46, align 4, !tbaa !3
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !8
  %50 = icmp sgt i32 %47, -1
  %51 = icmp sgt i32 %49, -1
  %52 = select i1 %50, i1 %51, i1 false
  br i1 %52, label %53, label %67

53:                                               ; preds = %45
  %54 = zext nneg i32 %47 to i64
  %55 = icmp samesign ugt i64 %21, %54
  br i1 %55, label %56, label %67

56:                                               ; preds = %53
  %57 = zext nneg i32 %49 to i64
  %58 = icmp samesign ugt i64 %21, %57
  br i1 %58, label %59, label %67

59:                                               ; preds = %56
  %60 = getelementptr inbounds nuw i64, ptr %25, i64 %54
  %61 = load i64, ptr %60, align 8, !tbaa !11
  %62 = add nsw i64 %61, 1
  store i64 %62, ptr %60, align 8, !tbaa !11
  %63 = getelementptr inbounds nuw i64, ptr %25, i64 %57
  %64 = load i64, ptr %63, align 8, !tbaa !11
  %65 = add nsw i64 %64, 1
  store i64 %65, ptr %63, align 8, !tbaa !11
  %66 = add nsw i64 %41, 2
  br label %67

67:                                               ; preds = %45, %53, %56, %59, %40
  %68 = phi i64 [ %41, %40 ], [ %66, %59 ], [ %41, %56 ], [ %41, %53 ], [ %41, %45 ]
  %69 = add nuw nsw i64 %42, 1
  %70 = load i64, ptr %24, align 8, !tbaa !63
  %71 = icmp slt i64 %69, %70
  br i1 %71, label %40, label %28, !llvm.loop !104

72:                                               ; preds = %115, %30
  %73 = phi i64 [ %32, %30 ], [ %116, %115 ]
  %74 = icmp eq i64 %21, 0
  br i1 %74, label %133, label %75

75:                                               ; preds = %72
  %76 = load i64, ptr %23, align 8
  %77 = and i64 %21, 3
  %78 = icmp samesign ult i64 %21, 4
  br i1 %78, label %119, label %79

79:                                               ; preds = %75
  %80 = and i64 %21, 4294967292
  %81 = getelementptr i8, ptr %23, i64 8
  %82 = getelementptr i8, ptr %23, i64 16
  %83 = getelementptr i8, ptr %23, i64 24
  br label %135

84:                                               ; preds = %36, %115
  %85 = phi i64 [ %32, %36 ], [ %116, %115 ]
  %86 = phi i64 [ 0, %36 ], [ %117, %115 ]
  %87 = getelementptr inbounds nuw i8, ptr %38, i64 %86
  %88 = load i8, ptr %87, align 1, !tbaa !65
  %89 = icmp eq i8 %88, 0
  br i1 %89, label %115, label %90

90:                                               ; preds = %84
  %91 = load ptr, ptr %39, align 8, !tbaa !69
  %92 = shl nuw nsw i64 %86, 1
  %93 = getelementptr inbounds nuw i32, ptr %91, i64 %92
  %94 = load i32, ptr %93, align 4, !tbaa !22
  %95 = or disjoint i64 %92, 1
  %96 = getelementptr inbounds nuw i32, ptr %91, i64 %95
  %97 = load i32, ptr %96, align 4, !tbaa !22
  %98 = icmp sgt i32 %94, -1
  %99 = icmp sgt i32 %97, -1
  %100 = select i1 %98, i1 %99, i1 false
  br i1 %100, label %101, label %115

101:                                              ; preds = %90
  %102 = zext nneg i32 %94 to i64
  %103 = icmp samesign ugt i64 %21, %102
  br i1 %103, label %104, label %115

104:                                              ; preds = %101
  %105 = zext nneg i32 %97 to i64
  %106 = icmp samesign ugt i64 %21, %105
  br i1 %106, label %107, label %115

107:                                              ; preds = %104
  %108 = getelementptr inbounds nuw i64, ptr %25, i64 %102
  %109 = load i64, ptr %108, align 8, !tbaa !11
  %110 = add nsw i64 %109, 1
  store i64 %110, ptr %108, align 8, !tbaa !11
  %111 = getelementptr inbounds nuw i64, ptr %25, i64 %105
  %112 = load i64, ptr %111, align 8, !tbaa !11
  %113 = add nsw i64 %112, 1
  store i64 %113, ptr %111, align 8, !tbaa !11
  %114 = add nsw i64 %85, 2
  br label %115

115:                                              ; preds = %90, %101, %104, %107, %84
  %116 = phi i64 [ %85, %84 ], [ %114, %107 ], [ %85, %104 ], [ %85, %101 ], [ %85, %90 ]
  %117 = add nuw nsw i64 %86, 1
  %118 = icmp eq i64 %117, %34
  br i1 %118, label %72, label %84, !llvm.loop !105

119:                                              ; preds = %135, %75
  %120 = phi i64 [ %76, %75 ], [ %150, %135 ]
  %121 = phi i64 [ 1, %75 ], [ %151, %135 ]
  %122 = icmp eq i64 %77, 0
  br i1 %122, label %133, label %123

123:                                              ; preds = %119, %123
  %124 = phi i64 [ %129, %123 ], [ %120, %119 ]
  %125 = phi i64 [ %130, %123 ], [ %121, %119 ]
  %126 = phi i64 [ %131, %123 ], [ 0, %119 ]
  %127 = getelementptr i64, ptr %23, i64 %125
  %128 = load i64, ptr %127, align 8, !tbaa !11
  %129 = add nsw i64 %128, %124
  store i64 %129, ptr %127, align 8, !tbaa !11
  %130 = add nuw nsw i64 %125, 1
  %131 = add i64 %126, 1
  %132 = icmp eq i64 %131, %77
  br i1 %132, label %133, label %123, !llvm.loop !106

133:                                              ; preds = %119, %123, %72
  %134 = icmp sgt i64 %73, 0
  br i1 %134, label %154, label %157

135:                                              ; preds = %135, %79
  %136 = phi i64 [ %76, %79 ], [ %150, %135 ]
  %137 = phi i64 [ 1, %79 ], [ %151, %135 ]
  %138 = phi i64 [ 0, %79 ], [ %152, %135 ]
  %139 = getelementptr i64, ptr %23, i64 %137
  %140 = load i64, ptr %139, align 8, !tbaa !11
  %141 = add nsw i64 %140, %136
  store i64 %141, ptr %139, align 8, !tbaa !11
  %142 = getelementptr i64, ptr %81, i64 %137
  %143 = load i64, ptr %142, align 8, !tbaa !11
  %144 = add nsw i64 %143, %141
  store i64 %144, ptr %142, align 8, !tbaa !11
  %145 = getelementptr i64, ptr %82, i64 %137
  %146 = load i64, ptr %145, align 8, !tbaa !11
  %147 = add nsw i64 %146, %144
  store i64 %147, ptr %145, align 8, !tbaa !11
  %148 = getelementptr i64, ptr %83, i64 %137
  %149 = load i64, ptr %148, align 8, !tbaa !11
  %150 = add nsw i64 %149, %147
  store i64 %150, ptr %148, align 8, !tbaa !11
  %151 = add nuw nsw i64 %137, 4
  %152 = add i64 %138, 4
  %153 = icmp eq i64 %152, %80
  br i1 %153, label %119, label %135, !llvm.loop !107

154:                                              ; preds = %133
  %155 = shl i64 %73, 2
  %156 = tail call noalias ptr @malloc(i64 noundef %155) #20
  br label %157

157:                                              ; preds = %133, %154
  %158 = phi ptr [ %156, %154 ], [ null, %133 ]
  %159 = shl nuw nsw i64 %22, 3
  %160 = tail call noalias ptr @malloc(i64 noundef %159) #20
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %160, ptr noundef nonnull align 8 dereferenceable(1) %23, i64 %159, i1 false)
  br i1 %31, label %170, label %163

161:                                              ; preds = %197
  %162 = load i64, ptr %33, align 8, !tbaa !60
  br label %163

163:                                              ; preds = %161, %157
  %164 = phi i64 [ %162, %161 ], [ %34, %157 ]
  %165 = icmp sgt i64 %164, 0
  br i1 %165, label %166, label %201

166:                                              ; preds = %163
  %167 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %168 = load ptr, ptr %167, align 8, !tbaa !64
  %169 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %203

170:                                              ; preds = %157, %197
  %171 = phi i64 [ %198, %197 ], [ 0, %157 ]
  %172 = trunc i64 %171 to i32
  %173 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %172) #19
  br i1 %173, label %174, label %197

174:                                              ; preds = %170
  %175 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %171
  %176 = load i32, ptr %175, align 4, !tbaa !3
  %177 = getelementptr inbounds nuw i8, ptr %175, i64 4
  %178 = load i32, ptr %177, align 4, !tbaa !8
  %179 = icmp sgt i32 %176, -1
  %180 = icmp sgt i32 %178, -1
  %181 = select i1 %179, i1 %180, i1 false
  br i1 %181, label %182, label %197

182:                                              ; preds = %174
  %183 = zext nneg i32 %176 to i64
  %184 = icmp samesign ugt i64 %21, %183
  br i1 %184, label %185, label %197

185:                                              ; preds = %182
  %186 = zext nneg i32 %178 to i64
  %187 = icmp samesign ugt i64 %21, %186
  br i1 %187, label %188, label %197

188:                                              ; preds = %185
  %189 = getelementptr inbounds nuw i64, ptr %160, i64 %183
  %190 = load i64, ptr %189, align 8, !tbaa !11
  %191 = add nsw i64 %190, 1
  store i64 %191, ptr %189, align 8, !tbaa !11
  %192 = getelementptr inbounds i32, ptr %158, i64 %190
  store i32 %178, ptr %192, align 4, !tbaa !22
  %193 = getelementptr inbounds nuw i64, ptr %160, i64 %186
  %194 = load i64, ptr %193, align 8, !tbaa !11
  %195 = add nsw i64 %194, 1
  store i64 %195, ptr %193, align 8, !tbaa !11
  %196 = getelementptr inbounds i32, ptr %158, i64 %194
  store i32 %176, ptr %196, align 4, !tbaa !22
  br label %197

197:                                              ; preds = %174, %182, %185, %188, %170
  %198 = add nuw nsw i64 %171, 1
  %199 = load i64, ptr %24, align 8, !tbaa !63
  %200 = icmp slt i64 %198, %199
  br i1 %200, label %170, label %161, !llvm.loop !108

201:                                              ; preds = %234, %163
  tail call void @free(ptr noundef %160) #19
  store i64 %21, ptr %1, align 8, !tbaa !11
  store ptr %23, ptr %2, align 8, !tbaa !17
  store ptr %158, ptr %3, align 8, !tbaa !20
  store i64 %73, ptr %4, align 8, !tbaa !11
  %202 = getelementptr inbounds nuw i8, ptr %0, i64 128
  store i32 1, ptr %202, align 8, !tbaa !51
  ret void

203:                                              ; preds = %166, %234
  %204 = phi i64 [ 0, %166 ], [ %235, %234 ]
  %205 = getelementptr inbounds nuw i8, ptr %168, i64 %204
  %206 = load i8, ptr %205, align 1, !tbaa !65
  %207 = icmp eq i8 %206, 0
  br i1 %207, label %234, label %208

208:                                              ; preds = %203
  %209 = load ptr, ptr %169, align 8, !tbaa !69
  %210 = shl nuw nsw i64 %204, 1
  %211 = getelementptr inbounds nuw i32, ptr %209, i64 %210
  %212 = load i32, ptr %211, align 4, !tbaa !22
  %213 = or disjoint i64 %210, 1
  %214 = getelementptr inbounds nuw i32, ptr %209, i64 %213
  %215 = load i32, ptr %214, align 4, !tbaa !22
  %216 = icmp sgt i32 %212, -1
  %217 = icmp sgt i32 %215, -1
  %218 = select i1 %216, i1 %217, i1 false
  br i1 %218, label %219, label %234

219:                                              ; preds = %208
  %220 = zext nneg i32 %212 to i64
  %221 = icmp samesign ugt i64 %21, %220
  br i1 %221, label %222, label %234

222:                                              ; preds = %219
  %223 = zext nneg i32 %215 to i64
  %224 = icmp samesign ugt i64 %21, %223
  br i1 %224, label %225, label %234

225:                                              ; preds = %222
  %226 = getelementptr inbounds nuw i64, ptr %160, i64 %220
  %227 = load i64, ptr %226, align 8, !tbaa !11
  %228 = add nsw i64 %227, 1
  store i64 %228, ptr %226, align 8, !tbaa !11
  %229 = getelementptr inbounds i32, ptr %158, i64 %227
  store i32 %215, ptr %229, align 4, !tbaa !22
  %230 = getelementptr inbounds nuw i64, ptr %160, i64 %223
  %231 = load i64, ptr %230, align 8, !tbaa !11
  %232 = add nsw i64 %231, 1
  store i64 %232, ptr %230, align 8, !tbaa !11
  %233 = getelementptr inbounds i32, ptr %158, i64 %231
  store i32 %212, ptr %233, align 4, !tbaa !22
  br label %234

234:                                              ; preds = %208, %219, %222, %225, %203
  %235 = add nuw nsw i64 %204, 1
  %236 = icmp eq i64 %235, %164
  br i1 %236, label %201, label %203, !llvm.loop !109
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @autograph_get_layout(ptr noundef readnone %0) local_unnamed_addr #12 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %17

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %17, label %9, !llvm.loop !38

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 8, !tbaa !39
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !50
  br label %17

17:                                               ; preds = %6, %1, %14
  %18 = phi i32 [ %16, %14 ], [ 3, %1 ], [ 3, %6 ]
  ret i32 %18
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_debug_print(ptr nocapture noundef readnone %0) local_unnamed_addr #0 {
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_get_meta_handles(ptr noundef %0, ptr noundef writeonly %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #1 {
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %51

7:                                                ; preds = %4
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %23, label %12, !llvm.loop !38

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 8, !tbaa !39
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %19 = load i32, ptr %18, align 8, !tbaa !46
  %20 = icmp eq i32 %19, 0
  br i1 %20, label %23, label %21

21:                                               ; preds = %17
  tail call void @autograph_ensure_layout_set(ptr noundef %0)
  %22 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  br label %23

23:                                               ; preds = %9, %17, %21
  %24 = phi i32 [ %5, %17 ], [ %22, %21 ], [ %5, %9 ]
  %25 = icmp sgt i32 %24, 0
  br i1 %25, label %26, label %51

26:                                               ; preds = %23
  %27 = zext nneg i32 %24 to i64
  br label %31

28:                                               ; preds = %31
  %29 = add nuw nsw i64 %32, 1
  %30 = icmp eq i64 %29, %27
  br i1 %30, label %51, label %31, !llvm.loop !38

31:                                               ; preds = %28, %26
  %32 = phi i64 [ 0, %26 ], [ %29, %28 ]
  %33 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %32
  %34 = load ptr, ptr %33, align 8, !tbaa !39
  %35 = icmp eq ptr %34, %0
  br i1 %35, label %36, label %28

36:                                               ; preds = %31
  %37 = icmp eq ptr %1, null
  br i1 %37, label %41, label %38

38:                                               ; preds = %36
  %39 = getelementptr inbounds nuw i8, ptr %33, i64 16
  %40 = load ptr, ptr %39, align 8, !tbaa !47
  store ptr %40, ptr %1, align 8, !tbaa !103
  br label %41

41:                                               ; preds = %38, %36
  %42 = icmp eq ptr %2, null
  br i1 %42, label %46, label %43

43:                                               ; preds = %41
  %44 = getelementptr inbounds nuw i8, ptr %33, i64 24
  %45 = load ptr, ptr %44, align 8, !tbaa !48
  store ptr %45, ptr %2, align 8, !tbaa !103
  br label %46

46:                                               ; preds = %43, %41
  %47 = icmp eq ptr %3, null
  br i1 %47, label %51, label %48

48:                                               ; preds = %46
  %49 = getelementptr inbounds nuw i8, ptr %33, i64 32
  %50 = load ptr, ptr %49, align 8, !tbaa !49
  store ptr %50, ptr %3, align 8, !tbaa !103
  br label %51

51:                                               ; preds = %28, %4, %23, %46, %48
  %52 = phi i32 [ 1, %48 ], [ 1, %46 ], [ 0, %23 ], [ 0, %4 ], [ 0, %28 ]
  ret i32 %52
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #1 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %26

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %26, label %10, !llvm.loop !38

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 8, !tbaa !39
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !47
  %18 = icmp eq ptr %17, null
  br i1 %18, label %26, label %19

19:                                               ; preds = %15
  tail call void @roaring_bitmap_add(ptr noundef nonnull %17, i32 noundef %1) #19
  %20 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %21 = load i32, ptr %20, align 8, !tbaa !50
  %22 = icmp eq i32 %21, 3
  br i1 %22, label %23, label %24

23:                                               ; preds = %19
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %12)
  br label %24

24:                                               ; preds = %23, %19
  %25 = getelementptr inbounds nuw i8, ptr %12, i64 80
  store i32 0, ptr %25, align 8, !tbaa !46
  br label %26

26:                                               ; preds = %7, %2, %15, %24
  %27 = phi i32 [ 1, %24 ], [ 0, %15 ], [ 0, %2 ], [ 0, %7 ]
  ret i32 %27
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #1 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %99

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %99, label %10, !llvm.loop !38

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 8, !tbaa !39
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !47
  %18 = icmp eq ptr %17, null
  br i1 %18, label %99, label %19

19:                                               ; preds = %15
  %20 = getelementptr inbounds nuw i8, ptr %12, i64 24
  %21 = load ptr, ptr %20, align 8, !tbaa !48
  %22 = icmp eq ptr %21, null
  br i1 %22, label %99, label %23

23:                                               ; preds = %19
  tail call void @roaring_bitmap_remove(ptr noundef nonnull %17, i32 noundef %1) #19
  %24 = getelementptr inbounds nuw i8, ptr %12, i64 32
  %25 = load ptr, ptr %24, align 8, !tbaa !49
  %26 = load ptr, ptr %20, align 8, !tbaa !48
  %27 = getelementptr inbounds nuw i8, ptr %12, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !63
  %29 = icmp sgt i64 %28, 0
  br i1 %29, label %38, label %30

30:                                               ; preds = %53, %23
  %31 = phi i64 [ 0, %23 ], [ %54, %53 ]
  %32 = getelementptr inbounds nuw i8, ptr %12, i64 64
  %33 = load i64, ptr %32, align 8, !tbaa !60
  %34 = icmp sgt i64 %33, 0
  br i1 %34, label %35, label %58

35:                                               ; preds = %30
  %36 = getelementptr inbounds nuw i8, ptr %12, i64 56
  %37 = getelementptr inbounds nuw i8, ptr %12, i64 48
  br label %63

38:                                               ; preds = %23, %53
  %39 = phi i64 [ %55, %53 ], [ 0, %23 ]
  %40 = phi i64 [ %54, %53 ], [ 0, %23 ]
  %41 = trunc i64 %39 to i32
  %42 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %26, i32 noundef %41) #19
  br i1 %42, label %43, label %53

43:                                               ; preds = %38
  %44 = getelementptr inbounds nuw %struct.EdgePair, ptr %25, i64 %39
  %45 = load i32, ptr %44, align 4, !tbaa !3
  %46 = icmp eq i32 %45, %1
  br i1 %46, label %51, label %47

47:                                               ; preds = %43
  %48 = getelementptr inbounds nuw i8, ptr %44, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !8
  %50 = icmp eq i32 %49, %1
  br i1 %50, label %51, label %53

51:                                               ; preds = %47, %43
  tail call void @roaring_bitmap_remove(ptr noundef %26, i32 noundef %41) #19
  %52 = add nsw i64 %40, 1
  br label %53

53:                                               ; preds = %47, %51, %38
  %54 = phi i64 [ %52, %51 ], [ %40, %47 ], [ %40, %38 ]
  %55 = add nuw nsw i64 %39, 1
  %56 = load i64, ptr %27, align 8, !tbaa !63
  %57 = icmp slt i64 %55, %56
  br i1 %57, label %38, label %30, !llvm.loop !110

58:                                               ; preds = %85, %30
  %59 = phi i64 [ %31, %30 ], [ %87, %85 ]
  %60 = getelementptr inbounds nuw i8, ptr %12, i64 88
  %61 = load i64, ptr %60, align 8, !tbaa !58
  %62 = icmp sgt i64 %61, -1
  br i1 %62, label %90, label %92

63:                                               ; preds = %35, %85
  %64 = phi i64 [ %33, %35 ], [ %86, %85 ]
  %65 = phi i64 [ 0, %35 ], [ %88, %85 ]
  %66 = phi i64 [ %31, %35 ], [ %87, %85 ]
  %67 = load ptr, ptr %36, align 8, !tbaa !64
  %68 = getelementptr inbounds nuw i8, ptr %67, i64 %65
  %69 = load i8, ptr %68, align 1, !tbaa !65
  %70 = icmp eq i8 %69, 0
  br i1 %70, label %85, label %71

71:                                               ; preds = %63
  %72 = load ptr, ptr %37, align 8, !tbaa !69
  %73 = shl nuw nsw i64 %65, 1
  %74 = getelementptr inbounds nuw i32, ptr %72, i64 %73
  %75 = load i32, ptr %74, align 4, !tbaa !22
  %76 = icmp eq i32 %75, %1
  br i1 %76, label %82, label %77

77:                                               ; preds = %71
  %78 = or disjoint i64 %73, 1
  %79 = getelementptr inbounds nuw i32, ptr %72, i64 %78
  %80 = load i32, ptr %79, align 4, !tbaa !22
  %81 = icmp eq i32 %80, %1
  br i1 %81, label %82, label %85

82:                                               ; preds = %77, %71
  store i8 0, ptr %68, align 1, !tbaa !65
  %83 = add nsw i64 %66, 1
  %84 = load i64, ptr %32, align 8, !tbaa !60
  br label %85

85:                                               ; preds = %77, %82, %63
  %86 = phi i64 [ %64, %63 ], [ %84, %82 ], [ %64, %77 ]
  %87 = phi i64 [ %66, %63 ], [ %83, %82 ], [ %66, %77 ]
  %88 = add nuw nsw i64 %65, 1
  %89 = icmp slt i64 %88, %86
  br i1 %89, label %63, label %58, !llvm.loop !111

90:                                               ; preds = %58
  %91 = sub nsw i64 %61, %59
  store i64 %91, ptr %60, align 8, !tbaa !58
  br label %92

92:                                               ; preds = %90, %58
  %93 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %94 = load i32, ptr %93, align 8, !tbaa !50
  %95 = icmp eq i32 %94, 3
  br i1 %95, label %96, label %97

96:                                               ; preds = %92
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %12)
  br label %97

97:                                               ; preds = %96, %92
  %98 = getelementptr inbounds nuw i8, ptr %12, i64 80
  store i32 0, ptr %98, align 8, !tbaa !46
  br label %99

99:                                               ; preds = %7, %2, %15, %19, %97
  %100 = phi i32 [ 1, %97 ], [ 0, %19 ], [ 0, %15 ], [ 0, %2 ], [ 0, %7 ]
  ret i32 %100
}

declare dso_local void @roaring_bitmap_remove(ptr noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #1 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %120

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %120, label %11, !llvm.loop !38

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !39
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 16
  %18 = load ptr, ptr %17, align 8, !tbaa !47
  %19 = icmp eq ptr %18, null
  br i1 %19, label %120, label %20

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %22 = load ptr, ptr %21, align 8, !tbaa !48
  %23 = icmp eq ptr %22, null
  br i1 %23, label %120, label %24

24:                                               ; preds = %20
  tail call void @roaring_bitmap_add(ptr noundef nonnull %18, i32 noundef %1) #19
  %25 = load ptr, ptr %17, align 8, !tbaa !47
  tail call void @roaring_bitmap_add(ptr noundef %25, i32 noundef %2) #19
  %26 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %27 = icmp sgt i32 %26, -1
  br i1 %27, label %28, label %41

28:                                               ; preds = %24
  %29 = load ptr, ptr %21, align 8, !tbaa !48
  %30 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %29, i32 noundef %26) #19
  tail call void @roaring_bitmap_add(ptr noundef %29, i32 noundef %26) #19
  br i1 %30, label %37, label %31

31:                                               ; preds = %28
  %32 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %33 = load i64, ptr %32, align 8, !tbaa !58
  %34 = icmp sgt i64 %33, -1
  br i1 %34, label %35, label %37

35:                                               ; preds = %31
  %36 = add nuw nsw i64 %33, 1
  store i64 %36, ptr %32, align 8, !tbaa !58
  br label %37

37:                                               ; preds = %35, %31, %28
  %38 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %39 = load i32, ptr %38, align 8, !tbaa !50
  %40 = icmp eq i32 %39, 3
  br i1 %40, label %117, label %118

41:                                               ; preds = %24
  %42 = tail call fastcc i64 @canonical_pair_find_extra(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %43 = icmp sgt i64 %42, -1
  br i1 %43, label %44, label %60

44:                                               ; preds = %41
  %45 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %46 = load ptr, ptr %45, align 8, !tbaa !64
  %47 = getelementptr inbounds nuw i8, ptr %46, i64 %42
  %48 = load i8, ptr %47, align 1, !tbaa !65
  %49 = icmp eq i8 %48, 0
  br i1 %49, label %50, label %56

50:                                               ; preds = %44
  %51 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %52 = load i64, ptr %51, align 8, !tbaa !58
  %53 = icmp sgt i64 %52, -1
  br i1 %53, label %54, label %56

54:                                               ; preds = %50
  %55 = add nuw nsw i64 %52, 1
  store i64 %55, ptr %51, align 8, !tbaa !58
  br label %56

56:                                               ; preds = %54, %50, %44
  store i8 1, ptr %47, align 1, !tbaa !65
  %57 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %58 = load i32, ptr %57, align 8, !tbaa !50
  %59 = icmp eq i32 %58, 3
  br i1 %59, label %117, label %118

60:                                               ; preds = %41
  %61 = getelementptr inbounds nuw i8, ptr %13, i64 64
  %62 = load i64, ptr %61, align 8, !tbaa !60
  %63 = add nsw i64 %62, 1
  %64 = tail call fastcc i32 @ensure_extra_capacity(ptr noundef %13, i64 noundef %63)
  %65 = icmp eq i32 %64, 0
  br i1 %65, label %120, label %66

66:                                               ; preds = %60
  %67 = load i64, ptr %61, align 8, !tbaa !60
  %68 = add nsw i64 %67, 1
  store i64 %68, ptr %61, align 8, !tbaa !60
  %69 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %70 = load ptr, ptr %69, align 8, !tbaa !69
  %71 = shl nsw i64 %67, 1
  %72 = getelementptr inbounds i32, ptr %70, i64 %71
  store i32 %1, ptr %72, align 4, !tbaa !22
  %73 = or disjoint i64 %71, 1
  %74 = getelementptr inbounds i32, ptr %70, i64 %73
  store i32 %2, ptr %74, align 4, !tbaa !22
  %75 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %76 = load ptr, ptr %75, align 8, !tbaa !64
  %77 = getelementptr inbounds i8, ptr %76, i64 %67
  store i8 1, ptr %77, align 1, !tbaa !65
  %78 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %79 = icmp slt i32 %78, 1
  %80 = zext nneg i32 %78 to i64
  %81 = add nsw i64 %80, -1
  %82 = icmp ugt i64 %12, %81
  %83 = select i1 %79, i1 true, i1 %82
  br i1 %83, label %107, label %84

84:                                               ; preds = %66
  %85 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %12
  %86 = load ptr, ptr %85, align 8, !tbaa !61
  %87 = icmp eq ptr %86, null
  br i1 %87, label %88, label %105

88:                                               ; preds = %84
  %89 = getelementptr inbounds nuw i8, ptr %13, i64 72
  %90 = load i64, ptr %89, align 8, !tbaa !59
  %91 = icmp sgt i64 %90, 0
  %92 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #20
  %93 = trunc i64 %90 to i32
  %94 = shl i32 %93, 2
  %95 = add i32 %94, 32
  %96 = select i1 %91, i32 %95, i32 128
  br label %97

97:                                               ; preds = %97, %88
  %98 = phi i32 [ 1, %88 ], [ %100, %97 ]
  %99 = icmp sgt i32 %98, %96
  %100 = shl i32 %98, 1
  br i1 %99, label %101, label %97, !llvm.loop !24

101:                                              ; preds = %97
  %102 = sext i32 %98 to i64
  %103 = getelementptr inbounds nuw i8, ptr %92, i64 8
  store i64 %102, ptr %103, align 8, !tbaa !34
  %104 = tail call noalias ptr @calloc(i64 noundef %102, i64 noundef 24) #18
  store ptr %104, ptr %92, align 8, !tbaa !37
  store ptr %92, ptr %85, align 8, !tbaa !61
  br label %105

105:                                              ; preds = %101, %84
  %106 = phi ptr [ %92, %101 ], [ %86, %84 ]
  tail call fastcc void @edge_hash_insert(ptr noundef nonnull %106, i32 noundef %1, i32 noundef %2, i64 noundef %67)
  br label %107

107:                                              ; preds = %66, %105
  %108 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %109 = load i64, ptr %108, align 8, !tbaa !58
  %110 = icmp sgt i64 %109, -1
  br i1 %110, label %111, label %113

111:                                              ; preds = %107
  %112 = add nuw nsw i64 %109, 1
  store i64 %112, ptr %108, align 8, !tbaa !58
  br label %113

113:                                              ; preds = %111, %107
  %114 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %115 = load i32, ptr %114, align 8, !tbaa !50
  %116 = icmp eq i32 %115, 3
  br i1 %116, label %117, label %118

117:                                              ; preds = %113, %56, %37
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %13)
  br label %118

118:                                              ; preds = %117, %113, %56, %37
  %119 = getelementptr inbounds nuw i8, ptr %13, i64 80
  store i32 0, ptr %119, align 8, !tbaa !46
  br label %120

120:                                              ; preds = %8, %118, %3, %60, %16, %20
  %121 = phi i32 [ 0, %20 ], [ 0, %16 ], [ 0, %60 ], [ 0, %3 ], [ 1, %118 ], [ 0, %8 ]
  ret i32 %121
}

; Function Attrs: nofree nounwind memory(readwrite, argmem: read) uwtable
define internal fastcc i32 @canonical_pair_find_static(ptr noundef nonnull readonly %0, i32 noundef %1, i32 noundef %2) unnamed_addr #13 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %5 = load ptr, ptr %4, align 8, !tbaa !49
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
  br i1 %18, label %136, label %12, !llvm.loop !112

19:                                               ; preds = %12
  %20 = and i64 %13, 4294967295
  %21 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %20
  %22 = load ptr, ptr %21, align 8, !tbaa !61
  %23 = icmp eq ptr %22, null
  br i1 %23, label %26, label %24

24:                                               ; preds = %19
  %25 = load ptr, ptr %22, align 8, !tbaa !37
  br label %91

26:                                               ; preds = %19
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !63
  %29 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #20
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
  store i64 %38, ptr %39, align 8, !tbaa !34
  %40 = tail call noalias ptr @calloc(i64 noundef %38, i64 noundef 24) #18
  store ptr %40, ptr %29, align 8, !tbaa !37
  store ptr %29, ptr %21, align 8, !tbaa !61
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
  br i1 %90, label %91, label %44, !llvm.loop !113

91:                                               ; preds = %88, %24, %37
  %92 = phi ptr [ %40, %37 ], [ %25, %24 ], [ %40, %88 ]
  %93 = phi ptr [ %29, %37 ], [ %22, %24 ], [ %29, %88 ]
  %94 = icmp eq ptr %92, null
  br i1 %94, label %136, label %95

95:                                               ; preds = %91
  %96 = getelementptr inbounds nuw i8, ptr %93, i64 8
  %97 = load i64, ptr %96, align 8, !tbaa !34
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

; Function Attrs: nofree nounwind memory(readwrite, argmem: read) uwtable
define internal fastcc i64 @canonical_pair_find_extra(ptr noundef nonnull readonly %0, i32 noundef %1, i32 noundef %2) unnamed_addr #13 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %5 = load i64, ptr %4, align 8, !tbaa !60
  %6 = icmp slt i64 %5, 1
  br i1 %6, label %87, label %7

7:                                                ; preds = %3
  %8 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %9 = icmp sgt i32 %8, 0
  br i1 %9, label %10, label %87

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
  br i1 %18, label %87, label %12, !llvm.loop !112

19:                                               ; preds = %12
  %20 = and i64 %13, 4294967295
  %21 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %20
  %22 = load ptr, ptr %21, align 8, !tbaa !61
  %23 = icmp eq ptr %22, null
  br i1 %23, label %24, label %41

24:                                               ; preds = %19
  %25 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %26 = load i64, ptr %25, align 8, !tbaa !59
  %27 = icmp sgt i64 %26, 0
  %28 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #20
  %29 = trunc i64 %26 to i32
  %30 = shl i32 %29, 2
  %31 = add i32 %30, 32
  %32 = select i1 %27, i32 %31, i32 128
  br label %33

33:                                               ; preds = %33, %24
  %34 = phi i32 [ 1, %24 ], [ %36, %33 ]
  %35 = icmp sgt i32 %34, %32
  %36 = shl i32 %34, 1
  br i1 %35, label %37, label %33, !llvm.loop !24

37:                                               ; preds = %33
  %38 = sext i32 %34 to i64
  %39 = getelementptr inbounds nuw i8, ptr %28, i64 8
  store i64 %38, ptr %39, align 8, !tbaa !34
  %40 = tail call noalias ptr @calloc(i64 noundef %38, i64 noundef 24) #18
  store ptr %40, ptr %28, align 8, !tbaa !37
  store ptr %28, ptr %21, align 8, !tbaa !61
  br label %43

41:                                               ; preds = %19
  %42 = load ptr, ptr %22, align 8, !tbaa !37
  br label %43

43:                                               ; preds = %41, %37
  %44 = phi ptr [ %42, %41 ], [ %40, %37 ]
  %45 = phi ptr [ %22, %41 ], [ %28, %37 ]
  %46 = icmp eq ptr %44, null
  br i1 %46, label %87, label %47

47:                                               ; preds = %43
  %48 = getelementptr inbounds nuw i8, ptr %45, i64 8
  %49 = load i64, ptr %48, align 8, !tbaa !34
  %50 = icmp slt i64 %49, 1
  br i1 %50, label %87, label %51

51:                                               ; preds = %47
  %52 = tail call i32 @llvm.smin.i32(i32 %1, i32 %2)
  %53 = tail call i32 @llvm.smax.i32(i32 %1, i32 %2)
  %54 = zext i32 %52 to i64
  %55 = shl nuw i64 %54, 32
  %56 = zext i32 %53 to i64
  %57 = or disjoint i64 %55, %56
  %58 = lshr i64 %57, 30
  %59 = xor i64 %58, %57
  %60 = mul i64 %59, -4658895280553007687
  %61 = lshr i64 %60, 27
  %62 = xor i64 %61, %60
  %63 = mul i64 %62, -7723592293110705685
  %64 = lshr i64 %63, 31
  %65 = xor i64 %64, %63
  %66 = add nsw i64 %49, -1
  %67 = and i64 %66, %65
  %68 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %44, i64 %67
  %69 = getelementptr inbounds nuw i8, ptr %68, i64 16
  %70 = load i8, ptr %69, align 8, !tbaa !25
  %71 = icmp eq i8 %70, 0
  br i1 %71, label %87, label %79

72:                                               ; preds = %79
  %73 = add i64 %81, 1
  %74 = and i64 %73, %66
  %75 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %44, i64 %74
  %76 = getelementptr inbounds nuw i8, ptr %75, i64 16
  %77 = load i8, ptr %76, align 8, !tbaa !25
  %78 = icmp eq i8 %77, 0
  br i1 %78, label %87, label %79, !llvm.loop !32

79:                                               ; preds = %51, %72
  %80 = phi ptr [ %75, %72 ], [ %68, %51 ]
  %81 = phi i64 [ %74, %72 ], [ %67, %51 ]
  %82 = load i64, ptr %80, align 8, !tbaa !27
  %83 = icmp eq i64 %82, %57
  br i1 %83, label %84, label %72

84:                                               ; preds = %79
  %85 = getelementptr inbounds nuw i8, ptr %80, i64 8
  %86 = load i64, ptr %85, align 8, !tbaa !28
  br label %87

87:                                               ; preds = %16, %72, %7, %84, %51, %47, %43, %3
  %88 = phi i64 [ -1, %3 ], [ -1, %47 ], [ -1, %43 ], [ %86, %84 ], [ -1, %51 ], [ -1, %7 ], [ -1, %72 ], [ -1, %16 ]
  ret i64 %88
}

; Function Attrs: nounwind uwtable
define internal fastcc range(i32 0, 2) i32 @ensure_extra_capacity(ptr noundef nonnull %0, i64 noundef range(i64 -9223372036854775807, -9223372036854775808) %1) unnamed_addr #1 {
  %3 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %4 = load i64, ptr %3, align 8, !tbaa !59
  %5 = icmp sgt i64 %1, %4
  br i1 %5, label %6, label %116

6:                                                ; preds = %2
  %7 = icmp sgt i64 %4, 0
  %8 = select i1 %7, i64 %4, i64 16
  br label %9

9:                                                ; preds = %9, %6
  %10 = phi i64 [ %8, %6 ], [ %12, %9 ]
  %11 = icmp slt i64 %10, %1
  %12 = shl nsw i64 %10, 1
  br i1 %11, label %9, label %13, !llvm.loop !114

13:                                               ; preds = %9
  %14 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %15 = load ptr, ptr %14, align 8, !tbaa !69
  %16 = shl i64 %10, 3
  %17 = tail call ptr @realloc(ptr noundef %15, i64 noundef %16) #21
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %19 = load ptr, ptr %18, align 8, !tbaa !64
  %20 = tail call ptr @realloc(ptr noundef %19, i64 noundef %10) #21
  %21 = icmp ne ptr %17, null
  %22 = icmp ne ptr %20, null
  %23 = and i1 %21, %22
  br i1 %23, label %24, label %116

24:                                               ; preds = %13
  %25 = load i64, ptr %3, align 8, !tbaa !59
  %26 = icmp sgt i64 %10, %25
  br i1 %26, label %27, label %30

27:                                               ; preds = %24
  %28 = getelementptr inbounds i8, ptr %20, i64 %25
  %29 = sub nsw i64 %10, %25
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %28, i8 0, i64 %29, i1 false)
  br label %30

30:                                               ; preds = %27, %24
  store ptr %17, ptr %14, align 8, !tbaa !69
  store ptr %20, ptr %18, align 8, !tbaa !64
  store i64 %10, ptr %3, align 8, !tbaa !59
  %31 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %32 = icmp sgt i32 %31, 0
  br i1 %32, label %33, label %116

33:                                               ; preds = %30
  %34 = zext nneg i32 %31 to i64
  br label %35

35:                                               ; preds = %39, %33
  %36 = phi i64 [ 0, %33 ], [ %40, %39 ]
  %37 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %36
  %38 = icmp eq ptr %37, %0
  br i1 %38, label %42, label %39

39:                                               ; preds = %35
  %40 = add nuw nsw i64 %36, 1
  %41 = icmp eq i64 %40, %34
  br i1 %41, label %116, label %35, !llvm.loop !112

42:                                               ; preds = %35
  %43 = and i64 %36, 4294967295
  %44 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %43
  %45 = load ptr, ptr %44, align 8, !tbaa !61
  %46 = icmp eq ptr %45, null
  br i1 %46, label %116, label %47

47:                                               ; preds = %42
  %48 = load ptr, ptr %45, align 8, !tbaa !37
  tail call void @free(ptr noundef %48) #19
  tail call void @free(ptr noundef nonnull %45) #19
  %49 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #20
  %50 = trunc i64 %10 to i32
  %51 = shl i32 %50, 2
  %52 = add i32 %51, 32
  br label %53

53:                                               ; preds = %53, %47
  %54 = phi i32 [ 1, %47 ], [ %56, %53 ]
  %55 = icmp sgt i32 %54, %52
  %56 = shl i32 %54, 1
  br i1 %55, label %57, label %53, !llvm.loop !24

57:                                               ; preds = %53
  %58 = sext i32 %54 to i64
  %59 = getelementptr inbounds nuw i8, ptr %49, i64 8
  store i64 %58, ptr %59, align 8, !tbaa !34
  %60 = tail call noalias ptr @calloc(i64 noundef %58, i64 noundef 24) #18
  store ptr %60, ptr %49, align 8, !tbaa !37
  store ptr %49, ptr %44, align 8, !tbaa !61
  %61 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %62 = load i64, ptr %61, align 8, !tbaa !60
  %63 = icmp sgt i64 %62, 0
  br i1 %63, label %64, label %116

64:                                               ; preds = %57
  %65 = load ptr, ptr %14, align 8, !tbaa !69
  %66 = add nsw i64 %58, -1
  br label %67

67:                                               ; preds = %64, %113
  %68 = phi i64 [ 0, %64 ], [ %114, %113 ]
  %69 = shl nuw nsw i64 %68, 1
  %70 = getelementptr inbounds nuw i32, ptr %65, i64 %69
  %71 = load i32, ptr %70, align 4, !tbaa !22
  %72 = or disjoint i64 %69, 1
  %73 = getelementptr inbounds nuw i32, ptr %65, i64 %72
  %74 = load i32, ptr %73, align 4, !tbaa !22
  %75 = tail call i32 @llvm.smin.i32(i32 %71, i32 %74)
  %76 = tail call i32 @llvm.smax.i32(i32 %71, i32 %74)
  %77 = zext i32 %75 to i64
  %78 = shl nuw i64 %77, 32
  %79 = zext i32 %76 to i64
  %80 = or disjoint i64 %78, %79
  %81 = lshr i64 %80, 30
  %82 = xor i64 %81, %80
  %83 = mul i64 %82, -4658895280553007687
  %84 = lshr i64 %83, 27
  %85 = xor i64 %84, %83
  %86 = mul i64 %85, -7723592293110705685
  %87 = lshr i64 %86, 31
  %88 = xor i64 %87, %86
  %89 = and i64 %88, %66
  %90 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %89
  %91 = getelementptr inbounds nuw i8, ptr %90, i64 16
  %92 = load i8, ptr %91, align 8, !tbaa !25
  %93 = icmp eq i8 %92, 0
  br i1 %93, label %108, label %94

94:                                               ; preds = %67, %101
  %95 = phi ptr [ %104, %101 ], [ %90, %67 ]
  %96 = phi i64 [ %103, %101 ], [ %89, %67 ]
  %97 = load i64, ptr %95, align 8, !tbaa !27
  %98 = icmp eq i64 %97, %80
  br i1 %98, label %99, label %101

99:                                               ; preds = %94
  %100 = getelementptr inbounds nuw i8, ptr %95, i64 8
  store i64 %68, ptr %100, align 8, !tbaa !28
  br label %113

101:                                              ; preds = %94
  %102 = add i64 %96, 1
  %103 = and i64 %102, %66
  %104 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %103
  %105 = getelementptr inbounds nuw i8, ptr %104, i64 16
  %106 = load i8, ptr %105, align 8, !tbaa !25
  %107 = icmp eq i8 %106, 0
  br i1 %107, label %108, label %94, !llvm.loop !29

108:                                              ; preds = %101, %67
  %109 = phi i64 [ %89, %67 ], [ %103, %101 ]
  %110 = phi ptr [ %90, %67 ], [ %104, %101 ]
  store i64 %80, ptr %110, align 8, !tbaa !27
  %111 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %109, i32 1
  store i64 %68, ptr %111, align 8, !tbaa !28
  %112 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %109, i32 2
  store i8 1, ptr %112, align 8, !tbaa !25
  br label %113

113:                                              ; preds = %99, %108
  %114 = add nuw nsw i64 %68, 1
  %115 = icmp eq i64 %114, %62
  br i1 %115, label %116, label %67, !llvm.loop !115

116:                                              ; preds = %39, %113, %57, %30, %13, %42, %2
  %117 = phi i32 [ 1, %2 ], [ 0, %13 ], [ 1, %42 ], [ 1, %30 ], [ 1, %57 ], [ 1, %113 ], [ 1, %39 ]
  ret i32 %117
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #1 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %62

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %62, label %11, !llvm.loop !38

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !39
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %18 = load ptr, ptr %17, align 8, !tbaa !48
  %19 = icmp eq ptr %18, null
  br i1 %19, label %62, label %20

20:                                               ; preds = %16
  %21 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %22 = icmp sgt i32 %21, -1
  br i1 %22, label %23, label %36

23:                                               ; preds = %20
  %24 = load ptr, ptr %17, align 8, !tbaa !48
  %25 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %24, i32 noundef %21) #19
  tail call void @roaring_bitmap_remove(ptr noundef %24, i32 noundef %21) #19
  br i1 %25, label %26, label %32

26:                                               ; preds = %23
  %27 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %28 = load i64, ptr %27, align 8, !tbaa !58
  %29 = icmp sgt i64 %28, -1
  br i1 %29, label %30, label %32

30:                                               ; preds = %26
  %31 = add nsw i64 %28, -1
  store i64 %31, ptr %27, align 8, !tbaa !58
  br label %32

32:                                               ; preds = %30, %26, %23
  %33 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %34 = load i32, ptr %33, align 8, !tbaa !50
  %35 = icmp eq i32 %34, 3
  br i1 %35, label %59, label %60

36:                                               ; preds = %20
  %37 = tail call fastcc i64 @canonical_pair_find_extra(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %38 = icmp sgt i64 %37, -1
  br i1 %38, label %39, label %55

39:                                               ; preds = %36
  %40 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %41 = load ptr, ptr %40, align 8, !tbaa !64
  %42 = getelementptr inbounds nuw i8, ptr %41, i64 %37
  %43 = load i8, ptr %42, align 1, !tbaa !65
  %44 = icmp eq i8 %43, 0
  br i1 %44, label %51, label %45

45:                                               ; preds = %39
  %46 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %47 = load i64, ptr %46, align 8, !tbaa !58
  %48 = icmp sgt i64 %47, -1
  br i1 %48, label %49, label %51

49:                                               ; preds = %45
  %50 = add nsw i64 %47, -1
  store i64 %50, ptr %46, align 8, !tbaa !58
  br label %51

51:                                               ; preds = %49, %45, %39
  store i8 0, ptr %42, align 1, !tbaa !65
  %52 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %53 = load i32, ptr %52, align 8, !tbaa !50
  %54 = icmp eq i32 %53, 3
  br i1 %54, label %59, label %60

55:                                               ; preds = %36
  %56 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %57 = load i32, ptr %56, align 8, !tbaa !50
  %58 = icmp eq i32 %57, 3
  br i1 %58, label %59, label %60

59:                                               ; preds = %55, %51, %32
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %13)
  br label %60

60:                                               ; preds = %59, %55, %51, %32
  %61 = getelementptr inbounds nuw i8, ptr %13, i64 80
  store i32 0, ptr %61, align 8, !tbaa !46
  br label %62

62:                                               ; preds = %8, %60, %3, %16
  %63 = phi i32 [ 0, %16 ], [ 0, %3 ], [ 1, %60 ], [ 0, %8 ]
  ret i32 %63
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_bcsr_add_edge(ptr noundef writeonly %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #1 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %137

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %137, label %11, !llvm.loop !38

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !39
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %18 = load i32, ptr %17, align 8, !tbaa !50
  %19 = icmp eq i32 %18, 2
  br i1 %19, label %20, label %137

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 160
  %22 = load ptr, ptr %21, align 8, !tbaa !56
  %23 = icmp eq ptr %22, null
  br i1 %23, label %137, label %24

24:                                               ; preds = %20
  %25 = getelementptr inbounds nuw i8, ptr %13, i64 168
  %26 = load ptr, ptr %25, align 8, !tbaa !57
  %27 = icmp eq ptr %26, null
  br i1 %27, label %137, label %28

28:                                               ; preds = %24
  %29 = getelementptr inbounds nuw i8, ptr %13, i64 176
  %30 = load i32, ptr %29, align 8, !tbaa !55
  %31 = icmp sgt i32 %30, 0
  %32 = or i32 %2, %1
  %33 = icmp sgt i32 %32, -1
  %34 = and i1 %33, %31
  br i1 %34, label %35, label %137

35:                                               ; preds = %28
  %36 = getelementptr inbounds nuw i8, ptr %13, i64 180
  %37 = load i32, ptr %36, align 4, !tbaa !54
  %38 = udiv i32 %1, %30
  %39 = urem i32 %1, %30
  %40 = icmp slt i32 %38, %37
  br i1 %40, label %41, label %137

41:                                               ; preds = %35
  %42 = zext nneg i32 %38 to i64
  %43 = getelementptr inbounds nuw i32, ptr %22, i64 %42
  %44 = load i32, ptr %43, align 4, !tbaa !22
  %45 = add nuw nsw i32 %38, 1
  %46 = zext nneg i32 %45 to i64
  %47 = getelementptr inbounds nuw i32, ptr %22, i64 %46
  %48 = load i32, ptr %47, align 4, !tbaa !22
  %49 = icmp slt i32 %44, %48
  br i1 %49, label %50, label %78

50:                                               ; preds = %41
  %51 = sext i32 %44 to i64
  %52 = sext i32 %48 to i64
  br label %56

53:                                               ; preds = %65
  %54 = sext i32 %44 to i64
  %55 = sext i32 %48 to i64
  br label %68

56:                                               ; preds = %50, %65
  %57 = phi i64 [ %51, %50 ], [ %66, %65 ]
  %58 = getelementptr inbounds i32, ptr %26, i64 %57
  %59 = load i32, ptr %58, align 4, !tbaa !22
  %60 = icmp eq i32 %59, %39
  br i1 %60, label %61, label %65

61:                                               ; preds = %56
  %62 = getelementptr i8, ptr %58, i64 4
  %63 = load i32, ptr %62, align 4, !tbaa !22
  %64 = icmp eq i32 %63, %2
  br i1 %64, label %137, label %65

65:                                               ; preds = %56, %61
  %66 = add nsw i64 %57, 2
  %67 = icmp slt i64 %66, %52
  br i1 %67, label %56, label %53, !llvm.loop !116

68:                                               ; preds = %53, %73
  %69 = phi i64 [ %54, %53 ], [ %74, %73 ]
  %70 = getelementptr inbounds i32, ptr %26, i64 %69
  %71 = load i32, ptr %70, align 4, !tbaa !22
  %72 = icmp sgt i32 %71, %39
  br i1 %72, label %76, label %73

73:                                               ; preds = %68
  %74 = add nsw i64 %69, 2
  %75 = icmp slt i64 %74, %55
  br i1 %75, label %68, label %78, !llvm.loop !117

76:                                               ; preds = %68
  %77 = trunc nsw i64 %69 to i32
  br label %78

78:                                               ; preds = %73, %76, %41
  %79 = phi i32 [ %48, %41 ], [ %77, %76 ], [ %48, %73 ]
  %80 = zext nneg i32 %37 to i64
  %81 = getelementptr inbounds nuw i32, ptr %22, i64 %80
  %82 = load i32, ptr %81, align 4, !tbaa !22
  %83 = add nsw i32 %82, 2
  %84 = sext i32 %83 to i64
  %85 = shl nsw i64 %84, 2
  %86 = tail call ptr @realloc(ptr noundef nonnull %26, i64 noundef %85) #21
  %87 = icmp eq ptr %86, null
  br i1 %87, label %137, label %88

88:                                               ; preds = %78
  %89 = icmp sgt i32 %82, %79
  %90 = sext i32 %79 to i64
  br i1 %89, label %91, label %97

91:                                               ; preds = %88
  %92 = getelementptr i32, ptr %86, i64 %90
  %93 = getelementptr i8, ptr %92, i64 8
  %94 = sub nsw i32 %82, %79
  %95 = sext i32 %94 to i64
  %96 = shl nsw i64 %95, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %93, ptr nonnull align 4 %92, i64 %96, i1 false)
  br label %97

97:                                               ; preds = %88, %91
  %98 = getelementptr inbounds i32, ptr %86, i64 %90
  store i32 %39, ptr %98, align 4, !tbaa !22
  %99 = getelementptr i8, ptr %98, i64 4
  store i32 %2, ptr %99, align 4, !tbaa !22
  %100 = tail call i32 @llvm.smax.i32(i32 %37, i32 %45)
  %101 = xor i32 %38, -1
  %102 = add i32 %100, %101
  %103 = zext i32 %102 to i64
  %104 = add nuw nsw i64 %103, 1
  %105 = icmp ult i32 %102, 7
  br i1 %105, label %122, label %106

106:                                              ; preds = %97
  %107 = and i64 %104, 8589934584
  %108 = add nuw nsw i64 %107, %46
  %109 = getelementptr i32, ptr %22, i64 %46
  br label %110

110:                                              ; preds = %110, %106
  %111 = phi i64 [ 0, %106 ], [ %118, %110 ]
  %112 = getelementptr i32, ptr %109, i64 %111
  %113 = getelementptr inbounds nuw i8, ptr %112, i64 16
  %114 = load <4 x i32>, ptr %112, align 4, !tbaa !22
  %115 = load <4 x i32>, ptr %113, align 4, !tbaa !22
  %116 = add nsw <4 x i32> %114, splat (i32 2)
  %117 = add nsw <4 x i32> %115, splat (i32 2)
  store <4 x i32> %116, ptr %112, align 4, !tbaa !22
  store <4 x i32> %117, ptr %113, align 4, !tbaa !22
  %118 = add nuw i64 %111, 8
  %119 = icmp eq i64 %118, %107
  br i1 %119, label %120, label %110, !llvm.loop !118

120:                                              ; preds = %110
  %121 = icmp eq i64 %104, %107
  br i1 %121, label %124, label %122

122:                                              ; preds = %97, %120
  %123 = phi i64 [ %46, %97 ], [ %108, %120 ]
  br label %129

124:                                              ; preds = %129, %120
  store ptr %86, ptr %25, align 8, !tbaa !57
  %125 = getelementptr inbounds nuw i8, ptr %13, i64 120
  %126 = load i64, ptr %125, align 8, !tbaa !45
  %127 = add nsw i64 %126, 1
  store i64 %127, ptr %125, align 8, !tbaa !45
  %128 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %127, ptr %128, align 8, !tbaa !11
  br label %137

129:                                              ; preds = %122, %129
  %130 = phi i64 [ %134, %129 ], [ %123, %122 ]
  %131 = getelementptr inbounds nuw i32, ptr %22, i64 %130
  %132 = load i32, ptr %131, align 4, !tbaa !22
  %133 = add nsw i32 %132, 2
  store i32 %133, ptr %131, align 4, !tbaa !22
  %134 = add nuw nsw i64 %130, 1
  %135 = trunc nuw i64 %130 to i32
  %136 = icmp sgt i32 %37, %135
  br i1 %136, label %129, label %124, !llvm.loop !119

137:                                              ; preds = %8, %61, %3, %35, %124, %78, %20, %24, %28, %16
  %138 = phi i32 [ 0, %16 ], [ 0, %28 ], [ 0, %24 ], [ 0, %20 ], [ 0, %35 ], [ 1, %124 ], [ 0, %78 ], [ 0, %3 ], [ 1, %61 ], [ 0, %8 ]
  ret i32 %138
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #6

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define dso_local range(i32 0, 2) i32 @autograph_bcsr_remove_edge(ptr noundef writeonly %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #8 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %121

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %121, label %11, !llvm.loop !38

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8, !tbaa !39
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %18 = load i32, ptr %17, align 8, !tbaa !50
  %19 = icmp eq i32 %18, 2
  br i1 %19, label %20, label %121

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 160
  %22 = load ptr, ptr %21, align 8, !tbaa !56
  %23 = icmp eq ptr %22, null
  br i1 %23, label %121, label %24

24:                                               ; preds = %20
  %25 = getelementptr inbounds nuw i8, ptr %13, i64 168
  %26 = load ptr, ptr %25, align 8, !tbaa !57
  %27 = icmp eq ptr %26, null
  br i1 %27, label %121, label %28

28:                                               ; preds = %24
  %29 = getelementptr inbounds nuw i8, ptr %13, i64 176
  %30 = load i32, ptr %29, align 8, !tbaa !55
  %31 = icmp sgt i32 %30, 0
  %32 = or i32 %2, %1
  %33 = icmp sgt i32 %32, -1
  %34 = and i1 %33, %31
  br i1 %34, label %35, label %121

35:                                               ; preds = %28
  %36 = getelementptr inbounds nuw i8, ptr %13, i64 180
  %37 = load i32, ptr %36, align 4, !tbaa !54
  %38 = udiv i32 %1, %30
  %39 = urem i32 %1, %30
  %40 = icmp slt i32 %38, %37
  br i1 %40, label %41, label %121

41:                                               ; preds = %35
  %42 = zext nneg i32 %38 to i64
  %43 = getelementptr inbounds nuw i32, ptr %22, i64 %42
  %44 = load i32, ptr %43, align 4, !tbaa !22
  %45 = add nuw nsw i32 %38, 1
  %46 = zext nneg i32 %45 to i64
  %47 = getelementptr inbounds nuw i32, ptr %22, i64 %46
  %48 = load i32, ptr %47, align 4, !tbaa !22
  %49 = icmp slt i32 %44, %48
  br i1 %49, label %50, label %121

50:                                               ; preds = %41
  %51 = sext i32 %44 to i64
  %52 = sext i32 %48 to i64
  br label %53

53:                                               ; preds = %50, %62
  %54 = phi i64 [ %51, %50 ], [ %63, %62 ]
  %55 = getelementptr inbounds i32, ptr %26, i64 %54
  %56 = load i32, ptr %55, align 4, !tbaa !22
  %57 = icmp eq i32 %56, %39
  br i1 %57, label %58, label %62

58:                                               ; preds = %53
  %59 = getelementptr i8, ptr %55, i64 4
  %60 = load i32, ptr %59, align 4, !tbaa !22
  %61 = icmp eq i32 %60, %2
  br i1 %61, label %65, label %62

62:                                               ; preds = %53, %58
  %63 = add nsw i64 %54, 2
  %64 = icmp slt i64 %63, %52
  br i1 %64, label %53, label %121, !llvm.loop !120

65:                                               ; preds = %58
  %66 = trunc nsw i64 %54 to i32
  %67 = icmp slt i64 %54, 0
  br i1 %67, label %121, label %68

68:                                               ; preds = %65
  %69 = zext nneg i32 %37 to i64
  %70 = getelementptr inbounds nuw i32, ptr %22, i64 %69
  %71 = load i32, ptr %70, align 4, !tbaa !22
  %72 = add nuw nsw i32 %66, 2
  %73 = icmp slt i32 %72, %71
  br i1 %73, label %74, label %83

74:                                               ; preds = %68
  %75 = and i64 %54, 4294967295
  %76 = getelementptr inbounds nuw i32, ptr %26, i64 %75
  %77 = zext nneg i32 %72 to i64
  %78 = getelementptr inbounds nuw i32, ptr %26, i64 %77
  %79 = sub i32 %71, %66
  %80 = add i32 %79, -2
  %81 = sext i32 %80 to i64
  %82 = shl nsw i64 %81, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %76, ptr nonnull align 4 %78, i64 %82, i1 false)
  br label %83

83:                                               ; preds = %68, %74
  %84 = tail call i32 @llvm.smax.i32(i32 %37, i32 %45)
  %85 = xor i32 %38, -1
  %86 = add i32 %84, %85
  %87 = zext i32 %86 to i64
  %88 = add nuw nsw i64 %87, 1
  %89 = icmp ult i32 %86, 7
  br i1 %89, label %106, label %90

90:                                               ; preds = %83
  %91 = and i64 %88, 8589934584
  %92 = add nuw nsw i64 %91, %46
  %93 = getelementptr i32, ptr %22, i64 %46
  br label %94

94:                                               ; preds = %94, %90
  %95 = phi i64 [ 0, %90 ], [ %102, %94 ]
  %96 = getelementptr i32, ptr %93, i64 %95
  %97 = getelementptr inbounds nuw i8, ptr %96, i64 16
  %98 = load <4 x i32>, ptr %96, align 4, !tbaa !22
  %99 = load <4 x i32>, ptr %97, align 4, !tbaa !22
  %100 = add nsw <4 x i32> %98, splat (i32 -2)
  %101 = add nsw <4 x i32> %99, splat (i32 -2)
  store <4 x i32> %100, ptr %96, align 4, !tbaa !22
  store <4 x i32> %101, ptr %97, align 4, !tbaa !22
  %102 = add nuw i64 %95, 8
  %103 = icmp eq i64 %102, %91
  br i1 %103, label %104, label %94, !llvm.loop !121

104:                                              ; preds = %94
  %105 = icmp eq i64 %88, %91
  br i1 %105, label %108, label %106

106:                                              ; preds = %83, %104
  %107 = phi i64 [ %46, %83 ], [ %92, %104 ]
  br label %113

108:                                              ; preds = %113, %104
  %109 = getelementptr inbounds nuw i8, ptr %13, i64 120
  %110 = load i64, ptr %109, align 8, !tbaa !45
  %111 = add nsw i64 %110, -1
  store i64 %111, ptr %109, align 8, !tbaa !45
  %112 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %111, ptr %112, align 8, !tbaa !11
  br label %121

113:                                              ; preds = %106, %113
  %114 = phi i64 [ %118, %113 ], [ %107, %106 ]
  %115 = getelementptr inbounds nuw i32, ptr %22, i64 %114
  %116 = load i32, ptr %115, align 4, !tbaa !22
  %117 = add nsw i32 %116, -2
  store i32 %117, ptr %115, align 4, !tbaa !22
  %118 = add nuw nsw i64 %114, 1
  %119 = trunc nuw i64 %114 to i32
  %120 = icmp sgt i32 %37, %119
  br i1 %120, label %113, label %108, !llvm.loop !122

121:                                              ; preds = %8, %62, %41, %3, %35, %65, %108, %20, %24, %28, %16
  %122 = phi i32 [ 0, %16 ], [ 0, %28 ], [ 0, %24 ], [ 0, %20 ], [ 0, %35 ], [ 1, %108 ], [ 0, %65 ], [ 0, %3 ], [ 0, %41 ], [ 0, %62 ], [ 0, %8 ]
  ret i32 %122
}

; Function Attrs: nounwind uwtable
define dso_local ptr @autograph_neighbor_scratch_acquire(i64 noundef %0) local_unnamed_addr #1 {
  %2 = tail call i64 @llvm.smax.i64(i64 %0, i64 1)
  %3 = icmp sgt i64 %0, 4611686018427387903
  br i1 %3, label %4, label %5

4:                                                ; preds = %1
  tail call void @abort() #22
  unreachable

5:                                                ; preds = %1
  %6 = tail call i32 @pthread_once(ptr noundef nonnull @g_neighbor_scratch_key_once, ptr noundef nonnull @autograph_neighbor_scratch_make_key) #19
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

8:                                                ; preds = %5
  tail call void @abort() #22
  unreachable

9:                                                ; preds = %5
  %10 = load i32, ptr @g_neighbor_scratch_key, align 4, !tbaa !22
  %11 = tail call ptr @pthread_getspecific(i32 noundef %10) #19
  %12 = icmp eq ptr %11, null
  br i1 %12, label %13, label %21

13:                                               ; preds = %9
  %14 = tail call noalias dereferenceable_or_null(24) ptr @calloc(i64 noundef 1, i64 noundef 24) #18
  %15 = icmp eq ptr %14, null
  br i1 %15, label %20, label %16

16:                                               ; preds = %13
  %17 = load i32, ptr @g_neighbor_scratch_key, align 4, !tbaa !22
  %18 = tail call i32 @pthread_setspecific(i32 noundef %17, ptr noundef nonnull %14) #19
  %19 = icmp eq i32 %18, 0
  br i1 %19, label %21, label %20

20:                                               ; preds = %16, %13
  tail call void @free(ptr noundef %14) #19
  tail call void @abort() #22
  unreachable

21:                                               ; preds = %16, %9
  %22 = phi ptr [ %11, %9 ], [ %14, %16 ]
  %23 = getelementptr inbounds nuw i8, ptr %22, i64 16
  %24 = load i64, ptr %23, align 8, !tbaa !123
  %25 = getelementptr inbounds nuw i8, ptr %22, i64 8
  %26 = load i64, ptr %25, align 8, !tbaa !125
  %27 = icmp eq i64 %24, %26
  br i1 %27, label %30, label %28

28:                                               ; preds = %21
  %29 = load ptr, ptr %22, align 8, !tbaa !126
  br label %50

30:                                               ; preds = %21
  %31 = icmp eq i64 %24, 0
  %32 = shl i64 %24, 1
  %33 = select i1 %31, i64 2, i64 %32
  %34 = icmp ult i64 %33, %24
  %35 = icmp ugt i64 %33, 1152921504606846975
  %36 = or i1 %34, %35
  br i1 %36, label %37, label %38

37:                                               ; preds = %30
  tail call void @abort() #22
  unreachable

38:                                               ; preds = %30
  %39 = load ptr, ptr %22, align 8, !tbaa !126
  %40 = shl nuw i64 %33, 4
  %41 = tail call ptr @realloc(ptr noundef %39, i64 noundef %40) #21
  %42 = icmp eq ptr %41, null
  br i1 %42, label %43, label %44

43:                                               ; preds = %38
  tail call void @abort() #22
  unreachable

44:                                               ; preds = %38
  %45 = load i64, ptr %25, align 8, !tbaa !125
  %46 = getelementptr inbounds nuw %struct.AutoNeighborScratchSlot, ptr %41, i64 %45
  %47 = sub i64 %33, %45
  %48 = shl i64 %47, 4
  tail call void @llvm.memset.p0.i64(ptr nonnull align 8 %46, i8 0, i64 %48, i1 false)
  store ptr %41, ptr %22, align 8, !tbaa !126
  store i64 %33, ptr %25, align 8, !tbaa !125
  %49 = load i64, ptr %23, align 8, !tbaa !123
  br label %50

50:                                               ; preds = %28, %44
  %51 = phi i64 [ %24, %28 ], [ %49, %44 ]
  %52 = phi ptr [ %29, %28 ], [ %41, %44 ]
  %53 = add i64 %51, 1
  store i64 %53, ptr %23, align 8, !tbaa !123
  %54 = getelementptr inbounds nuw %struct.AutoNeighborScratchSlot, ptr %52, i64 %51
  %55 = getelementptr inbounds nuw i8, ptr %54, i64 8
  %56 = load i64, ptr %55, align 8, !tbaa !127
  %57 = icmp slt i64 %56, %2
  %58 = load ptr, ptr %54, align 8, !tbaa !129
  br i1 %57, label %59, label %67

59:                                               ; preds = %50
  %60 = shl nuw i64 %2, 2
  %61 = tail call ptr @realloc(ptr noundef %58, i64 noundef %60) #21
  %62 = icmp eq ptr %61, null
  br i1 %62, label %63, label %66

63:                                               ; preds = %59
  %64 = load i64, ptr %23, align 8, !tbaa !123
  %65 = add i64 %64, -1
  store i64 %65, ptr %23, align 8, !tbaa !123
  tail call void @abort() #22
  unreachable

66:                                               ; preds = %59
  store ptr %61, ptr %54, align 8, !tbaa !129
  store i64 %2, ptr %55, align 8, !tbaa !127
  br label %67

67:                                               ; preds = %66, %50
  %68 = phi ptr [ %61, %66 ], [ %58, %50 ]
  ret ptr %68
}

; Function Attrs: cold nofree noreturn nounwind
declare dso_local void @abort() local_unnamed_addr #14

declare dso_local i32 @pthread_once(ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define internal void @autograph_neighbor_scratch_make_key() #1 {
  %1 = tail call i32 @pthread_key_create(ptr noundef nonnull @g_neighbor_scratch_key, ptr noundef nonnull @autograph_neighbor_scratch_destroy) #19
  %2 = icmp eq i32 %1, 0
  br i1 %2, label %4, label %3

3:                                                ; preds = %0
  tail call void @abort() #22
  unreachable

4:                                                ; preds = %0
  ret void
}

; Function Attrs: nounwind
declare dso_local ptr @pthread_getspecific(i32 noundef) local_unnamed_addr #15

; Function Attrs: nounwind
declare dso_local i32 @pthread_setspecific(i32 noundef, ptr noundef) local_unnamed_addr #15

; Function Attrs: nounwind uwtable
define dso_local void @autograph_neighbor_scratch_release() local_unnamed_addr #1 {
  %1 = tail call i32 @pthread_once(ptr noundef nonnull @g_neighbor_scratch_key_once, ptr noundef nonnull @autograph_neighbor_scratch_make_key) #19
  %2 = icmp eq i32 %1, 0
  br i1 %2, label %4, label %3

3:                                                ; preds = %0
  tail call void @abort() #22
  unreachable

4:                                                ; preds = %0
  %5 = load i32, ptr @g_neighbor_scratch_key, align 4, !tbaa !22
  %6 = tail call ptr @pthread_getspecific(i32 noundef %5) #19
  %7 = icmp eq ptr %6, null
  br i1 %7, label %12, label %8

8:                                                ; preds = %4
  %9 = getelementptr inbounds nuw i8, ptr %6, i64 16
  %10 = load i64, ptr %9, align 8, !tbaa !123
  %11 = icmp eq i64 %10, 0
  br i1 %11, label %12, label %13

12:                                               ; preds = %8, %4
  tail call void @abort() #22
  unreachable

13:                                               ; preds = %8
  %14 = add i64 %10, -1
  store i64 %14, ptr %9, align 8, !tbaa !123
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_get_neighbors(ptr noundef readnone %0, i64 noundef %1, ptr noundef writeonly %2, ptr nocapture noundef initializes((0, 8)) %3) local_unnamed_addr #1 {
  %5 = ptrtoint ptr %2 to i64
  store i64 0, ptr %3, align 8, !tbaa !11
  %6 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %7 = icmp sgt i32 %6, 0
  br i1 %7, label %8, label %282

8:                                                ; preds = %4
  %9 = zext nneg i32 %6 to i64
  br label %13

10:                                               ; preds = %13
  %11 = add nuw nsw i64 %14, 1
  %12 = icmp eq i64 %11, %9
  br i1 %12, label %282, label %13, !llvm.loop !38

13:                                               ; preds = %10, %8
  %14 = phi i64 [ 0, %8 ], [ %11, %10 ]
  %15 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %14
  %16 = load ptr, ptr %15, align 8, !tbaa !39
  %17 = icmp eq ptr %16, %0
  br i1 %17, label %18, label %10

18:                                               ; preds = %13
  %19 = icmp eq ptr %2, null
  br i1 %19, label %282, label %20

20:                                               ; preds = %18
  %21 = getelementptr inbounds nuw i8, ptr %15, i64 8
  %22 = load i32, ptr %21, align 8, !tbaa !50
  switch i32 %22, label %204 [
    i32 0, label %23
    i32 1, label %110
    i32 2, label %155
  ]

23:                                               ; preds = %20
  %24 = getelementptr inbounds nuw i8, ptr %15, i64 96
  %25 = load ptr, ptr %24, align 8, !tbaa !42
  %26 = icmp eq ptr %25, null
  br i1 %26, label %282, label %27

27:                                               ; preds = %23
  %28 = getelementptr inbounds nuw i8, ptr %15, i64 104
  %29 = load ptr, ptr %28, align 8, !tbaa !43
  %30 = ptrtoint ptr %29 to i64
  %31 = icmp eq ptr %29, null
  br i1 %31, label %282, label %32

32:                                               ; preds = %27
  %33 = getelementptr inbounds i64, ptr %25, i64 %1
  %34 = load i64, ptr %33, align 8, !tbaa !11
  %35 = getelementptr i8, ptr %33, i64 8
  %36 = load i64, ptr %35, align 8, !tbaa !11
  %37 = icmp slt i64 %34, %36
  br i1 %37, label %38, label %282

38:                                               ; preds = %32
  %39 = sub i64 %36, %34
  %40 = icmp ult i64 %39, 8
  br i1 %40, label %62, label %41

41:                                               ; preds = %38
  %42 = shl i64 %34, 2
  %43 = add i64 %42, %30
  %44 = sub i64 %5, %43
  %45 = icmp ult i64 %44, 32
  br i1 %45, label %62, label %46

46:                                               ; preds = %41
  %47 = and i64 %39, -8
  %48 = add i64 %34, %47
  %49 = getelementptr i32, ptr %29, i64 %34
  br label %50

50:                                               ; preds = %50, %46
  %51 = phi i64 [ 0, %46 ], [ %58, %50 ]
  %52 = getelementptr i32, ptr %49, i64 %51
  %53 = getelementptr inbounds nuw i8, ptr %52, i64 16
  %54 = load <4 x i32>, ptr %52, align 4, !tbaa !22
  %55 = load <4 x i32>, ptr %53, align 4, !tbaa !22
  %56 = getelementptr inbounds nuw i32, ptr %2, i64 %51
  %57 = getelementptr inbounds nuw i8, ptr %56, i64 16
  store <4 x i32> %54, ptr %56, align 4, !tbaa !22
  store <4 x i32> %55, ptr %57, align 4, !tbaa !22
  %58 = add nuw i64 %51, 8
  %59 = icmp eq i64 %58, %47
  br i1 %59, label %60, label %50, !llvm.loop !130

60:                                               ; preds = %50
  %61 = icmp eq i64 %39, %47
  br i1 %61, label %269, label %62

62:                                               ; preds = %41, %38, %60
  %63 = phi i64 [ 0, %38 ], [ 0, %41 ], [ %47, %60 ]
  %64 = phi i64 [ %34, %38 ], [ %34, %41 ], [ %48, %60 ]
  %65 = sub i64 %36, %64
  %66 = and i64 %65, 3
  %67 = icmp eq i64 %66, 0
  br i1 %67, label %79, label %68

68:                                               ; preds = %62, %68
  %69 = phi i64 [ %74, %68 ], [ %63, %62 ]
  %70 = phi i64 [ %76, %68 ], [ %64, %62 ]
  %71 = phi i64 [ %77, %68 ], [ 0, %62 ]
  %72 = getelementptr inbounds i32, ptr %29, i64 %70
  %73 = load i32, ptr %72, align 4, !tbaa !22
  %74 = add nuw nsw i64 %69, 1
  %75 = getelementptr inbounds nuw i32, ptr %2, i64 %69
  store i32 %73, ptr %75, align 4, !tbaa !22
  %76 = add nsw i64 %70, 1
  %77 = add i64 %71, 1
  %78 = icmp eq i64 %77, %66
  br i1 %78, label %79, label %68, !llvm.loop !131

79:                                               ; preds = %68, %62
  %80 = phi i64 [ poison, %62 ], [ %74, %68 ]
  %81 = phi i64 [ %63, %62 ], [ %74, %68 ]
  %82 = phi i64 [ %64, %62 ], [ %76, %68 ]
  %83 = sub i64 %64, %36
  %84 = icmp ugt i64 %83, -4
  br i1 %84, label %269, label %85

85:                                               ; preds = %79
  %86 = getelementptr i8, ptr %29, i64 4
  %87 = getelementptr i8, ptr %2, i64 4
  %88 = getelementptr i8, ptr %29, i64 8
  %89 = getelementptr i8, ptr %2, i64 8
  %90 = getelementptr i8, ptr %29, i64 12
  %91 = getelementptr i8, ptr %2, i64 12
  br label %92

92:                                               ; preds = %92, %85
  %93 = phi i64 [ %81, %85 ], [ %106, %92 ]
  %94 = phi i64 [ %82, %85 ], [ %108, %92 ]
  %95 = getelementptr inbounds i32, ptr %29, i64 %94
  %96 = load i32, ptr %95, align 4, !tbaa !22
  %97 = getelementptr inbounds nuw i32, ptr %2, i64 %93
  store i32 %96, ptr %97, align 4, !tbaa !22
  %98 = getelementptr i32, ptr %86, i64 %94
  %99 = load i32, ptr %98, align 4, !tbaa !22
  %100 = getelementptr i32, ptr %87, i64 %93
  store i32 %99, ptr %100, align 4, !tbaa !22
  %101 = getelementptr i32, ptr %88, i64 %94
  %102 = load i32, ptr %101, align 4, !tbaa !22
  %103 = getelementptr i32, ptr %89, i64 %93
  store i32 %102, ptr %103, align 4, !tbaa !22
  %104 = getelementptr i32, ptr %90, i64 %94
  %105 = load i32, ptr %104, align 4, !tbaa !22
  %106 = add nuw nsw i64 %93, 4
  %107 = getelementptr i32, ptr %91, i64 %93
  store i32 %105, ptr %107, align 4, !tbaa !22
  %108 = add nsw i64 %94, 4
  %109 = icmp eq i64 %108, %36
  br i1 %109, label %269, label %92, !llvm.loop !132

110:                                              ; preds = %20
  %111 = getelementptr inbounds nuw i8, ptr %15, i64 136
  %112 = load ptr, ptr %111, align 8, !tbaa !52
  %113 = icmp eq ptr %112, null
  br i1 %113, label %282, label %114

114:                                              ; preds = %110
  %115 = getelementptr inbounds nuw i8, ptr %15, i64 144
  %116 = load ptr, ptr %115, align 8, !tbaa !53
  %117 = icmp eq ptr %116, null
  br i1 %117, label %282, label %118

118:                                              ; preds = %114
  %119 = getelementptr inbounds i64, ptr %112, i64 %1
  %120 = load i64, ptr %119, align 8, !tbaa !11
  %121 = getelementptr i8, ptr %119, i64 8
  %122 = load i64, ptr %121, align 8, !tbaa !11
  %123 = icmp slt i64 %120, %122
  br i1 %123, label %124, label %282

124:                                              ; preds = %118
  %125 = sub i64 %122, %120
  %126 = add i64 %120, 1
  %127 = and i64 %125, 1
  %128 = icmp eq i64 %122, %126
  br i1 %128, label %271, label %129

129:                                              ; preds = %124
  %130 = and i64 %125, -2
  %131 = getelementptr i8, ptr %116, i64 4
  br label %132

132:                                              ; preds = %150, %129
  %133 = phi i64 [ 0, %129 ], [ %151, %150 ]
  %134 = phi i64 [ %120, %129 ], [ %152, %150 ]
  %135 = phi i64 [ 0, %129 ], [ %153, %150 ]
  %136 = getelementptr inbounds i32, ptr %116, i64 %134
  %137 = load i32, ptr %136, align 4, !tbaa !22
  %138 = icmp eq i32 %137, -1
  br i1 %138, label %142, label %139

139:                                              ; preds = %132
  %140 = add nsw i64 %133, 1
  store i64 %140, ptr %3, align 8, !tbaa !11
  %141 = getelementptr inbounds i32, ptr %2, i64 %133
  store i32 %137, ptr %141, align 4, !tbaa !22
  br label %142

142:                                              ; preds = %139, %132
  %143 = phi i64 [ %140, %139 ], [ %133, %132 ]
  %144 = getelementptr i32, ptr %131, i64 %134
  %145 = load i32, ptr %144, align 4, !tbaa !22
  %146 = icmp eq i32 %145, -1
  br i1 %146, label %150, label %147

147:                                              ; preds = %142
  %148 = add nsw i64 %143, 1
  store i64 %148, ptr %3, align 8, !tbaa !11
  %149 = getelementptr inbounds i32, ptr %2, i64 %143
  store i32 %145, ptr %149, align 4, !tbaa !22
  br label %150

150:                                              ; preds = %147, %142
  %151 = phi i64 [ %148, %147 ], [ %143, %142 ]
  %152 = add nsw i64 %134, 2
  %153 = add i64 %135, 2
  %154 = icmp eq i64 %153, %130
  br i1 %154, label %271, label %132, !llvm.loop !133

155:                                              ; preds = %20
  %156 = getelementptr inbounds nuw i8, ptr %15, i64 160
  %157 = load ptr, ptr %156, align 8, !tbaa !56
  %158 = icmp eq ptr %157, null
  br i1 %158, label %282, label %159

159:                                              ; preds = %155
  %160 = getelementptr inbounds nuw i8, ptr %15, i64 168
  %161 = load ptr, ptr %160, align 8, !tbaa !57
  %162 = icmp eq ptr %161, null
  br i1 %162, label %282, label %163

163:                                              ; preds = %159
  %164 = getelementptr inbounds nuw i8, ptr %15, i64 176
  %165 = load i32, ptr %164, align 8, !tbaa !55
  %166 = icmp slt i32 %165, 1
  br i1 %166, label %282, label %167

167:                                              ; preds = %163
  %168 = zext nneg i32 %165 to i64
  %169 = sdiv i64 %1, %168
  %170 = srem i64 %1, %168
  %171 = trunc nsw i64 %170 to i32
  %172 = shl i64 %169, 32
  %173 = ashr exact i64 %172, 30
  %174 = getelementptr inbounds i8, ptr %157, i64 %173
  %175 = load i32, ptr %174, align 4, !tbaa !22
  %176 = add i64 %172, 4294967296
  %177 = ashr exact i64 %176, 30
  %178 = getelementptr inbounds i8, ptr %157, i64 %177
  %179 = load i32, ptr %178, align 4, !tbaa !22
  %180 = icmp slt i32 %175, %179
  br i1 %180, label %181, label %282

181:                                              ; preds = %167
  %182 = getelementptr i8, ptr %161, i64 4
  br label %183

183:                                              ; preds = %181, %192
  %184 = phi i64 [ 0, %181 ], [ %195, %192 ]
  %185 = phi i32 [ %175, %181 ], [ %197, %192 ]
  br label %186

186:                                              ; preds = %183, %199
  %187 = phi i32 [ %201, %199 ], [ %185, %183 ]
  %188 = sext i32 %187 to i64
  %189 = getelementptr inbounds i32, ptr %161, i64 %188
  %190 = load i32, ptr %189, align 4, !tbaa !22
  %191 = icmp eq i32 %190, %171
  br i1 %191, label %192, label %199

192:                                              ; preds = %186
  %193 = getelementptr i32, ptr %182, i64 %188
  %194 = load i32, ptr %193, align 4, !tbaa !22
  %195 = add nuw nsw i64 %184, 1
  store i64 %195, ptr %3, align 8, !tbaa !11
  %196 = getelementptr inbounds nuw i32, ptr %2, i64 %184
  store i32 %194, ptr %196, align 4, !tbaa !22
  %197 = add nsw i32 %187, 2
  %198 = icmp slt i32 %197, %179
  br i1 %198, label %183, label %282, !llvm.loop !134

199:                                              ; preds = %186
  %200 = icmp sle i32 %190, %171
  %201 = add nsw i32 %187, 2
  %202 = icmp slt i32 %201, %179
  %203 = select i1 %200, i1 %202, i1 false
  br i1 %203, label %186, label %282, !llvm.loop !134

204:                                              ; preds = %20
  %205 = getelementptr inbounds nuw i8, ptr %15, i64 24
  %206 = load ptr, ptr %205, align 8, !tbaa !48
  %207 = getelementptr inbounds nuw i8, ptr %15, i64 32
  %208 = load ptr, ptr %207, align 8, !tbaa !49
  %209 = getelementptr inbounds nuw i8, ptr %15, i64 40
  %210 = load i64, ptr %209, align 8, !tbaa !63
  %211 = icmp sgt i64 %210, 0
  br i1 %211, label %212, label %214

212:                                              ; preds = %204
  %213 = trunc i64 %1 to i32
  br label %223

214:                                              ; preds = %240, %204
  %215 = getelementptr inbounds nuw i8, ptr %15, i64 64
  %216 = load i64, ptr %215, align 8, !tbaa !60
  %217 = icmp sgt i64 %216, 0
  br i1 %217, label %218, label %282

218:                                              ; preds = %214
  %219 = getelementptr inbounds nuw i8, ptr %15, i64 56
  %220 = load ptr, ptr %219, align 8, !tbaa !64
  %221 = getelementptr inbounds nuw i8, ptr %15, i64 48
  %222 = trunc i64 %1 to i32
  br label %244

223:                                              ; preds = %212, %240
  %224 = phi i64 [ 0, %212 ], [ %241, %240 ]
  %225 = trunc i64 %224 to i32
  %226 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %206, i32 noundef %225) #19
  br i1 %226, label %227, label %240

227:                                              ; preds = %223
  %228 = getelementptr inbounds nuw %struct.EdgePair, ptr %208, i64 %224
  %229 = load i32, ptr %228, align 4, !tbaa !3
  %230 = icmp eq i32 %229, %213
  %231 = getelementptr inbounds nuw i8, ptr %228, i64 4
  %232 = load i32, ptr %231, align 4, !tbaa !8
  br i1 %230, label %235, label %233

233:                                              ; preds = %227
  %234 = icmp eq i32 %232, %213
  br i1 %234, label %235, label %240

235:                                              ; preds = %233, %227
  %236 = phi i32 [ %232, %227 ], [ %229, %233 ]
  %237 = load i64, ptr %3, align 8, !tbaa !11
  %238 = add nsw i64 %237, 1
  store i64 %238, ptr %3, align 8, !tbaa !11
  %239 = getelementptr inbounds i32, ptr %2, i64 %237
  store i32 %236, ptr %239, align 4, !tbaa !22
  br label %240

240:                                              ; preds = %235, %233, %223
  %241 = add nuw nsw i64 %224, 1
  %242 = load i64, ptr %209, align 8, !tbaa !63
  %243 = icmp slt i64 %241, %242
  br i1 %243, label %223, label %214, !llvm.loop !135

244:                                              ; preds = %218, %265
  %245 = phi i64 [ 0, %218 ], [ %266, %265 ]
  %246 = getelementptr inbounds nuw i8, ptr %220, i64 %245
  %247 = load i8, ptr %246, align 1, !tbaa !65
  %248 = icmp eq i8 %247, 0
  br i1 %248, label %265, label %249

249:                                              ; preds = %244
  %250 = load ptr, ptr %221, align 8, !tbaa !69
  %251 = shl nuw nsw i64 %245, 1
  %252 = getelementptr inbounds nuw i32, ptr %250, i64 %251
  %253 = load i32, ptr %252, align 4, !tbaa !22
  %254 = or disjoint i64 %251, 1
  %255 = getelementptr inbounds nuw i32, ptr %250, i64 %254
  %256 = load i32, ptr %255, align 4, !tbaa !22
  %257 = icmp eq i32 %253, %222
  br i1 %257, label %260, label %258

258:                                              ; preds = %249
  %259 = icmp eq i32 %256, %222
  br i1 %259, label %260, label %265

260:                                              ; preds = %258, %249
  %261 = phi i32 [ %256, %249 ], [ %253, %258 ]
  %262 = load i64, ptr %3, align 8, !tbaa !11
  %263 = add nsw i64 %262, 1
  store i64 %263, ptr %3, align 8, !tbaa !11
  %264 = getelementptr inbounds i32, ptr %2, i64 %262
  store i32 %261, ptr %264, align 4, !tbaa !22
  br label %265

265:                                              ; preds = %260, %258, %244
  %266 = add nuw nsw i64 %245, 1
  %267 = load i64, ptr %215, align 8, !tbaa !60
  %268 = icmp slt i64 %266, %267
  br i1 %268, label %244, label %282, !llvm.loop !136

269:                                              ; preds = %79, %92, %60
  %270 = phi i64 [ %47, %60 ], [ %80, %79 ], [ %106, %92 ]
  store i64 %270, ptr %3, align 8, !tbaa !11
  br label %282

271:                                              ; preds = %150, %124
  %272 = phi i64 [ 0, %124 ], [ %151, %150 ]
  %273 = phi i64 [ %120, %124 ], [ %152, %150 ]
  %274 = icmp eq i64 %127, 0
  br i1 %274, label %282, label %275

275:                                              ; preds = %271
  %276 = getelementptr inbounds i32, ptr %116, i64 %273
  %277 = load i32, ptr %276, align 4, !tbaa !22
  %278 = icmp eq i32 %277, -1
  br i1 %278, label %282, label %279

279:                                              ; preds = %275
  %280 = add nsw i64 %272, 1
  store i64 %280, ptr %3, align 8, !tbaa !11
  %281 = getelementptr inbounds i32, ptr %2, i64 %272
  store i32 %277, ptr %281, align 4, !tbaa !22
  br label %282

282:                                              ; preds = %10, %199, %192, %271, %279, %275, %265, %167, %118, %32, %269, %214, %4, %27, %23, %114, %110, %163, %159, %155, %18
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @autograph_frontier_step(ptr noundef readnone %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4, ptr noundef %5, i32 noundef %6, i32 noundef %7, ptr noundef %8) local_unnamed_addr #1 {
  %10 = alloca %struct.AutoFrontierStepEnv, align 8
  %11 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %12 = icmp sgt i32 %11, 0
  br i1 %12, label %13, label %23

13:                                               ; preds = %9
  %14 = zext nneg i32 %11 to i64
  br label %18

15:                                               ; preds = %18
  %16 = add nuw nsw i64 %19, 1
  %17 = icmp eq i64 %16, %14
  br i1 %17, label %23, label %18, !llvm.loop !38

18:                                               ; preds = %15, %13
  %19 = phi i64 [ 0, %13 ], [ %16, %15 ]
  %20 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %19
  %21 = load ptr, ptr %20, align 8, !tbaa !39
  %22 = icmp eq ptr %21, %0
  br i1 %22, label %23, label %15

23:                                               ; preds = %15, %18, %9
  %24 = phi ptr [ null, %9 ], [ null, %15 ], [ %20, %18 ]
  %25 = insertelement <4 x ptr> poison, ptr %1, i64 0
  %26 = insertelement <4 x ptr> %25, ptr %24, i64 1
  %27 = insertelement <4 x ptr> %26, ptr %3, i64 2
  %28 = insertelement <4 x ptr> %27, ptr %5, i64 3
  %29 = icmp eq <4 x ptr> %28, zeroinitializer
  %30 = icmp slt i32 %2, 1
  %31 = bitcast <4 x i1> %29 to i4
  %32 = icmp ne i4 %31, 0
  %33 = or i1 %32, %30
  br i1 %33, label %117, label %34

34:                                               ; preds = %23
  %35 = getelementptr inbounds nuw i8, ptr %24, i64 112
  %36 = load i64, ptr %35, align 8, !tbaa !44
  %37 = icmp slt i64 %36, 1
  br i1 %37, label %117, label %38

38:                                               ; preds = %34
  %39 = icmp samesign ugt i64 %36, 2147483647
  %40 = icmp slt i32 %4, 0
  %41 = or i1 %40, %39
  %42 = zext nneg i32 %4 to i64
  %43 = icmp samesign ult i64 %36, %42
  %44 = select i1 %41, i1 true, i1 %43
  br i1 %44, label %117, label %45

45:                                               ; preds = %38
  call void @llvm.lifetime.start.p0(i64 64, ptr nonnull %10) #19
  store ptr %24, ptr %10, align 8, !tbaa !137
  %46 = getelementptr inbounds nuw i8, ptr %10, i64 8
  store ptr %1, ptr %46, align 8, !tbaa !139
  %47 = getelementptr inbounds nuw i8, ptr %10, i64 16
  store ptr %3, ptr %47, align 8, !tbaa !140
  %48 = getelementptr inbounds nuw i8, ptr %10, i64 24
  %49 = trunc nuw i64 %36 to i32
  store i32 %49, ptr %48, align 8, !tbaa !141
  %50 = getelementptr inbounds nuw i8, ptr %10, i64 28
  store i32 %4, ptr %50, align 4, !tbaa !142
  %51 = getelementptr inbounds nuw i8, ptr %10, i64 32
  store ptr %5, ptr %51, align 8, !tbaa !143
  %52 = getelementptr inbounds nuw i8, ptr %10, i64 40
  store i32 %6, ptr %52, align 8, !tbaa !144
  %53 = getelementptr inbounds nuw i8, ptr %10, i64 44
  store i32 %7, ptr %53, align 4, !tbaa !145
  %54 = getelementptr inbounds nuw i8, ptr %10, i64 48
  store ptr %8, ptr %54, align 8, !tbaa !146
  %55 = getelementptr inbounds nuw i8, ptr %10, i64 56
  store ptr null, ptr %55, align 8, !tbaa !147
  %56 = getelementptr inbounds nuw i8, ptr %24, i64 8
  %57 = load i32, ptr %56, align 8, !tbaa !50
  %58 = icmp eq i32 %57, 3
  %59 = zext nneg i32 %2 to i64
  %60 = mul nuw nsw i64 %59, 20
  %61 = icmp samesign ult i64 %60, %36
  %62 = select i1 %58, i1 true, i1 %61
  br i1 %62, label %113, label %63

63:                                               ; preds = %45
  %64 = tail call noalias ptr @calloc(i64 noundef %36, i64 noundef 1) #18
  %65 = icmp eq ptr %64, null
  br i1 %65, label %113, label %66

66:                                               ; preds = %63
  %67 = and i64 %59, 1
  %68 = icmp eq i32 %2, 1
  br i1 %68, label %98, label %69

69:                                               ; preds = %66
  %70 = and i64 %59, 2147483646
  br label %71

71:                                               ; preds = %94, %69
  %72 = phi i64 [ 0, %69 ], [ %95, %94 ]
  %73 = phi i64 [ 0, %69 ], [ %96, %94 ]
  %74 = getelementptr inbounds nuw i32, ptr %1, i64 %72
  %75 = load i32, ptr %74, align 4, !tbaa !22
  %76 = icmp sgt i32 %75, -1
  br i1 %76, label %77, label %83

77:                                               ; preds = %71
  %78 = zext nneg i32 %75 to i64
  %79 = load i64, ptr %35, align 8, !tbaa !44
  %80 = icmp sgt i64 %79, %78
  br i1 %80, label %81, label %83

81:                                               ; preds = %77
  %82 = getelementptr inbounds nuw i8, ptr %64, i64 %78
  store i8 1, ptr %82, align 1, !tbaa !65
  br label %83

83:                                               ; preds = %81, %77, %71
  %84 = or disjoint i64 %72, 1
  %85 = getelementptr inbounds nuw i32, ptr %1, i64 %84
  %86 = load i32, ptr %85, align 4, !tbaa !22
  %87 = icmp sgt i32 %86, -1
  br i1 %87, label %88, label %94

88:                                               ; preds = %83
  %89 = zext nneg i32 %86 to i64
  %90 = load i64, ptr %35, align 8, !tbaa !44
  %91 = icmp sgt i64 %90, %89
  br i1 %91, label %92, label %94

92:                                               ; preds = %88
  %93 = getelementptr inbounds nuw i8, ptr %64, i64 %89
  store i8 1, ptr %93, align 1, !tbaa !65
  br label %94

94:                                               ; preds = %92, %88, %83
  %95 = add nuw nsw i64 %72, 2
  %96 = add i64 %73, 2
  %97 = icmp eq i64 %96, %70
  br i1 %97, label %98, label %71, !llvm.loop !148

98:                                               ; preds = %94, %66
  %99 = phi i64 [ 0, %66 ], [ %95, %94 ]
  %100 = icmp eq i64 %67, 0
  br i1 %100, label %111, label %101

101:                                              ; preds = %98
  %102 = getelementptr inbounds nuw i32, ptr %1, i64 %99
  %103 = load i32, ptr %102, align 4, !tbaa !22
  %104 = icmp sgt i32 %103, -1
  br i1 %104, label %105, label %111

105:                                              ; preds = %101
  %106 = zext nneg i32 %103 to i64
  %107 = load i64, ptr %35, align 8, !tbaa !44
  %108 = icmp sgt i64 %107, %106
  br i1 %108, label %109, label %111

109:                                              ; preds = %105
  %110 = getelementptr inbounds nuw i8, ptr %64, i64 %106
  store i8 1, ptr %110, align 1, !tbaa !65
  br label %111

111:                                              ; preds = %101, %105, %109, %98
  %112 = load i64, ptr %35, align 8, !tbaa !44
  store ptr %64, ptr %55, align 8, !tbaa !147
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %112, i64 noundef 1, ptr noundef nonnull @autograph_frontier_pull_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #19
  br label %114

113:                                              ; preds = %45, %63
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %59, i64 noundef 1, ptr noundef nonnull @autograph_frontier_push_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #19
  br label %114

114:                                              ; preds = %113, %111
  %115 = phi ptr [ null, %113 ], [ %64, %111 ]
  call void @free(ptr noundef %115) #19
  %116 = load atomic i32, ptr %50 monotonic, align 4
  call void @llvm.lifetime.end.p0(i64 64, ptr nonnull %10) #19
  br label %117

117:                                              ; preds = %23, %34, %38, %114
  %118 = phi i32 [ %116, %114 ], [ %4, %38 ], [ %4, %34 ], [ %4, %23 ]
  ret i32 %118
}

declare dso_local void @parallel_for_runtime(i64 noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
define internal void @autograph_frontier_pull_body(i64 noundef %0, ptr nocapture noundef %1) #16 {
  %3 = trunc i64 %0 to i32
  %4 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %5 = load ptr, ptr %4, align 8, !tbaa !143
  %6 = shl i64 %0, 32
  %7 = ashr exact i64 %6, 30
  %8 = getelementptr inbounds i8, ptr %5, i64 %7
  %9 = load atomic i32, ptr %8 monotonic, align 4
  %10 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %11 = load i32, ptr %10, align 8, !tbaa !144
  %12 = icmp eq i32 %9, %11
  br i1 %12, label %13, label %173

13:                                               ; preds = %2
  %14 = load ptr, ptr %1, align 8, !tbaa !137
  %15 = getelementptr inbounds nuw i8, ptr %14, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !50
  switch i32 %16, label %173 [
    i32 0, label %17
    i32 1, label %52
    i32 2, label %87
  ]

17:                                               ; preds = %13
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %19 = load ptr, ptr %18, align 8, !tbaa !42
  %20 = icmp eq ptr %19, null
  br i1 %20, label %173, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %14, i64 104
  %23 = load ptr, ptr %22, align 8, !tbaa !43
  %24 = icmp eq ptr %23, null
  br i1 %24, label %173, label %25

25:                                               ; preds = %21
  %26 = ashr exact i64 %6, 29
  %27 = getelementptr inbounds i8, ptr %19, i64 %26
  %28 = load i64, ptr %27, align 8, !tbaa !11
  %29 = getelementptr inbounds nuw i8, ptr %1, i64 56
  %30 = getelementptr i8, ptr %27, i64 8
  %31 = load i64, ptr %30, align 8, !tbaa !11
  %32 = icmp slt i64 %28, %31
  br i1 %32, label %33, label %173

33:                                               ; preds = %25
  %34 = getelementptr inbounds nuw i8, ptr %14, i64 112
  br label %35

35:                                               ; preds = %49, %33
  %36 = phi i64 [ %28, %33 ], [ %50, %49 ]
  %37 = getelementptr inbounds i32, ptr %23, i64 %36
  %38 = load i32, ptr %37, align 4, !tbaa !22
  %39 = icmp sgt i32 %38, -1
  br i1 %39, label %40, label %49

40:                                               ; preds = %35
  %41 = zext nneg i32 %38 to i64
  %42 = load i64, ptr %34, align 8, !tbaa !44
  %43 = icmp sgt i64 %42, %41
  br i1 %43, label %44, label %49

44:                                               ; preds = %40
  %45 = load ptr, ptr %29, align 8, !tbaa !147
  %46 = getelementptr inbounds nuw i8, ptr %45, i64 %41
  %47 = load i8, ptr %46, align 1, !tbaa !65
  %48 = icmp eq i8 %47, 0
  br i1 %48, label %49, label %140

49:                                               ; preds = %44, %40, %35
  %50 = add i64 %36, 1
  %51 = icmp eq i64 %50, %31
  br i1 %51, label %173, label %35, !llvm.loop !149

52:                                               ; preds = %13
  %53 = getelementptr inbounds nuw i8, ptr %14, i64 136
  %54 = load ptr, ptr %53, align 8, !tbaa !52
  %55 = icmp eq ptr %54, null
  br i1 %55, label %173, label %56

56:                                               ; preds = %52
  %57 = getelementptr inbounds nuw i8, ptr %14, i64 144
  %58 = load ptr, ptr %57, align 8, !tbaa !53
  %59 = icmp eq ptr %58, null
  br i1 %59, label %173, label %60

60:                                               ; preds = %56
  %61 = ashr exact i64 %6, 29
  %62 = getelementptr inbounds i8, ptr %54, i64 %61
  %63 = load i64, ptr %62, align 8, !tbaa !11
  %64 = getelementptr inbounds nuw i8, ptr %1, i64 56
  %65 = getelementptr i8, ptr %62, i64 8
  %66 = load i64, ptr %65, align 8, !tbaa !11
  %67 = icmp slt i64 %63, %66
  br i1 %67, label %68, label %173

68:                                               ; preds = %60
  %69 = getelementptr inbounds nuw i8, ptr %14, i64 112
  br label %70

70:                                               ; preds = %84, %68
  %71 = phi i64 [ %63, %68 ], [ %85, %84 ]
  %72 = getelementptr inbounds i32, ptr %58, i64 %71
  %73 = load i32, ptr %72, align 4, !tbaa !22
  %74 = icmp sgt i32 %73, -1
  br i1 %74, label %75, label %84

75:                                               ; preds = %70
  %76 = zext nneg i32 %73 to i64
  %77 = load i64, ptr %69, align 8, !tbaa !44
  %78 = icmp sgt i64 %77, %76
  br i1 %78, label %79, label %84

79:                                               ; preds = %75
  %80 = load ptr, ptr %64, align 8, !tbaa !147
  %81 = getelementptr inbounds nuw i8, ptr %80, i64 %76
  %82 = load i8, ptr %81, align 1, !tbaa !65
  %83 = icmp eq i8 %82, 0
  br i1 %83, label %84, label %140

84:                                               ; preds = %79, %75, %70
  %85 = add i64 %71, 1
  %86 = icmp eq i64 %85, %66
  br i1 %86, label %173, label %70, !llvm.loop !150

87:                                               ; preds = %13
  %88 = getelementptr inbounds nuw i8, ptr %14, i64 160
  %89 = load ptr, ptr %88, align 8, !tbaa !56
  %90 = icmp eq ptr %89, null
  br i1 %90, label %173, label %91

91:                                               ; preds = %87
  %92 = getelementptr inbounds nuw i8, ptr %14, i64 168
  %93 = load ptr, ptr %92, align 8, !tbaa !57
  %94 = icmp eq ptr %93, null
  br i1 %94, label %173, label %95

95:                                               ; preds = %91
  %96 = getelementptr inbounds nuw i8, ptr %14, i64 176
  %97 = load i32, ptr %96, align 8, !tbaa !55
  %98 = icmp sgt i32 %97, 0
  br i1 %98, label %99, label %173

99:                                               ; preds = %95
  %100 = sdiv i32 %3, %97
  %101 = srem i32 %3, %97
  %102 = sext i32 %100 to i64
  %103 = getelementptr inbounds i32, ptr %89, i64 %102
  %104 = load i32, ptr %103, align 4, !tbaa !22
  %105 = getelementptr inbounds nuw i8, ptr %1, i64 56
  %106 = getelementptr i8, ptr %103, i64 4
  %107 = load i32, ptr %106, align 4, !tbaa !22
  %108 = icmp slt i32 %104, %107
  br i1 %108, label %109, label %173

109:                                              ; preds = %99
  %110 = getelementptr inbounds nuw i8, ptr %14, i64 112
  br label %111

111:                                              ; preds = %138, %109
  %112 = phi i32 [ %104, %109 ], [ %139, %138 ]
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds i32, ptr %93, i64 %113
  %115 = load i32, ptr %114, align 4, !tbaa !22
  %116 = icmp eq i32 %115, %101
  br i1 %116, label %117, label %130

117:                                              ; preds = %111
  %118 = getelementptr i8, ptr %114, i64 4
  %119 = load i32, ptr %118, align 4, !tbaa !22
  %120 = icmp sgt i32 %119, -1
  br i1 %120, label %121, label %135

121:                                              ; preds = %117
  %122 = zext nneg i32 %119 to i64
  %123 = load i64, ptr %110, align 8, !tbaa !44
  %124 = icmp sgt i64 %123, %122
  br i1 %124, label %125, label %135

125:                                              ; preds = %121
  %126 = load ptr, ptr %105, align 8, !tbaa !147
  %127 = getelementptr inbounds nuw i8, ptr %126, i64 %122
  %128 = load i8, ptr %127, align 1, !tbaa !65
  %129 = icmp eq i8 %128, 0
  br i1 %129, label %135, label %140

130:                                              ; preds = %111
  %131 = icmp sle i32 %115, %101
  %132 = add nsw i32 %112, 2
  %133 = icmp slt i32 %132, %107
  %134 = select i1 %131, i1 %133, i1 false
  br i1 %134, label %138, label %173

135:                                              ; preds = %125, %121, %117
  %136 = add nsw i32 %112, 2
  %137 = icmp slt i32 %136, %107
  br i1 %137, label %138, label %173

138:                                              ; preds = %135, %130
  %139 = phi i32 [ %136, %135 ], [ %132, %130 ]
  br label %111, !llvm.loop !151

140:                                              ; preds = %125, %79, %44
  %141 = phi i64 [ %42, %44 ], [ %77, %79 ], [ %123, %125 ]
  %142 = phi i32 [ %38, %44 ], [ %73, %79 ], [ %119, %125 ]
  %143 = icmp slt i32 %3, 0
  br i1 %143, label %173, label %144

144:                                              ; preds = %140
  %145 = and i64 %0, 2147483647
  %146 = icmp samesign ugt i64 %141, %145
  br i1 %146, label %147, label %173

147:                                              ; preds = %144
  %148 = getelementptr inbounds nuw i32, ptr %5, i64 %145
  %149 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %150 = load i32, ptr %149, align 4, !tbaa !145
  %151 = cmpxchg ptr %148, i32 %9, i32 %150 monotonic monotonic, align 4
  %152 = extractvalue { i32, i1 } %151, 1
  br i1 %152, label %153, label %173

153:                                              ; preds = %147
  %154 = getelementptr inbounds nuw i8, ptr %1, i64 28
  %155 = atomicrmw add ptr %154, i32 1 monotonic, align 4
  %156 = icmp slt i32 %155, 0
  br i1 %156, label %161, label %157

157:                                              ; preds = %153
  %158 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %159 = load i32, ptr %158, align 8, !tbaa !141
  %160 = icmp slt i32 %155, %159
  br i1 %160, label %162, label %161

161:                                              ; preds = %157, %153
  tail call void @abort() #22
  unreachable

162:                                              ; preds = %157
  %163 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %164 = load ptr, ptr %163, align 8, !tbaa !146
  %165 = icmp eq ptr %164, null
  br i1 %165, label %168, label %166

166:                                              ; preds = %162
  %167 = getelementptr inbounds nuw i32, ptr %164, i64 %145
  store i32 %142, ptr %167, align 4, !tbaa !22
  br label %168

168:                                              ; preds = %166, %162
  %169 = getelementptr inbounds nuw i8, ptr %1, i64 16
  %170 = load ptr, ptr %169, align 8, !tbaa !140
  %171 = zext nneg i32 %155 to i64
  %172 = getelementptr inbounds nuw i32, ptr %170, i64 %171
  store i32 %3, ptr %172, align 4, !tbaa !22
  br label %173

173:                                              ; preds = %130, %135, %84, %49, %99, %60, %25, %21, %17, %56, %52, %95, %91, %87, %13, %168, %147, %144, %140, %2
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @autograph_frontier_push_body(i64 noundef %0, ptr nocapture noundef %1) #1 {
  %3 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %4 = load ptr, ptr %3, align 8, !tbaa !139
  %5 = getelementptr inbounds i32, ptr %4, i64 %0
  %6 = load i32, ptr %5, align 4, !tbaa !22
  %7 = load ptr, ptr %1, align 8, !tbaa !137
  %8 = icmp slt i32 %6, 0
  br i1 %8, label %415, label %9

9:                                                ; preds = %2
  %10 = zext nneg i32 %6 to i64
  %11 = getelementptr inbounds nuw i8, ptr %7, i64 112
  %12 = load i64, ptr %11, align 8, !tbaa !44
  %13 = icmp sgt i64 %12, %10
  br i1 %13, label %14, label %415

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !50
  switch i32 %16, label %224 [
    i32 0, label %17
    i32 1, label %81
    i32 2, label %145
  ]

17:                                               ; preds = %14
  %18 = getelementptr inbounds nuw i8, ptr %7, i64 96
  %19 = load ptr, ptr %18, align 8, !tbaa !42
  %20 = icmp eq ptr %19, null
  br i1 %20, label %415, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %7, i64 104
  %23 = load ptr, ptr %22, align 8, !tbaa !43
  %24 = icmp eq ptr %23, null
  br i1 %24, label %415, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i64, ptr %19, i64 %10
  %27 = load i64, ptr %26, align 8, !tbaa !11
  %28 = getelementptr inbounds nuw i8, ptr %26, i64 8
  %29 = load i64, ptr %28, align 8, !tbaa !11
  %30 = icmp slt i64 %27, %29
  br i1 %30, label %31, label %415

31:                                               ; preds = %25
  %32 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %33 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %34 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %35 = getelementptr inbounds nuw i8, ptr %1, i64 28
  %36 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %37 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %38 = getelementptr inbounds nuw i8, ptr %1, i64 16
  br label %39

39:                                               ; preds = %74, %31
  %40 = phi i64 [ %27, %31 ], [ %75, %74 ]
  %41 = load ptr, ptr %22, align 8, !tbaa !43
  %42 = getelementptr inbounds i32, ptr %41, i64 %40
  %43 = load i32, ptr %42, align 4, !tbaa !22
  %44 = icmp slt i32 %43, 0
  br i1 %44, label %74, label %45

45:                                               ; preds = %39
  %46 = zext nneg i32 %43 to i64
  %47 = load ptr, ptr %1, align 8, !tbaa !137
  %48 = getelementptr inbounds nuw i8, ptr %47, i64 112
  %49 = load i64, ptr %48, align 8, !tbaa !44
  %50 = icmp sgt i64 %49, %46
  br i1 %50, label %51, label %74

51:                                               ; preds = %45
  %52 = load i32, ptr %32, align 8, !tbaa !144
  %53 = load ptr, ptr %33, align 8, !tbaa !143
  %54 = getelementptr inbounds nuw i32, ptr %53, i64 %46
  %55 = load i32, ptr %34, align 4, !tbaa !145
  %56 = cmpxchg ptr %54, i32 %52, i32 %55 monotonic monotonic, align 4
  %57 = extractvalue { i32, i1 } %56, 1
  br i1 %57, label %58, label %74

58:                                               ; preds = %51
  %59 = atomicrmw add ptr %35, i32 1 monotonic, align 4
  %60 = icmp slt i32 %59, 0
  br i1 %60, label %64, label %61

61:                                               ; preds = %58
  %62 = load i32, ptr %36, align 8, !tbaa !141
  %63 = icmp slt i32 %59, %62
  br i1 %63, label %65, label %64

64:                                               ; preds = %61, %58
  tail call void @abort() #22
  unreachable

65:                                               ; preds = %61
  %66 = load ptr, ptr %37, align 8, !tbaa !146
  %67 = icmp eq ptr %66, null
  br i1 %67, label %70, label %68

68:                                               ; preds = %65
  %69 = getelementptr inbounds nuw i32, ptr %66, i64 %46
  store i32 %6, ptr %69, align 4, !tbaa !22
  br label %70

70:                                               ; preds = %68, %65
  %71 = load ptr, ptr %38, align 8, !tbaa !140
  %72 = zext nneg i32 %59 to i64
  %73 = getelementptr inbounds nuw i32, ptr %71, i64 %72
  store i32 %43, ptr %73, align 4, !tbaa !22
  br label %74

74:                                               ; preds = %70, %51, %45, %39
  %75 = add nsw i64 %40, 1
  %76 = load ptr, ptr %18, align 8, !tbaa !42
  %77 = getelementptr inbounds nuw i64, ptr %76, i64 %10
  %78 = getelementptr inbounds nuw i8, ptr %77, i64 8
  %79 = load i64, ptr %78, align 8, !tbaa !11
  %80 = icmp slt i64 %75, %79
  br i1 %80, label %39, label %415, !llvm.loop !152

81:                                               ; preds = %14
  %82 = getelementptr inbounds nuw i8, ptr %7, i64 136
  %83 = load ptr, ptr %82, align 8, !tbaa !52
  %84 = icmp eq ptr %83, null
  br i1 %84, label %415, label %85

85:                                               ; preds = %81
  %86 = getelementptr inbounds nuw i8, ptr %7, i64 144
  %87 = load ptr, ptr %86, align 8, !tbaa !53
  %88 = icmp eq ptr %87, null
  br i1 %88, label %415, label %89

89:                                               ; preds = %85
  %90 = getelementptr inbounds nuw i64, ptr %83, i64 %10
  %91 = load i64, ptr %90, align 8, !tbaa !11
  %92 = getelementptr inbounds nuw i8, ptr %90, i64 8
  %93 = load i64, ptr %92, align 8, !tbaa !11
  %94 = icmp slt i64 %91, %93
  br i1 %94, label %95, label %415

95:                                               ; preds = %89
  %96 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %97 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %98 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %99 = getelementptr inbounds nuw i8, ptr %1, i64 28
  %100 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %101 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %102 = getelementptr inbounds nuw i8, ptr %1, i64 16
  br label %103

103:                                              ; preds = %138, %95
  %104 = phi i64 [ %91, %95 ], [ %139, %138 ]
  %105 = load ptr, ptr %86, align 8, !tbaa !53
  %106 = getelementptr inbounds i32, ptr %105, i64 %104
  %107 = load i32, ptr %106, align 4, !tbaa !22
  %108 = icmp slt i32 %107, 0
  br i1 %108, label %138, label %109

109:                                              ; preds = %103
  %110 = zext nneg i32 %107 to i64
  %111 = load ptr, ptr %1, align 8, !tbaa !137
  %112 = getelementptr inbounds nuw i8, ptr %111, i64 112
  %113 = load i64, ptr %112, align 8, !tbaa !44
  %114 = icmp sgt i64 %113, %110
  br i1 %114, label %115, label %138

115:                                              ; preds = %109
  %116 = load i32, ptr %96, align 8, !tbaa !144
  %117 = load ptr, ptr %97, align 8, !tbaa !143
  %118 = getelementptr inbounds nuw i32, ptr %117, i64 %110
  %119 = load i32, ptr %98, align 4, !tbaa !145
  %120 = cmpxchg ptr %118, i32 %116, i32 %119 monotonic monotonic, align 4
  %121 = extractvalue { i32, i1 } %120, 1
  br i1 %121, label %122, label %138

122:                                              ; preds = %115
  %123 = atomicrmw add ptr %99, i32 1 monotonic, align 4
  %124 = icmp slt i32 %123, 0
  br i1 %124, label %128, label %125

125:                                              ; preds = %122
  %126 = load i32, ptr %100, align 8, !tbaa !141
  %127 = icmp slt i32 %123, %126
  br i1 %127, label %129, label %128

128:                                              ; preds = %125, %122
  tail call void @abort() #22
  unreachable

129:                                              ; preds = %125
  %130 = load ptr, ptr %101, align 8, !tbaa !146
  %131 = icmp eq ptr %130, null
  br i1 %131, label %134, label %132

132:                                              ; preds = %129
  %133 = getelementptr inbounds nuw i32, ptr %130, i64 %110
  store i32 %6, ptr %133, align 4, !tbaa !22
  br label %134

134:                                              ; preds = %132, %129
  %135 = load ptr, ptr %102, align 8, !tbaa !140
  %136 = zext nneg i32 %123 to i64
  %137 = getelementptr inbounds nuw i32, ptr %135, i64 %136
  store i32 %107, ptr %137, align 4, !tbaa !22
  br label %138

138:                                              ; preds = %134, %115, %109, %103
  %139 = add nsw i64 %104, 1
  %140 = load ptr, ptr %82, align 8, !tbaa !52
  %141 = getelementptr inbounds nuw i64, ptr %140, i64 %10
  %142 = getelementptr inbounds nuw i8, ptr %141, i64 8
  %143 = load i64, ptr %142, align 8, !tbaa !11
  %144 = icmp slt i64 %139, %143
  br i1 %144, label %103, label %415, !llvm.loop !153

145:                                              ; preds = %14
  %146 = getelementptr inbounds nuw i8, ptr %7, i64 160
  %147 = load ptr, ptr %146, align 8, !tbaa !56
  %148 = icmp eq ptr %147, null
  br i1 %148, label %415, label %149

149:                                              ; preds = %145
  %150 = getelementptr inbounds nuw i8, ptr %7, i64 168
  %151 = load ptr, ptr %150, align 8, !tbaa !57
  %152 = icmp eq ptr %151, null
  br i1 %152, label %415, label %153

153:                                              ; preds = %149
  %154 = getelementptr inbounds nuw i8, ptr %7, i64 176
  %155 = load i32, ptr %154, align 8, !tbaa !55
  %156 = icmp sgt i32 %155, 0
  br i1 %156, label %157, label %415

157:                                              ; preds = %153
  %158 = udiv i32 %6, %155
  %159 = urem i32 %6, %155
  %160 = zext nneg i32 %158 to i64
  %161 = getelementptr inbounds nuw i32, ptr %147, i64 %160
  %162 = load i32, ptr %161, align 4, !tbaa !22
  %163 = getelementptr inbounds nuw i8, ptr %161, i64 4
  %164 = load i32, ptr %163, align 4, !tbaa !22
  %165 = icmp slt i32 %162, %164
  br i1 %165, label %166, label %415

166:                                              ; preds = %157
  %167 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %168 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %169 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %170 = getelementptr inbounds nuw i8, ptr %1, i64 28
  %171 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %172 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %173 = getelementptr inbounds nuw i8, ptr %1, i64 16
  %174 = sext i32 %162 to i64
  br label %175

175:                                              ; preds = %216, %166
  %176 = phi i64 [ %174, %166 ], [ %217, %216 ]
  %177 = load ptr, ptr %150, align 8, !tbaa !57
  %178 = getelementptr inbounds i32, ptr %177, i64 %176
  %179 = load i32, ptr %178, align 4, !tbaa !22
  %180 = icmp eq i32 %179, %159
  br i1 %180, label %181, label %214

181:                                              ; preds = %175
  %182 = getelementptr i8, ptr %178, i64 4
  %183 = load i32, ptr %182, align 4, !tbaa !22
  %184 = icmp slt i32 %183, 0
  br i1 %184, label %216, label %185

185:                                              ; preds = %181
  %186 = zext nneg i32 %183 to i64
  %187 = load ptr, ptr %1, align 8, !tbaa !137
  %188 = getelementptr inbounds nuw i8, ptr %187, i64 112
  %189 = load i64, ptr %188, align 8, !tbaa !44
  %190 = icmp sgt i64 %189, %186
  br i1 %190, label %191, label %216

191:                                              ; preds = %185
  %192 = load i32, ptr %167, align 8, !tbaa !144
  %193 = load ptr, ptr %168, align 8, !tbaa !143
  %194 = getelementptr inbounds nuw i32, ptr %193, i64 %186
  %195 = load i32, ptr %169, align 4, !tbaa !145
  %196 = cmpxchg ptr %194, i32 %192, i32 %195 monotonic monotonic, align 4
  %197 = extractvalue { i32, i1 } %196, 1
  br i1 %197, label %198, label %216

198:                                              ; preds = %191
  %199 = atomicrmw add ptr %170, i32 1 monotonic, align 4
  %200 = icmp slt i32 %199, 0
  br i1 %200, label %204, label %201

201:                                              ; preds = %198
  %202 = load i32, ptr %171, align 8, !tbaa !141
  %203 = icmp slt i32 %199, %202
  br i1 %203, label %205, label %204

204:                                              ; preds = %201, %198
  tail call void @abort() #22
  unreachable

205:                                              ; preds = %201
  %206 = load ptr, ptr %172, align 8, !tbaa !146
  %207 = icmp eq ptr %206, null
  br i1 %207, label %210, label %208

208:                                              ; preds = %205
  %209 = getelementptr inbounds nuw i32, ptr %206, i64 %186
  store i32 %6, ptr %209, align 4, !tbaa !22
  br label %210

210:                                              ; preds = %208, %205
  %211 = load ptr, ptr %173, align 8, !tbaa !140
  %212 = zext nneg i32 %199 to i64
  %213 = getelementptr inbounds nuw i32, ptr %211, i64 %212
  store i32 %183, ptr %213, align 4, !tbaa !22
  br label %216

214:                                              ; preds = %175
  %215 = icmp sgt i32 %179, %159
  br i1 %215, label %415, label %216

216:                                              ; preds = %214, %210, %191, %185, %181
  %217 = add nsw i64 %176, 2
  %218 = load ptr, ptr %146, align 8, !tbaa !56
  %219 = getelementptr inbounds nuw i32, ptr %218, i64 %160
  %220 = getelementptr inbounds nuw i8, ptr %219, i64 4
  %221 = load i32, ptr %220, align 4, !tbaa !22
  %222 = sext i32 %221 to i64
  %223 = icmp slt i64 %217, %222
  br i1 %223, label %175, label %415, !llvm.loop !154

224:                                              ; preds = %14
  %225 = getelementptr inbounds nuw i8, ptr %7, i64 24
  %226 = load ptr, ptr %225, align 8, !tbaa !48
  %227 = getelementptr inbounds nuw i8, ptr %7, i64 32
  %228 = load ptr, ptr %227, align 8, !tbaa !49
  %229 = icmp ne ptr %226, null
  %230 = icmp ne ptr %228, null
  %231 = select i1 %229, i1 %230, i1 false
  br i1 %231, label %232, label %320

232:                                              ; preds = %224
  %233 = getelementptr inbounds nuw i8, ptr %7, i64 40
  %234 = load i64, ptr %233, align 8, !tbaa !63
  %235 = icmp sgt i64 %234, 0
  br i1 %235, label %236, label %320

236:                                              ; preds = %232
  %237 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %238 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %239 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %240 = getelementptr inbounds nuw i8, ptr %1, i64 28
  %241 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %242 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %243 = getelementptr inbounds nuw i8, ptr %1, i64 16
  br label %244

244:                                              ; preds = %316, %236
  %245 = phi i64 [ 0, %236 ], [ %317, %316 ]
  %246 = trunc i64 %245 to i32
  %247 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef nonnull %226, i32 noundef %246) #19
  br i1 %247, label %248, label %316

248:                                              ; preds = %244
  %249 = getelementptr inbounds nuw %struct.EdgePair, ptr %228, i64 %245
  %250 = load i32, ptr %249, align 4, !tbaa !3
  %251 = icmp eq i32 %250, %6
  %252 = getelementptr inbounds nuw i8, ptr %249, i64 4
  %253 = load i32, ptr %252, align 4, !tbaa !8
  br i1 %251, label %254, label %281

254:                                              ; preds = %248
  %255 = icmp slt i32 %253, 0
  br i1 %255, label %316, label %256

256:                                              ; preds = %254
  %257 = zext nneg i32 %253 to i64
  %258 = load ptr, ptr %1, align 8, !tbaa !137
  %259 = getelementptr inbounds nuw i8, ptr %258, i64 112
  %260 = load i64, ptr %259, align 8, !tbaa !44
  %261 = icmp sgt i64 %260, %257
  br i1 %261, label %262, label %316

262:                                              ; preds = %256
  %263 = load i32, ptr %237, align 8, !tbaa !144
  %264 = load ptr, ptr %238, align 8, !tbaa !143
  %265 = getelementptr inbounds nuw i32, ptr %264, i64 %257
  %266 = load i32, ptr %239, align 4, !tbaa !145
  %267 = cmpxchg ptr %265, i32 %263, i32 %266 monotonic monotonic, align 4
  %268 = extractvalue { i32, i1 } %267, 1
  br i1 %268, label %269, label %316

269:                                              ; preds = %262
  %270 = atomicrmw add ptr %240, i32 1 monotonic, align 4
  %271 = icmp slt i32 %270, 0
  br i1 %271, label %275, label %272

272:                                              ; preds = %269
  %273 = load i32, ptr %241, align 8, !tbaa !141
  %274 = icmp slt i32 %270, %273
  br i1 %274, label %276, label %275

275:                                              ; preds = %272, %269
  tail call void @abort() #22
  unreachable

276:                                              ; preds = %272
  %277 = load ptr, ptr %242, align 8, !tbaa !146
  %278 = icmp eq ptr %277, null
  br i1 %278, label %310, label %279

279:                                              ; preds = %276
  %280 = getelementptr inbounds nuw i32, ptr %277, i64 %257
  store i32 %6, ptr %280, align 4, !tbaa !22
  br label %310

281:                                              ; preds = %248
  %282 = icmp ne i32 %253, %6
  %283 = icmp slt i32 %250, 0
  %284 = or i1 %283, %282
  br i1 %284, label %316, label %285

285:                                              ; preds = %281
  %286 = zext nneg i32 %250 to i64
  %287 = load ptr, ptr %1, align 8, !tbaa !137
  %288 = getelementptr inbounds nuw i8, ptr %287, i64 112
  %289 = load i64, ptr %288, align 8, !tbaa !44
  %290 = icmp sgt i64 %289, %286
  br i1 %290, label %291, label %316

291:                                              ; preds = %285
  %292 = load i32, ptr %237, align 8, !tbaa !144
  %293 = load ptr, ptr %238, align 8, !tbaa !143
  %294 = getelementptr inbounds nuw i32, ptr %293, i64 %286
  %295 = load i32, ptr %239, align 4, !tbaa !145
  %296 = cmpxchg ptr %294, i32 %292, i32 %295 monotonic monotonic, align 4
  %297 = extractvalue { i32, i1 } %296, 1
  br i1 %297, label %298, label %316

298:                                              ; preds = %291
  %299 = atomicrmw add ptr %240, i32 1 monotonic, align 4
  %300 = icmp slt i32 %299, 0
  br i1 %300, label %304, label %301

301:                                              ; preds = %298
  %302 = load i32, ptr %241, align 8, !tbaa !141
  %303 = icmp slt i32 %299, %302
  br i1 %303, label %305, label %304

304:                                              ; preds = %301, %298
  tail call void @abort() #22
  unreachable

305:                                              ; preds = %301
  %306 = load ptr, ptr %242, align 8, !tbaa !146
  %307 = icmp eq ptr %306, null
  br i1 %307, label %310, label %308

308:                                              ; preds = %305
  %309 = getelementptr inbounds nuw i32, ptr %306, i64 %286
  store i32 %6, ptr %309, align 4, !tbaa !22
  br label %310

310:                                              ; preds = %308, %305, %279, %276
  %311 = phi i32 [ %270, %279 ], [ %270, %276 ], [ %299, %308 ], [ %299, %305 ]
  %312 = phi i32 [ %253, %279 ], [ %253, %276 ], [ %250, %308 ], [ %250, %305 ]
  %313 = load ptr, ptr %243, align 8, !tbaa !140
  %314 = zext nneg i32 %311 to i64
  %315 = getelementptr inbounds nuw i32, ptr %313, i64 %314
  store i32 %312, ptr %315, align 4, !tbaa !22
  br label %316

316:                                              ; preds = %310, %291, %285, %281, %262, %256, %254, %244
  %317 = add nuw nsw i64 %245, 1
  %318 = load i64, ptr %233, align 8, !tbaa !63
  %319 = icmp slt i64 %317, %318
  br i1 %319, label %244, label %320, !llvm.loop !155

320:                                              ; preds = %316, %232, %224
  %321 = getelementptr inbounds nuw i8, ptr %7, i64 64
  %322 = load i64, ptr %321, align 8, !tbaa !60
  %323 = icmp sgt i64 %322, 0
  br i1 %323, label %324, label %415

324:                                              ; preds = %320
  %325 = getelementptr inbounds nuw i8, ptr %7, i64 56
  %326 = getelementptr inbounds nuw i8, ptr %7, i64 48
  %327 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %328 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %329 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %330 = getelementptr inbounds nuw i8, ptr %1, i64 28
  %331 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %332 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %333 = getelementptr inbounds nuw i8, ptr %1, i64 16
  br label %334

334:                                              ; preds = %411, %324
  %335 = phi i64 [ 0, %324 ], [ %412, %411 ]
  %336 = load ptr, ptr %325, align 8, !tbaa !64
  %337 = getelementptr inbounds nuw i8, ptr %336, i64 %335
  %338 = load i8, ptr %337, align 1, !tbaa !65
  %339 = icmp eq i8 %338, 0
  br i1 %339, label %411, label %340

340:                                              ; preds = %334
  %341 = load ptr, ptr %326, align 8, !tbaa !69
  %342 = shl nuw nsw i64 %335, 1
  %343 = getelementptr inbounds nuw i32, ptr %341, i64 %342
  %344 = load i32, ptr %343, align 4, !tbaa !22
  %345 = or disjoint i64 %342, 1
  %346 = getelementptr inbounds nuw i32, ptr %341, i64 %345
  %347 = load i32, ptr %346, align 4, !tbaa !22
  %348 = icmp eq i32 %344, %6
  br i1 %348, label %349, label %376

349:                                              ; preds = %340
  %350 = icmp slt i32 %347, 0
  br i1 %350, label %411, label %351

351:                                              ; preds = %349
  %352 = zext nneg i32 %347 to i64
  %353 = load ptr, ptr %1, align 8, !tbaa !137
  %354 = getelementptr inbounds nuw i8, ptr %353, i64 112
  %355 = load i64, ptr %354, align 8, !tbaa !44
  %356 = icmp sgt i64 %355, %352
  br i1 %356, label %357, label %411

357:                                              ; preds = %351
  %358 = load i32, ptr %327, align 8, !tbaa !144
  %359 = load ptr, ptr %328, align 8, !tbaa !143
  %360 = getelementptr inbounds nuw i32, ptr %359, i64 %352
  %361 = load i32, ptr %329, align 4, !tbaa !145
  %362 = cmpxchg ptr %360, i32 %358, i32 %361 monotonic monotonic, align 4
  %363 = extractvalue { i32, i1 } %362, 1
  br i1 %363, label %364, label %411

364:                                              ; preds = %357
  %365 = atomicrmw add ptr %330, i32 1 monotonic, align 4
  %366 = icmp slt i32 %365, 0
  br i1 %366, label %370, label %367

367:                                              ; preds = %364
  %368 = load i32, ptr %331, align 8, !tbaa !141
  %369 = icmp slt i32 %365, %368
  br i1 %369, label %371, label %370

370:                                              ; preds = %367, %364
  tail call void @abort() #22
  unreachable

371:                                              ; preds = %367
  %372 = load ptr, ptr %332, align 8, !tbaa !146
  %373 = icmp eq ptr %372, null
  br i1 %373, label %405, label %374

374:                                              ; preds = %371
  %375 = getelementptr inbounds nuw i32, ptr %372, i64 %352
  store i32 %6, ptr %375, align 4, !tbaa !22
  br label %405

376:                                              ; preds = %340
  %377 = icmp ne i32 %347, %6
  %378 = icmp slt i32 %344, 0
  %379 = or i1 %378, %377
  br i1 %379, label %411, label %380

380:                                              ; preds = %376
  %381 = zext nneg i32 %344 to i64
  %382 = load ptr, ptr %1, align 8, !tbaa !137
  %383 = getelementptr inbounds nuw i8, ptr %382, i64 112
  %384 = load i64, ptr %383, align 8, !tbaa !44
  %385 = icmp sgt i64 %384, %381
  br i1 %385, label %386, label %411

386:                                              ; preds = %380
  %387 = load i32, ptr %327, align 8, !tbaa !144
  %388 = load ptr, ptr %328, align 8, !tbaa !143
  %389 = getelementptr inbounds nuw i32, ptr %388, i64 %381
  %390 = load i32, ptr %329, align 4, !tbaa !145
  %391 = cmpxchg ptr %389, i32 %387, i32 %390 monotonic monotonic, align 4
  %392 = extractvalue { i32, i1 } %391, 1
  br i1 %392, label %393, label %411

393:                                              ; preds = %386
  %394 = atomicrmw add ptr %330, i32 1 monotonic, align 4
  %395 = icmp slt i32 %394, 0
  br i1 %395, label %399, label %396

396:                                              ; preds = %393
  %397 = load i32, ptr %331, align 8, !tbaa !141
  %398 = icmp slt i32 %394, %397
  br i1 %398, label %400, label %399

399:                                              ; preds = %396, %393
  tail call void @abort() #22
  unreachable

400:                                              ; preds = %396
  %401 = load ptr, ptr %332, align 8, !tbaa !146
  %402 = icmp eq ptr %401, null
  br i1 %402, label %405, label %403

403:                                              ; preds = %400
  %404 = getelementptr inbounds nuw i32, ptr %401, i64 %381
  store i32 %6, ptr %404, align 4, !tbaa !22
  br label %405

405:                                              ; preds = %403, %400, %374, %371
  %406 = phi i32 [ %365, %374 ], [ %365, %371 ], [ %394, %403 ], [ %394, %400 ]
  %407 = phi i32 [ %347, %374 ], [ %347, %371 ], [ %344, %403 ], [ %344, %400 ]
  %408 = load ptr, ptr %333, align 8, !tbaa !140
  %409 = zext nneg i32 %406 to i64
  %410 = getelementptr inbounds nuw i32, ptr %408, i64 %409
  store i32 %407, ptr %410, align 4, !tbaa !22
  br label %411

411:                                              ; preds = %405, %386, %380, %376, %357, %351, %349, %334
  %412 = add nuw nsw i64 %335, 1
  %413 = load i64, ptr %321, align 8, !tbaa !60
  %414 = icmp slt i64 %412, %413
  br i1 %414, label %334, label %415, !llvm.loop !156

415:                                              ; preds = %214, %216, %138, %74, %411, %2, %9, %17, %21, %25, %81, %85, %89, %145, %149, %153, %157, %320
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_init(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4, ptr noundef %5) local_unnamed_addr #1 {
  %7 = load i32, ptr @g_meta_count, align 4, !tbaa !22
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %9, label %21

9:                                                ; preds = %6
  %10 = zext nneg i32 %7 to i64
  br label %14

11:                                               ; preds = %14
  %12 = add nuw nsw i64 %15, 1
  %13 = icmp eq i64 %12, %10
  br i1 %13, label %19, label %14, !llvm.loop !38

14:                                               ; preds = %11, %9
  %15 = phi i64 [ 0, %9 ], [ %12, %11 ]
  %16 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %15
  %17 = load ptr, ptr %16, align 8, !tbaa !39
  %18 = icmp eq ptr %17, %0
  br i1 %18, label %26, label %11

19:                                               ; preds = %11
  %20 = icmp sgt i32 %7, 63
  br i1 %20, label %74, label %21

21:                                               ; preds = %19, %6
  %22 = add nsw i32 %7, 1
  store i32 %22, ptr @g_meta_count, align 4, !tbaa !22
  %23 = sext i32 %7 to i64
  %24 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %23
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(184) %25, i8 0, i64 176, i1 false)
  store ptr %0, ptr %24, align 8, !tbaa !39
  store i32 3, ptr %25, align 8, !tbaa !50
  br label %26

26:                                               ; preds = %14, %21
  %27 = phi i32 [ %22, %21 ], [ %7, %14 ]
  %28 = phi ptr [ %24, %21 ], [ %16, %14 ]
  %29 = icmp sgt i32 %27, 0
  br i1 %29, label %30, label %52

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
  br i1 %38, label %52, label %32, !llvm.loop !112

39:                                               ; preds = %32
  %40 = and i64 %33, 4294967295
  %41 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %40
  %42 = load ptr, ptr %41, align 8, !tbaa !61
  %43 = icmp eq ptr %42, null
  br i1 %43, label %46, label %44

44:                                               ; preds = %39
  %45 = load ptr, ptr %42, align 8, !tbaa !37
  tail call void @free(ptr noundef %45) #19
  tail call void @free(ptr noundef nonnull %42) #19
  store ptr null, ptr %41, align 8, !tbaa !61
  br label %46

46:                                               ; preds = %39, %44
  %47 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %40
  %48 = load ptr, ptr %47, align 8, !tbaa !61
  %49 = icmp eq ptr %48, null
  br i1 %49, label %52, label %50

50:                                               ; preds = %46
  %51 = load ptr, ptr %48, align 8, !tbaa !37
  tail call void @free(ptr noundef %51) #19
  tail call void @free(ptr noundef nonnull %48) #19
  store ptr null, ptr %47, align 8, !tbaa !61
  br label %52

52:                                               ; preds = %36, %26, %50, %46
  %53 = getelementptr inbounds nuw i8, ptr %28, i64 16
  store ptr %3, ptr %53, align 8, !tbaa !47
  %54 = getelementptr inbounds nuw i8, ptr %28, i64 24
  store ptr %4, ptr %54, align 8, !tbaa !48
  %55 = getelementptr inbounds nuw i8, ptr %28, i64 32
  store ptr %5, ptr %55, align 8, !tbaa !49
  %56 = icmp sgt i64 %2, 1
  %57 = zext i1 %56 to i64
  %58 = lshr i64 %2, %57
  %59 = getelementptr inbounds nuw i8, ptr %28, i64 40
  store i64 %58, ptr %59, align 8, !tbaa !63
  %60 = getelementptr inbounds nuw i8, ptr %28, i64 64
  %61 = getelementptr inbounds nuw i8, ptr %28, i64 48
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %60, i8 0, i64 16, i1 false)
  %62 = load ptr, ptr %61, align 8, !tbaa !69
  tail call void @free(ptr noundef %62) #19
  %63 = getelementptr inbounds nuw i8, ptr %28, i64 56
  %64 = load ptr, ptr %63, align 8, !tbaa !64
  tail call void @free(ptr noundef %64) #19
  %65 = getelementptr inbounds nuw i8, ptr %28, i64 112
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %61, i8 0, i64 16, i1 false)
  store i64 %1, ptr %65, align 8, !tbaa !44
  %66 = getelementptr inbounds nuw i8, ptr %28, i64 120
  store i64 %2, ptr %66, align 8, !tbaa !45
  %67 = load i64, ptr %59, align 8, !tbaa !63
  %68 = getelementptr inbounds nuw i8, ptr %28, i64 88
  store i64 %67, ptr %68, align 8, !tbaa !58
  %69 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %70 = getelementptr inbounds nuw i8, ptr %28, i64 96
  %71 = load <2 x ptr>, ptr %69, align 8, !tbaa !103
  store <2 x ptr> %71, ptr %70, align 8, !tbaa !103
  %72 = getelementptr inbounds nuw i8, ptr %28, i64 128
  store i32 0, ptr %72, align 8, !tbaa !51
  %73 = getelementptr inbounds nuw i8, ptr %28, i64 8
  store i32 0, ptr %73, align 8, !tbaa !50
  br label %74

74:                                               ; preds = %19, %52
  ret void
}

declare dso_local i64 @roaring_bitmap_get_cardinality(ptr noundef) local_unnamed_addr #3

declare dso_local i32 @roaring_bitmap_get_at_index(ptr noundef, i64 noundef) local_unnamed_addr #3

; Function Attrs: nounwind
declare dso_local i32 @clock_gettime(i32 noundef, ptr noundef) local_unnamed_addr #15

; Function Attrs: nounwind
declare dso_local i32 @pthread_key_create(ptr noundef, ptr noundef) local_unnamed_addr #15

; Function Attrs: nounwind uwtable
define internal void @autograph_neighbor_scratch_destroy(ptr noundef %0) #1 {
  %2 = icmp eq ptr %0, null
  br i1 %2, label %17, label %3

3:                                                ; preds = %1
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %5 = load i64, ptr %4, align 8, !tbaa !125
  %6 = icmp eq i64 %5, 0
  br i1 %6, label %7, label %9

7:                                                ; preds = %9, %3
  %8 = load ptr, ptr %0, align 8, !tbaa !126
  tail call void @free(ptr noundef %8) #19
  tail call void @free(ptr noundef nonnull %0) #19
  br label %17

9:                                                ; preds = %3, %9
  %10 = phi i64 [ %14, %9 ], [ 0, %3 ]
  %11 = load ptr, ptr %0, align 8, !tbaa !126
  %12 = getelementptr inbounds nuw %struct.AutoNeighborScratchSlot, ptr %11, i64 %10
  %13 = load ptr, ptr %12, align 8, !tbaa !129
  tail call void @free(ptr noundef %13) #19
  %14 = add nuw i64 %10, 1
  %15 = load i64, ptr %4, align 8, !tbaa !125
  %16 = icmp ult i64 %14, %15
  br i1 %16, label %9, label %7, !llvm.loop !157

17:                                               ; preds = %1, %7
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #17

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #17

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #17

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #17

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>) #17

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x i64> @llvm.smax.v2i64(<2 x i64>, <2 x i64>) #17

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #7 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #11 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #12 = { nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #13 = { nofree nounwind memory(readwrite, argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #14 = { cold nofree noreturn nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #15 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #16 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #17 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #18 = { nounwind allocsize(0,1) }
attributes #19 = { nounwind }
attributes #20 = { nounwind allocsize(0) }
attributes #21 = { nounwind allocsize(1) }
attributes #22 = { cold noreturn nounwind }

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
!34 = !{!35, !12, i64 8}
!35 = !{!"EdgeHashMap_s", !36, i64 0, !12, i64 8}
!36 = !{!"p1 _ZTS15EdgeHashEntry_s", !19, i64 0}
!37 = !{!35, !36, i64 0}
!38 = distinct !{!38, !10}
!39 = !{!40, !19, i64 0}
!40 = !{!"", !19, i64 0, !5, i64 8, !19, i64 16, !19, i64 24, !19, i64 32, !12, i64 40, !21, i64 48, !41, i64 56, !12, i64 64, !12, i64 72, !5, i64 80, !12, i64 88, !18, i64 96, !21, i64 104, !12, i64 112, !12, i64 120, !5, i64 128, !18, i64 136, !21, i64 144, !12, i64 152, !21, i64 160, !21, i64 168, !5, i64 176, !5, i64 180}
!41 = !{!"p1 omnipotent char", !19, i64 0}
!42 = !{!40, !18, i64 96}
!43 = !{!40, !21, i64 104}
!44 = !{!40, !12, i64 112}
!45 = !{!40, !12, i64 120}
!46 = !{!40, !5, i64 80}
!47 = !{!40, !19, i64 16}
!48 = !{!40, !19, i64 24}
!49 = !{!40, !19, i64 32}
!50 = !{!40, !5, i64 8}
!51 = !{!40, !5, i64 128}
!52 = !{!40, !18, i64 136}
!53 = !{!40, !21, i64 144}
!54 = !{!40, !5, i64 180}
!55 = !{!40, !5, i64 176}
!56 = !{!40, !21, i64 160}
!57 = !{!40, !21, i64 168}
!58 = !{!40, !12, i64 88}
!59 = !{!40, !12, i64 72}
!60 = !{!40, !12, i64 64}
!61 = !{!62, !62, i64 0}
!62 = !{!"p1 _ZTS13EdgeHashMap_s", !19, i64 0}
!63 = !{!40, !12, i64 40}
!64 = !{!40, !41, i64 56}
!65 = !{!6, !6, i64 0}
!66 = distinct !{!66, !10}
!67 = distinct !{!67, !10}
!68 = distinct !{!68, !10}
!69 = !{!40, !21, i64 48}
!70 = distinct !{!70, !10}
!71 = distinct !{!71, !10, !72, !73}
!72 = !{!"llvm.loop.isvectorized", i32 1}
!73 = !{!"llvm.loop.unroll.runtime.disable"}
!74 = distinct !{!74, !10}
!75 = distinct !{!75, !10, !73, !72}
!76 = distinct !{!76, !15}
!77 = distinct !{!77, !10}
!78 = distinct !{!78, !10}
!79 = distinct !{!79, !10}
!80 = distinct !{!80, !10}
!81 = distinct !{!81, !10}
!82 = distinct !{!82, !15}
!83 = distinct !{!83, !10}
!84 = distinct !{!84, !10}
!85 = distinct !{!85, !10}
!86 = distinct !{!86, !10, !72, !73}
!87 = distinct !{!87, !10}
!88 = distinct !{!88, !10, !73, !72}
!89 = distinct !{!89, !10}
!90 = distinct !{!90, !10}
!91 = distinct !{!91, !10, !72, !73}
!92 = distinct !{!92, !10, !72}
!93 = distinct !{!93, !10, !72, !73}
!94 = distinct !{!94, !10, !73, !72}
!95 = distinct !{!95, !10}
!96 = distinct !{!96, !10}
!97 = distinct !{!97, !10}
!98 = distinct !{!98, !10, !72, !73}
!99 = distinct !{!99, !10, !73, !72}
!100 = !{!101, !12, i64 0}
!101 = !{!"timespec", !12, i64 0, !12, i64 8}
!102 = !{!101, !12, i64 8}
!103 = !{!19, !19, i64 0}
!104 = distinct !{!104, !10}
!105 = distinct !{!105, !10}
!106 = distinct !{!106, !15}
!107 = distinct !{!107, !10}
!108 = distinct !{!108, !10}
!109 = distinct !{!109, !10}
!110 = distinct !{!110, !10}
!111 = distinct !{!111, !10}
!112 = distinct !{!112, !10}
!113 = distinct !{!113, !10}
!114 = distinct !{!114, !10}
!115 = distinct !{!115, !10}
!116 = distinct !{!116, !10}
!117 = distinct !{!117, !10}
!118 = distinct !{!118, !10, !72, !73}
!119 = distinct !{!119, !10, !73, !72}
!120 = distinct !{!120, !10}
!121 = distinct !{!121, !10, !72, !73}
!122 = distinct !{!122, !10, !73, !72}
!123 = !{!124, !12, i64 16}
!124 = !{!"", !19, i64 0, !12, i64 8, !12, i64 16}
!125 = !{!124, !12, i64 8}
!126 = !{!124, !19, i64 0}
!127 = !{!128, !12, i64 8}
!128 = !{!"", !21, i64 0, !12, i64 8}
!129 = !{!128, !21, i64 0}
!130 = distinct !{!130, !10, !72, !73}
!131 = distinct !{!131, !15}
!132 = distinct !{!132, !10, !72}
!133 = distinct !{!133, !10}
!134 = distinct !{!134, !10}
!135 = distinct !{!135, !10}
!136 = distinct !{!136, !10}
!137 = !{!138, !19, i64 0}
!138 = !{!"", !19, i64 0, !21, i64 8, !21, i64 16, !5, i64 24, !6, i64 28, !21, i64 32, !5, i64 40, !5, i64 44, !21, i64 48, !41, i64 56}
!139 = !{!138, !21, i64 8}
!140 = !{!138, !21, i64 16}
!141 = !{!138, !5, i64 24}
!142 = !{!138, !6, i64 28}
!143 = !{!138, !21, i64 32}
!144 = !{!138, !5, i64 40}
!145 = !{!138, !5, i64 44}
!146 = !{!138, !21, i64 48}
!147 = !{!138, !41, i64 56}
!148 = distinct !{!148, !10}
!149 = distinct !{!149, !10}
!150 = distinct !{!150, !10}
!151 = distinct !{!151, !10}
!152 = distinct !{!152, !10}
!153 = distinct !{!153, !10}
!154 = distinct !{!154, !10}
!155 = distinct !{!155, !10}
!156 = distinct !{!156, !10}
!157 = distinct !{!157, !10}
