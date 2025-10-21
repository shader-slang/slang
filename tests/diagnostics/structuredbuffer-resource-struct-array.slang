//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

struct HasResource
{
    Texture2D tex;
}

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<HasResource> bufferArray[4];

//CHECK-DAG: ([[# @LINE+1]]): error 38204
RWStructuredBuffer<Texture2D> rwBufferArray[2];

SamplerState sampler;
RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    // Force usage of all array elements and struct members
    float result = bufferArray[i % 4][i].tex.Sample(sampler, float2(0, 0)).x;
    rwBufferArray[i % 2][i] = bufferArray[0][i].tex;
    output[i] = result;
}

