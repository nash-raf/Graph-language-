; ModuleID = 'bfs_runtime_src.cpp'
source_filename = "bfs_runtime_src.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.Graph = type { i64, i64, ptr, ptr }

@.str = private unnamed_addr constant [6 x i8] c"Work1\00", align 1
@.str.1 = private unnamed_addr constant [6 x i8] c"Work2\00", align 1
@.str.2 = private unnamed_addr constant [6 x i8] c"Work3\00", align 1
@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str.3 = private unnamed_addr constant [43 x i8] c"[bfs_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.4 = private unnamed_addr constant [6 x i8] c"Work4\00", align 1
@.str.5 = private unnamed_addr constant [67 x i8] c"[bfs_runtime_src] src=%d visited %zu / %lld nodes in %.6f seconds\0A\00", align 1
@.str.7 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.8 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @bfs_runtime_src(ptr noundef readonly %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str)
  %6 = icmp ne ptr %0, null
  %7 = icmp ne ptr %2, null
  %8 = and i1 %6, %7
  %9 = icmp ne ptr %3, null
  %10 = and i1 %8, %9
  br i1 %10, label %11, label %568

11:                                               ; preds = %4
  %12 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1)
  %13 = load i64, ptr %0, align 8, !tbaa !3
  %14 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %15 = load ptr, ptr %14, align 8, !tbaa !9
  %16 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %17 = load ptr, ptr %16, align 8, !tbaa !10
  %18 = icmp slt i64 %13, 0
  br i1 %18, label %568, label %19

19:                                               ; preds = %11
  %20 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.2)
  %21 = icmp slt i32 %1, 0
  %22 = trunc i64 %13 to i32
  %23 = icmp sgt i32 %22, %1
  %24 = or i1 %21, %23
  br i1 %24, label %28, label %25

25:                                               ; preds = %19
  %26 = load ptr, ptr @stderr, align 8, !tbaa !11
  %27 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %26, ptr noundef nonnull @.str.3, i32 noundef %1, i64 noundef %13) #11
  br label %568

28:                                               ; preds = %19
  %29 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.4)
  %30 = icmp eq i64 %13, 0
  br i1 %30, label %38, label %31

31:                                               ; preds = %28
  %32 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #12
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %32, i8 0, i64 %13, i1 false)
  %33 = tail call i64 @llvm.umin.i64(i64 %13, i64 1024)
  %34 = shl nuw nsw i64 %33, 2
  %35 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %34) #12
          to label %36 unwind label %101

36:                                               ; preds = %31
  %37 = getelementptr inbounds i32, ptr %35, i64 %33
  br label %38

38:                                               ; preds = %28, %36
  %39 = phi ptr [ %32, %36 ], [ null, %28 ]
  %40 = phi ptr [ %35, %36 ], [ null, %28 ]
  %41 = phi ptr [ %37, %36 ], [ null, %28 ]
  %42 = invoke double @omp_get_wtime()
          to label %43 unwind label %103

43:                                               ; preds = %38
  %44 = icmp sgt i32 %1, -1
  br i1 %44, label %49, label %45

45:                                               ; preds = %43
  %46 = icmp sgt i32 %22, 0
  br i1 %46, label %47, label %463

47:                                               ; preds = %45
  %48 = and i64 %13, 4294967295
  br label %240

49:                                               ; preds = %43
  %50 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #12
          to label %51 unwind label %105

51:                                               ; preds = %49
  store i32 %1, ptr %50, align 4, !tbaa !12
  %52 = getelementptr inbounds i32, ptr %50, i64 1
  %53 = zext nneg i32 %1 to i64
  %54 = getelementptr inbounds i8, ptr %39, i64 %53
  store i8 1, ptr %54, align 1, !tbaa !14
  %55 = icmp eq ptr %40, %41
  br i1 %55, label %57, label %56

56:                                               ; preds = %51
  store i32 %1, ptr %40, align 4, !tbaa !12
  br label %64

57:                                               ; preds = %51
  %58 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #12
          to label %59 unwind label %105

59:                                               ; preds = %57
  store i32 %1, ptr %58, align 4, !tbaa !12
  %60 = icmp eq ptr %40, null
  br i1 %60, label %62, label %61

61:                                               ; preds = %59
  tail call void @_ZdlPv(ptr noundef nonnull %40) #13
  br label %62

62:                                               ; preds = %61, %59
  %63 = getelementptr inbounds i32, ptr %58, i64 1
  br label %64

64:                                               ; preds = %56, %62
  %65 = phi ptr [ %63, %62 ], [ %41, %56 ]
  %66 = phi ptr [ %58, %62 ], [ %40, %56 ]
  %67 = getelementptr inbounds i32, ptr %66, i64 1
  br label %68

68:                                               ; preds = %64, %99
  %69 = phi ptr [ %52, %64 ], [ %96, %99 ]
  %70 = phi ptr [ %50, %64 ], [ %95, %99 ]
  %71 = phi ptr [ %66, %64 ], [ %94, %99 ]
  %72 = phi ptr [ %65, %64 ], [ %93, %99 ]
  %73 = phi ptr [ %67, %64 ], [ %92, %99 ]
  %74 = ptrtoint ptr %69 to i64
  %75 = ptrtoint ptr %70 to i64
  %76 = sub i64 %74, %75
  %77 = ashr exact i64 %76, 2
  %78 = icmp sgt i64 %77, -1
  tail call void @llvm.assume(i1 %78)
  %79 = ashr exact i64 %76, 1
  %80 = icmp ugt i64 %79, 2305843009213693951
  br i1 %80, label %81, label %83

