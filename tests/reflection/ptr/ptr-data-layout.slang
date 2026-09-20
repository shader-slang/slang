//TEST:REFLECTION(filecheck=CHECK):-target spirv -no-codegen

// A pointer's pointee is laid out in reflection using the layout rules its
// data-layout argument selects, matching the layout the SPIR-V backend emits.
// `Ptr<T, ..., Std430DataLayout>` lays `T` out in std430 (a `float3` field aligns
// to 16); `Ptr<T, ..., ScalarDataLayout>` uses scalar rules (a `float3` aligns to
// 4). See shader-slang/slang#13188.

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

struct PushConstants
{
    Ptr<VertexStd430, Access.Read, AddressSpace.Device, Std430DataLayout> vStd430;
    Ptr<VertexScalar, Access.Read, AddressSpace.Device, ScalarDataLayout> vScalar;
};

[vk::push_constant]
PushConstants pc;

[shader("vertex")]
float4 vertexMain(uint vid : SV_VertexID) : SV_Position
{
    return float4(pc.vStd430[vid].color, 1) + float4(pc.vScalar[vid].color, 1);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1);
}

// std430 aligns `float3 color` to 16, so it sits at offset 16 and the pointee
// struct is size 32 / alignment 16.
// CHECK: "name": "VertexStd430"
// CHECK: "name": "color"
// CHECK: "offset": 16
// CHECK: "value": 32, "alignment": 16

// scalar rules align `float3 color` to 4, so it sits at offset 8 and the struct is
// size 20 / alignment 4.
// CHECK: "name": "VertexScalar"
// CHECK: "name": "color"
// CHECK: "offset": 8
// CHECK: "value": 20, "alignment": 4
