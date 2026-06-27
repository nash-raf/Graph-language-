; ModuleID = 'dfs_runtime_src.cpp'
source_filename = "dfs_runtime_src.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl" = type { %"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" }
%"struct.std::_Vector_base<char, std::allocator<char>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::vector.0" = type { %"struct.std::_Vector_base.1" }
%"struct.std::_Vector_base.1" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%class.anon = type { ptr, ptr, ptr, ptr }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [43 x i8] c"[dfs_runtime_src] invalid src %d (n=%lld)\0A\00", align 1
@.str.1 = private unnamed_addr constant [26 x i8] c"vector::_M_realloc_append\00", align 1
@.str.3 = private unnamed_addr constant [16 x i8] c"vector::reserve\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local range(i32 0, 5) i32 @dfs_runtime_src(ptr noundef %0, i32 noundef %1, ptr noundef writeonly %2, ptr noundef writeonly %3) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %5 = alloca ptr, align 8
  %6 = alloca %"class.std::vector", align 8
  %7 = alloca %"class.std::vector.0", align 8
  %8 = alloca %"class.std::vector.0", align 8
  %9 = alloca %class.anon, align 8
  store ptr %0, ptr %5, align 8, !tbaa !3
  %10 = icmp ne ptr %0, null
  %11 = icmp ne ptr %2, null
  %12 = and i1 %10, %11
  %13 = icmp ne ptr %3, null
  %14 = and i1 %12, %13
  br i1 %14, label %15, label %229

15:                                               ; preds = %4
  %16 = load i64, ptr %0, align 8, !tbaa !8
  %17 = icmp slt i64 %16, 0
  br i1 %17, label %229, label %18

18:                                               ; preds = %15
  %19 = icmp slt i32 %1, 0
  %20 = trunc i64 %16 to i32
  %21 = icmp slt i32 %1, %20
  %22 = or i1 %19, %21
  br i1 %22, label %26, label %23

23:                                               ; preds = %18
  %24 = load ptr, ptr @stderr, align 8, !tbaa !13
  %25 = tail call i32 (ptr, ptr, ...) @fprintf(ptr noundef %24, ptr noundef nonnull @.str, i32 noundef %1, i64 noundef %16) #13
  br label %229

26:                                               ; preds = %18
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %6) #14
  %27 = icmp eq i64 %16, 0
  br i1 %27, label %62, label %28

28:                                               ; preds = %26
  %29 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %16) #15
  store ptr %29, ptr %6, align 8, !tbaa !15
  %30 = getelementptr inbounds nuw i8, ptr %6, i64 8
  %31 = getelementptr inbounds nuw i8, ptr %29, i64 %16
  %32 = getelementptr inbounds nuw i8, ptr %6, i64 16
  store ptr %31, ptr %32, align 8, !tbaa !18
  tail call void @llvm.memset.p0.i64(ptr nonnull align 1 %29, i8 0, i64 %16, i1 false)
  store ptr %31, ptr %30, align 8, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %7, i8 0, i64 24, i1 false)
  %33 = icmp samesign ugt i64 %16, 2305843009213693951
  br i1 %33, label %34, label %36

34:                                               ; preds = %28
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.3) #16
          to label %35 unwind label %73

35:                                               ; preds = %34
  unreachable

36:                                               ; preds = %28
  %37 = getelementptr inbounds nuw i8, ptr %7, i64 16
  %38 = shl nuw nsw i64 %16, 2
  %39 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %38) #15
          to label %40 unwind label %73

40:                                               ; preds = %36
  %41 = getelementptr inbounds nuw i8, ptr %7, i64 8
  store ptr %39, ptr %7, align 8, !tbaa !20
  store ptr %39, ptr %41, align 8, !tbaa !22
  %42 = getelementptr inbounds nuw i32, ptr %39, i64 %16
  store ptr %42, ptr %37, align 8, !tbaa !23
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #14
  %43 = getelementptr inbounds nuw i8, ptr %8, i64 8
  store i64 0, ptr %43, align 8
  %44 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %38) #15
          to label %45 unwind label %75

