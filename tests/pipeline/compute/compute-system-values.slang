// compute-system-values.slang

//TEST(compute):COMPARE_COMPUTE:
//TEST(compute):COMPARE_COMPUTE: -cpu

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 2, 1)]
void computeMain(
    uint2 groupThreadID     : SV_GroupThreadID,
    int groupIndex          : SV_GroupIndex,
    uint3 dispatchThreadID  : SV_DispatchThreadID,
    int2 groupID            : SV_GroupID)
{
    int tid = groupID.x + groupIndex;

    int value = 0;
    value = value*16 + groupIndex;
    value = value*16 + groupID.x;
    value = value*16 + groupID.y;
    value = value*16 + groupThreadID.x;
    value = value*16 + groupThreadID.y;
    value = value*16 + dispatchThreadID.x;
    value = value*16 + dispatchThreadID.y;

    outputBuffer[tid] = value;
}