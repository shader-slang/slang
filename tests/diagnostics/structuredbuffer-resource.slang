//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Direct resource types as StructuredBuffer elements

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<Texture2D> textureBuffer;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<SamplerState> samplerBuffer;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<StructuredBuffer<float>> nestedBuffer;

//CHECK-DAG: ([[# @LINE+1]]): error 38204
RWStructuredBuffer<Texture2D<float>> rwTextureBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
