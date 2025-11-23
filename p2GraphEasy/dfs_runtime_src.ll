; ModuleID = 'dfs_runtime_src.cpp'
source_filename = "dfs_runtime_src.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::vector.0" = type { %"struct.std::_Vector_base.1" }
%"struct.std::_Vector_base.1" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%class.anon = type { ptr, ptr, ptr, ptr }
%struct.Graph = type { i64, i64, ptr, ptr }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [43 x i8] c"[dfs_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@.str.3 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @dfs_runtime_src(ptr noundef readonly %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca %"class.std::vector", align 8
  %8 = alloca %"class.std::vector.0", align 8
  %9 = alloca %class.anon, align 8
  %10 = icmp ne ptr %0, null
  %11 = icmp ne ptr %2, null
  %12 = and i1 %10, %11
  %13 = icmp ne ptr %3, null
  %14 = and i1 %12, %13
  br i1 %14, label %15, label %173

15:                                               ; preds = %4
  %16 = load i64, ptr %0, align 8, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %5) #12
  %17 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %18 = load ptr, ptr %17, align 8, !tbaa !9
  store ptr %18, ptr %5, align 8, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %6) #12
  %19 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %20 = load ptr, ptr %19, align 8, !tbaa !11
  store ptr %20, ptr %6, align 8, !tbaa !10
  %21 = icmp slt i64 %16, 0
  br i1 %21, label %171, label %22

22:                                               ; preds = %15
  %23 = icmp slt i32 %1, 0
  %24 = trunc i64 %16 to i32
  %25 = icmp sgt i32 %24, %1
  %26 = or i1 %23, %25
  br i1 %26, label %30, label %27

27:                                               ; preds = %22
  %28 = load ptr, ptr @stderr, align 8, !tbaa !10
  %29 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %28, ptr noundef nonnull @.str, i32 noundef %1, i64 noundef %16) #13
  br label %171

30:                                               ; preds = %22
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #12
  %31 = icmp eq i64 %16, 0
  br i1 %31, label %50, label %32

32:                                               ; preds = %30
  %33 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %16) #14
  store ptr %33, ptr %7, align 8, !tbaa !12
  %34 = getelementptr inbounds %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data", ptr %7, i64 0, i32 1
  %35 = getelementptr inbounds i8, ptr %33, i64 %16
  %36 = getelementptr inbounds %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data", ptr %7, i64 0, i32 2
  store ptr %35, ptr %36, align 8, !tbaa !14
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %33, i8 0, i64 %16, i1 false)
  store ptr %35, ptr %34, align 8, !tbaa !15
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #12
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %8, i8 0, i64 24, i1 false)
  %37 = icmp ugt i64 %16, 2305843009213693951
  br i1 %37, label %38, label %40

38:                                               ; preds = %32
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #15
          to label %39 unwind label %59

39:                                               ; preds = %38
  unreachable

40:                                               ; preds = %32
  %41 = shl nuw nsw i64 %16, 2
  %42 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %41) #14
          to label %43 unwind label %59

43:                                               ; preds = %40
  %44 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %8, i64 0, i32 1
  %45 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %8, i64 0, i32 2
  store ptr %42, ptr %8, align 8, !tbaa !16
  store ptr %42, ptr %44, align 8, !tbaa !18
  %46 = getelementptr inbounds i32, ptr %42, i64 %16
  store ptr %46, ptr %45, align 8, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %9) #12
  store ptr %7, ptr %9, align 8, !tbaa !10
  %47 = getelementptr inbounds %class.anon, ptr %9, i64 0, i32 1
  store ptr %8, ptr %47, align 8, !tbaa !10
  %48 = getelementptr inbounds %class.anon, ptr %9, i64 0, i32 2
  store ptr %5, ptr %48, align 8, !tbaa !10
  %49 = getelementptr inbounds %class.anon, ptr %9, i64 0, i32 3
  store ptr %6, ptr %49, align 8, !tbaa !10
  br i1 %23, label %54, label %58

50:                                               ; preds = %30
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %7, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #12
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %8, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %9) #12
  store ptr %7, ptr %9, align 8, !tbaa !10
  %51 = getelementptr inbounds %class.anon, ptr %9, i64 0, i32 1
  store ptr %8, ptr %51, align 8, !tbaa !10
  %52 = getelementptr inbounds %class.anon, ptr %9, i64 0, i32 2
  store ptr %5, ptr %52, align 8, !tbaa !10
  %53 = getelementptr inbounds %class.anon, ptr %9, i64 0, i32 3
  store ptr %6, ptr %53, align 8, !tbaa !10
  br i1 %23, label %76, label %58

