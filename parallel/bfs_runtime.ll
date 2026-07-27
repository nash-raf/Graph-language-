; ModuleID = 'bfs_runtime.bc'
source_filename = "bfs_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::vector.0" = type { %"struct.std::_Vector_base.1" }
%"struct.std::_Vector_base.1" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%struct.Graph = type { i64, i64, ptr, ptr }

$__clang_call_terminate = comdat any

$_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag = comdat any

@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 514, i32 0, i32 22, ptr @0 }, align 8
@2 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @0 }, align 8
@.gomp_critical_user_.var = common global [8 x i32] zeroinitializer, align 8
@.str = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@.str.2 = private unnamed_addr constant [24 x i8] c"vector::_M_range_insert\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @bfs_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca %"class.std::vector", align 8
  %5 = alloca %"class.std::vector.0", align 8
  %6 = alloca %"class.std::vector.0", align 8
  %7 = icmp eq ptr %0, null
  br i1 %7, label %73, label %8

8:                                                ; preds = %1
  %9 = load i64, ptr %0, align 8, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #3
  %10 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %11 = load ptr, ptr %10, align 8, !tbaa !10
  store ptr %11, ptr %2, align 8, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #3
  %12 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %13 = load ptr, ptr %12, align 8, !tbaa !12
  store ptr %13, ptr %3, align 8, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #3
  %14 = icmp slt i64 %9, 0
  br i1 %14, label %15, label %16

15:                                               ; preds = %8
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #13
  unreachable

16:                                               ; preds = %8
  %17 = icmp eq i64 %9, 0
  br i1 %17, label %36, label %18

18:                                               ; preds = %16
  %19 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %9) #14
  store ptr %19, ptr %4, align 8, !tbaa !13
  %20 = getelementptr inbounds %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  %21 = getelementptr inbounds i8, ptr %19, i64 %9
  %22 = getelementptr inbounds %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data", ptr %4, i64 0, i32 2
  store ptr %21, ptr %22, align 8, !tbaa !15
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %19, i8 0, i64 %9, i1 false)
  store ptr %21, ptr %20, align 8, !tbaa !16
  %23 = trunc i64 %9 to i32
  %24 = icmp sgt i32 %23, 0
  br i1 %24, label %25, label %34

25:                                               ; preds = %18
  %26 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %5, i64 0, i32 1
  %27 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %5, i64 0, i32 2
  %28 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %6, i64 0, i32 1
  %29 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %6, i64 0, i32 2
  %30 = and i64 %9, 4294967295
  br label %37

31:                                               ; preds = %70
  %32 = load ptr, ptr %4, align 8, !tbaa !13
  %33 = icmp eq ptr %32, null
  br i1 %33, label %36, label %34

34:                                               ; preds = %18, %31
  %35 = phi ptr [ %32, %31 ], [ %19, %18 ]
  call void @_ZdlPv(ptr noundef nonnull %35) #15
  br label %36

36:                                               ; preds = %16, %31, %34
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #3
  br label %73

37:                                               ; preds = %25, %70
  %38 = phi i64 [ 0, %25 ], [ %71, %70 ]
  %39 = load ptr, ptr %4, align 8, !tbaa !13
  %40 = getelementptr inbounds i8, ptr %39, i64 %38
  %41 = load i8, ptr %40, align 1, !tbaa !17
  %42 = icmp eq i8 %41, 0
  br i1 %42, label %43, label %70

43:                                               ; preds = %37
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %5) #3
  %44 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #14
          to label %45 unwind label %74

45:                                               ; preds = %43
  %46 = trunc i64 %38 to i32
  store i32 %46, ptr %44, align 4, !tbaa !18
  %47 = getelementptr inbounds i32, ptr %44, i64 1
  store ptr %44, ptr %5, align 8, !tbaa !20
  store ptr %47, ptr %26, align 8, !tbaa !22
  store ptr %47, ptr %27, align 8, !tbaa !23
  store i8 1, ptr %40, align 1, !tbaa !17
  %48 = load ptr, ptr %5, align 8, !tbaa !11
  %49 = load ptr, ptr %26, align 8, !tbaa !11
  %50 = icmp eq ptr %48, %49
  br i1 %50, label %65, label %51

