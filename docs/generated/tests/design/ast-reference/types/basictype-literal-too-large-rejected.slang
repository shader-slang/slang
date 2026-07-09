//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=An integer literal too large to be represented in any integer BasicExpressionType is diagnosed with an out-of-range error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative boundary probe: a literal larger than the largest integer
// scalar BasicExpressionType can hold cannot be represented in any
// integer type. The compiler diagnoses the out-of-range literal;
// CHECK pins the too-large-literal error on the offending line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    uint x = 99999999999999999999;
//CHECK:     ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type
//CHECK:     ^^^^^^^^^^^^^^^^^^^^ integer constant overflow in conversion
}
