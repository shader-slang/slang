//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -fvk-t-shift 128 0

// shader-slang/slang#13096. No binding is checked because `-fvk-t-shift` does not shift entry-point
// parameters, although it still reclassifies the texture slot.

[shader("fragment")]
float4 main([[vk::input_attachment_index(1)]] SubpassInput epInput) : SV_Target
{
    return epInput.SubpassLoad();
}

// CHECK-NOT: E99997
// CHECK-DAG: OpCapability InputAttachment
// CHECK-DAG: OpDecorate %entryPointParams_epInput InputAttachmentIndex 1
// CHECK-DAG: %entryPointParams_epInput = OpVariable %{{.*}} UniformConstant
