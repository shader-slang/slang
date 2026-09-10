//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// A combined texture sampler occupies 16 bytes and one slot for each of its two handles, the
// texture and the sampler, so s lands at byte offset 16 and slot 2.

struct PB
{
    Sampler2D ts;
    float s;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.s;
}

// CHECK: "name": "ts"
// CHECK: {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 2}
// CHECK: "name": "s"
// CHECK: {"kind": "uniform", "offset": 16, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 2}
