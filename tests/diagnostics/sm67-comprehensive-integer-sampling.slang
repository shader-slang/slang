//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Comprehensive test demonstrating SM 6.7 integer texture sampling support
// This validates the fix for issue #8055

Texture2D<uint4> uintTexture;
Texture2D<int4> intTexture;
Texture2D<uint> scalarUintTexture;
Texture2D<int> scalarIntTexture;
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint4> outputBuffer;

void computeMain()
{
    float2 uv = float2(0.5, 0.5);
    float level = 0.0;
    int2 offset = int2(0, 0);
    
    // Test Sample methods with integer textures (the main functionality from the issue)
    uint4 uintResult = uintTexture.Sample(samplerState, uv);
    int4 intResult = intTexture.Sample(samplerState, uv);
    uint scalarUintResult = scalarUintTexture.Sample(samplerState, uv);
    int scalarIntResult = scalarIntTexture.Sample(samplerState, uv);
    
    // Test SampleLevel methods with integer textures
    uint4 uintLevelResult = uintTexture.SampleLevel(samplerState, uv, level);
    int4 intLevelResult = intTexture.SampleLevel(samplerState, uv, level);
    
    // Test SampleLevel with offset
    uint4 uintOffsetResult = uintTexture.SampleLevel(samplerState, uv, level, offset);
    int4 intOffsetResult = intTexture.SampleLevel(samplerState, uv, level, offset);
    
    // Combine results to demonstrate all operations work
    outputBuffer[0] = uintResult + uint4(intResult) + uint4(scalarUintResult) + uint4(scalarIntResult) 
                    + uintLevelResult + uint4(intLevelResult) + uintOffsetResult + uint4(intOffsetResult);
    
    // CHECK: Sample
    // CHECK: SampleLevel
}