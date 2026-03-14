//TEST:SIMPLE(filecheck=CHECK):-target spirv-assembly -profile spirv_1_3 -entry main -stage compute -emit-spirv-directly

// Exercise non-uniform dynamic-resource indexing across storage buffers,
// sampled images, sampled-image formation, and storage images.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability RuntimeDescriptorArray
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: OpCapability StorageBufferArrayNonUniformIndexing
// CHECK-DAG: OpCapability StorageImageArrayNonUniformIndexing
// CHECK-DAG: %[[BUF_AC:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Uniform_StructuredBuffer %bindless_storage_buffers %[[BUF_IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: %[[BUF_PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Uniform_float %[[BUF_AC]] %int_0 %int_0
// CHECK-DAG: %[[TEX_AC:[A-Za-z0-9_]+]] = OpAccessChain %[[TEX_PTR_T:[A-Za-z0-9_]+]] %bindless_storage_buffers_0 %[[TEX_IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: %[[TEX:[A-Za-z0-9_]+]] = OpLoad %[[TEX_T:[A-Za-z0-9_]+]] %[[TEX_AC]]
// CHECK-DAG: %[[SAMPLED_IMAGE:[A-Za-z0-9_]+]] = OpSampledImage
// CHECK-DAG: %[[IMG_AC:[A-Za-z0-9_]+]] = OpAccessChain %[[IMG_PTR_T:[A-Za-z0-9_]+]] %bindless_storage_buffers_1 %[[IMG_IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: %[[RW_IMG:[A-Za-z0-9_]+]] = OpLoad %[[IMG_T:[A-Za-z0-9_]+]] %[[IMG_AC]]
// CHECK-DAG: OpDecorate %[[BUF_IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[BUF_AC]] NonUniform
// CHECK-DAG: OpDecorate %[[BUF_PTR]] NonUniform
// CHECK-DAG: OpDecorate %[[TEX_IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[TEX_AC]] NonUniform
// CHECK-DAG: OpDecorate %[[TEX]] NonUniform
// CHECK-DAG: OpDecorate %[[IMG_IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[IMG_AC]] NonUniform
// CHECK-DAG: OpDecorate %[[RW_IMG]] NonUniform
// CHECK-DAG: OpDecorate %[[SAMPLED_IMAGE]] NonUniform

[vk::binding(0, 0)]
RWTexture1D<float> render_target;

[vk::binding(1, 0)]
SamplerState sampler;

[vk::binding(2, 0)]
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
