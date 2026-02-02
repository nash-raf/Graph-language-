; ModuleID = 'dijkstra_runtime.cpp'
source_filename = "dijkstra_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%"class.std::priority_queue" = type <{ %"class.std::vector.0", %"struct.std::greater", [7 x i8] }>
%"class.std::vector.0" = type { %"struct.std::_Vector_base.1" }
%"struct.std::_Vector_base.1" = type { %"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl" }
%"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl" = type { %"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl_data" }
%"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"struct.std::greater" = type { i8 }
%struct.Graph = type { i64, i64, ptr, ptr, ptr }
%"struct.std::pair" = type <{ i64, i32, [4 x i8] }>

$_ZNSt6vectorISt4pairIxiESaIS1_EE17_M_realloc_insertIJxRiEEEvN9__gnu_cxx17__normal_iteratorIPS1_S3_EEDpOT_ = comdat any

$_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_ = comdat any

@.str = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_insert\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @dijkstra_runtime(ptr noundef readonly %0, i32 noundef %1, ptr noundef writeonly %2) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %4 = alloca i32, align 4
  %5 = alloca %"class.std::priority_queue", align 8
  %6 = alloca i64, align 8
  store i32 %1, ptr %4, align 4, !tbaa !5
  %7 = icmp eq ptr %0, null
  br i1 %7, label %289, label %8

8:                                                ; preds = %3
  %9 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %10 = load ptr, ptr %9, align 8, !tbaa !9
  %11 = icmp eq ptr %10, null
  br i1 %11, label %289, label %12

12:                                               ; preds = %8
  %13 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %14 = load ptr, ptr %13, align 8, !tbaa !13
  %15 = icmp eq ptr %14, null
  br i1 %15, label %289, label %16

16:                                               ; preds = %12
  %17 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 4
  %18 = load ptr, ptr %17, align 8, !tbaa !14
  %19 = icmp eq ptr %18, null
  br i1 %19, label %289, label %20

20:                                               ; preds = %16
  %21 = load i64, ptr %0, align 8, !tbaa !15
  %22 = icmp slt i64 %21, 1
  br i1 %22, label %289, label %23

23:                                               ; preds = %20
  %24 = icmp sgt i32 %1, -1
  %25 = trunc i64 %21 to i32
  %26 = icmp sgt i32 %25, %1
  %27 = and i1 %24, %26
  br i1 %27, label %28, label %289

28:                                               ; preds = %23
  %29 = icmp ugt i64 %21, 1152921504606846975
  br i1 %29, label %30, label %31

30:                                               ; preds = %28
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str) #9
  unreachable

31:                                               ; preds = %28
  %32 = shl nuw nsw i64 %21, 3
  %33 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %32) #10
  %34 = getelementptr inbounds i64, ptr %33, i64 %21
  %35 = add nuw nsw i64 %21, 2305843009213693951
  %36 = and i64 %35, 2305843009213693951
  %37 = add nuw nsw i64 %36, 1
  %38 = icmp ult i64 %36, 3
  br i1 %38, label %52, label %39

39:                                               ; preds = %31
  %40 = and i64 %37, 4611686018427387900
  %41 = shl i64 %40, 3
  %42 = getelementptr i8, ptr %33, i64 %41
  br label %43

43:                                               ; preds = %43, %39
  %44 = phi i64 [ 0, %39 ], [ %48, %43 ]
  %45 = shl i64 %44, 3
  %46 = getelementptr i8, ptr %33, i64 %45
  %47 = getelementptr i64, ptr %46, i64 2
  store <2 x i64> <i64 2305843009213693951, i64 2305843009213693951>, ptr %46, align 8, !tbaa !16
  store <2 x i64> <i64 2305843009213693951, i64 2305843009213693951>, ptr %47, align 8, !tbaa !16
  %48 = add nuw i64 %44, 4
  %49 = icmp eq i64 %48, %40
  br i1 %49, label %50, label %43, !llvm.loop !18

50:                                               ; preds = %43
  %51 = icmp eq i64 %37, %40
  br i1 %51, label %58, label %52

52:                                               ; preds = %31, %50
  %53 = phi ptr [ %33, %31 ], [ %42, %50 ]
  br label %54

