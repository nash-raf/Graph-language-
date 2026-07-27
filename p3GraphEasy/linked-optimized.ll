; ModuleID = 'linked.opt.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }
%struct.timespec = type { i64, i64 }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl" }
%"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl" = type { %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data" }
%"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::allocator.2" = type { i8 }
%struct.Graph = type { i64, i64, ptr, ptr }
%"struct.std::_UninitDestroyGuard.5" = type { ptr, ptr }
%"class.__gnu_cxx::__normal_iterator" = type { ptr }

$_ZNSt6vectorIiSaIiEEC2EmRKiRKS0_ = comdat any

$_ZNSt6vectorIS_IiSaIiEESaIS1_EEC2EmRKS1_RKS2_ = comdat any

$_ZNSt15__new_allocatorISt6vectorIiSaIiEEED2Ev = comdat any

$_ZNSt6vectorIiSaIiEED2Ev = comdat any

$_ZNSt15__new_allocatorIiED2Ev = comdat any

$_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm = comdat any

$_ZNSt6vectorIiSaIiEEixEm = comdat any

$_ZNSt6vectorIS_IiSaIiEESaIS1_EED2Ev = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE19_M_get_Tp_allocatorEv = comdat any

$_ZSt8_DestroyIPSt6vectorIiSaIiEEEvT_S4_ = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EED2Ev = comdat any

$__clang_call_terminate = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE13_M_deallocateEPS2_m = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implD2Ev = comdat any

$_ZNSt15__new_allocatorISt6vectorIiSaIiEEE10deallocateEPS2_m = comdat any

$_ZSt8_DestroyISt6vectorIiSaIiEEEvPT_ = comdat any

$_ZNKSt6vectorIiSaIiEE4sizeEv = comdat any

$_ZNKSt6vectorIS_IiSaIiEESaIS1_EE4sizeEv = comdat any

$_ZNSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv = comdat any

$_ZSt8_DestroyIPiEvT_S1_ = comdat any

$_ZNSt12_Vector_baseIiSaIiEED2Ev = comdat any

$_ZNSt12_Vector_baseIiSaIiEE13_M_deallocateEPim = comdat any

$_ZNSt12_Vector_baseIiSaIiEE12_Vector_implD2Ev = comdat any

$_ZNSt15__new_allocatorIiE10deallocateEPim = comdat any

$_ZNSt6vectorIS_IiSaIiEESaIS1_EE17_S_check_init_lenEmRKS2_ = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EEC2EmRKS3_ = comdat any

$_ZNSt6vectorIS_IiSaIiEESaIS1_EE18_M_fill_initializeEmRKS1_ = comdat any

$_ZSt24__uninitialized_fill_n_aIPSt6vectorIiSaIiEEmS2_S2_ET_S4_T0_RKT1_RSaIT2_E = comdat any

$_ZSt20uninitialized_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_ = comdat any

$_ZSt18__do_uninit_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_ = comdat any

$_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvEC2ERS3_ = comdat any

$_ZSt10_ConstructISt6vectorIiSaIiEEJRKS2_EEvPT_DpOT0_ = comdat any

$_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvED2Ev = comdat any

$_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvE7releaseEv = comdat any

$_ZNSt6vectorIiSaIiEEC2ERKS1_ = comdat any

$_ZNKSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv = comdat any

$_ZNSt12_Vector_baseIiSaIiEEC2EmRKS0_ = comdat any

$_ZNKSt6vectorIiSaIiEE5beginEv = comdat any

$_ZNKSt6vectorIiSaIiEE3endEv = comdat any

$_ZSt22__uninitialized_copy_aIN9__gnu_cxx17__normal_iteratorIPKiSt6vectorIiSaIiEEEES7_PiiET1_T_T0_S9_RSaIT2_E = comdat any

$_ZSt18uninitialized_copyIN9__gnu_cxx17__normal_iteratorIPKiSt6vectorIiSaIiEEEEPiET0_T_SA_S9_ = comdat any

$_ZNSt12_Vector_baseIiSaIiEE12_Vector_implC2ERKS0_ = comdat any

$_ZNSt12_Vector_baseIiSaIiEE17_M_create_storageEm = comdat any

$_ZNSt12_Vector_baseIiSaIiEE11_M_allocateEm = comdat any

$_ZNSt15__new_allocatorIiE8allocateEmPKv = comdat any

$_ZNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataC2Ev = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implC2ERKS3_ = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE17_M_create_storageEm = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE11_M_allocateEm = comdat any

$_ZNSt15__new_allocatorISt6vectorIiSaIiEEE8allocateEmPKv = comdat any

$_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE17_Vector_impl_dataC2Ev = comdat any

$_ZNSt6vectorIS_IiSaIiEESaIS1_EE11_S_max_sizeERKS2_ = comdat any

$_ZSt3minImERKT_S2_S2_ = comdat any

$_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_ = comdat any

$_ZNSt6vectorIiSaIiEE18_M_fill_initializeEmRKi = comdat any

$_ZSt24__uninitialized_fill_n_aIPimiiET_S1_T0_RKT1_RSaIT2_E = comdat any

$_ZSt20uninitialized_fill_nIPimiET_S1_T0_RKT1_ = comdat any

$_ZSt18__do_uninit_fill_nIPimiET_S1_T0_RKT1_ = comdat any

$_ZNSt19_UninitDestroyGuardIPivEC2ERS0_ = comdat any

$_ZSt10_ConstructIiJRKiEEvPT_DpOT0_ = comdat any

$_ZNSt19_UninitDestroyGuardIPivED2Ev = comdat any

$_ZNSt19_UninitDestroyGuardIPivE7releaseEv = comdat any

$_ZNSt6vectorIiSaIiEE11_S_max_sizeERKS0_ = comdat any

$_ZNKSt6vectorIS_IiSaIiEESaIS1_EEixEm = comdat any

$_ZNKSt6vectorIiSaIiEEixEm = comdat any

$_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag = comdat any

$_ZSt16__set_differenceIN9__gnu_cxx17__normal_iteratorIPiSt6vectorIiSaIiEEEES6_St20back_insert_iteratorIS5_ENS0_5__ops15_Iter_less_iterEET1_T_SC_T0_SD_SB_T2_ = comdat any

@0 = private unnamed_addr constant [18 x i8] c"Elapsed: %lld ms\0A\00", align 1
@.str = private unnamed_addr constant [5 x i8] c"INF \00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"%3d \00", align 1
@.str.2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.str.3 = private unnamed_addr constant [27 x i8] c"Initial adjacency matrix:\0A\00", align 1
@.str.4 = private unnamed_addr constant [34 x i8] c"\0AAll-pairs shortest path matrix:\0A\00", align 1
@.str.5 = private unnamed_addr constant [62 x i8] c"\0A[Floyd-Warshall] Completed in %.6f seconds using %d threads\0A\00", align 1
@.str.7 = private unnamed_addr constant [95 x i8] c"/usr/bin/../lib/gcc/x86_64-redhat-linux/15/../../../../include/c++/15/bits/stl_uninitialized.h\00", align 1
@__PRETTY_FUNCTION__._ZSt18__do_uninit_fill_nIPimiET_S1_T0_RKT1_ = private unnamed_addr constant [140 x i8] c"_ForwardIterator std::__do_uninit_fill_n(_ForwardIterator, _Size, const _Tp &) [_ForwardIterator = int *, _Size = unsigned long, _Tp = int]\00", align 1
@.str.8 = private unnamed_addr constant [9 x i8] c"__n >= 0\00", align 1
@__PRETTY_FUNCTION__._ZSt18__do_uninit_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_ = private unnamed_addr constant [166 x i8] c"_ForwardIterator std::__do_uninit_fill_n(_ForwardIterator, _Size, const _Tp &) [_ForwardIterator = std::vector<int> *, _Size = unsigned long, _Tp = std::vector<int>]\00", align 1
@.str.9 = private unnamed_addr constant [88 x i8] c"/usr/bin/../lib/gcc/x86_64-redhat-linux/15/../../../../include/c++/15/bits/stl_vector.h\00", align 1
@__PRETTY_FUNCTION__._ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm = private unnamed_addr constant [131 x i8] c"reference std::vector<std::vector<int>>::operator[](size_type) [_Tp = std::vector<int>, _Alloc = std::allocator<std::vector<int>>]\00", align 1
@.str.10 = private unnamed_addr constant [19 x i8] c"__n < this->size()\00", align 1
@__PRETTY_FUNCTION__._ZNSt6vectorIiSaIiEEixEm = private unnamed_addr constant [92 x i8] c"reference std::vector<int>::operator[](size_type) [_Tp = int, _Alloc = std::allocator<int>]\00", align 1
@__PRETTY_FUNCTION__._ZNKSt6vectorIS_IiSaIiEESaIS1_EEixEm = private unnamed_addr constant [143 x i8] c"const_reference std::vector<std::vector<int>>::operator[](size_type) const [_Tp = std::vector<int>, _Alloc = std::allocator<std::vector<int>>]\00", align 1
@__PRETTY_FUNCTION__._ZNKSt6vectorIiSaIiEEixEm = private unnamed_addr constant [104 x i8] c"const_reference std::vector<int>::operator[](size_type) const [_Tp = int, _Alloc = std::allocator<int>]\00", align 1
@.str.1.12 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@1 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@2 = private unnamed_addr constant %struct.ident_t { i32 0, i32 514, i32 0, i32 22, ptr @1 }, align 8
@3 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 22, ptr @1 }, align 8
@.gomp_critical_user_.var = common global [8 x i32] zeroinitializer, align 8
@.str.17 = private unnamed_addr constant [50 x i8] c"[BFS] Completed in %.6f seconds using %d threads\0A\00", align 1
@.str.4.16 = private unnamed_addr constant [24 x i8] c"vector::_M_range_insert\00", align 1
@.str.18 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@.str.2.19 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1

define noundef i32 @main() local_unnamed_addr {
entry:
  %tStart = alloca %struct.timespec, align 8
  %tEnd = alloca %struct.timespec, align 8
  %0 = call i32 @clock_gettime(i32 1, ptr nonnull %tStart)
  %1 = call i32 @clock_gettime(i32 1, ptr nonnull %tEnd)
  %start_sec = load i64, ptr %tStart, align 8
  %2 = getelementptr inbounds %struct.timespec, ptr %tStart, i64 0, i32 1
  %start_nsec = load i64, ptr %2, align 8
  %end_sec = load i64, ptr %tEnd, align 8
  %3 = getelementptr inbounds %struct.timespec, ptr %tEnd, i64 0, i32 1
  %end_nsec = load i64, ptr %3, align 8
  %secDiff = sub i64 %end_sec, %start_sec
  %nsecDiff = sub i64 %end_nsec, %start_nsec
  %secToNS = mul i64 %secDiff, 1000000000
  %elapsedNS = add i64 %nsecDiff, %secToNS
  %elapsedMS = sdiv i64 %elapsedNS, 1000000
  %4 = call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i64 %elapsedMS)
  ret i32 0
}

declare i32 @clock_gettime(i32, ptr) local_unnamed_addr

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @csr_to_adj_matrix(ptr dead_on_unwind noalias writable sret(%"class.std::vector") align 8 %0, ptr noundef %1) local_unnamed_addr #2 personality ptr @__gxx_personality_v0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca i64, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca i32, align 4
  %17 = alloca i1, align 1
  %18 = alloca %"class.std::vector", align 8
  %19 = alloca %"class.std::allocator.2", align 1
  %20 = alloca ptr, align 8
  %21 = alloca i32, align 4
  %22 = alloca %"class.std::allocator.2", align 1
  %23 = alloca i64, align 8
  %24 = alloca i64, align 8
  %25 = alloca i64, align 8
  %26 = alloca i32, align 4
  store ptr %0, ptr %11, align 8
  store ptr %1, ptr %12, align 8
  %27 = load ptr, ptr %12, align 8
  %28 = getelementptr inbounds %struct.Graph, ptr %27, i32 0, i32 0
  %29 = load i64, ptr %28, align 8
  store i64 %29, ptr %13, align 8
  %30 = load ptr, ptr %12, align 8
  %31 = getelementptr inbounds %struct.Graph, ptr %30, i32 0, i32 2
  %32 = load ptr, ptr %31, align 8
  store ptr %32, ptr %14, align 8
  %33 = load ptr, ptr %12, align 8
  %34 = getelementptr inbounds %struct.Graph, ptr %33, i32 0, i32 3
  %35 = load ptr, ptr %34, align 8
  store ptr %35, ptr %15, align 8
  store i32 1000000000, ptr %16, align 4
  store i1 false, ptr %17, align 1
  %36 = load i64, ptr %13, align 8
  %37 = load i64, ptr %13, align 8
  store ptr %19, ptr %10, align 8
  %38 = load ptr, ptr %10, align 8
  store ptr %38, ptr %4, align 8
  %39 = load ptr, ptr %4, align 8
  invoke void @_ZNSt6vectorIiSaIiEEC2EmRKiRKS0_(ptr noundef nonnull align 8 dereferenceable(24) %18, i64 noundef %37, ptr noundef nonnull align 4 dereferenceable(4) %16, ptr noundef nonnull align 1 dereferenceable(1) %19)
          to label %40 unwind label %58

40:                                               ; preds = %2
  store ptr %22, ptr %9, align 8
  %41 = load ptr, ptr %9, align 8
  store ptr %41, ptr %3, align 8
  %42 = load ptr, ptr %3, align 8
  invoke void @_ZNSt6vectorIS_IiSaIiEESaIS1_EEC2EmRKS1_RKS2_(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %36, ptr noundef nonnull align 8 dereferenceable(24) %18, ptr noundef nonnull align 1 dereferenceable(1) %22)
          to label %43 unwind label %62

43:                                               ; preds = %40
  store ptr %22, ptr %8, align 8
  %44 = load ptr, ptr %8, align 8
  call void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %44) #18
  call void @_ZNSt6vectorIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %18) #18
  store ptr %19, ptr %6, align 8
  %45 = load ptr, ptr %6, align 8
  call void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %45) #18
  store i64 0, ptr %23, align 8
  br label %46

46:                                               ; preds = %55, %43
  %47 = load i64, ptr %23, align 8
  %48 = load i64, ptr %13, align 8
  %49 = icmp slt i64 %47, %48
  br i1 %49, label %50, label %69

50:                                               ; preds = %46
  %51 = load i64, ptr %23, align 8
  %52 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %51) #18
  %53 = load i64, ptr %23, align 8
  %54 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %52, i64 noundef %53) #18
  store i32 0, ptr %54, align 4
  br label %55

55:                                               ; preds = %50
  %56 = load i64, ptr %23, align 8
  %57 = add nsw i64 %56, 1
  store i64 %57, ptr %23, align 8
  br label %46, !llvm.loop !5

58:                                               ; preds = %2
  %59 = landingpad { ptr, i32 }
          cleanup
  %60 = extractvalue { ptr, i32 } %59, 0
  store ptr %60, ptr %20, align 8
  %61 = extractvalue { ptr, i32 } %59, 1
  store i32 %61, ptr %21, align 4
  br label %67

62:                                               ; preds = %40
  %63 = landingpad { ptr, i32 }
          cleanup
  %64 = extractvalue { ptr, i32 } %63, 0
  store ptr %64, ptr %20, align 8
  %65 = extractvalue { ptr, i32 } %63, 1
  store i32 %65, ptr %21, align 4
  store ptr %22, ptr %7, align 8
  %66 = load ptr, ptr %7, align 8
  call void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %66) #18
  call void @_ZNSt6vectorIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %18) #18
  br label %67

67:                                               ; preds = %62, %58
  store ptr %19, ptr %5, align 8
  %68 = load ptr, ptr %5, align 8
  call void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %68) #18
  br label %113

69:                                               ; preds = %46
  store i64 0, ptr %24, align 8
  br label %70

70:                                               ; preds = %106, %69
  %71 = load i64, ptr %24, align 8
  %72 = load i64, ptr %13, align 8
  %73 = icmp slt i64 %71, %72
  br i1 %73, label %74, label %109

74:                                               ; preds = %70
  %75 = load ptr, ptr %14, align 8
  %76 = load i64, ptr %24, align 8
  %77 = getelementptr inbounds i64, ptr %75, i64 %76
  %78 = load i64, ptr %77, align 8
  store i64 %78, ptr %25, align 8
  br label %79

79:                                               ; preds = %102, %74
  %80 = load i64, ptr %25, align 8
  %81 = load ptr, ptr %14, align 8
  %82 = load i64, ptr %24, align 8
  %83 = add nsw i64 %82, 1
  %84 = getelementptr inbounds i64, ptr %81, i64 %83
  %85 = load i64, ptr %84, align 8
  %86 = icmp slt i64 %80, %85
  br i1 %86, label %87, label %105

87:                                               ; preds = %79
  %88 = load ptr, ptr %15, align 8
  %89 = load i64, ptr %25, align 8
  %90 = getelementptr inbounds i32, ptr %88, i64 %89
  %91 = load i32, ptr %90, align 4
  store i32 %91, ptr %26, align 4
  %92 = load i64, ptr %24, align 8
  %93 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %92) #18
  %94 = load i32, ptr %26, align 4
  %95 = sext i32 %94 to i64
  %96 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %93, i64 noundef %95) #18
  store i32 1, ptr %96, align 4
  %97 = load i32, ptr %26, align 4
  %98 = sext i32 %97 to i64
  %99 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %98) #18
  %100 = load i64, ptr %24, align 8
  %101 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %99, i64 noundef %100) #18
  store i32 1, ptr %101, align 4
  br label %102

102:                                              ; preds = %87
  %103 = load i64, ptr %25, align 8
  %104 = add nsw i64 %103, 1
  store i64 %104, ptr %25, align 8
  br label %79, !llvm.loop !7

105:                                              ; preds = %79
  br label %106

106:                                              ; preds = %105
  %107 = load i64, ptr %24, align 8
  %108 = add nsw i64 %107, 1
  store i64 %108, ptr %24, align 8
  br label %70, !llvm.loop !8

109:                                              ; preds = %70
  store i1 true, ptr %17, align 1
  %110 = load i1, ptr %17, align 1
  br i1 %110, label %112, label %111

111:                                              ; preds = %109
  call void @_ZNSt6vectorIS_IiSaIiEESaIS1_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) #18
  br label %112

112:                                              ; preds = %111, %109
  ret void

113:                                              ; preds = %67
  %114 = load ptr, ptr %20, align 8
  %115 = load i32, ptr %21, align 4
  %116 = insertvalue { ptr, i32 } poison, ptr %114, 0
  %117 = insertvalue { ptr, i32 } %116, i32 %115, 1
  resume { ptr, i32 } %117
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIiSaIiEEC2EmRKiRKS0_(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 4 dereferenceable(4) %2, ptr noundef nonnull align 1 dereferenceable(1) %3) unnamed_addr #2 comdat align 2 personality ptr @__gxx_personality_v0 {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  store ptr %2, ptr %7, align 8
  store ptr %3, ptr %8, align 8
  %11 = load ptr, ptr %5, align 8
  %12 = load i64, ptr %6, align 8
  %13 = load ptr, ptr %8, align 8
  %14 = call noundef i64 @_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_(i64 noundef %12, ptr noundef nonnull align 1 dereferenceable(1) %13)
  %15 = load ptr, ptr %8, align 8
  call void @_ZNSt12_Vector_baseIiSaIiEEC2EmRKS0_(ptr noundef nonnull align 8 dereferenceable(24) %11, i64 noundef %14, ptr noundef nonnull align 1 dereferenceable(1) %15)
  %16 = load i64, ptr %6, align 8
  %17 = load ptr, ptr %7, align 8
  invoke void @_ZNSt6vectorIiSaIiEE18_M_fill_initializeEmRKi(ptr noundef nonnull align 8 dereferenceable(24) %11, i64 noundef %16, ptr noundef nonnull align 4 dereferenceable(4) %17)
          to label %18 unwind label %19

18:                                               ; preds = %4
  ret void

19:                                               ; preds = %4
  %20 = landingpad { ptr, i32 }
          cleanup
  %21 = extractvalue { ptr, i32 } %20, 0
  store ptr %21, ptr %9, align 8
  %22 = extractvalue { ptr, i32 } %20, 1
  store i32 %22, ptr %10, align 4
  call void @_ZNSt12_Vector_baseIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %11) #18
  br label %23

23:                                               ; preds = %19
  %24 = load ptr, ptr %9, align 8
  %25 = load i32, ptr %10, align 4
  %26 = insertvalue { ptr, i32 } poison, ptr %24, 0
  %27 = insertvalue { ptr, i32 } %26, i32 %25, 1
  resume { ptr, i32 } %27
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIS_IiSaIiEESaIS1_EEC2EmRKS1_RKS2_(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef nonnull align 1 dereferenceable(1) %3) unnamed_addr #2 comdat align 2 personality ptr @__gxx_personality_v0 {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  store ptr %2, ptr %7, align 8
  store ptr %3, ptr %8, align 8
  %11 = load ptr, ptr %5, align 8
  %12 = load i64, ptr %6, align 8
  %13 = load ptr, ptr %8, align 8
  %14 = call noundef i64 @_ZNSt6vectorIS_IiSaIiEESaIS1_EE17_S_check_init_lenEmRKS2_(i64 noundef %12, ptr noundef nonnull align 1 dereferenceable(1) %13)
  %15 = load ptr, ptr %8, align 8
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EEC2EmRKS3_(ptr noundef nonnull align 8 dereferenceable(24) %11, i64 noundef %14, ptr noundef nonnull align 1 dereferenceable(1) %15)
  %16 = load i64, ptr %6, align 8
  %17 = load ptr, ptr %7, align 8
  invoke void @_ZNSt6vectorIS_IiSaIiEESaIS1_EE18_M_fill_initializeEmRKS1_(ptr noundef nonnull align 8 dereferenceable(24) %11, i64 noundef %16, ptr noundef nonnull align 8 dereferenceable(24) %17)
          to label %18 unwind label %19

18:                                               ; preds = %4
  ret void

19:                                               ; preds = %4
  %20 = landingpad { ptr, i32 }
          cleanup
  %21 = extractvalue { ptr, i32 } %20, 0
  store ptr %21, ptr %9, align 8
  %22 = extractvalue { ptr, i32 } %20, 1
  store i32 %22, ptr %10, align 4
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %11) #18
  br label %23

23:                                               ; preds = %19
  %24 = load ptr, ptr %9, align 8
  %25 = load i32, ptr %10, align 4
  %26 = insertvalue { ptr, i32 } poison, ptr %24, 0
  %27 = insertvalue { ptr, i32 } %26, i32 %25, 1
  resume { ptr, i32 } %27
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %6, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %6, i32 0, i32 0
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %10, i32 0, i32 1
  %12 = load ptr, ptr %11, align 8
  %13 = call noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %6) #18
  store ptr %9, ptr %2, align 8
  store ptr %12, ptr %3, align 8
  store ptr %13, ptr %4, align 8
  %14 = load ptr, ptr %2, align 8
  %15 = load ptr, ptr %3, align 8
  call void @_ZSt8_DestroyIPiEvT_S1_(ptr noundef %14, ptr noundef %15)
  br label %16

