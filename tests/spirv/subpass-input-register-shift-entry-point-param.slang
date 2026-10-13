//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -fvk-t-shift 128 0

// shader-slang/slang#13096. `-fvk-t-shift` still lays the texture slot out as `ShaderResource`
// here, so this reached the same E99997 abort as a global `SubpassInput`. The binding stays 0
// because `-fvk-t-shift` does not shift entry-point parameters; that is pre-existing (a `Texture2D`
// entry-point parameter is not shifted either), and this test pins it so any change is deliberate.

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
