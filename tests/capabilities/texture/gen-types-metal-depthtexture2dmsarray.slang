// THIS IS A GENERATED FILE. DO NOT EDIT!
//
// Texture types capability test: metal / DepthTexture2DMSArray
// - Supported since: 2.0
//
// To regenerate, run: ./generate-types-tests.py

//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target metal
// Compute test disabled, see https://github.com/shader-slang/slang/issues/8457
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -mtl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(format=D32Float, size=4, content = zero, sampleCount = two, arrayLength=2):name texHandle
DepthTexture2DMSArray texHandle;

[shader("fragment")]
void fragMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: fragMain
// POSITIVE-NOT: error

// NEGATIVE: result code = -1
// NEGATIVE: error

    int ret = 0;
    ret = int((texHandle.Load(int3(0, 0, 0), 0)).x);
    outputBuffer[0] = 0x12345 + ret;
 }

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    ret = int((texHandle.Load(int3(0, 0, 0), 0)).x);
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
