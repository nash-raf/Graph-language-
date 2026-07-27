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
%class.anon = type { ptr, ptr, ptr }
%struct.AutoNeighborIter = type { ptr, i64, i64, i64, i32, i32, i32, i32 }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [43 x i8] c"[dfs_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@.str.3 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local range(i32 0, 5) i32 @dfs_runtime_src(ptr noundef %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = alloca ptr, align 8
  %6 = alloca %"class.std::vector", align 8
  %7 = alloca %"class.std::vector.0", align 8
  %8 = alloca %class.anon, align 8
  store ptr %0, ptr %5, align 8, !tbaa !3
  %9 = icmp ne ptr %0, null
  %10 = icmp ne ptr %2, null
  %11 = and i1 %9, %10
  %12 = icmp ne ptr %3, null
  %13 = and i1 %11, %12
  br i1 %13, label %14, label %186

14:                                               ; preds = %4
  %15 = load i64, ptr %0, align 8, !tbaa !8
  %16 = icmp slt i64 %15, 0
  br i1 %16, label %186, label %17

17:                                               ; preds = %14
  %18 = icmp slt i32 %1, 0
  %19 = trunc i64 %15 to i32
  %20 = icmp slt i32 %1, %19
  %21 = or i1 %18, %20
  br i1 %21, label %25, label %22

22:                                               ; preds = %17
  %23 = load ptr, ptr @stderr, align 8, !tbaa !13
  %24 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %23, ptr noundef nonnull @.str, i32 noundef %1, i64 noundef %15) #13
  br label %186

25:                                               ; preds = %17
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %6) #14
  %26 = icmp eq i64 %15, 0
  br i1 %26, label %44, label %27

27:                                               ; preds = %25
  %28 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %15) #15
  store ptr %28, ptr %6, align 8, !tbaa !15
  %29 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %30 = getelementptr inbounds nuw i8, ptr %28, i64 %15
  %31 = getelementptr inbounds nuw i8, ptr %6, i64 16
  store ptr %30, ptr %31, align 8, !tbaa !18
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %28, i8 0, i64 %15, i1 false)
  store ptr %30, ptr %29, align 8, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %7, i8 0, i64 24, i1 false)
  %32 = icmp samesign ugt i64 %15, 2305843009213693951
  br i1 %32, label %33, label %35

33:                                               ; preds = %27
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #16
          to label %34 unwind label %54

34:                                               ; preds = %33
  unreachable

35:                                               ; preds = %27
  %36 = getelementptr inbounds nuw i8, ptr %7, i64 16
  %37 = shl nuw nsw i64 %15, 2
  %38 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %37) #15
          to label %39 unwind label %54

39:                                               ; preds = %35
  %40 = getelementptr inbounds nuw i8, ptr %7, i64 8
  store ptr %38, ptr %7, align 8, !tbaa !20
  store ptr %38, ptr %40, align 8, !tbaa !22
  %41 = getelementptr inbounds nuw i32, ptr %38, i64 %15
  store ptr %41, ptr %36, align 8, !tbaa !23
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #14
  store ptr %6, ptr %8, align 8, !tbaa !24
  %42 = getelementptr inbounds nuw i8, ptr %8, i64 8
  store ptr %7, ptr %42, align 8, !tbaa !26
  %43 = getelementptr inbounds nuw i8, ptr %8, i64 16
  store ptr %5, ptr %43, align 8, !tbaa !28
  br i1 %18, label %48, label %52

44:                                               ; preds = %25
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %6, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %7, i8 0, i64 24, i1 false)
  %45 = getelementptr inbounds nuw i8, ptr %7, i64 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #14
  store ptr %6, ptr %8, align 8, !tbaa !24
  %46 = getelementptr inbounds nuw i8, ptr %8, i64 8
  store ptr %7, ptr %46, align 8, !tbaa !26
  %47 = getelementptr inbounds nuw i8, ptr %8, i64 16
  store ptr %5, ptr %47, align 8, !tbaa !28
  br i1 %18, label %71, label %52

48:                                               ; preds = %39
  %49 = icmp sgt i32 %19, 0
  br i1 %49, label %50, label %71

50:                                               ; preds = %48
  %51 = and i64 %15, 2147483647
  br label %58

