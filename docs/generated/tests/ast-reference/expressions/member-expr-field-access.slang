//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=a8293cbfbfbb580f987e71889bbd1c8f5697b1440fa8a8bc081a878f12d3aabf
//META: purpose=A MemberExpr `a.b` accesses a field on a value-typed base; the read returns the field's value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says MemberExpr represents `a.b` where `a` is a value. The
// checker resolves `.x` and `.y` to the respective field declarations
// and reads return the stored values.

//TEST:INTERPRET(filecheck=CHECK):

struct Pt { int x; int y; }

void main()
{
    Pt p = {3, 4};
    // CHECK: x=3
    printf("x=%d\n", p.x);
    // CHECK: y=4
    printf("y=%d\n", p.y);
}
