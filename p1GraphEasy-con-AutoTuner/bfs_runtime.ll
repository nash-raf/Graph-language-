; ModuleID = 'bfs_runtime.cpp'
source_filename = "bfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.AutoNeighborIter = type { ptr, i64, i64, i64, i32, i32, i32, i32 }

@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @bfs_runtime(ptr noundef %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca %struct.AutoNeighborIter, align 8
  %3 = alloca i32, align 4
  %4 = icmp eq ptr %0, null
  br i1 %4, label %152, label %5

5:                                                ; preds = %1
  %6 = load i64, ptr %0, align 8, !tbaa !3
  %7 = icmp slt i64 %6, 1
  br i1 %7, label %152, label %8

8:                                                ; preds = %5
  %9 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %6) #10
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %9, i8 0, i64 %6, i1 false)
  %10 = invoke double @omp_get_wtime()
          to label %11 unwind label %18

11:                                               ; preds = %8
  %12 = trunc i64 %6 to i32
  %13 = icmp sgt i32 %12, 0
  br i1 %13, label %14, label %16

14:                                               ; preds = %11
  %15 = and i64 %6, 2147483647
  br label %20

16:                                               ; preds = %143, %11
  %17 = invoke double @omp_get_wtime()
          to label %151 unwind label %153

18:                                               ; preds = %8
  %19 = landingpad { ptr, i32 }
          cleanup
  br label %155

20:                                               ; preds = %14, %143
  %21 = phi i64 [ 0, %14 ], [ %144, %143 ]
  %22 = getelementptr inbounds nuw i8, ptr %9, i64 %21
  %23 = load i8, ptr %22, align 1, !tbaa !11
  %24 = icmp eq i8 %23, 0
  br i1 %24, label %25, label %143

25:                                               ; preds = %20
  %26 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #10
          to label %27 unwind label %52

27:                                               ; preds = %25
  %28 = trunc nuw nsw i64 %21 to i32
  store i32 %28, ptr %26, align 4, !tbaa !12
  %29 = getelementptr inbounds nuw i8, ptr %26, i64 4
  store i8 1, ptr %22, align 1, !tbaa !11
  br label %30

30:                                               ; preds = %27, %50
  %31 = phi ptr [ %29, %27 ], [ %45, %50 ]
  %32 = phi ptr [ %29, %27 ], [ %44, %50 ]
  %33 = phi ptr [ %26, %27 ], [ %43, %50 ]
  %34 = ptrtoint ptr %32 to i64
  %35 = ptrtoint ptr %33 to i64
  %36 = sub i64 %34, %35
  %37 = ashr exact i64 %36, 2
  %38 = icmp sgt i64 %37, -1
  call void @llvm.assume(i1 %38)
  %39 = icmp eq ptr %32, %33
  br i1 %39, label %42, label %40

40:                                               ; preds = %30
  %41 = call i64 @llvm.umax.i64(i64 %37, i64 1)
  br label %54

42:                                               ; preds = %125, %30
  %43 = phi ptr [ null, %30 ], [ %64, %125 ]
  %44 = phi ptr [ null, %30 ], [ %68, %125 ]
  %45 = phi ptr [ null, %30 ], [ %66, %125 ]
  %46 = icmp eq ptr %33, null
  br i1 %46, label %50, label %47

47:                                               ; preds = %42
  %48 = ptrtoint ptr %31 to i64
  %49 = sub i64 %48, %35
  call void @_ZdlPvm(ptr noundef nonnull %33, i64 noundef %49) #11
  br label %50

50:                                               ; preds = %42, %47
  %51 = icmp eq ptr %43, %44
  br i1 %51, label %137, label %30, !llvm.loop !14

52:                                               ; preds = %25
  %53 = landingpad { ptr, i32 }
          cleanup
  br label %155