16:                                               ; preds = %1
  call void @_ZNSt12_Vector_baseIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %6) #18
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  br label %6

6:                                                ; preds = %2
  %7 = load i64, ptr %4, align 8
  %8 = call noundef i64 @_ZNKSt6vectorIS_IiSaIiEESaIS1_EE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %5) #18
  %9 = icmp ult i64 %7, %8
  %10 = xor i1 %9, true
  br i1 %10, label %11, label %12

11:                                               ; preds = %6
  call void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef @.str.9, i32 noundef 1263, ptr noundef @__PRETTY_FUNCTION__._ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm, ptr noundef @.str.10) #20
  unreachable

12:                                               ; preds = %6
  br label %13

13:                                               ; preds = %12
  br label %14

14:                                               ; preds = %13
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %4, align 8
  %19 = getelementptr inbounds %"class.std::vector", ptr %17, i64 %18
  ret ptr %19
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  br label %6

6:                                                ; preds = %2
  %7 = load i64, ptr %4, align 8
  %8 = call noundef i64 @_ZNKSt6vectorIiSaIiEE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %5) #18
  %9 = icmp ult i64 %7, %8
  %10 = xor i1 %9, true
  br i1 %10, label %11, label %12

11:                                               ; preds = %6
  call void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef @.str.9, i32 noundef 1263, ptr noundef @__PRETTY_FUNCTION__._ZNSt6vectorIiSaIiEEixEm, ptr noundef @.str.10) #20
  unreachable

12:                                               ; preds = %6
  br label %13

13:                                               ; preds = %12
  br label %14

14:                                               ; preds = %13
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %4, align 8
  %19 = getelementptr inbounds i32, ptr %17, i64 %18
  ret ptr %19
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIS_IiSaIiEESaIS1_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %6, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %6, i32 0, i32 0
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %10, i32 0, i32 1
  %12 = load ptr, ptr %11, align 8
  %13 = call noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %6) #18
  store ptr %9, ptr %2, align 8
  store ptr %12, ptr %3, align 8
  store ptr %13, ptr %4, align 8
  %14 = load ptr, ptr %2, align 8
  %15 = load ptr, ptr %3, align 8
  invoke void @_ZSt8_DestroyIPSt6vectorIiSaIiEEEvT_S4_(ptr noundef %14, ptr noundef %15)
          to label %16 unwind label %18

16:                                               ; preds = %1
  br label %17

17:                                               ; preds = %16
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %6) #18
  ret void

18:                                               ; preds = %1
  %19 = landingpad { ptr, i32 }
          catch ptr null
  %20 = extractvalue { ptr, i32 } %19, 0
  call void @__clang_call_terminate(ptr %20) #21
  unreachable
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  ret ptr %4
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZSt8_DestroyIPSt6vectorIiSaIiEEEvT_S4_(ptr noundef %0, ptr noundef %1) local_unnamed_addr #2 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  br label %5

5:                                                ; preds = %11, %2
  %6 = load ptr, ptr %3, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = icmp ne ptr %6, %7
  br i1 %8, label %9, label %14

9:                                                ; preds = %5
  %10 = load ptr, ptr %3, align 8
  call void @_ZSt8_DestroyISt6vectorIiSaIiEEEvPT_(ptr noundef %10)
  br label %11

11:                                               ; preds = %9
  %12 = load ptr, ptr %3, align 8
  %13 = getelementptr inbounds %"class.std::vector", ptr %12, i32 1
  store ptr %13, ptr %3, align 8
  br label %5, !llvm.loop !9

14:                                               ; preds = %5
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %5 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i32 0, i32 2
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = ptrtoint ptr %9 to i64
  %14 = ptrtoint ptr %12 to i64
  %15 = sub i64 %13, %14
  %16 = sdiv exact i64 %15, 24
  invoke void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE13_M_deallocateEPS2_m(ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef %6, i64 noundef %16)
          to label %17 unwind label %19

17:                                               ; preds = %1
  %18 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implD2Ev(ptr noundef nonnull align 8 dereferenceable(24) %18) #18
  ret void

19:                                               ; preds = %1
  %20 = landingpad { ptr, i32 }
          catch ptr null
  %21 = extractvalue { ptr, i32 } %20, 0
  call void @__clang_call_terminate(ptr %21) #21
  unreachable
}

; Function Attrs: noinline noreturn nounwind uwtable
define linkonce_odr hidden void @__clang_call_terminate(ptr noundef %0) local_unnamed_addr #4 comdat {
  %2 = tail call ptr @__cxa_begin_catch(ptr %0) #18
  tail call void @_ZSt9terminatev() #21
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr

declare dso_local void @_ZSt9terminatev() local_unnamed_addr

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE13_M_deallocateEPS2_m(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr noundef %1, i64 noundef %2) local_unnamed_addr #2 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  store ptr %0, ptr %7, align 8
  store ptr %1, ptr %8, align 8
  store i64 %2, ptr %9, align 8
  %10 = load ptr, ptr %7, align 8
  %11 = load ptr, ptr %8, align 8
  %12 = icmp ne ptr %11, null
  br i1 %12, label %13, label %20

13:                                               ; preds = %3
  %14 = getelementptr inbounds %"struct.std::_Vector_base", ptr %10, i32 0, i32 0
  %15 = load ptr, ptr %8, align 8
  %16 = load i64, ptr %9, align 8
  store ptr %14, ptr %4, align 8
  store ptr %15, ptr %5, align 8
  store i64 %16, ptr %6, align 8
  %17 = load ptr, ptr %4, align 8
  %18 = load ptr, ptr %5, align 8
  %19 = load i64, ptr %6, align 8
  call void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEE10deallocateEPS2_m(ptr noundef nonnull align 1 dereferenceable(1) %17, ptr noundef %18, i64 noundef %19)
  br label %20

20:                                               ; preds = %13, %3
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implD2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  %4 = load ptr, ptr %3, align 8
  store ptr %4, ptr %2, align 8
  %5 = load ptr, ptr %2, align 8
  call void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %5) #18
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEE10deallocateEPS2_m(ptr noundef nonnull align 1 dereferenceable(1) %0, ptr noundef %1, i64 noundef %2) local_unnamed_addr #3 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = load ptr, ptr %5, align 8
  call void @_ZdlPv(ptr noundef %8) #22
  ret void
}

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #5

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZSt8_DestroyISt6vectorIiSaIiEEEvPT_(ptr noundef %0) local_unnamed_addr #3 comdat {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  call void @_ZNSt6vectorIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %3) #18
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNKSt6vectorIiSaIiEE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  %4 = load ptr, ptr %2, align 8
  %5 = getelementptr inbounds %"struct.std::_Vector_base", ptr %4, i32 0, i32 0
  %6 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %5, i32 0, i32 1
  %7 = load ptr, ptr %6, align 8
  %8 = getelementptr inbounds %"struct.std::_Vector_base", ptr %4, i32 0, i32 0
  %9 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i32 0, i32 0
  %10 = load ptr, ptr %9, align 8
  %11 = ptrtoint ptr %7 to i64
  %12 = ptrtoint ptr %10 to i64
  %13 = sub i64 %11, %12
  %14 = sdiv exact i64 %13, 4
  store i64 %14, ptr %3, align 8
  %15 = load i64, ptr %3, align 8
  %16 = icmp slt i64 %15, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %1
  unreachable

18:                                               ; preds = %1
  %19 = load i64, ptr %3, align 8
  ret i64 %19
}

; Function Attrs: cold noreturn nounwind
declare dso_local void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef, i32 noundef, ptr noundef, ptr noundef) local_unnamed_addr #6

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNKSt6vectorIS_IiSaIiEESaIS1_EE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  %4 = load ptr, ptr %2, align 8
  %5 = getelementptr inbounds %"struct.std::_Vector_base", ptr %4, i32 0, i32 0
  %6 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %5, i32 0, i32 1
  %7 = load ptr, ptr %6, align 8
  %8 = getelementptr inbounds %"struct.std::_Vector_base", ptr %4, i32 0, i32 0
  %9 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i32 0, i32 0
  %10 = load ptr, ptr %9, align 8
  %11 = ptrtoint ptr %7 to i64
  %12 = ptrtoint ptr %10 to i64
  %13 = sub i64 %11, %12
  %14 = sdiv exact i64 %13, 24
  store i64 %14, ptr %3, align 8
  %15 = load i64, ptr %3, align 8
  %16 = icmp slt i64 %15, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %1
  unreachable

18:                                               ; preds = %1
  %19 = load i64, ptr %3, align 8
  ret i64 %19
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  ret ptr %4
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZSt8_DestroyIPiEvT_S1_(ptr noundef %0, ptr noundef %1) local_unnamed_addr #3 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %5 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i32 0, i32 0
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i32 0, i32 2
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = ptrtoint ptr %9 to i64
  %14 = ptrtoint ptr %12 to i64
  %15 = sub i64 %13, %14
  %16 = sdiv exact i64 %15, 4
  invoke void @_ZNSt12_Vector_baseIiSaIiEE13_M_deallocateEPim(ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef %6, i64 noundef %16)
          to label %17 unwind label %19

17:                                               ; preds = %1
  %18 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  call void @_ZNSt12_Vector_baseIiSaIiEE12_Vector_implD2Ev(ptr noundef nonnull align 8 dereferenceable(24) %18) #18
  ret void

19:                                               ; preds = %1
  %20 = landingpad { ptr, i32 }
          catch ptr null
  %21 = extractvalue { ptr, i32 } %20, 0
  call void @__clang_call_terminate(ptr %21) #21
  unreachable
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEE13_M_deallocateEPim(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr noundef %1, i64 noundef %2) local_unnamed_addr #2 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  store ptr %0, ptr %7, align 8
  store ptr %1, ptr %8, align 8
  store i64 %2, ptr %9, align 8
  %10 = load ptr, ptr %7, align 8
  %11 = load ptr, ptr %8, align 8
  %12 = icmp ne ptr %11, null
  br i1 %12, label %13, label %20

13:                                               ; preds = %3
  %14 = getelementptr inbounds %"struct.std::_Vector_base", ptr %10, i32 0, i32 0
  %15 = load ptr, ptr %8, align 8
  %16 = load i64, ptr %9, align 8
  store ptr %14, ptr %4, align 8
  store ptr %15, ptr %5, align 8
  store i64 %16, ptr %6, align 8
  %17 = load ptr, ptr %4, align 8
  %18 = load ptr, ptr %5, align 8
  %19 = load i64, ptr %6, align 8
  call void @_ZNSt15__new_allocatorIiE10deallocateEPim(ptr noundef nonnull align 1 dereferenceable(1) %17, ptr noundef %18, i64 noundef %19)
  br label %20

20:                                               ; preds = %13, %3
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEE12_Vector_implD2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  %4 = load ptr, ptr %3, align 8
  store ptr %4, ptr %2, align 8
  %5 = load ptr, ptr %2, align 8
  call void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %5) #18
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt15__new_allocatorIiE10deallocateEPim(ptr noundef nonnull align 1 dereferenceable(1) %0, ptr noundef %1, i64 noundef %2) local_unnamed_addr #3 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = load ptr, ptr %5, align 8
  call void @_ZdlPv(ptr noundef %8) #22
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNSt6vectorIS_IiSaIiEESaIS1_EE17_S_check_init_lenEmRKS2_(i64 noundef %0, ptr noundef nonnull align 1 dereferenceable(1) %1) local_unnamed_addr #2 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i64, align 8
  %9 = alloca ptr, align 8
  %10 = alloca %"class.std::allocator.2", align 1
  store i64 %0, ptr %8, align 8
  store ptr %1, ptr %9, align 8
  %11 = load i64, ptr %8, align 8
  %12 = load ptr, ptr %9, align 8
  store ptr %10, ptr %5, align 8
  store ptr %12, ptr %6, align 8
  %13 = load ptr, ptr %5, align 8
  %14 = load ptr, ptr %6, align 8
  store ptr %13, ptr %3, align 8
  store ptr %14, ptr %4, align 8
  %15 = load ptr, ptr %3, align 8
  %16 = call noundef i64 @_ZNSt6vectorIS_IiSaIiEESaIS1_EE11_S_max_sizeERKS2_(ptr noundef nonnull align 1 dereferenceable(1) %10) #18
  %17 = icmp ugt i64 %11, %16
  store ptr %10, ptr %7, align 8
  %18 = load ptr, ptr %7, align 8
  call void @_ZNSt15__new_allocatorISt6vectorIiSaIiEEED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %18) #18
  br i1 %17, label %19, label %20

19:                                               ; preds = %2
  call void @_ZSt20__throw_length_errorPKc(ptr noundef @.str.2.19) #23
  unreachable

20:                                               ; preds = %2
  %21 = load i64, ptr %8, align 8
  ret i64 %21
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EEC2EmRKS3_(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 1 dereferenceable(1) %2) unnamed_addr #2 comdat align 2 personality ptr @__gxx_personality_v0 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %9 = load ptr, ptr %4, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %9, i32 0, i32 0
  %11 = load ptr, ptr %6, align 8
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(24) %10, ptr noundef nonnull align 1 dereferenceable(1) %11) #18
  %12 = load i64, ptr %5, align 8
  invoke void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE17_M_create_storageEm(ptr noundef nonnull align 8 dereferenceable(24) %9, i64 noundef %12)
          to label %13 unwind label %14

13:                                               ; preds = %3
  ret void

14:                                               ; preds = %3
  %15 = landingpad { ptr, i32 }
          cleanup
  %16 = extractvalue { ptr, i32 } %15, 0
  store ptr %16, ptr %7, align 8
  %17 = extractvalue { ptr, i32 } %15, 1
  store i32 %17, ptr %8, align 4
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implD2Ev(ptr noundef nonnull align 8 dereferenceable(24) %10) #18
  br label %18

18:                                               ; preds = %14
  %19 = load ptr, ptr %7, align 8
  %20 = load i32, ptr %8, align 4
  %21 = insertvalue { ptr, i32 } poison, ptr %19, 0
  %22 = insertvalue { ptr, i32 } %21, i32 %20, 1
  resume { ptr, i32 } %22
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIS_IiSaIiEESaIS1_EE18_M_fill_initializeEmRKS1_(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 8 dereferenceable(24) %2) local_unnamed_addr #2 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = getelementptr inbounds %"struct.std::_Vector_base", ptr %7, i32 0, i32 0
  %9 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i32 0, i32 0
  %10 = load ptr, ptr %9, align 8
  %11 = load i64, ptr %5, align 8
  %12 = load ptr, ptr %6, align 8
  %13 = call noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %7) #18
  %14 = call noundef ptr @_ZSt24__uninitialized_fill_n_aIPSt6vectorIiSaIiEEmS2_S2_ET_S4_T0_RKT1_RSaIT2_E(ptr noundef %10, i64 noundef %11, ptr noundef nonnull align 8 dereferenceable(24) %12, ptr noundef nonnull align 1 dereferenceable(1) %13)
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %7, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 1
  store ptr %14, ptr %16, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt24__uninitialized_fill_n_aIPSt6vectorIiSaIiEEmS2_S2_ET_S4_T0_RKT1_RSaIT2_E(ptr noundef %0, i64 noundef %1, ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef nonnull align 1 dereferenceable(1) %3) local_unnamed_addr #2 comdat {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  store ptr %2, ptr %7, align 8
  store ptr %3, ptr %8, align 8
  %9 = load ptr, ptr %5, align 8
  %10 = load i64, ptr %6, align 8
  %11 = load ptr, ptr %7, align 8
  %12 = call noundef ptr @_ZSt20uninitialized_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_(ptr noundef %9, i64 noundef %10, ptr noundef nonnull align 8 dereferenceable(24) %11)
  ret ptr %12
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt20uninitialized_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_(ptr noundef %0, i64 noundef %1, ptr noundef nonnull align 8 dereferenceable(24) %2) local_unnamed_addr #2 comdat {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = load i64, ptr %5, align 8
  %9 = load ptr, ptr %6, align 8
  %10 = call noundef ptr @_ZSt18__do_uninit_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_(ptr noundef %7, i64 noundef %8, ptr noundef nonnull align 8 dereferenceable(24) %9)
  ret ptr %10
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt18__do_uninit_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_(ptr noundef %0, i64 noundef %1, ptr noundef nonnull align 8 dereferenceable(24) %2) local_unnamed_addr #2 comdat personality ptr @__gxx_personality_v0 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca %"struct.std::_UninitDestroyGuard.5", align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  call void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvEC2ERS3_(ptr noundef nonnull align 8 dereferenceable(16) %7, ptr noundef nonnull align 8 dereferenceable(8) %4)
  br label %10

10:                                               ; preds = %3
  %11 = load i64, ptr %5, align 8
  %12 = icmp uge i64 %11, 0
  %13 = xor i1 %12, true
  br i1 %13, label %14, label %15

14:                                               ; preds = %10
  call void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef @.str.7, i32 noundef 463, ptr noundef @__PRETTY_FUNCTION__._ZSt18__do_uninit_fill_nIPSt6vectorIiSaIiEEmS2_ET_S4_T0_RKT1_, ptr noundef @.str.8) #20
  unreachable

15:                                               ; preds = %10
  br label %16

16:                                               ; preds = %15
  br label %17

17:                                               ; preds = %16
  br label %18

18:                                               ; preds = %26, %17
  %19 = load i64, ptr %5, align 8
  %20 = add i64 %19, -1
  store i64 %20, ptr %5, align 8
  %21 = icmp ne i64 %19, 0
  br i1 %21, label %22, label %33

22:                                               ; preds = %18
  %23 = load ptr, ptr %4, align 8
  %24 = load ptr, ptr %6, align 8
  invoke void @_ZSt10_ConstructISt6vectorIiSaIiEEJRKS2_EEvPT_DpOT0_(ptr noundef %23, ptr noundef nonnull align 8 dereferenceable(24) %24)
          to label %25 unwind label %29

25:                                               ; preds = %22
  br label %26

26:                                               ; preds = %25
  %27 = load ptr, ptr %4, align 8
  %28 = getelementptr inbounds %"class.std::vector", ptr %27, i32 1
  store ptr %28, ptr %4, align 8
  br label %18, !llvm.loop !10

29:                                               ; preds = %22
  %30 = landingpad { ptr, i32 }
          cleanup
  %31 = extractvalue { ptr, i32 } %30, 0
  store ptr %31, ptr %8, align 8
  %32 = extractvalue { ptr, i32 } %30, 1
  store i32 %32, ptr %9, align 4
  call void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvED2Ev(ptr noundef nonnull align 8 dereferenceable(16) %7) #18
  br label %36

33:                                               ; preds = %18
  call void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvE7releaseEv(ptr noundef nonnull align 8 dereferenceable(16) %7)
  br label %34

34:                                               ; preds = %33
  %35 = load ptr, ptr %4, align 8
  call void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvED2Ev(ptr noundef nonnull align 8 dereferenceable(16) %7) #18
  ret ptr %35

36:                                               ; preds = %29
  %37 = load ptr, ptr %8, align 8
  %38 = load i32, ptr %9, align 4
  %39 = insertvalue { ptr, i32 } poison, ptr %37, 0
  %40 = insertvalue { ptr, i32 } %39, i32 %38, 1
  resume { ptr, i32 } %40
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvEC2ERS3_(ptr noundef nonnull align 8 dereferenceable(16) %0, ptr noundef nonnull align 8 dereferenceable(8) %1) unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %5, i32 0, i32 0
  %7 = load ptr, ptr %4, align 8
  %8 = load ptr, ptr %7, align 8
  store ptr %8, ptr %6, align 8
  %9 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %5, i32 0, i32 1
  %10 = load ptr, ptr %4, align 8
  store ptr %10, ptr %9, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZSt10_ConstructISt6vectorIiSaIiEEJRKS2_EEvPT_DpOT0_(ptr noundef %0, ptr noundef nonnull align 8 dereferenceable(24) %1) local_unnamed_addr #2 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load ptr, ptr %4, align 8
  call void @_ZNSt6vectorIiSaIiEEC2ERKS1_(ptr noundef nonnull align 8 dereferenceable(24) %5, ptr noundef nonnull align 8 dereferenceable(24) %6)
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvED2Ev(ptr noundef nonnull align 8 dereferenceable(16) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 1
  %5 = load ptr, ptr %4, align 8
  %6 = icmp ne ptr %5, null
  br i1 %6, label %7, label %14

7:                                                ; preds = %1
  %8 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 1
  %11 = load ptr, ptr %10, align 8
  %12 = load ptr, ptr %11, align 8
  invoke void @_ZSt8_DestroyIPSt6vectorIiSaIiEEEvT_S4_(ptr noundef %9, ptr noundef %12)
          to label %13 unwind label %15

13:                                               ; preds = %7
  br label %14

14:                                               ; preds = %13, %1
  ret void

15:                                               ; preds = %7
  %16 = landingpad { ptr, i32 }
          catch ptr null
  %17 = extractvalue { ptr, i32 } %16, 0
  call void @__clang_call_terminate(ptr %17) #21
  unreachable
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt19_UninitDestroyGuardIPSt6vectorIiSaIiEEvE7releaseEv(ptr noundef nonnull align 8 dereferenceable(16) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 1
  store ptr null, ptr %4, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIiSaIiEEC2ERKS1_(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr noundef nonnull align 8 dereferenceable(24) %1) unnamed_addr #2 comdat align 2 personality ptr @__gxx_personality_v0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca %"class.std::allocator.2", align 1
  %16 = alloca ptr, align 8
  %17 = alloca i32, align 4
  %18 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %19 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  store ptr %0, ptr %13, align 8
  store ptr %1, ptr %14, align 8
  %20 = load ptr, ptr %13, align 8
  %21 = load ptr, ptr %14, align 8
  %22 = call noundef i64 @_ZNKSt6vectorIiSaIiEE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %21) #18
  %23 = load ptr, ptr %14, align 8
  %24 = call noundef nonnull align 1 dereferenceable(1) ptr @_ZNKSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %23) #18
  call void @llvm.experimental.noalias.scope.decl(metadata !11)
  store ptr %15, ptr %9, align 8, !noalias !11
  store ptr %24, ptr %10, align 8, !noalias !11
  %25 = load ptr, ptr %10, align 8, !noalias !11
  call void @llvm.experimental.noalias.scope.decl(metadata !14)
  store ptr %15, ptr %7, align 8, !noalias !14
  store ptr %25, ptr %8, align 8, !noalias !14
  %26 = load ptr, ptr %8, align 8, !noalias !14
  store ptr %15, ptr %5, align 8, !noalias !14
  store ptr %26, ptr %6, align 8, !noalias !14
  %27 = load ptr, ptr %5, align 8, !noalias !14
  %28 = load ptr, ptr %6, align 8, !noalias !14
  store ptr %27, ptr %3, align 8, !noalias !14
  store ptr %28, ptr %4, align 8, !noalias !14
  %29 = load ptr, ptr %3, align 8, !noalias !14
  invoke void @_ZNSt12_Vector_baseIiSaIiEEC2EmRKS0_(ptr noundef nonnull align 8 dereferenceable(24) %20, i64 noundef %22, ptr noundef nonnull align 1 dereferenceable(1) %15)
          to label %30 unwind label %50

