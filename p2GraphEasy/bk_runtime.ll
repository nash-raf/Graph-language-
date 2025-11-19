; ModuleID = 'bk_runtime.bc'
source_filename = "bk_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%struct.Graph = type { i64, i64, ptr, ptr }

$_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag = comdat any

$_ZSt16__set_differenceIN9__gnu_cxx17__normal_iteratorIPiSt6vectorIiSaIiEEEES6_St20back_insert_iteratorIS5_ENS0_5__ops15_Iter_less_iterEET1_T_SC_T0_SD_SB_T2_ = comdat any

@.str = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@.str.1 = private unnamed_addr constant [24 x i8] c"vector::_M_range_insert\00", align 1
@.str.2 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @intersect_sorted(ptr dead_on_unwind noalias nocapture writable writeonly sret(%"class.std::vector") align 8 %0, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %1, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %2) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %0, i8 0, i64 24, i1 false)
  %4 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %1, i64 0, i32 1
  %5 = load ptr, ptr %4, align 8, !tbaa !3
  %6 = load ptr, ptr %1, align 8, !tbaa !8
  %7 = ptrtoint ptr %5 to i64
  %8 = ptrtoint ptr %6 to i64
  %9 = sub i64 %7, %8
  %10 = icmp sgt i64 %9, -1
  tail call void @llvm.assume(i1 %10)
  %11 = icmp eq ptr %5, %6
  br i1 %11, label %119, label %12

12:                                               ; preds = %3
  %13 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %2, i64 0, i32 1
  %14 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  %15 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  br label %16

16:                                               ; preds = %12, %101
  %17 = phi ptr [ %6, %12 ], [ %102, %101 ]
  %18 = phi ptr [ %5, %12 ], [ %103, %101 ]
  %19 = phi ptr [ %6, %12 ], [ %104, %101 ]
  %20 = phi ptr [ %5, %12 ], [ %105, %101 ]
  %21 = phi ptr [ %6, %12 ], [ %106, %101 ]
  %22 = phi ptr [ %5, %12 ], [ %107, %101 ]
  %23 = phi ptr [ null, %12 ], [ %108, %101 ]
  %24 = phi ptr [ null, %12 ], [ %109, %101 ]
  %25 = phi i64 [ 0, %12 ], [ %112, %101 ]
  %26 = phi i64 [ 0, %12 ], [ %111, %101 ]
  %27 = phi ptr [ null, %12 ], [ %110, %101 ]
  %28 = load ptr, ptr %13, align 8, !tbaa !3
  %29 = load ptr, ptr %2, align 8, !tbaa !8
  %30 = ptrtoint ptr %28 to i64
  %31 = ptrtoint ptr %29 to i64
  %32 = sub i64 %30, %31
  %33 = ashr exact i64 %32, 2
  %34 = icmp sgt i64 %33, -1
  tail call void @llvm.assume(i1 %34)
  %35 = icmp ult i64 %26, %33
  br i1 %35, label %36, label %119

36:                                               ; preds = %16
  %37 = getelementptr inbounds i32, ptr %21, i64 %25
  %38 = load i32, ptr %37, align 4, !tbaa !9
  %39 = getelementptr inbounds i32, ptr %29, i64 %26
  %40 = load i32, ptr %39, align 4, !tbaa !9
  %41 = icmp slt i32 %38, %40
  br i1 %41, label %42, label %44

42:                                               ; preds = %36
  %43 = add nuw nsw i64 %25, 1
  br label %101

44:                                               ; preds = %36
  %45 = icmp sgt i32 %38, %40
  br i1 %45, label %46, label %48

46:                                               ; preds = %44
  %47 = add nuw nsw i64 %26, 1
  br label %101

48:                                               ; preds = %44
  %49 = icmp eq ptr %24, %23
  br i1 %49, label %52, label %50

50:                                               ; preds = %48
  store i32 %38, ptr %24, align 4, !tbaa !9
  %51 = getelementptr inbounds i32, ptr %24, i64 1
  store ptr %51, ptr %14, align 8, !tbaa !3
  br label %82

52:                                               ; preds = %48
  %53 = ptrtoint ptr %23 to i64
  %54 = ptrtoint ptr %27 to i64
  %55 = sub i64 %53, %54
  %56 = ashr exact i64 %55, 2
  %57 = icmp sgt i64 %56, -1
  tail call void @llvm.assume(i1 %57)
  %58 = icmp eq i64 %55, 9223372036854775804
  br i1 %58, label %59, label %61

59:                                               ; preds = %52
  store ptr %27, ptr %0, align 8, !tbaa !11
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #9
          to label %60 unwind label %94

60:                                               ; preds = %59
  unreachable

61:                                               ; preds = %52
  %62 = tail call i64 @llvm.umax.i64(i64 %56, i64 1)
  %63 = add nuw nsw i64 %62, %56
  %64 = tail call noundef i64 @llvm.umin.i64(i64 %63, i64 2305843009213693951)
  %65 = shl nuw nsw i64 %64, 2
  %66 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %65) #10
          to label %67 unwind label %92

67:                                               ; preds = %61
  %68 = getelementptr inbounds i8, ptr %66, i64 %55
  %69 = load i32, ptr %37, align 4, !tbaa !9
  store i32 %69, ptr %68, align 4, !tbaa !9
  %70 = icmp sgt i64 %55, 0
  br i1 %70, label %71, label %72

71:                                               ; preds = %67
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %66, ptr align 4 %27, i64 %55, i1 false)
  br label %72

72:                                               ; preds = %71, %67
  %73 = icmp eq ptr %27, null
  br i1 %73, label %77, label %74

74:                                               ; preds = %72
  tail call void @_ZdlPv(ptr noundef nonnull %27) #11
  %75 = load ptr, ptr %4, align 8, !tbaa !3
  %76 = load ptr, ptr %1, align 8, !tbaa !8
  br label %77

77:                                               ; preds = %74, %72
  %78 = phi ptr [ %76, %74 ], [ %17, %72 ]
  %79 = phi ptr [ %75, %74 ], [ %18, %72 ]
  %80 = getelementptr inbounds i32, ptr %68, i64 1
  store ptr %80, ptr %14, align 8, !tbaa !3
  %81 = getelementptr inbounds i32, ptr %66, i64 %64
  store ptr %81, ptr %15, align 8, !tbaa !12
  br label %82

82:                                               ; preds = %77, %50
  %83 = phi ptr [ %78, %77 ], [ %17, %50 ]
  %84 = phi ptr [ %79, %77 ], [ %18, %50 ]
  %85 = phi ptr [ %78, %77 ], [ %19, %50 ]
  %86 = phi ptr [ %79, %77 ], [ %20, %50 ]
  %87 = phi ptr [ %81, %77 ], [ %23, %50 ]
  %88 = phi ptr [ %80, %77 ], [ %51, %50 ]
  %89 = phi ptr [ %66, %77 ], [ %27, %50 ]
  %90 = add nuw nsw i64 %25, 1
  %91 = add nuw nsw i64 %26, 1
  br label %101

92:                                               ; preds = %61
  %93 = landingpad { ptr, i32 }
          cleanup
  store ptr %27, ptr %0, align 8, !tbaa !11
  br label %96

94:                                               ; preds = %59
  %95 = landingpad { ptr, i32 }
          cleanup
  br label %96

96:                                               ; preds = %94, %92
  %97 = phi { ptr, i32 } [ %93, %92 ], [ %95, %94 ]
  %98 = icmp eq ptr %27, null
  br i1 %98, label %100, label %99

99:                                               ; preds = %96
  tail call void @_ZdlPv(ptr noundef nonnull %27) #11
  br label %100

