; ModuleID = 'bfs_runtime_src.cpp'
source_filename = "bfs_runtime_src.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.AutoNeighborIter = type { ptr, i64, i64, i64, i32, i32, i32, i32 }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [43 x i8] c"[bfs_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.3 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local range(i32 0, 5) i32 @bfs_runtime_src(ptr noundef %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = alloca %struct.AutoNeighborIter, align 8
  %6 = alloca i32, align 4
  %7 = alloca %struct.AutoNeighborIter, align 8
  %8 = alloca i32, align 4
  %9 = icmp ne ptr %0, null
  %10 = icmp ne ptr %2, null
  %11 = and i1 %9, %10
  %12 = icmp ne ptr %3, null
  %13 = and i1 %11, %12
  br i1 %13, label %14, label %620

14:                                               ; preds = %4
  %15 = load i64, ptr %0, align 8, !tbaa !3
  %16 = icmp slt i64 %15, 0
  br i1 %16, label %620, label %17

17:                                               ; preds = %14
  %18 = icmp slt i32 %1, 0
  %19 = trunc i64 %15 to i32
  %20 = icmp slt i32 %1, %19
  %21 = or i1 %18, %20
  br i1 %21, label %25, label %22

22:                                               ; preds = %17
  %23 = load ptr, ptr @stderr, align 8, !tbaa !11
  %24 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %23, ptr noundef nonnull @.str, i32 noundef %1, i64 noundef %15) #12
  br label %620

25:                                               ; preds = %17
  %26 = icmp eq i64 %15, 0
  br i1 %26, label %36, label %27

27:                                               ; preds = %25
  %28 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %15) #13
  %29 = getelementptr inbounds nuw i8, ptr %28, i64 %15
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %28, i8 0, i64 %15, i1 false)
  %30 = ptrtoint ptr %29 to i64
  %31 = tail call i64 @llvm.umin.i64(i64 %15, i64 1024)
  %32 = shl nuw nsw i64 %31, 2
  %33 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %32) #13
          to label %34 unwind label %104

34:                                               ; preds = %27
  %35 = getelementptr inbounds nuw i32, ptr %33, i64 %31
  br label %36

36:                                               ; preds = %25, %34
  %37 = phi ptr [ %28, %34 ], [ null, %25 ]
  %38 = phi i64 [ %30, %34 ], [ 0, %25 ]
  %39 = phi ptr [ %33, %34 ], [ null, %25 ]
  %40 = phi ptr [ %35, %34 ], [ null, %25 ]
  %41 = invoke double @omp_get_wtime()
          to label %42 unwind label %106

42:                                               ; preds = %36
  %43 = icmp sgt i32 %1, -1
  br i1 %43, label %48, label %44

44:                                               ; preds = %42
  %45 = icmp sgt i32 %19, 0
  br i1 %45, label %46, label %510

46:                                               ; preds = %44
  %47 = and i64 %15, 2147483647
  br label %262

48:                                               ; preds = %42
  %49 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #13
          to label %50 unwind label %108

50:                                               ; preds = %48
  store i32 %1, ptr %49, align 4, !tbaa !13
  %51 = getelementptr inbounds nuw i8, ptr %49, i64 4
  %52 = zext nneg i32 %1 to i64
  %53 = getelementptr inbounds nuw i8, ptr %37, i64 %52
  store i8 1, ptr %53, align 1, !tbaa !15
  %54 = icmp eq ptr %39, %40
  br i1 %54, label %56, label %55

55:                                               ; preds = %50
  store i32 %1, ptr %39, align 4, !tbaa !13
  br label %63

56:                                               ; preds = %50
  %57 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #13
          to label %58 unwind label %108

58:                                               ; preds = %56
  store i32 %1, ptr %57, align 4, !tbaa !13
  %59 = icmp eq ptr %39, null
  br i1 %59, label %61, label %60

60:                                               ; preds = %58
  tail call void @_ZdlPvm(ptr noundef nonnull %39, i64 noundef 0) #14
  br label %61

61:                                               ; preds = %60, %58
  %62 = getelementptr inbounds nuw i8, ptr %57, i64 4
  br label %63

63:                                               ; preds = %55, %61
  %64 = phi ptr [ %57, %61 ], [ %39, %55 ]
  %65 = phi ptr [ %62, %61 ], [ %40, %55 ]
  %66 = getelementptr inbounds nuw i8, ptr %64, i64 4
  br label %67

67:                                               ; preds = %63, %102
  %68 = phi ptr [ %51, %63 ], [ %97, %102 ]
  %69 = phi ptr [ %51, %63 ], [ %96, %102 ]
  %70 = phi ptr [ %49, %63 ], [ %95, %102 ]
  %71 = phi ptr [ %65, %63 ], [ %94, %102 ]
  %72 = phi ptr [ %66, %63 ], [ %93, %102 ]
  %73 = phi ptr [ %64, %63 ], [ %92, %102 ]
  %74 = ptrtoint ptr %69 to i64
  %75 = ptrtoint ptr %70 to i64
  %76 = sub i64 %74, %75
  %77 = ashr exact i64 %76, 2
  %78 = icmp sgt i64 %77, -1
  call void @llvm.assume(i1 %78)
  %79 = ashr exact i64 %76, 1
  %80 = icmp samesign ugt i64 %79, 2305843009213693951
  br i1 %80, label %81, label %83

81:                                               ; preds = %67
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #15
          to label %82 unwind label %114

82:                                               ; preds = %81
  unreachable

83:                                               ; preds = %67
  %84 = icmp eq ptr %69, %70
  br i1 %84, label %91, label %85

85:                                               ; preds = %83
  %86 = shl nuw nsw i64 %76, 1
  %87 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %86) #13
          to label %88 unwind label %112

88:                                               ; preds = %85
  %89 = getelementptr inbounds nuw i32, ptr %87, i64 %79
  %90 = call i64 @llvm.umax.i64(i64 %77, i64 1)
  br label %116

