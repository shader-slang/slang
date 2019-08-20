//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cpu
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    double d = double(dispatchThreadID.x);
    
    uint hi, low;
    asuint(d, low, hi);
    
    // Reconstruct 
    double r = asdouble(low, hi);
    
    outputBuffer[dispatchThreadID.x] = int(r);
}