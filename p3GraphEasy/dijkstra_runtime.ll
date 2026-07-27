; ModuleID = 'dijkstra_runtime.bc'
source_filename = "dijkstra_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<long long, std::allocator<long long>>::_Vector_impl" }
%"struct.std::_Vector_base<long long, std::allocator<long long>>::_Vector_impl" = type { %"struct.std::_Vector_base<long long, std::allocator<long long>>::_Vector_impl_data" }
%"struct.std::_Vector_base<long long, std::allocator<long long>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%struct.Graph = type { i64, i64, ptr, ptr, ptr }
%"struct.std::pair" = type <{ i64, i32, [4 x i8] }>

$_ZNSt6vectorIxSaIxEE14_M_fill_assignEmRKx = comdat any

$_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_ = comdat any

@.str.2 = private unnamed_addr constant [12 x i8] c"running1\0A  \00", align 1
@.str.4 = private unnamed_addr constant [9 x i8] c"running3\00", align 1
@.str.5 = private unnamed_addr constant [41 x i8] c"Shortest distances from source node %d:\0A\00", align 1
@.str.6 = private unnamed_addr constant [29 x i8] c"Node %ld: INF (unreachable)\0A\00", align 1
@.str.7 = private unnamed_addr constant [16 x i8] c"Node %ld: %lld\0A\00", align 1
@.str.9 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.10 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@str = private unnamed_addr constant [10 x i8] c"running00\00", align 1
@str.11 = private unnamed_addr constant [9 x i8] c"running0\00", align 1
@str.12 = private unnamed_addr constant [9 x i8] c"running2\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @dijkstra_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca i64, align 8
  %3 = alloca %"class.std::vector", align 8
  %4 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  %5 = icmp eq ptr %0, null
  br i1 %5, label %207, label %6

6:                                                ; preds = %1
  %7 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.11)
  %8 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %9 = load ptr, ptr %8, align 8, !tbaa !3
  %10 = icmp eq ptr %9, null
  br i1 %10, label %207, label %11

11:                                               ; preds = %6
  %12 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %13 = load ptr, ptr %12, align 8, !tbaa !9
  %14 = icmp eq ptr %13, null
  br i1 %14, label %207, label %15

15:                                               ; preds = %11
  %16 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 4
  %17 = load ptr, ptr %16, align 8, !tbaa !10
  %18 = icmp eq ptr %17, null
  br i1 %18, label %207, label %19

19:                                               ; preds = %15
  %20 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.2)
  %21 = load i64, ptr %0, align 8, !tbaa !11
  %22 = icmp slt i64 %21, 1
  br i1 %22, label %207, label %23

23:                                               ; preds = %19
  %24 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.12)
  %25 = trunc i64 %21 to i32
  %26 = icmp sgt i32 %25, 0
  br i1 %26, label %27, label %207

27:                                               ; preds = %23
  %28 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.4)
  %29 = load ptr, ptr %8, align 8, !tbaa !3
  %30 = load ptr, ptr %12, align 8, !tbaa !9
  %31 = load ptr, ptr %16, align 8, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %2) #11
  store i64 2305843009213693951, ptr %2, align 8, !tbaa !12
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %3) #11
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %3, i8 0, i64 24, i1 false)
  invoke void @_ZNSt6vectorIxSaIxEE14_M_fill_assignEmRKx(ptr noundef nonnull align 8 dereferenceable(24) %3, i64 noundef %21, ptr noundef nonnull align 8 dereferenceable(8) %2)
          to label %32 unwind label %63

32:                                               ; preds = %27
  %33 = load ptr, ptr %3, align 8, !tbaa !14
  store i64 0, ptr %33, align 8, !tbaa !12
  %34 = invoke noalias noundef nonnull dereferenceable(16) ptr @_Znwm(i64 noundef 16) #12
          to label %35 unwind label %65

35:                                               ; preds = %32
  %36 = getelementptr inbounds %"struct.std::pair", ptr %34, i64 1
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(12) %34, i8 0, i64 12, i1 false)
  br label %37

37:                                               ; preds = %35, %178
  %38 = phi ptr [ %34, %35 ], [ %181, %178 ]
  %39 = phi ptr [ %36, %35 ], [ %180, %178 ]
  %40 = phi ptr [ %36, %35 ], [ %179, %178 ]
  %41 = load i64, ptr %38, align 8
  %42 = getelementptr inbounds i8, ptr %38, i64 8
  %43 = load i32, ptr %42, align 8
  %44 = ptrtoint ptr %39 to i64
  %45 = ptrtoint ptr %38 to i64
  %46 = sub i64 %44, %45
  %47 = icmp sgt i64 %46, 16
  br i1 %47, label %48, label %56