45:                                               ; preds = %40
  store ptr %44, ptr %8, align 8, !tbaa !20
  %46 = getelementptr inbounds nuw i32, ptr %44, i64 %16
  %47 = getelementptr inbounds nuw i8, ptr %8, i64 16
  store ptr %46, ptr %47, align 8, !tbaa !23
  store i32 0, ptr %44, align 4, !tbaa !24
  %48 = getelementptr i8, ptr %44, i64 4
  %49 = icmp eq i64 %16, 1
  br i1 %49, label %57, label %50

50:                                               ; preds = %45
  %51 = getelementptr i8, ptr %44, i64 %38
  %52 = add nsw i64 %38, -4
  tail call void @llvm.memset.p0.i64(ptr align 4 %48, i8 0, i64 %52, i1 false), !tbaa !24
  %53 = getelementptr inbounds nuw i8, ptr %8, i64 8
  store ptr %51, ptr %53, align 8, !tbaa !22
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %9) #14
  store ptr %6, ptr %9, align 8, !tbaa !26
  %54 = getelementptr inbounds nuw i8, ptr %9, i64 8
  store ptr %7, ptr %54, align 8, !tbaa !28
  %55 = getelementptr inbounds nuw i8, ptr %9, i64 16
  store ptr %5, ptr %55, align 8, !tbaa !30
  %56 = getelementptr inbounds nuw i8, ptr %9, i64 24
  store ptr %8, ptr %56, align 8, !tbaa !28
  br i1 %19, label %67, label %71

57:                                               ; preds = %45
  %58 = getelementptr inbounds nuw i8, ptr %8, i64 8
  store ptr %48, ptr %58, align 8, !tbaa !22
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %9) #14
  store ptr %6, ptr %9, align 8, !tbaa !26
  %59 = getelementptr inbounds nuw i8, ptr %9, i64 8
  store ptr %7, ptr %59, align 8, !tbaa !28
  %60 = getelementptr inbounds nuw i8, ptr %9, i64 16
  store ptr %5, ptr %60, align 8, !tbaa !30
  %61 = getelementptr inbounds nuw i8, ptr %9, i64 24
  store ptr %8, ptr %61, align 8, !tbaa !28
  br i1 %19, label %69, label %71

62:                                               ; preds = %26
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %6, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %7) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %7, i8 0, i64 24, i1 false)
  %63 = getelementptr inbounds nuw i8, ptr %7, i64 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %8) #14
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(24) %8, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %9) #14
  store ptr %6, ptr %9, align 8, !tbaa !26
  %64 = getelementptr inbounds nuw i8, ptr %9, i64 8
  store ptr %7, ptr %64, align 8, !tbaa !28
  %65 = getelementptr inbounds nuw i8, ptr %9, i64 16
  store ptr %5, ptr %65, align 8, !tbaa !30
  %66 = getelementptr inbounds nuw i8, ptr %9, i64 24
  store ptr %8, ptr %66, align 8, !tbaa !28
  br i1 %19, label %92, label %71

67:                                               ; preds = %50
  %68 = icmp sgt i32 %20, 0
  br i1 %68, label %69, label %92

69:                                               ; preds = %57, %67
  %70 = and i64 %16, 2147483647
  br label %79

71:                                               ; preds = %57, %62, %50
  %72 = phi ptr [ %63, %62 ], [ %37, %50 ], [ %37, %57 ]
  invoke fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr noundef nonnull align 8 dereferenceable(32) %9, i32 noundef %1)
          to label %92 unwind label %77

73:                                               ; preds = %36, %34
  %74 = landingpad { ptr, i32 }
          cleanup
  br label %209

75:                                               ; preds = %40
  %76 = landingpad { ptr, i32 }
          cleanup
  br label %207

77:                                               ; preds = %71
  %78 = landingpad { ptr, i32 }
          cleanup
  br label %197

79:                                               ; preds = %69, %89
  %80 = phi i64 [ 0, %69 ], [ %90, %89 ]
  %81 = load ptr, ptr %6, align 8, !tbaa !15
  %82 = getelementptr inbounds nuw i8, ptr %81, i64 %80
  %83 = load i8, ptr %82, align 1, !tbaa !32
  %84 = icmp eq i8 %83, 0
  br i1 %84, label %85, label %89

85:                                               ; preds = %79
  %86 = trunc nuw nsw i64 %80 to i32
  invoke fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr noundef nonnull align 8 dereferenceable(32) %9, i32 noundef %86)
          to label %89 unwind label %87

