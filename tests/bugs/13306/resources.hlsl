//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target metal -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/resources.hlsl -Gec -target cpp -entry main -validate-ir

// Enabling compatibility must not introduce static storage for resources, resource arrays,
// explicit parameter groups, or legacy buffers whose element structs contain resources.
Texture2D<float4> gTex;
Texture2D<float4> textures[2];
SamplerState sampler;
struct Data { float4 value; };
ConstantBuffer<Data> explicitBuffer;
cbuffer Mixed
{
    Texture2D<float4> mixedTexture;
    float4 mixedValue;
};
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = gTex.SampleLevel(sampler, float2(0), 0) +
        textures[1].Load(int3(0)) + explicitBuffer.value +
        mixedTexture.Load(int3(0)) + mixedValue;
}
