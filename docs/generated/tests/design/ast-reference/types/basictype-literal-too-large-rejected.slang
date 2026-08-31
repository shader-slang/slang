//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=eb9e797ca9c3dbb949a62072e232852c0f820f49dd331388c5e7ffcde24a8c1c
//META: purpose=An integer literal too large to be represented in any integer BasicExpressionType is diagnosed rather than silently truncated.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative boundary probe past the top of every integer arithmetic
// leaf: no scalar built-in can hold the literal, so the checker must
// reject it instead of wrapping. CHECK pins the too-large-literal
// diagnostic and the conversion-overflow companion on the same span.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    uint x = 99999999999999999999;
//CHECK:     ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type
//CHECK:     ^^^^^^^^^^^^^^^^^^^^ integer constant overflow in conversion
}
