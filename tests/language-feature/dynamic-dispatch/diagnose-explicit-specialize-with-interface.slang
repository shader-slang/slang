// Explicitly specializing a constrained generic function with an
// interface type should be rejected with the same diagnostic as
// implicit inference (error 33180). Previously crashed with an
// internal compiler error (99999) on a different code path than
// the implicit case in diagnose-generic-func-with-interface.slang.

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
//CHECK:                            ^ cannot specialize generic with existential type
//CHECK:                            ^ specializing 'genericFunc' with an existential type is not allowed.

    outputBuffer[0] = result;
}