81:                                               ; preds = %68
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.7) #14
          to label %82 unwind label %110

82:                                               ; preds = %81
  unreachable

83:                                               ; preds = %68
  %84 = icmp eq ptr %69, %70
  br i1 %84, label %91, label %85

85:                                               ; preds = %83
  %86 = shl nuw nsw i64 %76, 1
  %87 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %86) #12
          to label %88 unwind label %108

88:                                               ; preds = %85
  %89 = getelementptr inbounds i32, ptr %87, i64 %79
  %90 = tail call i64 @llvm.umax.i64(i64 %77, i64 1)
  br label %112

91:                                               ; preds = %128, %83
  %92 = phi ptr [ %73, %83 ], [ %129, %128 ]
  %93 = phi ptr [ %72, %83 ], [ %130, %128 ]
  %94 = phi ptr [ %71, %83 ], [ %131, %128 ]
  %95 = phi ptr [ null, %83 ], [ %132, %128 ]
  %96 = phi ptr [ null, %83 ], [ %133, %128 ]
  %97 = icmp eq ptr %70, null
  br i1 %97, label %99, label %98

98:                                               ; preds = %91
  tail call void @_ZdlPv(ptr noundef nonnull %70) #13
  br label %99

99:                                               ; preds = %91, %98
  %100 = icmp eq ptr %95, %96
  br i1 %100, label %231, label %68, !llvm.loop !15

101:                                              ; preds = %31
  %102 = landingpad { ptr, i32 }
          cleanup
  br label %563

103:                                              ; preds = %38
  %104 = landingpad { ptr, i32 }
          cleanup
  br label %556

105:                                              ; preds = %57, %49
  %106 = phi ptr [ %50, %57 ], [ null, %49 ]
  %107 = landingpad { ptr, i32 }
          cleanup
  br label %234

108:                                              ; preds = %85
  %109 = landingpad { ptr, i32 }
          cleanup
  br label %234

110:                                              ; preds = %81
  %111 = landingpad { ptr, i32 }
          cleanup
  br label %234

112:                                              ; preds = %88, %128
  %113 = phi i64 [ %135, %128 ], [ 0, %88 ]
  %114 = phi ptr [ %134, %128 ], [ %89, %88 ]
  %115 = phi ptr [ %133, %128 ], [ %87, %88 ]
  %116 = phi ptr [ %132, %128 ], [ %87, %88 ]
  %117 = phi ptr [ %131, %128 ], [ %71, %88 ]
  %118 = phi ptr [ %130, %128 ], [ %72, %88 ]
  %119 = phi ptr [ %129, %128 ], [ %73, %88 ]
  %120 = getelementptr inbounds i32, ptr %70, i64 %113
  %121 = load i32, ptr %120, align 4, !tbaa !12
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds i64, ptr %15, i64 %122
  %124 = load i64, ptr %123, align 8, !tbaa !17
  %125 = getelementptr i64, ptr %123, i64 1
  %126 = load i64, ptr %125, align 8, !tbaa !17
  %127 = icmp slt i64 %124, %126
  br i1 %127, label %137, label %128

128:                                              ; preds = %211, %112
  %129 = phi ptr [ %119, %112 ], [ %212, %211 ]
  %130 = phi ptr [ %118, %112 ], [ %213, %211 ]
  %131 = phi ptr [ %117, %112 ], [ %214, %211 ]
  %132 = phi ptr [ %116, %112 ], [ %215, %211 ]
  %133 = phi ptr [ %115, %112 ], [ %216, %211 ]
  %134 = phi ptr [ %114, %112 ], [ %217, %211 ]
  %135 = add nuw nsw i64 %113, 1
  %136 = icmp eq i64 %135, %90
  br i1 %136, label %91, label %112, !llvm.loop !18

137:                                              ; preds = %112, %211
  %138 = phi i64 [ %218, %211 ], [ %124, %112 ]
  %139 = phi ptr [ %217, %211 ], [ %114, %112 ]
  %140 = phi ptr [ %216, %211 ], [ %115, %112 ]
  %141 = phi ptr [ %215, %211 ], [ %116, %112 ]
  %142 = phi ptr [ %214, %211 ], [ %117, %112 ]
  %143 = phi ptr [ %213, %211 ], [ %118, %112 ]
  %144 = phi ptr [ %212, %211 ], [ %119, %112 ]
  %145 = getelementptr inbounds i32, ptr %17, i64 %138
  %146 = load i32, ptr %145, align 4, !tbaa !12
  %147 = sext i32 %146 to i64
  %148 = getelementptr inbounds i8, ptr %39, i64 %147
  %149 = load i8, ptr %148, align 1, !tbaa !14
  %150 = icmp eq i8 %149, 0
  br i1 %150, label %151, label %211

151:                                              ; preds = %137
  store i8 1, ptr %148, align 1, !tbaa !14
  %152 = icmp eq ptr %140, %139
  br i1 %152, label %154, label %153

153:                                              ; preds = %151
  store i32 %146, ptr %140, align 4, !tbaa !12
  br label %178

154:                                              ; preds = %151
  %155 = ptrtoint ptr %139 to i64
  %156 = ptrtoint ptr %141 to i64
  %157 = sub i64 %155, %156
  %158 = ashr exact i64 %157, 2
  %159 = icmp sgt i64 %158, -1
  tail call void @llvm.assume(i1 %159)
  %160 = icmp eq i64 %157, 9223372036854775804
  br i1 %160, label %161, label %163

