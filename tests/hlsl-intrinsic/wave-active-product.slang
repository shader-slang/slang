//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=cuda):-capability cuda_sm_7_0
//TEST:COMPARE_COMPUTE(targets=metal):

// WGSL doesn't support wave functions in a dynamic control flow; it works with uniform control flow.

//TEST_INPUT:ubuffer(data=[0 0 0 0  0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = int(dispatchThreadID.x);

#if 1
    if (idx < 3)
    {
        // Diverge!!
        outputBuffer[idx] = -1; // thread 0, 1 and 2 will get 0xFFFFFFFF
        return;
    }

    // thread 2, 3, 4, 5, 6, 7 will get the result of 2 * 3 * 4 * 5 * 6 * 7
    // which is 2520, which is 9D8 in hex
    outputBuffer[idx] = WaveActiveProduct(idx);
#else

    /// NOTE! Can't say I totally understand WaveActiveProduct.
    /// The following returns 0x240 on CUDA - which is what I'd expect
    /// On DX12, it returns 0

    outputBuffer[idx] = WaveActiveProduct((idx & 3) + 1);
#endif
}
