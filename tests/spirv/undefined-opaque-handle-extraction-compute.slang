// Check that a helper given a combined texture-sampler with a never-written sampler or texture still
// reads the defined texture or samples with the defined sampler extracted from it, and that
// removing a sample through the never-written handle keeps the helper's other effects. The sample
// through the never-written sampler is behind a condition that is false at runtime, so every
// result checked here is defined.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41016
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -g -xslang -Wno-41016

//TEST_INPUT:ubuffer(data=[0], stride=4):name=flags
RWStructuredBuffer<uint> flags;
//TEST_INPUT: Texture2D(size=4, content = one):name gTexture
Texture2D<float4> gTexture;
//TEST_INPUT: Sampler:name gSampler
SamplerState gSampler;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float4> outputBuffer;

float4 readTextureAndWrite(Sampler2D<float4> combined)
{
    outputBuffer[2] = float4(1, 2, 3, 4);
    float4 result = combined.__getTexture().Load(int3(0));
    if (flags[0] != 0)
        result += combined.SampleLevel(float2(0), 0);
    return result;
}

float4 useSampler(Sampler2D<float4> combined)
{
    return gTexture.SampleLevel(combined.__getSampler(), float2(0), 0);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    SamplerState s;
    Texture2D<float4> t;
    outputBuffer[0] = readTextureAndWrite(__makeCombinedTextureSampler(gTexture, s));
    outputBuffer[1] = useSampler(__makeCombinedTextureSampler(t, gSampler));
    // CHECK: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 3.0
    // CHECK-NEXT: 4.0
}
