// THIS IS A GENERATED FILE. DO NOT EDIT!
//
// Texture types capability test: wgsl / RasterizerOrderedTexture2D<float4>
// - Supported since: None
//
// To regenerate, run: ./generate-types-tests.py

//TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target wgsl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(size=4, content = zero):name texHandle
RasterizerOrderedTexture2D<float4> texHandle;

[shader("fragment")]
void fragMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: fragMain
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
