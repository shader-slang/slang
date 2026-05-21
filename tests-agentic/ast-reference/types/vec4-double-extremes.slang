//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<double, 4> survives lane construction with extreme magnitudes; a comparison >1e30 holds per-lane.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: float-edge value probe at the double4 corner.
// Large magnitudes (1e100) are representable in double per lane
// and the comparison >1e30 returns true per-lane.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double4 v = double4(1e100lf, -1e100lf, 0.0lf, 1.0lf);
    int big0 = (v.x > 1e30lf) ? 1 : 0;
    int big1 = (v.y < -1e30lf) ? 1 : 0;
    int small = (v.z == 0.0lf) ? 1 : 0;
    // CHECK: r=1,1,1
    printf("r=%d,%d,%d\n", big0, big1, small);
}