54:                                               ; preds = %52, %54
  %55 = phi ptr [ %56, %54 ], [ %53, %52 ]
  store i64 2305843009213693951, ptr %55, align 8, !tbaa !16
  %56 = getelementptr inbounds i64, ptr %55, i64 1
  %57 = icmp eq ptr %56, %34
  br i1 %57, label %58, label %54, !llvm.loop !22

58:                                               ; preds = %54, %50
  %59 = sext i32 %1 to i64
  %60 = getelementptr inbounds i64, ptr %33, i64 %59
  store i64 0, ptr %60, align 8, !tbaa !16
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %5) #11
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %5, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %6) #11
  store i64 0, ptr %6, align 8, !tbaa !16
  %61 = getelementptr inbounds %"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl_data", ptr %5, i64 0, i32 1
  %62 = getelementptr inbounds %"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl_data", ptr %5, i64 0, i32 2
  invoke void @_ZNSt6vectorISt4pairIxiESaIS1_EE17_M_realloc_insertIJxRiEEEvN9__gnu_cxx17__normal_iteratorIPS1_S3_EEDpOT_(ptr noundef nonnull align 8 dereferenceable(24) %5, ptr null, ptr noundef nonnull align 8 dereferenceable(8) %6, ptr noundef nonnull align 4 dereferenceable(4) %4)
          to label %63 unwind label %129

63:                                               ; preds = %58
  %64 = load ptr, ptr %61, align 8, !tbaa !23
  %65 = load ptr, ptr %5, align 8, !tbaa !23
  %66 = getelementptr inbounds %"struct.std::pair", ptr %64, i64 -1
  %67 = load i64, ptr %66, align 8
  %68 = getelementptr %"struct.std::pair", ptr %64, i64 -1, i32 1
  %69 = load i32, ptr %68, align 8
  %70 = ptrtoint ptr %64 to i64
  %71 = ptrtoint ptr %65 to i64
  %72 = sub i64 %70, %71
  %73 = ashr exact i64 %72, 4
  %74 = add nsw i64 %73, -1
  %75 = icmp sgt i64 %73, 1
  br i1 %75, label %76, label %97

76:                                               ; preds = %63, %92
  %77 = phi i64 [ %79, %92 ], [ %74, %63 ]
  %78 = add nsw i64 %77, -1
  %79 = lshr i64 %78, 1
  %80 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %79
  %81 = load i64, ptr %80, align 8, !tbaa !16
  %82 = icmp sgt i64 %81, %67
  br i1 %82, label %83, label %86

83:                                               ; preds = %76
  %84 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %79, i32 1
  %85 = load i32, ptr %84, align 4, !tbaa !5
  br label %92

86:                                               ; preds = %76
  %87 = icmp slt i64 %81, %67
  br i1 %87, label %97, label %88

88:                                               ; preds = %86
  %89 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %79, i32 1
  %90 = load i32, ptr %89, align 8, !tbaa !5
  %91 = icmp sgt i32 %90, %69
  br i1 %91, label %92, label %97

92:                                               ; preds = %88, %83
  %93 = phi i32 [ %85, %83 ], [ %90, %88 ]
  %94 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %77
  store i64 %81, ptr %94, align 8, !tbaa !24
  %95 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %77, i32 1
  store i32 %93, ptr %95, align 8, !tbaa !26
  %96 = icmp ult i64 %78, 2
  br i1 %96, label %97, label %76, !llvm.loop !27

97:                                               ; preds = %92, %88, %86, %63
  %98 = phi i64 [ %74, %63 ], [ %77, %88 ], [ 0, %92 ], [ %77, %86 ]
  %99 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %98
  store i64 %67, ptr %99, align 8, !tbaa !24
  %100 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 %98, i32 1
  store i32 %69, ptr %100, align 8, !tbaa !26
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6) #11
  %101 = icmp eq ptr %65, %64
  br i1 %101, label %250, label %102

