//TEST_CATEGORY(wave, compute)
//DISABLE_TEST:COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//DISABLE_TEST:COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -profile cs_6_0 -shaderobj
//TEST(vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj -Xslang -DWGPU
//TEST:COMPARE_COMPUTE_EX:-metal -compute -shaderobj -Xslang -DMETAL

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// wgpu and metal use global in variables for WaveGetLaneIndex (see __builtinWaveLaneIndex)
// but mixing these with entry point variables is not supported, so we need to
// account for this here.
#if defined(WGPU) || defined(METAL)
in uint3 dispatchThreadID : SV_DispatchThreadID;
#endif

[numthreads(4, 1, 1)]
#if defined(WGPU) || defined(METAL)
void computeMain()
#else
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
#endif
{
    int idx = dispatchThreadID.x;
    uint laneId = WaveGetLaneIndex();
    // The laneCount will be dependent on target hardware. It seems a count of 1 is valid in spec. 
    // For now we'll just check it's not 0.
    uint laneCount = WaveGetLaneCount();
    outputBuffer[idx] = int(((laneCount > 0) ? 0x100 : 0) + laneId);
}
