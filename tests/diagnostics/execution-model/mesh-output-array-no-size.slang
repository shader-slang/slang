// Test that mesh shader output arrays must have a size.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry meshMain -stage mesh

struct VertexOutput
{
    float4 position : SV_Position;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(
/*CHECK:
     ^^^^^^^^ mesh shader missing OutputVertices/OutputIndices output
     ^^^^^^^^ mesh shader entry point 'meshMain' must declare both an 'OutputVertices' and an 'OutputIndices' output parameter
*/
    out vertices VertexOutput verts[],
    /*CHECK:
                              ^^^^^ mesh output array must have size
                              ^^^^^ HLSL style mesh shader output arrays must have a length specified
    */
    out indices uint3 tris[1])
{
}