102:                                              ; preds = %97, %246
  %103 = phi ptr [ %247, %246 ], [ %64, %97 ]
  %104 = phi ptr [ %248, %246 ], [ %65, %97 ]
  %105 = load i64, ptr %104, align 8
  %106 = getelementptr inbounds i8, ptr %104, i64 8
  %107 = load i32, ptr %106, align 8
  %108 = ptrtoint ptr %103 to i64
  %109 = ptrtoint ptr %104 to i64
  %110 = sub i64 %108, %109
  %111 = icmp sgt i64 %110, 16
  br i1 %111, label %112, label %122

112:                                              ; preds = %102
  %113 = getelementptr inbounds %"struct.std::pair", ptr %103, i64 -1
  %114 = load i64, ptr %113, align 8
  %115 = getelementptr %"struct.std::pair", ptr %103, i64 -1, i32 1
  %116 = load i32, ptr %115, align 8
  store i64 %105, ptr %113, align 8, !tbaa !24
  store i32 %107, ptr %115, align 8, !tbaa !26
  %117 = ptrtoint ptr %113 to i64
  %118 = sub i64 %117, %109
  %119 = ashr exact i64 %118, 4
  invoke void @_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_(ptr nonnull %104, i64 noundef 0, i64 noundef %119, i64 %114, i32 %116)
          to label %120 unwind label %131

120:                                              ; preds = %112
  %121 = load ptr, ptr %61, align 8, !tbaa !28
  br label %122

122:                                              ; preds = %120, %102
  %123 = phi ptr [ %103, %102 ], [ %121, %120 ]
  %124 = getelementptr inbounds %"struct.std::pair", ptr %123, i64 -1
  store ptr %124, ptr %61, align 8, !tbaa !28
  %125 = sext i32 %107 to i64
  %126 = getelementptr inbounds i64, ptr %33, i64 %125
  %127 = load i64, ptr %126, align 8, !tbaa !16
  %128 = icmp eq i64 %105, %127
  br i1 %128, label %133, label %246, !llvm.loop !30

129:                                              ; preds = %58
  %130 = landingpad { ptr, i32 }
          cleanup
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6) #11
  br label %290

131:                                              ; preds = %112
  %132 = landingpad { ptr, i32 }
          cleanup
  br label %290

133:                                              ; preds = %122
  %134 = getelementptr inbounds i64, ptr %10, i64 %125
  %135 = load i64, ptr %134, align 8, !tbaa !31
  %136 = getelementptr i64, ptr %134, i64 1
  %137 = load i64, ptr %136, align 8, !tbaa !31
  %138 = icmp slt i64 %135, %137
  br i1 %138, label %139, label %246

139:                                              ; preds = %133
  %140 = icmp eq i64 %105, 2305843009213693951
  br label %141

141:                                              ; preds = %139, %241
  %142 = phi i64 [ %135, %139 ], [ %242, %241 ]
  %143 = getelementptr inbounds i32, ptr %14, i64 %142
  %144 = load i32, ptr %143, align 4, !tbaa !5
  %145 = getelementptr inbounds i32, ptr %18, i64 %142
  %146 = load i32, ptr %145, align 4, !tbaa !5
  %147 = icmp slt i32 %146, 0
  %148 = select i1 %147, i1 true, i1 %140
  br i1 %148, label %241, label %149

149:                                              ; preds = %141
  %150 = zext nneg i32 %146 to i64
  %151 = add nsw i64 %105, %150
  %152 = sext i32 %144 to i64
  %153 = getelementptr inbounds i64, ptr %33, i64 %152
  %154 = load i64, ptr %153, align 8, !tbaa !16
  %155 = icmp slt i64 %151, %154
  br i1 %155, label %156, label %241

156:                                              ; preds = %149
  store i64 %151, ptr %153, align 8, !tbaa !16
  %157 = load ptr, ptr %61, align 8, !tbaa !23
  %158 = load ptr, ptr %62, align 8, !tbaa !32
  %159 = icmp eq ptr %157, %158
  br i1 %159, label %164, label %160

160:                                              ; preds = %156
  store i64 %151, ptr %157, align 8, !tbaa !24
  %161 = getelementptr inbounds %"struct.std::pair", ptr %157, i64 0, i32 1
  store i32 %144, ptr %161, align 8, !tbaa !26
  %162 = getelementptr inbounds %"struct.std::pair", ptr %157, i64 1
  store ptr %162, ptr %61, align 8, !tbaa !28
  %163 = load ptr, ptr %5, align 8, !tbaa !23
  br label %199

