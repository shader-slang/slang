//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test for https://github.com/shader-slang/slang/issues/10525
// Ensure NonUniform decorations are placed on the actual resource operands
// consumed by memory/sampling instructions, not just on the index/access-chain.

// Capabilities:
// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability StorageBufferArrayNonUniformIndexing
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing

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
