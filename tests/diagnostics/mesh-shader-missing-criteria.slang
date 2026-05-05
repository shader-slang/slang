//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0

// Regression test for #9444.
//
// A mesh shader with `outputtopology` but no output vertices/indices
// outputs used to silently produce invalid SPIR-V; missing
// `[outputtopology(...)]` likewise. We now flag both at compile time.

[shader("mesh")]
[outputtopology("point")]
[numthreads(1, 1, 1)]
void main()
/*diag:
     ^^^^ mesh shader missing OutputVertices/OutputIndices output
     ^^^^ mesh shader entry point 'main' must declare both an 'OutputVertices' and an 'OutputIndices' output parameter
*/
{
}
