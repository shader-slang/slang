//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[shader("compute")]
[numthreads(1, 1, 1)]
void fill<T : __BuiltinArithmeticType>(RWStructuredBuffer<T> output)
{
    output[0] = T(6);
}

[host]
public void main(RWStructuredBuffer<float> output)
{
    dispatch(fill<float>, uint3(1, 1, 1))(output);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_FILL:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 4 "fill"
// CHECK-DAG: [[STR_FLOAT:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 5 "float"
// CHECK-DAG: [[BLOB:%[A-Za-z0-9_]+]] = OpBinaryLit {{[0-9]+}} { ... }

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[FLOAT:%[A-Za-z0-9_]+]] = OpFloat32Type
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[FLOAT]]
// CHECK-DAG: [[VEC3:%[A-Za-z0-9_]+]] = OpVectorType 3 [[UINT]]
// CHECK-DAG: [[FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[BUFFER]]
// CHECK-DAG: [[DISPATCH_FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[VEC3]] [[BUFFER]]

// CHECK: Section("FUNC")
// CHECK-DAG: OpCall [[DISPATCHER:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}

// CHECK: Section("EMBD")
// CHECK-DAG: [[MODULE:%[A-Za-z0-9_]+]] = OpSlangModule {{%[A-Za-z0-9_]+}} [[BLOB]]
// CHECK-DAG: [[FILL_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint [[MODULE]] [[STR_FILL]]
// CHECK-DAG: [[SPECIALIZED_FILL:%[A-Za-z0-9_]+]] = OpSpecializeSlangEntryPoint [[FILL_ENTRY]] Type [[STR_FLOAT]]
// CHECK-DAG: [[DISPATCHER]] : [[DISPATCH_FUNC_TYPE]] = OpMakeDispatcher Compute [[SPECIALIZED_FILL]]
