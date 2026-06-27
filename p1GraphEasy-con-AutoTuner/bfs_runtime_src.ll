; ModuleID = 'bfs_runtime_src.cpp'
source_filename = "bfs_runtime_src.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [43 x i8] c"[bfs_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.1 = private unnamed_addr constant [67 x i8] c"[bfs_runtime_src] src=%d visited %zu / %lld nodes in %.6f seconds\0A\00", align 1
@.str.2 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.3 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.4 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local range(i32 0, 5) i32 @bfs_runtime_src(ptr noundef %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = icmp ne ptr %0, null
  %8 = icmp ne ptr %2, null
  %9 = and i1 %7, %8
  %10 = icmp ne ptr %3, null
  %11 = and i1 %9, %10
  br i1 %11, label %12, label %665

12:                                               ; preds = %4
  %13 = load i64, ptr %0, align 8, !tbaa !3
  %14 = icmp slt i64 %13, 0
  br i1 %14, label %665, label %15

15:                                               ; preds = %12
  %16 = icmp slt i32 %1, 0
  %17 = trunc i64 %13 to i32
  %18 = icmp slt i32 %1, %17
  %19 = or i1 %16, %18
  br i1 %19, label %23, label %20

20:                                               ; preds = %15
  %21 = load ptr, ptr @stderr, align 8, !tbaa !11
  %22 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %21, ptr noundef nonnull @.str, i32 noundef %1, i64 noundef %13) #12
  br label %665

23:                                               ; preds = %15
  %24 = icmp eq i64 %13, 0
  br i1 %24, label %46, label %25

25:                                               ; preds = %23
  %26 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %13) #13
  %27 = getelementptr inbounds nuw i8, ptr %26, i64 %13
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %26, i8 0, i64 %13, i1 false)
  %28 = ptrtoint ptr %27 to i64
  %29 = tail call i64 @llvm.umin.i64(i64 %13, i64 1024)
  %30 = shl nuw nsw i64 %29, 2
  %31 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %30) #13
          to label %32 unwind label %116

32:                                               ; preds = %25
  %33 = getelementptr inbounds nuw i32, ptr %31, i64 %29
  %34 = icmp samesign ugt i64 %13, 2305843009213693951
  br i1 %34, label %35, label %37

35:                                               ; preds = %32
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #14
          to label %36 unwind label %118

36:                                               ; preds = %35
  unreachable

37:                                               ; preds = %32
  %38 = shl nuw nsw i64 %13, 2
  %39 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %38) #13
          to label %40 unwind label %118

40:                                               ; preds = %37
  %41 = getelementptr inbounds nuw i32, ptr %39, i64 %13
  store i32 0, ptr %39, align 4, !tbaa !13
  %42 = icmp eq i64 %13, 1
  br i1 %42, label %46, label %43

43:                                               ; preds = %40
  %44 = getelementptr i8, ptr %39, i64 4
  %45 = add nsw i64 %38, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %44, i8 0, i64 %45, i1 false), !tbaa !13
  br label %46

46:                                               ; preds = %23, %43, %40
  %47 = phi ptr [ %26, %40 ], [ %26, %43 ], [ null, %23 ]
  %48 = phi i64 [ %28, %40 ], [ %28, %43 ], [ 0, %23 ]
  %49 = phi ptr [ %33, %40 ], [ %33, %43 ], [ null, %23 ]
  %50 = phi ptr [ %31, %40 ], [ %31, %43 ], [ null, %23 ]
  %51 = phi ptr [ %39, %40 ], [ %39, %43 ], [ null, %23 ]
  %52 = phi ptr [ %41, %40 ], [ %41, %43 ], [ null, %23 ]
  %53 = invoke double @omp_get_wtime()
          to label %54 unwind label %120

54:                                               ; preds = %46
  %55 = icmp sgt i32 %1, -1
  br i1 %55, label %60, label %56

56:                                               ; preds = %54
  %57 = icmp sgt i32 %17, 0
  br i1 %57, label %58, label %527

58:                                               ; preds = %56
  %59 = and i64 %13, 2147483647
  br label %277

60:                                               ; preds = %54
  %61 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #13
          to label %62 unwind label %122

62:                                               ; preds = %60
  store i32 %1, ptr %61, align 4, !tbaa !13
  %63 = getelementptr inbounds nuw i8, ptr %61, i64 4
  %64 = zext nneg i32 %1 to i64
  %65 = getelementptr inbounds nuw i8, ptr %47, i64 %64
  store i8 1, ptr %65, align 1, !tbaa !15
  %66 = icmp eq ptr %50, %49
  br i1 %66, label %68, label %67

67:                                               ; preds = %62
  store i32 %1, ptr %50, align 4, !tbaa !13
  br label %75

68:                                               ; preds = %62
  %69 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #13
          to label %70 unwind label %122

70:                                               ; preds = %68
  store i32 %1, ptr %69, align 4, !tbaa !13
  %71 = icmp eq ptr %49, null
  br i1 %71, label %73, label %72

72:                                               ; preds = %70
  tail call void @_ZdlPvm(ptr noundef nonnull %50, i64 noundef 0) #15
  br label %73

73:                                               ; preds = %72, %70
  %74 = getelementptr inbounds nuw i8, ptr %69, i64 4
  br label %75

75:                                               ; preds = %67, %73
  %76 = phi ptr [ %74, %73 ], [ %49, %67 ]
  %77 = phi ptr [ %69, %73 ], [ %50, %67 ]
  %78 = getelementptr inbounds nuw i8, ptr %77, i64 4
  br label %79

79:                                               ; preds = %75, %114
  %80 = phi ptr [ %63, %75 ], [ %109, %114 ]
  %81 = phi ptr [ %63, %75 ], [ %108, %114 ]
  %82 = phi ptr [ %61, %75 ], [ %107, %114 ]
  %83 = phi ptr [ %77, %75 ], [ %106, %114 ]
  %84 = phi ptr [ %78, %75 ], [ %105, %114 ]
  %85 = phi ptr [ %76, %75 ], [ %104, %114 ]
  %86 = ptrtoint ptr %81 to i64
  %87 = ptrtoint ptr %82 to i64
  %88 = sub i64 %86, %87
  %89 = ashr exact i64 %88, 2
  %90 = icmp sgt i64 %89, -1
  call void @llvm.assume(i1 %90)
  %91 = ashr exact i64 %88, 1
  %92 = icmp samesign ugt i64 %91, 2305843009213693951
  br i1 %92, label %93, label %95