87:                                               ; preds = %85
  %88 = landingpad { ptr, i32 }
          cleanup
  br label %197

89:                                               ; preds = %79, %85
  %90 = add nuw nsw i64 %80, 1
  %91 = icmp eq i64 %90, %70
  br i1 %91, label %92, label %79, !llvm.loop !33

92:                                               ; preds = %89, %62, %67, %71
  %93 = phi ptr [ %37, %67 ], [ %72, %71 ], [ %63, %62 ], [ %37, %89 ]
  %94 = load ptr, ptr %7, align 8, !tbaa !35
  %95 = getelementptr inbounds nuw i8, ptr %7, i64 8
  %96 = load ptr, ptr %95, align 8, !tbaa !35
  %97 = icmp eq ptr %94, %96
  br i1 %97, label %98, label %99

98:                                               ; preds = %92
  store ptr null, ptr %2, align 8, !tbaa !35
  br label %166

99:                                               ; preds = %92
  %100 = ptrtoint ptr %96 to i64
  %101 = ptrtoint ptr %94 to i64
  %102 = sub i64 %100, %101
  %103 = ashr exact i64 %102, 2
  %104 = icmp sgt i64 %103, -1
  call void @llvm.assume(i1 %104)
  %105 = call noalias ptr @malloc(i64 noundef %102) #17
  %106 = icmp eq ptr %105, null
  br i1 %106, label %168, label %107

107:                                              ; preds = %99
  %108 = ptrtoint ptr %105 to i64
  %109 = call i64 @llvm.umax.i64(i64 %103, i64 1)
  %110 = icmp samesign ult i64 %103, 8
  %111 = sub i64 %108, %101
  %112 = icmp ult i64 %111, 32
  %113 = or i1 %110, %112
  br i1 %113, label %128, label %114

114:                                              ; preds = %107
  %115 = and i64 %109, 9223372036854775800
  br label %116

116:                                              ; preds = %116, %114
  %117 = phi i64 [ 0, %114 ], [ %124, %116 ]
  %118 = getelementptr inbounds nuw i32, ptr %94, i64 %117
  %119 = getelementptr inbounds nuw i8, ptr %118, i64 16
  %120 = load <4 x i32>, ptr %118, align 4, !tbaa !24
  %121 = load <4 x i32>, ptr %119, align 4, !tbaa !24
  %122 = getelementptr inbounds nuw i32, ptr %105, i64 %117
  %123 = getelementptr inbounds nuw i8, ptr %122, i64 16
  store <4 x i32> %120, ptr %122, align 4, !tbaa !24
  store <4 x i32> %121, ptr %123, align 4, !tbaa !24
  %124 = add nuw i64 %117, 8
  %125 = icmp eq i64 %124, %115
  br i1 %125, label %126, label %116, !llvm.loop !36

126:                                              ; preds = %116
  %127 = icmp eq i64 %103, %115
  br i1 %127, label %145, label %128

128:                                              ; preds = %107, %126
  %129 = phi i64 [ 0, %107 ], [ %115, %126 ]
  %130 = and i64 %109, 3
  %131 = icmp eq i64 %130, 0
  br i1 %131, label %141, label %132

132:                                              ; preds = %128, %132
  %133 = phi i64 [ %138, %132 ], [ %129, %128 ]
  %134 = phi i64 [ %139, %132 ], [ 0, %128 ]
  %135 = getelementptr inbounds nuw i32, ptr %94, i64 %133
  %136 = load i32, ptr %135, align 4, !tbaa !24
  %137 = getelementptr inbounds nuw i32, ptr %105, i64 %133
  store i32 %136, ptr %137, align 4, !tbaa !24
  %138 = add nuw nsw i64 %133, 1
  %139 = add i64 %134, 1
  %140 = icmp eq i64 %139, %130
  br i1 %140, label %141, label %132, !llvm.loop !39

141:                                              ; preds = %132, %128
  %142 = phi i64 [ %129, %128 ], [ %138, %132 ]
  %143 = sub nsw i64 %129, %109
  %144 = icmp ugt i64 %143, -4
  br i1 %144, label %145, label %147

145:                                              ; preds = %141, %147, %126
  store ptr %105, ptr %2, align 8, !tbaa !35
  %146 = trunc i64 %103 to i32
  br label %166

