//TEST:SIMPLE(filecheck=CHECK):-target spirv-assembly -entry main -stage mesh -profile glsl_450+spirv_1_4 -obfuscate

// Verify mesh shader compiles with -obfuscate without crashing.

// CHECK: OpEntryPoint MeshEXT %{{.*}} "main"

const static float2 positions[3] = {
  float2(0.0, -0.5),
  float2(0.5, 0.5),
  float2(-0.5, 0.5)
};

struct Vertex
{
    float4 pos : SV_Position;
};

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void main(
    in uint tig : SV_GroupIndex,
    OutputVertices<Vertex, 3> verts,
    OutputIndices<uint3, 1> triangles)
{
    const uint numVertices = 3;
    const uint numPrimitives = 1;
    SetMeshOutputCounts(numVertices, numPrimitives);

    verts[tig] = { float4(positions[tig], 0, 1) };

    if (tig == 0)
    {
        triangles[0] = uint3(0, 1, 2);
    }
}
