//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation

// Regression test for #9435: signed `int` is not a valid
// `OutputIndices` element type; only `uint`/`uint2`/`uint3` are.

struct V { float4 pos; };

[shader("mesh")]
[outputtopology("point")]
[numthreads(1, 1, 1)]
void main(out vertices V verts[1], out OutputIndices<int, 0> a)
//diag:                                                      ^ type cannot be used as entry-point varying parameter or return type
//diag:                                                      ^ type 'OutputIndices<int, 0>' cannot be used as output parameter 'a' of entry point 'main' because OutputIndices element type must be uint, uint2, or uint3
{
}
