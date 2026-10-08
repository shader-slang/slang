//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float4 main(
    // Use a non-zero attachment index so the check proves the explicit
    // `[[vk::input_attachment_index]]` value is honored, rather than matching the
    // positional default (0) that a single subpass input would otherwise get.
    [[vk::binding(7, 3)]]
    [[vk::input_attachment_index(5)]]
    SubpassInput<float4> input)
    : SV_Target
{
    return input.SubpassLoad();
}

// CHECK-DAG: OpDecorate %entryPointParams_input Binding 7
// CHECK-DAG: OpDecorate %entryPointParams_input DescriptorSet 3
// CHECK-DAG: OpDecorate %entryPointParams_input InputAttachmentIndex 5