30:                                               ; preds = %2
  store ptr %15, ptr %12, align 8
  %31 = load ptr, ptr %12, align 8
  call void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %31) #18
  %32 = load ptr, ptr %14, align 8
  %33 = call ptr @_ZNKSt6vectorIiSaIiEE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %32) #18
  %34 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %18, i32 0, i32 0
  store ptr %33, ptr %34, align 8
  %35 = load ptr, ptr %14, align 8
  %36 = call ptr @_ZNKSt6vectorIiSaIiEE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %35) #18
  %37 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %19, i32 0, i32 0
  store ptr %36, ptr %37, align 8
  %38 = getelementptr inbounds %"struct.std::_Vector_base", ptr %20, i32 0, i32 0
  %39 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %38, i32 0, i32 0
  %40 = load ptr, ptr %39, align 8
  %41 = call noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %20) #18
  %42 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %18, i32 0, i32 0
  %43 = load ptr, ptr %42, align 8
  %44 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %19, i32 0, i32 0
  %45 = load ptr, ptr %44, align 8
  %46 = invoke noundef ptr @_ZSt22__uninitialized_copy_aIN9__gnu_cxx17__normal_iteratorIPKiSt6vectorIiSaIiEEEES7_PiiET1_T_T0_S9_RSaIT2_E(ptr %43, ptr %45, ptr noundef %40, ptr noundef nonnull align 1 dereferenceable(1) %41)
          to label %47 unwind label %55

47:                                               ; preds = %30
  %48 = getelementptr inbounds %"struct.std::_Vector_base", ptr %20, i32 0, i32 0
  %49 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %48, i32 0, i32 1
  store ptr %46, ptr %49, align 8
  ret void

50:                                               ; preds = %2
  %51 = landingpad { ptr, i32 }
          cleanup
  %52 = extractvalue { ptr, i32 } %51, 0
  store ptr %52, ptr %16, align 8
  %53 = extractvalue { ptr, i32 } %51, 1
  store i32 %53, ptr %17, align 4
  store ptr %15, ptr %11, align 8
  %54 = load ptr, ptr %11, align 8
  call void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %54) #18
  br label %59

55:                                               ; preds = %30
  %56 = landingpad { ptr, i32 }
          cleanup
  %57 = extractvalue { ptr, i32 } %56, 0
  store ptr %57, ptr %16, align 8
  %58 = extractvalue { ptr, i32 } %56, 1
  store i32 %58, ptr %17, align 4
  call void @_ZNSt12_Vector_baseIiSaIiEED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %20) #18
  br label %59

59:                                               ; preds = %55, %50
  %60 = load ptr, ptr %16, align 8
  %61 = load i32, ptr %17, align 4
  %62 = insertvalue { ptr, i32 } poison, ptr %60, 0
  %63 = insertvalue { ptr, i32 } %62, i32 %61, 1
  resume { ptr, i32 } %63
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 1 dereferenceable(1) ptr @_ZNKSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  ret ptr %4
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #7

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEEC2EmRKS0_(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 1 dereferenceable(1) %2) unnamed_addr #2 comdat align 2 personality ptr @__gxx_personality_v0 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %9 = load ptr, ptr %4, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %9, i32 0, i32 0
  %11 = load ptr, ptr %6, align 8
  call void @_ZNSt12_Vector_baseIiSaIiEE12_Vector_implC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(24) %10, ptr noundef nonnull align 1 dereferenceable(1) %11) #18
  %12 = load i64, ptr %5, align 8
  invoke void @_ZNSt12_Vector_baseIiSaIiEE17_M_create_storageEm(ptr noundef nonnull align 8 dereferenceable(24) %9, i64 noundef %12)
          to label %13 unwind label %14

13:                                               ; preds = %3
  ret void

14:                                               ; preds = %3
  %15 = landingpad { ptr, i32 }
          cleanup
  %16 = extractvalue { ptr, i32 } %15, 0
  store ptr %16, ptr %7, align 8
  %17 = extractvalue { ptr, i32 } %15, 1
  store i32 %17, ptr %8, align 4
  call void @_ZNSt12_Vector_baseIiSaIiEE12_Vector_implD2Ev(ptr noundef nonnull align 8 dereferenceable(24) %10) #18
  br label %18

18:                                               ; preds = %14
  %19 = load ptr, ptr %7, align 8
  %20 = load i32, ptr %8, align 4
  %21 = insertvalue { ptr, i32 } poison, ptr %19, 0
  %22 = insertvalue { ptr, i32 } %21, i32 %20, 1
  resume { ptr, i32 } %22
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local ptr @_ZNKSt6vectorIiSaIiEE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %6, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i32 0, i32 0
  store ptr %4, ptr %2, align 8
  store ptr %8, ptr %3, align 8
  %9 = load ptr, ptr %2, align 8
  %10 = load ptr, ptr %3, align 8
  %11 = load ptr, ptr %10, align 8
  store ptr %11, ptr %9, align 8
  %12 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %4, i32 0, i32 0
  %13 = load ptr, ptr %12, align 8
  ret ptr %13
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local ptr @_ZNKSt6vectorIiSaIiEE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %6, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i32 0, i32 1
  store ptr %4, ptr %2, align 8
  store ptr %8, ptr %3, align 8
  %9 = load ptr, ptr %2, align 8
  %10 = load ptr, ptr %3, align 8
  %11 = load ptr, ptr %10, align 8
  store ptr %11, ptr %9, align 8
  %12 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %4, i32 0, i32 0
  %13 = load ptr, ptr %12, align 8
  ret ptr %13
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt22__uninitialized_copy_aIN9__gnu_cxx17__normal_iteratorIPKiSt6vectorIiSaIiEEEES7_PiiET1_T_T0_S9_RSaIT2_E(ptr %0, ptr %1, ptr noundef %2, ptr noundef nonnull align 1 dereferenceable(1) %3) local_unnamed_addr #2 comdat {
  %5 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %6 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %10 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %11 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %5, i32 0, i32 0
  store ptr %0, ptr %11, align 8
  %12 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %6, i32 0, i32 0
  store ptr %1, ptr %12, align 8
  store ptr %2, ptr %7, align 8
  store ptr %3, ptr %8, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %9, ptr align 8 %5, i64 8, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %10, ptr align 8 %6, i64 8, i1 false)
  %13 = load ptr, ptr %7, align 8
  %14 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %9, i32 0, i32 0
  %15 = load ptr, ptr %14, align 8
  %16 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %10, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = call noundef ptr @_ZSt18uninitialized_copyIN9__gnu_cxx17__normal_iteratorIPKiSt6vectorIiSaIiEEEEPiET0_T_SA_S9_(ptr %15, ptr %17, ptr noundef %13)
  ret ptr %18
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt18uninitialized_copyIN9__gnu_cxx17__normal_iteratorIPKiSt6vectorIiSaIiEEEEPiET0_T_SA_S9_(ptr %0, ptr %1, ptr noundef %2) local_unnamed_addr #3 comdat {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %12 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %13 = alloca ptr, align 8
  %14 = alloca i64, align 8
  %15 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %16 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %11, i32 0, i32 0
  store ptr %0, ptr %16, align 8
  %17 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %12, i32 0, i32 0
  store ptr %1, ptr %17, align 8
  store ptr %2, ptr %13, align 8
  store ptr %12, ptr %9, align 8
  store ptr %11, ptr %10, align 8
  %18 = load ptr, ptr %9, align 8
  store ptr %18, ptr %4, align 8
  %19 = load ptr, ptr %4, align 8
  %20 = load ptr, ptr %19, align 8
  %21 = load ptr, ptr %10, align 8
  store ptr %21, ptr %5, align 8
  %22 = load ptr, ptr %5, align 8
  %23 = load ptr, ptr %22, align 8
  %24 = ptrtoint ptr %20 to i64
  %25 = ptrtoint ptr %23 to i64
  %26 = sub i64 %24, %25
  %27 = sdiv exact i64 %26, 4
  store i64 %27, ptr %14, align 8
  %28 = load i64, ptr %14, align 8
  %29 = icmp sgt i64 %28, 0
  br i1 %29, label %30, label %42

30:                                               ; preds = %3
  %31 = load ptr, ptr %13, align 8
  store ptr %31, ptr %8, align 8
  %32 = load ptr, ptr %8, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %15, ptr align 8 %11, i64 8, i1 false)
  %33 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %15, i32 0, i32 0
  %34 = load ptr, ptr %33, align 8
  store ptr %34, ptr %7, align 8
  store ptr %7, ptr %6, align 8
  %35 = load ptr, ptr %6, align 8
  %36 = load ptr, ptr %35, align 8
  %37 = load i64, ptr %14, align 8
  %38 = mul i64 %37, 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %32, ptr align 4 %36, i64 %38, i1 false)
  %39 = load i64, ptr %14, align 8
  %40 = load ptr, ptr %13, align 8
  %41 = getelementptr inbounds i32, ptr %40, i64 %39
  store ptr %41, ptr %13, align 8
  br label %42

42:                                               ; preds = %30, %3
  %43 = load ptr, ptr %13, align 8
  ret ptr %43
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEE12_Vector_implC2ERKS0_(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr noundef nonnull align 1 dereferenceable(1) %1) unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  store ptr %1, ptr %8, align 8
  %9 = load ptr, ptr %7, align 8
  %10 = load ptr, ptr %8, align 8
  store ptr %9, ptr %5, align 8
  store ptr %10, ptr %6, align 8
  %11 = load ptr, ptr %5, align 8
  %12 = load ptr, ptr %6, align 8
  store ptr %11, ptr %3, align 8
  store ptr %12, ptr %4, align 8
  %13 = load ptr, ptr %3, align 8
  call void @_ZNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataC2Ev(ptr noundef nonnull align 8 dereferenceable(24) %9) #18
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEE17_M_create_storageEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #2 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load i64, ptr %4, align 8
  %7 = call noundef ptr @_ZNSt12_Vector_baseIiSaIiEE11_M_allocateEm(ptr noundef nonnull align 8 dereferenceable(24) %5, i64 noundef %6)
  %8 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %9 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i32 0, i32 0
  store ptr %7, ptr %9, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %14 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %13, i32 0, i32 1
  store ptr %12, ptr %14, align 8
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %4, align 8
  %19 = getelementptr inbounds i32, ptr %17, i64 %18
  %20 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %21 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %20, i32 0, i32 2
  store ptr %19, ptr %21, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZNSt12_Vector_baseIiSaIiEE11_M_allocateEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #2 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  %7 = load ptr, ptr %5, align 8
  %8 = load i64, ptr %6, align 8
  %9 = icmp ne i64 %8, 0
  br i1 %9, label %10, label %16

10:                                               ; preds = %2
  %11 = getelementptr inbounds %"struct.std::_Vector_base", ptr %7, i32 0, i32 0
  %12 = load i64, ptr %6, align 8
  store ptr %11, ptr %3, align 8
  store i64 %12, ptr %4, align 8
  %13 = load ptr, ptr %3, align 8
  %14 = load i64, ptr %4, align 8
  %15 = call noundef ptr @_ZNSt15__new_allocatorIiE8allocateEmPKv(ptr noundef nonnull align 1 dereferenceable(1) %13, i64 noundef %14, ptr noundef null)
  br label %17

16:                                               ; preds = %2
  br label %17

17:                                               ; preds = %16, %10
  %18 = phi ptr [ %15, %10 ], [ null, %16 ]
  ret ptr %18
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZNSt15__new_allocatorIiE8allocateEmPKv(ptr noundef nonnull align 1 dereferenceable(1) %0, i64 noundef %1, ptr noundef %2) local_unnamed_addr #2 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  store ptr %2, ptr %7, align 8
  %8 = load ptr, ptr %5, align 8
  %9 = load i64, ptr %6, align 8
  store ptr %8, ptr %4, align 8
  %10 = load ptr, ptr %4, align 8
  %11 = icmp ugt i64 %9, 2305843009213693951
  br i1 %11, label %12, label %17

12:                                               ; preds = %3
  %13 = load i64, ptr %6, align 8
  %14 = icmp ugt i64 %13, 4611686018427387903
  br i1 %14, label %15, label %16

15:                                               ; preds = %12
  call void @_ZSt28__throw_bad_array_new_lengthv() #24
  unreachable

16:                                               ; preds = %12
  call void @_ZSt17__throw_bad_allocv() #24
  unreachable

17:                                               ; preds = %3
  %18 = load i64, ptr %6, align 8
  %19 = mul i64 %18, 4
  %20 = call noalias noundef nonnull ptr @_Znwm(i64 noundef %19) #25
  ret ptr %20
}

; Function Attrs: noreturn
declare dso_local void @_ZSt28__throw_bad_array_new_lengthv() local_unnamed_addr #8

; Function Attrs: noreturn
declare dso_local void @_ZSt17__throw_bad_allocv() local_unnamed_addr #8

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #9

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataC2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i32 0, i32 0
  store ptr null, ptr %4, align 8
  %5 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i32 0, i32 1
  store ptr null, ptr %5, align 8
  %6 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i32 0, i32 2
  store ptr null, ptr %6, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE12_Vector_implC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr noundef nonnull align 1 dereferenceable(1) %1) unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  store ptr %1, ptr %8, align 8
  %9 = load ptr, ptr %7, align 8
  %10 = load ptr, ptr %8, align 8
  store ptr %9, ptr %5, align 8
  store ptr %10, ptr %6, align 8
  %11 = load ptr, ptr %5, align 8
  %12 = load ptr, ptr %6, align 8
  store ptr %11, ptr %3, align 8
  store ptr %12, ptr %4, align 8
  %13 = load ptr, ptr %3, align 8
  call void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE17_Vector_impl_dataC2Ev(ptr noundef nonnull align 8 dereferenceable(24) %9) #18
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE17_M_create_storageEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #2 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load i64, ptr %4, align 8
  %7 = call noundef ptr @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE11_M_allocateEm(ptr noundef nonnull align 8 dereferenceable(24) %5, i64 noundef %6)
  %8 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %9 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i32 0, i32 0
  store ptr %7, ptr %9, align 8
  %10 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %10, i32 0, i32 0
  %12 = load ptr, ptr %11, align 8
  %13 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %14 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %13, i32 0, i32 1
  store ptr %12, ptr %14, align 8
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %4, align 8
  %19 = getelementptr inbounds %"class.std::vector", ptr %17, i64 %18
  %20 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %21 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %20, i32 0, i32 2
  store ptr %19, ptr %21, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE11_M_allocateEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #2 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  %7 = load ptr, ptr %5, align 8
  %8 = load i64, ptr %6, align 8
  %9 = icmp ne i64 %8, 0
  br i1 %9, label %10, label %16

10:                                               ; preds = %2
  %11 = getelementptr inbounds %"struct.std::_Vector_base", ptr %7, i32 0, i32 0
  %12 = load i64, ptr %6, align 8
  store ptr %11, ptr %3, align 8
  store i64 %12, ptr %4, align 8
  %13 = load ptr, ptr %3, align 8
  %14 = load i64, ptr %4, align 8
  %15 = call noundef ptr @_ZNSt15__new_allocatorISt6vectorIiSaIiEEE8allocateEmPKv(ptr noundef nonnull align 1 dereferenceable(1) %13, i64 noundef %14, ptr noundef null)
  br label %17

16:                                               ; preds = %2
  br label %17

17:                                               ; preds = %16, %10
  %18 = phi ptr [ %15, %10 ], [ null, %16 ]
  ret ptr %18
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZNSt15__new_allocatorISt6vectorIiSaIiEEE8allocateEmPKv(ptr noundef nonnull align 1 dereferenceable(1) %0, i64 noundef %1, ptr noundef %2) local_unnamed_addr #2 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  store ptr %2, ptr %7, align 8
  %8 = load ptr, ptr %5, align 8
  %9 = load i64, ptr %6, align 8
  store ptr %8, ptr %4, align 8
  %10 = load ptr, ptr %4, align 8
  %11 = icmp ugt i64 %9, 384307168202282325
  br i1 %11, label %12, label %17

12:                                               ; preds = %3
  %13 = load i64, ptr %6, align 8
  %14 = icmp ugt i64 %13, 768614336404564650
  br i1 %14, label %15, label %16

15:                                               ; preds = %12
  call void @_ZSt28__throw_bad_array_new_lengthv() #24
  unreachable

16:                                               ; preds = %12
  call void @_ZSt17__throw_bad_allocv() #24
  unreachable

17:                                               ; preds = %3
  %18 = load i64, ptr %6, align 8
  %19 = mul i64 %18, 24
  %20 = call noalias noundef nonnull ptr @_Znwm(i64 noundef %19) #25
  ret ptr %20
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt12_Vector_baseISt6vectorIiSaIiEESaIS2_EE17_Vector_impl_dataC2Ev(ptr noundef nonnull align 8 dereferenceable(24) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i32 0, i32 0
  store ptr null, ptr %4, align 8
  %5 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i32 0, i32 1
  store ptr null, ptr %5, align 8
  %6 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i32 0, i32 2
  store ptr null, ptr %6, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNSt6vectorIS_IiSaIiEESaIS1_EE11_S_max_sizeERKS2_(ptr noundef nonnull align 1 dereferenceable(1) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  store ptr %0, ptr %5, align 8
  store i64 384307168202282325, ptr %6, align 8
  %8 = load ptr, ptr %5, align 8
  store ptr %8, ptr %4, align 8
  %9 = load ptr, ptr %4, align 8
  store ptr %9, ptr %3, align 8
  %10 = load ptr, ptr %3, align 8
  store ptr %10, ptr %2, align 8
  %11 = load ptr, ptr %2, align 8
  store i64 384307168202282325, ptr %7, align 8
  %12 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZSt3minImERKT_S2_S2_(ptr noundef nonnull align 8 dereferenceable(8) %6, ptr noundef nonnull align 8 dereferenceable(8) %7)
  %13 = load i64, ptr %12, align 8
  ret i64 %13
}

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #10

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) ptr @_ZSt3minImERKT_S2_S2_(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef nonnull align 8 dereferenceable(8) %1) local_unnamed_addr #3 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %6 = load ptr, ptr %5, align 8
  %7 = load i64, ptr %6, align 8
  %8 = load ptr, ptr %4, align 8
  %9 = load i64, ptr %8, align 8
  %10 = icmp ult i64 %7, %9
  br i1 %10, label %11, label %13

11:                                               ; preds = %2
  %12 = load ptr, ptr %5, align 8
  store ptr %12, ptr %3, align 8
  br label %15

13:                                               ; preds = %2
  %14 = load ptr, ptr %4, align 8
  store ptr %14, ptr %3, align 8
  br label %15

15:                                               ; preds = %13, %11
  %16 = load ptr, ptr %3, align 8
  ret ptr %16
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNSt6vectorIiSaIiEE17_S_check_init_lenEmRKS0_(i64 noundef %0, ptr noundef nonnull align 1 dereferenceable(1) %1) local_unnamed_addr #2 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i64, align 8
  %9 = alloca ptr, align 8
  %10 = alloca %"class.std::allocator.2", align 1
  store i64 %0, ptr %8, align 8
  store ptr %1, ptr %9, align 8
  %11 = load i64, ptr %8, align 8
  %12 = load ptr, ptr %9, align 8
  store ptr %10, ptr %5, align 8
  store ptr %12, ptr %6, align 8
  %13 = load ptr, ptr %5, align 8
  %14 = load ptr, ptr %6, align 8
  store ptr %13, ptr %3, align 8
  store ptr %14, ptr %4, align 8
  %15 = load ptr, ptr %3, align 8
  %16 = call noundef i64 @_ZNSt6vectorIiSaIiEE11_S_max_sizeERKS0_(ptr noundef nonnull align 1 dereferenceable(1) %10) #18
  %17 = icmp ugt i64 %11, %16
  store ptr %10, ptr %7, align 8
  %18 = load ptr, ptr %7, align 8
  call void @_ZNSt15__new_allocatorIiED2Ev(ptr noundef nonnull align 1 dereferenceable(1) %18) #18
  br i1 %17, label %19, label %20

19:                                               ; preds = %2
  call void @_ZSt20__throw_length_errorPKc(ptr noundef @.str.2.19) #23
  unreachable

20:                                               ; preds = %2
  %21 = load i64, ptr %8, align 8
  ret i64 %21
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIiSaIiEE18_M_fill_initializeEmRKi(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 4 dereferenceable(4) %2) local_unnamed_addr #2 comdat align 2 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = getelementptr inbounds %"struct.std::_Vector_base", ptr %7, i32 0, i32 0
  %9 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i32 0, i32 0
  %10 = load ptr, ptr %9, align 8
  %11 = load i64, ptr %5, align 8
  %12 = load ptr, ptr %6, align 8
  %13 = call noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt12_Vector_baseIiSaIiEE19_M_get_Tp_allocatorEv(ptr noundef nonnull align 8 dereferenceable(24) %7) #18
  %14 = call noundef ptr @_ZSt24__uninitialized_fill_n_aIPimiiET_S1_T0_RKT1_RSaIT2_E(ptr noundef %10, i64 noundef %11, ptr noundef nonnull align 4 dereferenceable(4) %12, ptr noundef nonnull align 1 dereferenceable(1) %13)
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %7, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 1
  store ptr %14, ptr %16, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt24__uninitialized_fill_n_aIPimiiET_S1_T0_RKT1_RSaIT2_E(ptr noundef %0, i64 noundef %1, ptr noundef nonnull align 4 dereferenceable(4) %2, ptr noundef nonnull align 1 dereferenceable(1) %3) local_unnamed_addr #2 comdat {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  store i64 %1, ptr %6, align 8
  store ptr %2, ptr %7, align 8
  store ptr %3, ptr %8, align 8
  %9 = load ptr, ptr %5, align 8
  %10 = load i64, ptr %6, align 8
  %11 = load ptr, ptr %7, align 8
  %12 = call noundef ptr @_ZSt20uninitialized_fill_nIPimiET_S1_T0_RKT1_(ptr noundef %9, i64 noundef %10, ptr noundef nonnull align 4 dereferenceable(4) %11)
  ret ptr %12
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt20uninitialized_fill_nIPimiET_S1_T0_RKT1_(ptr noundef %0, i64 noundef %1, ptr noundef nonnull align 4 dereferenceable(4) %2) local_unnamed_addr #2 comdat {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = load i64, ptr %5, align 8
  %9 = load ptr, ptr %6, align 8
  %10 = call noundef ptr @_ZSt18__do_uninit_fill_nIPimiET_S1_T0_RKT1_(ptr noundef %7, i64 noundef %8, ptr noundef nonnull align 4 dereferenceable(4) %9)
  ret ptr %10
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef ptr @_ZSt18__do_uninit_fill_nIPimiET_S1_T0_RKT1_(ptr noundef %0, i64 noundef %1, ptr noundef nonnull align 4 dereferenceable(4) %2) local_unnamed_addr #2 comdat personality ptr @__gxx_personality_v0 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca %"struct.std::_UninitDestroyGuard.5", align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  call void @_ZNSt19_UninitDestroyGuardIPivEC2ERS0_(ptr noundef nonnull align 8 dereferenceable(16) %7, ptr noundef nonnull align 8 dereferenceable(8) %4)
  br label %10