161:                                              ; preds = %154
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.8) #14
          to label %162 unwind label %223

162:                                              ; preds = %161
  unreachable

163:                                              ; preds = %154
  %164 = tail call i64 @llvm.umax.i64(i64 %158, i64 1)
  %165 = add nuw nsw i64 %164, %158
  %166 = tail call noundef i64 @llvm.umin.i64(i64 %165, i64 2305843009213693951)
  %167 = shl nuw nsw i64 %166, 2
  %168 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %167) #12
          to label %169 unwind label %220

169:                                              ; preds = %163
  %170 = getelementptr inbounds i8, ptr %168, i64 %157
  store i32 %146, ptr %170, align 4, !tbaa !12
  %171 = icmp sgt i64 %157, 0
  br i1 %171, label %172, label %173

172:                                              ; preds = %169
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %168, ptr align 4 %141, i64 %157, i1 false)
  br label %173

173:                                              ; preds = %172, %169
  %174 = icmp eq ptr %141, null
  br i1 %174, label %176, label %175

175:                                              ; preds = %173
  tail call void @_ZdlPv(ptr noundef nonnull %141) #13
  br label %176

176:                                              ; preds = %175, %173
  %177 = getelementptr inbounds i32, ptr %168, i64 %166
  br label %178

178:                                              ; preds = %176, %153
  %179 = phi ptr [ %168, %176 ], [ %141, %153 ]
  %180 = phi ptr [ %170, %176 ], [ %140, %153 ]
  %181 = phi ptr [ %177, %176 ], [ %139, %153 ]
  %182 = getelementptr inbounds i32, ptr %180, i64 1
  %183 = icmp eq ptr %144, %143
  br i1 %183, label %186, label %184

184:                                              ; preds = %178
  store i32 %146, ptr %144, align 4, !tbaa !12
  %185 = getelementptr inbounds i32, ptr %144, i64 1
  br label %211

186:                                              ; preds = %178
  %187 = ptrtoint ptr %143 to i64
  %188 = ptrtoint ptr %142 to i64
  %189 = sub i64 %187, %188
  %190 = ashr exact i64 %189, 2
  %191 = icmp sgt i64 %190, -1
  tail call void @llvm.assume(i1 %191)
  %192 = icmp eq i64 %189, 9223372036854775804
  br i1 %192, label %193, label %195

193:                                              ; preds = %186
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.8) #14
          to label %194 unwind label %223

194:                                              ; preds = %193
  unreachable

195:                                              ; preds = %186
  %196 = tail call i64 @llvm.umax.i64(i64 %190, i64 1)
  %197 = add nuw nsw i64 %196, %190
  %198 = tail call noundef i64 @llvm.umin.i64(i64 %197, i64 2305843009213693951)
  %199 = shl nuw nsw i64 %198, 2
  %200 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %199) #12
          to label %201 unwind label %220

201:                                              ; preds = %195
  %202 = getelementptr inbounds i8, ptr %200, i64 %189
  store i32 %146, ptr %202, align 4, !tbaa !12
  %203 = icmp sgt i64 %189, 0
  br i1 %203, label %204, label %205

204:                                              ; preds = %201
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %200, ptr align 4 %142, i64 %189, i1 false)
  br label %205

205:                                              ; preds = %204, %201
  %206 = icmp eq ptr %142, null
  br i1 %206, label %208, label %207

207:                                              ; preds = %205
  tail call void @_ZdlPv(ptr noundef nonnull %142) #13
  br label %208

208:                                              ; preds = %207, %205
  %209 = getelementptr inbounds i32, ptr %202, i64 1
  %210 = getelementptr inbounds i32, ptr %200, i64 %198
  br label %211

211:                                              ; preds = %208, %184, %137
  %212 = phi ptr [ %144, %137 ], [ %209, %208 ], [ %185, %184 ]
  %213 = phi ptr [ %143, %137 ], [ %210, %208 ], [ %143, %184 ]
  %214 = phi ptr [ %142, %137 ], [ %200, %208 ], [ %142, %184 ]
  %215 = phi ptr [ %141, %137 ], [ %179, %208 ], [ %179, %184 ]
  %216 = phi ptr [ %140, %137 ], [ %182, %208 ], [ %182, %184 ]
  %217 = phi ptr [ %139, %137 ], [ %181, %208 ], [ %181, %184 ]
  %218 = add i64 %138, 1
  %219 = icmp eq i64 %218, %126
  br i1 %219, label %128, label %137, !llvm.loop !19

220:                                              ; preds = %195, %163
  %221 = phi ptr [ %141, %163 ], [ %179, %195 ]
  %222 = landingpad { ptr, i32 }
          cleanup
  br label %226

223:                                              ; preds = %193, %161
  %224 = phi ptr [ %179, %193 ], [ %141, %161 ]
  %225 = landingpad { ptr, i32 }
          cleanup
  br label %226

226:                                              ; preds = %223, %220
  %227 = phi ptr [ %221, %220 ], [ %224, %223 ]
  %228 = phi { ptr, i32 } [ %222, %220 ], [ %225, %223 ]
  %229 = icmp eq ptr %227, null
  br i1 %229, label %234, label %230

230:                                              ; preds = %226
  tail call void @_ZdlPv(ptr noundef nonnull %227) #13
  br label %234

