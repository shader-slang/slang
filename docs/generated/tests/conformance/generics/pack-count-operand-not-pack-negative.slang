//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=The operand of the left-side `countof(...)` must be a direct reference to a generic pack parameter; a non-pack value parameter is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand restriction on the constrained side of a pack-count
// constraint. `countof(N)` names an ordinary (non-pack) value parameter, which
// is not a pack the constraint can describe. CHECK asserts E30430 fires on the
// declaration, proving the restriction is enforced where the constraint is
// written rather than deferred to a use site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

int notAPack<let N : int>() where countof(N) == 2 { return N; }
//CHECK: E30430

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
