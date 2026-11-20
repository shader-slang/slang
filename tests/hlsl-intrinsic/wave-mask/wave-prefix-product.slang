//TEST_CATEGORY(wave-mask, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_5 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature hardware-device,subgroup-partitioned-ext
//TEST:COMPARE_COMPUTE(targets=cuda):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    WaveMask mask = 0xff;

    int idx = int(dispatchThreadID.x);


    float2 v1 = float2(1, idx + 1);

    int r0 = WaveMaskPrefixProduct(mask, idx + 1);
    float2 r1 = WaveMaskPrefixProduct(mask, v1);

    int r2 = int(r1.x) + int(r1.y) - 1;

    outputBuffer[idx] = r0 + (r2 << 16);

}
