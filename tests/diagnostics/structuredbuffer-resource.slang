//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

// Direct resource types as StructuredBuffer elements

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<Texture2D<float>> textureBuffer;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<SamplerState> samplerBuffer;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<StructuredBuffer<float>> nestedBuffer;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
RWStructuredBuffer<Texture2D<float>> rwTextureBuffer;

RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    float result = textureBuffer[i].Sample(samplerBuffer[i], float2(0, 0)).x 
                 + nestedBuffer[i][0];

    rwTextureBuffer[i] = textureBuffer[i];
    output[i] = result;
}
