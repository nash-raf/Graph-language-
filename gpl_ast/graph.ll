; ModuleID = 'graph.cpp'
source_filename = "graph.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"struct.std::hash" = type { i8 }
%class.Edge_t = type { i32, i32, float }
%class.HashMap_t = type { i64, i64, ptr, ptr }
%"struct.HashMap_t::HashItem_t" = type { %class.Edge_t, i32 }
%class.Graph = type { i32, i32, i8, i8, i8, ptr, ptr, ptr, ptr, ptr }
%"class.__gnu_cxx::__normal_iterator" = type { ptr }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl" }
%"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl" = type { %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data" }
%"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data" = type { ptr, ptr, ptr }

$_ZNKSt4hashIiEclEi = comdat any

$_ZN9HashMap_t10HashItem_tC2Ev = comdat any

$_ZNKSt6vectorI6Edge_tSaIS0_EE5beginEv = comdat any

$_ZNKSt6vectorI6Edge_tSaIS0_EE3endEv = comdat any

$_ZN9__gnu_cxxneIPK6Edge_tSt6vectorIS1_SaIS1_EEEEbRKNS_17__normal_iteratorIT_T0_EESC_ = comdat any

$_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEdeEv = comdat any

$_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEppEv = comdat any

$_ZNKSt6vectorI6Edge_tSaIS0_EE4sizeEv = comdat any

$_ZNKSt6vectorI6Edge_tSaIS0_EEixEm = comdat any

$_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEC2ERKS3_ = comdat any

$_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEE4baseEv = comdat any

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@.str = private unnamed_addr constant [27 x i8] c"table_size[pos] < MAX_COLL\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"graph.cpp\00", align 1
@__PRETTY_FUNCTION__._ZN9HashMap_t6insertERK6Edge_ti = private unnamed_addr constant [44 x i8] c"void HashMap_t::insert(const Edge_t &, int)\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_graph.cpp, ptr null }]

@_ZN6Edge_tC1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN6Edge_tC2Ev
@_ZN6Edge_tC1Eiib = dso_local unnamed_addr alias void (ptr, i32, i32, i1), ptr @_ZN6Edge_tC2Eiib
@_ZN6Edge_tC1Eiifb = dso_local unnamed_addr alias void (ptr, i32, i32, float, i1), ptr @_ZN6Edge_tC2Eiifb
@_ZN9HashMap_tC1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN9HashMap_tC2Ev
@_ZN9HashMap_tD1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN9HashMap_tD2Ev
@_ZN5GraphC1Eiibbb = dso_local unnamed_addr alias void (ptr, i32, i32, i1, i1, i1), ptr @_ZN5GraphC2Eiibbb
@_ZN5GraphD1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN5GraphD2Ev

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init() #0 section ".text.startup" {
  call void @_ZNSt8ios_base4InitC1Ev(ptr noundef nonnull align 1 dereferenceable(1) @_ZStL8__ioinit)
  %1 = call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZStL8__ioinit, ptr @__dso_handle) #3
  ret void
}

declare void @_ZNSt8ios_base4InitC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nounwind
declare void @_ZNSt8ios_base4InitD1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #2

; Function Attrs: nounwind
declare i32 @__cxa_atexit(ptr, ptr, ptr) #3

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i64 @_Z1hRK6Edge_t(ptr noundef nonnull align 4 dereferenceable(12) %0) #4 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i64, align 8
  %6 = alloca %"struct.std::hash", align 1
  store ptr %0, ptr %2, align 8
  %7 = load ptr, ptr %2, align 8
  %8 = getelementptr inbounds %class.Edge_t, ptr %7, i32 0, i32 0
  %9 = load i32, ptr %8, align 4
  %10 = load ptr, ptr %2, align 8
  %11 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 1
  %12 = load i32, ptr %11, align 4
  %13 = icmp slt i32 %9, %12
  br i1 %13, label %14, label %18

14:                                               ; preds = %1
  %15 = load ptr, ptr %2, align 8
  %16 = getelementptr inbounds %class.Edge_t, ptr %15, i32 0, i32 0
  %17 = load i32, ptr %16, align 4
  br label %22

18:                                               ; preds = %1
  %19 = load ptr, ptr %2, align 8
  %20 = getelementptr inbounds %class.Edge_t, ptr %19, i32 0, i32 1
  %21 = load i32, ptr %20, align 4
  br label %22

22:                                               ; preds = %18, %14
  %23 = phi i32 [ %17, %14 ], [ %21, %18 ]
  store i32 %23, ptr %3, align 4
  %24 = load ptr, ptr %2, align 8
  %25 = getelementptr inbounds %class.Edge_t, ptr %24, i32 0, i32 0
  %26 = load i32, ptr %25, align 4
  %27 = load ptr, ptr %2, align 8
  %28 = getelementptr inbounds %class.Edge_t, ptr %27, i32 0, i32 1
  %29 = load i32, ptr %28, align 4
  %30 = icmp slt i32 %26, %29
  br i1 %30, label %31, label %35

31:                                               ; preds = %22
  %32 = load ptr, ptr %2, align 8
  %33 = getelementptr inbounds %class.Edge_t, ptr %32, i32 0, i32 1
  %34 = load i32, ptr %33, align 4
  br label %39

35:                                               ; preds = %22
  %36 = load ptr, ptr %2, align 8
  %37 = getelementptr inbounds %class.Edge_t, ptr %36, i32 0, i32 0
  %38 = load i32, ptr %37, align 4
  br label %39

39:                                               ; preds = %35, %31
  %40 = phi i32 [ %34, %31 ], [ %38, %35 ]
  store i32 %40, ptr %4, align 4
  store i64 1, ptr %5, align 8
  %41 = load i32, ptr %3, align 4
  %42 = call noundef i64 @_ZNKSt4hashIiEclEi(ptr noundef nonnull align 1 dereferenceable(1) %6, i32 noundef %41) #3
  %43 = add i64 %42, 2654435769
  %44 = load i64, ptr %5, align 8
  %45 = shl i64 %44, 6
  %46 = add i64 %43, %45
  %47 = load i64, ptr %5, align 8
  %48 = lshr i64 %47, 2
  %49 = add i64 %46, %48
  %50 = load i64, ptr %5, align 8
  %51 = xor i64 %50, %49
  store i64 %51, ptr %5, align 8
  %52 = load i32, ptr %4, align 4
  %53 = call noundef i64 @_ZNKSt4hashIiEclEi(ptr noundef nonnull align 1 dereferenceable(1) %6, i32 noundef %52) #3
  %54 = add i64 %53, 2654435769
  %55 = load i64, ptr %5, align 8
  %56 = shl i64 %55, 6
  %57 = add i64 %54, %56
  %58 = load i64, ptr %5, align 8
  %59 = lshr i64 %58, 2
  %60 = add i64 %57, %59
  %61 = load i64, ptr %5, align 8
  %62 = xor i64 %61, %60
  store i64 %62, ptr %5, align 8
  %63 = load i64, ptr %5, align 8
  ret i64 %63
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNKSt4hashIiEclEi(ptr noundef nonnull align 1 dereferenceable(1) %0, i32 noundef %1) #4 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  %5 = load ptr, ptr %3, align 8
  %6 = load i32, ptr %4, align 4
  %7 = sext i32 %6 to i64
  ret i64 %7
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN6Edge_tC2Ev(ptr noundef nonnull align 4 dereferenceable(12) %0) unnamed_addr #4 align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %class.Edge_t, ptr %3, i32 0, i32 0
  store i32 0, ptr %4, align 4
  %5 = getelementptr inbounds %class.Edge_t, ptr %3, i32 0, i32 1
  store i32 0, ptr %5, align 4
  %6 = getelementptr inbounds %class.Edge_t, ptr %3, i32 0, i32 2
  store float 1.000000e+00, ptr %6, align 4
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN6Edge_tC2Eiib(ptr noundef nonnull align 4 dereferenceable(12) %0, i32 noundef %1, i32 noundef %2, i1 noundef zeroext %3) unnamed_addr #4 align 2 {
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i8, align 1
  store ptr %0, ptr %5, align 8
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  %9 = zext i1 %3 to i8
  store i8 %9, ptr %8, align 1
  %10 = load ptr, ptr %5, align 8
  %11 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 0
  store i32 0, ptr %11, align 4
  %12 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 1
  store i32 0, ptr %12, align 4
  %13 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 2
  store float 1.000000e+00, ptr %13, align 4
  %14 = load i8, ptr %8, align 1
  %15 = trunc i8 %14 to i1
  br i1 %15, label %16, label %21

