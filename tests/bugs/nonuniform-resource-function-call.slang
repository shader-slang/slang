//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// CHECK: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: %[[CALL_IDX:[A-Za-z0-9_]+]] = OpCompositeExtract %uint %{{[A-Za-z0-9_]+}} 0
// CHECK-DAG: OpDecorate %[[NONUNIFORM_ACCESS:[A-Za-z0-9_]+]] NonUniform
// CHECK-DAG: OpDecorate %[[NONUNIFORM_SAMPLED_IMAGE:[A-Za-z0-9_]+]] NonUniform
// CHECK-DAG: %[[UNIFORM_CALL:[A-Za-z0-9_]+]] = OpFunctionCall %float %sampleHelper %int_0 %{{[A-Za-z0-9_]+}}
// CHECK-DAG: %[[NONUNIFORM_CALL:[A-Za-z0-9_]+]] = OpFunctionCall %float %sampleHelper_0 %[[CALL_IDX]] %{{[A-Za-z0-9_]+}}
// CHECK-LABEL: %sampleHelper_0 = OpFunction
// CHECK: %[[NONUNIFORM_PARAM:[A-Za-z0-9_]+]] = OpFunctionParameter %uint
// CHECK: %[[NONUNIFORM_SAMPLER_PARAM:[A-Za-z0-9_]+]] = OpFunctionParameter %11
// CHECK: %[[NONUNIFORM_ACCESS]] = OpAccessChain %_ptr_UniformConstant_23 %resources %[[NONUNIFORM_PARAM]]
// CHECK: %[[NONUNIFORM_LOAD:[A-Za-z0-9_]+]] = OpLoad %23 %[[NONUNIFORM_ACCESS]]
// CHECK: %[[NONUNIFORM_SAMPLED_IMAGE]] = OpSampledImage %31 %[[NONUNIFORM_LOAD]] %[[NONUNIFORM_SAMPLER_PARAM]]

[vk::binding(0, 0)]
RWTexture1D<float> render_target;

[vk::binding(1, 0)]
SamplerState sampler;

[vk::binding(2, 0)]
__DynamicResource<__DynamicResourceKind.General> resources[];

[noinline]
float sampleHelper(Texture1D<float> tex, SamplerState s)
{
    return tex.SampleLevel(s, 0.0, 0);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint index: SV_DispatchThreadID)
{
    Texture1D<float> uniformTex = resources[0].asOpaqueDescriptor<Texture1D<float>>();
    Texture1D<float> tex =
        resources[NonUniformResourceIndex(index)].asOpaqueDescriptor<Texture1D<float>>();
    render_target[1] = sampleHelper(uniformTex, sampler);
    render_target[0] = sampleHelper(tex, sampler);
}
