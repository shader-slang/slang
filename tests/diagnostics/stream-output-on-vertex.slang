//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation

// Regression test for #9430.
//
// Stream output types are only meaningful on geometry shader entry
// points; placing them on a vertex shader (or any non-geometry stage)
// used to segfault during code generation.

[shader("vertex")]
void main(LineStream<int> a)
//diag:                   ^ type cannot be used as entry-point varying parameter or return type
//diag:                   ^ type 'LineStream<int>' cannot be used as input parameter 'a' of entry point 'main' because stream output types are only valid on geometry shader entry points
{
}
