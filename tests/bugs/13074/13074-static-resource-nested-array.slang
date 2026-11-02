// This HLSL test verifies that initializer movement and resource-global replacement preserve both
// dimensions of a nested texture array. Vulkan's `UniformConstant` interface rules permit only one
// array layer around the resource type in this kind of global shader parameter, so
// 13074-static-resource-kinds.slang covers the corresponding one-dimensional SPIR-V case.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute -validate-ir
//TEST:SIMPLE(filecheck=NOCACHE): -target hlsl -entry main -stage compute -validate-ir

//NOCACHE-NOT: cachedTextures
//CHECK: Texture2D<float4 >   [[TEXTURES:textures_[0-9]+]][int(1)][int(1)]
//CHECK-LABEL: void main()
//CHECK: [[TEXTURES]][int(0)][int(0)].Load

Texture2D<float4> textures[1][1];
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTextures[1][1] = textures;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTextures[0][0].Load(int3(0, 0, 0));
}
