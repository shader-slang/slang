//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

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

RWStructuredBuffer<float4> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    // Use all struct members from all buffers
    float4 result = bufferWithTexture[i].color * bufferWithTexture[i].scale
                  + bufferWithTexture[i].tex.Sample(bufferWithSampler[i].sampler, bufferWithSampler[i].uv)
                  + bufferNested[i].data.tex.Sample(bufferWithSampler[0].sampler, float2(0, 0)) * bufferNested[i].data.scale
                  + float4(bufferNested[i].value, 0, 0, 0);

    output[i] = result;
}

