; ModuleID = 'dijkstra_runtime.cpp'
source_filename = "dijkstra_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.Graph = type { i64, i64, ptr, ptr, ptr }
%"struct.std::pair" = type <{ i64, i32, [4 x i8] }>

$_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_ = comdat any

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [48 x i8] c"[dijkstra_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.1 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.2 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @dijkstra_runtime_src(ptr noundef readonly %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %270, label %6

6:                                                ; preds = %4
  %7 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %8 = load ptr, ptr %7, align 8, !tbaa !3
  %9 = icmp eq ptr %8, null
  br i1 %9, label %270, label %10

10:                                               ; preds = %6
  %11 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %12 = load ptr, ptr %11, align 8, !tbaa !9
  %13 = icmp ne ptr %12, null
  %14 = icmp ne ptr %2, null
  %15 = and i1 %14, %13
  %16 = icmp ne ptr %3, null
  %17 = and i1 %16, %15
  br i1 %17, label %18, label %270

18:                                               ; preds = %10
  %19 = load i64, ptr %0, align 8, !tbaa !10
  %20 = icmp slt i64 %19, 1
  br i1 %20, label %21, label %22

21:                                               ; preds = %18
  store ptr null, ptr %2, align 8, !tbaa !11
  store i32 0, ptr %3, align 4, !tbaa !12
  br label %270

22:                                               ; preds = %18
  %23 = icmp slt i32 %1, 0
  br i1 %23, label %27, label %24

24:                                               ; preds = %22
  %25 = trunc i64 %19 to i32
  %26 = icmp sgt i32 %25, %1
  br i1 %26, label %30, label %27

27:                                               ; preds = %24, %22
  %28 = load ptr, ptr @stderr, align 8, !tbaa !11
  %29 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %28, ptr noundef nonnull @.str, i32 noundef %1, i64 noundef %19) #10
  br label %270

30:                                               ; preds = %24
  %31 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 4
  %32 = load ptr, ptr %31, align 8, !tbaa !14
  %33 = icmp ugt i64 %19, 1152921504606846975
  br i1 %33, label %34, label %35

34:                                               ; preds = %30
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #11
  unreachable

35:                                               ; preds = %30
  %36 = shl nuw nsw i64 %19, 3
  %37 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %36) #12
  %38 = icmp ult i64 %19, 4
  br i1 %38, label %53, label %39

39:                                               ; preds = %35
  %40 = and i64 %19, 1152921504606846972
  %41 = and i64 %19, 3
  %42 = shl nuw nsw i64 %40, 3
  %43 = getelementptr i8, ptr %37, i64 %42
  br label %44

44:                                               ; preds = %44, %39
  %45 = phi i64 [ 0, %39 ], [ %49, %44 ]
  %46 = shl i64 %45, 3
  %47 = getelementptr i8, ptr %37, i64 %46
  %48 = getelementptr i64, ptr %47, i64 2
  store <2 x i64> <i64 2305843009213693951, i64 2305843009213693951>, ptr %47, align 8, !tbaa !15
  store <2 x i64> <i64 2305843009213693951, i64 2305843009213693951>, ptr %48, align 8, !tbaa !15
  %49 = add nuw i64 %45, 4
  %50 = icmp eq i64 %49, %40
  br i1 %50, label %51, label %44, !llvm.loop !17

51:                                               ; preds = %44
  %52 = icmp eq i64 %19, %40
  br i1 %52, label %62, label %53

53:                                               ; preds = %35, %51
  %54 = phi i64 [ %19, %35 ], [ %41, %51 ]
  %55 = phi ptr [ %37, %35 ], [ %43, %51 ]
  br label %56

56:                                               ; preds = %53, %56
  %57 = phi i64 [ %59, %56 ], [ %54, %53 ]
  %58 = phi ptr [ %60, %56 ], [ %55, %53 ]
  %59 = add i64 %57, -1
  store i64 2305843009213693951, ptr %58, align 8, !tbaa !15
  %60 = getelementptr inbounds i64, ptr %58, i64 1
  %61 = icmp eq i64 %59, 0
  br i1 %61, label %62, label %56, !llvm.loop !21

