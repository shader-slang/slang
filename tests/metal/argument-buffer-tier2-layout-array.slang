//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// The array occupies 16 bytes and one slot for each of its four elements,
// so v lands at byte offset 16 and slot 4.

struct PB
{
    float arr[4];
    float4 v;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.arr[0] + block.v.x;
}

// CHECK: "name": "arr"
// CHECK: {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 4}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 4}
// CHECK: "name": "v"
// CHECK: {"kind": "uniform", "offset": 16, "size": 16, "elementStride": 4}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 4}
