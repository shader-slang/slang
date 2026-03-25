//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test for https://github.com/shader-slang/slang/issues/10525
// Ensure NonUniform decorations are placed on the actual resource operands
// consumed by memory/sampling instructions, not just on the index/access-chain.

// Capabilities:
// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability StorageBufferArrayNonUniformIndexing
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing

// Structured buffer: access chain and inner element pointer must be NonUniform.
// CHECK-DAG: %[[SBAC:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_StructuredBuffer %bindless_storage_buffers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[SBAC]] NonUniform
// CHECK-DAG: %[[SBEP:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_float %[[SBAC]]
// CHECK-DAG: OpDecorate %[[SBEP]] NonUniform
// CHECK-DAG: %[[SBLOAD:[A-Za-z0-9_]+]] = OpLoad %float %[[SBEP]]
// CHECK-DAG: OpDecorate %[[SBLOAD]] NonUniform

// Texture access chain must be NonUniform.
// CHECK-DAG: %[[TEXAC:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_UniformConstant_{{[A-Za-z0-9_]+}} %bindless_storage_buffers_0 %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[TEXAC]] NonUniform

// OpSampledImage result must be NonUniform.
// CHECK-DAG: OpDecorate %sampledImage NonUniform

// Storage image: access chain and loaded image must be NonUniform.
// CHECK-DAG: %[[RWAC:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_UniformConstant_{{[A-Za-z0-9_]+}} %bindless_storage_buffers_1 %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[RWAC]] NonUniform
// CHECK-DAG: %[[RWLOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[RWAC]]
// CHECK-DAG: OpDecorate %[[RWLOAD]] NonUniform

RWTexture1D<float> render_target;
SamplerState sampler;
__DynamicResource<__DynamicResourceKind.General> bindless_storage_buffers[];

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint index: SV_DispatchThreadID)
{
    const StructuredBuffer<float> buffer =
        bindless_storage_buffers[NonUniformResourceIndex(index)]
            .asOpaqueDescriptor<StructuredBuffer<float>>();

    const Texture1D<float> tex1d =
        bindless_storage_buffers[NonUniformResourceIndex(index)]
            .asOpaqueDescriptor<Texture1D<float>>();

    const RWTexture1D<float> rwtex1d =
        bindless_storage_buffers[NonUniformResourceIndex(index)]
            .asOpaqueDescriptor<RWTexture1D<float>>();

    render_target[0] = buffer[0];
    render_target[1] = tex1d[0];
    render_target[2] = tex1d.SampleLevel(sampler, 0.0, 0);
    rwtex1d[0] = 1.0;
}
