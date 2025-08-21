//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Test the exact code from issue #8055

Texture2D<uint4> tex;
SamplerState sampler;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint4> outputBuffer;

void computeMain()
{
    float2 uv = float2(0.5, 0.5);
    float level = 0.0;
    int2 offset = int2(0, 0);
    
    // This was the failing code from the issue
    uint4 c = tex.SampleLevel(sampler, uv, level, offset);
    outputBuffer[0] = c;
    
    // CHECK: SampleLevel
}