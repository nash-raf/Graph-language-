; ModuleID = 'dfs_runtime.bc'
source_filename = "dfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.Graph = type { i64, i64, ptr, ptr }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.3 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @dfs_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = icmp eq ptr %0, null
  br i1 %2, label %141, label %3

3:                                                ; preds = %1
  %4 = load i64, ptr %0, align 8, !tbaa !3
  %5 = freeze i64 %4
  %6 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %7 = load ptr, ptr %6, align 8, !tbaa !9
  %8 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %9 = load ptr, ptr %8, align 8, !tbaa !10
  %10 = icmp slt i64 %5, 0
  br i1 %10, label %11, label %12

11:                                               ; preds = %3
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #9
  unreachable

12:                                               ; preds = %3
  %13 = icmp eq i64 %5, 0
  br i1 %13, label %141, label %14

14:                                               ; preds = %12
  %15 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %5) #10
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %15, i8 0, i64 %5, i1 false)
  %16 = trunc i64 %5 to i32
  %17 = icmp sgt i32 %16, 0
  br i1 %17, label %18, label %30

18:                                               ; preds = %14
  %19 = icmp ugt i64 %5, 2305843009213693951
  %20 = shl nuw nsw i64 %5, 2
  %21 = and i64 %5, 4294967295
  br i1 %19, label %22, label %31

22:                                               ; preds = %18, %27
  %23 = phi i64 [ %28, %27 ], [ 0, %18 ]
  %24 = getelementptr inbounds i8, ptr %15, i64 %23
  %25 = load i8, ptr %24, align 1, !tbaa !11
  %26 = icmp eq i8 %25, 0
  br i1 %26, label %37, label %27

27:                                               ; preds = %22
  %28 = add nuw nsw i64 %23, 1
  %29 = icmp eq i64 %28, %21
  br i1 %29, label %30, label %22, !llvm.loop !12

30:                                               ; preds = %132, %27, %14
  tail call void @_ZdlPv(ptr noundef nonnull %15) #11
  br label %141

31:                                               ; preds = %18, %132
  %32 = phi i64 [ %133, %132 ], [ 0, %18 ]
  %33 = getelementptr inbounds i8, ptr %15, i64 %32
  %34 = load i8, ptr %33, align 1, !tbaa !11
  %35 = icmp eq i8 %34, 0
  br i1 %35, label %36, label %132

36:                                               ; preds = %31
  br i1 %13, label %44, label %39

37:                                               ; preds = %22
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #9
          to label %38 unwind label %65

38:                                               ; preds = %37
  unreachable

39:                                               ; preds = %36
  %40 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %20) #10
          to label %41 unwind label %63

41:                                               ; preds = %39
  %42 = getelementptr inbounds i32, ptr %40, i64 %5
  %43 = trunc i64 %32 to i32
  store i32 %43, ptr %40, align 4, !tbaa !14
  br label %49

44:                                               ; preds = %36
  %45 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #10
          to label %46 unwind label %63

46:                                               ; preds = %44
  %47 = trunc i64 %32 to i32
  store i32 %47, ptr %45, align 4, !tbaa !14
  %48 = getelementptr inbounds i32, ptr %45, i64 1
  br label %49

49:                                               ; preds = %41, %46
  %50 = phi ptr [ %45, %46 ], [ %40, %41 ]
  %51 = phi ptr [ %48, %46 ], [ %42, %41 ]
  %52 = getelementptr inbounds i32, ptr %50, i64 1
  br label %53

53:                                               ; preds = %49, %124
  %54 = phi ptr [ %51, %49 ], [ %127, %124 ]
  %55 = phi ptr [ %52, %49 ], [ %126, %124 ]
  %56 = phi ptr [ %50, %49 ], [ %125, %124 ]
  %57 = getelementptr inbounds i32, ptr %55, i64 -1
  %58 = load i32, ptr %57, align 4, !tbaa !14
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds i8, ptr %15, i64 %59
  %61 = load i8, ptr %60, align 1, !tbaa !11
  %62 = icmp eq i8 %61, 0
  br i1 %62, label %67, label %124, !llvm.loop !16

63:                                               ; preds = %39, %44
  %64 = landingpad { ptr, i32 }
          cleanup
  br label %139

65:                                               ; preds = %37
  %66 = landingpad { ptr, i32 }
          cleanup
  br label %139

67:                                               ; preds = %53
  store i8 1, ptr %60, align 1, !tbaa !11
  %68 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef %58)
  %69 = getelementptr inbounds i64, ptr %7, i64 %59
  %70 = load i64, ptr %69, align 8, !tbaa !17
  %71 = getelementptr i64, ptr %69, i64 1
  %72 = load i64, ptr %71, align 8, !tbaa !17
  %73 = icmp sgt i64 %72, %70
  br i1 %73, label %74, label %124

74:                                               ; preds = %67, %119
  %75 = phi i64 [ %79, %119 ], [ %72, %67 ]
  %76 = phi ptr [ %122, %119 ], [ %54, %67 ]
  %77 = phi ptr [ %121, %119 ], [ %57, %67 ]
  %78 = phi ptr [ %120, %119 ], [ %56, %67 ]
  %79 = add nsw i64 %75, -1
  %80 = getelementptr inbounds i32, ptr %9, i64 %79
  %81 = load i32, ptr %80, align 4, !tbaa !14
  %82 = sext i32 %81 to i64
  %83 = getelementptr inbounds i8, ptr %15, i64 %82
  %84 = load i8, ptr %83, align 1, !tbaa !11
  %85 = icmp eq i8 %84, 0
  br i1 %85, label %86, label %119

