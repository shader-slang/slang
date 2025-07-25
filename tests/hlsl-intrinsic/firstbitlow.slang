//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(5, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    uint testValues[5] = {0, 1, 2, 4, 8};
    uint expectedResults[5] = {0xFFFFFFFF, 0, 1, 2, 3}; // ~0u, 0, 1, 2, 3
    
    uint value = testValues[idx];
    uint result = firstbitlow(value);
    
    outputBuffer[idx] = result;
}