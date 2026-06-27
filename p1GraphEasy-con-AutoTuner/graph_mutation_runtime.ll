; ModuleID = 'graph_mutation_runtime.c'
source_filename = "graph_mutation_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.timespec = type { i64, i64 }

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_node(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = alloca %struct.timespec, align 8
  %6 = alloca %struct.timespec, align 8
  %7 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %7) #6
  %8 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %7) #6
  %9 = load i64, ptr %7, align 8, !tbaa !3
  %10 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %11 = load i64, ptr %10, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %7) #6
  %12 = icmp eq ptr %0, null
  br i1 %12, label %13, label %18

13:                                               ; preds = %4
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #6
  %14 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #6
  %15 = load i64, ptr %6, align 8, !tbaa !3
  %16 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %17 = load i64, ptr %16, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #6
  br label %37

18:                                               ; preds = %4
  %19 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %20 = icmp eq i32 %19, 3
  br i1 %20, label %30, label %21

21:                                               ; preds = %18
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %22 = call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #6
  %23 = load i64, ptr %0, align 8, !tbaa !9
  %24 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %25 = load i64, ptr %24, align 8, !tbaa !14
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %27 = load ptr, ptr %26, align 8, !tbaa !15
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %29 = load ptr, ptr %28, align 8, !tbaa !16
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %23, i64 noundef %25, ptr noundef %27, ptr noundef %29, ptr noundef %1, ptr noundef null, ptr noundef %2, i32 noundef %19) #6
  br label %32

30:                                               ; preds = %18
  %31 = call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #6
  br label %32

32:                                               ; preds = %30, %21
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #6
  %33 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #6
  %34 = load i64, ptr %5, align 8, !tbaa !3
  %35 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %36 = load i64, ptr %35, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #6
  br label %37

37:                                               ; preds = %32, %13
  %38 = phi i64 [ %36, %32 ], [ %17, %13 ]
  %39 = phi i64 [ %34, %32 ], [ %15, %13 ]
  %40 = mul i64 %39, 1000000000
  %41 = mul i64 %9, -1000000000
  %42 = sub i64 %41, %11
  %43 = add i64 %38, %42
  %44 = add i64 %43, %40
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %44) #6
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local void @autograph_profile_record_kernel_ns(i32 noundef, i64 noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_get_layout(ptr noundef) local_unnamed_addr #2

declare dso_local void @autograph_ensure_layout_set(ptr noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_canonical_add_node(ptr noundef, i32 noundef) local_unnamed_addr #2

declare dso_local void @autograph_ensure_layout(ptr noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_node(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = alloca %struct.timespec, align 8
  %6 = alloca %struct.timespec, align 8
  %7 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %7) #6
  %8 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %7) #6
  %9 = load i64, ptr %7, align 8, !tbaa !3
  %10 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %11 = load i64, ptr %10, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %7) #6
  %12 = icmp eq ptr %0, null
  br i1 %12, label %13, label %18

13:                                               ; preds = %4
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #6
  %14 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #6
  %15 = load i64, ptr %6, align 8, !tbaa !3
  %16 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %17 = load i64, ptr %16, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #6
  br label %37

18:                                               ; preds = %4
  %19 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %20 = icmp eq i32 %19, 3
  br i1 %20, label %30, label %21

21:                                               ; preds = %18
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %22 = call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #6
  %23 = load i64, ptr %0, align 8, !tbaa !9
  %24 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %25 = load i64, ptr %24, align 8, !tbaa !14
  %26 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %27 = load ptr, ptr %26, align 8, !tbaa !15
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %29 = load ptr, ptr %28, align 8, !tbaa !16
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %23, i64 noundef %25, ptr noundef %27, ptr noundef %29, ptr noundef %1, ptr noundef null, ptr noundef %2, i32 noundef %19) #6
  br label %32

30:                                               ; preds = %18
  %31 = call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #6
  br label %32

32:                                               ; preds = %30, %21
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #6
  %33 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #6
  %34 = load i64, ptr %5, align 8, !tbaa !3
  %35 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %36 = load i64, ptr %35, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #6
  br label %37

37:                                               ; preds = %32, %13
  %38 = phi i64 [ %36, %32 ], [ %17, %13 ]
  %39 = phi i64 [ %34, %32 ], [ %15, %13 ]
  %40 = mul i64 %39, 1000000000
  %41 = mul i64 %9, -1000000000
  %42 = sub i64 %41, %11
  %43 = add i64 %38, %42
  %44 = add i64 %43, %40
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %44) #6
  ret void
}