52:                                               ; preds = %44, %39
  %53 = phi ptr [ %45, %44 ], [ %36, %39 ]
  invoke fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr noundef nonnull align 8 dereferenceable(24) %8, i32 noundef %1)
          to label %71 unwind label %56

54:                                               ; preds = %33, %35
  %55 = landingpad { ptr, i32 }
          cleanup
  br label %175

56:                                               ; preds = %52
  %57 = landingpad { ptr, i32 }
          cleanup
  br label %165

58:                                               ; preds = %50, %68
  %59 = phi i64 [ 0, %50 ], [ %69, %68 ]
  %60 = load ptr, ptr %6, align 8, !tbaa !15
  %61 = getelementptr inbounds nuw i8, ptr %60, i64 %59
  %62 = load i8, ptr %61, align 1, !tbaa !30
  %63 = icmp eq i8 %62, 0
  br i1 %63, label %64, label %68

64:                                               ; preds = %58
  %65 = trunc nuw nsw i64 %59 to i32
  invoke fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr noundef nonnull align 8 dereferenceable(24) %8, i32 noundef %65)
          to label %68 unwind label %66

66:                                               ; preds = %64
  %67 = landingpad { ptr, i32 }
          cleanup
  br label %165

68:                                               ; preds = %58, %64
  %69 = add nuw nsw i64 %59, 1
  %70 = icmp eq i64 %69, %51
  br i1 %70, label %71, label %58, !llvm.loop !31

71:                                               ; preds = %68, %44, %48, %52
  %72 = phi ptr [ %36, %48 ], [ %53, %52 ], [ %45, %44 ], [ %36, %68 ]
  %73 = load ptr, ptr %7, align 8, !tbaa !33
  %74 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %75 = load ptr, ptr %74, align 8, !tbaa !33
  %76 = icmp eq ptr %73, %75
  br i1 %76, label %77, label %78

77:                                               ; preds = %71
  store ptr null, ptr %2, align 8, !tbaa !33
  store i32 0, ptr %3, align 4, !tbaa !34
  br label %145

78:                                               ; preds = %71
  %79 = ptrtoint ptr %75 to i64
  %80 = ptrtoint ptr %73 to i64
  %81 = sub i64 %79, %80
  %82 = ashr exact i64 %81, 2
  %83 = icmp sgt i64 %82, -1
  call void @llvm.assume(i1 %83)
  %84 = call noalias ptr @malloc(i64 noundef %81) #17
  %85 = icmp eq ptr %84, null
  br i1 %85, label %145, label %86

86:                                               ; preds = %78
  %87 = ptrtoint ptr %84 to i64
  %88 = call i64 @llvm.umax.i64(i64 %82, i64 1)
  %89 = icmp samesign ult i64 %82, 8
  %90 = sub i64 %87, %80
  %91 = icmp ult i64 %90, 32
  %92 = or i1 %89, %91
  br i1 %92, label %107, label %93

93:                                               ; preds = %86
  %94 = and i64 %88, 9223372036854775800
  br label %95

95:                                               ; preds = %95, %93
  %96 = phi i64 [ 0, %93 ], [ %103, %95 ]
  %97 = getelementptr inbounds nuw i32, ptr %73, i64 %96
  %98 = getelementptr inbounds nuw i8, ptr %97, i64 16
  %99 = load <4 x i32>, ptr %97, align 4, !tbaa !34
  %100 = load <4 x i32>, ptr %98, align 4, !tbaa !34
  %101 = getelementptr inbounds nuw i32, ptr %84, i64 %96
  %102 = getelementptr inbounds nuw i8, ptr %101, i64 16
  store <4 x i32> %99, ptr %101, align 4, !tbaa !34
  store <4 x i32> %100, ptr %102, align 4, !tbaa !34
  %103 = add nuw i64 %96, 8
  %104 = icmp eq i64 %103, %94
  br i1 %104, label %105, label %95, !llvm.loop !36

105:                                              ; preds = %95
  %106 = icmp eq i64 %82, %94
  br i1 %106, label %124, label %107

107:                                              ; preds = %86, %105
  %108 = phi i64 [ 0, %86 ], [ %94, %105 ]
  %109 = and i64 %88, 3
  %110 = icmp eq i64 %109, 0
  br i1 %110, label %120, label %111