93:                                               ; preds = %79
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #14
          to label %94 unwind label %128

94:                                               ; preds = %93
  unreachable

95:                                               ; preds = %79
  %96 = icmp eq ptr %81, %82
  br i1 %96, label %103, label %97

97:                                               ; preds = %95
  %98 = shl nuw nsw i64 %88, 1
  %99 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %98) #13
          to label %100 unwind label %126

100:                                              ; preds = %97
  %101 = getelementptr inbounds nuw i32, ptr %99, i64 %91
  %102 = call i64 @llvm.umax.i64(i64 %89, i64 1)
  br label %130

103:                                              ; preds = %144, %95
  %104 = phi ptr [ %85, %95 ], [ %145, %144 ]
  %105 = phi ptr [ %84, %95 ], [ %146, %144 ]
  %106 = phi ptr [ %83, %95 ], [ %147, %144 ]
  %107 = phi ptr [ null, %95 ], [ %148, %144 ]
  %108 = phi ptr [ null, %95 ], [ %149, %144 ]
  %109 = phi ptr [ null, %95 ], [ %150, %144 ]
  %110 = icmp eq ptr %82, null
  br i1 %110, label %114, label %111

111:                                              ; preds = %103
  %112 = ptrtoint ptr %80 to i64
  %113 = sub i64 %112, %87
  call void @_ZdlPvm(ptr noundef nonnull %82, i64 noundef %113) #15
  br label %114

114:                                              ; preds = %103, %111
  %115 = icmp eq ptr %107, %108
  br i1 %115, label %260, label %79, !llvm.loop !16

116:                                              ; preds = %25
  %117 = landingpad { ptr, i32 }
          cleanup
  br label %657

118:                                              ; preds = %35, %37
  %119 = landingpad { ptr, i32 }
          cleanup
  br label %643

120:                                              ; preds = %46
  %121 = landingpad { ptr, i32 }
          cleanup
  br label %632

122:                                              ; preds = %68, %60
  %123 = phi ptr [ %61, %68 ], [ null, %60 ]
  %124 = phi ptr [ %63, %68 ], [ null, %60 ]
  %125 = landingpad { ptr, i32 }
          cleanup
  br label %266

126:                                              ; preds = %97
  %127 = landingpad { ptr, i32 }
          cleanup
  br label %266

128:                                              ; preds = %93
  %129 = landingpad { ptr, i32 }
          cleanup
  br label %266

130:                                              ; preds = %100, %144
  %131 = phi i64 [ %151, %144 ], [ 0, %100 ]
  %132 = phi ptr [ %150, %144 ], [ %101, %100 ]
  %133 = phi ptr [ %149, %144 ], [ %99, %100 ]
  %134 = phi ptr [ %148, %144 ], [ %99, %100 ]
  %135 = phi ptr [ %147, %144 ], [ %83, %100 ]
  %136 = phi ptr [ %146, %144 ], [ %84, %100 ]
  %137 = phi ptr [ %145, %144 ], [ %85, %100 ]
  %138 = getelementptr inbounds nuw i32, ptr %82, i64 %131
  %139 = load i32, ptr %138, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %5) #16
  store i64 0, ptr %5, align 8, !tbaa !18
  %140 = sext i32 %139 to i64
  invoke void @autograph_get_neighbors(ptr noundef nonnull %0, i64 noundef %140, ptr noundef %51, ptr noundef nonnull %5)
          to label %141 unwind label %153

141:                                              ; preds = %130
  %142 = load i64, ptr %5, align 8, !tbaa !18
  %143 = icmp sgt i64 %142, 0
  br i1 %143, label %155, label %144

144:                                              ; preds = %239, %141
  %145 = phi ptr [ %137, %141 ], [ %240, %239 ]
  %146 = phi ptr [ %136, %141 ], [ %241, %239 ]
  %147 = phi ptr [ %135, %141 ], [ %242, %239 ]
  %148 = phi ptr [ %134, %141 ], [ %243, %239 ]
  %149 = phi ptr [ %133, %141 ], [ %244, %239 ]
  %150 = phi ptr [ %132, %141 ], [ %245, %239 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %5) #16
  %151 = add nuw nsw i64 %131, 1
  %152 = icmp eq i64 %151, %102
  br i1 %152, label %103, label %130, !llvm.loop !19

153:                                              ; preds = %130
  %154 = landingpad { ptr, i32 }
          cleanup
  br label %249

155:                                              ; preds = %141, %239
  %156 = phi i64 [ %246, %239 ], [ 0, %141 ]
  %157 = phi ptr [ %245, %239 ], [ %132, %141 ]
  %158 = phi ptr [ %244, %239 ], [ %133, %141 ]
  %159 = phi ptr [ %243, %239 ], [ %134, %141 ]
  %160 = phi ptr [ %242, %239 ], [ %135, %141 ]
  %161 = phi ptr [ %241, %239 ], [ %136, %141 ]
  %162 = phi ptr [ %240, %239 ], [ %137, %141 ]
  %163 = getelementptr inbounds nuw i32, ptr %51, i64 %156
  %164 = load i32, ptr %163, align 4, !tbaa !13
  %165 = sext i32 %164 to i64
  %166 = getelementptr inbounds nuw i8, ptr %47, i64 %165
  %167 = load i8, ptr %166, align 1, !tbaa !15
  %168 = icmp eq i8 %167, 0
  br i1 %168, label %169, label %239

169:                                              ; preds = %155
  store i8 1, ptr %166, align 1, !tbaa !15
  %170 = icmp eq ptr %158, %157
  br i1 %170, label %172, label %171

171:                                              ; preds = %169
  store i32 %164, ptr %158, align 4, !tbaa !13
  br label %196

172:                                              ; preds = %169
  %173 = ptrtoint ptr %157 to i64
  %174 = ptrtoint ptr %159 to i64
  %175 = sub i64 %173, %174
  %176 = ashr exact i64 %175, 2
  %177 = icmp sgt i64 %176, -1
  call void @llvm.assume(i1 %177)
  %178 = icmp eq i64 %175, 9223372036854775804
  br i1 %178, label %179, label %181

179:                                              ; preds = %172
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4) #14
          to label %180 unwind label %234

180:                                              ; preds = %179
  unreachable