91:                                               ; preds = %231, %83
  %92 = phi ptr [ %73, %83 ], [ %129, %231 ]
  %93 = phi ptr [ %72, %83 ], [ %136, %231 ]
  %94 = phi ptr [ %71, %83 ], [ %131, %231 ]
  %95 = phi ptr [ null, %83 ], [ %137, %231 ]
  %96 = phi ptr [ null, %83 ], [ %138, %231 ]
  %97 = phi ptr [ null, %83 ], [ %139, %231 ]
  %98 = icmp eq ptr %70, null
  br i1 %98, label %102, label %99

99:                                               ; preds = %91
  %100 = ptrtoint ptr %68 to i64
  %101 = sub i64 %100, %75
  call void @_ZdlPvm(ptr noundef nonnull %70, i64 noundef %101) #14
  br label %102

102:                                              ; preds = %91, %99
  %103 = icmp eq ptr %95, %96
  br i1 %103, label %245, label %67, !llvm.loop !16

104:                                              ; preds = %27
  %105 = landingpad { ptr, i32 }
          cleanup
  br label %612

106:                                              ; preds = %36
  %107 = landingpad { ptr, i32 }
          cleanup
  br label %601

108:                                              ; preds = %56, %48
  %109 = phi ptr [ %49, %56 ], [ null, %48 ]
  %110 = phi ptr [ %51, %56 ], [ null, %48 ]
  %111 = landingpad { ptr, i32 }
          cleanup
  br label %251

112:                                              ; preds = %85
  %113 = landingpad { ptr, i32 }
          cleanup
  br label %251

114:                                              ; preds = %81
  %115 = landingpad { ptr, i32 }
          cleanup
  br label %251

116:                                              ; preds = %88, %231
  %117 = phi i64 [ %232, %231 ], [ 0, %88 ]
  %118 = phi ptr [ %139, %231 ], [ %89, %88 ]
  %119 = phi ptr [ %138, %231 ], [ %87, %88 ]
  %120 = phi ptr [ %137, %231 ], [ %87, %88 ]
  %121 = phi ptr [ %131, %231 ], [ %71, %88 ]
  %122 = phi ptr [ %136, %231 ], [ %72, %88 ]
  %123 = phi ptr [ %129, %231 ], [ %73, %88 ]
  %124 = getelementptr inbounds nuw i32, ptr %70, i64 %117
  %125 = load i32, ptr %124, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %5) #16
  %126 = sext i32 %125 to i64
  invoke void @autograph_neighbor_iter_init(ptr noundef nonnull %0, i64 noundef %126, ptr noundef nonnull %5)
          to label %127 unwind label %213

127:                                              ; preds = %116
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %6) #16
  br label %128

128:                                              ; preds = %210, %127
  %129 = phi ptr [ %201, %210 ], [ %123, %127 ]
  %130 = phi ptr [ %211, %210 ], [ %122, %127 ]
  %131 = phi ptr [ %212, %210 ], [ %121, %127 ]
  %132 = phi ptr [ %179, %210 ], [ %120, %127 ]
  %133 = phi ptr [ %182, %210 ], [ %119, %127 ]
  %134 = phi ptr [ %181, %210 ], [ %118, %127 ]
  br label %135

135:                                              ; preds = %128, %184
  %136 = phi ptr [ %130, %128 ], [ %186, %184 ]
  %137 = phi ptr [ %132, %128 ], [ %179, %184 ]
  %138 = phi ptr [ %133, %128 ], [ %182, %184 ]
  %139 = phi ptr [ %134, %128 ], [ %181, %184 ]
  br label %140

140:                                              ; preds = %135, %144
  %141 = invoke i32 @autograph_neighbor_iter_next(ptr noundef nonnull %5, ptr noundef nonnull %6)
          to label %142 unwind label %215

142:                                              ; preds = %140
  %143 = icmp eq i32 %141, 0
  br i1 %143, label %231, label %144

144:                                              ; preds = %142
  %145 = load i32, ptr %6, align 4, !tbaa !13
  %146 = sext i32 %145 to i64
  %147 = getelementptr inbounds nuw i8, ptr %37, i64 %146
  %148 = load i8, ptr %147, align 1, !tbaa !15
  %149 = icmp eq i8 %148, 0
  br i1 %149, label %150, label %140, !llvm.loop !18

150:                                              ; preds = %144
  %151 = getelementptr inbounds nuw i8, ptr %37, i64 %146
  store i8 1, ptr %151, align 1, !tbaa !15
  %152 = icmp eq ptr %138, %139
  br i1 %152, label %154, label %153

153:                                              ; preds = %150
  store i32 %145, ptr %138, align 4, !tbaa !13
  br label %178

154:                                              ; preds = %150
  %155 = ptrtoint ptr %138 to i64
  %156 = ptrtoint ptr %137 to i64
  %157 = sub i64 %155, %156
  %158 = ashr exact i64 %157, 2
  %159 = icmp sgt i64 %158, -1
  call void @llvm.assume(i1 %159)
  %160 = icmp eq i64 %157, 9223372036854775804
  br i1 %160, label %161, label %163

161:                                              ; preds = %154
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #15
          to label %162 unwind label %221

162:                                              ; preds = %161
  unreachable

163:                                              ; preds = %154
  %164 = call i64 @llvm.umax.i64(i64 %158, i64 1)
  %165 = add nuw nsw i64 %164, %158
  %166 = call noundef i64 @llvm.umin.i64(i64 %165, i64 2305843009213693951)
  %167 = shl nuw nsw i64 %166, 2
  %168 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %167) #13
          to label %169 unwind label %217

169:                                              ; preds = %163
  %170 = getelementptr inbounds i8, ptr %168, i64 %157
  store i32 %145, ptr %170, align 4, !tbaa !13
  %171 = icmp sgt i64 %157, 0
  br i1 %171, label %172, label %173

172:                                              ; preds = %169
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %168, ptr align 4 %137, i64 %157, i1 false)
  br label %173

173:                                              ; preds = %172, %169
  %174 = icmp eq ptr %137, null
  br i1 %174, label %176, label %175