16:                                               ; preds = %4
  %17 = load i32, ptr %6, align 4
  %18 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 0
  store i32 %17, ptr %18, align 4
  %19 = load i32, ptr %7, align 4
  %20 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 1
  store i32 %19, ptr %20, align 4
  br label %42

21:                                               ; preds = %4
  %22 = load i32, ptr %6, align 4
  %23 = load i32, ptr %7, align 4
  %24 = icmp slt i32 %22, %23
  br i1 %24, label %25, label %27

25:                                               ; preds = %21
  %26 = load i32, ptr %6, align 4
  br label %29

27:                                               ; preds = %21
  %28 = load i32, ptr %7, align 4
  br label %29

29:                                               ; preds = %27, %25
  %30 = phi i32 [ %26, %25 ], [ %28, %27 ]
  %31 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 0
  store i32 %30, ptr %31, align 4
  %32 = load i32, ptr %6, align 4
  %33 = load i32, ptr %7, align 4
  %34 = icmp slt i32 %32, %33
  br i1 %34, label %35, label %37

35:                                               ; preds = %29
  %36 = load i32, ptr %7, align 4
  br label %39

37:                                               ; preds = %29
  %38 = load i32, ptr %6, align 4
  br label %39

39:                                               ; preds = %37, %35
  %40 = phi i32 [ %36, %35 ], [ %38, %37 ]
  %41 = getelementptr inbounds %class.Edge_t, ptr %10, i32 0, i32 1
  store i32 %40, ptr %41, align 4
  br label %42

42:                                               ; preds = %39, %16
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN6Edge_tC2Eiifb(ptr noundef nonnull align 4 dereferenceable(12) %0, i32 noundef %1, i32 noundef %2, float noundef %3, i1 noundef zeroext %4) unnamed_addr #4 align 2 {
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca float, align 4
  %10 = alloca i8, align 1
  store ptr %0, ptr %6, align 8
  store i32 %1, ptr %7, align 4
  store i32 %2, ptr %8, align 4
  store float %3, ptr %9, align 4
  %11 = zext i1 %4 to i8
  store i8 %11, ptr %10, align 1
  %12 = load ptr, ptr %6, align 8
  %13 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 0
  store i32 0, ptr %13, align 4
  %14 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 1
  store i32 0, ptr %14, align 4
  %15 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 2
  %16 = load float, ptr %9, align 4
  store float %16, ptr %15, align 4
  %17 = load i8, ptr %10, align 1
  %18 = trunc i8 %17 to i1
  br i1 %18, label %19, label %24

19:                                               ; preds = %5
  %20 = load i32, ptr %7, align 4
  %21 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 0
  store i32 %20, ptr %21, align 4
  %22 = load i32, ptr %8, align 4
  %23 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 1
  store i32 %22, ptr %23, align 4
  br label %45

24:                                               ; preds = %5
  %25 = load i32, ptr %7, align 4
  %26 = load i32, ptr %8, align 4
  %27 = icmp slt i32 %25, %26
  br i1 %27, label %28, label %30

28:                                               ; preds = %24
  %29 = load i32, ptr %7, align 4
  br label %32

30:                                               ; preds = %24
  %31 = load i32, ptr %8, align 4
  br label %32

32:                                               ; preds = %30, %28
  %33 = phi i32 [ %29, %28 ], [ %31, %30 ]
  %34 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 0
  store i32 %33, ptr %34, align 4
  %35 = load i32, ptr %7, align 4
  %36 = load i32, ptr %8, align 4
  %37 = icmp slt i32 %35, %36
  br i1 %37, label %38, label %40

38:                                               ; preds = %32
  %39 = load i32, ptr %8, align 4
  br label %42

40:                                               ; preds = %32
  %41 = load i32, ptr %7, align 4
  br label %42

42:                                               ; preds = %40, %38
  %43 = phi i32 [ %39, %38 ], [ %41, %40 ]
  %44 = getelementptr inbounds %class.Edge_t, ptr %12, i32 0, i32 1
  store i32 %43, ptr %44, align 4
  br label %45

45:                                               ; preds = %42, %19
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef zeroext i1 @_ZNK6Edge_tltERKS_(ptr noundef nonnull align 4 dereferenceable(12) %0, ptr noundef nonnull align 4 dereferenceable(12) %1) #4 align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = getelementptr inbounds %class.Edge_t, ptr %5, i32 0, i32 0
  %7 = load i32, ptr %6, align 4
  %8 = load ptr, ptr %4, align 8
  %9 = getelementptr inbounds %class.Edge_t, ptr %8, i32 0, i32 0
  %10 = load i32, ptr %9, align 4
  %11 = icmp slt i32 %7, %10
  br i1 %11, label %28, label %12

12:                                               ; preds = %2
  %13 = getelementptr inbounds %class.Edge_t, ptr %5, i32 0, i32 0
  %14 = load i32, ptr %13, align 4
  %15 = load ptr, ptr %4, align 8
  %16 = getelementptr inbounds %class.Edge_t, ptr %15, i32 0, i32 0
  %17 = load i32, ptr %16, align 4
  %18 = icmp eq i32 %14, %17
  br i1 %18, label %19, label %26

19:                                               ; preds = %12
  %20 = getelementptr inbounds %class.Edge_t, ptr %5, i32 0, i32 1
  %21 = load i32, ptr %20, align 4
  %22 = load ptr, ptr %4, align 8
  %23 = getelementptr inbounds %class.Edge_t, ptr %22, i32 0, i32 1
  %24 = load i32, ptr %23, align 4
  %25 = icmp slt i32 %21, %24
  br label %26

26:                                               ; preds = %19, %12
  %27 = phi i1 [ false, %12 ], [ %25, %19 ]
  br label %28

28:                                               ; preds = %26, %2
  %29 = phi i1 [ true, %2 ], [ %27, %26 ]
  ret i1 %29
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef zeroext i1 @_ZNK6Edge_teqERKS_(ptr noundef nonnull align 4 dereferenceable(12) %0, ptr noundef nonnull align 4 dereferenceable(12) %1) #4 align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = getelementptr inbounds %class.Edge_t, ptr %5, i32 0, i32 0
  %7 = load i32, ptr %6, align 4
  %8 = load ptr, ptr %4, align 8
  %9 = getelementptr inbounds %class.Edge_t, ptr %8, i32 0, i32 0
  %10 = load i32, ptr %9, align 4
  %11 = icmp eq i32 %7, %10
  br i1 %11, label %12, label %19

12:                                               ; preds = %2
  %13 = getelementptr inbounds %class.Edge_t, ptr %5, i32 0, i32 1
  %14 = load i32, ptr %13, align 4
  %15 = load ptr, ptr %4, align 8
  %16 = getelementptr inbounds %class.Edge_t, ptr %15, i32 0, i32 1
  %17 = load i32, ptr %16, align 4
  %18 = icmp eq i32 %14, %17
  br label %19

