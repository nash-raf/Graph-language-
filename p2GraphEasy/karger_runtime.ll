; ModuleID = 'karger_runtime.cpp'
source_filename = "karger_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char>::_Alloc_hider", i64, %union.anon.9 }
%"struct.std::__cxx11::basic_string<char>::_Alloc_hider" = type { ptr }
%union.anon.9 = type { i64, [8 x i8] }
%"class.std::random_device" = type { %union.anon }
%union.anon = type { %"class.std::mersenne_twister_engine" }
%"class.std::mersenne_twister_engine" = type { [624 x i64], i64 }
%"class.std::mersenne_twister_engine.0" = type { [312 x i64], i64 }
%struct.UnionFind = type { %"class.std::vector.1", %"class.std::vector.1" }
%"class.std::vector.1" = type { %"struct.std::_Vector_base.2" }
%"struct.std::_Vector_base.2" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%struct.Graph = type { i64, i64, ptr, ptr }
%"struct.std::pair" = type { i32, i32 }

$_ZN9UnionFindC2Ei = comdat any

$_ZN9UnionFind4findEi = comdat any

$_ZN9UnionFindD2Ev = comdat any

$__clang_call_terminate = comdat any

$_ZNSt23mersenne_twister_engineImLm64ELm312ELm156ELm31ELm13043109905998158313ELm29ELm6148914691236517205ELm17ELm8202884508482404352ELm37ELm18444473444759240704ELm43ELm6364136223846793005EEclEv = comdat any

@.str = private unnamed_addr constant [38 x i8] c"[Karger] graph has no edges (n=%lld)\0A\00", align 1
@.str.1 = private unnamed_addr constant [51 x i8] c"[Karger] n=%lld, m=%lld, trials=%lld -> mincut=%d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"default\00", align 1
@.str.4 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1
@.str.5 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1
@.str.6 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local i32 @karger_runtime(ptr noundef readonly %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca %"class.std::__cxx11::basic_string", align 8
  %3 = alloca %"class.std::random_device", align 8
  %4 = alloca %"class.std::mersenne_twister_engine.0", align 8
  %5 = alloca %struct.UnionFind, align 8
  %6 = icmp eq ptr %0, null
  br i1 %6, label %356, label %7

7:                                                ; preds = %1
  %8 = load i64, ptr %0, align 8, !tbaa !3
  %9 = icmp slt i64 %8, 1
  br i1 %9, label %356, label %10

10:                                               ; preds = %7
  %11 = icmp eq i64 %8, 1
  br i1 %11, label %356, label %12

12:                                               ; preds = %10
  %13 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 2
  %14 = load ptr, ptr %13, align 8, !tbaa !9
  %15 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 3
  %16 = load ptr, ptr %15, align 8, !tbaa !10
  %17 = getelementptr inbounds %struct.Graph, ptr %0, i64 0, i32 1
  %18 = load i64, ptr %17, align 8, !tbaa !11
  %19 = lshr i64 %18, 1
  %20 = icmp ugt i64 %18, 2305843009213693951
  br i1 %20, label %21, label %22

21:                                               ; preds = %12
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.5) #14
  unreachable

22:                                               ; preds = %12
  %23 = icmp ult i64 %18, 2
  br i1 %23, label %28, label %24

24:                                               ; preds = %22
  %25 = shl nuw nsw i64 %19, 3
  %26 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %25) #15
  %27 = getelementptr inbounds %"struct.std::pair", ptr %26, i64 %19
  br label %28

28:                                               ; preds = %24, %22
  %29 = phi ptr [ %27, %24 ], [ null, %22 ]
  %30 = phi ptr [ %26, %24 ], [ null, %22 ]
  %31 = trunc i64 %8 to i32
  %32 = icmp sgt i32 %31, 0
  br i1 %32, label %33, label %150

33:                                               ; preds = %28
  %34 = and i64 %8, 4294967295
  %35 = load i64, ptr %14, align 8, !tbaa !12
  br label %44

36:                                               ; preds = %143, %44
  %37 = phi i64 [ %52, %44 ], [ %144, %143 ]
  %38 = phi ptr [ %49, %44 ], [ %145, %143 ]
  %39 = phi ptr [ %48, %44 ], [ %146, %143 ]
  %40 = phi ptr [ %47, %44 ], [ %147, %143 ]
  %41 = icmp eq i64 %50, %34
  br i1 %41, label %42, label %44, !llvm.loop !13

42:                                               ; preds = %36
  %43 = icmp eq ptr %40, %39
  br i1 %43, label %150, label %153

44:                                               ; preds = %33, %36
  %45 = phi i64 [ %35, %33 ], [ %37, %36 ]
  %46 = phi i64 [ 0, %33 ], [ %50, %36 ]
  %47 = phi ptr [ %30, %33 ], [ %40, %36 ]
  %48 = phi ptr [ %30, %33 ], [ %39, %36 ]
  %49 = phi ptr [ %29, %33 ], [ %38, %36 ]
  %50 = add nuw nsw i64 %46, 1
  %51 = getelementptr inbounds i64, ptr %14, i64 %50
  %52 = load i64, ptr %51, align 8, !tbaa !12
  %53 = icmp slt i64 %45, %52
  br i1 %53, label %54, label %36

54:                                               ; preds = %44
  %55 = trunc i64 %46 to i32
  %56 = trunc i64 %46 to i32
  br label %57

57:                                               ; preds = %54, %143
  %58 = phi i64 [ %144, %143 ], [ %52, %54 ]
  %59 = phi i64 [ %148, %143 ], [ %45, %54 ]
  %60 = phi ptr [ %147, %143 ], [ %47, %54 ]
  %61 = phi ptr [ %146, %143 ], [ %48, %54 ]
  %62 = phi ptr [ %145, %143 ], [ %49, %54 ]
  %63 = getelementptr inbounds i32, ptr %16, i64 %59
  %64 = load i32, ptr %63, align 4, !tbaa !15
  %65 = sext i32 %64 to i64
  %66 = icmp slt i64 %46, %65
  br i1 %66, label %67, label %143

