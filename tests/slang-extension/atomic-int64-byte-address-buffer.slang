// No atomic support on CPU
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
// No support for int64_t on DX11
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
// No support for int64_t on fxc - we need SM6.0 and dxil
// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
// Disable for now, because can only test when NVAPI is available, and it is not by default.
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_0 -use-dxil 
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

[numthreads(16, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    uint tid = dispatchThreadID.x;
    int idx = (tid & 3) ^ (tid >> 2); 

    const int64_t delta = 4;

    int64_t previousValue = 0;
    outputBuffer.InterlockedAddI64((idx << 3), 1, previousValue);
    
    int anotherIdx = tid >> 2;
    outputBuffer.InterlockedAddI64(anotherIdx << 3, -3);
}

