//TEST(smoke,compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
// This is a basic test for Slang compute shader.

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float(dispatchThreadID.x);
}