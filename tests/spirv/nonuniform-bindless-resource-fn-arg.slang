//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test for the CastDynamicResource -> function-call specialization
// path in slang-ir-specialize-function-call.cpp. A bindless resource obtained
// via `asOpaqueDescriptor` with a NonUniformResourceIndex is passed as a
// function argument. The call must be specialized away (no indirect call
// remains) and NonUniform must reach the resource operands consumed inside the
// specialized callee (VUID-RuntimeSpirv-None-10148).

// The function call is specialized/inlined away, so no call survives.
// CHECK-NOT: OpFunctionCall

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability StorageBufferArrayNonUniformIndexing

// The descriptor-array access chain, the inner element pointer, and the load
// that the (specialized) callee performs must all be decorated NonUniform.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_StructuredBuffer %bindless %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[EP:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_float %[[AC]]
// CHECK-DAG: OpDecorate %[[EP]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %float %[[EP]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

RWTexture1D<float> render_target;
__DynamicResource<__DynamicResourceKind.General> bindless[];

float readBuf(StructuredBuffer<float> buf)
{
    return buf[0];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint index: SV_DispatchThreadID)
{
    StructuredBuffer<float> b =
        bindless[NonUniformResourceIndex(index)].asOpaqueDescriptor<StructuredBuffer<float> >();
    render_target[0] = readBuf(b);
}