54:                                               ; preds = %43
  %55 = icmp sgt i32 %24, 0
  br i1 %55, label %56, label %76

56:                                               ; preds = %54
  %57 = and i64 %16, 4294967295
  br label %63

58:                                               ; preds = %50, %43
  invoke fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr noundef nonnull align 8 dereferenceable(32) %9, i32 noundef %1)
          to label %76 unwind label %61

59:                                               ; preds = %38, %40
  %60 = landingpad { ptr, i32 }
          cleanup
  br label %165

61:                                               ; preds = %58
  %62 = landingpad { ptr, i32 }
          cleanup
  br label %160

63:                                               ; preds = %56, %73
  %64 = phi i64 [ 0, %56 ], [ %74, %73 ]
  %65 = load ptr, ptr %7, align 8, !tbaa !12
  %66 = getelementptr inbounds i8, ptr %65, i64 %64
  %67 = load i8, ptr %66, align 1, !tbaa !20
  %68 = icmp eq i8 %67, 0
  br i1 %68, label %69, label %73

69:                                               ; preds = %63
  %70 = trunc i64 %64 to i32
  invoke fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr noundef nonnull align 8 dereferenceable(32) %9, i32 noundef %70)
          to label %73 unwind label %71

71:                                               ; preds = %69
  %72 = landingpad { ptr, i32 }
          cleanup
  br label %160

73:                                               ; preds = %63, %69
  %74 = add nuw nsw i64 %64, 1
  %75 = icmp eq i64 %74, %57
  br i1 %75, label %76, label %63, !llvm.loop !21

76:                                               ; preds = %73, %50, %54, %58
  %77 = load ptr, ptr %8, align 8, !tbaa !10
  %78 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %8, i64 0, i32 1
  %79 = load ptr, ptr %78, align 8, !tbaa !10
  %80 = icmp eq ptr %77, %79
  br i1 %80, label %81, label %82

81:                                               ; preds = %76
  store ptr null, ptr %2, align 8, !tbaa !10
  store i32 0, ptr %3, align 4, !tbaa !23
  br label %149

82:                                               ; preds = %76
  %83 = ptrtoint ptr %79 to i64
  %84 = ptrtoint ptr %77 to i64
  %85 = sub i64 %83, %84
  %86 = ashr exact i64 %85, 2
  %87 = icmp sgt i64 %86, -1
  call void @llvm.assume(i1 %87)
  %88 = call noalias ptr @malloc(i64 noundef %85) #16
  %89 = icmp eq ptr %88, null
  br i1 %89, label %149, label %90

90:                                               ; preds = %82
  %91 = ptrtoint ptr %88 to i64
  %92 = call i64 @llvm.umax.i64(i64 %86, i64 1)
  %93 = icmp ult i64 %86, 8
  %94 = sub i64 %91, %84
  %95 = icmp ult i64 %94, 32
  %96 = or i1 %93, %95
  br i1 %96, label %111, label %97

97:                                               ; preds = %90
  %98 = and i64 %92, 9223372036854775800
  br label %99

99:                                               ; preds = %99, %97
  %100 = phi i64 [ 0, %97 ], [ %107, %99 ]
  %101 = getelementptr inbounds i32, ptr %77, i64 %100
  %102 = getelementptr inbounds i32, ptr %101, i64 4
  %103 = load <4 x i32>, ptr %101, align 4, !tbaa !23
  %104 = load <4 x i32>, ptr %102, align 4, !tbaa !23
  %105 = getelementptr inbounds i32, ptr %88, i64 %100
  %106 = getelementptr inbounds i32, ptr %105, i64 4
  store <4 x i32> %103, ptr %105, align 4, !tbaa !23
  store <4 x i32> %104, ptr %106, align 4, !tbaa !23
  %107 = add nuw i64 %100, 8
  %108 = icmp eq i64 %107, %98
  br i1 %108, label %109, label %99, !llvm.loop !25

109:                                              ; preds = %99
  %110 = icmp eq i64 %92, %98
  br i1 %110, label %128, label %111

