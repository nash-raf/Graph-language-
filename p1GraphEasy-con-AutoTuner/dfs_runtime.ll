; ModuleID = 'dfs_runtime.cpp'
source_filename = "dfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.AutoNeighborIter = type { ptr, i64, i64, i64, i32, i32, i32, i32 }

@.str.1 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.2 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @dfs_runtime(ptr noundef %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca %struct.AutoNeighborIter, align 8
  %3 = alloca i32, align 4
  %4 = icmp eq ptr %0, null
  br i1 %4, label %202, label %5

5:                                                ; preds = %1
  %6 = load i64, ptr %0, align 8, !tbaa !3
  %7 = icmp slt i64 %6, 1
  br i1 %7, label %202, label %8

8:                                                ; preds = %5
  %9 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %6) #10
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %9, i8 0, i64 %6, i1 false)
  %10 = trunc i64 %6 to i32
  %11 = icmp sgt i32 %10, 0
  br i1 %11, label %12, label %16

12:                                               ; preds = %8
  %13 = icmp samesign ugt i64 %6, 2305843009213693951
  %14 = shl nuw nsw i64 %6, 2
  %15 = and i64 %6, 2147483647
  br label %17

16:                                               ; preds = %188, %8
  call void @_ZdlPvm(ptr noundef nonnull %9, i64 noundef %6) #11
  br label %202

17:                                               ; preds = %12, %188
  %18 = phi i64 [ 0, %12 ], [ %189, %188 ]
  %19 = getelementptr inbounds nuw i8, ptr %9, i64 %18
  %20 = load i8, ptr %19, align 1, !tbaa !11
  %21 = icmp eq i8 %20, 0
  br i1 %21, label %22, label %188

22:                                               ; preds = %17
  br i1 %13, label %23, label %25

23:                                               ; preds = %22
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #12
          to label %24 unwind label %43

24:                                               ; preds = %23
  unreachable

25:                                               ; preds = %22
  %26 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %14) #10
          to label %27 unwind label %41

27:                                               ; preds = %25
  %28 = getelementptr inbounds nuw i32, ptr %26, i64 %6
  %29 = trunc nuw nsw i64 %18 to i32
  store i32 %29, ptr %26, align 4, !tbaa !12
  %30 = getelementptr inbounds nuw i8, ptr %26, i64 4
  br label %31

31:                                               ; preds = %27, %156
  %32 = phi ptr [ %28, %27 ], [ %159, %156 ]
  %33 = phi ptr [ %30, %27 ], [ %158, %156 ]
  %34 = phi ptr [ %26, %27 ], [ %157, %156 ]
  %35 = getelementptr inbounds i8, ptr %33, i64 -4
  %36 = load i32, ptr %35, align 4, !tbaa !12
  %37 = sext i32 %36 to i64
  %38 = getelementptr inbounds nuw i8, ptr %9, i64 %37
  %39 = load i8, ptr %38, align 1, !tbaa !11
  %40 = icmp eq i8 %39, 0
  br i1 %40, label %45, label %156, !llvm.loop !14

41:                                               ; preds = %25
  %42 = landingpad { ptr, i32 }
          cleanup
  br label %200

43:                                               ; preds = %23
  %44 = landingpad { ptr, i32 }
          cleanup
  br label %200

45:                                               ; preds = %31
  store i8 1, ptr %38, align 1, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %2) #13
  invoke void @autograph_neighbor_iter_init(ptr noundef nonnull %0, i64 noundef %37, ptr noundef nonnull %2)
          to label %46 unwind label %88

46:                                               ; preds = %45
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %3) #13
  br label %47

47:                                               ; preds = %83, %46
  %48 = phi ptr [ null, %46 ], [ %84, %83 ]
  %49 = phi ptr [ null, %46 ], [ %87, %83 ]
  %50 = phi ptr [ null, %46 ], [ %86, %83 ]
  %51 = invoke i32 @autograph_neighbor_iter_next(ptr noundef nonnull %2, ptr noundef nonnull %3)
          to label %52 unwind label %161