62:                                               ; preds = %56, %51
  %63 = zext nneg i32 %1 to i64
  %64 = getelementptr inbounds i64, ptr %37, i64 %63
  store i64 0, ptr %64, align 8, !tbaa !15
  %65 = invoke noalias noundef nonnull dereferenceable(16) ptr @_Znwm(i64 noundef 16) #12
          to label %66 unwind label %95

66:                                               ; preds = %62
  %67 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 0, i32 1
  %68 = getelementptr inbounds %"struct.std::pair", ptr %65, i64 1
  store i64 0, ptr %65, align 8, !tbaa !22
  store i32 %1, ptr %67, align 8, !tbaa !24
  %69 = icmp eq ptr %32, null
  br label %70

70:                                               ; preds = %66, %206
  %71 = phi ptr [ %65, %66 ], [ %209, %206 ]
  %72 = phi ptr [ %68, %66 ], [ %208, %206 ]
  %73 = phi ptr [ %68, %66 ], [ %207, %206 ]
  %74 = load i64, ptr %71, align 8
  %75 = getelementptr inbounds i8, ptr %71, i64 8
  %76 = load i32, ptr %75, align 8
  %77 = ptrtoint ptr %72 to i64
  %78 = ptrtoint ptr %71 to i64
  %79 = sub i64 %77, %78
  %80 = icmp sgt i64 %79, 16
  br i1 %80, label %81, label %89

81:                                               ; preds = %70
  %82 = getelementptr inbounds %"struct.std::pair", ptr %72, i64 -1
  %83 = load i64, ptr %82, align 8
  %84 = getelementptr %"struct.std::pair", ptr %72, i64 -1, i32 1
  %85 = load i32, ptr %84, align 8
  store i64 %74, ptr %82, align 8, !tbaa !22
  store i32 %76, ptr %84, align 8, !tbaa !24
  %86 = ptrtoint ptr %82 to i64
  %87 = sub i64 %86, %78
  %88 = ashr exact i64 %87, 4
  invoke void @_ZSt13__adjust_heapIN9__gnu_cxx17__normal_iteratorIPSt4pairIxiESt6vectorIS3_SaIS3_EEEElS3_NS0_5__ops15_Iter_comp_iterISt7greaterIS3_EEEEvT_T0_SF_T1_T2_(ptr nonnull %71, i64 noundef 0, i64 noundef %88, i64 %83, i32 %85)
          to label %89 unwind label %97

89:                                               ; preds = %70, %81
  %90 = getelementptr inbounds %"struct.std::pair", ptr %72, i64 -1
  %91 = sext i32 %76 to i64
  %92 = getelementptr inbounds i64, ptr %37, i64 %91
  %93 = load i64, ptr %92, align 8, !tbaa !15
  %94 = icmp eq i64 %74, %93
  br i1 %94, label %99, label %206, !llvm.loop !25

95:                                               ; preds = %62
  %96 = landingpad { ptr, i32 }
          cleanup
  br label %268

97:                                               ; preds = %81
  %98 = landingpad { ptr, i32 }
          cleanup
  br label %265

99:                                               ; preds = %89
  %100 = getelementptr inbounds i64, ptr %8, i64 %91
  %101 = load i64, ptr %100, align 8, !tbaa !26
  %102 = getelementptr i64, ptr %100, i64 1
  %103 = load i64, ptr %102, align 8, !tbaa !26
  %104 = icmp slt i64 %101, %103
  br i1 %104, label %105, label %206

105:                                              ; preds = %99, %200
  %106 = phi i64 [ %204, %200 ], [ %101, %99 ]
  %107 = phi ptr [ %203, %200 ], [ %71, %99 ]
  %108 = phi ptr [ %202, %200 ], [ %90, %99 ]
  %109 = phi ptr [ %201, %200 ], [ %73, %99 ]
  %110 = getelementptr inbounds i32, ptr %12, i64 %106
  %111 = load i32, ptr %110, align 4, !tbaa !12
  br i1 %69, label %117, label %112

112:                                              ; preds = %105
  %113 = getelementptr inbounds i32, ptr %32, i64 %106
  %114 = load i32, ptr %113, align 4, !tbaa !12
  %115 = zext nneg i32 %114 to i64
  %116 = icmp slt i32 %114, 0
  br i1 %116, label %200, label %117