111:                                              ; preds = %90, %109
  %112 = phi i64 [ 0, %90 ], [ %98, %109 ]
  %113 = and i64 %92, 3
  %114 = icmp eq i64 %113, 0
  br i1 %114, label %124, label %115

115:                                              ; preds = %111, %115
  %116 = phi i64 [ %121, %115 ], [ %112, %111 ]
  %117 = phi i64 [ %122, %115 ], [ 0, %111 ]
  %118 = getelementptr inbounds i32, ptr %77, i64 %116
  %119 = load i32, ptr %118, align 4, !tbaa !23
  %120 = getelementptr inbounds i32, ptr %88, i64 %116
  store i32 %119, ptr %120, align 4, !tbaa !23
  %121 = add nuw nsw i64 %116, 1
  %122 = add i64 %117, 1
  %123 = icmp eq i64 %122, %113
  br i1 %123, label %124, label %115, !llvm.loop !28

124:                                              ; preds = %115, %111
  %125 = phi i64 [ %112, %111 ], [ %121, %115 ]
  %126 = sub nsw i64 %112, %92
  %127 = icmp ugt i64 %126, -4
  br i1 %127, label %128, label %130

128:                                              ; preds = %124, %130, %109
  store ptr %88, ptr %2, align 8, !tbaa !10
  %129 = trunc i64 %86 to i32
  store i32 %129, ptr %3, align 4, !tbaa !23
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %9) #12
  br label %152

130:                                              ; preds = %124, %130
  %131 = phi i64 [ %147, %130 ], [ %125, %124 ]
  %132 = getelementptr inbounds i32, ptr %77, i64 %131
  %133 = load i32, ptr %132, align 4, !tbaa !23
  %134 = getelementptr inbounds i32, ptr %88, i64 %131
  store i32 %133, ptr %134, align 4, !tbaa !23
  %135 = add nuw nsw i64 %131, 1
  %136 = getelementptr inbounds i32, ptr %77, i64 %135
  %137 = load i32, ptr %136, align 4, !tbaa !23
  %138 = getelementptr inbounds i32, ptr %88, i64 %135
  store i32 %137, ptr %138, align 4, !tbaa !23
  %139 = add nuw nsw i64 %131, 2
  %140 = getelementptr inbounds i32, ptr %77, i64 %139
  %141 = load i32, ptr %140, align 4, !tbaa !23
  %142 = getelementptr inbounds i32, ptr %88, i64 %139
  store i32 %141, ptr %142, align 4, !tbaa !23
  %143 = add nuw nsw i64 %131, 3
  %144 = getelementptr inbounds i32, ptr %77, i64 %143
  %145 = load i32, ptr %144, align 4, !tbaa !23
  %146 = getelementptr inbounds i32, ptr %88, i64 %143
  store i32 %145, ptr %146, align 4, !tbaa !23
  %147 = add nuw nsw i64 %131, 4
  %148 = icmp eq i64 %86, %147
  br i1 %148, label %128, label %130, !llvm.loop !30

149:                                              ; preds = %81, %82
  %150 = phi i32 [ 4, %82 ], [ 0, %81 ]
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %9) #12
  %151 = icmp eq ptr %77, null
  br i1 %151, label %154, label %152

152:                                              ; preds = %128, %149
  %153 = phi i32 [ 0, %128 ], [ %150, %149 ]
  call void @_ZdlPv(ptr noundef nonnull %77) #17
  br label %154

154:                                              ; preds = %149, %152
  %155 = phi i32 [ %150, %149 ], [ %153, %152 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #12
  %156 = load ptr, ptr %7, align 8, !tbaa !12
  %157 = icmp eq ptr %156, null
  br i1 %157, label %159, label %158

158:                                              ; preds = %154
  call void @_ZdlPv(ptr noundef nonnull %156) #17
  br label %159

159:                                              ; preds = %154, %158
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #12
  br label %171

160:                                              ; preds = %61, %71
  %161 = phi { ptr, i32 } [ %62, %61 ], [ %72, %71 ]
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %9) #12
  %162 = load ptr, ptr %8, align 8, !tbaa !16
  %163 = icmp eq ptr %162, null
  br i1 %163, label %165, label %164

164:                                              ; preds = %160
  call void @_ZdlPv(ptr noundef nonnull %162) #17
  br label %165