52:                                               ; preds = %47
  %53 = icmp eq i32 %51, 0
  br i1 %53, label %90, label %54

54:                                               ; preds = %52
  %55 = icmp eq ptr %49, %50
  br i1 %55, label %58, label %56

56:                                               ; preds = %54
  %57 = load i32, ptr %3, align 4, !tbaa !12
  store i32 %57, ptr %49, align 4, !tbaa !12
  br label %83

58:                                               ; preds = %54
  %59 = ptrtoint ptr %49 to i64
  %60 = ptrtoint ptr %48 to i64
  %61 = sub i64 %59, %60
  %62 = ashr exact i64 %61, 2
  %63 = icmp sgt i64 %62, -1
  call void @llvm.assume(i1 %63)
  %64 = icmp eq i64 %61, 9223372036854775804
  br i1 %64, label %65, label %67

65:                                               ; preds = %58
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #12
          to label %66 unwind label %164

66:                                               ; preds = %65
  unreachable

67:                                               ; preds = %58
  %68 = call i64 @llvm.umax.i64(i64 %62, i64 1)
  %69 = add nuw nsw i64 %68, %62
  %70 = call noundef i64 @llvm.umin.i64(i64 %69, i64 2305843009213693951)
  %71 = shl nuw nsw i64 %70, 2
  %72 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %71) #10
          to label %73 unwind label %161

73:                                               ; preds = %67
  %74 = getelementptr inbounds i8, ptr %72, i64 %61
  %75 = load i32, ptr %3, align 4, !tbaa !12
  store i32 %75, ptr %74, align 4, !tbaa !12
  %76 = icmp sgt i64 %61, 0
  br i1 %76, label %77, label %78

77:                                               ; preds = %73
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %72, ptr align 4 %48, i64 %61, i1 false)
  br label %78

78:                                               ; preds = %77, %73
  %79 = icmp eq ptr %48, null
  br i1 %79, label %81, label %80

80:                                               ; preds = %78
  call void @_ZdlPvm(ptr noundef nonnull %48, i64 noundef %61) #11
  br label %81

81:                                               ; preds = %80, %78
  %82 = getelementptr inbounds nuw i32, ptr %72, i64 %70
  br label %83

83:                                               ; preds = %56, %81
  %84 = phi ptr [ %72, %81 ], [ %48, %56 ]
  %85 = phi ptr [ %74, %81 ], [ %49, %56 ]
  %86 = phi ptr [ %82, %81 ], [ %50, %56 ]
  %87 = getelementptr inbounds nuw i8, ptr %85, i64 4
  br label %47

88:                                               ; preds = %45
  %89 = landingpad { ptr, i32 }
          cleanup
  br label %191

90:                                               ; preds = %52
  %91 = ptrtoint ptr %49 to i64
  %92 = ptrtoint ptr %48 to i64
  %93 = sub i64 %91, %92
  %94 = ashr exact i64 %93, 2
  %95 = icmp sgt i64 %94, -1
  call void @llvm.assume(i1 %95)
  %96 = icmp eq ptr %49, %48
  br i1 %96, label %97, label %109

97:                                               ; preds = %90
  %98 = icmp eq ptr %48, null
  br i1 %98, label %105, label %99

99:                                               ; preds = %151, %97
  %100 = phi ptr [ %32, %97 ], [ %154, %151 ]
  %101 = phi ptr [ %35, %97 ], [ %153, %151 ]
  %102 = phi ptr [ %34, %97 ], [ %152, %151 ]
  %103 = ptrtoint ptr %50 to i64
  %104 = sub i64 %103, %92
  call void @_ZdlPvm(ptr noundef nonnull %48, i64 noundef %104) #11
  br label %105

