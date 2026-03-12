//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// CHECK: OpCapability UniformBufferArrayNonUniformIndexing
// CHECK-DAG: %[[BUFFER_PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Uniform_{{[A-Za-z0-9_]+}} %resources %{{[A-Za-z0-9_]+}}
// CHECK-DAG: %[[FIELD_PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Uniform_float %[[BUFFER_PTR]] %int_0
// CHECK-DAG: OpDecorate %[[BUFFER_PTR]] NonUniform
// CHECK-DAG: OpDecorate %[[FIELD_PTR]] NonUniform

struct Data
{
    float value;
};

[vk::binding(0, 0)]
RWTexture1D<float> render_target;

[vk::binding(1, 0)]
__DynamicResource<__DynamicResourceKind.General> resources[];

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint index: SV_DispatchThreadID)
{
    ConstantBuffer<Data> buffer =
        resources[NonUniformResourceIndex(index)].asOpaqueDescriptor<ConstantBuffer<Data>>();
    render_target[0] = buffer.value;
}
