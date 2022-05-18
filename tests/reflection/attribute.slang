// attribute.slang

// Tests reflection of user defined attributes.

//TEST:REFLECTION:-stage compute -entry main -target hlsl -no-codegen

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

[MyStruct(0, 1.0)]
struct A
{
    float x;
    [DefaultValue(1)]
    float y;
};

[MyStruct(0, 2.0)]
struct B
{
    float x;
    [DefaultValue(1+1)]
    float z;
};

ParameterBlock<A> param;
ParameterBlock<B> param2;

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
}