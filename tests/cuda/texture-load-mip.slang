// Read-only `Load` on CUDA must read the mip level given in the last component of the location
// (#12630). With `content=gradient` each level is generated at its own size, so texel 1 of a size-4
// RGBA8Unorm texture holds 1/3 at level 0 (size 4) and 1 at level 1 (size 2). A fetch that ignores
// the level reads 1/3 for both. `mipMaps=2` makes slang-rhi back the 1D texture with a mipmapped
// CUDA array. The CPU backend ignores the level, so this runs on CUDA only.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT: Texture1D(format=RGBA8Unorm, size=4, content=gradient, mipMaps=2):name t1
Texture1D<float4> t1;
//TEST_INPUT: Texture2D(format=RGBA8Unorm, size=4, content=gradient, mipMaps=2):name t2
Texture2D<float4> t2;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = t1.Load(int2(1, 0)).x;
    outputBuffer[1] = t1.Load(int2(1, 1)).x;
    outputBuffer[2] = t2.Load(int3(1, 0, 0)).x;
    outputBuffer[3] = t2.Load(int3(1, 0, 1)).x;
}

//CHECK: type: float
// Texture1D, texel 1 at level 0 and level 1
//CHECK-NEXT: {{^}}0.333333{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
// Texture2D, texel (1, 0) at level 0 and level 1
//CHECK-NEXT: {{^}}0.333333{{$}}
//CHECK-NEXT: {{^}}1.000000{{$}}