100:                                              ; preds = %96, %99
  resume { ptr, i32 } %97

101:                                              ; preds = %46, %82, %42
  %102 = phi ptr [ %17, %42 ], [ %17, %46 ], [ %83, %82 ]
  %103 = phi ptr [ %18, %42 ], [ %18, %46 ], [ %84, %82 ]
  %104 = phi ptr [ %19, %42 ], [ %19, %46 ], [ %85, %82 ]
  %105 = phi ptr [ %20, %42 ], [ %20, %46 ], [ %86, %82 ]
  %106 = phi ptr [ %21, %42 ], [ %21, %46 ], [ %85, %82 ]
  %107 = phi ptr [ %22, %42 ], [ %22, %46 ], [ %86, %82 ]
  %108 = phi ptr [ %23, %42 ], [ %23, %46 ], [ %87, %82 ]
  %109 = phi ptr [ %24, %42 ], [ %24, %46 ], [ %88, %82 ]
  %110 = phi ptr [ %27, %42 ], [ %27, %46 ], [ %89, %82 ]
  %111 = phi i64 [ %26, %42 ], [ %47, %46 ], [ %91, %82 ]
  %112 = phi i64 [ %43, %42 ], [ %25, %46 ], [ %90, %82 ]
  %113 = ptrtoint ptr %107 to i64
  %114 = ptrtoint ptr %106 to i64
  %115 = sub i64 %113, %114
  %116 = ashr exact i64 %115, 2
  %117 = icmp sgt i64 %116, -1
  tail call void @llvm.assume(i1 %117)
  %118 = icmp ult i64 %112, %116
  br i1 %118, label %16, label %119, !llvm.loop !13

119:                                              ; preds = %101, %16, %3
  %120 = phi ptr [ null, %3 ], [ %27, %16 ], [ %110, %101 ]
  store ptr %120, ptr %0, align 8, !tbaa !11
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress uwtable
define dso_local void @bron_kerbosch_with_pivot(ptr nocapture noundef readonly %0, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %1, ptr nocapture noundef nonnull align 8 dereferenceable(24) %2, ptr nocapture noundef nonnull align 8 dereferenceable(24) %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = alloca %"class.std::vector", align 8
  %6 = alloca %"class.std::vector", align 8
  %7 = alloca %"class.std::vector", align 8
  %8 = alloca %"class.std::vector", align 8
  %9 = alloca %"class.std::vector", align 8
  %10 = alloca %"class.std::vector", align 8
  %11 = load ptr, ptr %2, align 8, !tbaa !11
  %12 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %2, i64 0, i32 1
  %13 = load ptr, ptr %12, align 8, !tbaa !11
  %14 = icmp eq ptr %11, %13
  br i1 %14, label %15, label %22

15:                                               ; preds = %4
  %16 = load ptr, ptr %3, align 8, !tbaa !11
  %17 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  %18 = load ptr, ptr %17, align 8, !tbaa !11
  %19 = icmp eq ptr %16, %18
  br i1 %19, label %351, label %20

20:                                               ; preds = %15
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %5) #12
  %21 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %5, i64 0, i32 1
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %5, i8 0, i64 24, i1 false)
  br label %34

22:                                               ; preds = %4
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %5) #12
  %23 = ptrtoint ptr %13 to i64
  %24 = ptrtoint ptr %11 to i64
  %25 = sub i64 %23, %24
  %26 = icmp sgt i64 %25, -1
  tail call void @llvm.assume(i1 %26)
  %27 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %25) #10
  store ptr %27, ptr %5, align 8, !tbaa !8
  %28 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %5, i64 0, i32 1
  store ptr %27, ptr %28, align 8, !tbaa !3
  %29 = getelementptr inbounds i8, ptr %27, i64 %25
  %30 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %5, i64 0, i32 2
  store ptr %29, ptr %30, align 8, !tbaa !12
  %31 = icmp sgt i64 %25, 0
  br i1 %31, label %32, label %34, !prof !15

32:                                               ; preds = %22
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %27, ptr align 4 %11, i64 %25, i1 false)
  %33 = getelementptr inbounds i8, ptr %27, i64 %25
  br label %34

34:                                               ; preds = %20, %22, %32
  %35 = phi ptr [ %28, %32 ], [ %28, %22 ], [ %21, %20 ]
  %36 = phi ptr [ %27, %32 ], [ %27, %22 ], [ null, %20 ]
  %37 = phi ptr [ %33, %32 ], [ %27, %22 ], [ null, %20 ]
  store ptr %37, ptr %35, align 8, !tbaa !3
  %38 = load ptr, ptr %3, align 8, !tbaa !11
  %39 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  %40 = load ptr, ptr %39, align 8, !tbaa !11
  %41 = ptrtoint ptr %37 to i64
  %42 = ptrtoint ptr %36 to i64
  %43 = sub i64 %41, %42
  %44 = getelementptr inbounds i8, ptr %36, i64 %43
  invoke void @_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag(ptr noundef nonnull align 8 dereferenceable(24) %5, ptr %44, ptr %38, ptr %40)
          to label %45 unwind label %77

45:                                               ; preds = %34
  %46 = load ptr, ptr %5, align 8, !tbaa !11
  %47 = load ptr, ptr %35, align 8, !tbaa !11
  %48 = icmp eq ptr %46, %47
  %49 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %50 = load ptr, ptr %49, align 8, !tbaa !16
  br i1 %48, label %51, label %79

51:                                               ; preds = %79, %45
  %52 = phi i32 [ -1, %45 ], [ %92, %79 ]
  %53 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %54 = sext i32 %52 to i64
  %55 = getelementptr inbounds i64, ptr %50, i64 %54
  %56 = load i64, ptr %55, align 8, !tbaa !19
  %57 = getelementptr i64, ptr %55, i64 1
  %58 = load i64, ptr %57, align 8, !tbaa !19
  %59 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %60 = load ptr, ptr %59, align 8, !tbaa !20
  %61 = getelementptr inbounds i32, ptr %60, i64 %56
  %62 = getelementptr inbounds i32, ptr %60, i64 %58
  %63 = ptrtoint ptr %62 to i64
  %64 = ptrtoint ptr %61 to i64
  %65 = sub i64 %63, %64
  %66 = icmp ugt i64 %65, 9223372036854775804
  br i1 %66, label %67, label %69

67:                                               ; preds = %51
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #9
          to label %68 unwind label %75

68:                                               ; preds = %67
  unreachable

69:                                               ; preds = %51
  %70 = icmp eq i64 %58, %56
  br i1 %70, label %95, label %71

71:                                               ; preds = %69
  %72 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %65) #10
          to label %73 unwind label %75

73:                                               ; preds = %71
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %72, ptr align 4 %61, i64 %65, i1 false)
  %74 = getelementptr inbounds i8, ptr %72, i64 %65
  br label %95

75:                                               ; preds = %67, %71
  %76 = landingpad { ptr, i32 }
          cleanup
  br label %360

77:                                               ; preds = %34
  %78 = landingpad { ptr, i32 }
          cleanup
  br label %360

