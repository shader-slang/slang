// shader-slang/slang#13530: unsized arrays of subpass inputs, dynamic resources and entry-point
// textures hold no ordinary data, so they stay valid as uniform parameters. A pointer to a
// struct with a trailing unsized array is a sized value, so it stays valid too.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main

// CHECK-NOT: E31215
// CHECK: OpTypeRuntimeArray

struct TrailingData
{
    float4 a;
    float4 x[];
}

[[vk::input_attachment_index(0)]]
uniform SubpassInput<float4> subpassInputs[];
uniform __DynamicResource dynamicResources[];
uniform TrailingData* trailingDataPointer;

[shader("fragment")]
float4 main(uniform Texture2D entryPointTextures[]) : SV_Target
{
    return subpassInputs[1].SubpassLoad() +
           dynamicResources[4].as<StructuredBuffer<float4>>()[0] +
           entryPointTextures[2].Load(int3(0)) + trailingDataPointer->x[3] + trailingDataPointer->a;
}