declare dso_local i32 @autograph_canonical_remove_node(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_edge(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = alloca %struct.timespec, align 8
  %7 = alloca %struct.timespec, align 8
  %8 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %8) #6
  %9 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %8) #6
  %10 = load i64, ptr %8, align 8, !tbaa !3
  %11 = getelementptr inbounds nuw i8, ptr %8, i64 8
  %12 = load i64, ptr %11, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %8) #6
  %13 = icmp eq ptr %0, null
  br i1 %13, label %14, label %19

14:                                               ; preds = %5
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %7) #6
  %15 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %7) #6
  %16 = load i64, ptr %7, align 8, !tbaa !3
  %17 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %18 = load i64, ptr %17, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %7) #6
  br label %159

19:                                               ; preds = %5
  %20 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %21 = icmp sgt i32 %2, -1
  br i1 %21, label %22, label %31

22:                                               ; preds = %19
  %23 = zext nneg i32 %2 to i64
  %24 = load i64, ptr %0, align 8, !tbaa !9
  %25 = icmp sgt i64 %24, %23
  %26 = icmp sgt i32 %3, -1
  %27 = and i1 %26, %25
  br i1 %27, label %28, label %31

28:                                               ; preds = %22
  %29 = zext nneg i32 %3 to i64
  %30 = icmp samesign ugt i64 %24, %29
  br label %31

31:                                               ; preds = %28, %22, %19
  %32 = phi i1 [ false, %22 ], [ false, %19 ], [ %30, %28 ]
  switch i32 %20, label %152 [
    i32 0, label %33
    i32 1, label %52
    i32 2, label %141
  ]

33:                                               ; preds = %31
  %34 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %35 = load i64, ptr %34, align 8, !tbaa !14
  call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %2, i32 noundef %3)
  call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %3, i32 noundef %2)
  %36 = load i64, ptr %0, align 8, !tbaa !9
  %37 = load i64, ptr %34, align 8, !tbaa !14
  %38 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %39 = load ptr, ptr %38, align 8, !tbaa !15
  %40 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %41 = load ptr, ptr %40, align 8, !tbaa !16
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %36, i64 noundef %37, ptr noundef %39, ptr noundef %41) #6
  %42 = load i64, ptr %34, align 8, !tbaa !14
  %43 = icmp eq i64 %42, %35
  br i1 %43, label %45, label %44

44:                                               ; preds = %33
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %45

45:                                               ; preds = %44, %33
  br i1 %32, label %154, label %46

46:                                               ; preds = %45
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %47 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %48 = load i64, ptr %0, align 8, !tbaa !9
  %49 = load i64, ptr %34, align 8, !tbaa !14
  %50 = load ptr, ptr %38, align 8, !tbaa !15
  %51 = load ptr, ptr %40, align 8, !tbaa !16
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %48, i64 noundef %49, ptr noundef %50, ptr noundef %51, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 0) #6
  br label %154

52:                                               ; preds = %31
  %53 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %54 = load ptr, ptr %53, align 8, !tbaa !15
  %55 = icmp eq ptr %54, null
  br i1 %55, label %56, label %60

56:                                               ; preds = %52
  %57 = load i64, ptr %0, align 8, !tbaa !9
  %58 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %59 = load ptr, ptr %58, align 8, !tbaa !16
  br label %122

60:                                               ; preds = %52
  %61 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %62 = load ptr, ptr %61, align 8, !tbaa !16
  %63 = icmp eq ptr %62, null
  %64 = icmp slt i32 %2, 0
  %65 = or i1 %64, %63
  %66 = load i64, ptr %0, align 8, !tbaa !9
  br i1 %65, label %93, label %67

67:                                               ; preds = %60
  %68 = zext nneg i32 %2 to i64
  %69 = icmp sgt i64 %66, %68
  br i1 %69, label %70, label %93

70:                                               ; preds = %67
  %71 = icmp sgt i32 %3, -1
  %72 = zext nneg i32 %3 to i64
  %73 = icmp samesign ugt i64 %66, %72
  %74 = select i1 %71, i1 %73, i1 false
  br i1 %74, label %75, label %93

75:                                               ; preds = %70
  %76 = getelementptr inbounds nuw i64, ptr %54, i64 %68
  %77 = load i64, ptr %76, align 8, !tbaa !17
  %78 = getelementptr inbounds nuw i8, ptr %76, i64 8
  %79 = load i64, ptr %78, align 8, !tbaa !17
  %80 = icmp slt i64 %77, %79
  br i1 %80, label %81, label %93

81:                                               ; preds = %75, %90
  %82 = phi i64 [ %91, %90 ], [ %77, %75 ]
  %83 = getelementptr inbounds i32, ptr %62, i64 %82
  %84 = load i32, ptr %83, align 4, !tbaa !18
  %85 = icmp eq i32 %84, %3
  br i1 %85, label %93, label %86

