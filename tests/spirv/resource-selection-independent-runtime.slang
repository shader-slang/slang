//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly -xslang -DMANUAL
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -validate-ir -validate-ir-detailed

// Each invocation compares independently selected texture and sampler operands with a
// hand-written conditional. We use gradient textures of different sizes with point and linear
// filtering, sampled away from texel centers, so all four texture/sampler pairs differ.
// A fifth output checks that the fixture distinguishes those pairs on the test device.
// The `MANUAL` run validates that fixture without selecting any resources.
// SPIRV: OpImageSampleExplicitLod
// GLSL: textureLod
//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA8Unorm):name textureA
Texture2D<float4> textureA;
//TEST_INPUT: Texture2D(size=8, content=gradient, format=RGBA8Unorm):name textureB
Texture2D<float4> textureB;
//TEST_INPUT: Sampler(filteringMode=point):name samplerA
SamplerState samplerA;
//TEST_INPUT: Sampler(filteringMode=linear):name samplerB
SamplerState samplerB;
//TEST_INPUT: ubuffer(data=[0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    bool chooseTextureA = (threadID.x & 1) == 0;
    bool chooseSamplerA = (threadID.x & 2) == 0;
    float2 uv = float2(0.4, 0.6);
#ifndef MANUAL
    Texture2D<float4> texture = select(chooseTextureA, textureA, textureB);
    SamplerState sampler = select(chooseSamplerA, samplerA, samplerB);
    float4 actual = texture.SampleLevel(sampler, uv, 0);
#endif

    // We spell out all four operation choices independently of the selected resource values.
    // The comparison therefore detects a transformation that couples the two predicates.
    float4 expected;
    if (chooseTextureA)
    {
        if (chooseSamplerA)
            expected = textureA.SampleLevel(samplerA, uv, 0);
        else
            expected = textureA.SampleLevel(samplerB, uv, 0);
    }
    else
    {
        if (chooseSamplerA)
            expected = textureB.SampleLevel(samplerA, uv, 0);
        else
            expected = textureB.SampleLevel(samplerB, uv, 0);
    }
#ifdef MANUAL
    float4 actual = expected;
#endif
    outputBuffer[threadID.x] = all(actual == expected) ? 1 : 0;

    // Only one invocation writes the fixture check. We compare every pair of direct results
    // so neither a wrong texture nor a wrong sampler can accidentally match the right pair.
    if (threadID.x == 0)
    {
        float4 aPoint = textureA.SampleLevel(samplerA, uv, 0);
        float4 aLinear = textureA.SampleLevel(samplerB, uv, 0);
        float4 bPoint = textureB.SampleLevel(samplerA, uv, 0);
        float4 bLinear = textureB.SampleLevel(samplerB, uv, 0);
        uint distinctPairs = 0;
        distinctPairs += any(aPoint != aLinear) ? 1 : 0;
        distinctPairs += any(aPoint != bPoint) ? 1 : 0;
        distinctPairs += any(aPoint != bLinear) ? 1 : 0;
        distinctPairs += any(aLinear != bPoint) ? 1 : 0;
        distinctPairs += any(aLinear != bLinear) ? 1 : 0;
        distinctPairs += any(bPoint != bLinear) ? 1 : 0;
        outputBuffer[4] = distinctPairs == 6 ? 1 : 0;
    }
}

// CHECK: type: uint32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
