//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=a8293cbfbfbb580f987e71889bbd1c8f5697b1440fa8a8bc081a878f12d3aabf
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
