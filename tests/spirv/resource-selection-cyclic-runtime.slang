//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -validate-ir -validate-ir-detailed

// Each invocation rotates three selected textures zero through five times. We rotate their
// sampled values separately to establish the expected results without resource selections.
// Each output must pass six checks: the three initial values differ pairwise, and all three
// textures selected after the loop produce the corresponding rotated reference values.
// SPIRV: OpLoopMerge
// SPIRV: OpImageSampleExplicitLod

//TEST_INPUT: Texture2D(size=4, content=one):name textureA
Texture2D<float4> textureA;
//TEST_INPUT: Texture2D(size=4, content=zero):name textureB
Texture2D<float4> textureB;
//TEST_INPUT: Texture2D(size=4, content=gradient):name textureC
Texture2D<float4> textureC;
//TEST_INPUT: Sampler(filteringMode=linear):name sampler
SamplerState sampler;
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(6, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    float2 uv = float2(0.4, 0.6);
    float4 originalA = textureA.SampleLevel(sampler, uv, 0);
    float4 originalB = textureB.SampleLevel(sampler, uv, 0);
    float4 originalC = textureC.SampleLevel(sampler, uv, 0);
    float4 expectedA = originalA;
    float4 expectedB = originalB;
    float4 expectedC = originalC;
    Texture2D<float4> selectedA = textureA;
    Texture2D<float4> selectedB = textureB;
    Texture2D<float4> selectedC = textureC;

    // We save `selectedA` before changing any selected binding. Starting from
    // `(textureA, textureB, textureC)`, one iteration produces `(textureB, textureC, textureA)`.
    // Zero, one, and two iterations distinguish every position; three through five iterations
    // also check that the state survives a complete rotation.
    for (uint iteration = 0; iteration < threadID.x; iteration++)
    {
        Texture2D<float4> previousA = selectedA;
        selectedA = selectedB;
        selectedB = selectedC;
        selectedC = previousA;

        float4 previousExpectedA = expectedA;
        expectedA = expectedB;
        expectedB = expectedC;
        expectedC = previousExpectedA;
    }

    // A total of six requires every comparison to pass. The initial pairwise comparisons
    // prevent indistinguishable texture contents from hiding an incorrect permutation.
    uint passedChecks = 0;
    passedChecks += any(originalA != originalB) ? 1 : 0;
    passedChecks += any(originalA != originalC) ? 1 : 0;
    passedChecks += any(originalB != originalC) ? 1 : 0;
    passedChecks += all(selectedA.SampleLevel(sampler, uv, 0) == expectedA) ? 1 : 0;
    passedChecks += all(selectedB.SampleLevel(sampler, uv, 0) == expectedB) ? 1 : 0;
    passedChecks += all(selectedC.SampleLevel(sampler, uv, 0) == expectedC) ? 1 : 0;
    outputBuffer[threadID.x] = passedChecks;
}

// CHECK: type: uint32_t
// CHECK-NEXT: 6
// CHECK-NEXT: 6
// CHECK-NEXT: 6
// CHECK-NEXT: 6
// CHECK-NEXT: 6
// CHECK-NEXT: 6
