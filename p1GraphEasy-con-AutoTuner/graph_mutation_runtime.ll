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
  br label %39

18:                                               ; preds = %4
  call void @graph_ensure_owned_storage(ptr noundef nonnull %0) #6
  %19 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %20 = load ptr, ptr %19, align 8, !tbaa !9
  %21 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %22 = load ptr, ptr %21, align 8, !tbaa !14
  call void @autograph_update_csr_pointers(ptr noundef nonnull %0, ptr noundef %20, ptr noundef %22) #6
  %23 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %24 = icmp eq i32 %23, 3
  br i1 %24, label %32, label %25

25:                                               ; preds = %18
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %26 = call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #6
  %27 = load i64, ptr %0, align 8, !tbaa !15
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %29 = load i64, ptr %28, align 8, !tbaa !16
  %30 = load ptr, ptr %19, align 8, !tbaa !9
  %31 = load ptr, ptr %21, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %27, i64 noundef %29, ptr noundef %30, ptr noundef %31, ptr noundef %1, ptr noundef null, ptr noundef %2, i32 noundef %23) #6
  br label %34

32:                                               ; preds = %18
  %33 = call i32 @autograph_canonical_add_node(ptr noundef nonnull %0, i32 noundef %3) #6
  br label %34

34:                                               ; preds = %32, %25
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #6
  %35 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #6
  %36 = load i64, ptr %5, align 8, !tbaa !3
  %37 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %38 = load i64, ptr %37, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #6
  br label %39

39:                                               ; preds = %34, %13
  %40 = phi i64 [ %38, %34 ], [ %17, %13 ]
  %41 = phi i64 [ %36, %34 ], [ %15, %13 ]
  %42 = mul i64 %41, 1000000000
  %43 = mul i64 %9, -1000000000
  %44 = sub i64 %43, %11
  %45 = add i64 %40, %44
  %46 = add i64 %45, %42
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %46) #6
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local void @autograph_profile_record_kernel_ns(i32 noundef, i64 noundef) local_unnamed_addr #2

declare dso_local void @graph_ensure_owned_storage(ptr noundef) local_unnamed_addr #2

declare dso_local void @autograph_update_csr_pointers(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

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
  br label %39

18:                                               ; preds = %4
  call void @graph_ensure_owned_storage(ptr noundef nonnull %0) #6
  %19 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %20 = load ptr, ptr %19, align 8, !tbaa !9
  %21 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %22 = load ptr, ptr %21, align 8, !tbaa !14
  call void @autograph_update_csr_pointers(ptr noundef nonnull %0, ptr noundef %20, ptr noundef %22) #6
  %23 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %24 = icmp eq i32 %23, 3
  br i1 %24, label %32, label %25

25:                                               ; preds = %18
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %26 = call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #6
  %27 = load i64, ptr %0, align 8, !tbaa !15
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %29 = load i64, ptr %28, align 8, !tbaa !16
  %30 = load ptr, ptr %19, align 8, !tbaa !9
  %31 = load ptr, ptr %21, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %27, i64 noundef %29, ptr noundef %30, ptr noundef %31, ptr noundef %1, ptr noundef null, ptr noundef %2, i32 noundef %23) #6
  br label %34

32:                                               ; preds = %18
  %33 = call i32 @autograph_canonical_remove_node(ptr noundef nonnull %0, i32 noundef %3) #6
  br label %34

34:                                               ; preds = %32, %25
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %5) #6
  %35 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %5) #6
  %36 = load i64, ptr %5, align 8, !tbaa !3
  %37 = getelementptr inbounds nuw i8, ptr %5, i64 8
  %38 = load i64, ptr %37, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %5) #6
  br label %39

39:                                               ; preds = %34, %13
  %40 = phi i64 [ %38, %34 ], [ %17, %13 ]
  %41 = phi i64 [ %36, %34 ], [ %15, %13 ]
  %42 = mul i64 %41, 1000000000
  %43 = mul i64 %9, -1000000000
  %44 = sub i64 %43, %11
  %45 = add i64 %40, %44
  %46 = add i64 %45, %42
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %46) #6
  ret void
}

declare dso_local i32 @autograph_canonical_remove_node(ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_edge(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = alloca %struct.timespec, align 8
  %7 = alloca %struct.timespec, align 8
  %8 = alloca %struct.timespec, align 8
  %9 = alloca %struct.timespec, align 8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %9) #6
  %10 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %9) #6
  %11 = load i64, ptr %9, align 8, !tbaa !3
  %12 = getelementptr inbounds nuw i8, ptr %9, i64 8
  %13 = load i64, ptr %12, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %9) #6
  %14 = icmp eq ptr %0, null
  br i1 %14, label %15, label %20

