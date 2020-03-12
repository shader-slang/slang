//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -profile cs_6_0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);
    
    float2 v1 = float2(1, idx + 1);
  
    int r0 = WavePrefixProduct(idx + 1);
    float2 r1 = WavePrefixProduct(v1);
    
    int r2 = int(r1.x) + int(r1.y) - 1;
    
    outputBuffer[idx] = r0 + (r2 << 16);
    
}