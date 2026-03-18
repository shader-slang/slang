// An unconstrained generic function cannot be specialized with an
// existential type because the concrete type is not known at compile
// time. When the generic type parameter T is inferred from an
// interface-typed argument, the compiler must reject the call.
//
// Unlike the constrained case (T : IFoo), where the compiler
// recognizes the pattern and emits error 33180, unconstrained generics
// currently crash with an internal compiler error (99999) during
// type-flow specialization instead of producing a diagnostic.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

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

    // Inferring T from an interface-typed argument requires specializing
    // the generic with the existential type, which is not allowed.
    // CHECK: error[E33180]
    IFoo result = identity(obj);

    outputBuffer[0] = result.calc(3.0);
}
