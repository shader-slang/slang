//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<vector<float, 8>> bufferIn1;
RWStructuredBuffer<vector<float, 0>> bufferIn2;
RWStructuredBuffer<float> resultOut;

[shader("compute")]
[numthreads(32,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK: error 38203: vector has invalid element count '0', valid values are between '1' and '4' inclusive
    // CHECK: error 38203: vector has invalid element count '8', valid values are between '1' and '4' inclusive
    uint index = threadId.x;
    resultOut[index] = bufferIn1[index][0] + bufferIn2[index][0];
}