117:                                              ; preds = %105, %112
  %118 = phi i64 [ %115, %112 ], [ 1, %105 ]
  %119 = add nsw i64 %118, %74
  %120 = sext i32 %111 to i64
  %121 = getelementptr inbounds i64, ptr %37, i64 %120
  %122 = load i64, ptr %121, align 8, !tbaa !15
  %123 = icmp slt i64 %119, %122
  br i1 %123, label %124, label %200

124:                                              ; preds = %117
  store i64 %119, ptr %121, align 8, !tbaa !15
  %125 = icmp eq ptr %108, %109
  br i1 %125, label %128, label %126

126:                                              ; preds = %124
  store i64 %119, ptr %108, align 8, !tbaa !22
  %127 = getelementptr inbounds %"struct.std::pair", ptr %108, i64 0, i32 1
  store i32 %111, ptr %127, align 8, !tbaa !24
  br label %162

128:                                              ; preds = %124
  %129 = ptrtoint ptr %108 to i64
  %130 = ptrtoint ptr %107 to i64
  %131 = sub i64 %129, %130
  %132 = ashr exact i64 %131, 4
  %133 = icmp sgt i64 %132, -1
  tail call void @llvm.assume(i1 %133)
  %134 = icmp eq i64 %131, 9223372036854775792
  br i1 %134, label %135, label %137

135:                                              ; preds = %128
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.2) #11
          to label %136 unwind label %260

136:                                              ; preds = %135
  unreachable

137:                                              ; preds = %128
  %138 = tail call i64 @llvm.umax.i64(i64 %132, i64 1)
  %139 = add nuw nsw i64 %138, %132
  %140 = tail call noundef i64 @llvm.umin.i64(i64 %139, i64 576460752303423487)
  %141 = shl nuw nsw i64 %140, 4
  %142 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %141) #12
          to label %143 unwind label %258

143:                                              ; preds = %137
  %144 = getelementptr inbounds i8, ptr %142, i64 %131
  store i64 %119, ptr %144, align 8, !tbaa !22
  %145 = getelementptr inbounds %"struct.std::pair", ptr %144, i64 0, i32 1
  store i32 %111, ptr %145, align 8, !tbaa !24
  %146 = icmp eq ptr %107, %108
  br i1 %146, label %153, label %147

147:                                              ; preds = %143, %147
  %148 = phi ptr [ %151, %147 ], [ %142, %143 ]
  %149 = phi ptr [ %150, %147 ], [ %107, %143 ]
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %148, ptr noundef nonnull align 8 dereferenceable(16) %149, i64 16, i1 false), !alias.scope !27
  %150 = getelementptr inbounds %"struct.std::pair", ptr %149, i64 1
  %151 = getelementptr inbounds %"struct.std::pair", ptr %148, i64 1
  %152 = icmp eq ptr %150, %108
  br i1 %152, label %153, label %147, !llvm.loop !31

153:                                              ; preds = %147, %143
  %154 = phi ptr [ %142, %143 ], [ %151, %147 ]
  %155 = icmp eq ptr %107, null
  br i1 %155, label %157, label %156

156:                                              ; preds = %153
  tail call void @_ZdlPv(ptr noundef nonnull %107) #13
  br label %157

157:                                              ; preds = %156, %153
  %158 = getelementptr inbounds %"struct.std::pair", ptr %142, i64 %140
  %159 = load i64, ptr %154, align 8
  %160 = getelementptr %"struct.std::pair", ptr %154, i64 0, i32 1
  %161 = load i32, ptr %160, align 8
  br label %162

162:                                              ; preds = %157, %126
  %163 = phi i32 [ %161, %157 ], [ %111, %126 ]
  %164 = phi i64 [ %159, %157 ], [ %119, %126 ]
  %165 = phi ptr [ %158, %157 ], [ %109, %126 ]
  %166 = phi ptr [ %154, %157 ], [ %108, %126 ]
  %167 = phi ptr [ %142, %157 ], [ %107, %126 ]
  %168 = getelementptr inbounds %"struct.std::pair", ptr %166, i64 1
  %169 = ptrtoint ptr %168 to i64
  %170 = ptrtoint ptr %167 to i64
  %171 = sub i64 %169, %170
  %172 = ashr exact i64 %171, 4
  %173 = add nsw i64 %172, -1
  %174 = icmp sgt i64 %172, 1
  br i1 %174, label %175, label %196

