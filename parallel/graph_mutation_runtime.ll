; ModuleID = 'graph_mutation_runtime.c'
source_filename = "graph_mutation_runtime.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_node(ptr noundef %0, ptr nocapture noundef readnone %1, ptr nocapture noundef readnone %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %34, label %6

6:                                                ; preds = %4
  %7 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %17, label %9

9:                                                ; preds = %6
  %10 = load i64, ptr %0, align 8, !tbaa !3
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %12 = load i64, ptr %11, align 8, !tbaa !11
  %13 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %14 = load ptr, ptr %13, align 8, !tbaa !12
  %15 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %16 = load ptr, ptr %15, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %10, i64 noundef %12, ptr noundef %14, ptr noundef %16, ptr noundef null, ptr noundef null, ptr noundef null, i32 noundef 0) #4
  br label %17

17:                                               ; preds = %6, %9
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %19 = load ptr, ptr %18, align 8, !tbaa !12
  %20 = icmp eq ptr %19, null
  br i1 %20, label %34, label %21

21:                                               ; preds = %17
  %22 = load i64, ptr %0, align 8, !tbaa !3
  %23 = shl i64 %22, 3
  %24 = add i64 %23, 16
  %25 = tail call ptr @realloc(ptr noundef nonnull %19, i64 noundef %24) #5
  %26 = icmp eq ptr %25, null
  br i1 %26, label %34, label %27

27:                                               ; preds = %21
  store ptr %25, ptr %18, align 8, !tbaa !12
  %28 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %29 = load i64, ptr %28, align 8, !tbaa !11
  %30 = add nsw i64 %22, 1
  %31 = getelementptr inbounds i64, ptr %25, i64 %30
  store i64 %29, ptr %31, align 8, !tbaa !14
  store i64 %30, ptr %0, align 8, !tbaa !3
  %32 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %33 = load ptr, ptr %32, align 8, !tbaa !13
  tail call void @autograph_update_csr_pointers(ptr noundef nonnull %0, ptr noundef nonnull %25, ptr noundef %33) #4
  br label %34

34:                                               ; preds = %27, %21, %17, %4
  ret void
}

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #1

declare dso_local void @autograph_update_csr_pointers(ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_node(ptr noundef %0, ptr nocapture noundef readnone %1, ptr nocapture noundef readnone %2, i32 noundef %3) local_unnamed_addr #0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %198, label %6

6:                                                ; preds = %4
  %7 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %17, label %9

9:                                                ; preds = %6
  %10 = load i64, ptr %0, align 8, !tbaa !3
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %12 = load i64, ptr %11, align 8, !tbaa !11
  %13 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %14 = load ptr, ptr %13, align 8, !tbaa !12
  %15 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %16 = load ptr, ptr %15, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %10, i64 noundef %12, ptr noundef %14, ptr noundef %16, ptr noundef null, ptr noundef null, ptr noundef null, i32 noundef 0) #4
  br label %17

17:                                               ; preds = %6, %9
  %18 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %19 = load ptr, ptr %18, align 8, !tbaa !12
  %20 = icmp eq ptr %19, null
  br i1 %20, label %198, label %21

21:                                               ; preds = %17
  %22 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %23 = load ptr, ptr %22, align 8, !tbaa !13
  %24 = icmp eq ptr %23, null
  %25 = icmp slt i32 %3, 0
  %26 = or i1 %25, %24
  br i1 %26, label %198, label %27

27:                                               ; preds = %21
  %28 = zext nneg i32 %3 to i64
  %29 = load i64, ptr %0, align 8, !tbaa !3
  %30 = icmp sgt i64 %29, %28
  br i1 %30, label %31, label %198

31:                                               ; preds = %27
  %32 = getelementptr inbounds nuw i64, ptr %19, i64 %28
  %33 = load i64, ptr %32, align 8, !tbaa !14
  %34 = getelementptr inbounds nuw i8, ptr %32, i64 8
  %35 = load i64, ptr %34, align 8, !tbaa !14
  %36 = icmp slt i64 %33, %35
  br i1 %36, label %37, label %41

37:                                               ; preds = %31
  %38 = getelementptr inbounds nuw i8, ptr %0, i64 8
  br label %46

39:                                               ; preds = %91
  %40 = icmp sgt i64 %94, 0
  br i1 %40, label %41, label %198