147:                                              ; preds = %141, %147
  %148 = phi i64 [ %164, %147 ], [ %142, %141 ]
  %149 = getelementptr inbounds nuw i32, ptr %94, i64 %148
  %150 = load i32, ptr %149, align 4, !tbaa !24
  %151 = getelementptr inbounds nuw i32, ptr %105, i64 %148
  store i32 %150, ptr %151, align 4, !tbaa !24
  %152 = add nuw nsw i64 %148, 1
  %153 = getelementptr inbounds nuw i32, ptr %94, i64 %152
  %154 = load i32, ptr %153, align 4, !tbaa !24
  %155 = getelementptr inbounds nuw i32, ptr %105, i64 %152
  store i32 %154, ptr %155, align 4, !tbaa !24
  %156 = add nuw nsw i64 %148, 2
  %157 = getelementptr inbounds nuw i32, ptr %94, i64 %156
  %158 = load i32, ptr %157, align 4, !tbaa !24
  %159 = getelementptr inbounds nuw i32, ptr %105, i64 %156
  store i32 %158, ptr %159, align 4, !tbaa !24
  %160 = add nuw nsw i64 %148, 3
  %161 = getelementptr inbounds nuw i32, ptr %94, i64 %160
  %162 = load i32, ptr %161, align 4, !tbaa !24
  %163 = getelementptr inbounds nuw i32, ptr %105, i64 %160
  store i32 %162, ptr %163, align 4, !tbaa !24
  %164 = add nuw nsw i64 %148, 4
  %165 = icmp eq i64 %103, %164
  br i1 %165, label %145, label %147, !llvm.loop !41

166:                                              ; preds = %145, %98
  %167 = phi i32 [ 0, %98 ], [ %146, %145 ]
  store i32 %167, ptr %3, align 4, !tbaa !24
  br label %168

168:                                              ; preds = %166, %99
  %169 = phi i32 [ 4, %99 ], [ 0, %166 ]
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %9) #14
  %170 = load ptr, ptr %8, align 8, !tbaa !20
  %171 = icmp eq ptr %170, null
  br i1 %171, label %179, label %172

172:                                              ; preds = %168
  %173 = getelementptr inbounds nuw i8, ptr %8, i64 16
  %174 = load ptr, ptr %173, align 8, !tbaa !23
  %175 = ptrtoint ptr %174 to i64
  %176 = ptrtoint ptr %170 to i64
  %177 = sub i64 %175, %176
  call void @_ZdlPvm(ptr noundef nonnull %170, i64 noundef %177) #18
  %178 = load ptr, ptr %7, align 8, !tbaa !20
  br label %179

179:                                              ; preds = %168, %172
  %180 = phi ptr [ %94, %168 ], [ %178, %172 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #14
  %181 = icmp eq ptr %180, null
  br i1 %181, label %187, label %182

182:                                              ; preds = %179
  %183 = load ptr, ptr %93, align 8, !tbaa !23
  %184 = ptrtoint ptr %183 to i64
  %185 = ptrtoint ptr %180 to i64
  %186 = sub i64 %184, %185
  call void @_ZdlPvm(ptr noundef nonnull %180, i64 noundef %186) #18
  br label %187

187:                                              ; preds = %179, %182
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #14
  %188 = load ptr, ptr %6, align 8, !tbaa !15
  %189 = icmp eq ptr %188, null
  br i1 %189, label %196, label %190

190:                                              ; preds = %187
  %191 = getelementptr inbounds nuw i8, ptr %6, i64 16
  %192 = load ptr, ptr %191, align 8, !tbaa !18
  %193 = ptrtoint ptr %192 to i64
  %194 = ptrtoint ptr %188 to i64
  %195 = sub i64 %193, %194
  call void @_ZdlPvm(ptr noundef nonnull %188, i64 noundef %195) #18
  br label %196

196:                                              ; preds = %187, %190
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #14
  br label %229

197:                                              ; preds = %87, %77
  %198 = phi { ptr, i32 } [ %78, %77 ], [ %88, %87 ]
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %9) #14
  %199 = load ptr, ptr %8, align 8, !tbaa !20
  %200 = icmp eq ptr %199, null
  br i1 %200, label %207, label %201

