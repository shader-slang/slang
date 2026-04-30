// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-tests-capabilities-texture-types.py, and
// regenerate this test by running:
//
//     extras/test-generators/generate-tests-capabilities-texture-types.py
//
// Texture types capability test: glsl-vk / TextureCubeArray<float4>
// - Type supported since target version:  400
// - Target version for positive test:     400
// - Target version for negative test:     330
//

//TEST:SIMPLE(filecheck=POSITIVE): -restrictive-capability-check -entry fragMain -stage fragment -target glsl -profile glsl_400
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -vk -emit-spirv-via-glsl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: TextureCube(size=4, content = one, arrayLength=2):name texHandle
TextureCubeArray<float4> texHandle;

//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;

[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: main
// POSITIVE-NOT: {{(error|warning).*}}:

    int ret = 0;
    ret = int((texHandle.SampleLevel(samplerState, float4(0, 0, 0, 0), float(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    ret = int((texHandle.SampleLevel(samplerState, float4(0, 0, 0, 0), float(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12346
