//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Each scalar field occupies four bytes and one slot, so i lands at byte
// offset 4 and slot 1.

struct PB
{
    float s;
    int i;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.s + float(block.i);
}

// CHECK: "name": "s"
// CHECK: {"kind": "uniform", "offset": 0, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0}
// CHECK: "name": "i"
// CHECK: {"kind": "uniform", "offset": 4, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 1}