175:                                              ; preds = %162, %191
  %176 = phi i64 [ %178, %191 ], [ %173, %162 ]
  %177 = add nsw i64 %176, -1
  %178 = lshr i64 %177, 1
  %179 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %178
  %180 = load i64, ptr %179, align 8, !tbaa !15
  %181 = icmp sgt i64 %180, %164
  br i1 %181, label %182, label %185

182:                                              ; preds = %175
  %183 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %178, i32 1
  %184 = load i32, ptr %183, align 4, !tbaa !12
  br label %191

185:                                              ; preds = %175
  %186 = icmp slt i64 %180, %164
  br i1 %186, label %196, label %187

187:                                              ; preds = %185
  %188 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %178, i32 1
  %189 = load i32, ptr %188, align 8, !tbaa !12
  %190 = icmp sgt i32 %189, %163
  br i1 %190, label %191, label %196

191:                                              ; preds = %187, %182
  %192 = phi i32 [ %184, %182 ], [ %189, %187 ]
  %193 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %176
  store i64 %180, ptr %193, align 8, !tbaa !22
  %194 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %176, i32 1
  store i32 %192, ptr %194, align 8, !tbaa !24
  %195 = icmp ult i64 %177, 2
  br i1 %195, label %196, label %175, !llvm.loop !32

196:                                              ; preds = %185, %187, %191, %162
  %197 = phi i64 [ %173, %162 ], [ %176, %187 ], [ 0, %191 ], [ %176, %185 ]
  %198 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %197
  store i64 %164, ptr %198, align 8, !tbaa !22
  %199 = getelementptr inbounds %"struct.std::pair", ptr %167, i64 %197, i32 1
  store i32 %163, ptr %199, align 8, !tbaa !24
  br label %200

200:                                              ; preds = %117, %196, %112
  %201 = phi ptr [ %109, %112 ], [ %165, %196 ], [ %109, %117 ]
  %202 = phi ptr [ %108, %112 ], [ %168, %196 ], [ %108, %117 ]
  %203 = phi ptr [ %107, %112 ], [ %167, %196 ], [ %107, %117 ]
  %204 = add i64 %106, 1
  %205 = icmp eq i64 %204, %103
  br i1 %205, label %206, label %105, !llvm.loop !33

206:                                              ; preds = %200, %99, %89
  %207 = phi ptr [ %73, %89 ], [ %73, %99 ], [ %201, %200 ]
  %208 = phi ptr [ %90, %89 ], [ %90, %99 ], [ %202, %200 ]
  %209 = phi ptr [ %71, %89 ], [ %71, %99 ], [ %203, %200 ]
  %210 = icmp eq ptr %209, %208
  br i1 %210, label %211, label %70

211:                                              ; preds = %206
  %212 = shl nuw nsw i64 %19, 2
  %213 = tail call noalias ptr @malloc(i64 noundef %212) #14
  %214 = icmp eq ptr %213, null
  br i1 %214, label %253, label %215

215:                                              ; preds = %211
  %216 = and i64 %19, 1
  %217 = icmp eq i64 %19, 1
  br i1 %217, label %220, label %218

218:                                              ; preds = %215
  %219 = and i64 %19, 1152921504606846974
  br label %232

220:                                              ; preds = %232, %215
  %221 = phi i64 [ 0, %215 ], [ %250, %232 ]
  %222 = icmp eq i64 %216, 0
  br i1 %222, label %231, label %223

223:                                              ; preds = %220
  %224 = getelementptr inbounds i64, ptr %37, i64 %221
  %225 = load i64, ptr %224, align 8, !tbaa !15
  %226 = icmp sgt i64 %225, 1152921504606846974
  %227 = tail call i64 @llvm.smin.i64(i64 %225, i64 2147483647)
  %228 = trunc i64 %227 to i32
  %229 = select i1 %226, i32 -1, i32 %228
  %230 = getelementptr inbounds i32, ptr %213, i64 %221
  store i32 %229, ptr %230, align 4, !tbaa !12
  br label %231