41:                                               ; preds = %31, %39
  %42 = phi i64 [ %94, %39 ], [ %29, %31 ]
  %43 = phi ptr [ %93, %39 ], [ %19, %31 ]
  %44 = phi ptr [ %92, %39 ], [ %19, %31 ]
  %45 = getelementptr inbounds nuw i8, ptr %0, i64 8
  br label %100

46:                                               ; preds = %37, %91
  %47 = phi ptr [ %19, %37 ], [ %92, %91 ]
  %48 = phi ptr [ %19, %37 ], [ %93, %91 ]
  %49 = phi i64 [ %29, %37 ], [ %94, %91 ]
  %50 = phi i64 [ %35, %37 ], [ %98, %91 ]
  %51 = phi i64 [ %33, %37 ], [ %96, %91 ]
  %52 = load ptr, ptr %22, align 8, !tbaa !13
  %53 = getelementptr inbounds i32, ptr %52, i64 %51
  %54 = load i32, ptr %53, align 4, !tbaa !15
  %55 = icmp sgt i64 %49, %28
  br i1 %55, label %56, label %91

56:                                               ; preds = %46, %61
  %57 = phi i64 [ %62, %61 ], [ %51, %46 ]
  %58 = getelementptr inbounds i32, ptr %52, i64 %57
  %59 = load i32, ptr %58, align 4, !tbaa !15
  %60 = icmp eq i32 %59, %54
  br i1 %60, label %64, label %61

61:                                               ; preds = %56
  %62 = add i64 %57, 1
  %63 = icmp eq i64 %62, %50
  br i1 %63, label %91, label %56, !llvm.loop !17

64:                                               ; preds = %56
  %65 = getelementptr inbounds i32, ptr %52, i64 %57
  %66 = icmp slt i64 %57, 0
  br i1 %66, label %91, label %67

67:                                               ; preds = %64
  %68 = load i64, ptr %38, align 8, !tbaa !11
  %69 = add nuw nsw i64 %57, 1
  %70 = icmp slt i64 %69, %68
  br i1 %70, label %71, label %78

71:                                               ; preds = %67
  %72 = getelementptr inbounds nuw i32, ptr %52, i64 %69
  %73 = xor i64 %57, -1
  %74 = add nsw i64 %68, %73
  %75 = shl i64 %74, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %65, ptr nonnull align 4 %72, i64 %75, i1 false)
  %76 = load i64, ptr %0, align 8, !tbaa !3
  %77 = load ptr, ptr %18, align 8, !tbaa !12
  br label %78

78:                                               ; preds = %71, %67
  %79 = phi ptr [ %77, %71 ], [ %47, %67 ]
  %80 = phi i64 [ %76, %71 ], [ %49, %67 ]
  %81 = add nsw i64 %68, -1
  store i64 %81, ptr %38, align 8, !tbaa !11
  %82 = icmp sgt i64 %80, %28
  br i1 %82, label %83, label %91

83:                                               ; preds = %78, %83
  %84 = phi i64 [ %85, %83 ], [ %28, %78 ]
  %85 = add nuw nsw i64 %84, 1
  %86 = getelementptr inbounds nuw i64, ptr %79, i64 %85
  %87 = load i64, ptr %86, align 8, !tbaa !14
  %88 = add nsw i64 %87, -1
  store i64 %88, ptr %86, align 8, !tbaa !14
  %89 = load i64, ptr %0, align 8, !tbaa !3
  %90 = icmp slt i64 %85, %89
  br i1 %90, label %83, label %91, !llvm.loop !19

91:                                               ; preds = %61, %83, %46, %64, %78
  %92 = phi ptr [ %47, %46 ], [ %47, %64 ], [ %79, %78 ], [ %79, %83 ], [ %47, %61 ]
  %93 = phi ptr [ %48, %46 ], [ %48, %64 ], [ %79, %78 ], [ %79, %83 ], [ %48, %61 ]
  %94 = phi i64 [ %49, %46 ], [ %49, %64 ], [ %80, %78 ], [ %89, %83 ], [ %49, %61 ]
  %95 = getelementptr inbounds nuw i64, ptr %93, i64 %28
  %96 = load i64, ptr %95, align 8, !tbaa !14
  %97 = getelementptr inbounds nuw i8, ptr %95, i64 8
  %98 = load i64, ptr %97, align 8, !tbaa !14
  %99 = icmp slt i64 %96, %98
  br i1 %99, label %46, label %39, !llvm.loop !20