86:                                               ; preds = %81
  %87 = icmp eq i32 %84, -1
  br i1 %87, label %88, label %90

88:                                               ; preds = %86
  %89 = getelementptr inbounds i32, ptr %62, i64 %82
  store i32 %3, ptr %89, align 4, !tbaa !18
  br label %93

90:                                               ; preds = %86
  %91 = add i64 %82, 1
  %92 = icmp eq i64 %91, %79
  br i1 %92, label %93, label %81, !llvm.loop !20

93:                                               ; preds = %90, %81, %88, %75, %70, %67, %60
  %94 = phi i1 [ false, %75 ], [ false, %70 ], [ false, %67 ], [ false, %60 ], [ true, %88 ], [ false, %81 ], [ false, %90 ]
  %95 = icmp slt i32 %3, 0
  %96 = or i1 %95, %63
  br i1 %96, label %122, label %97

97:                                               ; preds = %93
  %98 = zext nneg i32 %3 to i64
  %99 = icmp sgt i64 %66, %98
  %100 = zext nneg i32 %2 to i64
  %101 = icmp samesign ugt i64 %66, %100
  %102 = and i1 %21, %99
  %103 = select i1 %102, i1 %101, i1 false
  br i1 %103, label %104, label %122

104:                                              ; preds = %97
  %105 = getelementptr inbounds nuw i64, ptr %54, i64 %98
  %106 = load i64, ptr %105, align 8, !tbaa !17
  %107 = getelementptr inbounds nuw i8, ptr %105, i64 8
  %108 = load i64, ptr %107, align 8, !tbaa !17
  %109 = icmp slt i64 %106, %108
  br i1 %109, label %110, label %122

110:                                              ; preds = %104, %117
  %111 = phi i64 [ %118, %117 ], [ %106, %104 ]
  %112 = getelementptr inbounds i32, ptr %62, i64 %111
  %113 = load i32, ptr %112, align 4, !tbaa !18
  %114 = icmp eq i32 %113, %2
  br i1 %114, label %122, label %115

115:                                              ; preds = %110
  %116 = icmp eq i32 %113, -1
  br i1 %116, label %120, label %117

117:                                              ; preds = %115
  %118 = add i64 %111, 1
  %119 = icmp eq i64 %118, %108
  br i1 %119, label %122, label %110, !llvm.loop !20

120:                                              ; preds = %115
  %121 = getelementptr inbounds i32, ptr %62, i64 %111
  store i32 %2, ptr %121, align 4, !tbaa !18
  br i1 %94, label %127, label %122

122:                                              ; preds = %110, %117, %120, %93, %97, %104, %56
  %123 = phi ptr [ %59, %56 ], [ %62, %104 ], [ %62, %97 ], [ %62, %93 ], [ %62, %120 ], [ %62, %117 ], [ %62, %110 ]
  %124 = phi i64 [ %57, %56 ], [ %66, %104 ], [ %66, %97 ], [ %66, %93 ], [ %66, %120 ], [ %66, %117 ], [ %66, %110 ]
  %125 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %126 = load i64, ptr %125, align 8, !tbaa !14
  call void @autograph_record_adjacency_state(ptr noundef %0, i64 noundef %124, i64 noundef %126, ptr noundef %54, ptr noundef %123) #6
  br label %132

127:                                              ; preds = %120
  %128 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %129 = load i64, ptr %128, align 8, !tbaa !14
  %130 = add nsw i64 %129, 2
  store i64 %130, ptr %128, align 8, !tbaa !14
  %131 = getelementptr inbounds nuw i8, ptr %0, i64 8
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %66, i64 noundef %130, ptr noundef nonnull %54, ptr noundef nonnull %62) #6
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %132

132:                                              ; preds = %122, %127
  %133 = phi ptr [ %125, %122 ], [ %131, %127 ]
  br i1 %32, label %154, label %134

134:                                              ; preds = %132
  %135 = getelementptr inbounds nuw i8, ptr %0, i64 24
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %136 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %137 = load i64, ptr %0, align 8, !tbaa !9
  %138 = load i64, ptr %133, align 8, !tbaa !14
  %139 = load ptr, ptr %53, align 8, !tbaa !15
  %140 = load ptr, ptr %135, align 8, !tbaa !16
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %137, i64 noundef %138, ptr noundef %139, ptr noundef %140, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 1) #6
  br label %154

141:                                              ; preds = %31
  %142 = call i32 @autograph_bcsr_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %143 = call i32 @autograph_bcsr_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  %144 = icmp sgt i32 %142, 0
  %145 = icmp sgt i32 %143, 0
  %146 = select i1 %144, i1 %145, i1 false
  br i1 %146, label %147, label %148