181:                                              ; preds = %172
  %182 = call i64 @llvm.umax.i64(i64 %176, i64 1)
  %183 = add nuw nsw i64 %182, %176
  %184 = call noundef i64 @llvm.umin.i64(i64 %183, i64 2305843009213693951)
  %185 = shl nuw nsw i64 %184, 2
  %186 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %185) #13
          to label %187 unwind label %229

187:                                              ; preds = %181
  %188 = getelementptr inbounds i8, ptr %186, i64 %175
  store i32 %164, ptr %188, align 4, !tbaa !13
  %189 = icmp sgt i64 %175, 0
  br i1 %189, label %190, label %191

190:                                              ; preds = %187
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %186, ptr align 4 %159, i64 %175, i1 false)
  br label %191

191:                                              ; preds = %190, %187
  %192 = icmp eq ptr %159, null
  br i1 %192, label %194, label %193

193:                                              ; preds = %191
  call void @_ZdlPvm(ptr noundef nonnull %159, i64 noundef %175) #15
  br label %194

194:                                              ; preds = %193, %191
  %195 = getelementptr inbounds nuw i32, ptr %186, i64 %184
  br label %196

196:                                              ; preds = %194, %171
  %197 = phi ptr [ %186, %194 ], [ %159, %171 ]
  %198 = phi ptr [ %188, %194 ], [ %158, %171 ]
  %199 = phi ptr [ %195, %194 ], [ %157, %171 ]
  %200 = getelementptr inbounds nuw i8, ptr %198, i64 4
  %201 = icmp eq ptr %161, %162
  br i1 %201, label %204, label %202

202:                                              ; preds = %196
  store i32 %164, ptr %161, align 4, !tbaa !13
  %203 = getelementptr inbounds nuw i8, ptr %161, i64 4
  br label %239

204:                                              ; preds = %196
  %205 = ptrtoint ptr %161 to i64
  %206 = ptrtoint ptr %160 to i64
  %207 = sub i64 %205, %206
  %208 = ashr exact i64 %207, 2
  %209 = icmp sgt i64 %208, -1
  call void @llvm.assume(i1 %209)
  %210 = icmp eq i64 %207, 9223372036854775804
  br i1 %210, label %211, label %213

211:                                              ; preds = %204
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4) #14
          to label %212 unwind label %234

212:                                              ; preds = %211
  unreachable

213:                                              ; preds = %204
  %214 = call i64 @llvm.umax.i64(i64 %208, i64 1)
  %215 = add nuw nsw i64 %214, %208
  %216 = call noundef i64 @llvm.umin.i64(i64 %215, i64 2305843009213693951)
  %217 = shl nuw nsw i64 %216, 2
  %218 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %217) #13
          to label %219 unwind label %229

219:                                              ; preds = %213
  %220 = getelementptr inbounds i8, ptr %218, i64 %207
  store i32 %164, ptr %220, align 4, !tbaa !13
  %221 = icmp sgt i64 %207, 0
  br i1 %221, label %222, label %223

222:                                              ; preds = %219
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %218, ptr align 4 %160, i64 %207, i1 false)
  br label %223

223:                                              ; preds = %222, %219
  %224 = icmp eq ptr %160, null
  br i1 %224, label %226, label %225

225:                                              ; preds = %223
  call void @_ZdlPvm(ptr noundef nonnull %160, i64 noundef %207) #15
  br label %226

226:                                              ; preds = %225, %223
  %227 = getelementptr inbounds nuw i8, ptr %220, i64 4
  %228 = getelementptr inbounds nuw i32, ptr %218, i64 %216
  br label %239

229:                                              ; preds = %181, %213
  %230 = phi ptr [ %162, %181 ], [ %161, %213 ]
  %231 = phi ptr [ %159, %181 ], [ %197, %213 ]
  %232 = phi ptr [ %157, %181 ], [ %199, %213 ]
  %233 = landingpad { ptr, i32 }
          cleanup
  br label %249

234:                                              ; preds = %179, %211
  %235 = phi ptr [ %161, %211 ], [ %162, %179 ]
  %236 = phi ptr [ %197, %211 ], [ %159, %179 ]
  %237 = phi ptr [ %199, %211 ], [ %157, %179 ]
  %238 = landingpad { ptr, i32 }
          cleanup
  br label %249

239:                                              ; preds = %226, %202, %155
  %240 = phi ptr [ %162, %155 ], [ %228, %226 ], [ %162, %202 ]
  %241 = phi ptr [ %161, %155 ], [ %227, %226 ], [ %203, %202 ]
  %242 = phi ptr [ %160, %155 ], [ %218, %226 ], [ %160, %202 ]
  %243 = phi ptr [ %159, %155 ], [ %197, %226 ], [ %197, %202 ]
  %244 = phi ptr [ %158, %155 ], [ %200, %226 ], [ %200, %202 ]
  %245 = phi ptr [ %157, %155 ], [ %199, %226 ], [ %199, %202 ]
  %246 = add nuw nsw i64 %156, 1
  %247 = load i64, ptr %5, align 8, !tbaa !18
  %248 = icmp slt i64 %246, %247
  br i1 %248, label %155, label %144, !llvm.loop !20

249:                                              ; preds = %229, %234, %153
  %250 = phi ptr [ %137, %153 ], [ %230, %229 ], [ %235, %234 ]
  %251 = phi ptr [ %135, %153 ], [ %160, %229 ], [ %160, %234 ]
  %252 = phi ptr [ %134, %153 ], [ %231, %229 ], [ %236, %234 ]
  %253 = phi ptr [ %132, %153 ], [ %232, %229 ], [ %237, %234 ]
  %254 = phi { ptr, i32 } [ %154, %153 ], [ %233, %229 ], [ %238, %234 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %5) #16
  %255 = icmp eq ptr %252, null
  br i1 %255, label %266, label %256

256:                                              ; preds = %249
  %257 = ptrtoint ptr %253 to i64
  %258 = ptrtoint ptr %252 to i64
  %259 = sub i64 %257, %258
  call void @_ZdlPvm(ptr noundef nonnull %252, i64 noundef %259) #15
  br label %266

260:                                              ; preds = %114
  %261 = icmp eq ptr %107, null
  br i1 %261, label %527, label %262

262:                                              ; preds = %260
  %263 = ptrtoint ptr %109 to i64
  %264 = ptrtoint ptr %107 to i64
  %265 = sub i64 %263, %264
  call void @_ZdlPvm(ptr noundef nonnull %107, i64 noundef %265) #15
  br label %527