164:                                              ; preds = %156
  %165 = load ptr, ptr %5, align 8, !tbaa !23
  %166 = ptrtoint ptr %157 to i64
  %167 = ptrtoint ptr %165 to i64
  %168 = sub i64 %166, %167
  %169 = icmp eq i64 %168, 9223372036854775792
  br i1 %169, label %170, label %172

170:                                              ; preds = %164
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #9
          to label %171 unwind label %239

171:                                              ; preds = %170
  unreachable

172:                                              ; preds = %164
  %173 = ashr exact i64 %168, 4
  %174 = call i64 @llvm.umax.i64(i64 %173, i64 1)
  %175 = add nsw i64 %174, %173
  %176 = icmp ult i64 %175, %173
  %177 = call i64 @llvm.umin.i64(i64 %175, i64 576460752303423487)
  %178 = select i1 %176, i64 576460752303423487, i64 %177
  %179 = icmp ne i64 %178, 0
  call void @llvm.assume(i1 %179)
  %180 = shl nuw nsw i64 %178, 4
  %181 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %180) #10
          to label %182 unwind label %237

182:                                              ; preds = %172
  %183 = getelementptr inbounds i8, ptr %181, i64 %168
  store i64 %151, ptr %183, align 8, !tbaa !24
  %184 = getelementptr inbounds %"struct.std::pair", ptr %181, i64 %173, i32 1
  store i32 %144, ptr %184, align 8, !tbaa !26
  %185 = icmp eq ptr %165, %157
  br i1 %185, label %192, label %186

186:                                              ; preds = %182, %186
  %187 = phi ptr [ %190, %186 ], [ %181, %182 ]
  %188 = phi ptr [ %189, %186 ], [ %165, %182 ]
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %187, ptr noundef nonnull align 8 dereferenceable(16) %188, i64 16, i1 false), !alias.scope !33
  %189 = getelementptr inbounds %"struct.std::pair", ptr %188, i64 1
  %190 = getelementptr inbounds %"struct.std::pair", ptr %187, i64 1
  %191 = icmp eq ptr %189, %157
  br i1 %191, label %192, label %186, !llvm.loop !37

192:                                              ; preds = %186, %182
  %193 = phi ptr [ %181, %182 ], [ %190, %186 ]
  %194 = getelementptr %"struct.std::pair", ptr %193, i64 1
  %195 = icmp eq ptr %165, null
  br i1 %195, label %197, label %196

196:                                              ; preds = %192
  call void @_ZdlPv(ptr noundef nonnull %165) #12
  br label %197

197:                                              ; preds = %196, %192
  store ptr %181, ptr %5, align 8, !tbaa !38
  store ptr %194, ptr %61, align 8, !tbaa !28
  %198 = getelementptr inbounds %"struct.std::pair", ptr %181, i64 %178
  store ptr %198, ptr %62, align 8, !tbaa !32
  br label %199

199:                                              ; preds = %197, %160
  %200 = phi ptr [ %163, %160 ], [ %181, %197 ]
  %201 = phi ptr [ %162, %160 ], [ %194, %197 ]
  %202 = getelementptr inbounds %"struct.std::pair", ptr %201, i64 -1
  %203 = load i64, ptr %202, align 8
  %204 = getelementptr %"struct.std::pair", ptr %201, i64 -1, i32 1
  %205 = load i32, ptr %204, align 8
  %206 = ptrtoint ptr %201 to i64
  %207 = ptrtoint ptr %200 to i64
  %208 = sub i64 %206, %207
  %209 = ashr exact i64 %208, 4
  %210 = add nsw i64 %209, -1
  %211 = icmp sgt i64 %209, 1
  br i1 %211, label %212, label %233

212:                                              ; preds = %199, %228
  %213 = phi i64 [ %215, %228 ], [ %210, %199 ]
  %214 = add nsw i64 %213, -1
  %215 = lshr i64 %214, 1
  %216 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %215
  %217 = load i64, ptr %216, align 8, !tbaa !16
  %218 = icmp sgt i64 %217, %203
  br i1 %218, label %219, label %222