147:                                              ; preds = %141
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %148

148:                                              ; preds = %147, %141
  br i1 %32, label %154, label %149

149:                                              ; preds = %148
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %150 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %151 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  br label %154

152:                                              ; preds = %31
  %153 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  br label %154

154:                                              ; preds = %152, %46, %45, %134, %132, %149, %148
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #6
  %155 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #6
  %156 = load i64, ptr %6, align 8, !tbaa !3
  %157 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %158 = load i64, ptr %157, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #6
  br label %159

159:                                              ; preds = %154, %14
  %160 = phi i64 [ %158, %154 ], [ %18, %14 ]
  %161 = phi i64 [ %156, %154 ], [ %16, %14 ]
  %162 = mul i64 %161, 1000000000
  %163 = mul i64 %10, -1000000000
  %164 = sub i64 %163, %12
  %165 = add i64 %160, %164
  %166 = add i64 %165, %162
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %166) #6
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @csr_add_directed(ptr nocapture noundef nonnull %0, ptr noundef nonnull %1, i32 noundef %2, i32 noundef %3) unnamed_addr #0 {
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !15
  %7 = icmp eq ptr %6, null
  br i1 %7, label %57, label %8

8:                                                ; preds = %4
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %10 = load ptr, ptr %9, align 8, !tbaa !16
  %11 = icmp eq ptr %10, null
  %12 = icmp slt i32 %2, 0
  %13 = or i1 %12, %11
  br i1 %13, label %57, label %14

14:                                               ; preds = %8
  %15 = zext nneg i32 %2 to i64
  %16 = load i64, ptr %0, align 8, !tbaa !9
  %17 = icmp sgt i64 %16, %15
  %18 = icmp sgt i32 %3, -1
  %19 = and i1 %18, %17
  %20 = zext nneg i32 %3 to i64
  %21 = icmp sgt i64 %16, %20
  %22 = select i1 %19, i1 %21, i1 false
  br i1 %22, label %23, label %57

23:                                               ; preds = %14
  %24 = getelementptr inbounds nuw i64, ptr %6, i64 %15
  %25 = getelementptr inbounds nuw i8, ptr %24, i64 8
  %26 = load i64, ptr %25, align 8, !tbaa !17
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %28 = load i64, ptr %27, align 8, !tbaa !14
  %29 = add nsw i64 %28, 1
  %30 = shl i64 %29, 2
  %31 = tail call ptr @realloc(ptr noundef nonnull %10, i64 noundef %30) #7
  %32 = icmp eq ptr %31, null
  br i1 %32, label %57, label %33

33:                                               ; preds = %23
  store ptr %31, ptr %9, align 8, !tbaa !16
  %34 = load ptr, ptr %5, align 8, !tbaa !15
  tail call void @autograph_update_csr_pointers(ptr noundef nonnull %1, ptr noundef %34, ptr noundef nonnull %31) #6
  %35 = icmp slt i64 %26, %28
  br i1 %35, label %36, label %42

36:                                               ; preds = %33
  %37 = load ptr, ptr %9, align 8, !tbaa !16
  %38 = getelementptr i32, ptr %37, i64 %26
  %39 = getelementptr i8, ptr %38, i64 4
  %40 = sub nsw i64 %28, %26
  %41 = shl i64 %40, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %39, ptr align 4 %38, i64 %41, i1 false)
  br label %42

42:                                               ; preds = %36, %33
  %43 = load ptr, ptr %9, align 8, !tbaa !16
  %44 = getelementptr inbounds i32, ptr %43, i64 %26
  store i32 %3, ptr %44, align 4, !tbaa !18
  store i64 %29, ptr %27, align 8, !tbaa !14
  %45 = load i64, ptr %0, align 8, !tbaa !9
  %46 = icmp sgt i64 %45, %15
  br i1 %46, label %47, label %57

47:                                               ; preds = %42
  %48 = load ptr, ptr %5, align 8, !tbaa !15
  br label %49

49:                                               ; preds = %47, %49
  %50 = phi i64 [ %15, %47 ], [ %51, %49 ]
  %51 = add nuw nsw i64 %50, 1
  %52 = getelementptr inbounds nuw i64, ptr %48, i64 %51
  %53 = load i64, ptr %52, align 8, !tbaa !17
  %54 = add nsw i64 %53, 1
  store i64 %54, ptr %52, align 8, !tbaa !17
  %55 = load i64, ptr %0, align 8, !tbaa !9
  %56 = icmp slt i64 %51, %55
  br i1 %56, label %49, label %57, !llvm.loop !22

57:                                               ; preds = %49, %42, %23, %14, %4, %8
  ret void
}