201:                                              ; preds = %197
  %202 = getelementptr inbounds nuw i8, ptr %8, i64 16
  %203 = load ptr, ptr %202, align 8, !tbaa !23
  %204 = ptrtoint ptr %203 to i64
  %205 = ptrtoint ptr %199 to i64
  %206 = sub i64 %204, %205
  call void @_ZdlPvm(ptr noundef nonnull %199, i64 noundef %206) #18
  br label %207

207:                                              ; preds = %201, %197, %75
  %208 = phi { ptr, i32 } [ %76, %75 ], [ %198, %197 ], [ %198, %201 ]
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %8) #14
  br label %209

209:                                              ; preds = %207, %73
  %210 = phi { ptr, i32 } [ %208, %207 ], [ %74, %73 ]
  %211 = load ptr, ptr %7, align 8, !tbaa !20
  %212 = icmp eq ptr %211, null
  br i1 %212, label %219, label %213

213:                                              ; preds = %209
  %214 = getelementptr inbounds nuw i8, ptr %7, i64 16
  %215 = load ptr, ptr %214, align 8, !tbaa !23
  %216 = ptrtoint ptr %215 to i64
  %217 = ptrtoint ptr %211 to i64
  %218 = sub i64 %216, %217
  call void @_ZdlPvm(ptr noundef nonnull %211, i64 noundef %218) #18
  br label %219

219:                                              ; preds = %209, %213
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %7) #14
  %220 = load ptr, ptr %6, align 8, !tbaa !15
  %221 = icmp eq ptr %220, null
  br i1 %221, label %228, label %222

222:                                              ; preds = %219
  %223 = getelementptr inbounds nuw i8, ptr %6, i64 16
  %224 = load ptr, ptr %223, align 8, !tbaa !18
  %225 = ptrtoint ptr %224 to i64
  %226 = ptrtoint ptr %220 to i64
  %227 = sub i64 %225, %226
  call void @_ZdlPvm(ptr noundef nonnull %220, i64 noundef %227) #18
  br label %228

228:                                              ; preds = %222, %219
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %6) #14
  resume { ptr, i32 } %210

229:                                              ; preds = %23, %196, %15, %4
  %230 = phi i32 [ 1, %4 ], [ 3, %23 ], [ %169, %196 ], [ 2, %15 ]
  ret i32 %230
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: inlinehint mustprogress uwtable
define internal fastcc void @"_ZZ15dfs_runtime_srcENK3$_0clEi"(ptr nocapture noundef nonnull readonly align 8 dereferenceable(32) %0, i32 noundef %1) unnamed_addr #3 align 2 personality ptr @__gxx_personality_v0 {
  %3 = alloca i64, align 8
  %4 = tail call noalias noundef nonnull dereferenceable(256) ptr @_Znwm(i64 noundef 256) #15
  %5 = getelementptr inbounds nuw i8, ptr %4, i64 256
  store i32 %1, ptr %4, align 4, !tbaa !24
  %6 = getelementptr inbounds nuw i8, ptr %4, i64 4
  %7 = getelementptr inbounds nuw i8, ptr %0, i64 8
  %8 = getelementptr inbounds nuw i8, ptr %0, i64 16
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 24
  br label %10

10:                                               ; preds = %2, %126
  %11 = phi ptr [ %4, %2 ], [ %129, %126 ]
  %12 = phi ptr [ %6, %2 ], [ %128, %126 ]
  %13 = phi ptr [ %5, %2 ], [ %127, %126 ]
  %14 = getelementptr inbounds i8, ptr %12, i64 -4
  %15 = load i32, ptr %14, align 4, !tbaa !24
  %16 = load ptr, ptr %0, align 8, !tbaa !42
  %17 = sext i32 %15 to i64
  %18 = load ptr, ptr %16, align 8, !tbaa !15
  %19 = getelementptr inbounds nuw i8, ptr %18, i64 %17
  %20 = load i8, ptr %19, align 1, !tbaa !32
  %21 = icmp eq i8 %20, 0
  br i1 %21, label %22, label %126, !llvm.loop !44

22:                                               ; preds = %10
  store i8 1, ptr %19, align 1, !tbaa !32
  %23 = load ptr, ptr %7, align 8, !tbaa !45
  %24 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %25 = load ptr, ptr %24, align 8, !tbaa !22
  %26 = getelementptr inbounds nuw i8, ptr %23, i64 16
  %27 = load ptr, ptr %26, align 8, !tbaa !23
  %28 = icmp eq ptr %25, %27
  br i1 %28, label %31, label %29

