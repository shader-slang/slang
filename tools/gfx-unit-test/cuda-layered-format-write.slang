// cuda-layered-format-write.slang - Stores float texels through `[format("rgba8")]` layered
// surfaces, so the CUDA backend has to use the converting `surf{1D,2D}Layeredwrite_convert` path.
// The x, y and layer coordinates are distinct so that a swapped coordinate lands on another texel.

[format("rgba8")]
uniform RWTexture2DArray<float4> texture2DArray;

[format("rgba8")]
uniform RWTexture1DArray<float4> texture1DArray;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 sv_dispatchThreadID: SV_DispatchThreadID)
{
    texture2DArray[uint3(2, 1, 3)] = float4(0.25, 0.5, 0.75, 1.0);
    texture1DArray[uint2(2, 3)] = float4(1.0, 0.75, 0.5, 0.25);
}
