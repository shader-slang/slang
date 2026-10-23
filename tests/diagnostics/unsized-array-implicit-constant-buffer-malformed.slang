// shader-slang/slang#13530: finding the trailing unsized array of a global uniform parameter
// stops at a field whose type is an error instead of looping, and follows a struct without
// instance fields of its own into its base struct instead of dereferencing a missing field.

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
uniform OnlyStatic onlyStatic;
//CHECK:           ^^^^^^^^^^ see declaration of 'onlyStatic'

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = withErrorField.x[1] + onlyStatic.x[1] + onlyStatic.a;
}
