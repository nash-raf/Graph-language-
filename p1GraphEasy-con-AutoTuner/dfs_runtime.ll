; ModuleID = 'dfs_runtime.cpp'
source_filename = "dfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

@.str = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.2 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @dfs_runtime(ptr noundef %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca i64, align 8
  %3 = icmp eq ptr %0, null
  br i1 %3, label %130, label %4

4:                                                ; preds = %1
  %5 = load i64, ptr %0, align 8, !tbaa !3
  %6 = icmp slt i64 %5, 1
  br i1 %6, label %130, label %7

7:                                                ; preds = %4
  %8 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %5) #10
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %8, i8 0, i64 %5, i1 false)
  %9 = icmp samesign ugt i64 %5, 2305843009213693951
  br i1 %9, label %10, label %12

10:                                               ; preds = %7
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #11
          to label %11 unwind label %25

11:                                               ; preds = %10
  unreachable

12:                                               ; preds = %7
  %13 = shl nuw nsw i64 %5, 2
  %14 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #10
          to label %15 unwind label %25

15:                                               ; preds = %12
  store i32 0, ptr %14, align 4, !tbaa !11
  %16 = icmp eq i64 %5, 1
  br i1 %16, label %22, label %17

17:                                               ; preds = %15
  %18 = getelementptr i8, ptr %14, i64 4
  %19 = add nsw i64 %13, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %18, i8 0, i64 %19, i1 false), !tbaa !11
  %20 = trunc i64 %5 to i32
  %21 = icmp sgt i32 %20, 0
  br i1 %21, label %22, label %24

22:                                               ; preds = %15, %17
  %23 = and i64 %5, 2147483647
  br label %27

24:                                               ; preds = %117, %17
  call void @_ZdlPvm(ptr noundef nonnull %14, i64 noundef %13) #12
  call void @_ZdlPvm(ptr noundef nonnull %8, i64 noundef %5) #12
  br label %130

25:                                               ; preds = %12, %10
  %26 = landingpad { ptr, i32 }
          cleanup
  br label %131

27:                                               ; preds = %22, %117
  %28 = phi i64 [ 0, %22 ], [ %118, %117 ]
  %29 = getelementptr inbounds nuw i8, ptr %8, i64 %28
  %30 = load i8, ptr %29, align 1, !tbaa !13
  %31 = icmp eq i8 %30, 0
  br i1 %31, label %32, label %117

32:                                               ; preds = %27
  %33 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #10
          to label %34 unwind label %48

34:                                               ; preds = %32
  %35 = getelementptr inbounds nuw i32, ptr %33, i64 %5
  %36 = trunc nuw nsw i64 %28 to i32
  store i32 %36, ptr %33, align 4, !tbaa !11
  %37 = getelementptr inbounds nuw i8, ptr %33, i64 4
  br label %38

38:                                               ; preds = %34, %108
  %39 = phi ptr [ %35, %34 ], [ %111, %108 ]
  %40 = phi ptr [ %37, %34 ], [ %110, %108 ]
  %41 = phi ptr [ %33, %34 ], [ %109, %108 ]
  %42 = getelementptr inbounds i8, ptr %40, i64 -4
  %43 = load i32, ptr %42, align 4, !tbaa !11
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds nuw i8, ptr %8, i64 %44
  %46 = load i8, ptr %45, align 1, !tbaa !13
  %47 = icmp eq i8 %46, 0
  br i1 %47, label %50, label %108, !llvm.loop !14

48:                                               ; preds = %32
  %49 = landingpad { ptr, i32 }
          cleanup
  br label %128

50:                                               ; preds = %38
  store i8 1, ptr %45, align 1, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #13
  store i64 0, ptr %2, align 8, !tbaa !16
  invoke void @autograph_get_neighbors(ptr noundef nonnull %0, i64 noundef %44, ptr noundef nonnull %14, ptr noundef nonnull %2)
          to label %51 unwind label %58

51:                                               ; preds = %50
  %52 = load i64, ptr %2, align 8, !tbaa !16
  %53 = icmp sgt i64 %52, 0
  br i1 %53, label %61, label %54

54:                                               ; preds = %103, %51
  %55 = phi ptr [ %41, %51 ], [ %104, %103 ]
  %56 = phi ptr [ %42, %51 ], [ %105, %103 ]
  %57 = phi ptr [ %39, %51 ], [ %106, %103 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #13
  br label %108

58:                                               ; preds = %50
  %59 = landingpad { ptr, i32 }
          cleanup
  %60 = ptrtoint ptr %39 to i64
  br label %120

61:                                               ; preds = %51, %103
  %62 = phi i64 [ %66, %103 ], [ %52, %51 ]
  %63 = phi ptr [ %106, %103 ], [ %39, %51 ]
  %64 = phi ptr [ %105, %103 ], [ %42, %51 ]
  %65 = phi ptr [ %104, %103 ], [ %41, %51 ]
  %66 = add nsw i64 %62, -1
  %67 = getelementptr inbounds nuw i32, ptr %14, i64 %66
  %68 = load i32, ptr %67, align 4, !tbaa !11
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds nuw i8, ptr %8, i64 %69
  %71 = load i8, ptr %70, align 1, !tbaa !13
  %72 = icmp eq i8 %71, 0
  br i1 %72, label %73, label %103

73:                                               ; preds = %61
  %74 = icmp eq ptr %64, %63
  br i1 %74, label %77, label %75

75:                                               ; preds = %73
  store i32 %68, ptr %64, align 4, !tbaa !11
  %76 = getelementptr inbounds nuw i8, ptr %64, i64 4
  br label %103

77:                                               ; preds = %73
  %78 = ptrtoint ptr %63 to i64
  %79 = ptrtoint ptr %65 to i64
  %80 = sub i64 %78, %79
  %81 = ashr exact i64 %80, 2
  %82 = icmp sgt i64 %81, -1
  call void @llvm.assume(i1 %82)
  %83 = icmp eq i64 %80, 9223372036854775804
  br i1 %83, label %84, label %86

84:                                               ; preds = %77
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #11
          to label %85 unwind label %101

85:                                               ; preds = %84
  unreachable

86:                                               ; preds = %77
  %87 = call i64 @llvm.umax.i64(i64 %81, i64 1)
  %88 = add nuw nsw i64 %87, %81
  %89 = call noundef i64 @llvm.umin.i64(i64 %88, i64 2305843009213693951)
  %90 = shl nuw nsw i64 %89, 2
  %91 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %90) #10
          to label %92 unwind label %99