54:                                               ; preds = %40, %125
  %55 = phi i64 [ %126, %125 ], [ 0, %40 ]
  %56 = phi ptr [ %66, %125 ], [ null, %40 ]
  %57 = phi ptr [ %68, %125 ], [ null, %40 ]
  %58 = phi ptr [ %64, %125 ], [ null, %40 ]
  %59 = getelementptr inbounds nuw i32, ptr %33, i64 %55
  %60 = load i32, ptr %59, align 4, !tbaa !12
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %2) #12
  %61 = sext i32 %60 to i64
  invoke void @autograph_neighbor_iter_init(ptr noundef nonnull %0, i64 noundef %61, ptr noundef nonnull %2)
          to label %62 unwind label %80

62:                                               ; preds = %54
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %3) #12
  br label %63

63:                                               ; preds = %122, %62
  %64 = phi ptr [ %114, %122 ], [ %58, %62 ]
  %65 = phi ptr [ %123, %122 ], [ %57, %62 ]
  %66 = phi ptr [ %124, %122 ], [ %56, %62 ]
  br label %67

67:                                               ; preds = %63, %98
  %68 = phi ptr [ %65, %63 ], [ %99, %98 ]
  br label %69

69:                                               ; preds = %67, %91
  br label %70

70:                                               ; preds = %74, %69
  %71 = invoke i32 @autograph_neighbor_iter_next(ptr noundef nonnull %2, ptr noundef nonnull %3)
          to label %72 unwind label %82

72:                                               ; preds = %70
  %73 = icmp eq i32 %71, 0
  br i1 %73, label %125, label %74

74:                                               ; preds = %72
  %75 = load i32, ptr %3, align 4, !tbaa !12
  %76 = icmp sgt i32 %75, -1
  %77 = zext nneg i32 %75 to i64
  %78 = icmp sgt i64 %6, %77
  %79 = select i1 %76, i1 %78, i1 false
  br i1 %79, label %91, label %70, !llvm.loop !16

80:                                               ; preds = %54
  %81 = landingpad { ptr, i32 }
          cleanup
  br label %128

82:                                               ; preds = %70
  %83 = landingpad { ptr, i32 }
          cleanup
  br label %88

84:                                               ; preds = %109
  %85 = landingpad { ptr, i32 }
          cleanup
  br label %88

86:                                               ; preds = %107
  %87 = landingpad { ptr, i32 }
          cleanup
  br label %88

88:                                               ; preds = %82, %84, %86
  %89 = phi ptr [ %68, %86 ], [ %66, %82 ], [ %68, %84 ]
  %90 = phi { ptr, i32 } [ %87, %86 ], [ %83, %82 ], [ %85, %84 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %3) #12
  br label %128

91:                                               ; preds = %74
  %92 = getelementptr inbounds nuw i8, ptr %9, i64 %77
  %93 = load i8, ptr %92, align 1, !tbaa !11
  %94 = icmp eq i8 %93, 0
  br i1 %94, label %95, label %69, !llvm.loop !16

95:                                               ; preds = %91
  %96 = getelementptr inbounds nuw i8, ptr %9, i64 %77
  store i8 1, ptr %96, align 1, !tbaa !11
  %97 = icmp eq ptr %68, %66
  br i1 %97, label %100, label %98

98:                                               ; preds = %95
  store i32 %75, ptr %68, align 4, !tbaa !12
  %99 = getelementptr inbounds nuw i8, ptr %68, i64 4
  br label %67, !llvm.loop !16

100:                                              ; preds = %95
  %101 = ptrtoint ptr %68 to i64
  %102 = ptrtoint ptr %64 to i64
  %103 = sub i64 %101, %102
  %104 = ashr exact i64 %103, 2
  %105 = icmp sgt i64 %104, -1
  call void @llvm.assume(i1 %105)
  %106 = icmp eq i64 %103, 9223372036854775804
  br i1 %106, label %107, label %109

107:                                              ; preds = %100
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #13
          to label %108 unwind label %86

108:                                              ; preds = %107
  unreachable

109:                                              ; preds = %100
  %110 = call i64 @llvm.umax.i64(i64 %104, i64 1)
  %111 = add nuw nsw i64 %110, %104
  %112 = call noundef i64 @llvm.umin.i64(i64 %111, i64 2305843009213693951)
  %113 = shl nuw nsw i64 %112, 2
  %114 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %113) #10
          to label %115 unwind label %84

