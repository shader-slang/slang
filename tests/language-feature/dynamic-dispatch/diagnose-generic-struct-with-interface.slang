// User-defined generic structs cannot be specialized with interface types
// because the concrete type is not known at compile time.

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

// User-defined generic struct that uses its type parameter
struct Wrapper<T : IFoo>
{
    T value;

    float compute(float x)
    {
        return value.calc(x);
    }
}

// Generic function that creates and uses the wrapper
float wrapAndCompute<T : IFoo>(T item, float x)
{
    Wrapper<T> wrapper;
    wrapper.value = item;
    return wrapper.compute(x);
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

    // Attempting to specialize generic struct via generic function with interface type
    // CHECK: ([[# @LINE+1]]): error 33180
    float result = wrapAndCompute(obj, 3.0);

    outputBuffer[0] = result;
}
