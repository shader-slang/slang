// Test: Verify that calling extension methods on dynamically dispatched
//       interface types produces error 33180.
// Gap: Negative test for "Extensions to dynamically dispatched interface"

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

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

// Extension that adds a method to all types conforming to IFoo
// extension<T : IFoo> T is a generic extension pattern
extension<T : IFoo> T
{
    float calcDouble(float x)
    {
        return this.calc(x) * 2;
    }
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

    // Attempting to call extension method through dynamic dispatch
    // CHECK: ([[# @LINE+1]]): error 33180
    float result = obj.calcDouble(3.0);

    outputBuffer[0] = result;
}
