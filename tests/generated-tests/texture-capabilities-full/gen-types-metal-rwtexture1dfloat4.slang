// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit generate-types-tests.py and then regenerate this test
// by running: ./generate-types-tests.py
//
// Texture types capability test: metal / RWTexture1D<float4>
// - Type supported since target version:  1.0
// - Target version for positive test:     2.3
// - Target version for negative test:     None
//

//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target metal -profile metallib_2_3
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -mtl
//DISABLE_TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target None

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: RWTexture1D(size=4, content = zero):name texHandle
RWTexture1D<float4> texHandle;

[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: fragMain
// POSITIVE-NOT: {{(error|warning).*}}:

// Expect either a compilation error or warning 41012 for upgraded profile to support functionality
// NEGATIVE: {{error [[:digit:]]+|warning 41012}}:

    int ret = 0;
    ret = int((texHandle.Load(int(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    ret = int((texHandle.Load(int(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
