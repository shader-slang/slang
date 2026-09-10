//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=A witness lookup left unresolved inside an entry point because the interface has no conforming type anywhere in the linkage is reported as E50100 at the call site, naming the interface.
//META: intent=characterization
//META: covers=source/slang/slang-ir-typeflow-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes `diagnoseUnresolvedLookupWitnesses` in
// `slang-ir-typeflow-specialize.cpp`. After type-flow propagation has run,
// that routine walks every entry-point body looking for a
// `lookup_witness_method` whose witness table operand is still symbolic. It
// only diagnoses inside entry points, on the argument that a helper function
// may still be inlined or dead-code-eliminated before codegen, so a helper
// with an unresolved lookup is not necessarily an error.
//
// Reaching it needs an interface-typed value that the compiler cannot resolve
// to any concrete type at all. A global `StructuredBuffer<IFoo>` does exactly
// that: the element type is the interface itself, so `buf[0]` produces an
// existential whose concrete type would have to come from a type conformance
// supplied by the linkage. `struct A : IFoo` is declared below but is never
// used, so the linker never brings its witness table in, and
// `collectExistentialTables` finds an empty set.
//
// Note the two halves of the behaviour this pins: the diagnostic fires from
// the *lookup* (the `.get()` call site), not from the buffer declaration, and
// the interface name is printed from the IR interface type. Adding
// `-conformance A:IFoo` to the command line makes the same source compile
// cleanly, which is what tells you the diagnostic is about the linkage and
// not about the source.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry computeMain -stage compute

interface IFoo { int get(); }

// Declared, but never referenced -- so no conformance reaches the linkage.
struct A : IFoo { int v; int get() { return v; } }

StructuredBuffer<IFoo> buf;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = buf[0].get();
//CHECK:                 ^ no type conformances found
//CHECK:                 ^ No type conformances are found for interface 'IFoo'
}
