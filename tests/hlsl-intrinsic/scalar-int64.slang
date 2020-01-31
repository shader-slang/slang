//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// No support for int64_t on D3D11 (no sm 6.0)
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
// No support with Dx12 with dxbc. Needs SM6.0 + dxil
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -profile cs_6_0 -dx12 -use-dxil
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int64_t idx = int64_t(dispatchThreadID.x);
    
    int64_t ti =0;
    
    ti += max(2, idx);
    ti += min(idx, 1);
    ti += abs(idx - 2);
    ti += (idx * 3) % 5;
    
    ti += clamp(idx * 10, 11, 23);

    int64_t v = (ti * 0x400010035435435ll) / 3ll + 7ll - 9ll; 
    
    outputBuffer[idx] = int(v) ^ int(((v >> 32) & 0xffffffff)); 
}