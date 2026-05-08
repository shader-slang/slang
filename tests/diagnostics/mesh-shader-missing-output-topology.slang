//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0

// Regression test for #9444 (`[outputtopology(...)]` missing variant).

[shader("mesh")]
[numthreads(1, 1, 1)]
void main(out OutputVertices<int, 0> verts, out OutputIndices<uint3, 0> prims)
/*diag:
     ^^^^ mesh shader missing [outputtopology] attribute
     ^^^^ mesh shader entry point 'main' must have an '[outputtopology("point"|"line"|"triangle")]' attribute
*/
{
    SetMeshOutputCounts(0, 0);
}