231:                                              ; preds = %220, %223
  store ptr %213, ptr %2, align 8, !tbaa !11
  store i32 %25, ptr %3, align 4, !tbaa !12
  br label %253

232:                                              ; preds = %232, %218
  %233 = phi i64 [ 0, %218 ], [ %250, %232 ]
  %234 = phi i64 [ 0, %218 ], [ %251, %232 ]
  %235 = getelementptr inbounds i64, ptr %37, i64 %233
  %236 = load i64, ptr %235, align 8, !tbaa !15
  %237 = icmp sgt i64 %236, 1152921504606846974
  %238 = tail call i64 @llvm.smin.i64(i64 %236, i64 2147483647)
  %239 = trunc i64 %238 to i32
  %240 = select i1 %237, i32 -1, i32 %239
  %241 = getelementptr inbounds i32, ptr %213, i64 %233
  store i32 %240, ptr %241, align 4, !tbaa !12
  %242 = or disjoint i64 %233, 1
  %243 = getelementptr inbounds i64, ptr %37, i64 %242
  %244 = load i64, ptr %243, align 8, !tbaa !15
  %245 = icmp sgt i64 %244, 1152921504606846974
  %246 = tail call i64 @llvm.smin.i64(i64 %244, i64 2147483647)
  %247 = trunc i64 %246 to i32
  %248 = select i1 %245, i32 -1, i32 %247
  %249 = getelementptr inbounds i32, ptr %213, i64 %242
  store i32 %248, ptr %249, align 4, !tbaa !12
  %250 = add nuw nsw i64 %233, 2
  %251 = add i64 %234, 2
  %252 = icmp eq i64 %251, %219
  br i1 %252, label %220, label %232, !llvm.loop !34

253:                                              ; preds = %211, %231
  %254 = phi i32 [ 0, %231 ], [ 3, %211 ]
  %255 = icmp eq ptr %208, null
  br i1 %255, label %257, label %256

256:                                              ; preds = %253
  tail call void @_ZdlPv(ptr noundef nonnull %208) #13
  br label %257

257:                                              ; preds = %253, %256
  tail call void @_ZdlPv(ptr noundef nonnull %37) #13
  br label %270

258:                                              ; preds = %137
  %259 = landingpad { ptr, i32 }
          cleanup
  br label %262

260:                                              ; preds = %135
  %261 = landingpad { ptr, i32 }
          cleanup
  br label %262

262:                                              ; preds = %260, %258
  %263 = phi { ptr, i32 } [ %259, %258 ], [ %261, %260 ]
  %264 = icmp eq ptr %107, null
  br i1 %264, label %268, label %265

265:                                              ; preds = %97, %262
  %266 = phi { ptr, i32 } [ %98, %97 ], [ %263, %262 ]
  %267 = phi ptr [ %71, %97 ], [ %107, %262 ]
  tail call void @_ZdlPv(ptr noundef nonnull %267) #13
  br label %268

268:                                              ; preds = %95, %262, %265
  %269 = phi { ptr, i32 } [ %96, %95 ], [ %263, %262 ], [ %266, %265 ]
  tail call void @_ZdlPv(ptr noundef nonnull %37) #13
  resume { ptr, i32 } %269

270:                                              ; preds = %21, %27, %257, %4, %6, %10
  %271 = phi i32 [ 1, %10 ], [ 1, %6 ], [ 1, %4 ], [ 0, %21 ], [ 2, %27 ], [ %254, %257 ]
  ret i32 %271
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @dijkstra_runtime(ptr nocapture noundef readnone %0) local_unnamed_addr #4 {
  ret void
}

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #5

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #6

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #7

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
  %16 = load i64, ptr %15, align 8, !tbaa !15
  %17 = load i64, ptr %13, align 8, !tbaa !15
  %18 = icmp slt i64 %16, %17
  br i1 %18, label %28, label %19

19:                                               ; preds = %9
  %20 = icmp slt i64 %17, %16
  br i1 %20, label %29, label %21

21:                                               ; preds = %19
  %22 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %14, i32 1
  %23 = load i32, ptr %22, align 8, !tbaa !24
  %24 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %12, i32 1
  %25 = load i32, ptr %24, align 8, !tbaa !24
  %26 = icmp slt i32 %23, %25
  %27 = freeze i1 %26
  br i1 %27, label %28, label %29

