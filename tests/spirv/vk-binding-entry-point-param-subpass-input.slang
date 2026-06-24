//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float4 main(
    [[vk::binding(7, 3)]]
    [[vk::input_attachment_index(0)]]
    SubpassInput<float4> input)
    : SV_Target
{
    return input.SubpassLoad();
}

// CHECK-DAG: OpDecorate %entryPointParams_input Binding 7
// CHECK-DAG: OpDecorate %entryPointParams_input DescriptorSet 3
// CHECK-DAG: OpDecorate %entryPointParams_input InputAttachmentIndex 0