10:                                               ; preds = %3
  %11 = load i64, ptr %5, align 8
  %12 = icmp uge i64 %11, 0
  %13 = xor i1 %12, true
  br i1 %13, label %14, label %15

14:                                               ; preds = %10
  call void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef @.str.7, i32 noundef 463, ptr noundef @__PRETTY_FUNCTION__._ZSt18__do_uninit_fill_nIPimiET_S1_T0_RKT1_, ptr noundef @.str.8) #20
  unreachable

15:                                               ; preds = %10
  br label %16

16:                                               ; preds = %15
  br label %17

17:                                               ; preds = %16
  br label %18

18:                                               ; preds = %26, %17
  %19 = load i64, ptr %5, align 8
  %20 = add i64 %19, -1
  store i64 %20, ptr %5, align 8
  %21 = icmp ne i64 %19, 0
  br i1 %21, label %22, label %29

22:                                               ; preds = %18
  %23 = load ptr, ptr %4, align 8
  %24 = load ptr, ptr %6, align 8
  call void @_ZSt10_ConstructIiJRKiEEvPT_DpOT0_(ptr noundef %23, ptr noundef nonnull align 4 dereferenceable(4) %24)
  br label %25

25:                                               ; preds = %22
  br label %26

26:                                               ; preds = %25
  %27 = load ptr, ptr %4, align 8
  %28 = getelementptr inbounds i32, ptr %27, i32 1
  store ptr %28, ptr %4, align 8
  br label %18, !llvm.loop !17

29:                                               ; preds = %18
  call void @_ZNSt19_UninitDestroyGuardIPivE7releaseEv(ptr noundef nonnull align 8 dereferenceable(16) %7)
  br label %30

30:                                               ; preds = %29
  %31 = load ptr, ptr %4, align 8
  call void @_ZNSt19_UninitDestroyGuardIPivED2Ev(ptr noundef nonnull align 8 dereferenceable(16) %7) #18
  ret ptr %31
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt19_UninitDestroyGuardIPivEC2ERS0_(ptr noundef nonnull align 8 dereferenceable(16) %0, ptr noundef nonnull align 8 dereferenceable(8) %1) unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %5, i32 0, i32 0
  %7 = load ptr, ptr %4, align 8
  %8 = load ptr, ptr %7, align 8
  store ptr %8, ptr %6, align 8
  %9 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %5, i32 0, i32 1
  %10 = load ptr, ptr %4, align 8
  store ptr %10, ptr %9, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZSt10_ConstructIiJRKiEEvPT_DpOT0_(ptr noundef %0, ptr noundef nonnull align 4 dereferenceable(4) %1) local_unnamed_addr #3 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load ptr, ptr %4, align 8
  %7 = load i32, ptr %6, align 4
  store i32 %7, ptr %5, align 4
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt19_UninitDestroyGuardIPivED2Ev(ptr noundef nonnull align 8 dereferenceable(16) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 1
  %5 = load ptr, ptr %4, align 8
  %6 = icmp ne ptr %5, null
  br i1 %6, label %7, label %14

7:                                                ; preds = %1
  %8 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 1
  %11 = load ptr, ptr %10, align 8
  %12 = load ptr, ptr %11, align 8
  call void @_ZSt8_DestroyIPiEvT_S1_(ptr noundef %9, ptr noundef %12)
  br label %13

13:                                               ; preds = %7
  br label %14

14:                                               ; preds = %13, %1
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt19_UninitDestroyGuardIPivE7releaseEv(ptr noundef nonnull align 8 dereferenceable(16) %0) local_unnamed_addr #3 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_UninitDestroyGuard.5", ptr %3, i32 0, i32 1
  store ptr null, ptr %4, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNSt6vectorIiSaIiEE11_S_max_sizeERKS0_(ptr noundef nonnull align 1 dereferenceable(1) %0) local_unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  store ptr %0, ptr %5, align 8
  store i64 2305843009213693951, ptr %6, align 8
  %8 = load ptr, ptr %5, align 8
  store ptr %8, ptr %4, align 8
  %9 = load ptr, ptr %4, align 8
  store ptr %9, ptr %3, align 8
  %10 = load ptr, ptr %3, align 8
  store ptr %10, ptr %2, align 8
  %11 = load ptr, ptr %2, align 8
  store i64 2305843009213693951, ptr %7, align 8
  %12 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZSt3minImERKT_S2_S2_(ptr noundef nonnull align 8 dereferenceable(8) %6, ptr noundef nonnull align 8 dereferenceable(8) %7)
  br label %13

13:                                               ; preds = %1
  %14 = load i64, ptr %12, align 8
  ret i64 %14
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @print_matrix(ptr noundef nonnull align 8 dereferenceable(24) %0) local_unnamed_addr #2 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store ptr %0, ptr %2, align 8
  %6 = load ptr, ptr %2, align 8
  %7 = call noundef i64 @_ZNKSt6vectorIS_IiSaIiEESaIS1_EE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %6) #18
  %8 = trunc i64 %7 to i32
  store i32 %8, ptr %3, align 4
  store i32 0, ptr %4, align 4
  br label %9

9:                                                ; preds = %47, %1
  %10 = load i32, ptr %4, align 4
  %11 = load i32, ptr %3, align 4
  %12 = icmp slt i32 %10, %11
  br i1 %12, label %13, label %50

13:                                               ; preds = %9
  store i32 0, ptr %5, align 4
  br label %14

14:                                               ; preds = %42, %13
  %15 = load i32, ptr %5, align 4
  %16 = load i32, ptr %3, align 4
  %17 = icmp slt i32 %15, %16
  br i1 %17, label %18, label %45

18:                                               ; preds = %14
  %19 = load ptr, ptr %2, align 8
  %20 = load i32, ptr %4, align 4
  %21 = sext i32 %20 to i64
  %22 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNKSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %19, i64 noundef %21) #18
  %23 = load i32, ptr %5, align 4
  %24 = sext i32 %23 to i64
  %25 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNKSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %22, i64 noundef %24) #18
  %26 = load i32, ptr %25, align 4
  %27 = sitofp i32 %26 to double
  %28 = fcmp oge double %27, 1.000000e+09
  br i1 %28, label %29, label %31

29:                                               ; preds = %18
  %30 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  br label %41

31:                                               ; preds = %18
  %32 = load ptr, ptr %2, align 8
  %33 = load i32, ptr %4, align 4
  %34 = sext i32 %33 to i64
  %35 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNKSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %32, i64 noundef %34) #18
  %36 = load i32, ptr %5, align 4
  %37 = sext i32 %36 to i64
  %38 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNKSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %35, i64 noundef %37) #18
  %39 = load i32, ptr %38, align 4
  %40 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %39)
  br label %41

41:                                               ; preds = %31, %29
  br label %42

42:                                               ; preds = %41
  %43 = load i32, ptr %5, align 4
  %44 = add nsw i32 %43, 1
  store i32 %44, ptr %5, align 4
  br label %14, !llvm.loop !18

45:                                               ; preds = %14
  %46 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  br label %47

47:                                               ; preds = %45
  %48 = load i32, ptr %4, align 4
  %49 = add nsw i32 %48, 1
  store i32 %49, ptr %4, align 4
  br label %9, !llvm.loop !19

50:                                               ; preds = %9
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(24) ptr @_ZNKSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  br label %6

6:                                                ; preds = %2
  %7 = load i64, ptr %4, align 8
  %8 = call noundef i64 @_ZNKSt6vectorIS_IiSaIiEESaIS1_EE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %5) #18
  %9 = icmp ult i64 %7, %8
  %10 = xor i1 %9, true
  br i1 %10, label %11, label %12

11:                                               ; preds = %6
  call void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef @.str.9, i32 noundef 1282, ptr noundef @__PRETTY_FUNCTION__._ZNKSt6vectorIS_IiSaIiEESaIS1_EEixEm, ptr noundef @.str.10) #20
  unreachable

12:                                               ; preds = %6
  br label %13

13:                                               ; preds = %12
  br label %14

14:                                               ; preds = %13
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %4, align 8
  %19 = getelementptr inbounds %"class.std::vector", ptr %17, i64 %18
  ret ptr %19
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 4 dereferenceable(4) ptr @_ZNKSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) local_unnamed_addr #3 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  br label %6

6:                                                ; preds = %2
  %7 = load i64, ptr %4, align 8
  %8 = call noundef i64 @_ZNKSt6vectorIiSaIiEE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %5) #18
  %9 = icmp ult i64 %7, %8
  %10 = xor i1 %9, true
  br i1 %10, label %11, label %12

11:                                               ; preds = %6
  call void @_ZSt21__glibcxx_assert_failPKciS0_S0_(ptr noundef @.str.9, i32 noundef 1282, ptr noundef @__PRETTY_FUNCTION__._ZNKSt6vectorIiSaIiEEixEm, ptr noundef @.str.10) #20
  unreachable

12:                                               ; preds = %6
  br label %13

13:                                               ; preds = %12
  br label %14

14:                                               ; preds = %13
  %15 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %15, i32 0, i32 0
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %4, align 8
  %19 = getelementptr inbounds i32, ptr %17, i64 %18
  ret ptr %19
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @floyd_runtime(ptr noundef %0) local_unnamed_addr #2 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  %3 = alloca %"class.std::vector", align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca double, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca double, align 8
  store ptr %0, ptr %2, align 8
  %12 = load ptr, ptr %2, align 8
  %13 = icmp ne ptr %12, null
  br i1 %13, label %15, label %14

14:                                               ; preds = %1
  br label %115

15:                                               ; preds = %1
  %16 = load ptr, ptr %2, align 8
  call void @csr_to_adj_matrix(ptr dead_on_unwind writable sret(%"class.std::vector") align 8 %3, ptr noundef %16)
  %17 = call i32 (ptr, ...) @printf(ptr noundef @.str.3)
  br label %18

18:                                               ; preds = %15
  invoke void @print_matrix(ptr noundef nonnull align 8 dereferenceable(24) %3)
          to label %19 unwind label %86

19:                                               ; preds = %18
  %20 = load ptr, ptr %2, align 8
  %21 = getelementptr inbounds %struct.Graph, ptr %20, i32 0, i32 0
  %22 = load i64, ptr %21, align 8
  %23 = trunc i64 %22 to i32
  store i32 %23, ptr %6, align 4
  %24 = call double @omp_get_wtime()
  br label %25

25:                                               ; preds = %19
  store double %24, ptr %7, align 8
  store i32 0, ptr %8, align 4
  br label %26

26:                                               ; preds = %99, %25
  %27 = load i32, ptr %8, align 4
  %28 = load i32, ptr %6, align 4
  %29 = icmp slt i32 %27, %28
  br i1 %29, label %30, label %102

30:                                               ; preds = %26
  store i32 0, ptr %9, align 4
  br label %31

31:                                               ; preds = %95, %30
  %32 = load i32, ptr %9, align 4
  %33 = load i32, ptr %6, align 4
  %34 = icmp slt i32 %32, %33
  br i1 %34, label %35, label %98

35:                                               ; preds = %31
  store i32 0, ptr %10, align 4
  br label %36

36:                                               ; preds = %91, %35
  %37 = load i32, ptr %10, align 4
  %38 = load i32, ptr %6, align 4
  %39 = icmp slt i32 %37, %38
  br i1 %39, label %40, label %94

40:                                               ; preds = %36
  %41 = load i32, ptr %9, align 4
  %42 = sext i32 %41 to i64
  %43 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %42) #18
  %44 = load i32, ptr %8, align 4
  %45 = sext i32 %44 to i64
  %46 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %43, i64 noundef %45) #18
  %47 = load i32, ptr %46, align 4
  %48 = load i32, ptr %8, align 4
  %49 = sext i32 %48 to i64
  %50 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %49) #18
  %51 = load i32, ptr %10, align 4
  %52 = sext i32 %51 to i64
  %53 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %50, i64 noundef %52) #18
  %54 = load i32, ptr %53, align 4
  %55 = add nsw i32 %47, %54
  %56 = load i32, ptr %9, align 4
  %57 = sext i32 %56 to i64
  %58 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %57) #18
  %59 = load i32, ptr %10, align 4
  %60 = sext i32 %59 to i64
  %61 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %58, i64 noundef %60) #18
  %62 = load i32, ptr %61, align 4
  %63 = icmp slt i32 %55, %62
  br i1 %63, label %64, label %90

64:                                               ; preds = %40
  %65 = load i32, ptr %9, align 4
  %66 = sext i32 %65 to i64
  %67 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %66) #18
  %68 = load i32, ptr %8, align 4
  %69 = sext i32 %68 to i64
  %70 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %67, i64 noundef %69) #18
  %71 = load i32, ptr %70, align 4
  %72 = load i32, ptr %8, align 4
  %73 = sext i32 %72 to i64
  %74 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %73) #18
  %75 = load i32, ptr %10, align 4
  %76 = sext i32 %75 to i64
  %77 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %74, i64 noundef %76) #18
  %78 = load i32, ptr %77, align 4
  %79 = add nsw i32 %71, %78
  %80 = load i32, ptr %9, align 4
  %81 = sext i32 %80 to i64
  %82 = call noundef nonnull align 8 dereferenceable(24) ptr @_ZNSt6vectorIS_IiSaIiEESaIS1_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %81) #18
  %83 = load i32, ptr %10, align 4
  %84 = sext i32 %83 to i64
  %85 = call noundef nonnull align 4 dereferenceable(4) ptr @_ZNSt6vectorIiSaIiEEixEm(ptr noundef nonnull align 8 dereferenceable(24) %82, i64 noundef %84) #18
  store i32 %79, ptr %85, align 4
  br label %90

86:                                               ; preds = %106, %18
  %87 = landingpad { ptr, i32 }
          cleanup
  %88 = extractvalue { ptr, i32 } %87, 0
  store ptr %88, ptr %4, align 8
  %89 = extractvalue { ptr, i32 } %87, 1
  store i32 %89, ptr %5, align 4
  call void @_ZNSt6vectorIS_IiSaIiEESaIS1_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %3) #18
  br label %116

90:                                               ; preds = %64, %40
  br label %91

91:                                               ; preds = %90
  %92 = load i32, ptr %10, align 4
  %93 = add nsw i32 %92, 1
  store i32 %93, ptr %10, align 4
  br label %36, !llvm.loop !20

94:                                               ; preds = %36
  br label %95

95:                                               ; preds = %94
  %96 = load i32, ptr %9, align 4
  %97 = add nsw i32 %96, 1
  store i32 %97, ptr %9, align 4
  br label %31, !llvm.loop !21

98:                                               ; preds = %31
  br label %99

99:                                               ; preds = %98
  %100 = load i32, ptr %8, align 4
  %101 = add nsw i32 %100, 1
  store i32 %101, ptr %8, align 4
  br label %26, !llvm.loop !22

102:                                              ; preds = %26
  %103 = call double @omp_get_wtime()
  br label %104

104:                                              ; preds = %102
  store double %103, ptr %11, align 8
  %105 = call i32 (ptr, ...) @printf(ptr noundef @.str.4)
  br label %106

106:                                              ; preds = %104
  invoke void @print_matrix(ptr noundef nonnull align 8 dereferenceable(24) %3)
          to label %107 unwind label %86

107:                                              ; preds = %106
  %108 = load double, ptr %11, align 8
  %109 = load double, ptr %7, align 8
  %110 = fsub double %108, %109
  %111 = call i32 @omp_get_max_threads()
  br label %112

112:                                              ; preds = %107
  %113 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, double noundef %110, i32 noundef %111)
  br label %114

114:                                              ; preds = %112
  call void @_ZNSt6vectorIS_IiSaIiEESaIS1_EED2Ev(ptr noundef nonnull align 8 dereferenceable(24) %3) #18
  br label %115

115:                                              ; preds = %114, %14
  ret void

116:                                              ; preds = %86
  %117 = load ptr, ptr %4, align 8
  %118 = load i32, ptr %5, align 4
  %119 = insertvalue { ptr, i32 } poison, ptr %117, 0
  %120 = insertvalue { ptr, i32 } %119, i32 %118, 1
  resume { ptr, i32 } %120
}

; Function Attrs: nounwind
declare dso_local double @omp_get_wtime() local_unnamed_addr #11

; Function Attrs: nounwind
declare dso_local i32 @omp_get_max_threads() local_unnamed_addr #11

; Function Attrs: mustprogress uwtable
define dso_local void @dfs_runtime(ptr noundef readonly %0) local_unnamed_addr #12 personality ptr @__gxx_personality_v0 {
  %2 = icmp eq ptr %0, null
  br i1 %2, label %127, label %3

3:                                                ; preds = %1
  %4 = load i64, ptr %0, align 8, !tbaa !23
  %5 = freeze i64 %4
  %6 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %7 = load ptr, ptr %6, align 8, !tbaa !29
  %8 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %9 = load ptr, ptr %8, align 8, !tbaa !30
  %10 = icmp slt i64 %5, 0
  br i1 %10, label %11, label %12

11:                                               ; preds = %3
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2.19) #23
  unreachable

12:                                               ; preds = %3
  %13 = icmp eq i64 %5, 0
  br i1 %13, label %127, label %14

14:                                               ; preds = %12
  %15 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %5) #25
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %15, i8 0, i64 %5, i1 false)
  %16 = trunc i64 %5 to i32
  %17 = icmp sgt i32 %16, 0
  br i1 %17, label %18, label %.loopexit

18:                                               ; preds = %14
  %19 = icmp ugt i64 %5, 2305843009213693951
  %20 = shl nuw nsw i64 %5, 2
  %21 = and i64 %5, 4294967295
  br i1 %19, label %.preheader, label %.preheader16

.preheader:                                       ; preds = %18, %26
  %22 = phi i64 [ %27, %26 ], [ 0, %18 ]
  %23 = getelementptr inbounds i8, ptr %15, i64 %22
  %24 = load i8, ptr %23, align 1, !tbaa !31
  %25 = icmp eq i8 %24, 0
  br i1 %25, label %33, label %26

26:                                               ; preds = %.preheader
  %27 = add nuw nsw i64 %22, 1
  %28 = icmp eq i64 %27, %21
  br i1 %28, label %.loopexit, label %.preheader, !llvm.loop !32

.loopexit:                                        ; preds = %117, %26, %14
  tail call void @_ZdlPv(ptr noundef nonnull %15) #22
  br label %127

.preheader16:                                     ; preds = %18, %117
  %29 = phi i64 [ %118, %117 ], [ 0, %18 ]
  %30 = getelementptr inbounds i8, ptr %15, i64 %29
  %31 = load i8, ptr %30, align 1, !tbaa !31
  %32 = icmp eq i8 %31, 0
  br i1 %32, label %35, label %117

33:                                               ; preds = %.preheader
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1.12) #23
          to label %34 unwind label %53

34:                                               ; preds = %33
  unreachable

35:                                               ; preds = %.preheader16
  %36 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %20) #25
          to label %37 unwind label %51

37:                                               ; preds = %35
  %38 = trunc i64 %29 to i32
  store i32 %38, ptr %36, align 4, !tbaa !33
  %39 = getelementptr inbounds i32, ptr %36, i64 %5
  %40 = getelementptr inbounds i32, ptr %36, i64 1
  br label %41

41:                                               ; preds = %37, %.loopexit15
  %42 = phi ptr [ %39, %37 ], [ %112, %.loopexit15 ]
  %43 = phi ptr [ %40, %37 ], [ %111, %.loopexit15 ]
  %44 = phi ptr [ %36, %37 ], [ %110, %.loopexit15 ]
  %45 = getelementptr inbounds i32, ptr %43, i64 -1
  %46 = load i32, ptr %45, align 4, !tbaa !33
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds i8, ptr %15, i64 %47
  %49 = load i8, ptr %48, align 1, !tbaa !31
  %50 = icmp eq i8 %49, 0
  br i1 %50, label %55, label %.loopexit15, !llvm.loop !35

51:                                               ; preds = %35
  %52 = landingpad { ptr, i32 }
          cleanup
  br label %125

53:                                               ; preds = %33
  %54 = landingpad { ptr, i32 }
          cleanup
  br label %125

55:                                               ; preds = %41
  store i8 1, ptr %48, align 1, !tbaa !31
  %56 = getelementptr inbounds i64, ptr %7, i64 %47
  %57 = load i64, ptr %56, align 8, !tbaa !36
  %58 = getelementptr i64, ptr %56, i64 1
  %59 = load i64, ptr %58, align 8, !tbaa !36
  %60 = icmp sgt i64 %59, %57
  br i1 %60, label %.preheader14, label %.loopexit15

.preheader14:                                     ; preds = %55, %105
  %61 = phi i64 [ %65, %105 ], [ %59, %55 ]
  %62 = phi ptr [ %108, %105 ], [ %42, %55 ]
  %63 = phi ptr [ %107, %105 ], [ %45, %55 ]
  %64 = phi ptr [ %106, %105 ], [ %44, %55 ]
  %65 = add nsw i64 %61, -1
  %66 = getelementptr inbounds i32, ptr %9, i64 %65
  %67 = load i32, ptr %66, align 4, !tbaa !33
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds i8, ptr %15, i64 %68
  %70 = load i8, ptr %69, align 1, !tbaa !31
  %71 = icmp eq i8 %70, 0
  br i1 %71, label %72, label %105

72:                                               ; preds = %.preheader14
  %73 = icmp eq ptr %63, %62
  br i1 %73, label %76, label %74

74:                                               ; preds = %72
  store i32 %67, ptr %63, align 4, !tbaa !33
  %75 = getelementptr inbounds i32, ptr %63, i64 1
  br label %105

76:                                               ; preds = %72
  %77 = ptrtoint ptr %62 to i64
  %78 = ptrtoint ptr %64 to i64
  %79 = sub i64 %77, %78
  %80 = ashr exact i64 %79, 2
  %81 = icmp sgt i64 %80, -1
  tail call void @llvm.assume(i1 %81)
  %82 = icmp eq i64 %79, 9223372036854775804
  br i1 %82, label %83, label %85

83:                                               ; preds = %76
  %.lcssa = phi ptr [ %64, %76 ]
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
          to label %84 unwind label %103

84:                                               ; preds = %83
  unreachable

85:                                               ; preds = %76
  %86 = tail call i64 @llvm.umax.i64(i64 %80, i64 1)
  %87 = add nuw nsw i64 %86, %80
  %88 = tail call noundef i64 @llvm.umin.i64(i64 %87, i64 2305843009213693951)
  %89 = shl nuw nsw i64 %88, 2
  %90 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %89) #25
          to label %91 unwind label %101

91:                                               ; preds = %85
  %92 = getelementptr inbounds i8, ptr %90, i64 %79
  store i32 %67, ptr %92, align 4, !tbaa !33
  %93 = icmp sgt i64 %79, 0
  br i1 %93, label %94, label %95

94:                                               ; preds = %91
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %90, ptr align 4 %64, i64 %79, i1 false)
  br label %95

95:                                               ; preds = %94, %91
  %96 = icmp eq ptr %64, null
  br i1 %96, label %98, label %97

97:                                               ; preds = %95
  tail call void @_ZdlPv(ptr noundef nonnull %64) #22
  br label %98

98:                                               ; preds = %97, %95
  %99 = getelementptr inbounds i32, ptr %92, i64 1
  %100 = getelementptr inbounds i32, ptr %90, i64 %88
  br label %105