111:                                              ; preds = %107, %111
  %112 = phi i64 [ %117, %111 ], [ %108, %107 ]
  %113 = phi i64 [ %118, %111 ], [ 0, %107 ]
  %114 = getelementptr inbounds nuw i32, ptr %73, i64 %112
  %115 = load i32, ptr %114, align 4, !tbaa !34
  %116 = getelementptr inbounds nuw i32, ptr %84, i64 %112
  store i32 %115, ptr %116, align 4, !tbaa !34
  %117 = add nuw nsw i64 %112, 1
  %118 = add i64 %113, 1
  %119 = icmp eq i64 %118, %109
  br i1 %119, label %120, label %111, !llvm.loop !39

120:                                              ; preds = %111, %107
  %121 = phi i64 [ %108, %107 ], [ %117, %111 ]
  %122 = sub nsw i64 %108, %88
  %123 = icmp ugt i64 %122, -4
  br i1 %123, label %124, label %126

124:                                              ; preds = %120, %126, %105
  store ptr %84, ptr %2, align 8, !tbaa !33
  %125 = trunc i64 %82 to i32
  store i32 %125, ptr %3, align 4, !tbaa !34
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #14
  br label %148

126:                                              ; preds = %120, %126
  %127 = phi i64 [ %143, %126 ], [ %121, %120 ]
  %128 = getelementptr inbounds nuw i32, ptr %73, i64 %127
  %129 = load i32, ptr %128, align 4, !tbaa !34
  %130 = getelementptr inbounds nuw i32, ptr %84, i64 %127
  store i32 %129, ptr %130, align 4, !tbaa !34
  %131 = add nuw nsw i64 %127, 1
  %132 = getelementptr inbounds nuw i32, ptr %73, i64 %131
  %133 = load i32, ptr %132, align 4, !tbaa !34
  %134 = getelementptr inbounds nuw i32, ptr %84, i64 %131
  store i32 %133, ptr %134, align 4, !tbaa !34
  %135 = add nuw nsw i64 %127, 2
  %136 = getelementptr inbounds nuw i32, ptr %73, i64 %135
  %137 = load i32, ptr %136, align 4, !tbaa !34
  %138 = getelementptr inbounds nuw i32, ptr %84, i64 %135
  store i32 %137, ptr %138, align 4, !tbaa !34
  %139 = add nuw nsw i64 %127, 3
  %140 = getelementptr inbounds nuw i32, ptr %73, i64 %139
  %141 = load i32, ptr %140, align 4, !tbaa !34
  %142 = getelementptr inbounds nuw i32, ptr %84, i64 %139
  store i32 %141, ptr %142, align 4, !tbaa !34
  %143 = add nuw nsw i64 %127, 4
  %144 = icmp eq i64 %82, %143
  br i1 %144, label %124, label %126, !llvm.loop !41

