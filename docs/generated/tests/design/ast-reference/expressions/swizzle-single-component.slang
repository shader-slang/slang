//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
//META: purpose=Verifies that a single-element SwizzleExpr (`v.y`) carries a one-entry elementIndices ShortList and extracts the corresponding scalar component.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for SwizzleExpr.elementIndices: a single
// index. `v.y` on (1,2,3) selects the scalar 2.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float3 v = float3(1.0f, 2.0f, 3.0f);
    float y = v.y;
    // CHECK: y=2
    printf("y=%d\n", int(y));
}
