//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=An ExplicitCastExpr from a struct to bool is rejected because no conversion exists — a distinct destination type from the existing struct-to-int negative case.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Different destination type than the sibling cast-incompatible-rejected
// negative test. The TypeCastExpr family must reject struct->bool too.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S { int a; }

void main()
{
    S s;
    bool x = (bool) s;
//CHECK:            ^ type mismatch in expression
//CHECK:            ^ expected an expression of type 'bool', got 'S'
// CHECK: E30019
}
