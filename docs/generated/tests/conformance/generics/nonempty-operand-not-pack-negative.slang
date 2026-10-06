//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-queries
//META: doc_section_digest=c04a621bcf00069773d305dfdf4582e5e2e0ce23042884e2675ec84c546547fe
//META: purpose=The operand of `nonempty(...)` must be a direct reference to a generic pack parameter; a concrete type is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand restriction on the non-emptiness guard. `nonempty(int)`
// names a concrete type instead of a pack parameter declared by this generic,
// so there is no pack whose emptiness could be constrained. CHECK asserts
// E30413 fires on the declaration, proving the guard only accepts a direct
// reference to a type or value pack parameter of the current generic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

int notAPack<each T>() where nonempty(int) { return 1; }
//CHECK: E30413

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
