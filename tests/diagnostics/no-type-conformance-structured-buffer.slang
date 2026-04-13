//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// Test that loading from StructuredBuffer<IInterface> without -conformance flags
// produces a clean E50100 diagnostic instead of crashing (issue #10519).

interface IFoo
{
    float getValue();
}

struct Impl : IFoo
{
    float v;
    float getValue() { return v; }
}

StructuredBuffer<IFoo> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = inputBuffer[0].getValue();
//CHECK:                         ^ no type conformances found
//CHECK:                         ^ No type conformances are found for interface 'IFoo'. Code generation for current target requires at least one implementation type present in the linkage.
}
