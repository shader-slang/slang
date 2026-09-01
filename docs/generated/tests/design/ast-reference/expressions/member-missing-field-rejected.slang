//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=A MemberExpr `a.b` where the base type does not have a member `b` is rejected with a "member not found" diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// MemberExpr's name lookup on the base type produces an error when
// the named member doesn't exist on the base. We name a non-existent
// field `zzz` on a struct that only has `x` and `y`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Pt { int x; int y; }

void main()
{
    Pt p;
    int z = p.zzz;
//CHECK:      ^^^ member not found
//CHECK:      ^^^ 'zzz' is not a member of 'Pt'.
// CHECK: E30027
}