92:                                               ; preds = %86
  %93 = getelementptr inbounds i8, ptr %91, i64 %80
  store i32 %68, ptr %93, align 4, !tbaa !11
  %94 = icmp sgt i64 %80, 0
  br i1 %94, label %95, label %96

95:                                               ; preds = %92
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %91, ptr align 4 %65, i64 %80, i1 false)
  br label %96

96:                                               ; preds = %95, %92
  call void @_ZdlPvm(ptr noundef nonnull %65, i64 noundef %80) #12
  %97 = getelementptr inbounds nuw i8, ptr %93, i64 4
  %98 = getelementptr inbounds nuw i32, ptr %91, i64 %89
  br label %103

99:                                               ; preds = %86
  %100 = landingpad { ptr, i32 }
          cleanup
  br label %120

101:                                              ; preds = %84
  %102 = landingpad { ptr, i32 }
          cleanup
  br label %120

103:                                              ; preds = %96, %75, %61
  %104 = phi ptr [ %65, %61 ], [ %91, %96 ], [ %65, %75 ]
  %105 = phi ptr [ %64, %61 ], [ %97, %96 ], [ %76, %75 ]
  %106 = phi ptr [ %63, %61 ], [ %98, %96 ], [ %63, %75 ]
  %107 = icmp samesign ugt i64 %62, 1
  br i1 %107, label %61, label %54, !llvm.loop !17

108:                                              ; preds = %38, %54
  %109 = phi ptr [ %55, %54 ], [ %41, %38 ]
  %110 = phi ptr [ %56, %54 ], [ %42, %38 ]
  %111 = phi ptr [ %57, %54 ], [ %39, %38 ]
  %112 = icmp eq ptr %109, %110
  br i1 %112, label %113, label %38

113:                                              ; preds = %108
  %114 = ptrtoint ptr %111 to i64
  %115 = ptrtoint ptr %109 to i64
  %116 = sub i64 %114, %115
  call void @_ZdlPvm(ptr noundef nonnull %109, i64 noundef %116) #12
  br label %117

117:                                              ; preds = %113, %27
  %118 = add nuw nsw i64 %28, 1
  %119 = icmp eq i64 %118, %23
  br i1 %119, label %24, label %27, !llvm.loop !18

120:                                              ; preds = %58, %101, %99
  %121 = phi i64 [ %78, %99 ], [ %78, %101 ], [ %60, %58 ]
  %122 = phi ptr [ %65, %99 ], [ %65, %101 ], [ %41, %58 ]
  %123 = phi { ptr, i32 } [ %100, %99 ], [ %102, %101 ], [ %59, %58 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #13
  %124 = icmp eq ptr %122, null
  br i1 %124, label %128, label %125

125:                                              ; preds = %120
  %126 = ptrtoint ptr %122 to i64
  %127 = sub i64 %121, %126
  call void @_ZdlPvm(ptr noundef nonnull %122, i64 noundef %127) #12
  br label %128

128:                                              ; preds = %48, %120, %125
  %129 = phi { ptr, i32 } [ %49, %48 ], [ %123, %120 ], [ %123, %125 ]
  call void @_ZdlPvm(ptr noundef nonnull %14, i64 noundef %13) #12
  br label %131

130:                                              ; preds = %24, %4, %1
  ret void

131:                                              ; preds = %128, %25
  %132 = phi { ptr, i32 } [ %129, %128 ], [ %26, %25 ]
  call void @_ZdlPvm(ptr noundef nonnull %8, i64 noundef %5) #12
  resume { ptr, i32 } %132
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local void @autograph_get_neighbors(ptr noundef, i64 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #3

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPvm(ptr noundef, i64 noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #9

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #6 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #9 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #10 = { builtin allocsize(0) }
attributes #11 = { cold noreturn }
attributes #12 = { builtin nounwind }
attributes #13 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"_ZTS5Graph", !5, i64 0, !5, i64 8, !8, i64 16, !10, i64 24}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"p1 long", !9, i64 0}
!9 = !{!"any pointer", !6, i64 0}
!10 = !{!"p1 int", !9, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !6, i64 0}
!13 = !{!6, !6, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.mustprogress"}
!16 = !{!5, !5, i64 0}
!17 = distinct !{!17, !15}
!18 = distinct !{!18, !15}
