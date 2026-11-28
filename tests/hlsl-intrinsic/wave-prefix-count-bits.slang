//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=cuda):-capability cuda_sm_7_0
//TEST:COMPARE_COMPUTE(targets=metal):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    outputBuffer[idx] = int(WavePrefixCountBits(bool(idx & 5)));
}
