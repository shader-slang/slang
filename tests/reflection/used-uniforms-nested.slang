// used-uniforms-nested.slang

// Tests narrowing through nested struct field access. The expected
// output should list the byte ranges for the leaves that are read and
// nothing for the sibling subtrees that are not.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

struct Inner
{
    float4 a;
    float4 b;
}

struct Outer
{
    Inner used;
    Inner unused;
    float topUsed;
    float topUnused;
}

cbuffer CB
{
    Outer params;
}

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = params.used.a + float4(params.topUsed.x);
}
