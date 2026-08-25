//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// A constant buffer is not a Metal argument buffer, so its fields carry a byte
// offset but no argument buffer slot.

struct Inner
{
    float4 v;
    float s;
}

ConstantBuffer<Inner> cb;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = cb.v.x + cb.s;
}

// CHECK: "name": "cb"
// CHECK: "binding": {"kind": "constantBuffer", "index": 0}
// CHECK-NOT: metalArgumentBufferElement
// CHECK: "name": "v"
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 16, "elementStride": 4}
// CHECK-NOT: metalArgumentBufferElement
// CHECK: "name": "s"
// CHECK: "binding": {"kind": "uniform", "offset": 16, "size": 4, "elementStride": 0}
// CHECK-NOT: metalArgumentBufferElement