15:                                               ; preds = %5
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %8) #6
  %16 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %8) #6
  %17 = load i64, ptr %8, align 8, !tbaa !3
  %18 = getelementptr inbounds nuw i8, ptr %8, i64 8
  %19 = load i64, ptr %18, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %8) #6
  br label %170

20:                                               ; preds = %5
  %21 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %22 = icmp eq i32 %21, 3
  br i1 %22, label %23, label %29

23:                                               ; preds = %20
  %24 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %7) #6
  %25 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %7) #6
  %26 = load i64, ptr %7, align 8, !tbaa !3
  %27 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %28 = load i64, ptr %27, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %7) #6
  br label %170

29:                                               ; preds = %20
  call void @graph_ensure_owned_storage(ptr noundef nonnull %0) #6
  %30 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %31 = load ptr, ptr %30, align 8, !tbaa !9
  %32 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %33 = load ptr, ptr %32, align 8, !tbaa !14
  call void @autograph_update_csr_pointers(ptr noundef nonnull %0, ptr noundef %31, ptr noundef %33) #6
  %34 = icmp sgt i32 %2, -1
  br i1 %34, label %35, label %44

35:                                               ; preds = %29
  %36 = zext nneg i32 %2 to i64
  %37 = load i64, ptr %0, align 8, !tbaa !15
  %38 = icmp sgt i64 %37, %36
  %39 = icmp sgt i32 %3, -1
  %40 = and i1 %39, %38
  br i1 %40, label %41, label %44

41:                                               ; preds = %35
  %42 = zext nneg i32 %3 to i64
  %43 = icmp samesign ugt i64 %37, %42
  br label %44

44:                                               ; preds = %41, %35, %29
  %45 = phi i1 [ false, %35 ], [ false, %29 ], [ %43, %41 ]
  switch i32 %21, label %163 [
    i32 0, label %46
    i32 1, label %63
    i32 2, label %148
  ]

46:                                               ; preds = %44
  %47 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %48 = load i64, ptr %47, align 8, !tbaa !16
  call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %2, i32 noundef %3)
  call fastcc void @csr_add_directed(ptr noundef %0, ptr noundef %0, i32 noundef %3, i32 noundef %2)
  %49 = load i64, ptr %0, align 8, !tbaa !15
  %50 = load i64, ptr %47, align 8, !tbaa !16
  %51 = load ptr, ptr %30, align 8, !tbaa !9
  %52 = load ptr, ptr %32, align 8, !tbaa !14
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %49, i64 noundef %50, ptr noundef %51, ptr noundef %52) #6
  %53 = load i64, ptr %47, align 8, !tbaa !16
  %54 = icmp eq i64 %53, %48
  br i1 %54, label %56, label %55

55:                                               ; preds = %46
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %56

56:                                               ; preds = %55, %46
  br i1 %45, label %165, label %57

57:                                               ; preds = %56
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %58 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %59 = load i64, ptr %0, align 8, !tbaa !15
  %60 = load i64, ptr %47, align 8, !tbaa !16
  %61 = load ptr, ptr %30, align 8, !tbaa !9
  %62 = load ptr, ptr %32, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %59, i64 noundef %60, ptr noundef %61, ptr noundef %62, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 0) #6
  br label %165

63:                                               ; preds = %44
  %64 = load ptr, ptr %30, align 8, !tbaa !9
  %65 = icmp eq ptr %64, null
  br i1 %65, label %66, label %69

66:                                               ; preds = %63
  %67 = load i64, ptr %0, align 8, !tbaa !15
  %68 = load ptr, ptr %32, align 8, !tbaa !14
  br label %130

69:                                               ; preds = %63
  %70 = load ptr, ptr %32, align 8, !tbaa !14
  %71 = icmp eq ptr %70, null
  %72 = icmp slt i32 %2, 0
  %73 = or i1 %72, %71
  %74 = load i64, ptr %0, align 8, !tbaa !15
  br i1 %73, label %101, label %75

75:                                               ; preds = %69
  %76 = zext nneg i32 %2 to i64
  %77 = icmp sgt i64 %74, %76
  br i1 %77, label %78, label %101

78:                                               ; preds = %75
  %79 = icmp sgt i32 %3, -1
  %80 = zext nneg i32 %3 to i64
  %81 = icmp samesign ugt i64 %74, %80
  %82 = select i1 %79, i1 %81, i1 false
  br i1 %82, label %83, label %101

83:                                               ; preds = %78
  %84 = getelementptr inbounds nuw i64, ptr %64, i64 %76
  %85 = load i64, ptr %84, align 8, !tbaa !17
  %86 = getelementptr inbounds nuw i8, ptr %84, i64 8
  %87 = load i64, ptr %86, align 8, !tbaa !17
  %88 = icmp slt i64 %85, %87
  br i1 %88, label %89, label %101

