; ModuleID = 'bfs_runtime.bc'
source_filename = "bfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.Graph = type { i64, i64, i64*, i32*, i32* }

@.str = private unnamed_addr constant [46 x i8] c"[BFS] Completed in %.6f seconds (sequential)\0A\00", align 1
@.str.1 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.2 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @bfs_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = icmp eq ptr %0, null
  br i1 %2, label %144, label %3

3:                                                ; preds = %1
  %4 = load i64, ptr %0, align 8, !tbaa !3
  %5 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %6 = load ptr, ptr %5, align 8, !tbaa !9
  %7 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %8 = load ptr, ptr %7, align 8, !tbaa !10
  %9 = icmp slt i64 %4, 0
  br i1 %9, label %10, label %11

10:                                               ; preds = %3
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #10
  unreachable

11:                                               ; preds = %3
  %12 = icmp eq i64 %4, 0
  br i1 %12, label %15, label %13

13:                                               ; preds = %11
  %14 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %4) #11
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %14, i8 0, i64 %4, i1 false)
  br label %15

15:                                               ; preds = %13, %11
  %16 = phi ptr [ %14, %13 ], [ null, %11 ]
  %17 = invoke double @omp_get_wtime()
          to label %18 unwind label %25

18:                                               ; preds = %15
  %19 = trunc i64 %4 to i32
  %20 = icmp sgt i32 %19, 0
  br i1 %20, label %21, label %23

21:                                               ; preds = %18
  %22 = and i64 %4, 4294967295
  br label %27

23:                                               ; preds = %133, %18
  %24 = invoke double @omp_get_wtime()
          to label %139 unwind label %145

25:                                               ; preds = %15
  %26 = landingpad { ptr, i32 }
          cleanup
  br label %147

27:                                               ; preds = %21, %133
  %28 = phi i64 [ 0, %21 ], [ %134, %133 ]
  %29 = getelementptr inbounds i8, ptr %16, i64 %28
  %30 = load i8, ptr %29, align 1, !tbaa !11
  %31 = icmp eq i8 %30, 0
  br i1 %31, label %32, label %133

32:                                               ; preds = %27
  %33 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #11
          to label %34 unwind label %55

34:                                               ; preds = %32
  %35 = trunc i64 %28 to i32
  store i32 %35, ptr %33, align 4, !tbaa !12
  %36 = getelementptr inbounds i32, ptr %33, i64 1
  store i8 1, ptr %29, align 1, !tbaa !11
  br label %37

37:                                               ; preds = %34, %53
  %38 = phi ptr [ %36, %34 ], [ %50, %53 ]
  %39 = phi ptr [ %33, %34 ], [ %49, %53 ]
  %40 = ptrtoint ptr %38 to i64
  %41 = ptrtoint ptr %39 to i64
  %42 = sub i64 %40, %41
  %43 = ashr exact i64 %42, 2
  %44 = icmp sgt i64 %43, -1
  tail call void @llvm.assume(i1 %44)
  %45 = icmp eq ptr %38, %39
  br i1 %45, label %48, label %46

46:                                               ; preds = %37
  %47 = tail call i64 @llvm.umax.i64(i64 %43, i64 1)
  br label %57

48:                                               ; preds = %70, %37
  %49 = phi ptr [ null, %37 ], [ %71, %70 ]
  %50 = phi ptr [ null, %37 ], [ %72, %70 ]
  %51 = icmp eq ptr %39, null
  br i1 %51, label %53, label %52

52:                                               ; preds = %48
  tail call void @_ZdlPv(ptr noundef nonnull %39) #12
  br label %53

53:                                               ; preds = %48, %52
  %54 = icmp eq ptr %49, %50
  br i1 %54, label %130, label %37, !llvm.loop !14

55:                                               ; preds = %32
  %56 = landingpad { ptr, i32 }
          cleanup
  br label %150

57:                                               ; preds = %46, %70
  %58 = phi i64 [ %74, %70 ], [ 0, %46 ]
  %59 = phi ptr [ %73, %70 ], [ null, %46 ]
  %60 = phi ptr [ %72, %70 ], [ null, %46 ]
  %61 = phi ptr [ %71, %70 ], [ null, %46 ]
  %62 = getelementptr inbounds i32, ptr %39, i64 %58
  %63 = load i32, ptr %62, align 4, !tbaa !12
  %64 = sext i32 %63 to i64
  %65 = getelementptr inbounds i64, ptr %6, i64 %64
  %66 = load i64, ptr %65, align 8, !tbaa !16
  %67 = getelementptr i64, ptr %65, i64 1
  %68 = load i64, ptr %67, align 8, !tbaa !16
  %69 = icmp slt i64 %66, %68
  br i1 %69, label %76, label %70

70:                                               ; preds = %124, %57
  %71 = phi ptr [ %61, %57 ], [ %125, %124 ]
  %72 = phi ptr [ %60, %57 ], [ %126, %124 ]
  %73 = phi ptr [ %59, %57 ], [ %127, %124 ]
  %74 = add nuw nsw i64 %58, 1
  %75 = icmp eq i64 %74, %47
  br i1 %75, label %48, label %57, !llvm.loop !17

76:                                               ; preds = %57, %124
  %77 = phi i64 [ %128, %124 ], [ %66, %57 ]
  %78 = phi ptr [ %127, %124 ], [ %59, %57 ]
  %79 = phi ptr [ %126, %124 ], [ %60, %57 ]
  %80 = phi ptr [ %125, %124 ], [ %61, %57 ]
  %81 = getelementptr inbounds i32, ptr %8, i64 %77
  %82 = load i32, ptr %81, align 4, !tbaa !12
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds i8, ptr %16, i64 %83
  %85 = load i8, ptr %84, align 1, !tbaa !11
  %86 = icmp eq i8 %85, 0
  br i1 %86, label %87, label %124

