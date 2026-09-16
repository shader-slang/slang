//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -fvk-t-shift 128 0

// Regression test for https://github.com/shader-slang/slang/issues/13096.
// A `SubpassInput` occupies two layout slots (a texture/`ShaderResource` slot and an
// `InputAttachmentIndex` slot). `-fvk-t-shift` reclassifies the texture slot to
// `ShaderResource`; the SPIR-V storage-class resolver then could not reconcile it against
// the `InputAttachmentIndex` slot (which carries no storage class) and aborted with E99997.
// The compile must now succeed, keep the input-attachment decoration, resolve the opaque
// image to `UniformConstant`, and apply the `t` shift to its descriptor binding.

[[vk::input_attachment_index(0)]]
SubpassInput input;

float4 main() : SV_Target
{
    return input.SubpassLoad();
}

// CHECK-DAG: OpCapability InputAttachment
// CHECK-DAG: OpDecorate %input InputAttachmentIndex 0
// CHECK-DAG: OpDecorate %input Binding 128
// CHECK: OpVariable %{{.*}} UniformConstant