145:                                              ; preds = %77, %78
  %146 = phi i32 [ 4, %78 ], [ 0, %77 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #14
  %147 = icmp eq ptr %73, null
  br i1 %147, label %154, label %148

148:                                              ; preds = %124, %145
  %149 = phi i32 [ 0, %124 ], [ %146, %145 ]
  %150 = load ptr, ptr %72, align 8, !tbaa !23
  %151 = ptrtoint ptr %150 to i64
  %152 = ptrtoint ptr %73 to i64
  %153 = sub i64 %151, %152
  call void @_ZdlPvm(ptr noundef nonnull %73, i64 noundef %153) #18
  br label %154

154:                                              ; preds = %145, %148
  %155 = phi i32 [ %146, %145 ], [ %149, %148 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #14
  %156 = load ptr, ptr %6, align 8, !tbaa !15
  %157 = icmp eq ptr %156, null
  br i1 %157, label %164, label %158

158:                                              ; preds = %154
  %159 = getelementptr inbounds nuw i8, ptr %6, i64 16
  %160 = load ptr, ptr %159, align 8, !tbaa !18
  %161 = ptrtoint ptr %160 to i64
  %162 = ptrtoint ptr %156 to i64
  %163 = sub i64 %161, %162
  call void @_ZdlPvm(ptr noundef nonnull %156, i64 noundef %163) #18
  br label %164

164:                                              ; preds = %154, %158
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #14
  br label %186

165:                                              ; preds = %56, %66
  %166 = phi { ptr, i32 } [ %57, %56 ], [ %67, %66 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #14
  %167 = load ptr, ptr %7, align 8, !tbaa !20
  %168 = icmp eq ptr %167, null
  br i1 %168, label %175, label %169

169:                                              ; preds = %165
  %170 = getelementptr inbounds nuw i8, ptr %7, i64 16
  %171 = load ptr, ptr %170, align 8, !tbaa !23
  %172 = ptrtoint ptr %171 to i64
  %173 = ptrtoint ptr %167 to i64
  %174 = sub i64 %172, %173
  call void @_ZdlPvm(ptr noundef nonnull %167, i64 noundef %174) #18
  br label %175

175:                                              ; preds = %54, %165, %169
  %176 = phi { ptr, i32 } [ %55, %54 ], [ %166, %165 ], [ %166, %169 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #14
  %177 = load ptr, ptr %6, align 8, !tbaa !15
  %178 = icmp eq ptr %177, null
  br i1 %178, label %185, label %179

179:                                              ; preds = %175
  %180 = getelementptr inbounds nuw i8, ptr %6, i64 16
  %181 = load ptr, ptr %180, align 8, !tbaa !18
  %182 = ptrtoint ptr %181 to i64
  %183 = ptrtoint ptr %177 to i64
  %184 = sub i64 %182, %183
  call void @_ZdlPvm(ptr noundef nonnull %177, i64 noundef %184) #18
  br label %185

185:                                              ; preds = %179, %175
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #14
  resume { ptr, i32 } %176

186:                                              ; preds = %22, %164, %14, %4
  %187 = phi i32 [ 1, %4 ], [ 3, %22 ], [ %155, %164 ], [ 2, %14 ]
  ret i32 %187
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: inlinehint mustprogress uwtable
define internal fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %0, i32 noundef %1) unnamed_addr #3 align 2 personality ptr @__gxx_personality_v0 {
  %3 = alloca %struct.AutoNeighborIter, align 8
  %4 = alloca i32, align 4
  %5 = tail call noalias noundef nonnull dereferenceable(256) ptr @_Znwm(i64 noundef 256) #15
  %6 = getelementptr inbounds nuw i8, ptr %5, i64 256
  store i32 %1, ptr %5, align 4, !tbaa !34
  %7 = getelementptr inbounds nuw i8, ptr %5, i64 4
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 16
  br label %10

10:                                               ; preds = %2, %176
  %11 = phi ptr [ %5, %2 ], [ %179, %176 ]
  %12 = phi ptr [ %6, %2 ], [ %178, %176 ]
  %13 = phi ptr [ %7, %2 ], [ %177, %176 ]
  %14 = getelementptr inbounds i8, ptr %13, i64 -4
  %15 = load i32, ptr %14, align 4, !tbaa !34
  %16 = load ptr, ptr %0, align 8, !tbaa !42
  %17 = sext i32 %15 to i64
  %18 = load ptr, ptr %16, align 8, !tbaa !15
  %19 = getelementptr inbounds nuw i8, ptr %18, i64 %17
  %20 = load i8, ptr %19, align 1, !tbaa !30
  %21 = icmp eq i8 %20, 0
  br i1 %21, label %22, label %176, !llvm.loop !44

22:                                               ; preds = %10
  store i8 1, ptr %19, align 1, !tbaa !30
  %23 = load ptr, ptr %8, align 8, !tbaa !45
  %24 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %25 = load ptr, ptr %24, align 8, !tbaa !22
  %26 = getelementptr inbounds nuw i8, ptr %23, i64 16
  %27 = load ptr, ptr %26, align 8, !tbaa !23
  %28 = icmp eq ptr %25, %27
  br i1 %28, label %31, label %29

29:                                               ; preds = %22
  store i32 %15, ptr %25, align 4, !tbaa !34
  %30 = getelementptr inbounds nuw i8, ptr %25, i64 4
  store ptr %30, ptr %24, align 8, !tbaa !22
  br label %57

31:                                               ; preds = %22
  %32 = load ptr, ptr %23, align 8, !tbaa !20
  %33 = ptrtoint ptr %25 to i64
  %34 = ptrtoint ptr %32 to i64
  %35 = sub i64 %33, %34
  %36 = ashr exact i64 %35, 2
  %37 = icmp sgt i64 %36, -1
  call void @llvm.assume(i1 %37)
  %38 = icmp eq i64 %35, 9223372036854775804
  br i1 %38, label %39, label %41

39:                                               ; preds = %31
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #16
          to label %40 unwind label %104

40:                                               ; preds = %39
  unreachable

41:                                               ; preds = %31
  %42 = call i64 @llvm.umax.i64(i64 %36, i64 1)
  %43 = add nuw nsw i64 %42, %36
  %44 = call noundef i64 @llvm.umin.i64(i64 %43, i64 2305843009213693951)
  %45 = shl nuw nsw i64 %44, 2
  %46 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %45) #15
          to label %47 unwind label %102

47:                                               ; preds = %41
  %48 = getelementptr inbounds i8, ptr %46, i64 %35
  store i32 %15, ptr %48, align 4, !tbaa !34
  %49 = icmp sgt i64 %35, 0
  br i1 %49, label %50, label %51

50:                                               ; preds = %47
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %46, ptr align 4 %32, i64 %35, i1 false)
  br label %51

51:                                               ; preds = %50, %47
  %52 = icmp eq ptr %32, null
  br i1 %52, label %54, label %53

53:                                               ; preds = %51
  call void @_ZdlPvm(ptr noundef nonnull %32, i64 noundef %35) #18
  br label %54

54:                                               ; preds = %53, %51
  %55 = getelementptr inbounds nuw i8, ptr %48, i64 4
  store ptr %46, ptr %23, align 8, !tbaa !20
  store ptr %55, ptr %24, align 8, !tbaa !22
  %56 = getelementptr inbounds nuw i32, ptr %46, i64 %44
  store ptr %56, ptr %26, align 8, !tbaa !23
  br label %57

57:                                               ; preds = %54, %29
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %3) #14
  %58 = load ptr, ptr %9, align 8, !tbaa !46
  %59 = load ptr, ptr %58, align 8, !tbaa !3
  invoke void @autograph_neighbor_iter_init(ptr noundef %59, i64 noundef %17, ptr noundef nonnull %3)
          to label %60 unwind label %106

