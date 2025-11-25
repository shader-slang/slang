//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target glsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -vk -emit-spirv-via-glsl

// This is a minimal repro for issue 8802

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(format=D32Float, size=4, content = zero):name texHandle
DepthTexture2D texHandle;

//TEST_INPUT: Sampler(depthCompare):name samplerComparisonState
SamplerComparisonState samplerComparisonState;

[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: main
// POSITIVE-NOT: {{(error|warning).*}}:

// Check that we get a properly constructed shadow sampler
// POSITIVE: textureLod(sampler2DShadow(texHandle{{[^,]*}},samplerComparisonState{{[^,)]*}}),
// POSITIVE-NOT: {{(error|warning).*}}:

    int ret = 0;
    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float2(0, 0), float(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    ret = int((texHandle.SampleCmpLevelZero(samplerComparisonState, float2(0, 0), float(0))).x);
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