101:                                              ; preds = %85
  %.lcssa2 = phi ptr [ %64, %85 ]
  %102 = landingpad { ptr, i32 }
          cleanup
  br label %120

103:                                              ; preds = %83
  %104 = landingpad { ptr, i32 }
          cleanup
  br label %120

105:                                              ; preds = %98, %74, %.preheader14
  %106 = phi ptr [ %64, %.preheader14 ], [ %90, %98 ], [ %64, %74 ]
  %107 = phi ptr [ %63, %.preheader14 ], [ %99, %98 ], [ %75, %74 ]
  %108 = phi ptr [ %62, %.preheader14 ], [ %100, %98 ], [ %62, %74 ]
  %109 = icmp sgt i64 %65, %57
  br i1 %109, label %.preheader14, label %.loopexit15, !llvm.loop !37

.loopexit15:                                      ; preds = %105, %55, %41
  %110 = phi ptr [ %44, %41 ], [ %44, %55 ], [ %106, %105 ]
  %111 = phi ptr [ %45, %41 ], [ %45, %55 ], [ %107, %105 ]
  %112 = phi ptr [ %42, %41 ], [ %42, %55 ], [ %108, %105 ]
  %113 = icmp eq ptr %110, %111
  br i1 %113, label %114, label %41

114:                                              ; preds = %.loopexit15
  %.lcssa7 = phi ptr [ %110, %.loopexit15 ]
  %115 = icmp eq ptr %.lcssa7, null
  br i1 %115, label %117, label %116

116:                                              ; preds = %114
  tail call void @_ZdlPv(ptr noundef nonnull %.lcssa7) #22
  br label %117

117:                                              ; preds = %116, %114, %.preheader16
  %118 = add nuw nsw i64 %29, 1
  %119 = icmp eq i64 %118, %21
  br i1 %119, label %.loopexit, label %.preheader16, !llvm.loop !32

120:                                              ; preds = %101, %103
  %121 = phi ptr [ %.lcssa2, %101 ], [ %.lcssa, %103 ]
  %122 = phi { ptr, i32 } [ %102, %101 ], [ %104, %103 ]
  %123 = icmp eq ptr %121, null
  br i1 %123, label %125, label %124

124:                                              ; preds = %120
  tail call void @_ZdlPv(ptr noundef nonnull %121) #22
  br label %125

125:                                              ; preds = %51, %53, %120, %124
  %126 = phi { ptr, i32 } [ %122, %120 ], [ %122, %124 ], [ %52, %51 ], [ %54, %53 ]
  tail call void @_ZdlPv(ptr noundef nonnull %15) #22
  resume { ptr, i32 } %126

127:                                              ; preds = %12, %.loopexit, %1
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #13

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #14

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #15

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #15

; Function Attrs: mustprogress uwtable
define dso_local void @bfs_runtime(ptr noundef readonly %0) local_unnamed_addr #12 personality ptr @__gxx_personality_v0 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca %"class.std::vector", align 8
  %5 = alloca double, align 8
  %6 = alloca %"class.std::vector", align 8
  %7 = alloca %"class.std::vector", align 8
  %8 = alloca double, align 8
  %9 = icmp eq ptr %0, null
  br i1 %9, label %78, label %10

10:                                               ; preds = %1
  %11 = load i64, ptr %0, align 8, !tbaa !23
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #18
  %12 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %13 = load ptr, ptr %12, align 8, !tbaa !29
  store ptr %13, ptr %2, align 8, !tbaa !38
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #18
  %14 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %15 = load ptr, ptr %14, align 8, !tbaa !30
  store ptr %15, ptr %3, align 8, !tbaa !38
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #18
  %16 = icmp slt i64 %11, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %10
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2.19) #23
  unreachable

18:                                               ; preds = %10
  %19 = icmp eq i64 %11, 0
  br i1 %19, label %20, label %22

20:                                               ; preds = %18
  %21 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %4, i8 0, i64 24, i1 false)
  br label %27

22:                                               ; preds = %18
  %23 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %11) #25
  store ptr %23, ptr %4, align 8, !tbaa !39
  %24 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  store ptr %23, ptr %24, align 8, !tbaa !41
  %25 = getelementptr inbounds i8, ptr %23, i64 %11
  %26 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 2
  store ptr %25, ptr %26, align 8, !tbaa !42
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %23, i8 0, i64 %11, i1 false)
  br label %27

27:                                               ; preds = %22, %20
  %28 = phi ptr [ %24, %22 ], [ %21, %20 ]
  %29 = phi ptr [ %25, %22 ], [ null, %20 ]
  store ptr %29, ptr %28, align 8, !tbaa !41
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %5) #18
  %30 = tail call double @omp_get_wtime()
  store double %30, ptr %5, align 8, !tbaa !43
  %31 = trunc i64 %11 to i32
  %32 = icmp sgt i32 %31, 0
  br i1 %32, label %33, label %.loopexit8

33:                                               ; preds = %27
  %34 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %6, i64 0, i32 1
  %35 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %6, i64 0, i32 2
  %36 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i64 0, i32 1
  %37 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i64 0, i32 2
  %38 = and i64 %11, 4294967295
  br label %44

.loopexit8:                                       ; preds = %75, %27
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %8) #18
  %39 = call double @omp_get_wtime()
  store double %39, ptr %8, align 8, !tbaa !43
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @3, i32 2, ptr nonnull @bfs_runtime.omp_outlined.1, ptr nonnull %8, ptr nonnull %5)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %8) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %5) #18
  %40 = load ptr, ptr %4, align 8, !tbaa !39
  %41 = icmp eq ptr %40, null
  br i1 %41, label %43, label %42

42:                                               ; preds = %.loopexit8
  call void @_ZdlPv(ptr noundef nonnull %40) #22
  br label %43

43:                                               ; preds = %.loopexit8, %42
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #18
  br label %78

44:                                               ; preds = %33, %75
  %45 = phi i64 [ 0, %33 ], [ %76, %75 ]
  %46 = load ptr, ptr %4, align 8, !tbaa !39
  %47 = getelementptr inbounds i8, ptr %46, i64 %45
  %48 = load i8, ptr %47, align 1, !tbaa !31
  %49 = icmp eq i8 %48, 0
  br i1 %49, label %50, label %75

50:                                               ; preds = %44
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %6) #18
  %51 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #25
          to label %52 unwind label %79

52:                                               ; preds = %50
  %53 = trunc i64 %45 to i32
  store i32 %53, ptr %51, align 4, !tbaa !33
  %54 = getelementptr inbounds i32, ptr %51, i64 1
  store ptr %51, ptr %6, align 8, !tbaa !45
  store ptr %54, ptr %34, align 8, !tbaa !47
  store ptr %54, ptr %35, align 8, !tbaa !48
  store i8 1, ptr %47, align 1, !tbaa !31
  %55 = load ptr, ptr %6, align 8, !tbaa !38
  %56 = load ptr, ptr %34, align 8, !tbaa !38
  %57 = icmp eq ptr %55, %56
  br i1 %57, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %52, %67
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #18
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %7, i8 0, i64 24, i1 false)
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @3, i32 5, ptr nonnull @bfs_runtime.omp_outlined, ptr nonnull %6, ptr nonnull %2, ptr nonnull %3, ptr nonnull %4, ptr nonnull %7)
  %58 = load ptr, ptr %6, align 8, !tbaa !45
  %59 = load ptr, ptr %7, align 8, !tbaa !45
  store ptr %59, ptr %6, align 8, !tbaa !45
  %60 = load ptr, ptr %36, align 8, !tbaa !47
  %61 = load ptr, ptr %37, align 8, !tbaa !48
  store ptr %58, ptr %7, align 8, !tbaa !45
  %62 = load <2 x ptr>, ptr %34, align 8, !tbaa !38
  store ptr %60, ptr %34, align 8, !tbaa !47
  store ptr %61, ptr %35, align 8, !tbaa !48
  store <2 x ptr> %62, ptr %36, align 8, !tbaa !38
  %63 = icmp eq ptr %58, null
  br i1 %63, label %67, label %64

64:                                               ; preds = %.preheader
  call void @_ZdlPv(ptr noundef nonnull %58) #22
  %65 = load ptr, ptr %6, align 8, !tbaa !38
  %66 = load ptr, ptr %34, align 8, !tbaa !38
  br label %67

67:                                               ; preds = %.preheader, %64
  %68 = phi ptr [ %60, %.preheader ], [ %66, %64 ]
  %69 = phi ptr [ %59, %.preheader ], [ %65, %64 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #18
  %70 = icmp eq ptr %69, %68
  br i1 %70, label %.loopexit, label %.preheader, !llvm.loop !49

.loopexit:                                        ; preds = %67, %52
  %71 = phi ptr [ %55, %52 ], [ %68, %67 ]
  %72 = icmp eq ptr %71, null
  br i1 %72, label %74, label %73

73:                                               ; preds = %.loopexit
  call void @_ZdlPv(ptr noundef nonnull %71) #22
  br label %74

74:                                               ; preds = %.loopexit, %73
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #18
  br label %75

75:                                               ; preds = %44, %74
  %76 = add nuw nsw i64 %45, 1
  %77 = icmp eq i64 %76, %38
  br i1 %77, label %.loopexit8, label %44, !llvm.loop !50

78:                                               ; preds = %1, %43
  ret void

79:                                               ; preds = %50
  %.lcssa1 = phi ptr [ %46, %50 ]
  %80 = landingpad { ptr, i32 }
          cleanup
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %5) #18
  call void @_ZdlPv(ptr noundef nonnull %.lcssa1) #22
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #18
  resume { ptr, i32 } %80
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #16

; Function Attrs: alwaysinline norecurse nounwind uwtable
define internal void @bfs_runtime.omp_outlined.1(ptr noalias nocapture noundef readonly %0, ptr noalias nocapture readnone %1, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %2, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %3) #17 personality ptr @__gxx_personality_v0 {
  %5 = load i32, ptr %0, align 4, !tbaa !33
  %6 = tail call i32 @__kmpc_master(ptr nonnull @3, i32 %5)
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %14, label %8

8:                                                ; preds = %4
  %9 = load double, ptr %2, align 8, !tbaa !43
  %10 = load double, ptr %3, align 8, !tbaa !43
  %11 = tail call i32 @omp_get_max_threads()
  %12 = fsub double %9, %10
  %13 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.17, double noundef %12, i32 noundef %11)
  tail call void @__kmpc_end_master(ptr nonnull @3, i32 %5)
  br label %14

14:                                               ; preds = %8, %4
  ret void
}

; Function Attrs: nounwind
declare !callback !51 void @__kmpc_fork_call(ptr, i32, ptr, ...) local_unnamed_addr #18

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #16

; Function Attrs: alwaysinline norecurse nounwind uwtable
define internal void @bfs_runtime.omp_outlined(ptr noalias nocapture noundef readonly %0, ptr noalias nocapture readnone %1, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %2, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %3, ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %4, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %5, ptr noundef nonnull align 8 dereferenceable(24) %6) #17 personality ptr @__gxx_personality_v0 {
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i32, align 4
  %12 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %2, i64 0, i32 1
  %13 = load ptr, ptr %12, align 8, !tbaa !47
  %14 = load ptr, ptr %2, align 8, !tbaa !45
  %15 = ptrtoint ptr %13 to i64
  %16 = ptrtoint ptr %14 to i64
  %17 = sub i64 %15, %16
  %18 = ashr exact i64 %17, 2
  %19 = icmp sgt i64 %18, -1
  tail call void @llvm.assume(i1 %19)
  %20 = icmp eq ptr %13, %14
  %21 = load i32, ptr %0, align 4, !tbaa !33
  br i1 %20, label %106, label %22

22:                                               ; preds = %7
  %23 = add nsw i64 %18, -1
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %8) #18
  store i64 0, ptr %8, align 8, !tbaa !36
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %9) #18
  store i64 %23, ptr %9, align 8, !tbaa !36
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #18
  store i64 1, ptr %10, align 8, !tbaa !36
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %11) #18
  store i32 0, ptr %11, align 4, !tbaa !33
  call void @__kmpc_for_static_init_8u(ptr nonnull @2, i32 %21, i32 34, ptr nonnull %11, ptr nonnull %8, ptr nonnull %9, ptr nonnull %10, i64 1, i64 1)
  %24 = load i64, ptr %9, align 8
  %25 = call i64 @llvm.umin.i64(i64 %24, i64 %23)
  store i64 %25, ptr %9, align 8, !tbaa !36
  %26 = load i64, ptr %8, align 8, !tbaa !36
  %27 = add i64 %25, 1
  %28 = icmp ult i64 %26, %27
  br i1 %28, label %.preheader9, label %.loopexit

.preheader9:                                      ; preds = %22, %46
  %29 = phi i64 [ %47, %46 ], [ %25, %22 ]
  %30 = phi i64 [ %51, %46 ], [ %26, %22 ]
  %31 = phi ptr [ %50, %46 ], [ null, %22 ]
  %32 = phi ptr [ %49, %46 ], [ null, %22 ]
  %33 = phi ptr [ %48, %46 ], [ null, %22 ]
  %34 = load ptr, ptr %2, align 8, !tbaa !45
  %35 = getelementptr inbounds i32, ptr %34, i64 %30
  %36 = load i32, ptr %35, align 4, !tbaa !33
  %37 = load ptr, ptr %3, align 8, !tbaa !38
  %38 = sext i32 %36 to i64
  %39 = getelementptr inbounds i64, ptr %37, i64 %38
  %40 = load i64, ptr %39, align 8, !tbaa !36
  %41 = getelementptr i64, ptr %39, i64 1
  %42 = load i64, ptr %41, align 8, !tbaa !36
  %43 = icmp slt i64 %40, %42
  br i1 %43, label %.preheader, label %46

44:                                               ; preds = %98
  %.lcssa2 = phi ptr [ %99, %98 ]
  %.lcssa1 = phi ptr [ %100, %98 ]
  %.lcssa = phi ptr [ %101, %98 ]
  %45 = load i64, ptr %9, align 8, !tbaa !36
  br label %46

46:                                               ; preds = %44, %.preheader9
  %47 = phi i64 [ %29, %.preheader9 ], [ %45, %44 ]
  %48 = phi ptr [ %33, %.preheader9 ], [ %.lcssa2, %44 ]
  %49 = phi ptr [ %32, %.preheader9 ], [ %.lcssa1, %44 ]
  %50 = phi ptr [ %31, %.preheader9 ], [ %.lcssa, %44 ]
  %51 = add nuw i64 %30, 1
  %52 = add i64 %47, 1
  %53 = icmp ult i64 %51, %52
  br i1 %53, label %.preheader9, label %.loopexit

.preheader:                                       ; preds = %.preheader9, %98
  %54 = phi i64 [ %102, %98 ], [ %40, %.preheader9 ]
  %55 = phi ptr [ %101, %98 ], [ %31, %.preheader9 ]
  %56 = phi ptr [ %100, %98 ], [ %32, %.preheader9 ]
  %57 = phi ptr [ %99, %98 ], [ %33, %.preheader9 ]
  %58 = load ptr, ptr %4, align 8, !tbaa !38
  %59 = getelementptr inbounds i32, ptr %58, i64 %54
  %60 = load i32, ptr %59, align 4, !tbaa !33
  %61 = sext i32 %60 to i64
  %62 = load ptr, ptr %5, align 8, !tbaa !39
  %63 = getelementptr inbounds i8, ptr %62, i64 %61
  %64 = load i8, ptr %63, align 1, !tbaa !31
  %65 = icmp eq i8 %64, 0
  br i1 %65, label %66, label %98

66:                                               ; preds = %.preheader
  %67 = atomicrmw xchg ptr %63, i8 1 seq_cst, align 1
  %68 = icmp eq i8 %67, 0
  br i1 %68, label %69, label %98

69:                                               ; preds = %66
  %70 = icmp eq ptr %56, %57
  br i1 %70, label %73, label %71

71:                                               ; preds = %69
  store i32 %60, ptr %56, align 4, !tbaa !33
  %72 = getelementptr inbounds i32, ptr %56, i64 1
  br label %98

73:                                               ; preds = %69
  %74 = ptrtoint ptr %56 to i64
  %75 = ptrtoint ptr %55 to i64
  %76 = sub i64 %74, %75
  %77 = ashr exact i64 %76, 2
  %78 = icmp sgt i64 %77, -1
  call void @llvm.assume(i1 %78)
  %79 = icmp eq i64 %76, 9223372036854775804
  br i1 %79, label %80, label %82

80:                                               ; preds = %73
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
          to label %81 unwind label %122

81:                                               ; preds = %80
  unreachable

82:                                               ; preds = %73
  %83 = call i64 @llvm.umax.i64(i64 %77, i64 1)
  %84 = add nuw nsw i64 %83, %77
  %85 = call noundef i64 @llvm.umin.i64(i64 %84, i64 2305843009213693951)
  %86 = shl nuw nsw i64 %85, 2
  %87 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %86) #25
          to label %88 unwind label %120

88:                                               ; preds = %82
  %89 = getelementptr inbounds i8, ptr %87, i64 %76
  store i32 %60, ptr %89, align 4, !tbaa !33
  %90 = icmp sgt i64 %76, 0
  br i1 %90, label %91, label %92

91:                                               ; preds = %88
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %87, ptr align 4 %55, i64 %76, i1 false)
  br label %92

92:                                               ; preds = %91, %88
  %93 = icmp eq ptr %55, null
  br i1 %93, label %95, label %94

94:                                               ; preds = %92
  call void @_ZdlPv(ptr noundef nonnull %55) #22
  br label %95

95:                                               ; preds = %94, %92
  %96 = getelementptr inbounds i32, ptr %89, i64 1
  %97 = getelementptr inbounds i32, ptr %87, i64 %85
  br label %98

98:                                               ; preds = %95, %71, %66, %.preheader
  %99 = phi ptr [ %57, %66 ], [ %57, %.preheader ], [ %97, %95 ], [ %57, %71 ]
  %100 = phi ptr [ %56, %66 ], [ %56, %.preheader ], [ %96, %95 ], [ %72, %71 ]
  %101 = phi ptr [ %55, %66 ], [ %55, %.preheader ], [ %87, %95 ], [ %55, %71 ]
  %102 = add i64 %54, 1
  %103 = icmp eq i64 %102, %42
  br i1 %103, label %44, label %.preheader, !llvm.loop !53

.loopexit:                                        ; preds = %46, %22
  %104 = phi ptr [ null, %22 ], [ %49, %46 ]
  %105 = phi ptr [ null, %22 ], [ %50, %46 ]
  call void @__kmpc_for_static_fini(ptr nonnull @2, i32 %21)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %11) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %9) #18
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %8) #18
  br label %106

106:                                              ; preds = %.loopexit, %7
  %107 = phi ptr [ null, %7 ], [ %104, %.loopexit ]
  %108 = phi ptr [ null, %7 ], [ %105, %.loopexit ]
  call void @__kmpc_critical(ptr nonnull @3, i32 %21, ptr nonnull @.gomp_critical_user_.var)
  %109 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %6, i64 0, i32 1
  %110 = load ptr, ptr %109, align 8, !tbaa !38
  %111 = load ptr, ptr %6, align 8, !tbaa !38
  %112 = ptrtoint ptr %110 to i64
  %113 = ptrtoint ptr %111 to i64
  %114 = sub i64 %112, %113
  %115 = getelementptr inbounds i8, ptr %111, i64 %114
  invoke void @_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag(ptr noundef nonnull align 8 dereferenceable(24) %6, ptr %115, ptr %108, ptr %107)
          to label %116 unwind label %122

116:                                              ; preds = %106
  call void @__kmpc_end_critical(ptr nonnull @3, i32 %21, ptr nonnull @.gomp_critical_user_.var)
  %117 = icmp eq ptr %108, null
  br i1 %117, label %119, label %118

118:                                              ; preds = %116
  call void @_ZdlPv(ptr noundef nonnull %108) #22
  br label %119

119:                                              ; preds = %116, %118
  ret void

120:                                              ; preds = %82
  %121 = landingpad { ptr, i32 }
          catch ptr null
  br label %124

122:                                              ; preds = %80, %106
  %123 = landingpad { ptr, i32 }
          catch ptr null
  br label %124

124:                                              ; preds = %122, %120
  %125 = phi { ptr, i32 } [ %121, %120 ], [ %123, %122 ]
  %126 = extractvalue { ptr, i32 } %125, 0
  call void @__clang_call_terminate(ptr %126) #21
  unreachable
}

; Function Attrs: nounwind
declare void @__kmpc_for_static_init_8u(ptr, i32, i32, ptr, ptr, ptr, ptr, i64, i64) local_unnamed_addr #18

; Function Attrs: nounwind
declare void @__kmpc_for_static_fini(ptr, i32) local_unnamed_addr #18

; Function Attrs: convergent nounwind
declare void @__kmpc_critical(ptr, i32, ptr) local_unnamed_addr #19

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr %1, ptr %2, ptr %3) local_unnamed_addr #12 comdat align 2 personality ptr @__gxx_personality_v0 {
  %5 = ptrtoint ptr %1 to i64
  %6 = icmp eq ptr %2, %3
  br i1 %6, label %227, label %7

7:                                                ; preds = %4
  %8 = ptrtoint ptr %3 to i64
  %9 = ptrtoint ptr %2 to i64
  %10 = sub i64 %8, %9
  %11 = ashr exact i64 %10, 2
  %12 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  %13 = load ptr, ptr %12, align 8, !tbaa !48
  %14 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  %15 = load ptr, ptr %14, align 8, !tbaa !38
  %16 = ptrtoint ptr %13 to i64
  %17 = ptrtoint ptr %15 to i64
  %18 = sub i64 %16, %17
  %19 = icmp ult i64 %18, %10
  br i1 %19, label %128, label %20

20:                                               ; preds = %7
  %21 = sub i64 %17, %5
  %22 = ashr exact i64 %21, 2
  %23 = icmp ugt i64 %22, %11
  br i1 %23, label %24, label %75

24:                                               ; preds = %20
  %25 = sub nsw i64 0, %11
  %26 = getelementptr i32, ptr %15, i64 %25
  %27 = add i64 %10, -4
  %28 = lshr i64 %27, 2
  %29 = add nuw nsw i64 %28, 1
  %30 = icmp ult i64 %10, 32
  br i1 %30, label %.preheader35, label %31

31:                                               ; preds = %24
  %32 = and i64 %29, 9223372036854775800
  %33 = shl i64 %32, 2
  br label %34

34:                                               ; preds = %34, %31
  %35 = phi i64 [ 0, %31 ], [ %43, %34 ]
  %36 = shl i64 %35, 2
  %37 = getelementptr i8, ptr %26, i64 %36
  %38 = getelementptr i8, ptr %15, i64 %36
  %39 = getelementptr i32, ptr %37, i64 4
  %40 = load <4 x i32>, ptr %37, align 4, !tbaa !33
  %41 = load <4 x i32>, ptr %39, align 4, !tbaa !33
  %42 = getelementptr i32, ptr %38, i64 4
  store <4 x i32> %40, ptr %38, align 4, !tbaa !33
  store <4 x i32> %41, ptr %42, align 4, !tbaa !33
  %43 = add nuw i64 %35, 8
  %44 = icmp eq i64 %43, %32
  br i1 %44, label %45, label %34, !llvm.loop !54