89:                                               ; preds = %83, %98
  %90 = phi i64 [ %99, %98 ], [ %85, %83 ]
  %91 = getelementptr inbounds i32, ptr %70, i64 %90
  %92 = load i32, ptr %91, align 4, !tbaa !18
  %93 = icmp eq i32 %92, %3
  br i1 %93, label %101, label %94

94:                                               ; preds = %89
  %95 = icmp eq i32 %92, -1
  br i1 %95, label %96, label %98

96:                                               ; preds = %94
  %97 = getelementptr inbounds i32, ptr %70, i64 %90
  store i32 %3, ptr %97, align 4, !tbaa !18
  br label %101

98:                                               ; preds = %94
  %99 = add i64 %90, 1
  %100 = icmp eq i64 %99, %87
  br i1 %100, label %101, label %89, !llvm.loop !20

101:                                              ; preds = %98, %89, %96, %83, %78, %75, %69
  %102 = phi i1 [ false, %83 ], [ false, %78 ], [ false, %75 ], [ false, %69 ], [ true, %96 ], [ false, %89 ], [ false, %98 ]
  %103 = icmp slt i32 %3, 0
  %104 = or i1 %103, %71
  br i1 %104, label %130, label %105

105:                                              ; preds = %101
  %106 = zext nneg i32 %3 to i64
  %107 = icmp sgt i64 %74, %106
  %108 = zext nneg i32 %2 to i64
  %109 = icmp samesign ugt i64 %74, %108
  %110 = and i1 %34, %107
  %111 = select i1 %110, i1 %109, i1 false
  br i1 %111, label %112, label %130

112:                                              ; preds = %105
  %113 = getelementptr inbounds nuw i64, ptr %64, i64 %106
  %114 = load i64, ptr %113, align 8, !tbaa !17
  %115 = getelementptr inbounds nuw i8, ptr %113, i64 8
  %116 = load i64, ptr %115, align 8, !tbaa !17
  %117 = icmp slt i64 %114, %116
  br i1 %117, label %118, label %130

118:                                              ; preds = %112, %125
  %119 = phi i64 [ %126, %125 ], [ %114, %112 ]
  %120 = getelementptr inbounds i32, ptr %70, i64 %119
  %121 = load i32, ptr %120, align 4, !tbaa !18
  %122 = icmp eq i32 %121, %2
  br i1 %122, label %130, label %123

123:                                              ; preds = %118
  %124 = icmp eq i32 %121, -1
  br i1 %124, label %128, label %125

125:                                              ; preds = %123
  %126 = add i64 %119, 1
  %127 = icmp eq i64 %126, %116
  br i1 %127, label %130, label %118, !llvm.loop !20

128:                                              ; preds = %123
  %129 = getelementptr inbounds i32, ptr %70, i64 %119
  store i32 %2, ptr %129, align 4, !tbaa !18
  br i1 %102, label %135, label %130

130:                                              ; preds = %118, %125, %128, %101, %105, %112, %66
  %131 = phi ptr [ %68, %66 ], [ %70, %112 ], [ %70, %105 ], [ %70, %101 ], [ %70, %128 ], [ %70, %125 ], [ %70, %118 ]
  %132 = phi i64 [ %67, %66 ], [ %74, %112 ], [ %74, %105 ], [ %74, %101 ], [ %74, %128 ], [ %74, %125 ], [ %74, %118 ]
  %133 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %134 = load i64, ptr %133, align 8, !tbaa !16
  call void @autograph_record_adjacency_state(ptr noundef %0, i64 noundef %132, i64 noundef %134, ptr noundef %64, ptr noundef %131) #6
  br label %140

135:                                              ; preds = %128
  %136 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %137 = load i64, ptr %136, align 8, !tbaa !16
  %138 = add nsw i64 %137, 2
  store i64 %138, ptr %136, align 8, !tbaa !16
  %139 = getelementptr inbounds nuw i8, ptr %0, i64 8
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %74, i64 noundef %138, ptr noundef nonnull %64, ptr noundef nonnull %70) #6
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %140

140:                                              ; preds = %130, %135
  %141 = phi ptr [ %133, %130 ], [ %139, %135 ]
  br i1 %45, label %165, label %142

142:                                              ; preds = %140
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %143 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %144 = load i64, ptr %0, align 8, !tbaa !15
  %145 = load i64, ptr %141, align 8, !tbaa !16
  %146 = load ptr, ptr %30, align 8, !tbaa !9
  %147 = load ptr, ptr %32, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %144, i64 noundef %145, ptr noundef %146, ptr noundef %147, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 1) #6
  br label %165

