// When an interface has multiple associated types and both are passed
// to unconstrained generics, the compiler must emit error 33180 for each.
// Requested by reviewer: test with `let` instead of `var` (#9934).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain

#lang slang 2025

interface ITransform
{
    associatedtype Output;
    associatedtype Config;
    Output transform(Config c, float x);
}

struct FloatTransform : ITransform
{
    typealias Output = float;
    typealias Config = float;
    float transform(float c, float x) { return c * x; }
}

struct IntTransform : ITransform
{
    typealias Output = int;
    typealias Config = int;
    int transform(int c, float x) { return c * int(x); }
}

T processOutput<T>(T val) { return val; }
T processConfig<T>(T val) { return val; }

ITransform factory(uint id)
{
    if (id == 0) return FloatTransform();
    return IntTransform();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let obj = factory(dispatchThreadID.x);
    let config = processConfig(obj.Config());
//CHECK:                      ^ cannot specialize generic with existential type
//CHECK:                      ^ specializing 'processConfig' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
    let result = obj.transform(config, 1.0);
    let processed = processOutput(result);
//CHECK:                         ^ cannot specialize generic with existential type
//CHECK:                         ^ specializing 'processOutput' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
    outputBuffer[0] = 0;
}
