
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);
    
    float v = 0.0f;

    // Test to check handling of + and -inf
    if (idx == 0)
    {
        v = 5e+40;
    }
    else if (idx == 1)
    {
        v = -5e+40;
    }
    
    outputBuffer[idx] = v;
}