148:                                              ; preds = %44
  %149 = call i32 @autograph_bcsr_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %150 = call i32 @autograph_bcsr_add_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  %151 = icmp sgt i32 %149, 0
  %152 = icmp sgt i32 %150, 0
  %153 = select i1 %151, i1 %152, i1 false
  br i1 %153, label %154, label %155

154:                                              ; preds = %148
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %155

155:                                              ; preds = %154, %148
  br i1 %45, label %165, label %156

156:                                              ; preds = %155
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %157 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %158 = load i64, ptr %0, align 8, !tbaa !15
  %159 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %160 = load i64, ptr %159, align 8, !tbaa !16
  %161 = load ptr, ptr %30, align 8, !tbaa !9
  %162 = load ptr, ptr %32, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %158, i64 noundef %160, ptr noundef %161, ptr noundef %162, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 2) #6
  br label %165

163:                                              ; preds = %44
  %164 = call i32 @autograph_canonical_add_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  br label %165

165:                                              ; preds = %163, %57, %56, %142, %140, %156, %155
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #6
  %166 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #6
  %167 = load i64, ptr %6, align 8, !tbaa !3
  %168 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %169 = load i64, ptr %168, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #6
  br label %170

170:                                              ; preds = %23, %165, %15
  %171 = phi i64 [ %28, %23 ], [ %169, %165 ], [ %19, %15 ]
  %172 = phi i64 [ %26, %23 ], [ %167, %165 ], [ %17, %15 ]
  %173 = mul i64 %172, 1000000000
  %174 = mul i64 %11, -1000000000
  %175 = sub i64 %174, %13
  %176 = add i64 %171, %175
  %177 = add i64 %176, %173
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %177) #6
  ret void
}

declare dso_local i32 @autograph_canonical_add_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define internal fastcc void @csr_add_directed(ptr nocapture noundef nonnull %0, ptr noundef nonnull %1, i32 noundef %2, i32 noundef %3) unnamed_addr #0 {
  %5 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %6 = load ptr, ptr %5, align 8, !tbaa !9
  %7 = icmp eq ptr %6, null
  br i1 %7, label %57, label %8

8:                                                ; preds = %4
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %10 = load ptr, ptr %9, align 8, !tbaa !14
  %11 = icmp eq ptr %10, null
  %12 = icmp slt i32 %2, 0
  %13 = or i1 %12, %11
  br i1 %13, label %57, label %14

14:                                               ; preds = %8
  %15 = zext nneg i32 %2 to i64
  %16 = load i64, ptr %0, align 8, !tbaa !15
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
  %28 = load i64, ptr %27, align 8, !tbaa !16
  %29 = add nsw i64 %28, 1
  %30 = shl i64 %29, 2
  %31 = tail call ptr @realloc(ptr noundef nonnull %10, i64 noundef %30) #7
  %32 = icmp eq ptr %31, null
  br i1 %32, label %57, label %33

33:                                               ; preds = %23
  store ptr %31, ptr %9, align 8, !tbaa !14
  %34 = load ptr, ptr %5, align 8, !tbaa !9
  tail call void @autograph_update_csr_pointers(ptr noundef nonnull %1, ptr noundef %34, ptr noundef nonnull %31) #6
  %35 = icmp slt i64 %26, %28
  br i1 %35, label %36, label %42

36:                                               ; preds = %33
  %37 = load ptr, ptr %9, align 8, !tbaa !14
  %38 = getelementptr i32, ptr %37, i64 %26
  %39 = getelementptr i8, ptr %38, i64 4
  %40 = sub nsw i64 %28, %26
  %41 = shl i64 %40, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %39, ptr align 4 %38, i64 %41, i1 false)
  br label %42

42:                                               ; preds = %36, %33
  %43 = load ptr, ptr %9, align 8, !tbaa !14
  %44 = getelementptr inbounds i32, ptr %43, i64 %26
  store i32 %3, ptr %44, align 4, !tbaa !18
  store i64 %29, ptr %27, align 8, !tbaa !16
  %45 = load i64, ptr %0, align 8, !tbaa !15
  %46 = icmp sgt i64 %45, %15
  br i1 %46, label %47, label %57

47:                                               ; preds = %42
  %48 = load ptr, ptr %5, align 8, !tbaa !9
  br label %49

49:                                               ; preds = %47, %49
  %50 = phi i64 [ %15, %47 ], [ %51, %49 ]
  %51 = add nuw nsw i64 %50, 1
  %52 = getelementptr inbounds nuw i64, ptr %48, i64 %51
  %53 = load i64, ptr %52, align 8, !tbaa !17
  %54 = add nsw i64 %53, 1
  store i64 %54, ptr %52, align 8, !tbaa !17
  %55 = load i64, ptr %0, align 8, !tbaa !15
  %56 = icmp slt i64 %51, %55
  br i1 %56, label %49, label %57, !llvm.loop !22

