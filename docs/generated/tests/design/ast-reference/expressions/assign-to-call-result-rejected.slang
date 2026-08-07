//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=An AssignExpr with the return value of a non-ref-returning function call on the LHS is rejected with "left of '=' is not an l-value".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative shape distinct from `1 + 2 = 7`: the LHS is an InvokeExpr
// returning a temporary, which is not an l-value. The diagnostic
// should still be the canonical l-value error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int foo() { return 1; }

void main()
{
    foo() = 7;
//CHECK:  ^ left of '=' is not an l-value
//CHECK:  ^ left of '=' is not an l-value.
// CHECK: E30011
}
