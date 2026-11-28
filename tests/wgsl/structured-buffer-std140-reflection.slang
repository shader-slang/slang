//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target wgsl -no-codegen

// A StructuredBuffer with an explicit Std140DataLayout is emitted with std140 element strides on
// WGSL, so reflection reports the same strides. The default layout stays std430.

struct U
{
    float a[4];
    float b;
};

StructuredBuffer<U, Std140DataLayout> s140;
StructuredBuffer<U> sdefault;
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outputBuffer[i] = s140[0].a[i] + s140[0].b + sdefault[0].a[i] + sdefault[0].b;
}

// CHECK: struct _Array_std140_float4_{{[0-9]+}}

// REFLECT: "name": "s140",
// REFLECT: "name": "a",
// REFLECT: "offset": 0, "size": 64, "elementStride": 16
// REFLECT: "name": "b",
// REFLECT: "offset": 64, "size": 4
// REFLECT: "name": "sdefault",
// REFLECT: "name": "a",
// REFLECT: "offset": 0, "size": 16, "elementStride": 4
// REFLECT: "name": "b",
// REFLECT: "offset": 16, "size": 4
