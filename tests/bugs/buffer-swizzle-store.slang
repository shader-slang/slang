//TEST(compute):COMPARE_COMPUTE(targets=all-cuda-cpu-wgpu-llvm):-shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(targets=vk+metal): -output-using-type
// Not supported in WGSL: read-write storage texture with "rg16f" format
// Disabled due to missing texture support in LLVM target.

//TEST_INPUT: RWTexture2D(format=RG16Float, size=4, content = one, mipMaps = 1):name g_test
[format("rg16f")]
RWTexture2D<float2> g_test;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain( uint2 dispatchThreadID : SV_DispatchThreadID )
{
    g_test[dispatchThreadID].xy = float2(0.0, 1.0);
    outputBuffer[dispatchThreadID.x] = g_test[dispatchThreadID].y;
}
