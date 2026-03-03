// Generic functions constrained by an interface require compile-time type
// resolution to specialize. When called with an existential (interface-typed)
// value, the associated types cannot be statically determined, so the compiler
// must reject the specialization with error 33180.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

#lang slang 2025

interface ICalculation
{
    associatedtype Data;
    float calc(Data d, float x);
    Data make(float q);
}

struct A : ICalculation
{
    typealias Data = float;
    float calc(Data d, float x) { return d * x * x; }
    Data make(float q) { return q; }
}

struct BData
{
    float x;
    float y;
}

struct B : ICalculation
{
    typealias Data = BData;
    float calc(Data d, float x) { return d.x * x * x + d.y; }
    Data make(float q) { return {q, q}; }
}

// This generic function uses the associated type T.Data internally.
// When T is an existential, T.Data cannot be statically resolved.
float processCalc<T : ICalculation>(T obj, float x)
{
    T.Data d = obj.make(x);
    return obj.calc(d, x);
}

ICalculation factory(uint id)
{
    if (id == 0) return A();
    return B();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let obj = factory(dispatchThreadID.x);
    // CHECK: ([[# @LINE+1]]): error 33180
    outputBuffer[0] = processCalc(obj, 3.0);
}