48:                                               ; preds = %37
  %49 = getelementptr inbounds %"struct.std::pair", ptr %39, i64 -1
  %50 = load i64, ptr %49, align 8
  %51 = getelementptr %"struct.std::pair", ptr %39, i64 -1, i32 1
  %52 = load i32, ptr %51, align 8
  store i64 %41, ptr %49, align 8, !tbaa !16
  store i32 %43, ptr %51, align 8, !tbaa !19
  %53 = ptrtoint ptr %49 to i64
  %54 = sub i64 %53, %45
  %55 = ashr exact i64 %54, 4
  invoke void @_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_(ptr nonnull %38, i64 noundef 0, i64 noundef %55, i64 %50, i32 %52)
          to label %56 unwind label %67

56:                                               ; preds = %37, %48
  %57 = getelementptr inbounds %"struct.std::pair", ptr %39, i64 -1
  %58 = sext i32 %43 to i64
  %59 = load ptr, ptr %3, align 8, !tbaa !14
  %60 = getelementptr inbounds i64, ptr %59, i64 %58
  %61 = load i64, ptr %60, align 8, !tbaa !12
  %62 = icmp eq i64 %41, %61
  br i1 %62, label %69, label %178, !llvm.loop !20

63:                                               ; preds = %27
  %64 = landingpad { ptr, i32 }
          cleanup
  br label %218

65:                                               ; preds = %32
  %66 = landingpad { ptr, i32 }
          cleanup
  br label %222

67:                                               ; preds = %48
  %68 = landingpad { ptr, i32 }
          cleanup
  br label %215

69:                                               ; preds = %56
  %70 = getelementptr inbounds i64, ptr %29, i64 %58
  %71 = load i64, ptr %70, align 8, !tbaa !22
  %72 = getelementptr i64, ptr %70, i64 1
  %73 = load i64, ptr %72, align 8, !tbaa !22
  %74 = icmp slt i64 %71, %73
  br i1 %74, label %75, label %178

75:                                               ; preds = %69
  %76 = icmp eq i64 %41, 2305843009213693951
  br label %77

77:                                               ; preds = %75, %172
  %78 = phi i64 [ %71, %75 ], [ %176, %172 ]
  %79 = phi ptr [ %38, %75 ], [ %175, %172 ]
  %80 = phi ptr [ %57, %75 ], [ %174, %172 ]
  %81 = phi ptr [ %40, %75 ], [ %173, %172 ]
  %82 = getelementptr inbounds i32, ptr %30, i64 %78
  %83 = load i32, ptr %82, align 4, !tbaa !23
  %84 = getelementptr inbounds i32, ptr %31, i64 %78
  %85 = load i32, ptr %84, align 4, !tbaa !23
  %86 = icmp slt i32 %85, 0
  %87 = select i1 %86, i1 true, i1 %76
  br i1 %87, label %172, label %88

88:                                               ; preds = %77
  %89 = zext nneg i32 %85 to i64
  %90 = add nsw i64 %41, %89
  %91 = sext i32 %83 to i64
  %92 = load ptr, ptr %3, align 8, !tbaa !14
  %93 = getelementptr inbounds i64, ptr %92, i64 %91
  %94 = load i64, ptr %93, align 8, !tbaa !12
  %95 = icmp slt i64 %90, %94
  br i1 %95, label %96, label %172

96:                                               ; preds = %88
  store i64 %90, ptr %93, align 8, !tbaa !12
  %97 = icmp eq ptr %80, %81
  br i1 %97, label %100, label %98

98:                                               ; preds = %96
  store i64 %90, ptr %80, align 8, !tbaa !16
  %99 = getelementptr inbounds %"struct.std::pair", ptr %80, i64 0, i32 1
  store i32 %83, ptr %99, align 8, !tbaa !19
  br label %134

100:                                              ; preds = %96
  %101 = ptrtoint ptr %80 to i64
  %102 = ptrtoint ptr %79 to i64
  %103 = sub i64 %101, %102
  %104 = ashr exact i64 %103, 4
  %105 = icmp sgt i64 %104, -1
  call void @llvm.assume(i1 %105)
  %106 = icmp eq i64 %103, 9223372036854775792
  br i1 %106, label %107, label %109

107:                                              ; preds = %100
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.10) #13
          to label %108 unwind label %210

108:                                              ; preds = %107
  unreachable