231:                                              ; preds = %99
  %232 = icmp eq ptr %95, null
  br i1 %232, label %463, label %233

233:                                              ; preds = %231
  tail call void @_ZdlPv(ptr noundef nonnull %95) #13
  br label %463

234:                                              ; preds = %108, %110, %230, %226, %105
  %235 = phi ptr [ %40, %105 ], [ %142, %226 ], [ %142, %230 ], [ %71, %108 ], [ %71, %110 ]
  %236 = phi ptr [ %106, %105 ], [ %70, %226 ], [ %70, %230 ], [ %70, %108 ], [ %70, %110 ]
  %237 = phi { ptr, i32 } [ %107, %105 ], [ %228, %226 ], [ %228, %230 ], [ %109, %108 ], [ %111, %110 ]
  %238 = icmp eq ptr %236, null
  br i1 %238, label %556, label %239

239:                                              ; preds = %234
  tail call void @_ZdlPv(ptr noundef nonnull %236) #13
  br label %556

240:                                              ; preds = %47, %448
  %241 = phi i64 [ 0, %47 ], [ %452, %448 ]
  %242 = phi ptr [ %40, %47 ], [ %451, %448 ]
  %243 = phi ptr [ %41, %47 ], [ %450, %448 ]
  %244 = phi ptr [ %40, %47 ], [ %449, %448 ]
  %245 = getelementptr inbounds i8, ptr %39, i64 %241
  %246 = load i8, ptr %245, align 1, !tbaa !14
  %247 = icmp eq i8 %246, 0
  br i1 %247, label %248, label %448

248:                                              ; preds = %240
  %249 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #12
          to label %250 unwind label %317

250:                                              ; preds = %248
  %251 = trunc i64 %241 to i32
  store i32 %251, ptr %249, align 4, !tbaa !12
  %252 = getelementptr inbounds i32, ptr %249, i64 1
  store i8 1, ptr %245, align 1, !tbaa !14
  %253 = icmp eq ptr %244, %243
  br i1 %253, label %255, label %254

254:                                              ; preds = %250
  store i32 %251, ptr %244, align 4, !tbaa !12
  br label %279

255:                                              ; preds = %250
  %256 = ptrtoint ptr %243 to i64
  %257 = ptrtoint ptr %242 to i64
  %258 = sub i64 %256, %257
  %259 = ashr exact i64 %258, 2
  %260 = icmp sgt i64 %259, -1
  tail call void @llvm.assume(i1 %260)
  %261 = icmp eq i64 %258, 9223372036854775804
  br i1 %261, label %262, label %264

262:                                              ; preds = %255
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.8) #14
          to label %263 unwind label %320

263:                                              ; preds = %262
  unreachable

264:                                              ; preds = %255
  %265 = tail call i64 @llvm.umax.i64(i64 %259, i64 1)
  %266 = add nuw nsw i64 %265, %259
  %267 = tail call noundef i64 @llvm.umin.i64(i64 %266, i64 2305843009213693951)
  %268 = shl nuw nsw i64 %267, 2
  %269 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %268) #12
          to label %270 unwind label %317

270:                                              ; preds = %264
  %271 = getelementptr inbounds i8, ptr %269, i64 %258
  store i32 %251, ptr %271, align 4, !tbaa !12
  %272 = icmp sgt i64 %258, 0
  br i1 %272, label %273, label %274

273:                                              ; preds = %270
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %269, ptr align 4 %242, i64 %258, i1 false)
  br label %274

274:                                              ; preds = %273, %270
  %275 = icmp eq ptr %242, null
  br i1 %275, label %277, label %276

276:                                              ; preds = %274
  tail call void @_ZdlPv(ptr noundef nonnull %242) #13
  br label %277

277:                                              ; preds = %276, %274
  %278 = getelementptr inbounds i32, ptr %269, i64 %267
  br label %279

279:                                              ; preds = %254, %277
  %280 = phi ptr [ %271, %277 ], [ %244, %254 ]
  %281 = phi ptr [ %278, %277 ], [ %243, %254 ]
  %282 = phi ptr [ %269, %277 ], [ %242, %254 ]
  %283 = getelementptr inbounds i32, ptr %280, i64 1
  br label %284

284:                                              ; preds = %279, %315
  %285 = phi ptr [ %252, %279 ], [ %312, %315 ]
  %286 = phi ptr [ %249, %279 ], [ %311, %315 ]
  %287 = phi ptr [ %282, %279 ], [ %310, %315 ]
  %288 = phi ptr [ %281, %279 ], [ %309, %315 ]
  %289 = phi ptr [ %283, %279 ], [ %308, %315 ]
  %290 = ptrtoint ptr %285 to i64
  %291 = ptrtoint ptr %286 to i64
  %292 = sub i64 %290, %291
  %293 = ashr exact i64 %292, 2
  %294 = icmp sgt i64 %293, -1
  tail call void @llvm.assume(i1 %294)
  %295 = ashr exact i64 %292, 1
  %296 = icmp ugt i64 %295, 2305843009213693951
  br i1 %296, label %297, label %299

297:                                              ; preds = %284
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.7) #14
          to label %298 unwind label %324

298:                                              ; preds = %297
  unreachable

299:                                              ; preds = %284
  %300 = icmp eq ptr %285, %286
  br i1 %300, label %307, label %301

301:                                              ; preds = %299
  %302 = shl nuw nsw i64 %292, 1
  %303 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %302) #12
          to label %304 unwind label %322