67:                                               ; preds = %57
  %68 = icmp eq ptr %61, %62
  br i1 %68, label %72, label %69

69:                                               ; preds = %67
  store i32 %55, ptr %61, align 4, !tbaa !17
  %70 = getelementptr inbounds %"struct.std::pair", ptr %61, i64 0, i32 1
  store i32 %64, ptr %70, align 4, !tbaa !19
  %71 = getelementptr inbounds %"struct.std::pair", ptr %61, i64 1
  br label %143

72:                                               ; preds = %67
  %73 = ptrtoint ptr %61 to i64
  %74 = ptrtoint ptr %60 to i64
  %75 = sub i64 %73, %74
  %76 = ashr exact i64 %75, 3
  %77 = icmp sgt i64 %76, -1
  tail call void @llvm.assume(i1 %77)
  %78 = icmp eq i64 %75, 9223372036854775800
  br i1 %78, label %79, label %81

79:                                               ; preds = %72
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.6) #14
          to label %80 unwind label %141

80:                                               ; preds = %79
  unreachable

81:                                               ; preds = %72
  %82 = tail call i64 @llvm.umax.i64(i64 %76, i64 1)
  %83 = add nuw nsw i64 %82, %76
  %84 = tail call noundef i64 @llvm.umin.i64(i64 %83, i64 1152921504606846975)
  %85 = shl nuw nsw i64 %84, 3
  %86 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %85) #15
          to label %87 unwind label %139

87:                                               ; preds = %81
  %88 = getelementptr inbounds i8, ptr %86, i64 %75
  store i32 %56, ptr %88, align 4, !tbaa !17
  %89 = getelementptr inbounds %"struct.std::pair", ptr %88, i64 0, i32 1
  store i32 %64, ptr %89, align 4, !tbaa !19
  %90 = icmp eq ptr %60, %61
  br i1 %90, label %131, label %91

91:                                               ; preds = %87
  %92 = ptrtoint ptr %86 to i64
  %93 = add i64 %73, -8
  %94 = sub i64 %93, %74
  %95 = lshr i64 %94, 3
  %96 = add nuw nsw i64 %95, 1
  %97 = icmp ult i64 %94, 24
  %98 = sub i64 %92, %74
  %99 = icmp ult i64 %98, 32
  %100 = or i1 %97, %99
  br i1 %100, label %121, label %101

101:                                              ; preds = %91
  %102 = and i64 %96, 4611686018427387900
  %103 = shl i64 %102, 3
  %104 = getelementptr i8, ptr %86, i64 %103
  %105 = shl i64 %102, 3
  %106 = getelementptr i8, ptr %60, i64 %105
  br label %107

107:                                              ; preds = %107, %101
  %108 = phi i64 [ 0, %101 ], [ %117, %107 ]
  %109 = shl i64 %108, 3
  %110 = getelementptr i8, ptr %86, i64 %109
  %111 = shl i64 %108, 3
  %112 = getelementptr i8, ptr %60, i64 %111
  tail call void @llvm.experimental.noalias.scope.decl(metadata !20)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !23)
  %113 = getelementptr i64, ptr %112, i64 2
  %114 = load <2 x i64>, ptr %112, align 4, !alias.scope !23, !noalias !20
  %115 = load <2 x i64>, ptr %113, align 4, !alias.scope !23, !noalias !20
  %116 = getelementptr i64, ptr %110, i64 2
  store <2 x i64> %114, ptr %110, align 4, !alias.scope !20, !noalias !23
  store <2 x i64> %115, ptr %116, align 4, !alias.scope !20, !noalias !23
  %117 = add nuw i64 %108, 4
  %118 = icmp eq i64 %117, %102
  br i1 %118, label %119, label %107, !llvm.loop !25

119:                                              ; preds = %107
  %120 = icmp eq i64 %96, %102
  br i1 %120, label %131, label %121

121:                                              ; preds = %91, %119
  %122 = phi ptr [ %86, %91 ], [ %104, %119 ]
  %123 = phi ptr [ %60, %91 ], [ %106, %119 ]
  br label %124

124:                                              ; preds = %121, %124
  %125 = phi ptr [ %129, %124 ], [ %122, %121 ]
  %126 = phi ptr [ %128, %124 ], [ %123, %121 ]
  tail call void @llvm.experimental.noalias.scope.decl(metadata !20)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !23)
  %127 = load i64, ptr %126, align 4, !alias.scope !23, !noalias !20
  store i64 %127, ptr %125, align 4, !alias.scope !20, !noalias !23
  %128 = getelementptr inbounds %"struct.std::pair", ptr %126, i64 1
  %129 = getelementptr inbounds %"struct.std::pair", ptr %125, i64 1
  %130 = icmp eq ptr %128, %61
  br i1 %130, label %131, label %124, !llvm.loop !28

131:                                              ; preds = %124, %119, %87
  %132 = phi ptr [ %86, %87 ], [ %104, %119 ], [ %129, %124 ]
  %133 = icmp eq ptr %60, null
  br i1 %133, label %135, label %134

134:                                              ; preds = %131
  tail call void @_ZdlPv(ptr noundef nonnull %60) #16
  br label %135

135:                                              ; preds = %134, %131
  %136 = getelementptr inbounds %"struct.std::pair", ptr %132, i64 1
  %137 = getelementptr inbounds %"struct.std::pair", ptr %86, i64 %84
  %138 = load i64, ptr %51, align 8, !tbaa !12
  br label %143

139:                                              ; preds = %81
  %140 = landingpad { ptr, i32 }
          cleanup
  br label %350

141:                                              ; preds = %79
  %142 = landingpad { ptr, i32 }
          cleanup
  br label %350