165:                                              ; preds = %59, %160, %164
  %166 = phi { ptr, i32 } [ %60, %59 ], [ %161, %160 ], [ %161, %164 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #12
  %167 = load ptr, ptr %7, align 8, !tbaa !12
  %168 = icmp eq ptr %167, null
  br i1 %168, label %170, label %169

169:                                              ; preds = %165
  call void @_ZdlPv(ptr noundef nonnull %167) #17
  br label %170

170:                                              ; preds = %169, %165
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #12
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6) #12
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %5) #12
  resume { ptr, i32 } %166

171:                                              ; preds = %15, %159, %27
  %172 = phi i32 [ 3, %27 ], [ %155, %159 ], [ 2, %15 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6) #12
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %5) #12
  br label %173

173:                                              ; preds = %4, %171
  %174 = phi i32 [ %172, %171 ], [ 1, %4 ]
  ret i32 %174
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: inlinehint mustprogress uwtable
define internal fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) %0, i32 noundef %1) unnamed_addr #3 align 2 personality ptr @__gxx_personality_v0 {
  %3 = tail call noalias noundef nonnull dereferenceable(256) ptr @_Znwm(i64 noundef 256) #14
  %4 = getelementptr inbounds i32, ptr %3, i64 64
  store i32 %1, ptr %3, align 4, !tbaa !23
  %5 = getelementptr inbounds i32, ptr %3, i64 1
  %6 = getelementptr inbounds %class.anon, ptr %0, i64 0, i32 1
  %7 = getelementptr inbounds %class.anon, ptr %0, i64 0, i32 2
  %8 = getelementptr inbounds %class.anon, ptr %0, i64 0, i32 3
  br label %9

9:                                                ; preds = %2, %122
  %10 = phi ptr [ %3, %2 ], [ %125, %122 ]
  %11 = phi ptr [ %5, %2 ], [ %124, %122 ]
  %12 = phi ptr [ %4, %2 ], [ %123, %122 ]
  %13 = getelementptr inbounds i32, ptr %11, i64 -1
  %14 = load i32, ptr %13, align 4, !tbaa !23
  %15 = load ptr, ptr %0, align 8, !tbaa !31
  %16 = sext i32 %14 to i64
  %17 = load ptr, ptr %15, align 8, !tbaa !12
  %18 = getelementptr inbounds i8, ptr %17, i64 %16
  %19 = load i8, ptr %18, align 1, !tbaa !20
  %20 = icmp eq i8 %19, 0
  br i1 %20, label %21, label %122, !llvm.loop !33

21:                                               ; preds = %9
  store i8 1, ptr %18, align 1, !tbaa !20
  %22 = load ptr, ptr %6, align 8, !tbaa !34
  %23 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %22, i64 0, i32 1
  %24 = load ptr, ptr %23, align 8, !tbaa !10
  %25 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %22, i64 0, i32 2
  %26 = load ptr, ptr %25, align 8, !tbaa !19
  %27 = icmp eq ptr %24, %26
  br i1 %27, label %30, label %28

28:                                               ; preds = %21
  store i32 %14, ptr %24, align 4, !tbaa !23
  %29 = getelementptr inbounds i32, ptr %24, i64 1
  store ptr %29, ptr %23, align 8, !tbaa !18
  br label %56

30:                                               ; preds = %21
  %31 = load ptr, ptr %22, align 8, !tbaa !10
  %32 = ptrtoint ptr %24 to i64
  %33 = ptrtoint ptr %31 to i64
  %34 = sub i64 %32, %33
  %35 = ashr exact i64 %34, 2
  %36 = icmp sgt i64 %35, -1
  tail call void @llvm.assume(i1 %36)
  %37 = icmp eq i64 %34, 9223372036854775804
  br i1 %37, label %38, label %40

38:                                               ; preds = %30
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #15
          to label %39 unwind label %66

39:                                               ; preds = %38
  unreachable

40:                                               ; preds = %30
  %41 = tail call i64 @llvm.umax.i64(i64 %35, i64 1)
  %42 = add nuw nsw i64 %41, %35
  %43 = tail call noundef i64 @llvm.umin.i64(i64 %42, i64 2305843009213693951)
  %44 = shl nuw nsw i64 %43, 2
  %45 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %44) #14
          to label %46 unwind label %64

46:                                               ; preds = %40
  %47 = getelementptr inbounds i8, ptr %45, i64 %34
  store i32 %14, ptr %47, align 4, !tbaa !23
  %48 = icmp sgt i64 %34, 0
  br i1 %48, label %49, label %50

