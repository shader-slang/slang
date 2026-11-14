// Test that `[vk::offset(N)]` on an entry-point `uniform` parameter places the
// parameter at byte N of the entry point's push-constant block, so that a vertex
// and a fragment entry point can keep their push constants apart (issue #9643).
// Reflection must report the same offsets as the generated code.

//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv -emit-spirv-directly -profile spirv_1_4
//TEST:SIMPLE(filecheck=CHECK_FRAG_SPV): -target spirv -emit-spirv-via-glsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -entry fragmentMain -stage fragment
//TEST:REFLECTION(filecheck=CHECK_REFL): -target spirv -profile spirv_1_4 -no-codegen

uniform Texture2D textures[];
uniform SamplerState samplers[];

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv;
};

// The vertex stage owns bytes [0, 8) of the push-constant range.
[shader("vertex")]
VertexOutput vertexMain(uint vertexID : SV_VertexID, uniform float2* vertices)
{
    VertexOutput output;
    output.position = float4(vertices[vertexID], 0.0, 1.0);
    output.uv = vertices[vertexID];
    return output;
}

// The fragment stage starts at byte 8; `samplerIndex` follows `textureIndex`.
[shader("fragment")]
float4 fragmentMain(
    VertexOutput input,
    [vk::offset(8)] uniform uint textureIndex,
    uniform uint samplerIndex) : SV_Target
{
    return textures[textureIndex].Sample(samplers[samplerIndex], input.uv);
}

// CHECK_SPV-DAG: OpMemberName %[[VERT_PARAMS:[A-Za-z0-9_]+]] 0 "vertices"
// CHECK_SPV-DAG: OpMemberName %[[FRAG_PARAMS:[A-Za-z0-9_]+]] 0 "textureIndex"
// CHECK_SPV-DAG: OpMemberDecorate %[[VERT_PARAMS]] 0 Offset 0
// CHECK_SPV-DAG: OpMemberDecorate %[[FRAG_PARAMS]] 0 Offset 8
// CHECK_SPV-DAG: OpMemberDecorate %[[FRAG_PARAMS]] 1 Offset 12

// CHECK_FRAG_SPV: OpDecorate %[[BLOCK:[A-Za-z0-9_]+]] Block
// CHECK_FRAG_SPV-DAG: OpMemberDecorate %[[BLOCK]] 0 Offset 8
// CHECK_FRAG_SPV-DAG: OpMemberDecorate %[[BLOCK]] 1 Offset 12

// CHECK_GLSL: layout(push_constant)
// CHECK_GLSL-NEXT: layout(std430) uniform
// CHECK_GLSL-NEXT: {
// CHECK_GLSL-NEXT: layout(offset = 8) uint textureIndex
// CHECK_GLSL-NEXT: uint samplerIndex

// CHECK_REFL: "name": "vertexMain"
// CHECK_REFL: "name": "vertices"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 0, "size": 8
// CHECK_REFL: "name": "fragmentMain"
// CHECK_REFL: "name": "textureIndex"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 8, "size": 4
// CHECK_REFL: "name": "samplerIndex"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 12, "size": 4