266:                                              ; preds = %126, %128, %256, %249, %122
  %267 = phi ptr [ %49, %122 ], [ %250, %249 ], [ %250, %256 ], [ %85, %126 ], [ %85, %128 ]
  %268 = phi ptr [ %50, %122 ], [ %251, %249 ], [ %251, %256 ], [ %83, %126 ], [ %83, %128 ]
  %269 = phi ptr [ %123, %122 ], [ %82, %249 ], [ %82, %256 ], [ %82, %126 ], [ %82, %128 ]
  %270 = phi ptr [ %124, %122 ], [ %80, %249 ], [ %80, %256 ], [ %80, %126 ], [ %80, %128 ]
  %271 = phi { ptr, i32 } [ %125, %122 ], [ %254, %249 ], [ %254, %256 ], [ %127, %126 ], [ %129, %128 ]
  %272 = icmp eq ptr %269, null
  br i1 %272, label %632, label %273

273:                                              ; preds = %266
  %274 = ptrtoint ptr %270 to i64
  %275 = ptrtoint ptr %269 to i64
  %276 = sub i64 %274, %275
  call void @_ZdlPvm(ptr noundef nonnull %269, i64 noundef %276) #15
  br label %632

277:                                              ; preds = %58, %505
  %278 = phi i64 [ 0, %58 ], [ %509, %505 ]
  %279 = phi ptr [ %50, %58 ], [ %508, %505 ]
  %280 = phi ptr [ %50, %58 ], [ %507, %505 ]
  %281 = phi ptr [ %49, %58 ], [ %506, %505 ]
  %282 = getelementptr inbounds nuw i8, ptr %47, i64 %278
  %283 = load i8, ptr %282, align 1, !tbaa !15
  %284 = icmp eq i8 %283, 0
  br i1 %284, label %285, label %505

285:                                              ; preds = %277
  %286 = invoke noalias noundef nonnull dereferenceable(4) ptr @_Znwm(i64 noundef 4) #13
          to label %287 unwind label %358

287:                                              ; preds = %285
  %288 = trunc nuw nsw i64 %278 to i32
  store i32 %288, ptr %286, align 4, !tbaa !13
  %289 = getelementptr inbounds nuw i8, ptr %286, i64 4
  store i8 1, ptr %282, align 1, !tbaa !15
  %290 = icmp eq ptr %280, %281
  br i1 %290, label %292, label %291

291:                                              ; preds = %287
  store i32 %288, ptr %280, align 4, !tbaa !13
  br label %316

292:                                              ; preds = %287
  %293 = ptrtoint ptr %280 to i64
  %294 = ptrtoint ptr %279 to i64
  %295 = sub i64 %293, %294
  %296 = ashr exact i64 %295, 2
  %297 = icmp sgt i64 %296, -1
  call void @llvm.assume(i1 %297)
  %298 = icmp eq i64 %295, 9223372036854775804
  br i1 %298, label %299, label %301

299:                                              ; preds = %292
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4) #14
          to label %300 unwind label %363

300:                                              ; preds = %299
  unreachable

301:                                              ; preds = %292
  %302 = call i64 @llvm.umax.i64(i64 %296, i64 1)
  %303 = add nuw nsw i64 %302, %296
  %304 = call noundef i64 @llvm.umin.i64(i64 %303, i64 2305843009213693951)
  %305 = shl nuw nsw i64 %304, 2
  %306 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %305) #13
          to label %307 unwind label %358

307:                                              ; preds = %301
  %308 = getelementptr inbounds i8, ptr %306, i64 %295
  store i32 %288, ptr %308, align 4, !tbaa !13
  %309 = icmp sgt i64 %295, 0
  br i1 %309, label %310, label %311

310:                                              ; preds = %307
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %306, ptr align 4 %279, i64 %295, i1 false)
  br label %311

311:                                              ; preds = %310, %307
  %312 = icmp eq ptr %279, null
  br i1 %312, label %314, label %313

313:                                              ; preds = %311
  call void @_ZdlPvm(ptr noundef nonnull %279, i64 noundef %295) #15
  br label %314

314:                                              ; preds = %313, %311
  %315 = getelementptr inbounds nuw i32, ptr %306, i64 %304
  br label %316

316:                                              ; preds = %291, %314
  %317 = phi ptr [ %315, %314 ], [ %281, %291 ]
  %318 = phi ptr [ %308, %314 ], [ %280, %291 ]
  %319 = phi ptr [ %306, %314 ], [ %279, %291 ]
  %320 = getelementptr inbounds nuw i8, ptr %318, i64 4
  br label %321

321:                                              ; preds = %316, %356
  %322 = phi ptr [ %289, %316 ], [ %351, %356 ]
  %323 = phi ptr [ %289, %316 ], [ %350, %356 ]
  %324 = phi ptr [ %286, %316 ], [ %349, %356 ]
  %325 = phi ptr [ %319, %316 ], [ %348, %356 ]
  %326 = phi ptr [ %320, %316 ], [ %347, %356 ]
  %327 = phi ptr [ %317, %316 ], [ %346, %356 ]
  %328 = ptrtoint ptr %323 to i64
  %329 = ptrtoint ptr %324 to i64
  %330 = sub i64 %328, %329
  %331 = ashr exact i64 %330, 2
  %332 = icmp sgt i64 %331, -1
  call void @llvm.assume(i1 %332)
  %333 = ashr exact i64 %330, 1
  %334 = icmp samesign ugt i64 %333, 2305843009213693951
  br i1 %334, label %335, label %337

335:                                              ; preds = %321
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #14
          to label %336 unwind label %367

336:                                              ; preds = %335
  unreachable

337:                                              ; preds = %321
  %338 = icmp eq ptr %323, %324
  br i1 %338, label %345, label %339

339:                                              ; preds = %337
  %340 = shl nuw nsw i64 %330, 1
  %341 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %340) #13
          to label %342 unwind label %365

342:                                              ; preds = %339
  %343 = getelementptr inbounds nuw i32, ptr %341, i64 %333
  %344 = call i64 @llvm.umax.i64(i64 %331, i64 1)
  br label %369

