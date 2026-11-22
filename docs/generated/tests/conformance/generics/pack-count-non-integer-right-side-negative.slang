//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-12T00:00:00+00:00
//META: source_commit=d592afa9b9efaf43740554b05a3fdf5b6f6e5077
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=The right side of a countof(...) pack-count constraint must be a compile-time integer expression; a float operand is rejected with E30432.
//META: intent=negative
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The spec requires that "the right side must be a compile-time integer
// expression". `float(2)` is compile-time but not integral, which separates
// the integer requirement from the compile-time requirement: this is rejected
// for its type, not for being non-constant.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void nonIntegerCount<each T>() where countof(T) == float(2) {}
//CHECK: E30432

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
