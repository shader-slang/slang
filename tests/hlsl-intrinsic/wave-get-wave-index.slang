//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12 -profile cs_6_0 -shaderobj
//TEST(vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj

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
