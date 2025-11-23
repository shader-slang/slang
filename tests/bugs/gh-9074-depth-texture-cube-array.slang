//TEST:SIMPLE(filecheck=POSITIVE): -entry fragMain -stage fragment -target glsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): -vk -emit-spirv-via-glsl

// This is a minimal repro for issue 8802

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: TextureCube(format=D32Float, size=4, arrayLength=2, content = zero):name texHandle
DepthTextureCubeArray texHandle;

//TEST_INPUT: Sampler(depthCompare):name samplerComparisonState
SamplerComparisonState samplerComparisonState;

//TEST_INPUT: TextureSamplerCube(format=D32Float, size=4, content = zero, arrayLength=2):name sampler
SamplerCubeArrayShadow sampler;

[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE: result code = 0
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: main
// POSITIVE-NOT: {{(error|warning).*}}:


    int ret = 0;

// POSITIVE: texture(samplerCubeArrayShadow(texHandle{{[^,]*}},samplerComparisonState{{[^,)]*}}),
    ret += int((texHandle.SampleCmp(samplerComparisonState, float4(0, 0, 0, 0), float(0))).x);

// POSITIVE-COUNT-2: textureLod(samplerCubeArrayShadow(texHandle{{[^,]*}},samplerComparisonState{{[^,)]*}}),
    ret += int((texHandle.SampleCmpLevelZero(samplerComparisonState, float4(0, 0, 0, 0), float(0))).x);
    ret += int((texHandle.SampleCmpLevel(samplerComparisonState, float4(0, 0, 0, 0), float(0), float(0))).x);

// POSITIVE: texture{{\(+}}sampler
    ret += int((sampler.SampleCmp(float4(0, 0, 0, 0), float(0))).x);

// POSITIVE-COUNT-2: textureLod{{\(+}}sampler
    ret += int((sampler.SampleCmpLevelZero(float4(0, 0, 0, 0), float(0))).x);
    ret += int((sampler.SampleCmpLevel(float4(0, 0, 0, 0), float(0), float(0))).x);


// POSITIVE-NOT: {{(error|warning).*}}:

    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;

    ret += int((texHandle.SampleCmpLevelZero(samplerComparisonState, float4(0, 0, 0, 0), float(0))).x);
    ret += int((sampler.SampleCmpLevelZero(float4(0, 0, 0, 0), float(0))).x);

    ret += int((texHandle.SampleCmpLevel(samplerComparisonState, float4(0, 0, 0, 0), float(0), float(0))).x);
    ret += int((sampler.SampleCmpLevel(float4(0, 0, 0, 0), float(0), float(0))).x);

    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12345
