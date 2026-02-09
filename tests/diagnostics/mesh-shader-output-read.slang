//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry entry_mesh -stage mesh

const static float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

struct Vertex
{
    float4 pos : SV_Position;
    [mutating]
    func set_pos(float4 v) { pos = v; }
};

struct Primitive
{
    [[vk::location(0)]] nointerpolation float3 tint;
    [mutating]
    func set_tint(float3 v) { tint = v; }
}

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

[outputtopology("triangle")]
[numthreads(3, 1, 1)]
[shader("mesh")]
void entry_mesh(
    in uint tig: SV_GroupIndex,
    OutputIndices<uint3, MAX_PRIMS> triangles,
    OutputVertices<Vertex, MAX_VERTS> verts,
    OutputPrimitives<Primitive, MAX_PRIMS> primitives)
{
    const uint numVertices = 3;
    const uint numPrimitives = 1;
    SetMeshOutputCounts(numVertices, numPrimitives);

    // this should error: reading from mesh shader output isn't allowed (but DCE removes this)
    // C HECK: (41): error 54005:
    Vertex v = verts[2];

    if (tig < numVertices) {
        // this should not error: this parameter passing mode shouldn't result in a read from
        // positions[tig].
        verts[tig].set_pos(float4(positions[tig], 0, 0));

        // this should error: reading from mesh shader output isn't allowed
        // CHECK: (50): error 54005:
        verts[tig].pos.x += 1.0f;
    }

    if (tig < numPrimitives) {
        triangles[tig] = uint3(0, 1, 2);

        // this should error: reading from mesh shader output isn't allowed
        // CHECK: (58): error 54005:
        triangles[tig].x += 1;

        // this should not error: this parameter passing mode shouldn't result in a read from
        // primitives[tig].
        primitives[tig].set_tint(float3(1, 0, 0));

        // this should error: reading from mesh shader output isn't allowed
        // CHECK: (66): error 54005:
        primitives[tig].tint.x += 1.0f;
    }

    // this should error: reading from mesh shader output isn't allowed (but DCE removes this)
    // C HECK: (71): error 54005:
    uint uiColor = triangles[0].x + 2;
}
