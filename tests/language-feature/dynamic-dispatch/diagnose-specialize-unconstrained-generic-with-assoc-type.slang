// When the return value of an interface method (whose type is an associated
// type) is passed to an unconstrained generic function, the type parameter
// cannot be statically resolved. The compiler must emit error 33180 instead
// of crashing.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain

#lang slang 2025

interface ITransform
{
    associatedtype Output;
    Output transform(float x);
}

struct FloatTransform : ITransform
{
    typealias Output = float;
    float transform(float x) { return x * 2; }
}

struct IntTransform : ITransform
{
    typealias Output = int;
    int transform(float x) { return int(x); }
}

// Unconstrained generic -- T is inferred from the argument
T process<T>(T val) { return val; }

ITransform factory(uint id)
{
    if (id == 0) return FloatTransform();
    return IntTransform();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ITransform obj = factory(dispatchThreadID.x);
    let result = obj.transform(1.0);
    let processed = process(result);
//CHECK:                   ^ cannot specialize generic with existential type
//CHECK:                   ^ specializing 'process' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
    outputBuffer[0] = 0;
}