143:                                              ; preds = %135, %69, %57
  %144 = phi i64 [ %58, %57 ], [ %138, %135 ], [ %58, %69 ]
  %145 = phi ptr [ %62, %57 ], [ %137, %135 ], [ %62, %69 ]
  %146 = phi ptr [ %61, %57 ], [ %136, %135 ], [ %71, %69 ]
  %147 = phi ptr [ %60, %57 ], [ %86, %135 ], [ %60, %69 ]
  %148 = add nsw i64 %59, 1
  %149 = icmp slt i64 %148, %144
  br i1 %149, label %57, label %36, !llvm.loop !29

150:                                              ; preds = %28, %42
  %151 = phi ptr [ %40, %42 ], [ %30, %28 ]
  %152 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i64 noundef %8)
  br label %345

153:                                              ; preds = %42
  %154 = ptrtoint ptr %39 to i64
  %155 = ptrtoint ptr %40 to i64
  %156 = sub i64 %154, %155
  %157 = ashr exact i64 %156, 3
  %158 = icmp sgt i64 %157, -1
  tail call void @llvm.assume(i1 %158)
  %159 = mul i64 %8, %8
  %160 = tail call i64 @llvm.umin.i64(i64 %159, i64 20)
  call void @llvm.lifetime.start.p0(i64 5000, ptr nonnull %3) #17
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %2) #17
  %161 = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %2, i64 0, i32 2
  store ptr %161, ptr %2, align 8, !tbaa !30
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(7) %161, ptr noundef nonnull align 1 dereferenceable(7) @.str.2, i64 7, i1 false)
  %162 = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %2, i64 0, i32 1
  store i64 7, ptr %162, align 8, !tbaa !32
  %163 = getelementptr inbounds i8, ptr %2, i64 23
  store i8 0, ptr %163, align 1, !tbaa !34
  invoke void @_ZNSt13random_device7_M_initERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE(ptr noundef nonnull align 8 dereferenceable(5000) %3, ptr noundef nonnull align 8 dereferenceable(32) %2)
          to label %164 unwind label %171

164:                                              ; preds = %153
  %165 = load ptr, ptr %2, align 8, !tbaa !35
  %166 = icmp eq ptr %165, %161
  br i1 %166, label %167, label %170

167:                                              ; preds = %164
  %168 = load i64, ptr %162, align 8, !tbaa !32
  %169 = icmp ult i64 %168, 16
  call void @llvm.assume(i1 %169)
  br label %180

170:                                              ; preds = %164
  call void @_ZdlPv(ptr noundef %165) #16
  br label %180

171:                                              ; preds = %153
  %172 = landingpad { ptr, i32 }
          cleanup
  %173 = load ptr, ptr %2, align 8, !tbaa !35
  %174 = icmp eq ptr %173, %161
  br i1 %174, label %175, label %178

175:                                              ; preds = %171
  %176 = load i64, ptr %162, align 8, !tbaa !32
  %177 = icmp ult i64 %176, 16
  call void @llvm.assume(i1 %177)
  br label %179

178:                                              ; preds = %171
  call void @_ZdlPv(ptr noundef %173) #16
  br label %179

179:                                              ; preds = %178, %175
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %2) #17
  br label %343

180:                                              ; preds = %170, %167
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %2) #17
  call void @llvm.lifetime.start.p0(i64 2504, ptr nonnull %4) #17
  %181 = invoke noundef i32 @_ZNSt13random_device9_M_getvalEv(ptr noundef nonnull align 8 dereferenceable(5000) %3)
          to label %182 unwind label %210

182:                                              ; preds = %180
  %183 = zext i32 %181 to i64
  store i64 %183, ptr %4, align 8, !tbaa !12
  br label %184

184:                                              ; preds = %194, %182
  %185 = phi i64 [ %183, %182 ], [ %198, %194 ]
  %186 = phi i64 [ 1, %182 ], [ %200, %194 ]
  %187 = lshr i64 %185, 62
  %188 = xor i64 %187, %185
  %189 = mul i64 %188, 6364136223846793005
  %190 = add i64 %189, %186
  %191 = getelementptr inbounds [312 x i64], ptr %4, i64 0, i64 %186
  store i64 %190, ptr %191, align 8, !tbaa !12
  %192 = add nuw nsw i64 %186, 1
  %193 = icmp eq i64 %192, 312
  br i1 %193, label %201, label %194, !llvm.loop !36

194:                                              ; preds = %184
  %195 = lshr i64 %190, 62
  %196 = xor i64 %195, %190
  %197 = mul i64 %196, 6364136223846793005
  %198 = add i64 %197, %192
  %199 = getelementptr inbounds [312 x i64], ptr %4, i64 0, i64 %192
  store i64 %198, ptr %199, align 8, !tbaa !12
  %200 = add nuw nsw i64 %186, 2
  br label %184

201:                                              ; preds = %184
  %202 = getelementptr inbounds %"class.std::mersenne_twister_engine.0", ptr %4, i64 0, i32 1
  store i64 312, ptr %202, align 8, !tbaa !37
  %203 = add nsw i64 %157, -1
  %204 = icmp sgt i32 %31, 2
  %205 = zext nneg i64 %157 to i128
  %206 = sub nsw i64 0, %157
  %207 = getelementptr inbounds %struct.UnionFind, ptr %5, i64 0, i32 1
  %208 = call i64 @llvm.umax.i64(i64 %157, i64 1)
  %209 = call i64 @llvm.umax.i64(i64 %160, i64 1)
  br label %212

210:                                              ; preds = %180
  %211 = landingpad { ptr, i32 }
          cleanup
  br label %338

212:                                              ; preds = %304, %201
  %213 = phi i32 [ 2147483647, %201 ], [ %295, %304 ]
  %214 = phi i64 [ 0, %201 ], [ %305, %304 ]
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %5) #17
  invoke void @_ZN9UnionFindC2Ei(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %31)
          to label %215 unwind label %251

215:                                              ; preds = %212
  br i1 %204, label %217, label %216

216:                                              ; preds = %289, %215
  br label %308