109:                                              ; preds = %100
  %110 = call i64 @llvm.umax.i64(i64 %104, i64 1)
  %111 = add nuw nsw i64 %110, %104
  %112 = call noundef i64 @llvm.umin.i64(i64 %111, i64 576460752303423487)
  %113 = shl nuw nsw i64 %112, 4
  %114 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %113) #12
          to label %115 unwind label %208

115:                                              ; preds = %109
  %116 = getelementptr inbounds i8, ptr %114, i64 %103
  store i64 %90, ptr %116, align 8, !tbaa !16
  %117 = getelementptr inbounds %"struct.std::pair", ptr %116, i64 0, i32 1
  store i32 %83, ptr %117, align 8, !tbaa !19
  %118 = icmp eq ptr %79, %80
  br i1 %118, label %125, label %119

119:                                              ; preds = %115, %119
  %120 = phi ptr [ %123, %119 ], [ %114, %115 ]
  %121 = phi ptr [ %122, %119 ], [ %79, %115 ]
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %120, ptr noundef nonnull align 8 dereferenceable(16) %121, i64 16, i1 false), !alias.scope !24
  %122 = getelementptr inbounds %"struct.std::pair", ptr %121, i64 1
  %123 = getelementptr inbounds %"struct.std::pair", ptr %120, i64 1
  %124 = icmp eq ptr %122, %80
  br i1 %124, label %125, label %119, !llvm.loop !28

125:                                              ; preds = %119, %115
  %126 = phi ptr [ %114, %115 ], [ %123, %119 ]
  %127 = icmp eq ptr %79, null
  br i1 %127, label %129, label %128

128:                                              ; preds = %125
  call void @_ZdlPv(ptr noundef nonnull %79) #14
  br label %129

129:                                              ; preds = %128, %125
  %130 = getelementptr inbounds %"struct.std::pair", ptr %114, i64 %112
  %131 = load i64, ptr %126, align 8
  %132 = getelementptr %"struct.std::pair", ptr %126, i64 0, i32 1
  %133 = load i32, ptr %132, align 8
  br label %134

134:                                              ; preds = %129, %98
  %135 = phi i32 [ %133, %129 ], [ %83, %98 ]
  %136 = phi i64 [ %131, %129 ], [ %90, %98 ]
  %137 = phi ptr [ %130, %129 ], [ %81, %98 ]
  %138 = phi ptr [ %126, %129 ], [ %80, %98 ]
  %139 = phi ptr [ %114, %129 ], [ %79, %98 ]
  %140 = getelementptr inbounds %"struct.std::pair", ptr %138, i64 1
  %141 = ptrtoint ptr %140 to i64
  %142 = ptrtoint ptr %139 to i64
  %143 = sub i64 %141, %142
  %144 = ashr exact i64 %143, 4
  %145 = add nsw i64 %144, -1
  %146 = icmp sgt i64 %144, 1
  br i1 %146, label %147, label %168

147:                                              ; preds = %134, %163
  %148 = phi i64 [ %150, %163 ], [ %145, %134 ]
  %149 = add nsw i64 %148, -1
  %150 = lshr i64 %149, 1
  %151 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %150
  %152 = load i64, ptr %151, align 8, !tbaa !12
  %153 = icmp sgt i64 %152, %136
  br i1 %153, label %154, label %157

154:                                              ; preds = %147
  %155 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %150, i32 1
  %156 = load i32, ptr %155, align 4, !tbaa !23
  br label %163

157:                                              ; preds = %147
  %158 = icmp slt i64 %152, %136
  br i1 %158, label %168, label %159

159:                                              ; preds = %157
  %160 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %150, i32 1
  %161 = load i32, ptr %160, align 8, !tbaa !23
  %162 = icmp sgt i32 %161, %135
  br i1 %162, label %163, label %168

163:                                              ; preds = %159, %154
  %164 = phi i32 [ %156, %154 ], [ %161, %159 ]
  %165 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %148
  store i64 %152, ptr %165, align 8, !tbaa !16
  %166 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %148, i32 1
  store i32 %164, ptr %166, align 8, !tbaa !19
  %167 = icmp ult i64 %149, 2
  br i1 %167, label %168, label %147, !llvm.loop !29

168:                                              ; preds = %157, %159, %163, %134
  %169 = phi i64 [ %145, %134 ], [ %148, %159 ], [ 0, %163 ], [ %148, %157 ]
  %170 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %169
  store i64 %136, ptr %170, align 8, !tbaa !16
  %171 = getelementptr inbounds %"struct.std::pair", ptr %139, i64 %169, i32 1
  store i32 %135, ptr %171, align 8, !tbaa !19
  br label %172

