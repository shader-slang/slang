//TEST:REFLECTION(filecheck=CHECK):-target spirv -no-codegen

// A pointer's pointee is laid out in reflection using the layout rules its
// data-layout argument selects, matching the layout the SPIR-V backend emits.
// `Ptr<T, ..., Std430DataLayout>` lays `T` out in std430 (a `float3` field aligns
// to 16), `Ptr<T, ..., Std140DataLayout>` in std140 (an array element rounds its
// stride up to 16), and both an explicit `ScalarDataLayout` and a pointer with no
// data-layout argument (which resolves to the DefaultDataLayout) use scalar rules.
// See shader-slang/slang#13188.
//
// Each pointer references a distinctly-named struct so the reflection JSON emits a
// separately-named layout that the CHECK-LABELs below can anchor on; the structs
// are otherwise field-identical on purpose — do not merge them.

struct VertexStd430
{
    float2 position;
    float3 color;
};

struct VertexScalar
{
    float2 position;
    float3 color;
};

struct VertexDefault
{
    float2 position;
    float3 color;
};

struct ArrStd140
{
    float2 v[2];
};

struct ArrScalar
{
    float2 v[2];
};

struct PushConstants
{
    Ptr<VertexStd430, Access.Read, AddressSpace.Device, Std430DataLayout> vStd430;
    Ptr<VertexScalar, Access.Read, AddressSpace.Device, ScalarDataLayout> vScalar;
    // No data-layout argument: resolves to DefaultDataLayout, which uses scalar rules.
    Ptr<VertexDefault, Access.Read, AddressSpace.Device> vDefault;
    Ptr<ArrStd140, Access.Read, AddressSpace.Device, Std140DataLayout> vArr140;
    Ptr<ArrScalar, Access.Read, AddressSpace.Device, ScalarDataLayout> vArrScalar;
};

[vk::push_constant]
PushConstants pc;

[shader("vertex")]
float4 vertexMain(uint vid : SV_VertexID) : SV_Position
{
    return float4(pc.vStd430[vid].color, 1) + float4(pc.vScalar[vid].color, 1) +
           float4(pc.vDefault[vid].color, 1) + float4(pc.vArr140[vid].v[1], 0, 1) +
           float4(pc.vArrScalar[vid].v[1], 0, 1);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1);
}

// std430 aligns `float3 color` to 16, so it sits at offset 16 and the pointee
// struct is size 32 / alignment 16.
// CHECK-LABEL: "name": "VertexStd430"
// CHECK: "name": "color"
// CHECK: "offset": 16
// CHECK: "value": 32, "alignment": 16

// An explicit ScalarDataLayout aligns `float3 color` to 4, so it sits at offset 8
// and the struct is size 20 / alignment 4.
// CHECK-LABEL: "name": "VertexScalar"
// CHECK: "name": "color"
// CHECK: "offset": 8
// CHECK: "value": 20, "alignment": 4

// A pointer with no data-layout argument falls back to scalar rules, identical to
// the explicit ScalarDataLayout case above.
// CHECK-LABEL: "name": "VertexDefault"
// CHECK: "name": "color"
// CHECK: "offset": 8
// CHECK: "value": 20, "alignment": 4

// std140 rounds the `float2` array element stride up to 16, so the two-element
// array is size 32 / alignment 16.
// CHECK-LABEL: "name": "ArrStd140"
// CHECK: "value": 32, "alignment": 16

// scalar rules keep the `float2` array element stride at 8, so the same array is
// size 16 / alignment 4.
// CHECK-LABEL: "name": "ArrScalar"
// CHECK: "value": 16, "alignment": 4
