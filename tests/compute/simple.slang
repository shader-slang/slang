//TEST(smoke,compute):COMPARE_COMPUTE:
//TEST(smoke,compute):COMPARE_COMPUTE:-cpu

// This is a basic test for Slang compute shader.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float(dispatchThreadID.x);
}