172:                                              ; preds = %88, %168, %77
  %173 = phi ptr [ %81, %77 ], [ %137, %168 ], [ %81, %88 ]
  %174 = phi ptr [ %80, %77 ], [ %140, %168 ], [ %80, %88 ]
  %175 = phi ptr [ %79, %77 ], [ %139, %168 ], [ %79, %88 ]
  %176 = add i64 %78, 1
  %177 = icmp eq i64 %176, %73
  br i1 %177, label %178, label %77, !llvm.loop !30

178:                                              ; preds = %172, %69, %56
  %179 = phi ptr [ %40, %56 ], [ %40, %69 ], [ %173, %172 ]
  %180 = phi ptr [ %57, %56 ], [ %57, %69 ], [ %174, %172 ]
  %181 = phi ptr [ %38, %56 ], [ %38, %69 ], [ %175, %172 ]
  %182 = icmp eq ptr %181, %180
  br i1 %182, label %183, label %37

183:                                              ; preds = %178
  %184 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.5, i32 noundef 0)
  br label %194

185:                                              ; preds = %204
  %186 = call i32 @putchar(i32 10)
  %187 = icmp eq ptr %180, null
  br i1 %187, label %189, label %188

188:                                              ; preds = %185
  call void @_ZdlPv(ptr noundef nonnull %180) #14
  br label %189

189:                                              ; preds = %185, %188
  %190 = load ptr, ptr %3, align 8, !tbaa !14
  %191 = icmp eq ptr %190, null
  br i1 %191, label %193, label %192

192:                                              ; preds = %189
  call void @_ZdlPv(ptr noundef nonnull %190) #14
  br label %193

193:                                              ; preds = %189, %192
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #11
  br label %207

194:                                              ; preds = %183, %204
  %195 = phi i64 [ %205, %204 ], [ 0, %183 ]
  %196 = load ptr, ptr %3, align 8, !tbaa !14
  %197 = getelementptr inbounds i64, ptr %196, i64 %195
  %198 = load i64, ptr %197, align 8, !tbaa !12
  %199 = icmp sgt i64 %198, 1152921504606846974
  br i1 %199, label %200, label %202

200:                                              ; preds = %194
  %201 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.6, i64 noundef %195)
  br label %204

202:                                              ; preds = %194
  %203 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.7, i64 noundef %195, i64 noundef %198)
  br label %204

204:                                              ; preds = %200, %202
  %205 = add nuw nsw i64 %195, 1
  %206 = icmp eq i64 %205, %21
  br i1 %206, label %185, label %194, !llvm.loop !31

207:                                              ; preds = %193, %19, %23, %6, %11, %15, %1
  ret void

208:                                              ; preds = %109
  %209 = landingpad { ptr, i32 }
          cleanup
  br label %212

210:                                              ; preds = %107
  %211 = landingpad { ptr, i32 }
          cleanup
  br label %212

212:                                              ; preds = %210, %208
  %213 = phi { ptr, i32 } [ %209, %208 ], [ %211, %210 ]
  %214 = icmp eq ptr %79, null
  br i1 %214, label %218, label %215

215:                                              ; preds = %67, %212
  %216 = phi { ptr, i32 } [ %68, %67 ], [ %213, %212 ]
  %217 = phi ptr [ %38, %67 ], [ %79, %212 ]
  call void @_ZdlPv(ptr noundef nonnull %217) #14
  br label %218

218:                                              ; preds = %215, %212, %63
  %219 = phi { ptr, i32 } [ %216, %215 ], [ %213, %212 ], [ %64, %63 ]
  %220 = load ptr, ptr %3, align 8, !tbaa !14
  %221 = icmp eq ptr %220, null
  br i1 %221, label %225, label %222

222:                                              ; preds = %65, %218
  %223 = phi { ptr, i32 } [ %66, %65 ], [ %219, %218 ]
  %224 = phi ptr [ %33, %65 ], [ %220, %218 ]
  call void @_ZdlPv(ptr noundef nonnull %224) #14
  br label %225

