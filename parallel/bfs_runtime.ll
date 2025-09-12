; ModuleID = 'bfs_runtime.bc'
source_filename = "bfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Graph = type { i64, i64, ptr, ptr }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.3 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_insert\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @bfs_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = icmp eq ptr %0, null
  br i1 %2, label %143, label %3

3:                                                ; preds = %1
  %4 = load i64, ptr %0, align 8, !tbaa !5
  %5 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %6 = load ptr, ptr %5, align 8, !tbaa !11
  %7 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %8 = load ptr, ptr %7, align 8, !tbaa !12
  %9 = icmp slt i64 %4, 0
  br i1 %9, label %10, label %11

10:                                               ; preds = %3
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #8
  unreachable

11:                                               ; preds = %3
  %12 = icmp eq i64 %4, 0
  br i1 %12, label %143, label %13

13:                                               ; preds = %11
  %14 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %4) #9
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %14, i8 0, i64 %4, i1 false)
  %15 = icmp ugt i64 %4, 2305843009213693951
  br i1 %15, label %16, label %20

16:                                               ; preds = %13
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #8
          to label %19 unwind label %17

17:                                               ; preds = %16
  %18 = landingpad { ptr, i32 }
          cleanup
  br label %151

19:                                               ; preds = %16
  unreachable

20:                                               ; preds = %13
  %21 = shl nuw nsw i64 %4, 2
  %22 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %21) #9
          to label %23 unwind label %149

23:                                               ; preds = %20
  %24 = trunc i64 %4 to i32
  %25 = icmp sgt i32 %24, 0
  br i1 %25, label %26, label %31

26:                                               ; preds = %23
  %27 = getelementptr inbounds i32, ptr %22, i64 %4
  %28 = and i64 %4, 4294967295
  br label %34

29:                                               ; preds = %138
  %30 = icmp eq ptr %139, null
  br i1 %30, label %33, label %31

31:                                               ; preds = %23, %29
  %32 = phi ptr [ %139, %29 ], [ %22, %23 ]
  tail call void @_ZdlPv(ptr noundef nonnull %32) #10
  br label %33

33:                                               ; preds = %29, %31
  tail call void @_ZdlPv(ptr noundef nonnull %14) #10
  br label %143

34:                                               ; preds = %26, %138
  %35 = phi i64 [ 0, %26 ], [ %141, %138 ]
  %36 = phi ptr [ %27, %26 ], [ %140, %138 ]
  %37 = phi ptr [ %22, %26 ], [ %139, %138 ]
  %38 = getelementptr inbounds i8, ptr %14, i64 %35
  %39 = load i8, ptr %38, align 1, !tbaa !13
  %40 = icmp eq i8 %39, 0
  br i1 %40, label %41, label %138

41:                                               ; preds = %34
  %42 = icmp eq ptr %37, %36
  br i1 %42, label %45, label %43

43:                                               ; preds = %41
  %44 = trunc i64 %35 to i32
  store i32 %44, ptr %37, align 4, !tbaa !14
  br label %53

45:                                               ; preds = %41
  %46 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #9
          to label %47 unwind label %81

47:                                               ; preds = %45
  %48 = trunc i64 %35 to i32
  store i32 %48, ptr %46, align 4, !tbaa !14
  %49 = icmp eq ptr %36, null
  br i1 %49, label %51, label %50

50:                                               ; preds = %47
  tail call void @_ZdlPv(ptr noundef nonnull %36) #10
  br label %51

51:                                               ; preds = %50, %47
  %52 = getelementptr inbounds i32, ptr %46, i64 1
  br label %53

53:                                               ; preds = %51, %43
  %54 = phi ptr [ %46, %51 ], [ %37, %43 ]
  %55 = phi ptr [ %52, %51 ], [ %36, %43 ]
  %56 = getelementptr inbounds i32, ptr %54, i64 1
  store i8 1, ptr %38, align 1, !tbaa !13
  br label %66

57:                                               ; preds = %132, %66
  %58 = phi ptr [ %70, %66 ], [ %133, %132 ]
  %59 = phi ptr [ %69, %66 ], [ %134, %132 ]
  %60 = phi ptr [ %68, %66 ], [ %135, %132 ]
  %61 = ptrtoint ptr %59 to i64
  %62 = ptrtoint ptr %58 to i64
  %63 = sub i64 %61, %62
  %64 = ashr exact i64 %63, 2
  %65 = icmp ult i64 %71, %64
  br i1 %65, label %66, label %138, !llvm.loop !16

66:                                               ; preds = %53, %57
  %67 = phi i64 [ 0, %53 ], [ %71, %57 ]
  %68 = phi ptr [ %55, %53 ], [ %60, %57 ]
  %69 = phi ptr [ %56, %53 ], [ %59, %57 ]
  %70 = phi ptr [ %54, %53 ], [ %58, %57 ]
  %71 = add nuw i64 %67, 1
  %72 = getelementptr inbounds i32, ptr %70, i64 %67
  %73 = load i32, ptr %72, align 4, !tbaa !14
  %74 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef %73)
  %75 = sext i32 %73 to i64
  %76 = getelementptr inbounds i64, ptr %6, i64 %75
  %77 = load i64, ptr %76, align 8, !tbaa !18
  %78 = getelementptr i64, ptr %76, i64 1
  %79 = load i64, ptr %78, align 8, !tbaa !18
  %80 = icmp slt i64 %77, %79
  br i1 %80, label %83, label %57

81:                                               ; preds = %45
  %82 = landingpad { ptr, i32 }
          cleanup
  br label %144

