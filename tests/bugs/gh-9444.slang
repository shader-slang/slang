// Test that a mesh shader entry point without required parameters (outputtopology,
// out vertices, and out indices) produces clear compiler errors instead of
// silently generating invalid SPIR-V.
// See: https://github.com/shader-slang/slang/issues/9444

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1,non-exhaustive): -target spirv -entry main1 -stage mesh
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2,non-exhaustive): -target spirv -entry main2 -stage mesh
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3,non-exhaustive): -target spirv -entry main3 -stage mesh

struct Vertex
{
    float4 pos : SV_Position;
    float3 color;
}

const static uint MAX_VERTS = 3;
const static uint MAX_PRIMS = 1;

[shader("mesh")]
[outputtopology("point")]
void main1(in uint tig, out vertices Vertex verts[MAX_VERTS]) {}
//CHECK1: mesh shader missing indices output parameter

[shader("mesh")]
[outputtopology("point")]
void main2(in uint tig, out indices uint3 triangles[MAX_PRIMS]) {}
//CHECK2: mesh shader missing vertices output parameter

[shader("mesh")]
void main3(in uint tig, out vertices Vertex verts[MAX_VERTS], out indices uint3 triangles[MAX_PRIMS]) {}
//CHECK3: mesh shader missing [outputtopology] attribute