225:                                              ; preds = %218, %222
  %226 = phi { ptr, i32 } [ %219, %218 ], [ %223, %222 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %3) #11
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %2) #11
  resume { ptr, i32 } %226
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #4

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNSt6vectorIxSaIxEE14_M_fill_assignEmRKx(ptr noundef nonnull align 8 dereferenceable(24) %0, i64 noundef %1, ptr noundef nonnull align 8 dereferenceable(8) %2) local_unnamed_addr #0 comdat align 2 personality ptr @__gxx_personality_v0 {
  %4 = getelementptr inbounds %"struct.std::_Vector_base<long long, std::allocator<long long>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  %5 = load ptr, ptr %4, align 8, !tbaa !32
  %6 = load ptr, ptr %0, align 8, !tbaa !32
  %7 = ptrtoint ptr %5 to i64
  %8 = ptrtoint ptr %6 to i64
  %9 = sub i64 %7, %8
  %10 = ashr exact i64 %9, 3
  %11 = icmp sgt i64 %10, -1
  tail call void @llvm.assume(i1 %11)
  %12 = getelementptr inbounds %"struct.std::_Vector_base<long long, std::allocator<long long>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  %13 = load ptr, ptr %12, align 8, !tbaa !33
  %14 = ptrtoint ptr %13 to i64
  %15 = sub i64 %14, %8
  %16 = ashr exact i64 %15, 3
  %17 = icmp sgt i64 %16, -1
  tail call void @llvm.assume(i1 %17)
  %18 = icmp ult i64 %16, %1
  br i1 %18, label %19, label %57

19:                                               ; preds = %3
  %20 = icmp ugt i64 %1, 1152921504606846975
  br i1 %20, label %21, label %22

21:                                               ; preds = %19
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.9) #13
  unreachable

22:                                               ; preds = %19
  %23 = shl nuw nsw i64 %1, 3
  %24 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %23) #12
  %25 = load i64, ptr %2, align 8, !tbaa !12
  %26 = icmp ult i64 %1, 4
  br i1 %26, label %43, label %27

27:                                               ; preds = %22
  %28 = and i64 %1, 1152921504606846972
  %29 = and i64 %1, 3
  %30 = shl nuw nsw i64 %28, 3
  %31 = getelementptr i8, ptr %24, i64 %30
  %32 = insertelement <2 x i64> poison, i64 %25, i64 0
  %33 = shufflevector <2 x i64> %32, <2 x i64> poison, <2 x i32> zeroinitializer
  br label %34

34:                                               ; preds = %34, %27
  %35 = phi i64 [ 0, %27 ], [ %39, %34 ]
  %36 = shl i64 %35, 3
  %37 = getelementptr i8, ptr %24, i64 %36
  %38 = getelementptr i64, ptr %37, i64 2
  store <2 x i64> %33, ptr %37, align 8, !tbaa !12
  store <2 x i64> %33, ptr %38, align 8, !tbaa !12
  %39 = add nuw i64 %35, 4
  %40 = icmp eq i64 %39, %28
  br i1 %40, label %41, label %34, !llvm.loop !34

41:                                               ; preds = %34
  %42 = icmp eq i64 %28, %1
  br i1 %42, label %52, label %43

43:                                               ; preds = %22, %41
  %44 = phi i64 [ %1, %22 ], [ %29, %41 ]
  %45 = phi ptr [ %24, %22 ], [ %31, %41 ]
  br label %46

46:                                               ; preds = %43, %46
  %47 = phi i64 [ %49, %46 ], [ %44, %43 ]
  %48 = phi ptr [ %50, %46 ], [ %45, %43 ]
  %49 = add i64 %47, -1
  store i64 %25, ptr %48, align 8, !tbaa !12
  %50 = getelementptr inbounds i64, ptr %48, i64 1
  %51 = icmp eq i64 %49, 0
  br i1 %51, label %52, label %46, !llvm.loop !37

52:                                               ; preds = %46, %41
  %53 = phi ptr [ %31, %41 ], [ %50, %46 ]
  %54 = getelementptr inbounds i64, ptr %24, i64 %1
  store ptr %24, ptr %0, align 8, !tbaa !14
  store ptr %53, ptr %4, align 8, !tbaa !38
  store ptr %54, ptr %12, align 8, !tbaa !33
  %55 = icmp eq ptr %6, null
  br i1 %55, label %156, label %56

56:                                               ; preds = %52
  tail call void @_ZdlPv(ptr noundef nonnull %6) #14
  br label %156

57:                                               ; preds = %3
  %58 = icmp ult i64 %10, %1
  br i1 %58, label %59, label %122

59:                                               ; preds = %57
  %60 = load i64, ptr %2, align 8, !tbaa !12
  %61 = icmp eq ptr %6, %5
  br i1 %61, label %89, label %62

62:                                               ; preds = %59
  %63 = add i64 %7, -8
  %64 = sub i64 %63, %8
  %65 = lshr i64 %64, 3
  %66 = add nuw nsw i64 %65, 1
  %67 = icmp ult i64 %64, 24
  br i1 %67, label %83, label %68