79:                                               ; preds = %45, %79
  %80 = phi i32 [ %92, %79 ], [ -1, %45 ]
  %81 = phi i64 [ %91, %79 ], [ -1, %45 ]
  %82 = phi ptr [ %93, %79 ], [ %46, %45 ]
  %83 = load i32, ptr %82, align 4, !tbaa !9
  %84 = sext i32 %83 to i64
  %85 = getelementptr i64, ptr %50, i64 %84
  %86 = getelementptr i64, ptr %85, i64 1
  %87 = load i64, ptr %86, align 8, !tbaa !19
  %88 = load i64, ptr %85, align 8, !tbaa !19
  %89 = sub nsw i64 %87, %88
  %90 = icmp sgt i64 %89, %81
  %91 = call i64 @llvm.smax.i64(i64 %89, i64 %81)
  %92 = select i1 %90, i32 %83, i32 %80
  %93 = getelementptr inbounds i32, ptr %82, i64 1
  %94 = icmp eq ptr %93, %47
  br i1 %94, label %51, label %79

95:                                               ; preds = %69, %73
  %96 = phi ptr [ %72, %73 ], [ null, %69 ]
  %97 = phi ptr [ %74, %73 ], [ null, %69 ]
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %6) #12
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %6, i8 0, i64 24, i1 false)
  %98 = load ptr, ptr %2, align 8, !tbaa !11
  %99 = load ptr, ptr %12, align 8, !tbaa !11
  %100 = invoke ptr @_ZSt16__set_differenceIN9__gnu_cxx17__normal_iteratorIPiSt6vectorIiSaIiEEEES6_St20back_insert_iteratorIS5_ENS0_5__ops15_Iter_less_iterEET1_T_SC_T0_SD_SB_T2_(ptr %98, ptr %99, ptr %96, ptr %97, ptr nonnull %6)
          to label %101 unwind label %127

101:                                              ; preds = %95
  %102 = load ptr, ptr %6, align 8, !tbaa !11
  %103 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %6, i64 0, i32 1
  %104 = load ptr, ptr %103, align 8, !tbaa !11
  %105 = icmp eq ptr %102, %104
  br i1 %105, label %115, label %106

106:                                              ; preds = %101
  %107 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %7, i64 0, i32 1
  %108 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %7, i64 0, i32 2
  %109 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %1, i64 0, i32 1
  %110 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %8, i64 0, i32 1
  %111 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %8, i64 0, i32 2
  %112 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %3, i64 0, i32 2
  br label %129

113:                                              ; preds = %311
  %114 = load ptr, ptr %6, align 8, !tbaa !8
  br label %115

115:                                              ; preds = %113, %101
  %116 = phi ptr [ %114, %113 ], [ %102, %101 ]
  %117 = icmp eq ptr %116, null
  br i1 %117, label %119, label %118

118:                                              ; preds = %115
  call void @_ZdlPv(ptr noundef nonnull %116) #11
  br label %119

119:                                              ; preds = %115, %118
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #12
  %120 = icmp eq ptr %96, null
  br i1 %120, label %122, label %121

121:                                              ; preds = %119
  call void @_ZdlPv(ptr noundef nonnull %96) #11
  br label %122

122:                                              ; preds = %119, %121
  %123 = load ptr, ptr %5, align 8, !tbaa !8
  %124 = icmp eq ptr %123, null
  br i1 %124, label %126, label %125

125:                                              ; preds = %122
  call void @_ZdlPv(ptr noundef nonnull %123) #11
  br label %126

126:                                              ; preds = %122, %125
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #12
  br label %351

127:                                              ; preds = %95
  %128 = landingpad { ptr, i32 }
          cleanup
  br label %352

129:                                              ; preds = %106, %311
  %130 = phi ptr [ %102, %106 ], [ %312, %311 ]
  %131 = load i32, ptr %130, align 4, !tbaa !9
  %132 = load ptr, ptr %53, align 8, !tbaa !16
  %133 = sext i32 %131 to i64
  %134 = getelementptr inbounds i64, ptr %132, i64 %133
  %135 = load i64, ptr %134, align 8, !tbaa !19
  %136 = getelementptr i64, ptr %134, i64 1
  %137 = load i64, ptr %136, align 8, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #12
  %138 = load ptr, ptr %59, align 8, !tbaa !20
  %139 = getelementptr inbounds i32, ptr %138, i64 %135
  %140 = getelementptr inbounds i32, ptr %138, i64 %137
  %141 = ptrtoint ptr %140 to i64
  %142 = ptrtoint ptr %139 to i64
  %143 = sub i64 %141, %142
  %144 = ashr exact i64 %143, 2
  %145 = icmp ugt i64 %144, 2305843009213693951
  br i1 %145, label %146, label %148

146:                                              ; preds = %129
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #9
          to label %147 unwind label %160

147:                                              ; preds = %146
  unreachable

148:                                              ; preds = %129
  %149 = icmp eq i64 %137, %135
  br i1 %149, label %152, label %150

150:                                              ; preds = %148
  %151 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %143) #10
          to label %152 unwind label %158

152:                                              ; preds = %150, %148
  %153 = phi ptr [ null, %148 ], [ %151, %150 ]
  store ptr %153, ptr %7, align 8, !tbaa !8
  %154 = getelementptr inbounds i32, ptr %153, i64 %144
  store ptr %154, ptr %108, align 8, !tbaa !12
  %155 = icmp sgt i64 %143, 0
  br i1 %155, label %156, label %162, !prof !21

156:                                              ; preds = %152
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %153, ptr align 4 %139, i64 %143, i1 false)
  %157 = getelementptr inbounds i8, ptr %153, i64 %143
  br label %162

158:                                              ; preds = %150
  %159 = landingpad { ptr, i32 }
          cleanup
  br label %349

160:                                              ; preds = %146
  %161 = landingpad { ptr, i32 }
          cleanup
  br label %349

162:                                              ; preds = %156, %152
  %163 = phi ptr [ %157, %156 ], [ %153, %152 ]
  store ptr %163, ptr %107, align 8, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #12
  %164 = load ptr, ptr %109, align 8, !tbaa !11
  %165 = load ptr, ptr %1, align 8, !tbaa !11
  %166 = ptrtoint ptr %164 to i64
  %167 = ptrtoint ptr %165 to i64
  %168 = sub i64 %166, %167
  %169 = ashr exact i64 %168, 2
  %170 = icmp sgt i64 %169, -1
  call void @llvm.assume(i1 %170)
  %171 = icmp eq ptr %164, %165
  br i1 %171, label %174, label %172

172:                                              ; preds = %162
  %173 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %168) #10
          to label %174 unwind label %314

174:                                              ; preds = %172, %162
  %175 = phi ptr [ null, %162 ], [ %173, %172 ]
  store ptr %175, ptr %8, align 8, !tbaa !8
  %176 = getelementptr inbounds i32, ptr %175, i64 %169
  store ptr %176, ptr %111, align 8, !tbaa !12
  %177 = icmp sgt i64 %168, 0
  br i1 %177, label %178, label %180, !prof !21

178:                                              ; preds = %174
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %175, ptr align 4 %165, i64 %168, i1 false)
  %179 = getelementptr inbounds i8, ptr %175, i64 %168
  br label %180

180:                                              ; preds = %178, %174
  %181 = phi ptr [ %179, %178 ], [ %175, %174 ]
  store ptr %181, ptr %110, align 8, !tbaa !3
  %182 = icmp eq ptr %181, %176
  br i1 %182, label %185, label %183

183:                                              ; preds = %180
  store i32 %131, ptr %181, align 4, !tbaa !9
  %184 = getelementptr inbounds i32, ptr %181, i64 1
  store ptr %184, ptr %110, align 8, !tbaa !3
  br label %207

185:                                              ; preds = %180
  %186 = ashr exact i64 %168, 2
  %187 = icmp sgt i64 %186, -1
  call void @llvm.assume(i1 %187)
  %188 = icmp eq i64 %168, 9223372036854775804
  br i1 %188, label %189, label %191

189:                                              ; preds = %185
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #9
          to label %190 unwind label %318

