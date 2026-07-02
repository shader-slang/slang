//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// Exercise the additional type arms of `isVkBindingCompatibleEntryPointParameterType`
// that the other entry-point vk::binding tests do not cover: a plain texture
// (`ResourceType`), a sampler (`SamplerStateType`), and a `ByteAddressBuffer`
// (`UntypedBufferResourceType`). Each honors its explicit `[[vk::binding]]`.
// (`HLSLStructuredBufferTypeBase`, `UniformParameterGroupType`, and
// `SubpassInputType` are covered by the sibling tests.)

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(1, 0)]]
    Texture2D tex,
    [[vk::binding(2, 0)]]
    SamplerState samp,
    [[vk::binding(3, 0)]]
    ByteAddressBuffer rawBuf,
    RWStructuredBuffer<float> outBuf)
{
    outBuf[0] = tex.SampleLevel(samp, float2(0, 0), 0).x + rawBuf.Load(0);
}

// CHECK-DAG: OpDecorate %entryPointParams_tex Binding 1
// CHECK-DAG: OpDecorate %entryPointParams_tex DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_samp Binding 2
// CHECK-DAG: OpDecorate %entryPointParams_samp DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_rawBuf Binding 3
// CHECK-DAG: OpDecorate %entryPointParams_rawBuf DescriptorSet 0
