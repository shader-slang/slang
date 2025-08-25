//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_7

// Test that SM 6.7 allows integer texture sampling

Texture2D<uint4> uintTexture;
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint4> outputBuffer;

void computeMain()
{
    float2 uv = float2(0.5, 0.5);
    
    // This should now work in SM 6.7+
    // CHECK-NOT: error 41400: {{.*}} HLSL supports only float and half type textures
    uint4 result = uintTexture.Sample(samplerState, uv);
    outputBuffer[0] = result;
}