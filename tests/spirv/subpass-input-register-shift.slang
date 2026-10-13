//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -fvk-t-shift 128 0
//TEST:SIMPLE(filecheck=NOSHIFT):-target spirv-asm -entry main -stage fragment

// shader-slang/slang#13096: a `SubpassInput` has both a texture slot and an `InputAttachmentIndex`
// slot. `-fvk-t-shift` turns the texture slot into a shifted `ShaderResource` slot, and the image must
// still be a `UniformConstant` input attachment.

[[vk::input_attachment_index(0)]]
SubpassInput input;

float4 main() : SV_Target
{
    return input.SubpassLoad();
}

// CHECK-NOT: E99997
// CHECK-DAG: OpCapability InputAttachment
// CHECK-DAG: OpDecorate %input InputAttachmentIndex 0
// CHECK-DAG: OpDecorate %input Binding 128
// CHECK-DAG: %input = OpVariable %{{.*}} UniformConstant

// NOSHIFT-DAG: OpDecorate %input InputAttachmentIndex 0
// NOSHIFT-DAG: OpDecorate %input Binding 0
// NOSHIFT-DAG: %input = OpVariable %{{.*}} UniformConstant
