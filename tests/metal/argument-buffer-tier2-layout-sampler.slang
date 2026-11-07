//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// The sampler occupies 8 bytes and one slot, so s lands at byte offset 8 and slot 1.

struct PB
{
    SamplerState samp;
    float s;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.s;
}

// CHECK: "name": "samp"
// CHECK: {"kind": "uniform", "offset": 0, "size": 8, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0}
// CHECK: "name": "s"
// CHECK: {"kind": "uniform", "offset": 8, "size": 4, "elementStride": 0}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 1}