57:                                               ; preds = %49, %42, %23, %14, %4, %8
  ret void
}

declare dso_local void @autograph_record_adjacency_state(ptr noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare dso_local void @autograph_mark_canonical_dirty(ptr noundef) local_unnamed_addr #2

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
  br label %259

19:                                               ; preds = %5
  call void @graph_ensure_owned_storage(ptr noundef nonnull %0) #6
  %20 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %21 = load ptr, ptr %20, align 8, !tbaa !9
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !14
  call void @autograph_update_csr_pointers(ptr noundef nonnull %0, ptr noundef %21, ptr noundef %23) #6
  %24 = call i32 @autograph_get_layout(ptr noundef nonnull %0) #6
  %25 = icmp sgt i32 %2, -1
  br i1 %25, label %26, label %35

26:                                               ; preds = %19
  %27 = zext nneg i32 %2 to i64
  %28 = load i64, ptr %0, align 8, !tbaa !15
  %29 = icmp sgt i64 %28, %27
  %30 = icmp sgt i32 %3, -1
  %31 = and i1 %30, %29
  br i1 %31, label %32, label %35

32:                                               ; preds = %26
  %33 = zext nneg i32 %3 to i64
  %34 = icmp samesign ugt i64 %28, %33
  br label %35

35:                                               ; preds = %32, %26, %19
  %36 = phi i1 [ false, %26 ], [ false, %19 ], [ %34, %32 ]
  switch i32 %24, label %252 [
    i32 0, label %37
    i32 1, label %160
    i32 2, label %241
  ]

37:                                               ; preds = %35
  %38 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %39 = load i64, ptr %38, align 8, !tbaa !16
  %40 = load ptr, ptr %20, align 8, !tbaa !9
  %41 = icmp eq ptr %40, null
  br i1 %41, label %145, label %42

42:                                               ; preds = %37
  %43 = load ptr, ptr %22, align 8, !tbaa !14
  %44 = icmp eq ptr %43, null
  %45 = icmp slt i32 %2, 0
  %46 = or i1 %45, %44
  br i1 %46, label %94, label %47

47:                                               ; preds = %42
  %48 = zext nneg i32 %2 to i64
  %49 = load i64, ptr %0, align 8, !tbaa !15
  %50 = icmp sgt i64 %49, %48
  br i1 %50, label %51, label %94

51:                                               ; preds = %47
  %52 = getelementptr inbounds nuw i64, ptr %40, i64 %48
  %53 = load i64, ptr %52, align 8, !tbaa !17
  %54 = getelementptr inbounds nuw i8, ptr %52, i64 8
  %55 = load i64, ptr %54, align 8, !tbaa !17
  %56 = icmp slt i64 %53, %55
  br i1 %56, label %57, label %94

57:                                               ; preds = %51, %62
  %58 = phi i64 [ %63, %62 ], [ %53, %51 ]
  %59 = getelementptr inbounds i32, ptr %43, i64 %58
  %60 = load i32, ptr %59, align 4, !tbaa !18
  %61 = icmp eq i32 %60, %3
  br i1 %61, label %65, label %62

62:                                               ; preds = %57
  %63 = add i64 %58, 1
  %64 = icmp eq i64 %63, %55
  br i1 %64, label %91, label %57, !llvm.loop !23

65:                                               ; preds = %57
  %66 = getelementptr inbounds i32, ptr %43, i64 %58
  %67 = icmp slt i64 %58, 0
  br i1 %67, label %91, label %68

68:                                               ; preds = %65
  %69 = add nuw nsw i64 %58, 1
  %70 = icmp slt i64 %69, %39
  br i1 %70, label %71, label %78

71:                                               ; preds = %68
  %72 = getelementptr inbounds nuw i32, ptr %43, i64 %69
  %73 = xor i64 %58, -1
  %74 = add nsw i64 %39, %73
  %75 = shl i64 %74, 2
  call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %66, ptr nonnull align 4 %72, i64 %75, i1 false)
  %76 = load i64, ptr %0, align 8, !tbaa !15
  %77 = load ptr, ptr %20, align 8, !tbaa !9
  br label %78

78:                                               ; preds = %71, %68
  %79 = phi ptr [ %77, %71 ], [ %40, %68 ]
  %80 = phi i64 [ %76, %71 ], [ %49, %68 ]
  %81 = add nsw i64 %39, -1
  store i64 %81, ptr %38, align 8, !tbaa !16
  %82 = icmp sgt i64 %80, %48
  br i1 %82, label %83, label %91