86:                                               ; preds = %74
  %87 = icmp eq ptr %77, %76
  br i1 %87, label %90, label %88

88:                                               ; preds = %86
  store i32 %81, ptr %77, align 4, !tbaa !14
  %89 = getelementptr inbounds i32, ptr %77, i64 1
  br label %119

90:                                               ; preds = %86
  %91 = ptrtoint ptr %76 to i64
  %92 = ptrtoint ptr %78 to i64
  %93 = sub i64 %91, %92
  %94 = ashr exact i64 %93, 2
  %95 = icmp sgt i64 %94, -1
  tail call void @llvm.assume(i1 %95)
  %96 = icmp eq i64 %93, 9223372036854775804
  br i1 %96, label %97, label %99

97:                                               ; preds = %90
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #9
          to label %98 unwind label %117

98:                                               ; preds = %97
  unreachable

99:                                               ; preds = %90
  %100 = tail call i64 @llvm.umax.i64(i64 %94, i64 1)
  %101 = add nuw nsw i64 %100, %94
  %102 = tail call noundef i64 @llvm.umin.i64(i64 %101, i64 2305843009213693951)
  %103 = shl nuw nsw i64 %102, 2
  %104 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %103) #10
          to label %105 unwind label %115

105:                                              ; preds = %99
  %106 = getelementptr inbounds i8, ptr %104, i64 %93
  store i32 %81, ptr %106, align 4, !tbaa !14
  %107 = icmp sgt i64 %93, 0
  br i1 %107, label %108, label %109

108:                                              ; preds = %105
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %104, ptr align 4 %78, i64 %93, i1 false)
  br label %109

109:                                              ; preds = %108, %105
  %110 = icmp eq ptr %78, null
  br i1 %110, label %112, label %111

111:                                              ; preds = %109
  tail call void @_ZdlPv(ptr noundef nonnull %78) #11
  br label %112

112:                                              ; preds = %111, %109
  %113 = getelementptr inbounds i32, ptr %106, i64 1
  %114 = getelementptr inbounds i32, ptr %104, i64 %102
  br label %119

115:                                              ; preds = %99
  %116 = landingpad { ptr, i32 }
          cleanup
  br label %135

117:                                              ; preds = %97
  %118 = landingpad { ptr, i32 }
          cleanup
  br label %135

119:                                              ; preds = %112, %88, %74
  %120 = phi ptr [ %78, %74 ], [ %104, %112 ], [ %78, %88 ]
  %121 = phi ptr [ %77, %74 ], [ %113, %112 ], [ %89, %88 ]
  %122 = phi ptr [ %76, %74 ], [ %114, %112 ], [ %76, %88 ]
  %123 = icmp sgt i64 %79, %70
  br i1 %123, label %74, label %124, !llvm.loop !18

124:                                              ; preds = %119, %67, %53
  %125 = phi ptr [ %56, %53 ], [ %56, %67 ], [ %120, %119 ]
  %126 = phi ptr [ %57, %53 ], [ %57, %67 ], [ %121, %119 ]
  %127 = phi ptr [ %54, %53 ], [ %54, %67 ], [ %122, %119 ]
  %128 = icmp eq ptr %125, %126
  br i1 %128, label %129, label %53

129:                                              ; preds = %124
  %130 = icmp eq ptr %125, null
  br i1 %130, label %132, label %131

131:                                              ; preds = %129
  tail call void @_ZdlPv(ptr noundef nonnull %125) #11
  br label %132

132:                                              ; preds = %131, %129, %31
  %133 = add nuw nsw i64 %32, 1
  %134 = icmp eq i64 %133, %21
  br i1 %134, label %30, label %31, !llvm.loop !12

135:                                              ; preds = %115, %117
  %136 = phi { ptr, i32 } [ %116, %115 ], [ %118, %117 ]
  %137 = icmp eq ptr %78, null
  br i1 %137, label %139, label %138

138:                                              ; preds = %135
  tail call void @_ZdlPv(ptr noundef nonnull %78) #11
  br label %139

139:                                              ; preds = %63, %65, %135, %138
  %140 = phi { ptr, i32 } [ %136, %135 ], [ %136, %138 ], [ %64, %63 ], [ %66, %65 ]
  tail call void @_ZdlPv(ptr noundef nonnull %15) #11
  resume { ptr, i32 } %140

141:                                              ; preds = %12, %30, %1
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #2

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #3

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #5

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #6

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #7

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #8

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #5 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #7 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #8 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #9 = { cold noreturn }
attributes #10 = { builtin allocsize(0) }
attributes #11 = { builtin nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"_ZTS5Graph", !5, i64 0, !5, i64 8, !8, i64 16, !8, i64 24}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"any pointer", !6, i64 0}
!9 = !{!4, !8, i64 16}
!10 = !{!4, !8, i64 24}
!11 = !{!6, !6, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !6, i64 0}
!16 = distinct !{!16, !13}
!17 = !{!5, !5, i64 0}
!18 = distinct !{!18, !13}
