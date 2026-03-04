//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -skip-spirv-validation

// Test that SV_CullDistance can be used as output in mesh shaders.

// CHECK: OpCapability CullDistance
// CHECK: OpDecorate %gl_CullDistance BuiltIn CullDistance

struct Vertex
{
    float4 position : SV_Position;
    float cull_dst[2] : SV_CullDistance;
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
        verts[0].cull_dst[0] = 1.0;
        verts[0].cull_dst[1] = 1.0;
        indices[0] = uint3(0, 0, 0);
    }
}