28:                                               ; preds = %9, %21
  br label %29

29:                                               ; preds = %19, %21, %28
  %30 = phi i64 [ %16, %28 ], [ %17, %21 ], [ %17, %19 ]
  %31 = phi i64 [ %14, %28 ], [ %12, %21 ], [ %12, %19 ]
  %32 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %10
  store i64 %30, ptr %32, align 8, !tbaa !22
  %33 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %31, i32 1
  %34 = load i32, ptr %33, align 4, !tbaa !12
  %35 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %10, i32 1
  store i32 %34, ptr %35, align 8, !tbaa !24
  %36 = icmp slt i64 %31, %7
  br i1 %36, label %9, label %37, !llvm.loop !35

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
  %50 = load i64, ptr %48, align 8, !tbaa !15
  store i64 %50, ptr %49, align 8, !tbaa !22
  %51 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %47, i32 1
  %52 = load i32, ptr %51, align 4, !tbaa !12
  %53 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %38, i32 1
  store i32 %52, ptr %53, align 8, !tbaa !24
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
  %62 = load i64, ptr %61, align 8, !tbaa !15
  %63 = icmp sgt i64 %62, %3
  br i1 %63, label %64, label %67

64:                                               ; preds = %57
  %65 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60, i32 1
  %66 = load i32, ptr %65, align 4, !tbaa !12
  br label %73

67:                                               ; preds = %57
  %68 = icmp slt i64 %62, %3
  br i1 %68, label %78, label %69

69:                                               ; preds = %67
  %70 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %60, i32 1
  %71 = load i32, ptr %70, align 8, !tbaa !12
  %72 = icmp sgt i32 %71, %4
  br i1 %72, label %73, label %78

73:                                               ; preds = %69, %64
  %74 = phi i32 [ %66, %64 ], [ %71, %69 ]
  %75 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %58
  store i64 %62, ptr %75, align 8, !tbaa !22
  %76 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %58, i32 1
  store i32 %74, ptr %76, align 8, !tbaa !24
  %77 = icmp sgt i64 %60, %1
  br i1 %77, label %57, label %78, !llvm.loop !32

78:                                               ; preds = %67, %69, %73, %54
  %79 = phi i64 [ %55, %54 ], [ %58, %69 ], [ %60, %73 ], [ %58, %67 ]
  %80 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %79
  store i64 %3, ptr %80, align 8, !tbaa !22
  %81 = getelementptr inbounds %"struct.std::pair", ptr %0, i64 %79, i32 1
  store i32 %4, ptr %81, align 8, !tbaa !24
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #8

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #9

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smin.i64(i64, i64) #9

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #9 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #10 = { cold }
attributes #11 = { cold noreturn }
attributes #12 = { builtin allocsize(0) }
attributes #13 = { builtin nounwind }
attributes #14 = { nounwind allocsize(0) }

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
!10 = !{!4, !5, i64 0}
!11 = !{!8, !8, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !6, i64 0}
!14 = !{!4, !8, i64 32}
!15 = !{!16, !16, i64 0}
!16 = !{!"long long", !6, i64 0}
!17 = distinct !{!17, !18, !19, !20}
!18 = !{!"llvm.loop.mustprogress"}
!19 = !{!"llvm.loop.isvectorized", i32 1}
!20 = !{!"llvm.loop.unroll.runtime.disable"}
!21 = distinct !{!21, !18, !20, !19}
!22 = !{!23, !16, i64 0}
!23 = !{!"_ZTSSt4pairIxiE", !16, i64 0, !13, i64 8}
!24 = !{!23, !13, i64 8}
!25 = distinct !{!25, !18}
!26 = !{!5, !5, i64 0}
!27 = !{!28, !30}
!28 = distinct !{!28, !29, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 0"}
!29 = distinct !{!29, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_"}
!30 = distinct !{!30, !29, !"_ZSt19__relocate_object_aISt4pairIxiES1_SaIS1_EEvPT_PT0_RT1_: argument 1"}
!31 = distinct !{!31, !18}
!32 = distinct !{!32, !18}
!33 = distinct !{!33, !18}
!34 = distinct !{!34, !18}
!35 = distinct !{!35, !18}
