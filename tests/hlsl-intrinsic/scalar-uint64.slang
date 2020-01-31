//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// No support for uint64_t on DX11
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
// No support for uint64_t on fxc - we need SM6.0 and dxil
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_0 -use-dxil 
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    uint64_t idx = dispatchThreadID.x;
    
    uint64_t ti = 0;
   
    ti += max(2, idx);
    ti += min(idx, 1ull);
    ti += (idx * 3) % 5;
    
    ti += clamp(idx * 10, 11, 23);
    
    ti += countbits(idx * 13);
   
    uint64_t v = uint64_t(ti) * 0x8000100354354354ull; 
    // Let's check all the bits make it
    uint64_t u = v | 0x8000000000000000ull;
    
    v = max(u, v);
      
    outputBuffer[dispatchThreadID.x] = int(v) ^ int(v >> 32); 
}