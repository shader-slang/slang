//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

Texture2D<int4> t2D;
SamplerState s;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int4> outputBuffer;

void computeMain()
{
    // Integer texture sampling is now supported
    // CHECK: Sample
    outputBuffer[0] = t2D.Sample(s, float2(0.0, 0.0));
}

