//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -fvk-t-shift 128 0

// shader-slang/slang#13096. `-fvk-t-shift` changes the layout rules for entry-point parameters too,
// so this texture slot is a `ShaderResource` slot, but only global parameters receive the shift
// itself (a `Texture2D` entry-point parameter also stays at binding 0). The binding is pinned so a
// change to that behaviour is deliberate.

[shader("fragment")]
float4 main([[vk::input_attachment_index(1)]] SubpassInput epInput) : SV_Target
{
    return epInput.SubpassLoad();
}

// CHECK-NOT: E99997
// CHECK-DAG: OpCapability InputAttachment
// CHECK-DAG: OpDecorate %entryPointParams_epInput InputAttachmentIndex 1
// CHECK-DAG: OpDecorate %entryPointParams_epInput Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_epInput DescriptorSet 0
// CHECK-DAG: %entryPointParams_epInput = OpVariable %{{.*}} UniformConstant
