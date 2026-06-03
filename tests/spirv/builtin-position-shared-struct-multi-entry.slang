//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry vs_main -entry mesh_main -entry ps_main -emit-spirv-directly -profile glsl_450+spirv_1_4

// Regression test for https://github.com/shader-slang/slang/issues/10872
// A single shared struct with SV_Position used across vertex, mesh, and pixel
// entry points must produce separate gl_Position variables for vertex (float4)
// and mesh (float4[N]) because the pointee types differ.

// CHECK-DAG: OpDecorate %gl_Position BuiltIn Position
// CHECK-DAG: OpDecorate %gl_Position_0 BuiltIn Position

struct Interp
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
};

[shader("vertex")]
Interp vs_main(uint vid : SV_VertexID)
{
    Interp o;
    o.position = float4(float(vid), 0, 0, 1);
    o.normal = float3(0, 1, 0);
    o.uv = float2(0, 0);
    return o;
}

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void mesh_main(
    in uint tig : SV_GroupIndex,
    OutputVertices<Interp, 3> verts,
    OutputIndices<uint3, 1> triangles)
{
    SetMeshOutputCounts(3, 1);
    if (tig < 3)
    {
        Interp v;
        v.position = float4(float(tig), 0, 0, 1);
        v.normal = float3(0, 1, 0);
        v.uv = float2(0, 0);
        verts[tig] = v;
    }
    if (tig < 1)
    {
        triangles[tig] = uint3(0, 1, 2);
    }
}

[shader("pixel")]
float4 ps_main(Interp interp) : SV_Target0
{
    return interp.position;
}
