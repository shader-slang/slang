//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=A MemberExpr `a.b` accesses a field on a value-typed base; the read returns the field's value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `a.b` on a value-typed base resolves to the named field and
// reads its stored value. Two distinct fields are written and then read back
// through separate member accesses, so the CHECKs fail if either access
// resolved to the wrong field.

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