19:                                               ; preds = %12, %2
  %20 = phi i1 [ false, %2 ], [ %18, %12 ]
  ret i1 %20
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i64 @_ZNK6Edge_t11Hash_Edge_tclERKS_(ptr noundef nonnull align 1 dereferenceable(1) %0, ptr noundef nonnull align 4 dereferenceable(12) %1) #4 align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load ptr, ptr %4, align 8
  %7 = call noundef i64 @_Z1hRK6Edge_t(ptr noundef nonnull align 4 dereferenceable(12) %6)
  ret i64 %7
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN9HashMap_tC2Ev(ptr noundef nonnull align 8 dereferenceable(32) %0) unnamed_addr #5 align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %class.HashMap_t, ptr %3, i32 0, i32 0
  store i64 8388607, ptr %4, align 8
  %5 = getelementptr inbounds %class.HashMap_t, ptr %3, i32 0, i32 1
  store i64 100, ptr %5, align 8
  %6 = getelementptr inbounds %class.HashMap_t, ptr %3, i32 0, i32 0
  %7 = load i64, ptr %6, align 8
  %8 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %7, i64 8)
  %9 = extractvalue { i64, i1 } %8, 1
  %10 = extractvalue { i64, i1 } %8, 0
  %11 = select i1 %9, i64 -1, i64 %10
  %12 = call noalias noundef nonnull ptr @_Znam(i64 noundef %11) #12
  %13 = getelementptr inbounds %class.HashMap_t, ptr %3, i32 0, i32 3
  store ptr %12, ptr %13, align 8
  %14 = getelementptr inbounds %class.HashMap_t, ptr %3, i32 0, i32 0
  %15 = load i64, ptr %14, align 8
  %16 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %15, i64 4)
  %17 = extractvalue { i64, i1 } %16, 1
  %18 = extractvalue { i64, i1 } %16, 0
  %19 = select i1 %17, i64 -1, i64 %18
  %20 = call noalias noundef nonnull ptr @_Znam(i64 noundef %19) #12
  call void @llvm.memset.p0.i64(ptr align 4 %20, i8 0, i64 %19, i1 false)
  %21 = getelementptr inbounds %class.HashMap_t, ptr %3, i32 0, i32 2
  store ptr %20, ptr %21, align 8
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare { i64, i1 } @llvm.umul.with.overflow.i64(i64, i64) #6

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znam(i64 noundef) #7

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #8

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN9HashMap_tD2Ev(ptr noundef nonnull align 8 dereferenceable(32) %0) unnamed_addr #4 align 2 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  %4 = load ptr, ptr %2, align 8
  store i64 0, ptr %3, align 8
  br label %5

5:                                                ; preds = %27, %1
  %6 = load i64, ptr %3, align 8
  %7 = getelementptr inbounds %class.HashMap_t, ptr %4, i32 0, i32 0
  %8 = load i64, ptr %7, align 8
  %9 = icmp ult i64 %6, %8
  br i1 %9, label %10, label %30

10:                                               ; preds = %5
  %11 = getelementptr inbounds %class.HashMap_t, ptr %4, i32 0, i32 2
  %12 = load ptr, ptr %11, align 8
  %13 = load i64, ptr %3, align 8
  %14 = getelementptr inbounds i32, ptr %12, i64 %13
  %15 = load i32, ptr %14, align 4
  %16 = icmp sgt i32 %15, 0
  br i1 %16, label %17, label %26

17:                                               ; preds = %10
  %18 = getelementptr inbounds %class.HashMap_t, ptr %4, i32 0, i32 3
  %19 = load ptr, ptr %18, align 8
  %20 = load i64, ptr %3, align 8
  %21 = getelementptr inbounds ptr, ptr %19, i64 %20
  %22 = load ptr, ptr %21, align 8
  %23 = icmp eq ptr %22, null
  br i1 %23, label %25, label %24

24:                                               ; preds = %17
  call void @_ZdaPv(ptr noundef %22) #13
  br label %25

25:                                               ; preds = %24, %17
  br label %26

26:                                               ; preds = %25, %10
  br label %27

27:                                               ; preds = %26
  %28 = load i64, ptr %3, align 8
  %29 = add i64 %28, 1
  store i64 %29, ptr %3, align 8
  br label %5, !llvm.loop !6

30:                                               ; preds = %5
  %31 = getelementptr inbounds %class.HashMap_t, ptr %4, i32 0, i32 3
  %32 = load ptr, ptr %31, align 8
  %33 = icmp eq ptr %32, null
  br i1 %33, label %35, label %34

34:                                               ; preds = %30
  call void @_ZdaPv(ptr noundef %32) #13
  br label %35

35:                                               ; preds = %34, %30
  %36 = getelementptr inbounds %class.HashMap_t, ptr %4, i32 0, i32 2
  %37 = load ptr, ptr %36, align 8
  %38 = icmp eq ptr %37, null
  br i1 %38, label %40, label %39

39:                                               ; preds = %35
  call void @_ZdaPv(ptr noundef %37) #13
  br label %40

40:                                               ; preds = %39, %35
  ret void
}

; Function Attrs: nobuiltin nounwind
declare void @_ZdaPv(ptr noundef) #9

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_ZN9HashMap_t5existERK6Edge_t(ptr noundef nonnull align 8 dereferenceable(32) %0, ptr noundef nonnull align 4 dereferenceable(12) %1) #4 align 2 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %9 = load ptr, ptr %4, align 8
  %10 = load ptr, ptr %5, align 8
  %11 = call noundef i64 @_Z1hRK6Edge_t(ptr noundef nonnull align 4 dereferenceable(12) %10)
  %12 = getelementptr inbounds %class.HashMap_t, ptr %9, i32 0, i32 0
  %13 = load i64, ptr %12, align 8
  %14 = urem i64 %11, %13
  store i64 %14, ptr %6, align 8
  %15 = getelementptr inbounds %class.HashMap_t, ptr %9, i32 0, i32 2
  %16 = load ptr, ptr %15, align 8
  %17 = load i64, ptr %6, align 8
  %18 = getelementptr inbounds i32, ptr %16, i64 %17
  %19 = load i32, ptr %18, align 4
  %20 = icmp sgt i32 %19, 0
  br i1 %20, label %21, label %87

21:                                               ; preds = %2
  store i32 0, ptr %7, align 4
  br label %22

22:                                               ; preds = %83, %21
  %23 = load i32, ptr %7, align 4
  %24 = getelementptr inbounds %class.HashMap_t, ptr %9, i32 0, i32 2
  %25 = load ptr, ptr %24, align 8
  %26 = load i64, ptr %6, align 8
  %27 = getelementptr inbounds i32, ptr %25, i64 %26
  %28 = load i32, ptr %27, align 4
  %29 = icmp slt i32 %23, %28
  br i1 %29, label %30, label %86

30:                                               ; preds = %22
  %31 = getelementptr inbounds %class.HashMap_t, ptr %9, i32 0, i32 3
  %32 = load ptr, ptr %31, align 8
  %33 = load i64, ptr %6, align 8
  %34 = getelementptr inbounds ptr, ptr %32, i64 %33
  %35 = load ptr, ptr %34, align 8
  %36 = load i32, ptr %7, align 4
  %37 = sext i32 %36 to i64
  %38 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %35, i64 %37
  %39 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %38, i32 0, i32 0
  store ptr %39, ptr %8, align 8
  %40 = load ptr, ptr %8, align 8
  %41 = getelementptr inbounds %class.Edge_t, ptr %40, i32 0, i32 0
  %42 = load i32, ptr %41, align 4
  %43 = load ptr, ptr %5, align 8
  %44 = getelementptr inbounds %class.Edge_t, ptr %43, i32 0, i32 0
  %45 = load i32, ptr %44, align 4
  %46 = icmp eq i32 %42, %45
  br i1 %46, label %47, label %55

47:                                               ; preds = %30
  %48 = load ptr, ptr %8, align 8
  %49 = getelementptr inbounds %class.Edge_t, ptr %48, i32 0, i32 1
  %50 = load i32, ptr %49, align 4
  %51 = load ptr, ptr %5, align 8
  %52 = getelementptr inbounds %class.Edge_t, ptr %51, i32 0, i32 1
  %53 = load i32, ptr %52, align 4
  %54 = icmp eq i32 %50, %53
  br i1 %54, label %71, label %55

55:                                               ; preds = %47, %30
  %56 = load ptr, ptr %8, align 8
  %57 = getelementptr inbounds %class.Edge_t, ptr %56, i32 0, i32 0
  %58 = load i32, ptr %57, align 4
  %59 = load ptr, ptr %5, align 8
  %60 = getelementptr inbounds %class.Edge_t, ptr %59, i32 0, i32 1
  %61 = load i32, ptr %60, align 4
  %62 = icmp eq i32 %58, %61
  br i1 %62, label %63, label %82

63:                                               ; preds = %55
  %64 = load ptr, ptr %8, align 8
  %65 = getelementptr inbounds %class.Edge_t, ptr %64, i32 0, i32 1
  %66 = load i32, ptr %65, align 4
  %67 = load ptr, ptr %5, align 8
  %68 = getelementptr inbounds %class.Edge_t, ptr %67, i32 0, i32 0
  %69 = load i32, ptr %68, align 4
  %70 = icmp eq i32 %66, %69
  br i1 %70, label %71, label %82

