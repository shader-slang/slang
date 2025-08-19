//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -spirv

struct WithStaticTexture
{
    float4 color;
    static Texture2D tex;
    float scale;
}

struct WithStaticSampler
{
    static SamplerState sampler;
    float2 uv;
}

struct NestedWithStatic
{
    WithStaticTexture data;
    float value;
}

// These should NOT produce error 38204 because resources are static
//CHECK-NOT: error 38204
StructuredBuffer<WithStaticTexture> bufferWithStaticTexture;

//CHECK-NOT: error 38204
StructuredBuffer<WithStaticSampler> bufferWithStaticSampler;

//CHECK-NOT: error 38204
StructuredBuffer<NestedWithStatic> bufferNestedStatic;

RWStructuredBuffer<float4> output;

Texture2D tex;
SamplerState sampler;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    WithStaticTexture.tex = tex;
    WithStaticSampler.sampler = sampler;

    // Use all non-static members and static resources
    float4 result = bufferWithStaticTexture[i].color * bufferWithStaticTexture[i].scale
                  + WithStaticTexture.tex.Sample(WithStaticSampler.sampler, bufferWithStaticSampler[i].uv)
                  + bufferNestedStatic[i].data.color * bufferNestedStatic[i].data.scale
                  + float4(bufferNestedStatic[i].value, 0, 0, 0);

    output[i] = result;
}