175:                                              ; preds = %173
  call void @_ZdlPvm(ptr noundef nonnull %137, i64 noundef %157) #14
  br label %176

176:                                              ; preds = %175, %173
  %177 = getelementptr inbounds nuw i32, ptr %168, i64 %166
  br label %178

178:                                              ; preds = %176, %153
  %179 = phi ptr [ %168, %176 ], [ %137, %153 ]
  %180 = phi ptr [ %170, %176 ], [ %138, %153 ]
  %181 = phi ptr [ %177, %176 ], [ %139, %153 ]
  %182 = getelementptr inbounds nuw i8, ptr %180, i64 4
  %183 = icmp eq ptr %136, %131
  br i1 %183, label %187, label %184

184:                                              ; preds = %178
  %185 = load i32, ptr %6, align 4, !tbaa !13
  store i32 %185, ptr %136, align 4, !tbaa !13
  %186 = getelementptr inbounds nuw i8, ptr %136, i64 4
  br label %135, !llvm.loop !18

187:                                              ; preds = %178
  %188 = ptrtoint ptr %136 to i64
  %189 = ptrtoint ptr %129 to i64
  %190 = sub i64 %188, %189
  %191 = ashr exact i64 %190, 2
  %192 = icmp sgt i64 %191, -1
  call void @llvm.assume(i1 %192)
  %193 = icmp eq i64 %190, 9223372036854775804
  br i1 %193, label %194, label %196

194:                                              ; preds = %187
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #15
          to label %195 unwind label %221

195:                                              ; preds = %194
  unreachable

196:                                              ; preds = %187
  %197 = call i64 @llvm.umax.i64(i64 %191, i64 1)
  %198 = add nuw nsw i64 %197, %191
  %199 = call noundef i64 @llvm.umin.i64(i64 %198, i64 2305843009213693951)
  %200 = shl nuw nsw i64 %199, 2
  %201 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %200) #13
          to label %202 unwind label %219

202:                                              ; preds = %196
  %203 = getelementptr inbounds i8, ptr %201, i64 %190
  %204 = load i32, ptr %6, align 4, !tbaa !13
  store i32 %204, ptr %203, align 4, !tbaa !13
  %205 = icmp sgt i64 %190, 0
  br i1 %205, label %206, label %207

206:                                              ; preds = %202
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %201, ptr align 4 %129, i64 %190, i1 false)
  br label %207

207:                                              ; preds = %206, %202
  %208 = icmp eq ptr %129, null
  br i1 %208, label %210, label %209

209:                                              ; preds = %207
  call void @_ZdlPvm(ptr noundef nonnull %129, i64 noundef %190) #14
  br label %210

210:                                              ; preds = %209, %207
  %211 = getelementptr inbounds nuw i8, ptr %203, i64 4
  %212 = getelementptr inbounds nuw i32, ptr %201, i64 %199
  br label %128, !llvm.loop !18

213:                                              ; preds = %116
  %214 = landingpad { ptr, i32 }
          cleanup
  br label %234

215:                                              ; preds = %140
  %216 = landingpad { ptr, i32 }
          cleanup
  br label %226

217:                                              ; preds = %163
  %218 = landingpad { ptr, i32 }
          cleanup
  br label %226

219:                                              ; preds = %196
  %220 = landingpad { ptr, i32 }
          cleanup
  br label %226

221:                                              ; preds = %161, %194
  %222 = phi ptr [ %136, %194 ], [ %131, %161 ]
  %223 = phi ptr [ %179, %194 ], [ %137, %161 ]
  %224 = phi ptr [ %181, %194 ], [ %138, %161 ]
  %225 = landingpad { ptr, i32 }
          cleanup
  br label %226

226:                                              ; preds = %219, %217, %215, %221
  %227 = phi ptr [ %222, %221 ], [ %136, %219 ], [ %131, %217 ], [ %131, %215 ]
  %228 = phi ptr [ %223, %221 ], [ %179, %219 ], [ %137, %217 ], [ %137, %215 ]
  %229 = phi ptr [ %224, %221 ], [ %181, %219 ], [ %139, %215 ], [ %138, %217 ]
  %230 = phi { ptr, i32 } [ %225, %221 ], [ %220, %219 ], [ %216, %215 ], [ %218, %217 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %6) #16
  br label %234

231:                                              ; preds = %142
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %6) #16
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %5) #16
  %232 = add nuw nsw i64 %117, 1
  %233 = icmp eq i64 %232, %90
  br i1 %233, label %91, label %116, !llvm.loop !19

234:                                              ; preds = %213, %226
  %235 = phi ptr [ %129, %226 ], [ %123, %213 ]
  %236 = phi ptr [ %227, %226 ], [ %121, %213 ]
  %237 = phi ptr [ %228, %226 ], [ %120, %213 ]
  %238 = phi ptr [ %229, %226 ], [ %118, %213 ]
  %239 = phi { ptr, i32 } [ %230, %226 ], [ %214, %213 ]
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %5) #16
  %240 = icmp eq ptr %237, null
  br i1 %240, label %251, label %241

241:                                              ; preds = %234
  %242 = ptrtoint ptr %238 to i64
  %243 = ptrtoint ptr %237 to i64
  %244 = sub i64 %242, %243
  call void @_ZdlPvm(ptr noundef nonnull %237, i64 noundef %244) #14
  br label %251

245:                                              ; preds = %102
  %246 = icmp eq ptr %95, null
  br i1 %246, label %510, label %247

247:                                              ; preds = %245
  %248 = ptrtoint ptr %97 to i64
  %249 = ptrtoint ptr %95 to i64
  %250 = sub i64 %248, %249
  call void @_ZdlPvm(ptr noundef nonnull %95, i64 noundef %250) #14
  br label %510

