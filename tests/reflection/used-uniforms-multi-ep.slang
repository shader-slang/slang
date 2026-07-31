// used-uniforms-multi-ep.slang

// Tests that two entry points reading different fields each report
// only the ranges they actually touch. The expected output should
// list disjoint ranges for mainA and mainB.

//TEST:REFLECTION:-stage compute -entry mainA -target hlsl
//TEST:REFLECTION:-stage compute -entry mainB -target hlsl

cbuffer CB
{
    float4 a;
    float4 b;
    float4 c;
}

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void mainA(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = a;
}

[numthreads(1, 1, 1)]
void mainB(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = c;
}
