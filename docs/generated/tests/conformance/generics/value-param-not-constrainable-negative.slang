//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=06568c8014909b1a1fe61ae1dd314428312f177b6412e33a4e643d370a1fc6d9
//META: purpose=A generic value parameter that is not a pack cannot carry a type constraint.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the restriction that non-pack value parameters cannot be
// constrained. The where-clause names the value parameter `N` on the left of a
// conformance constraint, a position that requires a type. CHECK asserts the
// compiler rejects it -- E30060 for the value-where-a-type-was-expected, and
// E30402 for the resulting invalid left-hand side -- proving the constraint is
// diagnosed at the declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

int constrainedValue<let N : int>() where N : IInteger { return N; }
//CHECK: E30060
//CHECK: E30402

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