190:                                              ; preds = %189
  unreachable

191:                                              ; preds = %185
  %192 = call i64 @llvm.umax.i64(i64 %186, i64 1)
  %193 = add nuw nsw i64 %192, %186
  %194 = call noundef i64 @llvm.umin.i64(i64 %193, i64 2305843009213693951)
  %195 = shl nuw nsw i64 %194, 2
  %196 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %195) #10
          to label %197 unwind label %316

197:                                              ; preds = %191
  %198 = getelementptr inbounds i8, ptr %196, i64 %168
  store i32 %131, ptr %198, align 4, !tbaa !9
  %199 = icmp sgt i64 %168, 0
  br i1 %199, label %200, label %201

200:                                              ; preds = %197
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %196, ptr align 4 %175, i64 %168, i1 false)
  br label %201

201:                                              ; preds = %200, %197
  %202 = icmp eq ptr %175, null
  br i1 %202, label %204, label %203

203:                                              ; preds = %201
  call void @_ZdlPv(ptr noundef nonnull %175) #11
  br label %204

204:                                              ; preds = %203, %201
  %205 = getelementptr inbounds i32, ptr %198, i64 1
  store ptr %196, ptr %8, align 8, !tbaa !8
  store ptr %205, ptr %110, align 8, !tbaa !3
  %206 = getelementptr inbounds i32, ptr %196, i64 %194
  store ptr %206, ptr %111, align 8, !tbaa !12
  br label %207

207:                                              ; preds = %204, %183
  %208 = phi ptr [ %196, %204 ], [ %175, %183 ]
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %9) #12
  invoke void @intersect_sorted(ptr dead_on_unwind nonnull writable sret(%"class.std::vector") align 8 %9, ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef nonnull align 8 dereferenceable(24) %7)
          to label %209 unwind label %320

209:                                              ; preds = %207
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %10) #12
  invoke void @intersect_sorted(ptr dead_on_unwind nonnull writable sret(%"class.std::vector") align 8 %10, ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef nonnull align 8 dereferenceable(24) %7)
          to label %210 unwind label %322

210:                                              ; preds = %209
  invoke void @bron_kerbosch_with_pivot(ptr noundef nonnull %0, ptr noundef nonnull align 8 dereferenceable(24) %8, ptr noundef nonnull align 8 dereferenceable(24) %9, ptr noundef nonnull align 8 dereferenceable(24) %10)
          to label %211 unwind label %324

211:                                              ; preds = %210
  %212 = load ptr, ptr %2, align 8, !tbaa !11
  %213 = load ptr, ptr %12, align 8, !tbaa !11
  %214 = icmp eq ptr %212, %213
  br i1 %214, label %240, label %215

215:                                              ; preds = %211, %237
  %216 = phi ptr [ %238, %237 ], [ %212, %211 ]
  %217 = load i32, ptr %216, align 4, !tbaa !9
  %218 = icmp eq i32 %217, %131
  br i1 %218, label %240, label %219

219:                                              ; preds = %215
  %220 = getelementptr inbounds i32, ptr %216, i64 1
  %221 = icmp eq ptr %220, %213
  br i1 %221, label %265, label %222, !llvm.loop !22

222:                                              ; preds = %219
  %223 = load i32, ptr %220, align 4, !tbaa !9
  %224 = icmp eq i32 %223, %131
  br i1 %224, label %240, label %225

225:                                              ; preds = %222
  %226 = getelementptr inbounds i32, ptr %216, i64 2
  %227 = icmp eq ptr %226, %213
  br i1 %227, label %265, label %228, !llvm.loop !22

228:                                              ; preds = %225
  %229 = load i32, ptr %226, align 4, !tbaa !9
  %230 = icmp eq i32 %229, %131
  br i1 %230, label %240, label %231

231:                                              ; preds = %228
  %232 = getelementptr inbounds i32, ptr %216, i64 3
  %233 = icmp eq ptr %232, %213
  br i1 %233, label %265, label %234, !llvm.loop !22

234:                                              ; preds = %231
  %235 = load i32, ptr %232, align 4, !tbaa !9
  %236 = icmp eq i32 %235, %131
  br i1 %236, label %240, label %237

237:                                              ; preds = %234
  %238 = getelementptr inbounds i32, ptr %216, i64 4
  %239 = icmp eq ptr %238, %213
  br i1 %239, label %265, label %215, !llvm.loop !24

240:                                              ; preds = %234, %228, %222, %215, %211
  %241 = phi ptr [ %212, %211 ], [ %216, %215 ], [ %220, %222 ], [ %226, %228 ], [ %232, %234 ]
  %242 = icmp eq ptr %241, %213
  %243 = getelementptr inbounds i32, ptr %241, i64 1
  %244 = icmp eq ptr %243, %213
  %245 = select i1 %242, i1 true, i1 %244
  br i1 %245, label %257, label %246

246:                                              ; preds = %240, %253
  %247 = phi ptr [ %255, %253 ], [ %243, %240 ]
  %248 = phi ptr [ %254, %253 ], [ %241, %240 ]
  %249 = load i32, ptr %247, align 4, !tbaa !9
  %250 = icmp eq i32 %249, %131
  br i1 %250, label %253, label %251

251:                                              ; preds = %246
  store i32 %249, ptr %248, align 4, !tbaa !9
  %252 = getelementptr inbounds i32, ptr %248, i64 1
  br label %253

253:                                              ; preds = %251, %246
  %254 = phi ptr [ %248, %246 ], [ %252, %251 ]
  %255 = getelementptr inbounds i32, ptr %247, i64 1
  %256 = icmp eq ptr %255, %213
  br i1 %256, label %257, label %246, !llvm.loop !26

257:                                              ; preds = %253, %240
  %258 = phi ptr [ %241, %240 ], [ %254, %253 ]
  %259 = icmp eq ptr %258, %213
  br i1 %259, label %265, label %260

260:                                              ; preds = %257
  %261 = ptrtoint ptr %258 to i64
  %262 = ptrtoint ptr %212 to i64
  %263 = sub i64 %261, %262
  %264 = getelementptr inbounds i8, ptr %212, i64 %263
  store ptr %264, ptr %12, align 8, !tbaa !3
  br label %265

265:                                              ; preds = %219, %225, %231, %237, %260, %257
  %266 = load ptr, ptr %39, align 8, !tbaa !11
  %267 = load ptr, ptr %112, align 8, !tbaa !12
  %268 = icmp eq ptr %266, %267
  br i1 %268, label %271, label %269

269:                                              ; preds = %265
  store i32 %131, ptr %266, align 4, !tbaa !9
  %270 = getelementptr inbounds i32, ptr %266, i64 1
  store ptr %270, ptr %39, align 8, !tbaa !3
  br label %297

271:                                              ; preds = %265
  %272 = load ptr, ptr %3, align 8, !tbaa !11
  %273 = ptrtoint ptr %266 to i64
  %274 = ptrtoint ptr %272 to i64
  %275 = sub i64 %273, %274
  %276 = ashr exact i64 %275, 2
  %277 = icmp sgt i64 %276, -1
  call void @llvm.assume(i1 %277)
  %278 = icmp eq i64 %275, 9223372036854775804
  br i1 %278, label %279, label %281

279:                                              ; preds = %271
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #9
          to label %280 unwind label %326

280:                                              ; preds = %279
  unreachable

281:                                              ; preds = %271
  %282 = call i64 @llvm.umax.i64(i64 %276, i64 1)
  %283 = add nuw nsw i64 %282, %276
  %284 = call noundef i64 @llvm.umin.i64(i64 %283, i64 2305843009213693951)
  %285 = shl nuw nsw i64 %284, 2
  %286 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %285) #10
          to label %287 unwind label %324

