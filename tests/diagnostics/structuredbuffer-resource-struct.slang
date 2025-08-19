//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct WithTexture
{
    float4 color;
    Texture2D tex;
    float scale;
}

struct WithSampler
{
    SamplerState sampler;
    float2 uv;
}

struct Nested
{
    WithTexture data;
    float value;
}

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<WithTexture> bufferWithTexture;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<WithSampler> bufferWithSampler;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<Nested> bufferNested;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
