//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Test SM 6.7 integer texture sampling functionality

//TEST_INPUT: Texture2D(size=4, content=one, format=R32_UINT):name uintTexture
Texture2D<uint4> uintTexture;

//TEST_INPUT: Texture2D(size=4, content=one, format=R32_SINT):name intTexture  
Texture2D<int4> intTexture;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint4> outputBuffer;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 uv = float2(0.5, 0.5);
    uint2 offset = uint2(0, 0);
    float level = 0.0;
    
    // Test Sample methods with integer textures (SM 6.7+)
    uint4 uintResult = uintTexture.Sample(samplerState, uv);
    int4 intResult = intTexture.Sample(samplerState, uv);
    
    // Test SampleLevel methods with integer textures (SM 6.7+)
    uint4 uintLevelResult = uintTexture.SampleLevel(samplerState, uv, level);
    int4 intLevelResult = intTexture.SampleLevel(samplerState, uv, level);
    
    // Test SampleLevel with offset
    uint4 uintOffsetResult = uintTexture.SampleLevel(samplerState, uv, level, offset);
    int4 intOffsetResult = intTexture.SampleLevel(samplerState, uv, level, offset);
    
    // Store results
    outputBuffer[0] = uintResult + uint4(intResult) + uintLevelResult + uint4(intLevelResult) + uintOffsetResult + uint4(intOffsetResult);
    
    // CHECK: 6
}