100:                                              ; preds = %41, %191
  %101 = phi ptr [ %44, %41 ], [ %192, %191 ]
  %102 = phi i64 [ %42, %41 ], [ %193, %191 ]
  %103 = phi ptr [ %43, %41 ], [ %194, %191 ]
  %104 = phi ptr [ %43, %41 ], [ %195, %191 ]
  %105 = phi i64 [ 0, %41 ], [ %196, %191 ]
  %106 = trunc i64 %105 to i32
  %107 = icmp eq i32 %3, %106
  br i1 %107, label %191, label %108

108:                                              ; preds = %100
  %109 = getelementptr inbounds nuw i64, ptr %104, i64 %105
  %110 = load i64, ptr %109, align 8, !tbaa !14
  %111 = getelementptr inbounds nuw i8, ptr %109, i64 8
  %112 = load i64, ptr %111, align 8, !tbaa !14
  %113 = icmp slt i64 %110, %112
  br i1 %113, label %114, label %191

114:                                              ; preds = %108
  %115 = icmp sgt i32 %106, -1
  %116 = and i64 %105, 2147483647
  br label %117

117:                                              ; preds = %114, %181
  %118 = phi ptr [ %101, %114 ], [ %182, %181 ]
  %119 = phi i64 [ %102, %114 ], [ %183, %181 ]
  %120 = phi ptr [ %103, %114 ], [ %184, %181 ]
  %121 = phi i64 [ %102, %114 ], [ %185, %181 ]
  %122 = phi ptr [ %104, %114 ], [ %184, %181 ]
  %123 = phi i64 [ %112, %114 ], [ %189, %181 ]
  %124 = phi i64 [ %110, %114 ], [ %187, %181 ]
  %125 = load ptr, ptr %22, align 8, !tbaa !13
  br label %129

126:                                              ; preds = %129
  %127 = add nsw i64 %130, 1
  %128 = icmp slt i64 %127, %123
  br i1 %128, label %129, label %191, !llvm.loop !21

129:                                              ; preds = %117, %126
  %130 = phi i64 [ %124, %117 ], [ %127, %126 ]
  %131 = getelementptr inbounds i32, ptr %125, i64 %130
  %132 = load i32, ptr %131, align 4, !tbaa !15
  %133 = icmp eq i32 %132, %3
  br i1 %133, label %134, label %126

134:                                              ; preds = %129
  %135 = icmp ne ptr %122, null
  %136 = and i1 %115, %135
  %137 = icmp sgt i64 %121, %116
  %138 = select i1 %136, i1 %137, i1 false
  br i1 %138, label %139, label %181

139:                                              ; preds = %134
  %140 = getelementptr inbounds nuw i64, ptr %122, i64 %116
  %141 = load i64, ptr %140, align 8, !tbaa !14
  %142 = getelementptr inbounds nuw i8, ptr %140, i64 8
  %143 = load i64, ptr %142, align 8, !tbaa !14
  %144 = icmp slt i64 %141, %143
  br i1 %144, label %145, label %181

145:                                              ; preds = %139, %150
  %146 = phi i64 [ %151, %150 ], [ %141, %139 ]
  %147 = getelementptr inbounds i32, ptr %125, i64 %146
  %148 = load i32, ptr %147, align 4, !tbaa !15
  %149 = icmp eq i32 %148, %3
  br i1 %149, label %153, label %150

150:                                              ; preds = %145
  %151 = add i64 %146, 1
  %152 = icmp eq i64 %151, %143
  br i1 %152, label %181, label %145, !llvm.loop !17

153:                                              ; preds = %145
  %154 = getelementptr inbounds i32, ptr %125, i64 %146
  %155 = icmp slt i64 %146, 0
  br i1 %155, label %181, label %156

156:                                              ; preds = %153
  %157 = load i64, ptr %45, align 8, !tbaa !11
  %158 = add nuw nsw i64 %146, 1
  %159 = icmp slt i64 %158, %157
  br i1 %159, label %160, label %167

160:                                              ; preds = %156
  %161 = getelementptr inbounds nuw i32, ptr %125, i64 %158
  %162 = xor i64 %146, -1
  %163 = add nsw i64 %157, %162
  %164 = shl i64 %163, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %154, ptr nonnull align 4 %161, i64 %164, i1 false)
  %165 = load i64, ptr %0, align 8, !tbaa !3
  %166 = load ptr, ptr %18, align 8, !tbaa !12
  br label %167

167:                                              ; preds = %160, %156
  %168 = phi ptr [ %166, %160 ], [ %118, %156 ]
  %169 = phi i64 [ %165, %160 ], [ %119, %156 ]
  %170 = phi i64 [ %165, %160 ], [ %121, %156 ]
  %171 = add nsw i64 %157, -1
  store i64 %171, ptr %45, align 8, !tbaa !11
  %172 = icmp sgt i64 %170, %116
  br i1 %172, label %173, label %181

