; ModuleID = 'bfs_runtime.cpp'
source_filename = "bfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

@.str = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @bfs_runtime(ptr noundef %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca i64, align 8
  %3 = icmp eq ptr %0, null
  br i1 %3, label %170, label %4

4:                                                ; preds = %1
  %5 = load i64, ptr %0, align 8, !tbaa !3
  %6 = icmp slt i64 %5, 1
  br i1 %6, label %170, label %7

7:                                                ; preds = %4
  %8 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %5) #10
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %8, i8 0, i64 %5, i1 false)
  %9 = icmp samesign ugt i64 %5, 2305843009213693951
  br i1 %9, label %10, label %12

10:                                               ; preds = %7
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #11
          to label %11 unwind label %29

11:                                               ; preds = %10
  unreachable

12:                                               ; preds = %7
  %13 = shl nuw nsw i64 %5, 2
  %14 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #10
          to label %15 unwind label %29

15:                                               ; preds = %12
  store i32 0, ptr %14, align 4, !tbaa !11
  %16 = icmp eq i64 %5, 1
  br i1 %16, label %20, label %17

17:                                               ; preds = %15
  %18 = getelementptr i8, ptr %14, i64 4
  %19 = add nsw i64 %13, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %18, i8 0, i64 %19, i1 false), !tbaa !11
  br label %20

20:                                               ; preds = %17, %15
  %21 = invoke double @omp_get_wtime()
          to label %22 unwind label %31

22:                                               ; preds = %20
  %23 = trunc i64 %5 to i32
  %24 = icmp sgt i32 %23, 0
  br i1 %24, label %25, label %27

25:                                               ; preds = %22
  %26 = and i64 %5, 2147483647
  br label %33

27:                                               ; preds = %161, %22
  %28 = invoke double @omp_get_wtime()
          to label %169 unwind label %171

29:                                               ; preds = %12, %10
  %30 = landingpad { ptr, i32 }
          cleanup
  br label %175

31:                                               ; preds = %20
  %32 = landingpad { ptr, i32 }
          cleanup
  br label %173

33:                                               ; preds = %25, %161
  %34 = phi i64 [ 0, %25 ], [ %162, %161 ]
  %35 = getelementptr inbounds nuw i8, ptr %8, i64 %34
  %36 = load i8, ptr %35, align 1, !tbaa !13
  %37 = icmp eq i8 %36, 0
  br i1 %37, label %38, label %161

38:                                               ; preds = %33
  %39 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #10
          to label %40 unwind label %65

40:                                               ; preds = %38
  %41 = trunc nuw nsw i64 %34 to i32
  store i32 %41, ptr %39, align 4, !tbaa !11
  %42 = getelementptr inbounds nuw i8, ptr %39, i64 4
  store i8 1, ptr %35, align 1, !tbaa !13
  br label %43

43:                                               ; preds = %40, %63
  %44 = phi ptr [ %42, %40 ], [ %58, %63 ]
  %45 = phi ptr [ %42, %40 ], [ %57, %63 ]
  %46 = phi ptr [ %39, %40 ], [ %56, %63 ]
  %47 = ptrtoint ptr %45 to i64
  %48 = ptrtoint ptr %46 to i64
  %49 = sub i64 %47, %48
  %50 = ashr exact i64 %49, 2
  %51 = icmp sgt i64 %50, -1
  call void @llvm.assume(i1 %51)
  %52 = icmp eq ptr %45, %46
  br i1 %52, label %55, label %53

53:                                               ; preds = %43
  %54 = call i64 @llvm.umax.i64(i64 %50, i64 1)
  br label %67

55:                                               ; preds = %78, %43
  %56 = phi ptr [ null, %43 ], [ %79, %78 ]
  %57 = phi ptr [ null, %43 ], [ %80, %78 ]
  %58 = phi ptr [ null, %43 ], [ %81, %78 ]
  %59 = icmp eq ptr %46, null
  br i1 %59, label %63, label %60

60:                                               ; preds = %55
  %61 = ptrtoint ptr %44 to i64
  %62 = sub i64 %61, %48
  call void @_ZdlPvm(ptr noundef nonnull %46, i64 noundef %62) #12
  br label %63

63:                                               ; preds = %55, %60
  %64 = icmp eq ptr %56, %57
  br i1 %64, label %155, label %43, !llvm.loop !14

65:                                               ; preds = %38
  %66 = landingpad { ptr, i32 }
          cleanup
  br label %173

