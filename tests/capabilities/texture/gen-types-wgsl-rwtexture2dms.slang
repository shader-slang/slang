// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit generate-types-tests.py and then regenerate this test
// by running: ./generate-types-tests.py
//
// Texture types capability test: wgsl / RWTexture2DMS<float4>
// - Type supported since target version:  None
// - Target version for positive test:     None
// - Target version for negative test:     1.0
//

//TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target wgsl

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: RWTexture2D(size=4, content = zero, sampleCount = two):name texHandle
RWTexture2DMS<float4> texHandle;

[shader("fragment")]
void fragMain()
{
// Expect either a compilation error or warning 41012 for upgraded profile to support functionality
// NEGATIVE: {{error [[:digit:]]+|warning 41012}}:

    int ret = 0;
    ret = int((texHandle.Load(int2(0, 0), 0)).x);
    outputBuffer[0] = 0x12345 + ret;
}