251:                                              ; preds = %112, %114, %241, %234, %108
  %252 = phi ptr [ %39, %108 ], [ %235, %234 ], [ %235, %241 ], [ %73, %112 ], [ %73, %114 ]
  %253 = phi ptr [ %40, %108 ], [ %236, %234 ], [ %236, %241 ], [ %71, %112 ], [ %71, %114 ]
  %254 = phi ptr [ %109, %108 ], [ %70, %234 ], [ %70, %241 ], [ %70, %112 ], [ %70, %114 ]
  %255 = phi ptr [ %110, %108 ], [ %68, %234 ], [ %68, %241 ], [ %68, %112 ], [ %68, %114 ]
  %256 = phi { ptr, i32 } [ %111, %108 ], [ %239, %234 ], [ %239, %241 ], [ %113, %112 ], [ %115, %114 ]
  %257 = icmp eq ptr %254, null
  br i1 %257, label %601, label %258

258:                                              ; preds = %251
  %259 = ptrtoint ptr %255 to i64
  %260 = ptrtoint ptr %254 to i64
  %261 = sub i64 %259, %260
  call void @_ZdlPvm(ptr noundef nonnull %254, i64 noundef %261) #14
  br label %601

262:                                              ; preds = %46, %488
  %263 = phi i64 [ 0, %46 ], [ %492, %488 ]
  %264 = phi ptr [ %40, %46 ], [ %491, %488 ]
  %265 = phi ptr [ %39, %46 ], [ %490, %488 ]
  %266 = phi ptr [ %39, %46 ], [ %489, %488 ]
  %267 = getelementptr inbounds nuw i8, ptr %37, i64 %263
  %268 = load i8, ptr %267, align 1, !tbaa !15
  %269 = icmp eq i8 %268, 0
  br i1 %269, label %270, label %488

270:                                              ; preds = %262
  %271 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #13
          to label %272 unwind label %343

272:                                              ; preds = %270
  %273 = trunc nuw nsw i64 %263 to i32
  store i32 %273, ptr %271, align 4, !tbaa !13
  %274 = getelementptr inbounds nuw i8, ptr %271, i64 4
  store i8 1, ptr %267, align 1, !tbaa !15
  %275 = icmp eq ptr %265, %264
  br i1 %275, label %277, label %276

276:                                              ; preds = %272
  store i32 %273, ptr %265, align 4, !tbaa !13
  br label %301

277:                                              ; preds = %272
  %278 = ptrtoint ptr %264 to i64
  %279 = ptrtoint ptr %266 to i64
  %280 = sub i64 %278, %279
  %281 = ashr exact i64 %280, 2
  %282 = icmp sgt i64 %281, -1
  call void @llvm.assume(i1 %282)
  %283 = icmp eq i64 %280, 9223372036854775804
  br i1 %283, label %284, label %286

284:                                              ; preds = %277
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #15
          to label %285 unwind label %347

285:                                              ; preds = %284
  unreachable

286:                                              ; preds = %277
  %287 = call i64 @llvm.umax.i64(i64 %281, i64 1)
  %288 = add nuw nsw i64 %287, %281
  %289 = call noundef i64 @llvm.umin.i64(i64 %288, i64 2305843009213693951)
  %290 = shl nuw nsw i64 %289, 2
  %291 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %290) #13
          to label %292 unwind label %343

292:                                              ; preds = %286
  %293 = getelementptr inbounds i8, ptr %291, i64 %280
  store i32 %273, ptr %293, align 4, !tbaa !13
  %294 = icmp sgt i64 %280, 0
  br i1 %294, label %295, label %296

295:                                              ; preds = %292
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %291, ptr align 4 %266, i64 %280, i1 false)
  br label %296

296:                                              ; preds = %295, %292
  %297 = icmp eq ptr %266, null
  br i1 %297, label %299, label %298

298:                                              ; preds = %296
  call void @_ZdlPvm(ptr noundef nonnull %266, i64 noundef %280) #14
  br label %299

299:                                              ; preds = %298, %296
  %300 = getelementptr inbounds nuw i32, ptr %291, i64 %289
  br label %301

301:                                              ; preds = %276, %299
  %302 = phi ptr [ %291, %299 ], [ %266, %276 ]
  %303 = phi ptr [ %293, %299 ], [ %265, %276 ]
  %304 = phi ptr [ %300, %299 ], [ %264, %276 ]
  %305 = getelementptr inbounds nuw i8, ptr %303, i64 4
  br label %306

306:                                              ; preds = %301, %341
  %307 = phi ptr [ %274, %301 ], [ %336, %341 ]
  %308 = phi ptr [ %274, %301 ], [ %335, %341 ]
  %309 = phi ptr [ %271, %301 ], [ %334, %341 ]
  %310 = phi ptr [ %304, %301 ], [ %333, %341 ]
  %311 = phi ptr [ %305, %301 ], [ %332, %341 ]
  %312 = phi ptr [ %302, %301 ], [ %331, %341 ]
  %313 = ptrtoint ptr %308 to i64
  %314 = ptrtoint ptr %309 to i64
  %315 = sub i64 %313, %314
  %316 = ashr exact i64 %315, 2
  %317 = icmp sgt i64 %316, -1
  call void @llvm.assume(i1 %317)
  %318 = ashr exact i64 %315, 1
  %319 = icmp samesign ugt i64 %318, 2305843009213693951
  br i1 %319, label %320, label %322

320:                                              ; preds = %306
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #15
          to label %321 unwind label %351

321:                                              ; preds = %320
  unreachable

322:                                              ; preds = %306
  %323 = icmp eq ptr %308, %309
  br i1 %323, label %330, label %324

324:                                              ; preds = %322
  %325 = shl nuw nsw i64 %315, 1
  %326 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %325) #13
          to label %327 unwind label %349

327:                                              ; preds = %324
  %328 = getelementptr inbounds nuw i32, ptr %326, i64 %318
  %329 = call i64 @llvm.umax.i64(i64 %316, i64 1)
  br label %353

330:                                              ; preds = %468, %322
  %331 = phi ptr [ %312, %322 ], [ %366, %468 ]
  %332 = phi ptr [ %311, %322 ], [ %373, %468 ]
  %333 = phi ptr [ %310, %322 ], [ %368, %468 ]
  %334 = phi ptr [ null, %322 ], [ %374, %468 ]
  %335 = phi ptr [ null, %322 ], [ %375, %468 ]
  %336 = phi ptr [ null, %322 ], [ %376, %468 ]
  %337 = icmp eq ptr %309, null
  br i1 %337, label %341, label %338