45:                                               ; preds = %34
  %46 = getelementptr i8, ptr %26, i64 %33
  %47 = getelementptr i8, ptr %15, i64 %33
  %48 = icmp eq i64 %29, %32
  br i1 %48, label %.loopexit23, label %.preheader35

.preheader35:                                     ; preds = %24, %45
  %.ph36 = phi ptr [ %46, %45 ], [ %26, %24 ]
  %.ph37 = phi ptr [ %47, %45 ], [ %15, %24 ]
  br label %49

49:                                               ; preds = %.preheader35, %49
  %50 = phi ptr [ %53, %49 ], [ %.ph36, %.preheader35 ]
  %51 = phi ptr [ %54, %49 ], [ %.ph37, %.preheader35 ]
  %52 = load i32, ptr %50, align 4, !tbaa !33
  store i32 %52, ptr %51, align 4, !tbaa !33
  %53 = getelementptr inbounds i32, ptr %50, i64 1
  %54 = getelementptr inbounds i32, ptr %51, i64 1
  %55 = icmp eq ptr %53, %15
  br i1 %55, label %.loopexit23, label %49, !llvm.loop !57

.loopexit23:                                      ; preds = %49, %45
  %56 = getelementptr inbounds i8, ptr %15, i64 %10
  store ptr %56, ptr %14, align 8, !tbaa !47
  %57 = ptrtoint ptr %26 to i64
  %58 = sub i64 %57, %5
  %59 = ashr exact i64 %58, 2
  %60 = sub nsw i64 0, %59
  %61 = getelementptr inbounds i32, ptr %15, i64 %60
  %62 = icmp sgt i64 %59, 1
  br i1 %62, label %63, label %64, !prof !58

63:                                               ; preds = %.loopexit23
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %61, ptr align 4 %1, i64 %58, i1 false)
  br label %68

64:                                               ; preds = %.loopexit23
  %65 = icmp eq i64 %58, 4
  br i1 %65, label %66, label %68

66:                                               ; preds = %64
  %67 = load i32, ptr %1, align 4, !tbaa !33
  store i32 %67, ptr %61, align 4, !tbaa !33
  br label %68

68:                                               ; preds = %63, %64, %66
  %69 = icmp sgt i64 %10, 4
  br i1 %69, label %70, label %71, !prof !59

70:                                               ; preds = %68
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %1, ptr align 4 %2, i64 %10, i1 false)
  br label %227

71:                                               ; preds = %68
  %72 = icmp eq i64 %10, 4
  br i1 %72, label %73, label %227

73:                                               ; preds = %71
  %74 = load i32, ptr %2, align 4, !tbaa !33
  store i32 %74, ptr %1, align 4, !tbaa !33
  br label %227

75:                                               ; preds = %20
  %76 = getelementptr inbounds i8, ptr %2, i64 %21
  %77 = ptrtoint ptr %76 to i64
  %78 = sub i64 %8, %77
  %79 = icmp sgt i64 %78, 0
  br i1 %79, label %80, label %82, !prof !59

80:                                               ; preds = %75
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %15, ptr align 4 %76, i64 %78, i1 false)
  %81 = load ptr, ptr %14, align 8, !tbaa !47
  %.pre = ptrtoint ptr %81 to i64
  br label %82

82:                                               ; preds = %75, %80
  %.pre-phi = phi i64 [ %17, %75 ], [ %.pre, %80 ]
  %83 = phi ptr [ %15, %75 ], [ %81, %80 ]
  %84 = sub nsw i64 %11, %22
  %85 = getelementptr i32, ptr %83, i64 %84
  %86 = icmp eq ptr %15, %1
  br i1 %86, label %.loopexit24, label %87

87:                                               ; preds = %82
  %88 = add i64 %21, -4
  %89 = lshr i64 %88, 2
  %90 = add nuw nsw i64 %89, 1
  %91 = icmp ult i64 %88, 76
  br i1 %91, label %.preheader38, label %92

92:                                               ; preds = %87
  %93 = sub i64 %10, %17
  %94 = add i64 %93, %.pre-phi
  %95 = icmp ult i64 %94, 32
  br i1 %95, label %.preheader38, label %96

96:                                               ; preds = %92
  %97 = and i64 %90, 9223372036854775800
  %98 = shl i64 %97, 2
  br label %99

99:                                               ; preds = %99, %96
  %100 = phi i64 [ 0, %96 ], [ %108, %99 ]
  %101 = shl i64 %100, 2
  %102 = getelementptr i8, ptr %1, i64 %101
  %103 = getelementptr i8, ptr %85, i64 %101
  %104 = getelementptr i32, ptr %102, i64 4
  %105 = load <4 x i32>, ptr %102, align 4, !tbaa !33
  %106 = load <4 x i32>, ptr %104, align 4, !tbaa !33
  %107 = getelementptr i32, ptr %103, i64 4
  store <4 x i32> %105, ptr %103, align 4, !tbaa !33
  store <4 x i32> %106, ptr %107, align 4, !tbaa !33
  %108 = add nuw i64 %100, 8
  %109 = icmp eq i64 %108, %97
  br i1 %109, label %110, label %99, !llvm.loop !60

110:                                              ; preds = %99
  %111 = getelementptr i8, ptr %1, i64 %98
  %112 = getelementptr i8, ptr %85, i64 %98
  %113 = icmp eq i64 %90, %97
  br i1 %113, label %.loopexit24, label %.preheader38

.preheader38:                                     ; preds = %92, %87, %110
  %.ph39 = phi ptr [ %111, %110 ], [ %1, %87 ], [ %1, %92 ]
  %.ph40 = phi ptr [ %112, %110 ], [ %85, %87 ], [ %85, %92 ]
  br label %114

114:                                              ; preds = %.preheader38, %114
  %115 = phi ptr [ %118, %114 ], [ %.ph39, %.preheader38 ]
  %116 = phi ptr [ %119, %114 ], [ %.ph40, %.preheader38 ]
  %117 = load i32, ptr %115, align 4, !tbaa !33
  store i32 %117, ptr %116, align 4, !tbaa !33
  %118 = getelementptr inbounds i32, ptr %115, i64 1
  %119 = getelementptr inbounds i32, ptr %116, i64 1
  %120 = icmp eq ptr %118, %15
  br i1 %120, label %.loopexit24, label %114, !llvm.loop !61

.loopexit24:                                      ; preds = %114, %110, %82
  %121 = getelementptr inbounds i8, ptr %83, i64 %10
  store ptr %121, ptr %14, align 8, !tbaa !47
  %122 = icmp sgt i64 %21, 4
  br i1 %122, label %123, label %124, !prof !59

123:                                              ; preds = %.loopexit24
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %1, ptr align 4 %2, i64 %21, i1 false)
  br label %227

124:                                              ; preds = %.loopexit24
  %125 = icmp eq i64 %21, 4
  br i1 %125, label %126, label %227

126:                                              ; preds = %124
  %127 = load i32, ptr %2, align 4, !tbaa !33
  store i32 %127, ptr %1, align 4, !tbaa !33
  br label %227

128:                                              ; preds = %7
  %129 = load ptr, ptr %0, align 8, !tbaa !45
  %130 = ptrtoint ptr %129 to i64
  %131 = sub i64 %17, %130
  %132 = ashr exact i64 %131, 2
  %133 = icmp sgt i64 %132, -1
  tail call void @llvm.assume(i1 %133)
  %134 = sub nuw nsw i64 2305843009213693951, %132
  %135 = icmp ult i64 %134, %11
  br i1 %135, label %136, label %137

136:                                              ; preds = %128
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4.16) #23
  unreachable

137:                                              ; preds = %128
  %138 = tail call i64 @llvm.umax.i64(i64 %132, i64 %11)
  %139 = add nuw nsw i64 %138, %132
  %140 = tail call noundef i64 @llvm.umin.i64(i64 %139, i64 2305843009213693951)
  %141 = shl nuw nsw i64 %140, 2
  %142 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %141) #25
  %143 = icmp eq ptr %129, %1
  br i1 %143, label %.loopexit22, label %144

144:                                              ; preds = %137
  %145 = ptrtoint ptr %142 to i64
  %146 = add i64 %5, -4
  %147 = sub i64 %146, %130
  %148 = lshr i64 %147, 2
  %149 = add nuw nsw i64 %148, 1
  %150 = icmp ult i64 %147, 28
  %151 = sub i64 %145, %130
  %152 = icmp ult i64 %151, 32
  %153 = or i1 %150, %152
  br i1 %153, label %.preheader31, label %154

154:                                              ; preds = %144
  %155 = and i64 %149, 9223372036854775800
  %156 = shl i64 %155, 2
  br label %157

157:                                              ; preds = %157, %154
  %158 = phi i64 [ 0, %154 ], [ %166, %157 ]
  %159 = shl i64 %158, 2
  %160 = getelementptr i8, ptr %129, i64 %159
  %161 = getelementptr i8, ptr %142, i64 %159
  %162 = getelementptr i32, ptr %160, i64 4
  %163 = load <4 x i32>, ptr %160, align 4, !tbaa !33
  %164 = load <4 x i32>, ptr %162, align 4, !tbaa !33
  %165 = getelementptr i32, ptr %161, i64 4
  store <4 x i32> %163, ptr %161, align 4, !tbaa !33
  store <4 x i32> %164, ptr %165, align 4, !tbaa !33
  %166 = add nuw i64 %158, 8
  %167 = icmp eq i64 %166, %155
  br i1 %167, label %168, label %157, !llvm.loop !62

168:                                              ; preds = %157
  %169 = getelementptr i8, ptr %129, i64 %156
  %170 = getelementptr i8, ptr %142, i64 %156
  %171 = icmp eq i64 %149, %155
  br i1 %171, label %.loopexit22, label %.preheader31

.preheader31:                                     ; preds = %144, %168
  %.ph32 = phi ptr [ %169, %168 ], [ %129, %144 ]
  %.ph33 = phi ptr [ %170, %168 ], [ %142, %144 ]
  br label %172

172:                                              ; preds = %.preheader31, %172
  %173 = phi ptr [ %176, %172 ], [ %.ph32, %.preheader31 ]
  %174 = phi ptr [ %177, %172 ], [ %.ph33, %.preheader31 ]
  %175 = load i32, ptr %173, align 4, !tbaa !33
  store i32 %175, ptr %174, align 4, !tbaa !33
  %176 = getelementptr inbounds i32, ptr %173, i64 1
  %177 = getelementptr inbounds i32, ptr %174, i64 1
  %178 = icmp eq ptr %176, %1
  br i1 %178, label %.loopexit22, label %172, !llvm.loop !63

.loopexit22:                                      ; preds = %172, %168, %137
  %179 = phi ptr [ %142, %137 ], [ %170, %168 ], [ %177, %172 ]
  %180 = ptrtoint ptr %179 to i64
  %181 = icmp sgt i64 %10, 0
  br i1 %181, label %182, label %184, !prof !59

182:                                              ; preds = %.loopexit22
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %179, ptr align 4 %2, i64 %10, i1 false)
  %183 = getelementptr i8, ptr %179, i64 %10
  br label %184

184:                                              ; preds = %182, %.loopexit22
  %185 = phi ptr [ %183, %182 ], [ %179, %.loopexit22 ]
  %186 = icmp eq ptr %15, %1
  br i1 %186, label %.loopexit, label %187

187:                                              ; preds = %184
  %reass.sub26 = sub i64 %17, %5
  %188 = add i64 %reass.sub26, -4
  %189 = lshr i64 %188, 2
  %190 = add nuw nsw i64 %189, 1
  %191 = icmp ult i64 %188, 60
  br i1 %191, label %.preheader, label %192

192:                                              ; preds = %187
  %193 = tail call i64 @llvm.smax.i64(i64 %10, i64 0)
  %194 = sub i64 %193, %5
  %195 = add i64 %194, %180
  %196 = icmp ult i64 %195, 32
  br i1 %196, label %.preheader, label %197

197:                                              ; preds = %192
  %198 = and i64 %190, 9223372036854775800
  %199 = shl i64 %198, 2
  br label %200

200:                                              ; preds = %200, %197
  %201 = phi i64 [ 0, %197 ], [ %209, %200 ]
  %202 = shl i64 %201, 2
  %203 = getelementptr i8, ptr %1, i64 %202
  %204 = getelementptr i8, ptr %185, i64 %202
  %205 = getelementptr i32, ptr %203, i64 4
  %206 = load <4 x i32>, ptr %203, align 4, !tbaa !33
  %207 = load <4 x i32>, ptr %205, align 4, !tbaa !33
  %208 = getelementptr i32, ptr %204, i64 4
  store <4 x i32> %206, ptr %204, align 4, !tbaa !33
  store <4 x i32> %207, ptr %208, align 4, !tbaa !33
  %209 = add nuw i64 %201, 8
  %210 = icmp eq i64 %209, %198
  br i1 %210, label %211, label %200, !llvm.loop !64

211:                                              ; preds = %200
  %212 = getelementptr i8, ptr %1, i64 %199
  %213 = getelementptr i8, ptr %185, i64 %199
  %214 = icmp eq i64 %190, %198
  br i1 %214, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %192, %187, %211
  %.ph = phi ptr [ %212, %211 ], [ %1, %187 ], [ %1, %192 ]
  %.ph30 = phi ptr [ %213, %211 ], [ %185, %187 ], [ %185, %192 ]
  br label %215

215:                                              ; preds = %.preheader, %215
  %216 = phi ptr [ %219, %215 ], [ %.ph, %.preheader ]
  %217 = phi ptr [ %220, %215 ], [ %.ph30, %.preheader ]
  %218 = load i32, ptr %216, align 4, !tbaa !33
  store i32 %218, ptr %217, align 4, !tbaa !33
  %219 = getelementptr inbounds i32, ptr %216, i64 1
  %220 = getelementptr inbounds i32, ptr %217, i64 1
  %221 = icmp eq ptr %219, %15
  br i1 %221, label %.loopexit, label %215, !llvm.loop !65

.loopexit:                                        ; preds = %215, %211, %184
  %222 = phi ptr [ %185, %184 ], [ %213, %211 ], [ %220, %215 ]
  %223 = icmp eq ptr %129, null
  br i1 %223, label %225, label %224

224:                                              ; preds = %.loopexit
  tail call void @_ZdlPv(ptr noundef nonnull %129) #22
  br label %225

225:                                              ; preds = %.loopexit, %224
  store ptr %142, ptr %0, align 8, !tbaa !45
  store ptr %222, ptr %14, align 8, !tbaa !47
  %226 = getelementptr inbounds i32, ptr %142, i64 %140
  store ptr %226, ptr %12, align 8, !tbaa !48
  br label %227

227:                                              ; preds = %126, %124, %123, %73, %71, %70, %225, %4
  ret void
}

; Function Attrs: convergent nounwind
declare void @__kmpc_end_critical(ptr, i32, ptr) local_unnamed_addr #19

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #0

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #15

; Function Attrs: nounwind
declare i32 @__kmpc_master(ptr, i32) local_unnamed_addr #18

; Function Attrs: nounwind
declare void @__kmpc_end_master(ptr, i32) local_unnamed_addr #18

; Function Attrs: mustprogress uwtable
define dso_local void @intersect_sorted(ptr dead_on_unwind noalias nocapture writable writeonly sret(%"class.std::vector") align 8 %0, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %1, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %2) local_unnamed_addr #12 personality ptr @__gxx_personality_v0 {
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %0, i8 0, i64 24, i1 false)
  %4 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %1, i64 0, i32 1
  %5 = load ptr, ptr %4, align 8, !tbaa !47
  %6 = load ptr, ptr %1, align 8, !tbaa !45
  %7 = ptrtoint ptr %5 to i64
  %8 = ptrtoint ptr %6 to i64
  %9 = sub i64 %7, %8
  %10 = icmp sgt i64 %9, -1
  tail call void @llvm.assume(i1 %10)
  %11 = icmp eq ptr %5, %6
  br i1 %11, label %.loopexit, label %12

12:                                               ; preds = %3
  %13 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %2, i64 0, i32 1
  %14 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  %15 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  br label %16

16:                                               ; preds = %12, %102
  %17 = phi ptr [ %6, %12 ], [ %103, %102 ]
  %18 = phi ptr [ %5, %12 ], [ %104, %102 ]
  %19 = phi ptr [ %6, %12 ], [ %105, %102 ]
  %20 = phi ptr [ %5, %12 ], [ %106, %102 ]
  %21 = phi ptr [ %6, %12 ], [ %107, %102 ]
  %22 = phi ptr [ %5, %12 ], [ %108, %102 ]
  %23 = phi ptr [ null, %12 ], [ %109, %102 ]
  %24 = phi ptr [ null, %12 ], [ %110, %102 ]
  %25 = phi i64 [ 0, %12 ], [ %113, %102 ]
  %26 = phi i64 [ 0, %12 ], [ %112, %102 ]
  %27 = phi ptr [ null, %12 ], [ %111, %102 ]
  %28 = load ptr, ptr %13, align 8, !tbaa !47
  %29 = load ptr, ptr %2, align 8, !tbaa !45
  %30 = ptrtoint ptr %28 to i64
  %31 = ptrtoint ptr %29 to i64
  %32 = sub i64 %30, %31
  %33 = ashr exact i64 %32, 2
  %34 = icmp sgt i64 %33, -1
  tail call void @llvm.assume(i1 %34)
  %35 = icmp ult i64 %26, %33
  br i1 %35, label %36, label %.loopexit

36:                                               ; preds = %16
  %37 = getelementptr inbounds i32, ptr %21, i64 %25
  %38 = load i32, ptr %37, align 4, !tbaa !33
  %39 = getelementptr inbounds i32, ptr %29, i64 %26
  %40 = load i32, ptr %39, align 4, !tbaa !33
  %41 = icmp slt i32 %38, %40
  br i1 %41, label %42, label %44

42:                                               ; preds = %36
  %43 = add nuw nsw i64 %25, 1
  br label %102

44:                                               ; preds = %36
  %45 = icmp sgt i32 %38, %40
  br i1 %45, label %46, label %48

46:                                               ; preds = %44
  %47 = add nuw nsw i64 %26, 1
  br label %102

48:                                               ; preds = %44
  %49 = icmp eq ptr %24, %23
  br i1 %49, label %52, label %50

50:                                               ; preds = %48
  store i32 %38, ptr %24, align 4, !tbaa !33
  %51 = getelementptr inbounds i32, ptr %24, i64 1
  store ptr %51, ptr %14, align 8, !tbaa !47
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
  %.lcssa1 = phi ptr [ %27, %52 ]
  store ptr %.lcssa1, ptr %0, align 8, !tbaa !38
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
          to label %60 unwind label %94

60:                                               ; preds = %59
  unreachable

61:                                               ; preds = %52
  %62 = tail call i64 @llvm.umax.i64(i64 %56, i64 1)
  %63 = add nuw nsw i64 %62, %56
  %64 = tail call noundef i64 @llvm.umin.i64(i64 %63, i64 2305843009213693951)
  %65 = shl nuw nsw i64 %64, 2
  %66 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %65) #25
          to label %67 unwind label %92

67:                                               ; preds = %61
  %68 = getelementptr inbounds i8, ptr %66, i64 %55
  %69 = load i32, ptr %37, align 4, !tbaa !33
  store i32 %69, ptr %68, align 4, !tbaa !33
  %70 = icmp sgt i64 %55, 0
  br i1 %70, label %71, label %72

71:                                               ; preds = %67
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %66, ptr align 4 %27, i64 %55, i1 false)
  br label %72

72:                                               ; preds = %71, %67
  %73 = icmp eq ptr %27, null
  br i1 %73, label %77, label %74

74:                                               ; preds = %72
  tail call void @_ZdlPv(ptr noundef nonnull %27) #22
  %75 = load ptr, ptr %4, align 8, !tbaa !47
  %76 = load ptr, ptr %1, align 8, !tbaa !45
  br label %77

77:                                               ; preds = %74, %72
  %78 = phi ptr [ %76, %74 ], [ %17, %72 ]
  %79 = phi ptr [ %75, %74 ], [ %18, %72 ]
  %80 = getelementptr inbounds i32, ptr %68, i64 1
  store ptr %80, ptr %14, align 8, !tbaa !47
  %81 = getelementptr inbounds i32, ptr %66, i64 %64
  store ptr %81, ptr %15, align 8, !tbaa !48
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
  br label %102

92:                                               ; preds = %61
  %.lcssa2 = phi ptr [ %27, %61 ]
  %93 = landingpad { ptr, i32 }
          cleanup
  store ptr %.lcssa2, ptr %0, align 8, !tbaa !38
  br label %96

94:                                               ; preds = %59
  %95 = landingpad { ptr, i32 }
          cleanup
  br label %96

96:                                               ; preds = %94, %92
  %97 = phi ptr [ %.lcssa2, %92 ], [ %.lcssa1, %94 ]
  %98 = phi { ptr, i32 } [ %93, %92 ], [ %95, %94 ]
  %99 = icmp eq ptr %97, null
  br i1 %99, label %101, label %100

100:                                              ; preds = %96
  tail call void @_ZdlPv(ptr noundef nonnull %97) #22
  br label %101

101:                                              ; preds = %96, %100
  resume { ptr, i32 } %98

102:                                              ; preds = %46, %82, %42
  %103 = phi ptr [ %17, %42 ], [ %17, %46 ], [ %83, %82 ]
  %104 = phi ptr [ %18, %42 ], [ %18, %46 ], [ %84, %82 ]
  %105 = phi ptr [ %19, %42 ], [ %19, %46 ], [ %85, %82 ]
  %106 = phi ptr [ %20, %42 ], [ %20, %46 ], [ %86, %82 ]
  %107 = phi ptr [ %21, %42 ], [ %21, %46 ], [ %85, %82 ]
  %108 = phi ptr [ %22, %42 ], [ %22, %46 ], [ %86, %82 ]
  %109 = phi ptr [ %23, %42 ], [ %23, %46 ], [ %87, %82 ]
  %110 = phi ptr [ %24, %42 ], [ %24, %46 ], [ %88, %82 ]
  %111 = phi ptr [ %27, %42 ], [ %27, %46 ], [ %89, %82 ]
  %112 = phi i64 [ %26, %42 ], [ %47, %46 ], [ %91, %82 ]
  %113 = phi i64 [ %43, %42 ], [ %25, %46 ], [ %90, %82 ]
  %114 = ptrtoint ptr %108 to i64
  %115 = ptrtoint ptr %107 to i64
  %116 = sub i64 %114, %115
  %117 = ashr exact i64 %116, 2
  %118 = icmp sgt i64 %117, -1
  tail call void @llvm.assume(i1 %118)
  %119 = icmp ult i64 %113, %117
  br i1 %119, label %16, label %.loopexit, !llvm.loop !66

.loopexit:                                        ; preds = %102, %16, %3
  %120 = phi ptr [ null, %3 ], [ %27, %16 ], [ %111, %102 ]
  store ptr %120, ptr %0, align 8, !tbaa !38
  ret void
}

