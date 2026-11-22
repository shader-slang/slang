//TEST_CATEGORY(wave-mask, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=cuda):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const WaveMask mask0 = 0xff;

    int idx = int(dispatchThreadID.x);

    const WaveMask mask1 = WaveMaskBallot(mask0, idx < 3);
    if (idx < 3)
    {
        // Diverge!!
        outputBuffer[idx] = -1;
        return;
    }

    const WaveMask mask2 = mask0 & ~mask1;

    outputBuffer[idx] = WaveMaskIsFirstLane(mask2);
}