//TEST_CATEGORY(wave-mask, compute)
// We need SM6.5 for these tests
// Disable because version of dxc we are currently using doesn't support SM6.5
// Disabled because we don't have GLSL intrinsics for these it seems
//TEST:COMPARE_COMPUTE(targets=dx12+cuda):-capability cuda_sm_7_0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    int value = 0;

    WaveMask mask = 0xff;

    // Scalar
    value += WaveMaskPrefixSum(mask, 1 << idx);

    outputBuffer[idx] = value;
}
