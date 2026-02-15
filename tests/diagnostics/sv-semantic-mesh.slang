//TEST:SIMPLE(filecheck=CHECK): -target spirv -skip-spirv-validation

// Test system value semantic validation for mesh shaders.
// Mesh shaders have specific allowed inputs and outputs.

// Test 1: Invalid input semantic in mesh shader
// SV_VertexID is only valid in vertex shaders, not mesh shaders

struct Vertex
{
    float4 pos : SV_Position;
};

struct Primitive
{
    uint primId : SV_PrimitiveID;
};

// CHECK: ([[# @LINE+5]]): error 30702: system value semantic 'SV_VertexID' cannot be used as input in 'mesh' shader stage
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
[shader("mesh")]
void meshMainInvalidInput(
    uint vertexId : SV_VertexID,
    OutputVertices<Vertex, 3> verts,
    OutputIndices<uint3, 1> indices,
    OutputPrimitives<Primitive, 1> prims)
{
    SetMeshOutputCounts(3, 1);
    verts[0].pos = float4(0, 0, 0, 1);
    verts[1].pos = float4(1, 0, 0, 1);
    verts[2].pos = float4(0, 1, 0, 1);
    indices[0] = uint3(0, 1, 2);
    prims[0].primId = 0;
}

// Test 2: Invalid output semantic in mesh shader
// SV_Depth is only valid as output in fragment shaders

struct VertexWithDepth
{
    float4 pos : SV_Position;
    // CHECK: ([[# @LINE+1]]): error 30702: system value semantic 'SV_Depth' cannot be used as output in 'mesh' shader stage
    float depth : SV_Depth;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
[shader("mesh")]
void meshMainInvalidOutput(
    uint tid : SV_GroupIndex,
    OutputVertices<VertexWithDepth, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    verts[0].pos = float4(0, 0, 0, 1);
    verts[0].depth = 0.5;
    verts[1].pos = float4(1, 0, 0, 1);
    verts[1].depth = 0.5;
    verts[2].pos = float4(0, 1, 0, 1);
    verts[2].depth = 0.5;
    indices[0] = uint3(0, 1, 2);
}
