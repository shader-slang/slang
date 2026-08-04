//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=a8293cbfbfbb580f987e71889bbd1c8f5697b1440fa8a8bc081a878f12d3aabf
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
