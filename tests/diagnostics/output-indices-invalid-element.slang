//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation

// Regression test for #9435.
//
// `OutputIndices<T, N>` requires `T` to be `uint` (point indices),
// `uint2` (line indices), or `uint3` (triangle indices). A non-integral
// element type (such as `float`) used to crash GLSL legalization with
// a null `IRIntLit` dereference; it now produces a clean diagnostic.

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out OutputIndices<float, 0> a)
//diag:                               ^ type cannot be used as entry-point varying parameter or return type
//diag:                               ^ type 'OutputIndices<float, 0>' cannot be used as output parameter 'a' of entry point 'main' because OutputIndices element type must be uint, uint2, or uint3
{
}
