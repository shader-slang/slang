// An explicit constant buffer whose element type has no instance fields of its own ends where
// its base struct ends, so a trailing unsized array of ordinary data in the base is diagnosed
// (E31215).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

struct UnsizedBase
{
    float4 a;
    float4 x[];
//CHECK:   ^ unsized type in constant buffer
//CHECK:   ^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
}
struct OnlyStatic : UnsizedBase
//CHECK:            ^^^^^^^^^^^ support for inheritance is unstable
{
    static int k;
}
ConstantBuffer<OnlyStatic> explicitOnlyStatic;
//CHECK:                   ^^^^^^^^^^^^^^^^^^ see constant buffer definition.

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = explicitOnlyStatic.x[2] + explicitOnlyStatic.a;
}