60:                                               ; preds = %57
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %4) #14
  br label %61

61:                                               ; preds = %97, %60
  %62 = phi ptr [ null, %60 ], [ %98, %97 ]
  %63 = phi ptr [ null, %60 ], [ %101, %97 ]
  %64 = phi ptr [ null, %60 ], [ %100, %97 ]
  %65 = invoke i32 @autograph_neighbor_iter_next(ptr noundef nonnull %3, ptr noundef nonnull %4)
          to label %66 unwind label %181

66:                                               ; preds = %61
  %67 = icmp eq i32 %65, 0
  br i1 %67, label %108, label %68

68:                                               ; preds = %66
  %69 = icmp eq ptr %63, %64
  br i1 %69, label %72, label %70

70:                                               ; preds = %68
  %71 = load i32, ptr %4, align 4, !tbaa !34
  store i32 %71, ptr %63, align 4, !tbaa !34
  br label %97

72:                                               ; preds = %68
  %73 = ptrtoint ptr %63 to i64
  %74 = ptrtoint ptr %62 to i64
  %75 = sub i64 %73, %74
  %76 = ashr exact i64 %75, 2
  %77 = icmp sgt i64 %76, -1
  call void @llvm.assume(i1 %77)
  %78 = icmp eq i64 %75, 9223372036854775804
  br i1 %78, label %79, label %81

79:                                               ; preds = %72
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #16
          to label %80 unwind label %184

80:                                               ; preds = %79
  unreachable

81:                                               ; preds = %72
  %82 = call i64 @llvm.umax.i64(i64 %76, i64 1)
  %83 = add nuw nsw i64 %82, %76
  %84 = call noundef i64 @llvm.umin.i64(i64 %83, i64 2305843009213693951)
  %85 = shl nuw nsw i64 %84, 2
  %86 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %85) #15
          to label %87 unwind label %181

87:                                               ; preds = %81
  %88 = getelementptr inbounds i8, ptr %86, i64 %75
  %89 = load i32, ptr %4, align 4, !tbaa !34
  store i32 %89, ptr %88, align 4, !tbaa !34
  %90 = icmp sgt i64 %75, 0
  br i1 %90, label %91, label %92

