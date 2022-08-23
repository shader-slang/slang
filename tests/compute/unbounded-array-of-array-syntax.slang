//IGNORE_TEST:CPU_REFLECTION: -profile cs_5_0 -entry computeMain -target cpp
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute
//TEST:CROSS_COMPILE:-target dxbc-assembly -entry computeMain -profile cs_5_1
//TEST:CROSS_COMPILE:-target spirv-assembly -entry computeMain -profile cs_5_1
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:array(size=2):name g_aoa
RWStructuredBuffer<int> g_aoa[];

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=g_aoa[0]
//TEST_INPUT:ubuffer(data=[8 17 34], stride=4):name=g_aoa[1]

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    
    int baseIndex = index >> 2;
    int innerIndex = index & 3;

    RWStructuredBuffer<int> buffer = g_aoa[NonUniformResourceIndex(baseIndex)];

    // Get the size 
    uint bufferCount, bufferStride;
    buffer.GetDimensions(bufferCount, bufferStride);

    if (innerIndex >= bufferCount)
    {
        innerIndex = int(bufferCount - 1);
    }
	outputBuffer[index] = buffer[innerIndex]; 
}