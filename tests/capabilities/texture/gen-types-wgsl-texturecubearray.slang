// THIS IS A GENERATED FILE. DO NOT EDIT!
//
// Texture types capability test: wgsl / TextureCubeArray<float4>
// - Supported since: 1.0
//
// To regenerate, run: ./generate-types-tests.py

//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target wgsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -wgpu

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: TextureCube(size=4, content = zero, arrayLength=2):name texHandle
TextureCubeArray<float4> texHandle;

//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;

[shader("fragment")]
void fragMain()
{
// POSITIVE: result code = 0
// POSITIVE-LABEL: fragMain
// POSITIVE-NOT: error

// NEGATIVE: result code = -1
// NEGATIVE: error

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
// POSITIVE_RESULT: 12345