217:                                              ; preds = %215, %289
  %218 = phi i32 [ %290, %289 ], [ %31, %215 ]
  %219 = invoke noundef i64 @_ZNSt23mersenne_twister_engineImLm64ELm312ELm156ELm31ELm13043109905998158313ELm29ELm6148914691236517205ELm17ELm8202884508482404352ELm37ELm18444473444759240704ELm43ELm6364136223846793005EEclEv(ptr noundef nonnull align 8 dereferenceable(2504) %4)
          to label %220 unwind label %255

220:                                              ; preds = %217
  %221 = zext i64 %219 to i128
  %222 = mul nuw nsw i128 %221, %205
  %223 = trunc i128 %222 to i64
  %224 = icmp ult i64 %203, %223
  %225 = lshr i128 %222, 64
  %226 = trunc i128 %225 to i64
  br i1 %224, label %240, label %227

227:                                              ; preds = %220
  %228 = urem i64 %206, %157
  %229 = icmp ugt i64 %228, %223
  br i1 %229, label %230, label %240

230:                                              ; preds = %227, %232
  %231 = invoke noundef i64 @_ZNSt23mersenne_twister_engineImLm64ELm312ELm156ELm31ELm13043109905998158313ELm29ELm6148914691236517205ELm17ELm8202884508482404352ELm37ELm18444473444759240704ELm43ELm6364136223846793005EEclEv(ptr noundef nonnull align 8 dereferenceable(2504) %4)
          to label %232 unwind label %253

232:                                              ; preds = %230
  %233 = zext i64 %231 to i128
  %234 = mul nuw nsw i128 %233, %205
  %235 = trunc i128 %234 to i64
  %236 = icmp ugt i64 %228, %235
  br i1 %236, label %230, label %237, !llvm.loop !39

237:                                              ; preds = %232
  %238 = lshr i128 %234, 64
  %239 = trunc i128 %238 to i64
  br label %240

240:                                              ; preds = %237, %227, %220
  %241 = phi i64 [ %226, %220 ], [ %239, %237 ], [ %226, %227 ]
  %242 = getelementptr inbounds %"struct.std::pair", ptr %40, i64 %241
  %243 = load i32, ptr %242, align 4, !tbaa !17
  %244 = getelementptr inbounds %"struct.std::pair", ptr %40, i64 %241, i32 1
  %245 = load i32, ptr %244, align 4, !tbaa !19
  %246 = invoke noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %243)
          to label %247 unwind label %257

247:                                              ; preds = %240
  %248 = invoke noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %245)
          to label %249 unwind label %259

249:                                              ; preds = %247
  %250 = icmp eq i32 %246, %248
  br i1 %250, label %289, label %261, !llvm.loop !40

251:                                              ; preds = %212
  %252 = landingpad { ptr, i32 }
          cleanup
  br label %328

253:                                              ; preds = %230
  %254 = landingpad { ptr, i32 }
          cleanup
  br label %326

255:                                              ; preds = %217
  %256 = landingpad { ptr, i32 }
          cleanup
  br label %326

257:                                              ; preds = %240
  %258 = landingpad { ptr, i32 }
          cleanup
  br label %326

259:                                              ; preds = %263, %261, %247
  %260 = landingpad { ptr, i32 }
          cleanup
  br label %326

261:                                              ; preds = %249
  %262 = invoke noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %246)
          to label %263 unwind label %259

263:                                              ; preds = %261
  %264 = invoke noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %248)
          to label %265 unwind label %259

265:                                              ; preds = %263
  %266 = icmp eq i32 %262, %264
  br i1 %266, label %287, label %267

267:                                              ; preds = %265
  %268 = sext i32 %262 to i64
  %269 = load ptr, ptr %207, align 8, !tbaa !41
  %270 = getelementptr inbounds i32, ptr %269, i64 %268
  %271 = load i32, ptr %270, align 4, !tbaa !15
  %272 = sext i32 %264 to i64
  %273 = getelementptr inbounds i32, ptr %269, i64 %272
  %274 = load i32, ptr %273, align 4, !tbaa !15
  %275 = icmp slt i32 %271, %274
  %276 = select i1 %275, i32 %264, i32 %262
  %277 = select i1 %275, i32 %262, i32 %264
  %278 = sext i32 %277 to i64
  %279 = load ptr, ptr %5, align 8, !tbaa !41
  %280 = getelementptr inbounds i32, ptr %279, i64 %278
  store i32 %276, ptr %280, align 4, !tbaa !15
  %281 = getelementptr inbounds i32, ptr %269, i64 %278
  %282 = load i32, ptr %281, align 4, !tbaa !15
  %283 = sext i32 %276 to i64
  %284 = getelementptr inbounds i32, ptr %269, i64 %283
  %285 = load i32, ptr %284, align 4, !tbaa !15
  %286 = add nsw i32 %285, %282
  store i32 %286, ptr %284, align 4, !tbaa !15
  br label %287

287:                                              ; preds = %267, %265
  %288 = add nsw i32 %218, -1
  br label %289

289:                                              ; preds = %249, %287
  %290 = phi i32 [ %288, %287 ], [ %218, %249 ]
  %291 = icmp sgt i32 %290, 2
  br i1 %291, label %217, label %216

292:                                              ; preds = %318
  %293 = icmp slt i32 %321, %213
  %294 = icmp ult i32 %321, 2
  %295 = call i32 @llvm.smin.i32(i32 %321, i32 %213)
  %296 = and i1 %293, %294
  %297 = load ptr, ptr %207, align 8, !tbaa !41
  %298 = icmp eq ptr %297, null
  br i1 %298, label %300, label %299

299:                                              ; preds = %292
  call void @_ZdlPv(ptr noundef nonnull %297) #16
  br label %300

300:                                              ; preds = %299, %292
  %301 = load ptr, ptr %5, align 8, !tbaa !41
  %302 = icmp eq ptr %301, null
  br i1 %302, label %304, label %303

303:                                              ; preds = %300
  call void @_ZdlPv(ptr noundef nonnull %301) #16
  br label %304

