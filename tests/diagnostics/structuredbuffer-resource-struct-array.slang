//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct HasResource
{
    Texture2D tex;
}

//CHECK-DAG: ([[# @LINE+1]]): error 38204
StructuredBuffer<HasResource> bufferArray[4];

//CHECK-DAG: ([[# @LINE+1]]): error 38204
RWStructuredBuffer<Texture2D> rwBufferArray[2];

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

