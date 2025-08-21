//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Test SM 6.7 integer texture sampling with original Sample method

Texture2D<uint> uintTexture;
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void computeMain()
{
    float2 uv = float2(0.5, 0.5);
    
    // Test with scalar uint (simpler case)
    uint result = uintTexture.Sample(samplerState, uv);
    outputBuffer[0] = result;
    // CHECK: Sample
}