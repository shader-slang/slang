//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[shader("compute")]
[numthreads(1, 1, 1)]
void fill(RWStructuredBuffer<uint> output)
{
    output[0] = 123;
}


[host]
public void main(RWStructuredBuffer<uint> output)
{
    dispatch(fill, uint3(1, 1, 1))(output);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_FILL:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 4 "fill"
// CHECK-DAG: [[BLOB:%[A-Za-z0-9_]+]] = OpBinaryLit {{[0-9]+}} { ... }
// CHECK-DAG: [[ONE:%[A-Za-z0-9_]+]] : [[UINT:%[A-Za-z0-9_]+]] = OpIntLit 1

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT]] = OpUIntType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]
// CHECK-DAG: [[VEC3:%[A-Za-z0-9_]+]] = OpVectorType 3 [[UINT]]
// CHECK-DAG: [[FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[BUFFER]]
// CHECK-DAG: [[DISPATCH_FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[VEC3]] [[BUFFER]]

// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[FUNC_TYPE]] = OpFunc
// CHECK: [[OUTPUT:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpParam
// CHECK: [[DISPATCH_SIZE:%[A-Za-z0-9_]+]] : [[VEC3]] = OpMakeVector [[ONE]] [[ONE]] [[ONE]]
// CHECK: OpCall [[DISPATCHER:%[A-Za-z0-9_]+]] [[DISPATCH_SIZE]] [[OUTPUT]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd

// CHECK: Section("EMBD")
// CHECK: [[MODULE:%[A-Za-z0-9_]+]] = OpSlangModule {{%[A-Za-z0-9_]+}} [[BLOB]]
// CHECK: [[FILL_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint [[MODULE]] [[STR_FILL]]
// CHECK: [[HOST_LAYOUT:%[A-Za-z0-9_]+]] = OpTypeLayout [[BUFFER]] Host Value {{%[A-Za-z0-9_]+}} 0
// CHECK: [[SHADER_LAYOUT:%[A-Za-z0-9_]+]] = OpTypeLayout [[BUFFER]] ShaderObject ShaderParameter [[DISPATCHER]] 0
// CHECK: [[CONVERTER:%[A-Za-z0-9_]+]] = OpLayoutConverter [[HOST_LAYOUT]] [[SHADER_LAYOUT]]
// CHECK: [[DISPATCHER]] : [[DISPATCH_FUNC_TYPE]] = OpMakeDispatcher Compute [[FILL_ENTRY]] [[CONVERTER]]

/*
%122 = OpSlangModule {<BinaryBlob>}
%123 = OpLookup "fill" %122
%123 = OpMakeComputePipeline %123, %121, %output
%124 = OpCall %123 %125 %126

    ...
case OpMakeComputePipeline:
    rhi->createComputePipeline(
        api->loadModule(...)->codegen(...),
        ...)
case OpCall:
    ...
*/