67:                                               ; preds = %53, %78
  %68 = phi i64 [ %82, %78 ], [ 0, %53 ]
  %69 = phi ptr [ %81, %78 ], [ null, %53 ]
  %70 = phi ptr [ %80, %78 ], [ null, %53 ]
  %71 = phi ptr [ %79, %78 ], [ null, %53 ]
  %72 = getelementptr inbounds nuw i32, ptr %46, i64 %68
  %73 = load i32, ptr %72, align 4, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #13
  store i64 0, ptr %2, align 8, !tbaa !16
  %74 = sext i32 %73 to i64
  invoke void @autograph_get_neighbors(ptr noundef nonnull %0, i64 noundef %74, ptr noundef nonnull %14, ptr noundef nonnull %2)
          to label %75 unwind label %84

75:                                               ; preds = %67
  %76 = load i64, ptr %2, align 8, !tbaa !16
  %77 = icmp sgt i64 %76, 0
  br i1 %77, label %86, label %78

78:                                               ; preds = %138, %75
  %79 = phi ptr [ %71, %75 ], [ %141, %138 ]
  %80 = phi ptr [ %70, %75 ], [ %142, %138 ]
  %81 = phi ptr [ %69, %75 ], [ %143, %138 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #13
  %82 = add nuw nsw i64 %68, 1
  %83 = icmp eq i64 %82, %54
  br i1 %83, label %55, label %67, !llvm.loop !17

84:                                               ; preds = %67
  %85 = landingpad { ptr, i32 }
          cleanup
  br label %146

86:                                               ; preds = %75, %138
  %87 = phi i64 [ %139, %138 ], [ %76, %75 ]
  %88 = phi i64 [ %140, %138 ], [ %76, %75 ]
  %89 = phi i64 [ %144, %138 ], [ 0, %75 ]
  %90 = phi ptr [ %143, %138 ], [ %69, %75 ]
  %91 = phi ptr [ %142, %138 ], [ %70, %75 ]
  %92 = phi ptr [ %141, %138 ], [ %71, %75 ]
  %93 = getelementptr inbounds nuw i32, ptr %14, i64 %89
  %94 = load i32, ptr %93, align 4, !tbaa !11
  %95 = icmp slt i32 %94, 0
  br i1 %95, label %138, label %96

96:                                               ; preds = %86
  %97 = zext nneg i32 %94 to i64
  %98 = icmp sgt i64 %5, %97
  br i1 %98, label %99, label %138

99:                                               ; preds = %96
  %100 = getelementptr inbounds nuw i8, ptr %8, i64 %97
  %101 = load i8, ptr %100, align 1, !tbaa !13
  %102 = icmp eq i8 %101, 0
  br i1 %102, label %103, label %138

103:                                              ; preds = %99
  store i8 1, ptr %100, align 1, !tbaa !13
  %104 = icmp eq ptr %91, %90
  br i1 %104, label %107, label %105

105:                                              ; preds = %103
  store i32 %94, ptr %91, align 4, !tbaa !11
  %106 = getelementptr inbounds nuw i8, ptr %91, i64 4
  br label %138

107:                                              ; preds = %103
  %108 = ptrtoint ptr %90 to i64
  %109 = ptrtoint ptr %92 to i64
  %110 = sub i64 %108, %109
  %111 = ashr exact i64 %110, 2
  %112 = icmp sgt i64 %111, -1
  call void @llvm.assume(i1 %112)
  %113 = icmp eq i64 %110, 9223372036854775804
  br i1 %113, label %114, label %116

114:                                              ; preds = %107
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #11
          to label %115 unwind label %136

115:                                              ; preds = %114
  unreachable

116:                                              ; preds = %107
  %117 = call i64 @llvm.umax.i64(i64 %111, i64 1)
  %118 = add nuw nsw i64 %117, %111
  %119 = call noundef i64 @llvm.umin.i64(i64 %118, i64 2305843009213693951)
  %120 = shl nuw nsw i64 %119, 2
  %121 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %120) #10
          to label %122 unwind label %134

122:                                              ; preds = %116
  %123 = getelementptr inbounds i8, ptr %121, i64 %110
  store i32 %94, ptr %123, align 4, !tbaa !11
  %124 = icmp sgt i64 %110, 0
  br i1 %124, label %125, label %126

125:                                              ; preds = %122
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %121, ptr align 4 %92, i64 %110, i1 false)
  br label %126

126:                                              ; preds = %125, %122
  %127 = icmp eq ptr %92, null
  br i1 %127, label %130, label %128

