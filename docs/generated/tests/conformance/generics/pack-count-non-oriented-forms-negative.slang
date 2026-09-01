//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=Pack-count constraints are oriented: `N == countof(T)`, `countof(T) != N`, and `countof(T) >= N` are all rejected as pack-count constraints.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the orientation and operator restrictions the doc lists as "not pack
// count constraints". Three declarations spell the constraint the wrong way
// round or with the wrong comparison operator. CHECK asserts E30437 for the
// reversed form (countof must be on the left) and E30436 for each non-`==`
// operator, proving none of the three is accepted as a pack-count constraint.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void reversed<let N : int, each T>() where N == countof(T) {}
//CHECK: E30437

void notEqual<let N : int, each T>() where countof(T) != N {}
//CHECK: E30436

void greaterEqual<let N : int, each T>() where countof(T) >= N {}
//CHECK: E30436

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
