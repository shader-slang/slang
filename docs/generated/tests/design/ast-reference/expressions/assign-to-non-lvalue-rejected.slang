//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=An AssignExpr whose left side is not an l-value is rejected with a diagnostic about "left of '=' is not an l-value".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// AssignExpr requires its left to be an l-value. The expression
// `1 + 2` is a temporary, not an l-value; assignment to it must be
// diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    1 + 2 = 7;
//CHECK:  ^ left of '=' is not an l-value
//CHECK:  ^ left of '=' is not an l-value.
// CHECK: E30011
}
