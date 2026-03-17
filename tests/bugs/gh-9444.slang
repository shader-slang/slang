// Test that a mesh shader entry point without required parameters (outputtopology,
// out vertices, and out indices) produces clear compiler errors instead of
// silently generating invalid SPIR-V.
// See: https://github.com/shader-slang/slang/issues/9444

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage mesh

//CHECK: mesh shader missing vertices output parameter
//CHECK: mesh shader missing indices output parameter

[shader("mesh")]
[outputtopology("point")]
void main() {}
