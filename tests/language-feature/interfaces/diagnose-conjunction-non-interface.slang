// A non-interface operand of an interface conjunction `&` must be rejected as not an interface.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    float f();
}

typealias Bad = IFoo & int;
// The `diag=CHECK` harness matches the diagnostic message text, which is unique to E30307.
//CHECK: expected an interface or a conjunction of interfaces

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 0.0;
}
