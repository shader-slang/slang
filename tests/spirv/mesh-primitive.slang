// TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// VUID-CullPrimitiveEXT-CullPrimitiveEXT-07038
// CHECK: OpDecorate %[[CULLPRIMITIVE:[0-9]+]] BuiltIn CullPrimitiveEXT
// CHECK: OpDecorate %[[CULLPRIMITIVE]] PerPrimitive

// CHECK: OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK: OpDecorate %gl_PrimitiveID PerPrimitiveEXT
// CHECK: OpDecorate %out_primitives_color Location 0
// CHECK: OpDecorate %out_primitives_color PerPrimitiveEXT
// CHECK: OpDecorate %out_primitives_values Location 2
// CHECK: OpDecorate %out_primitives_values PerPrimitiveEXT
// CHECK: OpDecorate %entryPointParam_entry_mesh_out_normal Location 1
// CHECK: OpDecorate %entryPointParam_entry_mesh_out_normal PerPrimitiveEXT

// CHECK: OpDecorate %gl_PrimitiveID_0 BuiltIn PrimitiveId
// CHECK: OpDecorate %gl_PrimitiveID_0 PerPrimitiveEXT
// CHECK: OpDecorate %in_normal Location 1
// CHECK: OpDecorate %in_normal PerPrimitiveEXT
// CHECK: OpDecorate %in_color Location 0
// CHECK: OpDecorate %in_color PerPrimitiveEXT
// CHECK: OpDecorate %in_values Location 2
// CHECK: OpDecorate %in_values PerPrimitiveEXT

const static uint MAX_VERTS = 6;
const static uint MAX_PRIMS = 2;

const static float2 positions[MAX_VERTS] = {
    float2(0.0, -0.5),
    float2(0.5, 0),
    float2(-0.5, 0),
    float2(0.0, 0.5),
    float2(0.5, 0),
    float2(-0.5, 0),
};

struct Vertex
{
    float4 pos : SV_Position;
};

struct Primitive
{
    bool cull : SV_CullPrimitive;
    uint primitive_id : SV_PrimitiveID;
    [[vk::location(0)]] float3 color;
    [[vk::location(2)]] float values[2];
}

[[vk::location(1)]] out perprimitive float3 out_normal[MAX_PRIMS];

[outputtopology("triangle")]
[numthreads(MAX_VERTS, 1, 1)]
[shader("mesh")]
void entry_mesh(
    in uint tig : SV_GroupThreadID,
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputIndices<uint3, MAX_PRIMS> triangles,
    OutputPrimitives<Primitive, MAX_PRIMS> out_primitives)
{
    const uint numVertices = MAX_VERTS;
    const uint numPrimitives = MAX_PRIMS;
    SetMeshOutputCounts(numVertices, numPrimitives);

    if(tig < numVertices) {
        verts[tig] = {float4(positions[tig], 0, 1)};
    }

    if (tig < numPrimitives) {
        triangles[tig] = uint3(0, 1, 2) + tig * 3;
        out_primitives[tig] = { false, tig, float3(1, 0, 0) };
        out_normal[tig] = float3(1.0, 0.0, 0.0);
    }
}

struct FragmentOut
{
    [[vk::location(0)]] float3 frag_color;
};

[[vk::location(1)]] in perprimitive float3 in_normal;
[[vk::location(2)]] in perprimitive float in_values[2];
in perprimitive uint in_primitive_id : SV_PrimitiveID;

[shader("fragment")]
FragmentOut entry_fragment([[vk::location(0)]] in perprimitive float3 in_color, )
{
    let n_dot_l = dot(in_normal, float3(0.0, 1.0, 0.0)) + in_values[0] + in_primitive_id;

    FragmentOut frag_out;
    frag_out.frag_color = in_color * n_dot_l;
    return frag_out;
}
