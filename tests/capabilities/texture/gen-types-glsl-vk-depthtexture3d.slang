// THIS IS A GENERATED FILE. DO NOT EDIT!
//
// Texture types capability test: glsl-vk / DepthTexture3D
// - Supported since: None
//
// To regenerate, run: ./generate-types-tests.py

// Test disabled, see https://github.com/shader-slang/slang/issues/8721
//DISABLE_TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target glsl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture3D(format=D32Float, size=4, content = zero):name texHandle
DepthTexture3D texHandle;

//TEST_INPUT: Sampler(depthCompare):name samplerComparisonState
SamplerComparisonState samplerComparisonState;

[shader("fragment")]
void fragMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: main
// POSITIVE-NOT: error

// NEGATIVE: result code = -1
// NEGATIVE: error

    int ret = 0;
    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float3(0, 0, 0), float(0))).x);
    outputBuffer[0] = 0x12345 + ret;
 }

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float3(0, 0, 0), float(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