304:                                              ; preds = %300, %303
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %5) #17
  %305 = add nuw nsw i64 %214, 1
  %306 = icmp eq i64 %305, %209
  %307 = select i1 %296, i1 true, i1 %306
  br i1 %307, label %330, label %212, !llvm.loop !43

308:                                              ; preds = %216, %318
  %309 = phi i64 [ %322, %318 ], [ 0, %216 ]
  %310 = phi i32 [ %321, %318 ], [ 0, %216 ]
  %311 = getelementptr inbounds %"struct.std::pair", ptr %40, i64 %309
  %312 = load i32, ptr %311, align 4, !tbaa !17
  %313 = getelementptr inbounds %"struct.std::pair", ptr %40, i64 %309, i32 1
  %314 = load i32, ptr %313, align 4, !tbaa !19
  %315 = invoke noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %312)
          to label %316 unwind label %324

316:                                              ; preds = %308
  %317 = invoke noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %5, i32 noundef %314)
          to label %318 unwind label %324

318:                                              ; preds = %316
  %319 = icmp ne i32 %315, %317
  %320 = zext i1 %319 to i32
  %321 = add nuw nsw i32 %310, %320
  %322 = add nuw nsw i64 %309, 1
  %323 = icmp eq i64 %322, %208
  br i1 %323, label %292, label %308, !llvm.loop !44

324:                                              ; preds = %316, %308
  %325 = landingpad { ptr, i32 }
          cleanup
  br label %326

326:                                              ; preds = %253, %255, %259, %257, %324
  %327 = phi { ptr, i32 } [ %325, %324 ], [ %260, %259 ], [ %258, %257 ], [ %254, %253 ], [ %256, %255 ]
  call void @_ZN9UnionFindD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %5) #17
  br label %328

328:                                              ; preds = %326, %251
  %329 = phi { ptr, i32 } [ %327, %326 ], [ %252, %251 ]
  call void @llvm.lifetime.end.p0(i64 48, ptr nonnull %5) #17
  br label %338

330:                                              ; preds = %304
  %331 = load i64, ptr %0, align 8, !tbaa !3
  %332 = load i64, ptr %17, align 8, !tbaa !11
  %333 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i64 noundef %331, i64 noundef %332, i64 noundef %160, i32 noundef %295)
  call void @llvm.lifetime.end.p0(i64 2504, ptr nonnull %4) #17
  invoke void @_ZNSt13random_device7_M_finiEv(ptr noundef nonnull align 8 dereferenceable(5000) %3)
          to label %337 unwind label %334

334:                                              ; preds = %330
  %335 = landingpad { ptr, i32 }
          catch ptr null
  %336 = extractvalue { ptr, i32 } %335, 0
  call void @__clang_call_terminate(ptr %336) #18
  unreachable

337:                                              ; preds = %330
  call void @llvm.lifetime.end.p0(i64 5000, ptr nonnull %3) #17
  br label %345

338:                                              ; preds = %328, %210
  %339 = phi { ptr, i32 } [ %211, %210 ], [ %329, %328 ]
  call void @llvm.lifetime.end.p0(i64 2504, ptr nonnull %4) #17
  invoke void @_ZNSt13random_device7_M_finiEv(ptr noundef nonnull align 8 dereferenceable(5000) %3)
          to label %343 unwind label %340

340:                                              ; preds = %338
  %341 = landingpad { ptr, i32 }
          catch ptr null
  %342 = extractvalue { ptr, i32 } %341, 0
  call void @__clang_call_terminate(ptr %342) #18
  unreachable

343:                                              ; preds = %338, %179
  %344 = phi { ptr, i32 } [ %172, %179 ], [ %339, %338 ]
  call void @llvm.lifetime.end.p0(i64 5000, ptr nonnull %3) #17
  br label %350

345:                                              ; preds = %337, %150
  %346 = phi ptr [ %151, %150 ], [ %40, %337 ]
  %347 = phi i32 [ 0, %150 ], [ %295, %337 ]
  %348 = icmp eq ptr %346, null
  br i1 %348, label %356, label %349

349:                                              ; preds = %345
  call void @_ZdlPv(ptr noundef nonnull %346) #16
  br label %356

350:                                              ; preds = %139, %141, %343
  %351 = phi ptr [ %40, %343 ], [ %60, %139 ], [ %60, %141 ]
  %352 = phi { ptr, i32 } [ %344, %343 ], [ %140, %139 ], [ %142, %141 ]
  %353 = icmp eq ptr %351, null
  br i1 %353, label %355, label %354

354:                                              ; preds = %350
  call void @_ZdlPv(ptr noundef nonnull %351) #16
  br label %355

355:                                              ; preds = %350, %354
  resume { ptr, i32 } %352

356:                                              ; preds = %349, %345, %10, %7, %1
  %357 = phi i32 [ -1, %1 ], [ -1, %7 ], [ 0, %10 ], [ %347, %345 ], [ %347, %349 ]
  ret i32 %357
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN9UnionFindC2Ei(ptr noundef nonnull align 8 dereferenceable(48) %0, i32 noundef %1) unnamed_addr #0 comdat align 2 personality ptr @__gxx_personality_v0 {
  %3 = sext i32 %1 to i64
  %4 = icmp slt i32 %1, 0
  br i1 %4, label %5, label %6

5:                                                ; preds = %2
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.4) #14
  unreachable

6:                                                ; preds = %2
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %0, i8 0, i64 24, i1 false)
  %7 = icmp eq i32 %1, 0
  br i1 %7, label %17, label %8

8:                                                ; preds = %6
  %9 = shl nuw nsw i64 %3, 2
  %10 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %9) #15
  store ptr %10, ptr %0, align 8, !tbaa !41
  %11 = getelementptr inbounds i32, ptr %10, i64 %3
  %12 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %0, i64 0, i32 2
  store ptr %11, ptr %12, align 8, !tbaa !45
  store i32 0, ptr %10, align 4, !tbaa !15
  %13 = getelementptr i32, ptr %10, i64 1
  %14 = icmp eq i32 %1, 1
  br i1 %14, label %18, label %15