68:                                               ; preds = %62
  %69 = and i64 %66, 4611686018427387900
  %70 = shl i64 %69, 3
  %71 = getelementptr i8, ptr %6, i64 %70
  %72 = insertelement <2 x i64> poison, i64 %60, i64 0
  %73 = shufflevector <2 x i64> %72, <2 x i64> poison, <2 x i32> zeroinitializer
  br label %74

74:                                               ; preds = %74, %68
  %75 = phi i64 [ 0, %68 ], [ %79, %74 ]
  %76 = shl i64 %75, 3
  %77 = getelementptr i8, ptr %6, i64 %76
  %78 = getelementptr i64, ptr %77, i64 2
  store <2 x i64> %73, ptr %77, align 8, !tbaa !12
  store <2 x i64> %73, ptr %78, align 8, !tbaa !12
  %79 = add nuw i64 %75, 4
  %80 = icmp eq i64 %79, %69
  br i1 %80, label %81, label %74, !llvm.loop !39

81:                                               ; preds = %74
  %82 = icmp eq i64 %66, %69
  br i1 %82, label %89, label %83

83:                                               ; preds = %62, %81
  %84 = phi ptr [ %6, %62 ], [ %71, %81 ]
  br label %85

85:                                               ; preds = %83, %85
  %86 = phi ptr [ %87, %85 ], [ %84, %83 ]
  store i64 %60, ptr %86, align 8, !tbaa !12
  %87 = getelementptr inbounds i64, ptr %86, i64 1
  %88 = icmp eq ptr %87, %5
  br i1 %88, label %89, label %85, !llvm.loop !40

89:                                               ; preds = %85, %81, %59
  %90 = sub i64 %1, %10
  %91 = icmp eq i64 %90, 0
  br i1 %91, label %120, label %92

92:                                               ; preds = %89
  %93 = load i64, ptr %2, align 8, !tbaa !12
  %94 = icmp ult i64 %90, 4
  br i1 %94, label %111, label %95

95:                                               ; preds = %92
  %96 = and i64 %90, -4
  %97 = and i64 %90, 3
  %98 = shl i64 %96, 3
  %99 = getelementptr i8, ptr %5, i64 %98
  %100 = insertelement <2 x i64> poison, i64 %93, i64 0
  %101 = shufflevector <2 x i64> %100, <2 x i64> poison, <2 x i32> zeroinitializer
  br label %102

102:                                              ; preds = %102, %95
  %103 = phi i64 [ 0, %95 ], [ %107, %102 ]
  %104 = shl i64 %103, 3
  %105 = getelementptr i8, ptr %5, i64 %104
  %106 = getelementptr i64, ptr %105, i64 2
  store <2 x i64> %101, ptr %105, align 8, !tbaa !12
  store <2 x i64> %101, ptr %106, align 8, !tbaa !12
  %107 = add nuw i64 %103, 4
  %108 = icmp eq i64 %107, %96
  br i1 %108, label %109, label %102, !llvm.loop !41

109:                                              ; preds = %102
  %110 = icmp eq i64 %90, %96
  br i1 %110, label %120, label %111

111:                                              ; preds = %92, %109
  %112 = phi i64 [ %90, %92 ], [ %97, %109 ]
  %113 = phi ptr [ %5, %92 ], [ %99, %109 ]
  br label %114

114:                                              ; preds = %111, %114
  %115 = phi i64 [ %117, %114 ], [ %112, %111 ]
  %116 = phi ptr [ %118, %114 ], [ %113, %111 ]
  %117 = add i64 %115, -1
  store i64 %93, ptr %116, align 8, !tbaa !12
  %118 = getelementptr inbounds i64, ptr %116, i64 1
  %119 = icmp eq i64 %117, 0
  br i1 %119, label %120, label %114, !llvm.loop !42

120:                                              ; preds = %114, %109, %89
  %121 = phi ptr [ %5, %89 ], [ %99, %109 ], [ %118, %114 ]
  store ptr %121, ptr %4, align 8, !tbaa !38
  br label %156

122:                                              ; preds = %57
  %123 = icmp eq i64 %1, 0
  br i1 %123, label %152, label %124

124:                                              ; preds = %122
  %125 = getelementptr inbounds i64, ptr %6, i64 %1
  %126 = load i64, ptr %2, align 8, !tbaa !12
  %127 = add i64 %1, 2305843009213693951
  %128 = and i64 %127, 2305843009213693951
  %129 = add nuw nsw i64 %128, 1
  %130 = icmp ult i64 %128, 3
  br i1 %130, label %146, label %131

