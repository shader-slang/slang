//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// A consume structured buffer occupies 16 bytes and one slot for each of its two
// device pointers, the data and the counter, so after lands at byte offset 16 and
// slot 2.

struct PB
{
    ConsumeStructuredBuffer<float> cs;
    float after;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.after;
}

// CHECK: "name": "cs"
// CHECK: {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 2}
// CHECK: "name": "after"
// CHECK: {"kind": "uniform", "offset": 16, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 2}
