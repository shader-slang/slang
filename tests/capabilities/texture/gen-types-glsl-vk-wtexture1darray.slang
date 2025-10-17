// THIS IS A GENERATED FILE. DO NOT EDIT!
//
// Texture types capability test: glsl-vk / WTexture1DArray<float4>
// - Supported since: 420
//
// To regenerate, run: ./generate-types-tests.py

//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target glsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -vk -emit-spirv-via-glsl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: RWTexture1D(size=4, content = zero, arrayLength=2):name texHandle
WTexture1DArray<float4> texHandle;

[shader("fragment")]
void fragMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: main
// POSITIVE-NOT: error

// NEGATIVE: result code = -1
// NEGATIVE: error

    int ret = 0;
    texHandle.Store(int2(0, 0), float4(0, 0, 0, 0));
    ret = 0;
    outputBuffer[0] = 0x12345 + ret;
 }

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    texHandle.Store(int2(0, 0), float4(0, 0, 0, 0));
    ret = 0;
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