51:                                               ; preds = %45, %61
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %6) #3
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %6, i8 0, i64 24, i1 false)
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @2, i32 5, ptr nonnull @bfs_runtime.omp_outlined, ptr nonnull %5, ptr nonnull %2, ptr nonnull %3, ptr nonnull %4, ptr nonnull %6)
  %52 = load ptr, ptr %5, align 8, !tbaa !20
  %53 = load ptr, ptr %6, align 8, !tbaa !20
  store ptr %53, ptr %5, align 8, !tbaa !20
  %54 = load ptr, ptr %28, align 8, !tbaa !22
  %55 = load ptr, ptr %29, align 8, !tbaa !23
  store ptr %52, ptr %6, align 8, !tbaa !20
  %56 = load <2 x ptr>, ptr %26, align 8, !tbaa !11
  store ptr %54, ptr %26, align 8, !tbaa !22
  store ptr %55, ptr %27, align 8, !tbaa !23
  store <2 x ptr> %56, ptr %28, align 8, !tbaa !11
  %57 = icmp eq ptr %52, null
  br i1 %57, label %61, label %58

58:                                               ; preds = %51
  call void @_ZdlPv(ptr noundef nonnull %52) #15
  %59 = load ptr, ptr %5, align 8, !tbaa !11
  %60 = load ptr, ptr %26, align 8, !tbaa !11
  br label %61

61:                                               ; preds = %51, %58
  %62 = phi ptr [ %54, %51 ], [ %60, %58 ]
  %63 = phi ptr [ %53, %51 ], [ %59, %58 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #3
  %64 = icmp eq ptr %63, %62
  br i1 %64, label %65, label %51, !llvm.loop !24

65:                                               ; preds = %61, %45
  %66 = phi ptr [ %48, %45 ], [ %62, %61 ]
  %67 = icmp eq ptr %66, null
  br i1 %67, label %69, label %68

68:                                               ; preds = %65
  call void @_ZdlPv(ptr noundef nonnull %66) #15
  br label %69

69:                                               ; preds = %65, %68
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #3
  br label %70

70:                                               ; preds = %37, %69
  %71 = add nuw nsw i64 %38, 1
  %72 = icmp eq i64 %71, %30
  br i1 %72, label %31, label %37, !llvm.loop !26

73:                                               ; preds = %1, %36
  ret void

74:                                               ; preds = %43
  %75 = landingpad { ptr, i32 }
          cleanup
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #3
  call void @_ZdlPv(ptr noundef nonnull %39) #15
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #3
  resume { ptr, i32 } %75
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: alwaysinline norecurse nounwind uwtable
define internal void @bfs_runtime.omp_outlined(ptr noalias nocapture noundef readonly %0, ptr noalias nocapture readnone %1, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %2, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %3, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %4, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %5, ptr noundef nonnull align 8 dereferenceable(24) %6) #2 personality ptr @__gxx_personality_v0 {
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i32, align 4
  %12 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %2, i64 0, i32 1
  %13 = load ptr, ptr %12, align 8, !tbaa !22
  %14 = load ptr, ptr %2, align 8, !tbaa !20
  %15 = ptrtoint ptr %13 to i64
  %16 = ptrtoint ptr %14 to i64
  %17 = sub i64 %15, %16
  %18 = ashr exact i64 %17, 2
  %19 = icmp sgt i64 %18, -1
  tail call void @llvm.assume(i1 %19)
  %20 = icmp eq ptr %13, %14
  %21 = load i32, ptr %0, align 4, !tbaa !18
  br i1 %20, label %110, label %22

22:                                               ; preds = %7
  %23 = add nsw i64 %18, -1
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %8) #3
  store i64 0, ptr %8, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %9) #3
  store i64 %23, ptr %9, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #3
  store i64 1, ptr %10, align 8, !tbaa !27
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %11) #3
  store i32 0, ptr %11, align 4, !tbaa !18
  call void @__kmpc_for_static_init_8u(ptr nonnull @1, i32 %21, i32 34, ptr nonnull %11, ptr nonnull %8, ptr nonnull %9, ptr nonnull %10, i64 1, i64 1)
  %24 = load i64, ptr %9, align 8
  %25 = call i64 @llvm.umin.i64(i64 %24, i64 %23)
  store i64 %25, ptr %9, align 8, !tbaa !27
  %26 = load i64, ptr %8, align 8, !tbaa !27
  %27 = add i64 %25, 1
  %28 = icmp ult i64 %26, %27
  br i1 %28, label %29, label %106

29:                                               ; preds = %22, %47
  %30 = phi i64 [ %48, %47 ], [ %25, %22 ]
  %31 = phi i64 [ %52, %47 ], [ %26, %22 ]
  %32 = phi ptr [ %51, %47 ], [ null, %22 ]
  %33 = phi ptr [ %50, %47 ], [ null, %22 ]
  %34 = phi ptr [ %49, %47 ], [ null, %22 ]
  %35 = load ptr, ptr %2, align 8, !tbaa !20
  %36 = getelementptr inbounds i32, ptr %35, i64 %31
  %37 = load i32, ptr %36, align 4, !tbaa !18
  %38 = load ptr, ptr %3, align 8, !tbaa !11
  %39 = sext i32 %37 to i64
  %40 = getelementptr inbounds i64, ptr %38, i64 %39
  %41 = load i64, ptr %40, align 8, !tbaa !27
  %42 = getelementptr i64, ptr %40, i64 1
  %43 = load i64, ptr %42, align 8, !tbaa !27
  %44 = icmp slt i64 %41, %43
  br i1 %44, label %55, label %47