declare dso_local void @autograph_record_adjacency_state(ptr noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare dso_local void @autograph_mark_canonical_dirty(ptr noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_canonical_add_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_bcsr_add_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_edge(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = alloca %struct.timespec, align 8
  %7 = alloca %struct.timespec, align 8
  %8 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %8) #6
  %9 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %8) #6
  %10 = load i64, ptr %8, align 8, !tbaa !3
  %11 = getelementptr inbounds nuw i8, ptr %8, i64 8
  %12 = load i64, ptr %11, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %8) #6
  %13 = icmp eq ptr %0, null
  br i1 %13, label %14, label %19

14:                                               ; preds = %5
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %7) #6
  %15 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %7) #6
  %16 = load i64, ptr %7, align 8, !tbaa !3
  %17 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %18 = load i64, ptr %17, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %7) #6
  br label %262

19:                                               ; preds = %5
  %20 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %21 = icmp sgt i32 %2, -1
  br i1 %21, label %22, label %31

22:                                               ; preds = %19
  %23 = zext nneg i32 %2 to i64
  %24 = load i64, ptr %0, align 8, !tbaa !9
  %25 = icmp sgt i64 %24, %23
  %26 = icmp sgt i32 %3, -1
  %27 = and i1 %26, %25
  br i1 %27, label %28, label %31

28:                                               ; preds = %22
  %29 = zext nneg i32 %3 to i64
  %30 = icmp samesign ugt i64 %24, %29
  br label %31

31:                                               ; preds = %28, %22, %19
  %32 = phi i1 [ false, %22 ], [ false, %19 ], [ %30, %28 ]
  switch i32 %20, label %255 [
    i32 0, label %33
    i32 1, label %159
    i32 2, label %244
  ]

33:                                               ; preds = %31
  %34 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %35 = load i64, ptr %34, align 8, !tbaa !14
  %36 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %37 = load ptr, ptr %36, align 8, !tbaa !15
  %38 = icmp eq ptr %37, null
  br i1 %38, label %143, label %39

39:                                               ; preds = %33
  %40 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %41 = load ptr, ptr %40, align 8, !tbaa !16
  %42 = icmp eq ptr %41, null
  %43 = icmp slt i32 %2, 0
  %44 = or i1 %43, %42
  br i1 %44, label %92, label %45

45:                                               ; preds = %39
  %46 = zext nneg i32 %2 to i64
  %47 = load i64, ptr %0, align 8, !tbaa !9
  %48 = icmp sgt i64 %47, %46
  br i1 %48, label %49, label %92

49:                                               ; preds = %45
  %50 = getelementptr inbounds nuw i64, ptr %37, i64 %46
  %51 = load i64, ptr %50, align 8, !tbaa !17
  %52 = getelementptr inbounds nuw i8, ptr %50, i64 8
  %53 = load i64, ptr %52, align 8, !tbaa !17
  %54 = icmp slt i64 %51, %53
  br i1 %54, label %55, label %92

55:                                               ; preds = %49, %60
  %56 = phi i64 [ %61, %60 ], [ %51, %49 ]
  %57 = getelementptr inbounds i32, ptr %41, i64 %56
  %58 = load i32, ptr %57, align 4, !tbaa !18
  %59 = icmp eq i32 %58, %3
  br i1 %59, label %63, label %60

60:                                               ; preds = %55
  %61 = add i64 %56, 1
  %62 = icmp eq i64 %61, %53
  br i1 %62, label %89, label %55, !llvm.loop !23

63:                                               ; preds = %55
  %64 = getelementptr inbounds i32, ptr %41, i64 %56
  %65 = icmp slt i64 %56, 0
  br i1 %65, label %89, label %66

66:                                               ; preds = %63
  %67 = add nuw nsw i64 %56, 1
  %68 = icmp slt i64 %67, %35
  br i1 %68, label %69, label %76

69:                                               ; preds = %66
  %70 = getelementptr inbounds nuw i32, ptr %41, i64 %67
  %71 = xor i64 %56, -1
  %72 = add nsw i64 %35, %71
  %73 = shl i64 %72, 2
  call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %64, ptr nonnull align 4 %70, i64 %73, i1 false)
  %74 = load i64, ptr %0, align 8, !tbaa !9
  %75 = load ptr, ptr %36, align 8, !tbaa !15
  br label %76

76:                                               ; preds = %69, %66
  %77 = phi ptr [ %75, %69 ], [ %37, %66 ]
  %78 = phi i64 [ %74, %69 ], [ %47, %66 ]
  %79 = add nsw i64 %35, -1
  store i64 %79, ptr %34, align 8, !tbaa !14
  %80 = icmp sgt i64 %78, %46
  br i1 %80, label %81, label %89

