//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

// Test that loading from StructuredBuffer<IInterface> and RWStructuredBuffer<IInterface>
// without -conformance flags produces a clean E50100 diagnostic instead of crashing (issue #10519).

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
RWStructuredBuffer<IFoo> rwInputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = inputBuffer[0].getValue();
//CHECK:                         ^ no type conformances found
//CHECK:                         ^ No type conformances are found for interface 'IFoo'. Code generation for current target requires at least one implementation type present in the linkage.
    outputBuffer[1] = rwInputBuffer[0].getValue();
//CHECK:                                       ^ no type conformances found
//CHECK:                                       ^ No type conformances are found for interface 'IFoo'. Code generation for current target requires at least one implementation type present in the linkage.
}