83:                                               ; preds = %78, %83
  %84 = phi i64 [ %85, %83 ], [ %48, %78 ]
  %85 = add nuw nsw i64 %84, 1
  %86 = getelementptr inbounds nuw i64, ptr %79, i64 %85
  %87 = load i64, ptr %86, align 8, !tbaa !17
  %88 = add nsw i64 %87, -1
  store i64 %88, ptr %86, align 8, !tbaa !17
  %89 = load i64, ptr %0, align 8, !tbaa !15
  %90 = icmp slt i64 %85, %89
  br i1 %90, label %83, label %94, !llvm.loop !24

91:                                               ; preds = %62, %65, %78
  %92 = phi ptr [ %40, %65 ], [ %79, %78 ], [ %40, %62 ]
  %93 = icmp eq ptr %92, null
  br i1 %93, label %145, label %94

94:                                               ; preds = %83, %51, %47, %42, %91
  %95 = phi ptr [ %92, %91 ], [ %40, %51 ], [ %40, %47 ], [ %40, %42 ], [ %79, %83 ]
  %96 = load ptr, ptr %22, align 8, !tbaa !14
  %97 = icmp eq ptr %96, null
  %98 = icmp slt i32 %3, 0
  %99 = or i1 %98, %97
  br i1 %99, label %145, label %100

100:                                              ; preds = %94
  %101 = zext nneg i32 %3 to i64
  %102 = load i64, ptr %0, align 8, !tbaa !15
  %103 = icmp sgt i64 %102, %101
  br i1 %103, label %104, label %145

104:                                              ; preds = %100
  %105 = getelementptr inbounds nuw i64, ptr %95, i64 %101
  %106 = load i64, ptr %105, align 8, !tbaa !17
  %107 = getelementptr inbounds nuw i8, ptr %105, i64 8
  %108 = load i64, ptr %107, align 8, !tbaa !17
  %109 = icmp slt i64 %106, %108
  br i1 %109, label %110, label %145

110:                                              ; preds = %104, %115
  %111 = phi i64 [ %116, %115 ], [ %106, %104 ]
  %112 = getelementptr inbounds i32, ptr %96, i64 %111
  %113 = load i32, ptr %112, align 4, !tbaa !18
  %114 = icmp eq i32 %113, %2
  br i1 %114, label %118, label %115

115:                                              ; preds = %110
  %116 = add i64 %111, 1
  %117 = icmp eq i64 %116, %108
  br i1 %117, label %145, label %110, !llvm.loop !23

118:                                              ; preds = %110
  %119 = getelementptr inbounds i32, ptr %96, i64 %111
  %120 = icmp slt i64 %111, 0
  br i1 %120, label %145, label %121

121:                                              ; preds = %118
  %122 = load i64, ptr %38, align 8, !tbaa !16
  %123 = add nuw nsw i64 %111, 1
  %124 = icmp slt i64 %123, %122
  br i1 %124, label %125, label %132

125:                                              ; preds = %121
  %126 = getelementptr inbounds nuw i32, ptr %96, i64 %123
  %127 = xor i64 %111, -1
  %128 = add nsw i64 %122, %127
  %129 = shl i64 %128, 2
  call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %119, ptr nonnull align 4 %126, i64 %129, i1 false)
  %130 = load i64, ptr %0, align 8, !tbaa !15
  %131 = load ptr, ptr %20, align 8, !tbaa !9
  br label %132

132:                                              ; preds = %125, %121
  %133 = phi ptr [ %131, %125 ], [ %95, %121 ]
  %134 = phi i64 [ %130, %125 ], [ %102, %121 ]
  %135 = add nsw i64 %122, -1
  store i64 %135, ptr %38, align 8, !tbaa !16
  %136 = icmp sgt i64 %134, %101
  br i1 %136, label %137, label %145

137:                                              ; preds = %132, %137
  %138 = phi i64 [ %139, %137 ], [ %101, %132 ]
  %139 = add nuw nsw i64 %138, 1
  %140 = getelementptr inbounds nuw i64, ptr %133, i64 %139
  %141 = load i64, ptr %140, align 8, !tbaa !17
  %142 = add nsw i64 %141, -1
  store i64 %142, ptr %140, align 8, !tbaa !17
  %143 = load i64, ptr %0, align 8, !tbaa !15
  %144 = icmp slt i64 %139, %143
  br i1 %144, label %137, label %145, !llvm.loop !24

145:                                              ; preds = %115, %137, %37, %91, %94, %100, %104, %118, %132
  %146 = phi ptr [ null, %37 ], [ null, %91 ], [ %95, %94 ], [ %95, %100 ], [ %95, %104 ], [ %95, %118 ], [ %133, %132 ], [ %133, %137 ], [ %95, %115 ]
  %147 = load i64, ptr %0, align 8, !tbaa !15
  %148 = load i64, ptr %38, align 8, !tbaa !16
  %149 = load ptr, ptr %22, align 8, !tbaa !14
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %147, i64 noundef %148, ptr noundef %146, ptr noundef %149) #6
  %150 = load i64, ptr %38, align 8, !tbaa !16
  %151 = icmp eq i64 %150, %39
  br i1 %151, label %153, label %152