81:                                               ; preds = %76, %81
  %82 = phi i64 [ %83, %81 ], [ %46, %76 ]
  %83 = add nuw nsw i64 %82, 1
  %84 = getelementptr inbounds nuw i64, ptr %77, i64 %83
  %85 = load i64, ptr %84, align 8, !tbaa !17
  %86 = add nsw i64 %85, -1
  store i64 %86, ptr %84, align 8, !tbaa !17
  %87 = load i64, ptr %0, align 8, !tbaa !9
  %88 = icmp slt i64 %83, %87
  br i1 %88, label %81, label %92, !llvm.loop !24

89:                                               ; preds = %60, %63, %76
  %90 = phi ptr [ %37, %63 ], [ %77, %76 ], [ %37, %60 ]
  %91 = icmp eq ptr %90, null
  br i1 %91, label %143, label %92

92:                                               ; preds = %81, %49, %45, %39, %89
  %93 = phi ptr [ %90, %89 ], [ %37, %49 ], [ %37, %45 ], [ %37, %39 ], [ %77, %81 ]
  %94 = load ptr, ptr %40, align 8, !tbaa !16
  %95 = icmp eq ptr %94, null
  %96 = icmp slt i32 %3, 0
  %97 = or i1 %96, %95
  br i1 %97, label %143, label %98

98:                                               ; preds = %92
  %99 = zext nneg i32 %3 to i64
  %100 = load i64, ptr %0, align 8, !tbaa !9
  %101 = icmp sgt i64 %100, %99
  br i1 %101, label %102, label %143

102:                                              ; preds = %98
  %103 = getelementptr inbounds nuw i64, ptr %93, i64 %99
  %104 = load i64, ptr %103, align 8, !tbaa !17
  %105 = getelementptr inbounds nuw i8, ptr %103, i64 8
  %106 = load i64, ptr %105, align 8, !tbaa !17
  %107 = icmp slt i64 %104, %106
  br i1 %107, label %108, label %143

108:                                              ; preds = %102, %113
  %109 = phi i64 [ %114, %113 ], [ %104, %102 ]
  %110 = getelementptr inbounds i32, ptr %94, i64 %109
  %111 = load i32, ptr %110, align 4, !tbaa !18
  %112 = icmp eq i32 %111, %2
  br i1 %112, label %116, label %113

113:                                              ; preds = %108
  %114 = add i64 %109, 1
  %115 = icmp eq i64 %114, %106
  br i1 %115, label %143, label %108, !llvm.loop !23

116:                                              ; preds = %108
  %117 = getelementptr inbounds i32, ptr %94, i64 %109
  %118 = icmp slt i64 %109, 0
  br i1 %118, label %143, label %119

119:                                              ; preds = %116
  %120 = load i64, ptr %34, align 8, !tbaa !14
  %121 = add nuw nsw i64 %109, 1
  %122 = icmp slt i64 %121, %120
  br i1 %122, label %123, label %130

123:                                              ; preds = %119
  %124 = getelementptr inbounds nuw i32, ptr %94, i64 %121
  %125 = xor i64 %109, -1
  %126 = add nsw i64 %120, %125
  %127 = shl i64 %126, 2
  call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %117, ptr nonnull align 4 %124, i64 %127, i1 false)
  %128 = load i64, ptr %0, align 8, !tbaa !9
  %129 = load ptr, ptr %36, align 8, !tbaa !15
  br label %130

130:                                              ; preds = %123, %119
  %131 = phi ptr [ %129, %123 ], [ %93, %119 ]
  %132 = phi i64 [ %128, %123 ], [ %100, %119 ]
  %133 = add nsw i64 %120, -1
  store i64 %133, ptr %34, align 8, !tbaa !14
  %134 = icmp sgt i64 %132, %99
  br i1 %134, label %135, label %143

135:                                              ; preds = %130, %135
  %136 = phi i64 [ %137, %135 ], [ %99, %130 ]
  %137 = add nuw nsw i64 %136, 1
  %138 = getelementptr inbounds nuw i64, ptr %131, i64 %137
  %139 = load i64, ptr %138, align 8, !tbaa !17
  %140 = add nsw i64 %139, -1
  store i64 %140, ptr %138, align 8, !tbaa !17
  %141 = load i64, ptr %0, align 8, !tbaa !9
  %142 = icmp slt i64 %137, %141
  br i1 %142, label %135, label %143, !llvm.loop !24

