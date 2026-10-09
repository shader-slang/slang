// shader-slang/slang#13530: a lone unsized array of ordinary data is diagnosed as a global
// uniform parameter, even with no other uniform data after it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

uniform float4 input[];
//CHECK:       ^^^^^ unsized type in constant buffer
//CHECK:       ^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = input[3];
}