; Function Attrs: mustprogress uwtable
define dso_local void @bron_kerbosch_with_pivot(ptr nocapture noundef readonly %0, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %1, ptr nocapture noundef nonnull align 8 dereferenceable(24) %2, ptr nocapture noundef nonnull align 8 dereferenceable(24) %3) local_unnamed_addr #12 personality ptr @__gxx_personality_v0 {
  %5 = alloca %"class.std::vector", align 8
  %6 = alloca %"class.std::vector", align 8
  %7 = alloca %"class.std::vector", align 8
  %8 = alloca %"class.std::vector", align 8
  %9 = alloca %"class.std::vector", align 8
  %10 = alloca %"class.std::vector", align 8
  %11 = load ptr, ptr %2, align 8, !tbaa !38
  %12 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %2, i64 0, i32 1
  %13 = load ptr, ptr %12, align 8, !tbaa !38
  %14 = icmp eq ptr %11, %13
  br i1 %14, label %15, label %22

15:                                               ; preds = %4
  %16 = load ptr, ptr %3, align 8, !tbaa !38
  %17 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  %18 = load ptr, ptr %17, align 8, !tbaa !38
  %19 = icmp eq ptr %16, %18
  br i1 %19, label %345, label %20

20:                                               ; preds = %15
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %5) #18
  %21 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %5, i64 0, i32 1
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %5, i8 0, i64 24, i1 false)
  br label %31

22:                                               ; preds = %4
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %5) #18
  %23 = ptrtoint ptr %13 to i64
  %24 = ptrtoint ptr %11 to i64
  %25 = sub i64 %23, %24
  %26 = icmp sgt i64 %25, -1
  tail call void @llvm.assume(i1 %26)
  %27 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %25) #25
  store ptr %27, ptr %5, align 8, !tbaa !45
  %28 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %5, i64 0, i32 1
  store ptr %27, ptr %28, align 8, !tbaa !47
  %29 = getelementptr inbounds i8, ptr %27, i64 %25
  %30 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %5, i64 0, i32 2
  store ptr %29, ptr %30, align 8, !tbaa !48
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %27, ptr align 4 %11, i64 %25, i1 false)
  %.pre = load ptr, ptr %3, align 8, !tbaa !38
  %.phi.trans.insert = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  %.pre92 = load ptr, ptr %.phi.trans.insert, align 8, !tbaa !38
  br label %31

31:                                               ; preds = %20, %22
  %32 = phi ptr [ %.pre92, %22 ], [ %18, %20 ]
  %33 = phi ptr [ %.pre, %22 ], [ %16, %20 ]
  %34 = phi ptr [ %28, %22 ], [ %21, %20 ]
  %35 = phi ptr [ %27, %22 ], [ null, %20 ]
  %36 = phi ptr [ %29, %22 ], [ null, %20 ]
  store ptr %36, ptr %34, align 8, !tbaa !47
  %37 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  %38 = ptrtoint ptr %36 to i64
  %39 = ptrtoint ptr %35 to i64
  %40 = sub i64 %38, %39
  %41 = getelementptr inbounds i8, ptr %35, i64 %40
  invoke void @_ZNSt6vectorIiSaIiEE15_M_range_insertIN9__gnu_cxx17__normal_iteratorIPiS1_EEEEvS6_T_S7_St20forward_iterator_tag(ptr noundef nonnull align 8 dereferenceable(24) %5, ptr %41, ptr %33, ptr %32)
          to label %42 unwind label %72

42:                                               ; preds = %31
  %43 = load ptr, ptr %5, align 8, !tbaa !38
  %44 = load ptr, ptr %34, align 8, !tbaa !38
  %45 = icmp eq ptr %43, %44
  %46 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %47 = load ptr, ptr %46, align 8, !tbaa !29
  br i1 %45, label %.loopexit49, label %.preheader48

.loopexit49:                                      ; preds = %.preheader48, %42
  %48 = phi i32 [ -1, %42 ], [ %86, %.preheader48 ]
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds i64, ptr %47, i64 %49
  %51 = load i64, ptr %50, align 8, !tbaa !36
  %52 = getelementptr i64, ptr %50, i64 1
  %53 = load i64, ptr %52, align 8, !tbaa !36
  %54 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %55 = load ptr, ptr %54, align 8, !tbaa !30
  %56 = getelementptr inbounds i32, ptr %55, i64 %51
  %57 = getelementptr inbounds i32, ptr %55, i64 %53
  %58 = ptrtoint ptr %57 to i64
  %59 = ptrtoint ptr %56 to i64
  %60 = sub i64 %58, %59
  %61 = icmp ugt i64 %60, 9223372036854775804
  br i1 %61, label %62, label %64

62:                                               ; preds = %.loopexit49
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2.19) #23
          to label %63 unwind label %70

63:                                               ; preds = %62
  unreachable

64:                                               ; preds = %.loopexit49
  %65 = icmp eq i64 %53, %51
  br i1 %65, label %89, label %66

66:                                               ; preds = %64
  %67 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %60) #25
          to label %68 unwind label %70

68:                                               ; preds = %66
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %67, ptr align 4 %56, i64 %60, i1 false)
  %69 = getelementptr inbounds i8, ptr %67, i64 %60
  br label %89

70:                                               ; preds = %62, %66
  %71 = landingpad { ptr, i32 }
          cleanup
  br label %354

72:                                               ; preds = %31
  %73 = landingpad { ptr, i32 }
          cleanup
  br label %354

.preheader48:                                     ; preds = %42, %.preheader48
  %74 = phi i32 [ %86, %.preheader48 ], [ -1, %42 ]
  %75 = phi i64 [ %85, %.preheader48 ], [ -1, %42 ]
  %76 = phi ptr [ %87, %.preheader48 ], [ %43, %42 ]
  %77 = load i32, ptr %76, align 4, !tbaa !33
  %78 = sext i32 %77 to i64
  %79 = getelementptr i64, ptr %47, i64 %78
  %80 = getelementptr i64, ptr %79, i64 1
  %81 = load i64, ptr %80, align 8, !tbaa !36
  %82 = load i64, ptr %79, align 8, !tbaa !36
  %83 = sub nsw i64 %81, %82
  %84 = icmp sgt i64 %83, %75
  %85 = call i64 @llvm.smax.i64(i64 %83, i64 %75)
  %86 = select i1 %84, i32 %77, i32 %74
  %87 = getelementptr inbounds i32, ptr %76, i64 1
  %88 = icmp eq ptr %87, %44
  br i1 %88, label %.loopexit49, label %.preheader48

89:                                               ; preds = %64, %68
  %90 = phi ptr [ %67, %68 ], [ null, %64 ]
  %91 = phi ptr [ %69, %68 ], [ null, %64 ]
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %6) #18
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %6, i8 0, i64 24, i1 false)
  %92 = load ptr, ptr %2, align 8, !tbaa !38
  %93 = load ptr, ptr %12, align 8, !tbaa !38
  %94 = invoke ptr @_ZSt16__set_differenceIN9__gnu_cxx17__normal_iteratorIPiSt6vectorIiSaIiEEEES6_St20back_insert_iteratorIS5_ENS0_5__ops15_Iter_less_iterEET1_T_SC_T0_SD_SB_T2_(ptr %92, ptr %93, ptr %90, ptr %91, ptr nonnull %6)
          to label %95 unwind label %121

95:                                               ; preds = %89
  %96 = load ptr, ptr %6, align 8, !tbaa !38
  %97 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %6, i64 0, i32 1
  %98 = load ptr, ptr %97, align 8, !tbaa !38
  %99 = icmp eq ptr %96, %98
  br i1 %99, label %109, label %100

100:                                              ; preds = %95
  %101 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i64 0, i32 1
  %102 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %7, i64 0, i32 2
  %103 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %1, i64 0, i32 1
  %104 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i64 0, i32 1
  %105 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %8, i64 0, i32 2
  %106 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 2
  br label %123

107:                                              ; preds = %297
  %108 = load ptr, ptr %6, align 8, !tbaa !45
  br label %109

109:                                              ; preds = %107, %95
  %110 = phi ptr [ %108, %107 ], [ %96, %95 ]
  %111 = icmp eq ptr %110, null
  br i1 %111, label %113, label %112

112:                                              ; preds = %109
  call void @_ZdlPv(ptr noundef nonnull %110) #22
  br label %113

113:                                              ; preds = %109, %112
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #18
  %114 = icmp eq ptr %90, null
  br i1 %114, label %116, label %115

115:                                              ; preds = %113
  call void @_ZdlPv(ptr noundef nonnull %90) #22
  br label %116

116:                                              ; preds = %113, %115
  %117 = load ptr, ptr %5, align 8, !tbaa !45
  %118 = icmp eq ptr %117, null
  br i1 %118, label %120, label %119

119:                                              ; preds = %116
  call void @_ZdlPv(ptr noundef nonnull %117) #22
  br label %120

120:                                              ; preds = %116, %119
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #18
  br label %345

121:                                              ; preds = %89
  %122 = landingpad { ptr, i32 }
          cleanup
  br label %346

123:                                              ; preds = %100, %297
  %124 = phi ptr [ %96, %100 ], [ %298, %297 ]
  %125 = load i32, ptr %124, align 4, !tbaa !33
  %126 = load ptr, ptr %46, align 8, !tbaa !29
  %127 = sext i32 %125 to i64
  %128 = getelementptr inbounds i64, ptr %126, i64 %127
  %129 = load i64, ptr %128, align 8, !tbaa !36
  %130 = getelementptr i64, ptr %128, i64 1
  %131 = load i64, ptr %130, align 8, !tbaa !36
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #18
  %132 = load ptr, ptr %54, align 8, !tbaa !30
  %133 = getelementptr inbounds i32, ptr %132, i64 %129
  %134 = getelementptr inbounds i32, ptr %132, i64 %131
  %135 = ptrtoint ptr %134 to i64
  %136 = ptrtoint ptr %133 to i64
  %137 = sub i64 %135, %136
  %138 = ashr exact i64 %137, 2
  %139 = icmp ugt i64 %138, 2305843009213693951
  br i1 %139, label %140, label %142

140:                                              ; preds = %123
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2.19) #23
          to label %141 unwind label %154

141:                                              ; preds = %140
  unreachable

142:                                              ; preds = %123
  %143 = icmp eq i64 %131, %129
  br i1 %143, label %146, label %144

144:                                              ; preds = %142
  %145 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %137) #25
          to label %146 unwind label %152

146:                                              ; preds = %144, %142
  %147 = phi ptr [ null, %142 ], [ %145, %144 ]
  store ptr %147, ptr %7, align 8, !tbaa !45
  %148 = getelementptr inbounds i32, ptr %147, i64 %138
  store ptr %148, ptr %102, align 8, !tbaa !48
  %149 = icmp sgt i64 %137, 0
  br i1 %149, label %150, label %156, !prof !59

150:                                              ; preds = %146
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %147, ptr align 4 %133, i64 %137, i1 false)
  %151 = getelementptr inbounds i8, ptr %147, i64 %137
  br label %156

152:                                              ; preds = %144
  %153 = landingpad { ptr, i32 }
          cleanup
  br label %343

154:                                              ; preds = %140
  %155 = landingpad { ptr, i32 }
          cleanup
  br label %343

156:                                              ; preds = %150, %146
  %157 = phi ptr [ %151, %150 ], [ %147, %146 ]
  store ptr %157, ptr %101, align 8, !tbaa !47
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #18
  %158 = load ptr, ptr %103, align 8, !tbaa !38
  %159 = load ptr, ptr %1, align 8, !tbaa !38
  %160 = ptrtoint ptr %158 to i64
  %161 = ptrtoint ptr %159 to i64
  %162 = sub i64 %160, %161
  %163 = ashr exact i64 %162, 2
  %164 = icmp sgt i64 %163, -1
  call void @llvm.assume(i1 %164)
  %165 = icmp eq ptr %158, %159
  br i1 %165, label %168, label %166

166:                                              ; preds = %156
  %167 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %162) #25
          to label %168 unwind label %300

168:                                              ; preds = %166, %156
  %169 = phi ptr [ null, %156 ], [ %167, %166 ]
  store ptr %169, ptr %8, align 8, !tbaa !45
  %170 = getelementptr inbounds i32, ptr %169, i64 %163
  store ptr %170, ptr %105, align 8, !tbaa !48
  %171 = icmp sgt i64 %162, 0
  br i1 %171, label %172, label %174, !prof !59

172:                                              ; preds = %168
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %169, ptr align 4 %159, i64 %162, i1 false)
  %173 = getelementptr inbounds i8, ptr %169, i64 %162
  br label %174

174:                                              ; preds = %172, %168
  %175 = phi ptr [ %173, %172 ], [ %169, %168 ]
  store ptr %175, ptr %104, align 8, !tbaa !47
  %176 = icmp eq ptr %175, %170
  br i1 %176, label %179, label %177

177:                                              ; preds = %174
  store i32 %125, ptr %175, align 4, !tbaa !33
  %178 = getelementptr inbounds i32, ptr %175, i64 1
  store ptr %178, ptr %104, align 8, !tbaa !47
  br label %198

179:                                              ; preds = %174
  %180 = icmp eq i64 %162, 9223372036854775804
  br i1 %180, label %181, label %183

181:                                              ; preds = %179
  %.lcssa9 = phi ptr [ %169, %179 ]
  %.lcssa2 = phi ptr [ %147, %179 ]
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
          to label %182 unwind label %304

182:                                              ; preds = %181
  unreachable

183:                                              ; preds = %179
  %184 = call i64 @llvm.umax.i64(i64 %163, i64 1)
  %185 = add nuw nsw i64 %184, %163
  %186 = call noundef i64 @llvm.umin.i64(i64 %185, i64 2305843009213693951)
  %187 = shl nuw nsw i64 %186, 2
  %188 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %187) #25
          to label %189 unwind label %302

189:                                              ; preds = %183
  %190 = getelementptr inbounds i8, ptr %188, i64 %162
  store i32 %125, ptr %190, align 4, !tbaa !33
  br i1 %171, label %191, label %192

191:                                              ; preds = %189
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %188, ptr align 4 %169, i64 %162, i1 false)
  br label %192

192:                                              ; preds = %191, %189
  %193 = icmp eq ptr %169, null
  br i1 %193, label %195, label %194

194:                                              ; preds = %192
  call void @_ZdlPv(ptr noundef nonnull %169) #22
  br label %195

195:                                              ; preds = %194, %192
  %196 = getelementptr inbounds i32, ptr %190, i64 1
  store ptr %188, ptr %8, align 8, !tbaa !45
  store ptr %196, ptr %104, align 8, !tbaa !47
  %197 = getelementptr inbounds i32, ptr %188, i64 %186
  store ptr %197, ptr %105, align 8, !tbaa !48
  br label %198

198:                                              ; preds = %195, %177
  %199 = phi ptr [ %188, %195 ], [ %169, %177 ]
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %9) #18
  invoke void @intersect_sorted(ptr dead_on_unwind nonnull writable sret(%"class.std::vector") align 8 %9, ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef nonnull align 8 dereferenceable(24) %7)
          to label %200 unwind label %306

200:                                              ; preds = %198
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %10) #18
  invoke void @intersect_sorted(ptr dead_on_unwind nonnull writable sret(%"class.std::vector") align 8 %10, ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef nonnull align 8 dereferenceable(24) %7)
          to label %201 unwind label %308

201:                                              ; preds = %200
  invoke void @bron_kerbosch_with_pivot(ptr noundef nonnull %0, ptr noundef nonnull align 8 dereferenceable(24) %8, ptr noundef nonnull align 8 dereferenceable(24) %9, ptr noundef nonnull align 8 dereferenceable(24) %10)
          to label %202 unwind label %310

202:                                              ; preds = %201
  %203 = load ptr, ptr %2, align 8, !tbaa !38
  %204 = load ptr, ptr %12, align 8, !tbaa !38
  %205 = icmp eq ptr %203, %204
  br i1 %205, label %.loopexit46, label %.preheader45

.preheader45:                                     ; preds = %202, %227
  %206 = phi ptr [ %228, %227 ], [ %203, %202 ]
  %207 = load i32, ptr %206, align 4, !tbaa !33
  %208 = icmp eq i32 %207, %125
  br i1 %208, label %.loopexit46, label %209

209:                                              ; preds = %.preheader45
  %210 = getelementptr inbounds i32, ptr %206, i64 1
  %211 = icmp eq ptr %210, %204
  br i1 %211, label %.loopexit47, label %212, !llvm.loop !67

212:                                              ; preds = %209
  %213 = load i32, ptr %210, align 4, !tbaa !33
  %214 = icmp eq i32 %213, %125
  br i1 %214, label %.loopexit46, label %215

215:                                              ; preds = %212
  %216 = getelementptr inbounds i32, ptr %206, i64 2
  %217 = icmp eq ptr %216, %204
  br i1 %217, label %.loopexit47, label %218, !llvm.loop !67

218:                                              ; preds = %215
  %219 = load i32, ptr %216, align 4, !tbaa !33
  %220 = icmp eq i32 %219, %125
  br i1 %220, label %.loopexit46, label %221

221:                                              ; preds = %218
  %222 = getelementptr inbounds i32, ptr %206, i64 3
  %223 = icmp eq ptr %222, %204
  br i1 %223, label %.loopexit47, label %224, !llvm.loop !67

224:                                              ; preds = %221
  %225 = load i32, ptr %222, align 4, !tbaa !33
  %226 = icmp eq i32 %225, %125
  br i1 %226, label %.loopexit46, label %227

227:                                              ; preds = %224
  %228 = getelementptr inbounds i32, ptr %206, i64 4
  %229 = icmp eq ptr %228, %204
  br i1 %229, label %.loopexit47, label %.preheader45, !llvm.loop !69

.loopexit46:                                      ; preds = %224, %218, %212, %.preheader45, %202
  %230 = phi ptr [ %203, %202 ], [ %206, %.preheader45 ], [ %210, %212 ], [ %216, %218 ], [ %222, %224 ]
  %231 = icmp eq ptr %230, %204
  %232 = getelementptr inbounds i32, ptr %230, i64 1
  %233 = icmp eq ptr %232, %204
  %234 = select i1 %231, i1 true, i1 %233
  br i1 %234, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %.loopexit46, %241
  %235 = phi ptr [ %243, %241 ], [ %232, %.loopexit46 ]
  %236 = phi ptr [ %242, %241 ], [ %230, %.loopexit46 ]
  %237 = load i32, ptr %235, align 4, !tbaa !33
  %238 = icmp eq i32 %237, %125
  br i1 %238, label %241, label %239

239:                                              ; preds = %.preheader
  store i32 %237, ptr %236, align 4, !tbaa !33
  %240 = getelementptr inbounds i32, ptr %236, i64 1
  br label %241

241:                                              ; preds = %239, %.preheader
  %242 = phi ptr [ %236, %.preheader ], [ %240, %239 ]
  %243 = getelementptr inbounds i32, ptr %235, i64 1
  %244 = icmp eq ptr %243, %204
  br i1 %244, label %.loopexit, label %.preheader, !llvm.loop !71

.loopexit:                                        ; preds = %241, %.loopexit46
  %245 = phi ptr [ %230, %.loopexit46 ], [ %242, %241 ]
  %246 = icmp eq ptr %245, %204
  br i1 %246, label %.loopexit47, label %247

247:                                              ; preds = %.loopexit
  %248 = ptrtoint ptr %245 to i64
  %249 = ptrtoint ptr %203 to i64
  %250 = sub i64 %248, %249
  %251 = getelementptr inbounds i8, ptr %203, i64 %250
  store ptr %251, ptr %12, align 8, !tbaa !47
  br label %.loopexit47

.loopexit47:                                      ; preds = %209, %215, %221, %227, %247, %.loopexit
  %252 = load ptr, ptr %37, align 8, !tbaa !38
  %253 = load ptr, ptr %106, align 8, !tbaa !48
  %254 = icmp eq ptr %252, %253
  br i1 %254, label %257, label %255

255:                                              ; preds = %.loopexit47
  store i32 %125, ptr %252, align 4, !tbaa !33
  %256 = getelementptr inbounds i32, ptr %252, i64 1
  store ptr %256, ptr %37, align 8, !tbaa !47
  br label %283

257:                                              ; preds = %.loopexit47
  %258 = load ptr, ptr %3, align 8, !tbaa !38
  %259 = ptrtoint ptr %252 to i64
  %260 = ptrtoint ptr %258 to i64
  %261 = sub i64 %259, %260
  %262 = ashr exact i64 %261, 2
  %263 = icmp sgt i64 %262, -1
  call void @llvm.assume(i1 %263)
  %264 = icmp eq i64 %261, 9223372036854775804
  br i1 %264, label %265, label %267

265:                                              ; preds = %257
  %.lcssa19 = phi ptr [ %199, %257 ]
  %.lcssa7 = phi ptr [ %147, %257 ]
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
          to label %266 unwind label %312

266:                                              ; preds = %265
  unreachable

267:                                              ; preds = %257
  %268 = call i64 @llvm.umax.i64(i64 %262, i64 1)
  %269 = add nuw nsw i64 %268, %262
  %270 = call noundef i64 @llvm.umin.i64(i64 %269, i64 2305843009213693951)
  %271 = shl nuw nsw i64 %270, 2
  %272 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %271) #25
          to label %273 unwind label %310

273:                                              ; preds = %267
  %274 = getelementptr inbounds i8, ptr %272, i64 %261
  store i32 %125, ptr %274, align 4, !tbaa !33
  %275 = icmp sgt i64 %261, 0
  br i1 %275, label %276, label %277

276:                                              ; preds = %273
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %272, ptr align 4 %258, i64 %261, i1 false)
  br label %277

277:                                              ; preds = %276, %273
  %278 = icmp eq ptr %258, null
  br i1 %278, label %280, label %279

279:                                              ; preds = %277
  call void @_ZdlPv(ptr noundef nonnull %258) #22
  br label %280

280:                                              ; preds = %279, %277
  %281 = getelementptr inbounds i32, ptr %274, i64 1
  store ptr %272, ptr %3, align 8, !tbaa !45
  store ptr %281, ptr %37, align 8, !tbaa !47
  %282 = getelementptr inbounds i32, ptr %272, i64 %270
  store ptr %282, ptr %106, align 8, !tbaa !48
  br label %283

283:                                              ; preds = %280, %255
  %284 = load ptr, ptr %10, align 8, !tbaa !45
  %285 = icmp eq ptr %284, null
  br i1 %285, label %287, label %286

286:                                              ; preds = %283
  call void @_ZdlPv(ptr noundef nonnull %284) #22
  br label %287

287:                                              ; preds = %283, %286
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %10) #18
  %288 = load ptr, ptr %9, align 8, !tbaa !45
  %289 = icmp eq ptr %288, null
  br i1 %289, label %291, label %290

290:                                              ; preds = %287
  call void @_ZdlPv(ptr noundef nonnull %288) #22
  br label %291

291:                                              ; preds = %287, %290
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %9) #18
  %292 = icmp eq ptr %199, null
  br i1 %292, label %294, label %293

293:                                              ; preds = %291
  call void @_ZdlPv(ptr noundef nonnull %199) #22
  br label %294

294:                                              ; preds = %291, %293
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #18
  %295 = icmp eq ptr %147, null
  br i1 %295, label %297, label %296