304:                                              ; preds = %301
  %305 = getelementptr inbounds i32, ptr %303, i64 %295
  %306 = tail call i64 @llvm.umax.i64(i64 %293, i64 1)
  br label %326

307:                                              ; preds = %342, %299
  %308 = phi ptr [ %289, %299 ], [ %343, %342 ]
  %309 = phi ptr [ %288, %299 ], [ %344, %342 ]
  %310 = phi ptr [ %287, %299 ], [ %345, %342 ]
  %311 = phi ptr [ null, %299 ], [ %346, %342 ]
  %312 = phi ptr [ null, %299 ], [ %347, %342 ]
  %313 = icmp eq ptr %286, null
  br i1 %313, label %315, label %314

314:                                              ; preds = %307
  tail call void @_ZdlPv(ptr noundef nonnull %286) #13
  br label %315

315:                                              ; preds = %307, %314
  %316 = icmp eq ptr %311, %312
  br i1 %316, label %445, label %284, !llvm.loop !20

317:                                              ; preds = %248, %264
  %318 = phi ptr [ null, %248 ], [ %249, %264 ]
  %319 = landingpad { ptr, i32 }
          cleanup
  br label %454

320:                                              ; preds = %262
  %321 = landingpad { ptr, i32 }
          cleanup
  br label %459

322:                                              ; preds = %301
  %323 = landingpad { ptr, i32 }
          cleanup
  br label %454

324:                                              ; preds = %297
  %325 = landingpad { ptr, i32 }
          cleanup
  br label %454

326:                                              ; preds = %304, %342
  %327 = phi i64 [ %349, %342 ], [ 0, %304 ]
  %328 = phi ptr [ %348, %342 ], [ %305, %304 ]
  %329 = phi ptr [ %347, %342 ], [ %303, %304 ]
  %330 = phi ptr [ %346, %342 ], [ %303, %304 ]
  %331 = phi ptr [ %345, %342 ], [ %287, %304 ]
  %332 = phi ptr [ %344, %342 ], [ %288, %304 ]
  %333 = phi ptr [ %343, %342 ], [ %289, %304 ]
  %334 = getelementptr inbounds i32, ptr %286, i64 %327
  %335 = load i32, ptr %334, align 4, !tbaa !12
  %336 = sext i32 %335 to i64
  %337 = getelementptr inbounds i64, ptr %15, i64 %336
  %338 = load i64, ptr %337, align 8, !tbaa !17
  %339 = getelementptr i64, ptr %337, i64 1
  %340 = load i64, ptr %339, align 8, !tbaa !17
  %341 = icmp slt i64 %338, %340
  br i1 %341, label %351, label %342

342:                                              ; preds = %425, %326
  %343 = phi ptr [ %333, %326 ], [ %426, %425 ]
  %344 = phi ptr [ %332, %326 ], [ %427, %425 ]
  %345 = phi ptr [ %331, %326 ], [ %428, %425 ]
  %346 = phi ptr [ %330, %326 ], [ %429, %425 ]
  %347 = phi ptr [ %329, %326 ], [ %430, %425 ]
  %348 = phi ptr [ %328, %326 ], [ %431, %425 ]
  %349 = add nuw nsw i64 %327, 1
  %350 = icmp eq i64 %349, %306
  br i1 %350, label %307, label %326, !llvm.loop !21

351:                                              ; preds = %326, %425
  %352 = phi i64 [ %432, %425 ], [ %338, %326 ]
  %353 = phi ptr [ %431, %425 ], [ %328, %326 ]
  %354 = phi ptr [ %430, %425 ], [ %329, %326 ]
  %355 = phi ptr [ %429, %425 ], [ %330, %326 ]
  %356 = phi ptr [ %428, %425 ], [ %331, %326 ]
  %357 = phi ptr [ %427, %425 ], [ %332, %326 ]
  %358 = phi ptr [ %426, %425 ], [ %333, %326 ]
  %359 = getelementptr inbounds i32, ptr %17, i64 %352
  %360 = load i32, ptr %359, align 4, !tbaa !12
  %361 = sext i32 %360 to i64
  %362 = getelementptr inbounds i8, ptr %39, i64 %361
  %363 = load i8, ptr %362, align 1, !tbaa !14
  %364 = icmp eq i8 %363, 0
  br i1 %364, label %365, label %425

365:                                              ; preds = %351
  store i8 1, ptr %362, align 1, !tbaa !14
  %366 = icmp eq ptr %354, %353
  br i1 %366, label %368, label %367

367:                                              ; preds = %365
  store i32 %360, ptr %354, align 4, !tbaa !12
  br label %392

368:                                              ; preds = %365
  %369 = ptrtoint ptr %353 to i64
  %370 = ptrtoint ptr %355 to i64
  %371 = sub i64 %369, %370
  %372 = ashr exact i64 %371, 2
  %373 = icmp sgt i64 %372, -1
  tail call void @llvm.assume(i1 %373)
  %374 = icmp eq i64 %371, 9223372036854775804
  br i1 %374, label %375, label %377

375:                                              ; preds = %368
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.8) #14
          to label %376 unwind label %437

376:                                              ; preds = %375
  unreachable

377:                                              ; preds = %368
  %378 = tail call i64 @llvm.umax.i64(i64 %372, i64 1)
  %379 = add nuw nsw i64 %378, %372
  %380 = tail call noundef i64 @llvm.umin.i64(i64 %379, i64 2305843009213693951)
  %381 = shl nuw nsw i64 %380, 2
  %382 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %381) #12
          to label %383 unwind label %434

