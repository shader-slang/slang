//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0
//TEST:COMPARE_COMPUTE(targets=vk+metal):
//TEST:COMPARE_COMPUTE(targets=cuda):-capability cuda_sm_7_0

// WGSL doesn't support wave functions in a dynamic control flow; it works with uniform control flow.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    int value = 0;

    if (idx == 2)
    {
        // diverge
        return;
    }

    value = WaveActiveMin(idx + 1);

    outputBuffer[idx] = value;
}