15:                                               ; preds = %8
  %16 = add nsw i64 %9, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %13, i8 0, i64 %16, i1 false), !tbaa !15
  br label %18

17:                                               ; preds = %6
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(48) %0, i8 0, i64 48, i1 false)
  br label %72

18:                                               ; preds = %8, %15
  %19 = phi ptr [ %11, %15 ], [ %13, %8 ]
  %20 = getelementptr inbounds %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data", ptr %0, i64 0, i32 1
  store ptr %19, ptr %20, align 8, !tbaa !46
  %21 = getelementptr inbounds %struct.UnionFind, ptr %0, i64 0, i32 1
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %21, i8 0, i64 24, i1 false)
  %22 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %9) #15
          to label %23 unwind label %79

23:                                               ; preds = %18
  store ptr %22, ptr %21, align 8, !tbaa !41
  %24 = getelementptr inbounds i32, ptr %22, i64 %3
  %25 = getelementptr inbounds %struct.UnionFind, ptr %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 2
  store ptr %24, ptr %25, align 8, !tbaa !45
  %26 = icmp ult i32 %1, 8
  br i1 %26, label %41, label %27

27:                                               ; preds = %23
  %28 = and i64 %3, 2147483640
  %29 = and i64 %3, 7
  %30 = shl nuw nsw i64 %28, 2
  %31 = getelementptr i8, ptr %22, i64 %30
  br label %32

32:                                               ; preds = %32, %27
  %33 = phi i64 [ 0, %27 ], [ %37, %32 ]
  %34 = shl i64 %33, 2
  %35 = getelementptr i8, ptr %22, i64 %34
  %36 = getelementptr i32, ptr %35, i64 4
  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, ptr %35, align 4, !tbaa !15
  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, ptr %36, align 4, !tbaa !15
  %37 = add nuw i64 %33, 8
  %38 = icmp eq i64 %37, %28
  br i1 %38, label %39, label %32, !llvm.loop !47

39:                                               ; preds = %32
  %40 = icmp eq i64 %28, %3
  br i1 %40, label %50, label %41

41:                                               ; preds = %23, %39
  %42 = phi i64 [ %3, %23 ], [ %29, %39 ]
  %43 = phi ptr [ %22, %23 ], [ %31, %39 ]
  br label %44

44:                                               ; preds = %41, %44
  %45 = phi i64 [ %47, %44 ], [ %42, %41 ]
  %46 = phi ptr [ %48, %44 ], [ %43, %41 ]
  %47 = add i64 %45, -1
  store i32 1, ptr %46, align 4, !tbaa !15
  %48 = getelementptr inbounds i32, ptr %46, i64 1
  %49 = icmp eq i64 %47, 0
  br i1 %49, label %50, label %44, !llvm.loop !48

50:                                               ; preds = %44, %39
  %51 = phi ptr [ %31, %39 ], [ %48, %44 ]
  %52 = getelementptr inbounds %struct.UnionFind, ptr %0, i64 0, i32 1, i32 0, i32 0, i32 0, i32 1
  store ptr %51, ptr %52, align 8, !tbaa !46
  br i1 %7, label %72, label %53

53:                                               ; preds = %50
  %54 = tail call i32 @llvm.smax.i32(i32 %1, i32 1)
  %55 = zext nneg i32 %54 to i64
  %56 = icmp slt i32 %1, 8
  br i1 %56, label %70, label %57

57:                                               ; preds = %53
  %58 = and i64 %55, 2147483640
  br label %59

59:                                               ; preds = %59, %57
  %60 = phi i64 [ 0, %57 ], [ %65, %59 ]
  %61 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %57 ], [ %66, %59 ]
  %62 = add <4 x i32> %61, <i32 4, i32 4, i32 4, i32 4>
  %63 = getelementptr inbounds i32, ptr %10, i64 %60
  %64 = getelementptr inbounds i32, ptr %63, i64 4
  store <4 x i32> %61, ptr %63, align 4, !tbaa !15
  store <4 x i32> %62, ptr %64, align 4, !tbaa !15
  %65 = add nuw i64 %60, 8
  %66 = add <4 x i32> %61, <i32 8, i32 8, i32 8, i32 8>
  %67 = icmp eq i64 %65, %58
  br i1 %67, label %68, label %59, !llvm.loop !49

68:                                               ; preds = %59
  %69 = icmp eq i64 %58, %55
  br i1 %69, label %72, label %70

70:                                               ; preds = %53, %68
  %71 = phi i64 [ 0, %53 ], [ %58, %68 ]
  br label %73

72:                                               ; preds = %73, %68, %17, %50
  ret void

73:                                               ; preds = %70, %73
  %74 = phi i64 [ %77, %73 ], [ %71, %70 ]
  %75 = getelementptr inbounds i32, ptr %10, i64 %74
  %76 = trunc i64 %74 to i32
  store i32 %76, ptr %75, align 4, !tbaa !15
  %77 = add nuw nsw i64 %74, 1
  %78 = icmp eq i64 %77, %55
  br i1 %78, label %72, label %73, !llvm.loop !50

79:                                               ; preds = %18
  %80 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZdlPv(ptr noundef nonnull %10) #16
  resume { ptr, i32 } %80
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %0, i32 noundef %1) local_unnamed_addr #0 comdat align 2 {
  %3 = sext i32 %1 to i64
  %4 = load ptr, ptr %0, align 8, !tbaa !41
  %5 = getelementptr inbounds i32, ptr %4, i64 %3
  %6 = load i32, ptr %5, align 4, !tbaa !15
  %7 = icmp eq i32 %6, %1
  br i1 %7, label %8, label %10

8:                                                ; preds = %2, %10
  %9 = phi i32 [ %11, %10 ], [ %1, %2 ]
  ret i32 %9