345:                                              ; preds = %383, %337
  %346 = phi ptr [ %327, %337 ], [ %384, %383 ]
  %347 = phi ptr [ %326, %337 ], [ %385, %383 ]
  %348 = phi ptr [ %325, %337 ], [ %386, %383 ]
  %349 = phi ptr [ null, %337 ], [ %387, %383 ]
  %350 = phi ptr [ null, %337 ], [ %388, %383 ]
  %351 = phi ptr [ null, %337 ], [ %389, %383 ]
  %352 = icmp eq ptr %324, null
  br i1 %352, label %356, label %353

353:                                              ; preds = %345
  %354 = ptrtoint ptr %322 to i64
  %355 = sub i64 %354, %329
  call void @_ZdlPvm(ptr noundef nonnull %324, i64 noundef %355) #15
  br label %356

356:                                              ; preds = %345, %353
  %357 = icmp eq ptr %349, %350
  br i1 %357, label %499, label %321, !llvm.loop !21

358:                                              ; preds = %285, %301
  %359 = phi ptr [ %281, %285 ], [ %280, %301 ]
  %360 = phi ptr [ null, %285 ], [ %286, %301 ]
  %361 = phi ptr [ null, %285 ], [ %289, %301 ]
  %362 = landingpad { ptr, i32 }
          cleanup
  br label %511

363:                                              ; preds = %299
  %364 = landingpad { ptr, i32 }
          cleanup
  br label %518

365:                                              ; preds = %339
  %366 = landingpad { ptr, i32 }
          cleanup
  br label %511

367:                                              ; preds = %335
  %368 = landingpad { ptr, i32 }
          cleanup
  br label %511

369:                                              ; preds = %342, %383
  %370 = phi i64 [ %390, %383 ], [ 0, %342 ]
  %371 = phi ptr [ %389, %383 ], [ %343, %342 ]
  %372 = phi ptr [ %388, %383 ], [ %341, %342 ]
  %373 = phi ptr [ %387, %383 ], [ %341, %342 ]
  %374 = phi ptr [ %386, %383 ], [ %325, %342 ]
  %375 = phi ptr [ %385, %383 ], [ %326, %342 ]
  %376 = phi ptr [ %384, %383 ], [ %327, %342 ]
  %377 = getelementptr inbounds nuw i32, ptr %324, i64 %370
  %378 = load i32, ptr %377, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %6) #16
  store i64 0, ptr %6, align 8, !tbaa !18
  %379 = sext i32 %378 to i64
  invoke void @autograph_get_neighbors(ptr noundef nonnull %0, i64 noundef %379, ptr noundef %51, ptr noundef nonnull %6)
          to label %380 unwind label %392

380:                                              ; preds = %369
  %381 = load i64, ptr %6, align 8, !tbaa !18
  %382 = icmp sgt i64 %381, 0
  br i1 %382, label %394, label %383

383:                                              ; preds = %478, %380
  %384 = phi ptr [ %376, %380 ], [ %479, %478 ]
  %385 = phi ptr [ %375, %380 ], [ %480, %478 ]
  %386 = phi ptr [ %374, %380 ], [ %481, %478 ]
  %387 = phi ptr [ %373, %380 ], [ %482, %478 ]
  %388 = phi ptr [ %372, %380 ], [ %483, %478 ]
  %389 = phi ptr [ %371, %380 ], [ %484, %478 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6) #16
  %390 = add nuw nsw i64 %370, 1
  %391 = icmp eq i64 %390, %344
  br i1 %391, label %345, label %369, !llvm.loop !22

392:                                              ; preds = %369
  %393 = landingpad { ptr, i32 }
          cleanup
  br label %488

394:                                              ; preds = %380, %478
  %395 = phi i64 [ %485, %478 ], [ 0, %380 ]
  %396 = phi ptr [ %484, %478 ], [ %371, %380 ]
  %397 = phi ptr [ %483, %478 ], [ %372, %380 ]
  %398 = phi ptr [ %482, %478 ], [ %373, %380 ]
  %399 = phi ptr [ %481, %478 ], [ %374, %380 ]
  %400 = phi ptr [ %480, %478 ], [ %375, %380 ]
  %401 = phi ptr [ %479, %478 ], [ %376, %380 ]
  %402 = getelementptr inbounds nuw i32, ptr %51, i64 %395
  %403 = load i32, ptr %402, align 4, !tbaa !13
  %404 = sext i32 %403 to i64
  %405 = getelementptr inbounds nuw i8, ptr %47, i64 %404
  %406 = load i8, ptr %405, align 1, !tbaa !15
  %407 = icmp eq i8 %406, 0
  br i1 %407, label %408, label %478

408:                                              ; preds = %394
  store i8 1, ptr %405, align 1, !tbaa !15
  %409 = icmp eq ptr %397, %396
  br i1 %409, label %411, label %410

410:                                              ; preds = %408
  store i32 %403, ptr %397, align 4, !tbaa !13
  br label %435

411:                                              ; preds = %408
  %412 = ptrtoint ptr %396 to i64
  %413 = ptrtoint ptr %398 to i64
  %414 = sub i64 %412, %413
  %415 = ashr exact i64 %414, 2
  %416 = icmp sgt i64 %415, -1
  call void @llvm.assume(i1 %416)
  %417 = icmp eq i64 %414, 9223372036854775804
  br i1 %417, label %418, label %420

418:                                              ; preds = %411
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4) #14
          to label %419 unwind label %473

419:                                              ; preds = %418
  unreachable

420:                                              ; preds = %411
  %421 = call i64 @llvm.umax.i64(i64 %415, i64 1)
  %422 = add nuw nsw i64 %421, %415
  %423 = call noundef i64 @llvm.umin.i64(i64 %422, i64 2305843009213693951)
  %424 = shl nuw nsw i64 %423, 2
  %425 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %424) #13
          to label %426 unwind label %468

426:                                              ; preds = %420
  %427 = getelementptr inbounds i8, ptr %425, i64 %414
  store i32 %403, ptr %427, align 4, !tbaa !13
  %428 = icmp sgt i64 %414, 0
  br i1 %428, label %429, label %430

429:                                              ; preds = %426
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %425, ptr align 4 %398, i64 %414, i1 false)
  br label %430

430:                                              ; preds = %429, %426
  %431 = icmp eq ptr %398, null
  br i1 %431, label %433, label %432

432:                                              ; preds = %430
  call void @_ZdlPvm(ptr noundef nonnull %398, i64 noundef %414) #15
  br label %433

433:                                              ; preds = %432, %430
  %434 = getelementptr inbounds nuw i32, ptr %425, i64 %423
  br label %435

