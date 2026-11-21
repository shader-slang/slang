//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=BUF):-profile cs_6_0
//TEST:COMPARE_COMPUTE(targets=vk+metal+cuda, filecheck-buffer=BUF):

// 4 slots: one per wave (128 threads / 32 lanes = 4 waves)
//TEST_INPUT:ubuffer(data=[0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// 128 threads = 4 waves of 32. Lane 0 of each wave writes its wave index.
[numthreads(128, 1, 1)]
void computeMain()
{
    uint laneId = WaveGetLaneIndex();
    uint waveIdx = WaveGetWaveIndex();

    if (laneId == 0)
        outputBuffer[waveIdx] = int(waveIdx);
}

// BUF: 0
// BUF-NEXT: 1
// BUF-NEXT: 2
// BUF-NEXT: 3