71:                                               ; preds = %63, %47
  %72 = getelementptr inbounds %class.HashMap_t, ptr %9, i32 0, i32 3
  %73 = load ptr, ptr %72, align 8
  %74 = load i64, ptr %6, align 8
  %75 = getelementptr inbounds ptr, ptr %73, i64 %74
  %76 = load ptr, ptr %75, align 8
  %77 = load i32, ptr %7, align 4
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %76, i64 %78
  %80 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %79, i32 0, i32 1
  %81 = load i32, ptr %80, align 4
  store i32 %81, ptr %3, align 4
  br label %88

82:                                               ; preds = %63, %55
  br label %83

83:                                               ; preds = %82
  %84 = load i32, ptr %7, align 4
  %85 = add nsw i32 %84, 1
  store i32 %85, ptr %7, align 4
  br label %22, !llvm.loop !8

86:                                               ; preds = %22
  br label %87

87:                                               ; preds = %86, %2
  store i32 -1, ptr %3, align 4
  br label %88

88:                                               ; preds = %87, %71
  %89 = load i32, ptr %3, align 4
  ret i32 %89
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN9HashMap_t6insertERK6Edge_ti(ptr noundef nonnull align 8 dereferenceable(32) %0, ptr noundef nonnull align 4 dereferenceable(12) %1, i32 noundef %2) #5 align 2 personality ptr @__gxx_personality_v0 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i32 %2, ptr %6, align 4
  %10 = load ptr, ptr %4, align 8
  %11 = load ptr, ptr %5, align 8
  %12 = call noundef i64 @_Z1hRK6Edge_t(ptr noundef nonnull align 4 dereferenceable(12) %11)
  %13 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 0
  %14 = load i64, ptr %13, align 8
  %15 = urem i64 %12, %14
  store i64 %15, ptr %7, align 8
  %16 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 2
  %17 = load ptr, ptr %16, align 8
  %18 = load i64, ptr %7, align 8
  %19 = getelementptr inbounds i32, ptr %17, i64 %18
  %20 = load i32, ptr %19, align 4
  %21 = icmp eq i32 %20, 0
  br i1 %21, label %22, label %47

22:                                               ; preds = %3
  %23 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 1
  %24 = load i64, ptr %23, align 8
  %25 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %24, i64 16)
  %26 = extractvalue { i64, i1 } %25, 1
  %27 = extractvalue { i64, i1 } %25, 0
  %28 = select i1 %26, i64 -1, i64 %27
  %29 = call noalias noundef nonnull ptr @_Znam(i64 noundef %28) #12
  %30 = icmp eq i64 %24, 0
  br i1 %30, label %38, label %31

31:                                               ; preds = %22
  %32 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %29, i64 %24
  br label %33

33:                                               ; preds = %35, %31
  %34 = phi ptr [ %29, %31 ], [ %36, %35 ]
  invoke void @_ZN9HashMap_t10HashItem_tC2Ev(ptr noundef nonnull align 4 dereferenceable(16) %34)
          to label %35 unwind label %43

35:                                               ; preds = %33
  %36 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %34, i64 1
  %37 = icmp eq ptr %36, %32
  br i1 %37, label %38, label %33

38:                                               ; preds = %22, %35
  %39 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 3
  %40 = load ptr, ptr %39, align 8
  %41 = load i64, ptr %7, align 8
  %42 = getelementptr inbounds ptr, ptr %40, i64 %41
  store ptr %29, ptr %42, align 8
  br label %47

43:                                               ; preds = %33
  %44 = landingpad { ptr, i32 }
          cleanup
  %45 = extractvalue { ptr, i32 } %44, 0
  store ptr %45, ptr %8, align 8
  %46 = extractvalue { ptr, i32 } %44, 1
  store i32 %46, ptr %9, align 4
  call void @_ZdaPv(ptr noundef %29) #13
  br label %90

47:                                               ; preds = %38, %3
  %48 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 2
  %49 = load ptr, ptr %48, align 8
  %50 = load i64, ptr %7, align 8
  %51 = getelementptr inbounds i32, ptr %49, i64 %50
  %52 = load i32, ptr %51, align 4
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 1
  %55 = load i64, ptr %54, align 8
  %56 = icmp ult i64 %53, %55
  br i1 %56, label %57, label %58

57:                                               ; preds = %47
  br label %60

58:                                               ; preds = %47
  call void @__assert_fail(ptr noundef @.str, ptr noundef @.str.1, i32 noundef 90, ptr noundef @__PRETTY_FUNCTION__._ZN9HashMap_t6insertERK6Edge_ti) #14
  unreachable

59:                                               ; No predecessors!
  br label %60

60:                                               ; preds = %59, %57
  %61 = load ptr, ptr %5, align 8
  %62 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 3
  %63 = load ptr, ptr %62, align 8
  %64 = load i64, ptr %7, align 8
  %65 = getelementptr inbounds ptr, ptr %63, i64 %64
  %66 = load ptr, ptr %65, align 8
  %67 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 2
  %68 = load ptr, ptr %67, align 8
  %69 = load i64, ptr %7, align 8
  %70 = getelementptr inbounds i32, ptr %68, i64 %69
  %71 = load i32, ptr %70, align 4
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %66, i64 %72
  %74 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %73, i32 0, i32 0
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %74, ptr align 4 %61, i64 12, i1 false)
  %75 = load i32, ptr %6, align 4
  %76 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 3
  %77 = load ptr, ptr %76, align 8
  %78 = load i64, ptr %7, align 8
  %79 = getelementptr inbounds ptr, ptr %77, i64 %78
  %80 = load ptr, ptr %79, align 8
  %81 = getelementptr inbounds %class.HashMap_t, ptr %10, i32 0, i32 2
  %82 = load ptr, ptr %81, align 8
  %83 = load i64, ptr %7, align 8
  %84 = getelementptr inbounds i32, ptr %82, i64 %83
  %85 = load i32, ptr %84, align 4
  %86 = add nsw i32 %85, 1
  store i32 %86, ptr %84, align 4
  %87 = sext i32 %85 to i64
  %88 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %80, i64 %87
  %89 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %88, i32 0, i32 1
  store i32 %75, ptr %89, align 4
  ret void

90:                                               ; preds = %43
  %91 = load ptr, ptr %8, align 8
  %92 = load i32, ptr %9, align 4
  %93 = insertvalue { ptr, i32 } poison, ptr %91, 0
  %94 = insertvalue { ptr, i32 } %93, i32 %92, 1
  resume { ptr, i32 } %94
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZN9HashMap_t10HashItem_tC2Ev(ptr noundef nonnull align 4 dereferenceable(16) %0) unnamed_addr #5 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %3, i32 0, i32 0
  call void @_ZN6Edge_tC1Ev(ptr noundef nonnull align 4 dereferenceable(12) %4)
  %5 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %3, i32 0, i32 1
  store i32 -1, ptr %5, align 4
  ret void
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: noreturn nounwind
declare void @__assert_fail(ptr noundef, ptr noundef, i32 noundef, ptr noundef) #10

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #11

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN9HashMap_t6removeERK6Edge_t(ptr noundef nonnull align 8 dereferenceable(32) %0, ptr noundef nonnull align 4 dereferenceable(12) %1) #4 align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %7 = load ptr, ptr %3, align 8
  %8 = load ptr, ptr %4, align 8
  %9 = call noundef i64 @_Z1hRK6Edge_t(ptr noundef nonnull align 4 dereferenceable(12) %8)
  %10 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 0
  %11 = load i64, ptr %10, align 8
  %12 = urem i64 %9, %11
  store i64 %12, ptr %5, align 8
  %13 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 2
  %14 = load ptr, ptr %13, align 8
  %15 = load i64, ptr %5, align 8
  %16 = getelementptr inbounds i32, ptr %14, i64 %15
  %17 = load i32, ptr %16, align 4
  %18 = icmp sgt i32 %17, 0
  br i1 %18, label %19, label %83