219:                                              ; preds = %212
  %220 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %215, i32 1
  %221 = load i32, ptr %220, align 4, !tbaa !5
  br label %228

222:                                              ; preds = %212
  %223 = icmp slt i64 %217, %203
  br i1 %223, label %233, label %224

224:                                              ; preds = %222
  %225 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %215, i32 1
  %226 = load i32, ptr %225, align 8, !tbaa !5
  %227 = icmp sgt i32 %226, %205
  br i1 %227, label %228, label %233

228:                                              ; preds = %224, %219
  %229 = phi i32 [ %221, %219 ], [ %226, %224 ]
  %230 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %213
  store i64 %217, ptr %230, align 8, !tbaa !24
  %231 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %213, i32 1
  store i32 %229, ptr %231, align 8, !tbaa !26
  %232 = icmp ult i64 %214, 2
  br i1 %232, label %233, label %212, !llvm.loop !27

233:                                              ; preds = %222, %224, %228, %199
  %234 = phi i64 [ %210, %199 ], [ %213, %224 ], [ 0, %228 ], [ %213, %222 ]
  %235 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %234
  store i64 %203, ptr %235, align 8, !tbaa !24
  %236 = getelementptr inbounds %"struct.std::pair", ptr %200, i64 %234, i32 1
  store i32 %205, ptr %236, align 8, !tbaa !26
  br label %241

237:                                              ; preds = %172
  %238 = landingpad { ptr, i32 }
          cleanup
  br label %290

239:                                              ; preds = %170
  %240 = landingpad { ptr, i32 }
          cleanup
  br label %290

241:                                              ; preds = %149, %233, %141
  %242 = add i64 %142, 1
  %243 = icmp eq i64 %242, %137
  br i1 %243, label %244, label %141, !llvm.loop !39

244:                                              ; preds = %241
  %245 = load ptr, ptr %61, align 8, !tbaa !23
  br label %246

246:                                              ; preds = %244, %133, %122
  %247 = phi ptr [ %245, %244 ], [ %124, %133 ], [ %124, %122 ]
  %248 = load ptr, ptr %5, align 8, !tbaa !23
  %249 = icmp eq ptr %248, %247
  br i1 %249, label %250, label %102

250:                                              ; preds = %246, %97
  %251 = phi ptr [ %64, %97 ], [ %247, %246 ]
  %252 = icmp eq ptr %2, null
  br i1 %252, label %285, label %253

253:                                              ; preds = %250
  %254 = call i64 @llvm.smax.i64(i64 %21, i64 1)
  %255 = icmp slt i64 %21, 4
  br i1 %255, label %274, label %256

256:                                              ; preds = %253
  %257 = and i64 %254, 1152921504606846972
  br label %258

258:                                              ; preds = %258, %256
  %259 = phi i64 [ 0, %256 ], [ %270, %258 ]
  %260 = getelementptr inbounds i64, ptr %33, i64 %259
  %261 = getelementptr inbounds i64, ptr %260, i64 2
  %262 = load <2 x i64>, ptr %260, align 8, !tbaa !16
  %263 = load <2 x i64>, ptr %261, align 8, !tbaa !16
  %264 = icmp sgt <2 x i64> %262, <i64 1152921504606846974, i64 1152921504606846974>
  %265 = icmp sgt <2 x i64> %263, <i64 1152921504606846974, i64 1152921504606846974>
  %266 = select <2 x i1> %264, <2 x i64> <i64 9223372036854775807, i64 9223372036854775807>, <2 x i64> %262
  %267 = select <2 x i1> %265, <2 x i64> <i64 9223372036854775807, i64 9223372036854775807>, <2 x i64> %263
  %268 = getelementptr inbounds i64, ptr %2, i64 %259
  %269 = getelementptr inbounds i64, ptr %268, i64 2
  store <2 x i64> %266, ptr %268, align 8
  store <2 x i64> %267, ptr %269, align 8
  %270 = add nuw i64 %259, 4
  %271 = icmp eq i64 %270, %257
  br i1 %271, label %272, label %258, !llvm.loop !40

