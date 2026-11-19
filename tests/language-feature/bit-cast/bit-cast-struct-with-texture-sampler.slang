// Test that bit_cast between matching structs holding a texture and a sampler, next to
// ordinary data, compiles on targets that legalize resource types (issue #13480).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute

// CHECK: result code = 0
// CHECK: outputBuffer

struct Material
{
    Texture2D<float4> texture;
    SamplerState sampler;
    float4 tint;
}

struct MaterialAlias
{
    Texture2D<float4> texture;
    SamplerState sampler;
    uint4 tintBits;
}

uniform Material gMaterial;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    MaterialAlias m = bit_cast<MaterialAlias, Material>(gMaterial);
    float4 color = m.texture.SampleLevel(m.sampler, float2(0.5), 0);
    outputBuffer[0] = color * asfloat(m.tintBits);
}
