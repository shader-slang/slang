//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// Extends the E50100 diagnostic coverage of `no-type-conformance-structured-buffer.slang`
// to interface types reached *transitively* — through a struct field or an
// array element — rather than only when the buffer's direct element type is
// an interface.  Without the post-specialization `lookupWitnessMethod` check,
// these paths ICE in codegen with "Unhandled local inst: lookupWitness".
// See #9445.
//
// Each case uses a distinct interface so the cross-site dedup
// (`diagnosedNoTypeConformancesInterfaces`) doesn't suppress the diagnostic
// on the second access — we want each nesting shape to verify independently.

interface IFoo
{
    float getValue();
}

interface IBar
{
    float getValue();
}

struct Wrapped
{
    IFoo foo;
    float extra;
}

struct ArrayWrapped
{
    IBar bar;
}

StructuredBuffer<Wrapped> wrappedBuffer;
StructuredBuffer<ArrayWrapped[4]> arrayBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = wrappedBuffer[0].foo.getValue();
//CHECK:                                           ^ no type conformances found
//CHECK:                                           ^ No type conformances are found for interface 'IFoo'. Code generation for current target requires at least one implementation type present in the linkage.
    outputBuffer[1] = arrayBuffer[0][2].bar.getValue();
//CHECK:                                            ^ no type conformances found
//CHECK:                                            ^ No type conformances are found for interface 'IBar'. Code generation for current target requires at least one implementation type present in the linkage.
}
