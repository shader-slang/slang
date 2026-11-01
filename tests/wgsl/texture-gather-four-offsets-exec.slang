// A four-offset gather must give, in component i, the (i0, j0) texel (component w) of a gather with
// offset i: the Vulkan run checks the emulation against native ConstOffsets.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -output-using-type

//TEST_INPUT: Texture2D(size=8, content=gradient, mipMaps=1):name tex
Texture2D<float4> tex;
//TEST_INPUT: Sampler:name samp
SamplerState samp;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 uv = float2(0.4375, 0.5625);
    float4 four = tex.GatherRed(samp, uv, int2(-1, 0), int2(1, 0), int2(0, 1), int2(1, -1));
    float4 each = float4(
        tex.GatherRed(samp, uv, int2(-1, 0)).w,
        tex.GatherRed(samp, uv, int2(1, 0)).w,
        tex.GatherRed(samp, uv, int2(0, 1)).w,
        tex.GatherRed(samp, uv, int2(1, -1)).w);
    // The offsets must matter: the plain gather differs.
    float4 plain = tex.GatherRed(samp, uv);

    // CHECK: 1
    // CHECK-NEXT: 1
    outputBuffer[0] = all(four == each) ? 1 : 0;
    outputBuffer[1] = any(four != plain) ? 1 : 0;
}
