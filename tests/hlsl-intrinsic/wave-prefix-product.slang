//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=cuda):-capability cuda_sm_7_0
//TEST:COMPARE_COMPUTE(targets=metal+wgpu):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    float2 v1 = float2(1, idx + 1);

    int r0 = WavePrefixProduct(idx + 1);
    float2 r1 = WavePrefixProduct(v1);

    int r2 = int(r1.x) + int(r1.y) - 1;

    outputBuffer[idx] = r0 + (r2 << 16);

}
