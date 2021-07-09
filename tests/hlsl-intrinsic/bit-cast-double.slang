//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-slang -vk -compute -shaderobj -render-feature double
//TEST(compute):COMPARE_COMPUTE_EX:-slang -cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
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