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

// Third struct referencing the same `IFoo` as `Wrapped` — used below to
// exercise the `diagnosedNoTypeConformancesInterfaces` cross-site dedup:
// only one E50100 should fire for `IFoo` across `wrappedBuffer` and
// `anotherFooBuffer` even though both loads produce unresolved
// `lookupWitnessMethod` insts on `IFoo`.
struct AlsoWrapped
{
    IFoo anotherFoo;
}

// Non-entry-point helper with an interface-typed parameter. The
// walker explicitly skips functions without IREntryPointDecoration
// to avoid false positives on imported-library helpers like
// slangpy's `write_arg(IPrintable)` whose body contains an
// unresolved lookup until inlining replaces the parameter with a
// concrete type. The exhaustive-CHECK mode of this test pins that
// behaviour: if the walker were broadened to non-entry-point
// functions, this `helper.getValue()` call would emit an
// unannotated E50100 for `IUnused` and the test would fail.
interface IUnused
{
    float getValue();
}

float unusedHelper(IUnused helper)
{
    return helper.getValue();
}

StructuredBuffer<Wrapped> wrappedBuffer;
StructuredBuffer<ArrayWrapped[4]> arrayBuffer;
StructuredBuffer<AlsoWrapped> anotherFooBuffer;
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
    // Second access on the same `IFoo` interface — the dedup set must
    // suppress it so we don't re-emit.  The exhaustive-CHECK mode
    // implicitly verifies this: if a duplicate E50100 for IFoo fired
    // here, the test would fail with "unannotated diagnostic" on this
    // line.
    outputBuffer[2] = anotherFooBuffer[0].anotherFoo.getValue();
}
