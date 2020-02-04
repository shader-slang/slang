// Disable the test in general, exists to check the output when the fix is in.

//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// No support for int64_t on dx11 (no sm 6.0)
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
// No support with Dx12 with dxbc. Needs SM6.0 + dxil
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -profile cs_6_0 -dx12 -use-dxil
// GLSL notices the narrowing of a 64 bit literal into 32 bits and for it this is an error.
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    // The multiply here will see dispatchThreadID.x as being uint, 
    // and then the multiply will coerce the lhs 0x7fff... into being uint, 
    // meaning 0x7fff.... will be interpreted as -1. int32_t(0x7ffff...) -> 0xffffffff 
    int64_t m = 0x7fffffffffffffff * idx; 
    
    m += 0x7fffffffffffffff * int(idx);
    
    int64_t v = 0x7fffffffffffffff;
    
    int64_t r = m + v;
    
    outputBuffer[idx * 2] = int(uint64_t(r) >> 32); 
    outputBuffer[idx * 2 + 1] = int(r); 
}