10:                                               ; preds = %2
  %11 = tail call noundef i32 @_ZN9UnionFind4findEi(ptr noundef nonnull align 8 dereferenceable(48) %0, i32 noundef %6)
  %12 = load ptr, ptr %0, align 8, !tbaa !41
  %13 = getelementptr inbounds i32, ptr %12, i64 %3
  store i32 %11, ptr %13, align 4, !tbaa !15
  br label %8
}

; Function Attrs: inlinehint mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN9UnionFindD2Ev(ptr noundef nonnull align 8 dereferenceable(48) %0) unnamed_addr #3 comdat align 2 personality ptr @__gxx_personality_v0 {
  %2 = getelementptr inbounds %struct.UnionFind, ptr %0, i64 0, i32 1
  %3 = load ptr, ptr %2, align 8, !tbaa !41
  %4 = icmp eq ptr %3, null
  br i1 %4, label %6, label %5

5:                                                ; preds = %1
  tail call void @_ZdlPv(ptr noundef nonnull %3) #16
  br label %6

6:                                                ; preds = %1, %5
  %7 = load ptr, ptr %0, align 8, !tbaa !41
  %8 = icmp eq ptr %7, null
  br i1 %8, label %10, label %9

9:                                                ; preds = %6
  tail call void @_ZdlPv(ptr noundef nonnull %7) #16
  br label %10

10:                                               ; preds = %6, %9
  ret void
}

declare dso_local void @_ZNSt13random_device7_M_initERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE(ptr noundef nonnull align 8 dereferenceable(5000), ptr noundef nonnull align 8 dereferenceable(32)) local_unnamed_addr #4

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: noinline noreturn nounwind uwtable
define linkonce_odr hidden void @__clang_call_terminate(ptr noundef %0) local_unnamed_addr #6 comdat {
  %2 = tail call ptr @__cxa_begin_catch(ptr %0) #17
  tail call void @_ZSt9terminatev() #18
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr

declare dso_local void @_ZSt9terminatev() local_unnamed_addr

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #7

declare dso_local noundef i32 @_ZNSt13random_device9_M_getvalEv(ptr noundef nonnull align 8 dereferenceable(5000)) local_unnamed_addr #4

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #8

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #9

declare dso_local void @_ZNSt13random_device7_M_finiEv(ptr noundef nonnull align 8 dereferenceable(5000)) local_unnamed_addr #4

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef i64 @_ZNSt23mersenne_twister_engineImLm64ELm312ELm156ELm31ELm13043109905998158313ELm29ELm6148914691236517205ELm17ELm8202884508482404352ELm37ELm18444473444759240704ELm43ELm6364136223846793005EEclEv(ptr noundef nonnull align 8 dereferenceable(2504) %0) local_unnamed_addr #0 comdat align 2 {
  %2 = getelementptr inbounds %"class.std::mersenne_twister_engine.0", ptr %0, i64 0, i32 1
  %3 = load i64, ptr %2, align 8, !tbaa !37
  %4 = icmp ugt i64 %3, 311
  br i1 %4, label %5, label %86

5:                                                ; preds = %1
  %6 = load i64, ptr %0, align 8, !tbaa !12
  %7 = insertelement <2 x i64> poison, i64 %6, i64 1
  br label %8

8:                                                ; preds = %8, %5
  %9 = phi i64 [ 0, %5 ], [ %28, %8 ]
  %10 = phi <2 x i64> [ %7, %5 ], [ %14, %8 ]
  %11 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %9
  %12 = or disjoint i64 %9, 1
  %13 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %12
  %14 = load <2 x i64>, ptr %13, align 8, !tbaa !12
  %15 = shufflevector <2 x i64> %10, <2 x i64> %14, <2 x i32> <i32 1, i32 2>
  %16 = and <2 x i64> %15, <i64 -2147483648, i64 -2147483648>
  %17 = and <2 x i64> %14, <i64 2147483646, i64 2147483646>
  %18 = or disjoint <2 x i64> %17, %16
  %19 = add nuw nsw i64 %9, 156
  %20 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %19
  %21 = load <2 x i64>, ptr %20, align 8, !tbaa !12
  %22 = lshr exact <2 x i64> %18, <i64 1, i64 1>
  %23 = xor <2 x i64> %22, %21
  %24 = and <2 x i64> %14, <i64 1, i64 1>
  %25 = icmp eq <2 x i64> %24, zeroinitializer
  %26 = select <2 x i1> %25, <2 x i64> zeroinitializer, <2 x i64> <i64 -5403634167711393303, i64 -5403634167711393303>
  %27 = xor <2 x i64> %23, %26
  store <2 x i64> %27, ptr %11, align 8, !tbaa !12
  %28 = add nuw i64 %9, 2
  %29 = icmp eq i64 %28, 156
  br i1 %29, label %30, label %8, !llvm.loop !51

30:                                               ; preds = %8
  %31 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 156
  %32 = load i64, ptr %31, align 8, !tbaa !12
  %33 = insertelement <2 x i64> poison, i64 %32, i64 1
  br label %34

34:                                               ; preds = %34, %30
  %35 = phi i64 [ 0, %30 ], [ %54, %34 ]
  %36 = phi <2 x i64> [ %33, %30 ], [ %41, %34 ]
  %37 = add i64 %35, 156
  %38 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %37
  %39 = add i64 %35, 157
  %40 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %39
  %41 = load <2 x i64>, ptr %40, align 8, !tbaa !12
  %42 = shufflevector <2 x i64> %36, <2 x i64> %41, <2 x i32> <i32 1, i32 2>
  %43 = and <2 x i64> %42, <i64 -2147483648, i64 -2147483648>
  %44 = and <2 x i64> %41, <i64 2147483646, i64 2147483646>
  %45 = or disjoint <2 x i64> %44, %43
  %46 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %35
  %47 = load <2 x i64>, ptr %46, align 8, !tbaa !12
  %48 = lshr exact <2 x i64> %45, <i64 1, i64 1>
  %49 = xor <2 x i64> %48, %47
  %50 = and <2 x i64> %41, <i64 1, i64 1>
  %51 = icmp eq <2 x i64> %50, zeroinitializer
  %52 = select <2 x i1> %51, <2 x i64> zeroinitializer, <2 x i64> <i64 -5403634167711393303, i64 -5403634167711393303>
  %53 = xor <2 x i64> %49, %52
  store <2 x i64> %53, ptr %38, align 8, !tbaa !12
  %54 = add nuw i64 %35, 2
  %55 = icmp eq i64 %54, 154
  br i1 %55, label %56, label %34, !llvm.loop !52

