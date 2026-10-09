// shader-slang/slang#13530: uniform entry-point parameters share an implicit constant
// buffer, so an unsized array of ordinary data among them is diagnosed like a global one. A
// compute parameter without a system-value semantic is uniform too (E38040).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

struct Mixed
{
    Texture2D t;
    float4 v;
}

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uniform float4 input[],
//CHECK:           ^^^^^ unsized type in constant buffer
//CHECK:           ^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
    uniform float4 addend,
    uniform Mixed mixed[],
//CHECK:          ^^^^^ unsized type in constant buffer
//CHECK:          ^^^^^ cannot use unsized type 'Mixed[]' in a constant buffer.
    uniform float4* pointers[],
//CHECK:            ^^^^^^^^ unsized type in constant buffer
//CHECK:            ^^^^^^^^ cannot use unsized type 'Ptr<vector<float,4>
    float4 autoValues[])
//CHECK:   ^^^^^^^^^^ entry point parameter treated as uniform
//CHECK:   ^^^^^^^^^^ parameter 'autoValues' is treated as 'uniform' because it does not have a system-value semantic.
//CHECK:   ^^^^^^^^^^ unsized type in constant buffer
//CHECK:   ^^^^^^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
{
    output[0] = input[3] + addend + mixed[2].v + *pointers[1] + autoValues[1];
}
