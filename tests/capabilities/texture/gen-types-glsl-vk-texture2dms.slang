// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit generate-types-tests.py and then regenerate this test
// by running: ./generate-types-tests.py
//
// Texture types capability test: glsl-vk / Texture2DMS<float4>
// - Type supported since target version:  None
// - Target version for positive test:     None
// - Target version for negative test:     460
//

//DISABLE_TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target None
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): None
//TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target glsl -profile glsl_460

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(size=4, content = zero, sampleCount = two):name texHandle
Texture2DMS<float4> texHandle;

[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: main
// POSITIVE-NOT: {{(error|warning).*}}:

// Expect either a compilation error or warning 41012 for upgraded profile to support functionality
// NEGATIVE: {{error [[:digit:]]+|warning 41012}}:

    int ret = 0;
    ret = int((texHandle.Load(int3(0, 0, 0))).x);
    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    ret = int((texHandle.Load(int3(0, 0, 0))).x);
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
