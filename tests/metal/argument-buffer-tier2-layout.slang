//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry main -matrix-layout-column-major

// A field of a Metal argument buffer carries two units, a byte offset for tier 2 and an
// argument buffer slot for tier 1, and reflection reports both. This checks a matrix and a
// nested constant buffer, each of which takes its own layout path.

struct Inner
{
    float4 v;
}

struct PB
{
    float4x4 m;
    ConstantBuffer<Inner> cb;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = block.m[0][0] + block.cb.v.x;
}

// The matrix occupies 64 bytes and one slot for each of its four columns.
// CHECK: "name": "m"
// CHECK: {"kind": "uniform", "offset": 0, "size": 64, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 4}

// The nested constant buffer is a device pointer of 8 bytes and one slot, placed after the
// matrix, so it lands at byte offset 64 and slot 4.
// CHECK: "name": "cb"
// CHECK: {"kind": "uniform", "offset": 64, "size": 8, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 4}
