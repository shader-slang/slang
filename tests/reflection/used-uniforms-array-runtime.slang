// used-uniforms-array-runtime.slang

// Tests the conservative fallback for an array indexed by a runtime
// value. The expected output should mark the whole array as used since
// the index is not known at compile time.

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
    outBuf[tid] = colors[tid];
}
