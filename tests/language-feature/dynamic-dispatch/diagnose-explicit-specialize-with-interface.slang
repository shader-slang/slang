// Explicitly specializing a constrained generic function with an
// interface type should be rejected with the same diagnostic as
// implicit inference (error 33180). Currently crashes with an
// internal compiler error (99999) on a different code path than
// the implicit case in diagnose-generic-func-with-interface.slang.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

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

    // Explicit specialization with an interface type should be rejected
    // the same way implicit inference is (error 33180).
    // CHECK: error[E33180]
    float result = genericFunc<IFoo>(obj, 3.0);

    outputBuffer[0] = result;
}
