//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -entry computeMain -profile cs_6_0

// HLSL code calls a default method on a constant buffer value whose type holds resources. The
// call passes a copy of the receiver argument (issue #13964), and resource legalization still
// produces code that samples the bound texture, as well as valid SPIR-V and DXIL.

struct Material
{
    Texture2D<float4> texture;
    SamplerState sampler;
    float tint;

    float4 sample(float2 uv) { return texture.SampleLevel(sampler, uv, 0) * tint; }
};

//TEST_INPUT: set gMaterial = new Material{Texture2D(size=4, content=one), Sampler, 2.0}
ConstantBuffer<Material> gMaterial;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // SPIRV: OpEntryPoint GLCompute
    // DXIL: define void @computeMain()
    float4 color = gMaterial.sample(float2(0.5, 0.5));
    // CHECK: 2.0
    outputBuffer[0] = color.x;
    // CHECK: 2.0
    outputBuffer[1] = color.w;
}
