//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry vertMain -entry meshMain -emit-spirv-directly -profile glsl_450+spirv_1_4

// Verify that vertex and mesh entry points get separate gl_Position variables
// when they have different types (float4 vs float4[3]).
// CHECK-DAG: OpDecorate %gl_Position BuiltIn Position
// CHECK-DAG: OpDecorate %gl_Position_0 BuiltIn Position
// CHECK-DAG: %gl_Position = OpVariable %_ptr_Output_v4float Output
// CHECK-DAG: %gl_Position_0 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output

struct VOut
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VOut vertMain(uint vid : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    return o;
}

struct MeshVertex
{
    float4 pos : SV_Position;
};

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void meshMain(
    in uint tig : SV_GroupIndex,
    OutputVertices<MeshVertex, 3> verts,
    OutputIndices<uint3, 1> triangles)
{
    SetMeshOutputCounts(3, 1);
    if (tig < 3)
    {
        MeshVertex v;
        v.pos = float4(float(tig), 0, 0, 1);
        verts[tig] = v;
    }
    if (tig < 1)
    {
        triangles[tig] = uint3(0, 1, 2);
    }
}
