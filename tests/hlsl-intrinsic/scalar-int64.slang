//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// No support for int64_t on dx11
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
// No support for int64_t on HLSL
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    int64_t v = int64_t(idx) * 0x400010035435435ll; 
    
    outputBuffer[idx] = int(v) ^ int(((v >> 32) & 0xffffffff)); 
}