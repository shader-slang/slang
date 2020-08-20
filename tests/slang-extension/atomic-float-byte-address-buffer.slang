// atomic-float-byte-address-buffer.slang

//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type
// Produces a different result
//TEST(compute):COMPARE_COMPUTE_EX:-dx11 -slang -compute -render-features nvapi -output-using-type
// Disabled because crashes currently on vulkan 
// https://vulkan.gpuinfo.org/listdevicescoverage.php?extension=VK_EXT_shader_atomic_float
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type
// Doesn't work on D3D12 for unknown reasons
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -render-features nvapi -output-using-type -compile-arg -O2
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -use-dxil -render-features nvapi -output-using-type -compile-arg -O2
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type

// We don't use this, but having this defined makes the 0 slot available if nvapi is going to be used
// Only strictly necessary on the dx12 path
//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):name=nvapiBuffer
RWStructuredBuffer<int> nvapiBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=workBuffer
RWByteAddressBuffer workBuffer;

//TEST_INPUT:ubuffer(data=[0.7 0.5 0.2 0.6], stride=4):name=anotherBuffer
RWStructuredBuffer<float> anotherBuffer;

[numthreads(16, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int idx = (tid & 3) ^ (tid >> 2); 

    //const float delta = anotherBuffer[idx & 3];
    
    float previousValue = 0;
    workBuffer.InterlockedAddFp32((idx << 2), 1.0f, previousValue);
    //workBuffer.InterlockedAddFp32((idx ^ 2) << 2, 2.0f + delta);
    
    // The sum of values in anotherBuffer should also be added
    //int anotherIdx = tid >> 2;
    //workBuffer.InterlockedAddFp32(anotherIdx << 2, delta);
    
    GroupMemoryBarrierWithGroupSync();
    
    if (tid < 4)
    {
        outputBuffer[tid] = asfloat(workBuffer.Load(tid << 2));
    }
}