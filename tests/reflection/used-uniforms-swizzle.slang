// used-uniforms-swizzle.slang

// Tests narrowing through a vector swizzle. Only one component of the
// vector is read so the expected output should list a sub range that
// is smaller than the full vector.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

cbuffer CB
{
    float4 vec;
    float4 unused;
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = vec.y;
}
