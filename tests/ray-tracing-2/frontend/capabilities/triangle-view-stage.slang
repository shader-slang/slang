//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry main -target metal -no-codegen

import slang.raytracing;

RWStructuredBuffer<uint> output;

// Admitting Metal hit stages must not make triangle hit state available in a compute shader.
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    rt::TriangleData attributes = {};
    output[0] = uint(attributes.frontFacing);
}

//CHECK: unavailable features in entry point
//CHECK: 'compute' stage
