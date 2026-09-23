//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=A MemberExpr `a.b` where the base is a scalar (`int`) with no field `foo` is rejected with the same "member not found" diagnostic as a missing struct field.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative shape for MemberExpr resolution: a non-aggregate base
// type has no fields. The diagnostic should name `foo` as not a
// member of `int`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 7;
    int y = x.foo;
//CHECK:      ^^^ member not found
//CHECK:      ^^^ 'foo' is not a member of 'int'.
// CHECK: E30027
}