131:                                              ; preds = %124
  %132 = and i64 %129, 4611686018427387900
  %133 = shl i64 %132, 3
  %134 = getelementptr i8, ptr %6, i64 %133
  %135 = insertelement <2 x i64> poison, i64 %126, i64 0
  %136 = shufflevector <2 x i64> %135, <2 x i64> poison, <2 x i32> zeroinitializer
  br label %137

137:                                              ; preds = %137, %131
  %138 = phi i64 [ 0, %131 ], [ %142, %137 ]
  %139 = shl i64 %138, 3
  %140 = getelementptr i8, ptr %6, i64 %139
  %141 = getelementptr i64, ptr %140, i64 2
  store <2 x i64> %136, ptr %140, align 8, !tbaa !12
  store <2 x i64> %136, ptr %141, align 8, !tbaa !12
  %142 = add nuw i64 %138, 4
  %143 = icmp eq i64 %142, %132
  br i1 %143, label %144, label %137, !llvm.loop !43

144:                                              ; preds = %137
  %145 = icmp eq i64 %129, %132
  br i1 %145, label %152, label %146

146:                                              ; preds = %124, %144
  %147 = phi ptr [ %6, %124 ], [ %134, %144 ]
  br label %148

148:                                              ; preds = %146, %148
  %149 = phi ptr [ %150, %148 ], [ %147, %146 ]
  store i64 %126, ptr %149, align 8, !tbaa !12
  %150 = getelementptr inbounds i64, ptr %149, i64 1
  %151 = icmp eq ptr %150, %125
  br i1 %151, label %152, label %148, !llvm.loop !44

152:                                              ; preds = %148, %144, %122
  %153 = phi ptr [ %6, %122 ], [ %125, %144 ], [ %125, %148 ]
  %154 = icmp eq ptr %5, %153
  br i1 %154, label %156, label %155

155:                                              ; preds = %152
  store ptr %153, ptr %4, align 8, !tbaa !38
  br label %156

156:                                              ; preds = %155, %152, %56, %52, %120
  ret void
}

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_(ptr %0, i64 noundef %1, i64 noundef %2, i64 %3, i32 %4) local_unnamed_addr #0 comdat {
  %6 = add nsw i64 %2, -1
  %7 = sdiv i64 %6, 2
  %8 = icmp sgt i64 %7, %1
  br i1 %8, label %9, label %37

9:                                                ; preds = %5, %29
  %10 = phi i64 [ %31, %29 ], [ %1, %5 ]
  %11 = shl i64 %10, 1
  %12 = add i64 %11, 2
  %13 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %12
  %14 = or disjoint i64 %11, 1
  %15 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %14
  %16 = load i64, ptr %15, align 8, !tbaa !12
  %17 = load i64, ptr %13, align 8, !tbaa !12
  %18 = icmp slt i64 %16, %17
  br i1 %18, label %28, label %19

19:                                               ; preds = %9
  %20 = icmp slt i64 %17, %16
  br i1 %20, label %29, label %21

21:                                               ; preds = %19
  %22 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %14, i32 1
  %23 = load i32, ptr %22, align 8, !tbaa !19
  %24 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %12, i32 1
  %25 = load i32, ptr %24, align 8, !tbaa !19
  %26 = icmp slt i32 %23, %25
  %27 = freeze i1 %26
  br i1 %27, label %28, label %29

28:                                               ; preds = %9, %21
  br label %29

29:                                               ; preds = %19, %21, %28
  %30 = phi i64 [ %16, %28 ], [ %17, %21 ], [ %17, %19 ]
  %31 = phi i64 [ %14, %28 ], [ %12, %21 ], [ %12, %19 ]
  %32 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %10
  store i64 %30, ptr %32, align 8, !tbaa !16
  %33 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %31, i32 1
  %34 = load i32, ptr %33, align 4, !tbaa !23
  %35 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %10, i32 1
  store i32 %34, ptr %35, align 8, !tbaa !19
  %36 = icmp slt i64 %31, %7
  br i1 %36, label %9, label %37, !llvm.loop !45

37:                                               ; preds = %29, %5
  %38 = phi i64 [ %1, %5 ], [ %31, %29 ]
  %39 = and i64 %2, 1
  %40 = icmp eq i64 %39, 0
  br i1 %40, label %41, label %54

41:                                               ; preds = %37
  %42 = add nsw i64 %2, -2
  %43 = ashr exact i64 %42, 1
  %44 = icmp eq i64 %38, %43
  br i1 %44, label %45, label %54

