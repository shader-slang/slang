//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// No support for uint64_t on DX11
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
// No support for uint64_t on fxc - we need SM6.0 and dxil
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_3 -use-dxil 
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    uint64_t v = uint64_t(idx) * 0x8000100354354354ull; 
    // Let's check all the bits make it
    v |= 0x8000000000000000ull;
    
    outputBuffer[idx] = int(v) ^ int(v >> 32); 
}