45:                                               ; preds = %100
  %46 = load i64, ptr %9, align 8, !tbaa !27
  br label %47

47:                                               ; preds = %45, %29
  %48 = phi i64 [ %30, %29 ], [ %46, %45 ]
  %49 = phi ptr [ %34, %29 ], [ %101, %45 ]
  %50 = phi ptr [ %33, %29 ], [ %102, %45 ]
  %51 = phi ptr [ %32, %29 ], [ %103, %45 ]
  %52 = add nuw i64 %31, 1
  %53 = add i64 %48, 1
  %54 = icmp ult i64 %52, %53
  br i1 %54, label %29, label %106

55:                                               ; preds = %29, %100
  %56 = phi i64 [ %104, %100 ], [ %41, %29 ]
  %57 = phi ptr [ %103, %100 ], [ %32, %29 ]
  %58 = phi ptr [ %102, %100 ], [ %33, %29 ]
  %59 = phi ptr [ %101, %100 ], [ %34, %29 ]
  %60 = load ptr, ptr %4, align 8, !tbaa !11
  %61 = getelementptr inbounds i32, ptr %60, i64 %56
  %62 = load i32, ptr %61, align 4, !tbaa !18
  %63 = sext i32 %62 to i64
  %64 = load ptr, ptr %5, align 8, !tbaa !13
  %65 = getelementptr inbounds i8, ptr %64, i64 %63
  %66 = load i8, ptr %65, align 1, !tbaa !17
  %67 = icmp eq i8 %66, 0
  br i1 %67, label %68, label %100

68:                                               ; preds = %55
  %69 = atomicrmw xchg ptr %65, i8 1 seq_cst, align 1
  %70 = icmp eq i8 %69, 0
  br i1 %70, label %71, label %100

71:                                               ; preds = %68
  %72 = icmp eq ptr %58, %59
  br i1 %72, label %75, label %73

73:                                               ; preds = %71
  store i32 %62, ptr %58, align 4, !tbaa !18
  %74 = getelementptr inbounds i32, ptr %58, i64 1
  br label %100

75:                                               ; preds = %71
  %76 = ptrtoint ptr %58 to i64
  %77 = ptrtoint ptr %57 to i64
  %78 = sub i64 %76, %77
  %79 = ashr exact i64 %78, 2
  %80 = icmp sgt i64 %79, -1
  call void @llvm.assume(i1 %80)
  %81 = icmp eq i64 %78, 9223372036854775804
  br i1 %81, label %82, label %84

82:                                               ; preds = %75
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #13
          to label %83 unwind label %127

83:                                               ; preds = %82
  unreachable

84:                                               ; preds = %75
  %85 = call i64 @llvm.umax.i64(i64 %79, i64 1)
  %86 = add nuw nsw i64 %85, %79
  %87 = call noundef i64 @llvm.umin.i64(i64 %86, i64 2305843009213693951)
  %88 = shl nuw nsw i64 %87, 2
  %89 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %88) #14
          to label %90 unwind label %125

90:                                               ; preds = %84
  %91 = getelementptr inbounds i8, ptr %89, i64 %78
  store i32 %62, ptr %91, align 4, !tbaa !18
  %92 = icmp sgt i64 %78, 0
  br i1 %92, label %93, label %94

93:                                               ; preds = %90
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %89, ptr align 4 %57, i64 %78, i1 false)
  br label %94

94:                                               ; preds = %93, %90
  %95 = icmp eq ptr %57, null
  br i1 %95, label %97, label %96

96:                                               ; preds = %94
  call void @_ZdlPv(ptr noundef nonnull %57) #15
  br label %97

97:                                               ; preds = %96, %94
  %98 = getelementptr inbounds i32, ptr %91, i64 1
  %99 = getelementptr inbounds i32, ptr %89, i64 %87
  br label %100

100:                                              ; preds = %97, %73, %68, %55
  %101 = phi ptr [ %59, %68 ], [ %59, %55 ], [ %99, %97 ], [ %59, %73 ]
  %102 = phi ptr [ %58, %68 ], [ %58, %55 ], [ %98, %97 ], [ %74, %73 ]
  %103 = phi ptr [ %57, %68 ], [ %57, %55 ], [ %89, %97 ], [ %57, %73 ]
  %104 = add i64 %56, 1
  %105 = icmp eq i64 %104, %43
  br i1 %105, label %45, label %55, !llvm.loop !28