152:                                              ; preds = %145
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %153

153:                                              ; preds = %152, %145
  br i1 %36, label %254, label %154

154:                                              ; preds = %153
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %155 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %156 = load i64, ptr %0, align 8, !tbaa !15
  %157 = load i64, ptr %38, align 8, !tbaa !16
  %158 = load ptr, ptr %20, align 8, !tbaa !9
  %159 = load ptr, ptr %22, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %156, i64 noundef %157, ptr noundef %158, ptr noundef %159, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 0) #6
  br label %254

160:                                              ; preds = %35
  %161 = load ptr, ptr %20, align 8, !tbaa !9
  %162 = icmp eq ptr %161, null
  br i1 %162, label %163, label %166

163:                                              ; preds = %160
  %164 = load i64, ptr %0, align 8, !tbaa !15
  %165 = load ptr, ptr %22, align 8, !tbaa !14
  br label %223

166:                                              ; preds = %160
  %167 = load ptr, ptr %22, align 8, !tbaa !14
  %168 = icmp eq ptr %167, null
  %169 = icmp slt i32 %2, 0
  %170 = or i1 %169, %168
  %171 = load i64, ptr %0, align 8, !tbaa !15
  br i1 %170, label %196, label %172

172:                                              ; preds = %166
  %173 = zext nneg i32 %2 to i64
  %174 = icmp sgt i64 %171, %173
  br i1 %174, label %175, label %196

175:                                              ; preds = %172
  %176 = icmp sgt i32 %3, -1
  %177 = zext nneg i32 %3 to i64
  %178 = icmp samesign ugt i64 %171, %177
  %179 = select i1 %176, i1 %178, i1 false
  br i1 %179, label %180, label %196

180:                                              ; preds = %175
  %181 = getelementptr inbounds nuw i64, ptr %161, i64 %173
  %182 = load i64, ptr %181, align 8, !tbaa !17
  %183 = getelementptr inbounds nuw i8, ptr %181, i64 8
  %184 = load i64, ptr %183, align 8, !tbaa !17
  %185 = icmp slt i64 %182, %184
  br i1 %185, label %186, label %196

186:                                              ; preds = %180, %193
  %187 = phi i64 [ %194, %193 ], [ %182, %180 ]
  %188 = getelementptr inbounds i32, ptr %167, i64 %187
  %189 = load i32, ptr %188, align 4, !tbaa !18
  %190 = icmp eq i32 %189, %3
  br i1 %190, label %191, label %193

191:                                              ; preds = %186
  %192 = getelementptr inbounds i32, ptr %167, i64 %187
  store i32 -1, ptr %192, align 4, !tbaa !18
  br label %196

193:                                              ; preds = %186
  %194 = add i64 %187, 1
  %195 = icmp eq i64 %194, %184
  br i1 %195, label %196, label %186, !llvm.loop !25

196:                                              ; preds = %193, %191, %180, %175, %172, %166
  %197 = phi i1 [ true, %166 ], [ true, %172 ], [ true, %175 ], [ true, %180 ], [ false, %191 ], [ true, %193 ]
  %198 = icmp slt i32 %3, 0
  %199 = or i1 %198, %168
  br i1 %199, label %223, label %200

200:                                              ; preds = %196
  %201 = zext nneg i32 %3 to i64
  %202 = icmp sgt i64 %171, %201
  %203 = zext nneg i32 %2 to i64
  %204 = icmp samesign ugt i64 %171, %203
  %205 = and i1 %25, %202
  %206 = select i1 %205, i1 %204, i1 false
  br i1 %206, label %207, label %223

207:                                              ; preds = %200
  %208 = getelementptr inbounds nuw i64, ptr %161, i64 %201
  %209 = load i64, ptr %208, align 8, !tbaa !17
  %210 = getelementptr inbounds nuw i8, ptr %208, i64 8
  %211 = load i64, ptr %210, align 8, !tbaa !17
  %212 = icmp slt i64 %209, %211
  br i1 %212, label %213, label %223

213:                                              ; preds = %207, %218
  %214 = phi i64 [ %219, %218 ], [ %209, %207 ]
  %215 = getelementptr inbounds i32, ptr %167, i64 %214
  %216 = load i32, ptr %215, align 4, !tbaa !18
  %217 = icmp eq i32 %216, %2
  br i1 %217, label %221, label %218

218:                                              ; preds = %213
  %219 = add i64 %214, 1
  %220 = icmp eq i64 %219, %211
  br i1 %220, label %223, label %213, !llvm.loop !25