435:                                              ; preds = %433, %410
  %436 = phi ptr [ %425, %433 ], [ %398, %410 ]
  %437 = phi ptr [ %427, %433 ], [ %397, %410 ]
  %438 = phi ptr [ %434, %433 ], [ %396, %410 ]
  %439 = getelementptr inbounds nuw i8, ptr %437, i64 4
  %440 = icmp eq ptr %400, %401
  br i1 %440, label %443, label %441

441:                                              ; preds = %435
  store i32 %403, ptr %400, align 4, !tbaa !13
  %442 = getelementptr inbounds nuw i8, ptr %400, i64 4
  br label %478

443:                                              ; preds = %435
  %444 = ptrtoint ptr %400 to i64
  %445 = ptrtoint ptr %399 to i64
  %446 = sub i64 %444, %445
  %447 = ashr exact i64 %446, 2
  %448 = icmp sgt i64 %447, -1
  call void @llvm.assume(i1 %448)
  %449 = icmp eq i64 %446, 9223372036854775804
  br i1 %449, label %450, label %452

450:                                              ; preds = %443
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4) #14
          to label %451 unwind label %473

451:                                              ; preds = %450
  unreachable

452:                                              ; preds = %443
  %453 = call i64 @llvm.umax.i64(i64 %447, i64 1)
  %454 = add nuw nsw i64 %453, %447
  %455 = call noundef i64 @llvm.umin.i64(i64 %454, i64 2305843009213693951)
  %456 = shl nuw nsw i64 %455, 2
  %457 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %456) #13
          to label %458 unwind label %468

458:                                              ; preds = %452
  %459 = getelementptr inbounds i8, ptr %457, i64 %446
  store i32 %403, ptr %459, align 4, !tbaa !13
  %460 = icmp sgt i64 %446, 0
  br i1 %460, label %461, label %462

461:                                              ; preds = %458
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %457, ptr align 4 %399, i64 %446, i1 false)
  br label %462

462:                                              ; preds = %461, %458
  %463 = icmp eq ptr %399, null
  br i1 %463, label %465, label %464

464:                                              ; preds = %462
  call void @_ZdlPvm(ptr noundef nonnull %399, i64 noundef %446) #15
  br label %465

465:                                              ; preds = %464, %462
  %466 = getelementptr inbounds nuw i8, ptr %459, i64 4
  %467 = getelementptr inbounds nuw i32, ptr %457, i64 %455
  br label %478

468:                                              ; preds = %420, %452
  %469 = phi ptr [ %401, %420 ], [ %400, %452 ]
  %470 = phi ptr [ %398, %420 ], [ %436, %452 ]
  %471 = phi ptr [ %396, %420 ], [ %438, %452 ]
  %472 = landingpad { ptr, i32 }
          cleanup
  br label %488

473:                                              ; preds = %418, %450
  %474 = phi ptr [ %400, %450 ], [ %401, %418 ]
  %475 = phi ptr [ %436, %450 ], [ %398, %418 ]
  %476 = phi ptr [ %438, %450 ], [ %396, %418 ]
  %477 = landingpad { ptr, i32 }
          cleanup
  br label %488

478:                                              ; preds = %465, %441, %394
  %479 = phi ptr [ %401, %394 ], [ %467, %465 ], [ %401, %441 ]
  %480 = phi ptr [ %400, %394 ], [ %466, %465 ], [ %442, %441 ]
  %481 = phi ptr [ %399, %394 ], [ %457, %465 ], [ %399, %441 ]
  %482 = phi ptr [ %398, %394 ], [ %436, %465 ], [ %436, %441 ]
  %483 = phi ptr [ %397, %394 ], [ %439, %465 ], [ %439, %441 ]
  %484 = phi ptr [ %396, %394 ], [ %438, %465 ], [ %438, %441 ]
  %485 = add nuw nsw i64 %395, 1
  %486 = load i64, ptr %6, align 8, !tbaa !18
  %487 = icmp slt i64 %485, %486
  br i1 %487, label %394, label %383, !llvm.loop !23

488:                                              ; preds = %468, %473, %392
  %489 = phi ptr [ %376, %392 ], [ %469, %468 ], [ %474, %473 ]
  %490 = phi ptr [ %374, %392 ], [ %399, %468 ], [ %399, %473 ]
  %491 = phi ptr [ %373, %392 ], [ %470, %468 ], [ %475, %473 ]
  %492 = phi ptr [ %371, %392 ], [ %471, %468 ], [ %476, %473 ]
  %493 = phi { ptr, i32 } [ %393, %392 ], [ %472, %468 ], [ %477, %473 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %6) #16
  %494 = icmp eq ptr %491, null
  br i1 %494, label %511, label %495

495:                                              ; preds = %488
  %496 = ptrtoint ptr %492 to i64
  %497 = ptrtoint ptr %491 to i64
  %498 = sub i64 %496, %497
  call void @_ZdlPvm(ptr noundef nonnull %491, i64 noundef %498) #15
  br label %511

499:                                              ; preds = %356
  %500 = icmp eq ptr %349, null
  br i1 %500, label %505, label %501

501:                                              ; preds = %499
  %502 = ptrtoint ptr %351 to i64
  %503 = ptrtoint ptr %349 to i64
  %504 = sub i64 %502, %503
  call void @_ZdlPvm(ptr noundef nonnull %349, i64 noundef %504) #15
  br label %505

505:                                              ; preds = %501, %499, %277
  %506 = phi ptr [ %281, %277 ], [ %346, %499 ], [ %346, %501 ]
  %507 = phi ptr [ %280, %277 ], [ %347, %499 ], [ %347, %501 ]
  %508 = phi ptr [ %279, %277 ], [ %348, %499 ], [ %348, %501 ]
  %509 = add nuw nsw i64 %278, 1
  %510 = icmp eq i64 %509, %59
  br i1 %510, label %527, label %277, !llvm.loop !24

511:                                              ; preds = %365, %367, %358, %495, %488
  %512 = phi ptr [ %489, %488 ], [ %489, %495 ], [ %359, %358 ], [ %327, %365 ], [ %327, %367 ]
  %513 = phi ptr [ %490, %488 ], [ %490, %495 ], [ %279, %358 ], [ %325, %365 ], [ %325, %367 ]
  %514 = phi ptr [ %324, %488 ], [ %324, %495 ], [ %360, %358 ], [ %324, %365 ], [ %324, %367 ]
  %515 = phi ptr [ %322, %488 ], [ %322, %495 ], [ %361, %358 ], [ %322, %365 ], [ %322, %367 ]
  %516 = phi { ptr, i32 } [ %493, %488 ], [ %493, %495 ], [ %362, %358 ], [ %366, %365 ], [ %368, %367 ]
  %517 = icmp eq ptr %514, null
  br i1 %517, label %632, label %518