91:                                               ; preds = %87
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %86, ptr align 4 %62, i64 %75, i1 false)
  br label %92

92:                                               ; preds = %91, %87
  %93 = icmp eq ptr %62, null
  br i1 %93, label %95, label %94

94:                                               ; preds = %92
  call void @_ZdlPvm(ptr noundef nonnull %62, i64 noundef %75) #18
  br label %95

95:                                               ; preds = %94, %92
  %96 = getelementptr inbounds nuw i32, ptr %86, i64 %84
  br label %97

97:                                               ; preds = %70, %95
  %98 = phi ptr [ %86, %95 ], [ %62, %70 ]
  %99 = phi ptr [ %88, %95 ], [ %63, %70 ]
  %100 = phi ptr [ %96, %95 ], [ %64, %70 ]
  %101 = getelementptr inbounds nuw i8, ptr %99, i64 4
  br label %61

102:                                              ; preds = %41
  %103 = landingpad { ptr, i32 }
          cleanup
  br label %212

104:                                              ; preds = %39
  %105 = landingpad { ptr, i32 }
          cleanup
  br label %212

106:                                              ; preds = %57
  %107 = landingpad { ptr, i32 }
          cleanup
  br label %204

108:                                              ; preds = %66
  %109 = ptrtoint ptr %63 to i64
  %110 = ptrtoint ptr %62 to i64
  %111 = sub i64 %109, %110
  %112 = ashr exact i64 %111, 2
  %113 = icmp sgt i64 %112, -1
  call void @llvm.assume(i1 %113)
  %114 = icmp eq ptr %63, %62
  br i1 %114, label %115, label %127

115:                                              ; preds = %108
  %116 = icmp eq ptr %62, null
  br i1 %116, label %123, label %117

117:                                              ; preds = %171, %115
  %118 = phi ptr [ %11, %115 ], [ %174, %171 ]
  %119 = phi ptr [ %12, %115 ], [ %173, %171 ]
  %120 = phi ptr [ %14, %115 ], [ %172, %171 ]
  %121 = ptrtoint ptr %64 to i64
  %122 = sub i64 %121, %110
  call void @_ZdlPvm(ptr noundef nonnull %62, i64 noundef %122) #18
  br label %123

123:                                              ; preds = %115, %117
  %124 = phi ptr [ %11, %115 ], [ %118, %117 ]
  %125 = phi ptr [ %12, %115 ], [ %119, %117 ]
  %126 = phi ptr [ %14, %115 ], [ %120, %117 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %4) #14
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %3) #14
  br label %176

127:                                              ; preds = %108, %171
  %128 = phi i64 [ %132, %171 ], [ %112, %108 ]
  %129 = phi ptr [ %174, %171 ], [ %11, %108 ]
  %130 = phi ptr [ %173, %171 ], [ %12, %108 ]
  %131 = phi ptr [ %172, %171 ], [ %14, %108 ]
  %132 = add nsw i64 %128, -1
  %133 = getelementptr inbounds nuw i32, ptr %62, i64 %132
  %134 = load i32, ptr %133, align 4, !tbaa !34
  store i32 %134, ptr %4, align 4, !tbaa !34
  %135 = load ptr, ptr %0, align 8, !tbaa !42
  %136 = sext i32 %134 to i64
  %137 = load ptr, ptr %135, align 8, !tbaa !15
  %138 = getelementptr inbounds nuw i8, ptr %137, i64 %136
  %139 = load i8, ptr %138, align 1, !tbaa !30
  %140 = icmp eq i8 %139, 0
  br i1 %140, label %141, label %171

141:                                              ; preds = %127
  %142 = icmp eq ptr %131, %130
  br i1 %142, label %145, label %143

143:                                              ; preds = %141
  store i32 %134, ptr %131, align 4, !tbaa !34
  %144 = getelementptr inbounds nuw i8, ptr %131, i64 4
  br label %171

145:                                              ; preds = %141
  %146 = ptrtoint ptr %130 to i64
  %147 = ptrtoint ptr %129 to i64
  %148 = sub i64 %146, %147
  %149 = ashr exact i64 %148, 2
  %150 = icmp sgt i64 %149, -1
  call void @llvm.assume(i1 %150)
  %151 = icmp eq i64 %148, 9223372036854775804
  br i1 %151, label %152, label %154

