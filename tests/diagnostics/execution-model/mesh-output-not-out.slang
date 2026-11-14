// Test that mesh shader outputs must be declared with 'out'.
// Both 'vertices' and 'indices' without 'out' produce warnings.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry meshMain -stage mesh

struct VertexOutput
{
    float4 position : SV_Position;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(
    in vertices VertexOutput verts[3],
    /*CHECK:
                             ^^^^^ mesh output must be out
                             ^^^^^ Mesh shader outputs must be declared with 'out'.
    */
    in indices uint3 tris[1])
    /*CHECK:
                     ^^^^ mesh output must be out
                     ^^^^ Mesh shader outputs must be declared with 'out'.
    */
{
}
