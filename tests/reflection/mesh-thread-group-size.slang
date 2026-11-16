// Mesh and amplification entry points declare `[numthreads]` like compute ones, and the JSON
// reflection reports it for all three. Metal sizes object and mesh threadgroups at draw time,
// so a host needs this to dispatch at the size the shader requires.

//TEST:REFLECTION(filecheck=MESH): -target spirv -stage mesh -entry meshMain
//TEST:REFLECTION(filecheck=AMPLIFICATION): -target spirv -stage amplification -entry taskMain

// MESH: "stage": "mesh"
// MESH: "threadGroupSize": [32, 2, 1]

// AMPLIFICATION: "stage": "amplification"
// AMPLIFICATION: "threadGroupSize": [16, 1, 1]

struct Vertex
{
    float4 position : SV_Position;
};

struct Payload
{
    uint count;
};

[shader("mesh")]
[numthreads(32, 2, 1)]
[outputtopology("triangle")]
void meshMain(out vertices Vertex verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);
}

groupshared Payload gPayload;

[shader("amplification")]
[numthreads(16, 1, 1)]
void taskMain()
{
    gPayload.count = 1;
    DispatchMesh(1, 1, 1, gPayload);
}