106:                                              ; preds = %47, %22
  %107 = phi ptr [ null, %22 ], [ %50, %47 ]
  %108 = phi ptr [ null, %22 ], [ %51, %47 ]
  call void @__kmpc_for_static_fini(ptr nonnull @1, i32 %21)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %11) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %9) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %8) #3
  %109 = load i32, ptr %0, align 4, !tbaa !18
  br label %110

110:                                              ; preds = %106, %7
  %111 = phi i32 [ %21, %7 ], [ %109, %106 ]
  %112 = phi ptr [ null, %7 ], [ %107, %106 ]
  %113 = phi ptr [ null, %7 ], [ %108, %106 ]
  call void @__kmpc_critical(ptr nonnull @2, i32 %111, ptr nonnull @.gomp_critical_user_.var)
  %114 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %6, i64 0, i32 1
  %115 = load ptr, ptr %114, align 8, !tbaa !11
  %116 = load ptr, ptr %6, align 8, !tbaa !11
  %117 = ptrtoint ptr %115 to i64
  %118 = ptrtoint ptr %116 to i64
  %119 = sub i64 %117, %118
  %120 = getelementptr inbounds i8, ptr %116, i64 %119
  invoke void @_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag(ptr noundef nonnull align 8 dereferenceable(24) %6, ptr %120, ptr %113, ptr %112)
          to label %121 unwind label %127

121:                                              ; preds = %110
  call void @__kmpc_end_critical(ptr nonnull @2, i32 %111, ptr nonnull @.gomp_critical_user_.var)
  %122 = icmp eq ptr %113, null
  br i1 %122, label %124, label %123

123:                                              ; preds = %121
  call void @_ZdlPv(ptr noundef nonnull %113) #15
  br label %124

124:                                              ; preds = %121, %123
  ret void

125:                                              ; preds = %84
  %126 = landingpad { ptr, i32 }
          catch ptr null
  br label %129

127:                                              ; preds = %82, %110
  %128 = landingpad { ptr, i32 }
          catch ptr null
  br label %129

129:                                              ; preds = %127, %125
  %130 = phi { ptr, i32 } [ %126, %125 ], [ %128, %127 ]
  %131 = extractvalue { ptr, i32 } %130, 0
  call void @__clang_call_terminate(ptr %131) #16
  unreachable
}

; Function Attrs: nounwind
declare void @__kmpc_for_static_init_8u(ptr, i32, i32, ptr, ptr, ptr, ptr, i64, i64) local_unnamed_addr #3

; Function Attrs: noinline noreturn nounwind uwtable
define linkonce_odr hidden void @__clang_call_terminate(ptr noundef %0) local_unnamed_addr #4 comdat {
  %2 = tail call ptr @__cxa_begin_catch(ptr %0) #3
  tail call void @_ZSt9terminatev() #16
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr

declare dso_local void @_ZSt9terminatev() local_unnamed_addr

; Function Attrs: nounwind
declare void @__kmpc_for_static_fini(ptr, i32) local_unnamed_addr #3

; Function Attrs: convergent nounwind
declare void @__kmpc_critical(ptr, i32, ptr) local_unnamed_addr #5

; Function Attrs: convergent nounwind
declare void @__kmpc_end_critical(ptr, i32, ptr) local_unnamed_addr #5

; Function Attrs: nounwind
declare !callback !29 void @__kmpc_fork_call(ptr, i32, ptr, ...) local_unnamed_addr #3

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #6

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #7

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #8

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #9

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #10

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr %1, ptr %2, ptr %3) local_unnamed_addr #0 comdat align 2 personality ptr @__gxx_personality_v0 {
  %5 = ptrtoint ptr %1 to i64
  %6 = icmp eq ptr %2, %3
  br i1 %6, label %255, label %7

7:                                                ; preds = %4
  %8 = ptrtoint ptr %3 to i64
  %9 = ptrtoint ptr %2 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 2
  %12 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  %13 = load ptr, ptr %12, align 8, !tbaa !23
  %14 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  %15 = load ptr, ptr %14, align 8, !tbaa !11
  %16 = ptrtoint ptr %13 to i64
  %17 = ptrtoint ptr %15 to i64
  %18 = sub i64 %16, %17
  %19 = icmp ult i64 %18, %10
  br i1 %19, label %143, label %20

