// No atomic support on CPU
// No support for int64_t on DX11
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -render-features atomic-int64 -compile-arg -O2
//TEST:COMPARE_COMPUTE(targets=vk):-render-features atomic-int64
//TEST:COMPARE_COMPUTE(targets=cuda):

// The test doesn't directly use this, but having this defined makes the 0 slot available if NVAPI is going to be used
// Only strictly necessary on the D3D12 path
//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):name=nvapiBuffer
RWStructuredBuffer<int> nvapiBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

// With only 4 threads there is no contention - which makes for a simple test
// but doesn't actually test for the exchange atomicity
[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;

    // Try directly reading
    uint2 currentValue2 = outputBuffer.Load2(idx << 3);
    uint64_t currentValue = uint64_t(currentValue2.y) | currentValue2.x;

    uint64_t readValue = outputBuffer.InterlockedExchangeU64(idx << 3, currentValue + 1);
}