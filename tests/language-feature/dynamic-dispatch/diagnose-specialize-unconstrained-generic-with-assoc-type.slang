// When the return value of an interface method (whose type is an associated
// type) is passed to an unconstrained generic function, the type parameter
// cannot be statically resolved. The compiler should emit error 33180, but
// currently produces an internal compiler error (99999) instead.

// Disabled: ICE -- should emit error 33180 but crashes with error 99999
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv

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
    var result = obj.transform(1.0);
    // CHECK: error 33180
    let processed = process(result);
    outputBuffer[0] = 0;
}
