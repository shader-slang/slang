//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

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

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