338:                                              ; preds = %330
  %339 = ptrtoint ptr %307 to i64
  %340 = sub i64 %339, %314
  call void @_ZdlPvm(ptr noundef nonnull %309, i64 noundef %340) #14
  br label %341

341:                                              ; preds = %330, %338
  %342 = icmp eq ptr %334, %335
  br i1 %342, label %482, label %306, !llvm.loop !20

343:                                              ; preds = %270, %286
  %344 = phi ptr [ null, %270 ], [ %271, %286 ]
  %345 = phi ptr [ null, %270 ], [ %274, %286 ]
  %346 = landingpad { ptr, i32 }
          cleanup
  br label %494

347:                                              ; preds = %284
  %348 = landingpad { ptr, i32 }
          cleanup
  br label %501

349:                                              ; preds = %324
  %350 = landingpad { ptr, i32 }
          cleanup
  br label %494

351:                                              ; preds = %320
  %352 = landingpad { ptr, i32 }
          cleanup
  br label %494

353:                                              ; preds = %327, %468
  %354 = phi i64 [ %469, %468 ], [ 0, %327 ]
  %355 = phi ptr [ %376, %468 ], [ %328, %327 ]
  %356 = phi ptr [ %375, %468 ], [ %326, %327 ]
  %357 = phi ptr [ %374, %468 ], [ %326, %327 ]
  %358 = phi ptr [ %368, %468 ], [ %310, %327 ]
  %359 = phi ptr [ %373, %468 ], [ %311, %327 ]
  %360 = phi ptr [ %366, %468 ], [ %312, %327 ]
  %361 = getelementptr inbounds nuw i32, ptr %309, i64 %354
  %362 = load i32, ptr %361, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %7) #16
  %363 = sext i32 %362 to i64
  invoke void @autograph_neighbor_iter_init(ptr noundef nonnull %0, i64 noundef %363, ptr noundef nonnull %7)
          to label %364 unwind label %450

364:                                              ; preds = %353
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %8) #16
  br label %365

365:                                              ; preds = %447, %364
  %366 = phi ptr [ %438, %447 ], [ %360, %364 ]
  %367 = phi ptr [ %448, %447 ], [ %359, %364 ]
  %368 = phi ptr [ %449, %447 ], [ %358, %364 ]
  %369 = phi ptr [ %416, %447 ], [ %357, %364 ]
  %370 = phi ptr [ %419, %447 ], [ %356, %364 ]
  %371 = phi ptr [ %418, %447 ], [ %355, %364 ]
  br label %372

372:                                              ; preds = %365, %421
  %373 = phi ptr [ %367, %365 ], [ %423, %421 ]
  %374 = phi ptr [ %369, %365 ], [ %416, %421 ]
  %375 = phi ptr [ %370, %365 ], [ %419, %421 ]
  %376 = phi ptr [ %371, %365 ], [ %418, %421 ]
  br label %377

377:                                              ; preds = %372, %381
  %378 = invoke i32 @autograph_neighbor_iter_next(ptr noundef nonnull %7, ptr noundef nonnull %8)
          to label %379 unwind label %452

379:                                              ; preds = %377
  %380 = icmp eq i32 %378, 0
  br i1 %380, label %468, label %381

381:                                              ; preds = %379
  %382 = load i32, ptr %8, align 4, !tbaa !13
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds nuw i8, ptr %37, i64 %383
  %385 = load i8, ptr %384, align 1, !tbaa !15
  %386 = icmp eq i8 %385, 0
  br i1 %386, label %387, label %377, !llvm.loop !21

387:                                              ; preds = %381
  %388 = getelementptr inbounds nuw i8, ptr %37, i64 %383
  store i8 1, ptr %388, align 1, !tbaa !15
  %389 = icmp eq ptr %375, %376
  br i1 %389, label %391, label %390

390:                                              ; preds = %387
  store i32 %382, ptr %375, align 4, !tbaa !13
  br label %415

391:                                              ; preds = %387
  %392 = ptrtoint ptr %375 to i64
  %393 = ptrtoint ptr %374 to i64
  %394 = sub i64 %392, %393
  %395 = ashr exact i64 %394, 2
  %396 = icmp sgt i64 %395, -1
  call void @llvm.assume(i1 %396)
  %397 = icmp eq i64 %394, 9223372036854775804
  br i1 %397, label %398, label %400

398:                                              ; preds = %391
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #15
          to label %399 unwind label %458

399:                                              ; preds = %398
  unreachable

400:                                              ; preds = %391
  %401 = call i64 @llvm.umax.i64(i64 %395, i64 1)
  %402 = add nuw nsw i64 %401, %395
  %403 = call noundef i64 @llvm.umin.i64(i64 %402, i64 2305843009213693951)
  %404 = shl nuw nsw i64 %403, 2
  %405 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %404) #13
          to label %406 unwind label %454

406:                                              ; preds = %400
  %407 = getelementptr inbounds i8, ptr %405, i64 %394
  store i32 %382, ptr %407, align 4, !tbaa !13
  %408 = icmp sgt i64 %394, 0
  br i1 %408, label %409, label %410

409:                                              ; preds = %406
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %405, ptr align 4 %374, i64 %394, i1 false)
  br label %410

410:                                              ; preds = %409, %406
  %411 = icmp eq ptr %374, null
  br i1 %411, label %413, label %412

412:                                              ; preds = %410
  call void @_ZdlPvm(ptr noundef nonnull %374, i64 noundef %394) #14
  br label %413

413:                                              ; preds = %412, %410
  %414 = getelementptr inbounds nuw i32, ptr %405, i64 %403
  br label %415

