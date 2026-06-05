// used-uniforms-array-stride.slang

// Tests per element narrowing for arrays of under aligned element types.
// Under the constant buffer rules an array strides by 16 bytes per
// element regardless of the element's natural size, so float[N], int[N]
// and float2[N] all have a 16 byte stride. The recorded byte ranges must
// land on index times 16, not index times the natural element size. The
// float4[N] case is covered separately in used-uniforms-array-const; here
// every element type is smaller than the stride so a natural size
// computation would point at the wrong byte.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

cbuffer CB
{
    float  f[4];
    int    i[4];
    float2 v2[4];
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    outBuf[tid] = f[1] + float(i[3]) + v2[2].x;
}
