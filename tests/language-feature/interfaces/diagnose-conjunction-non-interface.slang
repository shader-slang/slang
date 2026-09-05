// A non-interface operand of an interface conjunction `&` must be rejected with E30307. Before the
// ExistentialType work enforced the operand rule (`checkInterfaceOrConjunctionType`), a conjunction
// with a non-interface operand was accepted silently.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    float f();
}

typealias Bad = IFoo & int;
//CHECK: expected an interface or a conjunction of interfaces

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 0.0;
}
