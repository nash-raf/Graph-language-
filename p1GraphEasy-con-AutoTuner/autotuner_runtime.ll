; ModuleID = 'autotuner_runtime.c'
source_filename = "autotuner_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.AutoProfileRegion = type { i32, i32, i32, double, i64, i64 }
%struct.AutoGraphMeta = type { ptr, i32, ptr, ptr, ptr, i64, ptr, ptr, i64, i64, i32, i64, ptr, ptr, ptr, i64, i64, i32, ptr, ptr, i64, ptr, ptr, i32, i32, i32, ptr, i32, i64, ptr, ptr, ptr, i32, [12 x double], i8, [12 x double], i8 }
%struct.timespec = type { i64, i64 }
%struct.EdgePair = type { i32, i32 }
%struct.EdgeHashEntry_s = type { i64, i64, i8 }
%struct.AutoFrontierStepEnv = type { ptr, ptr, ptr, i32, ptr, i32, i32, ptr, ptr }
%struct.AutoFrontierLane = type { ptr, i32, i32 }
%struct.AutoMotifFrontierEnv = type { ptr, ptr, ptr, i32, ptr, ptr, ptr, i32, i32, ptr, ptr }
%struct.AutoFrontierMergeEnv = type { ptr, ptr, ptr }

@g_profile_atexit_once = internal global i32 0, align 4
@g_profile_regions = internal global [1024 x %struct.AutoProfileRegion] zeroinitializer, align 16
@g_active_region_id = internal thread_local unnamed_addr global i32 -1, align 4
@g_active_region_start_ns = internal thread_local unnamed_addr global i64 0, align 8
@g_kernel_measured_ns = internal global [3 x i64] zeroinitializer, align 16
@g_conversion_ns = internal unnamed_addr global i64 0, align 8
@g_conversions_injected = internal unnamed_addr global i32 0, align 4
@g_meta_count = internal unnamed_addr global i32 0, align 4
@g_meta = internal global [64 x %struct.AutoGraphMeta] zeroinitializer, align 16
@shift_prof_reloc_cnt = internal unnamed_addr global i64 0, align 8
@shift_prof_reloc_bytes = internal unnamed_addr global i64 0, align 8
@shift_prof_ns = internal unnamed_addr global i64 0, align 8
@shift_prof_cnt = internal unnamed_addr global i64 0, align 8
@shift_prof_bytes = internal unnamed_addr global i64 0, align 8
@g_neighbor_scan_start_ns = internal thread_local unnamed_addr global i64 0, align 8
@g_static_edge_hash = internal unnamed_addr global [64 x ptr] zeroinitializer, align 16
@g_extra_edge_hash = internal unnamed_addr global [64 x ptr] zeroinitializer, align 16
@stderr = external local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [55 x i8] c"[AutoTunerProfile] predicted_vs_measured_region_times\0A\00", align 1
@.str.1 = private unnamed_addr constant [130 x i8] c"[AutoTunerProfile] region=%d kind=%s layout=%s visits=%llu predicted_ns=%.3f measured_ns=%llu predicted_ms=%.6f measured_ms=%.6f\0A\00", align 1
@.str.2 = private unnamed_addr constant [144 x i8] c"[AutoTunerProfile] total kind=%s predicted_ns=%.3f measured_ns=%llu predicted_ms=%.6f measured_ms=%.6f pure_kernel_ns=%llu pure_kernel_ms=%.6f\0A\00", align 1
@.str.3 = private unnamed_addr constant [77 x i8] c"[AutoTunerProfile] injected %d layout conversions total, conversion_ns=%llu\0A\00", align 1
@.str.4 = private unnamed_addr constant [9 x i8] c"Traverse\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"Insert\00", align 1
@.str.6 = private unnamed_addr constant [6 x i8] c"Query\00", align 1
@.str.7 = private unnamed_addr constant [8 x i8] c"Unknown\00", align 1
@.str.8 = private unnamed_addr constant [4 x i8] c"CSR\00", align 1
@.str.9 = private unnamed_addr constant [5 x i8] c"PCSR\00", align 1
@.str.10 = private unnamed_addr constant [5 x i8] c"BCSR\00", align 1
@.str.11 = private unnamed_addr constant [4 x i8] c"SET\00", align 1
@.str.12 = private unnamed_addr constant [8 x i8] c"UNKNOWN\00", align 1
@shift_prof_state = internal unnamed_addr global i32 -1, align 4
@.str.13 = private unnamed_addr constant [24 x i8] c"AUTOTUNER_SHIFT_PROFILE\00", align 1
@.str.14 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@.str.15 = private unnamed_addr constant [124 x i8] c"{\22shift_ns\22: %lld, \22shift_cnt\22: %lld, \22shift_bytes\22: %lld, \22shift_per_line\22: %.4f, \22reloc_cnt\22: %lld, \22reloc_bytes\22: %lld}\0A\00", align 1
@.str.16 = private unnamed_addr constant [19 x i8] c"SGPL_FRONTIER_MODE\00", align 1
@.str.17 = private unnamed_addr constant [5 x i8] c"push\00", align 1
@.str.18 = private unnamed_addr constant [5 x i8] c"pull\00", align 1
@switch.table.autograph_profile_report.rel = private unnamed_addr constant [3 x i32] [i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.4 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.rel to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.5 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.rel to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.6 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.rel to i64)) to i32)], align 4
@switch.table.autograph_profile_report.19.rel = private unnamed_addr constant [4 x i32] [i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.8 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.19.rel to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.9 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.19.rel to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.10 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.19.rel to i64)) to i32), i32 trunc (i64 sub (i64 ptrtoint (ptr @.str.11 to i64), i64 ptrtoint (ptr @switch.table.autograph_profile_report.19.rel to i64)) to i32)], align 4

; Function Attrs: nounwind uwtable
define dso_local void @autograph_profile_region_enter(i32 noundef %0, i32 noundef %1, i32 noundef %2, double noundef %3) local_unnamed_addr #0 {
  %5 = alloca %struct.timespec, align 8
  %6 = alloca %struct.timespec, align 8
  %7 = icmp ugt i32 %0, 1023
  br i1 %7, label %56, label %8

8:                                                ; preds = %4
  %9 = tail call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %10 = zext nneg i32 %0 to i64
  %11 = getelementptr inbounds nuw [1024 x %struct.AutoProfileRegion], ptr @g_profile_regions, i64 0, i64 %10
  %12 = cmpxchg ptr %11, i32 0, i32 -1 acq_rel acquire, align 8
  %13 = extractvalue { i32, i1 } %12, 1
  br i1 %13, label %14, label %18

14:                                               ; preds = %8
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 4
  store i32 %1, ptr %15, align 4, !tbaa !5
  %16 = getelementptr inbounds nuw i8, ptr %11, i64 8
  store i32 %2, ptr %16, align 8, !tbaa !11
  %17 = getelementptr inbounds nuw i8, ptr %11, i64 16
  store double %3, ptr %17, align 8, !tbaa !12
  store atomic i32 1, ptr %11 release, align 8
  br label %21

18:                                               ; preds = %8, %18
  %19 = load atomic i32, ptr %11 acquire, align 8
  %20 = icmp eq i32 %19, 1
  br i1 %20, label %21, label %18, !llvm.loop !13

21:                                               ; preds = %18, %14
  %22 = getelementptr inbounds nuw i8, ptr %11, i64 32
  %23 = atomicrmw add ptr %22, i64 1 monotonic, align 8
  %24 = tail call align 4 ptr @llvm.threadlocal.address.p0(ptr align 4 @g_active_region_id)
  %25 = load i32, ptr %24, align 4, !tbaa !15
  %26 = icmp eq i32 %25, %0
  br i1 %26, label %56, label %27

27:                                               ; preds = %21
  %28 = icmp ugt i32 %25, 1023
  br i1 %28, label %29, label %31

29:                                               ; preds = %27
  %30 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_active_region_start_ns)
  br label %48

31:                                               ; preds = %27
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #28
  %32 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #28
  %33 = load i64, ptr %6, align 8, !tbaa !16
  %34 = mul i64 %33, 1000000000
  %35 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %36 = load i64, ptr %35, align 8, !tbaa !19
  %37 = add i64 %34, %36
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #28
  %38 = call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_active_region_start_ns)
  %39 = load i64, ptr %38, align 8, !tbaa !20
  %40 = icmp ult i64 %37, %39
  br i1 %40, label %47, label %41

41:                                               ; preds = %31
  %42 = load i32, ptr %24, align 4, !tbaa !15
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds [1024 x %struct.AutoProfileRegion], ptr @g_profile_regions, i64 0, i64 %43, i32 4
  %45 = sub nuw i64 %37, %39
  %46 = atomicrmw add ptr %44, i64 %45 monotonic, align 8
  br label %47

47:                                               ; preds = %41, %31
  store i64 0, ptr %38, align 8, !tbaa !20
  br label %48

48:                                               ; preds = %29, %47
  %49 = phi ptr [ %30, %29 ], [ %38, %47 ]
  store i32 %0, ptr %24, align 4, !tbaa !15
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #28
  %50 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #28
  %51 = load i64, ptr %5, align 8, !tbaa !16
  %52 = mul i64 %51, 1000000000
  %53 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %54 = load i64, ptr %53, align 8, !tbaa !19
  %55 = add i64 %52, %54
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #28
  store i64 %55, ptr %49, align 8, !tbaa !20
  br label %56

56:                                               ; preds = %48, %21, %4
  ret void
}

declare i32 @pthread_once(ptr noundef, ptr noundef) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define internal void @autograph_profile_install_atexit() #2 {
  %1 = tail call i32 @atexit(ptr noundef nonnull @autograph_profile_report) #28
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare nonnull ptr @llvm.threadlocal.address.p0(ptr nonnull) #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nounwind uwtable
define dso_local void @autograph_profile_region_exit(i32 noundef %0) local_unnamed_addr #0 {
  %2 = alloca %struct.timespec, align 8
  %3 = icmp ugt i32 %0, 1023
  %4 = tail call align 4 ptr @llvm.threadlocal.address.p0(ptr align 4 @g_active_region_id)
  %5 = load i32, ptr %4, align 4
  %6 = icmp ne i32 %5, %0
  %7 = icmp ugt i32 %5, 1023
  %8 = or i1 %6, %7
  %9 = select i1 %3, i1 true, i1 %8
  br i1 %9, label %27, label %10

10:                                               ; preds = %1
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %2) #28
  %11 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %2) #28
  %12 = load i64, ptr %2, align 8, !tbaa !16
  %13 = mul i64 %12, 1000000000
  %14 = getelementptr inbounds nuw i8, ptr %2, i64 8
  %15 = load i64, ptr %14, align 8, !tbaa !19
  %16 = add i64 %13, %15
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %2) #28
  %17 = call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_active_region_start_ns)
  %18 = load i64, ptr %17, align 8, !tbaa !20
  %19 = icmp ult i64 %16, %18
  br i1 %19, label %26, label %20

20:                                               ; preds = %10
  %21 = load i32, ptr %4, align 4, !tbaa !15
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds [1024 x %struct.AutoProfileRegion], ptr @g_profile_regions, i64 0, i64 %22, i32 4
  %24 = sub nuw i64 %16, %18
  %25 = atomicrmw add ptr %23, i64 %24 monotonic, align 8
  br label %26

26:                                               ; preds = %20, %10
  store i32 -1, ptr %4, align 4, !tbaa !15
  store i64 0, ptr %17, align 8, !tbaa !20
  br label %27

27:                                               ; preds = %26, %1
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_profile_record_kernel_ns(i32 noundef %0, i64 noundef %1) local_unnamed_addr #0 {
  %3 = icmp ugt i32 %0, 2
  br i1 %3, label %9, label %4

4:                                                ; preds = %2
  %5 = tail call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %6 = zext nneg i32 %0 to i64
  %7 = getelementptr inbounds nuw [3 x i64], ptr @g_kernel_measured_ns, i64 0, i64 %6
  %8 = atomicrmw add ptr %7, i64 %1 monotonic, align 8
  br label %9

9:                                                ; preds = %2, %4
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @build_csr_from_set(i64 noundef %0, i64 noundef %1, ptr noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = icmp sgt i64 %1, 0
  br i1 %8, label %15, label %9

9:                                                ; preds = %7
  %10 = tail call noalias dereferenceable_or_null(8) ptr @calloc(i64 noundef 1, i64 noundef 8) #29
  br label %104

11:                                               ; preds = %29
  %12 = add nsw i64 %30, 2
  %13 = tail call noalias ptr @calloc(i64 noundef %12, i64 noundef 8) #29
  %14 = getelementptr inbounds nuw i8, ptr %13, i64 8
  br label %45

15:                                               ; preds = %7, %29
  %16 = phi i64 [ %30, %29 ], [ -1, %7 ]
  %17 = phi i64 [ %31, %29 ], [ 0, %7 ]
  %18 = trunc i64 %17 to i32
  %19 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %18) #28
  br i1 %19, label %20, label %29

20:                                               ; preds = %15
  %21 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %17
  %22 = load i32, ptr %21, align 4, !tbaa !21
  %23 = sext i32 %22 to i64
  %24 = tail call i64 @llvm.smax.i64(i64 %16, i64 %23)
  %25 = getelementptr inbounds nuw i8, ptr %21, i64 4
  %26 = load i32, ptr %25, align 4, !tbaa !23
  %27 = sext i32 %26 to i64
  %28 = tail call i64 @llvm.smax.i64(i64 %24, i64 %27)
  br label %29

29:                                               ; preds = %20, %15
  %30 = phi i64 [ %16, %15 ], [ %28, %20 ]
  %31 = add nuw nsw i64 %17, 1
  %32 = icmp eq i64 %31, %1
  br i1 %32, label %11, label %15, !llvm.loop !24

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
  %49 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %48) #28
  br i1 %49, label %50, label %62

50:                                               ; preds = %45
  %51 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %46
  %52 = load i32, ptr %51, align 4, !tbaa !21
  %53 = icmp slt i32 %52, 0
  %54 = zext nneg i32 %52 to i64
  %55 = icmp slt i64 %30, %54
  %56 = select i1 %53, i1 true, i1 %55
  br i1 %56, label %62, label %57

57:                                               ; preds = %50
  %58 = getelementptr inbounds nuw i64, ptr %14, i64 %54
  %59 = load i64, ptr %58, align 8, !tbaa !20
  %60 = add nsw i64 %59, 1
  store i64 %60, ptr %58, align 8, !tbaa !20
  %61 = add nsw i64 %47, 1
  br label %62

62:                                               ; preds = %45, %50, %57
  %63 = phi i64 [ %61, %57 ], [ %47, %50 ], [ %47, %45 ]
  %64 = add nuw nsw i64 %46, 1
  %65 = icmp eq i64 %64, %1
  br i1 %65, label %33, label %45, !llvm.loop !25

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
  %75 = load i64, ptr %74, align 8, !tbaa !20
  %76 = add nsw i64 %75, %71
  store i64 %76, ptr %74, align 8, !tbaa !20
  %77 = add nuw i64 %72, 1
  %78 = add i64 %73, 1
  %79 = icmp eq i64 %78, %38
  br i1 %79, label %80, label %70, !llvm.loop !26

80:                                               ; preds = %66, %70, %33
  %81 = icmp sgt i64 %63, 0
  br i1 %81, label %101, label %104

82:                                               ; preds = %82, %40
  %83 = phi i64 [ %36, %40 ], [ %97, %82 ]
  %84 = phi i64 [ 1, %40 ], [ %98, %82 ]
  %85 = phi i64 [ 0, %40 ], [ %99, %82 ]
  %86 = getelementptr i64, ptr %13, i64 %84
  %87 = load i64, ptr %86, align 8, !tbaa !20
  %88 = add nsw i64 %87, %83
  store i64 %88, ptr %86, align 8, !tbaa !20
  %89 = getelementptr i64, ptr %42, i64 %84
  %90 = load i64, ptr %89, align 8, !tbaa !20
  %91 = add nsw i64 %90, %88
  store i64 %91, ptr %89, align 8, !tbaa !20
  %92 = getelementptr i64, ptr %43, i64 %84
  %93 = load i64, ptr %92, align 8, !tbaa !20
  %94 = add nsw i64 %93, %91
  store i64 %94, ptr %92, align 8, !tbaa !20
  %95 = getelementptr i64, ptr %44, i64 %84
  %96 = load i64, ptr %95, align 8, !tbaa !20
  %97 = add nsw i64 %96, %94
  store i64 %97, ptr %95, align 8, !tbaa !20
  %98 = add nuw i64 %84, 4
  %99 = add i64 %85, 4
  %100 = icmp eq i64 %99, %41
  br i1 %100, label %66, label %82, !llvm.loop !28

101:                                              ; preds = %80
  %102 = shl i64 %63, 2
  %103 = tail call noalias ptr @malloc(i64 noundef %102) #30
  br label %104

104:                                              ; preds = %9, %80, %101
  %105 = phi ptr [ %13, %101 ], [ %13, %80 ], [ %10, %9 ]
  %106 = phi i64 [ %12, %101 ], [ %12, %80 ], [ 1, %9 ]
  %107 = phi i64 [ %30, %101 ], [ %30, %80 ], [ -1, %9 ]
  %108 = phi i64 [ %63, %101 ], [ %63, %80 ], [ 0, %9 ]
  %109 = phi ptr [ %103, %101 ], [ null, %80 ], [ null, %9 ]
  %110 = shl nuw nsw i64 %106, 3
  %111 = tail call noalias ptr @malloc(i64 noundef %110) #30
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %111, ptr align 8 %105, i64 %110, i1 false)
  br i1 %8, label %113, label %112

112:                                              ; preds = %131, %104
  tail call void @free(ptr noundef %111) #28
  store ptr %105, ptr %4, align 8, !tbaa !29
  store ptr %109, ptr %5, align 8, !tbaa !32
  store i64 %108, ptr %6, align 8, !tbaa !20
  ret void

113:                                              ; preds = %104, %131
  %114 = phi i64 [ %132, %131 ], [ 0, %104 ]
  %115 = trunc i64 %114 to i32
  %116 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %2, i32 noundef %115) #28
  br i1 %116, label %117, label %131

117:                                              ; preds = %113
  %118 = getelementptr inbounds nuw %struct.EdgePair, ptr %3, i64 %114
  %119 = load i32, ptr %118, align 4, !tbaa !21
  %120 = icmp sgt i32 %119, -1
  br i1 %120, label %121, label %131

121:                                              ; preds = %117
  %122 = zext nneg i32 %119 to i64
  %123 = icmp slt i64 %107, %122
  br i1 %123, label %131, label %124

124:                                              ; preds = %121
  %125 = getelementptr inbounds nuw i8, ptr %118, i64 4
  %126 = load i32, ptr %125, align 4, !tbaa !23
  %127 = getelementptr inbounds nuw i64, ptr %111, i64 %122
  %128 = load i64, ptr %127, align 8, !tbaa !20
  %129 = add nsw i64 %128, 1
  store i64 %129, ptr %127, align 8, !tbaa !20
  %130 = getelementptr inbounds i32, ptr %109, i64 %128
  store i32 %126, ptr %130, align 4, !tbaa !15
  br label %131

131:                                              ; preds = %113, %117, %121, %124
  %132 = add nuw nsw i64 %114, 1
  %133 = icmp eq i64 %132, %1
  br i1 %133, label %112, label %113, !llvm.loop !34
}

declare zeroext i1 @roaring_bitmap_contains(ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

; Function Attrs: mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite)
declare void @free(ptr allocptr nocapture noundef) local_unnamed_addr #8

; Function Attrs: nounwind uwtable
define dso_local void @rebuild_sets_from_csr(i64 noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readonly %3, ptr nocapture noundef readonly %4, ptr noundef %5, ptr noundef %6, ptr nocapture noundef readonly %7) local_unnamed_addr #0 {
  tail call void @roaring_bitmap_clear(ptr noundef %5) #28
  tail call void @roaring_bitmap_clear(ptr noundef %6) #28
  %9 = trunc i64 %1 to i32
  %10 = shl i32 %9, 1
  %11 = add i32 %10, 32
  br label %12

12:                                               ; preds = %12, %8
  %13 = phi i32 [ 1, %8 ], [ %15, %12 ]
  %14 = icmp sgt i32 %13, %11
  %15 = shl i32 %13, 1
  br i1 %14, label %16, label %12, !llvm.loop !35

16:                                               ; preds = %12
  %17 = sext i32 %13 to i64
  %18 = tail call noalias ptr @calloc(i64 noundef %17, i64 noundef 24) #29
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
  %32 = load i32, ptr %31, align 4, !tbaa !21
  %33 = getelementptr inbounds nuw i8, ptr %31, i64 4
  %34 = load i32, ptr %33, align 4, !tbaa !23
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
  %52 = load i8, ptr %51, align 8, !tbaa !36
  %53 = icmp eq i8 %52, 0
  br i1 %53, label %68, label %54

54:                                               ; preds = %29, %61
  %55 = phi ptr [ %64, %61 ], [ %50, %29 ]
  %56 = phi i64 [ %63, %61 ], [ %49, %29 ]
  %57 = load i64, ptr %55, align 8, !tbaa !38
  %58 = icmp eq i64 %57, %40
  br i1 %58, label %59, label %61

59:                                               ; preds = %54
  %60 = getelementptr inbounds nuw i8, ptr %55, i64 8
  store i64 %30, ptr %60, align 8, !tbaa !39
  br label %73

61:                                               ; preds = %54
  %62 = add i64 %56, 1
  %63 = and i64 %62, %21
  %64 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %63
  %65 = getelementptr inbounds nuw i8, ptr %64, i64 16
  %66 = load i8, ptr %65, align 8, !tbaa !36
  %67 = icmp eq i8 %66, 0
  br i1 %67, label %68, label %54, !llvm.loop !40

68:                                               ; preds = %61, %29
  %69 = phi i64 [ %49, %29 ], [ %63, %61 ]
  %70 = phi ptr [ %50, %29 ], [ %64, %61 ]
  store i64 %40, ptr %70, align 8, !tbaa !38
  %71 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %69, i32 1
  store i64 %30, ptr %71, align 8, !tbaa !39
  %72 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %69, i32 2
  store i8 1, ptr %72, align 8, !tbaa !36
  br label %73

73:                                               ; preds = %59, %68
  %74 = add nuw nsw i64 %30, 1
  %75 = icmp eq i64 %74, %1
  br i1 %75, label %22, label %29, !llvm.loop !41

76:                                               ; preds = %132, %79
  %77 = icmp eq i64 %83, %0
  br i1 %77, label %78, label %79, !llvm.loop !42

78:                                               ; preds = %76, %22
  tail call void @free(ptr noundef %18) #28
  ret void

79:                                               ; preds = %24, %76
  %80 = phi i64 [ 0, %24 ], [ %83, %76 ]
  %81 = getelementptr inbounds nuw i64, ptr %3, i64 %80
  %82 = load i64, ptr %81, align 8, !tbaa !20
  %83 = add nuw nsw i64 %80, 1
  %84 = getelementptr inbounds nuw i64, ptr %3, i64 %83
  %85 = load i64, ptr %84, align 8, !tbaa !20
  %86 = icmp sgt i64 %85, %82
  br i1 %86, label %87, label %76

87:                                               ; preds = %79
  %88 = trunc i64 %80 to i32
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %88) #28
  %89 = trunc i64 %80 to i32
  br label %90

90:                                               ; preds = %87, %132
  %91 = phi i64 [ %82, %87 ], [ %133, %132 ]
  %92 = getelementptr inbounds i32, ptr %4, i64 %91
  %93 = load i32, ptr %92, align 4, !tbaa !15
  tail call void @roaring_bitmap_add(ptr noundef %5, i32 noundef %93) #28
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
  %112 = load i8, ptr %111, align 8, !tbaa !36
  %113 = icmp eq i8 %112, 0
  br i1 %113, label %132, label %121

114:                                              ; preds = %121
  %115 = add i64 %123, 1
  %116 = and i64 %115, %28
  %117 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %18, i64 %116
  %118 = getelementptr inbounds nuw i8, ptr %117, i64 16
  %119 = load i8, ptr %118, align 8, !tbaa !36
  %120 = icmp eq i8 %119, 0
  br i1 %120, label %132, label %121, !llvm.loop !43

121:                                              ; preds = %94, %114
  %122 = phi ptr [ %117, %114 ], [ %110, %94 ]
  %123 = phi i64 [ %116, %114 ], [ %109, %94 ]
  %124 = load i64, ptr %122, align 8, !tbaa !38
  %125 = icmp eq i64 %124, %100
  br i1 %125, label %126, label %114

126:                                              ; preds = %121
  %127 = getelementptr inbounds nuw i8, ptr %122, i64 8
  %128 = load i64, ptr %127, align 8, !tbaa !39
  %129 = icmp sgt i64 %128, -1
  br i1 %129, label %130, label %132

130:                                              ; preds = %126
  %131 = trunc i64 %128 to i32
  tail call void @roaring_bitmap_add(ptr noundef %6, i32 noundef %131) #28
  br label %132

132:                                              ; preds = %114, %94, %90, %130, %126
  %133 = add i64 %91, 1
  %134 = icmp eq i64 %133, %85
  br i1 %134, label %76, label %90, !llvm.loop !44
}

declare void @roaring_bitmap_clear(ptr noundef) local_unnamed_addr #1

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define internal fastcc void @edge_hash_insert(ptr nocapture noundef readonly %0, i32 noundef %1, i32 noundef %2, i64 noundef %3) unnamed_addr #9 {
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
  %20 = load i64, ptr %19, align 8, !tbaa !45
  %21 = add nsw i64 %20, -1
  %22 = load ptr, ptr %0, align 8, !tbaa !48
  %23 = and i64 %18, %21
  %24 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %23
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 16
  %26 = load i8, ptr %25, align 8, !tbaa !36
  %27 = icmp eq i8 %26, 0
  br i1 %27, label %42, label %28

28:                                               ; preds = %4, %35
  %29 = phi ptr [ %38, %35 ], [ %24, %4 ]
  %30 = phi i64 [ %37, %35 ], [ %23, %4 ]
  %31 = load i64, ptr %29, align 8, !tbaa !38
  %32 = icmp eq i64 %31, %10
  br i1 %32, label %33, label %35

33:                                               ; preds = %28
  %34 = getelementptr inbounds nuw i8, ptr %29, i64 8
  store i64 %3, ptr %34, align 8, !tbaa !39
  br label %47

35:                                               ; preds = %28
  %36 = add i64 %30, 1
  %37 = and i64 %36, %21
  %38 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %37
  %39 = getelementptr inbounds nuw i8, ptr %38, i64 16
  %40 = load i8, ptr %39, align 8, !tbaa !36
  %41 = icmp eq i8 %40, 0
  br i1 %41, label %42, label %28, !llvm.loop !40

42:                                               ; preds = %35, %4
  %43 = phi i64 [ %23, %4 ], [ %37, %35 ]
  %44 = phi ptr [ %24, %4 ], [ %38, %35 ]
  store i64 %10, ptr %44, align 8, !tbaa !38
  %45 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %43, i32 1
  store i64 %3, ptr %45, align 8, !tbaa !39
  %46 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %22, i64 %43, i32 2
  store i8 1, ptr %46, align 8, !tbaa !36
  br label %47

47:                                               ; preds = %42, %33
  ret void
}

declare void @roaring_bitmap_add(ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_update_csr_pointers(ptr noundef readnone %0, ptr noundef %1, ptr noundef %2) local_unnamed_addr #10 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %23

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %23, label %11, !llvm.loop !49

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 16, !tbaa !50
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %18 = load i32, ptr %17, align 8, !tbaa !53
  %19 = icmp eq i32 %18, 0
  br i1 %19, label %20, label %23

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 96
  store ptr %1, ptr %21, align 8, !tbaa !54
  %22 = getelementptr inbounds nuw i8, ptr %13, i64 104
  store ptr %2, ptr %22, align 8, !tbaa !55
  br label %23

23:                                               ; preds = %8, %3, %20, %16
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_record_adjacency_state(ptr noundef readnone %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4) local_unnamed_addr #10 {
  %6 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %7 = icmp sgt i32 %6, 0
  br i1 %7, label %8, label %27

8:                                                ; preds = %5
  %9 = zext nneg i32 %6 to i64
  br label %13

10:                                               ; preds = %13
  %11 = add nuw nsw i64 %14, 1
  %12 = icmp eq i64 %11, %9
  br i1 %12, label %27, label %13, !llvm.loop !49

13:                                               ; preds = %10, %8
  %14 = phi i64 [ 0, %8 ], [ %11, %10 ]
  %15 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %14
  %16 = load ptr, ptr %15, align 16, !tbaa !50
  %17 = icmp eq ptr %16, %0
  br i1 %17, label %18, label %10

18:                                               ; preds = %13
  %19 = getelementptr inbounds nuw i8, ptr %15, i64 8
  %20 = load i32, ptr %19, align 8, !tbaa !53
  %21 = icmp eq i32 %20, 0
  br i1 %21, label %22, label %27

22:                                               ; preds = %18
  %23 = getelementptr inbounds nuw i8, ptr %15, i64 120
  store i64 %1, ptr %23, align 8, !tbaa !56
  %24 = getelementptr inbounds nuw i8, ptr %15, i64 128
  store i64 %2, ptr %24, align 8, !tbaa !57
  %25 = getelementptr inbounds nuw i8, ptr %15, i64 96
  store ptr %3, ptr %25, align 8, !tbaa !54
  %26 = getelementptr inbounds nuw i8, ptr %15, i64 104
  store ptr %4, ptr %26, align 8, !tbaa !55
  br label %27

27:                                               ; preds = %10, %5, %18, %22
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @autograph_mark_canonical_dirty(ptr noundef readnone %0) local_unnamed_addr #10 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %16

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %16, label %9, !llvm.loop !49

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 16, !tbaa !50
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 80
  store i32 1, ptr %15, align 8, !tbaa !58
  br label %16

16:                                               ; preds = %6, %1, %14
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_sync_canonical_if_dirty(ptr noundef %0) local_unnamed_addr #0 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %19

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %19, label %9, !llvm.loop !49

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 16, !tbaa !50
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 80
  %16 = load i32, ptr %15, align 8, !tbaa !58
  %17 = icmp eq i32 %16, 0
  br i1 %17, label %19, label %18

18:                                               ; preds = %14
  tail call void @autograph_ensure_layout_set(ptr noundef %0)
  br label %19

19:                                               ; preds = %6, %1, %14, %18
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout_set(ptr noundef writeonly %0) local_unnamed_addr #0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %364

7:                                                ; preds = %1
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %364, label %12, !llvm.loop !49

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 16, !tbaa !50
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 16
  %19 = load ptr, ptr %18, align 8, !tbaa !59
  %20 = icmp eq ptr %19, null
  br i1 %20, label %364, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %14, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !60
  %24 = icmp eq ptr %23, null
  br i1 %24, label %364, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %14, i64 32
  %27 = load ptr, ptr %26, align 8, !tbaa !61
  %28 = icmp eq ptr %27, null
  br i1 %28, label %364, label %29

29:                                               ; preds = %25
  %30 = getelementptr inbounds nuw i8, ptr %14, i64 8
  %31 = load i32, ptr %30, align 8, !tbaa !53
  %32 = icmp eq i32 %31, 3
  br i1 %32, label %33, label %38

33:                                               ; preds = %29
  %34 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %35 = load i32, ptr %34, align 8, !tbaa !58
  %36 = icmp eq i32 %35, 0
  br i1 %36, label %364, label %37

37:                                               ; preds = %33
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #28
  store ptr null, ptr %2, align 8, !tbaa !29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #28
  store ptr null, ptr %3, align 8, !tbaa !32
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %4) #28
  store i64 0, ptr %4, align 8, !tbaa !20
  br label %75

38:                                               ; preds = %29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #28
  store ptr null, ptr %2, align 8, !tbaa !29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #28
  store ptr null, ptr %3, align 8, !tbaa !32
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %4) #28
  store i64 0, ptr %4, align 8, !tbaa !20
  switch i32 %31, label %75 [
    i32 0, label %39
    i32 1, label %51
    i32 2, label %61
  ]

39:                                               ; preds = %38
  %40 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %41 = load ptr, ptr %40, align 8, !tbaa !54
  store ptr %41, ptr %2, align 8, !tbaa !29
  %42 = getelementptr inbounds nuw i8, ptr %14, i64 104
  %43 = load ptr, ptr %42, align 8, !tbaa !55
  store ptr %43, ptr %3, align 8, !tbaa !32
  %44 = getelementptr inbounds nuw i8, ptr %14, i64 120
  %45 = load i64, ptr %44, align 8, !tbaa !56
  %46 = getelementptr inbounds nuw i8, ptr %14, i64 128
  %47 = load i64, ptr %46, align 8, !tbaa !57
  store i64 %47, ptr %4, align 8, !tbaa !20
  %48 = getelementptr inbounds nuw i8, ptr %14, i64 136
  %49 = load i32, ptr %48, align 8, !tbaa !62
  %50 = icmp ne i32 %49, 0
  br label %75

51:                                               ; preds = %38
  %52 = getelementptr inbounds nuw i8, ptr %14, i64 120
  %53 = load i64, ptr %52, align 8, !tbaa !56
  %54 = getelementptr inbounds nuw i8, ptr %14, i64 144
  %55 = load ptr, ptr %54, align 8, !tbaa !63
  %56 = getelementptr inbounds nuw i8, ptr %14, i64 152
  %57 = load ptr, ptr %56, align 8, !tbaa !64
  call void @convert_pcsr_to_csr(i64 noundef %53, i64 poison, ptr noundef %55, ptr noundef %57, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %58 = load i64, ptr %52, align 8, !tbaa !56
  %59 = load ptr, ptr %54, align 8, !tbaa !63
  tail call void @free(ptr noundef %59) #28
  store ptr null, ptr %54, align 8, !tbaa !63
  %60 = load ptr, ptr %56, align 8, !tbaa !64
  tail call void @free(ptr noundef %60) #28
  store ptr null, ptr %56, align 8, !tbaa !64
  br label %75

61:                                               ; preds = %38
  %62 = getelementptr inbounds nuw i8, ptr %14, i64 120
  %63 = load i64, ptr %62, align 8, !tbaa !56
  %64 = getelementptr inbounds nuw i8, ptr %14, i64 188
  %65 = load i32, ptr %64, align 4, !tbaa !65
  %66 = getelementptr inbounds nuw i8, ptr %14, i64 184
  %67 = load i32, ptr %66, align 8, !tbaa !66
  %68 = getelementptr inbounds nuw i8, ptr %14, i64 168
  %69 = load ptr, ptr %68, align 8, !tbaa !67
  %70 = getelementptr inbounds nuw i8, ptr %14, i64 176
  %71 = load ptr, ptr %70, align 8, !tbaa !68
  call void @convert_bcsr_to_csr(i64 noundef %63, i32 noundef %65, i32 noundef %67, ptr noundef %69, ptr noundef %71, ptr noundef nonnull %2, ptr noundef nonnull %3, ptr noundef nonnull %4)
  %72 = load i64, ptr %62, align 8, !tbaa !56
  %73 = load ptr, ptr %68, align 8, !tbaa !67
  call void @free(ptr noundef %73) #28
  store ptr null, ptr %68, align 8, !tbaa !67
  %74 = load ptr, ptr %70, align 8, !tbaa !68
  call void @free(ptr noundef %74) #28
  store ptr null, ptr %70, align 8, !tbaa !68
  br label %75

75:                                               ; preds = %37, %38, %51, %61, %39
  %76 = phi i64 [ %45, %39 ], [ %58, %51 ], [ %72, %61 ], [ 0, %38 ], [ 0, %37 ]
  %77 = phi i1 [ %50, %39 ], [ true, %51 ], [ true, %61 ], [ false, %38 ], [ false, %37 ]
  %78 = load ptr, ptr %2, align 8, !tbaa !29
  %79 = icmp ne ptr %78, null
  %80 = load ptr, ptr %3, align 8
  %81 = icmp ne ptr %80, null
  %82 = select i1 %79, i1 %81, i1 false
  br i1 %82, label %83, label %342

83:                                               ; preds = %75
  %84 = getelementptr inbounds nuw i8, ptr %14, i64 88
  store i64 -1, ptr %84, align 8, !tbaa !69
  %85 = load ptr, ptr %18, align 8, !tbaa !59
  %86 = load ptr, ptr %22, align 8, !tbaa !60
  %87 = load ptr, ptr %26, align 8, !tbaa !61
  call void @roaring_bitmap_clear(ptr noundef %85) #28
  call void @roaring_bitmap_clear(ptr noundef %86) #28
  %88 = getelementptr inbounds nuw i8, ptr %14, i64 72
  %89 = load i64, ptr %88, align 8, !tbaa !70
  %90 = icmp sgt i64 %89, 0
  br i1 %90, label %91, label %93

91:                                               ; preds = %83
  %92 = getelementptr inbounds nuw i8, ptr %14, i64 56
  br label %123

93:                                               ; preds = %123, %83
  %94 = getelementptr inbounds nuw i8, ptr %14, i64 64
  store i64 0, ptr %94, align 8, !tbaa !71
  %95 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %96 = icmp slt i32 %95, 1
  %97 = zext nneg i32 %95 to i64
  %98 = add nsw i64 %97, -1
  %99 = icmp ugt i64 %13, %98
  %100 = select i1 %96, i1 true, i1 %99
  br i1 %100, label %107, label %101

101:                                              ; preds = %93
  %102 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %13
  %103 = load ptr, ptr %102, align 8, !tbaa !72
  %104 = icmp eq ptr %103, null
  br i1 %104, label %107, label %105

105:                                              ; preds = %101
  %106 = load ptr, ptr %103, align 8, !tbaa !48
  call void @free(ptr noundef %106) #28
  call void @free(ptr noundef nonnull %103) #28
  store ptr null, ptr %102, align 8, !tbaa !72
  br label %107

107:                                              ; preds = %105, %101, %93
  %108 = getelementptr inbounds nuw i8, ptr %14, i64 40
  %109 = load i64, ptr %108, align 8, !tbaa !74
  %110 = trunc i64 %109 to i32
  %111 = shl i32 %110, 1
  %112 = add i32 %111, 32
  br label %113

113:                                              ; preds = %113, %107
  %114 = phi i32 [ 1, %107 ], [ %116, %113 ]
  %115 = icmp sgt i32 %114, %112
  %116 = shl i32 %114, 1
  br i1 %115, label %117, label %113, !llvm.loop !35

117:                                              ; preds = %113
  %118 = sext i32 %114 to i64
  %119 = call noalias ptr @calloc(i64 noundef %118, i64 noundef 24) #29
  %120 = icmp sgt i64 %109, 0
  br i1 %120, label %121, label %130

121:                                              ; preds = %117
  %122 = add nsw i64 %118, -1
  br label %140

123:                                              ; preds = %123, %91
  %124 = phi i64 [ 0, %91 ], [ %127, %123 ]
  %125 = load ptr, ptr %92, align 8, !tbaa !75
  %126 = getelementptr inbounds nuw i8, ptr %125, i64 %124
  store i8 0, ptr %126, align 1, !tbaa !76
  %127 = add nuw nsw i64 %124, 1
  %128 = load i64, ptr %88, align 8, !tbaa !70
  %129 = icmp slt i64 %127, %128
  br i1 %129, label %123, label %93, !llvm.loop !77

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
  %143 = load i32, ptr %142, align 4, !tbaa !21
  %144 = getelementptr inbounds nuw i8, ptr %142, i64 4
  %145 = load i32, ptr %144, align 4, !tbaa !23
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
  %163 = load i8, ptr %162, align 8, !tbaa !36
  %164 = icmp eq i8 %163, 0
  br i1 %164, label %179, label %165

165:                                              ; preds = %140, %172
  %166 = phi ptr [ %175, %172 ], [ %161, %140 ]
  %167 = phi i64 [ %174, %172 ], [ %160, %140 ]
  %168 = load i64, ptr %166, align 8, !tbaa !38
  %169 = icmp eq i64 %168, %151
  br i1 %169, label %170, label %172

170:                                              ; preds = %165
  %171 = getelementptr inbounds nuw i8, ptr %166, i64 8
  store i64 %141, ptr %171, align 8, !tbaa !39
  br label %184

172:                                              ; preds = %165
  %173 = add i64 %167, 1
  %174 = and i64 %173, %122
  %175 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %174
  %176 = getelementptr inbounds nuw i8, ptr %175, i64 16
  %177 = load i8, ptr %176, align 8, !tbaa !36
  %178 = icmp eq i8 %177, 0
  br i1 %178, label %179, label %165, !llvm.loop !40

179:                                              ; preds = %172, %140
  %180 = phi i64 [ %160, %140 ], [ %174, %172 ]
  %181 = phi ptr [ %161, %140 ], [ %175, %172 ]
  store i64 %151, ptr %181, align 8, !tbaa !38
  %182 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %180, i32 1
  store i64 %141, ptr %182, align 8, !tbaa !39
  %183 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %180, i32 2
  store i8 1, ptr %183, align 8, !tbaa !36
  br label %184

184:                                              ; preds = %179, %170
  %185 = add nuw nsw i64 %141, 1
  %186 = icmp eq i64 %185, %109
  br i1 %186, label %130, label %140, !llvm.loop !78

187:                                              ; preds = %336, %189
  %188 = icmp eq i64 %193, %76
  br i1 %188, label %339, label %189, !llvm.loop !79

189:                                              ; preds = %187, %132
  %190 = phi i64 [ 0, %132 ], [ %193, %187 ]
  %191 = getelementptr inbounds nuw i64, ptr %78, i64 %190
  %192 = load i64, ptr %191, align 8, !tbaa !20
  %193 = add nuw nsw i64 %190, 1
  %194 = getelementptr inbounds nuw i64, ptr %78, i64 %193
  %195 = load i64, ptr %194, align 8, !tbaa !20
  %196 = icmp sgt i64 %195, %192
  br i1 %196, label %197, label %187

197:                                              ; preds = %189
  %198 = trunc i64 %190 to i32
  call void @roaring_bitmap_add(ptr noundef %85, i32 noundef %198) #28
  br label %199

199:                                              ; preds = %336, %197
  %200 = phi i64 [ %192, %197 ], [ %337, %336 ]
  %201 = getelementptr inbounds i32, ptr %80, i64 %200
  %202 = load i32, ptr %201, align 4, !tbaa !15
  call void @roaring_bitmap_add(ptr noundef %85, i32 noundef %202) #28
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
  %224 = load i8, ptr %223, align 8, !tbaa !36
  %225 = icmp eq i8 %224, 0
  br i1 %225, label %244, label %233

226:                                              ; preds = %233
  %227 = add i64 %235, 1
  %228 = and i64 %227, %136
  %229 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %119, i64 %228
  %230 = getelementptr inbounds nuw i8, ptr %229, i64 16
  %231 = load i8, ptr %230, align 8, !tbaa !36
  %232 = icmp eq i8 %231, 0
  br i1 %232, label %244, label %233, !llvm.loop !43

233:                                              ; preds = %206, %226
  %234 = phi ptr [ %229, %226 ], [ %222, %206 ]
  %235 = phi i64 [ %228, %226 ], [ %221, %206 ]
  %236 = load i64, ptr %234, align 8, !tbaa !38
  %237 = icmp eq i64 %236, %212
  br i1 %237, label %238, label %226

238:                                              ; preds = %233
  %239 = getelementptr inbounds nuw i8, ptr %234, i64 8
  %240 = load i64, ptr %239, align 8, !tbaa !39
  %241 = icmp sgt i64 %240, -1
  br i1 %241, label %242, label %244

242:                                              ; preds = %238
  %243 = trunc i64 %240 to i32
  call void @roaring_bitmap_add(ptr noundef %86, i32 noundef %243) #28
  br label %336

244:                                              ; preds = %226, %238, %206, %205
  %245 = call fastcc i64 @canonical_pair_find_extra(ptr noundef nonnull %14, i32 noundef %198, i32 noundef %202)
  %246 = icmp sgt i64 %245, -1
  br i1 %246, label %247, label %250

247:                                              ; preds = %244
  %248 = load ptr, ptr %138, align 8, !tbaa !75
  %249 = getelementptr inbounds nuw i8, ptr %248, i64 %245
  store i8 1, ptr %249, align 1, !tbaa !76
  br label %336

250:                                              ; preds = %244
  %251 = load i64, ptr %94, align 8, !tbaa !71
  %252 = add nsw i64 %251, 1
  %253 = call fastcc i32 @ensure_extra_capacity(ptr noundef nonnull %14, i64 noundef %252)
  %254 = icmp eq i32 %253, 0
  br i1 %254, label %336, label %255

255:                                              ; preds = %250
  %256 = load i64, ptr %94, align 8, !tbaa !71
  %257 = add nsw i64 %256, 1
  store i64 %257, ptr %94, align 8, !tbaa !71
  %258 = load ptr, ptr %137, align 8, !tbaa !80
  %259 = shl nsw i64 %256, 1
  %260 = getelementptr inbounds i32, ptr %258, i64 %259
  store i32 %198, ptr %260, align 4, !tbaa !15
  %261 = or disjoint i64 %259, 1
  %262 = getelementptr inbounds i32, ptr %258, i64 %261
  store i32 %202, ptr %262, align 4, !tbaa !15
  %263 = load ptr, ptr %138, align 8, !tbaa !75
  %264 = getelementptr inbounds i8, ptr %263, i64 %256
  store i8 1, ptr %264, align 1, !tbaa !76
  %265 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %266 = icmp slt i32 %265, 1
  %267 = zext nneg i32 %265 to i64
  %268 = add nsw i64 %267, -1
  %269 = icmp ugt i64 %13, %268
  %270 = select i1 %266, i1 true, i1 %269
  br i1 %270, label %336, label %271

271:                                              ; preds = %255
  %272 = load ptr, ptr %139, align 8, !tbaa !72
  %273 = icmp eq ptr %272, null
  br i1 %273, label %278, label %274

274:                                              ; preds = %271
  %275 = getelementptr inbounds nuw i8, ptr %272, i64 8
  %276 = load i64, ptr %275, align 8, !tbaa !45
  %277 = load ptr, ptr %272, align 8, !tbaa !48
  br label %294

278:                                              ; preds = %271
  %279 = load i64, ptr %88, align 8, !tbaa !70
  %280 = icmp sgt i64 %279, 0
  %281 = call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #30
  %282 = trunc i64 %279 to i32
  %283 = shl i32 %282, 2
  %284 = add i32 %283, 32
  %285 = select i1 %280, i32 %284, i32 128
  br label %286

286:                                              ; preds = %286, %278
  %287 = phi i32 [ 1, %278 ], [ %289, %286 ]
  %288 = icmp sgt i32 %287, %285
  %289 = shl i32 %287, 1
  br i1 %288, label %290, label %286, !llvm.loop !35

290:                                              ; preds = %286
  %291 = sext i32 %287 to i64
  %292 = getelementptr inbounds nuw i8, ptr %281, i64 8
  store i64 %291, ptr %292, align 8, !tbaa !45
  %293 = call noalias ptr @calloc(i64 noundef %291, i64 noundef 24) #29
  store ptr %293, ptr %281, align 8, !tbaa !48
  store ptr %281, ptr %139, align 8, !tbaa !72
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
  %315 = load i8, ptr %314, align 8, !tbaa !36
  %316 = icmp eq i8 %315, 0
  br i1 %316, label %331, label %317

317:                                              ; preds = %294, %324
  %318 = phi ptr [ %327, %324 ], [ %313, %294 ]
  %319 = phi i64 [ %326, %324 ], [ %312, %294 ]
  %320 = load i64, ptr %318, align 8, !tbaa !38
  %321 = icmp eq i64 %320, %302
  br i1 %321, label %322, label %324

322:                                              ; preds = %317
  %323 = getelementptr inbounds nuw i8, ptr %318, i64 8
  store i64 %256, ptr %323, align 8, !tbaa !39
  br label %336

324:                                              ; preds = %317
  %325 = add i64 %319, 1
  %326 = and i64 %325, %311
  %327 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %326
  %328 = getelementptr inbounds nuw i8, ptr %327, i64 16
  %329 = load i8, ptr %328, align 8, !tbaa !36
  %330 = icmp eq i8 %329, 0
  br i1 %330, label %331, label %317, !llvm.loop !40

331:                                              ; preds = %324, %294
  %332 = phi i64 [ %312, %294 ], [ %326, %324 ]
  %333 = phi ptr [ %313, %294 ], [ %327, %324 ]
  store i64 %302, ptr %333, align 8, !tbaa !38
  %334 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %332, i32 1
  store i64 %256, ptr %334, align 8, !tbaa !39
  %335 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %295, i64 %332, i32 2
  store i8 1, ptr %335, align 8, !tbaa !36
  br label %336

336:                                              ; preds = %331, %322, %255, %250, %247, %242, %199
  %337 = add i64 %200, 1
  %338 = icmp eq i64 %337, %195
  br i1 %338, label %187, label %199, !llvm.loop !81

339:                                              ; preds = %187, %130
  call void @free(ptr noundef %119) #28
  %340 = getelementptr inbounds nuw i8, ptr %14, i64 80
  store i32 0, ptr %340, align 8, !tbaa !58
  call fastcc void @refresh_graph_counts_from_canonical(ptr noundef nonnull %14)
  %341 = load ptr, ptr %2, align 8
  br label %342

342:                                              ; preds = %339, %75
  %343 = phi ptr [ %341, %339 ], [ %78, %75 ]
  %344 = icmp ne ptr %343, null
  %345 = select i1 %77, i1 %344, i1 false
  br i1 %345, label %346, label %347

346:                                              ; preds = %342
  call void @free(ptr noundef nonnull %343) #28
  br label %347

347:                                              ; preds = %346, %342
  %348 = load ptr, ptr %3, align 8
  %349 = icmp ne ptr %348, null
  %350 = select i1 %77, i1 %349, i1 false
  br i1 %350, label %351, label %352

351:                                              ; preds = %347
  call void @free(ptr noundef nonnull %348) #28
  br label %352

352:                                              ; preds = %351, %347
  %353 = getelementptr inbounds nuw i8, ptr %14, i64 96
  %354 = getelementptr inbounds nuw i8, ptr %14, i64 136
  store i32 0, ptr %354, align 8, !tbaa !62
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %353, i8 0, i64 16, i1 false)
  call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %14)
  %355 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %355, i8 0, i64 16, i1 false)
  %356 = load i32, ptr %30, align 8, !tbaa !53
  %357 = icmp eq i32 %356, 3
  br i1 %357, label %362, label %358

358:                                              ; preds = %352
  %359 = getelementptr inbounds nuw i8, ptr %14, i64 192
  %360 = load i32, ptr %359, align 8, !tbaa !82
  %361 = add nsw i32 %360, 1
  store i32 %361, ptr %359, align 8, !tbaa !82
  br label %362

362:                                              ; preds = %352, %358
  store i32 3, ptr %30, align 8, !tbaa !53
  %363 = getelementptr inbounds nuw i8, ptr %14, i64 80
  store i32 0, ptr %363, align 8, !tbaa !58
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %4) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #28
  br label %364

364:                                              ; preds = %9, %1, %33, %17, %21, %25, %362
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_pcsr_to_csr(i64 noundef %0, i64 %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = add i64 %0, 1
  %9 = tail call noalias ptr @calloc(i64 noundef %8, i64 noundef 8) #29
  %10 = icmp sgt i64 %0, 0
  br i1 %10, label %11, label %89

11:                                               ; preds = %7
  %12 = load i64, ptr %2, align 8, !tbaa !20
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
  %28 = load i64, ptr %27, align 8, !tbaa !20
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
  %43 = load <2 x i32>, ptr %41, align 4, !tbaa !15
  %44 = load <2 x i32>, ptr %42, align 4, !tbaa !15
  %45 = icmp ne <2 x i32> %43, splat (i32 -1)
  %46 = icmp ne <2 x i32> %44, splat (i32 -1)
  %47 = zext <2 x i1> %45 to <2 x i64>
  %48 = zext <2 x i1> %46 to <2 x i64>
  %49 = add <2 x i64> %39, %47
  %50 = add <2 x i64> %40, %48
  %51 = add nuw i64 %38, 4
  %52 = icmp eq i64 %51, %34
  br i1 %52, label %53, label %37, !llvm.loop !83

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
  store i64 %61, ptr %62, align 8, !tbaa !20
  %63 = add nuw nsw i64 %61, %24
  %64 = icmp eq i64 %26, %0
  br i1 %64, label %13, label %22, !llvm.loop !86

65:                                               ; preds = %57, %65
  %66 = phi i64 [ %72, %65 ], [ %58, %57 ]
  %67 = phi i64 [ %73, %65 ], [ %59, %57 ]
  %68 = getelementptr inbounds i32, ptr %3, i64 %67
  %69 = load i32, ptr %68, align 4, !tbaa !15
  %70 = icmp ne i32 %69, -1
  %71 = zext i1 %70 to i64
  %72 = add nuw nsw i64 %66, %71
  %73 = add nsw i64 %67, 1
  %74 = icmp eq i64 %73, %28
  br i1 %74, label %60, label %65, !llvm.loop !87

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
  %84 = load i64, ptr %83, align 8, !tbaa !20
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !20
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %15
  br i1 %88, label %89, label %79, !llvm.loop !88

89:                                               ; preds = %75, %79, %7
  %90 = phi i64 [ 0, %7 ], [ %63, %79 ], [ %63, %75 ]
  %91 = shl i64 %90, 2
  %92 = tail call noalias ptr @malloc(i64 noundef %91) #30
  %93 = shl i64 %8, 3
  %94 = tail call noalias ptr @malloc(i64 noundef %93) #30
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 8 %94, ptr align 8 %9, i64 %93, i1 false)
  br i1 %10, label %95, label %119

95:                                               ; preds = %89
  %96 = load i64, ptr %2, align 8, !tbaa !20
  %97 = getelementptr i8, ptr %3, i64 4
  br label %120

98:                                               ; preds = %98, %17
  %99 = phi i64 [ %14, %17 ], [ %113, %98 ]
  %100 = phi i64 [ 1, %17 ], [ %114, %98 ]
  %101 = phi i64 [ 0, %17 ], [ %115, %98 ]
  %102 = getelementptr i64, ptr %9, i64 %100
  %103 = load i64, ptr %102, align 8, !tbaa !20
  %104 = add nsw i64 %103, %99
  store i64 %104, ptr %102, align 8, !tbaa !20
  %105 = getelementptr i64, ptr %19, i64 %100
  %106 = load i64, ptr %105, align 8, !tbaa !20
  %107 = add nsw i64 %106, %104
  store i64 %107, ptr %105, align 8, !tbaa !20
  %108 = getelementptr i64, ptr %20, i64 %100
  %109 = load i64, ptr %108, align 8, !tbaa !20
  %110 = add nsw i64 %109, %107
  store i64 %110, ptr %108, align 8, !tbaa !20
  %111 = getelementptr i64, ptr %21, i64 %100
  %112 = load i64, ptr %111, align 8, !tbaa !20
  %113 = add nsw i64 %112, %110
  store i64 %113, ptr %111, align 8, !tbaa !20
  %114 = add nuw i64 %100, 4
  %115 = add i64 %101, 4
  %116 = icmp eq i64 %115, %18
  br i1 %116, label %75, label %98, !llvm.loop !89

117:                                              ; preds = %143, %163, %120
  %118 = icmp eq i64 %123, %0
  br i1 %118, label %119, label %120, !llvm.loop !90

119:                                              ; preds = %117, %89
  tail call void @free(ptr noundef %94) #28
  store ptr %9, ptr %4, align 8, !tbaa !29
  store ptr %92, ptr %5, align 8, !tbaa !32
  store i64 %90, ptr %6, align 8, !tbaa !20
  ret void

120:                                              ; preds = %95, %117
  %121 = phi i64 [ %125, %117 ], [ %96, %95 ]
  %122 = phi i64 [ %123, %117 ], [ 0, %95 ]
  %123 = add nuw nsw i64 %122, 1
  %124 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %125 = load i64, ptr %124, align 8, !tbaa !20
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
  %135 = load i32, ptr %134, align 4, !tbaa !15
  %136 = icmp eq i32 %135, -1
  br i1 %136, label %141, label %137

137:                                              ; preds = %133
  %138 = load i64, ptr %128, align 8, !tbaa !20
  %139 = add nsw i64 %138, 1
  store i64 %139, ptr %128, align 8, !tbaa !20
  %140 = getelementptr inbounds i32, ptr %92, i64 %138
  store i32 %135, ptr %140, align 4, !tbaa !15
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
  %149 = load i32, ptr %148, align 4, !tbaa !15
  %150 = icmp eq i32 %149, -1
  br i1 %150, label %155, label %151

151:                                              ; preds = %146
  %152 = load i64, ptr %128, align 8, !tbaa !20
  %153 = add nsw i64 %152, 1
  store i64 %153, ptr %128, align 8, !tbaa !20
  %154 = getelementptr inbounds i32, ptr %92, i64 %152
  store i32 %149, ptr %154, align 4, !tbaa !15
  br label %155

155:                                              ; preds = %146, %151
  %156 = getelementptr i32, ptr %97, i64 %147
  %157 = load i32, ptr %156, align 4, !tbaa !15
  %158 = icmp eq i32 %157, -1
  br i1 %158, label %163, label %159

159:                                              ; preds = %155
  %160 = load i64, ptr %128, align 8, !tbaa !20
  %161 = add nsw i64 %160, 1
  store i64 %161, ptr %128, align 8, !tbaa !20
  %162 = getelementptr inbounds i32, ptr %92, i64 %160
  store i32 %157, ptr %162, align 4, !tbaa !15
  br label %163

163:                                              ; preds = %159, %155
  %164 = add nsw i64 %147, 2
  %165 = icmp eq i64 %164, %125
  br i1 %165, label %117, label %146, !llvm.loop !91
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
  %22 = tail call noalias ptr @calloc(i64 noundef %21, i64 noundef 8) #29
  %23 = icmp eq ptr %22, null
  br i1 %23, label %167, label %24

24:                                               ; preds = %20
  %25 = icmp eq i32 %1, 0
  br i1 %25, label %33, label %26

26:                                               ; preds = %24
  %27 = zext nneg i32 %2 to i64
  %28 = getelementptr i8, ptr %22, i64 8
  %29 = zext nneg i32 %1 to i64
  %30 = load i32, ptr %3, align 4, !tbaa !15
  br label %44

31:                                               ; preds = %71, %44
  %32 = icmp eq i64 %50, %29
  br i1 %32, label %33, label %44, !llvm.loop !92

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
  %52 = load i32, ptr %51, align 4, !tbaa !15
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
  %61 = load i32, ptr %60, align 4, !tbaa !15
  %62 = sext i32 %61 to i64
  %63 = add nsw i64 %47, %62
  %64 = icmp sgt i32 %61, -1
  %65 = icmp slt i64 %63, %49
  %66 = select i1 %64, i1 %65, i1 false
  br i1 %66, label %67, label %71

67:                                               ; preds = %57
  %68 = getelementptr i64, ptr %28, i64 %63
  %69 = load i64, ptr %68, align 8, !tbaa !20
  %70 = add nsw i64 %69, 1
  store i64 %70, ptr %68, align 8, !tbaa !20
  br label %71

71:                                               ; preds = %67, %57
  %72 = add nsw i64 %58, 2
  %73 = add i32 %59, 3
  %74 = icmp slt i32 %73, %52
  br i1 %74, label %57, label %31, !llvm.loop !93

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
  %84 = load i64, ptr %83, align 8, !tbaa !20
  %85 = add nsw i64 %84, %80
  store i64 %85, ptr %83, align 8, !tbaa !20
  %86 = add nuw i64 %81, 1
  %87 = add i64 %82, 1
  %88 = icmp eq i64 %87, %37
  br i1 %88, label %89, label %79, !llvm.loop !94

89:                                               ; preds = %75, %79, %33
  %90 = getelementptr inbounds i64, ptr %22, i64 %0
  %91 = load i64, ptr %90, align 8, !tbaa !20
  %92 = icmp sgt i64 %91, 0
  br i1 %92, label %112, label %117

93:                                               ; preds = %93, %39
  %94 = phi i64 [ %36, %39 ], [ %108, %93 ]
  %95 = phi i64 [ 1, %39 ], [ %109, %93 ]
  %96 = phi i64 [ 0, %39 ], [ %110, %93 ]
  %97 = getelementptr i64, ptr %22, i64 %95
  %98 = load i64, ptr %97, align 8, !tbaa !20
  %99 = add nsw i64 %98, %94
  store i64 %99, ptr %97, align 8, !tbaa !20
  %100 = getelementptr i64, ptr %41, i64 %95
  %101 = load i64, ptr %100, align 8, !tbaa !20
  %102 = add nsw i64 %101, %99
  store i64 %102, ptr %100, align 8, !tbaa !20
  %103 = getelementptr i64, ptr %42, i64 %95
  %104 = load i64, ptr %103, align 8, !tbaa !20
  %105 = add nsw i64 %104, %102
  store i64 %105, ptr %103, align 8, !tbaa !20
  %106 = getelementptr i64, ptr %43, i64 %95
  %107 = load i64, ptr %106, align 8, !tbaa !20
  %108 = add nsw i64 %107, %105
  store i64 %108, ptr %106, align 8, !tbaa !20
  %109 = add nuw i64 %95, 4
  %110 = add i64 %96, 4
  %111 = icmp eq i64 %110, %40
  br i1 %111, label %75, label %93, !llvm.loop !95

112:                                              ; preds = %89
  %113 = shl i64 %91, 2
  %114 = tail call noalias ptr @malloc(i64 noundef %113) #30
  %115 = icmp eq ptr %114, null
  br i1 %115, label %116, label %117

116:                                              ; preds = %112
  tail call void @free(ptr noundef nonnull %22) #28
  br label %167

117:                                              ; preds = %112, %89
  %118 = phi ptr [ %114, %112 ], [ null, %89 ]
  %119 = shl i64 %21, 3
  %120 = tail call noalias ptr @malloc(i64 noundef %119) #30
  %121 = icmp eq ptr %120, null
  br i1 %121, label %122, label %123

122:                                              ; preds = %117
  tail call void @free(ptr noundef nonnull %22) #28
  tail call void @free(ptr noundef %118) #28
  br label %167

123:                                              ; preds = %117
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 %120, ptr nonnull align 8 %22, i64 %119, i1 false)
  br i1 %25, label %130, label %124

124:                                              ; preds = %123
  %125 = zext nneg i32 %2 to i64
  %126 = zext nneg i32 %1 to i64
  %127 = load i32, ptr %3, align 4, !tbaa !15
  br label %131

128:                                              ; preds = %162, %131
  %129 = icmp eq i64 %137, %126
  br i1 %129, label %130, label %131, !llvm.loop !96

130:                                              ; preds = %128, %123
  tail call void @free(ptr noundef %120) #28
  store ptr %22, ptr %5, align 8, !tbaa !29
  store ptr %118, ptr %6, align 8, !tbaa !32
  store i64 %91, ptr %7, align 8, !tbaa !20
  br label %167

131:                                              ; preds = %124, %128
  %132 = phi i32 [ %127, %124 ], [ %139, %128 ]
  %133 = phi i64 [ 0, %124 ], [ %137, %128 ]
  %134 = mul nuw nsw i64 %133, %125
  %135 = add nuw nsw i64 %134, %125
  %136 = tail call i64 @llvm.smin.i64(i64 %135, i64 %0)
  %137 = add nuw nsw i64 %133, 1
  %138 = getelementptr inbounds nuw i32, ptr %3, i64 %137
  %139 = load i32, ptr %138, align 4, !tbaa !15
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
  %148 = load i32, ptr %147, align 4, !tbaa !15
  %149 = sext i32 %148 to i64
  %150 = add nsw i64 %134, %149
  %151 = icmp sgt i32 %148, -1
  %152 = icmp slt i64 %150, %136
  %153 = select i1 %151, i1 %152, i1 false
  br i1 %153, label %154, label %162

154:                                              ; preds = %144
  %155 = sext i32 %146 to i64
  %156 = getelementptr inbounds i32, ptr %4, i64 %155
  %157 = load i32, ptr %156, align 4, !tbaa !15
  %158 = getelementptr inbounds nuw i64, ptr %120, i64 %150
  %159 = load i64, ptr %158, align 8, !tbaa !20
  %160 = add nsw i64 %159, 1
  store i64 %160, ptr %158, align 8, !tbaa !20
  %161 = getelementptr inbounds i32, ptr %118, i64 %159
  store i32 %157, ptr %161, align 4, !tbaa !15
  br label %162

162:                                              ; preds = %154, %144
  %163 = add nsw i64 %145, 2
  %164 = trunc nsw i64 %145 to i32
  %165 = add i32 %164, 3
  %166 = icmp slt i32 %165, %139
  br i1 %166, label %144, label %128, !llvm.loop !97

167:                                              ; preds = %20, %122, %130, %116, %8
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @refresh_graph_counts_from_canonical(ptr nocapture noundef nonnull %0) unnamed_addr #0 {
  %2 = load ptr, ptr %0, align 8, !tbaa !50
  %3 = icmp eq ptr %2, null
  br i1 %3, label %92, label %4

4:                                                ; preds = %1
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !59
  %7 = icmp eq ptr %6, null
  br i1 %7, label %16, label %8

8:                                                ; preds = %4
  %9 = tail call i64 @roaring_bitmap_get_cardinality(ptr noundef nonnull %6) #28
  %10 = icmp eq i64 %9, 0
  br i1 %10, label %16, label %11

11:                                               ; preds = %8
  %12 = add i64 %9, -1
  %13 = tail call i32 @roaring_bitmap_get_at_index(ptr noundef nonnull %6, i64 noundef %12) #28
  %14 = zext i32 %13 to i64
  %15 = add nuw nsw i64 %14, 1
  br label %16

16:                                               ; preds = %4, %8, %11
  %17 = phi i64 [ 0, %4 ], [ %15, %11 ], [ 0, %8 ]
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 88
  %19 = load i64, ptr %18, align 8, !tbaa !69
  %20 = icmp slt i64 %19, 0
  br i1 %20, label %21, label %86

21:                                               ; preds = %16
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !60
  %24 = icmp eq ptr %23, null
  br i1 %24, label %84, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %27 = load i64, ptr %26, align 8, !tbaa !74
  %28 = icmp sgt i64 %27, 0
  br i1 %28, label %64, label %29

29:                                               ; preds = %64, %25
  %30 = phi i64 [ 0, %25 ], [ %70, %64 ]
  %31 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %32 = load i64, ptr %31, align 8, !tbaa !71
  %33 = icmp sgt i64 %32, 0
  br i1 %33, label %34, label %84

34:                                               ; preds = %29
  %35 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %36 = load ptr, ptr %35, align 8, !tbaa !75
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
  %47 = load <2 x i8>, ptr %45, align 1, !tbaa !76
  %48 = load <2 x i8>, ptr %46, align 1, !tbaa !76
  %49 = icmp ne <2 x i8> %47, zeroinitializer
  %50 = icmp ne <2 x i8> %48, zeroinitializer
  %51 = zext <2 x i1> %49 to <2 x i64>
  %52 = zext <2 x i1> %50 to <2 x i64>
  %53 = add <2 x i64> %43, %51
  %54 = add <2 x i64> %44, %52
  %55 = add nuw i64 %42, 4
  %56 = icmp eq i64 %55, %39
  br i1 %56, label %57, label %41, !llvm.loop !98

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
  %68 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef nonnull %23, i32 noundef %67) #28
  %69 = zext i1 %68 to i64
  %70 = add nuw nsw i64 %66, %69
  %71 = add nuw nsw i64 %65, 1
  %72 = load i64, ptr %26, align 8, !tbaa !74
  %73 = icmp slt i64 %71, %72
  br i1 %73, label %64, label %29, !llvm.loop !99

74:                                               ; preds = %61, %74
  %75 = phi i64 [ %82, %74 ], [ %62, %61 ]
  %76 = phi i64 [ %81, %74 ], [ %63, %61 ]
  %77 = getelementptr inbounds nuw i8, ptr %36, i64 %75
  %78 = load i8, ptr %77, align 1, !tbaa !76
  %79 = icmp ne i8 %78, 0
  %80 = zext i1 %79 to i64
  %81 = add nsw i64 %76, %80
  %82 = add nuw nsw i64 %75, 1
  %83 = icmp eq i64 %82, %32
  br i1 %83, label %84, label %74, !llvm.loop !100

84:                                               ; preds = %74, %57, %29, %21
  %85 = phi i64 [ 0, %21 ], [ %30, %29 ], [ %59, %57 ], [ %81, %74 ]
  store i64 %85, ptr %18, align 8, !tbaa !69
  br label %86

86:                                               ; preds = %16, %84
  %87 = phi i64 [ %19, %16 ], [ %85, %84 ]
  %88 = shl nsw i64 %87, 1
  store i64 %17, ptr %2, align 8, !tbaa !20
  %89 = getelementptr inbounds nuw i8, ptr %2, i64 8
  store i64 %88, ptr %89, align 8, !tbaa !20
  %90 = getelementptr inbounds nuw i8, ptr %0, i64 120
  store i64 %17, ptr %90, align 8, !tbaa !56
  %91 = getelementptr inbounds nuw i8, ptr %0, i64 128
  store i64 %88, ptr %91, align 8, !tbaa !57
  br label %92

92:                                               ; preds = %1, %86
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @convert_csr_to_pcsr(i64 noundef %0, i64 noundef %1, ptr nocapture noundef readonly %2, ptr nocapture noundef readonly %3, ptr nocapture noundef writeonly %4, ptr nocapture noundef writeonly %5, ptr nocapture noundef writeonly %6) local_unnamed_addr #0 {
  %8 = tail call i64 @llvm.smax.i64(i64 %1, i64 16)
  %9 = shl nuw i64 %8, 1
  %10 = shl i64 %8, 3
  %11 = tail call noalias ptr @malloc(i64 noundef %10) #30
  %12 = shl i64 %0, 3
  %13 = add i64 %12, 8
  %14 = tail call noalias ptr @malloc(i64 noundef %13) #30
  tail call void @llvm.memset.p0.i64(ptr align 4 %11, i8 -1, i64 %10, i1 false)
  %15 = icmp sgt i64 %0, 0
  br i1 %15, label %16, label %20

16:                                               ; preds = %7
  %17 = ptrtoint ptr %3 to i64
  br label %25

18:                                               ; preds = %84, %80, %50
  %19 = icmp eq i64 %30, %0
  br i1 %19, label %20, label %25, !llvm.loop !101

20:                                               ; preds = %18, %7
  %21 = phi i64 [ 0, %7 ], [ %38, %18 ]
  %22 = phi ptr [ %11, %7 ], [ %51, %18 ]
  %23 = phi i64 [ %9, %7 ], [ %52, %18 ]
  %24 = getelementptr inbounds i64, ptr %14, i64 %0
  store i64 %21, ptr %24, align 8, !tbaa !20
  store ptr %14, ptr %4, align 8, !tbaa !29
  store ptr %22, ptr %5, align 8, !tbaa !32
  store i64 %23, ptr %6, align 8, !tbaa !20
  ret void

25:                                               ; preds = %16, %18
  %26 = phi i64 [ %52, %18 ], [ %9, %16 ]
  %27 = phi ptr [ %51, %18 ], [ %11, %16 ]
  %28 = phi i64 [ %38, %18 ], [ 0, %16 ]
  %29 = phi i64 [ %30, %18 ], [ 0, %16 ]
  %30 = add nuw nsw i64 %29, 1
  %31 = getelementptr inbounds nuw i64, ptr %2, i64 %30
  %32 = load i64, ptr %31, align 8, !tbaa !20
  %33 = getelementptr inbounds nuw i64, ptr %2, i64 %29
  %34 = load i64, ptr %33, align 8, !tbaa !20
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
  br i1 %42, label %40, label %44, !llvm.loop !102

44:                                               ; preds = %40
  %45 = shl i64 %41, 2
  %46 = tail call ptr @realloc(ptr noundef %27, i64 noundef %45) #31
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
  store i64 %28, ptr %54, align 8, !tbaa !20
  %55 = icmp sgt i64 %35, 0
  br i1 %55, label %56, label %18

56:                                               ; preds = %50
  %57 = load i64, ptr %33, align 8, !tbaa !20
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
  %74 = load <4 x i32>, ptr %72, align 4, !tbaa !15
  %75 = load <4 x i32>, ptr %73, align 4, !tbaa !15
  %76 = getelementptr i32, ptr %59, i64 %71
  %77 = getelementptr i8, ptr %76, i64 16
  store <4 x i32> %74, ptr %76, align 4, !tbaa !15
  store <4 x i32> %75, ptr %77, align 4, !tbaa !15
  %78 = add nuw i64 %71, 8
  %79 = icmp eq i64 %78, %69
  br i1 %79, label %80, label %70, !llvm.loop !103

80:                                               ; preds = %70
  %81 = icmp eq i64 %35, %69
  br i1 %81, label %18, label %82

82:                                               ; preds = %61, %56, %80
  %83 = phi i64 [ 0, %56 ], [ 0, %61 ], [ %69, %80 ]
  br label %84

84:                                               ; preds = %82, %84
  %85 = phi i64 [ %89, %84 ], [ %83, %82 ]
  %86 = getelementptr i32, ptr %58, i64 %85
  %87 = load i32, ptr %86, align 4, !tbaa !15
  %88 = getelementptr i32, ptr %59, i64 %85
  store i32 %87, ptr %88, align 4, !tbaa !15
  %89 = add nuw nsw i64 %85, 1
  %90 = icmp eq i64 %89, %35
  br i1 %90, label %18, label %84, !llvm.loop !104
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #11

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #12

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
  %25 = tail call noalias ptr @calloc(i64 noundef %24, i64 noundef 4) #29
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
  %48 = load i64, ptr %47, align 8, !tbaa !20
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
  %63 = load <2 x i64>, ptr %61, align 8, !tbaa !20
  %64 = load <2 x i64>, ptr %62, align 8, !tbaa !20
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
  br i1 %74, label %75, label %55, !llvm.loop !105

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
  %97 = load i64, ptr %96, align 8, !tbaa !20
  %98 = sub nsw i64 %97, %92
  %99 = tail call i64 @llvm.smax.i64(i64 %98, i64 0)
  %100 = add nuw nsw i64 %99, %94
  %101 = icmp slt i64 %95, %44
  br i1 %101, label %91, label %84, !llvm.loop !106

102:                                              ; preds = %87
  %103 = trunc nuw nsw i64 %89 to i32
  %104 = getelementptr inbounds nuw i32, ptr %28, i64 %33
  store i32 %103, ptr %104, align 4, !tbaa !15
  %105 = add nuw nsw i64 %33, 1
  %106 = icmp eq i64 %105, %31
  br i1 %106, label %108, label %32, !llvm.loop !107

107:                                              ; preds = %87
  tail call void @free(ptr noundef %25) #28
  br label %191

108:                                              ; preds = %102
  %109 = icmp eq i64 %89, 0
  br i1 %109, label %115, label %110

110:                                              ; preds = %108
  %111 = shl nuw nsw i64 %89, 2
  %112 = tail call noalias ptr @malloc(i64 noundef %111) #30
  %113 = icmp eq ptr %112, null
  br i1 %113, label %114, label %115

114:                                              ; preds = %110
  tail call void @free(ptr noundef nonnull %25) #28
  br label %191

115:                                              ; preds = %108, %110
  %116 = phi ptr [ %112, %110 ], [ null, %108 ]
  %117 = and i64 %20, 2147483647
  %118 = getelementptr i8, ptr %116, i64 16
  br label %121

119:                                              ; preds = %138, %27
  %120 = phi ptr [ null, %27 ], [ %116, %138 ]
  store ptr %25, ptr %4, align 8, !tbaa !32
  store ptr %120, ptr %5, align 8, !tbaa !32
  store i32 %21, ptr %7, align 4, !tbaa !15
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
  %129 = load i32, ptr %128, align 4, !tbaa !15
  %130 = getelementptr inbounds nuw i64, ptr %2, i64 %123
  %131 = load i64, ptr %130, align 8, !tbaa !20
  br label %141

132:                                              ; preds = %181, %176
  %133 = phi i64 [ %158, %176 ], [ %187, %181 ]
  %134 = trunc nsw i64 %133 to i32
  br label %135

135:                                              ; preds = %132, %141
  %136 = phi i32 [ %144, %141 ], [ %134, %132 ]
  %137 = icmp slt i64 %147, %125
  br i1 %137, label %141, label %138, !llvm.loop !108

138:                                              ; preds = %135, %121
  %139 = add nuw nsw i64 %122, 1
  %140 = icmp eq i64 %139, %117
  br i1 %140, label %119, label %121, !llvm.loop !109

141:                                              ; preds = %127, %135
  %142 = phi i64 [ %149, %135 ], [ %131, %127 ]
  %143 = phi i64 [ %147, %135 ], [ %123, %127 ]
  %144 = phi i32 [ %136, %135 ], [ %129, %127 ]
  %145 = sub nuw nsw i64 %143, %123
  %146 = trunc i64 %145 to i32
  %147 = add nuw nsw i64 %143, 1
  %148 = getelementptr inbounds nuw i64, ptr %2, i64 %147
  %149 = load i64, ptr %148, align 8, !tbaa !20
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
  %170 = load <2 x i32>, ptr %168, align 4, !tbaa !15
  %171 = load <2 x i32>, ptr %169, align 4, !tbaa !15
  %172 = shufflevector <2 x i32> %160, <2 x i32> %170, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %172, ptr %166, align 4, !tbaa !15
  %173 = shufflevector <2 x i32> %160, <2 x i32> %171, <4 x i32> <i32 0, i32 2, i32 0, i32 3>
  store <4 x i32> %173, ptr %167, align 4, !tbaa !15
  %174 = add nuw i64 %163, 4
  %175 = icmp eq i64 %174, %156
  br i1 %175, label %176, label %162, !llvm.loop !110

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
  store i32 %146, ptr %184, align 4, !tbaa !15
  %185 = getelementptr inbounds i32, ptr %3, i64 %183
  %186 = load i32, ptr %185, align 4, !tbaa !15
  %187 = add nsw i64 %182, 2
  %188 = getelementptr i8, ptr %184, i64 4
  store i32 %186, ptr %188, align 4, !tbaa !15
  %189 = add nsw i64 %183, 1
  %190 = icmp eq i64 %189, %149
  br i1 %190, label %132, label %181, !llvm.loop !111

191:                                              ; preds = %107, %16, %114, %119, %8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_ensure_layout(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr nocapture noundef readnone %3, ptr nocapture noundef readnone %4, ptr noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8) local_unnamed_addr #0 {
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
  %24 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %25 = icmp sgt i32 %24, 0
  br i1 %25, label %26, label %257

26:                                               ; preds = %9
  %27 = zext nneg i32 %24 to i64
  br label %31

28:                                               ; preds = %31
  %29 = add nuw nsw i64 %32, 1
  %30 = icmp eq i64 %29, %27
  br i1 %30, label %257, label %31, !llvm.loop !49

31:                                               ; preds = %28, %26
  %32 = phi i64 [ 0, %26 ], [ %29, %28 ]
  %33 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %32
  %34 = load ptr, ptr %33, align 16, !tbaa !50
  %35 = icmp eq ptr %34, %0
  br i1 %35, label %36, label %28

36:                                               ; preds = %31
  %37 = icmp eq ptr %5, null
  br i1 %37, label %40, label %38

38:                                               ; preds = %36
  %39 = getelementptr inbounds nuw i8, ptr %33, i64 16
  store ptr %5, ptr %39, align 8, !tbaa !59
  br label %40

40:                                               ; preds = %38, %36
  %41 = icmp eq ptr %6, null
  br i1 %41, label %44, label %42

42:                                               ; preds = %40
  %43 = getelementptr inbounds nuw i8, ptr %33, i64 24
  store ptr %6, ptr %43, align 8, !tbaa !60
  br label %44

44:                                               ; preds = %42, %40
  %45 = icmp eq ptr %7, null
  br i1 %45, label %48, label %46

46:                                               ; preds = %44
  %47 = getelementptr inbounds nuw i8, ptr %33, i64 32
  store ptr %7, ptr %47, align 8, !tbaa !61
  br label %48

48:                                               ; preds = %46, %44
  %49 = getelementptr inbounds nuw i8, ptr %33, i64 16
  %50 = load ptr, ptr %49, align 8, !tbaa !59
  %51 = icmp eq ptr %50, null
  br i1 %51, label %257, label %52

52:                                               ; preds = %48
  %53 = getelementptr inbounds nuw i8, ptr %33, i64 24
  %54 = load ptr, ptr %53, align 8, !tbaa !60
  %55 = icmp eq ptr %54, null
  br i1 %55, label %257, label %56

56:                                               ; preds = %52
  %57 = getelementptr inbounds nuw i8, ptr %33, i64 32
  %58 = load ptr, ptr %57, align 8, !tbaa !61
  %59 = icmp eq ptr %58, null
  %60 = icmp ugt i32 %8, 3
  %61 = or i1 %60, %59
  br i1 %61, label %257, label %62

62:                                               ; preds = %56
  %63 = getelementptr inbounds nuw i8, ptr %33, i64 8
  %64 = load i32, ptr %63, align 8, !tbaa !53
  %65 = icmp eq i32 %64, %8
  br i1 %65, label %257, label %66

66:                                               ; preds = %62
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %12) #28
  %67 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %12) #28
  %68 = load i64, ptr %12, align 8, !tbaa !16
  %69 = mul i64 %68, -1000000000
  %70 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %71 = load i64, ptr %70, align 8, !tbaa !19
  %72 = sub i64 %69, %71
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %12) #28
  %73 = icmp eq i32 %8, 3
  br i1 %73, label %74, label %79

74:                                               ; preds = %66
  call void @autograph_ensure_layout_set(ptr noundef %0)
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %11) #28
  %75 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %11) #28
  %76 = load i64, ptr %11, align 8, !tbaa !16
  %77 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %78 = load i64, ptr %77, align 8, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %11) #28
  br label %247

79:                                               ; preds = %66
  %80 = load i32, ptr %63, align 8, !tbaa !53
  %81 = icmp eq i32 %80, 3
  br i1 %81, label %82, label %138

82:                                               ; preds = %79
  switch i32 %8, label %118 [
    i32 0, label %83
    i32 1, label %98
  ]

83:                                               ; preds = %82
  %84 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %85 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %86 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %87 = getelementptr inbounds nuw i8, ptr %33, i64 128
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %84, ptr noundef %85, ptr noundef %86, ptr noundef %87)
  %88 = load ptr, ptr %85, align 8, !tbaa !54
  %89 = icmp eq ptr %88, null
  br i1 %89, label %225, label %90

90:                                               ; preds = %83
  %91 = load i32, ptr %63, align 8, !tbaa !53
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %97, label %93

93:                                               ; preds = %90
  %94 = getelementptr inbounds nuw i8, ptr %33, i64 192
  %95 = load i32, ptr %94, align 8, !tbaa !82
  %96 = add nsw i32 %95, 1
  store i32 %96, ptr %94, align 8, !tbaa !82
  br label %97

97:                                               ; preds = %90, %93
  store i32 0, ptr %63, align 8, !tbaa !53
  br label %225

98:                                               ; preds = %82
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %13) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %14) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %15) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %16) #28
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %14, ptr noundef %15, ptr noundef %16, ptr noundef %13)
  %99 = load i64, ptr %14, align 8, !tbaa !20
  %100 = getelementptr inbounds nuw i8, ptr %33, i64 120
  store i64 %99, ptr %100, align 8, !tbaa !56
  %101 = load i64, ptr %13, align 8, !tbaa !20
  %102 = load ptr, ptr %15, align 8, !tbaa !29
  %103 = load ptr, ptr %16, align 8, !tbaa !32
  %104 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %105 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %106 = getelementptr inbounds nuw i8, ptr %33, i64 160
  call void @convert_csr_to_pcsr(i64 noundef %99, i64 noundef %101, ptr noundef %102, ptr noundef %103, ptr noundef nonnull %104, ptr noundef nonnull %105, ptr noundef nonnull %106)
  call void @free(ptr noundef %102) #28
  call void @free(ptr noundef %103) #28
  %107 = load ptr, ptr %104, align 8, !tbaa !63
  %108 = icmp eq ptr %107, null
  br i1 %108, label %117, label %109

109:                                              ; preds = %98
  %110 = load i32, ptr %63, align 8, !tbaa !53
  %111 = icmp eq i32 %110, 1
  br i1 %111, label %116, label %112

112:                                              ; preds = %109
  %113 = getelementptr inbounds nuw i8, ptr %33, i64 192
  %114 = load i32, ptr %113, align 8, !tbaa !82
  %115 = add nsw i32 %114, 1
  store i32 %115, ptr %113, align 8, !tbaa !82
  br label %116

116:                                              ; preds = %109, %112
  store i32 1, ptr %63, align 8, !tbaa !53
  br label %117

117:                                              ; preds = %116, %98
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %16) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %15) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %14) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %13) #28
  br label %225

118:                                              ; preds = %82
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %17) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %18) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %19) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %20) #28
  call fastcc void @build_csr_from_meta(ptr noundef %33, ptr noundef %18, ptr noundef %19, ptr noundef %20, ptr noundef %17)
  %119 = load i64, ptr %18, align 8, !tbaa !20
  %120 = getelementptr inbounds nuw i8, ptr %33, i64 120
  store i64 %119, ptr %120, align 8, !tbaa !56
  %121 = getelementptr inbounds nuw i8, ptr %33, i64 184
  store i32 64, ptr %121, align 8, !tbaa !66
  %122 = load ptr, ptr %19, align 8, !tbaa !29
  %123 = load ptr, ptr %20, align 8, !tbaa !32
  %124 = getelementptr inbounds nuw i8, ptr %33, i64 168
  %125 = getelementptr inbounds nuw i8, ptr %33, i64 176
  %126 = getelementptr inbounds nuw i8, ptr %33, i64 188
  call void @convert_csr_to_bcsr(i64 noundef %119, i64 poison, ptr noundef %122, ptr noundef %123, ptr noundef nonnull %124, ptr noundef nonnull %125, i32 noundef 64, ptr noundef nonnull %126)
  call void @free(ptr noundef %122) #28
  call void @free(ptr noundef %123) #28
  %127 = load ptr, ptr %124, align 8, !tbaa !67
  %128 = icmp eq ptr %127, null
  br i1 %128, label %137, label %129

129:                                              ; preds = %118
  %130 = load i32, ptr %63, align 8, !tbaa !53
  %131 = icmp eq i32 %130, 2
  br i1 %131, label %136, label %132

132:                                              ; preds = %129
  %133 = getelementptr inbounds nuw i8, ptr %33, i64 192
  %134 = load i32, ptr %133, align 8, !tbaa !82
  %135 = add nsw i32 %134, 1
  store i32 %135, ptr %133, align 8, !tbaa !82
  br label %136

136:                                              ; preds = %129, %132
  store i32 2, ptr %63, align 8, !tbaa !53
  br label %137

137:                                              ; preds = %136, %118
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %20) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %19) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %18) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %17) #28
  br label %225

138:                                              ; preds = %79
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %21) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %22) #28
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %23) #28
  switch i32 %80, label %161 [
    i32 1, label %139
    i32 2, label %148
  ]

139:                                              ; preds = %138
  %140 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %141 = load i64, ptr %140, align 8, !tbaa !56
  %142 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %143 = load ptr, ptr %142, align 8, !tbaa !63
  %144 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %145 = load ptr, ptr %144, align 8, !tbaa !64
  call void @convert_pcsr_to_csr(i64 noundef %141, i64 poison, ptr noundef %143, ptr noundef %145, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %146 = load ptr, ptr %142, align 8, !tbaa !63
  call void @free(ptr noundef %146) #28
  store ptr null, ptr %142, align 8, !tbaa !63
  %147 = load ptr, ptr %144, align 8, !tbaa !64
  call void @free(ptr noundef %147) #28
  store ptr null, ptr %144, align 8, !tbaa !64
  br label %170

148:                                              ; preds = %138
  %149 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %150 = load i64, ptr %149, align 8, !tbaa !56
  %151 = getelementptr inbounds nuw i8, ptr %33, i64 188
  %152 = load i32, ptr %151, align 4, !tbaa !65
  %153 = getelementptr inbounds nuw i8, ptr %33, i64 184
  %154 = load i32, ptr %153, align 8, !tbaa !66
  %155 = getelementptr inbounds nuw i8, ptr %33, i64 168
  %156 = load ptr, ptr %155, align 8, !tbaa !67
  %157 = getelementptr inbounds nuw i8, ptr %33, i64 176
  %158 = load ptr, ptr %157, align 8, !tbaa !68
  call void @convert_bcsr_to_csr(i64 noundef %150, i32 noundef %152, i32 noundef %154, ptr noundef %156, ptr noundef %158, ptr noundef nonnull %22, ptr noundef nonnull %23, ptr noundef nonnull %21)
  %159 = load ptr, ptr %155, align 8, !tbaa !67
  call void @free(ptr noundef %159) #28
  store ptr null, ptr %155, align 8, !tbaa !67
  %160 = load ptr, ptr %157, align 8, !tbaa !68
  call void @free(ptr noundef %160) #28
  store ptr null, ptr %157, align 8, !tbaa !68
  br label %170

161:                                              ; preds = %138
  %162 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %163 = load i64, ptr %162, align 8, !tbaa !57
  store i64 %163, ptr %21, align 8, !tbaa !20
  %164 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %165 = load ptr, ptr %164, align 8, !tbaa !54
  store ptr %165, ptr %22, align 8, !tbaa !29
  %166 = getelementptr inbounds nuw i8, ptr %33, i64 104
  %167 = load ptr, ptr %166, align 8, !tbaa !55
  store ptr %167, ptr %23, align 8, !tbaa !32
  %168 = getelementptr inbounds nuw i8, ptr %33, i64 136
  %169 = load i32, ptr %168, align 8, !tbaa !62
  br label %170

170:                                              ; preds = %148, %161, %139
  %171 = phi i32 [ 1, %139 ], [ 1, %148 ], [ %169, %161 ]
  switch i32 %8, label %199 [
    i32 0, label %172
    i32 1, label %181
  ]

172:                                              ; preds = %170
  %173 = load i64, ptr %21, align 8, !tbaa !20
  %174 = getelementptr inbounds nuw i8, ptr %33, i64 128
  store i64 %173, ptr %174, align 8, !tbaa !57
  %175 = load ptr, ptr %22, align 8, !tbaa !29
  %176 = getelementptr inbounds nuw i8, ptr %33, i64 96
  store ptr %175, ptr %176, align 8, !tbaa !54
  %177 = load ptr, ptr %23, align 8, !tbaa !32
  %178 = getelementptr inbounds nuw i8, ptr %33, i64 104
  store ptr %177, ptr %178, align 8, !tbaa !55
  %179 = getelementptr inbounds nuw i8, ptr %33, i64 136
  store i32 %171, ptr %179, align 8, !tbaa !62
  %180 = load i32, ptr %63, align 8, !tbaa !53
  br label %217

181:                                              ; preds = %170
  %182 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %183 = load i64, ptr %182, align 8, !tbaa !56
  %184 = load i64, ptr %21, align 8, !tbaa !20
  %185 = load ptr, ptr %22, align 8, !tbaa !29
  %186 = load ptr, ptr %23, align 8, !tbaa !32
  %187 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %188 = getelementptr inbounds nuw i8, ptr %33, i64 152
  %189 = getelementptr inbounds nuw i8, ptr %33, i64 160
  call void @convert_csr_to_pcsr(i64 noundef %183, i64 noundef %184, ptr noundef %185, ptr noundef %186, ptr noundef nonnull %187, ptr noundef nonnull %188, ptr noundef nonnull %189)
  %190 = icmp eq i32 %171, 0
  br i1 %190, label %194, label %191

191:                                              ; preds = %181
  %192 = load ptr, ptr %22, align 8, !tbaa !29
  call void @free(ptr noundef %192) #28
  %193 = load ptr, ptr %23, align 8, !tbaa !32
  call void @free(ptr noundef %193) #28
  br label %194

194:                                              ; preds = %191, %181
  %195 = load i32, ptr %63, align 8, !tbaa !53
  %196 = icmp eq i32 %195, 0
  br i1 %196, label %197, label %217

197:                                              ; preds = %194
  %198 = getelementptr inbounds nuw i8, ptr %33, i64 96
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %198, i8 0, i64 16, i1 false)
  br label %217

199:                                              ; preds = %170
  %200 = getelementptr inbounds nuw i8, ptr %33, i64 184
  store i32 64, ptr %200, align 8, !tbaa !66
  %201 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %202 = load i64, ptr %201, align 8, !tbaa !56
  %203 = load ptr, ptr %22, align 8, !tbaa !29
  %204 = load ptr, ptr %23, align 8, !tbaa !32
  %205 = getelementptr inbounds nuw i8, ptr %33, i64 168
  %206 = getelementptr inbounds nuw i8, ptr %33, i64 176
  %207 = getelementptr inbounds nuw i8, ptr %33, i64 188
  call void @convert_csr_to_bcsr(i64 noundef %202, i64 poison, ptr noundef %203, ptr noundef %204, ptr noundef nonnull %205, ptr noundef nonnull %206, i32 noundef 64, ptr noundef nonnull %207)
  %208 = icmp eq i32 %171, 0
  br i1 %208, label %212, label %209

209:                                              ; preds = %199
  %210 = load ptr, ptr %22, align 8, !tbaa !29
  call void @free(ptr noundef %210) #28
  %211 = load ptr, ptr %23, align 8, !tbaa !32
  call void @free(ptr noundef %211) #28
  br label %212

212:                                              ; preds = %209, %199
  %213 = load i32, ptr %63, align 8, !tbaa !53
  %214 = icmp eq i32 %213, 0
  br i1 %214, label %215, label %217

215:                                              ; preds = %212
  %216 = getelementptr inbounds nuw i8, ptr %33, i64 96
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %216, i8 0, i64 16, i1 false)
  br label %217

217:                                              ; preds = %197, %194, %212, %215, %172
  %218 = phi i32 [ 0, %197 ], [ %195, %194 ], [ %213, %212 ], [ 0, %215 ], [ %180, %172 ]
  %219 = icmp eq i32 %218, %8
  br i1 %219, label %224, label %220

220:                                              ; preds = %217
  %221 = getelementptr inbounds nuw i8, ptr %33, i64 192
  %222 = load i32, ptr %221, align 8, !tbaa !82
  %223 = add nsw i32 %222, 1
  store i32 %223, ptr %221, align 8, !tbaa !82
  br label %224

224:                                              ; preds = %217, %220
  store i32 %8, ptr %63, align 8, !tbaa !53
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %23) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %22) #28
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %21) #28
  br label %225

225:                                              ; preds = %97, %83, %137, %117, %224
  %226 = getelementptr inbounds nuw i8, ptr %33, i64 120
  %227 = load i64, ptr %226, align 8, !tbaa !56
  store i64 %227, ptr %0, align 8, !tbaa !20
  %228 = getelementptr inbounds nuw i8, ptr %33, i64 128
  %229 = load i64, ptr %228, align 8, !tbaa !57
  %230 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %229, ptr %230, align 8, !tbaa !20
  %231 = load i32, ptr %63, align 8, !tbaa !53
  switch i32 %231, label %240 [
    i32 0, label %232
    i32 1, label %236
  ]

232:                                              ; preds = %225
  %233 = getelementptr inbounds nuw i8, ptr %33, i64 96
  %234 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %235 = load <2 x ptr>, ptr %233, align 8, !tbaa !112
  store <2 x ptr> %235, ptr %234, align 8, !tbaa !112
  br label %242

236:                                              ; preds = %225
  %237 = getelementptr inbounds nuw i8, ptr %33, i64 144
  %238 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %239 = load <2 x ptr>, ptr %237, align 8, !tbaa !112
  store <2 x ptr> %239, ptr %238, align 8, !tbaa !112
  br label %242

240:                                              ; preds = %225
  %241 = getelementptr inbounds nuw i8, ptr %0, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %241, i8 0, i64 16, i1 false)
  br label %242

242:                                              ; preds = %236, %240, %232
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %10) #28
  %243 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %10) #28
  %244 = load i64, ptr %10, align 8, !tbaa !16
  %245 = getelementptr inbounds nuw i8, ptr %10, i64 8
  %246 = load i64, ptr %245, align 8, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %10) #28
  br label %247

247:                                              ; preds = %242, %74
  %248 = phi i64 [ %246, %242 ], [ %78, %74 ]
  %249 = phi i64 [ %244, %242 ], [ %76, %74 ]
  %250 = mul i64 %249, 1000000000
  %251 = load i64, ptr @g_conversion_ns, align 8, !tbaa !20
  %252 = add i64 %248, %72
  %253 = add i64 %252, %250
  %254 = add i64 %253, %251
  store i64 %254, ptr @g_conversion_ns, align 8, !tbaa !20
  %255 = load i32, ptr @g_conversions_injected, align 4, !tbaa !15
  %256 = add nsw i32 %255, 1
  store i32 %256, ptr @g_conversions_injected, align 4, !tbaa !15
  br label %257

257:                                              ; preds = %28, %9, %62, %48, %52, %56, %247
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @build_csr_from_meta(ptr nocapture noundef nonnull %0, ptr nocapture noundef nonnull writeonly %1, ptr nocapture noundef nonnull writeonly %2, ptr nocapture noundef nonnull writeonly %3, ptr nocapture noundef nonnull writeonly %4) unnamed_addr #0 {
  %6 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %7 = load ptr, ptr %6, align 8, !tbaa !59
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %9 = load ptr, ptr %8, align 8, !tbaa !60
  %10 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %11 = load ptr, ptr %10, align 8, !tbaa !61
  %12 = tail call i64 @roaring_bitmap_get_cardinality(ptr noundef %7) #28
  %13 = icmp slt i64 %12, 1
  br i1 %13, label %20, label %14

14:                                               ; preds = %5
  %15 = add nsw i64 %12, -1
  %16 = tail call i32 @roaring_bitmap_get_at_index(ptr noundef %7, i64 noundef %15) #28
  %17 = tail call i32 @llvm.smax.i32(i32 %16, i32 -1)
  %18 = add i32 %17, 1
  %19 = zext i32 %18 to i64
  br label %20

20:                                               ; preds = %14, %5
  %21 = phi i64 [ %19, %14 ], [ 0, %5 ]
  %22 = add nuw nsw i64 %21, 1
  %23 = tail call noalias ptr @calloc(i64 noundef %22, i64 noundef 8) #29
  %24 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %25 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %26 = load i64, ptr %24, align 8, !tbaa !74
  %27 = icmp sgt i64 %26, 0
  br i1 %27, label %40, label %30

28:                                               ; preds = %67
  %29 = icmp sgt i64 %70, 0
  br label %30

30:                                               ; preds = %28, %20
  %31 = phi i1 [ false, %20 ], [ %29, %28 ]
  %32 = phi i64 [ 0, %20 ], [ %68, %28 ]
  %33 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %34 = load i64, ptr %33, align 8, !tbaa !71
  %35 = icmp sgt i64 %34, 0
  br i1 %35, label %36, label %72

36:                                               ; preds = %30
  %37 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %38 = load ptr, ptr %37, align 8, !tbaa !75
  %39 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %84

40:                                               ; preds = %20, %67
  %41 = phi i64 [ %68, %67 ], [ 0, %20 ]
  %42 = phi i64 [ %69, %67 ], [ 0, %20 ]
  %43 = trunc i64 %42 to i32
  %44 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %43) #28
  br i1 %44, label %45, label %67

45:                                               ; preds = %40
  %46 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %42
  %47 = load i32, ptr %46, align 4, !tbaa !21
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !23
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
  %61 = load i64, ptr %60, align 8, !tbaa !20
  %62 = add nsw i64 %61, 1
  store i64 %62, ptr %60, align 8, !tbaa !20
  %63 = getelementptr inbounds nuw i64, ptr %25, i64 %57
  %64 = load i64, ptr %63, align 8, !tbaa !20
  %65 = add nsw i64 %64, 1
  store i64 %65, ptr %63, align 8, !tbaa !20
  %66 = add nsw i64 %41, 2
  br label %67

67:                                               ; preds = %45, %53, %56, %59, %40
  %68 = phi i64 [ %41, %40 ], [ %66, %59 ], [ %41, %56 ], [ %41, %53 ], [ %41, %45 ]
  %69 = add nuw nsw i64 %42, 1
  %70 = load i64, ptr %24, align 8, !tbaa !74
  %71 = icmp slt i64 %69, %70
  br i1 %71, label %40, label %28, !llvm.loop !113

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
  %88 = load i8, ptr %87, align 1, !tbaa !76
  %89 = icmp eq i8 %88, 0
  br i1 %89, label %115, label %90

90:                                               ; preds = %84
  %91 = load ptr, ptr %39, align 8, !tbaa !80
  %92 = shl nuw nsw i64 %86, 1
  %93 = getelementptr inbounds nuw i32, ptr %91, i64 %92
  %94 = load i32, ptr %93, align 4, !tbaa !15
  %95 = or disjoint i64 %92, 1
  %96 = getelementptr inbounds nuw i32, ptr %91, i64 %95
  %97 = load i32, ptr %96, align 4, !tbaa !15
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
  %109 = load i64, ptr %108, align 8, !tbaa !20
  %110 = add nsw i64 %109, 1
  store i64 %110, ptr %108, align 8, !tbaa !20
  %111 = getelementptr inbounds nuw i64, ptr %25, i64 %105
  %112 = load i64, ptr %111, align 8, !tbaa !20
  %113 = add nsw i64 %112, 1
  store i64 %113, ptr %111, align 8, !tbaa !20
  %114 = add nsw i64 %85, 2
  br label %115

115:                                              ; preds = %90, %101, %104, %107, %84
  %116 = phi i64 [ %85, %84 ], [ %114, %107 ], [ %85, %104 ], [ %85, %101 ], [ %85, %90 ]
  %117 = add nuw nsw i64 %86, 1
  %118 = icmp eq i64 %117, %34
  br i1 %118, label %72, label %84, !llvm.loop !114

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
  %128 = load i64, ptr %127, align 8, !tbaa !20
  %129 = add nsw i64 %128, %124
  store i64 %129, ptr %127, align 8, !tbaa !20
  %130 = add nuw nsw i64 %125, 1
  %131 = add i64 %126, 1
  %132 = icmp eq i64 %131, %77
  br i1 %132, label %133, label %123, !llvm.loop !115

133:                                              ; preds = %119, %123, %72
  %134 = icmp sgt i64 %73, 0
  br i1 %134, label %154, label %157

135:                                              ; preds = %135, %79
  %136 = phi i64 [ %76, %79 ], [ %150, %135 ]
  %137 = phi i64 [ 1, %79 ], [ %151, %135 ]
  %138 = phi i64 [ 0, %79 ], [ %152, %135 ]
  %139 = getelementptr i64, ptr %23, i64 %137
  %140 = load i64, ptr %139, align 8, !tbaa !20
  %141 = add nsw i64 %140, %136
  store i64 %141, ptr %139, align 8, !tbaa !20
  %142 = getelementptr i64, ptr %81, i64 %137
  %143 = load i64, ptr %142, align 8, !tbaa !20
  %144 = add nsw i64 %143, %141
  store i64 %144, ptr %142, align 8, !tbaa !20
  %145 = getelementptr i64, ptr %82, i64 %137
  %146 = load i64, ptr %145, align 8, !tbaa !20
  %147 = add nsw i64 %146, %144
  store i64 %147, ptr %145, align 8, !tbaa !20
  %148 = getelementptr i64, ptr %83, i64 %137
  %149 = load i64, ptr %148, align 8, !tbaa !20
  %150 = add nsw i64 %149, %147
  store i64 %150, ptr %148, align 8, !tbaa !20
  %151 = add nuw nsw i64 %137, 4
  %152 = add i64 %138, 4
  %153 = icmp eq i64 %152, %80
  br i1 %153, label %119, label %135, !llvm.loop !116

154:                                              ; preds = %133
  %155 = shl i64 %73, 2
  %156 = tail call noalias ptr @malloc(i64 noundef %155) #30
  br label %157

157:                                              ; preds = %133, %154
  %158 = phi ptr [ %156, %154 ], [ null, %133 ]
  %159 = shl nuw nsw i64 %22, 3
  %160 = tail call noalias ptr @malloc(i64 noundef %159) #30
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %160, ptr noundef nonnull align 8 dereferenceable(1) %23, i64 %159, i1 false)
  br i1 %31, label %170, label %163

161:                                              ; preds = %197
  %162 = load i64, ptr %33, align 8, !tbaa !71
  br label %163

163:                                              ; preds = %161, %157
  %164 = phi i64 [ %162, %161 ], [ %34, %157 ]
  %165 = icmp sgt i64 %164, 0
  br i1 %165, label %166, label %201

166:                                              ; preds = %163
  %167 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %168 = load ptr, ptr %167, align 8, !tbaa !75
  %169 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %203

170:                                              ; preds = %157, %197
  %171 = phi i64 [ %198, %197 ], [ 0, %157 ]
  %172 = trunc i64 %171 to i32
  %173 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %9, i32 noundef %172) #28
  br i1 %173, label %174, label %197

174:                                              ; preds = %170
  %175 = getelementptr inbounds nuw %struct.EdgePair, ptr %11, i64 %171
  %176 = load i32, ptr %175, align 4, !tbaa !21
  %177 = getelementptr inbounds nuw i8, ptr %175, i64 4
  %178 = load i32, ptr %177, align 4, !tbaa !23
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
  %190 = load i64, ptr %189, align 8, !tbaa !20
  %191 = add nsw i64 %190, 1
  store i64 %191, ptr %189, align 8, !tbaa !20
  %192 = getelementptr inbounds i32, ptr %158, i64 %190
  store i32 %178, ptr %192, align 4, !tbaa !15
  %193 = getelementptr inbounds nuw i64, ptr %160, i64 %186
  %194 = load i64, ptr %193, align 8, !tbaa !20
  %195 = add nsw i64 %194, 1
  store i64 %195, ptr %193, align 8, !tbaa !20
  %196 = getelementptr inbounds i32, ptr %158, i64 %194
  store i32 %176, ptr %196, align 4, !tbaa !15
  br label %197

197:                                              ; preds = %174, %182, %185, %188, %170
  %198 = add nuw nsw i64 %171, 1
  %199 = load i64, ptr %24, align 8, !tbaa !74
  %200 = icmp slt i64 %198, %199
  br i1 %200, label %170, label %161, !llvm.loop !117

201:                                              ; preds = %234, %163
  tail call void @free(ptr noundef %160) #28
  store i64 %21, ptr %1, align 8, !tbaa !20
  store ptr %23, ptr %2, align 8, !tbaa !29
  store ptr %158, ptr %3, align 8, !tbaa !32
  store i64 %73, ptr %4, align 8, !tbaa !20
  %202 = getelementptr inbounds nuw i8, ptr %0, i64 136
  store i32 1, ptr %202, align 8, !tbaa !62
  ret void

203:                                              ; preds = %166, %234
  %204 = phi i64 [ 0, %166 ], [ %235, %234 ]
  %205 = getelementptr inbounds nuw i8, ptr %168, i64 %204
  %206 = load i8, ptr %205, align 1, !tbaa !76
  %207 = icmp eq i8 %206, 0
  br i1 %207, label %234, label %208

208:                                              ; preds = %203
  %209 = load ptr, ptr %169, align 8, !tbaa !80
  %210 = shl nuw nsw i64 %204, 1
  %211 = getelementptr inbounds nuw i32, ptr %209, i64 %210
  %212 = load i32, ptr %211, align 4, !tbaa !15
  %213 = or disjoint i64 %210, 1
  %214 = getelementptr inbounds nuw i32, ptr %209, i64 %213
  %215 = load i32, ptr %214, align 4, !tbaa !15
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
  %227 = load i64, ptr %226, align 8, !tbaa !20
  %228 = add nsw i64 %227, 1
  store i64 %228, ptr %226, align 8, !tbaa !20
  %229 = getelementptr inbounds i32, ptr %158, i64 %227
  store i32 %215, ptr %229, align 4, !tbaa !15
  %230 = getelementptr inbounds nuw i64, ptr %160, i64 %223
  %231 = load i64, ptr %230, align 8, !tbaa !20
  %232 = add nsw i64 %231, 1
  store i64 %232, ptr %230, align 8, !tbaa !20
  %233 = getelementptr inbounds i32, ptr %158, i64 %231
  store i32 %212, ptr %233, align 4, !tbaa !15
  br label %234

234:                                              ; preds = %208, %219, %222, %225, %203
  %235 = add nuw nsw i64 %204, 1
  %236 = icmp eq i64 %235, %164
  br i1 %236, label %201, label %203, !llvm.loop !118
}

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @autograph_get_layout(ptr noundef readnone %0) local_unnamed_addr #13 {
  %2 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %17

4:                                                ; preds = %1
  %5 = zext nneg i32 %2 to i64
  br label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, %5
  br i1 %8, label %17, label %9, !llvm.loop !49

9:                                                ; preds = %6, %4
  %10 = phi i64 [ 0, %4 ], [ %7, %6 ]
  %11 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %10
  %12 = load ptr, ptr %11, align 16, !tbaa !50
  %13 = icmp eq ptr %12, %0
  br i1 %13, label %14, label %6

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !53
  br label %17

17:                                               ; preds = %6, %1, %14
  %18 = phi i32 [ %16, %14 ], [ 3, %1 ], [ 3, %6 ]
  ret i32 %18
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @autograph_debug_print(ptr nocapture noundef readnone %0) local_unnamed_addr #14 {
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_get_meta_handles(ptr noundef %0, ptr noundef writeonly %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 {
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %51

7:                                                ; preds = %4
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %23, label %12, !llvm.loop !49

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 16, !tbaa !50
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %12
  %18 = getelementptr inbounds nuw i8, ptr %14, i64 80
  %19 = load i32, ptr %18, align 8, !tbaa !58
  %20 = icmp eq i32 %19, 0
  br i1 %20, label %23, label %21

21:                                               ; preds = %17
  tail call void @autograph_ensure_layout_set(ptr noundef %0)
  %22 = load i32, ptr @g_meta_count, align 4, !tbaa !15
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
  br i1 %30, label %51, label %31, !llvm.loop !49

31:                                               ; preds = %28, %26
  %32 = phi i64 [ 0, %26 ], [ %29, %28 ]
  %33 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %32
  %34 = load ptr, ptr %33, align 16, !tbaa !50
  %35 = icmp eq ptr %34, %0
  br i1 %35, label %36, label %28

36:                                               ; preds = %31
  %37 = icmp eq ptr %1, null
  br i1 %37, label %41, label %38

38:                                               ; preds = %36
  %39 = getelementptr inbounds nuw i8, ptr %33, i64 16
  %40 = load ptr, ptr %39, align 8, !tbaa !59
  store ptr %40, ptr %1, align 8, !tbaa !112
  br label %41

41:                                               ; preds = %38, %36
  %42 = icmp eq ptr %2, null
  br i1 %42, label %46, label %43

43:                                               ; preds = %41
  %44 = getelementptr inbounds nuw i8, ptr %33, i64 24
  %45 = load ptr, ptr %44, align 8, !tbaa !60
  store ptr %45, ptr %2, align 8, !tbaa !112
  br label %46

46:                                               ; preds = %43, %41
  %47 = icmp eq ptr %3, null
  br i1 %47, label %51, label %48

48:                                               ; preds = %46
  %49 = getelementptr inbounds nuw i8, ptr %33, i64 32
  %50 = load ptr, ptr %49, align 8, !tbaa !61
  store ptr %50, ptr %3, align 8, !tbaa !112
  br label %51

51:                                               ; preds = %28, %4, %23, %46, %48
  %52 = phi i32 [ 1, %48 ], [ 1, %46 ], [ 0, %23 ], [ 0, %4 ], [ 0, %28 ]
  ret i32 %52
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %26

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %26, label %10, !llvm.loop !49

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 16, !tbaa !50
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !59
  %18 = icmp eq ptr %17, null
  br i1 %18, label %26, label %19

19:                                               ; preds = %15
  tail call void @roaring_bitmap_add(ptr noundef nonnull %17, i32 noundef %1) #28
  %20 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %21 = load i32, ptr %20, align 8, !tbaa !53
  %22 = icmp eq i32 %21, 3
  br i1 %22, label %23, label %24

23:                                               ; preds = %19
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %12)
  br label %24

24:                                               ; preds = %23, %19
  %25 = getelementptr inbounds nuw i8, ptr %12, i64 80
  store i32 0, ptr %25, align 8, !tbaa !58
  br label %26

26:                                               ; preds = %7, %2, %15, %24
  %27 = phi i32 [ 1, %24 ], [ 0, %15 ], [ 0, %2 ], [ 0, %7 ]
  ret i32 %27
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_node(ptr noundef readnone %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %99

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %99, label %10, !llvm.loop !49

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 16, !tbaa !50
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %15, label %7

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %12, i64 16
  %17 = load ptr, ptr %16, align 8, !tbaa !59
  %18 = icmp eq ptr %17, null
  br i1 %18, label %99, label %19

19:                                               ; preds = %15
  %20 = getelementptr inbounds nuw i8, ptr %12, i64 24
  %21 = load ptr, ptr %20, align 8, !tbaa !60
  %22 = icmp eq ptr %21, null
  br i1 %22, label %99, label %23

23:                                               ; preds = %19
  tail call void @roaring_bitmap_remove(ptr noundef nonnull %17, i32 noundef %1) #28
  %24 = getelementptr inbounds nuw i8, ptr %12, i64 32
  %25 = load ptr, ptr %24, align 8, !tbaa !61
  %26 = load ptr, ptr %20, align 8, !tbaa !60
  %27 = getelementptr inbounds nuw i8, ptr %12, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !74
  %29 = icmp sgt i64 %28, 0
  br i1 %29, label %38, label %30

30:                                               ; preds = %53, %23
  %31 = phi i64 [ 0, %23 ], [ %54, %53 ]
  %32 = getelementptr inbounds nuw i8, ptr %12, i64 64
  %33 = load i64, ptr %32, align 8, !tbaa !71
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
  %42 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %26, i32 noundef %41) #28
  br i1 %42, label %43, label %53

43:                                               ; preds = %38
  %44 = getelementptr inbounds nuw %struct.EdgePair, ptr %25, i64 %39
  %45 = load i32, ptr %44, align 4, !tbaa !21
  %46 = icmp eq i32 %45, %1
  br i1 %46, label %51, label %47

47:                                               ; preds = %43
  %48 = getelementptr inbounds nuw i8, ptr %44, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !23
  %50 = icmp eq i32 %49, %1
  br i1 %50, label %51, label %53

51:                                               ; preds = %47, %43
  tail call void @roaring_bitmap_remove(ptr noundef %26, i32 noundef %41) #28
  %52 = add nsw i64 %40, 1
  br label %53

53:                                               ; preds = %47, %51, %38
  %54 = phi i64 [ %52, %51 ], [ %40, %47 ], [ %40, %38 ]
  %55 = add nuw nsw i64 %39, 1
  %56 = load i64, ptr %27, align 8, !tbaa !74
  %57 = icmp slt i64 %55, %56
  br i1 %57, label %38, label %30, !llvm.loop !119

58:                                               ; preds = %85, %30
  %59 = phi i64 [ %31, %30 ], [ %87, %85 ]
  %60 = getelementptr inbounds nuw i8, ptr %12, i64 88
  %61 = load i64, ptr %60, align 8, !tbaa !69
  %62 = icmp sgt i64 %61, -1
  br i1 %62, label %90, label %92

63:                                               ; preds = %35, %85
  %64 = phi i64 [ %33, %35 ], [ %86, %85 ]
  %65 = phi i64 [ 0, %35 ], [ %88, %85 ]
  %66 = phi i64 [ %31, %35 ], [ %87, %85 ]
  %67 = load ptr, ptr %36, align 8, !tbaa !75
  %68 = getelementptr inbounds nuw i8, ptr %67, i64 %65
  %69 = load i8, ptr %68, align 1, !tbaa !76
  %70 = icmp eq i8 %69, 0
  br i1 %70, label %85, label %71

71:                                               ; preds = %63
  %72 = load ptr, ptr %37, align 8, !tbaa !80
  %73 = shl nuw nsw i64 %65, 1
  %74 = getelementptr inbounds nuw i32, ptr %72, i64 %73
  %75 = load i32, ptr %74, align 4, !tbaa !15
  %76 = icmp eq i32 %75, %1
  br i1 %76, label %82, label %77

77:                                               ; preds = %71
  %78 = or disjoint i64 %73, 1
  %79 = getelementptr inbounds nuw i32, ptr %72, i64 %78
  %80 = load i32, ptr %79, align 4, !tbaa !15
  %81 = icmp eq i32 %80, %1
  br i1 %81, label %82, label %85

82:                                               ; preds = %77, %71
  store i8 0, ptr %68, align 1, !tbaa !76
  %83 = add nsw i64 %66, 1
  %84 = load i64, ptr %32, align 8, !tbaa !71
  br label %85

85:                                               ; preds = %77, %82, %63
  %86 = phi i64 [ %64, %63 ], [ %84, %82 ], [ %64, %77 ]
  %87 = phi i64 [ %66, %63 ], [ %83, %82 ], [ %66, %77 ]
  %88 = add nuw nsw i64 %65, 1
  %89 = icmp slt i64 %88, %86
  br i1 %89, label %63, label %58, !llvm.loop !120

90:                                               ; preds = %58
  %91 = sub nsw i64 %61, %59
  store i64 %91, ptr %60, align 8, !tbaa !69
  br label %92

92:                                               ; preds = %90, %58
  %93 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %94 = load i32, ptr %93, align 8, !tbaa !53
  %95 = icmp eq i32 %94, 3
  br i1 %95, label %96, label %97

96:                                               ; preds = %92
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %12)
  br label %97

97:                                               ; preds = %96, %92
  %98 = getelementptr inbounds nuw i8, ptr %12, i64 80
  store i32 0, ptr %98, align 8, !tbaa !58
  br label %99

99:                                               ; preds = %7, %2, %15, %19, %97
  %100 = phi i32 [ 1, %97 ], [ 0, %19 ], [ 0, %15 ], [ 0, %2 ], [ 0, %7 ]
  ret i32 %100
}

declare void @roaring_bitmap_remove(ptr noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_add_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %120

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %120, label %11, !llvm.loop !49

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 16, !tbaa !50
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 16
  %18 = load ptr, ptr %17, align 8, !tbaa !59
  %19 = icmp eq ptr %18, null
  br i1 %19, label %120, label %20

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %22 = load ptr, ptr %21, align 8, !tbaa !60
  %23 = icmp eq ptr %22, null
  br i1 %23, label %120, label %24

24:                                               ; preds = %20
  tail call void @roaring_bitmap_add(ptr noundef nonnull %18, i32 noundef %1) #28
  %25 = load ptr, ptr %17, align 8, !tbaa !59
  tail call void @roaring_bitmap_add(ptr noundef %25, i32 noundef %2) #28
  %26 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %27 = icmp sgt i32 %26, -1
  br i1 %27, label %28, label %41

28:                                               ; preds = %24
  %29 = load ptr, ptr %21, align 8, !tbaa !60
  %30 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %29, i32 noundef %26) #28
  tail call void @roaring_bitmap_add(ptr noundef %29, i32 noundef %26) #28
  br i1 %30, label %37, label %31

31:                                               ; preds = %28
  %32 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %33 = load i64, ptr %32, align 8, !tbaa !69
  %34 = icmp sgt i64 %33, -1
  br i1 %34, label %35, label %37

35:                                               ; preds = %31
  %36 = add nuw nsw i64 %33, 1
  store i64 %36, ptr %32, align 8, !tbaa !69
  br label %37

37:                                               ; preds = %35, %31, %28
  %38 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %39 = load i32, ptr %38, align 8, !tbaa !53
  %40 = icmp eq i32 %39, 3
  br i1 %40, label %117, label %118

41:                                               ; preds = %24
  %42 = tail call fastcc i64 @canonical_pair_find_extra(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %43 = icmp sgt i64 %42, -1
  br i1 %43, label %44, label %60

44:                                               ; preds = %41
  %45 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %46 = load ptr, ptr %45, align 8, !tbaa !75
  %47 = getelementptr inbounds nuw i8, ptr %46, i64 %42
  %48 = load i8, ptr %47, align 1, !tbaa !76
  %49 = icmp eq i8 %48, 0
  br i1 %49, label %50, label %56

50:                                               ; preds = %44
  %51 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %52 = load i64, ptr %51, align 8, !tbaa !69
  %53 = icmp sgt i64 %52, -1
  br i1 %53, label %54, label %56

54:                                               ; preds = %50
  %55 = add nuw nsw i64 %52, 1
  store i64 %55, ptr %51, align 8, !tbaa !69
  br label %56

56:                                               ; preds = %54, %50, %44
  store i8 1, ptr %47, align 1, !tbaa !76
  %57 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %58 = load i32, ptr %57, align 8, !tbaa !53
  %59 = icmp eq i32 %58, 3
  br i1 %59, label %117, label %118

60:                                               ; preds = %41
  %61 = getelementptr inbounds nuw i8, ptr %13, i64 64
  %62 = load i64, ptr %61, align 8, !tbaa !71
  %63 = add nsw i64 %62, 1
  %64 = tail call fastcc i32 @ensure_extra_capacity(ptr noundef %13, i64 noundef %63)
  %65 = icmp eq i32 %64, 0
  br i1 %65, label %120, label %66

66:                                               ; preds = %60
  %67 = load i64, ptr %61, align 8, !tbaa !71
  %68 = add nsw i64 %67, 1
  store i64 %68, ptr %61, align 8, !tbaa !71
  %69 = getelementptr inbounds nuw i8, ptr %13, i64 48
  %70 = load ptr, ptr %69, align 8, !tbaa !80
  %71 = shl nsw i64 %67, 1
  %72 = getelementptr inbounds i32, ptr %70, i64 %71
  store i32 %1, ptr %72, align 4, !tbaa !15
  %73 = or disjoint i64 %71, 1
  %74 = getelementptr inbounds i32, ptr %70, i64 %73
  store i32 %2, ptr %74, align 4, !tbaa !15
  %75 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %76 = load ptr, ptr %75, align 8, !tbaa !75
  %77 = getelementptr inbounds i8, ptr %76, i64 %67
  store i8 1, ptr %77, align 1, !tbaa !76
  %78 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %79 = icmp slt i32 %78, 1
  %80 = zext nneg i32 %78 to i64
  %81 = add nsw i64 %80, -1
  %82 = icmp ugt i64 %12, %81
  %83 = select i1 %79, i1 true, i1 %82
  br i1 %83, label %107, label %84

84:                                               ; preds = %66
  %85 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %12
  %86 = load ptr, ptr %85, align 8, !tbaa !72
  %87 = icmp eq ptr %86, null
  br i1 %87, label %88, label %105

88:                                               ; preds = %84
  %89 = getelementptr inbounds nuw i8, ptr %13, i64 72
  %90 = load i64, ptr %89, align 8, !tbaa !70
  %91 = icmp sgt i64 %90, 0
  %92 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #30
  %93 = trunc i64 %90 to i32
  %94 = shl i32 %93, 2
  %95 = add i32 %94, 32
  %96 = select i1 %91, i32 %95, i32 128
  br label %97

97:                                               ; preds = %97, %88
  %98 = phi i32 [ 1, %88 ], [ %100, %97 ]
  %99 = icmp sgt i32 %98, %96
  %100 = shl i32 %98, 1
  br i1 %99, label %101, label %97, !llvm.loop !35

101:                                              ; preds = %97
  %102 = sext i32 %98 to i64
  %103 = getelementptr inbounds nuw i8, ptr %92, i64 8
  store i64 %102, ptr %103, align 8, !tbaa !45
  %104 = tail call noalias ptr @calloc(i64 noundef %102, i64 noundef 24) #29
  store ptr %104, ptr %92, align 8, !tbaa !48
  store ptr %92, ptr %85, align 8, !tbaa !72
  br label %105

105:                                              ; preds = %101, %84
  %106 = phi ptr [ %92, %101 ], [ %86, %84 ]
  tail call fastcc void @edge_hash_insert(ptr noundef nonnull %106, i32 noundef %1, i32 noundef %2, i64 noundef %67)
  br label %107

107:                                              ; preds = %66, %105
  %108 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %109 = load i64, ptr %108, align 8, !tbaa !69
  %110 = icmp sgt i64 %109, -1
  br i1 %110, label %111, label %113

111:                                              ; preds = %107
  %112 = add nuw nsw i64 %109, 1
  store i64 %112, ptr %108, align 8, !tbaa !69
  br label %113

113:                                              ; preds = %111, %107
  %114 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %115 = load i32, ptr %114, align 8, !tbaa !53
  %116 = icmp eq i32 %115, 3
  br i1 %116, label %117, label %118

117:                                              ; preds = %113, %56, %37
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %13)
  br label %118

118:                                              ; preds = %117, %113, %56, %37
  %119 = getelementptr inbounds nuw i8, ptr %13, i64 80
  store i32 0, ptr %119, align 8, !tbaa !58
  br label %120

120:                                              ; preds = %8, %118, %3, %60, %16, %20
  %121 = phi i32 [ 0, %20 ], [ 0, %16 ], [ 0, %60 ], [ 0, %3 ], [ 1, %118 ], [ 0, %8 ]
  ret i32 %121
}

; Function Attrs: nofree nounwind memory(readwrite, argmem: read) uwtable
define internal fastcc i32 @canonical_pair_find_static(ptr noundef nonnull readonly %0, i32 noundef %1, i32 noundef %2) unnamed_addr #15 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %5 = load ptr, ptr %4, align 8, !tbaa !61
  %6 = icmp eq ptr %5, null
  br i1 %6, label %136, label %7

7:                                                ; preds = %3
  %8 = load i32, ptr @g_meta_count, align 4, !tbaa !15
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
  br i1 %18, label %136, label %12, !llvm.loop !121

19:                                               ; preds = %12
  %20 = and i64 %13, 4294967295
  %21 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %20
  %22 = load ptr, ptr %21, align 8, !tbaa !72
  %23 = icmp eq ptr %22, null
  br i1 %23, label %26, label %24

24:                                               ; preds = %19
  %25 = load ptr, ptr %22, align 8, !tbaa !48
  br label %91

26:                                               ; preds = %19
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %28 = load i64, ptr %27, align 8, !tbaa !74
  %29 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #30
  %30 = trunc i64 %28 to i32
  %31 = shl i32 %30, 1
  %32 = add i32 %31, 32
  br label %33

33:                                               ; preds = %33, %26
  %34 = phi i32 [ 1, %26 ], [ %36, %33 ]
  %35 = icmp sgt i32 %34, %32
  %36 = shl i32 %34, 1
  br i1 %35, label %37, label %33, !llvm.loop !35

37:                                               ; preds = %33
  %38 = sext i32 %34 to i64
  %39 = getelementptr inbounds nuw i8, ptr %29, i64 8
  store i64 %38, ptr %39, align 8, !tbaa !45
  %40 = tail call noalias ptr @calloc(i64 noundef %38, i64 noundef 24) #29
  store ptr %40, ptr %29, align 8, !tbaa !48
  store ptr %29, ptr %21, align 8, !tbaa !72
  %41 = icmp sgt i64 %28, 0
  br i1 %41, label %42, label %91

42:                                               ; preds = %37
  %43 = add nsw i64 %38, -1
  br label %44

44:                                               ; preds = %88, %42
  %45 = phi i64 [ 0, %42 ], [ %89, %88 ]
  %46 = getelementptr inbounds nuw %struct.EdgePair, ptr %5, i64 %45
  %47 = load i32, ptr %46, align 4, !tbaa !21
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 4
  %49 = load i32, ptr %48, align 4, !tbaa !23
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
  %67 = load i8, ptr %66, align 8, !tbaa !36
  %68 = icmp eq i8 %67, 0
  br i1 %68, label %83, label %69

69:                                               ; preds = %44, %76
  %70 = phi ptr [ %79, %76 ], [ %65, %44 ]
  %71 = phi i64 [ %78, %76 ], [ %64, %44 ]
  %72 = load i64, ptr %70, align 8, !tbaa !38
  %73 = icmp eq i64 %72, %55
  br i1 %73, label %74, label %76

74:                                               ; preds = %69
  %75 = getelementptr inbounds nuw i8, ptr %70, i64 8
  store i64 %45, ptr %75, align 8, !tbaa !39
  br label %88

76:                                               ; preds = %69
  %77 = add i64 %71, 1
  %78 = and i64 %77, %43
  %79 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %78
  %80 = getelementptr inbounds nuw i8, ptr %79, i64 16
  %81 = load i8, ptr %80, align 8, !tbaa !36
  %82 = icmp eq i8 %81, 0
  br i1 %82, label %83, label %69, !llvm.loop !40

83:                                               ; preds = %76, %44
  %84 = phi i64 [ %64, %44 ], [ %78, %76 ]
  %85 = phi ptr [ %65, %44 ], [ %79, %76 ]
  store i64 %55, ptr %85, align 8, !tbaa !38
  %86 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %84, i32 1
  store i64 %45, ptr %86, align 8, !tbaa !39
  %87 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %40, i64 %84, i32 2
  store i8 1, ptr %87, align 8, !tbaa !36
  br label %88

88:                                               ; preds = %83, %74
  %89 = add nuw nsw i64 %45, 1
  %90 = icmp eq i64 %89, %28
  br i1 %90, label %91, label %44, !llvm.loop !122

91:                                               ; preds = %88, %24, %37
  %92 = phi ptr [ %40, %37 ], [ %25, %24 ], [ %40, %88 ]
  %93 = phi ptr [ %29, %37 ], [ %22, %24 ], [ %29, %88 ]
  %94 = icmp eq ptr %92, null
  br i1 %94, label %136, label %95

95:                                               ; preds = %91
  %96 = getelementptr inbounds nuw i8, ptr %93, i64 8
  %97 = load i64, ptr %96, align 8, !tbaa !45
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
  %118 = load i8, ptr %117, align 8, !tbaa !36
  %119 = icmp eq i8 %118, 0
  br i1 %119, label %136, label %127

120:                                              ; preds = %127
  %121 = add i64 %129, 1
  %122 = and i64 %121, %114
  %123 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %92, i64 %122
  %124 = getelementptr inbounds nuw i8, ptr %123, i64 16
  %125 = load i8, ptr %124, align 8, !tbaa !36
  %126 = icmp eq i8 %125, 0
  br i1 %126, label %136, label %127, !llvm.loop !43

127:                                              ; preds = %99, %120
  %128 = phi ptr [ %123, %120 ], [ %116, %99 ]
  %129 = phi i64 [ %122, %120 ], [ %115, %99 ]
  %130 = load i64, ptr %128, align 8, !tbaa !38
  %131 = icmp eq i64 %130, %105
  br i1 %131, label %132, label %120

132:                                              ; preds = %127
  %133 = getelementptr inbounds nuw i8, ptr %128, i64 8
  %134 = load i64, ptr %133, align 8, !tbaa !39
  %135 = trunc i64 %134 to i32
  br label %136

136:                                              ; preds = %16, %120, %132, %99, %95, %91, %7, %3
  %137 = phi i32 [ -1, %3 ], [ -1, %95 ], [ -1, %91 ], [ %135, %132 ], [ -1, %99 ], [ -1, %7 ], [ -1, %120 ], [ -1, %16 ]
  ret i32 %137
}

; Function Attrs: nofree nounwind memory(readwrite, argmem: read) uwtable
define internal fastcc i64 @canonical_pair_find_extra(ptr noundef nonnull readonly %0, i32 noundef %1, i32 noundef %2) unnamed_addr #15 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %5 = load i64, ptr %4, align 8, !tbaa !71
  %6 = icmp slt i64 %5, 1
  br i1 %6, label %87, label %7

7:                                                ; preds = %3
  %8 = load i32, ptr @g_meta_count, align 4, !tbaa !15
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
  br i1 %18, label %87, label %12, !llvm.loop !121

19:                                               ; preds = %12
  %20 = and i64 %13, 4294967295
  %21 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %20
  %22 = load ptr, ptr %21, align 8, !tbaa !72
  %23 = icmp eq ptr %22, null
  br i1 %23, label %24, label %41

24:                                               ; preds = %19
  %25 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %26 = load i64, ptr %25, align 8, !tbaa !70
  %27 = icmp sgt i64 %26, 0
  %28 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #30
  %29 = trunc i64 %26 to i32
  %30 = shl i32 %29, 2
  %31 = add i32 %30, 32
  %32 = select i1 %27, i32 %31, i32 128
  br label %33

33:                                               ; preds = %33, %24
  %34 = phi i32 [ 1, %24 ], [ %36, %33 ]
  %35 = icmp sgt i32 %34, %32
  %36 = shl i32 %34, 1
  br i1 %35, label %37, label %33, !llvm.loop !35

37:                                               ; preds = %33
  %38 = sext i32 %34 to i64
  %39 = getelementptr inbounds nuw i8, ptr %28, i64 8
  store i64 %38, ptr %39, align 8, !tbaa !45
  %40 = tail call noalias ptr @calloc(i64 noundef %38, i64 noundef 24) #29
  store ptr %40, ptr %28, align 8, !tbaa !48
  store ptr %28, ptr %21, align 8, !tbaa !72
  br label %43

41:                                               ; preds = %19
  %42 = load ptr, ptr %22, align 8, !tbaa !48
  br label %43

43:                                               ; preds = %41, %37
  %44 = phi ptr [ %42, %41 ], [ %40, %37 ]
  %45 = phi ptr [ %22, %41 ], [ %28, %37 ]
  %46 = icmp eq ptr %44, null
  br i1 %46, label %87, label %47

47:                                               ; preds = %43
  %48 = getelementptr inbounds nuw i8, ptr %45, i64 8
  %49 = load i64, ptr %48, align 8, !tbaa !45
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
  %70 = load i8, ptr %69, align 8, !tbaa !36
  %71 = icmp eq i8 %70, 0
  br i1 %71, label %87, label %79

72:                                               ; preds = %79
  %73 = add i64 %81, 1
  %74 = and i64 %73, %66
  %75 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %44, i64 %74
  %76 = getelementptr inbounds nuw i8, ptr %75, i64 16
  %77 = load i8, ptr %76, align 8, !tbaa !36
  %78 = icmp eq i8 %77, 0
  br i1 %78, label %87, label %79, !llvm.loop !43

79:                                               ; preds = %51, %72
  %80 = phi ptr [ %75, %72 ], [ %68, %51 ]
  %81 = phi i64 [ %74, %72 ], [ %67, %51 ]
  %82 = load i64, ptr %80, align 8, !tbaa !38
  %83 = icmp eq i64 %82, %57
  br i1 %83, label %84, label %72

84:                                               ; preds = %79
  %85 = getelementptr inbounds nuw i8, ptr %80, i64 8
  %86 = load i64, ptr %85, align 8, !tbaa !39
  br label %87

87:                                               ; preds = %16, %72, %7, %84, %51, %47, %43, %3
  %88 = phi i64 [ -1, %3 ], [ -1, %47 ], [ -1, %43 ], [ %86, %84 ], [ -1, %51 ], [ -1, %7 ], [ -1, %72 ], [ -1, %16 ]
  ret i64 %88
}

; Function Attrs: nounwind uwtable
define internal fastcc range(i32 0, 2) i32 @ensure_extra_capacity(ptr noundef nonnull %0, i64 noundef range(i64 -9223372036854775807, -9223372036854775808) %1) unnamed_addr #0 {
  %3 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %4 = load i64, ptr %3, align 8, !tbaa !70
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
  br i1 %11, label %9, label %13, !llvm.loop !123

13:                                               ; preds = %9
  %14 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %15 = load ptr, ptr %14, align 8, !tbaa !80
  %16 = shl i64 %10, 3
  %17 = tail call ptr @realloc(ptr noundef %15, i64 noundef %16) #31
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %19 = load ptr, ptr %18, align 8, !tbaa !75
  %20 = tail call ptr @realloc(ptr noundef %19, i64 noundef %10) #31
  %21 = icmp ne ptr %17, null
  %22 = icmp ne ptr %20, null
  %23 = and i1 %21, %22
  br i1 %23, label %24, label %116

24:                                               ; preds = %13
  %25 = load i64, ptr %3, align 8, !tbaa !70
  %26 = icmp sgt i64 %10, %25
  br i1 %26, label %27, label %30

27:                                               ; preds = %24
  %28 = getelementptr inbounds i8, ptr %20, i64 %25
  %29 = sub nsw i64 %10, %25
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %28, i8 0, i64 %29, i1 false)
  br label %30

30:                                               ; preds = %27, %24
  store ptr %17, ptr %14, align 8, !tbaa !80
  store ptr %20, ptr %18, align 8, !tbaa !75
  store i64 %10, ptr %3, align 8, !tbaa !70
  %31 = load i32, ptr @g_meta_count, align 4, !tbaa !15
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
  br i1 %41, label %116, label %35, !llvm.loop !121

42:                                               ; preds = %35
  %43 = and i64 %36, 4294967295
  %44 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %43
  %45 = load ptr, ptr %44, align 8, !tbaa !72
  %46 = icmp eq ptr %45, null
  br i1 %46, label %116, label %47

47:                                               ; preds = %42
  %48 = load ptr, ptr %45, align 8, !tbaa !48
  tail call void @free(ptr noundef %48) #28
  tail call void @free(ptr noundef nonnull %45) #28
  %49 = tail call noalias dereferenceable_or_null(16) ptr @malloc(i64 noundef 16) #30
  %50 = trunc i64 %10 to i32
  %51 = shl i32 %50, 2
  %52 = add i32 %51, 32
  br label %53

53:                                               ; preds = %53, %47
  %54 = phi i32 [ 1, %47 ], [ %56, %53 ]
  %55 = icmp sgt i32 %54, %52
  %56 = shl i32 %54, 1
  br i1 %55, label %57, label %53, !llvm.loop !35

57:                                               ; preds = %53
  %58 = sext i32 %54 to i64
  %59 = getelementptr inbounds nuw i8, ptr %49, i64 8
  store i64 %58, ptr %59, align 8, !tbaa !45
  %60 = tail call noalias ptr @calloc(i64 noundef %58, i64 noundef 24) #29
  store ptr %60, ptr %49, align 8, !tbaa !48
  store ptr %49, ptr %44, align 8, !tbaa !72
  %61 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %62 = load i64, ptr %61, align 8, !tbaa !71
  %63 = icmp sgt i64 %62, 0
  br i1 %63, label %64, label %116

64:                                               ; preds = %57
  %65 = load ptr, ptr %14, align 8, !tbaa !80
  %66 = add nsw i64 %58, -1
  br label %67

67:                                               ; preds = %64, %113
  %68 = phi i64 [ 0, %64 ], [ %114, %113 ]
  %69 = shl nuw nsw i64 %68, 1
  %70 = getelementptr inbounds nuw i32, ptr %65, i64 %69
  %71 = load i32, ptr %70, align 4, !tbaa !15
  %72 = or disjoint i64 %69, 1
  %73 = getelementptr inbounds nuw i32, ptr %65, i64 %72
  %74 = load i32, ptr %73, align 4, !tbaa !15
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
  %92 = load i8, ptr %91, align 8, !tbaa !36
  %93 = icmp eq i8 %92, 0
  br i1 %93, label %108, label %94

94:                                               ; preds = %67, %101
  %95 = phi ptr [ %104, %101 ], [ %90, %67 ]
  %96 = phi i64 [ %103, %101 ], [ %89, %67 ]
  %97 = load i64, ptr %95, align 8, !tbaa !38
  %98 = icmp eq i64 %97, %80
  br i1 %98, label %99, label %101

99:                                               ; preds = %94
  %100 = getelementptr inbounds nuw i8, ptr %95, i64 8
  store i64 %68, ptr %100, align 8, !tbaa !39
  br label %113

101:                                              ; preds = %94
  %102 = add i64 %96, 1
  %103 = and i64 %102, %66
  %104 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %103
  %105 = getelementptr inbounds nuw i8, ptr %104, i64 16
  %106 = load i8, ptr %105, align 8, !tbaa !36
  %107 = icmp eq i8 %106, 0
  br i1 %107, label %108, label %94, !llvm.loop !40

108:                                              ; preds = %101, %67
  %109 = phi i64 [ %89, %67 ], [ %103, %101 ]
  %110 = phi ptr [ %90, %67 ], [ %104, %101 ]
  store i64 %80, ptr %110, align 8, !tbaa !38
  %111 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %109, i32 1
  store i64 %68, ptr %111, align 8, !tbaa !39
  %112 = getelementptr inbounds nuw %struct.EdgeHashEntry_s, ptr %60, i64 %109, i32 2
  store i8 1, ptr %112, align 8, !tbaa !36
  br label %113

113:                                              ; preds = %99, %108
  %114 = add nuw nsw i64 %68, 1
  %115 = icmp eq i64 %114, %62
  br i1 %115, label %116, label %67, !llvm.loop !124

116:                                              ; preds = %39, %113, %57, %30, %13, %42, %2
  %117 = phi i32 [ 1, %2 ], [ 0, %13 ], [ 1, %42 ], [ 1, %30 ], [ 1, %57 ], [ 1, %113 ], [ 1, %39 ]
  ret i32 %117
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_canonical_remove_edge(ptr noundef readnone %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %62

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %62, label %11, !llvm.loop !49

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 16, !tbaa !50
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 24
  %18 = load ptr, ptr %17, align 8, !tbaa !60
  %19 = icmp eq ptr %18, null
  br i1 %19, label %62, label %20

20:                                               ; preds = %16
  %21 = tail call fastcc i32 @canonical_pair_find_static(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %22 = icmp sgt i32 %21, -1
  br i1 %22, label %23, label %36

23:                                               ; preds = %20
  %24 = load ptr, ptr %17, align 8, !tbaa !60
  %25 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef %24, i32 noundef %21) #28
  tail call void @roaring_bitmap_remove(ptr noundef %24, i32 noundef %21) #28
  br i1 %25, label %26, label %32

26:                                               ; preds = %23
  %27 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %28 = load i64, ptr %27, align 8, !tbaa !69
  %29 = icmp sgt i64 %28, -1
  br i1 %29, label %30, label %32

30:                                               ; preds = %26
  %31 = add nsw i64 %28, -1
  store i64 %31, ptr %27, align 8, !tbaa !69
  br label %32

32:                                               ; preds = %30, %26, %23
  %33 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %34 = load i32, ptr %33, align 8, !tbaa !53
  %35 = icmp eq i32 %34, 3
  br i1 %35, label %59, label %60

36:                                               ; preds = %20
  %37 = tail call fastcc i64 @canonical_pair_find_extra(ptr noundef %13, i32 noundef %1, i32 noundef %2)
  %38 = icmp sgt i64 %37, -1
  br i1 %38, label %39, label %55

39:                                               ; preds = %36
  %40 = getelementptr inbounds nuw i8, ptr %13, i64 56
  %41 = load ptr, ptr %40, align 8, !tbaa !75
  %42 = getelementptr inbounds nuw i8, ptr %41, i64 %37
  %43 = load i8, ptr %42, align 1, !tbaa !76
  %44 = icmp eq i8 %43, 0
  br i1 %44, label %51, label %45

45:                                               ; preds = %39
  %46 = getelementptr inbounds nuw i8, ptr %13, i64 88
  %47 = load i64, ptr %46, align 8, !tbaa !69
  %48 = icmp sgt i64 %47, -1
  br i1 %48, label %49, label %51

49:                                               ; preds = %45
  %50 = add nsw i64 %47, -1
  store i64 %50, ptr %46, align 8, !tbaa !69
  br label %51

51:                                               ; preds = %49, %45, %39
  store i8 0, ptr %42, align 1, !tbaa !76
  %52 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %53 = load i32, ptr %52, align 8, !tbaa !53
  %54 = icmp eq i32 %53, 3
  br i1 %54, label %59, label %60

55:                                               ; preds = %36
  %56 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %57 = load i32, ptr %56, align 8, !tbaa !53
  %58 = icmp eq i32 %57, 3
  br i1 %58, label %59, label %60

59:                                               ; preds = %55, %51, %32
  tail call fastcc void @refresh_graph_counts_from_canonical(ptr noundef %13)
  br label %60

60:                                               ; preds = %59, %55, %51, %32
  %61 = getelementptr inbounds nuw i8, ptr %13, i64 80
  store i32 0, ptr %61, align 8, !tbaa !58
  br label %62

62:                                               ; preds = %8, %60, %3, %16
  %63 = phi i32 [ 0, %16 ], [ 0, %3 ], [ 1, %60 ], [ 0, %8 ]
  ret i32 %63
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_bcsr_add_edge(ptr noundef writeonly %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = alloca %struct.timespec, align 8
  %5 = alloca %struct.timespec, align 8
  %6 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %7 = icmp sgt i32 %6, 0
  br i1 %7, label %8, label %179

8:                                                ; preds = %3
  %9 = zext nneg i32 %6 to i64
  br label %13

10:                                               ; preds = %13
  %11 = add nuw nsw i64 %14, 1
  %12 = icmp eq i64 %11, %9
  br i1 %12, label %179, label %13, !llvm.loop !49

13:                                               ; preds = %10, %8
  %14 = phi i64 [ 0, %8 ], [ %11, %10 ]
  %15 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %14
  %16 = load ptr, ptr %15, align 16, !tbaa !50
  %17 = icmp eq ptr %16, %0
  br i1 %17, label %18, label %10

18:                                               ; preds = %13
  %19 = getelementptr inbounds nuw i8, ptr %15, i64 8
  %20 = load i32, ptr %19, align 8, !tbaa !53
  %21 = icmp eq i32 %20, 2
  br i1 %21, label %22, label %179

22:                                               ; preds = %18
  %23 = getelementptr inbounds nuw i8, ptr %15, i64 168
  %24 = load ptr, ptr %23, align 8, !tbaa !67
  %25 = icmp eq ptr %24, null
  br i1 %25, label %179, label %26

26:                                               ; preds = %22
  %27 = getelementptr inbounds nuw i8, ptr %15, i64 176
  %28 = load ptr, ptr %27, align 8, !tbaa !68
  %29 = icmp eq ptr %28, null
  br i1 %29, label %179, label %30

30:                                               ; preds = %26
  %31 = getelementptr inbounds nuw i8, ptr %15, i64 184
  %32 = load i32, ptr %31, align 8, !tbaa !66
  %33 = icmp sgt i32 %32, 0
  %34 = or i32 %2, %1
  %35 = icmp sgt i32 %34, -1
  %36 = and i1 %35, %33
  br i1 %36, label %37, label %179

37:                                               ; preds = %30
  %38 = getelementptr inbounds nuw i8, ptr %15, i64 188
  %39 = load i32, ptr %38, align 4, !tbaa !65
  %40 = udiv i32 %1, %32
  %41 = urem i32 %1, %32
  %42 = icmp slt i32 %40, %39
  br i1 %42, label %43, label %179

43:                                               ; preds = %37
  %44 = zext nneg i32 %40 to i64
  %45 = getelementptr inbounds nuw i32, ptr %24, i64 %44
  %46 = load i32, ptr %45, align 4, !tbaa !15
  %47 = add nuw nsw i32 %40, 1
  %48 = zext nneg i32 %47 to i64
  %49 = getelementptr inbounds nuw i32, ptr %24, i64 %48
  %50 = load i32, ptr %49, align 4, !tbaa !15
  %51 = icmp slt i32 %46, %50
  br i1 %51, label %52, label %80

52:                                               ; preds = %43
  %53 = sext i32 %46 to i64
  %54 = sext i32 %50 to i64
  br label %58

55:                                               ; preds = %67
  %56 = sext i32 %46 to i64
  %57 = sext i32 %50 to i64
  br label %70

58:                                               ; preds = %52, %67
  %59 = phi i64 [ %53, %52 ], [ %68, %67 ]
  %60 = getelementptr inbounds i32, ptr %28, i64 %59
  %61 = load i32, ptr %60, align 4, !tbaa !15
  %62 = icmp eq i32 %61, %41
  br i1 %62, label %63, label %67

63:                                               ; preds = %58
  %64 = getelementptr i8, ptr %60, i64 4
  %65 = load i32, ptr %64, align 4, !tbaa !15
  %66 = icmp eq i32 %65, %2
  br i1 %66, label %179, label %67

67:                                               ; preds = %58, %63
  %68 = add nsw i64 %59, 2
  %69 = icmp slt i64 %68, %54
  br i1 %69, label %58, label %55, !llvm.loop !125

70:                                               ; preds = %55, %75
  %71 = phi i64 [ %56, %55 ], [ %76, %75 ]
  %72 = getelementptr inbounds i32, ptr %28, i64 %71
  %73 = load i32, ptr %72, align 4, !tbaa !15
  %74 = icmp sgt i32 %73, %41
  br i1 %74, label %78, label %75

75:                                               ; preds = %70
  %76 = add nsw i64 %71, 2
  %77 = icmp slt i64 %76, %57
  br i1 %77, label %70, label %80, !llvm.loop !126

78:                                               ; preds = %70
  %79 = trunc nsw i64 %71 to i32
  br label %80

80:                                               ; preds = %75, %78, %43
  %81 = phi i32 [ %50, %43 ], [ %79, %78 ], [ %50, %75 ]
  %82 = zext nneg i32 %39 to i64
  %83 = getelementptr inbounds nuw i32, ptr %24, i64 %82
  %84 = load i32, ptr %83, align 4, !tbaa !15
  %85 = add nsw i32 %84, 2
  %86 = sext i32 %85 to i64
  %87 = shl nsw i64 %86, 2
  %88 = tail call ptr @realloc(ptr noundef nonnull %28, i64 noundef %87) #31
  %89 = icmp eq ptr %88, null
  br i1 %89, label %179, label %90

90:                                               ; preds = %80
  %91 = tail call fastcc i32 @shift_prof_on()
  %92 = icmp eq i32 %91, 0
  %93 = icmp eq ptr %88, %28
  %94 = or i1 %93, %92
  br i1 %94, label %102, label %95

95:                                               ; preds = %90
  %96 = load i64, ptr @shift_prof_reloc_cnt, align 8, !tbaa !20
  %97 = add nsw i64 %96, 1
  store i64 %97, ptr @shift_prof_reloc_cnt, align 8, !tbaa !20
  %98 = sext i32 %84 to i64
  %99 = shl nsw i64 %98, 2
  %100 = load i64, ptr @shift_prof_reloc_bytes, align 8, !tbaa !20
  %101 = add nsw i64 %100, %99
  store i64 %101, ptr @shift_prof_reloc_bytes, align 8, !tbaa !20
  br label %102

102:                                              ; preds = %95, %90
  %103 = icmp sgt i32 %84, %81
  br i1 %103, label %106, label %104

104:                                              ; preds = %102
  %105 = sext i32 %81 to i64
  br label %138

106:                                              ; preds = %102
  %107 = sub nsw i32 %84, %81
  %108 = sext i32 %107 to i64
  %109 = shl nsw i64 %108, 2
  %110 = tail call fastcc i32 @shift_prof_on()
  %111 = icmp eq i32 %110, 0
  br i1 %111, label %134, label %112

112:                                              ; preds = %106
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %4) #28
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #28
  %113 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %4) #28
  %114 = sext i32 %81 to i64
  %115 = getelementptr i32, ptr %88, i64 %114
  %116 = getelementptr i8, ptr %115, i64 8
  call void @llvm.memmove.p0.p0.i64(ptr align 4 %116, ptr nonnull align 4 %115, i64 %109, i1 false)
  %117 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #28
  %118 = load i64, ptr %5, align 8, !tbaa !16
  %119 = load i64, ptr %4, align 8, !tbaa !16
  %120 = sub nsw i64 %118, %119
  %121 = mul nsw i64 %120, 1000000000
  %122 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %123 = load i64, ptr %122, align 8, !tbaa !19
  %124 = getelementptr inbounds nuw i8, ptr %4, i64 8
  %125 = load i64, ptr %124, align 8, !tbaa !19
  %126 = sub i64 %123, %125
  %127 = add nsw i64 %126, %121
  %128 = load i64, ptr @shift_prof_ns, align 8, !tbaa !20
  %129 = add nsw i64 %127, %128
  store i64 %129, ptr @shift_prof_ns, align 8, !tbaa !20
  %130 = load i64, ptr @shift_prof_cnt, align 8, !tbaa !20
  %131 = add nsw i64 %130, 1
  store i64 %131, ptr @shift_prof_cnt, align 8, !tbaa !20
  %132 = load i64, ptr @shift_prof_bytes, align 8, !tbaa !20
  %133 = add nsw i64 %132, %109
  store i64 %133, ptr @shift_prof_bytes, align 8, !tbaa !20
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #28
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %4) #28
  br label %138

134:                                              ; preds = %106
  %135 = sext i32 %81 to i64
  %136 = getelementptr i32, ptr %88, i64 %135
  %137 = getelementptr i8, ptr %136, i64 8
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %137, ptr nonnull align 4 %136, i64 %109, i1 false)
  br label %138

138:                                              ; preds = %134, %112, %104
  %139 = phi i64 [ %105, %104 ], [ %114, %112 ], [ %135, %134 ]
  %140 = getelementptr inbounds i32, ptr %88, i64 %139
  store i32 %41, ptr %140, align 4, !tbaa !15
  %141 = getelementptr i8, ptr %140, i64 4
  store i32 %2, ptr %141, align 4, !tbaa !15
  %142 = call i32 @llvm.smax.i32(i32 %39, i32 %47)
  %143 = xor i32 %40, -1
  %144 = add i32 %142, %143
  %145 = zext i32 %144 to i64
  %146 = add nuw nsw i64 %145, 1
  %147 = icmp ult i32 %144, 7
  br i1 %147, label %164, label %148

148:                                              ; preds = %138
  %149 = and i64 %146, 8589934584
  %150 = add nuw nsw i64 %149, %48
  %151 = getelementptr i32, ptr %24, i64 %48
  br label %152

152:                                              ; preds = %152, %148
  %153 = phi i64 [ 0, %148 ], [ %160, %152 ]
  %154 = getelementptr i32, ptr %151, i64 %153
  %155 = getelementptr inbounds nuw i8, ptr %154, i64 16
  %156 = load <4 x i32>, ptr %154, align 4, !tbaa !15
  %157 = load <4 x i32>, ptr %155, align 4, !tbaa !15
  %158 = add nsw <4 x i32> %156, splat (i32 2)
  %159 = add nsw <4 x i32> %157, splat (i32 2)
  store <4 x i32> %158, ptr %154, align 4, !tbaa !15
  store <4 x i32> %159, ptr %155, align 4, !tbaa !15
  %160 = add nuw i64 %153, 8
  %161 = icmp eq i64 %160, %149
  br i1 %161, label %162, label %152, !llvm.loop !127

162:                                              ; preds = %152
  %163 = icmp eq i64 %146, %149
  br i1 %163, label %166, label %164

164:                                              ; preds = %138, %162
  %165 = phi i64 [ %48, %138 ], [ %150, %162 ]
  br label %171

166:                                              ; preds = %171, %162
  store ptr %88, ptr %27, align 8, !tbaa !68
  %167 = getelementptr inbounds nuw i8, ptr %15, i64 128
  %168 = load i64, ptr %167, align 8, !tbaa !57
  %169 = add nsw i64 %168, 1
  store i64 %169, ptr %167, align 8, !tbaa !57
  %170 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %169, ptr %170, align 8, !tbaa !20
  br label %179

171:                                              ; preds = %164, %171
  %172 = phi i64 [ %176, %171 ], [ %165, %164 ]
  %173 = getelementptr inbounds nuw i32, ptr %24, i64 %172
  %174 = load i32, ptr %173, align 4, !tbaa !15
  %175 = add nsw i32 %174, 2
  store i32 %175, ptr %173, align 4, !tbaa !15
  %176 = add nuw nsw i64 %172, 1
  %177 = trunc nuw i64 %172 to i32
  %178 = icmp sgt i32 %39, %177
  br i1 %178, label %171, label %166, !llvm.loop !128

179:                                              ; preds = %10, %63, %3, %37, %166, %80, %22, %26, %30, %18
  %180 = phi i32 [ 0, %18 ], [ 0, %30 ], [ 0, %26 ], [ 0, %22 ], [ 0, %37 ], [ 1, %166 ], [ 0, %80 ], [ 0, %3 ], [ 1, %63 ], [ 0, %10 ]
  ret i32 %180
}

; Function Attrs: nofree nounwind uwtable
define internal fastcc range(i32 -1, 2) i32 @shift_prof_on() unnamed_addr #2 {
  %1 = load i32, ptr @shift_prof_state, align 4, !tbaa !15
  %2 = icmp slt i32 %1, 0
  br i1 %2, label %3, label %10

3:                                                ; preds = %0
  %4 = tail call ptr @getenv(ptr noundef nonnull @.str.13) #28
  %5 = icmp eq ptr %4, null
  br i1 %5, label %9, label %6

6:                                                ; preds = %3
  store i32 1, ptr @shift_prof_state, align 4, !tbaa !15
  %7 = tail call i32 @atexit(ptr noundef nonnull @shift_prof_dump) #28
  %8 = load i32, ptr @shift_prof_state, align 4, !tbaa !15
  br label %10

9:                                                ; preds = %3
  store i32 0, ptr @shift_prof_state, align 4, !tbaa !15
  br label %10

10:                                               ; preds = %6, %9, %0
  %11 = phi i32 [ %8, %6 ], [ 0, %9 ], [ %1, %0 ]
  ret i32 %11
}

; Function Attrs: nounwind
declare i32 @clock_gettime(i32 noundef, ptr noundef) local_unnamed_addr #16

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #7

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define dso_local range(i32 0, 2) i32 @autograph_bcsr_remove_edge(ptr noundef writeonly %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #9 {
  %4 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %6, label %121

6:                                                ; preds = %3
  %7 = zext nneg i32 %4 to i64
  br label %11

8:                                                ; preds = %11
  %9 = add nuw nsw i64 %12, 1
  %10 = icmp eq i64 %9, %7
  br i1 %10, label %121, label %11, !llvm.loop !49

11:                                               ; preds = %8, %6
  %12 = phi i64 [ 0, %6 ], [ %9, %8 ]
  %13 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 16, !tbaa !50
  %15 = icmp eq ptr %14, %0
  br i1 %15, label %16, label %8

16:                                               ; preds = %11
  %17 = getelementptr inbounds nuw i8, ptr %13, i64 8
  %18 = load i32, ptr %17, align 8, !tbaa !53
  %19 = icmp eq i32 %18, 2
  br i1 %19, label %20, label %121

20:                                               ; preds = %16
  %21 = getelementptr inbounds nuw i8, ptr %13, i64 168
  %22 = load ptr, ptr %21, align 8, !tbaa !67
  %23 = icmp eq ptr %22, null
  br i1 %23, label %121, label %24

24:                                               ; preds = %20
  %25 = getelementptr inbounds nuw i8, ptr %13, i64 176
  %26 = load ptr, ptr %25, align 8, !tbaa !68
  %27 = icmp eq ptr %26, null
  br i1 %27, label %121, label %28

28:                                               ; preds = %24
  %29 = getelementptr inbounds nuw i8, ptr %13, i64 184
  %30 = load i32, ptr %29, align 8, !tbaa !66
  %31 = icmp sgt i32 %30, 0
  %32 = or i32 %2, %1
  %33 = icmp sgt i32 %32, -1
  %34 = and i1 %33, %31
  br i1 %34, label %35, label %121

35:                                               ; preds = %28
  %36 = getelementptr inbounds nuw i8, ptr %13, i64 188
  %37 = load i32, ptr %36, align 4, !tbaa !65
  %38 = udiv i32 %1, %30
  %39 = urem i32 %1, %30
  %40 = icmp slt i32 %38, %37
  br i1 %40, label %41, label %121

41:                                               ; preds = %35
  %42 = zext nneg i32 %38 to i64
  %43 = getelementptr inbounds nuw i32, ptr %22, i64 %42
  %44 = load i32, ptr %43, align 4, !tbaa !15
  %45 = add nuw nsw i32 %38, 1
  %46 = zext nneg i32 %45 to i64
  %47 = getelementptr inbounds nuw i32, ptr %22, i64 %46
  %48 = load i32, ptr %47, align 4, !tbaa !15
  %49 = icmp slt i32 %44, %48
  br i1 %49, label %50, label %121

50:                                               ; preds = %41
  %51 = sext i32 %44 to i64
  %52 = sext i32 %48 to i64
  br label %53

53:                                               ; preds = %50, %62
  %54 = phi i64 [ %51, %50 ], [ %63, %62 ]
  %55 = getelementptr inbounds i32, ptr %26, i64 %54
  %56 = load i32, ptr %55, align 4, !tbaa !15
  %57 = icmp eq i32 %56, %39
  br i1 %57, label %58, label %62

58:                                               ; preds = %53
  %59 = getelementptr i8, ptr %55, i64 4
  %60 = load i32, ptr %59, align 4, !tbaa !15
  %61 = icmp eq i32 %60, %2
  br i1 %61, label %65, label %62

62:                                               ; preds = %53, %58
  %63 = add nsw i64 %54, 2
  %64 = icmp slt i64 %63, %52
  br i1 %64, label %53, label %121, !llvm.loop !129

65:                                               ; preds = %58
  %66 = trunc nsw i64 %54 to i32
  %67 = icmp slt i64 %54, 0
  br i1 %67, label %121, label %68

68:                                               ; preds = %65
  %69 = zext nneg i32 %37 to i64
  %70 = getelementptr inbounds nuw i32, ptr %22, i64 %69
  %71 = load i32, ptr %70, align 4, !tbaa !15
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
  %98 = load <4 x i32>, ptr %96, align 4, !tbaa !15
  %99 = load <4 x i32>, ptr %97, align 4, !tbaa !15
  %100 = add nsw <4 x i32> %98, splat (i32 -2)
  %101 = add nsw <4 x i32> %99, splat (i32 -2)
  store <4 x i32> %100, ptr %96, align 4, !tbaa !15
  store <4 x i32> %101, ptr %97, align 4, !tbaa !15
  %102 = add nuw i64 %95, 8
  %103 = icmp eq i64 %102, %91
  br i1 %103, label %104, label %94, !llvm.loop !130

104:                                              ; preds = %94
  %105 = icmp eq i64 %88, %91
  br i1 %105, label %108, label %106

106:                                              ; preds = %83, %104
  %107 = phi i64 [ %46, %83 ], [ %92, %104 ]
  br label %113

108:                                              ; preds = %113, %104
  %109 = getelementptr inbounds nuw i8, ptr %13, i64 128
  %110 = load i64, ptr %109, align 8, !tbaa !57
  %111 = add nsw i64 %110, -1
  store i64 %111, ptr %109, align 8, !tbaa !57
  %112 = getelementptr inbounds nuw i8, ptr %0, i64 8
  store i64 %111, ptr %112, align 8, !tbaa !20
  br label %121

113:                                              ; preds = %106, %113
  %114 = phi i64 [ %118, %113 ], [ %107, %106 ]
  %115 = getelementptr inbounds nuw i32, ptr %22, i64 %114
  %116 = load i32, ptr %115, align 4, !tbaa !15
  %117 = add nsw i32 %116, -2
  store i32 %117, ptr %115, align 4, !tbaa !15
  %118 = add nuw nsw i64 %114, 1
  %119 = trunc nuw i64 %114 to i32
  %120 = icmp sgt i32 %37, %119
  br i1 %120, label %113, label %108, !llvm.loop !131

121:                                              ; preds = %8, %62, %41, %3, %35, %65, %108, %20, %24, %28, %16
  %122 = phi i32 [ 0, %16 ], [ 0, %28 ], [ 0, %24 ], [ 0, %20 ], [ 0, %35 ], [ 1, %108 ], [ 0, %65 ], [ 0, %3 ], [ 0, %41 ], [ 0, %62 ], [ 0, %8 ]
  ret i32 %122
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_neighbor_iter_init(ptr noundef readnone %0, i64 noundef %1, ptr nocapture noundef writeonly %2) local_unnamed_addr #0 {
  %4 = alloca %struct.timespec, align 8
  %5 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %7, label %17

7:                                                ; preds = %3
  %8 = zext nneg i32 %5 to i64
  br label %12

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %13, 1
  %11 = icmp eq i64 %10, %8
  br i1 %11, label %17, label %12, !llvm.loop !49

12:                                               ; preds = %9, %7
  %13 = phi i64 [ 0, %7 ], [ %10, %9 ]
  %14 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %13
  %15 = load ptr, ptr %14, align 16, !tbaa !50
  %16 = icmp eq ptr %15, %0
  br i1 %16, label %17, label %9

17:                                               ; preds = %9, %12, %3
  %18 = phi ptr [ null, %3 ], [ null, %9 ], [ %14, %12 ]
  store ptr %18, ptr %2, align 8, !tbaa !132
  %19 = getelementptr inbounds nuw i8, ptr %2, i64 8
  store i64 %1, ptr %19, align 8, !tbaa !134
  %20 = getelementptr inbounds nuw i8, ptr %2, i64 36
  store i32 3, ptr %20, align 4, !tbaa !135
  %21 = getelementptr inbounds nuw i8, ptr %2, i64 40
  store i32 0, ptr %21, align 8, !tbaa !136
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %4) #28
  %22 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %4) #28
  %23 = load i64, ptr %4, align 8, !tbaa !16
  %24 = mul i64 %23, 1000000000
  %25 = getelementptr inbounds nuw i8, ptr %4, i64 8
  %26 = load i64, ptr %25, align 8, !tbaa !19
  %27 = add i64 %24, %26
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %4) #28
  %28 = call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  store i64 %27, ptr %28, align 8, !tbaa !20
  %29 = icmp eq ptr %18, null
  br i1 %29, label %30, label %32

30:                                               ; preds = %17
  %31 = getelementptr inbounds nuw i8, ptr %2, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(20) %31, i8 0, i64 20, i1 false)
  br label %131

32:                                               ; preds = %17
  %33 = getelementptr inbounds nuw i8, ptr %18, i64 8
  %34 = load i32, ptr %33, align 8, !tbaa !53
  store i32 %34, ptr %20, align 4, !tbaa !135
  %35 = getelementptr inbounds nuw i8, ptr %18, i64 192
  %36 = load i32, ptr %35, align 8, !tbaa !82
  store i32 %36, ptr %21, align 8, !tbaa !136
  switch i32 %34, label %125 [
    i32 0, label %37
    i32 1, label %58
    i32 2, label %79
  ]

37:                                               ; preds = %32
  %38 = getelementptr inbounds nuw i8, ptr %18, i64 96
  %39 = load ptr, ptr %38, align 8, !tbaa !54
  %40 = icmp ne ptr %39, null
  %41 = icmp sgt i64 %1, -1
  %42 = and i1 %41, %40
  br i1 %42, label %43, label %54

43:                                               ; preds = %37
  %44 = getelementptr inbounds nuw i8, ptr %18, i64 120
  %45 = load i64, ptr %44, align 8, !tbaa !56
  %46 = icmp slt i64 %1, %45
  br i1 %46, label %47, label %54

47:                                               ; preds = %43
  %48 = getelementptr inbounds nuw i64, ptr %39, i64 %1
  %49 = load i64, ptr %48, align 8, !tbaa !20
  %50 = getelementptr inbounds nuw i8, ptr %2, i64 16
  store i64 %49, ptr %50, align 8, !tbaa !137
  %51 = getelementptr inbounds nuw i8, ptr %48, i64 8
  %52 = load i64, ptr %51, align 8, !tbaa !20
  %53 = getelementptr inbounds nuw i8, ptr %2, i64 24
  store i64 %52, ptr %53, align 8, !tbaa !138
  br label %56

54:                                               ; preds = %43, %37
  %55 = getelementptr inbounds nuw i8, ptr %2, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %55, i8 0, i64 16, i1 false)
  br label %56

56:                                               ; preds = %54, %47
  %57 = getelementptr inbounds nuw i8, ptr %2, i64 32
  store i32 0, ptr %57, align 8, !tbaa !139
  br label %131

58:                                               ; preds = %32
  %59 = getelementptr inbounds nuw i8, ptr %18, i64 144
  %60 = load ptr, ptr %59, align 8, !tbaa !63
  %61 = icmp ne ptr %60, null
  %62 = icmp sgt i64 %1, -1
  %63 = and i1 %62, %61
  br i1 %63, label %64, label %75

64:                                               ; preds = %58
  %65 = getelementptr inbounds nuw i8, ptr %18, i64 120
  %66 = load i64, ptr %65, align 8, !tbaa !56
  %67 = icmp slt i64 %1, %66
  br i1 %67, label %68, label %75

68:                                               ; preds = %64
  %69 = getelementptr inbounds nuw i64, ptr %60, i64 %1
  %70 = load i64, ptr %69, align 8, !tbaa !20
  %71 = getelementptr inbounds nuw i8, ptr %2, i64 16
  store i64 %70, ptr %71, align 8, !tbaa !137
  %72 = getelementptr inbounds nuw i8, ptr %69, i64 8
  %73 = load i64, ptr %72, align 8, !tbaa !20
  %74 = getelementptr inbounds nuw i8, ptr %2, i64 24
  store i64 %73, ptr %74, align 8, !tbaa !138
  br label %77

75:                                               ; preds = %64, %58
  %76 = getelementptr inbounds nuw i8, ptr %2, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %76, i8 0, i64 16, i1 false)
  br label %77

77:                                               ; preds = %75, %68
  %78 = getelementptr inbounds nuw i8, ptr %2, i64 32
  store i32 0, ptr %78, align 8, !tbaa !139
  br label %131

79:                                               ; preds = %32
  %80 = getelementptr inbounds nuw i8, ptr %18, i64 168
  %81 = load ptr, ptr %80, align 8, !tbaa !67
  %82 = icmp eq ptr %81, null
  br i1 %82, label %123, label %83

83:                                               ; preds = %79
  %84 = getelementptr inbounds nuw i8, ptr %18, i64 176
  %85 = load ptr, ptr %84, align 8, !tbaa !68
  %86 = icmp eq ptr %85, null
  br i1 %86, label %123, label %87

87:                                               ; preds = %83
  %88 = getelementptr inbounds nuw i8, ptr %18, i64 184
  %89 = load i32, ptr %88, align 8, !tbaa !66
  %90 = icmp sgt i32 %89, 0
  %91 = icmp sgt i64 %1, -1
  %92 = and i1 %91, %90
  br i1 %92, label %93, label %123

93:                                               ; preds = %87
  %94 = getelementptr inbounds nuw i8, ptr %18, i64 120
  %95 = load i64, ptr %94, align 8, !tbaa !56
  %96 = icmp slt i64 %1, %95
  br i1 %96, label %97, label %123

97:                                               ; preds = %93
  %98 = zext nneg i32 %89 to i64
  %99 = udiv i64 %1, %98
  %100 = urem i64 %1, %98
  %101 = trunc i64 %99 to i32
  %102 = icmp sgt i32 %101, -1
  br i1 %102, label %103, label %121

103:                                              ; preds = %97
  %104 = getelementptr inbounds nuw i8, ptr %18, i64 188
  %105 = load i32, ptr %104, align 4, !tbaa !65
  %106 = icmp sgt i32 %105, %101
  br i1 %106, label %107, label %121

107:                                              ; preds = %103
  %108 = and i64 %99, 2147483647
  %109 = getelementptr inbounds nuw i32, ptr %81, i64 %108
  %110 = load i32, ptr %109, align 4, !tbaa !15
  %111 = sext i32 %110 to i64
  %112 = getelementptr inbounds nuw i8, ptr %2, i64 16
  store i64 %111, ptr %112, align 8, !tbaa !137
  %113 = add nuw nsw i64 %99, 1
  %114 = and i64 %113, 4294967295
  %115 = getelementptr inbounds nuw i32, ptr %81, i64 %114
  %116 = load i32, ptr %115, align 4, !tbaa !15
  %117 = sext i32 %116 to i64
  %118 = getelementptr inbounds nuw i8, ptr %2, i64 24
  store i64 %117, ptr %118, align 8, !tbaa !138
  %119 = trunc nuw nsw i64 %100 to i32
  %120 = getelementptr inbounds nuw i8, ptr %2, i64 32
  store i32 %119, ptr %120, align 8, !tbaa !139
  br label %131

121:                                              ; preds = %103, %97
  %122 = getelementptr inbounds nuw i8, ptr %2, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(20) %122, i8 0, i64 20, i1 false)
  br label %131

123:                                              ; preds = %93, %87, %83, %79
  %124 = getelementptr inbounds nuw i8, ptr %2, i64 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(20) %124, i8 0, i64 20, i1 false)
  br label %131

125:                                              ; preds = %32
  %126 = getelementptr inbounds nuw i8, ptr %2, i64 16
  store i64 0, ptr %126, align 8, !tbaa !137
  %127 = getelementptr inbounds nuw i8, ptr %18, i64 40
  %128 = load i64, ptr %127, align 8, !tbaa !74
  %129 = getelementptr inbounds nuw i8, ptr %2, i64 24
  store i64 %128, ptr %129, align 8, !tbaa !138
  %130 = getelementptr inbounds nuw i8, ptr %2, i64 32
  store i32 0, ptr %130, align 8, !tbaa !139
  store i32 3, ptr %20, align 4, !tbaa !135
  br label %131

131:                                              ; preds = %56, %77, %125, %123, %121, %107, %30
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @autograph_neighbor_iter_next(ptr nocapture noundef %0, ptr noundef writeonly %1) local_unnamed_addr #0 {
  %3 = alloca %struct.timespec, align 8
  %4 = alloca %struct.timespec, align 8
  %5 = alloca %struct.timespec, align 8
  %6 = alloca %struct.timespec, align 8
  %7 = alloca %struct.timespec, align 8
  %8 = alloca %struct.timespec, align 8
  %9 = load ptr, ptr %0, align 8, !tbaa !132
  %10 = icmp ne ptr %9, null
  %11 = icmp ne ptr %1, null
  %12 = and i1 %11, %10
  br i1 %12, label %13, label %287

13:                                               ; preds = %2
  %14 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %15 = load i32, ptr %14, align 8, !tbaa !136
  %16 = getelementptr inbounds nuw i8, ptr %9, i64 192
  %17 = load i32, ptr %16, align 8, !tbaa !82
  %18 = icmp eq i32 %15, %17
  br i1 %18, label %19, label %25

19:                                               ; preds = %13
  %20 = getelementptr inbounds nuw i8, ptr %0, i64 36
  %21 = load i32, ptr %20, align 4, !tbaa !135
  %22 = getelementptr inbounds nuw i8, ptr %9, i64 8
  %23 = load i32, ptr %22, align 8, !tbaa !53
  %24 = icmp eq i32 %21, %23
  br i1 %24, label %44, label %25

25:                                               ; preds = %19, %13
  %26 = load ptr, ptr %9, align 8, !tbaa !50
  %27 = icmp eq ptr %26, null
  br i1 %27, label %287, label %28

28:                                               ; preds = %25
  %29 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %30 = load i64, ptr %29, align 8, !tbaa !134
  tail call void @autograph_neighbor_iter_init(ptr noundef nonnull %26, i64 noundef %30, ptr noundef nonnull %0)
  %31 = load ptr, ptr %0, align 8, !tbaa !132
  %32 = icmp eq ptr %31, null
  br i1 %32, label %287, label %33

33:                                               ; preds = %28
  %34 = load i32, ptr %14, align 8, !tbaa !136
  %35 = getelementptr inbounds nuw i8, ptr %31, i64 192
  %36 = load i32, ptr %35, align 8, !tbaa !82
  %37 = icmp eq i32 %34, %36
  br i1 %37, label %38, label %287

38:                                               ; preds = %33
  %39 = getelementptr inbounds nuw i8, ptr %0, i64 36
  %40 = load i32, ptr %39, align 4, !tbaa !135
  %41 = getelementptr inbounds nuw i8, ptr %31, i64 8
  %42 = load i32, ptr %41, align 8, !tbaa !53
  %43 = icmp eq i32 %40, %42
  br i1 %43, label %44, label %287

44:                                               ; preds = %38, %19
  %45 = phi i32 [ %21, %19 ], [ %40, %38 ]
  %46 = phi ptr [ %9, %19 ], [ %31, %38 ]
  switch i32 %45, label %166 [
    i32 0, label %47
    i32 1, label %76
    i32 2, label %109
  ]

47:                                               ; preds = %44
  %48 = getelementptr inbounds nuw i8, ptr %46, i64 104
  %49 = load ptr, ptr %48, align 8, !tbaa !55
  %50 = icmp eq ptr %49, null
  br i1 %50, label %57, label %51

51:                                               ; preds = %47
  %52 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %53 = load i64, ptr %52, align 8, !tbaa !137
  %54 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %55 = load i64, ptr %54, align 8, !tbaa !138
  %56 = icmp slt i64 %53, %55
  br i1 %56, label %72, label %57

57:                                               ; preds = %47, %51
  %58 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  %59 = load i64, ptr %58, align 8, !tbaa !20
  %60 = icmp eq i64 %59, 0
  br i1 %60, label %287, label %61

61:                                               ; preds = %57
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %8) #28
  %62 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %8) #28
  %63 = load i64, ptr %8, align 8, !tbaa !16
  %64 = mul i64 %63, 1000000000
  %65 = getelementptr inbounds nuw i8, ptr %8, i64 8
  %66 = load i64, ptr %65, align 8, !tbaa !19
  %67 = add i64 %64, %66
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %8) #28
  %68 = load i64, ptr %58, align 8, !tbaa !20
  %69 = sub i64 %67, %68
  %70 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %71 = atomicrmw add ptr @g_kernel_measured_ns, i64 %69 monotonic, align 8
  store i64 0, ptr %58, align 8, !tbaa !20
  br label %287

72:                                               ; preds = %51
  %73 = add nsw i64 %53, 1
  store i64 %73, ptr %52, align 8, !tbaa !137
  %74 = getelementptr inbounds i32, ptr %49, i64 %53
  %75 = load i32, ptr %74, align 4, !tbaa !15
  store i32 %75, ptr %1, align 4, !tbaa !15
  br label %287

76:                                               ; preds = %44
  %77 = getelementptr inbounds nuw i8, ptr %46, i64 152
  %78 = load ptr, ptr %77, align 8, !tbaa !64
  %79 = icmp eq ptr %78, null
  br i1 %79, label %287, label %80

80:                                               ; preds = %76
  %81 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %82 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %83 = load i64, ptr %82, align 8, !tbaa !138
  %84 = load i64, ptr %81, align 8, !tbaa !137
  br label %85

85:                                               ; preds = %80, %88
  %86 = phi i64 [ %84, %80 ], [ %89, %88 ]
  %87 = icmp slt i64 %86, %83
  br i1 %87, label %88, label %94

88:                                               ; preds = %85
  %89 = add nsw i64 %86, 1
  store i64 %89, ptr %81, align 8, !tbaa !137
  %90 = getelementptr inbounds i32, ptr %78, i64 %86
  %91 = load i32, ptr %90, align 4, !tbaa !15
  %92 = icmp eq i32 %91, -1
  br i1 %92, label %85, label %93, !llvm.loop !140

93:                                               ; preds = %88
  store i32 %91, ptr %1, align 4, !tbaa !15
  br label %287

94:                                               ; preds = %85
  %95 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  %96 = load i64, ptr %95, align 8, !tbaa !20
  %97 = icmp eq i64 %96, 0
  br i1 %97, label %287, label %98

98:                                               ; preds = %94
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %7) #28
  %99 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %7) #28
  %100 = load i64, ptr %7, align 8, !tbaa !16
  %101 = mul i64 %100, 1000000000
  %102 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %103 = load i64, ptr %102, align 8, !tbaa !19
  %104 = add i64 %101, %103
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %7) #28
  %105 = load i64, ptr %95, align 8, !tbaa !20
  %106 = sub i64 %104, %105
  %107 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %108 = atomicrmw add ptr @g_kernel_measured_ns, i64 %106 monotonic, align 8
  store i64 0, ptr %95, align 8, !tbaa !20
  br label %287

109:                                              ; preds = %44
  %110 = getelementptr inbounds nuw i8, ptr %46, i64 176
  %111 = load ptr, ptr %110, align 8, !tbaa !68
  %112 = icmp eq ptr %111, null
  br i1 %112, label %113, label %128

113:                                              ; preds = %109
  %114 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  %115 = load i64, ptr %114, align 8, !tbaa !20
  %116 = icmp eq i64 %115, 0
  br i1 %116, label %287, label %117

117:                                              ; preds = %113
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #28
  %118 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #28
  %119 = load i64, ptr %6, align 8, !tbaa !16
  %120 = mul i64 %119, 1000000000
  %121 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %122 = load i64, ptr %121, align 8, !tbaa !19
  %123 = add i64 %120, %122
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #28
  %124 = load i64, ptr %114, align 8, !tbaa !20
  %125 = sub i64 %123, %124
  %126 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %127 = atomicrmw add ptr @g_kernel_measured_ns, i64 %125 monotonic, align 8
  store i64 0, ptr %114, align 8, !tbaa !20
  br label %287

128:                                              ; preds = %109
  %129 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %130 = load i32, ptr %129, align 8, !tbaa !139
  %131 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %132 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %133 = load i64, ptr %132, align 8, !tbaa !138
  %134 = load i64, ptr %131, align 8, !tbaa !137
  %135 = icmp slt i64 %134, %133
  br i1 %135, label %136, label %151

136:                                              ; preds = %128, %148
  %137 = phi i64 [ %149, %148 ], [ %134, %128 ]
  %138 = getelementptr inbounds i32, ptr %111, i64 %137
  %139 = load i32, ptr %138, align 4, !tbaa !15
  %140 = icmp eq i32 %139, %130
  br i1 %140, label %141, label %146

141:                                              ; preds = %136
  %142 = getelementptr inbounds i32, ptr %111, i64 %137
  %143 = getelementptr i8, ptr %142, i64 4
  %144 = load i32, ptr %143, align 4, !tbaa !15
  store i32 %144, ptr %1, align 4, !tbaa !15
  %145 = add nsw i64 %137, 2
  store i64 %145, ptr %131, align 8, !tbaa !137
  br label %287

146:                                              ; preds = %136
  %147 = icmp sgt i32 %139, %130
  br i1 %147, label %151, label %148

148:                                              ; preds = %146
  %149 = add nsw i64 %137, 2
  store i64 %149, ptr %131, align 8, !tbaa !137
  %150 = icmp slt i64 %149, %133
  br i1 %150, label %136, label %151

151:                                              ; preds = %148, %146, %128
  %152 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  %153 = load i64, ptr %152, align 8, !tbaa !20
  %154 = icmp eq i64 %153, 0
  br i1 %154, label %287, label %155

155:                                              ; preds = %151
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #28
  %156 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #28
  %157 = load i64, ptr %5, align 8, !tbaa !16
  %158 = mul i64 %157, 1000000000
  %159 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %160 = load i64, ptr %159, align 8, !tbaa !19
  %161 = add i64 %158, %160
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #28
  %162 = load i64, ptr %152, align 8, !tbaa !20
  %163 = sub i64 %161, %162
  %164 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %165 = atomicrmw add ptr @g_kernel_measured_ns, i64 %163 monotonic, align 8
  store i64 0, ptr %152, align 8, !tbaa !20
  br label %287

166:                                              ; preds = %44
  %167 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %168 = load i32, ptr %167, align 8, !tbaa !139
  %169 = icmp eq i32 %168, 0
  br i1 %169, label %170, label %215

170:                                              ; preds = %166
  %171 = getelementptr inbounds nuw i8, ptr %46, i64 24
  %172 = load ptr, ptr %171, align 8, !tbaa !60
  %173 = getelementptr inbounds nuw i8, ptr %46, i64 32
  %174 = load ptr, ptr %173, align 8, !tbaa !61
  %175 = icmp ne ptr %172, null
  %176 = icmp ne ptr %174, null
  %177 = select i1 %175, i1 %176, i1 false
  br i1 %177, label %178, label %186

178:                                              ; preds = %170
  %179 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %180 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %181 = load i64, ptr %179, align 8, !tbaa !137
  %182 = load i64, ptr %180, align 8, !tbaa !138
  %183 = icmp slt i64 %181, %182
  br i1 %183, label %184, label %212

184:                                              ; preds = %178
  %185 = getelementptr inbounds nuw i8, ptr %0, i64 8
  br label %191

186:                                              ; preds = %170
  store i32 1, ptr %167, align 8, !tbaa !139
  %187 = getelementptr inbounds nuw i8, ptr %0, i64 16
  store i64 0, ptr %187, align 8, !tbaa !137
  %188 = getelementptr inbounds nuw i8, ptr %46, i64 64
  %189 = load i64, ptr %188, align 8, !tbaa !71
  %190 = getelementptr inbounds nuw i8, ptr %0, i64 24
  store i64 %189, ptr %190, align 8, !tbaa !138
  br label %215

191:                                              ; preds = %184, %208
  %192 = phi i64 [ %181, %184 ], [ %209, %208 ]
  %193 = add nsw i64 %192, 1
  store i64 %193, ptr %179, align 8, !tbaa !137
  %194 = trunc i64 %192 to i32
  %195 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef nonnull %172, i32 noundef %194) #28
  br i1 %195, label %196, label %208, !llvm.loop !141

196:                                              ; preds = %191
  %197 = getelementptr inbounds %struct.EdgePair, ptr %174, i64 %192
  %198 = load i32, ptr %197, align 4, !tbaa !21
  %199 = load i64, ptr %185, align 8, !tbaa !134
  %200 = trunc i64 %199 to i32
  %201 = icmp eq i32 %198, %200
  %202 = getelementptr inbounds nuw i8, ptr %197, i64 4
  %203 = load i32, ptr %202, align 4, !tbaa !23
  br i1 %201, label %204, label %205

204:                                              ; preds = %196
  store i32 %203, ptr %1, align 4, !tbaa !15
  br label %287

205:                                              ; preds = %196
  %206 = icmp eq i32 %203, %200
  br i1 %206, label %207, label %208

207:                                              ; preds = %205
  store i32 %198, ptr %1, align 4, !tbaa !15
  br label %287

208:                                              ; preds = %205, %191
  %209 = load i64, ptr %179, align 8, !tbaa !137
  %210 = load i64, ptr %180, align 8, !tbaa !138
  %211 = icmp slt i64 %209, %210
  br i1 %211, label %191, label %212

212:                                              ; preds = %208, %178
  store i32 1, ptr %167, align 8, !tbaa !139
  store i64 0, ptr %179, align 8, !tbaa !137
  %213 = getelementptr inbounds nuw i8, ptr %46, i64 64
  %214 = load i64, ptr %213, align 8, !tbaa !71
  store i64 %214, ptr %180, align 8, !tbaa !138
  br label %215

215:                                              ; preds = %186, %212, %166
  %216 = getelementptr inbounds nuw i8, ptr %46, i64 48
  %217 = load ptr, ptr %216, align 8, !tbaa !80
  %218 = icmp eq ptr %217, null
  br i1 %218, label %233, label %219

219:                                              ; preds = %215
  %220 = getelementptr inbounds nuw i8, ptr %46, i64 56
  %221 = load ptr, ptr %220, align 8, !tbaa !75
  %222 = icmp eq ptr %221, null
  br i1 %222, label %233, label %223

223:                                              ; preds = %219
  %224 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %225 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %226 = load i64, ptr %225, align 8, !tbaa !138
  %227 = load i64, ptr %224, align 8, !tbaa !137
  %228 = icmp slt i64 %227, %226
  br i1 %228, label %229, label %272

229:                                              ; preds = %223
  %230 = getelementptr inbounds nuw i8, ptr %46, i64 64
  %231 = load i64, ptr %230, align 8, !tbaa !71
  %232 = getelementptr inbounds nuw i8, ptr %0, i64 8
  br label %248

233:                                              ; preds = %215, %219
  %234 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  %235 = load i64, ptr %234, align 8, !tbaa !20
  %236 = icmp eq i64 %235, 0
  br i1 %236, label %287, label %237

237:                                              ; preds = %233
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %4) #28
  %238 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %4) #28
  %239 = load i64, ptr %4, align 8, !tbaa !16
  %240 = mul i64 %239, 1000000000
  %241 = getelementptr inbounds nuw i8, ptr %4, i64 8
  %242 = load i64, ptr %241, align 8, !tbaa !19
  %243 = add i64 %240, %242
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %4) #28
  %244 = load i64, ptr %234, align 8, !tbaa !20
  %245 = sub i64 %243, %244
  %246 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %247 = atomicrmw add ptr @g_kernel_measured_ns, i64 %245 monotonic, align 8
  store i64 0, ptr %234, align 8, !tbaa !20
  br label %287

248:                                              ; preds = %229, %270
  %249 = phi i64 [ %227, %229 ], [ %252, %270 ]
  %250 = icmp slt i64 %249, %231
  br i1 %250, label %251, label %272

251:                                              ; preds = %248
  %252 = add nsw i64 %249, 1
  store i64 %252, ptr %224, align 8, !tbaa !137
  %253 = getelementptr inbounds i8, ptr %221, i64 %249
  %254 = load i8, ptr %253, align 1, !tbaa !76
  %255 = icmp eq i8 %254, 0
  br i1 %255, label %270, label %256, !llvm.loop !142

256:                                              ; preds = %251
  %257 = shl nsw i64 %249, 1
  %258 = getelementptr inbounds i32, ptr %217, i64 %257
  %259 = load i32, ptr %258, align 4, !tbaa !15
  %260 = or disjoint i64 %257, 1
  %261 = getelementptr inbounds i32, ptr %217, i64 %260
  %262 = load i32, ptr %261, align 4, !tbaa !15
  %263 = load i64, ptr %232, align 8, !tbaa !134
  %264 = trunc i64 %263 to i32
  %265 = icmp eq i32 %259, %264
  br i1 %265, label %266, label %267

266:                                              ; preds = %256
  store i32 %262, ptr %1, align 4, !tbaa !15
  br label %287

267:                                              ; preds = %256
  %268 = icmp eq i32 %262, %264
  br i1 %268, label %269, label %270

269:                                              ; preds = %267
  store i32 %259, ptr %1, align 4, !tbaa !15
  br label %287

270:                                              ; preds = %267, %251
  %271 = icmp eq i64 %252, %226
  br i1 %271, label %272, label %248

272:                                              ; preds = %270, %248, %223
  %273 = tail call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_neighbor_scan_start_ns)
  %274 = load i64, ptr %273, align 8, !tbaa !20
  %275 = icmp eq i64 %274, 0
  br i1 %275, label %287, label %276

276:                                              ; preds = %272
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %3) #28
  %277 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %3) #28
  %278 = load i64, ptr %3, align 8, !tbaa !16
  %279 = mul i64 %278, 1000000000
  %280 = getelementptr inbounds nuw i8, ptr %3, i64 8
  %281 = load i64, ptr %280, align 8, !tbaa !19
  %282 = add i64 %279, %281
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %3) #28
  %283 = load i64, ptr %273, align 8, !tbaa !20
  %284 = sub i64 %282, %283
  %285 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %286 = atomicrmw add ptr @g_kernel_measured_ns, i64 %284 monotonic, align 8
  store i64 0, ptr %273, align 8, !tbaa !20
  br label %287

287:                                              ; preds = %269, %266, %207, %204, %141, %93, %272, %276, %233, %237, %155, %151, %113, %117, %94, %98, %76, %57, %61, %38, %33, %28, %25, %2, %72
  %288 = phi i32 [ 1, %72 ], [ 0, %2 ], [ 0, %25 ], [ 0, %28 ], [ 0, %33 ], [ 0, %38 ], [ 0, %61 ], [ 0, %57 ], [ 0, %76 ], [ 0, %98 ], [ 0, %94 ], [ 0, %117 ], [ 0, %113 ], [ 0, %155 ], [ 0, %151 ], [ 0, %237 ], [ 0, %233 ], [ 0, %276 ], [ 0, %272 ], [ 1, %93 ], [ 1, %141 ], [ 1, %204 ], [ 1, %207 ], [ 1, %266 ], [ 1, %269 ]
  ret i32 %288
}

; Function Attrs: nounwind uwtable
define dso_local i32 @autograph_frontier_step(ptr noundef %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4, ptr noundef %5, i32 noundef %6, i32 noundef %7, ptr noundef %8) local_unnamed_addr #0 {
  %10 = alloca %struct.timespec, align 8
  %11 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %11) #28
  %12 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %11) #28
  %13 = load i64, ptr %11, align 8, !tbaa !16
  %14 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %15 = load i64, ptr %14, align 8, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %11) #28
  %16 = call fastcc i32 @autograph_edgemap_cas_first(ptr noundef %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4, ptr noundef %5, ptr noundef %8, i32 noundef %6, i32 noundef %7)
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %10) #28
  %17 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %10) #28
  %18 = load i64, ptr %10, align 8, !tbaa !16
  %19 = getelementptr inbounds nuw i8, ptr %10, i64 8
  %20 = load i64, ptr %19, align 8, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %10) #28
  %21 = sub i64 %18, %13
  %22 = mul i64 %21, 1000000000
  %23 = sub i64 %20, %15
  %24 = add i64 %23, %22
  %25 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %26 = atomicrmw add ptr @g_kernel_measured_ns, i64 %24 monotonic, align 8
  ret i32 %16
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @autograph_edgemap_cas_first(ptr noundef readnone %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4, ptr noundef %5, ptr noundef %6, i32 noundef %7, i32 noundef %8) unnamed_addr #0 {
  %10 = alloca %struct.AutoFrontierStepEnv, align 8
  %11 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %12 = icmp sgt i32 %11, 0
  br i1 %12, label %13, label %23

13:                                               ; preds = %9
  %14 = zext nneg i32 %11 to i64
  br label %18

15:                                               ; preds = %18
  %16 = add nuw nsw i64 %19, 1
  %17 = icmp eq i64 %16, %14
  br i1 %17, label %23, label %18, !llvm.loop !49

18:                                               ; preds = %15, %13
  %19 = phi i64 [ 0, %13 ], [ %16, %15 ]
  %20 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %19
  %21 = load ptr, ptr %20, align 16, !tbaa !50
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
  br i1 %33, label %156, label %34

34:                                               ; preds = %23
  %35 = getelementptr inbounds nuw i8, ptr %24, i64 120
  %36 = load i64, ptr %35, align 8, !tbaa !56
  %37 = icmp slt i64 %36, 1
  br i1 %37, label %156, label %38

38:                                               ; preds = %34
  %39 = icmp samesign ugt i64 %36, 2147483647
  %40 = icmp slt i32 %4, 0
  %41 = or i1 %40, %39
  %42 = zext nneg i32 %4 to i64
  %43 = icmp samesign ult i64 %36, %42
  %44 = select i1 %41, i1 true, i1 %43
  br i1 %44, label %156, label %45

45:                                               ; preds = %38
  %46 = tail call i32 @sgpl_configured_worker_count() #28
  %47 = tail call i32 @llvm.smax.i32(i32 %46, i32 1)
  %48 = tail call fastcc i32 @autograph_scratch_ensure(ptr noundef nonnull %24, i32 noundef %47)
  %49 = icmp eq i32 %48, 0
  br i1 %49, label %156, label %50

50:                                               ; preds = %45
  %51 = getelementptr inbounds nuw i8, ptr %24, i64 200
  %52 = load ptr, ptr %51, align 8, !tbaa !143
  %53 = zext nneg i32 %47 to i64
  %54 = and i64 %53, 3
  %55 = icmp slt i32 %46, 4
  br i1 %55, label %71, label %56

56:                                               ; preds = %50
  %57 = and i64 %53, 2147483644
  br label %58

58:                                               ; preds = %58, %56
  %59 = phi i64 [ 0, %56 ], [ %68, %58 ]
  %60 = phi i64 [ 0, %56 ], [ %69, %58 ]
  %61 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %52, i64 %59, i32 1
  store i32 0, ptr %61, align 8, !tbaa !144
  %62 = or disjoint i64 %59, 1
  %63 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %52, i64 %62, i32 1
  store i32 0, ptr %63, align 8, !tbaa !144
  %64 = or disjoint i64 %59, 2
  %65 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %52, i64 %64, i32 1
  store i32 0, ptr %65, align 8, !tbaa !144
  %66 = or disjoint i64 %59, 3
  %67 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %52, i64 %66, i32 1
  store i32 0, ptr %67, align 8, !tbaa !144
  %68 = add nuw nsw i64 %59, 4
  %69 = add i64 %60, 4
  %70 = icmp eq i64 %69, %57
  br i1 %70, label %71, label %58, !llvm.loop !146

71:                                               ; preds = %58, %50
  %72 = phi i64 [ 0, %50 ], [ %68, %58 ]
  %73 = icmp eq i64 %54, 0
  br i1 %73, label %81, label %74

74:                                               ; preds = %71, %74
  %75 = phi i64 [ %78, %74 ], [ %72, %71 ]
  %76 = phi i64 [ %79, %74 ], [ 0, %71 ]
  %77 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %52, i64 %75, i32 1
  store i32 0, ptr %77, align 8, !tbaa !144
  %78 = add nuw nsw i64 %75, 1
  %79 = add i64 %76, 1
  %80 = icmp eq i64 %79, %54
  br i1 %80, label %81, label %74, !llvm.loop !147

81:                                               ; preds = %74, %71
  call void @llvm.lifetime.start.p0(i64 64, ptr nonnull %10) #28
  store ptr %24, ptr %10, align 8, !tbaa !148
  %82 = getelementptr inbounds nuw i8, ptr %10, i64 8
  store ptr %1, ptr %82, align 8, !tbaa !150
  %83 = getelementptr inbounds nuw i8, ptr %10, i64 16
  store ptr %52, ptr %83, align 8, !tbaa !151
  %84 = getelementptr inbounds nuw i8, ptr %10, i64 24
  store i32 %47, ptr %84, align 8, !tbaa !152
  %85 = getelementptr inbounds nuw i8, ptr %10, i64 28
  store i32 0, ptr %85, align 4
  %86 = getelementptr inbounds nuw i8, ptr %10, i64 32
  store ptr %5, ptr %86, align 8, !tbaa !153
  %87 = getelementptr inbounds nuw i8, ptr %10, i64 40
  store i32 %7, ptr %87, align 8, !tbaa !154
  %88 = getelementptr inbounds nuw i8, ptr %10, i64 44
  store i32 %8, ptr %88, align 4, !tbaa !155
  %89 = getelementptr inbounds nuw i8, ptr %10, i64 48
  store ptr %6, ptr %89, align 8, !tbaa !156
  %90 = getelementptr inbounds nuw i8, ptr %10, i64 56
  store ptr null, ptr %90, align 8, !tbaa !157
  %91 = tail call fastcc i32 @autograph_should_use_pull(ptr noundef %24, ptr noundef %1, i32 noundef %2)
  %92 = icmp eq i32 %91, 0
  br i1 %92, label %152, label %93

93:                                               ; preds = %81
  %94 = getelementptr inbounds nuw i8, ptr %24, i64 224
  %95 = load ptr, ptr %94, align 8, !tbaa !158
  %96 = icmp eq ptr %95, null
  br i1 %96, label %101, label %97

97:                                               ; preds = %93
  %98 = load i64, ptr %35, align 8, !tbaa !56
  %99 = icmp sgt i64 %98, 0
  br i1 %99, label %100, label %101

100:                                              ; preds = %97
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %95, i8 0, i64 %98, i1 false)
  br label %101

101:                                              ; preds = %100, %97, %93
  %102 = zext nneg i32 %2 to i64
  %103 = and i64 %102, 1
  %104 = icmp eq i32 %2, 1
  br i1 %104, label %136, label %105

105:                                              ; preds = %101
  %106 = and i64 %102, 2147483646
  br label %107

107:                                              ; preds = %132, %105
  %108 = phi i64 [ 0, %105 ], [ %133, %132 ]
  %109 = phi i64 [ 0, %105 ], [ %134, %132 ]
  %110 = getelementptr inbounds nuw i32, ptr %1, i64 %108
  %111 = load i32, ptr %110, align 4, !tbaa !15
  %112 = icmp sgt i32 %111, -1
  br i1 %112, label %113, label %120

113:                                              ; preds = %107
  %114 = zext nneg i32 %111 to i64
  %115 = load i64, ptr %35, align 8, !tbaa !56
  %116 = icmp sgt i64 %115, %114
  br i1 %116, label %117, label %120

117:                                              ; preds = %113
  %118 = load ptr, ptr %94, align 8, !tbaa !158
  %119 = getelementptr inbounds nuw i8, ptr %118, i64 %114
  store i8 1, ptr %119, align 1, !tbaa !76
  br label %120

120:                                              ; preds = %117, %113, %107
  %121 = or disjoint i64 %108, 1
  %122 = getelementptr inbounds nuw i32, ptr %1, i64 %121
  %123 = load i32, ptr %122, align 4, !tbaa !15
  %124 = icmp sgt i32 %123, -1
  br i1 %124, label %125, label %132

125:                                              ; preds = %120
  %126 = zext nneg i32 %123 to i64
  %127 = load i64, ptr %35, align 8, !tbaa !56
  %128 = icmp sgt i64 %127, %126
  br i1 %128, label %129, label %132

129:                                              ; preds = %125
  %130 = load ptr, ptr %94, align 8, !tbaa !158
  %131 = getelementptr inbounds nuw i8, ptr %130, i64 %126
  store i8 1, ptr %131, align 1, !tbaa !76
  br label %132

132:                                              ; preds = %129, %125, %120
  %133 = add nuw nsw i64 %108, 2
  %134 = add i64 %109, 2
  %135 = icmp eq i64 %134, %106
  br i1 %135, label %136, label %107, !llvm.loop !159

136:                                              ; preds = %132, %101
  %137 = phi i64 [ 0, %101 ], [ %133, %132 ]
  %138 = icmp eq i64 %103, 0
  br i1 %138, label %150, label %139

139:                                              ; preds = %136
  %140 = getelementptr inbounds nuw i32, ptr %1, i64 %137
  %141 = load i32, ptr %140, align 4, !tbaa !15
  %142 = icmp sgt i32 %141, -1
  br i1 %142, label %143, label %150

143:                                              ; preds = %139
  %144 = zext nneg i32 %141 to i64
  %145 = load i64, ptr %35, align 8, !tbaa !56
  %146 = icmp sgt i64 %145, %144
  br i1 %146, label %147, label %150

147:                                              ; preds = %143
  %148 = load ptr, ptr %94, align 8, !tbaa !158
  %149 = getelementptr inbounds nuw i8, ptr %148, i64 %144
  store i8 1, ptr %149, align 1, !tbaa !76
  br label %150

150:                                              ; preds = %139, %143, %147, %136
  %151 = load ptr, ptr %94, align 8, !tbaa !158
  store ptr %151, ptr %90, align 8, !tbaa !157
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %53, i64 noundef 1, ptr noundef nonnull @autograph_frontier_pull_partition_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  br label %154

152:                                              ; preds = %81
  %153 = zext nneg i32 %2 to i64
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %153, i64 noundef 1, ptr noundef nonnull @autograph_frontier_push_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  br label %154

154:                                              ; preds = %152, %150
  %155 = call fastcc i32 @autograph_scratch_merge_lanes(ptr noundef %24, i32 noundef %47, ptr noundef %3, i32 noundef %4)
  call void @llvm.lifetime.end.p0(i64 64, ptr nonnull %10) #28
  br label %156

156:                                              ; preds = %154, %45, %23, %34, %38
  %157 = phi i32 [ %4, %38 ], [ %4, %34 ], [ %4, %23 ], [ %155, %154 ], [ %4, %45 ]
  ret i32 %157
}

; Function Attrs: nounwind uwtable
define dso_local i32 @autograph_edgemap(ptr noundef %0, i32 noundef %1, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8, i32 noundef %9) local_unnamed_addr #0 {
  %11 = alloca %struct.timespec, align 8
  %12 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %12) #28
  %13 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %12) #28
  %14 = load i64, ptr %12, align 8, !tbaa !16
  %15 = getelementptr inbounds nuw i8, ptr %12, i64 8
  %16 = load i64, ptr %15, align 8, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %12) #28
  switch i32 %1, label %25 [
    i32 0, label %17
    i32 1, label %19
    i32 3, label %21
    i32 2, label %23
  ]

17:                                               ; preds = %10
  %18 = call fastcc i32 @autograph_edgemap_cas_first(ptr noundef %0, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8, i32 noundef %9)
  br label %25

19:                                               ; preds = %10
  %20 = call fastcc i32 @autograph_edgemap_motif(ptr noundef %0, i32 noundef 1, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef 0)
  br label %25

21:                                               ; preds = %10
  %22 = call fastcc i32 @autograph_edgemap_motif(ptr noundef %0, i32 noundef 3, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef 0)
  br label %25

23:                                               ; preds = %10
  %24 = call fastcc i32 @autograph_edgemap_motif(ptr noundef %0, i32 noundef 2, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8)
  br label %25

25:                                               ; preds = %10, %23, %21, %19, %17
  %26 = phi i32 [ %5, %10 ], [ %24, %23 ], [ %22, %21 ], [ %20, %19 ], [ %18, %17 ]
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %11) #28
  %27 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %11) #28
  %28 = load i64, ptr %11, align 8, !tbaa !16
  %29 = getelementptr inbounds nuw i8, ptr %11, i64 8
  %30 = load i64, ptr %29, align 8, !tbaa !19
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %11) #28
  %31 = sub i64 %28, %14
  %32 = mul i64 %31, 1000000000
  %33 = sub i64 %30, %16
  %34 = add i64 %33, %32
  %35 = call i32 @pthread_once(ptr noundef nonnull @g_profile_atexit_once, ptr noundef nonnull @autograph_profile_install_atexit) #28
  %36 = atomicrmw add ptr @g_kernel_measured_ns, i64 %34 monotonic, align 8
  ret i32 %26
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @autograph_edgemap_motif(ptr noundef readnone %0, i32 noundef range(i32 1, 4) %1, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8) unnamed_addr #0 {
  %10 = alloca %struct.AutoMotifFrontierEnv, align 8
  %11 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %12 = icmp sgt i32 %11, 0
  br i1 %12, label %13, label %23

13:                                               ; preds = %9
  %14 = zext nneg i32 %11 to i64
  br label %18

15:                                               ; preds = %18
  %16 = add nuw nsw i64 %19, 1
  %17 = icmp eq i64 %16, %14
  br i1 %17, label %23, label %18, !llvm.loop !49

18:                                               ; preds = %15, %13
  %19 = phi i64 [ 0, %13 ], [ %16, %15 ]
  %20 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %19
  %21 = load ptr, ptr %20, align 16, !tbaa !50
  %22 = icmp eq ptr %21, %0
  br i1 %22, label %23, label %15

23:                                               ; preds = %15, %18, %9
  %24 = phi ptr [ null, %9 ], [ null, %15 ], [ %20, %18 ]
  %25 = insertelement <4 x ptr> poison, ptr %2, i64 0
  %26 = insertelement <4 x ptr> %25, ptr %24, i64 1
  %27 = insertelement <4 x ptr> %26, ptr %4, i64 2
  %28 = insertelement <4 x ptr> %27, ptr %6, i64 3
  %29 = icmp eq <4 x ptr> %28, zeroinitializer
  %30 = icmp slt i32 %3, 1
  %31 = bitcast <4 x i1> %29 to i4
  %32 = icmp ne i4 %31, 0
  %33 = or i1 %32, %30
  br i1 %33, label %213, label %34

34:                                               ; preds = %23
  %35 = getelementptr inbounds nuw i8, ptr %24, i64 120
  %36 = load i64, ptr %35, align 8, !tbaa !56
  %37 = icmp slt i64 %36, 1
  br i1 %37, label %213, label %38

38:                                               ; preds = %34
  %39 = icmp samesign ugt i64 %36, 2147483647
  %40 = icmp slt i32 %5, 0
  %41 = or i1 %40, %39
  %42 = zext nneg i32 %5 to i64
  %43 = icmp samesign ult i64 %36, %42
  %44 = select i1 %41, i1 true, i1 %43
  br i1 %44, label %213, label %45

45:                                               ; preds = %38
  %46 = icmp eq i32 %1, 2
  br i1 %46, label %47, label %51

47:                                               ; preds = %45
  %48 = icmp eq ptr %7, null
  %49 = icmp slt i32 %8, 0
  %50 = or i1 %48, %49
  br i1 %50, label %213, label %57

51:                                               ; preds = %45
  %52 = icmp eq i32 %1, 3
  br i1 %52, label %53, label %57

53:                                               ; preds = %51
  %54 = getelementptr inbounds nuw i8, ptr %24, i64 8
  %55 = load i32, ptr %54, align 8, !tbaa !53
  %56 = icmp eq i32 %55, 0
  br i1 %56, label %57, label %213

57:                                               ; preds = %47, %53, %51
  %58 = phi i1 [ true, %53 ], [ false, %51 ], [ false, %47 ]
  %59 = tail call i32 @sgpl_configured_worker_count() #28
  %60 = tail call i32 @llvm.smax.i32(i32 %59, i32 1)
  %61 = tail call fastcc i32 @autograph_scratch_ensure(ptr noundef nonnull %24, i32 noundef %60)
  %62 = icmp eq i32 %61, 0
  br i1 %62, label %213, label %63

63:                                               ; preds = %57
  %64 = getelementptr inbounds nuw i8, ptr %24, i64 200
  %65 = load ptr, ptr %64, align 8, !tbaa !143
  %66 = zext nneg i32 %60 to i64
  %67 = and i64 %66, 3
  %68 = icmp slt i32 %59, 4
  br i1 %68, label %84, label %69

69:                                               ; preds = %63
  %70 = and i64 %66, 2147483644
  br label %71

71:                                               ; preds = %71, %69
  %72 = phi i64 [ 0, %69 ], [ %81, %71 ]
  %73 = phi i64 [ 0, %69 ], [ %82, %71 ]
  %74 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %65, i64 %72, i32 1
  store i32 0, ptr %74, align 8, !tbaa !144
  %75 = or disjoint i64 %72, 1
  %76 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %65, i64 %75, i32 1
  store i32 0, ptr %76, align 8, !tbaa !144
  %77 = or disjoint i64 %72, 2
  %78 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %65, i64 %77, i32 1
  store i32 0, ptr %78, align 8, !tbaa !144
  %79 = or disjoint i64 %72, 3
  %80 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %65, i64 %79, i32 1
  store i32 0, ptr %80, align 8, !tbaa !144
  %81 = add nuw nsw i64 %72, 4
  %82 = add i64 %73, 4
  %83 = icmp eq i64 %82, %70
  br i1 %83, label %84, label %71, !llvm.loop !146

84:                                               ; preds = %71, %63
  %85 = phi i64 [ 0, %63 ], [ %81, %71 ]
  %86 = icmp eq i64 %67, 0
  br i1 %86, label %94, label %87

87:                                               ; preds = %84, %87
  %88 = phi i64 [ %91, %87 ], [ %85, %84 ]
  %89 = phi i64 [ %92, %87 ], [ 0, %84 ]
  %90 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %65, i64 %88, i32 1
  store i32 0, ptr %90, align 8, !tbaa !144
  %91 = add nuw nsw i64 %88, 1
  %92 = add i64 %89, 1
  %93 = icmp eq i64 %92, %67
  br i1 %93, label %94, label %87, !llvm.loop !160

94:                                               ; preds = %87, %84
  %95 = getelementptr inbounds nuw i8, ptr %24, i64 232
  %96 = load ptr, ptr %95, align 8, !tbaa !161
  %97 = icmp eq ptr %96, null
  br i1 %97, label %104, label %98

98:                                               ; preds = %94
  %99 = load i64, ptr %35, align 8, !tbaa !56
  %100 = icmp sgt i64 %99, 0
  br i1 %100, label %101, label %104

101:                                              ; preds = %98
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %96, i8 0, i64 %99, i1 false)
  %102 = load ptr, ptr %64, align 8, !tbaa !143
  %103 = load ptr, ptr %95, align 8, !tbaa !161
  br label %104

104:                                              ; preds = %94, %98, %101
  %105 = phi ptr [ null, %94 ], [ %96, %98 ], [ %103, %101 ]
  %106 = phi ptr [ %65, %94 ], [ %65, %98 ], [ %102, %101 ]
  %107 = icmp eq i32 %1, 1
  %108 = or i1 %107, %58
  call void @llvm.lifetime.start.p0(i64 80, ptr nonnull %10) #28
  store ptr %24, ptr %10, align 8, !tbaa !162
  %109 = getelementptr inbounds nuw i8, ptr %10, i64 8
  store ptr %2, ptr %109, align 8, !tbaa !164
  %110 = getelementptr inbounds nuw i8, ptr %10, i64 16
  store ptr %106, ptr %110, align 8, !tbaa !165
  %111 = getelementptr inbounds nuw i8, ptr %10, i64 24
  store i32 %60, ptr %111, align 8, !tbaa !166
  %112 = getelementptr inbounds nuw i8, ptr %10, i64 28
  store i32 0, ptr %112, align 4
  %113 = getelementptr inbounds nuw i8, ptr %10, i64 32
  %114 = select i1 %108, ptr %6, ptr null
  store ptr %114, ptr %113, align 8, !tbaa !167
  %115 = getelementptr inbounds nuw i8, ptr %10, i64 40
  %116 = select i1 %46, ptr %6, ptr null
  store ptr %116, ptr %115, align 8, !tbaa !168
  %117 = getelementptr inbounds nuw i8, ptr %10, i64 48
  %118 = select i1 %46, ptr %7, ptr null
  store ptr %118, ptr %117, align 8, !tbaa !169
  %119 = getelementptr inbounds nuw i8, ptr %10, i64 56
  store i32 %8, ptr %119, align 8, !tbaa !170
  %120 = getelementptr inbounds nuw i8, ptr %10, i64 60
  %121 = zext i1 %58 to i32
  store i32 %121, ptr %120, align 4, !tbaa !171
  %122 = getelementptr inbounds nuw i8, ptr %10, i64 64
  store ptr null, ptr %122, align 8, !tbaa !172
  %123 = getelementptr inbounds nuw i8, ptr %10, i64 72
  store ptr %105, ptr %123, align 8, !tbaa !173
  %124 = tail call fastcc i32 @autograph_should_use_pull(ptr noundef nonnull %24, ptr noundef %2, i32 noundef %3)
  br i1 %58, label %125, label %130

125:                                              ; preds = %104
  %126 = getelementptr inbounds nuw i8, ptr %24, i64 112
  %127 = load ptr, ptr %126, align 8, !tbaa !174
  %128 = icmp eq ptr %127, null
  %129 = select i1 %128, i32 0, i32 %124
  br label %130

130:                                              ; preds = %125, %104
  %131 = phi i32 [ %124, %104 ], [ %129, %125 ]
  switch i32 %1, label %195 [
    i32 3, label %132
    i32 1, label %132
  ]

132:                                              ; preds = %130, %130
  %133 = icmp eq i32 %131, 0
  br i1 %133, label %193, label %134

134:                                              ; preds = %132
  %135 = getelementptr inbounds nuw i8, ptr %24, i64 224
  %136 = load ptr, ptr %135, align 8, !tbaa !158
  %137 = icmp eq ptr %136, null
  br i1 %137, label %142, label %138

138:                                              ; preds = %134
  %139 = load i64, ptr %35, align 8, !tbaa !56
  %140 = icmp sgt i64 %139, 0
  br i1 %140, label %141, label %142

141:                                              ; preds = %138
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %136, i8 0, i64 %139, i1 false)
  br label %142

142:                                              ; preds = %141, %138, %134
  %143 = zext nneg i32 %3 to i64
  %144 = and i64 %143, 1
  %145 = icmp eq i32 %3, 1
  br i1 %145, label %177, label %146

146:                                              ; preds = %142
  %147 = and i64 %143, 2147483646
  br label %148

148:                                              ; preds = %173, %146
  %149 = phi i64 [ 0, %146 ], [ %174, %173 ]
  %150 = phi i64 [ 0, %146 ], [ %175, %173 ]
  %151 = getelementptr inbounds nuw i32, ptr %2, i64 %149
  %152 = load i32, ptr %151, align 4, !tbaa !15
  %153 = icmp sgt i32 %152, -1
  br i1 %153, label %154, label %161

154:                                              ; preds = %148
  %155 = zext nneg i32 %152 to i64
  %156 = load i64, ptr %35, align 8, !tbaa !56
  %157 = icmp sgt i64 %156, %155
  br i1 %157, label %158, label %161

158:                                              ; preds = %154
  %159 = load ptr, ptr %135, align 8, !tbaa !158
  %160 = getelementptr inbounds nuw i8, ptr %159, i64 %155
  store i8 1, ptr %160, align 1, !tbaa !76
  br label %161

161:                                              ; preds = %158, %154, %148
  %162 = or disjoint i64 %149, 1
  %163 = getelementptr inbounds nuw i32, ptr %2, i64 %162
  %164 = load i32, ptr %163, align 4, !tbaa !15
  %165 = icmp sgt i32 %164, -1
  br i1 %165, label %166, label %173

166:                                              ; preds = %161
  %167 = zext nneg i32 %164 to i64
  %168 = load i64, ptr %35, align 8, !tbaa !56
  %169 = icmp sgt i64 %168, %167
  br i1 %169, label %170, label %173

170:                                              ; preds = %166
  %171 = load ptr, ptr %135, align 8, !tbaa !158
  %172 = getelementptr inbounds nuw i8, ptr %171, i64 %167
  store i8 1, ptr %172, align 1, !tbaa !76
  br label %173

173:                                              ; preds = %170, %166, %161
  %174 = add nuw nsw i64 %149, 2
  %175 = add i64 %150, 2
  %176 = icmp eq i64 %175, %147
  br i1 %176, label %177, label %148, !llvm.loop !159

177:                                              ; preds = %173, %142
  %178 = phi i64 [ 0, %142 ], [ %174, %173 ]
  %179 = icmp eq i64 %144, 0
  br i1 %179, label %191, label %180

180:                                              ; preds = %177
  %181 = getelementptr inbounds nuw i32, ptr %2, i64 %178
  %182 = load i32, ptr %181, align 4, !tbaa !15
  %183 = icmp sgt i32 %182, -1
  br i1 %183, label %184, label %191

184:                                              ; preds = %180
  %185 = zext nneg i32 %182 to i64
  %186 = load i64, ptr %35, align 8, !tbaa !56
  %187 = icmp sgt i64 %186, %185
  br i1 %187, label %188, label %191

188:                                              ; preds = %184
  %189 = load ptr, ptr %135, align 8, !tbaa !158
  %190 = getelementptr inbounds nuw i8, ptr %189, i64 %185
  store i8 1, ptr %190, align 1, !tbaa !76
  br label %191

191:                                              ; preds = %180, %184, %188, %177
  %192 = load ptr, ptr %135, align 8, !tbaa !158
  store ptr %192, ptr %122, align 8, !tbaa !172
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %66, i64 noundef 1, ptr noundef nonnull @autograph_motif_write_min_pull_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  br label %211

193:                                              ; preds = %132
  %194 = zext nneg i32 %3 to i64
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %194, i64 noundef 1, ptr noundef nonnull @autograph_motif_write_min_push_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  br label %211

195:                                              ; preds = %130
  %196 = icmp eq i32 %131, 0
  br i1 %196, label %209, label %197

197:                                              ; preds = %195
  %198 = getelementptr inbounds nuw i8, ptr %24, i64 224
  %199 = load ptr, ptr %198, align 8, !tbaa !158
  %200 = icmp eq ptr %199, null
  br i1 %200, label %206, label %201

201:                                              ; preds = %197
  %202 = load i64, ptr %35, align 8, !tbaa !56
  %203 = icmp sgt i64 %202, 0
  br i1 %203, label %204, label %206

204:                                              ; preds = %201
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %199, i8 0, i64 %202, i1 false)
  %205 = load ptr, ptr %198, align 8, !tbaa !158
  br label %206

206:                                              ; preds = %197, %201, %204
  %207 = phi ptr [ null, %197 ], [ %199, %201 ], [ %205, %204 ]
  store ptr %207, ptr %122, align 8, !tbaa !172
  %208 = zext nneg i32 %3 to i64
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %208, i64 noundef 1, ptr noundef nonnull @autograph_motif_peel_mark_removed_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %66, i64 noundef 1, ptr noundef nonnull @autograph_motif_peel_pull_partition_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  br label %211

209:                                              ; preds = %195
  %210 = zext nneg i32 %3 to i64
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %210, i64 noundef 1, ptr noundef nonnull @autograph_motif_peel_push_body, ptr noundef nonnull %10, i32 noundef 0, i32 noundef 0) #28
  br label %211

211:                                              ; preds = %206, %209, %191, %193
  %212 = call fastcc i32 @autograph_scratch_merge_lanes(ptr noundef nonnull %24, i32 noundef %60, ptr noundef %4, i32 noundef %5)
  call void @llvm.lifetime.end.p0(i64 80, ptr nonnull %10) #28
  br label %213

213:                                              ; preds = %211, %57, %53, %47, %23, %34, %38
  %214 = phi i32 [ %5, %38 ], [ %5, %34 ], [ %5, %23 ], [ %5, %47 ], [ %5, %53 ], [ %212, %211 ], [ %5, %57 ]
  ret i32 %214
}

; Function Attrs: nounwind uwtable
define dso_local i32 @autograph_motif_frontier_step(ptr noundef %0, i32 noundef %1, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8) local_unnamed_addr #0 {
  %10 = icmp eq i32 %1, 3
  %11 = select i1 %10, i32 3, i32 1
  %12 = icmp eq i32 %1, 2
  %13 = select i1 %12, i32 2, i32 %11
  %14 = tail call i32 @autograph_edgemap(ptr noundef %0, i32 noundef %13, ptr noundef %2, i32 noundef %3, ptr noundef %4, i32 noundef %5, ptr noundef %6, ptr noundef %7, i32 noundef %8, i32 noundef 0)
  ret i32 %14
}

; Function Attrs: nounwind uwtable
define dso_local void @autograph_init(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3, ptr noundef %4, ptr noundef %5) local_unnamed_addr #0 {
  %7 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %9, label %21

9:                                                ; preds = %6
  %10 = zext nneg i32 %7 to i64
  br label %14

11:                                               ; preds = %14
  %12 = add nuw nsw i64 %15, 1
  %13 = icmp eq i64 %12, %10
  br i1 %13, label %19, label %14, !llvm.loop !49

14:                                               ; preds = %11, %9
  %15 = phi i64 [ 0, %9 ], [ %12, %11 ]
  %16 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %15
  %17 = load ptr, ptr %16, align 16, !tbaa !50
  %18 = icmp eq ptr %17, %0
  br i1 %18, label %27, label %11

19:                                               ; preds = %11
  %20 = icmp sgt i32 %7, 63
  br i1 %20, label %87, label %21

21:                                               ; preds = %19, %6
  %22 = add nsw i32 %7, 1
  store i32 %22, ptr @g_meta_count, align 4, !tbaa !15
  %23 = sext i32 %7 to i64
  %24 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %23
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(464) %24, i8 0, i64 464, i1 false)
  store ptr %0, ptr %24, align 16, !tbaa !50
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  store i32 3, ptr %25, align 8, !tbaa !53
  %26 = getelementptr inbounds nuw i8, ptr %24, i64 192
  store i32 1, ptr %26, align 16, !tbaa !82
  br label %27

27:                                               ; preds = %14, %21
  %28 = phi i32 [ %22, %21 ], [ %7, %14 ]
  %29 = phi ptr [ %24, %21 ], [ %16, %14 ]
  %30 = icmp sgt i32 %28, 0
  br i1 %30, label %31, label %53

31:                                               ; preds = %27
  %32 = zext nneg i32 %28 to i64
  br label %33

33:                                               ; preds = %37, %31
  %34 = phi i64 [ 0, %31 ], [ %38, %37 ]
  %35 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %34
  %36 = icmp eq ptr %35, %29
  br i1 %36, label %40, label %37

37:                                               ; preds = %33
  %38 = add nuw nsw i64 %34, 1
  %39 = icmp eq i64 %38, %32
  br i1 %39, label %53, label %33, !llvm.loop !121

40:                                               ; preds = %33
  %41 = and i64 %34, 4294967295
  %42 = getelementptr inbounds nuw [64 x ptr], ptr @g_static_edge_hash, i64 0, i64 %41
  %43 = load ptr, ptr %42, align 8, !tbaa !72
  %44 = icmp eq ptr %43, null
  br i1 %44, label %47, label %45

45:                                               ; preds = %40
  %46 = load ptr, ptr %43, align 8, !tbaa !48
  tail call void @free(ptr noundef %46) #28
  tail call void @free(ptr noundef nonnull %43) #28
  store ptr null, ptr %42, align 8, !tbaa !72
  br label %47

47:                                               ; preds = %40, %45
  %48 = getelementptr inbounds nuw [64 x ptr], ptr @g_extra_edge_hash, i64 0, i64 %41
  %49 = load ptr, ptr %48, align 8, !tbaa !72
  %50 = icmp eq ptr %49, null
  br i1 %50, label %53, label %51

51:                                               ; preds = %47
  %52 = load ptr, ptr %49, align 8, !tbaa !48
  tail call void @free(ptr noundef %52) #28
  tail call void @free(ptr noundef nonnull %49) #28
  store ptr null, ptr %48, align 8, !tbaa !72
  br label %53

53:                                               ; preds = %37, %27, %51, %47
  %54 = getelementptr inbounds nuw i8, ptr %29, i64 16
  store ptr %3, ptr %54, align 8, !tbaa !59
  %55 = getelementptr inbounds nuw i8, ptr %29, i64 24
  store ptr %4, ptr %55, align 8, !tbaa !60
  %56 = getelementptr inbounds nuw i8, ptr %29, i64 32
  store ptr %5, ptr %56, align 8, !tbaa !61
  %57 = icmp sgt i64 %2, 1
  %58 = zext i1 %57 to i64
  %59 = lshr i64 %2, %58
  %60 = getelementptr inbounds nuw i8, ptr %29, i64 40
  store i64 %59, ptr %60, align 8, !tbaa !74
  %61 = getelementptr inbounds nuw i8, ptr %29, i64 64
  %62 = getelementptr inbounds nuw i8, ptr %29, i64 48
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %61, i8 0, i64 16, i1 false)
  %63 = load ptr, ptr %62, align 8, !tbaa !80
  tail call void @free(ptr noundef %63) #28
  %64 = getelementptr inbounds nuw i8, ptr %29, i64 56
  %65 = load ptr, ptr %64, align 8, !tbaa !75
  tail call void @free(ptr noundef %65) #28
  %66 = getelementptr inbounds nuw i8, ptr %29, i64 120
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %62, i8 0, i64 16, i1 false)
  store i64 %1, ptr %66, align 8, !tbaa !56
  %67 = getelementptr inbounds nuw i8, ptr %29, i64 128
  store i64 %2, ptr %67, align 8, !tbaa !57
  %68 = load i64, ptr %60, align 8, !tbaa !74
  %69 = getelementptr inbounds nuw i8, ptr %29, i64 88
  store i64 %68, ptr %69, align 8, !tbaa !69
  %70 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %71 = getelementptr inbounds nuw i8, ptr %29, i64 96
  %72 = load <2 x ptr>, ptr %70, align 8, !tbaa !112
  store <2 x ptr> %72, ptr %71, align 8, !tbaa !112
  %73 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %74 = load ptr, ptr %73, align 8, !tbaa !32
  %75 = getelementptr inbounds nuw i8, ptr %29, i64 112
  store ptr %74, ptr %75, align 8, !tbaa !174
  %76 = getelementptr inbounds nuw i8, ptr %29, i64 136
  store i32 0, ptr %76, align 8, !tbaa !62
  %77 = getelementptr inbounds nuw i8, ptr %29, i64 352
  store i8 0, ptr %77, align 8, !tbaa !175
  %78 = getelementptr inbounds nuw i8, ptr %29, i64 456
  store i8 0, ptr %78, align 8, !tbaa !176
  %79 = getelementptr inbounds nuw i8, ptr %29, i64 8
  %80 = load i32, ptr %79, align 8, !tbaa !53
  %81 = icmp eq i32 %80, 0
  br i1 %81, label %86, label %82

82:                                               ; preds = %53
  %83 = getelementptr inbounds nuw i8, ptr %29, i64 192
  %84 = load i32, ptr %83, align 8, !tbaa !82
  %85 = add nsw i32 %84, 1
  store i32 %85, ptr %83, align 8, !tbaa !82
  br label %86

86:                                               ; preds = %53, %82
  store i32 0, ptr %79, align 8, !tbaa !53
  br label %87

87:                                               ; preds = %19, %86
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: read, inaccessiblemem: none) uwtable
define dso_local void @autograph_set_class_tiers(ptr noundef %0, ptr noundef readonly %1) local_unnamed_addr #17 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %17

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %15, label %10, !llvm.loop !49

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 16, !tbaa !50
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %23, label %7

15:                                               ; preds = %7
  %16 = icmp sgt i32 %3, 63
  br i1 %16, label %65, label %17

17:                                               ; preds = %15, %2
  %18 = add nsw i32 %3, 1
  store i32 %18, ptr @g_meta_count, align 4, !tbaa !15
  %19 = sext i32 %3 to i64
  %20 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %19
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(464) %20, i8 0, i64 464, i1 false)
  store ptr %0, ptr %20, align 16, !tbaa !50
  %21 = getelementptr inbounds nuw i8, ptr %20, i64 8
  store i32 3, ptr %21, align 8, !tbaa !53
  %22 = getelementptr inbounds nuw i8, ptr %20, i64 192
  store i32 1, ptr %22, align 16, !tbaa !82
  br label %23

23:                                               ; preds = %10, %17
  %24 = phi ptr [ %20, %17 ], [ %12, %10 ]
  %25 = icmp eq ptr %1, null
  br i1 %25, label %62, label %26

26:                                               ; preds = %23
  %27 = getelementptr inbounds nuw i8, ptr %24, i64 256
  %28 = load double, ptr %1, align 8, !tbaa !177
  store double %28, ptr %27, align 8, !tbaa !177
  %29 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %30 = load double, ptr %29, align 8, !tbaa !177
  %31 = getelementptr inbounds nuw i8, ptr %24, i64 264
  store double %30, ptr %31, align 8, !tbaa !177
  %32 = getelementptr inbounds nuw i8, ptr %1, i64 16
  %33 = load double, ptr %32, align 8, !tbaa !177
  %34 = getelementptr inbounds nuw i8, ptr %24, i64 272
  store double %33, ptr %34, align 8, !tbaa !177
  %35 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %36 = load double, ptr %35, align 8, !tbaa !177
  %37 = getelementptr inbounds nuw i8, ptr %24, i64 280
  store double %36, ptr %37, align 8, !tbaa !177
  %38 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %39 = load double, ptr %38, align 8, !tbaa !177
  %40 = getelementptr inbounds nuw i8, ptr %24, i64 288
  store double %39, ptr %40, align 8, !tbaa !177
  %41 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %42 = load double, ptr %41, align 8, !tbaa !177
  %43 = getelementptr inbounds nuw i8, ptr %24, i64 296
  store double %42, ptr %43, align 8, !tbaa !177
  %44 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %45 = load double, ptr %44, align 8, !tbaa !177
  %46 = getelementptr inbounds nuw i8, ptr %24, i64 304
  store double %45, ptr %46, align 8, !tbaa !177
  %47 = getelementptr inbounds nuw i8, ptr %1, i64 56
  %48 = load double, ptr %47, align 8, !tbaa !177
  %49 = getelementptr inbounds nuw i8, ptr %24, i64 312
  store double %48, ptr %49, align 8, !tbaa !177
  %50 = getelementptr inbounds nuw i8, ptr %1, i64 64
  %51 = load double, ptr %50, align 8, !tbaa !177
  %52 = getelementptr inbounds nuw i8, ptr %24, i64 320
  store double %51, ptr %52, align 8, !tbaa !177
  %53 = getelementptr inbounds nuw i8, ptr %1, i64 72
  %54 = load double, ptr %53, align 8, !tbaa !177
  %55 = getelementptr inbounds nuw i8, ptr %24, i64 328
  store double %54, ptr %55, align 8, !tbaa !177
  %56 = getelementptr inbounds nuw i8, ptr %1, i64 80
  %57 = load double, ptr %56, align 8, !tbaa !177
  %58 = getelementptr inbounds nuw i8, ptr %24, i64 336
  store double %57, ptr %58, align 8, !tbaa !177
  %59 = getelementptr inbounds nuw i8, ptr %1, i64 88
  %60 = load double, ptr %59, align 8, !tbaa !177
  %61 = getelementptr inbounds nuw i8, ptr %24, i64 344
  store double %60, ptr %61, align 8, !tbaa !177
  br label %62

62:                                               ; preds = %23, %26
  %63 = phi i8 [ 1, %26 ], [ 0, %23 ]
  %64 = getelementptr inbounds nuw i8, ptr %24, i64 352
  store i8 %63, ptr %64, align 8, !tbaa !175
  br label %65

65:                                               ; preds = %62, %15
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: read, inaccessiblemem: none) uwtable
define dso_local void @autograph_set_class_tiers_csr(ptr noundef %0, ptr noundef readonly %1) local_unnamed_addr #17 {
  %3 = load i32, ptr @g_meta_count, align 4, !tbaa !15
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %17

5:                                                ; preds = %2
  %6 = zext nneg i32 %3 to i64
  br label %10

7:                                                ; preds = %10
  %8 = add nuw nsw i64 %11, 1
  %9 = icmp eq i64 %8, %6
  br i1 %9, label %15, label %10, !llvm.loop !49

10:                                               ; preds = %7, %5
  %11 = phi i64 [ 0, %5 ], [ %8, %7 ]
  %12 = getelementptr inbounds nuw [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %11
  %13 = load ptr, ptr %12, align 16, !tbaa !50
  %14 = icmp eq ptr %13, %0
  br i1 %14, label %23, label %7

15:                                               ; preds = %7
  %16 = icmp sgt i32 %3, 63
  br i1 %16, label %65, label %17

17:                                               ; preds = %15, %2
  %18 = add nsw i32 %3, 1
  store i32 %18, ptr @g_meta_count, align 4, !tbaa !15
  %19 = sext i32 %3 to i64
  %20 = getelementptr inbounds [64 x %struct.AutoGraphMeta], ptr @g_meta, i64 0, i64 %19
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(464) %20, i8 0, i64 464, i1 false)
  store ptr %0, ptr %20, align 16, !tbaa !50
  %21 = getelementptr inbounds nuw i8, ptr %20, i64 8
  store i32 3, ptr %21, align 8, !tbaa !53
  %22 = getelementptr inbounds nuw i8, ptr %20, i64 192
  store i32 1, ptr %22, align 16, !tbaa !82
  br label %23

23:                                               ; preds = %10, %17
  %24 = phi ptr [ %20, %17 ], [ %12, %10 ]
  %25 = icmp eq ptr %1, null
  br i1 %25, label %62, label %26

26:                                               ; preds = %23
  %27 = getelementptr inbounds nuw i8, ptr %24, i64 360
  %28 = load double, ptr %1, align 8, !tbaa !177
  store double %28, ptr %27, align 8, !tbaa !177
  %29 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %30 = load double, ptr %29, align 8, !tbaa !177
  %31 = getelementptr inbounds nuw i8, ptr %24, i64 368
  store double %30, ptr %31, align 8, !tbaa !177
  %32 = getelementptr inbounds nuw i8, ptr %1, i64 16
  %33 = load double, ptr %32, align 8, !tbaa !177
  %34 = getelementptr inbounds nuw i8, ptr %24, i64 376
  store double %33, ptr %34, align 8, !tbaa !177
  %35 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %36 = load double, ptr %35, align 8, !tbaa !177
  %37 = getelementptr inbounds nuw i8, ptr %24, i64 384
  store double %36, ptr %37, align 8, !tbaa !177
  %38 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %39 = load double, ptr %38, align 8, !tbaa !177
  %40 = getelementptr inbounds nuw i8, ptr %24, i64 392
  store double %39, ptr %40, align 8, !tbaa !177
  %41 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %42 = load double, ptr %41, align 8, !tbaa !177
  %43 = getelementptr inbounds nuw i8, ptr %24, i64 400
  store double %42, ptr %43, align 8, !tbaa !177
  %44 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %45 = load double, ptr %44, align 8, !tbaa !177
  %46 = getelementptr inbounds nuw i8, ptr %24, i64 408
  store double %45, ptr %46, align 8, !tbaa !177
  %47 = getelementptr inbounds nuw i8, ptr %1, i64 56
  %48 = load double, ptr %47, align 8, !tbaa !177
  %49 = getelementptr inbounds nuw i8, ptr %24, i64 416
  store double %48, ptr %49, align 8, !tbaa !177
  %50 = getelementptr inbounds nuw i8, ptr %1, i64 64
  %51 = load double, ptr %50, align 8, !tbaa !177
  %52 = getelementptr inbounds nuw i8, ptr %24, i64 424
  store double %51, ptr %52, align 8, !tbaa !177
  %53 = getelementptr inbounds nuw i8, ptr %1, i64 72
  %54 = load double, ptr %53, align 8, !tbaa !177
  %55 = getelementptr inbounds nuw i8, ptr %24, i64 432
  store double %54, ptr %55, align 8, !tbaa !177
  %56 = getelementptr inbounds nuw i8, ptr %1, i64 80
  %57 = load double, ptr %56, align 8, !tbaa !177
  %58 = getelementptr inbounds nuw i8, ptr %24, i64 440
  store double %57, ptr %58, align 8, !tbaa !177
  %59 = getelementptr inbounds nuw i8, ptr %1, i64 88
  %60 = load double, ptr %59, align 8, !tbaa !177
  %61 = getelementptr inbounds nuw i8, ptr %24, i64 448
  store double %60, ptr %61, align 8, !tbaa !177
  br label %62

62:                                               ; preds = %23, %26
  %63 = phi i8 [ 1, %26 ], [ 0, %23 ]
  %64 = getelementptr inbounds nuw i8, ptr %24, i64 456
  store i8 %63, ptr %64, align 8, !tbaa !176
  br label %65

65:                                               ; preds = %62, %15
  ret void
}

; Function Attrs: nofree nounwind
declare i32 @atexit(ptr noundef) local_unnamed_addr #18

; Function Attrs: nounwind uwtable
define internal void @autograph_profile_report() #0 {
  %1 = alloca %struct.timespec, align 8
  %2 = alloca [3 x double], align 16
  %3 = alloca [3 x i64], align 16
  %4 = tail call align 4 ptr @llvm.threadlocal.address.p0(ptr align 4 @g_active_region_id)
  %5 = load i32, ptr %4, align 4, !tbaa !15
  %6 = icmp ugt i32 %5, 1023
  br i1 %6, label %24, label %7

7:                                                ; preds = %0
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %1) #28
  %8 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %1) #28
  %9 = load i64, ptr %1, align 8, !tbaa !16
  %10 = mul i64 %9, 1000000000
  %11 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %12 = load i64, ptr %11, align 8, !tbaa !19
  %13 = add i64 %10, %12
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %1) #28
  %14 = call align 8 ptr @llvm.threadlocal.address.p0(ptr align 8 @g_active_region_start_ns)
  %15 = load i64, ptr %14, align 8, !tbaa !20
  %16 = icmp ult i64 %13, %15
  br i1 %16, label %23, label %17

17:                                               ; preds = %7
  %18 = load i32, ptr %4, align 4, !tbaa !15
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds [1024 x %struct.AutoProfileRegion], ptr @g_profile_regions, i64 0, i64 %19, i32 4
  %21 = sub nuw i64 %13, %15
  %22 = atomicrmw add ptr %20, i64 %21 monotonic, align 8
  br label %23

23:                                               ; preds = %17, %7
  store i32 -1, ptr %4, align 4, !tbaa !15
  store i64 0, ptr %14, align 8, !tbaa !20
  br label %24

24:                                               ; preds = %0, %23
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %2) #28
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %2, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %3) #28
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %3, i8 0, i64 24, i1 false)
  %25 = load ptr, ptr @stderr, align 8, !tbaa !178
  %26 = call i64 @fwrite(ptr nonnull @.str, i64 54, i64 1, ptr %25) #32
  br label %27

27:                                               ; preds = %24, %81
  %28 = phi i64 [ 0, %24 ], [ %82, %81 ]
  %29 = getelementptr inbounds nuw [1024 x %struct.AutoProfileRegion], ptr @g_profile_regions, i64 0, i64 %28
  %30 = load atomic i32, ptr %29 seq_cst, align 8, !tbaa !180
  %31 = icmp eq i32 %30, 0
  br i1 %31, label %81, label %32

32:                                               ; preds = %27
  %33 = getelementptr inbounds nuw i8, ptr %29, i64 4
  %34 = load i32, ptr %33, align 4, !tbaa !5
  %35 = icmp ult i32 %34, 3
  br i1 %35, label %36, label %49

36:                                               ; preds = %32
  %37 = getelementptr inbounds nuw i8, ptr %29, i64 16
  %38 = load double, ptr %37, align 8, !tbaa !12
  %39 = zext nneg i32 %34 to i64
  %40 = getelementptr inbounds nuw [3 x double], ptr %2, i64 0, i64 %39
  %41 = load double, ptr %40, align 8, !tbaa !177
  %42 = fadd double %38, %41
  store double %42, ptr %40, align 8, !tbaa !177
  %43 = getelementptr inbounds nuw i8, ptr %29, i64 24
  %44 = load atomic i64, ptr %43 seq_cst, align 8, !tbaa !181
  %45 = getelementptr inbounds nuw [3 x i64], ptr %3, i64 0, i64 %39
  %46 = load i64, ptr %45, align 8, !tbaa !20
  %47 = add i64 %46, %44
  store i64 %47, ptr %45, align 8, !tbaa !20
  %48 = load i32, ptr %33, align 4, !tbaa !5
  br label %49

49:                                               ; preds = %36, %32
  %50 = phi i32 [ %48, %36 ], [ %34, %32 ]
  %51 = load ptr, ptr @stderr, align 8, !tbaa !178
  %52 = icmp ult i32 %50, 3
  br i1 %52, label %53, label %57

53:                                               ; preds = %49
  %54 = zext nneg i32 %50 to i64
  %55 = shl i64 %54, 2
  %56 = call ptr @llvm.load.relative.i64(ptr @switch.table.autograph_profile_report.rel, i64 %55)
  br label %57

57:                                               ; preds = %49, %53
  %58 = phi ptr [ %56, %53 ], [ @.str.7, %49 ]
  %59 = getelementptr inbounds nuw i8, ptr %29, i64 8
  %60 = load i32, ptr %59, align 8, !tbaa !11
  %61 = icmp ult i32 %60, 4
  br i1 %61, label %62, label %66

62:                                               ; preds = %57
  %63 = zext nneg i32 %60 to i64
  %64 = shl i64 %63, 2
  %65 = call ptr @llvm.load.relative.i64(ptr @switch.table.autograph_profile_report.19.rel, i64 %64)
  br label %66

66:                                               ; preds = %57, %62
  %67 = phi ptr [ %65, %62 ], [ @.str.12, %57 ]
  %68 = getelementptr inbounds nuw i8, ptr %29, i64 32
  %69 = load atomic i64, ptr %68 seq_cst, align 8, !tbaa !182
  %70 = getelementptr inbounds nuw i8, ptr %29, i64 16
  %71 = load double, ptr %70, align 8, !tbaa !12
  %72 = getelementptr inbounds nuw i8, ptr %29, i64 24
  %73 = load atomic i64, ptr %72 seq_cst, align 8, !tbaa !181
  %74 = load double, ptr %70, align 8, !tbaa !12
  %75 = fdiv double %74, 1.000000e+06
  %76 = load atomic i64, ptr %72 seq_cst, align 8, !tbaa !181
  %77 = uitofp i64 %76 to double
  %78 = fdiv double %77, 1.000000e+06
  %79 = trunc nuw nsw i64 %28 to i32
  %80 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %51, ptr noundef nonnull @.str.1, i32 noundef %79, ptr noundef nonnull %58, ptr noundef nonnull %67, i64 noundef %69, double noundef %71, i64 noundef %73, double noundef %75, double noundef %78) #33
  br label %81

81:                                               ; preds = %27, %66
  %82 = add nuw nsw i64 %28, 1
  %83 = icmp eq i64 %82, 1024
  br i1 %83, label %84, label %27, !llvm.loop !183

84:                                               ; preds = %81
  %85 = load ptr, ptr @stderr, align 8, !tbaa !178
  %86 = load double, ptr %2, align 16, !tbaa !177
  %87 = load i64, ptr %3, align 16, !tbaa !20
  %88 = fdiv double %86, 1.000000e+06
  %89 = uitofp i64 %87 to double
  %90 = fdiv double %89, 1.000000e+06
  %91 = load atomic i64, ptr @g_kernel_measured_ns monotonic, align 16
  %92 = load atomic i64, ptr @g_kernel_measured_ns monotonic, align 16
  %93 = uitofp i64 %92 to double
  %94 = fdiv double %93, 1.000000e+06
  %95 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %85, ptr noundef nonnull @.str.2, ptr noundef nonnull @.str.4, double noundef %86, i64 noundef %87, double noundef %88, double noundef %90, i64 noundef %91, double noundef %94) #33
  %96 = load ptr, ptr @stderr, align 8, !tbaa !178
  %97 = getelementptr inbounds nuw i8, ptr %2, i64 8
  %98 = load double, ptr %97, align 8, !tbaa !177
  %99 = getelementptr inbounds nuw i8, ptr %3, i64 8
  %100 = load i64, ptr %99, align 8, !tbaa !20
  %101 = fdiv double %98, 1.000000e+06
  %102 = uitofp i64 %100 to double
  %103 = fdiv double %102, 1.000000e+06
  %104 = load atomic i64, ptr getelementptr inbounds nuw (i8, ptr @g_kernel_measured_ns, i64 8) monotonic, align 8
  %105 = load atomic i64, ptr getelementptr inbounds nuw (i8, ptr @g_kernel_measured_ns, i64 8) monotonic, align 8
  %106 = uitofp i64 %105 to double
  %107 = fdiv double %106, 1.000000e+06
  %108 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %96, ptr noundef nonnull @.str.2, ptr noundef nonnull @.str.5, double noundef %98, i64 noundef %100, double noundef %101, double noundef %103, i64 noundef %104, double noundef %107) #33
  %109 = load ptr, ptr @stderr, align 8, !tbaa !178
  %110 = getelementptr inbounds nuw i8, ptr %2, i64 16
  %111 = load double, ptr %110, align 16, !tbaa !177
  %112 = getelementptr inbounds nuw i8, ptr %3, i64 16
  %113 = load i64, ptr %112, align 16, !tbaa !20
  %114 = fdiv double %111, 1.000000e+06
  %115 = uitofp i64 %113 to double
  %116 = fdiv double %115, 1.000000e+06
  %117 = load atomic i64, ptr getelementptr inbounds nuw (i8, ptr @g_kernel_measured_ns, i64 16) monotonic, align 16
  %118 = load atomic i64, ptr getelementptr inbounds nuw (i8, ptr @g_kernel_measured_ns, i64 16) monotonic, align 16
  %119 = uitofp i64 %118 to double
  %120 = fdiv double %119, 1.000000e+06
  %121 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %109, ptr noundef nonnull @.str.2, ptr noundef nonnull @.str.6, double noundef %111, i64 noundef %113, double noundef %114, double noundef %116, i64 noundef %117, double noundef %120) #33
  %122 = load ptr, ptr @stderr, align 8, !tbaa !178
  %123 = load i32, ptr @g_conversions_injected, align 4, !tbaa !15
  %124 = load i64, ptr @g_conversion_ns, align 8, !tbaa !20
  %125 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %122, ptr noundef nonnull @.str.3, i32 noundef %123, i64 noundef %124) #33
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #28
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #28
  ret void
}

; Function Attrs: nofree nounwind
declare noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #18

declare i64 @roaring_bitmap_get_cardinality(ptr noundef) local_unnamed_addr #1

declare i32 @roaring_bitmap_get_at_index(ptr noundef, i64 noundef) local_unnamed_addr #1

; Function Attrs: nofree nounwind memory(read)
declare noundef ptr @getenv(ptr nocapture noundef) local_unnamed_addr #19

; Function Attrs: nofree nounwind uwtable
define internal void @shift_prof_dump() #2 {
  %1 = tail call ptr @getenv(ptr noundef nonnull @.str.13) #28
  %2 = icmp eq ptr %1, null
  %3 = load i64, ptr @shift_prof_cnt, align 8
  %4 = icmp slt i64 %3, 1
  %5 = select i1 %2, i1 true, i1 %4
  br i1 %5, label %23, label %6

6:                                                ; preds = %0
  %7 = tail call noalias ptr @fopen(ptr noundef nonnull %1, ptr noundef nonnull @.str.14)
  %8 = icmp eq ptr %7, null
  br i1 %8, label %23, label %9

9:                                                ; preds = %6
  %10 = load i64, ptr @shift_prof_ns, align 8, !tbaa !20
  %11 = load i64, ptr @shift_prof_cnt, align 8, !tbaa !20
  %12 = load i64, ptr @shift_prof_bytes, align 8, !tbaa !20
  %13 = icmp sgt i64 %12, 0
  %14 = sitofp i64 %10 to double
  %15 = uitofp nneg i64 %12 to double
  %16 = fmul double %15, 1.562500e-02
  %17 = fdiv double %14, %16
  %18 = select i1 %13, double %17, double 0.000000e+00
  %19 = load i64, ptr @shift_prof_reloc_cnt, align 8, !tbaa !20
  %20 = load i64, ptr @shift_prof_reloc_bytes, align 8, !tbaa !20
  %21 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef nonnull %7, ptr noundef nonnull @.str.15, i64 noundef %10, i64 noundef %11, i64 noundef %12, double noundef %18, i64 noundef %19, i64 noundef %20) #28
  %22 = tail call i32 @fclose(ptr noundef nonnull %7)
  br label %23

23:                                               ; preds = %9, %6, %0
  ret void
}

; Function Attrs: nofree nounwind
declare noalias noundef ptr @fopen(ptr nocapture noundef readonly, ptr nocapture noundef readonly) local_unnamed_addr #18

; Function Attrs: nofree nounwind
declare noundef i32 @fclose(ptr nocapture noundef) local_unnamed_addr #18

declare i32 @sgpl_configured_worker_count() local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define internal fastcc range(i32 0, 2) i32 @autograph_scratch_ensure(ptr noundef %0, i32 noundef %1) unnamed_addr #0 {
  %3 = icmp eq ptr %0, null
  br i1 %3, label %95, label %4

4:                                                ; preds = %2
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 120
  %6 = load i64, ptr %5, align 8, !tbaa !56
  %7 = icmp slt i64 %6, 1
  %8 = icmp slt i32 %1, 1
  %9 = or i1 %8, %7
  br i1 %9, label %95, label %10

10:                                               ; preds = %4
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 216
  %12 = load i64, ptr %11, align 8, !tbaa !184
  %13 = icmp slt i64 %12, %6
  br i1 %13, label %14, label %28

14:                                               ; preds = %10
  %15 = getelementptr inbounds nuw i8, ptr %0, i64 224
  %16 = load ptr, ptr %15, align 8, !tbaa !158
  %17 = tail call ptr @realloc(ptr noundef %16, i64 noundef %6) #31
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 232
  %19 = load ptr, ptr %18, align 8, !tbaa !161
  %20 = load i64, ptr %5, align 8, !tbaa !56
  %21 = tail call ptr @realloc(ptr noundef %19, i64 noundef %20) #31
  %22 = icmp ne ptr %17, null
  %23 = icmp ne ptr %21, null
  %24 = and i1 %22, %23
  br i1 %24, label %26, label %25

25:                                               ; preds = %14
  tail call void @abort() #34
  unreachable

26:                                               ; preds = %14
  store ptr %17, ptr %15, align 8, !tbaa !158
  store ptr %21, ptr %18, align 8, !tbaa !161
  %27 = load i64, ptr %5, align 8, !tbaa !56
  store i64 %27, ptr %11, align 8, !tbaa !184
  br label %28

28:                                               ; preds = %26, %10
  %29 = getelementptr inbounds nuw i8, ptr %0, i64 208
  %30 = load i32, ptr %29, align 8, !tbaa !185
  %31 = icmp slt i32 %30, %1
  br i1 %31, label %32, label %53

32:                                               ; preds = %28
  %33 = getelementptr inbounds nuw i8, ptr %0, i64 200
  %34 = load ptr, ptr %33, align 8, !tbaa !143
  %35 = zext nneg i32 %1 to i64
  %36 = shl nuw nsw i64 %35, 4
  %37 = tail call ptr @realloc(ptr noundef %34, i64 noundef %36) #31
  %38 = icmp eq ptr %37, null
  br i1 %38, label %39, label %40

39:                                               ; preds = %32
  tail call void @abort() #34
  unreachable

40:                                               ; preds = %32
  %41 = load i32, ptr %29, align 8, !tbaa !185
  %42 = icmp slt i32 %41, %1
  br i1 %42, label %43, label %52

43:                                               ; preds = %40
  %44 = sext i32 %41 to i64
  %45 = shl nsw i64 %44, 4
  %46 = getelementptr i8, ptr %37, i64 %45
  %47 = xor i32 %41, -1
  %48 = add i32 %1, %47
  %49 = zext i32 %48 to i64
  %50 = shl nuw nsw i64 %49, 4
  %51 = add nuw nsw i64 %50, 16
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %46, i8 0, i64 %51, i1 false), !tbaa !76
  br label %52

52:                                               ; preds = %43, %40
  store ptr %37, ptr %33, align 8, !tbaa !143
  store i32 %1, ptr %29, align 8, !tbaa !185
  br label %53

53:                                               ; preds = %52, %28
  %54 = getelementptr inbounds nuw i8, ptr %0, i64 248
  %55 = load i32, ptr %54, align 8, !tbaa !186
  %56 = add nuw nsw i32 %1, 1
  %57 = icmp sgt i32 %55, %1
  br i1 %57, label %67, label %58

58:                                               ; preds = %53
  %59 = getelementptr inbounds nuw i8, ptr %0, i64 240
  %60 = load ptr, ptr %59, align 8, !tbaa !187
  %61 = zext nneg i32 %56 to i64
  %62 = shl nuw nsw i64 %61, 3
  %63 = tail call ptr @realloc(ptr noundef %60, i64 noundef %62) #31
  %64 = icmp eq ptr %63, null
  br i1 %64, label %65, label %66

65:                                               ; preds = %58
  tail call void @abort() #34
  unreachable

66:                                               ; preds = %58
  store ptr %63, ptr %59, align 8, !tbaa !187
  store i32 %56, ptr %54, align 8, !tbaa !186
  br label %67

67:                                               ; preds = %53, %66
  %68 = getelementptr inbounds nuw i8, ptr %0, i64 200
  %69 = load ptr, ptr %68, align 8, !tbaa !143
  %70 = zext nneg i32 %1 to i64
  %71 = add nsw i64 %70, -1
  %72 = load i64, ptr %5, align 8, !tbaa !56
  %73 = add i64 %71, %72
  %74 = sdiv i64 %73, %70
  %75 = tail call i64 @llvm.smin.i64(i64 %74, i64 16384)
  %76 = tail call i64 @llvm.smax.i64(i64 %75, i64 64)
  %77 = trunc nuw nsw i64 %76 to i32
  %78 = shl nuw nsw i64 %76, 2
  br label %79

79:                                               ; preds = %67, %91
  %80 = phi i64 [ 0, %67 ], [ %93, %91 ]
  %81 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %69, i64 %80
  %82 = getelementptr inbounds nuw i8, ptr %81, i64 12
  %83 = load i32, ptr %82, align 4, !tbaa !188
  %84 = icmp slt i32 %83, %77
  br i1 %84, label %85, label %91

85:                                               ; preds = %79
  %86 = load ptr, ptr %81, align 8, !tbaa !189
  %87 = tail call ptr @realloc(ptr noundef %86, i64 noundef %78) #31
  %88 = icmp eq ptr %87, null
  br i1 %88, label %89, label %90

89:                                               ; preds = %85
  tail call void @abort() #34
  unreachable

90:                                               ; preds = %85
  store ptr %87, ptr %81, align 8, !tbaa !189
  store i32 %77, ptr %82, align 4, !tbaa !188
  br label %91

91:                                               ; preds = %90, %79
  %92 = getelementptr inbounds nuw i8, ptr %81, i64 8
  store i32 0, ptr %92, align 8, !tbaa !144
  %93 = add nuw nsw i64 %80, 1
  %94 = icmp eq i64 %93, %70
  br i1 %94, label %95, label %79, !llvm.loop !190

95:                                               ; preds = %91, %2, %4
  %96 = phi i32 [ 0, %4 ], [ 0, %2 ], [ 1, %91 ]
  ret i32 %96
}

; Function Attrs: nofree nounwind memory(read) uwtable
define internal fastcc range(i32 0, 2) i32 @autograph_should_use_pull(ptr nocapture noundef readonly %0, ptr nocapture noundef readonly %1, i32 noundef range(i32 1, -2147483648) %2) unnamed_addr #20 {
  %4 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %5 = load i32, ptr %4, align 8, !tbaa !53
  %6 = icmp eq i32 %5, 3
  br i1 %6, label %99, label %7

7:                                                ; preds = %3
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 120
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 168
  %10 = getelementptr inbounds nuw i8, ptr %0, i64 176
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 184
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 144
  %13 = getelementptr inbounds nuw i8, ptr %0, i64 96
  %14 = zext nneg i32 %2 to i64
  br label %15

15:                                               ; preds = %7, %77
  %16 = phi i64 [ 0, %7 ], [ %79, %77 ]
  %17 = phi i64 [ 0, %7 ], [ %78, %77 ]
  %18 = getelementptr inbounds nuw i32, ptr %1, i64 %16
  %19 = load i32, ptr %18, align 4, !tbaa !15
  %20 = icmp slt i32 %19, 0
  br i1 %20, label %77, label %21

21:                                               ; preds = %15
  %22 = zext nneg i32 %19 to i64
  %23 = load i64, ptr %8, align 8, !tbaa !56
  %24 = icmp sgt i64 %23, %22
  br i1 %24, label %25, label %77

25:                                               ; preds = %21
  switch i32 %5, label %77 [
    i32 0, label %26
    i32 1, label %29
    i32 2, label %32
  ]

26:                                               ; preds = %25
  %27 = load ptr, ptr %13, align 8, !tbaa !54
  %28 = icmp eq ptr %27, null
  br i1 %28, label %77, label %65

29:                                               ; preds = %25
  %30 = load ptr, ptr %12, align 8, !tbaa !63
  %31 = icmp eq ptr %30, null
  br i1 %31, label %77, label %65

32:                                               ; preds = %25
  %33 = load ptr, ptr %9, align 8, !tbaa !67
  %34 = icmp eq ptr %33, null
  br i1 %34, label %77, label %35

35:                                               ; preds = %32
  %36 = load ptr, ptr %10, align 8, !tbaa !68
  %37 = icmp eq ptr %36, null
  br i1 %37, label %77, label %38

38:                                               ; preds = %35
  %39 = load i32, ptr %11, align 8, !tbaa !66
  %40 = icmp sgt i32 %39, 0
  br i1 %40, label %41, label %77

41:                                               ; preds = %38
  %42 = udiv i32 %19, %39
  %43 = urem i32 %19, %39
  %44 = zext nneg i32 %42 to i64
  %45 = getelementptr inbounds nuw i32, ptr %33, i64 %44
  %46 = load i32, ptr %45, align 4, !tbaa !15
  %47 = getelementptr inbounds nuw i8, ptr %45, i64 4
  %48 = load i32, ptr %47, align 4, !tbaa !15
  %49 = icmp slt i32 %46, %48
  br i1 %49, label %50, label %77

50:                                               ; preds = %41
  %51 = sext i32 %46 to i64
  %52 = sext i32 %48 to i64
  br label %53

53:                                               ; preds = %53, %50
  %54 = phi i64 [ %51, %50 ], [ %62, %53 ]
  %55 = phi i64 [ 0, %50 ], [ %61, %53 ]
  %56 = getelementptr inbounds i32, ptr %36, i64 %54
  %57 = load i32, ptr %56, align 4, !tbaa !15
  %58 = icmp eq i32 %57, %43
  %59 = icmp sle i32 %57, %43
  %60 = zext i1 %58 to i64
  %61 = add nuw nsw i64 %55, %60
  %62 = add nsw i64 %54, 2
  %63 = icmp slt i64 %62, %52
  %64 = select i1 %59, i1 %63, i1 false
  br i1 %64, label %53, label %72, !llvm.loop !191

65:                                               ; preds = %29, %26
  %66 = phi ptr [ %27, %26 ], [ %30, %29 ]
  %67 = getelementptr inbounds nuw i64, ptr %66, i64 %22
  %68 = getelementptr inbounds nuw i8, ptr %67, i64 8
  %69 = load i64, ptr %68, align 8, !tbaa !20
  %70 = load i64, ptr %67, align 8, !tbaa !20
  %71 = sub nsw i64 %69, %70
  br label %72

72:                                               ; preds = %53, %65
  %73 = phi i64 [ %71, %65 ], [ %61, %53 ]
  %74 = sub nsw i64 9223372036854775807, %17
  %75 = icmp sgt i64 %73, %74
  %76 = add nsw i64 %73, %17
  br i1 %75, label %83, label %77

77:                                               ; preds = %41, %25, %32, %35, %38, %29, %26, %15, %21, %72
  %78 = phi i64 [ %76, %72 ], [ %17, %21 ], [ %17, %15 ], [ %17, %26 ], [ %17, %29 ], [ %17, %38 ], [ %17, %35 ], [ %17, %32 ], [ %17, %25 ], [ %17, %41 ]
  %79 = add nuw nsw i64 %16, 1
  %80 = icmp eq i64 %79, %14
  br i1 %80, label %81, label %15, !llvm.loop !192

81:                                               ; preds = %77
  %82 = load i64, ptr %8, align 8, !tbaa !56
  br label %83

83:                                               ; preds = %72, %81
  %84 = phi i64 [ %82, %81 ], [ %23, %72 ]
  %85 = phi i64 [ %78, %81 ], [ 9223372036854775807, %72 ]
  %86 = icmp sgt i64 %85, %84
  %87 = tail call ptr @getenv(ptr noundef nonnull @.str.16) #28
  %88 = icmp eq ptr %87, null
  br i1 %88, label %96, label %89

89:                                               ; preds = %83
  %90 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %87, ptr noundef nonnull dereferenceable(5) @.str.17) #35
  %91 = icmp eq i32 %90, 0
  br i1 %91, label %96, label %92

92:                                               ; preds = %89
  %93 = tail call i32 @strcmp(ptr noundef nonnull dereferenceable(1) %87, ptr noundef nonnull dereferenceable(5) @.str.18) #35
  %94 = icmp eq i32 %93, 0
  %95 = select i1 %94, i1 true, i1 %86
  br label %96

96:                                               ; preds = %92, %83, %89
  %97 = phi i1 [ false, %89 ], [ %95, %92 ], [ %86, %83 ]
  %98 = zext i1 %97 to i32
  br label %99

99:                                               ; preds = %3, %96
  %100 = phi i32 [ %98, %96 ], [ 0, %3 ]
  ret i32 %100
}

declare void @parallel_for_runtime(i64 noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define internal void @autograph_frontier_pull_partition_body(i64 noundef %0, ptr nocapture noundef readonly %1) #0 {
  %3 = load ptr, ptr %1, align 8, !tbaa !148
  %4 = getelementptr inbounds nuw i8, ptr %3, i64 120
  %5 = load i64, ptr %4, align 8, !tbaa !56
  %6 = mul nsw i64 %5, %0
  %7 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %8 = load i32, ptr %7, align 8, !tbaa !152
  %9 = sext i32 %8 to i64
  %10 = sdiv i64 %6, %9
  %11 = add nsw i64 %0, 1
  %12 = mul nsw i64 %5, %11
  %13 = sdiv i64 %12, %9
  %14 = icmp slt i64 %10, %13
  br i1 %14, label %15, label %20

15:                                               ; preds = %2
  %16 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %17 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %18 = getelementptr inbounds nuw i8, ptr %1, i64 56
  %19 = getelementptr inbounds nuw i8, ptr %1, i64 44
  br label %21

20:                                               ; preds = %163, %2
  ret void

21:                                               ; preds = %15, %163
  %22 = phi i64 [ %10, %15 ], [ %164, %163 ]
  %23 = load ptr, ptr %16, align 8, !tbaa !153
  %24 = getelementptr inbounds i32, ptr %23, i64 %22
  %25 = load i32, ptr %24, align 4, !tbaa !15
  %26 = load i32, ptr %17, align 8, !tbaa !154
  %27 = icmp eq i32 %25, %26
  br i1 %27, label %28, label %163

28:                                               ; preds = %21
  %29 = trunc i64 %22 to i32
  %30 = load ptr, ptr %1, align 8, !tbaa !148
  %31 = getelementptr inbounds nuw i8, ptr %30, i64 8
  %32 = load i32, ptr %31, align 8, !tbaa !53
  switch i32 %32, label %163 [
    i32 0, label %33
    i32 1, label %68
    i32 2, label %103
  ]

33:                                               ; preds = %28
  %34 = getelementptr inbounds nuw i8, ptr %30, i64 96
  %35 = load ptr, ptr %34, align 8, !tbaa !54
  %36 = icmp eq ptr %35, null
  br i1 %36, label %163, label %37

37:                                               ; preds = %33
  %38 = getelementptr inbounds nuw i8, ptr %30, i64 104
  %39 = load ptr, ptr %38, align 8, !tbaa !55
  %40 = icmp eq ptr %39, null
  br i1 %40, label %163, label %41

41:                                               ; preds = %37
  %42 = shl i64 %22, 32
  %43 = ashr exact i64 %42, 29
  %44 = getelementptr inbounds i8, ptr %35, i64 %43
  %45 = load i64, ptr %44, align 8, !tbaa !20
  %46 = getelementptr i8, ptr %44, i64 8
  %47 = load i64, ptr %46, align 8, !tbaa !20
  %48 = icmp slt i64 %45, %47
  br i1 %48, label %49, label %163

49:                                               ; preds = %41
  %50 = getelementptr inbounds nuw i8, ptr %30, i64 120
  br label %51

51:                                               ; preds = %65, %49
  %52 = phi i64 [ %45, %49 ], [ %66, %65 ]
  %53 = getelementptr inbounds i32, ptr %39, i64 %52
  %54 = load i32, ptr %53, align 4, !tbaa !15
  %55 = icmp sgt i32 %54, -1
  br i1 %55, label %56, label %65

56:                                               ; preds = %51
  %57 = zext nneg i32 %54 to i64
  %58 = load i64, ptr %50, align 8, !tbaa !56
  %59 = icmp sgt i64 %58, %57
  br i1 %59, label %60, label %65

60:                                               ; preds = %56
  %61 = load ptr, ptr %18, align 8, !tbaa !157
  %62 = getelementptr inbounds nuw i8, ptr %61, i64 %57
  %63 = load i8, ptr %62, align 1, !tbaa !76
  %64 = icmp eq i8 %63, 0
  br i1 %64, label %65, label %157

65:                                               ; preds = %60, %56, %51
  %66 = add i64 %52, 1
  %67 = icmp eq i64 %66, %47
  br i1 %67, label %163, label %51, !llvm.loop !193

68:                                               ; preds = %28
  %69 = getelementptr inbounds nuw i8, ptr %30, i64 144
  %70 = load ptr, ptr %69, align 8, !tbaa !63
  %71 = icmp eq ptr %70, null
  br i1 %71, label %163, label %72

72:                                               ; preds = %68
  %73 = getelementptr inbounds nuw i8, ptr %30, i64 152
  %74 = load ptr, ptr %73, align 8, !tbaa !64
  %75 = icmp eq ptr %74, null
  br i1 %75, label %163, label %76

76:                                               ; preds = %72
  %77 = shl i64 %22, 32
  %78 = ashr exact i64 %77, 29
  %79 = getelementptr inbounds i8, ptr %70, i64 %78
  %80 = load i64, ptr %79, align 8, !tbaa !20
  %81 = getelementptr i8, ptr %79, i64 8
  %82 = load i64, ptr %81, align 8, !tbaa !20
  %83 = icmp slt i64 %80, %82
  br i1 %83, label %84, label %163

84:                                               ; preds = %76
  %85 = getelementptr inbounds nuw i8, ptr %30, i64 120
  br label %86

86:                                               ; preds = %100, %84
  %87 = phi i64 [ %80, %84 ], [ %101, %100 ]
  %88 = getelementptr inbounds i32, ptr %74, i64 %87
  %89 = load i32, ptr %88, align 4, !tbaa !15
  %90 = icmp sgt i32 %89, -1
  br i1 %90, label %91, label %100

91:                                               ; preds = %86
  %92 = zext nneg i32 %89 to i64
  %93 = load i64, ptr %85, align 8, !tbaa !56
  %94 = icmp sgt i64 %93, %92
  br i1 %94, label %95, label %100

95:                                               ; preds = %91
  %96 = load ptr, ptr %18, align 8, !tbaa !157
  %97 = getelementptr inbounds nuw i8, ptr %96, i64 %92
  %98 = load i8, ptr %97, align 1, !tbaa !76
  %99 = icmp eq i8 %98, 0
  br i1 %99, label %100, label %157

100:                                              ; preds = %95, %91, %86
  %101 = add i64 %87, 1
  %102 = icmp eq i64 %101, %82
  br i1 %102, label %163, label %86, !llvm.loop !194

103:                                              ; preds = %28
  %104 = getelementptr inbounds nuw i8, ptr %30, i64 168
  %105 = load ptr, ptr %104, align 8, !tbaa !67
  %106 = icmp eq ptr %105, null
  br i1 %106, label %163, label %107

107:                                              ; preds = %103
  %108 = getelementptr inbounds nuw i8, ptr %30, i64 176
  %109 = load ptr, ptr %108, align 8, !tbaa !68
  %110 = icmp eq ptr %109, null
  br i1 %110, label %163, label %111

111:                                              ; preds = %107
  %112 = getelementptr inbounds nuw i8, ptr %30, i64 184
  %113 = load i32, ptr %112, align 8, !tbaa !66
  %114 = icmp sgt i32 %113, 0
  br i1 %114, label %115, label %163

115:                                              ; preds = %111
  %116 = sdiv i32 %29, %113
  %117 = srem i32 %29, %113
  %118 = sext i32 %116 to i64
  %119 = getelementptr inbounds i32, ptr %105, i64 %118
  %120 = load i32, ptr %119, align 4, !tbaa !15
  %121 = getelementptr i8, ptr %119, i64 4
  %122 = load i32, ptr %121, align 4, !tbaa !15
  %123 = icmp slt i32 %120, %122
  br i1 %123, label %124, label %163

124:                                              ; preds = %115
  %125 = getelementptr inbounds nuw i8, ptr %30, i64 120
  br label %126

126:                                              ; preds = %153, %124
  %127 = phi i32 [ %120, %124 ], [ %154, %153 ]
  %128 = sext i32 %127 to i64
  %129 = getelementptr inbounds i32, ptr %109, i64 %128
  %130 = load i32, ptr %129, align 4, !tbaa !15
  %131 = icmp eq i32 %130, %117
  br i1 %131, label %132, label %145

132:                                              ; preds = %126
  %133 = getelementptr i8, ptr %129, i64 4
  %134 = load i32, ptr %133, align 4, !tbaa !15
  %135 = icmp sgt i32 %134, -1
  br i1 %135, label %136, label %150

136:                                              ; preds = %132
  %137 = zext nneg i32 %134 to i64
  %138 = load i64, ptr %125, align 8, !tbaa !56
  %139 = icmp sgt i64 %138, %137
  br i1 %139, label %140, label %150

140:                                              ; preds = %136
  %141 = load ptr, ptr %18, align 8, !tbaa !157
  %142 = getelementptr inbounds nuw i8, ptr %141, i64 %137
  %143 = load i8, ptr %142, align 1, !tbaa !76
  %144 = icmp eq i8 %143, 0
  br i1 %144, label %150, label %155

145:                                              ; preds = %126
  %146 = icmp sle i32 %130, %117
  %147 = add nsw i32 %127, 2
  %148 = icmp slt i32 %147, %122
  %149 = select i1 %146, i1 %148, i1 false
  br i1 %149, label %153, label %163

150:                                              ; preds = %140, %136, %132
  %151 = add nsw i32 %127, 2
  %152 = icmp slt i32 %151, %122
  br i1 %152, label %153, label %163

153:                                              ; preds = %150, %145
  %154 = phi i32 [ %151, %150 ], [ %147, %145 ]
  br label %126, !llvm.loop !195

155:                                              ; preds = %140
  %156 = shl i64 %22, 32
  br label %157

157:                                              ; preds = %95, %60, %155
  %158 = phi i64 [ %156, %155 ], [ %42, %60 ], [ %77, %95 ]
  %159 = phi i32 [ %134, %155 ], [ %54, %60 ], [ %89, %95 ]
  %160 = load i32, ptr %19, align 4, !tbaa !155
  %161 = ashr exact i64 %158, 30
  %162 = getelementptr inbounds i8, ptr %23, i64 %161
  store i32 %160, ptr %162, align 4, !tbaa !15
  tail call fastcc void @autograph_frontier_append_claimed(ptr noundef nonnull readonly %1, i32 noundef range(i32 0, -2147483648) %159, i32 noundef %29)
  br label %163

163:                                              ; preds = %145, %150, %100, %65, %115, %76, %41, %37, %33, %72, %68, %111, %107, %103, %28, %157, %21
  %164 = add i64 %22, 1
  %165 = icmp eq i64 %164, %13
  br i1 %165, label %20, label %21, !llvm.loop !196
}

; Function Attrs: nounwind uwtable
define internal void @autograph_frontier_push_body(i64 noundef %0, ptr nocapture noundef readonly %1) #0 {
  %3 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %4 = load ptr, ptr %3, align 8, !tbaa !150
  %5 = getelementptr inbounds i32, ptr %4, i64 %0
  %6 = load i32, ptr %5, align 4, !tbaa !15
  %7 = load ptr, ptr %1, align 8, !tbaa !148
  %8 = icmp slt i32 %6, 0
  br i1 %8, label %294, label %9

9:                                                ; preds = %2
  %10 = zext nneg i32 %6 to i64
  %11 = getelementptr inbounds nuw i8, ptr %7, i64 120
  %12 = load i64, ptr %11, align 8, !tbaa !56
  %13 = icmp sgt i64 %12, %10
  br i1 %13, label %14, label %294

14:                                               ; preds = %9
  %15 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %16 = load i32, ptr %15, align 8, !tbaa !53
  switch i32 %16, label %167 [
    i32 0, label %17
    i32 1, label %62
    i32 2, label %107
  ]

17:                                               ; preds = %14
  %18 = getelementptr inbounds nuw i8, ptr %7, i64 96
  %19 = load ptr, ptr %18, align 8, !tbaa !54
  %20 = icmp eq ptr %19, null
  br i1 %20, label %294, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %7, i64 104
  %23 = load ptr, ptr %22, align 8, !tbaa !55
  %24 = icmp eq ptr %23, null
  br i1 %24, label %294, label %25

25:                                               ; preds = %21
  %26 = getelementptr inbounds nuw i64, ptr %19, i64 %10
  %27 = load i64, ptr %26, align 8, !tbaa !20
  %28 = getelementptr inbounds nuw i8, ptr %26, i64 8
  %29 = load i64, ptr %28, align 8, !tbaa !20
  %30 = icmp slt i64 %27, %29
  br i1 %30, label %31, label %294

31:                                               ; preds = %25
  %32 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %33 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %34 = getelementptr inbounds nuw i8, ptr %1, i64 44
  br label %35

35:                                               ; preds = %55, %31
  %36 = phi i64 [ %27, %31 ], [ %56, %55 ]
  %37 = load ptr, ptr %22, align 8, !tbaa !55
  %38 = getelementptr inbounds i32, ptr %37, i64 %36
  %39 = load i32, ptr %38, align 4, !tbaa !15
  %40 = icmp slt i32 %39, 0
  br i1 %40, label %55, label %41

41:                                               ; preds = %35
  %42 = zext nneg i32 %39 to i64
  %43 = load ptr, ptr %1, align 8, !tbaa !148
  %44 = getelementptr inbounds nuw i8, ptr %43, i64 120
  %45 = load i64, ptr %44, align 8, !tbaa !56
  %46 = icmp sgt i64 %45, %42
  br i1 %46, label %47, label %55

47:                                               ; preds = %41
  %48 = load i32, ptr %32, align 8, !tbaa !154
  %49 = load ptr, ptr %33, align 8, !tbaa !153
  %50 = getelementptr inbounds nuw i32, ptr %49, i64 %42
  %51 = load i32, ptr %34, align 4, !tbaa !155
  %52 = cmpxchg ptr %50, i32 %48, i32 %51 monotonic monotonic, align 4
  %53 = extractvalue { i32, i1 } %52, 1
  br i1 %53, label %54, label %55

54:                                               ; preds = %47
  tail call fastcc void @autograph_frontier_append_claimed(ptr noundef nonnull readonly %1, i32 noundef range(i32 0, -2147483648) %6, i32 noundef %39)
  br label %55

55:                                               ; preds = %54, %47, %41, %35
  %56 = add nsw i64 %36, 1
  %57 = load ptr, ptr %18, align 8, !tbaa !54
  %58 = getelementptr inbounds nuw i64, ptr %57, i64 %10
  %59 = getelementptr inbounds nuw i8, ptr %58, i64 8
  %60 = load i64, ptr %59, align 8, !tbaa !20
  %61 = icmp slt i64 %56, %60
  br i1 %61, label %35, label %294, !llvm.loop !197

62:                                               ; preds = %14
  %63 = getelementptr inbounds nuw i8, ptr %7, i64 144
  %64 = load ptr, ptr %63, align 8, !tbaa !63
  %65 = icmp eq ptr %64, null
  br i1 %65, label %294, label %66

66:                                               ; preds = %62
  %67 = getelementptr inbounds nuw i8, ptr %7, i64 152
  %68 = load ptr, ptr %67, align 8, !tbaa !64
  %69 = icmp eq ptr %68, null
  br i1 %69, label %294, label %70

70:                                               ; preds = %66
  %71 = getelementptr inbounds nuw i64, ptr %64, i64 %10
  %72 = load i64, ptr %71, align 8, !tbaa !20
  %73 = getelementptr inbounds nuw i8, ptr %71, i64 8
  %74 = load i64, ptr %73, align 8, !tbaa !20
  %75 = icmp slt i64 %72, %74
  br i1 %75, label %76, label %294

76:                                               ; preds = %70
  %77 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %78 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %79 = getelementptr inbounds nuw i8, ptr %1, i64 44
  br label %80

80:                                               ; preds = %100, %76
  %81 = phi i64 [ %72, %76 ], [ %101, %100 ]
  %82 = load ptr, ptr %67, align 8, !tbaa !64
  %83 = getelementptr inbounds i32, ptr %82, i64 %81
  %84 = load i32, ptr %83, align 4, !tbaa !15
  %85 = icmp slt i32 %84, 0
  br i1 %85, label %100, label %86

86:                                               ; preds = %80
  %87 = zext nneg i32 %84 to i64
  %88 = load ptr, ptr %1, align 8, !tbaa !148
  %89 = getelementptr inbounds nuw i8, ptr %88, i64 120
  %90 = load i64, ptr %89, align 8, !tbaa !56
  %91 = icmp sgt i64 %90, %87
  br i1 %91, label %92, label %100

92:                                               ; preds = %86
  %93 = load i32, ptr %77, align 8, !tbaa !154
  %94 = load ptr, ptr %78, align 8, !tbaa !153
  %95 = getelementptr inbounds nuw i32, ptr %94, i64 %87
  %96 = load i32, ptr %79, align 4, !tbaa !155
  %97 = cmpxchg ptr %95, i32 %93, i32 %96 monotonic monotonic, align 4
  %98 = extractvalue { i32, i1 } %97, 1
  br i1 %98, label %99, label %100

99:                                               ; preds = %92
  tail call fastcc void @autograph_frontier_append_claimed(ptr noundef nonnull readonly %1, i32 noundef range(i32 0, -2147483648) %6, i32 noundef %84)
  br label %100

100:                                              ; preds = %99, %92, %86, %80
  %101 = add nsw i64 %81, 1
  %102 = load ptr, ptr %63, align 8, !tbaa !63
  %103 = getelementptr inbounds nuw i64, ptr %102, i64 %10
  %104 = getelementptr inbounds nuw i8, ptr %103, i64 8
  %105 = load i64, ptr %104, align 8, !tbaa !20
  %106 = icmp slt i64 %101, %105
  br i1 %106, label %80, label %294, !llvm.loop !198

107:                                              ; preds = %14
  %108 = getelementptr inbounds nuw i8, ptr %7, i64 168
  %109 = load ptr, ptr %108, align 8, !tbaa !67
  %110 = icmp eq ptr %109, null
  br i1 %110, label %294, label %111

111:                                              ; preds = %107
  %112 = getelementptr inbounds nuw i8, ptr %7, i64 176
  %113 = load ptr, ptr %112, align 8, !tbaa !68
  %114 = icmp eq ptr %113, null
  br i1 %114, label %294, label %115

115:                                              ; preds = %111
  %116 = getelementptr inbounds nuw i8, ptr %7, i64 184
  %117 = load i32, ptr %116, align 8, !tbaa !66
  %118 = icmp sgt i32 %117, 0
  br i1 %118, label %119, label %294

119:                                              ; preds = %115
  %120 = udiv i32 %6, %117
  %121 = urem i32 %6, %117
  %122 = zext nneg i32 %120 to i64
  %123 = getelementptr inbounds nuw i32, ptr %109, i64 %122
  %124 = load i32, ptr %123, align 4, !tbaa !15
  %125 = getelementptr inbounds nuw i8, ptr %123, i64 4
  %126 = load i32, ptr %125, align 4, !tbaa !15
  %127 = icmp slt i32 %124, %126
  br i1 %127, label %128, label %294

128:                                              ; preds = %119
  %129 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %130 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %131 = getelementptr inbounds nuw i8, ptr %1, i64 44
  %132 = sext i32 %124 to i64
  br label %133

133:                                              ; preds = %159, %128
  %134 = phi i64 [ %132, %128 ], [ %160, %159 ]
  %135 = load ptr, ptr %112, align 8, !tbaa !68
  %136 = getelementptr inbounds i32, ptr %135, i64 %134
  %137 = load i32, ptr %136, align 4, !tbaa !15
  %138 = icmp eq i32 %137, %121
  br i1 %138, label %139, label %157

139:                                              ; preds = %133
  %140 = getelementptr i8, ptr %136, i64 4
  %141 = load i32, ptr %140, align 4, !tbaa !15
  %142 = icmp slt i32 %141, 0
  br i1 %142, label %159, label %143

143:                                              ; preds = %139
  %144 = zext nneg i32 %141 to i64
  %145 = load ptr, ptr %1, align 8, !tbaa !148
  %146 = getelementptr inbounds nuw i8, ptr %145, i64 120
  %147 = load i64, ptr %146, align 8, !tbaa !56
  %148 = icmp sgt i64 %147, %144
  br i1 %148, label %149, label %159

149:                                              ; preds = %143
  %150 = load i32, ptr %129, align 8, !tbaa !154
  %151 = load ptr, ptr %130, align 8, !tbaa !153
  %152 = getelementptr inbounds nuw i32, ptr %151, i64 %144
  %153 = load i32, ptr %131, align 4, !tbaa !155
  %154 = cmpxchg ptr %152, i32 %150, i32 %153 monotonic monotonic, align 4
  %155 = extractvalue { i32, i1 } %154, 1
  br i1 %155, label %156, label %159

156:                                              ; preds = %149
  tail call fastcc void @autograph_frontier_append_claimed(ptr noundef nonnull readonly %1, i32 noundef range(i32 0, -2147483648) %6, i32 noundef %141)
  br label %159

157:                                              ; preds = %133
  %158 = icmp sgt i32 %137, %121
  br i1 %158, label %294, label %159

159:                                              ; preds = %157, %156, %149, %143, %139
  %160 = add nsw i64 %134, 2
  %161 = load ptr, ptr %108, align 8, !tbaa !67
  %162 = getelementptr inbounds nuw i32, ptr %161, i64 %122
  %163 = getelementptr inbounds nuw i8, ptr %162, i64 4
  %164 = load i32, ptr %163, align 4, !tbaa !15
  %165 = sext i32 %164 to i64
  %166 = icmp slt i64 %160, %165
  br i1 %166, label %133, label %294, !llvm.loop !199

167:                                              ; preds = %14
  %168 = getelementptr inbounds nuw i8, ptr %7, i64 24
  %169 = load ptr, ptr %168, align 8, !tbaa !60
  %170 = getelementptr inbounds nuw i8, ptr %7, i64 32
  %171 = load ptr, ptr %170, align 8, !tbaa !61
  %172 = icmp ne ptr %169, null
  %173 = icmp ne ptr %171, null
  %174 = select i1 %172, i1 %173, i1 false
  br i1 %174, label %175, label %231

175:                                              ; preds = %167
  %176 = getelementptr inbounds nuw i8, ptr %7, i64 40
  %177 = load i64, ptr %176, align 8, !tbaa !74
  %178 = icmp sgt i64 %177, 0
  br i1 %178, label %179, label %231

179:                                              ; preds = %175
  %180 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %181 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %182 = getelementptr inbounds nuw i8, ptr %1, i64 44
  br label %183

183:                                              ; preds = %227, %179
  %184 = phi i64 [ 0, %179 ], [ %228, %227 ]
  %185 = trunc i64 %184 to i32
  %186 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef nonnull %169, i32 noundef %185) #28
  br i1 %186, label %187, label %227

187:                                              ; preds = %183
  %188 = getelementptr inbounds nuw %struct.EdgePair, ptr %171, i64 %184
  %189 = load i32, ptr %188, align 4, !tbaa !21
  %190 = icmp eq i32 %189, %6
  %191 = getelementptr inbounds nuw i8, ptr %188, i64 4
  %192 = load i32, ptr %191, align 4, !tbaa !23
  br i1 %190, label %193, label %208

193:                                              ; preds = %187
  %194 = icmp slt i32 %192, 0
  br i1 %194, label %227, label %195

195:                                              ; preds = %193
  %196 = zext nneg i32 %192 to i64
  %197 = load ptr, ptr %1, align 8, !tbaa !148
  %198 = getelementptr inbounds nuw i8, ptr %197, i64 120
  %199 = load i64, ptr %198, align 8, !tbaa !56
  %200 = icmp sgt i64 %199, %196
  br i1 %200, label %201, label %227

201:                                              ; preds = %195
  %202 = load i32, ptr %180, align 8, !tbaa !154
  %203 = load ptr, ptr %181, align 8, !tbaa !153
  %204 = getelementptr inbounds nuw i32, ptr %203, i64 %196
  %205 = load i32, ptr %182, align 4, !tbaa !155
  %206 = cmpxchg ptr %204, i32 %202, i32 %205 monotonic monotonic, align 4
  %207 = extractvalue { i32, i1 } %206, 1
  br i1 %207, label %225, label %227

208:                                              ; preds = %187
  %209 = icmp ne i32 %192, %6
  %210 = icmp slt i32 %189, 0
  %211 = or i1 %210, %209
  br i1 %211, label %227, label %212

212:                                              ; preds = %208
  %213 = zext nneg i32 %189 to i64
  %214 = load ptr, ptr %1, align 8, !tbaa !148
  %215 = getelementptr inbounds nuw i8, ptr %214, i64 120
  %216 = load i64, ptr %215, align 8, !tbaa !56
  %217 = icmp sgt i64 %216, %213
  br i1 %217, label %218, label %227

218:                                              ; preds = %212
  %219 = load i32, ptr %180, align 8, !tbaa !154
  %220 = load ptr, ptr %181, align 8, !tbaa !153
  %221 = getelementptr inbounds nuw i32, ptr %220, i64 %213
  %222 = load i32, ptr %182, align 4, !tbaa !155
  %223 = cmpxchg ptr %221, i32 %219, i32 %222 monotonic monotonic, align 4
  %224 = extractvalue { i32, i1 } %223, 1
  br i1 %224, label %225, label %227

225:                                              ; preds = %218, %201
  %226 = phi i32 [ %192, %201 ], [ %189, %218 ]
  tail call fastcc void @autograph_frontier_append_claimed(ptr noundef nonnull readonly %1, i32 noundef range(i32 0, -2147483648) %6, i32 noundef %226)
  br label %227

227:                                              ; preds = %225, %218, %212, %208, %201, %195, %193, %183
  %228 = add nuw nsw i64 %184, 1
  %229 = load i64, ptr %176, align 8, !tbaa !74
  %230 = icmp slt i64 %228, %229
  br i1 %230, label %183, label %231, !llvm.loop !200

231:                                              ; preds = %227, %175, %167
  %232 = getelementptr inbounds nuw i8, ptr %7, i64 64
  %233 = load i64, ptr %232, align 8, !tbaa !71
  %234 = icmp sgt i64 %233, 0
  br i1 %234, label %235, label %294

235:                                              ; preds = %231
  %236 = getelementptr inbounds nuw i8, ptr %7, i64 56
  %237 = getelementptr inbounds nuw i8, ptr %7, i64 48
  %238 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %239 = getelementptr inbounds nuw i8, ptr %1, i64 32
  %240 = getelementptr inbounds nuw i8, ptr %1, i64 44
  br label %241

241:                                              ; preds = %290, %235
  %242 = phi i64 [ 0, %235 ], [ %291, %290 ]
  %243 = load ptr, ptr %236, align 8, !tbaa !75
  %244 = getelementptr inbounds nuw i8, ptr %243, i64 %242
  %245 = load i8, ptr %244, align 1, !tbaa !76
  %246 = icmp eq i8 %245, 0
  br i1 %246, label %290, label %247

247:                                              ; preds = %241
  %248 = load ptr, ptr %237, align 8, !tbaa !80
  %249 = shl nuw nsw i64 %242, 1
  %250 = getelementptr inbounds nuw i32, ptr %248, i64 %249
  %251 = load i32, ptr %250, align 4, !tbaa !15
  %252 = or disjoint i64 %249, 1
  %253 = getelementptr inbounds nuw i32, ptr %248, i64 %252
  %254 = load i32, ptr %253, align 4, !tbaa !15
  %255 = icmp eq i32 %251, %6
  br i1 %255, label %256, label %271

256:                                              ; preds = %247
  %257 = icmp slt i32 %254, 0
  br i1 %257, label %290, label %258

258:                                              ; preds = %256
  %259 = zext nneg i32 %254 to i64
  %260 = load ptr, ptr %1, align 8, !tbaa !148
  %261 = getelementptr inbounds nuw i8, ptr %260, i64 120
  %262 = load i64, ptr %261, align 8, !tbaa !56
  %263 = icmp sgt i64 %262, %259
  br i1 %263, label %264, label %290

264:                                              ; preds = %258
  %265 = load i32, ptr %238, align 8, !tbaa !154
  %266 = load ptr, ptr %239, align 8, !tbaa !153
  %267 = getelementptr inbounds nuw i32, ptr %266, i64 %259
  %268 = load i32, ptr %240, align 4, !tbaa !155
  %269 = cmpxchg ptr %267, i32 %265, i32 %268 monotonic monotonic, align 4
  %270 = extractvalue { i32, i1 } %269, 1
  br i1 %270, label %288, label %290

271:                                              ; preds = %247
  %272 = icmp ne i32 %254, %6
  %273 = icmp slt i32 %251, 0
  %274 = or i1 %273, %272
  br i1 %274, label %290, label %275

275:                                              ; preds = %271
  %276 = zext nneg i32 %251 to i64
  %277 = load ptr, ptr %1, align 8, !tbaa !148
  %278 = getelementptr inbounds nuw i8, ptr %277, i64 120
  %279 = load i64, ptr %278, align 8, !tbaa !56
  %280 = icmp sgt i64 %279, %276
  br i1 %280, label %281, label %290

281:                                              ; preds = %275
  %282 = load i32, ptr %238, align 8, !tbaa !154
  %283 = load ptr, ptr %239, align 8, !tbaa !153
  %284 = getelementptr inbounds nuw i32, ptr %283, i64 %276
  %285 = load i32, ptr %240, align 4, !tbaa !155
  %286 = cmpxchg ptr %284, i32 %282, i32 %285 monotonic monotonic, align 4
  %287 = extractvalue { i32, i1 } %286, 1
  br i1 %287, label %288, label %290

288:                                              ; preds = %281, %264
  %289 = phi i32 [ %254, %264 ], [ %251, %281 ]
  tail call fastcc void @autograph_frontier_append_claimed(ptr noundef nonnull readonly %1, i32 noundef range(i32 0, -2147483648) %6, i32 noundef %289)
  br label %290

290:                                              ; preds = %288, %281, %275, %271, %264, %258, %256, %241
  %291 = add nuw nsw i64 %242, 1
  %292 = load i64, ptr %232, align 8, !tbaa !71
  %293 = icmp slt i64 %291, %292
  br i1 %293, label %241, label %294, !llvm.loop !201

294:                                              ; preds = %157, %159, %100, %55, %290, %2, %9, %17, %21, %25, %62, %66, %70, %107, %111, %115, %119, %231
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @autograph_scratch_merge_lanes(ptr nocapture noundef readonly %0, i32 noundef %1, ptr noundef %2, i32 noundef range(i32 0, -2147483648) %3) unnamed_addr #0 {
  %5 = alloca %struct.AutoFrontierMergeEnv, align 8
  %6 = getelementptr inbounds nuw i8, ptr %0, i64 200
  %7 = load ptr, ptr %6, align 8, !tbaa !143
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 240
  %9 = load ptr, ptr %8, align 8, !tbaa !187
  %10 = zext nneg i32 %3 to i64
  store i64 %10, ptr %9, align 8, !tbaa !20
  %11 = icmp sgt i32 %1, 0
  br i1 %11, label %12, label %15

12:                                               ; preds = %4
  %13 = getelementptr inbounds nuw i8, ptr %0, i64 120
  %14 = zext nneg i32 %1 to i64
  br label %32

15:                                               ; preds = %44, %4
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %5) #28
  store ptr %7, ptr %5, align 8, !tbaa !202
  %16 = getelementptr inbounds nuw i8, ptr %5, i64 8
  store ptr %9, ptr %16, align 8, !tbaa !204
  %17 = getelementptr inbounds nuw i8, ptr %5, i64 16
  store ptr %2, ptr %17, align 8, !tbaa !205
  %18 = sext i32 %1 to i64
  %19 = getelementptr inbounds i64, ptr %9, i64 %18
  %20 = load i64, ptr %19, align 8, !tbaa !20
  %21 = sub nsw i64 %20, %10
  %22 = icmp sgt i64 %21, 65535
  %23 = icmp sgt i32 %1, 1
  %24 = and i1 %23, %22
  br i1 %24, label %49, label %25

25:                                               ; preds = %15
  br i1 %11, label %26, label %96

26:                                               ; preds = %25
  %27 = zext nneg i32 %1 to i64
  %28 = and i64 %27, 1
  %29 = icmp eq i32 %1, 1
  br i1 %29, label %81, label %30

30:                                               ; preds = %26
  %31 = and i64 %27, 2147483646
  br label %50

32:                                               ; preds = %12, %44
  %33 = phi i64 [ %10, %12 ], [ %45, %44 ]
  %34 = phi i64 [ 0, %12 ], [ %46, %44 ]
  %35 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %7, i64 %34, i32 1
  %36 = load i32, ptr %35, align 8, !tbaa !144
  %37 = icmp slt i32 %36, 0
  br i1 %37, label %43, label %38

38:                                               ; preds = %32
  %39 = load i64, ptr %13, align 8, !tbaa !56
  %40 = zext nneg i32 %36 to i64
  %41 = sub nsw i64 %39, %40
  %42 = icmp sgt i64 %33, %41
  br i1 %42, label %43, label %44

43:                                               ; preds = %38, %32
  tail call void @abort() #34
  unreachable

44:                                               ; preds = %38
  %45 = add nuw nsw i64 %33, %40
  %46 = add nuw nsw i64 %34, 1
  %47 = getelementptr inbounds nuw i64, ptr %9, i64 %46
  store i64 %45, ptr %47, align 8, !tbaa !20
  %48 = icmp eq i64 %46, %14
  br i1 %48, label %15, label %32, !llvm.loop !206

49:                                               ; preds = %15
  call void @parallel_for_runtime(i64 noundef 0, i64 noundef %18, i64 noundef 1, ptr noundef nonnull @autograph_frontier_merge_lane, ptr noundef nonnull %5, i32 noundef 0, i32 noundef 0) #28
  br label %96

50:                                               ; preds = %77, %30
  %51 = phi i64 [ 0, %30 ], [ %78, %77 ]
  %52 = phi i64 [ 0, %30 ], [ %79, %77 ]
  %53 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %7, i64 %51
  %54 = getelementptr inbounds nuw i8, ptr %53, i64 8
  %55 = load i32, ptr %54, align 8, !tbaa !144
  %56 = icmp sgt i32 %55, 0
  br i1 %56, label %57, label %64

57:                                               ; preds = %50
  %58 = getelementptr inbounds nuw i64, ptr %9, i64 %51
  %59 = load i64, ptr %58, align 8, !tbaa !20
  %60 = getelementptr inbounds i32, ptr %2, i64 %59
  %61 = load ptr, ptr %53, align 8, !tbaa !189
  %62 = zext nneg i32 %55 to i64
  %63 = shl nuw nsw i64 %62, 2
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %60, ptr align 4 %61, i64 %63, i1 false)
  br label %64

64:                                               ; preds = %50, %57
  %65 = or disjoint i64 %51, 1
  %66 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %7, i64 %65
  %67 = getelementptr inbounds nuw i8, ptr %66, i64 8
  %68 = load i32, ptr %67, align 8, !tbaa !144
  %69 = icmp sgt i32 %68, 0
  br i1 %69, label %70, label %77

70:                                               ; preds = %64
  %71 = getelementptr inbounds nuw i64, ptr %9, i64 %65
  %72 = load i64, ptr %71, align 8, !tbaa !20
  %73 = getelementptr inbounds i32, ptr %2, i64 %72
  %74 = load ptr, ptr %66, align 8, !tbaa !189
  %75 = zext nneg i32 %68 to i64
  %76 = shl nuw nsw i64 %75, 2
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %73, ptr align 4 %74, i64 %76, i1 false)
  br label %77

77:                                               ; preds = %70, %64
  %78 = add nuw nsw i64 %51, 2
  %79 = add i64 %52, 2
  %80 = icmp eq i64 %79, %31
  br i1 %80, label %81, label %50, !llvm.loop !207

81:                                               ; preds = %77, %26
  %82 = phi i64 [ 0, %26 ], [ %78, %77 ]
  %83 = icmp eq i64 %28, 0
  br i1 %83, label %96, label %84

84:                                               ; preds = %81
  %85 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %7, i64 %82
  %86 = getelementptr inbounds nuw i8, ptr %85, i64 8
  %87 = load i32, ptr %86, align 8, !tbaa !144
  %88 = icmp sgt i32 %87, 0
  br i1 %88, label %89, label %96

89:                                               ; preds = %84
  %90 = getelementptr inbounds nuw i64, ptr %9, i64 %82
  %91 = load i64, ptr %90, align 8, !tbaa !20
  %92 = getelementptr inbounds i32, ptr %2, i64 %91
  %93 = load ptr, ptr %85, align 8, !tbaa !189
  %94 = zext nneg i32 %87 to i64
  %95 = shl nuw nsw i64 %94, 2
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %92, ptr align 4 %93, i64 %95, i1 false)
  br label %96

96:                                               ; preds = %81, %89, %84, %25, %49
  %97 = load i64, ptr %19, align 8, !tbaa !20
  %98 = trunc i64 %97 to i32
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #28
  ret i32 %98
}

; Function Attrs: cold nofree noreturn nounwind
declare void @abort() local_unnamed_addr #21

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr nocapture noundef, ptr nocapture noundef) local_unnamed_addr #22

; Function Attrs: nounwind uwtable
define internal fastcc void @autograph_frontier_append_claimed(ptr nocapture noundef readonly %0, i32 noundef range(i32 0, -2147483648) %1, i32 noundef %2) unnamed_addr #0 {
  %4 = tail call i32 @sgpl_current_worker_index() #28
  %5 = icmp slt i32 %4, 0
  br i1 %5, label %10, label %6

6:                                                ; preds = %3
  %7 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %8 = load i32, ptr %7, align 8, !tbaa !152
  %9 = icmp slt i32 %4, %8
  br i1 %9, label %11, label %10

10:                                               ; preds = %6, %3
  br label %11

11:                                               ; preds = %10, %6
  %12 = phi i32 [ 0, %10 ], [ %4, %6 ]
  %13 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %14 = load ptr, ptr %13, align 8, !tbaa !151
  %15 = zext nneg i32 %12 to i64
  %16 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %14, i64 %15
  %17 = getelementptr inbounds nuw i8, ptr %16, i64 8
  %18 = load i32, ptr %17, align 8, !tbaa !144
  %19 = getelementptr inbounds nuw i8, ptr %16, i64 12
  %20 = load i32, ptr %19, align 4, !tbaa !188
  %21 = icmp eq i32 %18, %20
  br i1 %21, label %24, label %22

22:                                               ; preds = %11
  %23 = load ptr, ptr %16, align 8, !tbaa !189
  br label %40

24:                                               ; preds = %11
  %25 = icmp eq i32 %18, 0
  %26 = sext i32 %18 to i64
  %27 = shl nsw i64 %26, 1
  %28 = select i1 %25, i64 64, i64 %27
  %29 = icmp ugt i64 %28, 2147483647
  br i1 %29, label %30, label %31

30:                                               ; preds = %24
  tail call void @abort() #34
  unreachable

31:                                               ; preds = %24
  %32 = load ptr, ptr %16, align 8, !tbaa !189
  %33 = shl nuw nsw i64 %28, 2
  %34 = tail call ptr @realloc(ptr noundef %32, i64 noundef %33) #31
  %35 = icmp eq ptr %34, null
  br i1 %35, label %36, label %37

36:                                               ; preds = %31
  tail call void @abort() #34
  unreachable

37:                                               ; preds = %31
  %38 = trunc nuw nsw i64 %28 to i32
  store ptr %34, ptr %16, align 8, !tbaa !189
  store i32 %38, ptr %19, align 4, !tbaa !188
  %39 = load i32, ptr %17, align 8, !tbaa !144
  br label %40

40:                                               ; preds = %22, %37
  %41 = phi i32 [ %18, %22 ], [ %39, %37 ]
  %42 = phi ptr [ %23, %22 ], [ %34, %37 ]
  %43 = add nsw i32 %41, 1
  store i32 %43, ptr %17, align 8, !tbaa !144
  %44 = sext i32 %41 to i64
  %45 = getelementptr inbounds i32, ptr %42, i64 %44
  store i32 %2, ptr %45, align 4, !tbaa !15
  %46 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %47 = load ptr, ptr %46, align 8, !tbaa !156
  %48 = icmp eq ptr %47, null
  br i1 %48, label %52, label %49

49:                                               ; preds = %40
  %50 = sext i32 %2 to i64
  %51 = getelementptr inbounds i32, ptr %47, i64 %50
  store i32 %1, ptr %51, align 4, !tbaa !15
  br label %52

52:                                               ; preds = %49, %40
  ret void
}

declare i32 @sgpl_current_worker_index() local_unnamed_addr #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable
define internal void @autograph_frontier_merge_lane(i64 noundef %0, ptr nocapture noundef readonly %1) #23 {
  %3 = load ptr, ptr %1, align 8, !tbaa !202
  %4 = getelementptr inbounds %struct.AutoFrontierLane, ptr %3, i64 %0
  %5 = getelementptr inbounds nuw i8, ptr %4, i64 8
  %6 = load i32, ptr %5, align 8, !tbaa !144
  %7 = icmp sgt i32 %6, 0
  br i1 %7, label %8, label %19

8:                                                ; preds = %2
  %9 = getelementptr inbounds nuw i8, ptr %1, i64 16
  %10 = load ptr, ptr %9, align 8, !tbaa !205
  %11 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %12 = load ptr, ptr %11, align 8, !tbaa !204
  %13 = getelementptr inbounds i64, ptr %12, i64 %0
  %14 = load i64, ptr %13, align 8, !tbaa !20
  %15 = getelementptr inbounds i32, ptr %10, i64 %14
  %16 = load ptr, ptr %4, align 8, !tbaa !189
  %17 = zext nneg i32 %6 to i64
  %18 = shl nuw nsw i64 %17, 2
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %15, ptr align 4 %16, i64 %18, i1 false)
  br label %19

19:                                               ; preds = %8, %2
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @autograph_motif_write_min_pull_body(i64 noundef %0, ptr nocapture noundef readonly %1) #0 {
  %3 = load ptr, ptr %1, align 8, !tbaa !162
  %4 = getelementptr inbounds nuw i8, ptr %3, i64 120
  %5 = load i64, ptr %4, align 8, !tbaa !56
  %6 = mul nsw i64 %5, %0
  %7 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %8 = load i32, ptr %7, align 8, !tbaa !166
  %9 = sext i32 %8 to i64
  %10 = sdiv i64 %6, %9
  %11 = add nsw i64 %0, 1
  %12 = mul nsw i64 %5, %11
  %13 = sdiv i64 %12, %9
  %14 = icmp slt i64 %10, %13
  br i1 %14, label %15, label %19

15:                                               ; preds = %2
  %16 = getelementptr inbounds nuw i8, ptr %1, i64 60
  %17 = getelementptr inbounds nuw i8, ptr %1, i64 64
  %18 = getelementptr inbounds nuw i8, ptr %1, i64 32
  br label %20

19:                                               ; preds = %233, %2
  ret void

20:                                               ; preds = %15, %233
  %21 = phi i64 [ %10, %15 ], [ %234, %233 ]
  %22 = trunc i64 %21 to i32
  %23 = load ptr, ptr %1, align 8, !tbaa !162
  %24 = load i32, ptr %16, align 4, !tbaa !171
  %25 = icmp eq i32 %24, 0
  %26 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %27 = load i32, ptr %26, align 8, !tbaa !53
  br i1 %25, label %84, label %28

28:                                               ; preds = %20
  %29 = icmp eq i32 %27, 0
  br i1 %29, label %30, label %233

30:                                               ; preds = %28
  %31 = getelementptr inbounds nuw i8, ptr %23, i64 96
  %32 = load ptr, ptr %31, align 8, !tbaa !54
  %33 = icmp eq ptr %32, null
  br i1 %33, label %233, label %34

34:                                               ; preds = %30
  %35 = getelementptr inbounds nuw i8, ptr %23, i64 104
  %36 = load ptr, ptr %35, align 8, !tbaa !55
  %37 = icmp eq ptr %36, null
  br i1 %37, label %233, label %38

38:                                               ; preds = %34
  %39 = shl i64 %21, 32
  %40 = ashr exact i64 %39, 29
  %41 = getelementptr inbounds i8, ptr %32, i64 %40
  %42 = load i64, ptr %41, align 8, !tbaa !20
  %43 = getelementptr i8, ptr %41, i64 8
  %44 = load i64, ptr %43, align 8, !tbaa !20
  %45 = icmp slt i64 %42, %44
  br i1 %45, label %46, label %233

46:                                               ; preds = %38
  %47 = getelementptr inbounds nuw i8, ptr %23, i64 112
  %48 = load ptr, ptr %47, align 8, !tbaa !174
  %49 = icmp eq ptr %48, null
  %50 = getelementptr inbounds nuw i8, ptr %23, i64 120
  br label %51

51:                                               ; preds = %80, %46
  %52 = phi i32 [ 2147483647, %46 ], [ %81, %80 ]
  %53 = phi i64 [ %42, %46 ], [ %82, %80 ]
  br i1 %49, label %57, label %54

54:                                               ; preds = %51
  %55 = getelementptr inbounds i32, ptr %48, i64 %53
  %56 = load i32, ptr %55, align 4, !tbaa !15
  br label %57

57:                                               ; preds = %54, %51
  %58 = phi i32 [ %56, %54 ], [ 1, %51 ]
  %59 = getelementptr inbounds i32, ptr %36, i64 %53
  %60 = load i32, ptr %59, align 4, !tbaa !15
  %61 = icmp sgt i32 %60, -1
  br i1 %61, label %62, label %80

62:                                               ; preds = %57
  %63 = zext nneg i32 %60 to i64
  %64 = load i64, ptr %50, align 8, !tbaa !56
  %65 = icmp sgt i64 %64, %63
  br i1 %65, label %66, label %80

66:                                               ; preds = %62
  %67 = load ptr, ptr %17, align 8, !tbaa !172
  %68 = getelementptr inbounds nuw i8, ptr %67, i64 %63
  %69 = load i8, ptr %68, align 1, !tbaa !76
  %70 = icmp eq i8 %69, 0
  br i1 %70, label %80, label %71

71:                                               ; preds = %66
  %72 = load ptr, ptr %18, align 8, !tbaa !167
  %73 = getelementptr inbounds nuw i32, ptr %72, i64 %63
  %74 = load i32, ptr %73, align 4, !tbaa !15
  %75 = sub nsw i32 2147483647, %58
  %76 = icmp sgt i32 %74, %75
  br i1 %76, label %80, label %77

77:                                               ; preds = %71
  %78 = add nsw i32 %74, %58
  %79 = tail call i32 @llvm.smin.i32(i32 %78, i32 %52)
  br label %80

80:                                               ; preds = %77, %71, %66, %62, %57
  %81 = phi i32 [ %52, %66 ], [ %52, %62 ], [ %52, %57 ], [ %79, %77 ], [ %52, %71 ]
  %82 = add nsw i64 %53, 1
  %83 = icmp eq i64 %82, %44
  br i1 %83, label %224, label %51, !llvm.loop !208

84:                                               ; preds = %20
  switch i32 %27, label %233 [
    i32 0, label %85
    i32 1, label %127
    i32 2, label %169
  ]

85:                                               ; preds = %84
  %86 = getelementptr inbounds nuw i8, ptr %23, i64 96
  %87 = load ptr, ptr %86, align 8, !tbaa !54
  %88 = icmp eq ptr %87, null
  br i1 %88, label %233, label %89

89:                                               ; preds = %85
  %90 = getelementptr inbounds nuw i8, ptr %23, i64 104
  %91 = load ptr, ptr %90, align 8, !tbaa !55
  %92 = icmp eq ptr %91, null
  br i1 %92, label %233, label %93

93:                                               ; preds = %89
  %94 = shl i64 %21, 32
  %95 = ashr exact i64 %94, 29
  %96 = getelementptr inbounds i8, ptr %87, i64 %95
  %97 = load i64, ptr %96, align 8, !tbaa !20
  %98 = getelementptr i8, ptr %96, i64 8
  %99 = load i64, ptr %98, align 8, !tbaa !20
  %100 = icmp slt i64 %97, %99
  br i1 %100, label %101, label %233

101:                                              ; preds = %93
  %102 = getelementptr inbounds nuw i8, ptr %23, i64 120
  br label %103

103:                                              ; preds = %123, %101
  %104 = phi i32 [ 2147483647, %101 ], [ %124, %123 ]
  %105 = phi i64 [ %97, %101 ], [ %125, %123 ]
  %106 = getelementptr inbounds i32, ptr %91, i64 %105
  %107 = load i32, ptr %106, align 4, !tbaa !15
  %108 = icmp sgt i32 %107, -1
  br i1 %108, label %109, label %123

109:                                              ; preds = %103
  %110 = zext nneg i32 %107 to i64
  %111 = load i64, ptr %102, align 8, !tbaa !56
  %112 = icmp sgt i64 %111, %110
  br i1 %112, label %113, label %123

113:                                              ; preds = %109
  %114 = load ptr, ptr %17, align 8, !tbaa !172
  %115 = getelementptr inbounds nuw i8, ptr %114, i64 %110
  %116 = load i8, ptr %115, align 1, !tbaa !76
  %117 = icmp eq i8 %116, 0
  br i1 %117, label %123, label %118

118:                                              ; preds = %113
  %119 = load ptr, ptr %18, align 8, !tbaa !167
  %120 = getelementptr inbounds nuw i32, ptr %119, i64 %110
  %121 = load i32, ptr %120, align 4, !tbaa !15
  %122 = tail call i32 @llvm.smin.i32(i32 %121, i32 %104)
  br label %123

123:                                              ; preds = %118, %113, %109, %103
  %124 = phi i32 [ %104, %113 ], [ %104, %109 ], [ %104, %103 ], [ %122, %118 ]
  %125 = add nsw i64 %105, 1
  %126 = icmp eq i64 %125, %99
  br i1 %126, label %224, label %103, !llvm.loop !209

127:                                              ; preds = %84
  %128 = getelementptr inbounds nuw i8, ptr %23, i64 144
  %129 = load ptr, ptr %128, align 8, !tbaa !63
  %130 = icmp eq ptr %129, null
  br i1 %130, label %233, label %131

131:                                              ; preds = %127
  %132 = getelementptr inbounds nuw i8, ptr %23, i64 152
  %133 = load ptr, ptr %132, align 8, !tbaa !64
  %134 = icmp eq ptr %133, null
  br i1 %134, label %233, label %135

135:                                              ; preds = %131
  %136 = shl i64 %21, 32
  %137 = ashr exact i64 %136, 29
  %138 = getelementptr inbounds i8, ptr %129, i64 %137
  %139 = load i64, ptr %138, align 8, !tbaa !20
  %140 = getelementptr i8, ptr %138, i64 8
  %141 = load i64, ptr %140, align 8, !tbaa !20
  %142 = icmp slt i64 %139, %141
  br i1 %142, label %143, label %233

143:                                              ; preds = %135
  %144 = getelementptr inbounds nuw i8, ptr %23, i64 120
  br label %145

145:                                              ; preds = %165, %143
  %146 = phi i32 [ 2147483647, %143 ], [ %166, %165 ]
  %147 = phi i64 [ %139, %143 ], [ %167, %165 ]
  %148 = getelementptr inbounds i32, ptr %133, i64 %147
  %149 = load i32, ptr %148, align 4, !tbaa !15
  %150 = icmp sgt i32 %149, -1
  br i1 %150, label %151, label %165

151:                                              ; preds = %145
  %152 = zext nneg i32 %149 to i64
  %153 = load i64, ptr %144, align 8, !tbaa !56
  %154 = icmp sgt i64 %153, %152
  br i1 %154, label %155, label %165

155:                                              ; preds = %151
  %156 = load ptr, ptr %17, align 8, !tbaa !172
  %157 = getelementptr inbounds nuw i8, ptr %156, i64 %152
  %158 = load i8, ptr %157, align 1, !tbaa !76
  %159 = icmp eq i8 %158, 0
  br i1 %159, label %165, label %160

160:                                              ; preds = %155
  %161 = load ptr, ptr %18, align 8, !tbaa !167
  %162 = getelementptr inbounds nuw i32, ptr %161, i64 %152
  %163 = load i32, ptr %162, align 4, !tbaa !15
  %164 = tail call i32 @llvm.smin.i32(i32 %163, i32 %146)
  br label %165

165:                                              ; preds = %160, %155, %151, %145
  %166 = phi i32 [ %146, %145 ], [ %146, %155 ], [ %146, %151 ], [ %164, %160 ]
  %167 = add nsw i64 %147, 1
  %168 = icmp eq i64 %167, %141
  br i1 %168, label %224, label %145, !llvm.loop !210

169:                                              ; preds = %84
  %170 = getelementptr inbounds nuw i8, ptr %23, i64 168
  %171 = load ptr, ptr %170, align 8, !tbaa !67
  %172 = icmp eq ptr %171, null
  br i1 %172, label %233, label %173

173:                                              ; preds = %169
  %174 = getelementptr inbounds nuw i8, ptr %23, i64 176
  %175 = load ptr, ptr %174, align 8, !tbaa !68
  %176 = icmp eq ptr %175, null
  br i1 %176, label %233, label %177

177:                                              ; preds = %173
  %178 = getelementptr inbounds nuw i8, ptr %23, i64 184
  %179 = load i32, ptr %178, align 8, !tbaa !66
  %180 = icmp sgt i32 %179, 0
  br i1 %180, label %181, label %233

181:                                              ; preds = %177
  %182 = sdiv i32 %22, %179
  %183 = srem i32 %22, %179
  %184 = sext i32 %182 to i64
  %185 = getelementptr inbounds i32, ptr %171, i64 %184
  %186 = load i32, ptr %185, align 4, !tbaa !15
  %187 = getelementptr i8, ptr %185, i64 4
  %188 = load i32, ptr %187, align 4, !tbaa !15
  %189 = icmp slt i32 %186, %188
  br i1 %189, label %190, label %233

190:                                              ; preds = %181
  %191 = getelementptr inbounds nuw i8, ptr %23, i64 120
  %192 = sext i32 %186 to i64
  %193 = sext i32 %188 to i64
  br label %194

194:                                              ; preds = %220, %190
  %195 = phi i64 [ %192, %190 ], [ %222, %220 ]
  %196 = phi i32 [ 2147483647, %190 ], [ %221, %220 ]
  %197 = getelementptr inbounds i32, ptr %175, i64 %195
  %198 = load i32, ptr %197, align 4, !tbaa !15
  %199 = icmp eq i32 %198, %183
  br i1 %199, label %200, label %218

200:                                              ; preds = %194
  %201 = getelementptr i8, ptr %197, i64 4
  %202 = load i32, ptr %201, align 4, !tbaa !15
  %203 = icmp sgt i32 %202, -1
  br i1 %203, label %204, label %220

204:                                              ; preds = %200
  %205 = zext nneg i32 %202 to i64
  %206 = load i64, ptr %191, align 8, !tbaa !56
  %207 = icmp sgt i64 %206, %205
  br i1 %207, label %208, label %220

208:                                              ; preds = %204
  %209 = load ptr, ptr %17, align 8, !tbaa !172
  %210 = getelementptr inbounds nuw i8, ptr %209, i64 %205
  %211 = load i8, ptr %210, align 1, !tbaa !76
  %212 = icmp eq i8 %211, 0
  br i1 %212, label %220, label %213

213:                                              ; preds = %208
  %214 = load ptr, ptr %18, align 8, !tbaa !167
  %215 = getelementptr inbounds nuw i32, ptr %214, i64 %205
  %216 = load i32, ptr %215, align 4, !tbaa !15
  %217 = tail call i32 @llvm.smin.i32(i32 %216, i32 %196)
  br label %220

218:                                              ; preds = %194
  %219 = icmp sgt i32 %198, %183
  br i1 %219, label %224, label %220

220:                                              ; preds = %218, %213, %208, %204, %200
  %221 = phi i32 [ %196, %218 ], [ %217, %213 ], [ %196, %200 ], [ %196, %204 ], [ %196, %208 ]
  %222 = add nsw i64 %195, 2
  %223 = icmp slt i64 %222, %193
  br i1 %223, label %194, label %224, !llvm.loop !211

224:                                              ; preds = %80, %218, %220, %165, %123
  %225 = phi i32 [ %124, %123 ], [ %166, %165 ], [ %196, %218 ], [ %221, %220 ], [ %81, %80 ]
  %226 = icmp eq i32 %225, 2147483647
  br i1 %226, label %233, label %227

227:                                              ; preds = %224
  %228 = load ptr, ptr %18, align 8, !tbaa !167
  %229 = getelementptr inbounds i32, ptr %228, i64 %21
  %230 = load i32, ptr %229, align 4, !tbaa !15
  %231 = icmp slt i32 %225, %230
  br i1 %231, label %232, label %233

232:                                              ; preds = %227
  store i32 %225, ptr %229, align 4, !tbaa !15
  tail call fastcc void @autograph_motif_activate_once(ptr noundef nonnull %1, i32 noundef %22)
  br label %233

233:                                              ; preds = %38, %181, %135, %93, %85, %89, %127, %131, %169, %173, %177, %84, %28, %30, %34, %227, %232, %224
  %234 = add i64 %21, 1
  %235 = icmp eq i64 %234, %13
  br i1 %235, label %19, label %20, !llvm.loop !212
}

; Function Attrs: nounwind uwtable
define internal void @autograph_motif_write_min_push_body(i64 noundef %0, ptr noundef %1) #0 {
  %3 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %4 = load ptr, ptr %3, align 8, !tbaa !164
  %5 = getelementptr inbounds i32, ptr %4, i64 %0
  %6 = load i32, ptr %5, align 4, !tbaa !15
  %7 = getelementptr inbounds nuw i8, ptr %1, i64 60
  %8 = load i32, ptr %7, align 4, !tbaa !171
  %9 = icmp eq i32 %8, 0
  %10 = load ptr, ptr %1, align 8, !tbaa !162
  br i1 %9, label %89, label %11

11:                                               ; preds = %2
  %12 = getelementptr inbounds nuw i8, ptr %10, i64 8
  %13 = load i32, ptr %12, align 8, !tbaa !53
  %14 = icmp eq i32 %13, 0
  br i1 %14, label %15, label %89

15:                                               ; preds = %11
  %16 = icmp slt i32 %6, 0
  br i1 %16, label %90, label %17

17:                                               ; preds = %15
  %18 = zext nneg i32 %6 to i64
  %19 = getelementptr inbounds nuw i8, ptr %10, i64 120
  %20 = load i64, ptr %19, align 8, !tbaa !56
  %21 = icmp sgt i64 %20, %18
  br i1 %21, label %22, label %90

22:                                               ; preds = %17
  %23 = getelementptr inbounds nuw i8, ptr %10, i64 96
  %24 = load ptr, ptr %23, align 8, !tbaa !54
  %25 = icmp eq ptr %24, null
  br i1 %25, label %90, label %26

26:                                               ; preds = %22
  %27 = getelementptr inbounds nuw i8, ptr %10, i64 104
  %28 = load ptr, ptr %27, align 8, !tbaa !55
  %29 = icmp eq ptr %28, null
  br i1 %29, label %90, label %30

30:                                               ; preds = %26
  %31 = getelementptr inbounds nuw i64, ptr %24, i64 %18
  %32 = load i64, ptr %31, align 8, !tbaa !20
  %33 = getelementptr inbounds nuw i8, ptr %31, i64 8
  %34 = load i64, ptr %33, align 8, !tbaa !20
  %35 = icmp slt i64 %32, %34
  br i1 %35, label %36, label %90

36:                                               ; preds = %30
  %37 = getelementptr inbounds nuw i8, ptr %10, i64 112
  %38 = getelementptr inbounds nuw i8, ptr %1, i64 32
  br label %39

39:                                               ; preds = %82, %36
  %40 = phi i64 [ %32, %36 ], [ %83, %82 ]
  %41 = load i32, ptr %7, align 4, !tbaa !171
  %42 = icmp eq i32 %41, 0
  br i1 %42, label %49, label %43

43:                                               ; preds = %39
  %44 = load ptr, ptr %37, align 8, !tbaa !174
  %45 = icmp eq ptr %44, null
  br i1 %45, label %49, label %46

46:                                               ; preds = %43
  %47 = getelementptr inbounds i32, ptr %44, i64 %40
  %48 = load i32, ptr %47, align 4, !tbaa !15
  br label %49

49:                                               ; preds = %46, %43, %39
  %50 = phi i32 [ %48, %46 ], [ 1, %43 ], [ 1, %39 ]
  %51 = load ptr, ptr %27, align 8, !tbaa !55
  %52 = getelementptr inbounds i32, ptr %51, i64 %40
  %53 = load i32, ptr %52, align 4, !tbaa !15
  %54 = icmp slt i32 %53, 0
  br i1 %54, label %82, label %55

55:                                               ; preds = %49
  %56 = zext nneg i32 %53 to i64
  %57 = load ptr, ptr %1, align 8, !tbaa !162
  %58 = getelementptr inbounds nuw i8, ptr %57, i64 120
  %59 = load i64, ptr %58, align 8, !tbaa !56
  %60 = icmp sgt i64 %59, %56
  br i1 %60, label %61, label %82

61:                                               ; preds = %55
  %62 = load ptr, ptr %38, align 8, !tbaa !167
  %63 = getelementptr inbounds nuw i32, ptr %62, i64 %18
  %64 = load i32, ptr %63, align 4, !tbaa !15
  br i1 %42, label %70, label %65

65:                                               ; preds = %61
  %66 = sub nsw i32 2147483647, %50
  %67 = icmp sgt i32 %64, %66
  br i1 %67, label %82, label %68

68:                                               ; preds = %65
  %69 = add nsw i32 %64, %50
  br label %70

70:                                               ; preds = %68, %61
  %71 = phi i32 [ %69, %68 ], [ %64, %61 ]
  %72 = getelementptr inbounds nuw i32, ptr %62, i64 %56
  %73 = load atomic i32, ptr %72 monotonic, align 4
  br label %74

74:                                               ; preds = %77, %70
  %75 = phi i32 [ %73, %70 ], [ %80, %77 ]
  %76 = icmp slt i32 %71, %75
  br i1 %76, label %77, label %82

77:                                               ; preds = %74
  %78 = cmpxchg weak ptr %72, i32 %75, i32 %71 monotonic monotonic, align 4
  %79 = extractvalue { i32, i1 } %78, 1
  %80 = extractvalue { i32, i1 } %78, 0
  br i1 %79, label %81, label %74, !llvm.loop !213

81:                                               ; preds = %77
  tail call fastcc void @autograph_motif_activate_once(ptr noundef nonnull readonly %1, i32 noundef %53)
  br label %82

82:                                               ; preds = %74, %81, %65, %55, %49
  %83 = add nsw i64 %40, 1
  %84 = load ptr, ptr %23, align 8, !tbaa !54
  %85 = getelementptr inbounds nuw i64, ptr %84, i64 %18
  %86 = getelementptr inbounds nuw i8, ptr %85, i64 8
  %87 = load i64, ptr %86, align 8, !tbaa !20
  %88 = icmp slt i64 %83, %87
  br i1 %88, label %39, label %90, !llvm.loop !214

89:                                               ; preds = %11, %2
  tail call fastcc void @autograph_motif_foreach_neighbor(ptr noundef %10, i32 noundef %6, ptr noundef nonnull @autograph_motif_write_min_edge, ptr noundef nonnull %1)
  br label %90

90:                                               ; preds = %82, %30, %26, %22, %17, %15, %89
  ret void
}

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable
define internal void @autograph_motif_peel_mark_removed_body(i64 noundef %0, ptr nocapture noundef readonly %1) #24 {
  %3 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %4 = load ptr, ptr %3, align 8, !tbaa !164
  %5 = getelementptr inbounds i32, ptr %4, i64 %0
  %6 = load i32, ptr %5, align 4, !tbaa !15
  %7 = icmp slt i32 %6, 0
  br i1 %7, label %25, label %8

8:                                                ; preds = %2
  %9 = zext nneg i32 %6 to i64
  %10 = load ptr, ptr %1, align 8, !tbaa !162
  %11 = getelementptr inbounds nuw i8, ptr %10, i64 120
  %12 = load i64, ptr %11, align 8, !tbaa !56
  %13 = icmp sgt i64 %12, %9
  br i1 %13, label %14, label %25

14:                                               ; preds = %8
  %15 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %16 = load ptr, ptr %15, align 8, !tbaa !168
  %17 = getelementptr inbounds nuw i32, ptr %16, i64 %9
  %18 = cmpxchg ptr %17, i32 1, i32 0 monotonic monotonic, align 4
  %19 = extractvalue { i32, i1 } %18, 1
  br i1 %19, label %20, label %25

20:                                               ; preds = %14
  %21 = load ptr, ptr %1, align 8, !tbaa !162
  %22 = getelementptr inbounds nuw i8, ptr %21, i64 224
  %23 = load ptr, ptr %22, align 8, !tbaa !158
  %24 = getelementptr inbounds nuw i8, ptr %23, i64 %9
  store i8 1, ptr %24, align 1, !tbaa !76
  br label %25

25:                                               ; preds = %20, %14, %2, %8
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @autograph_motif_peel_pull_partition_body(i64 noundef %0, ptr nocapture noundef readonly %1) #0 {
  %3 = load ptr, ptr %1, align 8, !tbaa !162
  %4 = getelementptr inbounds nuw i8, ptr %3, i64 120
  %5 = load i64, ptr %4, align 8, !tbaa !56
  %6 = mul nsw i64 %5, %0
  %7 = getelementptr inbounds nuw i8, ptr %1, i64 24
  %8 = load i32, ptr %7, align 8, !tbaa !166
  %9 = sext i32 %8 to i64
  %10 = sdiv i64 %6, %9
  %11 = add nsw i64 %0, 1
  %12 = mul nsw i64 %5, %11
  %13 = sdiv i64 %12, %9
  %14 = icmp slt i64 %10, %13
  br i1 %14, label %15, label %20

15:                                               ; preds = %2
  %16 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %17 = getelementptr inbounds nuw i8, ptr %1, i64 64
  %18 = getelementptr inbounds nuw i8, ptr %1, i64 48
  %19 = getelementptr inbounds nuw i8, ptr %1, i64 56
  br label %21

20:                                               ; preds = %263, %2
  ret void

21:                                               ; preds = %15, %263
  %22 = phi i64 [ %10, %15 ], [ %264, %263 ]
  %23 = load ptr, ptr %16, align 8, !tbaa !168
  %24 = getelementptr inbounds i32, ptr %23, i64 %22
  %25 = load i32, ptr %24, align 4, !tbaa !15
  %26 = icmp eq i32 %25, 0
  br i1 %26, label %263, label %27

27:                                               ; preds = %21
  %28 = trunc i64 %22 to i32
  %29 = load ptr, ptr %1, align 8, !tbaa !162
  %30 = getelementptr inbounds nuw i8, ptr %29, i64 8
  %31 = load i32, ptr %30, align 8, !tbaa !53
  switch i32 %31, label %263 [
    i32 0, label %32
    i32 1, label %96
    i32 2, label %160
  ]

32:                                               ; preds = %27
  %33 = getelementptr inbounds nuw i8, ptr %29, i64 96
  %34 = load ptr, ptr %33, align 8, !tbaa !54
  %35 = icmp eq ptr %34, null
  br i1 %35, label %263, label %36

36:                                               ; preds = %32
  %37 = getelementptr inbounds nuw i8, ptr %29, i64 104
  %38 = load ptr, ptr %37, align 8, !tbaa !55
  %39 = icmp eq ptr %38, null
  br i1 %39, label %263, label %40

40:                                               ; preds = %36
  %41 = shl i64 %22, 32
  %42 = ashr exact i64 %41, 29
  %43 = getelementptr inbounds i8, ptr %34, i64 %42
  %44 = load i64, ptr %43, align 8, !tbaa !20
  %45 = getelementptr i8, ptr %43, i64 8
  %46 = load i64, ptr %45, align 8, !tbaa !20
  %47 = icmp slt i64 %44, %46
  br i1 %47, label %48, label %263

48:                                               ; preds = %40
  %49 = getelementptr inbounds nuw i8, ptr %29, i64 120
  %50 = sub i64 %46, %44
  %51 = add i64 %44, 1
  %52 = and i64 %50, 1
  %53 = icmp eq i64 %46, %51
  br i1 %53, label %212, label %54

54:                                               ; preds = %48
  %55 = and i64 %50, -2
  %56 = getelementptr i8, ptr %38, i64 4
  br label %57

57:                                               ; preds = %91, %54
  %58 = phi i32 [ 0, %54 ], [ %92, %91 ]
  %59 = phi i64 [ %44, %54 ], [ %93, %91 ]
  %60 = phi i64 [ 0, %54 ], [ %94, %91 ]
  %61 = getelementptr inbounds i32, ptr %38, i64 %59
  %62 = load i32, ptr %61, align 4, !tbaa !15
  %63 = icmp sgt i32 %62, -1
  br i1 %63, label %64, label %75

64:                                               ; preds = %57
  %65 = zext nneg i32 %62 to i64
  %66 = load i64, ptr %49, align 8, !tbaa !56
  %67 = icmp sgt i64 %66, %65
  br i1 %67, label %68, label %75

68:                                               ; preds = %64
  %69 = load ptr, ptr %17, align 8, !tbaa !172
  %70 = getelementptr inbounds nuw i8, ptr %69, i64 %65
  %71 = load i8, ptr %70, align 1, !tbaa !76
  %72 = icmp ne i8 %71, 0
  %73 = zext i1 %72 to i32
  %74 = add nsw i32 %58, %73
  br label %75

75:                                               ; preds = %68, %64, %57
  %76 = phi i32 [ %58, %64 ], [ %58, %57 ], [ %74, %68 ]
  %77 = getelementptr i32, ptr %56, i64 %59
  %78 = load i32, ptr %77, align 4, !tbaa !15
  %79 = icmp sgt i32 %78, -1
  br i1 %79, label %80, label %91

80:                                               ; preds = %75
  %81 = zext nneg i32 %78 to i64
  %82 = load i64, ptr %49, align 8, !tbaa !56
  %83 = icmp sgt i64 %82, %81
  br i1 %83, label %84, label %91

84:                                               ; preds = %80
  %85 = load ptr, ptr %17, align 8, !tbaa !172
  %86 = getelementptr inbounds nuw i8, ptr %85, i64 %81
  %87 = load i8, ptr %86, align 1, !tbaa !76
  %88 = icmp ne i8 %87, 0
  %89 = zext i1 %88 to i32
  %90 = add nsw i32 %76, %89
  br label %91

91:                                               ; preds = %84, %80, %75
  %92 = phi i32 [ %76, %80 ], [ %76, %75 ], [ %90, %84 ]
  %93 = add nsw i64 %59, 2
  %94 = add i64 %60, 2
  %95 = icmp eq i64 %94, %55
  br i1 %95, label %212, label %57, !llvm.loop !215

96:                                               ; preds = %27
  %97 = getelementptr inbounds nuw i8, ptr %29, i64 144
  %98 = load ptr, ptr %97, align 8, !tbaa !63
  %99 = icmp eq ptr %98, null
  br i1 %99, label %263, label %100

100:                                              ; preds = %96
  %101 = getelementptr inbounds nuw i8, ptr %29, i64 152
  %102 = load ptr, ptr %101, align 8, !tbaa !64
  %103 = icmp eq ptr %102, null
  br i1 %103, label %263, label %104

104:                                              ; preds = %100
  %105 = shl i64 %22, 32
  %106 = ashr exact i64 %105, 29
  %107 = getelementptr inbounds i8, ptr %98, i64 %106
  %108 = load i64, ptr %107, align 8, !tbaa !20
  %109 = getelementptr i8, ptr %107, i64 8
  %110 = load i64, ptr %109, align 8, !tbaa !20
  %111 = icmp slt i64 %108, %110
  br i1 %111, label %112, label %263

112:                                              ; preds = %104
  %113 = getelementptr inbounds nuw i8, ptr %29, i64 120
  %114 = sub i64 %110, %108
  %115 = add i64 %108, 1
  %116 = and i64 %114, 1
  %117 = icmp eq i64 %110, %115
  br i1 %117, label %232, label %118

118:                                              ; preds = %112
  %119 = and i64 %114, -2
  %120 = getelementptr i8, ptr %102, i64 4
  br label %121

121:                                              ; preds = %155, %118
  %122 = phi i32 [ 0, %118 ], [ %156, %155 ]
  %123 = phi i64 [ %108, %118 ], [ %157, %155 ]
  %124 = phi i64 [ 0, %118 ], [ %158, %155 ]
  %125 = getelementptr inbounds i32, ptr %102, i64 %123
  %126 = load i32, ptr %125, align 4, !tbaa !15
  %127 = icmp sgt i32 %126, -1
  br i1 %127, label %128, label %139

128:                                              ; preds = %121
  %129 = zext nneg i32 %126 to i64
  %130 = load i64, ptr %113, align 8, !tbaa !56
  %131 = icmp sgt i64 %130, %129
  br i1 %131, label %132, label %139

132:                                              ; preds = %128
  %133 = load ptr, ptr %17, align 8, !tbaa !172
  %134 = getelementptr inbounds nuw i8, ptr %133, i64 %129
  %135 = load i8, ptr %134, align 1, !tbaa !76
  %136 = icmp ne i8 %135, 0
  %137 = zext i1 %136 to i32
  %138 = add nsw i32 %122, %137
  br label %139

139:                                              ; preds = %132, %128, %121
  %140 = phi i32 [ %122, %121 ], [ %122, %128 ], [ %138, %132 ]
  %141 = getelementptr i32, ptr %120, i64 %123
  %142 = load i32, ptr %141, align 4, !tbaa !15
  %143 = icmp sgt i32 %142, -1
  br i1 %143, label %144, label %155

144:                                              ; preds = %139
  %145 = zext nneg i32 %142 to i64
  %146 = load i64, ptr %113, align 8, !tbaa !56
  %147 = icmp sgt i64 %146, %145
  br i1 %147, label %148, label %155

148:                                              ; preds = %144
  %149 = load ptr, ptr %17, align 8, !tbaa !172
  %150 = getelementptr inbounds nuw i8, ptr %149, i64 %145
  %151 = load i8, ptr %150, align 1, !tbaa !76
  %152 = icmp ne i8 %151, 0
  %153 = zext i1 %152 to i32
  %154 = add nsw i32 %140, %153
  br label %155

155:                                              ; preds = %148, %144, %139
  %156 = phi i32 [ %140, %139 ], [ %140, %144 ], [ %154, %148 ]
  %157 = add nsw i64 %123, 2
  %158 = add i64 %124, 2
  %159 = icmp eq i64 %158, %119
  br i1 %159, label %232, label %121, !llvm.loop !216

160:                                              ; preds = %27
  %161 = getelementptr inbounds nuw i8, ptr %29, i64 168
  %162 = load ptr, ptr %161, align 8, !tbaa !67
  %163 = icmp eq ptr %162, null
  br i1 %163, label %263, label %164

164:                                              ; preds = %160
  %165 = getelementptr inbounds nuw i8, ptr %29, i64 176
  %166 = load ptr, ptr %165, align 8, !tbaa !68
  %167 = icmp eq ptr %166, null
  br i1 %167, label %263, label %168

168:                                              ; preds = %164
  %169 = getelementptr inbounds nuw i8, ptr %29, i64 184
  %170 = load i32, ptr %169, align 8, !tbaa !66
  %171 = icmp sgt i32 %170, 0
  br i1 %171, label %172, label %263

172:                                              ; preds = %168
  %173 = sdiv i32 %28, %170
  %174 = srem i32 %28, %170
  %175 = sext i32 %173 to i64
  %176 = getelementptr inbounds i32, ptr %162, i64 %175
  %177 = load i32, ptr %176, align 4, !tbaa !15
  %178 = getelementptr i8, ptr %176, i64 4
  %179 = load i32, ptr %178, align 4, !tbaa !15
  %180 = icmp slt i32 %177, %179
  br i1 %180, label %181, label %263

181:                                              ; preds = %172
  %182 = getelementptr inbounds nuw i8, ptr %29, i64 120
  %183 = sext i32 %177 to i64
  %184 = sext i32 %179 to i64
  br label %185

185:                                              ; preds = %208, %181
  %186 = phi i64 [ %183, %181 ], [ %210, %208 ]
  %187 = phi i32 [ 0, %181 ], [ %209, %208 ]
  %188 = getelementptr inbounds i32, ptr %166, i64 %186
  %189 = load i32, ptr %188, align 4, !tbaa !15
  %190 = icmp eq i32 %189, %174
  br i1 %190, label %191, label %206

191:                                              ; preds = %185
  %192 = getelementptr i8, ptr %188, i64 4
  %193 = load i32, ptr %192, align 4, !tbaa !15
  %194 = icmp sgt i32 %193, -1
  br i1 %194, label %195, label %208

195:                                              ; preds = %191
  %196 = zext nneg i32 %193 to i64
  %197 = load i64, ptr %182, align 8, !tbaa !56
  %198 = icmp sgt i64 %197, %196
  br i1 %198, label %199, label %208

199:                                              ; preds = %195
  %200 = load ptr, ptr %17, align 8, !tbaa !172
  %201 = getelementptr inbounds nuw i8, ptr %200, i64 %196
  %202 = load i8, ptr %201, align 1, !tbaa !76
  %203 = icmp ne i8 %202, 0
  %204 = zext i1 %203 to i32
  %205 = add nsw i32 %187, %204
  br label %208

206:                                              ; preds = %185
  %207 = icmp sgt i32 %189, %174
  br i1 %207, label %252, label %208

208:                                              ; preds = %206, %199, %195, %191
  %209 = phi i32 [ %187, %206 ], [ %205, %199 ], [ %187, %191 ], [ %187, %195 ]
  %210 = add nsw i64 %186, 2
  %211 = icmp slt i64 %210, %184
  br i1 %211, label %185, label %252, !llvm.loop !217

212:                                              ; preds = %91, %48
  %213 = phi i32 [ poison, %48 ], [ %92, %91 ]
  %214 = phi i32 [ 0, %48 ], [ %92, %91 ]
  %215 = phi i64 [ %44, %48 ], [ %93, %91 ]
  %216 = icmp eq i64 %52, 0
  br i1 %216, label %252, label %217

217:                                              ; preds = %212
  %218 = getelementptr inbounds i32, ptr %38, i64 %215
  %219 = load i32, ptr %218, align 4, !tbaa !15
  %220 = icmp sgt i32 %219, -1
  br i1 %220, label %221, label %252

221:                                              ; preds = %217
  %222 = zext nneg i32 %219 to i64
  %223 = load i64, ptr %49, align 8, !tbaa !56
  %224 = icmp sgt i64 %223, %222
  br i1 %224, label %225, label %252

225:                                              ; preds = %221
  %226 = load ptr, ptr %17, align 8, !tbaa !172
  %227 = getelementptr inbounds nuw i8, ptr %226, i64 %222
  %228 = load i8, ptr %227, align 1, !tbaa !76
  %229 = icmp ne i8 %228, 0
  %230 = zext i1 %229 to i32
  %231 = add nsw i32 %214, %230
  br label %252

232:                                              ; preds = %155, %112
  %233 = phi i32 [ poison, %112 ], [ %156, %155 ]
  %234 = phi i32 [ 0, %112 ], [ %156, %155 ]
  %235 = phi i64 [ %108, %112 ], [ %157, %155 ]
  %236 = icmp eq i64 %116, 0
  br i1 %236, label %252, label %237

237:                                              ; preds = %232
  %238 = getelementptr inbounds i32, ptr %102, i64 %235
  %239 = load i32, ptr %238, align 4, !tbaa !15
  %240 = icmp sgt i32 %239, -1
  br i1 %240, label %241, label %252

241:                                              ; preds = %237
  %242 = zext nneg i32 %239 to i64
  %243 = load i64, ptr %113, align 8, !tbaa !56
  %244 = icmp sgt i64 %243, %242
  br i1 %244, label %245, label %252

245:                                              ; preds = %241
  %246 = load ptr, ptr %17, align 8, !tbaa !172
  %247 = getelementptr inbounds nuw i8, ptr %246, i64 %242
  %248 = load i8, ptr %247, align 1, !tbaa !76
  %249 = icmp ne i8 %248, 0
  %250 = zext i1 %249 to i32
  %251 = add nsw i32 %234, %250
  br label %252

252:                                              ; preds = %206, %208, %232, %245, %241, %237, %212, %225, %221, %217
  %253 = phi i32 [ %213, %212 ], [ %214, %221 ], [ %214, %217 ], [ %231, %225 ], [ %233, %232 ], [ %234, %237 ], [ %234, %241 ], [ %251, %245 ], [ %187, %206 ], [ %209, %208 ]
  %254 = icmp slt i32 %253, 1
  br i1 %254, label %263, label %255

255:                                              ; preds = %252
  %256 = load ptr, ptr %18, align 8, !tbaa !169
  %257 = getelementptr inbounds i32, ptr %256, i64 %22
  %258 = load i32, ptr %257, align 4, !tbaa !15
  %259 = sub nsw i32 %258, %253
  store i32 %259, ptr %257, align 4, !tbaa !15
  %260 = load i32, ptr %19, align 8, !tbaa !170
  %261 = icmp slt i32 %259, %260
  br i1 %261, label %262, label %263

262:                                              ; preds = %255
  tail call fastcc void @autograph_motif_activate_once(ptr noundef nonnull %1, i32 noundef %28)
  br label %263

263:                                              ; preds = %172, %104, %40, %32, %36, %96, %100, %160, %164, %168, %27, %252, %262, %255, %21
  %264 = add i64 %22, 1
  %265 = icmp eq i64 %264, %13
  br i1 %265, label %20, label %21, !llvm.loop !218
}

; Function Attrs: nounwind uwtable
define internal void @autograph_motif_peel_push_body(i64 noundef %0, ptr noundef %1) #0 {
  %3 = getelementptr inbounds nuw i8, ptr %1, i64 8
  %4 = load ptr, ptr %3, align 8, !tbaa !164
  %5 = getelementptr inbounds i32, ptr %4, i64 %0
  %6 = load i32, ptr %5, align 4, !tbaa !15
  %7 = icmp slt i32 %6, 0
  br i1 %7, label %22, label %8

8:                                                ; preds = %2
  %9 = zext nneg i32 %6 to i64
  %10 = load ptr, ptr %1, align 8, !tbaa !162
  %11 = getelementptr inbounds nuw i8, ptr %10, i64 120
  %12 = load i64, ptr %11, align 8, !tbaa !56
  %13 = icmp sgt i64 %12, %9
  br i1 %13, label %14, label %22

14:                                               ; preds = %8
  %15 = getelementptr inbounds nuw i8, ptr %1, i64 40
  %16 = load ptr, ptr %15, align 8, !tbaa !168
  %17 = getelementptr inbounds nuw i32, ptr %16, i64 %9
  %18 = cmpxchg ptr %17, i32 1, i32 0 monotonic monotonic, align 4
  %19 = extractvalue { i32, i1 } %18, 1
  br i1 %19, label %20, label %22

20:                                               ; preds = %14
  %21 = load ptr, ptr %1, align 8, !tbaa !162
  tail call fastcc void @autograph_motif_foreach_neighbor(ptr noundef %21, i32 noundef %6, ptr noundef nonnull @autograph_motif_peel_edge, ptr noundef nonnull %1)
  br label %22

22:                                               ; preds = %20, %14, %2, %8
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @autograph_motif_activate_once(ptr nocapture noundef readonly %0, i32 noundef %1) unnamed_addr #0 {
  %3 = icmp slt i32 %1, 0
  br i1 %3, label %61, label %4

4:                                                ; preds = %2
  %5 = zext nneg i32 %1 to i64
  %6 = load ptr, ptr %0, align 8, !tbaa !162
  %7 = getelementptr inbounds nuw i8, ptr %6, i64 120
  %8 = load i64, ptr %7, align 8, !tbaa !56
  %9 = icmp sgt i64 %8, %5
  br i1 %9, label %10, label %61

10:                                               ; preds = %4
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 72
  %12 = load ptr, ptr %11, align 8, !tbaa !173
  %13 = icmp eq ptr %12, null
  br i1 %13, label %18, label %14

14:                                               ; preds = %10
  %15 = getelementptr inbounds nuw i8, ptr %12, i64 %5
  %16 = cmpxchg ptr %15, i8 0, i8 1 monotonic monotonic, align 1
  %17 = extractvalue { i8, i1 } %16, 1
  br i1 %17, label %18, label %61

18:                                               ; preds = %14, %10
  %19 = tail call i32 @sgpl_current_worker_index() #28
  %20 = icmp slt i32 %19, 0
  br i1 %20, label %25, label %21

21:                                               ; preds = %18
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %23 = load i32, ptr %22, align 8, !tbaa !166
  %24 = icmp slt i32 %19, %23
  br i1 %24, label %26, label %25

25:                                               ; preds = %21, %18
  br label %26

26:                                               ; preds = %25, %21
  %27 = phi i32 [ 0, %25 ], [ %19, %21 ]
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %29 = load ptr, ptr %28, align 8, !tbaa !165
  %30 = zext nneg i32 %27 to i64
  %31 = getelementptr inbounds nuw %struct.AutoFrontierLane, ptr %29, i64 %30
  %32 = getelementptr inbounds nuw i8, ptr %31, i64 8
  %33 = load i32, ptr %32, align 8, !tbaa !144
  %34 = getelementptr inbounds nuw i8, ptr %31, i64 12
  %35 = load i32, ptr %34, align 4, !tbaa !188
  %36 = icmp eq i32 %33, %35
  br i1 %36, label %39, label %37

37:                                               ; preds = %26
  %38 = load ptr, ptr %31, align 8, !tbaa !189
  br label %55

39:                                               ; preds = %26
  %40 = icmp eq i32 %33, 0
  %41 = sext i32 %33 to i64
  %42 = shl nsw i64 %41, 1
  %43 = select i1 %40, i64 64, i64 %42
  %44 = icmp ugt i64 %43, 2147483647
  br i1 %44, label %45, label %46

45:                                               ; preds = %39
  tail call void @abort() #34
  unreachable

46:                                               ; preds = %39
  %47 = load ptr, ptr %31, align 8, !tbaa !189
  %48 = shl nuw nsw i64 %43, 2
  %49 = tail call ptr @realloc(ptr noundef %47, i64 noundef %48) #31
  %50 = icmp eq ptr %49, null
  br i1 %50, label %51, label %52

51:                                               ; preds = %46
  tail call void @abort() #34
  unreachable

52:                                               ; preds = %46
  %53 = trunc nuw nsw i64 %43 to i32
  store ptr %49, ptr %31, align 8, !tbaa !189
  store i32 %53, ptr %34, align 4, !tbaa !188
  %54 = load i32, ptr %32, align 8, !tbaa !144
  br label %55

55:                                               ; preds = %37, %52
  %56 = phi i32 [ %33, %37 ], [ %54, %52 ]
  %57 = phi ptr [ %38, %37 ], [ %49, %52 ]
  %58 = add nsw i32 %56, 1
  store i32 %58, ptr %32, align 8, !tbaa !144
  %59 = sext i32 %56 to i64
  %60 = getelementptr inbounds i32, ptr %57, i64 %59
  store i32 %1, ptr %60, align 4, !tbaa !15
  br label %61

61:                                               ; preds = %14, %2, %4, %55
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @autograph_motif_foreach_neighbor(ptr nocapture noundef readonly %0, i32 noundef %1, ptr nocapture noundef readonly %2, ptr noundef %3) unnamed_addr #0 {
  %5 = icmp slt i32 %1, 0
  br i1 %5, label %173, label %6

6:                                                ; preds = %4
  %7 = zext nneg i32 %1 to i64
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 120
  %9 = load i64, ptr %8, align 8, !tbaa !56
  %10 = icmp sgt i64 %9, %7
  br i1 %10, label %11, label %173

11:                                               ; preds = %6
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %13 = load i32, ptr %12, align 8, !tbaa !53
  switch i32 %13, label %113 [
    i32 0, label %14
    i32 1, label %39
    i32 2, label %69
  ]

14:                                               ; preds = %11
  %15 = getelementptr inbounds nuw i8, ptr %0, i64 96
  %16 = load ptr, ptr %15, align 8, !tbaa !54
  %17 = icmp eq ptr %16, null
  br i1 %17, label %173, label %18

18:                                               ; preds = %14
  %19 = getelementptr inbounds nuw i8, ptr %0, i64 104
  %20 = load ptr, ptr %19, align 8, !tbaa !55
  %21 = icmp eq ptr %20, null
  br i1 %21, label %173, label %22

22:                                               ; preds = %18
  %23 = getelementptr inbounds nuw i64, ptr %16, i64 %7
  %24 = load i64, ptr %23, align 8, !tbaa !20
  %25 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %26 = load i64, ptr %25, align 8, !tbaa !20
  %27 = icmp slt i64 %24, %26
  br i1 %27, label %28, label %173

28:                                               ; preds = %22, %28
  %29 = phi i64 [ %33, %28 ], [ %24, %22 ]
  %30 = load ptr, ptr %19, align 8, !tbaa !55
  %31 = getelementptr inbounds i32, ptr %30, i64 %29
  %32 = load i32, ptr %31, align 4, !tbaa !15
  tail call void %2(ptr noundef %3, i32 noundef %1, i32 noundef %32) #28, !callees !219
  %33 = add nsw i64 %29, 1
  %34 = load ptr, ptr %15, align 8, !tbaa !54
  %35 = getelementptr inbounds nuw i64, ptr %34, i64 %7
  %36 = getelementptr inbounds nuw i8, ptr %35, i64 8
  %37 = load i64, ptr %36, align 8, !tbaa !20
  %38 = icmp slt i64 %33, %37
  br i1 %38, label %28, label %173, !llvm.loop !220

39:                                               ; preds = %11
  %40 = getelementptr inbounds nuw i8, ptr %0, i64 144
  %41 = load ptr, ptr %40, align 8, !tbaa !63
  %42 = icmp eq ptr %41, null
  br i1 %42, label %173, label %43

43:                                               ; preds = %39
  %44 = getelementptr inbounds nuw i8, ptr %0, i64 152
  %45 = load ptr, ptr %44, align 8, !tbaa !64
  %46 = icmp eq ptr %45, null
  br i1 %46, label %173, label %47

47:                                               ; preds = %43
  %48 = getelementptr inbounds nuw i64, ptr %41, i64 %7
  %49 = load i64, ptr %48, align 8, !tbaa !20
  %50 = getelementptr inbounds nuw i8, ptr %48, i64 8
  %51 = load i64, ptr %50, align 8, !tbaa !20
  %52 = icmp slt i64 %49, %51
  br i1 %52, label %53, label %173

53:                                               ; preds = %47, %62
  %54 = phi ptr [ %63, %62 ], [ %41, %47 ]
  %55 = phi i64 [ %64, %62 ], [ %49, %47 ]
  %56 = load ptr, ptr %44, align 8, !tbaa !64
  %57 = getelementptr inbounds i32, ptr %56, i64 %55
  %58 = load i32, ptr %57, align 4, !tbaa !15
  %59 = icmp eq i32 %58, -1
  br i1 %59, label %62, label %60

60:                                               ; preds = %53
  tail call void %2(ptr noundef %3, i32 noundef %1, i32 noundef %58) #28, !callees !219
  %61 = load ptr, ptr %40, align 8, !tbaa !63
  br label %62

62:                                               ; preds = %60, %53
  %63 = phi ptr [ %61, %60 ], [ %54, %53 ]
  %64 = add nsw i64 %55, 1
  %65 = getelementptr inbounds nuw i64, ptr %63, i64 %7
  %66 = getelementptr inbounds nuw i8, ptr %65, i64 8
  %67 = load i64, ptr %66, align 8, !tbaa !20
  %68 = icmp slt i64 %64, %67
  br i1 %68, label %53, label %173, !llvm.loop !221

69:                                               ; preds = %11
  %70 = getelementptr inbounds nuw i8, ptr %0, i64 168
  %71 = load ptr, ptr %70, align 8, !tbaa !67
  %72 = icmp eq ptr %71, null
  br i1 %72, label %173, label %73

73:                                               ; preds = %69
  %74 = getelementptr inbounds nuw i8, ptr %0, i64 176
  %75 = load ptr, ptr %74, align 8, !tbaa !68
  %76 = icmp eq ptr %75, null
  br i1 %76, label %173, label %77

77:                                               ; preds = %73
  %78 = getelementptr inbounds nuw i8, ptr %0, i64 184
  %79 = load i32, ptr %78, align 8, !tbaa !66
  %80 = icmp sgt i32 %79, 0
  br i1 %80, label %81, label %173

81:                                               ; preds = %77
  %82 = udiv i32 %1, %79
  %83 = urem i32 %1, %79
  %84 = zext nneg i32 %82 to i64
  %85 = getelementptr inbounds nuw i32, ptr %71, i64 %84
  %86 = load i32, ptr %85, align 4, !tbaa !15
  %87 = getelementptr inbounds nuw i8, ptr %85, i64 4
  %88 = load i32, ptr %87, align 4, !tbaa !15
  %89 = icmp slt i32 %86, %88
  br i1 %89, label %90, label %173

90:                                               ; preds = %81
  %91 = sext i32 %86 to i64
  br label %92

92:                                               ; preds = %90, %105
  %93 = phi ptr [ %71, %90 ], [ %106, %105 ]
  %94 = phi i64 [ %91, %90 ], [ %107, %105 ]
  %95 = load ptr, ptr %74, align 8, !tbaa !68
  %96 = getelementptr inbounds i32, ptr %95, i64 %94
  %97 = load i32, ptr %96, align 4, !tbaa !15
  %98 = icmp eq i32 %97, %83
  br i1 %98, label %99, label %103

99:                                               ; preds = %92
  %100 = getelementptr i8, ptr %96, i64 4
  %101 = load i32, ptr %100, align 4, !tbaa !15
  tail call void %2(ptr noundef %3, i32 noundef %1, i32 noundef %101) #28, !callees !219
  %102 = load ptr, ptr %70, align 8, !tbaa !67
  br label %105

103:                                              ; preds = %92
  %104 = icmp sgt i32 %97, %83
  br i1 %104, label %173, label %105

105:                                              ; preds = %103, %99
  %106 = phi ptr [ %93, %103 ], [ %102, %99 ]
  %107 = add nsw i64 %94, 2
  %108 = getelementptr inbounds nuw i32, ptr %106, i64 %84
  %109 = getelementptr inbounds nuw i8, ptr %108, i64 4
  %110 = load i32, ptr %109, align 4, !tbaa !15
  %111 = sext i32 %110 to i64
  %112 = icmp slt i64 %107, %111
  br i1 %112, label %92, label %173, !llvm.loop !222

113:                                              ; preds = %11
  %114 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %115 = load ptr, ptr %114, align 8, !tbaa !60
  %116 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %117 = load ptr, ptr %116, align 8, !tbaa !61
  %118 = icmp ne ptr %115, null
  %119 = icmp ne ptr %117, null
  %120 = select i1 %118, i1 %119, i1 false
  br i1 %120, label %121, label %143

121:                                              ; preds = %113
  %122 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %123 = load i64, ptr %122, align 8, !tbaa !74
  %124 = icmp sgt i64 %123, 0
  br i1 %124, label %125, label %143

125:                                              ; preds = %121, %139
  %126 = phi i64 [ %140, %139 ], [ 0, %121 ]
  %127 = trunc i64 %126 to i32
  %128 = tail call zeroext i1 @roaring_bitmap_contains(ptr noundef nonnull %115, i32 noundef %127) #28
  br i1 %128, label %129, label %139

129:                                              ; preds = %125
  %130 = getelementptr inbounds nuw %struct.EdgePair, ptr %117, i64 %126
  %131 = load i32, ptr %130, align 4, !tbaa !21
  %132 = icmp eq i32 %131, %1
  %133 = getelementptr inbounds nuw i8, ptr %130, i64 4
  %134 = load i32, ptr %133, align 4, !tbaa !23
  br i1 %132, label %137, label %135

135:                                              ; preds = %129
  %136 = icmp eq i32 %134, %1
  br i1 %136, label %137, label %139

137:                                              ; preds = %135, %129
  %138 = phi i32 [ %134, %129 ], [ %131, %135 ]
  tail call void %2(ptr noundef %3, i32 noundef %1, i32 noundef %138) #28
  br label %139

139:                                              ; preds = %137, %135, %125
  %140 = add nuw nsw i64 %126, 1
  %141 = load i64, ptr %122, align 8, !tbaa !74
  %142 = icmp slt i64 %140, %141
  br i1 %142, label %125, label %143, !llvm.loop !223

143:                                              ; preds = %139, %121, %113
  %144 = getelementptr inbounds nuw i8, ptr %0, i64 64
  %145 = load i64, ptr %144, align 8, !tbaa !71
  %146 = icmp sgt i64 %145, 0
  br i1 %146, label %147, label %173

147:                                              ; preds = %143
  %148 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %149 = getelementptr inbounds nuw i8, ptr %0, i64 48
  br label %150

150:                                              ; preds = %147, %169
  %151 = phi i64 [ 0, %147 ], [ %170, %169 ]
  %152 = load ptr, ptr %148, align 8, !tbaa !75
  %153 = getelementptr inbounds nuw i8, ptr %152, i64 %151
  %154 = load i8, ptr %153, align 1, !tbaa !76
  %155 = icmp eq i8 %154, 0
  br i1 %155, label %169, label %156

156:                                              ; preds = %150
  %157 = load ptr, ptr %149, align 8, !tbaa !80
  %158 = shl nuw nsw i64 %151, 1
  %159 = getelementptr inbounds nuw i32, ptr %157, i64 %158
  %160 = load i32, ptr %159, align 4, !tbaa !15
  %161 = or disjoint i64 %158, 1
  %162 = getelementptr inbounds nuw i32, ptr %157, i64 %161
  %163 = load i32, ptr %162, align 4, !tbaa !15
  %164 = icmp eq i32 %160, %1
  br i1 %164, label %167, label %165

165:                                              ; preds = %156
  %166 = icmp eq i32 %163, %1
  br i1 %166, label %167, label %169

167:                                              ; preds = %165, %156
  %168 = phi i32 [ %163, %156 ], [ %160, %165 ]
  tail call void %2(ptr noundef %3, i32 noundef %1, i32 noundef %168) #28
  br label %169

169:                                              ; preds = %167, %165, %150
  %170 = add nuw nsw i64 %151, 1
  %171 = load i64, ptr %144, align 8, !tbaa !71
  %172 = icmp slt i64 %170, %171
  br i1 %172, label %150, label %173, !llvm.loop !224

173:                                              ; preds = %103, %105, %62, %28, %169, %81, %47, %22, %143, %69, %73, %77, %39, %43, %14, %18, %4, %6
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @autograph_motif_write_min_edge(ptr nocapture noundef readonly %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 %2, 0
  br i1 %4, label %36, label %5

5:                                                ; preds = %3
  %6 = zext nneg i32 %2 to i64
  %7 = load ptr, ptr %0, align 8, !tbaa !162
  %8 = getelementptr inbounds nuw i8, ptr %7, i64 120
  %9 = load i64, ptr %8, align 8, !tbaa !56
  %10 = icmp sgt i64 %9, %6
  br i1 %10, label %11, label %36

11:                                               ; preds = %5
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 32
  %13 = load ptr, ptr %12, align 8, !tbaa !167
  %14 = sext i32 %1 to i64
  %15 = getelementptr inbounds i32, ptr %13, i64 %14
  %16 = load i32, ptr %15, align 4, !tbaa !15
  %17 = getelementptr inbounds nuw i8, ptr %0, i64 60
  %18 = load i32, ptr %17, align 4, !tbaa !171
  %19 = icmp eq i32 %18, 0
  br i1 %19, label %24, label %20

20:                                               ; preds = %11
  %21 = icmp eq i32 %16, 2147483647
  br i1 %21, label %36, label %22

22:                                               ; preds = %20
  %23 = add nsw i32 %16, 1
  br label %24

24:                                               ; preds = %22, %11
  %25 = phi i32 [ %23, %22 ], [ %16, %11 ]
  %26 = getelementptr inbounds nuw i32, ptr %13, i64 %6
  %27 = load atomic i32, ptr %26 monotonic, align 4
  br label %28

28:                                               ; preds = %31, %24
  %29 = phi i32 [ %27, %24 ], [ %34, %31 ]
  %30 = icmp slt i32 %25, %29
  br i1 %30, label %31, label %36

31:                                               ; preds = %28
  %32 = cmpxchg weak ptr %26, i32 %29, i32 %25 monotonic monotonic, align 4
  %33 = extractvalue { i32, i1 } %32, 1
  %34 = extractvalue { i32, i1 } %32, 0
  br i1 %33, label %35, label %28, !llvm.loop !213

35:                                               ; preds = %31
  tail call fastcc void @autograph_motif_activate_once(ptr noundef nonnull readonly %0, i32 noundef %2)
  br label %36

36:                                               ; preds = %28, %3, %5, %20, %35
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @autograph_motif_peel_edge(ptr nocapture noundef readonly %0, i32 %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 %2, 0
  br i1 %4, label %26, label %5

5:                                                ; preds = %3
  %6 = zext nneg i32 %2 to i64
  %7 = load ptr, ptr %0, align 8, !tbaa !162
  %8 = getelementptr inbounds nuw i8, ptr %7, i64 120
  %9 = load i64, ptr %8, align 8, !tbaa !56
  %10 = icmp sgt i64 %9, %6
  br i1 %10, label %11, label %26

11:                                               ; preds = %5
  %12 = getelementptr inbounds nuw i8, ptr %0, i64 40
  %13 = load ptr, ptr %12, align 8, !tbaa !168
  %14 = getelementptr inbounds nuw i32, ptr %13, i64 %6
  %15 = load i32, ptr %14, align 4, !tbaa !15
  %16 = icmp eq i32 %15, 0
  br i1 %16, label %26, label %17

17:                                               ; preds = %11
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 48
  %19 = load ptr, ptr %18, align 8, !tbaa !169
  %20 = getelementptr inbounds nuw i32, ptr %19, i64 %6
  %21 = atomicrmw sub ptr %20, i32 1 monotonic, align 4
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 56
  %23 = load i32, ptr %22, align 8, !tbaa !170
  %24 = icmp sgt i32 %21, %23
  br i1 %24, label %26, label %25

25:                                               ; preds = %17
  tail call fastcc void @autograph_motif_activate_once(ptr noundef nonnull %0, i32 noundef %2)
  br label %26

26:                                               ; preds = %17, %25, %11, %3, %5
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #25

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #25

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #26

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #25

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #25

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.vector.reduce.add.v2i64(<2 x i64>) #25

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x i64> @llvm.smax.v2i64(<2 x i64>, <2 x i64>) #25

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: read)
declare ptr @llvm.load.relative.i64(ptr, i64) #27

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { mustprogress nounwind willreturn allockind("free") memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #12 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #13 = { nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #14 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #15 = { nofree nounwind memory(readwrite, argmem: read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #16 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #17 = { nofree norecurse nosync nounwind memory(readwrite, argmem: read, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #18 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #19 = { nofree nounwind memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #20 = { nofree nounwind memory(read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #21 = { cold nofree noreturn nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #22 = { mustprogress nofree nounwind willreturn memory(argmem: read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #23 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #24 = { mustprogress nofree norecurse nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #25 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #26 = { nofree nounwind }
attributes #27 = { nocallback nofree nosync nounwind willreturn memory(argmem: read) }
attributes #28 = { nounwind }
attributes #29 = { nounwind allocsize(0,1) }
attributes #30 = { nounwind allocsize(0) }
attributes #31 = { nounwind allocsize(1) }
attributes #32 = { cold }
attributes #33 = { cold nounwind }
attributes #34 = { noreturn nounwind }
attributes #35 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 20.1.8 (https://github.com/llvm/llvm-project.git 87f0227cb60147a26a1eeb4fb06e3b505e9c7261)"}
!5 = !{!6, !9, i64 4}
!6 = !{!"", !7, i64 0, !9, i64 4, !9, i64 8, !10, i64 16, !7, i64 24, !7, i64 32}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!"int", !7, i64 0}
!10 = !{!"double", !7, i64 0}
!11 = !{!6, !9, i64 8}
!12 = !{!6, !10, i64 16}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.mustprogress"}
!15 = !{!9, !9, i64 0}
!16 = !{!17, !18, i64 0}
!17 = !{!"timespec", !18, i64 0, !18, i64 8}
!18 = !{!"long", !7, i64 0}
!19 = !{!17, !18, i64 8}
!20 = !{!18, !18, i64 0}
!21 = !{!22, !9, i64 0}
!22 = !{!"", !9, i64 0, !9, i64 4}
!23 = !{!22, !9, i64 4}
!24 = distinct !{!24, !14}
!25 = distinct !{!25, !14}
!26 = distinct !{!26, !27}
!27 = !{!"llvm.loop.unroll.disable"}
!28 = distinct !{!28, !14}
!29 = !{!30, !30, i64 0}
!30 = !{!"p1 long", !31, i64 0}
!31 = !{!"any pointer", !7, i64 0}
!32 = !{!33, !33, i64 0}
!33 = !{!"p1 int", !31, i64 0}
!34 = distinct !{!34, !14}
!35 = distinct !{!35, !14}
!36 = !{!37, !7, i64 16}
!37 = !{!"EdgeHashEntry_s", !18, i64 0, !18, i64 8, !7, i64 16}
!38 = !{!37, !18, i64 0}
!39 = !{!37, !18, i64 8}
!40 = distinct !{!40, !14}
!41 = distinct !{!41, !14}
!42 = distinct !{!42, !14}
!43 = distinct !{!43, !14}
!44 = distinct !{!44, !14}
!45 = !{!46, !18, i64 8}
!46 = !{!"EdgeHashMap_s", !47, i64 0, !18, i64 8}
!47 = !{!"p1 _ZTS15EdgeHashEntry_s", !31, i64 0}
!48 = !{!46, !47, i64 0}
!49 = distinct !{!49, !14}
!50 = !{!51, !31, i64 0}
!51 = !{!"", !31, i64 0, !9, i64 8, !31, i64 16, !31, i64 24, !31, i64 32, !18, i64 40, !33, i64 48, !52, i64 56, !18, i64 64, !18, i64 72, !9, i64 80, !18, i64 88, !30, i64 96, !33, i64 104, !33, i64 112, !18, i64 120, !18, i64 128, !9, i64 136, !30, i64 144, !33, i64 152, !18, i64 160, !33, i64 168, !33, i64 176, !9, i64 184, !9, i64 188, !9, i64 192, !31, i64 200, !9, i64 208, !18, i64 216, !52, i64 224, !52, i64 232, !30, i64 240, !9, i64 248, !7, i64 256, !7, i64 352, !7, i64 360, !7, i64 456}
!52 = !{!"p1 omnipotent char", !31, i64 0}
!53 = !{!51, !9, i64 8}
!54 = !{!51, !30, i64 96}
!55 = !{!51, !33, i64 104}
!56 = !{!51, !18, i64 120}
!57 = !{!51, !18, i64 128}
!58 = !{!51, !9, i64 80}
!59 = !{!51, !31, i64 16}
!60 = !{!51, !31, i64 24}
!61 = !{!51, !31, i64 32}
!62 = !{!51, !9, i64 136}
!63 = !{!51, !30, i64 144}
!64 = !{!51, !33, i64 152}
!65 = !{!51, !9, i64 188}
!66 = !{!51, !9, i64 184}
!67 = !{!51, !33, i64 168}
!68 = !{!51, !33, i64 176}
!69 = !{!51, !18, i64 88}
!70 = !{!51, !18, i64 72}
!71 = !{!51, !18, i64 64}
!72 = !{!73, !73, i64 0}
!73 = !{!"p1 _ZTS13EdgeHashMap_s", !31, i64 0}
!74 = !{!51, !18, i64 40}
!75 = !{!51, !52, i64 56}
!76 = !{!7, !7, i64 0}
!77 = distinct !{!77, !14}
!78 = distinct !{!78, !14}
!79 = distinct !{!79, !14}
!80 = !{!51, !33, i64 48}
!81 = distinct !{!81, !14}
!82 = !{!51, !9, i64 192}
!83 = distinct !{!83, !14, !84, !85}
!84 = !{!"llvm.loop.isvectorized", i32 1}
!85 = !{!"llvm.loop.unroll.runtime.disable"}
!86 = distinct !{!86, !14}
!87 = distinct !{!87, !14, !85, !84}
!88 = distinct !{!88, !27}
!89 = distinct !{!89, !14}
!90 = distinct !{!90, !14}
!91 = distinct !{!91, !14}
!92 = distinct !{!92, !14}
!93 = distinct !{!93, !14}
!94 = distinct !{!94, !27}
!95 = distinct !{!95, !14}
!96 = distinct !{!96, !14}
!97 = distinct !{!97, !14}
!98 = distinct !{!98, !14, !84, !85}
!99 = distinct !{!99, !14}
!100 = distinct !{!100, !14, !85, !84}
!101 = distinct !{!101, !14}
!102 = distinct !{!102, !14}
!103 = distinct !{!103, !14, !84, !85}
!104 = distinct !{!104, !14, !84}
!105 = distinct !{!105, !14, !84, !85}
!106 = distinct !{!106, !14, !85, !84}
!107 = distinct !{!107, !14}
!108 = distinct !{!108, !14}
!109 = distinct !{!109, !14}
!110 = distinct !{!110, !14, !84, !85}
!111 = distinct !{!111, !14, !85, !84}
!112 = !{!31, !31, i64 0}
!113 = distinct !{!113, !14}
!114 = distinct !{!114, !14}
!115 = distinct !{!115, !27}
!116 = distinct !{!116, !14}
!117 = distinct !{!117, !14}
!118 = distinct !{!118, !14}
!119 = distinct !{!119, !14}
!120 = distinct !{!120, !14}
!121 = distinct !{!121, !14}
!122 = distinct !{!122, !14}
!123 = distinct !{!123, !14}
!124 = distinct !{!124, !14}
!125 = distinct !{!125, !14}
!126 = distinct !{!126, !14}
!127 = distinct !{!127, !14, !84, !85}
!128 = distinct !{!128, !14, !85, !84}
!129 = distinct !{!129, !14}
!130 = distinct !{!130, !14, !84, !85}
!131 = distinct !{!131, !14, !85, !84}
!132 = !{!133, !31, i64 0}
!133 = !{!"", !31, i64 0, !18, i64 8, !18, i64 16, !18, i64 24, !9, i64 32, !9, i64 36, !9, i64 40, !9, i64 44}
!134 = !{!133, !18, i64 8}
!135 = !{!133, !9, i64 36}
!136 = !{!133, !9, i64 40}
!137 = !{!133, !18, i64 16}
!138 = !{!133, !18, i64 24}
!139 = !{!133, !9, i64 32}
!140 = distinct !{!140, !14}
!141 = distinct !{!141, !14}
!142 = distinct !{!142, !14}
!143 = !{!51, !31, i64 200}
!144 = !{!145, !9, i64 8}
!145 = !{!"", !33, i64 0, !9, i64 8, !9, i64 12}
!146 = distinct !{!146, !14}
!147 = distinct !{!147, !27}
!148 = !{!149, !31, i64 0}
!149 = !{!"", !31, i64 0, !33, i64 8, !31, i64 16, !9, i64 24, !33, i64 32, !9, i64 40, !9, i64 44, !33, i64 48, !52, i64 56}
!150 = !{!149, !33, i64 8}
!151 = !{!149, !31, i64 16}
!152 = !{!149, !9, i64 24}
!153 = !{!149, !33, i64 32}
!154 = !{!149, !9, i64 40}
!155 = !{!149, !9, i64 44}
!156 = !{!149, !33, i64 48}
!157 = !{!149, !52, i64 56}
!158 = !{!51, !52, i64 224}
!159 = distinct !{!159, !14}
!160 = distinct !{!160, !27}
!161 = !{!51, !52, i64 232}
!162 = !{!163, !31, i64 0}
!163 = !{!"", !31, i64 0, !33, i64 8, !31, i64 16, !9, i64 24, !33, i64 32, !33, i64 40, !33, i64 48, !9, i64 56, !9, i64 60, !52, i64 64, !52, i64 72}
!164 = !{!163, !33, i64 8}
!165 = !{!163, !31, i64 16}
!166 = !{!163, !9, i64 24}
!167 = !{!163, !33, i64 32}
!168 = !{!163, !33, i64 40}
!169 = !{!163, !33, i64 48}
!170 = !{!163, !9, i64 56}
!171 = !{!163, !9, i64 60}
!172 = !{!163, !52, i64 64}
!173 = !{!163, !52, i64 72}
!174 = !{!51, !33, i64 112}
!175 = !{!51, !7, i64 352}
!176 = !{!51, !7, i64 456}
!177 = !{!10, !10, i64 0}
!178 = !{!179, !179, i64 0}
!179 = !{!"p1 _ZTS8_IO_FILE", !31, i64 0}
!180 = !{!6, !7, i64 0}
!181 = !{!6, !7, i64 24}
!182 = !{!6, !7, i64 32}
!183 = distinct !{!183, !14}
!184 = !{!51, !18, i64 216}
!185 = !{!51, !9, i64 208}
!186 = !{!51, !9, i64 248}
!187 = !{!51, !30, i64 240}
!188 = !{!145, !9, i64 12}
!189 = !{!145, !33, i64 0}
!190 = distinct !{!190, !14}
!191 = distinct !{!191, !14}
!192 = distinct !{!192, !14}
!193 = distinct !{!193, !14}
!194 = distinct !{!194, !14}
!195 = distinct !{!195, !14}
!196 = distinct !{!196, !14}
!197 = distinct !{!197, !14}
!198 = distinct !{!198, !14}
!199 = distinct !{!199, !14}
!200 = distinct !{!200, !14}
!201 = distinct !{!201, !14}
!202 = !{!203, !31, i64 0}
!203 = !{!"", !31, i64 0, !30, i64 8, !33, i64 16}
!204 = !{!203, !30, i64 8}
!205 = !{!203, !33, i64 16}
!206 = distinct !{!206, !14}
!207 = distinct !{!207, !14}
!208 = distinct !{!208, !14}
!209 = distinct !{!209, !14}
!210 = distinct !{!210, !14}
!211 = distinct !{!211, !14}
!212 = distinct !{!212, !14}
!213 = distinct !{!213, !14}
!214 = distinct !{!214, !14}
!215 = distinct !{!215, !14}
!216 = distinct !{!216, !14}
!217 = distinct !{!217, !14}
!218 = distinct !{!218, !14}
!219 = !{ptr @autograph_motif_peel_edge, ptr @autograph_motif_write_min_edge}
!220 = distinct !{!220, !14}
!221 = distinct !{!221, !14}
!222 = distinct !{!222, !14}
!223 = distinct !{!223, !14}
!224 = distinct !{!224, !14}
