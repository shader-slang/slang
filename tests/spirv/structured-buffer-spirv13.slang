//TEST:SIMPLE(filecheck=CHECK):-target spirv -profile spirv_1_3 -entry computeMain -stage compute -emit-spirv-directly

// Ordinary StructuredBuffer indexing also lowers through RWStructuredBufferGetElementPtr,
// but should remain valid before SPIR-V 1.4 because it emits OpAccessChain, not OpPtrAccessChain.
// SPIR-V 1.3 uses Uniform storage class with BufferBlock decoration.

// CHECK: OpDecorate %[[ARRAY:[A-Za-z0-9_]+]] ArrayStride 16
// CHECK-DAG: %[[V3:[A-Za-z0-9_]+]] = OpTypeVector %float 3
// CHECK-DAG: %[[ARRAY]] = OpTypeRuntimeArray %[[V3]]
// CHECK-DAG: %[[PTR:[A-Za-z0-9_]+]] = OpTypePointer Uniform %[[V3]]
// CHECK: OpFunction
// CHECK-NOT: OpPtrAccessChain
// CHECK: OpAccessChain %[[PTR]]
// CHECK-NOT: OpPtrAccessChain
// CHECK: OpAccessChain %[[PTR]]
// CHECK-NOT: OpPtrAccessChain
// CHECK: OpFunctionEnd

StructuredBuffer<float3> input;
RWStructuredBuffer<float3> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    output[index] = input[index] + float3(1.0f);
}
