//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry computeMain

// The texture array occupies 32 bytes and one slot for each of its four elements, so v lands
// at byte offset 32 and slot 4.

struct PB
{
    Texture2D tex[4];
    float4 v;
}

ParameterBlock<PB> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = block.tex[0].Load(int3(0, 0, 0)).x + block.v.x;
}

// CHECK: "name": "tex"
// CHECK: {"kind": "uniform", "offset": 0, "size": 32, "elementStride": 8}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 0, "count": 4}
// CHECK: "name": "v"
// CHECK: {"kind": "uniform", "offset": 32, "size": 16, "elementStride": 4}
// CHECK: {"kind": "metalArgumentBufferElement", "index": 4}