20:                                               ; preds = %7
  %21 = ptrtoint ptr %1 to i64
  %22 = sub i64 %17, %21
  %23 = ashr exact i64 %22, 2
  %24 = icmp ugt i64 %23, %11
  br i1 %24, label %25, label %82

25:                                               ; preds = %20
  %26 = sub nsw i64 0, %11
  %27 = getelementptr i32, ptr %15, i64 %26
  %28 = add i64 %10, -4
  %29 = lshr i64 %28, 2
  %30 = add nuw nsw i64 %29, 1
  %31 = icmp ult i64 %10, 32
  br i1 %31, label %52, label %32

32:                                               ; preds = %25
  %33 = and i64 %30, 9223372036854775800
  %34 = shl i64 %33, 2
  %35 = getelementptr i8, ptr %27, i64 %34
  %36 = shl i64 %33, 2
  %37 = getelementptr i8, ptr %15, i64 %36
  br label %38

38:                                               ; preds = %38, %32
  %39 = phi i64 [ 0, %32 ], [ %48, %38 ]
  %40 = shl i64 %39, 2
  %41 = getelementptr i8, ptr %27, i64 %40
  %42 = shl i64 %39, 2
  %43 = getelementptr i8, ptr %15, i64 %42
  %44 = getelementptr i32, ptr %41, i64 4
  %45 = load <4 x i32>, ptr %41, align 4, !tbaa !18
  %46 = load <4 x i32>, ptr %44, align 4, !tbaa !18
  %47 = getelementptr i32, ptr %43, i64 4
  store <4 x i32> %45, ptr %43, align 4, !tbaa !18
  store <4 x i32> %46, ptr %47, align 4, !tbaa !18
  %48 = add nuw i64 %39, 8
  %49 = icmp eq i64 %48, %33
  br i1 %49, label %50, label %38, !llvm.loop !31

50:                                               ; preds = %38
  %51 = icmp eq i64 %30, %33
  br i1 %51, label %62, label %52

52:                                               ; preds = %25, %50
  %53 = phi ptr [ %27, %25 ], [ %35, %50 ]
  %54 = phi ptr [ %15, %25 ], [ %37, %50 ]
  br label %55

55:                                               ; preds = %52, %55
  %56 = phi ptr [ %59, %55 ], [ %53, %52 ]
  %57 = phi ptr [ %60, %55 ], [ %54, %52 ]
  %58 = load i32, ptr %56, align 4, !tbaa !18
  store i32 %58, ptr %57, align 4, !tbaa !18
  %59 = getelementptr inbounds i32, ptr %56, i64 1
  %60 = getelementptr inbounds i32, ptr %57, i64 1
  %61 = icmp eq ptr %59, %15
  br i1 %61, label %62, label %55, !llvm.loop !34

62:                                               ; preds = %55, %50
  %63 = getelementptr inbounds i8, ptr %15, i64 %10
  store ptr %63, ptr %14, align 8, !tbaa !22
  %64 = ptrtoint ptr %27 to i64
  %65 = sub i64 %64, %21
  %66 = ashr exact i64 %65, 2
  %67 = sub nsw i64 0, %66
  %68 = getelementptr inbounds i32, ptr %15, i64 %67
  %69 = icmp sgt i64 %66, 1
  br i1 %69, label %70, label %71, !prof !35

70:                                               ; preds = %62
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %68, ptr align 4 %1, i64 %65, i1 false)
  br label %75

71:                                               ; preds = %62
  %72 = icmp eq i64 %65, 4
  br i1 %72, label %73, label %75

73:                                               ; preds = %71
  %74 = load i32, ptr %1, align 4, !tbaa !18
  store i32 %74, ptr %68, align 4, !tbaa !18
  br label %75

75:                                               ; preds = %70, %71, %73
  %76 = icmp sgt i64 %10, 4
  br i1 %76, label %77, label %78, !prof !36

77:                                               ; preds = %75
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %1, ptr align 4 %2, i64 %10, i1 false)
  br label %255

78:                                               ; preds = %75
  %79 = icmp eq i64 %10, 4
  br i1 %79, label %80, label %255

80:                                               ; preds = %78
  %81 = load i32, ptr %2, align 4, !tbaa !18
  store i32 %81, ptr %1, align 4, !tbaa !18
  br label %255

82:                                               ; preds = %20
  %83 = getelementptr inbounds i8, ptr %2, i64 %22
  %84 = ptrtoint ptr %83 to i64
  %85 = sub i64 %8, %84
  %86 = icmp sgt i64 %85, 0
  br i1 %86, label %87, label %89, !prof !36

87:                                               ; preds = %82
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %15, ptr align 4 %83, i64 %85, i1 false)
  %88 = load ptr, ptr %14, align 8, !tbaa !22
  br label %89