49:                                               ; preds = %46
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %45, ptr align 4 %31, i64 %34, i1 false)
  br label %50

50:                                               ; preds = %49, %46
  %51 = icmp eq ptr %31, null
  br i1 %51, label %53, label %52

52:                                               ; preds = %50
  tail call void @_ZdlPv(ptr noundef nonnull %31) #17
  br label %53

53:                                               ; preds = %52, %50
  %54 = getelementptr inbounds i32, ptr %47, i64 1
  store ptr %45, ptr %22, align 8, !tbaa !16
  store ptr %54, ptr %23, align 8, !tbaa !18
  %55 = getelementptr inbounds i32, ptr %45, i64 %43
  store ptr %55, ptr %25, align 8, !tbaa !19
  br label %56

56:                                               ; preds = %53, %28
  %57 = load ptr, ptr %7, align 8, !tbaa !35
  %58 = load ptr, ptr %57, align 8, !tbaa !10
  %59 = getelementptr inbounds i64, ptr %58, i64 %16
  %60 = load i64, ptr %59, align 8, !tbaa !36
  %61 = getelementptr i64, ptr %59, i64 1
  %62 = load i64, ptr %61, align 8, !tbaa !36
  %63 = icmp sgt i64 %62, %60
  br i1 %63, label %68, label %122

64:                                               ; preds = %40
  %65 = landingpad { ptr, i32 }
          cleanup
  br label %131

66:                                               ; preds = %38
  %67 = landingpad { ptr, i32 }
          cleanup
  br label %131

68:                                               ; preds = %56, %117
  %69 = phi i64 [ %73, %117 ], [ %62, %56 ]
  %70 = phi ptr [ %120, %117 ], [ %10, %56 ]
  %71 = phi ptr [ %119, %117 ], [ %13, %56 ]
  %72 = phi ptr [ %118, %117 ], [ %12, %56 ]
  %73 = add nsw i64 %69, -1
  %74 = load ptr, ptr %8, align 8, !tbaa !37
  %75 = load ptr, ptr %74, align 8, !tbaa !10
  %76 = getelementptr inbounds i32, ptr %75, i64 %73
  %77 = load i32, ptr %76, align 4, !tbaa !23
  %78 = load ptr, ptr %0, align 8, !tbaa !31
  %79 = sext i32 %77 to i64
  %80 = load ptr, ptr %78, align 8, !tbaa !12
  %81 = getelementptr inbounds i8, ptr %80, i64 %79
  %82 = load i8, ptr %81, align 1, !tbaa !20
  %83 = icmp eq i8 %82, 0
  br i1 %83, label %84, label %117

84:                                               ; preds = %68
  %85 = icmp eq ptr %71, %72
  br i1 %85, label %88, label %86

86:                                               ; preds = %84
  store i32 %77, ptr %71, align 4, !tbaa !23
  %87 = getelementptr inbounds i32, ptr %71, i64 1
  br label %117

88:                                               ; preds = %84
  %89 = ptrtoint ptr %71 to i64
  %90 = ptrtoint ptr %70 to i64
  %91 = sub i64 %89, %90
  %92 = ashr exact i64 %91, 2
  %93 = icmp sgt i64 %92, -1
  tail call void @llvm.assume(i1 %93)
  %94 = icmp eq i64 %91, 9223372036854775804
  br i1 %94, label %95, label %97

95:                                               ; preds = %88
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #15
          to label %96 unwind label %115

96:                                               ; preds = %95
  unreachable

97:                                               ; preds = %88
  %98 = tail call i64 @llvm.umax.i64(i64 %92, i64 1)
  %99 = add nuw nsw i64 %98, %92
  %100 = tail call noundef i64 @llvm.umin.i64(i64 %99, i64 2305843009213693951)
  %101 = shl nuw nsw i64 %100, 2
  %102 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %101) #14
          to label %103 unwind label %113

103:                                              ; preds = %97
  %104 = getelementptr inbounds i8, ptr %102, i64 %91
  store i32 %77, ptr %104, align 4, !tbaa !23
  %105 = icmp sgt i64 %91, 0
  br i1 %105, label %106, label %107

106:                                              ; preds = %103
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %102, ptr align 4 %70, i64 %91, i1 false)
  br label %107