19:                                               ; preds = %2
  store i32 0, ptr %6, align 4
  br label %20

20:                                               ; preds = %79, %19
  %21 = load i32, ptr %6, align 4
  %22 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 2
  %23 = load ptr, ptr %22, align 8
  %24 = load i64, ptr %5, align 8
  %25 = getelementptr inbounds i32, ptr %23, i64 %24
  %26 = load i32, ptr %25, align 4
  %27 = icmp slt i32 %21, %26
  br i1 %27, label %28, label %82

28:                                               ; preds = %20
  %29 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 3
  %30 = load ptr, ptr %29, align 8
  %31 = load i64, ptr %5, align 8
  %32 = getelementptr inbounds ptr, ptr %30, i64 %31
  %33 = load ptr, ptr %32, align 8
  %34 = load i32, ptr %6, align 4
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %33, i64 %35
  %37 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %36, i32 0, i32 0
  %38 = load ptr, ptr %4, align 8
  %39 = call noundef zeroext i1 @_ZNK6Edge_teqERKS_(ptr noundef nonnull align 4 dereferenceable(12) %37, ptr noundef nonnull align 4 dereferenceable(12) %38)
  br i1 %39, label %40, label %78

40:                                               ; preds = %28
  %41 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 3
  %42 = load ptr, ptr %41, align 8
  %43 = load i64, ptr %5, align 8
  %44 = getelementptr inbounds ptr, ptr %42, i64 %43
  %45 = load ptr, ptr %44, align 8
  %46 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 2
  %47 = load ptr, ptr %46, align 8
  %48 = load i64, ptr %5, align 8
  %49 = getelementptr inbounds i32, ptr %47, i64 %48
  %50 = load i32, ptr %49, align 4
  %51 = add nsw i32 %50, -1
  store i32 %51, ptr %49, align 4
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %45, i64 %52
  %54 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 3
  %55 = load ptr, ptr %54, align 8
  %56 = load i64, ptr %5, align 8
  %57 = getelementptr inbounds ptr, ptr %55, i64 %56
  %58 = load ptr, ptr %57, align 8
  %59 = load i32, ptr %6, align 4
  %60 = sext i32 %59 to i64
  %61 = getelementptr inbounds %"struct.HashMap_t::HashItem_t", ptr %58, i64 %60
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %61, ptr align 4 %53, i64 16, i1 false)
  %62 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 2
  %63 = load ptr, ptr %62, align 8
  %64 = load i64, ptr %5, align 8
  %65 = getelementptr inbounds i32, ptr %63, i64 %64
  %66 = load i32, ptr %65, align 4
  %67 = icmp eq i32 %66, 0
  br i1 %67, label %68, label %77

68:                                               ; preds = %40
  %69 = getelementptr inbounds %class.HashMap_t, ptr %7, i32 0, i32 3
  %70 = load ptr, ptr %69, align 8
  %71 = load i64, ptr %5, align 8
  %72 = getelementptr inbounds ptr, ptr %70, i64 %71
  %73 = load ptr, ptr %72, align 8
  %74 = icmp eq ptr %73, null
  br i1 %74, label %76, label %75

75:                                               ; preds = %68
  call void @_ZdaPv(ptr noundef %73) #13
  br label %76

76:                                               ; preds = %75, %68
  br label %77

77:                                               ; preds = %76, %40
  ret void

78:                                               ; preds = %28
  br label %79

79:                                               ; preds = %78
  %80 = load i32, ptr %6, align 4
  %81 = add nsw i32 %80, 1
  store i32 %81, ptr %6, align 4
  br label %20, !llvm.loop !9

82:                                               ; preds = %20
  br label %83

83:                                               ; preds = %82, %2
  call void @exit(i32 noundef -1) #14
  unreachable
}

; Function Attrs: noreturn nounwind
declare void @exit(i32 noundef) #10

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN5GraphC2Eiibbb(ptr noundef nonnull align 8 dereferenceable(56) %0, i32 noundef %1, i32 noundef %2, i1 noundef zeroext %3, i1 noundef zeroext %4, i1 noundef zeroext %5) unnamed_addr #5 align 2 {
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i8, align 1
  %11 = alloca i8, align 1
  %12 = alloca i8, align 1
  store ptr %0, ptr %7, align 8
  store i32 %1, ptr %8, align 4
  store i32 %2, ptr %9, align 4
  %13 = zext i1 %3 to i8
  store i8 %13, ptr %10, align 1
  %14 = zext i1 %4 to i8
  store i8 %14, ptr %11, align 1
  %15 = zext i1 %5 to i8
  store i8 %15, ptr %12, align 1
  %16 = load ptr, ptr %7, align 8
  %17 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 0
  %18 = load i32, ptr %8, align 4
  store i32 %18, ptr %17, align 8
  %19 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 1
  %20 = load i32, ptr %9, align 4
  store i32 %20, ptr %19, align 4
  %21 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 2
  %22 = load i8, ptr %10, align 1
  %23 = trunc i8 %22 to i1
  %24 = zext i1 %23 to i8
  store i8 %24, ptr %21, align 8
  %25 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 3
  %26 = load i8, ptr %11, align 1
  %27 = trunc i8 %26 to i1
  %28 = zext i1 %27 to i8
  store i8 %28, ptr %25, align 1
  %29 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 4
  %30 = load i8, ptr %12, align 1
  %31 = trunc i8 %30 to i1
  %32 = zext i1 %31 to i8
  store i8 %32, ptr %29, align 2
  %33 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 5
  store ptr null, ptr %33, align 8
  %34 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 6
  store ptr null, ptr %34, align 8
  %35 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 7
  store ptr null, ptr %35, align 8
  %36 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 8
  store ptr null, ptr %36, align 8
  %37 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 9
  store ptr null, ptr %37, align 8
  %38 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 0
  %39 = load i32, ptr %38, align 8
  %40 = sext i32 %39 to i64
  %41 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %40, i64 4)
  %42 = extractvalue { i64, i1 } %41, 1
  %43 = extractvalue { i64, i1 } %41, 0
  %44 = select i1 %42, i64 -1, i64 %43
  %45 = call noalias noundef nonnull ptr @_Znam(i64 noundef %44) #12
  call void @llvm.memset.p0.i64(ptr align 4 %45, i8 0, i64 %44, i1 false)
  %46 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 5
  store ptr %45, ptr %46, align 8
  %47 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 0
  %48 = load i32, ptr %47, align 8
  %49 = add nsw i32 %48, 1
  %50 = sext i32 %49 to i64
  %51 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %50, i64 4)
  %52 = extractvalue { i64, i1 } %51, 1
  %53 = extractvalue { i64, i1 } %51, 0
  %54 = select i1 %52, i64 -1, i64 %53
  %55 = call noalias noundef nonnull ptr @_Znam(i64 noundef %54) #12
  call void @llvm.memset.p0.i64(ptr align 4 %55, i8 0, i64 %54, i1 false)
  %56 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 6
  store ptr %55, ptr %56, align 8
  %57 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 1
  %58 = load i32, ptr %57, align 4
  %59 = mul nsw i32 2, %58
  %60 = sext i32 %59 to i64
  %61 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %60, i64 4)
  %62 = extractvalue { i64, i1 } %61, 1
  %63 = extractvalue { i64, i1 } %61, 0
  %64 = select i1 %62, i64 -1, i64 %63
  %65 = call noalias noundef nonnull ptr @_Znam(i64 noundef %64) #12
  call void @llvm.memset.p0.i64(ptr align 4 %65, i8 0, i64 %64, i1 false)
  %66 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 7
  store ptr %65, ptr %66, align 8
  %67 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 8
  store ptr null, ptr %67, align 8
  %68 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 9
  store ptr null, ptr %68, align 8
  %69 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 3
  %70 = load i8, ptr %69, align 1
  %71 = trunc i8 %70 to i1
  br i1 %71, label %72, label %83

