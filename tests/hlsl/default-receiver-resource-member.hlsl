//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -entry computeMain -profile cs_6_0

// HLSL code calls a default method on a constant buffer value whose type holds resources. The
// call passes a copy of the receiver argument (issue #13964), and resource legalization still
// produces valid SPIR-V and DXIL for the copied receiver.

struct Material
{
    Texture2D<float4> texture;
    SamplerState sampler;
    float4 tint;

    float4 sample(float2 uv) { return texture.SampleLevel(sampler, uv, 0) * tint; }
};

ConstantBuffer<Material> gMaterial;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // SPIRV: OpEntryPoint GLCompute
    // DXIL: define void @computeMain()
    outputBuffer[0] = gMaterial.sample(float2(0.5, 0.5));
}
