//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation

// Regression test for #9435: only `uint` (points), `uint2` (lines)
// and `uint3` (triangles) are valid `OutputIndices` element shapes.
// `uint4` previously slipped through the front end and produced
// invalid SPIR-V.

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out OutputVertices<float4, 3> verts, out OutputIndices<uint4, 1> prims)
//diag:                                                                    ^^^^^ type cannot be used as entry-point varying parameter or return type
//diag:                                                                    ^^^^^ type 'OutputIndices<vector<uint,4>, 1>' cannot be used as output parameter 'prims' of entry point 'main' because OutputIndices element type must be uint, uint2, or uint3
{
    SetMeshOutputCounts(3, 1);
}
