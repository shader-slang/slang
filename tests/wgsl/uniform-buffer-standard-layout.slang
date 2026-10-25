//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target wgsl -no-codegen

// A ConstantBuffer with an explicit Std430DataLayout or ScalarDataLayout is a `var<uniform>` laid
// out by WGSL's standard rules (std430: no 16-byte array stride), which needs the
// `uniform_buffer_standard_layout` language extension. WGSL has no scalar layout, so
// ScalarDataLayout gets the same std430 struct. Reflection must report the offsets of the
// struct that is emitted; the default layout keeps std140.

// CHECK: requires uniform_buffer_standard_layout;
// CHECK: struct U_std430_{{[0-9]+}}
// CHECK-NEXT: {
// CHECK-NEXT: a_{{[0-9]+}} : array<f32, i32(4)>,
// CHECK: var<uniform> c430_{{[0-9]+}} : U_std430_{{[0-9]+}};
// CHECK: var<uniform> cscalar_{{[0-9]+}} : U_std430_{{[0-9]+}};
// CHECK: struct _Array_std140_float4_{{[0-9]+}}
// CHECK: var<uniform> cdefault_{{[0-9]+}} : U_std140_{{[0-9]+}};

// REFLECT: "name": "c430",
// REFLECT: "name": "a",
// REFLECT: "offset": 0, "size": 16, "elementStride": 4
// REFLECT: "name": "b",
// REFLECT: "offset": 16, "size": 12
// REFLECT: "name": "c",
// REFLECT: "offset": 28, "size": 4
// REFLECT: "name": "cscalar",
// REFLECT: "name": "a",
// REFLECT: "offset": 0, "size": 16, "elementStride": 4
// REFLECT: "name": "b",
// REFLECT: "offset": 16, "size": 12
// REFLECT: "name": "cdefault",
// REFLECT: "name": "a",
// REFLECT: "offset": 0, "size": 64, "elementStride": 16
// REFLECT: "name": "b",
// REFLECT: "offset": 64, "size": 12

struct U
{
    float a[4];
    float3 b;
    float c;
};

ConstantBuffer<U, Std430DataLayout> c430;
ConstantBuffer<U, ScalarDataLayout> cscalar;
ConstantBuffer<U> cdefault;
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outputBuffer[i] = c430.a[i] + c430.b.y + c430.c + cscalar.a[i] + cscalar.b.z + cdefault.a[i] +
                      cdefault.b.x;
}
