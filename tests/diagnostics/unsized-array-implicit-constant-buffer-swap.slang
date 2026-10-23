// shader-slang/slang#13530: an unsized array of ordinary data is diagnosed as a global uniform
// parameter whether or not it is the last one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

uniform float4 addend;
uniform float4 input[];
//CHECK:       ^^^^^ unsized type in constant buffer
//CHECK:       ^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = input[2277] + addend;
}
