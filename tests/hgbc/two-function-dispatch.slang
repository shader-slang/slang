//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[shader("compute")]
[numthreads(1, 1, 1)]
void fillIntermediate(RWStructuredBuffer<uint> output)
{
    output[0] = 40;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void finish(RWStructuredBuffer<uint> input, RWStructuredBuffer<uint> output)
{
    output[0] = input[0] + 2;
}

void runKernels(RWStructuredBuffer<uint> output)
{
    uint count;
    uint stride;
    output.GetDimensions(count, stride);
    let intermediate = RWStructuredBuffer<uint>::alloc(count);
    dispatch(fillIntermediate, uint3(1, 1, 1))(intermediate);
    dispatch(finish, uint3(1, 1, 1))(intermediate, output);
}

[host]
public void main(RWStructuredBuffer<uint> output)
{
    runKernels(output);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_FILL:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 16 "fillIntermediate"
// CHECK-DAG: [[STR_FINISH:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 6 "finish"
// CHECK-DAG: [[BLOB:%[A-Za-z0-9_]+]] = OpBinaryLit {{[0-9]+}} { ... }
// CHECK-DAG: [[ONE:%[A-Za-z0-9_]+]] : [[UINT:%[A-Za-z0-9_]+]] = OpIntLit 1

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT]] = OpUIntType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]
// CHECK-DAG: [[VEC3:%[A-Za-z0-9_]+]] = OpVectorType 3 [[UINT]]
// CHECK-DAG: [[VEC2:%[A-Za-z0-9_]+]] = OpVectorType 2 [[UINT]]
// CHECK-DAG: [[FUNC_TYPE_ONE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[BUFFER]]
// CHECK-DAG: [[DISPATCH_FUNC_TYPE_ONE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[VEC3]] [[BUFFER]]
// CHECK-DAG: [[DISPATCH_FUNC_TYPE_TWO:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[VEC3]] [[BUFFER]] [[BUFFER]]
// CHECK-DAG: [[ALLOC_FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[BUFFER]] [[UINT]]

// CHECK: Section("FUNC")
// CHECK: [[MAIN:%[A-Za-z0-9_]+]] : [[FUNC_TYPE_ONE]] = OpFunc
// CHECK: [[MAIN_OUTPUT:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpParam
// CHECK: OpCall [[HELPER:%[A-Za-z0-9_]+]] [[MAIN_OUTPUT]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
// CHECK: [[HELPER]] : [[FUNC_TYPE_ONE]] = OpFunc
// CHECK: [[HELPER_OUTPUT:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpParam
// CHECK: [[DIMS:%[A-Za-z0-9_]+]] : [[VEC2]] = OpStructuredBufferGetDimensions [[BUFFER]] [[HELPER_OUTPUT]]
// CHECK: [[COUNT:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[DIMS]]
// CHECK: [[HELPER_INTERMEDIATE:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpCall [[ALLOC_HELPER:%[A-Za-z0-9_]+]] [[COUNT]]
// CHECK: [[DISPATCH_SIZE:%[A-Za-z0-9_]+]] : [[VEC3]] = OpMakeVector [[ONE]] [[ONE]] [[ONE]]
// CHECK: OpCall [[FILL_DISPATCHER:%[A-Za-z0-9_]+]] [[DISPATCH_SIZE]] [[HELPER_INTERMEDIATE]]
// CHECK: OpCall [[FINISH_DISPATCHER:%[A-Za-z0-9_]+]] [[DISPATCH_SIZE]] [[HELPER_INTERMEDIATE]] [[HELPER_OUTPUT]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
// CHECK: [[ALLOC_HELPER]] : [[ALLOC_FUNC_TYPE]] = OpFunc
// CHECK: [[ALLOC_COUNT:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[ALLOC_RESULT:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpAllocateResource Buffer Buffer Undefined DeviceLocal UnorderedAccess
// CHECK: OpReturn [[ALLOC_RESULT]]
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
// CHECK-NOT: = OpFunc

// CHECK: Section("EMBD")
// CHECK: [[MODULE:%[A-Za-z0-9_]+]] = OpSlangModule {{%[A-Za-z0-9_]+}} [[BLOB]]
// CHECK: [[FILL_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint [[MODULE]] [[STR_FILL]]
// CHECK: [[FILL_DISPATCHER]] : [[DISPATCH_FUNC_TYPE_ONE]] = OpMakeDispatcher Compute [[FILL_ENTRY]]
// CHECK: [[FINISH_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint [[MODULE]] [[STR_FINISH]]
// CHECK: [[FINISH_DISPATCHER]] : [[DISPATCH_FUNC_TYPE_TWO]] = OpMakeDispatcher Compute [[FINISH_ENTRY]]
