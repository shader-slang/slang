// shader-slang/slang#13530: a generic element type `T` in `uniform T values[]` is checked after
// specialization. A type argument with ordinary data is diagnosed; a resource type argument
// stays valid (see unsized-array-implicit-constant-buffer-generic-valid.slang).

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
void main<T : IValue>(uniform T values[])
//CHECK:                        ^^^^^^ unsized type in constant buffer
//CHECK:                        ^^^^^^ cannot use unsized type 'Data[]' in a constant buffer.
{
    output[0] = values[2].get();
}
