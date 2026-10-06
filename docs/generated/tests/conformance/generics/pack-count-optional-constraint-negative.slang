//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-12T00:00:00+00:00
//META: source_commit=d592afa9b9efaf43740554b05a3fdf5b6f6e5077
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=An optional countof(...) pack-count constraint is rejected with E30434; optionality is not meaningful for a pack-count constraint.
//META: intent=negative
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `optional` is accepted on a type constraint (see the "Optional type
// constraint" example in the spec), so the spelling parses here; what fails is
// the combination. A pack count is either provably equal to the required count
// or it is not, so there is no "absent witness" case for optionality to
// describe, and the checker rejects the modifier outright.
//
// Note that the spec's "Pack count constraints" section does not mention
// `optional` at all -- see the doc gap recorded in this bundle's README.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void optionalCount<each T>() where optional countof(T) == 2 {}
//CHECK: E30434

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