143:                                              ; preds = %113, %135, %33, %89, %92, %98, %102, %116, %130
  %144 = phi ptr [ null, %33 ], [ null, %89 ], [ %93, %92 ], [ %93, %98 ], [ %93, %102 ], [ %93, %116 ], [ %131, %130 ], [ %131, %135 ], [ %93, %113 ]
  %145 = load i64, ptr %0, align 8, !tbaa !9
  %146 = load i64, ptr %34, align 8, !tbaa !14
  %147 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %148 = load ptr, ptr %147, align 8, !tbaa !16
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %145, i64 noundef %146, ptr noundef %144, ptr noundef %148) #6
  %149 = load i64, ptr %34, align 8, !tbaa !14
  %150 = icmp eq i64 %149, %35
  br i1 %150, label %152, label %151

151:                                              ; preds = %143
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %152

152:                                              ; preds = %151, %143
  br i1 %32, label %257, label %153

153:                                              ; preds = %152
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %154 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %155 = load i64, ptr %0, align 8, !tbaa !9
  %156 = load i64, ptr %34, align 8, !tbaa !14
  %157 = load ptr, ptr %36, align 8, !tbaa !15
  %158 = load ptr, ptr %147, align 8, !tbaa !16
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %155, i64 noundef %156, ptr noundef %157, ptr noundef %158, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 0) #6
  br label %257

159:                                              ; preds = %31
  %160 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %161 = load ptr, ptr %160, align 8, !tbaa !15
  %162 = icmp eq ptr %161, null
  br i1 %162, label %163, label %167

163:                                              ; preds = %159
  %164 = load i64, ptr %0, align 8, !tbaa !9
  %165 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %166 = load ptr, ptr %165, align 8, !tbaa !16
  br label %225

167:                                              ; preds = %159
  %168 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %169 = load ptr, ptr %168, align 8, !tbaa !16
  %170 = icmp eq ptr %169, null
  %171 = icmp slt i32 %2, 0
  %172 = or i1 %171, %170
  %173 = load i64, ptr %0, align 8, !tbaa !9
  br i1 %172, label %198, label %174

174:                                              ; preds = %167
  %175 = zext nneg i32 %2 to i64
  %176 = icmp sgt i64 %173, %175
  br i1 %176, label %177, label %198

177:                                              ; preds = %174
  %178 = icmp sgt i32 %3, -1
  %179 = zext nneg i32 %3 to i64
  %180 = icmp samesign ugt i64 %173, %179
  %181 = select i1 %178, i1 %180, i1 false
  br i1 %181, label %182, label %198

182:                                              ; preds = %177
  %183 = getelementptr inbounds nuw i64, ptr %161, i64 %175
  %184 = load i64, ptr %183, align 8, !tbaa !17
  %185 = getelementptr inbounds nuw i8, ptr %183, i64 8
  %186 = load i64, ptr %185, align 8, !tbaa !17
  %187 = icmp slt i64 %184, %186
  br i1 %187, label %188, label %198

188:                                              ; preds = %182, %195
  %189 = phi i64 [ %196, %195 ], [ %184, %182 ]
  %190 = getelementptr inbounds i32, ptr %169, i64 %189
  %191 = load i32, ptr %190, align 4, !tbaa !18
  %192 = icmp eq i32 %191, %3
  br i1 %192, label %193, label %195

193:                                              ; preds = %188
  %194 = getelementptr inbounds i32, ptr %169, i64 %189
  store i32 -1, ptr %194, align 4, !tbaa !18
  br label %198

195:                                              ; preds = %188
  %196 = add i64 %189, 1
  %197 = icmp eq i64 %196, %186
  br i1 %197, label %198, label %188, !llvm.loop !25

198:                                              ; preds = %195, %193, %182, %177, %174, %167
  %199 = phi i1 [ true, %167 ], [ true, %174 ], [ true, %177 ], [ true, %182 ], [ false, %193 ], [ true, %195 ]
  %200 = icmp slt i32 %3, 0
  %201 = or i1 %200, %170
  br i1 %201, label %225, label %202

202:                                              ; preds = %198
  %203 = zext nneg i32 %3 to i64
  %204 = icmp sgt i64 %173, %203
  %205 = zext nneg i32 %2 to i64
  %206 = icmp samesign ugt i64 %173, %205
  %207 = and i1 %21, %204
  %208 = select i1 %207, i1 %206, i1 false
  br i1 %208, label %209, label %225

209:                                              ; preds = %202
  %210 = getelementptr inbounds nuw i64, ptr %161, i64 %203
  %211 = load i64, ptr %210, align 8, !tbaa !17
  %212 = getelementptr inbounds nuw i8, ptr %210, i64 8
  %213 = load i64, ptr %212, align 8, !tbaa !17
  %214 = icmp slt i64 %211, %213
  br i1 %214, label %215, label %225