89:                                               ; preds = %82, %87
  %90 = phi ptr [ %15, %82 ], [ %88, %87 ]
  %91 = ptrtoint ptr %90 to i64
  %92 = sub nsw i64 %11, %23
  %93 = getelementptr i32, ptr %90, i64 %92
  %94 = icmp eq ptr %15, %1
  br i1 %94, label %135, label %95

95:                                               ; preds = %89
  %96 = add i64 %17, -4
  %97 = sub i64 %96, %21
  %98 = lshr i64 %97, 2
  %99 = add nuw nsw i64 %98, 1
  %100 = icmp ult i64 %97, 76
  br i1 %100, label %125, label %101

101:                                              ; preds = %95
  %102 = add i64 %10, %91
  %103 = sub i64 %102, %17
  %104 = icmp ult i64 %103, 32
  br i1 %104, label %125, label %105

105:                                              ; preds = %101
  %106 = and i64 %99, 9223372036854775800
  %107 = shl i64 %106, 2
  %108 = getelementptr i8, ptr %1, i64 %107
  %109 = shl i64 %106, 2
  %110 = getelementptr i8, ptr %93, i64 %109
  br label %111

111:                                              ; preds = %111, %105
  %112 = phi i64 [ 0, %105 ], [ %121, %111 ]
  %113 = shl i64 %112, 2
  %114 = getelementptr i8, ptr %1, i64 %113
  %115 = shl i64 %112, 2
  %116 = getelementptr i8, ptr %93, i64 %115
  %117 = getelementptr i32, ptr %114, i64 4
  %118 = load <4 x i32>, ptr %114, align 4, !tbaa !18
  %119 = load <4 x i32>, ptr %117, align 4, !tbaa !18
  %120 = getelementptr i32, ptr %116, i64 4
  store <4 x i32> %118, ptr %116, align 4, !tbaa !18
  store <4 x i32> %119, ptr %120, align 4, !tbaa !18
  %121 = add nuw i64 %112, 8
  %122 = icmp eq i64 %121, %106
  br i1 %122, label %123, label %111, !llvm.loop !37

123:                                              ; preds = %111
  %124 = icmp eq i64 %99, %106
  br i1 %124, label %135, label %125

125:                                              ; preds = %101, %95, %123
  %126 = phi ptr [ %1, %101 ], [ %1, %95 ], [ %108, %123 ]
  %127 = phi ptr [ %93, %101 ], [ %93, %95 ], [ %110, %123 ]
  br label %128

128:                                              ; preds = %125, %128
  %129 = phi ptr [ %132, %128 ], [ %126, %125 ]
  %130 = phi ptr [ %133, %128 ], [ %127, %125 ]
  %131 = load i32, ptr %129, align 4, !tbaa !18
  store i32 %131, ptr %130, align 4, !tbaa !18
  %132 = getelementptr inbounds i32, ptr %129, i64 1
  %133 = getelementptr inbounds i32, ptr %130, i64 1
  %134 = icmp eq ptr %132, %15
  br i1 %134, label %135, label %128, !llvm.loop !38

135:                                              ; preds = %128, %123, %89
  %136 = getelementptr inbounds i8, ptr %90, i64 %10
  store ptr %136, ptr %14, align 8, !tbaa !22
  %137 = icmp sgt i64 %22, 4
  br i1 %137, label %138, label %139, !prof !36

138:                                              ; preds = %135
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %1, ptr align 4 %2, i64 %22, i1 false)
  br label %255

139:                                              ; preds = %135
  %140 = icmp eq i64 %22, 4
  br i1 %140, label %141, label %255

141:                                              ; preds = %139
  %142 = load i32, ptr %2, align 4, !tbaa !18
  store i32 %142, ptr %1, align 4, !tbaa !18
  br label %255

143:                                              ; preds = %7
  %144 = load ptr, ptr %0, align 8, !tbaa !20
  %145 = ptrtoint ptr %144 to i64
  %146 = sub i64 %17, %145
  %147 = ashr exact i64 %146, 2
  %148 = icmp sgt i64 %147, -1
  tail call void @llvm.assume(i1 %148)
  %149 = sub nuw nsw i64 2305843009213693951, %147
  %150 = icmp ult i64 %149, %11
  br i1 %150, label %151, label %152

151:                                              ; preds = %143
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #13
  unreachable

152:                                              ; preds = %143
  %153 = tail call i64 @llvm.umax.i64(i64 %147, i64 %11)
  %154 = add nuw nsw i64 %153, %147
  %155 = tail call noundef i64 @llvm.umin.i64(i64 %154, i64 2305843009213693951)
  %156 = shl nuw nsw i64 %155, 2
  %157 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %156) #14
  %158 = icmp eq ptr %144, %1
  br i1 %158, label %199, label %159

