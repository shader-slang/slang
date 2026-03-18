//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

Texture2D<int4> t2D;
SamplerState s;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int4> outputBuffer;

void computeMain()
{
    // CHECK: error[E41400]: static assertion failed
    // CHECK: HLSL supports only float and half type textures
    outputBuffer[0] = t2D.Sample(s, float2(0.0, 0.0));
}