272:                                              ; preds = %258
  %273 = icmp eq i64 %254, %257
  br i1 %273, label %285, label %274

274:                                              ; preds = %253, %272
  %275 = phi i64 [ 0, %253 ], [ %257, %272 ]
  br label %276

276:                                              ; preds = %274, %276
  %277 = phi i64 [ %283, %276 ], [ %275, %274 ]
  %278 = getelementptr inbounds i64, ptr %33, i64 %277
  %279 = load i64, ptr %278, align 8, !tbaa !16
  %280 = icmp sgt i64 %279, 1152921504606846974
  %281 = select i1 %280, i64 9223372036854775807, i64 %279
  %282 = getelementptr inbounds i64, ptr %2, i64 %277
  store i64 %281, ptr %282, align 8
  %283 = add nuw nsw i64 %277, 1
  %284 = icmp eq i64 %283, %254
  br i1 %284, label %285, label %276, !llvm.loop !41

285:                                              ; preds = %276, %272, %250
  %286 = icmp eq ptr %251, null
  br i1 %286, label %288, label %287

287:                                              ; preds = %285
  call void @_ZdlPv(ptr noundef nonnull %251) #12
  br label %288

288:                                              ; preds = %285, %287
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %5) #11
  call void @_ZdlPv(ptr noundef nonnull %33) #12
  br label %289

289:                                              ; preds = %288, %20, %23, %8, %12, %16, %3
  ret void

290:                                              ; preds = %237, %239, %131, %129
  %291 = phi { ptr, i32 } [ %130, %129 ], [ %132, %131 ], [ %238, %237 ], [ %240, %239 ]
  %292 = load ptr, ptr %5, align 8, !tbaa !38
  %293 = icmp eq ptr %292, null
  br i1 %293, label %295, label %294

294:                                              ; preds = %290
  call void @_ZdlPv(ptr noundef nonnull %292) #12
  br label %295

295:                                              ; preds = %290, %294
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %5) #11
  call void @_ZdlPv(ptr noundef nonnull %33) #12
  resume { ptr, i32 } %291
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(ptr noundef) local_unnamed_addr #3

; Function Attrs: noreturn
declare void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #4

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #6

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZNSt6vectorISt4pairIxiESaIS1_EE17_M_realloc_insertIJxRiEEEvN9__gnu_cxx17__normal_iteratorIPS1_S3_EEDpOT_(ptr noundef nonnull align 8 dereferenceable(24) %0, ptr %1, ptr noundef nonnull align 8 dereferenceable(8) %2, ptr noundef nonnull align 4 dereferenceable(4) %3) local_unnamed_addr #0 comdat align 2 personality ptr @__gxx_personality_v0 {
  %5 = getelementptr inbounds %"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  %6 = load ptr, ptr %5, align 8, !tbaa !28
  %7 = load ptr, ptr %0, align 8, !tbaa !23
  %8 = ptrtoint ptr %6 to i64
  %9 = ptrtoint ptr %7 to i64
  %10 = sub i64 %8, %9
  %11 = icmp eq i64 %10, 9223372036854775792
  br i1 %11, label %12, label %13

12:                                               ; preds = %4
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #9
  unreachable

13:                                               ; preds = %4
  %14 = ashr exact i64 %10, 4
  %15 = tail call i64 @llvm.umax.i64(i64 %14, i64 1)
  %16 = add nsw i64 %15, %14
  %17 = icmp ult i64 %16, %14
  %18 = tail call i64 @llvm.umin.i64(i64 %16, i64 576460752303423487)
  %19 = select i1 %17, i64 576460752303423487, i64 %18
  %20 = ptrtoint ptr %1 to i64
  %21 = sub i64 %20, %9
  %22 = ashr exact i64 %21, 4
  %23 = icmp ne i64 %19, 0
  tail call void @llvm.assume(i1 %23)
  %24 = shl nuw nsw i64 %19, 4
  %25 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %24) #10
  %26 = getelementptr inbounds i8, ptr %25, i64 %21
  %27 = load i64, ptr %2, align 8, !tbaa !16
  store i64 %27, ptr %26, align 8, !tbaa !24
  %28 = getelementptr inbounds %"struct.std::pair", ptr %25, i64 %22, i32 1
  %29 = load i32, ptr %3, align 4, !tbaa !5
  store i32 %29, ptr %28, align 8, !tbaa !26
  %30 = icmp eq ptr %7, %1
  br i1 %30, label %37, label %31

