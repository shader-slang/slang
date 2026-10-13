//TEST:REFLECTION:-profile cs_6_0 -target hlsl -no-codegen

// Confirm that every type layout in the reflection JSON reports its size in each layout unit
// it has a non-zero size in, via the `sizes` array, and that the bytes (uniform) unit also
// reports alignment. Covers a struct with mixed uniform members, a fixed-size array, a scalar,
// and a resource (which reports a size in a resource layout unit rather than in bytes).

struct S
{
    float3 a;
    float  b;
    float4 c[2];
};

ConstantBuffer<S> cb;

StructuredBuffer<uint> buf;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = cb.a.x + cb.b + cb.c[0].x + (float)buf[0];
}