128:                                              ; preds = %126
  call void @_ZdlPvm(ptr noundef nonnull %92, i64 noundef %110) #12
  %129 = load i64, ptr %2, align 8, !tbaa !16
  br label %130

130:                                              ; preds = %128, %126
  %131 = phi i64 [ %129, %128 ], [ %87, %126 ]
  %132 = getelementptr inbounds nuw i8, ptr %123, i64 4
  %133 = getelementptr inbounds nuw i32, ptr %121, i64 %119
  br label %138

134:                                              ; preds = %116
  %135 = landingpad { ptr, i32 }
          cleanup
  br label %146

136:                                              ; preds = %114
  %137 = landingpad { ptr, i32 }
          cleanup
  br label %146

138:                                              ; preds = %130, %105, %99, %86, %96
  %139 = phi i64 [ %87, %86 ], [ %87, %99 ], [ %87, %96 ], [ %131, %130 ], [ %87, %105 ]
  %140 = phi i64 [ %88, %86 ], [ %88, %99 ], [ %88, %96 ], [ %131, %130 ], [ %88, %105 ]
  %141 = phi ptr [ %92, %86 ], [ %92, %99 ], [ %92, %96 ], [ %121, %130 ], [ %92, %105 ]
  %142 = phi ptr [ %91, %86 ], [ %91, %99 ], [ %91, %96 ], [ %132, %130 ], [ %106, %105 ]
  %143 = phi ptr [ %90, %86 ], [ %90, %99 ], [ %90, %96 ], [ %133, %130 ], [ %90, %105 ]
  %144 = add nuw nsw i64 %89, 1
  %145 = icmp slt i64 %144, %140
  br i1 %145, label %86, label %78, !llvm.loop !18

146:                                              ; preds = %134, %136, %84
  %147 = phi ptr [ %71, %84 ], [ %92, %134 ], [ %92, %136 ]
  %148 = phi ptr [ %69, %84 ], [ %90, %134 ], [ %90, %136 ]
  %149 = phi { ptr, i32 } [ %85, %84 ], [ %135, %134 ], [ %137, %136 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #13
  %150 = icmp eq ptr %147, null
  br i1 %150, label %164, label %151

151:                                              ; preds = %146
  %152 = ptrtoint ptr %148 to i64
  %153 = ptrtoint ptr %147 to i64
  %154 = sub i64 %152, %153
  call void @_ZdlPvm(ptr noundef nonnull %147, i64 noundef %154) #12
  br label %164

155:                                              ; preds = %63
  %156 = icmp eq ptr %56, null
  br i1 %156, label %161, label %157

157:                                              ; preds = %155
  %158 = ptrtoint ptr %58 to i64
  %159 = ptrtoint ptr %56 to i64
  %160 = sub i64 %158, %159
  call void @_ZdlPvm(ptr noundef nonnull %56, i64 noundef %160) #12
  br label %161

161:                                              ; preds = %157, %155, %33
  %162 = add nuw nsw i64 %34, 1
  %163 = icmp eq i64 %162, %26
  br i1 %163, label %27, label %33, !llvm.loop !19

164:                                              ; preds = %151, %146
  %165 = icmp eq ptr %46, null
  br i1 %165, label %173, label %166

166:                                              ; preds = %164
  %167 = ptrtoint ptr %44 to i64
  %168 = sub i64 %167, %48
  call void @_ZdlPvm(ptr noundef nonnull %46, i64 noundef %168) #12
  br label %173

169:                                              ; preds = %27
  call void @_ZdlPvm(ptr noundef nonnull %14, i64 noundef %13) #12
  call void @_ZdlPvm(ptr noundef nonnull %8, i64 noundef %5) #12
  br label %170

170:                                              ; preds = %169, %4, %1
  ret void

171:                                              ; preds = %27
  %172 = landingpad { ptr, i32 }
          cleanup
  br label %173

173:                                              ; preds = %166, %164, %65, %171, %31
  %174 = phi { ptr, i32 } [ %172, %171 ], [ %32, %31 ], [ %66, %65 ], [ %149, %164 ], [ %149, %166 ]
  call void @_ZdlPvm(ptr noundef nonnull %14, i64 noundef %13) #12
  br label %175

175:                                              ; preds = %173, %29
  %176 = phi { ptr, i32 } [ %174, %173 ], [ %30, %29 ]
  call void @_ZdlPvm(ptr noundef nonnull %8, i64 noundef %5) #12
  resume { ptr, i32 } %176
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local double @omp_get_wtime() local_unnamed_addr #2

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
!19 = distinct !{!19, !15}