31:                                               ; preds = %13, %31
  %32 = phi ptr [ %35, %31 ], [ %25, %13 ]
  %33 = phi ptr [ %34, %31 ], [ %7, %13 ]
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %32, ptr noundef nonnull align 8 dereferenceable(16) %33, i64 16, i1 false), !alias.scope !42
  %34 = getelementptr inbounds %"struct.std::pair", ptr %33, i64 1
  %35 = getelementptr inbounds %"struct.std::pair", ptr %32, i64 1
  %36 = icmp eq ptr %34, %1
  br i1 %36, label %37, label %31, !llvm.loop !37

37:                                               ; preds = %31, %13
  %38 = phi ptr [ %25, %13 ], [ %35, %31 ]
  %39 = getelementptr %"struct.std::pair", ptr %38, i64 1
  %40 = icmp eq ptr %6, %1
  br i1 %40, label %47, label %41

41:                                               ; preds = %37, %41
  %42 = phi ptr [ %45, %41 ], [ %39, %37 ]
  %43 = phi ptr [ %44, %41 ], [ %1, %37 ]
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %42, ptr noundef nonnull align 8 dereferenceable(16) %43, i64 16, i1 false), !alias.scope !46
  %44 = getelementptr inbounds %"struct.std::pair", ptr %43, i64 1
  %45 = getelementptr inbounds %"struct.std::pair", ptr %42, i64 1
  %46 = icmp eq ptr %44, %6
  br i1 %46, label %47, label %41, !llvm.loop !37

47:                                               ; preds = %41, %37
  %48 = phi ptr [ %39, %37 ], [ %45, %41 ]
  %49 = icmp eq ptr %7, null
  br i1 %49, label %51, label %50

50:                                               ; preds = %47
  tail call void @_ZdlPv(ptr noundef nonnull %7) #12
  br label %51

51:                                               ; preds = %47, %50
  %52 = getelementptr inbounds %"struct.std::_Vector_base<std::pair<long long, int>, std::allocator<std::pair<long long, int>>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  store ptr %25, ptr %0, align 8, !tbaa !38
  store ptr %48, ptr %5, align 8, !tbaa !28
  %53 = getelementptr inbounds %"struct.std::pair", ptr %25, i64 %19
  store ptr %53, ptr %52, align 8, !tbaa !32
  ret void
}

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
  %16 = load i64, ptr %15, align 8, !tbaa !16
  %17 = load i64, ptr %13, align 8, !tbaa !16
  %18 = icmp slt i64 %16, %17
  br i1 %18, label %28, label %19

19:                                               ; preds = %9
  %20 = icmp slt i64 %17, %16
  br i1 %20, label %29, label %21

21:                                               ; preds = %19
  %22 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %14, i32 1
  %23 = load i32, ptr %22, align 8, !tbaa !26
  %24 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %12, i32 1
  %25 = load i32, ptr %24, align 8, !tbaa !26
  %26 = icmp slt i32 %23, %25
  %27 = freeze i1 %26
  br i1 %27, label %28, label %29

28:                                               ; preds = %9, %21
  br label %29

29:                                               ; preds = %19, %21, %28
  %30 = phi i64 [ %16, %28 ], [ %17, %21 ], [ %17, %19 ]
  %31 = phi i64 [ %14, %28 ], [ %12, %21 ], [ %12, %19 ]
  %32 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %10
  store i64 %30, ptr %32, align 8, !tbaa !24
  %33 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %31, i32 1
  %34 = load i32, ptr %33, align 4, !tbaa !5
  %35 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %10, i32 1
  store i32 %34, ptr %35, align 8, !tbaa !26
  %36 = icmp slt i64 %31, %7
  br i1 %36, label %9, label %37, !llvm.loop !50

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
  %50 = load i64, ptr %48, align 8, !tbaa !16
  store i64 %50, ptr %49, align 8, !tbaa !24
  %51 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %47, i32 1
  %52 = load i32, ptr %51, align 4, !tbaa !5
  %53 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %38, i32 1
  store i32 %52, ptr %53, align 8, !tbaa !26
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
  %62 = load i64, ptr %61, align 8, !tbaa !16
  %63 = icmp sgt i64 %62, %3
  br i1 %63, label %64, label %67

