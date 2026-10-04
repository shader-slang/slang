//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that an expression position the parser cannot fill is reported as a syntax error rather than accepted.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When the parser reaches an expression position and finds no expression, it
// still has to produce a node so parsing can continue, and the position must
// be reported. The initializer of `x` is empty; the CHECK pins the syntax
// error code and its caret at the `;` that stands where the expression should
// have been.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry notMain -stage compute

void notMain()
{
    int x = ;
//CHECK:    ^ syntax error
//CHECK:    ^ syntax error.
}
