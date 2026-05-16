// used-uniforms-single-cb.slang

// Tests per uniform field tracking inside a single constant buffer.
// Only some fields are read; the expected output should list those byte
// ranges and exclude the others.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

cbuffer Params
{
    float4 usedColor;
    float4 unusedColor;
    float  usedScalar;
    float  unusedScalar;
}

RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = usedColor * usedScalar;
}