72:                                               ; preds = %6
  %73 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 1
  %74 = load i32, ptr %73, align 4
  %75 = mul nsw i32 2, %74
  %76 = sext i32 %75 to i64
  %77 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %76, i64 4)
  %78 = extractvalue { i64, i1 } %77, 1
  %79 = extractvalue { i64, i1 } %77, 0
  %80 = select i1 %78, i64 -1, i64 %79
  %81 = call noalias noundef nonnull ptr @_Znam(i64 noundef %80) #12
  call void @llvm.memset.p0.i64(ptr align 4 %81, i8 0, i64 %80, i1 false)
  %82 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 8
  store ptr %81, ptr %82, align 8
  br label %83

83:                                               ; preds = %72, %6
  %84 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 4
  %85 = load i8, ptr %84, align 2
  %86 = trunc i8 %85 to i1
  br i1 %86, label %87, label %98

87:                                               ; preds = %83
  %88 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 1
  %89 = load i32, ptr %88, align 4
  %90 = mul nsw i32 2, %89
  %91 = sext i32 %90 to i64
  %92 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %91, i64 4)
  %93 = extractvalue { i64, i1 } %92, 1
  %94 = extractvalue { i64, i1 } %92, 0
  %95 = select i1 %93, i64 -1, i64 %94
  %96 = call noalias noundef nonnull ptr @_Znam(i64 noundef %95) #12
  call void @llvm.memset.p0.i64(ptr align 4 %96, i8 0, i64 %95, i1 false)
  %97 = getelementptr inbounds %class.Graph, ptr %16, i32 0, i32 9
  store ptr %96, ptr %97, align 8
  br label %98

98:                                               ; preds = %87, %83
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN5GraphD2Ev(ptr noundef nonnull align 8 dereferenceable(56) %0) unnamed_addr #4 align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 5
  %5 = load ptr, ptr %4, align 8
  %6 = icmp eq ptr %5, null
  br i1 %6, label %8, label %7

7:                                                ; preds = %1
  call void @_ZdaPv(ptr noundef %5) #13
  br label %8

8:                                                ; preds = %7, %1
  %9 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 6
  %10 = load ptr, ptr %9, align 8
  %11 = icmp eq ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %8
  call void @_ZdaPv(ptr noundef %10) #13
  br label %13

13:                                               ; preds = %12, %8
  %14 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 7
  %15 = load ptr, ptr %14, align 8
  %16 = icmp eq ptr %15, null
  br i1 %16, label %18, label %17

17:                                               ; preds = %13
  call void @_ZdaPv(ptr noundef %15) #13
  br label %18

18:                                               ; preds = %17, %13
  %19 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 8
  %20 = load ptr, ptr %19, align 8
  %21 = icmp ne ptr %20, null
  br i1 %21, label %22, label %28

22:                                               ; preds = %18
  %23 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 8
  %24 = load ptr, ptr %23, align 8
  %25 = icmp eq ptr %24, null
  br i1 %25, label %27, label %26

26:                                               ; preds = %22
  call void @_ZdaPv(ptr noundef %24) #13
  br label %27

27:                                               ; preds = %26, %22
  br label %28

28:                                               ; preds = %27, %18
  %29 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 9
  %30 = load ptr, ptr %29, align 8
  %31 = icmp ne ptr %30, null
  br i1 %31, label %32, label %38

32:                                               ; preds = %28
  %33 = getelementptr inbounds %class.Graph, ptr %3, i32 0, i32 9
  %34 = load ptr, ptr %33, align 8
  %35 = icmp eq ptr %34, null
  br i1 %35, label %37, label %36

36:                                               ; preds = %32
  call void @_ZdaPv(ptr noundef %34) #13
  br label %37

37:                                               ; preds = %36, %32
  br label %38

38:                                               ; preds = %37, %28
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN5Graph10buildGraphERKSt6vectorI6Edge_tSaIS1_EE(ptr noundef nonnull align 8 dereferenceable(56) %0, ptr noundef nonnull align 8 dereferenceable(24) %1) #5 align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %8 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca ptr, align 8
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %15 = load ptr, ptr %3, align 8
  %16 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 0
  %17 = load i32, ptr %16, align 8
  %18 = sext i32 %17 to i64
  %19 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %18, i64 4)
  %20 = extractvalue { i64, i1 } %19, 1
  %21 = extractvalue { i64, i1 } %19, 0
  %22 = select i1 %20, i64 -1, i64 %21
  %23 = call noalias noundef nonnull ptr @_Znam(i64 noundef %22) #12
  call void @llvm.memset.p0.i64(ptr align 4 %23, i8 0, i64 %22, i1 false)
  store ptr %23, ptr %5, align 8
  %24 = load ptr, ptr %4, align 8
  store ptr %24, ptr %6, align 8
  %25 = load ptr, ptr %6, align 8
  %26 = call ptr @_ZNKSt6vectorI6Edge_tSaIS0_EE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %25) #3
  %27 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %7, i32 0, i32 0
  store ptr %26, ptr %27, align 8
  %28 = load ptr, ptr %6, align 8
  %29 = call ptr @_ZNKSt6vectorI6Edge_tSaIS0_EE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %28) #3
  %30 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %8, i32 0, i32 0
  store ptr %29, ptr %30, align 8
  br label %31

31:                                               ; preds = %51, %2
  %32 = call noundef zeroext i1 @_ZN9__gnu_cxxneIPK6Edge_tSt6vectorIS1_SaIS1_EEEEbRKNS_17__normal_iteratorIT_T0_EESC_(ptr noundef nonnull align 8 dereferenceable(8) %7, ptr noundef nonnull align 8 dereferenceable(8) %8) #3
  br i1 %32, label %33, label %53

33:                                               ; preds = %31
  %34 = call noundef nonnull align 4 dereferenceable(12) ptr @_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %7) #3
  store ptr %34, ptr %9, align 8
  %35 = load ptr, ptr %5, align 8
  %36 = load ptr, ptr %9, align 8
  %37 = getelementptr inbounds %class.Edge_t, ptr %36, i32 0, i32 0
  %38 = load i32, ptr %37, align 4
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds i32, ptr %35, i64 %39
  %41 = load i32, ptr %40, align 4
  %42 = add nsw i32 %41, 1
  store i32 %42, ptr %40, align 4
  %43 = load ptr, ptr %5, align 8
  %44 = load ptr, ptr %9, align 8
  %45 = getelementptr inbounds %class.Edge_t, ptr %44, i32 0, i32 1
  %46 = load i32, ptr %45, align 4
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds i32, ptr %43, i64 %47
  %49 = load i32, ptr %48, align 4
  %50 = add nsw i32 %49, 1
  store i32 %50, ptr %48, align 4
  br label %51

51:                                               ; preds = %33
  %52 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %7) #3
  br label %31

53:                                               ; preds = %31
  %54 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 6
  %55 = load ptr, ptr %54, align 8
  %56 = getelementptr inbounds i32, ptr %55, i64 0
  store i32 0, ptr %56, align 4
  store i32 1, ptr %10, align 4
  br label %57

57:                                               ; preds = %99, %53
  %58 = load i32, ptr %10, align 4
  %59 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 0
  %60 = load i32, ptr %59, align 8
  %61 = icmp sle i32 %58, %60
  br i1 %61, label %62, label %102

62:                                               ; preds = %57
  %63 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 6
  %64 = load ptr, ptr %63, align 8
  %65 = load i32, ptr %10, align 4
  %66 = sub nsw i32 %65, 1
  %67 = sext i32 %66 to i64
  %68 = getelementptr inbounds i32, ptr %64, i64 %67
  %69 = load i32, ptr %68, align 4
  %70 = load ptr, ptr %5, align 8
  %71 = load i32, ptr %10, align 4
  %72 = sub nsw i32 %71, 1
  %73 = sext i32 %72 to i64
  %74 = getelementptr inbounds i32, ptr %70, i64 %73
  %75 = load i32, ptr %74, align 4
  %76 = add nsw i32 %69, %75
  %77 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 6
  %78 = load ptr, ptr %77, align 8
  %79 = load i32, ptr %10, align 4
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds i32, ptr %78, i64 %80
  store i32 %76, ptr %81, align 4
  %82 = load ptr, ptr %5, align 8
  %83 = load i32, ptr %10, align 4
  %84 = sub nsw i32 %83, 1
  %85 = sext i32 %84 to i64
  %86 = getelementptr inbounds i32, ptr %82, i64 %85
  %87 = load i32, ptr %86, align 4
  %88 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 5
  %89 = load ptr, ptr %88, align 8
  %90 = load i32, ptr %10, align 4
  %91 = sub nsw i32 %90, 1
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds i32, ptr %89, i64 %92
  store i32 %87, ptr %93, align 4
  %94 = load ptr, ptr %5, align 8
  %95 = load i32, ptr %10, align 4
  %96 = sub nsw i32 %95, 1
  %97 = sext i32 %96 to i64
  %98 = getelementptr inbounds i32, ptr %94, i64 %97
  store i32 0, ptr %98, align 4
  br label %99

