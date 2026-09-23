//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-queries
//META: doc_section_digest=c04a621bcf00069773d305dfdf4582e5e2e0ce23042884e2675ec84c546547fe
//META: purpose=`optional nonempty(...)` parses but is rejected as an invalid constraint.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's remark that the `optional` modifier is parsed in front of
// a `nonempty(...)` guard but is not a meaningful constraint. The declaration
// below spells exactly that combination. CHECK asserts E30415 fires on it, so
// the form is diagnosed rather than silently treated as an ordinary
// non-emptiness requirement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

int maybeNonempty<each T>() where optional nonempty(T) { return 1; }
//CHECK: E30415

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
