// attribute.slang

// Tests reflection of user defined attributes.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl

[__AttributeUsage(_AttributeTargets.Struct)]
struct MyStructAttribute
{
    int iParam;
    float fParam;
};
[__AttributeUsage(_AttributeTargets.Var)]
struct DefaultValueAttribute
{
    int iParam;
};

[MyStruct(0, "stringVal")] // attribute arg type mismatch
//CHECK:                ^ expected an expression of type 'float', got 'String'
struct A
{
    [MyStruct(0, 10.0)] // attribute does not apply to this construct
/*CHECK:
     ^^^^^^^^ invalid attribute placement
     ^^^^^^^^ attribute 'MyStruct' is not valid here
*/
    float x;
    [DefaultValue(2.0)] // attribute arg type mismatch
//CHECK:          ^^^ implicit conversion not recommended
//CHECK:          ^^^ implicit conversion from 'float' to 'int' is not recommended
    float y;
};

ParameterBlock<A> param;

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