105:                                              ; preds = %97, %99
  %106 = phi ptr [ %32, %97 ], [ %100, %99 ]
  %107 = phi ptr [ %35, %97 ], [ %101, %99 ]
  %108 = phi ptr [ %34, %97 ], [ %102, %99 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %3) #13
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %2) #13
  br label %156

109:                                              ; preds = %90, %151
  %110 = phi i64 [ %114, %151 ], [ %94, %90 ]
  %111 = phi ptr [ %154, %151 ], [ %32, %90 ]
  %112 = phi ptr [ %153, %151 ], [ %35, %90 ]
  %113 = phi ptr [ %152, %151 ], [ %34, %90 ]
  %114 = add nsw i64 %110, -1
  %115 = getelementptr inbounds nuw i32, ptr %48, i64 %114
  %116 = load i32, ptr %115, align 4, !tbaa !12
  store i32 %116, ptr %3, align 4, !tbaa !12
  %117 = sext i32 %116 to i64
  %118 = getelementptr inbounds nuw i8, ptr %9, i64 %117
  %119 = load i8, ptr %118, align 1, !tbaa !11
  %120 = icmp eq i8 %119, 0
  br i1 %120, label %121, label %151

121:                                              ; preds = %109
  %122 = icmp eq ptr %112, %111
  br i1 %122, label %125, label %123

123:                                              ; preds = %121
  store i32 %116, ptr %112, align 4, !tbaa !12
  %124 = getelementptr inbounds nuw i8, ptr %112, i64 4
  br label %151

125:                                              ; preds = %121
  %126 = ptrtoint ptr %111 to i64
  %127 = ptrtoint ptr %113 to i64
  %128 = sub i64 %126, %127
  %129 = ashr exact i64 %128, 2
  %130 = icmp sgt i64 %129, -1
  call void @llvm.assume(i1 %130)
  %131 = icmp eq i64 %128, 9223372036854775804
  br i1 %131, label %132, label %134

132:                                              ; preds = %125
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #12
          to label %133 unwind label %149

133:                                              ; preds = %132
  unreachable

134:                                              ; preds = %125
  %135 = call i64 @llvm.umax.i64(i64 %129, i64 1)
  %136 = add nuw nsw i64 %135, %129
  %137 = call noundef i64 @llvm.umin.i64(i64 %136, i64 2305843009213693951)
  %138 = shl nuw nsw i64 %137, 2
  %139 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %138) #10
          to label %140 unwind label %147

140:                                              ; preds = %134
  %141 = getelementptr inbounds i8, ptr %139, i64 %128
  store i32 %116, ptr %141, align 4, !tbaa !12
  %142 = icmp sgt i64 %128, 0
  br i1 %142, label %143, label %144

143:                                              ; preds = %140
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %139, ptr align 4 %113, i64 %128, i1 false)
  br label %144

144:                                              ; preds = %143, %140
  call void @_ZdlPvm(ptr noundef nonnull %113, i64 noundef %128) #11
  %145 = getelementptr inbounds nuw i8, ptr %141, i64 4
  %146 = getelementptr inbounds nuw i32, ptr %139, i64 %137
  br label %151

147:                                              ; preds = %134
  %148 = landingpad { ptr, i32 }
          cleanup
  br label %172

149:                                              ; preds = %132
  %150 = landingpad { ptr, i32 }
          cleanup
  br label %172

151:                                              ; preds = %144, %123, %109
  %152 = phi ptr [ %113, %109 ], [ %139, %144 ], [ %113, %123 ]
  %153 = phi ptr [ %112, %109 ], [ %145, %144 ], [ %124, %123 ]
  %154 = phi ptr [ %111, %109 ], [ %146, %144 ], [ %111, %123 ]
  %155 = icmp samesign ugt i64 %110, 1
  br i1 %155, label %109, label %99, !llvm.loop !16