415:                                              ; preds = %413, %390
  %416 = phi ptr [ %405, %413 ], [ %374, %390 ]
  %417 = phi ptr [ %407, %413 ], [ %375, %390 ]
  %418 = phi ptr [ %414, %413 ], [ %376, %390 ]
  %419 = getelementptr inbounds nuw i8, ptr %417, i64 4
  %420 = icmp eq ptr %373, %368
  br i1 %420, label %424, label %421

421:                                              ; preds = %415
  %422 = load i32, ptr %8, align 4, !tbaa !13
  store i32 %422, ptr %373, align 4, !tbaa !13
  %423 = getelementptr inbounds nuw i8, ptr %373, i64 4
  br label %372, !llvm.loop !21

424:                                              ; preds = %415
  %425 = ptrtoint ptr %373 to i64
  %426 = ptrtoint ptr %366 to i64
  %427 = sub i64 %425, %426
  %428 = ashr exact i64 %427, 2
  %429 = icmp sgt i64 %428, -1
  call void @llvm.assume(i1 %429)
  %430 = icmp eq i64 %427, 9223372036854775804
  br i1 %430, label %431, label %433

431:                                              ; preds = %424
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #15
          to label %432 unwind label %458

432:                                              ; preds = %431
  unreachable

433:                                              ; preds = %424
  %434 = call i64 @llvm.umax.i64(i64 %428, i64 1)
  %435 = add nuw nsw i64 %434, %428
  %436 = call noundef i64 @llvm.umin.i64(i64 %435, i64 2305843009213693951)
  %437 = shl nuw nsw i64 %436, 2
  %438 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %437) #13
          to label %439 unwind label %456

439:                                              ; preds = %433
  %440 = getelementptr inbounds i8, ptr %438, i64 %427
  %441 = load i32, ptr %8, align 4, !tbaa !13
  store i32 %441, ptr %440, align 4, !tbaa !13
  %442 = icmp sgt i64 %427, 0
  br i1 %442, label %443, label %444

443:                                              ; preds = %439
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %438, ptr align 4 %366, i64 %427, i1 false)
  br label %444

444:                                              ; preds = %443, %439
  %445 = icmp eq ptr %366, null
  br i1 %445, label %447, label %446

446:                                              ; preds = %444
  call void @_ZdlPvm(ptr noundef nonnull %366, i64 noundef %427) #14
  br label %447

447:                                              ; preds = %446, %444
  %448 = getelementptr inbounds nuw i8, ptr %440, i64 4
  %449 = getelementptr inbounds nuw i32, ptr %438, i64 %436
  br label %365, !llvm.loop !21

450:                                              ; preds = %353
  %451 = landingpad { ptr, i32 }
          cleanup
  br label %471

452:                                              ; preds = %377
  %453 = landingpad { ptr, i32 }
          cleanup
  br label %463

454:                                              ; preds = %400
  %455 = landingpad { ptr, i32 }
          cleanup
  br label %463

456:                                              ; preds = %433
  %457 = landingpad { ptr, i32 }
          cleanup
  br label %463

458:                                              ; preds = %398, %431
  %459 = phi ptr [ %373, %431 ], [ %368, %398 ]
  %460 = phi ptr [ %416, %431 ], [ %374, %398 ]
  %461 = phi ptr [ %418, %431 ], [ %375, %398 ]
  %462 = landingpad { ptr, i32 }
          cleanup
  br label %463

463:                                              ; preds = %456, %454, %452, %458
  %464 = phi ptr [ %459, %458 ], [ %373, %456 ], [ %368, %454 ], [ %368, %452 ]
  %465 = phi ptr [ %460, %458 ], [ %416, %456 ], [ %374, %454 ], [ %374, %452 ]
  %466 = phi ptr [ %461, %458 ], [ %418, %456 ], [ %376, %452 ], [ %375, %454 ]
  %467 = phi { ptr, i32 } [ %462, %458 ], [ %457, %456 ], [ %453, %452 ], [ %455, %454 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %8) #16
  br label %471

468:                                              ; preds = %379
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %8) #16
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %7) #16
  %469 = add nuw nsw i64 %354, 1
  %470 = icmp eq i64 %469, %329
  br i1 %470, label %330, label %353, !llvm.loop !22

471:                                              ; preds = %450, %463
  %472 = phi ptr [ %366, %463 ], [ %360, %450 ]
  %473 = phi ptr [ %464, %463 ], [ %358, %450 ]
  %474 = phi ptr [ %465, %463 ], [ %357, %450 ]
  %475 = phi ptr [ %466, %463 ], [ %355, %450 ]
  %476 = phi { ptr, i32 } [ %467, %463 ], [ %451, %450 ]
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %7) #16
  %477 = icmp eq ptr %474, null
  br i1 %477, label %494, label %478

478:                                              ; preds = %471
  %479 = ptrtoint ptr %475 to i64
  %480 = ptrtoint ptr %474 to i64
  %481 = sub i64 %479, %480
  call void @_ZdlPvm(ptr noundef nonnull %474, i64 noundef %481) #14
  br label %494

482:                                              ; preds = %341
  %483 = icmp eq ptr %334, null
  br i1 %483, label %488, label %484

484:                                              ; preds = %482
  %485 = ptrtoint ptr %336 to i64
  %486 = ptrtoint ptr %334 to i64
  %487 = sub i64 %485, %486
  call void @_ZdlPvm(ptr noundef nonnull %334, i64 noundef %487) #14
  br label %488

488:                                              ; preds = %484, %482, %262
  %489 = phi ptr [ %266, %262 ], [ %331, %482 ], [ %331, %484 ]
  %490 = phi ptr [ %265, %262 ], [ %332, %482 ], [ %332, %484 ]
  %491 = phi ptr [ %264, %262 ], [ %333, %482 ], [ %333, %484 ]
  %492 = add nuw nsw i64 %263, 1
  %493 = icmp eq i64 %492, %47
  br i1 %493, label %510, label %262, !llvm.loop !23

