//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -fvk-t-shift 128 0
//TEST:SIMPLE(filecheck=NOSHIFT):-target spirv-asm -entry main -stage fragment

// shader-slang/slang#13096: a `SubpassInput` has a texture slot and an `InputAttachmentIndex` slot.
// `-fvk-t-shift` lays the texture slot out as `ShaderResource` (`StorageBuffer`); SPIR-V
// legalization must not treat the storage-class-less `InputAttachmentIndex` slot as a conflict
// with it (that aborted with E99997). The image stays a `UniformConstant` input attachment. The
// `NOSHIFT` run checks the unshifted layout, which takes the same skipped-slot path.

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
