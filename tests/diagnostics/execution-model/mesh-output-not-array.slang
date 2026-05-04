// Test that mesh shader outputs must be arrays.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry meshMain -stage mesh

struct VertexOutput
{
    float4 position : SV_Position;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(
    out vertices VertexOutput verts,
    /*CHECK:
                              ^^^^^ mesh output must be array
                              ^^^^^ HLSL style mesh shader outputs must be arrays
    */
    out indices uint3 tris[1])
{
}
