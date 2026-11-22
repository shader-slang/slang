//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0
//TEST:COMPARE_COMPUTE(targets=vk+metal+cuda+wgpu):

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    uint laneId = WaveGetLaneIndex();
    // The laneCount will be dependent on target hardware. It seems a count of 1 is valid in spec.
    // For now we'll just check it's not 0.
    uint laneCount = WaveGetLaneCount();
    outputBuffer[idx] = int(((laneCount > 0) ? 0x100 : 0) + laneId);
}
