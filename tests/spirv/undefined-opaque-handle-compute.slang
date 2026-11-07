// Regression test for https://github.com/shader-slang/slang/issues/14076
//
// A never-written texture is sampled only when `flags[0]` is nonzero. Here it is zero, so the
// shader must run and return the value it computes without the texture.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41016
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -g -xslang -Wno-41016

//TEST_INPUT:ubuffer(data=[0], stride=4):name=flags
RWStructuredBuffer<uint> flags;

//TEST_INPUT: Texture2D(size=4, content = one):name gTexture
Texture2D<float4> gTexture;

//TEST_INPUT: Sampler:name gSampler
SamplerState gSampler;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Texture2D<float4> t;
    float4 r = float4(1, 2, 3, 4);
    if (flags[0] != 0)
        r = t.SampleLevel(gSampler, float2(0), 0);
    outputBuffer[0] = r;
    outputBuffer[1] = gTexture.SampleLevel(gSampler, float2(0), 0);
    // CHECK: 1.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 3.0
    // CHECK-NEXT: 4.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
}
