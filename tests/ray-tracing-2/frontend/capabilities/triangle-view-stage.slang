//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry main -target metal -no-codegen

import slang.raytracing.portable;

RWStructuredBuffer<uint> output;

// Admitting Metal hit stages must not make triangle hit state available in a compute shader.
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
/*CHECK:
     ^^^^ unavailable features in entry point
     ^^^^ 'compute' stage
*/
{
    rt::TriangleData attributes = {};
    output[0] = uint(attributes.frontFacing);
}