115:                                              ; preds = %109
  %116 = getelementptr inbounds i8, ptr %114, i64 %103
  store i32 %75, ptr %116, align 4, !tbaa !12
  %117 = icmp sgt i64 %103, 0
  br i1 %117, label %118, label %119

118:                                              ; preds = %115
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %114, ptr align 4 %64, i64 %103, i1 false)
  br label %119

119:                                              ; preds = %118, %115
  %120 = icmp eq ptr %64, null
  br i1 %120, label %122, label %121

121:                                              ; preds = %119
  call void @_ZdlPvm(ptr noundef nonnull %64, i64 noundef %103) #11
  br label %122

122:                                              ; preds = %121, %119
  %123 = getelementptr inbounds nuw i8, ptr %116, i64 4
  %124 = getelementptr inbounds nuw i32, ptr %114, i64 %112
  br label %63, !llvm.loop !16

125:                                              ; preds = %72
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %3) #12
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %2) #12
  %126 = add nuw nsw i64 %55, 1
  %127 = icmp eq i64 %126, %41
  br i1 %127, label %42, label %54, !llvm.loop !17

128:                                              ; preds = %88, %80
  %129 = phi ptr [ %64, %88 ], [ %58, %80 ]
  %130 = phi ptr [ %89, %88 ], [ %56, %80 ]
  %131 = phi { ptr, i32 } [ %90, %88 ], [ %81, %80 ]
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %2) #12
  %132 = icmp eq ptr %129, null
  br i1 %132, label %146, label %133

133:                                              ; preds = %128
  %134 = ptrtoint ptr %130 to i64
  %135 = ptrtoint ptr %129 to i64
  %136 = sub i64 %134, %135
  call void @_ZdlPvm(ptr noundef nonnull %129, i64 noundef %136) #11
  br label %146

137:                                              ; preds = %50
  %138 = icmp eq ptr %43, null
  br i1 %138, label %143, label %139

139:                                              ; preds = %137
  %140 = ptrtoint ptr %45 to i64
  %141 = ptrtoint ptr %43 to i64
  %142 = sub i64 %140, %141
  call void @_ZdlPvm(ptr noundef nonnull %43, i64 noundef %142) #11
  br label %143

143:                                              ; preds = %139, %137, %20
  %144 = add nuw nsw i64 %21, 1
  %145 = icmp eq i64 %144, %15
  br i1 %145, label %16, label %20, !llvm.loop !18

146:                                              ; preds = %133, %128
  %147 = icmp eq ptr %33, null
  br i1 %147, label %155, label %148

148:                                              ; preds = %146
  %149 = ptrtoint ptr %31 to i64
  %150 = sub i64 %149, %35
  call void @_ZdlPvm(ptr noundef nonnull %33, i64 noundef %150) #11
  br label %155

151:                                              ; preds = %16
  call void @_ZdlPvm(ptr noundef nonnull %9, i64 noundef %6) #11
  br label %152

152:                                              ; preds = %151, %5, %1
  ret void

153:                                              ; preds = %16
  %154 = landingpad { ptr, i32 }
          cleanup
  br label %155

155:                                              ; preds = %148, %146, %52, %153, %18
  %156 = phi { ptr, i32 } [ %154, %153 ], [ %19, %18 ], [ %53, %52 ], [ %131, %146 ], [ %131, %148 ]
  call void @_ZdlPvm(ptr noundef nonnull %9, i64 noundef %6) #11
  resume { ptr, i32 } %156
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local double @omp_get_wtime() local_unnamed_addr #2

declare dso_local void @autograph_neighbor_iter_init(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_neighbor_iter_next(ptr noundef, ptr noundef) local_unnamed_addr #2

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
attributes #11 = { builtin nounwind }
attributes #12 = { nounwind }
attributes #13 = { cold noreturn }

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
!11 = !{!6, !6, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !6, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.mustprogress"}
!16 = distinct !{!16, !15}
!17 = distinct !{!17, !15}
!18 = distinct !{!18, !15}