296:                                              ; preds = %294
  call void @_ZdlPv(ptr noundef nonnull %147) #22
  br label %297

297:                                              ; preds = %294, %296
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #18
  %298 = getelementptr inbounds i32, ptr %124, i64 1
  %299 = icmp eq ptr %298, %98
  br i1 %299, label %107, label %123

300:                                              ; preds = %166
  %.lcssa1 = phi ptr [ %147, %166 ]
  %301 = landingpad { ptr, i32 }
          cleanup
  br label %338

302:                                              ; preds = %183
  %.lcssa10 = phi ptr [ %169, %183 ]
  %.lcssa3 = phi ptr [ %147, %183 ]
  %303 = landingpad { ptr, i32 }
          cleanup
  br label %332

304:                                              ; preds = %181
  %305 = landingpad { ptr, i32 }
          cleanup
  br label %332

306:                                              ; preds = %198
  %.lcssa16 = phi ptr [ %199, %198 ]
  %.lcssa4 = phi ptr [ %147, %198 ]
  %307 = landingpad { ptr, i32 }
          cleanup
  br label %328

308:                                              ; preds = %200
  %.lcssa17 = phi ptr [ %199, %200 ]
  %.lcssa5 = phi ptr [ %147, %200 ]
  %309 = landingpad { ptr, i32 }
          cleanup
  br label %321

310:                                              ; preds = %201, %267
  %.lcssa18 = phi ptr [ %199, %201 ], [ %199, %267 ]
  %.lcssa6 = phi ptr [ %147, %201 ], [ %147, %267 ]
  %311 = landingpad { ptr, i32 }
          cleanup
  br label %314

312:                                              ; preds = %265
  %313 = landingpad { ptr, i32 }
          cleanup
  br label %314

314:                                              ; preds = %312, %310
  %315 = phi ptr [ %.lcssa18, %310 ], [ %.lcssa19, %312 ]
  %316 = phi ptr [ %.lcssa6, %310 ], [ %.lcssa7, %312 ]
  %317 = phi { ptr, i32 } [ %311, %310 ], [ %313, %312 ]
  %318 = load ptr, ptr %10, align 8, !tbaa !45
  %319 = icmp eq ptr %318, null
  br i1 %319, label %321, label %320

320:                                              ; preds = %314
  call void @_ZdlPv(ptr noundef nonnull %318) #22
  br label %321

321:                                              ; preds = %320, %314, %308
  %322 = phi ptr [ %.lcssa17, %308 ], [ %315, %314 ], [ %315, %320 ]
  %323 = phi ptr [ %.lcssa5, %308 ], [ %316, %314 ], [ %316, %320 ]
  %324 = phi { ptr, i32 } [ %309, %308 ], [ %317, %314 ], [ %317, %320 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %10) #18
  %325 = load ptr, ptr %9, align 8, !tbaa !45
  %326 = icmp eq ptr %325, null
  br i1 %326, label %328, label %327

327:                                              ; preds = %321
  call void @_ZdlPv(ptr noundef nonnull %325) #22
  br label %328

328:                                              ; preds = %327, %321, %306
  %329 = phi ptr [ %.lcssa16, %306 ], [ %322, %321 ], [ %322, %327 ]
  %330 = phi ptr [ %.lcssa4, %306 ], [ %323, %321 ], [ %323, %327 ]
  %331 = phi { ptr, i32 } [ %307, %306 ], [ %324, %321 ], [ %324, %327 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %9) #18
  br label %332

332:                                              ; preds = %302, %304, %328
  %333 = phi ptr [ %330, %328 ], [ %.lcssa3, %302 ], [ %.lcssa2, %304 ]
  %334 = phi ptr [ %329, %328 ], [ %.lcssa10, %302 ], [ %.lcssa9, %304 ]
  %335 = phi { ptr, i32 } [ %331, %328 ], [ %303, %302 ], [ %305, %304 ]
  %336 = icmp eq ptr %334, null
  br i1 %336, label %338, label %337

337:                                              ; preds = %332
  call void @_ZdlPv(ptr noundef nonnull %334) #22
  br label %338

338:                                              ; preds = %337, %332, %300
  %339 = phi ptr [ %.lcssa1, %300 ], [ %333, %332 ], [ %333, %337 ]
  %340 = phi { ptr, i32 } [ %301, %300 ], [ %335, %332 ], [ %335, %337 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #18
  %341 = icmp eq ptr %339, null
  br i1 %341, label %343, label %342

342:                                              ; preds = %338
  call void @_ZdlPv(ptr noundef nonnull %339) #22
  br label %343

343:                                              ; preds = %152, %154, %342, %338
  %344 = phi { ptr, i32 } [ %340, %338 ], [ %340, %342 ], [ %153, %152 ], [ %155, %154 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #18
  br label %346

345:                                              ; preds = %15, %120
  ret void

346:                                              ; preds = %343, %121
  %347 = phi { ptr, i32 } [ %344, %343 ], [ %122, %121 ]
  %348 = load ptr, ptr %6, align 8, !tbaa !45
  %349 = icmp eq ptr %348, null
  br i1 %349, label %351, label %350

350:                                              ; preds = %346
  call void @_ZdlPv(ptr noundef nonnull %348) #22
  br label %351

351:                                              ; preds = %346, %350
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #18
  %352 = icmp eq ptr %90, null
  br i1 %352, label %354, label %353

353:                                              ; preds = %351
  call void @_ZdlPv(ptr noundef nonnull %90) #22
  br label %354

354:                                              ; preds = %70, %351, %353, %72
  %355 = phi { ptr, i32 } [ %73, %72 ], [ %71, %70 ], [ %347, %351 ], [ %347, %353 ]
  %356 = load ptr, ptr %5, align 8, !tbaa !45
  %357 = icmp eq ptr %356, null
  br i1 %357, label %359, label %358

358:                                              ; preds = %354
  call void @_ZdlPv(ptr noundef nonnull %356) #22
  br label %359

359:                                              ; preds = %354, %358
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %5) #18
  resume { ptr, i32 } %355
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local ptr @_ZSt16__set_differenceIN9__gnu_cxx17__normal_iteratorIPiSt6vectorIiSaIiEEEES6_St20back_insert_iteratorIS5_ENS0_5__ops15_Iter_less_iterEET1_T_SC_T0_SD_SB_T2_(ptr %0, ptr %1, ptr %2, ptr %3, ptr %4) local_unnamed_addr #12 comdat personality ptr @__gxx_personality_v0 {
  %6 = icmp ne ptr %0, %1
  %7 = icmp ne ptr %2, %3
  %8 = select i1 %6, i1 %7, i1 false
  br i1 %8, label %9, label %.loopexit20

9:                                                ; preds = %5
  %10 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  %11 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 2
  br label %12

12:                                               ; preds = %9, %58
  %13 = phi ptr [ %0, %9 ], [ %60, %58 ]
  %14 = phi ptr [ %2, %9 ], [ %59, %58 ]
  %15 = load i32, ptr %13, align 4, !tbaa !33
  %16 = load i32, ptr %14, align 4, !tbaa !33
  %17 = icmp slt i32 %15, %16
  br i1 %17, label %18, label %51

18:                                               ; preds = %12
  %19 = load ptr, ptr %10, align 8, !tbaa !38
  %20 = load ptr, ptr %11, align 8, !tbaa !48
  %21 = icmp eq ptr %19, %20
  br i1 %21, label %24, label %22

22:                                               ; preds = %18
  store i32 %15, ptr %19, align 4, !tbaa !33
  %23 = getelementptr inbounds i32, ptr %19, i64 1
  store ptr %23, ptr %10, align 8, !tbaa !47
  br label %49

24:                                               ; preds = %18
  %25 = load ptr, ptr %4, align 8, !tbaa !38
  %26 = ptrtoint ptr %19 to i64
  %27 = ptrtoint ptr %25 to i64
  %28 = sub i64 %26, %27
  %29 = ashr exact i64 %28, 2
  %30 = icmp sgt i64 %29, -1
  tail call void @llvm.assume(i1 %30)
  %31 = icmp eq i64 %28, 9223372036854775804
  br i1 %31, label %32, label %33

32:                                               ; preds = %24
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
  unreachable

33:                                               ; preds = %24
  %34 = tail call i64 @llvm.umax.i64(i64 %29, i64 1)
  %35 = add nuw nsw i64 %34, %29
  %36 = tail call noundef i64 @llvm.umin.i64(i64 %35, i64 2305843009213693951)
  %37 = shl nuw nsw i64 %36, 2
  %38 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %37) #25
  %39 = getelementptr inbounds i8, ptr %38, i64 %28
  %40 = load i32, ptr %13, align 4, !tbaa !33
  store i32 %40, ptr %39, align 4, !tbaa !33
  %41 = icmp sgt i64 %28, 0
  br i1 %41, label %42, label %43

42:                                               ; preds = %33
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %38, ptr align 4 %25, i64 %28, i1 false)
  br label %43

43:                                               ; preds = %42, %33
  %44 = icmp eq ptr %25, null
  br i1 %44, label %46, label %45

45:                                               ; preds = %43
  tail call void @_ZdlPv(ptr noundef nonnull %25) #22
  br label %46

46:                                               ; preds = %45, %43
  %47 = getelementptr inbounds i32, ptr %39, i64 1
  store ptr %38, ptr %4, align 8, !tbaa !45
  store ptr %47, ptr %10, align 8, !tbaa !47
  %48 = getelementptr inbounds i32, ptr %38, i64 %36
  store ptr %48, ptr %11, align 8, !tbaa !48
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
  br i1 %63, label %12, label %.loopexit20, !llvm.loop !72

.loopexit20:                                      ; preds = %58, %5
  %64 = phi ptr [ %0, %5 ], [ %60, %58 ]
  %65 = icmp eq ptr %64, %1
  br i1 %65, label %.loopexit, label %66

66:                                               ; preds = %.loopexit20
  %67 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 1
  %68 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %4, i64 0, i32 2
  %69 = load ptr, ptr %67, align 8, !tbaa !38
  %70 = load ptr, ptr %68, align 8, !tbaa !48
  br label %71

71:                                               ; preds = %104, %66
  %72 = phi ptr [ %70, %66 ], [ %105, %104 ]
  %73 = phi ptr [ %69, %66 ], [ %106, %104 ]
  %74 = phi ptr [ %64, %66 ], [ %107, %104 ]
  %75 = icmp eq ptr %73, %72
  br i1 %75, label %79, label %76

76:                                               ; preds = %71
  %77 = load i32, ptr %74, align 4, !tbaa !33
  store i32 %77, ptr %73, align 4, !tbaa !33
  %78 = getelementptr inbounds i32, ptr %73, i64 1
  store ptr %78, ptr %67, align 8, !tbaa !47
  br label %104

79:                                               ; preds = %71
  %80 = load ptr, ptr %4, align 8, !tbaa !38
  %81 = ptrtoint ptr %72 to i64
  %82 = ptrtoint ptr %80 to i64
  %83 = sub i64 %81, %82
  %84 = ashr exact i64 %83, 2
  %85 = icmp sgt i64 %84, -1
  tail call void @llvm.assume(i1 %85)
  %86 = icmp eq i64 %83, 9223372036854775804
  br i1 %86, label %87, label %88

87:                                               ; preds = %79
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.18) #23
  unreachable

88:                                               ; preds = %79
  %89 = tail call i64 @llvm.umax.i64(i64 %84, i64 1)
  %90 = add nuw nsw i64 %89, %84
  %91 = tail call noundef i64 @llvm.umin.i64(i64 %90, i64 2305843009213693951)
  %92 = shl nuw nsw i64 %91, 2
  %93 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %92) #25
  %94 = getelementptr inbounds i8, ptr %93, i64 %83
  %95 = load i32, ptr %74, align 4, !tbaa !33
  store i32 %95, ptr %94, align 4, !tbaa !33
  %96 = icmp sgt i64 %83, 0
  br i1 %96, label %97, label %98

97:                                               ; preds = %88
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %93, ptr align 4 %80, i64 %83, i1 false)
  br label %98

98:                                               ; preds = %97, %88
  %99 = icmp eq ptr %80, null
  br i1 %99, label %101, label %100

100:                                              ; preds = %98
  tail call void @_ZdlPv(ptr noundef nonnull %80) #22
  br label %101

101:                                              ; preds = %100, %98
  %102 = getelementptr inbounds i32, ptr %94, i64 1
  store ptr %93, ptr %4, align 8, !tbaa !45
  store ptr %102, ptr %67, align 8, !tbaa !47
  %103 = getelementptr inbounds i32, ptr %93, i64 %91
  store ptr %103, ptr %68, align 8, !tbaa !48
  br label %104

104:                                              ; preds = %101, %76
  %105 = phi ptr [ %72, %76 ], [ %103, %101 ]
  %106 = phi ptr [ %78, %76 ], [ %102, %101 ]
  %107 = getelementptr inbounds i32, ptr %74, i64 1
  %108 = icmp eq ptr %107, %1
  br i1 %108, label %.loopexit, label %71, !llvm.loop !73

.loopexit:                                        ; preds = %104, %.loopexit20
  ret ptr %4
}

; Function Attrs: mustprogress uwtable
define dso_local void @bk_runtime(ptr noundef readonly %0) local_unnamed_addr #12 personality ptr @__gxx_personality_v0 {
  %2 = alloca %"class.std::vector", align 8
  %3 = alloca %"class.std::vector", align 8
  %4 = alloca %"class.std::vector", align 8
  %5 = icmp eq ptr %0, null
  br i1 %5, label %53, label %6

6:                                                ; preds = %1
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %2) #18
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %2, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %3) #18
  %7 = load i64, ptr %0, align 8, !tbaa !23
  %8 = icmp ugt i64 %7, 2305843009213693951
  br i1 %8, label %9, label %10

9:                                                ; preds = %6
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2.19) #23
  unreachable

10:                                               ; preds = %6
  %11 = icmp eq i64 %7, 0
  br i1 %11, label %20, label %12

12:                                               ; preds = %10
  %13 = shl nuw nsw i64 %7, 2
  %14 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #25
  store ptr %14, ptr %3, align 8, !tbaa !45
  %15 = getelementptr inbounds i32, ptr %14, i64 %7
  %16 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 2
  store ptr %15, ptr %16, align 8, !tbaa !48
  store i32 0, ptr %14, align 4, !tbaa !33
  %17 = getelementptr i32, ptr %14, i64 1
  %18 = icmp eq i64 %7, 1
  br i1 %18, label %.thread, label %21

.thread:                                          ; preds = %12
  %19 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  store ptr %17, ptr %19, align 8, !tbaa !47
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #18
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %4, i8 0, i64 24, i1 false)
  br label %.preheader

20:                                               ; preds = %10
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %3, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #18
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %4, i8 0, i64 24, i1 false)
  br label %.loopexit

21:                                               ; preds = %12
  %22 = add nsw i64 %13, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %17, i8 0, i64 %22, i1 false), !tbaa !33
  %23 = getelementptr inbounds %"struct.std::_Vector_base<std::vector<int>, std::allocator<std::vector<int>>>::_Vector_impl_data", ptr %3, i64 0, i32 1
  store ptr %15, ptr %23, align 8, !tbaa !47
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %4) #18
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %4, i8 0, i64 24, i1 false)
  %24 = icmp ult i64 %7, 8
  br i1 %24, label %.preheader, label %25

25:                                               ; preds = %21
  %26 = and i64 %7, 2305843009213693944
  br label %27

27:                                               ; preds = %27, %25
  %28 = phi i64 [ 0, %25 ], [ %33, %27 ]
  %29 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %25 ], [ %34, %27 ]
  %30 = add <4 x i32> %29, splat (i32 4)
  %31 = getelementptr inbounds i32, ptr %14, i64 %28
  %32 = getelementptr inbounds i32, ptr %31, i64 4
  store <4 x i32> %29, ptr %31, align 4, !tbaa !33
  store <4 x i32> %30, ptr %32, align 4, !tbaa !33
  %33 = add nuw nsw i64 %28, 8
  %34 = add <4 x i32> %29, splat (i32 8)
  %35 = icmp eq i64 %33, %26
  br i1 %35, label %36, label %27, !llvm.loop !74

36:                                               ; preds = %27
  %37 = icmp eq i64 %7, %26
  br i1 %37, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %.thread, %21, %36
  %.ph = phi i64 [ 0, %.thread ], [ %26, %36 ], [ 0, %21 ]
  br label %38

.loopexit:                                        ; preds = %38, %36, %20
  invoke void @bron_kerbosch_with_pivot(ptr noundef nonnull %0, ptr noundef nonnull align 8 dereferenceable(24) %2, ptr noundef nonnull align 8 dereferenceable(24) %3, ptr noundef nonnull align 8 dereferenceable(24) %4)
          to label %44 unwind label %54

38:                                               ; preds = %.preheader, %38
  %39 = phi i64 [ %42, %38 ], [ %.ph, %.preheader ]
  %40 = getelementptr inbounds i32, ptr %14, i64 %39
  %41 = trunc i64 %39 to i32
  store i32 %41, ptr %40, align 4, !tbaa !33
  %42 = add nuw nsw i64 %39, 1
  %43 = icmp eq i64 %42, %7
  br i1 %43, label %.loopexit, label %38, !llvm.loop !75

44:                                               ; preds = %.loopexit
  %45 = load ptr, ptr %4, align 8, !tbaa !45
  %46 = icmp eq ptr %45, null
  br i1 %46, label %48, label %47

47:                                               ; preds = %44
  tail call void @_ZdlPv(ptr noundef nonnull %45) #22
  br label %48

48:                                               ; preds = %44, %47
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #18
  %49 = load ptr, ptr %3, align 8, !tbaa !45
  %50 = icmp eq ptr %49, null
  br i1 %50, label %52, label %51

51:                                               ; preds = %48
  tail call void @_ZdlPv(ptr noundef nonnull %49) #22
  br label %52

52:                                               ; preds = %51, %48
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #18
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #18
  br label %53

53:                                               ; preds = %1, %52
  ret void

54:                                               ; preds = %.loopexit
  %55 = landingpad { ptr, i32 }
          cleanup
  %56 = load ptr, ptr %4, align 8, !tbaa !45
  %57 = icmp eq ptr %56, null
  br i1 %57, label %59, label %58

58:                                               ; preds = %54
  tail call void @_ZdlPv(ptr noundef nonnull %56) #22
  br label %59

59:                                               ; preds = %54, %58
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %4) #18
  %60 = load ptr, ptr %3, align 8, !tbaa !45
  %61 = icmp eq ptr %60, null
  br i1 %61, label %63, label %62

62:                                               ; preds = %59
  tail call void @_ZdlPv(ptr noundef nonnull %60) #22
  br label %63

63:                                               ; preds = %59, %62
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #18
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #18
  resume { ptr, i32 } %55
}

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { nofree nounwind }
attributes #2 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { noinline noreturn nounwind uwtable "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nobuiltin nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { cold noreturn nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #8 = { noreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nobuiltin allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { cold noreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #12 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #13 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #14 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #15 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #16 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #17 = { alwaysinline norecurse nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #18 = { nounwind }
attributes #19 = { convergent nounwind }
attributes #20 = { cold noreturn nounwind }
attributes #21 = { noreturn nounwind }
attributes #22 = { builtin nounwind }
attributes #23 = { cold noreturn }
attributes #24 = { noreturn }
attributes #25 = { builtin allocsize(0) }

!llvm.ident = !{!0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{i32 7, !"openmp", i32 51}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !6}
!9 = distinct !{!9, !6}
!10 = distinct !{!10, !6}
!11 = !{!12}
!12 = distinct !{!12, !13, !"_ZN9__gnu_cxx14__alloc_traitsISaIiEiE17_S_select_on_copyERKS1_: argument 0"}
!13 = distinct !{!13, !"_ZN9__gnu_cxx14__alloc_traitsISaIiEiE17_S_select_on_copyERKS1_"}
!14 = !{!15}
!15 = distinct !{!15, !16, !"_ZNSt16allocator_traitsISaIiEE37select_on_container_copy_constructionERKS0_: argument 0"}
!16 = distinct !{!16, !"_ZNSt16allocator_traitsISaIiEE37select_on_container_copy_constructionERKS0_"}
!17 = distinct !{!17, !6}
!18 = distinct !{!18, !6}
!19 = distinct !{!19, !6}
!20 = distinct !{!20, !6}
!21 = distinct !{!21, !6}
!22 = distinct !{!22, !6}
!23 = !{!24, !25, i64 0}
!24 = !{!"_ZTS5Graph", !25, i64 0, !25, i64 8, !28, i64 16, !28, i64 24}
!25 = !{!"long", !26, i64 0}
!26 = !{!"omnipotent char", !27, i64 0}
!27 = !{!"Simple C++ TBAA"}
!28 = !{!"any pointer", !26, i64 0}
!29 = !{!24, !28, i64 16}
!30 = !{!24, !28, i64 24}
!31 = !{!26, !26, i64 0}
!32 = distinct !{!32, !6}
!33 = !{!34, !34, i64 0}
!34 = !{!"int", !26, i64 0}
!35 = distinct !{!35, !6}
!36 = !{!25, !25, i64 0}
!37 = distinct !{!37, !6}
!38 = !{!28, !28, i64 0}
!39 = !{!40, !28, i64 0}
!40 = !{!"_ZTSNSt12_Vector_baseIcSaIcEE17_Vector_impl_dataE", !28, i64 0, !28, i64 8, !28, i64 16}
!41 = !{!40, !28, i64 8}
!42 = !{!40, !28, i64 16}
!43 = !{!44, !44, i64 0}
!44 = !{!"double", !26, i64 0}
!45 = !{!46, !28, i64 0}
!46 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !28, i64 0, !28, i64 8, !28, i64 16}
!47 = !{!46, !28, i64 8}
!48 = !{!46, !28, i64 16}
!49 = distinct !{!49, !6}
!50 = distinct !{!50, !6}
!51 = !{!52}
!52 = !{i64 2, i64 -1, i64 -1, i1 true}
!53 = distinct !{!53, !6}
!54 = distinct !{!54, !6, !55, !56}
!55 = !{!"llvm.loop.isvectorized", i32 1}
!56 = !{!"llvm.loop.unroll.runtime.disable"}
!57 = distinct !{!57, !6, !55}
!58 = !{!"branch_weights", i32 -2147483648, i32 0}
!59 = !{!"branch_weights", i32 2000, i32 1}
!60 = distinct !{!60, !6, !55, !56}
!61 = distinct !{!61, !6, !55}
!62 = distinct !{!62, !6, !55, !56}
!63 = distinct !{!63, !6, !55}
!64 = distinct !{!64, !6, !55, !56}
!65 = distinct !{!65, !6, !55}
!66 = distinct !{!66, !6}
!67 = distinct !{!67, !6, !68}
!68 = !{!"llvm.loop.unroll.count", i32 4}
!69 = distinct !{!69, !6, !70}
!70 = !{!"llvm.loop.unroll.disable"}
!71 = distinct !{!71, !6}
!72 = distinct !{!72, !6}
!73 = distinct !{!73, !6}
!74 = distinct !{!74, !6, !55, !56}
!75 = distinct !{!75, !6, !56, !55}
