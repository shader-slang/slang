// An unconstrained generic function cannot be specialized with an
// existential type because the concrete type is not known at compile
// time. When the generic type parameter T is inferred from an
// interface-typed argument, the compiler must reject the call with
// error 33180 instead of crashing.

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

// Unconstrained generic -- T has no interface constraint.
// When called with an interface-typed argument, T is inferred as the
// extracted existential type, which cannot be statically resolved.
T identity<T>(T val) { return val; }

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

    IFoo result = identity(obj);
//CHECK:                  ^ cannot specialize generic with existential type
//CHECK:                  ^ specializing 'identity' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.

    outputBuffer[0] = result.calc(3.0);
}