156:                                              ; preds = %31, %105
  %157 = phi ptr [ %108, %105 ], [ %34, %31 ]
  %158 = phi ptr [ %107, %105 ], [ %35, %31 ]
  %159 = phi ptr [ %106, %105 ], [ %32, %31 ]
  %160 = icmp eq ptr %157, %158
  br i1 %160, label %184, label %31

161:                                              ; preds = %67, %47
  %162 = phi ptr [ %49, %67 ], [ %50, %47 ]
  %163 = landingpad { ptr, i32 }
          cleanup
  br label %166

164:                                              ; preds = %65
  %165 = landingpad { ptr, i32 }
          cleanup
  br label %166

166:                                              ; preds = %164, %161
  %167 = phi ptr [ %162, %161 ], [ %49, %164 ]
  %168 = phi { ptr, i32 } [ %163, %161 ], [ %165, %164 ]
  %169 = icmp eq ptr %48, null
  br i1 %169, label %180, label %170

170:                                              ; preds = %166
  %171 = ptrtoint ptr %48 to i64
  br label %172

172:                                              ; preds = %170, %147, %149
  %173 = phi i64 [ %171, %170 ], [ %92, %147 ], [ %92, %149 ]
  %174 = phi ptr [ %167, %170 ], [ %50, %147 ], [ %50, %149 ]
  %175 = phi { ptr, i32 } [ %168, %170 ], [ %148, %147 ], [ %150, %149 ]
  %176 = phi ptr [ %32, %170 ], [ %111, %147 ], [ %111, %149 ]
  %177 = phi ptr [ %34, %170 ], [ %113, %147 ], [ %113, %149 ]
  %178 = ptrtoint ptr %174 to i64
  %179 = sub i64 %178, %173
  call void @_ZdlPvm(ptr noundef nonnull %48, i64 noundef %179) #11
  br label %180

180:                                              ; preds = %166, %172
  %181 = phi { ptr, i32 } [ %168, %166 ], [ %175, %172 ]
  %182 = phi ptr [ %32, %166 ], [ %176, %172 ]
  %183 = phi ptr [ %34, %166 ], [ %177, %172 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %3) #13
  br label %191

184:                                              ; preds = %156
  %185 = ptrtoint ptr %159 to i64
  %186 = ptrtoint ptr %157 to i64
  %187 = sub i64 %185, %186
  call void @_ZdlPvm(ptr noundef nonnull %157, i64 noundef %187) #11
  br label %188

188:                                              ; preds = %184, %17
  %189 = add nuw nsw i64 %18, 1
  %190 = icmp eq i64 %189, %15
  br i1 %190, label %16, label %17, !llvm.loop !17

191:                                              ; preds = %88, %180
  %192 = phi ptr [ %183, %180 ], [ %34, %88 ]
  %193 = phi ptr [ %182, %180 ], [ %32, %88 ]
  %194 = phi { ptr, i32 } [ %181, %180 ], [ %89, %88 ]
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %2) #13
  %195 = icmp eq ptr %192, null
  br i1 %195, label %200, label %196

196:                                              ; preds = %191
  %197 = ptrtoint ptr %193 to i64
  %198 = ptrtoint ptr %192 to i64
  %199 = sub i64 %197, %198
  call void @_ZdlPvm(ptr noundef nonnull %192, i64 noundef %199) #11
  br label %200

200:                                              ; preds = %41, %43, %191, %196
  %201 = phi { ptr, i32 } [ %194, %191 ], [ %194, %196 ], [ %42, %41 ], [ %44, %43 ]
  call void @_ZdlPvm(ptr noundef nonnull %9, i64 noundef %6) #11
  resume { ptr, i32 } %201

202:                                              ; preds = %16, %5, %1
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

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
attributes #12 = { cold noreturn }
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
!11 = !{!6, !6, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !6, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.mustprogress"}
!16 = distinct !{!16, !15}
!17 = distinct !{!17, !15}
