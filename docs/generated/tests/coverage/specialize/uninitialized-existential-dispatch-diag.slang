//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T14:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization
//META: doc_section_digest=f9a2d5320ce13ef68d98ed961e6a7c6f951e9c3524d9cf700922c296e85c0f83
//META: purpose=Dynamic dispatch on an interface value left uninitialized on one control-flow path is rejected with E50101.
//META: intent=characterization
//META: covers=source/slang/slang-ir-typeflow-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes analyzeExtractExistentialWitnessTable: when the tagged-union
// witness-table set of an interface object contains an uninitialized element
// (one path assigns a conformer, the other leaves it default), a dynamic
// dispatch on it emits DynamicDispatchOnPotentiallyUninitializedExistential
// (E50101). Here `f` is assigned only when `tid.x == 0`; the call `f.get()`
// dispatches dynamically on the possibly-uninitialized object.
// The diagnostic fires twice on this target; non-exhaustive matches the form.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry computeMain -stage compute

interface IFoo { int get(); }
struct A : IFoo { int x; int get() { return x; } }

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IFoo f;
    if (tid.x == 0) { A a; a.x = 1; f = a; }
    outputBuffer[0] = f.get();
}
//CHECK: E50101
//CHECK: Cannot dynamically dispatch on potentially uninitialized interface object 'IFoo'
