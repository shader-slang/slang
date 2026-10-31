// A large vector whose element count is a link-time constant is laid out as an array once the
// count is resolved, and reflection reports the array's element stride: 16 bytes under std140,
// not the 4-byte element size.

//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target spirv -no-codegen

extern static const int kCount = 6;

// CHECK: "name": "constants"
// CHECK: "name": "v"
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 96, "elementStride": 16}
// CHECK: "name": "tag"
// CHECK: "binding": {"kind": "uniform", "offset": 96, "size": 4
cbuffer constants
{
    vector<float, kCount> v;
    float tag;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = v[5] + tag;
}
