//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain -matrix-layout-column-major

// Metal stores a matrix by column, so there is one slot per column, not per
// row. This float2x3 occupies three slots and 24 bytes, and s lands at byte
// offset 24 and slot 3.

struct PB
{
    float2x3 m;
    float s;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.m[0][0] + block.s;
}

// CHECK: "name": "m"
// CHECK: {"kind": "uniform", "offset": 0, "size": 24, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 3}
// CHECK: "name": "s"
// CHECK: {"kind": "uniform", "offset": 24, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 3}