215:                                              ; preds = %209, %220
  %216 = phi i64 [ %221, %220 ], [ %211, %209 ]
  %217 = getelementptr inbounds i32, ptr %169, i64 %216
  %218 = load i32, ptr %217, align 4, !tbaa !18
  %219 = icmp eq i32 %218, %2
  br i1 %219, label %223, label %220

220:                                              ; preds = %215
  %221 = add i64 %216, 1
  %222 = icmp eq i64 %221, %213
  br i1 %222, label %225, label %215, !llvm.loop !25

223:                                              ; preds = %215
  %224 = getelementptr inbounds i32, ptr %169, i64 %216
  store i32 -1, ptr %224, align 4, !tbaa !18
  br i1 %199, label %225, label %230

225:                                              ; preds = %220, %223, %198, %202, %209, %163
  %226 = phi ptr [ %166, %163 ], [ %169, %209 ], [ %169, %202 ], [ %169, %198 ], [ %169, %223 ], [ %169, %220 ]
  %227 = phi i64 [ %164, %163 ], [ %173, %209 ], [ %173, %202 ], [ %173, %198 ], [ %173, %223 ], [ %173, %220 ]
  %228 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %229 = load i64, ptr %228, align 8, !tbaa !14
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %227, i64 noundef %229, ptr noundef %161, ptr noundef %226) #6
  br label %235

230:                                              ; preds = %223
  %231 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %232 = load i64, ptr %231, align 8, !tbaa !14
  %233 = add nsw i64 %232, -2
  store i64 %233, ptr %231, align 8, !tbaa !14
  %234 = getelementptr inbounds nuw i8, ptr %0, i64 8
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %173, i64 noundef %233, ptr noundef nonnull %161, ptr noundef nonnull %169) #6
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %235

235:                                              ; preds = %225, %230
  %236 = phi ptr [ %228, %225 ], [ %234, %230 ]
  br i1 %32, label %257, label %237

237:                                              ; preds = %235
  %238 = getelementptr inbounds nuw i8, ptr %0, i64 24
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %239 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %240 = load i64, ptr %0, align 8, !tbaa !9
  %241 = load i64, ptr %236, align 8, !tbaa !14
  %242 = load ptr, ptr %160, align 8, !tbaa !15
  %243 = load ptr, ptr %238, align 8, !tbaa !16
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %240, i64 noundef %241, ptr noundef %242, ptr noundef %243, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 1) #6
  br label %257

244:                                              ; preds = %31
  %245 = call i32 @autograph_bcsr_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %246 = call i32 @autograph_bcsr_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  %247 = icmp sgt i32 %245, 0
  %248 = icmp sgt i32 %246, 0
  %249 = select i1 %247, i1 %248, i1 false
  br i1 %249, label %250, label %251

250:                                              ; preds = %244
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %251

251:                                              ; preds = %250, %244
  br i1 %32, label %257, label %252

252:                                              ; preds = %251
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %253 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %254 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  br label %257

255:                                              ; preds = %31
  %256 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  br label %257

257:                                              ; preds = %255, %153, %152, %237, %235, %252, %251
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #6
  %258 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #6
  %259 = load i64, ptr %6, align 8, !tbaa !3
  %260 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %261 = load i64, ptr %260, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #6
  br label %262

262:                                              ; preds = %257, %14
  %263 = phi i64 [ %261, %257 ], [ %18, %14 ]
  %264 = phi i64 [ %259, %257 ], [ %16, %14 ]
  %265 = mul i64 %264, 1000000000
  %266 = mul i64 %10, -1000000000
  %267 = sub i64 %266, %12
  %268 = add i64 %263, %267
  %269 = add i64 %268, %265
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %269) #6
  ret void
}

declare dso_local i32 @autograph_canonical_remove_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_bcsr_remove_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind
declare dso_local i32 @clock_gettime(i32 noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #4

declare dso_local void @autograph_update_csr_pointers(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #6 = { nounwind }
attributes #7 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"timespec", !5, i64 0, !5, i64 8}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!4, !5, i64 8}
!9 = !{!10, !5, i64 0}
!10 = !{!"", !5, i64 0, !5, i64 8, !11, i64 16, !13, i64 24}
!11 = !{!"p1 long", !12, i64 0}
!12 = !{!"any pointer", !6, i64 0}
!13 = !{!"p1 int", !12, i64 0}
!14 = !{!10, !5, i64 8}
!15 = !{!10, !11, i64 16}
!16 = !{!10, !13, i64 24}
!17 = !{!5, !5, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !6, i64 0}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.mustprogress"}
!22 = distinct !{!22, !21}
!23 = distinct !{!23, !21}
!24 = distinct !{!24, !21}
!25 = distinct !{!25, !21}