494:                                              ; preds = %349, %351, %343, %478, %471
  %495 = phi ptr [ %472, %471 ], [ %472, %478 ], [ %266, %343 ], [ %312, %349 ], [ %312, %351 ]
  %496 = phi ptr [ %473, %471 ], [ %473, %478 ], [ %264, %343 ], [ %310, %349 ], [ %310, %351 ]
  %497 = phi ptr [ %309, %471 ], [ %309, %478 ], [ %344, %343 ], [ %309, %349 ], [ %309, %351 ]
  %498 = phi ptr [ %307, %471 ], [ %307, %478 ], [ %345, %343 ], [ %307, %349 ], [ %307, %351 ]
  %499 = phi { ptr, i32 } [ %476, %471 ], [ %476, %478 ], [ %346, %343 ], [ %350, %349 ], [ %352, %351 ]
  %500 = icmp eq ptr %497, null
  br i1 %500, label %601, label %501

501:                                              ; preds = %347, %494
  %502 = phi { ptr, i32 } [ %348, %347 ], [ %499, %494 ]
  %503 = phi ptr [ %274, %347 ], [ %498, %494 ]
  %504 = phi ptr [ %271, %347 ], [ %497, %494 ]
  %505 = phi ptr [ %264, %347 ], [ %496, %494 ]
  %506 = phi ptr [ %266, %347 ], [ %495, %494 ]
  %507 = ptrtoint ptr %503 to i64
  %508 = ptrtoint ptr %504 to i64
  %509 = sub i64 %507, %508
  call void @_ZdlPvm(ptr noundef nonnull %504, i64 noundef %509) #14
  br label %601

510:                                              ; preds = %488, %44, %247, %245
  %511 = phi ptr [ %92, %245 ], [ %92, %247 ], [ %39, %44 ], [ %489, %488 ]
  %512 = phi ptr [ %93, %245 ], [ %93, %247 ], [ %39, %44 ], [ %490, %488 ]
  %513 = phi ptr [ %94, %245 ], [ %94, %247 ], [ %40, %44 ], [ %491, %488 ]
  %514 = invoke double @omp_get_wtime()
          to label %515 unwind label %518

515:                                              ; preds = %510
  %516 = icmp eq ptr %511, %512
  br i1 %516, label %517, label %520

517:                                              ; preds = %515
  store ptr null, ptr %2, align 8, !tbaa !24
  store i32 0, ptr %3, align 4, !tbaa !13
  br label %587

518:                                              ; preds = %510
  %519 = landingpad { ptr, i32 }
          cleanup
  br label %601

520:                                              ; preds = %515
  %521 = ptrtoint ptr %512 to i64
  %522 = ptrtoint ptr %511 to i64
  %523 = sub i64 %521, %522
  %524 = ashr exact i64 %523, 2
  %525 = icmp sgt i64 %524, -1
  call void @llvm.assume(i1 %525)
  %526 = call noalias ptr @malloc(i64 noundef %523) #17
  %527 = icmp eq ptr %526, null
  br i1 %527, label %587, label %528

528:                                              ; preds = %520
  %529 = ptrtoint ptr %526 to i64
  %530 = call i64 @llvm.umax.i64(i64 %524, i64 1)
  %531 = icmp samesign ult i64 %524, 8
  %532 = sub i64 %529, %522
  %533 = icmp ult i64 %532, 32
  %534 = or i1 %531, %533
  br i1 %534, label %549, label %535

535:                                              ; preds = %528
  %536 = and i64 %530, 9223372036854775800
  br label %537

537:                                              ; preds = %537, %535
  %538 = phi i64 [ 0, %535 ], [ %545, %537 ]
  %539 = getelementptr inbounds nuw i32, ptr %511, i64 %538
  %540 = getelementptr inbounds nuw i8, ptr %539, i64 16
  %541 = load <4 x i32>, ptr %539, align 4, !tbaa !13
  %542 = load <4 x i32>, ptr %540, align 4, !tbaa !13
  %543 = getelementptr inbounds nuw i32, ptr %526, i64 %538
  %544 = getelementptr inbounds nuw i8, ptr %543, i64 16
  store <4 x i32> %541, ptr %543, align 4, !tbaa !13
  store <4 x i32> %542, ptr %544, align 4, !tbaa !13
  %545 = add nuw i64 %538, 8
  %546 = icmp eq i64 %545, %536
  br i1 %546, label %547, label %537, !llvm.loop !25

547:                                              ; preds = %537
  %548 = icmp eq i64 %524, %536
  br i1 %548, label %566, label %549

549:                                              ; preds = %528, %547
  %550 = phi i64 [ 0, %528 ], [ %536, %547 ]
  %551 = and i64 %530, 3
  %552 = icmp eq i64 %551, 0
  br i1 %552, label %562, label %553

553:                                              ; preds = %549, %553
  %554 = phi i64 [ %559, %553 ], [ %550, %549 ]
  %555 = phi i64 [ %560, %553 ], [ 0, %549 ]
  %556 = getelementptr inbounds nuw i32, ptr %511, i64 %554
  %557 = load i32, ptr %556, align 4, !tbaa !13
  %558 = getelementptr inbounds nuw i32, ptr %526, i64 %554
  store i32 %557, ptr %558, align 4, !tbaa !13
  %559 = add nuw nsw i64 %554, 1
  %560 = add i64 %555, 1
  %561 = icmp eq i64 %560, %551
  br i1 %561, label %562, label %553, !llvm.loop !28

562:                                              ; preds = %553, %549
  %563 = phi i64 [ %550, %549 ], [ %559, %553 ]
  %564 = sub nsw i64 %550, %530
  %565 = icmp ugt i64 %564, -4
  br i1 %565, label %566, label %568

566:                                              ; preds = %562, %568, %547
  store ptr %526, ptr %2, align 8, !tbaa !24
  %567 = trunc i64 %524 to i32
  store i32 %567, ptr %3, align 4, !tbaa !13
  br label %590

