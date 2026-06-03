//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<double, 2>: +0.0lf and -0.0lf compare numerically equal at the lane level.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: double-edge value probe at the double2 corner.
// Both +0.0 and -0.0 are valid double values and both compare
// equal to 0 numerically at the lane level.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double2 v = double2(0.0lf, -0.0lf);
    int eqx = (v.x == 0.0lf) ? 1 : 0;
    int eqy = (v.y == 0.0lf) ? 1 : 0;
    // CHECK: eq=1,1
    printf("eq=%d,%d\n", eqx, eqy);
}
