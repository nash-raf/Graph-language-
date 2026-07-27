; ModuleID = 'chromacity_runtime.bc'
source_filename = "chromacity_runtime.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

%struct.Graph = type { i64, i64, ptr, ptr }

@.str = private unnamed_addr constant [32 x i8] c"[Chromacity] color number = %d\0A\00", align 1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local i32 @chromacity_runtime(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i64, align 8
  %11 = alloca i32, align 4
  %12 = alloca i64, align 8
  %13 = alloca i32, align 4
  %14 = alloca ptr, align 8
  %15 = alloca i32, align 4
  %16 = alloca i64, align 8
  %17 = alloca i32, align 4
  %18 = alloca i64, align 8
  %19 = alloca i32, align 4
  %20 = alloca i32, align 4
  %21 = alloca i64, align 8
  %22 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %23 = load ptr, ptr %4, align 8
  %24 = icmp ne ptr %23, null
  br i1 %24, label %26, label %25

25:                                               ; preds = %2
  store i32 -1, ptr %3, align 4
  br label %226

26:                                               ; preds = %2
  %27 = load ptr, ptr %4, align 8
  %28 = getelementptr inbounds %struct.Graph, ptr %27, i32 0, i32 0
  %29 = load i64, ptr %28, align 8
  store i64 %29, ptr %6, align 8
  %30 = load ptr, ptr %4, align 8
  %31 = getelementptr inbounds %struct.Graph, ptr %30, i32 0, i32 2
  %32 = load ptr, ptr %31, align 8
  store ptr %32, ptr %7, align 8
  %33 = load ptr, ptr %4, align 8
  %34 = getelementptr inbounds %struct.Graph, ptr %33, i32 0, i32 3
  %35 = load ptr, ptr %34, align 8
  store ptr %35, ptr %8, align 8
  %36 = load i64, ptr %6, align 8
  %37 = icmp eq i64 %36, 0
  br i1 %37, label %38, label %39

38:                                               ; preds = %26
  store i32 0, ptr %3, align 4
  br label %226

39:                                               ; preds = %26
  %40 = load i64, ptr %6, align 8
  %41 = mul i64 %40, 4
  %42 = call noalias ptr @malloc(i64 noundef %41) #5
  store ptr %42, ptr %9, align 8
  store i64 0, ptr %10, align 8
  br label %43

43:                                               ; preds = %51, %39
  %44 = load i64, ptr %10, align 8
  %45 = load i64, ptr %6, align 8
  %46 = icmp slt i64 %44, %45
  br i1 %46, label %47, label %54

47:                                               ; preds = %43
  %48 = load ptr, ptr %9, align 8
  %49 = load i64, ptr %10, align 8
  %50 = getelementptr inbounds i32, ptr %48, i64 %49
  store i32 -1, ptr %50, align 4
  br label %51

51:                                               ; preds = %47
  %52 = load i64, ptr %10, align 8
  %53 = add nsw i64 %52, 1
  store i64 %53, ptr %10, align 8
  br label %43, !llvm.loop !4

54:                                               ; preds = %43
  store i32 0, ptr %11, align 4
  store i64 0, ptr %12, align 8
  br label %55

55:                                               ; preds = %77, %54
  %56 = load i64, ptr %12, align 8
  %57 = load i64, ptr %6, align 8
  %58 = icmp slt i64 %56, %57
  br i1 %58, label %59, label %80

59:                                               ; preds = %55
  %60 = load ptr, ptr %7, align 8
  %61 = load i64, ptr %12, align 8
  %62 = add nsw i64 %61, 1
  %63 = getelementptr inbounds i64, ptr %60, i64 %62
  %64 = load i64, ptr %63, align 8
  %65 = load ptr, ptr %7, align 8
  %66 = load i64, ptr %12, align 8
  %67 = getelementptr inbounds i64, ptr %65, i64 %66
  %68 = load i64, ptr %67, align 8
  %69 = sub nsw i64 %64, %68
  %70 = trunc i64 %69 to i32
  store i32 %70, ptr %13, align 4
  %71 = load i32, ptr %13, align 4
  %72 = load i32, ptr %11, align 4
  %73 = icmp sgt i32 %71, %72
  br i1 %73, label %74, label %76

74:                                               ; preds = %59
  %75 = load i32, ptr %13, align 4
  store i32 %75, ptr %11, align 4
  br label %76

76:                                               ; preds = %74, %59
  br label %77

77:                                               ; preds = %76
  %78 = load i64, ptr %12, align 8
  %79 = add nsw i64 %78, 1
  store i64 %79, ptr %12, align 8
  br label %55, !llvm.loop !6

80:                                               ; preds = %55
  %81 = load i32, ptr %11, align 4
  %82 = add nsw i32 %81, 1
  %83 = sext i32 %82 to i64
  %84 = mul i64 %83, 4
  %85 = call noalias ptr @malloc(i64 noundef %84) #5
  store ptr %85, ptr %14, align 8
  %86 = load ptr, ptr %14, align 8
  %87 = load i32, ptr %11, align 4
  %88 = add nsw i32 %87, 1
  %89 = sext i32 %88 to i64
  %90 = mul i64 %89, 4
  call void @llvm.memset.p0.i64(ptr align 4 %86, i8 0, i64 %90, i1 false)
  %91 = load ptr, ptr %9, align 8
  %92 = getelementptr inbounds i32, ptr %91, i64 0
  store i32 0, ptr %92, align 4
  store i32 0, ptr %15, align 4
  store i64 1, ptr %16, align 8
  br label %93

93:                                               ; preds = %210, %80
  %94 = load i64, ptr %16, align 8
  %95 = load i64, ptr %6, align 8
  %96 = icmp slt i64 %94, %95
  br i1 %96, label %97, label %213

97:                                               ; preds = %93
  %98 = load ptr, ptr %7, align 8
  %99 = load i64, ptr %16, align 8
  %100 = add nsw i64 %99, 1
  %101 = getelementptr inbounds i64, ptr %98, i64 %100
  %102 = load i64, ptr %101, align 8
  %103 = load ptr, ptr %7, align 8
  %104 = load i64, ptr %16, align 8
  %105 = getelementptr inbounds i64, ptr %103, i64 %104
  %106 = load i64, ptr %105, align 8
  %107 = sub nsw i64 %102, %106
  %108 = trunc i64 %107 to i32
  store i32 %108, ptr %17, align 4
  %109 = load ptr, ptr %7, align 8
  %110 = load i64, ptr %16, align 8
  %111 = getelementptr inbounds i64, ptr %109, i64 %110
  %112 = load i64, ptr %111, align 8
  store i64 %112, ptr %18, align 8
  br label %113

113:                                              ; preds = %142, %97
  %114 = load i64, ptr %18, align 8
  %115 = load ptr, ptr %7, align 8
  %116 = load i64, ptr %16, align 8
  %117 = add nsw i64 %116, 1
  %118 = getelementptr inbounds i64, ptr %115, i64 %117
  %119 = load i64, ptr %118, align 8
  %120 = icmp slt i64 %114, %119
  br i1 %120, label %121, label %145

121:                                              ; preds = %113
  %122 = load ptr, ptr %8, align 8
  %123 = load i64, ptr %18, align 8
  %124 = getelementptr inbounds i32, ptr %122, i64 %123
  %125 = load i32, ptr %124, align 4
  store i32 %125, ptr %19, align 4
  %126 = load ptr, ptr %9, align 8
  %127 = load i32, ptr %19, align 4
  %128 = sext i32 %127 to i64
  %129 = getelementptr inbounds i32, ptr %126, i64 %128
  %130 = load i32, ptr %129, align 4
  %131 = icmp ne i32 %130, -1
  br i1 %131, label %132, label %141

132:                                              ; preds = %121
  %133 = load ptr, ptr %14, align 8
  %134 = load ptr, ptr %9, align 8
  %135 = load i32, ptr %19, align 4
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds i32, ptr %134, i64 %136
  %138 = load i32, ptr %137, align 4
  %139 = sext i32 %138 to i64
  %140 = getelementptr inbounds i32, ptr %133, i64 %139
  store i32 1, ptr %140, align 4
  br label %141

141:                                              ; preds = %132, %121
  br label %142

142:                                              ; preds = %141
  %143 = load i64, ptr %18, align 8
  %144 = add nsw i64 %143, 1
  store i64 %144, ptr %18, align 8
  br label %113, !llvm.loop !7

145:                                              ; preds = %113
  store i32 0, ptr %20, align 4
  br label %146

146:                                              ; preds = %169, %145
  %147 = load i32, ptr %20, align 4
  %148 = load i32, ptr %11, align 4
  %149 = icmp sle i32 %147, %148
  br i1 %149, label %150, label %172

150:                                              ; preds = %146
  %151 = load ptr, ptr %14, align 8
  %152 = load i32, ptr %20, align 4
  %153 = sext i32 %152 to i64
  %154 = getelementptr inbounds i32, ptr %151, i64 %153
  %155 = load i32, ptr %154, align 4
  %156 = icmp eq i32 %155, 0
  br i1 %156, label %157, label %168

157:                                              ; preds = %150
  %158 = load i32, ptr %20, align 4
  %159 = load ptr, ptr %9, align 8
  %160 = load i64, ptr %16, align 8
  %161 = getelementptr inbounds i32, ptr %159, i64 %160
  store i32 %158, ptr %161, align 4
  %162 = load i32, ptr %20, align 4
  %163 = load i32, ptr %15, align 4
  %164 = icmp sgt i32 %162, %163
  br i1 %164, label %165, label %167

165:                                              ; preds = %157
  %166 = load i32, ptr %20, align 4
  store i32 %166, ptr %15, align 4
  br label %167

167:                                              ; preds = %165, %157
  br label %172

168:                                              ; preds = %150
  br label %169

169:                                              ; preds = %168
  %170 = load i32, ptr %20, align 4
  %171 = add nsw i32 %170, 1
  store i32 %171, ptr %20, align 4
  br label %146, !llvm.loop !8

172:                                              ; preds = %167, %146
  %173 = load ptr, ptr %7, align 8
  %174 = load i64, ptr %16, align 8
  %175 = getelementptr inbounds i64, ptr %173, i64 %174
  %176 = load i64, ptr %175, align 8
  store i64 %176, ptr %21, align 8
  br label %177

177:                                              ; preds = %206, %172
  %178 = load i64, ptr %21, align 8
  %179 = load ptr, ptr %7, align 8
  %180 = load i64, ptr %16, align 8
  %181 = add nsw i64 %180, 1
  %182 = getelementptr inbounds i64, ptr %179, i64 %181
  %183 = load i64, ptr %182, align 8
  %184 = icmp slt i64 %178, %183
  br i1 %184, label %185, label %209

185:                                              ; preds = %177
  %186 = load ptr, ptr %8, align 8
  %187 = load i64, ptr %21, align 8
  %188 = getelementptr inbounds i32, ptr %186, i64 %187
  %189 = load i32, ptr %188, align 4
  store i32 %189, ptr %22, align 4
  %190 = load ptr, ptr %9, align 8
  %191 = load i32, ptr %22, align 4
  %192 = sext i32 %191 to i64
  %193 = getelementptr inbounds i32, ptr %190, i64 %192
  %194 = load i32, ptr %193, align 4
  %195 = icmp ne i32 %194, -1
  br i1 %195, label %196, label %205

196:                                              ; preds = %185
  %197 = load ptr, ptr %14, align 8
  %198 = load ptr, ptr %9, align 8
  %199 = load i32, ptr %22, align 4
  %200 = sext i32 %199 to i64
  %201 = getelementptr inbounds i32, ptr %198, i64 %200
  %202 = load i32, ptr %201, align 4
  %203 = sext i32 %202 to i64
  %204 = getelementptr inbounds i32, ptr %197, i64 %203
  store i32 0, ptr %204, align 4
  br label %205

205:                                              ; preds = %196, %185
  br label %206

206:                                              ; preds = %205
  %207 = load i64, ptr %21, align 8
  %208 = add nsw i64 %207, 1
  store i64 %208, ptr %21, align 8
  br label %177, !llvm.loop !9

209:                                              ; preds = %177
  br label %210

210:                                              ; preds = %209
  %211 = load i64, ptr %16, align 8
  %212 = add nsw i64 %211, 1
  store i64 %212, ptr %16, align 8
  br label %93, !llvm.loop !10

213:                                              ; preds = %93
  %214 = load i32, ptr %15, align 4
  %215 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %214)
  %216 = load ptr, ptr %14, align 8
  call void @free(ptr noundef %216) #6
  %217 = load ptr, ptr %5, align 8
  %218 = icmp ne ptr %217, null
  br i1 %218, label %219, label %222

219:                                              ; preds = %213
  %220 = load ptr, ptr %9, align 8
  %221 = load ptr, ptr %5, align 8
  store ptr %220, ptr %221, align 8
  br label %224

222:                                              ; preds = %213
  %223 = load ptr, ptr %9, align 8
  call void @free(ptr noundef %223) #6
  br label %224

224:                                              ; preds = %222, %219
  %225 = load i32, ptr %15, align 4
  store i32 %225, ptr %3, align 4
  br label %226

226:                                              ; preds = %224, %38, %25
  %227 = load i32, ptr %3, align 4
  ret i32 %227
}

; Function Attrs: nounwind allocsize(0)
declare dso_local noalias ptr @malloc(i64 noundef) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

declare dso_local i32 @printf(ptr noundef, ...) #3

; Function Attrs: nounwind
declare dso_local void @free(ptr noundef) #4

attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #3 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind allocsize(0) }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 18.1.8 (Fedora 18.1.8-5.fc42)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5}
!9 = distinct !{!9, !5}
!10 = distinct !{!10, !5}
