// THIS IS A GENERATED FILE. DO NOT EDIT!
//
// Texture types capability test: wgsl / WTexture2DArray<float4>
// - Supported since: 1.0
//
// To regenerate, run: ./generate-types-tests.py

//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target wgsl
// Compute test disabled, see https://github.com/shader-slang/slang/issues/8786
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -wgpu

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: RWTexture2D(size=4, content = zero, arrayLength=2):name texHandle
WTexture2DArray<float4> texHandle;

[shader("fragment")]
void fragMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: fragMain
// POSITIVE-NOT: error

// NEGATIVE: result code = -1
// NEGATIVE: error

    int ret = 0;
    texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));
    ret = 0;
    outputBuffer[0] = 0x12345 + ret;
 }

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));
    ret = 0;
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
