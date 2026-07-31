// used-uniforms-array-const.slang

// Tests per element narrowing for an array indexed by literal indices.
// The expected output should list ranges only for elements 0 and 2.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

cbuffer CB
{
    float4 colors[4];
    float4 misc;
}

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = colors[0] + colors[2];
}
