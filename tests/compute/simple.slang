//TEST(smoke,compute):COMPARE_COMPUTE:

// This is a basic test for Slang compute shader.

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float(dispatchThreadID.x);
}