383:                                              ; preds = %377
  %384 = getelementptr inbounds i8, ptr %382, i64 %371
  store i32 %360, ptr %384, align 4, !tbaa !12
  %385 = icmp sgt i64 %371, 0
  br i1 %385, label %386, label %387

386:                                              ; preds = %383
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %382, ptr align 4 %355, i64 %371, i1 false)
  br label %387

387:                                              ; preds = %386, %383
  %388 = icmp eq ptr %355, null
  br i1 %388, label %390, label %389

389:                                              ; preds = %387
  tail call void @_ZdlPv(ptr noundef nonnull %355) #13
  br label %390

390:                                              ; preds = %389, %387
  %391 = getelementptr inbounds i32, ptr %382, i64 %380
  br label %392

392:                                              ; preds = %390, %367
  %393 = phi ptr [ %382, %390 ], [ %355, %367 ]
  %394 = phi ptr [ %384, %390 ], [ %354, %367 ]
  %395 = phi ptr [ %391, %390 ], [ %353, %367 ]
  %396 = getelementptr inbounds i32, ptr %394, i64 1
  %397 = icmp eq ptr %358, %357
  br i1 %397, label %400, label %398

398:                                              ; preds = %392
  store i32 %360, ptr %358, align 4, !tbaa !12
  %399 = getelementptr inbounds i32, ptr %358, i64 1
  br label %425

400:                                              ; preds = %392
  %401 = ptrtoint ptr %357 to i64
  %402 = ptrtoint ptr %356 to i64
  %403 = sub i64 %401, %402
  %404 = ashr exact i64 %403, 2
  %405 = icmp sgt i64 %404, -1
  tail call void @llvm.assume(i1 %405)
  %406 = icmp eq i64 %403, 9223372036854775804
  br i1 %406, label %407, label %409

407:                                              ; preds = %400
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.8) #14
          to label %408 unwind label %437

408:                                              ; preds = %407
  unreachable

409:                                              ; preds = %400
  %410 = tail call i64 @llvm.umax.i64(i64 %404, i64 1)
  %411 = add nuw nsw i64 %410, %404
  %412 = tail call noundef i64 @llvm.umin.i64(i64 %411, i64 2305843009213693951)
  %413 = shl nuw nsw i64 %412, 2
  %414 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %413) #12
          to label %415 unwind label %434

415:                                              ; preds = %409
  %416 = getelementptr inbounds i8, ptr %414, i64 %403
  store i32 %360, ptr %416, align 4, !tbaa !12
  %417 = icmp sgt i64 %403, 0
  br i1 %417, label %418, label %419

418:                                              ; preds = %415
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %414, ptr align 4 %356, i64 %403, i1 false)
  br label %419

419:                                              ; preds = %418, %415
  %420 = icmp eq ptr %356, null
  br i1 %420, label %422, label %421

421:                                              ; preds = %419
  tail call void @_ZdlPv(ptr noundef nonnull %356) #13
  br label %422

422:                                              ; preds = %421, %419
  %423 = getelementptr inbounds i32, ptr %416, i64 1
  %424 = getelementptr inbounds i32, ptr %414, i64 %412
  br label %425

425:                                              ; preds = %422, %398, %351
  %426 = phi ptr [ %358, %351 ], [ %423, %422 ], [ %399, %398 ]
  %427 = phi ptr [ %357, %351 ], [ %424, %422 ], [ %357, %398 ]
  %428 = phi ptr [ %356, %351 ], [ %414, %422 ], [ %356, %398 ]
  %429 = phi ptr [ %355, %351 ], [ %393, %422 ], [ %393, %398 ]
  %430 = phi ptr [ %354, %351 ], [ %396, %422 ], [ %396, %398 ]
  %431 = phi ptr [ %353, %351 ], [ %395, %422 ], [ %395, %398 ]
  %432 = add i64 %352, 1
  %433 = icmp eq i64 %432, %340
  br i1 %433, label %342, label %351, !llvm.loop !22

434:                                              ; preds = %409, %377
  %435 = phi ptr [ %355, %377 ], [ %393, %409 ]
  %436 = landingpad { ptr, i32 }
          cleanup
  br label %440

437:                                              ; preds = %407, %375
  %438 = phi ptr [ %393, %407 ], [ %355, %375 ]
  %439 = landingpad { ptr, i32 }
          cleanup
  br label %440

440:                                              ; preds = %437, %434
  %441 = phi ptr [ %435, %434 ], [ %438, %437 ]
  %442 = phi { ptr, i32 } [ %436, %434 ], [ %439, %437 ]
  %443 = icmp eq ptr %441, null
  br i1 %443, label %454, label %444

444:                                              ; preds = %440
  tail call void @_ZdlPv(ptr noundef nonnull %441) #13
  br label %454

445:                                              ; preds = %315
  %446 = icmp eq ptr %311, null
  br i1 %446, label %448, label %447

447:                                              ; preds = %445
  tail call void @_ZdlPv(ptr noundef nonnull %311) #13
  br label %448

448:                                              ; preds = %447, %445, %240
  %449 = phi ptr [ %244, %240 ], [ %308, %445 ], [ %308, %447 ]
  %450 = phi ptr [ %243, %240 ], [ %309, %445 ], [ %309, %447 ]
  %451 = phi ptr [ %242, %240 ], [ %310, %445 ], [ %310, %447 ]
  %452 = add nuw nsw i64 %241, 1
  %453 = icmp eq i64 %452, %48
  br i1 %453, label %463, label %240, !llvm.loop !23