173:                                              ; preds = %167, %173
  %174 = phi i64 [ %175, %173 ], [ %116, %167 ]
  %175 = add nuw nsw i64 %174, 1
  %176 = getelementptr inbounds nuw i64, ptr %168, i64 %175
  %177 = load i64, ptr %176, align 8, !tbaa !14
  %178 = add nsw i64 %177, -1
  store i64 %178, ptr %176, align 8, !tbaa !14
  %179 = load i64, ptr %0, align 8, !tbaa !3
  %180 = icmp slt i64 %175, %179
  br i1 %180, label %173, label %181, !llvm.loop !19

181:                                              ; preds = %150, %173, %134, %139, %153, %167
  %182 = phi ptr [ %118, %134 ], [ %118, %139 ], [ %118, %153 ], [ %168, %167 ], [ %168, %173 ], [ %118, %150 ]
  %183 = phi i64 [ %119, %134 ], [ %119, %139 ], [ %119, %153 ], [ %169, %167 ], [ %179, %173 ], [ %119, %150 ]
  %184 = phi ptr [ %120, %134 ], [ %120, %139 ], [ %120, %153 ], [ %168, %167 ], [ %168, %173 ], [ %120, %150 ]
  %185 = phi i64 [ %121, %134 ], [ %121, %139 ], [ %121, %153 ], [ %170, %167 ], [ %179, %173 ], [ %121, %150 ]
  %186 = getelementptr inbounds nuw i64, ptr %184, i64 %105
  %187 = load i64, ptr %186, align 8, !tbaa !14
  %188 = getelementptr inbounds nuw i8, ptr %186, i64 8
  %189 = load i64, ptr %188, align 8, !tbaa !14
  %190 = icmp slt i64 %187, %189
  br i1 %190, label %117, label %191

191:                                              ; preds = %181, %126, %108, %100
  %192 = phi ptr [ %101, %108 ], [ %101, %100 ], [ %118, %126 ], [ %182, %181 ]
  %193 = phi i64 [ %102, %108 ], [ %102, %100 ], [ %119, %126 ], [ %183, %181 ]
  %194 = phi ptr [ %103, %108 ], [ %103, %100 ], [ %120, %126 ], [ %184, %181 ]
  %195 = phi ptr [ %104, %108 ], [ %104, %100 ], [ %122, %126 ], [ %184, %181 ]
  %196 = add nuw nsw i64 %105, 1
  %197 = icmp slt i64 %196, %193
  br i1 %197, label %100, label %198, !llvm.loop !22

198:                                              ; preds = %191, %39, %27, %17, %21, %4
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @graph_add_edge(ptr noundef %0, ptr nocapture noundef readnone %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  %7 = icmp slt i32 %2, 0
  %8 = or i1 %6, %7
  br i1 %8, label %144, label %9

9:                                                ; preds = %5
  %10 = zext nneg i32 %2 to i64
  %11 = load i64, ptr %0, align 8, !tbaa !3
  %12 = icmp sgt i64 %11, %10
  br i1 %12, label %13, label %144

13:                                               ; preds = %9
  %14 = icmp sgt i32 %3, -1
  %15 = zext nneg i32 %3 to i64
  %16 = icmp samesign ugt i64 %11, %15
  %17 = select i1 %14, i1 %16, i1 false
  br i1 %17, label %18, label %144

18:                                               ; preds = %13
  %19 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %20 = icmp eq i32 %19, 0
  br i1 %20, label %29, label %21

21:                                               ; preds = %18
  %22 = load i64, ptr %0, align 8, !tbaa !3
  %23 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %24 = load i64, ptr %23, align 8, !tbaa !11
  %25 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %26 = load ptr, ptr %25, align 8, !tbaa !12
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %28 = load ptr, ptr %27, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %22, i64 noundef %24, ptr noundef %26, ptr noundef %28, ptr noundef null, ptr noundef null, ptr noundef null, i32 noundef 0) #4
  br label %29

29:                                               ; preds = %18, %21
  %30 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %31 = load ptr, ptr %30, align 8, !tbaa !12
  %32 = icmp eq ptr %31, null
  br i1 %32, label %140, label %33

33:                                               ; preds = %29
  %34 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %35 = load ptr, ptr %34, align 8, !tbaa !13
  %36 = icmp eq ptr %35, null
  br i1 %36, label %140, label %37