287:                                              ; preds = %281
  %288 = getelementptr inbounds i8, ptr %286, i64 %275
  store i32 %131, ptr %288, align 4, !tbaa !9
  %289 = icmp sgt i64 %275, 0
  br i1 %289, label %290, label %291

290:                                              ; preds = %287
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %286, ptr align 4 %272, i64 %275, i1 false)
  br label %291

291:                                              ; preds = %290, %287
  %292 = icmp eq ptr %272, null
  br i1 %292, label %294, label %293

293:                                              ; preds = %291
  call void @_ZdlPv(ptr noundef nonnull %272) #11
  br label %294

294:                                              ; preds = %293, %291
  %295 = getelementptr inbounds i32, ptr %288, i64 1
  store ptr %286, ptr %3, align 8, !tbaa !8
  store ptr %295, ptr %39, align 8, !tbaa !3
  %296 = getelementptr inbounds i32, ptr %286, i64 %284
  store ptr %296, ptr %112, align 8, !tbaa !12
  br label %297

297:                                              ; preds = %294, %269
  %298 = load ptr, ptr %10, align 8, !tbaa !8
  %299 = icmp eq ptr %298, null
  br i1 %299, label %301, label %300

300:                                              ; preds = %297
  call void @_ZdlPv(ptr noundef nonnull %298) #11
  br label %301

301:                                              ; preds = %297, %300
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %10) #12
  %302 = load ptr, ptr %9, align 8, !tbaa !8
  %303 = icmp eq ptr %302, null
  br i1 %303, label %305, label %304

304:                                              ; preds = %301
  call void @_ZdlPv(ptr noundef nonnull %302) #11
  br label %305

305:                                              ; preds = %301, %304
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %9) #12
  %306 = icmp eq ptr %208, null
  br i1 %306, label %308, label %307

307:                                              ; preds = %305
  call void @_ZdlPv(ptr noundef nonnull %208) #11
  br label %308

308:                                              ; preds = %305, %307
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #12
  %309 = icmp eq ptr %153, null
  br i1 %309, label %311, label %310

310:                                              ; preds = %308
  call void @_ZdlPv(ptr noundef nonnull %153) #11
  br label %311

311:                                              ; preds = %308, %310
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #12
  %312 = getelementptr inbounds i32, ptr %130, i64 1
  %313 = icmp eq ptr %312, %104
  br i1 %313, label %113, label %129

314:                                              ; preds = %172
  %315 = landingpad { ptr, i32 }
          cleanup
  br label %345

316:                                              ; preds = %191
  %317 = landingpad { ptr, i32 }
          cleanup
  br label %340

318:                                              ; preds = %189
  %319 = landingpad { ptr, i32 }
          cleanup
  br label %340

320:                                              ; preds = %207
  %321 = landingpad { ptr, i32 }
          cleanup
  br label %338

322:                                              ; preds = %209
  %323 = landingpad { ptr, i32 }
          cleanup
  br label %333

324:                                              ; preds = %210, %281
  %325 = landingpad { ptr, i32 }
          cleanup
  br label %328

326:                                              ; preds = %279
  %327 = landingpad { ptr, i32 }
          cleanup
  br label %328

328:                                              ; preds = %326, %324
  %329 = phi { ptr, i32 } [ %325, %324 ], [ %327, %326 ]
  %330 = load ptr, ptr %10, align 8, !tbaa !8
  %331 = icmp eq ptr %330, null
  br i1 %331, label %333, label %332

332:                                              ; preds = %328
  call void @_ZdlPv(ptr noundef nonnull %330) #11
  br label %333

333:                                              ; preds = %332, %328, %322
  %334 = phi { ptr, i32 } [ %323, %322 ], [ %329, %328 ], [ %329, %332 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %10) #12
  %335 = load ptr, ptr %9, align 8, !tbaa !8
  %336 = icmp eq ptr %335, null
  br i1 %336, label %338, label %337

337:                                              ; preds = %333
  call void @_ZdlPv(ptr noundef nonnull %335) #11
  br label %338

338:                                              ; preds = %337, %333, %320
  %339 = phi { ptr, i32 } [ %321, %320 ], [ %334, %333 ], [ %334, %337 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %9) #12
  br label %340

340:                                              ; preds = %316, %318, %338
  %341 = phi ptr [ %208, %338 ], [ %175, %316 ], [ %175, %318 ]
  %342 = phi { ptr, i32 } [ %339, %338 ], [ %317, %316 ], [ %319, %318 ]
  %343 = icmp eq ptr %341, null
  br i1 %343, label %345, label %344

344:                                              ; preds = %340
  call void @_ZdlPv(ptr noundef nonnull %341) #11
  br label %345

345:                                              ; preds = %344, %340, %314
  %346 = phi { ptr, i32 } [ %315, %314 ], [ %342, %340 ], [ %342, %344 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #12
  %347 = icmp eq ptr %153, null
  br i1 %347, label %349, label %348

348:                                              ; preds = %345
  call void @_ZdlPv(ptr noundef nonnull %153) #11
  br label %349

349:                                              ; preds = %158, %160, %348, %345
  %350 = phi { ptr, i32 } [ %346, %345 ], [ %346, %348 ], [ %159, %158 ], [ %161, %160 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #12
  br label %352

351:                                              ; preds = %15, %126
  ret void

352:                                              ; preds = %349, %127
  %353 = phi { ptr, i32 } [ %350, %349 ], [ %128, %127 ]
  %354 = load ptr, ptr %6, align 8, !tbaa !8
  %355 = icmp eq ptr %354, null
  br i1 %355, label %357, label %356

356:                                              ; preds = %352
  call void @_ZdlPv(ptr noundef nonnull %354) #11
  br label %357

357:                                              ; preds = %352, %356
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #12
  %358 = icmp eq ptr %96, null
  br i1 %358, label %360, label %359

359:                                              ; preds = %357
  call void @_ZdlPv(ptr noundef nonnull %96) #11
  br label %360

360:                                              ; preds = %75, %357, %359, %77
  %361 = phi { ptr, i32 } [ %78, %77 ], [ %76, %75 ], [ %353, %357 ], [ %353, %359 ]
  %362 = load ptr, ptr %5, align 8, !tbaa !8
  %363 = icmp eq ptr %362, null
  br i1 %363, label %365, label %364

364:                                              ; preds = %360
  call void @_ZdlPv(ptr noundef nonnull %362) #11
  br label %365

365:                                              ; preds = %360, %364
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #12
  resume { ptr, i32 } %361
}

; Function Attrs: mustprogress uwtable
define dso_local void @bk_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca %"class.std::vector", align 8
  %3 = alloca %"class.std::vector", align 8
  %4 = alloca %"class.std::vector", align 8
  %5 = icmp eq ptr %0, null
  br i1 %5, label %57, label %6

6:                                                ; preds = %1
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %2) #12
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %2, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %3) #12
  %7 = load i64, ptr %0, align 8, !tbaa !27
  %8 = icmp ugt i64 %7, 2305843009213693951
  br i1 %8, label %9, label %10

9:                                                ; preds = %6
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #9
  unreachable

10:                                               ; preds = %6
  %11 = icmp eq i64 %7, 0
  br i1 %11, label %21, label %12