518:                                              ; preds = %363, %511
  %519 = phi { ptr, i32 } [ %364, %363 ], [ %516, %511 ]
  %520 = phi ptr [ %289, %363 ], [ %515, %511 ]
  %521 = phi ptr [ %286, %363 ], [ %514, %511 ]
  %522 = phi ptr [ %279, %363 ], [ %513, %511 ]
  %523 = phi ptr [ %280, %363 ], [ %512, %511 ]
  %524 = ptrtoint ptr %520 to i64
  %525 = ptrtoint ptr %521 to i64
  %526 = sub i64 %524, %525
  call void @_ZdlPvm(ptr noundef nonnull %521, i64 noundef %526) #15
  br label %632

527:                                              ; preds = %505, %56, %262, %260
  %528 = phi ptr [ %104, %260 ], [ %104, %262 ], [ %49, %56 ], [ %506, %505 ]
  %529 = phi ptr [ %105, %260 ], [ %105, %262 ], [ %50, %56 ], [ %507, %505 ]
  %530 = phi ptr [ %106, %260 ], [ %106, %262 ], [ %50, %56 ], [ %508, %505 ]
  %531 = invoke double @omp_get_wtime()
          to label %532 unwind label %539

532:                                              ; preds = %527
  %533 = icmp eq ptr %530, %529
  br i1 %533, label %534, label %541

534:                                              ; preds = %532
  store ptr null, ptr %2, align 8, !tbaa !25
  %535 = ptrtoint ptr %529 to i64
  %536 = ptrtoint ptr %530 to i64
  %537 = sub i64 %535, %536
  %538 = ashr exact i64 %537, 2
  br label %608

539:                                              ; preds = %527
  %540 = landingpad { ptr, i32 }
          cleanup
  br label %632

541:                                              ; preds = %532
  %542 = ptrtoint ptr %529 to i64
  %543 = ptrtoint ptr %530 to i64
  %544 = sub i64 %542, %543
  %545 = ashr exact i64 %544, 2
  %546 = icmp sgt i64 %545, -1
  call void @llvm.assume(i1 %546)
  %547 = call noalias ptr @malloc(i64 noundef %544) #17
  %548 = icmp eq ptr %547, null
  br i1 %548, label %614, label %549

549:                                              ; preds = %541
  %550 = ptrtoint ptr %547 to i64
  %551 = call i64 @llvm.umax.i64(i64 %545, i64 1)
  %552 = icmp samesign ult i64 %545, 8
  %553 = sub i64 %550, %543
  %554 = icmp ult i64 %553, 32
  %555 = or i1 %552, %554
  br i1 %555, label %570, label %556

556:                                              ; preds = %549
  %557 = and i64 %551, 9223372036854775800
  br label %558

558:                                              ; preds = %558, %556
  %559 = phi i64 [ 0, %556 ], [ %566, %558 ]
  %560 = getelementptr inbounds nuw i32, ptr %530, i64 %559
  %561 = getelementptr inbounds nuw i8, ptr %560, i64 16
  %562 = load <4 x i32>, ptr %560, align 4, !tbaa !13
  %563 = load <4 x i32>, ptr %561, align 4, !tbaa !13
  %564 = getelementptr inbounds nuw i32, ptr %547, i64 %559
  %565 = getelementptr inbounds nuw i8, ptr %564, i64 16
  store <4 x i32> %562, ptr %564, align 4, !tbaa !13
  store <4 x i32> %563, ptr %565, align 4, !tbaa !13
  %566 = add nuw i64 %559, 8
  %567 = icmp eq i64 %566, %557
  br i1 %567, label %568, label %558, !llvm.loop !26

568:                                              ; preds = %558
  %569 = icmp eq i64 %545, %557
  br i1 %569, label %587, label %570

570:                                              ; preds = %549, %568
  %571 = phi i64 [ 0, %549 ], [ %557, %568 ]
  %572 = and i64 %551, 3
  %573 = icmp eq i64 %572, 0
  br i1 %573, label %583, label %574

574:                                              ; preds = %570, %574
  %575 = phi i64 [ %580, %574 ], [ %571, %570 ]
  %576 = phi i64 [ %581, %574 ], [ 0, %570 ]
  %577 = getelementptr inbounds nuw i32, ptr %530, i64 %575
  %578 = load i32, ptr %577, align 4, !tbaa !13
  %579 = getelementptr inbounds nuw i32, ptr %547, i64 %575
  store i32 %578, ptr %579, align 4, !tbaa !13
  %580 = add nuw nsw i64 %575, 1
  %581 = add i64 %576, 1
  %582 = icmp eq i64 %581, %572
  br i1 %582, label %583, label %574, !llvm.loop !29

583:                                              ; preds = %574, %570
  %584 = phi i64 [ %571, %570 ], [ %580, %574 ]
  %585 = sub nsw i64 %571, %551
  %586 = icmp ugt i64 %585, -4
  br i1 %586, label %587, label %589

587:                                              ; preds = %583, %589, %568
  store ptr %547, ptr %2, align 8, !tbaa !25
  %588 = trunc i64 %545 to i32
  br label %608

589:                                              ; preds = %583, %589
  %590 = phi i64 [ %606, %589 ], [ %584, %583 ]
  %591 = getelementptr inbounds nuw i32, ptr %530, i64 %590
  %592 = load i32, ptr %591, align 4, !tbaa !13
  %593 = getelementptr inbounds nuw i32, ptr %547, i64 %590
  store i32 %592, ptr %593, align 4, !tbaa !13
  %594 = add nuw nsw i64 %590, 1
  %595 = getelementptr inbounds nuw i32, ptr %530, i64 %594
  %596 = load i32, ptr %595, align 4, !tbaa !13
  %597 = getelementptr inbounds nuw i32, ptr %547, i64 %594
  store i32 %596, ptr %597, align 4, !tbaa !13
  %598 = add nuw nsw i64 %590, 2
  %599 = getelementptr inbounds nuw i32, ptr %530, i64 %598
  %600 = load i32, ptr %599, align 4, !tbaa !13
  %601 = getelementptr inbounds nuw i32, ptr %547, i64 %598
  store i32 %600, ptr %601, align 4, !tbaa !13
  %602 = add nuw nsw i64 %590, 3
  %603 = getelementptr inbounds nuw i32, ptr %530, i64 %602
  %604 = load i32, ptr %603, align 4, !tbaa !13
  %605 = getelementptr inbounds nuw i32, ptr %547, i64 %602
  store i32 %604, ptr %605, align 4, !tbaa !13
  %606 = add nuw nsw i64 %590, 4
  %607 = icmp eq i64 %545, %606
  br i1 %607, label %587, label %589, !llvm.loop !31