454:                                              ; preds = %322, %324, %317, %444, %440
  %455 = phi ptr [ %356, %440 ], [ %356, %444 ], [ %242, %317 ], [ %287, %322 ], [ %287, %324 ]
  %456 = phi ptr [ %286, %440 ], [ %286, %444 ], [ %318, %317 ], [ %286, %322 ], [ %286, %324 ]
  %457 = phi { ptr, i32 } [ %442, %440 ], [ %442, %444 ], [ %319, %317 ], [ %323, %322 ], [ %325, %324 ]
  %458 = icmp eq ptr %456, null
  br i1 %458, label %556, label %459

459:                                              ; preds = %320, %454
  %460 = phi { ptr, i32 } [ %321, %320 ], [ %457, %454 ]
  %461 = phi ptr [ %249, %320 ], [ %456, %454 ]
  %462 = phi ptr [ %242, %320 ], [ %455, %454 ]
  tail call void @_ZdlPv(ptr noundef nonnull %461) #13
  br label %556

463:                                              ; preds = %448, %45, %233, %231
  %464 = phi ptr [ %92, %231 ], [ %92, %233 ], [ %40, %45 ], [ %449, %448 ]
  %465 = phi ptr [ %94, %231 ], [ %94, %233 ], [ %40, %45 ], [ %451, %448 ]
  %466 = invoke double @omp_get_wtime()
          to label %467 unwind label %474

467:                                              ; preds = %463
  %468 = icmp eq ptr %465, %464
  br i1 %468, label %469, label %476

469:                                              ; preds = %467
  store ptr null, ptr %2, align 8, !tbaa !11
  %470 = ptrtoint ptr %464 to i64
  %471 = ptrtoint ptr %465 to i64
  %472 = sub i64 %470, %471
  %473 = ashr exact i64 %472, 2
  br label %543

474:                                              ; preds = %463
  %475 = landingpad { ptr, i32 }
          cleanup
  br label %556

476:                                              ; preds = %467
  %477 = ptrtoint ptr %464 to i64
  %478 = ptrtoint ptr %465 to i64
  %479 = sub i64 %477, %478
  %480 = ashr exact i64 %479, 2
  %481 = icmp sgt i64 %480, -1
  tail call void @llvm.assume(i1 %481)
  %482 = tail call noalias ptr @malloc(i64 noundef %479) #15
  %483 = icmp eq ptr %482, null
  br i1 %483, label %549, label %484

484:                                              ; preds = %476
  %485 = ptrtoint ptr %482 to i64
  %486 = tail call i64 @llvm.umax.i64(i64 %480, i64 1)
  %487 = icmp ult i64 %480, 8
  %488 = sub i64 %485, %478
  %489 = icmp ult i64 %488, 32
  %490 = or i1 %487, %489
  br i1 %490, label %505, label %491

491:                                              ; preds = %484
  %492 = and i64 %486, 9223372036854775800
  br label %493

493:                                              ; preds = %493, %491
  %494 = phi i64 [ 0, %491 ], [ %501, %493 ]
  %495 = getelementptr inbounds i32, ptr %465, i64 %494
  %496 = getelementptr inbounds i32, ptr %495, i64 4
  %497 = load <4 x i32>, ptr %495, align 4, !tbaa !12
  %498 = load <4 x i32>, ptr %496, align 4, !tbaa !12
  %499 = getelementptr inbounds i32, ptr %482, i64 %494
  %500 = getelementptr inbounds i32, ptr %499, i64 4
  store <4 x i32> %497, ptr %499, align 4, !tbaa !12
  store <4 x i32> %498, ptr %500, align 4, !tbaa !12
  %501 = add nuw i64 %494, 8
  %502 = icmp eq i64 %501, %492
  br i1 %502, label %503, label %493, !llvm.loop !24

503:                                              ; preds = %493
  %504 = icmp eq i64 %486, %492
  br i1 %504, label %522, label %505

505:                                              ; preds = %484, %503
  %506 = phi i64 [ 0, %484 ], [ %492, %503 ]
  %507 = and i64 %486, 3
  %508 = icmp eq i64 %507, 0
  br i1 %508, label %518, label %509

509:                                              ; preds = %505, %509
  %510 = phi i64 [ %515, %509 ], [ %506, %505 ]
  %511 = phi i64 [ %516, %509 ], [ 0, %505 ]
  %512 = getelementptr inbounds i32, ptr %465, i64 %510
  %513 = load i32, ptr %512, align 4, !tbaa !12
  %514 = getelementptr inbounds i32, ptr %482, i64 %510
  store i32 %513, ptr %514, align 4, !tbaa !12
  %515 = add nuw nsw i64 %510, 1
  %516 = add i64 %511, 1
  %517 = icmp eq i64 %516, %507
  br i1 %517, label %518, label %509, !llvm.loop !27

518:                                              ; preds = %509, %505
  %519 = phi i64 [ %506, %505 ], [ %515, %509 ]
  %520 = sub nsw i64 %506, %486
  %521 = icmp ugt i64 %520, -4
  br i1 %521, label %522, label %524

522:                                              ; preds = %518, %524, %503
  store ptr %482, ptr %2, align 8, !tbaa !11
  %523 = trunc i64 %480 to i32
  br label %543

