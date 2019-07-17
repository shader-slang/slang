//TEST:CPU_EXECUTE: -profile cs_5_0 -entry computeMain -target sharedlib

[numthreads(4, 1, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID, 
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
    RWStructuredBuffer<int> outputBuffer)
{
    uint tid = dispatchThreadID.x;

    outputBuffer[tid] = int(tid * tid);
}