//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0

// Test for #9444: mesh shader has OutputVertices but no OutputIndices.

struct V {
    float4 pos : SV_Position;
};

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out OutputVertices<V, 3> verts)
/*diag:
     ^^^^ mesh shader missing OutputVertices/OutputIndices output
     ^^^^ mesh shader entry point 'main' must declare both an 'OutputVertices' and an 'OutputIndices' output parameter
*/
{
}
