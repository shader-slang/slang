// Specializing a generic entry point with an interface type, as `-specialize IFoo` does for
// `computeMain<I : IFoo>`, leaves no witness table for the constraint, so the entry point cannot
// be specialized. Linking used to crash on the failed specialization; it now reports the same
// error as other generic specializations with an existential type (E33180).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain -specialize IFoo
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -specialize IFoo
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -stage compute -entry computeMain -specialize IFoo

interface IFoo
{
    float get();
}

struct A : IFoo
{
    float v;
    float get() { return v; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain<I : IFoo>(uint3 tid: SV_DispatchThreadID, uniform I foo)
/*CHECK:
     ^^^^^^^^^^^ error E33180
     ^^^^^^^^^^^ specializing 'computeMain' with an existential type is not allowed.
*/
{
    outputBuffer[0] = foo.get();
}
