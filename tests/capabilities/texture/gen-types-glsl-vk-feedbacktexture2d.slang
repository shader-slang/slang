// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit generate-types-tests.py and then regenerate this test
// by running: ./generate-types-tests.py
//
// Texture types capability test: glsl-vk / FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>
// - Type supported since target version:  None
// - Target version for positive test:     None
// - Target version for negative test:     460
//

//TEST:SIMPLE(filecheck=NEGATIVE): -entry fragMain -stage fragment -target glsl -profile glsl_460

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(size=4, content = zero):name texHandle
FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> texHandle;

//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;

Texture2D<float4> feedbackSamplerInput2D;

[shader("fragment")]
void fragMain()
{
// Expect either a compilation error or warning 41012 for upgraded profile to support functionality
// NEGATIVE: {{error [[:digit:]]+|warning 41012}}:

    int ret = 0;
    texHandle.WriteSamplerFeedbackLevel(feedbackSamplerInput2D, samplerState, float2(0, 0), float(0));
    ret = 0;
    outputBuffer[0] = 0x12345 + ret;
}
