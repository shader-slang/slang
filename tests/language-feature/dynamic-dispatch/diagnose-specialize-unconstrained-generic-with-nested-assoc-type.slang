// When an associated type is looked up through a nested chain of interfaces
// (e.g. outer.Inner.Value), the compiler must still emit error 33180.
// This exercises the recursive existential-derived detection (#9934).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain

#lang slang 2025

interface IInner
{
    associatedtype Value;
    Value make();
}

interface IOuter
{
    associatedtype Inner : IInner;
    Inner getInner();
}

struct FloatInner : IInner
{
    typealias Value = float;
    float make() { return 1.0; }
}

struct IntInner : IInner
{
    typealias Value = int;
    int make() { return 1; }
}

struct MyOuter : IOuter
{
    typealias Inner = FloatInner;
    FloatInner getInner() { return FloatInner(); }
}

struct MyOuter2 : IOuter
{
    typealias Inner = IntInner;
    IntInner getInner() { return IntInner(); }
}

T process<T>(T val) { return val; }

IOuter factory(uint id)
{
    if (id == 0) return MyOuter();
    return MyOuter2();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let outer = factory(dispatchThreadID.x);
    let inner = outer.getInner();
    let val = inner.make();
    let result = process(val);
//CHECK:                ^ cannot specialize generic with existential type
//CHECK:                ^ specializing 'process' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time.
    outputBuffer[0] = 0;
}
