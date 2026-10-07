//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=acb151a083ad11459c8dc1bd5b90cc232de73c42a6dc1e1771f0fabb823d2ec8
//META: purpose=An `inout` parameter requires an l-value argument; passing a literal is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the write-back half of `inout` imposes an l-value requirement on
// the argument. A literal has no storage to write back into, so the checker must
// reject the call; CHECK pins the argument-not-an-l-value error code rather than
// the message wording, which is the stable part of the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void inc(inout int x)
{
    x = x + 1;
}

void main()
{
    inc(7);
    //CHECK: error
    //CHECK: E30047
}
