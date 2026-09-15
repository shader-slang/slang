// Positive proof that a namespace-qualified attribute binds the *correct* attribute, not merely
// "some" resolution. Applying `my_namespace::Example` with the wrong argument count triggers ITS
// argument-count check — which can only fire once resolution found `ExampleAttribute` and its
// `int iParam` parameter, i.e. it did not fall through to the 'unknown attribute' path. See
// shader-slang/slang#12668.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -no-codegen

namespace my_namespace
{
    [__AttributeUsage(_AttributeTargets.Function)]
    struct ExampleAttribute
    {
        int iParam;
    };
}

// CHECK: attribute 'my_namespace_Example' expects 1 arguments (0 provided)
[my_namespace::Example()]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
