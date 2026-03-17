// Test that a geometry shader entry point without required parameters (input
// primitive type, output stream, and [maxvertexcount]) produces clear compiler
// errors instead of silently generating invalid SPIR-V.
// See: https://github.com/shader-slang/slang/issues/9447

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage geometry

//CHECK: geometry shader missing output stream parameter
//CHECK: geometry shader missing [maxvertexcount] attribute

[shader("geometry")]
void main() {}
