// Test that a geometry shader entry point without required parameters (input
// primitive type, output stream, and [maxvertexcount]) produces clear compiler
// errors instead of silently generating invalid SPIR-V.
// See: https://github.com/shader-slang/slang/issues/9447

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage geometry

//CHECK: geometry shader missing output stream parameter
//CHECK: geometry shader missing [maxvertexcount] attribute
[shader("geometry")]
void main() {}

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2,non-exhaustive): -target spirv -entry gsNoMaxVertex -stage geometry
//CHECK2: geometry shader missing [maxvertexcount] attribute
[shader("geometry")]
void gsNoMaxVertex(triangle float4 input[3], inout TriangleStream<float4> stream) {}

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3,non-exhaustive): -target spirv -entry gsNoStream -stage geometry
//CHECK3: geometry shader missing output stream parameter
[shader("geometry")]
[maxvertexcount(3)]
void gsNoStream(triangle float4 input[3]) {}