608:                                              ; preds = %587, %534
  %609 = phi i64 [ %545, %587 ], [ %538, %534 ]
  %610 = phi i32 [ %588, %587 ], [ 0, %534 ]
  store i32 %610, ptr %3, align 4, !tbaa !13
  %611 = icmp sgt i64 %609, -1
  call void @llvm.assume(i1 %611)
  %612 = fsub double %531, %53
  %613 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i32 noundef %1, i64 noundef %609, i64 noundef %13, double noundef %612)
  br label %614

614:                                              ; preds = %541, %608
  %615 = phi i32 [ 0, %608 ], [ 4, %541 ]
  %616 = icmp eq ptr %51, null
  br i1 %616, label %621, label %617

617:                                              ; preds = %614
  %618 = ptrtoint ptr %52 to i64
  %619 = ptrtoint ptr %51 to i64
  %620 = sub i64 %618, %619
  call void @_ZdlPvm(ptr noundef nonnull %51, i64 noundef %620) #15
  br label %621

621:                                              ; preds = %614, %617
  %622 = icmp eq ptr %530, null
  br i1 %622, label %627, label %623

623:                                              ; preds = %621
  %624 = ptrtoint ptr %528 to i64
  %625 = ptrtoint ptr %530 to i64
  %626 = sub i64 %624, %625
  call void @_ZdlPvm(ptr noundef nonnull %530, i64 noundef %626) #15
  br label %627

627:                                              ; preds = %621, %623
  %628 = icmp eq ptr %47, null
  br i1 %628, label %665, label %629

629:                                              ; preds = %627
  %630 = ptrtoint ptr %47 to i64
  %631 = sub i64 %48, %630
  call void @_ZdlPvm(ptr noundef nonnull %47, i64 noundef %631) #15
  br label %665

632:                                              ; preds = %518, %511, %273, %266, %539, %120
  %633 = phi ptr [ %528, %539 ], [ %49, %120 ], [ %267, %266 ], [ %267, %273 ], [ %512, %511 ], [ %523, %518 ]
  %634 = phi ptr [ %530, %539 ], [ %50, %120 ], [ %268, %266 ], [ %268, %273 ], [ %513, %511 ], [ %522, %518 ]
  %635 = phi { ptr, i32 } [ %540, %539 ], [ %121, %120 ], [ %271, %266 ], [ %271, %273 ], [ %516, %511 ], [ %519, %518 ]
  %636 = icmp eq ptr %51, null
  br i1 %636, label %641, label %637

637:                                              ; preds = %632
  %638 = ptrtoint ptr %52 to i64
  %639 = ptrtoint ptr %51 to i64
  %640 = sub i64 %638, %639
  call void @_ZdlPvm(ptr noundef nonnull %51, i64 noundef %640) #15
  br label %641

641:                                              ; preds = %632, %637
  %642 = icmp eq ptr %634, null
  br i1 %642, label %652, label %643

643:                                              ; preds = %118, %641
  %644 = phi { ptr, i32 } [ %119, %118 ], [ %635, %641 ]
  %645 = phi ptr [ %31, %118 ], [ %634, %641 ]
  %646 = phi ptr [ %33, %118 ], [ %633, %641 ]
  %647 = phi ptr [ %26, %118 ], [ %47, %641 ]
  %648 = phi i64 [ %28, %118 ], [ %48, %641 ]
  %649 = ptrtoint ptr %646 to i64
  %650 = ptrtoint ptr %645 to i64
  %651 = sub i64 %649, %650
  call void @_ZdlPvm(ptr noundef nonnull %645, i64 noundef %651) #15
  br label %652

652:                                              ; preds = %641, %643
  %653 = phi ptr [ %47, %641 ], [ %647, %643 ]
  %654 = phi i64 [ %48, %641 ], [ %648, %643 ]
  %655 = phi { ptr, i32 } [ %635, %641 ], [ %644, %643 ]
  %656 = icmp eq ptr %653, null
  br i1 %656, label %663, label %657

657:                                              ; preds = %116, %652
  %658 = phi { ptr, i32 } [ %117, %116 ], [ %655, %652 ]
  %659 = phi i64 [ %28, %116 ], [ %654, %652 ]
  %660 = phi ptr [ %26, %116 ], [ %653, %652 ]
  %661 = ptrtoint ptr %660 to i64
  %662 = sub i64 %659, %661
  call void @_ZdlPvm(ptr noundef nonnull %660, i64 noundef %662) #15
  br label %663

663:                                              ; preds = %657, %652
  %664 = phi { ptr, i32 } [ %658, %657 ], [ %655, %652 ]
  resume { ptr, i32 } %664

665:                                              ; preds = %20, %12, %627, %629, %4
  %666 = phi i32 [ 1, %4 ], [ 3, %20 ], [ 2, %12 ], [ %615, %627 ], [ %615, %629 ]
  ret i32 %666
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local double @omp_get_wtime() local_unnamed_addr #3

declare dso_local void @autograph_get_neighbors(ptr noundef, i64 noundef, ptr noundef, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

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
attributes #14 = { cold noreturn }
attributes #15 = { builtin nounwind }
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
!18 = !{!5, !5, i64 0}
!19 = distinct !{!19, !17}
!20 = distinct !{!20, !17}
!21 = distinct !{!21, !17}
!22 = distinct !{!22, !17}
!23 = distinct !{!23, !17}
!24 = distinct !{!24, !17}
!25 = !{!10, !10, i64 0}
!26 = distinct !{!26, !17, !27, !28}
!27 = !{!"llvm.loop.isvectorized", i32 1}
!28 = !{!"llvm.loop.unroll.runtime.disable"}
!29 = distinct !{!29, !30}
!30 = !{!"llvm.loop.unroll.disable"}
!31 = distinct !{!31, !17, !27}
