//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=26c797434ab51700cf84e595593cca11917ffceaff2d95c7a0504524e997316e
//META: purpose=Verifies that the InfixExpr compound assignment `*=` updates the l-value in place, parallel to the documented `+=` case.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis for compound assignment: cover a second operator
// flavor (`*=`) beyond the smoke-test `+=`, to confirm the family
// is uniform.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 5;
    x *= 3;
    // CHECK: x=15
    printf("x=%d\n", x);
}
