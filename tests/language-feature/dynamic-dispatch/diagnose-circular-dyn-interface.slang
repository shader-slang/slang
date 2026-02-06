// Circular dependencies with `dyn interface` should produce a clear error.
//
// Currently this test is DISABLED because it causes an internal compiler error
// (ICE 99999: "unimplemented: Unhandled local inst in spirv-emit").
//
// Once the compiler is fixed to detect circular dependencies during semantic
// checking (before IR emission), this test should be enabled and verify that
// a proper diagnostic is emitted.
//
// Bug: Circular interface conformance with dyn interface crashes compiler
// Expected: Clear error like "circular dependency in interface conformance"

//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv

// When fixed, uncomment and adjust the error check:
// CHECK: error {{[0-9]+}}

dyn interface IFoo
{
    float getValue();
}

// This struct causes the crash: it conforms to IFoo and contains an IFoo field.
// When getValue() is called, it triggers a witness lookup for the unresolved
// interface method, which the SPIRV emitter cannot handle.
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
