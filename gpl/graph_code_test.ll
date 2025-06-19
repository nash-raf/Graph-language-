; ModuleID = 'gpl_module'
source_filename = "gpl_module"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @global__init__, ptr null }]

define void @global__init__() {
entry:
  %i = alloca i32, align 4
  %it = alloca i32, align 4
  store i32 0, ptr %it, align 4
  %arr = alloca [100000 x i32], align 4
  %arr2 = alloca [100000 x i32], align 4
  %arr3 = alloca [100000 x i32], align 4
  br label %loopcond

loopcond:                                         ; preds = %loopbody, %entry
  %it_val = load i32, ptr %it, align 4
  %lttmp = icmp slt i32 %it_val, 100000
  br i1 %lttmp, label %loopbody, label %afterloop

loopbody:                                         ; preds = %loopcond
  %it_val1 = load i32, ptr %it, align 4
  %it_val2 = load i32, ptr %it, align 4
  %add_tmp = add i32 %it_val2, 1
  %arr_elem_ptr = getelementptr [100000 x i32], ptr %arr, i32 0, i32 %it_val1
  store i32 %add_tmp, ptr %arr_elem_ptr, align 4
  %it_val3 = load i32, ptr %it, align 4
  %it_val4 = load i32, ptr %it, align 4
  %add_tmp5 = add i32 %it_val4, 100
  %arr2_elem_ptr = getelementptr [100000 x i32], ptr %arr2, i32 0, i32 %it_val3
  store i32 %add_tmp5, ptr %arr2_elem_ptr, align 4
  %it_val6 = load i32, ptr %it, align 4
  %add_tmp7 = add i32 %it_val6, 1
  store i32 %add_tmp7, ptr %it, align 4
  br label %loopcond

afterloop:                                        ; preds = %loopcond
  store i32 0, ptr %i, align 4
  br label %loopcond8

loopcond8:                                        ; preds = %loopbody9, %afterloop
  %i_val = load i32, ptr %i, align 4
  %lttmp11 = icmp slt i32 %i_val, 100000
  br i1 %lttmp11, label %loopbody9, label %afterloop10

loopbody9:                                        ; preds = %loopcond8
  %i_val12 = load i32, ptr %i, align 4
  %i_val13 = load i32, ptr %i, align 4
  %arr2_elem_ptr14 = getelementptr [100000 x i32], ptr %arr2, i32 0, i32 %i_val13
  %arr2_val = load i32, ptr %arr2_elem_ptr14, align 4
  %i_val15 = load i32, ptr %i, align 4
  %arr_elem_ptr16 = getelementptr [100000 x i32], ptr %arr, i32 0, i32 %i_val15
  %arr_val = load i32, ptr %arr_elem_ptr16, align 4
  %add_tmp17 = add i32 %arr2_val, %arr_val
  %arr3_elem_ptr = getelementptr [100000 x i32], ptr %arr3, i32 0, i32 %i_val12
  store i32 %add_tmp17, ptr %arr3_elem_ptr, align 4
  %i_val18 = load i32, ptr %i, align 4
  %add_tmp19 = add i32 %i_val18, 1
  store i32 %add_tmp19, ptr %i, align 4
  br label %loopcond8

afterloop10:                                      ; preds = %loopcond8
  ret void
}