221:                                              ; preds = %213
  %222 = getelementptr inbounds i32, ptr %167, i64 %214
  store i32 -1, ptr %222, align 4, !tbaa !18
  br i1 %197, label %223, label %228

223:                                              ; preds = %218, %221, %196, %200, %207, %163
  %224 = phi ptr [ %165, %163 ], [ %167, %207 ], [ %167, %200 ], [ %167, %196 ], [ %167, %221 ], [ %167, %218 ]
  %225 = phi i64 [ %164, %163 ], [ %171, %207 ], [ %171, %200 ], [ %171, %196 ], [ %171, %221 ], [ %171, %218 ]
  %226 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %227 = load i64, ptr %226, align 8, !tbaa !16
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %225, i64 noundef %227, ptr noundef %161, ptr noundef %224) #6
  br label %233

228:                                              ; preds = %221
  %229 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %230 = load i64, ptr %229, align 8, !tbaa !16
  %231 = add nsw i64 %230, -2
  store i64 %231, ptr %229, align 8, !tbaa !16
  %232 = getelementptr inbounds nuw i8, ptr %0, i64 8
  call void @autograph_record_adjacency_state(ptr noundef nonnull %0, i64 noundef %171, i64 noundef %231, ptr noundef nonnull %161, ptr noundef nonnull %167) #6
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %233

233:                                              ; preds = %223, %228
  %234 = phi ptr [ %226, %223 ], [ %232, %228 ]
  br i1 %36, label %254, label %235

235:                                              ; preds = %233
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %236 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %237 = load i64, ptr %0, align 8, !tbaa !15
  %238 = load i64, ptr %234, align 8, !tbaa !16
  %239 = load ptr, ptr %20, align 8, !tbaa !9
  %240 = load ptr, ptr %22, align 8, !tbaa !14
  call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %237, i64 noundef %238, ptr noundef %239, ptr noundef %240, ptr noundef null, ptr noundef %1, ptr noundef null, i32 noundef 1) #6
  br label %254

241:                                              ; preds = %35
  %242 = call i32 @autograph_bcsr_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %243 = call i32 @autograph_bcsr_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  %244 = icmp sgt i32 %242, 0
  %245 = icmp sgt i32 %243, 0
  %246 = select i1 %244, i1 %245, i1 false
  br i1 %246, label %247, label %248

247:                                              ; preds = %241
  call void @autograph_mark_canonical_dirty(ptr noundef nonnull %0) #6
  br label %248

248:                                              ; preds = %247, %241
  br i1 %36, label %254, label %249

249:                                              ; preds = %248
  call void @autograph_ensure_layout_set(ptr noundef nonnull %0) #6
  %250 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  %251 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %3, i32 noundef %2) #6
  br label %254

252:                                              ; preds = %35
  %253 = call i32 @autograph_canonical_remove_edge(ptr noundef nonnull %0, i32 noundef %2, i32 noundef %3) #6
  br label %254

254:                                              ; preds = %252, %154, %153, %235, %233, %249, %248
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %6) #6
  %255 = call i32 @clock_gettime(i32 noundef 1, ptr noundef nonnull %6) #6
  %256 = load i64, ptr %6, align 8, !tbaa !3
  %257 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %258 = load i64, ptr %257, align 8, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %6) #6
  br label %259

259:                                              ; preds = %254, %14
  %260 = phi i64 [ %258, %254 ], [ %18, %14 ]
  %261 = phi i64 [ %256, %254 ], [ %16, %14 ]
  %262 = mul i64 %261, 1000000000
  %263 = mul i64 %10, -1000000000
  %264 = sub i64 %263, %12
  %265 = add i64 %260, %264
  %266 = add i64 %265, %262
  call void @autograph_profile_record_kernel_ns(i32 noundef 1, i64 noundef %266) #6
  ret void
}

declare dso_local i32 @autograph_canonical_remove_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

declare dso_local i32 @autograph_bcsr_remove_edge(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: nounwind
declare dso_local i32 @clock_gettime(i32 noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #4

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
!9 = !{!10, !11, i64 16}
!10 = !{!"", !5, i64 0, !5, i64 8, !11, i64 16, !13, i64 24}
!11 = !{!"p1 long", !12, i64 0}
!12 = !{!"any pointer", !6, i64 0}
!13 = !{!"p1 int", !12, i64 0}
!14 = !{!10, !13, i64 24}
!15 = !{!10, !5, i64 0}
!16 = !{!10, !5, i64 8}
!17 = !{!5, !5, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !6, i64 0}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.mustprogress"}
!22 = distinct !{!22, !21}
!23 = distinct !{!23, !21}
!24 = distinct !{!24, !21}
!25 = distinct !{!25, !21}