524:                                              ; preds = %518, %524
  %525 = phi i64 [ %541, %524 ], [ %519, %518 ]
  %526 = getelementptr inbounds i32, ptr %465, i64 %525
  %527 = load i32, ptr %526, align 4, !tbaa !12
  %528 = getelementptr inbounds i32, ptr %482, i64 %525
  store i32 %527, ptr %528, align 4, !tbaa !12
  %529 = add nuw nsw i64 %525, 1
  %530 = getelementptr inbounds i32, ptr %465, i64 %529
  %531 = load i32, ptr %530, align 4, !tbaa !12
  %532 = getelementptr inbounds i32, ptr %482, i64 %529
  store i32 %531, ptr %532, align 4, !tbaa !12
  %533 = add nuw nsw i64 %525, 2
  %534 = getelementptr inbounds i32, ptr %465, i64 %533
  %535 = load i32, ptr %534, align 4, !tbaa !12
  %536 = getelementptr inbounds i32, ptr %482, i64 %533
  store i32 %535, ptr %536, align 4, !tbaa !12
  %537 = add nuw nsw i64 %525, 3
  %538 = getelementptr inbounds i32, ptr %465, i64 %537
  %539 = load i32, ptr %538, align 4, !tbaa !12
  %540 = getelementptr inbounds i32, ptr %482, i64 %537
  store i32 %539, ptr %540, align 4, !tbaa !12
  %541 = add nuw nsw i64 %525, 4
  %542 = icmp eq i64 %480, %541
  br i1 %542, label %522, label %524, !llvm.loop !29

543:                                              ; preds = %522, %469
  %544 = phi i64 [ %480, %522 ], [ %473, %469 ]
  %545 = phi i32 [ %523, %522 ], [ 0, %469 ]
  store i32 %545, ptr %3, align 4, !tbaa !12
  %546 = icmp sgt i64 %544, -1
  tail call void @llvm.assume(i1 %546)
  %547 = fsub double %466, %42
  %548 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.5, i32 noundef %1, i64 noundef %544, i64 noundef %13, double noundef %547)
  br label %549

549:                                              ; preds = %476, %543
  %550 = phi i32 [ 0, %543 ], [ 4, %476 ]
  %551 = icmp eq ptr %465, null
  br i1 %551, label %553, label %552

552:                                              ; preds = %549
  tail call void @_ZdlPv(ptr noundef nonnull %465) #13
  br label %553

553:                                              ; preds = %549, %552
  %554 = icmp eq ptr %39, null
  br i1 %554, label %568, label %555

555:                                              ; preds = %553
  tail call void @_ZdlPv(ptr noundef nonnull %39) #13
  br label %568

556:                                              ; preds = %459, %454, %239, %234, %103, %474
  %557 = phi ptr [ %465, %474 ], [ %40, %103 ], [ %235, %234 ], [ %235, %239 ], [ %455, %454 ], [ %462, %459 ]
  %558 = phi { ptr, i32 } [ %475, %474 ], [ %104, %103 ], [ %237, %234 ], [ %237, %239 ], [ %457, %454 ], [ %460, %459 ]
  %559 = icmp eq ptr %557, null
  br i1 %559, label %561, label %560

560:                                              ; preds = %556
  tail call void @_ZdlPv(ptr noundef nonnull %557) #13
  br label %561

561:                                              ; preds = %556, %560
  %562 = icmp eq ptr %39, null
  br i1 %562, label %566, label %563

563:                                              ; preds = %101, %561
  %564 = phi { ptr, i32 } [ %102, %101 ], [ %558, %561 ]
  %565 = phi ptr [ %32, %101 ], [ %39, %561 ]
  tail call void @_ZdlPv(ptr noundef nonnull %565) #13
  br label %566

566:                                              ; preds = %563, %561
  %567 = phi { ptr, i32 } [ %564, %563 ], [ %558, %561 ]
  resume { ptr, i32 } %567

568:                                              ; preds = %25, %11, %553, %555, %4
  %569 = phi i32 [ 1, %4 ], [ 3, %25 ], [ 2, %11 ], [ %550, %553 ], [ %550, %555 ]
  ret i32 %569
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #1

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local double @omp_get_wtime() local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #3

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #4

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #6

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #10

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #10

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #7 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #10 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #11 = { cold }
attributes #12 = { builtin allocsize(0) }
attributes #13 = { builtin nounwind }
attributes #14 = { cold noreturn }
attributes #15 = { nounwind allocsize(0) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"_ZTS5Graph", !5, i64 0, !5, i64 8, !8, i64 16, !8, i64 24}
!5 = !{!"long", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"any pointer", !6, i64 0}
!9 = !{!4, !8, i64 16}
!10 = !{!4, !8, i64 24}
!11 = !{!8, !8, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !6, i64 0}
!14 = !{!6, !6, i64 0}
!15 = distinct !{!15, !16}
!16 = !{!"llvm.loop.mustprogress"}
!17 = !{!5, !5, i64 0}
!18 = distinct !{!18, !16}
!19 = distinct !{!19, !16}
!20 = distinct !{!20, !16}
!21 = distinct !{!21, !16}
!22 = distinct !{!22, !16}
!23 = distinct !{!23, !16}
!24 = distinct !{!24, !16, !25, !26}
!25 = !{!"llvm.loop.isvectorized", i32 1}
!26 = !{!"llvm.loop.unroll.runtime.disable"}
!27 = distinct !{!27, !28}
!28 = !{!"llvm.loop.unroll.disable"}
!29 = distinct !{!29, !16, !25}