37:                                               ; preds = %33
  %38 = load i64, ptr %0, align 8, !tbaa !3
  %39 = icmp sgt i64 %38, %10
  %40 = icmp sgt i64 %38, %15
  %41 = select i1 %39, i1 %40, i1 false
  br i1 %41, label %42, label %88

42:                                               ; preds = %37
  %43 = getelementptr inbounds nuw i64, ptr %31, i64 %10
  %44 = load i64, ptr %43, align 8, !tbaa !14
  %45 = getelementptr inbounds nuw i8, ptr %43, i64 8
  %46 = load i64, ptr %45, align 8, !tbaa !14
  %47 = icmp slt i64 %44, %46
  br i1 %47, label %51, label %56

48:                                               ; preds = %51
  %49 = add i64 %52, 1
  %50 = icmp eq i64 %49, %46
  br i1 %50, label %56, label %51, !llvm.loop !23

51:                                               ; preds = %42, %48
  %52 = phi i64 [ %49, %48 ], [ %44, %42 ]
  %53 = getelementptr inbounds i32, ptr %35, i64 %52
  %54 = load i32, ptr %53, align 4, !tbaa !15
  %55 = icmp eq i32 %54, %3
  br i1 %55, label %85, label %48

56:                                               ; preds = %48, %42
  %57 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %58 = load i64, ptr %57, align 8, !tbaa !11
  %59 = add nsw i64 %58, 1
  %60 = shl i64 %59, 2
  %61 = tail call ptr @realloc(ptr noundef nonnull %35, i64 noundef %60) #5
  %62 = icmp eq ptr %61, null
  br i1 %62, label %63, label %65

63:                                               ; preds = %56
  %64 = load ptr, ptr %30, align 8, !tbaa !12
  br label %85

65:                                               ; preds = %56
  store ptr %61, ptr %34, align 8, !tbaa !13
  %66 = icmp slt i64 %46, %58
  br i1 %66, label %67, label %72

67:                                               ; preds = %65
  %68 = getelementptr i32, ptr %61, i64 %46
  %69 = getelementptr i8, ptr %68, i64 4
  %70 = sub nsw i64 %58, %46
  %71 = shl i64 %70, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %69, ptr nonnull align 4 %68, i64 %71, i1 false)
  br label %72

72:                                               ; preds = %67, %65
  %73 = getelementptr inbounds i32, ptr %61, i64 %46
  store i32 %3, ptr %73, align 4, !tbaa !15
  store i64 %59, ptr %57, align 8, !tbaa !11
  %74 = load i64, ptr %0, align 8, !tbaa !3
  %75 = icmp sgt i64 %74, %10
  %76 = load ptr, ptr %30, align 8, !tbaa !12
  br i1 %75, label %77, label %85

77:                                               ; preds = %72, %77
  %78 = phi i64 [ %79, %77 ], [ %10, %72 ]
  %79 = add nuw nsw i64 %78, 1
  %80 = getelementptr inbounds nuw i64, ptr %76, i64 %79
  %81 = load i64, ptr %80, align 8, !tbaa !14
  %82 = add nsw i64 %81, 1
  store i64 %82, ptr %80, align 8, !tbaa !14
  %83 = load i64, ptr %0, align 8, !tbaa !3
  %84 = icmp slt i64 %79, %83
  br i1 %84, label %77, label %88, !llvm.loop !24

85:                                               ; preds = %51, %63, %72
  %86 = phi ptr [ %64, %63 ], [ %76, %72 ], [ %31, %51 ]
  %87 = icmp eq ptr %86, null
  br i1 %87, label %140, label %88

88:                                               ; preds = %77, %37, %85
  %89 = phi ptr [ %31, %37 ], [ %86, %85 ], [ %76, %77 ]
  %90 = load ptr, ptr %34, align 8, !tbaa !13
  %91 = icmp eq ptr %90, null
  br i1 %91, label %140, label %92

92:                                               ; preds = %88
  %93 = load i64, ptr %0, align 8, !tbaa !3
  %94 = icmp sgt i64 %93, %15
  %95 = icmp sgt i64 %93, %10
  %96 = and i1 %94, %95
  br i1 %96, label %97, label %140

97:                                               ; preds = %92
  %98 = getelementptr inbounds nuw i64, ptr %89, i64 %15
  %99 = load i64, ptr %98, align 8, !tbaa !14
  %100 = getelementptr inbounds nuw i8, ptr %98, i64 8
  %101 = load i64, ptr %100, align 8, !tbaa !14
  %102 = icmp slt i64 %99, %101
  br i1 %102, label %106, label %111

