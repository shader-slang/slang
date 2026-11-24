//TEST:COMPARE_COMPUTE(targets=cpu):-compile-arg -O3 -compute-dispatch 3,1,1
//TEST:COMPARE_COMPUTE(targets=all):-compute-dispatch 3,1,1

//TEST_INPUT:ubuffer(data=[0 0 0 0  0 0 0 0  0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadId)
{
    outputBuffer[dispatchThreadID.x] = int((dispatchThreadID.x << 8) | (groupID.x << 4) | (groupThreadID.x));
}
