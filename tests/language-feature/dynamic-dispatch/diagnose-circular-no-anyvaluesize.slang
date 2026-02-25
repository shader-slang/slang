// Circular interface conformance without [anyValueSize] should produce a clear
// error, not an internal compiler error.
//
// When no [anyValueSize] is specified, the compiler attempts to infer the
// AnyValue size from conforming types. A circular dependency (struct conforms
// to an interface and contains a field of that interface type) causes the
// compiler to crash during SPIRV emission with an unhandled lookupWitness inst.
//
// Currently this test is DISABLED because it causes an ICE (error 99999).
// Once the compiler detects circular dependencies during semantic checking
// (before IR emission), this test should be enabled and verify that a proper
// diagnostic is emitted.

//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv

// When fixed, uncomment and adjust the error check:
// CHECK: error {{[0-9]+}}

interface IFoo
{
    float getValue();
}

// This struct causes the crash: it conforms to IFoo and contains an IFoo field.
// Without [anyValueSize], the compiler tries to compute the AnyValue size but
// hits infinite recursion in the witness lookup during SPIRV emission.
struct BadImpl : IFoo
{
    IFoo child;  // Circular: BadImpl conforms to IFoo and contains IFoo
    float getValue() { return child.getValue(); }
}

IFoo factory(uint id)
{
    return BadImpl();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo obj = factory(0);
    outputBuffer[0] = obj.getValue();
}
