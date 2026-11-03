// A `[Differentiable]` generic-extension method called through an interface-typed value
// that can hold more than one conforming type cannot be specialized statically, so the
// call is rejected, as it is for the same method without `[Differentiable]`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain

#lang slang 2026

RWStructuredBuffer<float> outputBuffer;

interface IValue
{
    float get();
}

struct A : IValue
{
    float get() { return 3.0; }
}

struct B : IValue
{
    float get() { return 5.0; }
}

extension<T : IValue> T
{
    [Differentiable]
    [BackwardDerivative(read_bwd)]
    float read() { return this.get(); }
    void read_bwd(float grad) {}
}

IValue makeValue(uint id)
{
    if (id == 0)
        return A();
    return B();
}

float callRead(IValue v)
{
    return v.read();
//CHECK:         ^ error E33180
//CHECK:         ^ specializing 'read' with an existential type is not allowed.
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = callRead(makeValue(tid.x));
}
