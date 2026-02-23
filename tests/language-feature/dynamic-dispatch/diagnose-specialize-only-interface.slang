// Interfaces marked with [Specialize] are intended for compile-time specialization
// only. When the compiler cannot resolve the concrete type and would need to generate
// dynamic dispatch code (witness table switch), it must reject the program with
// error 52008 rather than silently producing dynamic dispatch for such interfaces.

// Disabled: requires compiler fix to emit error 52008 for [Specialize] interfaces
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv -gnone

#lang slang 2025

[Specialize]
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

IFoo makeInterface(uint id)
{
    if (id == 0)
        return A();
    return B();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo obj = makeInterface(dispatchThreadID.x);
    // CHECK: ([[# @LINE+1]]): error 52008
    outputBuffer[0] = obj.calc(3.0);
}
