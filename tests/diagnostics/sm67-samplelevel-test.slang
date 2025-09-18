//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Test SM 6.7 integer texture SampleLevel functionality

Texture2D<uint4> uintTexture;
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint4> outputBuffer;

void computeMain()
{
    float2 uv = float2(0.5, 0.5);
    float level = 0.0;
    
    // Test SampleLevel with integer textures
    uint4 result = uintTexture.SampleLevel(samplerState, uv, level);
    outputBuffer[0] = result;
    // CHECK: SampleLevel
}