87:                                               ; preds = %76
  store i8 1, ptr %84, align 1, !tbaa !11
  %88 = icmp eq ptr %79, %78
  br i1 %88, label %91, label %89

89:                                               ; preds = %87
  store i32 %82, ptr %79, align 4, !tbaa !12
  %90 = getelementptr inbounds i32, ptr %79, i64 1
  br label %124

91:                                               ; preds = %87
  %92 = ptrtoint ptr %78 to i64
  %93 = ptrtoint ptr %80 to i64
  %94 = sub i64 %92, %93
  %95 = ashr exact i64 %94, 2
  %96 = icmp sgt i64 %95, -1
  tail call void @llvm.assume(i1 %96)
  %97 = icmp eq i64 %94, 9223372036854775804
  br i1 %97, label %98, label %100

98:                                               ; preds = %91
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #10
          to label %99 unwind label %118

99:                                               ; preds = %98
  unreachable

100:                                              ; preds = %91
  %101 = tail call i64 @llvm.umax.i64(i64 %95, i64 1)
  %102 = add nuw nsw i64 %101, %95
  %103 = tail call noundef i64 @llvm.umin.i64(i64 %102, i64 2305843009213693951)
  %104 = shl nuw nsw i64 %103, 2
  %105 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %104) #11
          to label %106 unwind label %116

106:                                              ; preds = %100
  %107 = getelementptr inbounds i8, ptr %105, i64 %94
  store i32 %82, ptr %107, align 4, !tbaa !12
  %108 = icmp sgt i64 %94, 0
  br i1 %108, label %109, label %110

109:                                              ; preds = %106
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %105, ptr align 4 %80, i64 %94, i1 false)
  br label %110

110:                                              ; preds = %109, %106
  %111 = icmp eq ptr %80, null
  br i1 %111, label %113, label %112

112:                                              ; preds = %110
  tail call void @_ZdlPv(ptr noundef nonnull %80) #12
  br label %113

113:                                              ; preds = %112, %110
  %114 = getelementptr inbounds i32, ptr %107, i64 1
  %115 = getelementptr inbounds i32, ptr %105, i64 %103
  br label %124

116:                                              ; preds = %100
  %117 = landingpad { ptr, i32 }
          cleanup
  br label %120

118:                                              ; preds = %98
  %119 = landingpad { ptr, i32 }
          cleanup
  br label %120

120:                                              ; preds = %118, %116
  %121 = phi { ptr, i32 } [ %117, %116 ], [ %119, %118 ]
  %122 = icmp eq ptr %80, null
  br i1 %122, label %136, label %123

123:                                              ; preds = %120
  tail call void @_ZdlPv(ptr noundef nonnull %80) #12
  br label %136

124:                                              ; preds = %113, %89, %76
  %125 = phi ptr [ %80, %76 ], [ %105, %113 ], [ %80, %89 ]
  %126 = phi ptr [ %79, %76 ], [ %114, %113 ], [ %90, %89 ]
  %127 = phi ptr [ %78, %76 ], [ %115, %113 ], [ %78, %89 ]
  %128 = add i64 %77, 1
  %129 = icmp eq i64 %128, %68
  br i1 %129, label %70, label %76, !llvm.loop !18

130:                                              ; preds = %53
  %131 = icmp eq ptr %49, null
  br i1 %131, label %133, label %132

132:                                              ; preds = %130
  tail call void @_ZdlPv(ptr noundef nonnull %49) #12
  br label %133

133:                                              ; preds = %132, %130, %27
  %134 = add nuw nsw i64 %28, 1
  %135 = icmp eq i64 %134, %22
  br i1 %135, label %23, label %27, !llvm.loop !19

136:                                              ; preds = %123, %120
  %137 = icmp eq ptr %39, null
  br i1 %137, label %150, label %138

138:                                              ; preds = %136
  tail call void @_ZdlPv(ptr noundef nonnull %39) #12
  br label %150

139:                                              ; preds = %23
  %140 = fsub double %24, %17
  %141 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, double noundef %140)
  %142 = icmp eq ptr %16, null
  br i1 %142, label %144, label %143

143:                                              ; preds = %139
  tail call void @_ZdlPv(ptr noundef nonnull %16) #12
  br label %144

144:                                              ; preds = %143, %139, %1
  ret void

145:                                              ; preds = %23
  %146 = landingpad { ptr, i32 }
          cleanup
  br label %147

147:                                              ; preds = %145, %25
  %148 = phi { ptr, i32 } [ %146, %145 ], [ %26, %25 ]
  %149 = icmp eq ptr %16, null
  br i1 %149, label %152, label %150

150:                                              ; preds = %55, %136, %138, %147
  %151 = phi { ptr, i32 } [ %148, %147 ], [ %121, %138 ], [ %121, %136 ], [ %56, %55 ]
  tail call void @_ZdlPv(ptr noundef nonnull %16) #12
  br label %152

152:                                              ; preds = %150, %147
  %153 = phi { ptr, i32 } [ %148, %147 ], [ %151, %150 ]
  resume { ptr, i32 } %153
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local double @omp_get_wtime() local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #3

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #4

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #9

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #6 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #9 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #10 = { cold noreturn }
attributes #11 = { builtin allocsize(0) }
attributes #12 = { builtin nounwind }

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
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !6, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.mustprogress"}
!16 = !{!5, !5, i64 0}
!17 = distinct !{!17, !15}
!18 = distinct !{!18, !15}
!19 = distinct !{!19, !15}