45:                                               ; preds = %41
  %46 = shl nsw i64 %38, 1
  %47 = or disjoint i64 %46, 1
  %48 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %47
  %49 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %38
  %50 = load i64, ptr %48, align 8, !tbaa !12
  store i64 %50, ptr %49, align 8, !tbaa !16
  %51 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %47, i32 1
  %52 = load i32, ptr %51, align 4, !tbaa !23
  %53 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %38, i32 1
  store i32 %52, ptr %53, align 8, !tbaa !19
  br label %54

54:                                               ; preds = %45, %41, %37
  %55 = phi i64 [ %47, %45 ], [ %38, %41 ], [ %38, %37 ]
  %56 = icmp sgt i64 %55, %1
  br i1 %56, label %57, label %78

57:                                               ; preds = %54, %73
  %58 = phi i64 [ %60, %73 ], [ %55, %54 ]
  %59 = add nsw i64 %58, -1
  %60 = sdiv i64 %59, 2
  %61 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60
  %62 = load i64, ptr %61, align 8, !tbaa !12
  %63 = icmp sgt i64 %62, %3
  br i1 %63, label %64, label %67

64:                                               ; preds = %57
  %65 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60, i32 1
  %66 = load i32, ptr %65, align 4, !tbaa !23
  br label %73

67:                                               ; preds = %57
  %68 = icmp slt i64 %62, %3
  br i1 %68, label %78, label %69

69:                                               ; preds = %67
  %70 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60, i32 1
  %71 = load i32, ptr %70, align 8, !tbaa !23
  %72 = icmp sgt i32 %71, %4
  br i1 %72, label %73, label %78

73:                                               ; preds = %69, %64
  %74 = phi i32 [ %66, %64 ], [ %71, %69 ]
  %75 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %58
  store i64 %62, ptr %75, align 8, !tbaa !16
  %76 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %58, i32 1
  store i32 %74, ptr %76, align 8, !tbaa !19
  %77 = icmp sgt i64 %60, %1
  br i1 %77, label %57, label %78, !llvm.loop !29

78:                                               ; preds = %67, %69, %73, %54
  %79 = phi i64 [ %55, %54 ], [ %58, %69 ], [ %60, %73 ], [ %58, %67 ]
  %80 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %79
  store i64 %3, ptr %80, align 8, !tbaa !16
  %81 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %79, i32 1
  store i32 %4, ptr %81, align 8, !tbaa !19
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #8

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #9

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #10

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #10

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #9 = { nofree nounwind }
attributes #10 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #11 = { nounwind }
attributes #12 = { builtin allocsize(0) }
attributes #13 = { cold noreturn }
attributes #14 = { builtin nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!3 = !{!4, !8, i64 16}
!4 = !{!"_ZTS5Graph", !5, i64 0, !5, i64 8, !8, i64 16, !8, i64 24, !8, i64 32}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"any pointer", !6, i64 0}
!9 = !{!4, !8, i64 24}
!10 = !{!4, !8, i64 32}
!11 = !{!4, !5, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"long long", !6, i64 0}
!14 = !{!15, !8, i64 0}
!15 = !{!"_ZTSNSt12_Vector_baseIxSaIxEE17_Vector_impl_dataE", !8, i64 0, !8, i64 8, !8, i64 16}
!16 = !{!17, !13, i64 0}
!17 = !{!"_ZTSSt4pairIxiE", !13, i64 0, !18, i64 8}
!18 = !{!"int", !6, i64 0}
!19 = !{!17, !18, i64 8}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.mustprogress"}
!22 = !{!5, !5, i64 0}
!23 = !{!18, !18, i64 0}
!24 = !{!25, !27}
!25 = distinct !{!25, !26, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 0"}
!26 = distinct !{!26, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_"}
!27 = distinct !{!27, !26, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 1"}
!28 = distinct !{!28, !21}
!29 = distinct !{!29, !21}
!30 = distinct !{!30, !21}
!31 = distinct !{!31, !21}
!32 = !{!8, !8, i64 0}
!33 = !{!15, !8, i64 16}
!34 = distinct !{!34, !21, !35, !36}
!35 = !{!"llvm.loop.isvectorized", i32 1}
!36 = !{!"llvm.loop.unroll.runtime.disable"}
!37 = distinct !{!37, !21, !36, !35}
!38 = !{!15, !8, i64 8}
!39 = distinct !{!39, !21, !35, !36}
!40 = distinct !{!40, !21, !36, !35}
!41 = distinct !{!41, !21, !35, !36}
!42 = distinct !{!42, !21, !36, !35}
!43 = distinct !{!43, !21, !35, !36}
!44 = distinct !{!44, !21, !36, !35}
!45 = distinct !{!45, !21}