159:                                              ; preds = %152
  %160 = ptrtoint ptr %157 to i64
  %161 = add i64 %5, -4
  %162 = sub i64 %161, %145
  %163 = lshr i64 %162, 2
  %164 = add nuw nsw i64 %163, 1
  %165 = icmp ult i64 %162, 28
  %166 = sub i64 %160, %145
  %167 = icmp ult i64 %166, 32
  %168 = or i1 %165, %167
  br i1 %168, label %189, label %169

169:                                              ; preds = %159
  %170 = and i64 %164, 9223372036854775800
  %171 = shl i64 %170, 2
  %172 = getelementptr i8, ptr %144, i64 %171
  %173 = shl i64 %170, 2
  %174 = getelementptr i8, ptr %157, i64 %173
  br label %175

175:                                              ; preds = %175, %169
  %176 = phi i64 [ 0, %169 ], [ %185, %175 ]
  %177 = shl i64 %176, 2
  %178 = getelementptr i8, ptr %144, i64 %177
  %179 = shl i64 %176, 2
  %180 = getelementptr i8, ptr %157, i64 %179
  %181 = getelementptr i32, ptr %178, i64 4
  %182 = load <4 x i32>, ptr %178, align 4, !tbaa !18
  %183 = load <4 x i32>, ptr %181, align 4, !tbaa !18
  %184 = getelementptr i32, ptr %180, i64 4
  store <4 x i32> %182, ptr %180, align 4, !tbaa !18
  store <4 x i32> %183, ptr %184, align 4, !tbaa !18
  %185 = add nuw i64 %176, 8
  %186 = icmp eq i64 %185, %170
  br i1 %186, label %187, label %175, !llvm.loop !39

187:                                              ; preds = %175
  %188 = icmp eq i64 %164, %170
  br i1 %188, label %199, label %189

189:                                              ; preds = %159, %187
  %190 = phi ptr [ %144, %159 ], [ %172, %187 ]
  %191 = phi ptr [ %157, %159 ], [ %174, %187 ]
  br label %192

192:                                              ; preds = %189, %192
  %193 = phi ptr [ %196, %192 ], [ %190, %189 ]
  %194 = phi ptr [ %197, %192 ], [ %191, %189 ]
  %195 = load i32, ptr %193, align 4, !tbaa !18
  store i32 %195, ptr %194, align 4, !tbaa !18
  %196 = getelementptr inbounds i32, ptr %193, i64 1
  %197 = getelementptr inbounds i32, ptr %194, i64 1
  %198 = icmp eq ptr %196, %1
  br i1 %198, label %199, label %192, !llvm.loop !40

199:                                              ; preds = %192, %187, %152
  %200 = phi ptr [ %157, %152 ], [ %174, %187 ], [ %197, %192 ]
  %201 = ptrtoint ptr %200 to i64
  %202 = icmp sgt i64 %10, 0
  br i1 %202, label %203, label %205, !prof !36

203:                                              ; preds = %199
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %200, ptr align 4 %2, i64 %10, i1 false)
  %204 = getelementptr i8, ptr %200, i64 %10
  br label %205

205:                                              ; preds = %203, %199
  %206 = phi ptr [ %204, %203 ], [ %200, %199 ]
  %207 = icmp eq ptr %15, %1
  br i1 %207, label %249, label %208

208:                                              ; preds = %205
  %209 = add i64 %17, -4
  %210 = sub i64 %209, %5
  %211 = lshr i64 %210, 2
  %212 = add nuw nsw i64 %211, 1
  %213 = icmp ult i64 %210, 60
  br i1 %213, label %239, label %214

214:                                              ; preds = %208
  %215 = tail call i64 @llvm.smax.i64(i64 %10, i64 0)
  %216 = add i64 %215, %201
  %217 = sub i64 %216, %5
  %218 = icmp ult i64 %217, 32
  br i1 %218, label %239, label %219

219:                                              ; preds = %214
  %220 = and i64 %212, 9223372036854775800
  %221 = shl i64 %220, 2
  %222 = getelementptr i8, ptr %1, i64 %221
  %223 = shl i64 %220, 2
  %224 = getelementptr i8, ptr %206, i64 %223
  br label %225

