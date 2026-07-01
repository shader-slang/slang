//TEST:REFLECTION(filecheck=REFLECTION): -target spirv-asm -entry main -stage fragment -bindless-space-index 0
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -bindless-space-index 0

// Regression test for #11860. A `SubpassInput` entry-point parameter carrying both
// `[[vk::binding]]` and `[[vk::input_attachment_index]]` used to falsely reserve
// descriptor set 0: the input-attachment index reservation hardcodes a placeholder
// descriptor space of 0 (an input-attachment index occupies no descriptor set), and
// that placeholder space was being marked "used". As a result a requested bindless
// space of 0 was reported unavailable (warning E39012) and the bindless resource heap
// was bumped to descriptor set 1. The input-attachment index does not occupy a
// descriptor set, so set 0 must stay available for the bindless heap.

// The requested bindless space (0) is honored now that the input-attachment index no
// longer occupies descriptor set 0.
// REFLECTION: "bindlessSpaceIndex": 0

[shader("fragment")]
float4 main(
    [[vk::binding(7, 3)]]
    [[vk::input_attachment_index(5)]]
    SubpassInput<float4> input)
    : SV_Target
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(uint2(0, 0));
    return input.SubpassLoad() + texture.Load(int3(0, 0, 0));
}

// The input attachment itself is still bound at set 3 / binding 7 with index 5.
// CHECK-DAG: OpDecorate %entryPointParams_input Binding 7
// CHECK-DAG: OpDecorate %entryPointParams_input DescriptorSet 3
// CHECK-DAG: OpDecorate %entryPointParams_input InputAttachmentIndex 5
