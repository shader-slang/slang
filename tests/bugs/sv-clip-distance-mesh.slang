//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -skip-spirv-validation

// Test that SV_ClipDistance can be used as output in mesh shaders (issue #10122).

// CHECK: OpCapability ClipDistance
// CHECK: OpDecorate %gl_ClipDistance BuiltIn ClipDistance

struct Vertex
{
    float4 position : SV_Position;
    noperspective float clip_dst[4] : SV_ClipDistance;
};

[shader("mesh")]
[numthreads(16, 1, 1)]
[outputtopology("triangle")]
void main_mesh(
    uint groupId : SV_GroupID,
    uint threadId : SV_GroupThreadID,
    OutputVertices<Vertex, 1> verts,
    OutputIndices<uint3, 1> indices,
)
{
    if (threadId == 0)
    {
        SetMeshOutputCounts(1, 1);
        verts[0].position = float4(0, 0, 0, 1);
        verts[0].clip_dst[0] = 1.0;
        verts[0].clip_dst[1] = 1.0;
        verts[0].clip_dst[2] = 1.0;
        verts[0].clip_dst[3] = 1.0;
        indices[0] = uint3(0, 0, 0);
    }
}