103:                                              ; preds = %106
  %104 = add i64 %107, 1
  %105 = icmp eq i64 %104, %101
  br i1 %105, label %111, label %106, !llvm.loop !23

106:                                              ; preds = %97, %103
  %107 = phi i64 [ %104, %103 ], [ %99, %97 ]
  %108 = getelementptr inbounds i32, ptr %90, i64 %107
  %109 = load i32, ptr %108, align 4, !tbaa !15
  %110 = icmp eq i32 %109, %2
  br i1 %110, label %140, label %103

111:                                              ; preds = %103, %97
  %112 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %113 = load i64, ptr %112, align 8, !tbaa !11
  %114 = add nsw i64 %113, 1
  %115 = shl i64 %114, 2
  %116 = tail call ptr @realloc(ptr noundef nonnull %90, i64 noundef %115) #5
  %117 = icmp eq ptr %116, null
  br i1 %117, label %118, label %120

118:                                              ; preds = %111
  %119 = load ptr, ptr %30, align 8, !tbaa !12
  br label %140

120:                                              ; preds = %111
  store ptr %116, ptr %34, align 8, !tbaa !13
  %121 = icmp slt i64 %101, %113
  br i1 %121, label %122, label %127

122:                                              ; preds = %120
  %123 = getelementptr i32, ptr %116, i64 %101
  %124 = getelementptr i8, ptr %123, i64 4
  %125 = sub nsw i64 %113, %101
  %126 = shl i64 %125, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr align 4 %124, ptr nonnull align 4 %123, i64 %126, i1 false)
  br label %127

127:                                              ; preds = %122, %120
  %128 = getelementptr inbounds i32, ptr %116, i64 %101
  store i32 %2, ptr %128, align 4, !tbaa !15
  store i64 %114, ptr %112, align 8, !tbaa !11
  %129 = load i64, ptr %0, align 8, !tbaa !3
  %130 = icmp sgt i64 %129, %15
  %131 = load ptr, ptr %30, align 8, !tbaa !12
  br i1 %130, label %132, label %140

132:                                              ; preds = %127, %132
  %133 = phi i64 [ %134, %132 ], [ %15, %127 ]
  %134 = add nuw nsw i64 %133, 1
  %135 = getelementptr inbounds nuw i64, ptr %131, i64 %134
  %136 = load i64, ptr %135, align 8, !tbaa !14
  %137 = add nsw i64 %136, 1
  store i64 %137, ptr %135, align 8, !tbaa !14
  %138 = load i64, ptr %0, align 8, !tbaa !3
  %139 = icmp slt i64 %134, %138
  br i1 %139, label %132, label %140, !llvm.loop !24

140:                                              ; preds = %106, %132, %33, %118, %29, %85, %88, %92, %127
  %141 = phi ptr [ %119, %118 ], [ null, %29 ], [ null, %85 ], [ %89, %88 ], [ %89, %92 ], [ %131, %127 ], [ %31, %33 ], [ %131, %132 ], [ %89, %106 ]
  %142 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %143 = load ptr, ptr %142, align 8, !tbaa !13
  tail call void @autograph_update_csr_pointers(ptr noundef nonnull %0, ptr noundef %141, ptr noundef %143) #4
  br label %144

