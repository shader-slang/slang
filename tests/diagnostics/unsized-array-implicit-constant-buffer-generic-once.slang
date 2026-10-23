// shader-slang/slang#13530: a non-generic unsized array of ordinary data next to a generic one
// is diagnosed by semantic checking. That error stops compilation before specialization, so it
// is reported once and the generic parameter is not checked again.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -specialize Data

interface IValue
{
    float4 get();
}

struct Data : IValue
{
    float4 v;
    float4 get() { return v; }
}

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main<T : IValue>(uniform T values[], uniform float4 extra[])
//CHECK:                                                 ^^^^^ unsized type in constant buffer
//CHECK:                                                 ^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
{
    output[0] = values[2].get() + extra[1];
}
