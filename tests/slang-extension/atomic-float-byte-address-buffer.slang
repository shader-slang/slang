// byte-address-buffer.slang

//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
// Note for this feature we require dxc and we can force that with -use-dxil
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -use-dxil
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -use-dxil

//TEST_INPUT:ubuffer(data=[0.1 0.2 0.3 0.4]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

//TEST_INPUT:ubuffer(data=[0.7 0.5 0.2 0.6]):name=anotherBuffer
RWStructuredBuffer<float> anotherBuffer;

[numthreads(16, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int idx = (tid & 3) ^ (tid >> 2); 

    //const float delta = anotherBuffer[idx & 3];
    const float delta = 0.0f;

    // If atomics work correctly then 3.0 should be added to each thread 4 times.
    
    float previousValue = 0;
    outputBuffer.InterlockedAddFp32((idx << 2), 1.0f, previousValue);
    //outputBuffer.InterlockedAddFp32((idx ^ 2) << 2, 2.0f + delta);
    
    // The sum of values in anotherBuffer should also be added
    //int anotherIdx = tid >> 2;
    //outputBuffer.InterlockedAddFp32(anotherIdx << 2, delta);
}