29:                                               ; preds = %22
  store i32 %15, ptr %25, align 4, !tbaa !24
  %30 = getelementptr inbounds nuw i8, ptr %25, i64 4
  store ptr %30, ptr %24, align 8, !tbaa !22
  br label %57

31:                                               ; preds = %22
  %32 = load ptr, ptr %23, align 8, !tbaa !20
  %33 = ptrtoint ptr %25 to i64
  %34 = ptrtoint ptr %32 to i64
  %35 = sub i64 %33, %34
  %36 = ashr exact i64 %35, 2
  %37 = icmp sgt i64 %36, -1
  call void @llvm.assume(i1 %37)
  %38 = icmp eq i64 %35, 9223372036854775804
  br i1 %38, label %39, label %41

39:                                               ; preds = %31
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #16
          to label %40 unwind label %71

40:                                               ; preds = %39
  unreachable

41:                                               ; preds = %31
  %42 = call i64 @llvm.umax.i64(i64 %36, i64 1)
  %43 = add nuw nsw i64 %42, %36
  %44 = call noundef i64 @llvm.umin.i64(i64 %43, i64 2305843009213693951)
  %45 = shl nuw nsw i64 %44, 2
  %46 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %45) #15
          to label %47 unwind label %69

47:                                               ; preds = %41
  %48 = getelementptr inbounds i8, ptr %46, i64 %35
  store i32 %15, ptr %48, align 4, !tbaa !24
  %49 = icmp sgt i64 %35, 0
  br i1 %49, label %50, label %51

50:                                               ; preds = %47
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %46, ptr align 4 %32, i64 %35, i1 false)
  br label %51

51:                                               ; preds = %50, %47
  %52 = icmp eq ptr %32, null
  br i1 %52, label %54, label %53

53:                                               ; preds = %51
  call void @_ZdlPvm(ptr noundef nonnull %32, i64 noundef %35) #18
  br label %54

54:                                               ; preds = %53, %51
  %55 = getelementptr inbounds nuw i8, ptr %48, i64 4
  store ptr %46, ptr %23, align 8, !tbaa !20
  store ptr %55, ptr %24, align 8, !tbaa !22
  %56 = getelementptr inbounds nuw i32, ptr %46, i64 %44
  store ptr %56, ptr %26, align 8, !tbaa !23
  br label %57

57:                                               ; preds = %54, %29
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %3) #14
  store i64 0, ptr %3, align 8, !tbaa !46
  %58 = load ptr, ptr %8, align 8, !tbaa !47
  %59 = load ptr, ptr %58, align 8, !tbaa !3
  %60 = load ptr, ptr %9, align 8, !tbaa !48
  %61 = load ptr, ptr %60, align 8, !tbaa !20
  invoke void @autograph_get_neighbors(ptr noundef %59, i64 noundef %17, ptr noundef %61, ptr noundef nonnull %3)
          to label %62 unwind label %73

62:                                               ; preds = %57
  %63 = load i64, ptr %3, align 8, !tbaa !46
  %64 = icmp sgt i64 %63, 0
  br i1 %64, label %75, label %65