152:                                              ; preds = %145
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #16
          to label %153 unwind label %169

153:                                              ; preds = %152
  unreachable

154:                                              ; preds = %145
  %155 = call i64 @llvm.umax.i64(i64 %149, i64 1)
  %156 = add nuw nsw i64 %155, %149
  %157 = call noundef i64 @llvm.umin.i64(i64 %156, i64 2305843009213693951)
  %158 = shl nuw nsw i64 %157, 2
  %159 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %158) #15
          to label %160 unwind label %167

160:                                              ; preds = %154
  %161 = getelementptr inbounds i8, ptr %159, i64 %148
  store i32 %134, ptr %161, align 4, !tbaa !34
  %162 = icmp sgt i64 %148, 0
  br i1 %162, label %163, label %164

163:                                              ; preds = %160
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %159, ptr align 4 %129, i64 %148, i1 false)
  br label %164

164:                                              ; preds = %163, %160
  call void @_ZdlPvm(ptr noundef nonnull %129, i64 noundef %148) #18
  %165 = getelementptr inbounds nuw i8, ptr %161, i64 4
  %166 = getelementptr inbounds nuw i32, ptr %159, i64 %157
  br label %171

167:                                              ; preds = %154
  %168 = landingpad { ptr, i32 }
          cleanup
  br label %192

169:                                              ; preds = %152
  %170 = landingpad { ptr, i32 }
          cleanup
  br label %192

171:                                              ; preds = %164, %143, %127
  %172 = phi ptr [ %131, %127 ], [ %165, %164 ], [ %144, %143 ]
  %173 = phi ptr [ %130, %127 ], [ %166, %164 ], [ %130, %143 ]
  %174 = phi ptr [ %129, %127 ], [ %159, %164 ], [ %129, %143 ]
  %175 = icmp samesign ugt i64 %128, 1
  br i1 %175, label %127, label %117, !llvm.loop !47

176:                                              ; preds = %10, %123
  %177 = phi ptr [ %126, %123 ], [ %14, %10 ]
  %178 = phi ptr [ %125, %123 ], [ %12, %10 ]
  %179 = phi ptr [ %124, %123 ], [ %11, %10 ]
  %180 = icmp eq ptr %179, %177
  br i1 %180, label %208, label %10

181:                                              ; preds = %81, %61
  %182 = phi ptr [ %63, %81 ], [ %64, %61 ]
  %183 = landingpad { ptr, i32 }
          cleanup
  br label %186

184:                                              ; preds = %79
  %185 = landingpad { ptr, i32 }
          cleanup
  br label %186

186:                                              ; preds = %184, %181
  %187 = phi ptr [ %182, %181 ], [ %63, %184 ]
  %188 = phi { ptr, i32 } [ %183, %181 ], [ %185, %184 ]
  %189 = icmp eq ptr %62, null
  br i1 %189, label %200, label %190

190:                                              ; preds = %186
  %191 = ptrtoint ptr %62 to i64
  br label %192

192:                                              ; preds = %190, %167, %169
  %193 = phi i64 [ %191, %190 ], [ %110, %167 ], [ %110, %169 ]
  %194 = phi ptr [ %187, %190 ], [ %64, %167 ], [ %64, %169 ]
  %195 = phi { ptr, i32 } [ %188, %190 ], [ %168, %167 ], [ %170, %169 ]
  %196 = phi ptr [ %11, %190 ], [ %129, %167 ], [ %129, %169 ]
  %197 = phi ptr [ %12, %190 ], [ %130, %167 ], [ %130, %169 ]
  %198 = ptrtoint ptr %194 to i64
  %199 = sub i64 %198, %193
  call void @_ZdlPvm(ptr noundef nonnull %62, i64 noundef %199) #18
  br label %200

200:                                              ; preds = %186, %192
  %201 = phi { ptr, i32 } [ %188, %186 ], [ %195, %192 ]
  %202 = phi ptr [ %11, %186 ], [ %196, %192 ]
  %203 = phi ptr [ %12, %186 ], [ %197, %192 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %4) #14
  br label %204

