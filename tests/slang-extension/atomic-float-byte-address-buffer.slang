// byte-address-buffer.slang

//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute

//TEST_INPUT:ubuffer(data=[0.1 0.2 0.3 0.4]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

[numthreads(16, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int idx = (tid & 3) ^ (tid >> 2); 

    int byteOffset = idx << 2;

    // If atomics work correctly then 3.0 should be added to each thread 4 times
    
    float previousValue = 0;
    outputBuffer.InterlockedAddFp32(byteOffset, 1.0f, previousValue);
    outputBuffer.InterlockedAddFp32((idx ^ 2) << 2, 2.0f);
}