65:                                               ; preds = %121, %62
  %66 = phi ptr [ %13, %62 ], [ %122, %121 ]
  %67 = phi ptr [ %14, %62 ], [ %123, %121 ]
  %68 = phi ptr [ %11, %62 ], [ %124, %121 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #14
  br label %126

69:                                               ; preds = %41
  %70 = landingpad { ptr, i32 }
          cleanup
  br label %139

71:                                               ; preds = %39
  %72 = landingpad { ptr, i32 }
          cleanup
  br label %139

73:                                               ; preds = %57
  %74 = landingpad { ptr, i32 }
          cleanup
  br label %131

75:                                               ; preds = %62, %121
  %76 = phi i64 [ %80, %121 ], [ %63, %62 ]
  %77 = phi ptr [ %124, %121 ], [ %11, %62 ]
  %78 = phi ptr [ %123, %121 ], [ %14, %62 ]
  %79 = phi ptr [ %122, %121 ], [ %13, %62 ]
  %80 = add nsw i64 %76, -1
  %81 = load ptr, ptr %9, align 8, !tbaa !48
  %82 = load ptr, ptr %81, align 8, !tbaa !20
  %83 = getelementptr inbounds nuw i32, ptr %82, i64 %80
  %84 = load i32, ptr %83, align 4, !tbaa !24
  %85 = load ptr, ptr %0, align 8, !tbaa !42
  %86 = sext i32 %84 to i64
  %87 = load ptr, ptr %85, align 8, !tbaa !15
  %88 = getelementptr inbounds nuw i8, ptr %87, i64 %86
  %89 = load i8, ptr %88, align 1, !tbaa !32
  %90 = icmp eq i8 %89, 0
  br i1 %90, label %91, label %121

91:                                               ; preds = %75
  %92 = icmp eq ptr %78, %79
  br i1 %92, label %95, label %93

93:                                               ; preds = %91
  store i32 %84, ptr %78, align 4, !tbaa !24
  %94 = getelementptr inbounds nuw i8, ptr %78, i64 4
  br label %121

95:                                               ; preds = %91
  %96 = ptrtoint ptr %78 to i64
  %97 = ptrtoint ptr %77 to i64
  %98 = sub i64 %96, %97
  %99 = ashr exact i64 %98, 2
  %100 = icmp sgt i64 %99, -1
  call void @llvm.assume(i1 %100)
  %101 = icmp eq i64 %98, 9223372036854775804
  br i1 %101, label %102, label %104

102:                                              ; preds = %95
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef nonnull @.str.1) #16
          to label %103 unwind label %119

103:                                              ; preds = %102
  unreachable

104:                                              ; preds = %95
  %105 = call i64 @llvm.umax.i64(i64 %99, i64 1)
  %106 = add nuw nsw i64 %105, %99
  %107 = call noundef i64 @llvm.umin.i64(i64 %106, i64 2305843009213693951)
  %108 = shl nuw nsw i64 %107, 2
  %109 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %108) #15
          to label %110 unwind label %117

110:                                              ; preds = %104
  %111 = getelementptr inbounds i8, ptr %109, i64 %98
  store i32 %84, ptr %111, align 4, !tbaa !24
  %112 = icmp sgt i64 %98, 0
  br i1 %112, label %113, label %114

113:                                              ; preds = %110
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 4 %109, ptr align 4 %77, i64 %98, i1 false)
  br label %114

114:                                              ; preds = %113, %110
  call void @_ZdlPvm(ptr noundef nonnull %77, i64 noundef %98) #18
  %115 = getelementptr inbounds nuw i8, ptr %111, i64 4
  %116 = getelementptr inbounds nuw i32, ptr %109, i64 %107
  br label %121

117:                                              ; preds = %104
  %118 = landingpad { ptr, i32 }
          cleanup
  br label %131

119:                                              ; preds = %102
  %120 = landingpad { ptr, i32 }
          cleanup
  br label %131

121:                                              ; preds = %114, %93, %75
  %122 = phi ptr [ %79, %75 ], [ %116, %114 ], [ %79, %93 ]
  %123 = phi ptr [ %78, %75 ], [ %115, %114 ], [ %94, %93 ]
  %124 = phi ptr [ %77, %75 ], [ %109, %114 ], [ %77, %93 ]
  %125 = icmp samesign ugt i64 %76, 1
  br i1 %125, label %75, label %65, !llvm.loop !49

126:                                              ; preds = %10, %65
  %127 = phi ptr [ %66, %65 ], [ %13, %10 ]
  %128 = phi ptr [ %67, %65 ], [ %14, %10 ]
  %129 = phi ptr [ %68, %65 ], [ %11, %10 ]
  %130 = icmp eq ptr %129, %128
  br i1 %130, label %135, label %10

131:                                              ; preds = %117, %119, %73
  %132 = phi ptr [ %13, %73 ], [ %78, %117 ], [ %78, %119 ]
  %133 = phi ptr [ %11, %73 ], [ %77, %117 ], [ %77, %119 ]
  %134 = phi { ptr, i32 } [ %74, %73 ], [ %118, %117 ], [ %120, %119 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %3) #14
  br label %139

135:                                              ; preds = %126
  %136 = ptrtoint ptr %127 to i64
  %137 = ptrtoint ptr %128 to i64
  %138 = sub i64 %136, %137
  call void @_ZdlPvm(ptr noundef nonnull %129, i64 noundef %138) #18
  ret void

