// Explicitly specializing a constrained generic function with an interface type must be
// rejected: the argument `IFoo` denotes the existential `dyn IFoo`, which does not conform to
// `IFoo` (shader-slang/slang#12430).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain

#lang slang 2025

interface IFoo
{
    float calc(float x);
}

struct A : IFoo
{
    float calc(float x) { return x * x; }
}

struct B : IFoo
{
    float calc(float x) { return x * 2; }
}

float genericFunc<T : IFoo>(T obj, float x)
{
    return obj.calc(x);
}

IFoo makeInterface(uint id)
{
    if (id == 0)
        return A();
    else
        return B();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo obj = makeInterface(dispatchThreadID.x);

    float result = genericFunc<IFoo>(obj, 3.0);
//CHECK:           ^^^^^^^^^^^ error E38029
//CHECK:           ^^^^^^^^^^^ type argument 'dyn IFoo' does not conform to the required interface 'IFoo'

    outputBuffer[0] = result;
}