83:                                               ; preds = %66, %132
  %84 = phi i64 [ %136, %132 ], [ %77, %66 ]
  %85 = phi ptr [ %135, %132 ], [ %68, %66 ]
  %86 = phi ptr [ %134, %132 ], [ %69, %66 ]
  %87 = phi ptr [ %133, %132 ], [ %70, %66 ]
  %88 = getelementptr inbounds i32, ptr %8, i64 %84
  %89 = load i32, ptr %88, align 4, !tbaa !14
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds i8, ptr %14, i64 %90
  %92 = load i8, ptr %91, align 1, !tbaa !13
  %93 = icmp eq i8 %92, 0
  br i1 %93, label %94, label %132

94:                                               ; preds = %83
  store i8 1, ptr %91, align 1, !tbaa !13
  %95 = icmp eq ptr %86, %85
  br i1 %95, label %98, label %96

96:                                               ; preds = %94
  store i32 %89, ptr %86, align 4, !tbaa !14
  %97 = getelementptr inbounds i32, ptr %86, i64 1
  br label %132

98:                                               ; preds = %94
  %99 = ptrtoint ptr %85 to i64
  %100 = ptrtoint ptr %87 to i64
  %101 = sub i64 %99, %100
  %102 = icmp eq i64 %101, 9223372036854775804
  br i1 %102, label %103, label %105

103:                                              ; preds = %98
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #8
          to label %104 unwind label %130

104:                                              ; preds = %103
  unreachable

105:                                              ; preds = %98
  %106 = ashr exact i64 %101, 2
  %107 = tail call i64 @llvm.umax.i64(i64 %106, i64 1)
  %108 = add nsw i64 %107, %106
  %109 = icmp ult i64 %108, %106
  %110 = tail call i64 @llvm.umin.i64(i64 %108, i64 2305843009213693951)
  %111 = select i1 %109, i64 2305843009213693951, i64 %110
  %112 = icmp eq i64 %111, 0
  br i1 %112, label %116, label %113

113:                                              ; preds = %105
  %114 = shl nuw nsw i64 %111, 2
  %115 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %114) #9
          to label %116 unwind label %128

116:                                              ; preds = %113, %105
  %117 = phi ptr [ null, %105 ], [ %115, %113 ]
  %118 = getelementptr inbounds i32, ptr %117, i64 %106
  store i32 %89, ptr %118, align 4, !tbaa !14
  %119 = icmp sgt i64 %101, 0
  br i1 %119, label %120, label %121

120:                                              ; preds = %116
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %117, ptr align 4 %87, i64 %101, i1 false)
  br label %121

121:                                              ; preds = %120, %116
  %122 = getelementptr inbounds i8, ptr %117, i64 %101
  %123 = getelementptr inbounds i32, ptr %122, i64 1
  %124 = icmp eq ptr %87, null
  br i1 %124, label %126, label %125

125:                                              ; preds = %121
  tail call void @_ZdlPv(ptr noundef nonnull %87) #10
  br label %126

126:                                              ; preds = %125, %121
  %127 = getelementptr inbounds i32, ptr %117, i64 %111
  br label %132

128:                                              ; preds = %113
  %129 = landingpad { ptr, i32 }
          cleanup
  br label %144

130:                                              ; preds = %103
  %131 = landingpad { ptr, i32 }
          cleanup
  br label %144

132:                                              ; preds = %126, %96, %83
  %133 = phi ptr [ %87, %83 ], [ %117, %126 ], [ %87, %96 ]
  %134 = phi ptr [ %86, %83 ], [ %123, %126 ], [ %97, %96 ]
  %135 = phi ptr [ %85, %83 ], [ %127, %126 ], [ %85, %96 ]
  %136 = add i64 %84, 1
  %137 = icmp eq i64 %136, %79
  br i1 %137, label %57, label %83, !llvm.loop !19

138:                                              ; preds = %57, %34
  %139 = phi ptr [ %37, %34 ], [ %58, %57 ]
  %140 = phi ptr [ %36, %34 ], [ %60, %57 ]
  %141 = add nuw nsw i64 %35, 1
  %142 = icmp eq i64 %141, %28
  br i1 %142, label %29, label %34, !llvm.loop !20

143:                                              ; preds = %11, %33, %1
  ret void

144:                                              ; preds = %128, %130, %81
  %145 = phi ptr [ %36, %81 ], [ %87, %128 ], [ %87, %130 ]
  %146 = phi { ptr, i32 } [ %82, %81 ], [ %129, %128 ], [ %131, %130 ]
  %147 = icmp eq ptr %145, null
  br i1 %147, label %151, label %148

148:                                              ; preds = %144
  tail call void @_ZdlPv(ptr noundef nonnull %145) #10
  br label %151

149:                                              ; preds = %20
  %150 = landingpad { ptr, i32 }
          cleanup
  br label %151

151:                                              ; preds = %17, %144, %148, %149
  %152 = phi { ptr, i32 } [ %150, %149 ], [ %18, %17 ], [ %146, %144 ], [ %146, %148 ]
  tail call void @_ZdlPv(ptr noundef nonnull %14) #10
  resume { ptr, i32 } %152
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: noreturn
declare void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #2

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #3

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(ptr noundef) local_unnamed_addr #5

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #6

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #7

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #7

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #5 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #7 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #8 = { noreturn }
attributes #9 = { allocsize(0) }
attributes #10 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)"}
!5 = !{!6, !7, i64 0}
!6 = !{!"_ZTS5Graph", !7, i64 0, !7, i64 8, !10, i64 16, !10, i64 24}
!7 = !{!"long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!"any pointer", !8, i64 0}
!11 = !{!6, !10, i64 16}
!12 = !{!6, !10, i64 24}
!13 = !{!8, !8, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !8, i64 0}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.mustprogress"}
!18 = !{!7, !7, i64 0}
!19 = distinct !{!19, !17}
!20 = distinct !{!20, !17}