99:                                               ; preds = %62
  %100 = load i32, ptr %10, align 4
  %101 = add nsw i32 %100, 1
  store i32 %101, ptr %10, align 4
  br label %57, !llvm.loop !10

102:                                              ; preds = %57
  store i32 0, ptr %11, align 4
  br label %103

103:                                              ; preds = %185, %102
  %104 = load i32, ptr %11, align 4
  %105 = load ptr, ptr %4, align 8
  %106 = call noundef i64 @_ZNKSt6vectorI6Edge_tSaIS0_EE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %105) #3
  %107 = trunc i64 %106 to i32
  %108 = icmp slt i32 %104, %107
  br i1 %108, label %109, label %188

109:                                              ; preds = %103
  %110 = load ptr, ptr %4, align 8
  %111 = load i32, ptr %11, align 4
  %112 = sext i32 %111 to i64
  %113 = call noundef nonnull align 4 dereferenceable(12) ptr @_ZNKSt6vectorI6Edge_tSaIS0_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %110, i64 noundef %112) #3
  store ptr %113, ptr %12, align 8
  %114 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 6
  %115 = load ptr, ptr %114, align 8
  %116 = load ptr, ptr %12, align 8
  %117 = getelementptr inbounds %class.Edge_t, ptr %116, i32 0, i32 0
  %118 = load i32, ptr %117, align 4
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds i32, ptr %115, i64 %119
  %121 = load i32, ptr %120, align 4
  %122 = load ptr, ptr %5, align 8
  %123 = load ptr, ptr %12, align 8
  %124 = getelementptr inbounds %class.Edge_t, ptr %123, i32 0, i32 0
  %125 = load i32, ptr %124, align 4
  %126 = sext i32 %125 to i64
  %127 = getelementptr inbounds i32, ptr %122, i64 %126
  %128 = load i32, ptr %127, align 4
  %129 = add nsw i32 %128, 1
  store i32 %129, ptr %127, align 4
  %130 = add nsw i32 %121, %128
  store i32 %130, ptr %13, align 4
  %131 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 6
  %132 = load ptr, ptr %131, align 8
  %133 = load ptr, ptr %12, align 8
  %134 = getelementptr inbounds %class.Edge_t, ptr %133, i32 0, i32 1
  %135 = load i32, ptr %134, align 4
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds i32, ptr %132, i64 %136
  %138 = load i32, ptr %137, align 4
  %139 = load ptr, ptr %5, align 8
  %140 = load ptr, ptr %12, align 8
  %141 = getelementptr inbounds %class.Edge_t, ptr %140, i32 0, i32 1
  %142 = load i32, ptr %141, align 4
  %143 = sext i32 %142 to i64
  %144 = getelementptr inbounds i32, ptr %139, i64 %143
  %145 = load i32, ptr %144, align 4
  %146 = add nsw i32 %145, 1
  store i32 %146, ptr %144, align 4
  %147 = add nsw i32 %138, %145
  store i32 %147, ptr %14, align 4
  %148 = load ptr, ptr %12, align 8
  %149 = getelementptr inbounds %class.Edge_t, ptr %148, i32 0, i32 1
  %150 = load i32, ptr %149, align 4
  %151 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 7
  %152 = load ptr, ptr %151, align 8
  %153 = load i32, ptr %13, align 4
  %154 = sext i32 %153 to i64
  %155 = getelementptr inbounds i32, ptr %152, i64 %154
  store i32 %150, ptr %155, align 4
  %156 = load ptr, ptr %12, align 8
  %157 = getelementptr inbounds %class.Edge_t, ptr %156, i32 0, i32 0
  %158 = load i32, ptr %157, align 4
  %159 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 7
  %160 = load ptr, ptr %159, align 8
  %161 = load i32, ptr %14, align 4
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds i32, ptr %160, i64 %162
  store i32 %158, ptr %163, align 4
  %164 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 3
  %165 = load i8, ptr %164, align 1
  %166 = trunc i8 %165 to i1
  br i1 %166, label %167, label %184

167:                                              ; preds = %109
  %168 = load ptr, ptr %12, align 8
  %169 = getelementptr inbounds %class.Edge_t, ptr %168, i32 0, i32 2
  %170 = load float, ptr %169, align 4
  %171 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 8
  %172 = load ptr, ptr %171, align 8
  %173 = load i32, ptr %13, align 4
  %174 = sext i32 %173 to i64
  %175 = getelementptr inbounds float, ptr %172, i64 %174
  store float %170, ptr %175, align 4
  %176 = load ptr, ptr %12, align 8
  %177 = getelementptr inbounds %class.Edge_t, ptr %176, i32 0, i32 2
  %178 = load float, ptr %177, align 4
  %179 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 8
  %180 = load ptr, ptr %179, align 8
  %181 = load i32, ptr %14, align 4
  %182 = sext i32 %181 to i64
  %183 = getelementptr inbounds float, ptr %180, i64 %182
  store float %178, ptr %183, align 4
  br label %184

184:                                              ; preds = %167, %109
  br label %185

185:                                              ; preds = %184
  %186 = load i32, ptr %11, align 4
  %187 = add nsw i32 %186, 1
  store i32 %187, ptr %11, align 4
  br label %103, !llvm.loop !11

188:                                              ; preds = %103
  %189 = load ptr, ptr %5, align 8
  %190 = icmp eq ptr %189, null
  br i1 %190, label %192, label %191

191:                                              ; preds = %188
  call void @_ZdaPv(ptr noundef %189) #13
  br label %192

192:                                              ; preds = %191, %188
  %193 = getelementptr inbounds %class.Graph, ptr %15, i32 0, i32 4
  %194 = load i8, ptr %193, align 2
  %195 = trunc i8 %194 to i1
  br i1 %195, label %196, label %198

196:                                              ; preds = %192
  %197 = call noundef zeroext i1 @_ZN5Graph16ensureStochasticEv(ptr noundef nonnull align 8 dereferenceable(56) %15)
  br label %198