144:                                              ; preds = %13, %9, %5, %140
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @graph_remove_edge(ptr noundef %0, ptr nocapture noundef readnone %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr #0 {
  %6 = icmp eq ptr %0, null
  %7 = icmp slt i32 %2, 0
  %8 = or i1 %6, %7
  br i1 %8, label %136, label %9

9:                                                ; preds = %5
  %10 = zext nneg i32 %2 to i64
  %11 = load i64, ptr %0, align 8, !tbaa !3
  %12 = icmp sgt i64 %11, %10
  br i1 %12, label %13, label %136

13:                                               ; preds = %9
  %14 = icmp sgt i32 %3, -1
  %15 = zext nneg i32 %3 to i64
  %16 = icmp samesign ugt i64 %11, %15
  %17 = select i1 %14, i1 %16, i1 false
  br i1 %17, label %18, label %136

18:                                               ; preds = %13
  %19 = tail call i32 @autograph_get_layout(ptr noundef nonnull %0) #4
  %20 = icmp eq i32 %19, 0
  br i1 %20, label %29, label %21

21:                                               ; preds = %18
  %22 = load i64, ptr %0, align 8, !tbaa !3
  %23 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %24 = load i64, ptr %23, align 8, !tbaa !11
  %25 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %26 = load ptr, ptr %25, align 8, !tbaa !12
  %27 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %28 = load ptr, ptr %27, align 8, !tbaa !13
  tail call void @autograph_ensure_layout(ptr noundef nonnull %0, i64 noundef %22, i64 noundef %24, ptr noundef %26, ptr noundef %28, ptr noundef null, ptr noundef null, ptr noundef null, i32 noundef 0) #4
  br label %29

29:                                               ; preds = %18, %21
  %30 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %31 = load ptr, ptr %30, align 8, !tbaa !12
  %32 = icmp eq ptr %31, null
  br i1 %32, label %136, label %33

33:                                               ; preds = %29
  %34 = getelementptr inbounds nuw i8, ptr %0, i64 24
  %35 = load ptr, ptr %34, align 8, !tbaa !13
  %36 = icmp eq ptr %35, null
  br i1 %36, label %136, label %37

37:                                               ; preds = %33
  %38 = load i64, ptr %0, align 8, !tbaa !3
  %39 = icmp sgt i64 %38, %10
  br i1 %39, label %40, label %89

40:                                               ; preds = %37
  %41 = getelementptr inbounds nuw i64, ptr %31, i64 %10
  %42 = load i64, ptr %41, align 8, !tbaa !14
  %43 = getelementptr inbounds nuw i8, ptr %41, i64 8
  %44 = load i64, ptr %43, align 8, !tbaa !14
  %45 = icmp slt i64 %42, %44
  br i1 %45, label %46, label %85

46:                                               ; preds = %40, %51
  %47 = phi i64 [ %52, %51 ], [ %42, %40 ]
  %48 = getelementptr inbounds i32, ptr %35, i64 %47
  %49 = load i32, ptr %48, align 4, !tbaa !15
  %50 = icmp eq i32 %49, %3
  br i1 %50, label %54, label %51

51:                                               ; preds = %46
  %52 = add i64 %47, 1
  %53 = icmp eq i64 %52, %44
  br i1 %53, label %82, label %46, !llvm.loop !17

54:                                               ; preds = %46
  %55 = getelementptr inbounds i32, ptr %35, i64 %47
  %56 = icmp slt i64 %47, 0
  br i1 %56, label %82, label %57

57:                                               ; preds = %54
  %58 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %59 = load i64, ptr %58, align 8, !tbaa !11
  %60 = add nuw nsw i64 %47, 1
  %61 = icmp slt i64 %60, %59
  br i1 %61, label %62, label %69

62:                                               ; preds = %57
  %63 = getelementptr inbounds nuw i32, ptr %35, i64 %60
  %64 = xor i64 %47, -1
  %65 = add nsw i64 %59, %64
  %66 = shl i64 %65, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %55, ptr nonnull align 4 %63, i64 %66, i1 false)
  %67 = load i64, ptr %0, align 8, !tbaa !3
  %68 = load ptr, ptr %30, align 8, !tbaa !12
  br label %69

69:                                               ; preds = %62, %57
  %70 = phi ptr [ %68, %62 ], [ %31, %57 ]
  %71 = phi i64 [ %67, %62 ], [ %38, %57 ]
  %72 = add nsw i64 %59, -1
  store i64 %72, ptr %58, align 8, !tbaa !11
  %73 = icmp sgt i64 %71, %10
  br i1 %73, label %74, label %82

74:                                               ; preds = %69, %74
  %75 = phi i64 [ %76, %74 ], [ %10, %69 ]
  %76 = add nuw nsw i64 %75, 1
  %77 = getelementptr inbounds nuw i64, ptr %70, i64 %76
  %78 = load i64, ptr %77, align 8, !tbaa !14
  %79 = add nsw i64 %78, -1
  store i64 %79, ptr %77, align 8, !tbaa !14
  %80 = load i64, ptr %0, align 8, !tbaa !3
  %81 = icmp slt i64 %76, %80
  br i1 %81, label %74, label %85, !llvm.loop !19

82:                                               ; preds = %51, %54, %69
  %83 = phi ptr [ %31, %54 ], [ %70, %69 ], [ %31, %51 ]
  %84 = icmp eq ptr %83, null
  br i1 %84, label %136, label %85

85:                                               ; preds = %74, %40, %82
  %86 = phi ptr [ %31, %40 ], [ %83, %82 ], [ %70, %74 ]
  %87 = load ptr, ptr %34, align 8, !tbaa !13
  %88 = icmp eq ptr %87, null
  br i1 %88, label %136, label %89