64:                                               ; preds = %57
  %65 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60, i32 1
  %66 = load i32, ptr %65, align 4, !tbaa !5
  br label %73

67:                                               ; preds = %57
  %68 = icmp slt i64 %62, %3
  br i1 %68, label %78, label %69

69:                                               ; preds = %67
  %70 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60, i32 1
  %71 = load i32, ptr %70, align 8, !tbaa !5
  %72 = icmp sgt i32 %71, %4
  br i1 %72, label %73, label %78

73:                                               ; preds = %69, %64
  %74 = phi i32 [ %66, %64 ], [ %71, %69 ]
  %75 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %58
  store i64 %62, ptr %75, align 8, !tbaa !24
  %76 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %58, i32 1
  store i32 %74, ptr %76, align 8, !tbaa !26
  %77 = icmp sgt i64 %60, %1
  br i1 %77, label %57, label %78, !llvm.loop !27

78:                                               ; preds = %67, %69, %73, %54
  %79 = phi i64 [ %55, %54 ], [ %58, %69 ], [ %60, %73 ], [ %58, %67 ]
  %80 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %79
  store i64 %3, ptr %80, align 8, !tbaa !24
  %81 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %79, i32 1
  store i32 %4, ptr %81, align 8, !tbaa !26
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #7

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #7

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #7

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #7 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #9 = { noreturn }
attributes #10 = { builtin allocsize(0) }
attributes #11 = { nounwind }
attributes #12 = { builtin nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !12, i64 16}
!10 = !{!"_ZTS5Graph", !11, i64 0, !11, i64 8, !12, i64 16, !12, i64 24, !12, i64 32}
!11 = !{!"long", !7, i64 0}
!12 = !{!"any pointer", !7, i64 0}
!13 = !{!10, !12, i64 24}
!14 = !{!10, !12, i64 32}
!15 = !{!10, !11, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"long long", !7, i64 0}
!18 = distinct !{!18, !19, !20, !21}
!19 = !{!"llvm.loop.mustprogress"}
!20 = !{!"llvm.loop.isvectorized", i32 1}
!21 = !{!"llvm.loop.unroll.runtime.disable"}
!22 = distinct !{!22, !19, !21, !20}
!23 = !{!12, !12, i64 0}
!24 = !{!25, !17, i64 0}
!25 = !{!"_ZTSSt4pairIxiE", !17, i64 0, !6, i64 8}
!26 = !{!25, !6, i64 8}
!27 = distinct !{!27, !19}
!28 = !{!29, !12, i64 8}
!29 = !{!"_ZTSNSt12_Vector_baseISt4pairIxiESaIS1_EE17_Vector_impl_dataE", !12, i64 0, !12, i64 8, !12, i64 16}
!30 = distinct !{!30, !19}
!31 = !{!11, !11, i64 0}
!32 = !{!29, !12, i64 16}
!33 = !{!34, !36}
!34 = distinct !{!34, !35, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 0"}
!35 = distinct !{!35, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_"}
!36 = distinct !{!36, !35, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 1"}
!37 = distinct !{!37, !19}
!38 = !{!29, !12, i64 0}
!39 = distinct !{!39, !19}
!40 = distinct !{!40, !19, !20, !21}
!41 = distinct !{!41, !19, !21, !20}
!42 = !{!43, !45}
!43 = distinct !{!43, !44, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 0"}
!44 = distinct !{!44, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_"}
!45 = distinct !{!45, !44, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 1"}
!46 = !{!47, !49}
!47 = distinct !{!47, !48, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 0"}
!48 = distinct !{!48, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_"}
!49 = distinct !{!49, !48, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 1"}
!50 = distinct !{!50, !19}