198:                                              ; preds = %196, %192
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local ptr @_ZNKSt6vectorI6Edge_tSaIS0_EE5beginEv(ptr noundef nonnull align 8 dereferenceable(24) %0) #4 comdat align 2 {
  %2 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  %4 = load ptr, ptr %3, align 8
  %5 = getelementptr inbounds %"struct.std::_Vector_base", ptr %4, i32 0, i32 0
  %6 = getelementptr inbounds %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data", ptr %5, i32 0, i32 0
  call void @_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(8) %2, ptr noundef nonnull align 8 dereferenceable(8) %6) #3
  %7 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %2, i32 0, i32 0
  %8 = load ptr, ptr %7, align 8
  ret ptr %8
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local ptr @_ZNKSt6vectorI6Edge_tSaIS0_EE3endEv(ptr noundef nonnull align 8 dereferenceable(24) %0) #4 comdat align 2 {
  %2 = alloca %"class.__gnu_cxx::__normal_iterator", align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  %4 = load ptr, ptr %3, align 8
  %5 = getelementptr inbounds %"struct.std::_Vector_base", ptr %4, i32 0, i32 0
  %6 = getelementptr inbounds %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data", ptr %5, i32 0, i32 1
  call void @_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(8) %2, ptr noundef nonnull align 8 dereferenceable(8) %6) #3
  %7 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %2, i32 0, i32 0
  %8 = load ptr, ptr %7, align 8
  ret ptr %8
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZN9__gnu_cxxneIPK6Edge_tSt6vectorIS1_SaIS1_EEEEbRKNS_17__normal_iteratorIT_T0_EESC_(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef nonnull align 8 dereferenceable(8) %1) #4 comdat {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEE4baseEv(ptr noundef nonnull align 8 dereferenceable(8) %5) #3
  %7 = load ptr, ptr %6, align 8
  %8 = load ptr, ptr %4, align 8
  %9 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEE4baseEv(ptr noundef nonnull align 8 dereferenceable(8) %8) #3
  %10 = load ptr, ptr %9, align 8
  %11 = icmp ne ptr %7, %10
  ret i1 %11
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 4 dereferenceable(12) ptr @_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEdeEv(ptr noundef nonnull align 8 dereferenceable(8) %0) #4 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %3, i32 0, i32 0
  %5 = load ptr, ptr %4, align 8
  ret ptr %5
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) ptr @_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEppEv(ptr noundef nonnull align 8 dereferenceable(8) %0) #4 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %3, i32 0, i32 0
  %5 = load ptr, ptr %4, align 8
  %6 = getelementptr inbounds %class.Edge_t, ptr %5, i32 1
  store ptr %6, ptr %4, align 8
  ret ptr %3
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i64 @_ZNKSt6vectorI6Edge_tSaIS0_EE4sizeEv(ptr noundef nonnull align 8 dereferenceable(24) %0) #4 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %5 = getelementptr inbounds %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data", ptr %4, i32 0, i32 1
  %6 = load ptr, ptr %5, align 8
  %7 = getelementptr inbounds %"struct.std::_Vector_base", ptr %3, i32 0, i32 0
  %8 = getelementptr inbounds %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data", ptr %7, i32 0, i32 0
  %9 = load ptr, ptr %8, align 8
  %10 = ptrtoint ptr %6 to i64
  %11 = ptrtoint ptr %9 to i64
  %12 = sub i64 %10, %11
  %13 = sdiv exact i64 %12, 12
  ret i64 %13
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 4 dereferenceable(12) ptr @_ZNKSt6vectorI6Edge_tSaIS0_EEixEm(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1) #4 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = getelementptr inbounds %"struct.std::_Vector_base", ptr %5, i32 0, i32 0
  %7 = getelementptr inbounds %"struct.std::_Vector_base<Edge_t, std::allocator<Edge_t>>::_Vector_impl_data", ptr %6, i32 0, i32 0
  %8 = load ptr, ptr %7, align 8
  %9 = load i64, ptr %4, align 8
  %10 = getelementptr inbounds %class.Edge_t, ptr %8, i64 %9
  ret ptr %10
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef zeroext i1 @_ZN5Graph16ensureStochasticEv(ptr noundef nonnull align 8 dereferenceable(56) %0) #4 align 2 {
  %2 = alloca i1, align 1
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca float, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  %8 = load ptr, ptr %3, align 8
  %9 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 3
  %10 = load i8, ptr %9, align 1
  %11 = trunc i8 %10 to i1
  br i1 %11, label %13, label %12

12:                                               ; preds = %1
  store i1 false, ptr %2, align 1
  br label %91

13:                                               ; preds = %1
  store i32 0, ptr %4, align 4
  br label %14

14:                                               ; preds = %87, %13
  %15 = load i32, ptr %4, align 4
  %16 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 0
  %17 = load i32, ptr %16, align 8
  %18 = icmp slt i32 %15, %17
  br i1 %18, label %19, label %90

19:                                               ; preds = %14
  store float 0.000000e+00, ptr %5, align 4
  %20 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 6
  %21 = load ptr, ptr %20, align 8
  %22 = load i32, ptr %4, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds i32, ptr %21, i64 %23
  %25 = load i32, ptr %24, align 4
  store i32 %25, ptr %6, align 4
  br label %26

26:                                               ; preds = %45, %19
  %27 = load i32, ptr %6, align 4
  %28 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 6
  %29 = load ptr, ptr %28, align 8
  %30 = load i32, ptr %4, align 4
  %31 = add nsw i32 %30, 1
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds i32, ptr %29, i64 %32
  %34 = load i32, ptr %33, align 4
  %35 = icmp slt i32 %27, %34
  br i1 %35, label %36, label %48

36:                                               ; preds = %26
  %37 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 8
  %38 = load ptr, ptr %37, align 8
  %39 = load i32, ptr %6, align 4
  %40 = sext i32 %39 to i64
  %41 = getelementptr inbounds float, ptr %38, i64 %40
  %42 = load float, ptr %41, align 4
  %43 = load float, ptr %5, align 4
  %44 = fadd float %43, %42
  store float %44, ptr %5, align 4
  br label %45

45:                                               ; preds = %36
  %46 = load i32, ptr %6, align 4
  %47 = add nsw i32 %46, 1
  store i32 %47, ptr %6, align 4
  br label %26, !llvm.loop !12

48:                                               ; preds = %26
  %49 = load float, ptr %5, align 4
  %50 = fcmp oeq float %49, 0.000000e+00
  br i1 %50, label %51, label %52

51:                                               ; preds = %48
  br label %87

52:                                               ; preds = %48
  %53 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 6
  %54 = load ptr, ptr %53, align 8
  %55 = load i32, ptr %4, align 4
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds i32, ptr %54, i64 %56
  %58 = load i32, ptr %57, align 4
  store i32 %58, ptr %7, align 4
  br label %59

59:                                               ; preds = %83, %52
  %60 = load i32, ptr %7, align 4
  %61 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 6
  %62 = load ptr, ptr %61, align 8
  %63 = load i32, ptr %4, align 4
  %64 = add nsw i32 %63, 1
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds i32, ptr %62, i64 %65
  %67 = load i32, ptr %66, align 4
  %68 = icmp slt i32 %60, %67
  br i1 %68, label %69, label %86

69:                                               ; preds = %59
  %70 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 8
  %71 = load ptr, ptr %70, align 8
  %72 = load i32, ptr %7, align 4
  %73 = sext i32 %72 to i64
  %74 = getelementptr inbounds float, ptr %71, i64 %73
  %75 = load float, ptr %74, align 4
  %76 = load float, ptr %5, align 4
  %77 = fdiv float %75, %76
  %78 = getelementptr inbounds %class.Graph, ptr %8, i32 0, i32 9
  %79 = load ptr, ptr %78, align 8
  %80 = load i32, ptr %7, align 4
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds float, ptr %79, i64 %81
  store float %77, ptr %82, align 4
  br label %83

83:                                               ; preds = %69
  %84 = load i32, ptr %7, align 4
  %85 = add nsw i32 %84, 1
  store i32 %85, ptr %7, align 4
  br label %59, !llvm.loop !13

86:                                               ; preds = %59
  br label %87

87:                                               ; preds = %86, %51
  %88 = load i32, ptr %4, align 4
  %89 = add nsw i32 %88, 1
  store i32 %89, ptr %4, align 4
  br label %14, !llvm.loop !14

90:                                               ; preds = %14
  store i1 true, ptr %2, align 1
  br label %91

91:                                               ; preds = %90, %12
  %92 = load i1, ptr %2, align 1
  ret i1 %92
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEEC2ERKS3_(ptr noundef nonnull align 8 dereferenceable(8) %0, ptr noundef nonnull align 8 dereferenceable(8) %1) unnamed_addr #4 comdat align 2 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %5, i32 0, i32 0
  %7 = load ptr, ptr %4, align 8
  %8 = load ptr, ptr %7, align 8
  store ptr %8, ptr %6, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef nonnull align 8 dereferenceable(8) ptr @_ZNK9__gnu_cxx17__normal_iteratorIPK6Edge_tSt6vectorIS1_SaIS1_EEE4baseEv(ptr noundef nonnull align 8 dereferenceable(8) %0) #4 comdat align 2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %"class.__gnu_cxx::__normal_iterator", ptr %3, i32 0, i32 0
  ret ptr %4
}

; Function Attrs: noinline uwtable
define internal void @_GLOBAL__sub_I_graph.cpp() #0 section ".text.startup" {
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { noinline uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }
attributes #4 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #7 = { nobuiltin allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #9 = { nobuiltin nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { noreturn nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #11 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #12 = { builtin allocsize(0) }
attributes #13 = { builtin nounwind }
attributes #14 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
!12 = distinct !{!12, !7}
!13 = distinct !{!13, !7}
!14 = distinct !{!14, !7}

