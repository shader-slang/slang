//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// The inlined struct's two fields each occupy 4 bytes and a slot, so after lands at byte
// offset 8 and slot 2.

struct Inner
{
    float a;
    float b;
}

struct PB
{
    Inner inner;
    float after;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.inner.a + block.after;
}

// CHECK: "name": "after"
// CHECK: {"kind": "uniform", "offset": 8, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 2}
