// shader-slang/slang#13530: an interface value is stored as an existential, which is ordinary
// data, so an unsized array of an interface type is diagnosed as a uniform parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

interface IValue
{
    float4 get();
}

uniform IValue globalValues[];
//CHECK:       ^^^^^^^^^^^^ unsized type in constant buffer
//CHECK:       ^^^^^^^^^^^^ cannot use unsized type 'IValue[]' in a constant buffer.

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform IValue values[])
//CHECK:                 ^^^^^^ unsized type in constant buffer
//CHECK:                 ^^^^^^ cannot use unsized type 'IValue[]' in a constant buffer.
{
    output[0] = values[2].get() + globalValues[1].get();
}
