//TEST_CATEGORY(wave-mask, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0
//TEST:COMPARE_COMPUTE(targets=vk+metal+cuda):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    WaveMask mask = WaveGetActiveMask();
    uint4 multi = WaveGetActiveMulti();

    // Should be 0xf, because numthreads is 4, 1, 1
    outputBuffer[idx * 2 + 0] = int(mask);
    outputBuffer[idx * 2 + 1] = int(multi.x);
}