225:                                              ; preds = %225, %219
  %226 = phi i64 [ 0, %219 ], [ %235, %225 ]
  %227 = shl i64 %226, 2
  %228 = getelementptr i8, ptr %1, i64 %227
  %229 = shl i64 %226, 2
  %230 = getelementptr i8, ptr %206, i64 %229
  %231 = getelementptr i32, ptr %228, i64 4
  %232 = load <4 x i32>, ptr %228, align 4, !tbaa !18
  %233 = load <4 x i32>, ptr %231, align 4, !tbaa !18
  %234 = getelementptr i32, ptr %230, i64 4
  store <4 x i32> %232, ptr %230, align 4, !tbaa !18
  store <4 x i32> %233, ptr %234, align 4, !tbaa !18
  %235 = add nuw i64 %226, 8
  %236 = icmp eq i64 %235, %220
  br i1 %236, label %237, label %225, !llvm.loop !41

237:                                              ; preds = %225
  %238 = icmp eq i64 %212, %220
  br i1 %238, label %249, label %239

239:                                              ; preds = %214, %208, %237
  %240 = phi ptr [ %1, %214 ], [ %1, %208 ], [ %222, %237 ]
  %241 = phi ptr [ %206, %214 ], [ %206, %208 ], [ %224, %237 ]
  br label %242

242:                                              ; preds = %239, %242
  %243 = phi ptr [ %246, %242 ], [ %240, %239 ]
  %244 = phi ptr [ %247, %242 ], [ %241, %239 ]
  %245 = load i32, ptr %243, align 4, !tbaa !18
  store i32 %245, ptr %244, align 4, !tbaa !18
  %246 = getelementptr inbounds i32, ptr %243, i64 1
  %247 = getelementptr inbounds i32, ptr %244, i64 1
  %248 = icmp eq ptr %246, %15
  br i1 %248, label %249, label %242, !llvm.loop !42

249:                                              ; preds = %242, %237, %205
  %250 = phi ptr [ %206, %205 ], [ %224, %237 ], [ %247, %242 ]
  %251 = icmp eq ptr %144, null
  br i1 %251, label %253, label %252

252:                                              ; preds = %249
  tail call void @_ZdlPv(ptr noundef nonnull %144) #15
  br label %253

253:                                              ; preds = %249, %252
  store ptr %157, ptr %0, align 8, !tbaa !20
  store ptr %250, ptr %14, align 8, !tbaa !22
  %254 = getelementptr inbounds i32, ptr %157, i64 %155
  store ptr %254, ptr %12, align 8, !tbaa !23
  br label %255

255:                                              ; preds = %141, %139, %138, %80, %78, %77, %253, %4
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #10

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #12

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { alwaysinline norecurse nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }
attributes #4 = { noinline noreturn nounwind uwtable "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { convergent nounwind }
attributes #6 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #9 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #11 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #12 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #13 = { cold noreturn }
attributes #14 = { builtin allocsize(0) }
attributes #15 = { builtin nounwind }
attributes #16 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!4 = !{!5, !6, i64 0}
!5 = !{!"_ZTS5Graph", !6, i64 0, !6, i64 8, !9, i64 16, !9, i64 24}
!6 = !{!"long", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!"any pointer", !7, i64 0}
!10 = !{!5, !9, i64 16}
!11 = !{!9, !9, i64 0}
!12 = !{!5, !9, i64 24}
!13 = !{!14, !9, i64 0}
!14 = !{!"_ZTSNSt12_Vector_baseIcSaIcEE17_Vector_impl_dataE", !9, i64 0, !9, i64 8, !9, i64 16}
!15 = !{!14, !9, i64 16}
!16 = !{!14, !9, i64 8}
!17 = !{!7, !7, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !7, i64 0}
!20 = !{!21, !9, i64 0}
!21 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !9, i64 0, !9, i64 8, !9, i64 16}
!22 = !{!21, !9, i64 8}
!23 = !{!21, !9, i64 16}
!24 = distinct !{!24, !25}
!25 = !{!"llvm.loop.mustprogress"}
!26 = distinct !{!26, !25}
!27 = !{!6, !6, i64 0}
!28 = distinct !{!28, !25}
!29 = !{!30}
!30 = !{i64 2, i64 -1, i64 -1, i1 true}
!31 = distinct !{!31, !25, !32, !33}
!32 = !{!"llvm.loop.isvectorized", i32 1}
!33 = !{!"llvm.loop.unroll.runtime.disable"}
!34 = distinct !{!34, !25, !32}
!35 = !{!"branch_weights", i32 -2147483648, i32 0}
!36 = !{!"branch_weights", i32 2000, i32 1}
!37 = distinct !{!37, !25, !32, !33}
!38 = distinct !{!38, !25, !32}
!39 = distinct !{!39, !25, !32, !33}
!40 = distinct !{!40, !25, !32}
!41 = distinct !{!41, !25, !32, !33}
!42 = distinct !{!42, !25, !32}