12:                                               ; preds = %10
  %13 = shl nuw nsw i64 %7, 2
  %14 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #10
  store ptr %14, ptr %3, align 8, !tbaa !8
  %15 = getelementptr inbounds i32, ptr %14, i64 %7
  %16 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %3, i64 0, i32 2
  store ptr %15, ptr %16, align 8, !tbaa !12
  store i32 0, ptr %14, align 4, !tbaa !9
  %17 = getelementptr i32, ptr %14, i64 1
  %18 = icmp eq i64 %7, 1
  br i1 %18, label %22, label %19

19:                                               ; preds = %12
  %20 = add nsw i64 %13, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %17, i8 0, i64 %20, i1 false), !tbaa !9
  br label %22

21:                                               ; preds = %10
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %3, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #12
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %4, i8 0, i64 24, i1 false)
  br label %41

22:                                               ; preds = %19, %12
  %23 = phi ptr [ %15, %19 ], [ %17, %12 ]
  %24 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  store ptr %23, ptr %24, align 8, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #12
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %4, i8 0, i64 24, i1 false)
  %25 = icmp ult i64 %7, 8
  br i1 %25, label %39, label %26

26:                                               ; preds = %22
  %27 = and i64 %7, 2305843009213693944
  br label %28

28:                                               ; preds = %28, %26
  %29 = phi i64 [ 0, %26 ], [ %34, %28 ]
  %30 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %26 ], [ %35, %28 ]
  %31 = add <4 x i32> %30, splat (i32 4)
  %32 = getelementptr inbounds i32, ptr %14, i64 %29
  %33 = getelementptr inbounds i32, ptr %32, i64 4
  store <4 x i32> %30, ptr %32, align 4, !tbaa !9
  store <4 x i32> %31, ptr %33, align 4, !tbaa !9
  %34 = add nuw i64 %29, 8
  %35 = add <4 x i32> %30, splat (i32 8)
  %36 = icmp eq i64 %34, %27
  br i1 %36, label %37, label %28, !llvm.loop !28

37:                                               ; preds = %28
  %38 = icmp eq i64 %7, %27
  br i1 %38, label %41, label %39

39:                                               ; preds = %22, %37
  %40 = phi i64 [ 0, %22 ], [ %27, %37 ]
  br label %42

41:                                               ; preds = %42, %37, %21
  invoke void @bron_kerbosch_with_pivot(ptr noundef nonnull %0, ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef nonnull align 8 dereferenceable(24) %4)
          to label %48 unwind label %58

42:                                               ; preds = %39, %42
  %43 = phi i64 [ %46, %42 ], [ %40, %39 ]
  %44 = getelementptr inbounds i32, ptr %14, i64 %43
  %45 = trunc i64 %43 to i32
  store i32 %45, ptr %44, align 4, !tbaa !9
  %46 = add nuw nsw i64 %43, 1
  %47 = icmp eq i64 %46, %7
  br i1 %47, label %41, label %42, !llvm.loop !31

48:                                               ; preds = %41
  %49 = load ptr, ptr %4, align 8, !tbaa !8
  %50 = icmp eq ptr %49, null
  br i1 %50, label %52, label %51

51:                                               ; preds = %48
  tail call void @_ZdlPv(ptr noundef nonnull %49) #11
  br label %52

52:                                               ; preds = %48, %51
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #12
  %53 = load ptr, ptr %3, align 8, !tbaa !8
  %54 = icmp eq ptr %53, null
  br i1 %54, label %56, label %55

55:                                               ; preds = %52
  tail call void @_ZdlPv(ptr noundef nonnull %53) #11
  br label %56

56:                                               ; preds = %55, %52
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #12
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #12
  br label %57

57:                                               ; preds = %1, %56
  ret void

58:                                               ; preds = %41
  %59 = landingpad { ptr, i32 }
          cleanup
  %60 = load ptr, ptr %4, align 8, !tbaa !8
  %61 = icmp eq ptr %60, null
  br i1 %61, label %63, label %62

62:                                               ; preds = %58
  tail call void @_ZdlPv(ptr noundef nonnull %60) #11
  br label %63

63:                                               ; preds = %58, %62
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #12
  %64 = load ptr, ptr %3, align 8, !tbaa !8
  %65 = icmp eq ptr %64, null
  br i1 %65, label %67, label %66

66:                                               ; preds = %63
  tail call void @_ZdlPv(ptr noundef nonnull %64) #11
  br label %67

67:                                               ; preds = %63, %66
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #12
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #12
  resume { ptr, i32 } %59
}

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #2

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #3

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #4

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

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
  %13 = load ptr, ptr %12, align 8, !tbaa !12
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
  %45 = load <4 x i32>, ptr %41, align 4, !tbaa !9
  %46 = load <4 x i32>, ptr %44, align 4, !tbaa !9
  %47 = getelementptr i32, ptr %43, i64 4
  store <4 x i32> %45, ptr %43, align 4, !tbaa !9
  store <4 x i32> %46, ptr %47, align 4, !tbaa !9
  %48 = add nuw i64 %39, 8
  %49 = icmp eq i64 %48, %33
  br i1 %49, label %50, label %38, !llvm.loop !32

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
  %58 = load i32, ptr %56, align 4, !tbaa !9
  store i32 %58, ptr %57, align 4, !tbaa !9
  %59 = getelementptr inbounds i32, ptr %56, i64 1
  %60 = getelementptr inbounds i32, ptr %57, i64 1
  %61 = icmp eq ptr %59, %15
  br i1 %61, label %62, label %55, !llvm.loop !33

62:                                               ; preds = %55, %50
  %63 = getelementptr inbounds i8, ptr %15, i64 %10
  store ptr %63, ptr %14, align 8, !tbaa !3
  %64 = ptrtoint ptr %27 to i64
  %65 = sub i64 %64, %21
  %66 = ashr exact i64 %65, 2
  %67 = sub nsw i64 0, %66
  %68 = getelementptr inbounds i32, ptr %15, i64 %67
  %69 = icmp sgt i64 %66, 1
  br i1 %69, label %70, label %71, !prof !15

70:                                               ; preds = %62
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %68, ptr align 4 %1, i64 %65, i1 false)
  br label %75

71:                                               ; preds = %62
  %72 = icmp eq i64 %65, 4
  br i1 %72, label %73, label %75

73:                                               ; preds = %71
  %74 = load i32, ptr %1, align 4, !tbaa !9
  store i32 %74, ptr %68, align 4, !tbaa !9
  br label %75

75:                                               ; preds = %70, %71, %73
  %76 = icmp sgt i64 %10, 4
  br i1 %76, label %77, label %78, !prof !21

77:                                               ; preds = %75
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %1, ptr align 4 %2, i64 %10, i1 false)
  br label %255

78:                                               ; preds = %75
  %79 = icmp eq i64 %10, 4
  br i1 %79, label %80, label %255

80:                                               ; preds = %78
  %81 = load i32, ptr %2, align 4, !tbaa !9
  store i32 %81, ptr %1, align 4, !tbaa !9
  br label %255

82:                                               ; preds = %20
  %83 = getelementptr inbounds i8, ptr %2, i64 %22
  %84 = ptrtoint ptr %83 to i64
  %85 = sub i64 %8, %84
  %86 = icmp sgt i64 %85, 0
  br i1 %86, label %87, label %89, !prof !21

