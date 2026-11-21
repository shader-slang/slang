//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12+cuda):-profile cs_6_0
//TEST:COMPARE_COMPUTE(targets=vk+metal+wgpu):

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    int value = 0;

    // Scalar

    value += WaveBroadcastLaneAt(idx, 1);

    // vector

    {
        float2 v = float2(idx + 1, idx + 2);
        float2 readValue = WaveBroadcastLaneAt(v, 4 & 3);

        value += int(readValue[0] + readValue[1]);
    }

    outputBuffer[idx] = value;
}
