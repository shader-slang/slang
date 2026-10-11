// `readonly` and `writeonly` stay the GLSL memory qualifiers on a `__ref` parameter, in either
// keyword order, so they are rejected on ordinary data rather than selecting a reference mode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

int readBefore(readonly __ref int x) { return x; }
//CHECK:                          ^ E31206
//CHECK:                          ^ modifier readonly is not allowed on a non image type parameter.

int readAfter(__ref readonly int x) { return x; }
//CHECK:                         ^ E31206
//CHECK:                         ^ modifier readonly is not allowed on a non image type parameter.

void writeBefore(writeonly __ref int x) { x = 1; }
//CHECK:                             ^ E31206
//CHECK:                             ^ modifier writeonly is not allowed on a non image type parameter.

[numthreads(1, 1, 1)]
void main()
{
}
