// WGSL cannot declare an `array` of textures or samplers; an array of them is a `binding_array`.
// Tint accepts a sized `binding_array` of sampled textures (the `sized_binding_array` language
// feature); naga (wgpu) also accepts unsized ones and arrays of samplers. Before this change Slang
// emitted `array<texture_2d<f32>, N>`, which both reject. See shader-slang/slang#9528.
//
// The SIZED output was checked with Tint from Dawn c943ea32 and with naga 25; the Tint bundled
// with slang-test does not know `binding_array`, so `wgsl-spirv-asm` cannot validate it here.

//TEST:SIMPLE(filecheck=SIZED): -target wgsl -stage fragment -entry sizedMain
//TEST:SIMPLE(filecheck=UNSIZED): -target wgsl -stage fragment -entry unsizedMain
//TEST:SIMPLE(filecheck=SAMPLERS): -target wgsl -stage fragment -entry samplerMain
//TEST:SIMPLE(filecheck=HEAP): -target wgsl -stage compute -entry heapMain

// SIZED: var textures_0 : binding_array<texture_2d<f32>, i32(8)>;
// SIZED-NOT: array<texture_2d
// SIZED: textureSample({{.*}}textures_0[

// UNSIZED: var unboundedTextures_0 : binding_array<texture_2d<f32>>;
// UNSIZED-NOT: array<texture_2d
// UNSIZED: textureSample({{.*}}unboundedTextures_0[

// SAMPLERS: var samplers_0 : binding_array<sampler, i32(4)>;
// SAMPLERS: textureSample({{.*}}samplers_0[

// The default `DescriptorHandle` lookup on WGSL indexes a global descriptor array.
// HEAP: var _slang_resource_heap{{.*}} : binding_array<texture_2d<f32>>;

Texture2D<float4> textures[8];
Texture2D<float4> unboundedTextures[];
SamplerState samplers[4];
SamplerState samp;
Texture2D<float4> tex;

cbuffer Params
{
    uint index;
}

[shader("fragment")]
float4 sizedMain(float2 uv : TEXCOORD0) : SV_Target
{
    return textures[index].Sample(samp, uv);
}

[shader("fragment")]
float4 unsizedMain(float2 uv : TEXCOORD0) : SV_Target
{
    return unboundedTextures[index].Sample(samp, uv);
}

[shader("fragment")]
float4 samplerMain(float2 uv : TEXCOORD0) : SV_Target
{
    return tex.Sample(samplers[index], uv);
}

StructuredBuffer<Texture2D<float4>.Handle> handles;
RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void heapMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = handles[tid.x].Load(int3(0));
}
