//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=Verifies that a `.` member access on a vector base reaches the numbered component rather than being looked up as a struct field.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A dot access is resolved against the base's type, and on a vector the base
// has no declared members, so the access has to be reinterpreted as a
// component selection. Each of the three components is read individually; the
// CHECKs assert the distinct values, so a component selection that resolved to
// the wrong index fails.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float3 v = float3(1.0f, 2.0f, 3.0f);
    // CHECK: x=1 y=2 z=3
    printf("x=%d y=%d z=%d\n", int(v.x), int(v.y), int(v.z));
}