87:                                               ; preds = %82
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %15, ptr align 4 %83, i64 %85, i1 false)
  %88 = load ptr, ptr %14, align 8, !tbaa !3
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
  %118 = load <4 x i32>, ptr %114, align 4, !tbaa !9
  %119 = load <4 x i32>, ptr %117, align 4, !tbaa !9
  %120 = getelementptr i32, ptr %116, i64 4
  store <4 x i32> %118, ptr %116, align 4, !tbaa !9
  store <4 x i32> %119, ptr %120, align 4, !tbaa !9
  %121 = add nuw i64 %112, 8
  %122 = icmp eq i64 %121, %106
  br i1 %122, label %123, label %111, !llvm.loop !34

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
  %131 = load i32, ptr %129, align 4, !tbaa !9
  store i32 %131, ptr %130, align 4, !tbaa !9
  %132 = getelementptr inbounds i32, ptr %129, i64 1
  %133 = getelementptr inbounds i32, ptr %130, i64 1
  %134 = icmp eq ptr %132, %15
  br i1 %134, label %135, label %128, !llvm.loop !35

135:                                              ; preds = %128, %123, %89
  %136 = getelementptr inbounds i8, ptr %90, i64 %10
  store ptr %136, ptr %14, align 8, !tbaa !3
  %137 = icmp sgt i64 %22, 4
  br i1 %137, label %138, label %139, !prof !21

138:                                              ; preds = %135
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %1, ptr align 4 %2, i64 %22, i1 false)
  br label %255

139:                                              ; preds = %135
  %140 = icmp eq i64 %22, 4
  br i1 %140, label %141, label %255

141:                                              ; preds = %139
  %142 = load i32, ptr %2, align 4, !tbaa !9
  store i32 %142, ptr %1, align 4, !tbaa !9
  br label %255

143:                                              ; preds = %7
  %144 = load ptr, ptr %0, align 8, !tbaa !8
  %145 = ptrtoint ptr %144 to i64
  %146 = sub i64 %17, %145
  %147 = ashr exact i64 %146, 2
  %148 = icmp sgt i64 %147, -1
  tail call void @llvm.assume(i1 %148)
  %149 = sub nuw nsw i64 2305843009213693951, %147
  %150 = icmp ult i64 %149, %11
  br i1 %150, label %151, label %152

151:                                              ; preds = %143
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #9
  unreachable

152:                                              ; preds = %143
  %153 = tail call i64 @llvm.umax.i64(i64 %147, i64 %11)
  %154 = add nuw nsw i64 %153, %147
  %155 = tail call noundef i64 @llvm.umin.i64(i64 %154, i64 2305843009213693951)
  %156 = shl nuw nsw i64 %155, 2
  %157 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %156) #10
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
  %182 = load <4 x i32>, ptr %178, align 4, !tbaa !9
  %183 = load <4 x i32>, ptr %181, align 4, !tbaa !9
  %184 = getelementptr i32, ptr %180, i64 4
  store <4 x i32> %182, ptr %180, align 4, !tbaa !9
  store <4 x i32> %183, ptr %184, align 4, !tbaa !9
  %185 = add nuw i64 %176, 8
  %186 = icmp eq i64 %185, %170
  br i1 %186, label %187, label %175, !llvm.loop !36

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
  %195 = load i32, ptr %193, align 4, !tbaa !9
  store i32 %195, ptr %194, align 4, !tbaa !9
  %196 = getelementptr inbounds i32, ptr %193, i64 1
  %197 = getelementptr inbounds i32, ptr %194, i64 1
  %198 = icmp eq ptr %196, %1
  br i1 %198, label %199, label %192, !llvm.loop !37

199:                                              ; preds = %192, %187, %152
  %200 = phi ptr [ %157, %152 ], [ %174, %187 ], [ %197, %192 ]
  %201 = ptrtoint ptr %200 to i64
  %202 = icmp sgt i64 %10, 0
  br i1 %202, label %203, label %205, !prof !21

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
  %232 = load <4 x i32>, ptr %228, align 4, !tbaa !9
  %233 = load <4 x i32>, ptr %231, align 4, !tbaa !9
  %234 = getelementptr i32, ptr %230, i64 4
  store <4 x i32> %232, ptr %230, align 4, !tbaa !9
  store <4 x i32> %233, ptr %234, align 4, !tbaa !9
  %235 = add nuw i64 %226, 8
  %236 = icmp eq i64 %235, %220
  br i1 %236, label %237, label %225, !llvm.loop !38

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
  %245 = load i32, ptr %243, align 4, !tbaa !9
  store i32 %245, ptr %244, align 4, !tbaa !9
  %246 = getelementptr inbounds i32, ptr %243, i64 1
  %247 = getelementptr inbounds i32, ptr %244, i64 1
  %248 = icmp eq ptr %246, %15
  br i1 %248, label %249, label %242, !llvm.loop !39

249:                                              ; preds = %242, %237, %205
  %250 = phi ptr [ %206, %205 ], [ %224, %237 ], [ %247, %242 ]
  %251 = icmp eq ptr %144, null
  br i1 %251, label %253, label %252

252:                                              ; preds = %249
  tail call void @_ZdlPv(ptr noundef nonnull %144) #11
  br label %253

253:                                              ; preds = %249, %252
  store ptr %157, ptr %0, align 8, !tbaa !8
  store ptr %250, ptr %14, align 8, !tbaa !3
  %254 = getelementptr inbounds i32, ptr %157, i64 %155
  store ptr %254, ptr %12, align 8, !tbaa !12
  br label %255

255:                                              ; preds = %141, %139, %138, %80, %78, %77, %253, %4
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #5

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local ptr @_ZSt16__set_differenceIN9__gnu_cxx17__normal_iteratorIPiSt6vectorIiSaIiEEEES6_St20back_insert_iteratorIS5_ENS0_5__ops15_Iter_less_iterEET1_T_SC_T0_SD_SB_T2_(ptr %0, ptr %1, ptr %2, ptr %3, ptr %4) local_unnamed_addr #0 comdat personality ptr @__gxx_personality_v0 {
  %6 = icmp ne ptr %0, %1
  %7 = icmp ne ptr %2, %3
  %8 = select i1 %6, i1 %7, i1 false
  br i1 %8, label %9, label %64

9:                                                ; preds = %5
  %10 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  %11 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %4, i64 0, i32 2
  br label %12

12:                                               ; preds = %9, %58
  %13 = phi ptr [ %0, %9 ], [ %60, %58 ]
  %14 = phi ptr [ %2, %9 ], [ %59, %58 ]
  %15 = load i32, ptr %13, align 4, !tbaa !9
  %16 = load i32, ptr %14, align 4, !tbaa !9
  %17 = icmp slt i32 %15, %16
  br i1 %17, label %18, label %51

18:                                               ; preds = %12
  %19 = load ptr, ptr %10, align 8, !tbaa !11
  %20 = load ptr, ptr %11, align 8, !tbaa !12
  %21 = icmp eq ptr %19, %20
  br i1 %21, label %24, label %22

22:                                               ; preds = %18
  store i32 %15, ptr %19, align 4, !tbaa !9
  %23 = getelementptr inbounds i32, ptr %19, i64 1
  store ptr %23, ptr %10, align 8, !tbaa !3
  br label %49

24:                                               ; preds = %18
  %25 = load ptr, ptr %4, align 8, !tbaa !11
  %26 = ptrtoint ptr %19 to i64
  %27 = ptrtoint ptr %25 to i64
  %28 = sub i64 %26, %27
  %29 = ashr exact i64 %28, 2
  %30 = icmp sgt i64 %29, -1
  tail call void @llvm.assume(i1 %30)
  %31 = icmp eq i64 %28, 9223372036854775804
  br i1 %31, label %32, label %33

32:                                               ; preds = %24
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #9
  unreachable

