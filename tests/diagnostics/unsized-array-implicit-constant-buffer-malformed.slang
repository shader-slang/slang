// shader-slang/slang#13530: finding the trailing unsized array of a global uniform parameter
// stops at a field whose type is an error, and at a struct whose only own members are
// `static`, instead of looping or dereferencing a missing field.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

struct WithErrorField
{
    float4 x[];
//CHECK:   ^ unsized member must be last
//CHECK:   ^ member with unknown size at compile time can only appear as the last member in a composite type.
    Undefined y;
/*CHECK:
    ^^^^^^^^^ undefined identifier
    ^^^^^^^^^ undefined identifier 'Undefined'.
*/
}
uniform WithErrorField withErrorField;

struct Base
{
    float4 a;
}
struct OnlyStatic : Base
//CHECK:            ^^^^ support for inheritance is unstable
{
    static int k;
}
uniform OnlyStatic onlyStatic;

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = withErrorField.x[1] + onlyStatic.a;
}