89:                                               ; preds = %37, %85
  %90 = phi ptr [ %86, %85 ], [ %31, %37 ]
  %91 = phi ptr [ %87, %85 ], [ %35, %37 ]
  %92 = load i64, ptr %0, align 8, !tbaa !3
  %93 = icmp sgt i64 %92, %15
  br i1 %93, label %94, label %136

94:                                               ; preds = %89
  %95 = getelementptr inbounds nuw i64, ptr %90, i64 %15
  %96 = load i64, ptr %95, align 8, !tbaa !14
  %97 = getelementptr inbounds nuw i8, ptr %95, i64 8
  %98 = load i64, ptr %97, align 8, !tbaa !14
  %99 = icmp slt i64 %96, %98
  br i1 %99, label %100, label %136

100:                                              ; preds = %94, %105
  %101 = phi i64 [ %106, %105 ], [ %96, %94 ]
  %102 = getelementptr inbounds i32, ptr %91, i64 %101
  %103 = load i32, ptr %102, align 4, !tbaa !15
  %104 = icmp eq i32 %103, %2
  br i1 %104, label %108, label %105

105:                                              ; preds = %100
  %106 = add i64 %101, 1
  %107 = icmp eq i64 %106, %98
  br i1 %107, label %136, label %100, !llvm.loop !17

108:                                              ; preds = %100
  %109 = getelementptr inbounds i32, ptr %91, i64 %101
  %110 = icmp slt i64 %101, 0
  br i1 %110, label %136, label %111

111:                                              ; preds = %108
  %112 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %113 = load i64, ptr %112, align 8, !tbaa !11
  %114 = add nuw nsw i64 %101, 1
  %115 = icmp slt i64 %114, %113
  br i1 %115, label %116, label %122

116:                                              ; preds = %111
  %117 = getelementptr inbounds nuw i32, ptr %91, i64 %114
  %118 = xor i64 %101, -1
  %119 = add nsw i64 %113, %118
  %120 = shl i64 %119, 2
  tail call void @llvm.memmove.p0.p0.i64(ptr nonnull align 4 %109, ptr nonnull align 4 %117, i64 %120, i1 false)
  %121 = load i64, ptr %0, align 8, !tbaa !3
  br label %122

122:                                              ; preds = %116, %111
  %123 = phi i64 [ %121, %116 ], [ %92, %111 ]
  %124 = add nsw i64 %113, -1
  store i64 %124, ptr %112, align 8, !tbaa !11
  %125 = icmp sgt i64 %123, %15
  br i1 %125, label %126, label %136

126:                                              ; preds = %122
  %127 = load ptr, ptr %30, align 8, !tbaa !12
  br label %128

128:                                              ; preds = %128, %126
  %129 = phi i64 [ %15, %126 ], [ %130, %128 ]
  %130 = add nuw nsw i64 %129, 1
  %131 = getelementptr inbounds nuw i64, ptr %127, i64 %130
  %132 = load i64, ptr %131, align 8, !tbaa !14
  %133 = add nsw i64 %132, -1
  store i64 %133, ptr %131, align 8, !tbaa !14
  %134 = load i64, ptr %0, align 8, !tbaa !3
  %135 = icmp slt i64 %130, %134
  br i1 %135, label %128, label %136, !llvm.loop !19

136:                                              ; preds = %105, %128, %33, %29, %122, %108, %94, %89, %85, %82, %13, %9, %5
  ret void
}

declare dso_local i32 @autograph_get_layout(ptr noundef) local_unnamed_addr #2

declare dso_local void @autograph_ensure_layout(ptr noundef, i64 noundef, i64 noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef, i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nounwind }
attributes #5 = { nounwind allocsize(1) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"", !5, i64 0, !5, i64 8, !8, i64 16, !10, i64 24}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"p1 long", !9, i64 0}
!9 = !{!"any pointer", !6, i64 0}
!10 = !{!"p1 int", !9, i64 0}
!11 = !{!4, !5, i64 8}
!12 = !{!4, !8, i64 16}
!13 = !{!4, !10, i64 24}
!14 = !{!5, !5, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !6, i64 0}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.mustprogress"}
!19 = distinct !{!19, !18}
!20 = distinct !{!20, !18}
!21 = distinct !{!21, !18}
!22 = distinct !{!22, !18}
!23 = distinct !{!23, !18}
!24 = distinct !{!24, !18}
