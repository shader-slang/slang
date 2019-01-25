//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int4 i4 = int4(int(dispatchThreadID.x), int(dispatchThreadID.x) + 1, int(dispatchThreadID.x) + 2, int(dispatchThreadID.x) + 3);
    
    outputBuffer[dispatchThreadID.x] = asint(float(dispatchThreadID.x) + 1.0f);
    
    outputBuffer[4 + dispatchThreadID.x] = int(i4.x + i4.y + i4.z + i4.w);     // 0 + 1 + 2 = 3 + (dispatchThread * 4)
}