33:                                               ; preds = %24
  %34 = tail call i64 @llvm.umax.i64(i64 %29, i64 1)
  %35 = add nuw nsw i64 %34, %29
  %36 = tail call noundef i64 @llvm.umin.i64(i64 %35, i64 2305843009213693951)
  %37 = shl nuw nsw i64 %36, 2
  %38 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %37) #10
  %39 = getelementptr inbounds i8, ptr %38, i64 %28
  %40 = load i32, ptr %13, align 4, !tbaa !9
  store i32 %40, ptr %39, align 4, !tbaa !9
  %41 = icmp sgt i64 %28, 0
  br i1 %41, label %42, label %43

42:                                               ; preds = %33
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %38, ptr align 4 %25, i64 %28, i1 false)
  br label %43

43:                                               ; preds = %42, %33
  %44 = icmp eq ptr %25, null
  br i1 %44, label %46, label %45

45:                                               ; preds = %43
  tail call void @_ZdlPv(ptr noundef nonnull %25) #11
  br label %46

46:                                               ; preds = %45, %43
  %47 = getelementptr inbounds i32, ptr %39, i64 1
  store ptr %38, ptr %4, align 8, !tbaa !8
  store ptr %47, ptr %10, align 8, !tbaa !3
  %48 = getelementptr inbounds i32, ptr %38, i64 %36
  store ptr %48, ptr %11, align 8, !tbaa !12
  br label %49

49:                                               ; preds = %22, %46
  %50 = getelementptr inbounds i32, ptr %13, i64 1
  br label %58

51:                                               ; preds = %12
  %52 = icmp slt i32 %16, %15
  br i1 %52, label %53, label %55

53:                                               ; preds = %51
  %54 = getelementptr inbounds i32, ptr %14, i64 1
  br label %58

55:                                               ; preds = %51
  %56 = getelementptr inbounds i32, ptr %13, i64 1
  %57 = getelementptr inbounds i32, ptr %14, i64 1
  br label %58

58:                                               ; preds = %53, %55, %49
  %59 = phi ptr [ %14, %49 ], [ %54, %53 ], [ %57, %55 ]
  %60 = phi ptr [ %50, %49 ], [ %13, %53 ], [ %56, %55 ]
  %61 = icmp ne ptr %60, %1
  %62 = icmp ne ptr %59, %3
  %63 = select i1 %61, i1 %62, i1 false
  br i1 %63, label %12, label %64, !llvm.loop !40

64:                                               ; preds = %58, %5
  %65 = phi ptr [ %0, %5 ], [ %60, %58 ]
  %66 = icmp eq ptr %65, %1
  br i1 %66, label %110, label %67

67:                                               ; preds = %64
  %68 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  %69 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %4, i64 0, i32 2
  %70 = load ptr, ptr %68, align 8, !tbaa !11
  %71 = load ptr, ptr %69, align 8, !tbaa !12
  br label %72

72:                                               ; preds = %105, %67
  %73 = phi ptr [ %71, %67 ], [ %106, %105 ]
  %74 = phi ptr [ %70, %67 ], [ %107, %105 ]
  %75 = phi ptr [ %65, %67 ], [ %108, %105 ]
  %76 = icmp eq ptr %74, %73
  br i1 %76, label %80, label %77

77:                                               ; preds = %72
  %78 = load i32, ptr %75, align 4, !tbaa !9
  store i32 %78, ptr %74, align 4, !tbaa !9
  %79 = getelementptr inbounds i32, ptr %74, i64 1
  store ptr %79, ptr %68, align 8, !tbaa !3
  br label %105

80:                                               ; preds = %72
  %81 = load ptr, ptr %4, align 8, !tbaa !11
  %82 = ptrtoint ptr %73 to i64
  %83 = ptrtoint ptr %81 to i64
  %84 = sub i64 %82, %83
  %85 = ashr exact i64 %84, 2
  %86 = icmp sgt i64 %85, -1
  tail call void @llvm.assume(i1 %86)
  %87 = icmp eq i64 %84, 9223372036854775804
  br i1 %87, label %88, label %89

88:                                               ; preds = %80
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #9
  unreachable

89:                                               ; preds = %80
  %90 = tail call i64 @llvm.umax.i64(i64 %85, i64 1)
  %91 = add nuw nsw i64 %90, %85
  %92 = tail call noundef i64 @llvm.umin.i64(i64 %91, i64 2305843009213693951)
  %93 = shl nuw nsw i64 %92, 2
  %94 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %93) #10
  %95 = getelementptr inbounds i8, ptr %94, i64 %84
  %96 = load i32, ptr %75, align 4, !tbaa !9
  store i32 %96, ptr %95, align 4, !tbaa !9
  %97 = icmp sgt i64 %84, 0
  br i1 %97, label %98, label %99

98:                                               ; preds = %89
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %94, ptr align 4 %81, i64 %84, i1 false)
  br label %99

99:                                               ; preds = %98, %89
  %100 = icmp eq ptr %81, null
  br i1 %100, label %102, label %101

101:                                              ; preds = %99
  tail call void @_ZdlPv(ptr noundef nonnull %81) #11
  br label %102

102:                                              ; preds = %101, %99
  %103 = getelementptr inbounds i32, ptr %95, i64 1
  store ptr %94, ptr %4, align 8, !tbaa !8
  store ptr %103, ptr %68, align 8, !tbaa !3
  %104 = getelementptr inbounds i32, ptr %94, i64 %92
  store ptr %104, ptr %69, align 8, !tbaa !12
  br label %105

105:                                              ; preds = %102, %77
  %106 = phi ptr [ %73, %77 ], [ %104, %102 ]
  %107 = phi ptr [ %79, %77 ], [ %103, %102 ]
  %108 = getelementptr inbounds i32, ptr %75, i64 1
  %109 = icmp eq ptr %108, %1
  br i1 %109, label %110, label %72, !llvm.loop !41

110:                                              ; preds = %105, %64
  ret ptr %4
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #8

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #6 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #7 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #9 = { cold noreturn }
attributes #10 = { builtin allocsize(0) }
attributes #11 = { builtin nounwind }
attributes #12 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!3 = !{!4, !5, i64 8}
!4 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !5, i64 0, !5, i64 8, !5, i64 16}
!5 = !{!"any pointer", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!4, !5, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !6, i64 0}
!11 = !{!5, !5, i64 0}
!12 = !{!4, !5, i64 16}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.mustprogress"}
!15 = !{!"branch_weights", i32 -2147483648, i32 0}
!16 = !{!17, !5, i64 16}
!17 = !{!"_ZTS5Graph", !18, i64 0, !18, i64 8, !5, i64 16, !5, i64 24}
!18 = !{!"long", !6, i64 0}
!19 = !{!18, !18, i64 0}
!20 = !{!17, !5, i64 24}
!21 = !{!"branch_weights", i32 2000, i32 1}
!22 = distinct !{!22, !14, !23}
!23 = !{!"llvm.loop.unroll.count", i32 4}
!24 = distinct !{!24, !14, !25}
!25 = !{!"llvm.loop.unroll.disable"}
!26 = distinct !{!26, !14}
!27 = !{!17, !18, i64 0}
!28 = distinct !{!28, !14, !29, !30}
!29 = !{!"llvm.loop.isvectorized", i32 1}
!30 = !{!"llvm.loop.unroll.runtime.disable"}
!31 = distinct !{!31, !14, !30, !29}
!32 = distinct !{!32, !14, !29, !30}
!33 = distinct !{!33, !14, !29}
!34 = distinct !{!34, !14, !29, !30}
!35 = distinct !{!35, !14, !29}
!36 = distinct !{!36, !14, !29, !30}
!37 = distinct !{!37, !14, !29}
!38 = distinct !{!38, !14, !29, !30}
!39 = distinct !{!39, !14, !29}
!40 = distinct !{!40, !14}
!41 = distinct !{!41, !14}