568:                                              ; preds = %562, %568
  %569 = phi i64 [ %585, %568 ], [ %563, %562 ]
  %570 = getelementptr inbounds nuw i32, ptr %511, i64 %569
  %571 = load i32, ptr %570, align 4, !tbaa !13
  %572 = getelementptr inbounds nuw i32, ptr %526, i64 %569
  store i32 %571, ptr %572, align 4, !tbaa !13
  %573 = add nuw nsw i64 %569, 1
  %574 = getelementptr inbounds nuw i32, ptr %511, i64 %573
  %575 = load i32, ptr %574, align 4, !tbaa !13
  %576 = getelementptr inbounds nuw i32, ptr %526, i64 %573
  store i32 %575, ptr %576, align 4, !tbaa !13
  %577 = add nuw nsw i64 %569, 2
  %578 = getelementptr inbounds nuw i32, ptr %511, i64 %577
  %579 = load i32, ptr %578, align 4, !tbaa !13
  %580 = getelementptr inbounds nuw i32, ptr %526, i64 %577
  store i32 %579, ptr %580, align 4, !tbaa !13
  %581 = add nuw nsw i64 %569, 3
  %582 = getelementptr inbounds nuw i32, ptr %511, i64 %581
  %583 = load i32, ptr %582, align 4, !tbaa !13
  %584 = getelementptr inbounds nuw i32, ptr %526, i64 %581
  store i32 %583, ptr %584, align 4, !tbaa !13
  %585 = add nuw nsw i64 %569, 4
  %586 = icmp eq i64 %524, %585
  br i1 %586, label %566, label %568, !llvm.loop !30

587:                                              ; preds = %517, %520
  %588 = phi i32 [ 4, %520 ], [ 0, %517 ]
  %589 = icmp eq ptr %511, null
  br i1 %589, label %595, label %590

590:                                              ; preds = %566, %587
  %591 = phi i32 [ 0, %566 ], [ %588, %587 ]
  %592 = ptrtoint ptr %513 to i64
  %593 = ptrtoint ptr %511 to i64
  %594 = sub i64 %592, %593
  call void @_ZdlPvm(ptr noundef nonnull %511, i64 noundef %594) #14
  br label %595

595:                                              ; preds = %587, %590
  %596 = phi i32 [ %588, %587 ], [ %591, %590 ]
  %597 = icmp eq ptr %37, null
  br i1 %597, label %620, label %598

598:                                              ; preds = %595
  %599 = ptrtoint ptr %37 to i64
  %600 = sub i64 %38, %599
  call void @_ZdlPvm(ptr noundef nonnull %37, i64 noundef %600) #14
  br label %620

601:                                              ; preds = %501, %494, %258, %251, %106, %518
  %602 = phi ptr [ %511, %518 ], [ %39, %106 ], [ %252, %251 ], [ %252, %258 ], [ %495, %494 ], [ %506, %501 ]
  %603 = phi ptr [ %513, %518 ], [ %40, %106 ], [ %253, %251 ], [ %253, %258 ], [ %496, %494 ], [ %505, %501 ]
  %604 = phi { ptr, i32 } [ %519, %518 ], [ %107, %106 ], [ %256, %251 ], [ %256, %258 ], [ %499, %494 ], [ %502, %501 ]
  %605 = icmp eq ptr %602, null
  br i1 %605, label %610, label %606

606:                                              ; preds = %601
  %607 = ptrtoint ptr %603 to i64
  %608 = ptrtoint ptr %602 to i64
  %609 = sub i64 %607, %608
  call void @_ZdlPvm(ptr noundef nonnull %602, i64 noundef %609) #14
  br label %610

610:                                              ; preds = %601, %606
  %611 = icmp eq ptr %37, null
  br i1 %611, label %618, label %612

612:                                              ; preds = %104, %610
  %613 = phi { ptr, i32 } [ %105, %104 ], [ %604, %610 ]
  %614 = phi i64 [ %30, %104 ], [ %38, %610 ]
  %615 = phi ptr [ %28, %104 ], [ %37, %610 ]
  %616 = ptrtoint ptr %615 to i64
  %617 = sub i64 %614, %616
  call void @_ZdlPvm(ptr noundef nonnull %615, i64 noundef %617) #14
  br label %618

618:                                              ; preds = %612, %610
  %619 = phi { ptr, i32 } [ %613, %612 ], [ %604, %610 ]
  resume { ptr, i32 } %619

620:                                              ; preds = %22, %14, %595, %598, %4
  %621 = phi i32 [ 1, %4 ], [ 3, %22 ], [ 2, %14 ], [ %596, %595 ], [ %596, %598 ]
  ret i32 %621
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local double @omp_get_wtime() local_unnamed_addr #3

declare dso_local void @autograph_neighbor_iter_init(ptr noundef, i64 noundef, ptr noundef) local_unnamed_addr #3

declare dso_local i32 @autograph_neighbor_iter_next(ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPvm(ptr noundef, i64 noundef) local_unnamed_addr #8

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #9

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #10

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #11

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #10 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #11 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #12 = { cold nounwind }
attributes #13 = { builtin allocsize(0) }
attributes #14 = { builtin nounwind }
attributes #15 = { cold noreturn }
attributes #16 = { nounwind }
attributes #17 = { nounwind allocsize(0) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"_ZTS5Graph", !5, i64 0, !5, i64 8, !8, i64 16, !10, i64 24}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"p1 long", !9, i64 0}
!9 = !{!"any pointer", !6, i64 0}
!10 = !{!"p1 int", !9, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"p1 _ZTS8_IO_FILE", !9, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !6, i64 0}
!15 = !{!6, !6, i64 0}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.mustprogress"}
!18 = distinct !{!18, !17}
!19 = distinct !{!19, !17}
!20 = distinct !{!20, !17}
!21 = distinct !{!21, !17}
!22 = distinct !{!22, !17}
!23 = distinct !{!23, !17}
!24 = !{!10, !10, i64 0}
!25 = distinct !{!25, !17, !26, !27}
!26 = !{!"llvm.loop.isvectorized", i32 1}
!27 = !{!"llvm.loop.unroll.runtime.disable"}
!28 = distinct !{!28, !29}
!29 = !{!"llvm.loop.unroll.disable"}
!30 = distinct !{!30, !17, !26}
