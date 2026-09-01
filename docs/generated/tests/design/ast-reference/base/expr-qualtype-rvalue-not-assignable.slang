//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=369743522fa835d335b8cfc1d7160c48df288006a4cdb5149e41fdce6b476f7e
//META: purpose=The type an Expr carries records l-value/r-value information as well as the type itself, so assigning to a call result is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the checked type of an expression is more than a type:
// it also says whether the expression denotes storage. A call result
// is an r-value, so it cannot appear on the left of `=`. CHECK pins
// E30011 and puts the caret on the `=` token the checker reports.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int makeValue() { return 3; }

void main()
{
    makeValue() = 5;
//CHECK:        ^ left of '=' is not an l-value
//CHECK:        ^ left of '=' is not an l-value.
// CHECK: E30011
}