139:                                              ; preds = %69, %71, %131
  %140 = phi ptr [ %132, %131 ], [ %13, %69 ], [ %13, %71 ]
  %141 = phi ptr [ %133, %131 ], [ %11, %69 ], [ %11, %71 ]
  %142 = phi { ptr, i32 } [ %134, %131 ], [ %70, %69 ], [ %72, %71 ]
  %143 = icmp eq ptr %141, null
  br i1 %143, label %148, label %144

144:                                              ; preds = %139
  %145 = ptrtoint ptr %140 to i64
  %146 = ptrtoint ptr %141 to i64
  %147 = sub i64 %145, %146
  call void @_ZdlPvm(ptr noundef nonnull %141, i64 noundef %147) #18
  br label %148

148:                                              ; preds = %139, %144
  resume { ptr, i32 } %142
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #4

declare dso_local void @autograph_get_neighbors(ptr noundef, i64 noundef, ptr noundef, ptr noundef) local_unnamed_addr #5

; Function Attrs: cold noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #6

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noalias noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #8

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPvm(ptr noundef, i64 noundef) local_unnamed_addr #9

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #10

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #11

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #12

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #12

attributes #0 = { mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { inlinehint mustprogress uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { cold noreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { nobuiltin allocsize(0) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #9 = { nobuiltin nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #10 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #11 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #12 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #13 = { cold nounwind }
attributes #14 = { nounwind }
attributes #15 = { builtin allocsize(0) }
attributes #16 = { cold noreturn }
attributes #17 = { nounwind allocsize(0) }
attributes #18 = { builtin nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"clang version 20.1.8 (Fedora 20.1.8-4.fc42)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"p1 _ZTS5Graph", !5, i64 0}
!5 = !{!"any pointer", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"_ZTS5Graph", !10, i64 0, !10, i64 8, !11, i64 16, !12, i64 24}
!10 = !{!"long", !6, i64 0}
!11 = !{!"p1 long", !5, i64 0}
!12 = !{!"p1 int", !5, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"p1 _ZTS8_IO_FILE", !5, i64 0}
!15 = !{!16, !17, i64 0}
!16 = !{!"_ZTSNSt12_Vector_baseIcSaIcEE17_Vector_impl_dataE", !17, i64 0, !17, i64 8, !17, i64 16}
!17 = !{!"p1 omnipotent char", !5, i64 0}
!18 = !{!16, !17, i64 16}
!19 = !{!16, !17, i64 8}
!20 = !{!21, !12, i64 0}
!21 = !{!"_ZTSNSt12_Vector_baseIiSaIiEE17_Vector_impl_dataE", !12, i64 0, !12, i64 8, !12, i64 16}
!22 = !{!21, !12, i64 8}
!23 = !{!21, !12, i64 16}
!24 = !{!25, !25, i64 0}
!25 = !{!"int", !6, i64 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"p1 _ZTSSt6vectorIcSaIcEE", !5, i64 0}
!28 = !{!29, !29, i64 0}
!29 = !{!"p1 _ZTSSt6vectorIiSaIiEE", !5, i64 0}
!30 = !{!31, !31, i64 0}
!31 = !{!"p2 _ZTS5Graph", !5, i64 0}
!32 = !{!6, !6, i64 0}
!33 = distinct !{!33, !34}
!34 = !{!"llvm.loop.mustprogress"}
!35 = !{!12, !12, i64 0}
!36 = distinct !{!36, !34, !37, !38}
!37 = !{!"llvm.loop.isvectorized", i32 1}
!38 = !{!"llvm.loop.unroll.runtime.disable"}
!39 = distinct !{!39, !40}
!40 = !{!"llvm.loop.unroll.disable"}
!41 = distinct !{!41, !34, !37}
!42 = !{!43, !27, i64 0}
!43 = !{!"_ZTSZ15dfs_runtime_srcE3$_0", !27, i64 0, !29, i64 8, !31, i64 16, !29, i64 24}
!44 = distinct !{!44, !34}
!45 = !{!43, !29, i64 8}
!46 = !{!10, !10, i64 0}
!47 = !{!43, !31, i64 16}
!48 = !{!43, !29, i64 24}
!49 = distinct !{!49, !34}
