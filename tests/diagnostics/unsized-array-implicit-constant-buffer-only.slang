// shader-slang/slang#13530: a lone unsized array of ordinary data is diagnosed as a global
// uniform parameter, even with no other uniform data after it, and so is one declared without
// `uniform`. The rule does not depend on the target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry main

uniform float4 input[];
//CHECK:       ^^^^^ unsized type in constant buffer
//CHECK:       ^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.

float4 bare[];
/*CHECK:
       ^^^^ unsized type in constant buffer
       ^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
*/

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = input[3] + bare[1];
}
