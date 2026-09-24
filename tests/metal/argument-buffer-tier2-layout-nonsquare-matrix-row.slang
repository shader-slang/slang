//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain -matrix-layout-row-major

// Metal can store a matrix by row, so there is one slot per row, not per
// column. This float2x3 occupies two slots and 32 bytes, and s lands at byte
// offset 32 and slot 2.

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
// CHECK: {"kind": "uniform", "offset": 0, "size": 32, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 2}
// CHECK: "name": "s"
// CHECK: {"kind": "uniform", "offset": 32, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 2}