107:                                              ; preds = %106, %103
  %108 = icmp eq ptr %70, null
  br i1 %108, label %110, label %109

109:                                              ; preds = %107
  tail call void @_ZdlPv(ptr noundef nonnull %70) #17
  br label %110

110:                                              ; preds = %109, %107
  %111 = getelementptr inbounds i32, ptr %104, i64 1
  %112 = getelementptr inbounds i32, ptr %102, i64 %100
  br label %117

113:                                              ; preds = %97
  %114 = landingpad { ptr, i32 }
          cleanup
  br label %131

115:                                              ; preds = %95
  %116 = landingpad { ptr, i32 }
          cleanup
  br label %131

117:                                              ; preds = %110, %86, %68
  %118 = phi ptr [ %72, %68 ], [ %112, %110 ], [ %72, %86 ]
  %119 = phi ptr [ %71, %68 ], [ %111, %110 ], [ %87, %86 ]
  %120 = phi ptr [ %70, %68 ], [ %102, %110 ], [ %70, %86 ]
  %121 = icmp sgt i64 %73, %60
  br i1 %121, label %68, label %122, !llvm.loop !38

122:                                              ; preds = %117, %56, %9
  %123 = phi ptr [ %12, %9 ], [ %12, %56 ], [ %118, %117 ]
  %124 = phi ptr [ %13, %9 ], [ %13, %56 ], [ %119, %117 ]
  %125 = phi ptr [ %10, %9 ], [ %10, %56 ], [ %120, %117 ]
  %126 = icmp eq ptr %125, %124
  br i1 %126, label %127, label %9

127:                                              ; preds = %122
  %128 = icmp eq ptr %124, null
  br i1 %128, label %130, label %129

129:                                              ; preds = %127
  tail call void @_ZdlPv(ptr noundef nonnull %124) #17
  br label %130

130:                                              ; preds = %127, %129
  ret void

131:                                              ; preds = %113, %115, %64, %66
  %132 = phi ptr [ %10, %64 ], [ %10, %66 ], [ %70, %113 ], [ %70, %115 ]
  %133 = phi { ptr, i32 } [ %65, %64 ], [ %67, %66 ], [ %114, %113 ], [ %116, %115 ]
  %134 = icmp eq ptr %132, null
  br i1 %134, label %136, label %135

135:                                              ; preds = %131
  tail call void @_ZdlPv(ptr noundef nonnull %132) #17
  br label %136

136:                                              ; preds = %131, %135
  resume { ptr, i32 } %133
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #8

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #9

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #10

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #11

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { inlinehint mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #10 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #11 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #12 = { nounwind }
attributes #13 = { cold }
attributes #14 = { builtin allocsize(0) }
attributes #15 = { cold noreturn }
attributes #16 = { nounwind allocsize(0) }
attributes #17 = { builtin nounwind }

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
!10 = !{!8, !8, i64 0}
!11 = !{!4, !8, i64 24}
!12 = !{!13, !8, i64 0}
!13 = !{!"_ZTSNSt12_Vector_baseIcSaIcEE17_Vector_impl_dataE", !8, i64 0, !8, i64 8, !8, i64 16}
!14 = !{!13, !8, i64 16}
!15 = !{!13, !8, i64 8}
!16 = !{!17, !8, i64 0}
!17 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !8, i64 0, !8, i64 8, !8, i64 16}
!18 = !{!17, !8, i64 8}
!19 = !{!17, !8, i64 16}
!20 = !{!6, !6, i64 0}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.mustprogress"}
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !6, i64 0}
!25 = distinct !{!25, !22, !26, !27}
!26 = !{!"llvm.loop.isvectorized", i32 1}
!27 = !{!"llvm.loop.unroll.runtime.disable"}
!28 = distinct !{!28, !29}
!29 = !{!"llvm.loop.unroll.disable"}
!30 = distinct !{!30, !22, !26}
!31 = !{!32, !8, i64 0}
!32 = !{!"_ZTSZ15dfs_runtime_srcE3$_0", !8, i64 0, !8, i64 8, !8, i64 16, !8, i64 24}
!33 = distinct !{!33, !22}
!34 = !{!32, !8, i64 8}
!35 = !{!32, !8, i64 16}
!36 = !{!5, !5, i64 0}
!37 = !{!32, !8, i64 24}
!38 = distinct !{!38, !22}
