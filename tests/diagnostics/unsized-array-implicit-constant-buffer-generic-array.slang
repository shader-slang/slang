// shader-slang/slang#13530: an entry-point parameter declared with a generic type can become
// an unsized array of ordinary data by specialization, so it is checked after specialization.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -specialize float4[]

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main<A : IArray<float4>>(uniform A values)
//CHECK:                                ^^^^^^ unsized type in constant buffer
//CHECK:                                ^^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
{
    output[0] = values[1];
}