56:                                               ; preds = %34
  %57 = extractelement <2 x i64> %41, i64 1
  %58 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 310
  %59 = and i64 %57, -2147483648
  %60 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 311
  %61 = load i64, ptr %60, align 8, !tbaa !12
  %62 = and i64 %61, 2147483646
  %63 = or disjoint i64 %62, %59
  %64 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 154
  %65 = load i64, ptr %64, align 8, !tbaa !12
  %66 = lshr exact i64 %63, 1
  %67 = xor i64 %66, %65
  %68 = and i64 %61, 1
  %69 = icmp eq i64 %68, 0
  %70 = select i1 %69, i64 0, i64 -5403634167711393303
  %71 = xor i64 %67, %70
  store i64 %71, ptr %58, align 8, !tbaa !12
  %72 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 311
  %73 = load i64, ptr %72, align 8, !tbaa !12
  %74 = and i64 %73, -2147483648
  %75 = load i64, ptr %0, align 8, !tbaa !12
  %76 = and i64 %75, 2147483646
  %77 = or disjoint i64 %76, %74
  %78 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 155
  %79 = load i64, ptr %78, align 8, !tbaa !12
  %80 = lshr exact i64 %77, 1
  %81 = xor i64 %80, %79
  %82 = and i64 %75, 1
  %83 = icmp eq i64 %82, 0
  %84 = select i1 %83, i64 0, i64 -5403634167711393303
  %85 = xor i64 %81, %84
  store i64 %85, ptr %72, align 8, !tbaa !12
  br label %86

86:                                               ; preds = %56, %1
  %87 = phi i64 [ 0, %56 ], [ %3, %1 ]
  %88 = add nuw nsw i64 %87, 1
  store i64 %88, ptr %2, align 8, !tbaa !37
  %89 = getelementptr inbounds [312 x i64], ptr %0, i64 0, i64 %87
  %90 = load i64, ptr %89, align 8, !tbaa !12
  %91 = lshr i64 %90, 29
  %92 = and i64 %91, 22906492245
  %93 = xor i64 %92, %90
  %94 = shl i64 %93, 17
  %95 = and i64 %94, 8202884508482404352
  %96 = xor i64 %95, %93
  %97 = shl i64 %96, 37
  %98 = and i64 %97, -2270628950310912
  %99 = xor i64 %98, %96
  %100 = lshr i64 %99, 43
  %101 = xor i64 %100, %99
  ret i64 %101
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #10

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #11

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #12

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #13

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smin.i32(i32, i32) #11

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { inlinehint mustprogress nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #6 = { noinline noreturn nounwind uwtable "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #11 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #12 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #13 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #14 = { cold noreturn }
attributes #15 = { builtin allocsize(0) }
attributes #16 = { builtin nounwind }
attributes #17 = { nounwind }
attributes #18 = { noreturn nounwind }

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
!11 = !{!4, !5, i64 8}
!12 = !{!5, !5, i64 0}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.mustprogress"}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !6, i64 0}
!17 = !{!18, !16, i64 0}
!18 = !{!"_ZTSSt4pairIiiE", !16, i64 0, !16, i64 4}
!19 = !{!18, !16, i64 4}
!20 = !{!21}
!21 = distinct !{!21, !22, !"_ZSt19__relocate_object_aISt4pairIiiES1_SaIS1_EEvPT_PT0_RT1_: argument 0"}
!22 = distinct !{!22, !"_ZSt19__relocate_object_aISt4pairIiiES1_SaIS1_EEvPT_PT0_RT1_"}
!23 = !{!24}
!24 = distinct !{!24, !22, !"_ZSt19__relocate_object_aISt4pairIiiES1_SaIS1_EEvPT_PT0_RT1_: argument 1"}
!25 = distinct !{!25, !14, !26, !27}
!26 = !{!"llvm.loop.isvectorized", i32 1}
!27 = !{!"llvm.loop.unroll.runtime.disable"}
!28 = distinct !{!28, !14, !26}
!29 = distinct !{!29, !14}
!30 = !{!31, !8, i64 0}
!31 = !{!"_ZTSNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_Alloc_hiderE", !8, i64 0}
!32 = !{!33, !5, i64 8}
!33 = !{!"_ZTSNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", !31, i64 0, !5, i64 8, !6, i64 16}
!34 = !{!6, !6, i64 0}
!35 = !{!33, !8, i64 0}
!36 = distinct !{!36, !14}
!37 = !{!38, !5, i64 2496}
!38 = !{!"_ZTSSt23mersenne_twister_engineImLm64ELm312ELm156ELm31ELm13043109905998158313ELm29ELm6148914691236517205ELm17ELm8202884508482404352ELm37ELm18444473444759240704ELm43ELm6364136223846793005EE", !6, i64 0, !5, i64 2496}
!39 = distinct !{!39, !14}
!40 = distinct !{!40, !14}
!41 = !{!42, !8, i64 0}
!42 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !8, i64 0, !8, i64 8, !8, i64 16}
!43 = distinct !{!43, !14}
!44 = distinct !{!44, !14}
!45 = !{!42, !8, i64 16}
!46 = !{!42, !8, i64 8}
!47 = distinct !{!47, !14, !26, !27}
!48 = distinct !{!48, !14, !27, !26}
!49 = distinct !{!49, !14, !26, !27}
!50 = distinct !{!50, !14, !27, !26}
!51 = distinct !{!51, !14, !26, !27}
!52 = distinct !{!52, !14, !26, !27}
