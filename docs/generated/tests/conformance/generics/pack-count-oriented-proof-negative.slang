//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A declared `countof(T) == countof(U)` constraint is not used backwards to prove a callee requirement stated about U.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the oriented proof matters while checking an unspecialized
// generic body. The caller declares `countof(T) == countof(U)` and calls a
// callee whose own requirement, after substitution, reads `countof(U) ==
// countof(T)` -- the same equation written the other way round. CHECK asserts
// E30435 fires on that call, proving the in-scope constraint is not read
// backwards to discharge the callee's requirement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void callee<each A, each B>() where countof(A) == countof(B) {}

void callerBad<each T, each U>() where countof(T) == countof(U)
{
    callee<U, T>();
    //CHECK: E30435
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    callerBad<int, float>();
    outputBuffer[0] = 0;
}