204:                                              ; preds = %200, %106
  %205 = phi ptr [ %203, %200 ], [ %12, %106 ]
  %206 = phi ptr [ %202, %200 ], [ %11, %106 ]
  %207 = phi { ptr, i32 } [ %201, %200 ], [ %107, %106 ]
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %3) #14
  br label %212

208:                                              ; preds = %176
  %209 = ptrtoint ptr %178 to i64
  %210 = ptrtoint ptr %177 to i64
  %211 = sub i64 %209, %210
  call void @_ZdlPvm(ptr noundef nonnull %179, i64 noundef %211) #18
  ret void

212:                                              ; preds = %102, %104, %204
  %213 = phi ptr [ %205, %204 ], [ %12, %102 ], [ %12, %104 ]
  %214 = phi ptr [ %206, %204 ], [ %11, %102 ], [ %11, %104 ]
  %215 = phi { ptr, i32 } [ %207, %204 ], [ %103, %102 ], [ %105, %104 ]
  %216 = icmp eq ptr %214, null
  br i1 %216, label %221, label %217

217:                                              ; preds = %212
  %218 = ptrtoint ptr %213 to i64
  %219 = ptrtoint ptr %214 to i64
  %220 = sub i64 %218, %219
  call void @_ZdlPvm(ptr noundef nonnull %214, i64 noundef %220) #18
  br label %221

221:                                              ; preds = %212, %217
  resume { ptr, i32 } %215
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

declare dso_local void @autograph_neighbor_iter_init(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #5

declare dso_local i32 @autograph_neighbor_iter_next(ptr noundef, ptr noundef) local_unnamed_addr #5

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #6

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPvm(ptr noundef, i64 noundef) local_unnamed_addr #9

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #10

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #12

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { inlinehint mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #11 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #12 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #13 = { cold nounwind }
attributes #14 = { nounwind }
attributes #15 = { builtin allocsize(0) }
attributes #16 = { cold noreturn }
attributes #17 = { nounwind allocsize(0) }
attributes #18 = { builtin nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"p1 _ZTS5Graph", !5, i64 0}
!5 = !{!"any pointer", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"_ZTS5Graph", !10, i64 0, !10, i64 8, !11, i64 16, !12, i64 24}
!10 = !{!"long", !6, i64 0}
!11 = !{!"p1 long", !5, i64 0}
!12 = !{!"p1 int", !5, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"p1 _ZTS8_IO_FILE", !5, i64 0}
!15 = !{!16, !17, i64 0}
!16 = !{!"_ZTSNSt12_Vector_baseIcSaIcEE17_Vector_impl_dataE", !17, i64 0, !17, i64 8, !17, i64 16}
!17 = !{!"p1 omnipotent char", !5, i64 0}
!18 = !{!16, !17, i64 16}
!19 = !{!16, !17, i64 8}
!20 = !{!21, !12, i64 0}
!21 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !12, i64 0, !12, i64 8, !12, i64 16}
!22 = !{!21, !12, i64 8}
!23 = !{!21, !12, i64 16}
!24 = !{!25, !25, i64 0}
!25 = !{!"p1 _ZTSSt6vectorIcSaIcEE", !5, i64 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"p1 _ZTSSt6vectorIiSaIiEE", !5, i64 0}
!28 = !{!29, !29, i64 0}
!29 = !{!"p2 _ZTS5Graph", !5, i64 0}
!30 = !{!6, !6, i64 0}
!31 = distinct !{!31, !32}
!32 = !{!"llvm.loop.mustprogress"}
!33 = !{!12, !12, i64 0}
!34 = !{!35, !35, i64 0}
!35 = !{!"int", !6, i64 0}
!36 = distinct !{!36, !32, !37, !38}
!37 = !{!"llvm.loop.isvectorized", i32 1}
!38 = !{!"llvm.loop.unroll.runtime.disable"}
!39 = distinct !{!39, !40}
!40 = !{!"llvm.loop.unroll.disable"}
!41 = distinct !{!41, !32, !37}
!42 = !{!43, !25, i64 0}
!43 = !{!"_ZTSZ15dfs_runtime_srcE3$_0", !25, i64 0, !27, i64 8, !29, i64 16}
!44 = distinct !{!44, !32}
!45 = !{!43, !27, i64 8}
!46 = !{!43, !29, i64 16}
!47 = distinct !{!47, !32}
